/* inflate.c -- zlib decompression
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * Change history:
 *
 * 1.2.beta0    24 Nov 2002
 * - First version -- complete rewrite of inflate to simplify code, avoid
 *   creation of window when not needed, minimize use of window when it is
 *   needed, make inffast.c even faster, implement gzip decoding, and to
 *   improve code readability and style over the previous zlib inflate code
 *
 * 1.2.beta1    25 Nov 2002
 * - Use pointers for available input and output checking in inffast.c
 * - Remove input and output counters in inffast.c
 * - Change inffast.c entry and loop from avail_in >= 7 to >= 6
 * - Remove unnecessary second byte pull from length extra in inffast.c
 * - Unroll direct copy to three copies per loop in inffast.c
 *
 * 1.2.beta2    4 Dec 2002
 * - Change external routine names to reduce potential conflicts
 * - Correct filename to inffixed.h for fixed tables in inflate.c
 * - Make hbuf[] unsigned char to match parameter type in inflate.c
 * - Change strm->next_out[-state->offset] to *(strm->next_out - state->offset)
 *   to avoid negation problem on Alphas (64 bit) in inflate.c
 *
 * 1.2.beta3    22 Dec 2002
 * - Add comments on state->bits assertion in inffast.c
 * - Add comments on op field in inftrees.h
 * - Fix bug in reuse of allocated window after inflateReset()
 * - Remove bit fields--back to byte structure for speed
 * - Remove distance extra == 0 check in inflate_fast()--only helps for lengths
 * - Change post-increments to pre-increments in inflate_fast(), PPC biased?
 * - Add compile time option, POSTINC, to use post-increments instead (Intel?)
 * - Make MATCH copy in inflate() much faster for when inflate_fast() not used
 * - Use local copies of stream next and avail values, as well as local bit
 *   buffer and bit count in inflate()--for speed when inflate_fast() not used
 *
 * 1.2.beta4    1 Jan 2003
 * - Split ptr - 257 statements in inflate_table() to avoid compiler warnings
 * - Move a comment on output buffer sizes from inffast.c to inflate.c
 * - Add comments in inffast.c to introduce the inflate_fast() routine
 * - Rearrange window copies in inflate_fast() for speed and simplification
 * - Unroll last copy for window match in inflate_fast()
 * - Use local copies of window variables in inflate_fast() for speed
 * - Pull out common wnext == 0 case for speed in inflate_fast()
 * - Make op and len in inflate_fast() unsigned for consistency
 * - Add FAR to lcode and dcode declarations in inflate_fast()
 * - Simplified bad distance check in inflate_fast()
 * - Added inflateBackInit(), inflateBack(), and inflateBackEnd() in new
 *   source file infback.c to provide a call-back interface to inflate for
 *   programs like gzip and unzip -- uses window as output buffer to avoid
 *   window copying
 *
 * 1.2.beta5    1 Jan 2003
 * - Improved inflateBack() interface to allow the caller to provide initial
 *   input in strm.
 * - Fixed stored blocks bug in inflateBack()
 *
 * 1.2.beta6    4 Jan 2003
 * - Added comments in inffast.c on effectiveness of POSTINC
 * - Typecasting all around to reduce compiler warnings
 * - Changed loops from while (1) or do {} while (1) to for (;;), again to
 *   make compilers happy
 * - Changed type of window in inflateBackInit() to unsigned char *
 *
 * 1.2.beta7    27 Jan 2003
 * - Changed many types to unsigned or unsigned short to avoid warnings
 * - Added inflateCopy() function
 *
 * 1.2.0        9 Mar 2003
 * - Changed inflateBack() interface to provide separate opaque descriptors
 *   for the in() and out() functions
 * - Changed inflateBack() argument and in_func typedef to swap the length
 *   and buffer address return values for the input function
 * - Check next_in and next_out for Z_NULL on entry to inflate()
 *
 * The history for versions after 1.2.0 are in ChangeLog in zlib distribution.
 */

#شامل "zutil.h"
#شامل "inftrees.h"
#شامل "inflate.h"
#شامل "inffast.h"

local صحیح inflateStateCheck(z_streamp strm) {
    ساخت inflate_state FAR *state;
    اگر (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        واپس 1;
    state = (ساخت inflate_state FAR *)strm->state;
    اگر (state == Z_NULL || state->strm != strm ||
        state->منوال < سر_2 || state->منوال > SYNC)
        واپس 1;
    واپس 0;
}

صحیح ZEXPORT inflateResetKeep(z_streamp strm) {
    ساخت inflate_state FAR *state;

    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    strm->کل_اندر = strm->کل_برآمد = state->کل = 0;
    strm->پیغام = Z_NULL;
    strm->ڈیٹا_type = 0;
    اگر (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->منوال = سر_2;
    state->آخری = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->سر = Z_NULL;
    state->hold = 0;
    state->بٹ = 0;
    state->lencode = state->distcode = state->اگلا = state->codes;
    state->sane = 1;
    state->عقب = -1;
    Tracev((stderr, "inflate: reset\n"));
    واپس Z_OK;
}

صحیح ZEXPORT inflateReset(z_streamp strm) {
    ساخت inflate_state FAR *state;

    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    واپس inflateResetKeep(strm);
}

صحیح ZEXPORT inflateReset2(z_streamp strm, صحیح کھڑکی_بٹ) {
    صحیح wrap;
    ساخت inflate_state FAR *state;

    /* get the state */
    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    اگر (کھڑکی_بٹ < 0) {
        اگر (کھڑکی_بٹ < -15)
            واپس Z_دھارا_ERROR;
        wrap = 0;
        کھڑکی_بٹ = -کھڑکی_بٹ;
    }
    ورنہ {
        wrap = (کھڑکی_بٹ >> 4) + 5;
#اگر_تعریف_شدہ GUNZIP
        اگر (کھڑکی_بٹ < 48)
            کھڑکی_بٹ &= 15;
#اگر_ختم
    }

    /* set number of window bits, free window if different */
    اگر (کھڑکی_بٹ && (کھڑکی_بٹ < 8 || کھڑکی_بٹ > 15))
        واپس Z_دھارا_ERROR;
    اگر (state->کھڑکی != Z_NULL && state->wbits != (غیرعلامتی)کھڑکی_بٹ) {
        ZFREE(strm, state->کھڑکی);
        state->کھڑکی = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (غیرعلامتی)کھڑکی_بٹ;
    واپس inflateReset(strm);
}

صحیح ZEXPORT inflateInit2_(z_streamp strm, صحیح کھڑکی_بٹ,
                          ثابت حرف *نسخہ, صحیح دھارا_سائز) {
    صحیح ret;
    ساخت inflate_state FAR *state;

    اگر (نسخہ == Z_NULL || نسخہ[0] != ZLIB_نسخہ[0] ||
        دھارا_سائز != (صحیح)(حجم(z_دھارا)))
        واپس Z_نسخہ_ERROR;
    اگر (strm == Z_NULL) واپس Z_دھارا_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    اگر (strm->zalloc == (alloc_func)0) {
#اگر_تعریف_شدہ Z_SOLO
        واپس Z_دھارا_ERROR;
#ورنہ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#اگر_ختم
    }
    اگر (strm->zfree == (free_func)0)
#اگر_تعریف_شدہ Z_SOLO
        واپس Z_دھارا_ERROR;
#ورنہ
        strm->zfree = zcfree;
#اگر_ختم
    state = (ساخت inflate_state FAR *)
            ZALLOC(strm, 1, حجم(ساخت inflate_state));
    اگر (state == Z_NULL) واپس Z_MEM_ERROR;
    zmemzero(state, حجم(ساخت inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ساخت اندرونی_state FAR *)state;
    state->strm = strm;
    state->کھڑکی = Z_NULL;
    state->منوال = سر_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, کھڑکی_بٹ);
    اگر (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    واپس ret;
}

صحیح ZEXPORT inflateInit_(z_streamp strm, ثابت حرف *نسخہ,
                         صحیح دھارا_سائز) {
    واپس inflateInit2_(strm, DEF_WBITS, نسخہ, دھارا_سائز);
}

صحیح ZEXPORT inflate_مفرد(z_streamp strm, صحیح بٹ, صحیح قدر) {
    ساخت inflate_state FAR *state;

    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    اگر (بٹ == 0)
        واپس Z_OK;
    state = (ساخت inflate_state FAR *)strm->state;
    اگر (بٹ < 0) {
        state->hold = 0;
        state->بٹ = 0;
        واپس Z_OK;
    }
    اگر (بٹ > 16 || state->بٹ + (uInt)بٹ > 32) واپس Z_دھارا_ERROR;
    قدر &= (1L << بٹ) - 1;
    state->hold += (غیرعلامتی طویل)قدر << state->بٹ;
    state->بٹ += (uInt)بٹ;
    واپس Z_OK;
}

/*
   Update the window with the last wsize (normally 32K) bytes written before
   returning.  If window does not exist yet, create it.  This is only called
   when a window is already in use, or when output has been written during this
   inflate call, but the end of the deflate stream has not been reached yet.
   It is also called to create a window for dictionary data when a dictionary
   is loaded.

   Providing output buffers larger than 32K to inflate() should provide a speed
   advantage, since only the last 32K of output is copied to the sliding window
   upon return from inflate(), and since all distances after the first 32K of
   output will fall in the output data, making match copies simpler and faster.
   The advantage may be dependent on the size of the processor's data caches.
 */
local صحیح updatewindow(z_streamp strm, ثابت Bytef *انتہا, غیرعلامتی copy) {
    ساخت inflate_state FAR *state;
    غیرعلامتی dist;

    state = (ساخت inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    اگر (state->کھڑکی == Z_NULL) {
        state->کھڑکی = (غیرعلامتی حرف FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               حجم(غیرعلامتی حرف));
        اگر (state->کھڑکی == Z_NULL) واپس 1;
    }

    /* if window not in use yet, initialize */
    اگر (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    اگر (copy >= state->wsize) {
        zmemcpy(state->کھڑکی, انتہا - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    ورنہ {
        dist = state->wsize - state->wnext;
        اگر (dist > copy) dist = copy;
        zmemcpy(state->کھڑکی + state->wnext, انتہا - copy, dist);
        copy -= dist;
        اگر (copy) {
            zmemcpy(state->کھڑکی, انتہا - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        ورنہ {
            state->wnext += dist;
            اگر (state->wnext == state->wsize) state->wnext = 0;
            اگر (state->whave < state->wsize) state->whave += dist;
        }
    }
    واپس 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#اگر_تعریف_شدہ GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? چکری_جانچ_32_2(check, buf, len) : adler32(check, buf, len))
#ورنہ
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#اگر_ختم

/* check macros for header crc */
#اگر_تعریف_شدہ GUNZIP
#  define CRC2(check, لفظ) \
    کرو { \
        hbuf[0] = (غیرعلامتی حرف)(لفظ); \
        hbuf[1] = (غیرعلامتی حرف)((لفظ) >> 8); \
        check = چکری_جانچ_32_2(check, hbuf, 2); \
    } جب (0)

#  define CRC4(check, لفظ) \
    کرو { \
        hbuf[0] = (غیرعلامتی حرف)(لفظ); \
        hbuf[1] = (غیرعلامتی حرف)((لفظ) >> 8); \
        hbuf[2] = (غیرعلامتی حرف)((لفظ) >> 16); \
        hbuf[3] = (غیرعلامتی حرف)((لفظ) >> 24); \
        check = چکری_جانچ_32_2(check, hbuf, 4); \
    } جب (0)
#اگر_ختم

/* Load registers with state in inflate() for speed */
#تعریف_کرو بھرو_2() \
    کرو { \
        put = strm->اگلا_برآمد; \
        بایاں = strm->avail_برآمد; \
        اگلا = strm->اگلا_اندر; \
        have = strm->avail_اندر; \
        hold = state->hold; \
        بٹ = state->بٹ; \
    } جب (0)

/* Restore state from registers in inflate() */
#تعریف_کرو RESTORE() \
    کرو { \
        strm->اگلا_برآمد = put; \
        strm->avail_برآمد = بایاں; \
        strm->اگلا_اندر = اگلا; \
        strm->avail_اندر = have; \
        state->hold = hold; \
        state->بٹ = بٹ; \
    } جب (0)

/* Clear the input bit accumulator */
#تعریف_کرو INITBITS() \
    کرو { \
        hold = 0; \
        بٹ = 0; \
    } جب (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#تعریف_کرو PULLBYTE() \
    کرو { \
        اگر (have == 0) جاؤ inf_leave; \
        have--; \
        hold += (غیرعلامتی طویل)(*اگلا++) << بٹ; \
        بٹ += 8; \
    } جب (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#تعریف_کرو NEEDBITS(n) \
    کرو { \
        جب (بٹ < (غیرعلامتی)(n)) \
            PULLBYTE(); \
    } جب (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کرو بٹ_2(n) \
    ((غیرعلامتی)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کرو DROPBITS(n) \
    کرو { \
        hold >>= (n); \
        بٹ -= (غیرعلامتی)(n); \
    } جب (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کرو BYTEBITS() \
    کرو { \
        hold >>= بٹ & 7; \
        بٹ -= بٹ & 7; \
    } جب (0)

/*
   inflate() uses a state machine to process as much input data and generate as
   much output data as possible before returning.  The state machine is
   structured roughly as follows:

    for (;;) switch (state) {
    ...
    case STATEn:
        if (not enough input data or output space to make progress)
            return;
        ... make progress ...
        state = STATEm;
        break;
    ...
    }

   so when inflate() is called again, the same case is attempted again, and
   if the appropriate resources are provided, the machine proceeds to the
   next state.  The NEEDBITS() macro is usually the way the state evaluates
   whether it can proceed or should return.  NEEDBITS() does the return if
   the requested bits are not available.  The typical use of the BITS macros
   is:

        NEEDBITS(n);
        ... do something with BITS(n) ...
        DROPBITS(n);

   where NEEDBITS(n) either returns from inflate() if there isn't enough
   input left to load n bits into the accumulator, or it continues.  BITS(n)
   gives the low n bits in the accumulator.  When done, DROPBITS(n) drops
   the low n bits off the accumulator.  INITBITS() clears the accumulator
   and sets the number of available bits to zero.  BYTEBITS() discards just
   enough bits to put the accumulator on a byte boundary.  After BYTEBITS()
   and a NEEDBITS(8), then BITS(8) would return the next byte in the stream.

   NEEDBITS(n) uses PULLBYTE() to get an available byte of input, or to return
   if there is no input available.  The decoding of variable length codes uses
   PULLBYTE() directly in order to pull just enough bytes to decode the next
   code, and no more.

   Some states loop until they get enough input, making sure that enough
   state information is maintained to continue the loop where it left off
   if NEEDBITS() returns in the loop.  For example, want, need, and keep
   would all have to actually be part of the saved state in case NEEDBITS()
   returns:

    case STATEw:
        while (want < need) {
            NEEDBITS(n);
            keep[want++] = BITS(n);
            DROPBITS(n);
        }
        state = STATEx;
    case STATEx:

   As shown above, if the next state is also the next case, then the break
   is omitted.

   A state may also return if there is not enough output space available to
   complete that state.  Those states are copying stored data, writing a
   literal byte, and copying a matching string.

   When returning, a "goto inf_leave" is used to update the total counters,
   update the check value, and determine whether any progress has been made
   during that inflate() call in order to return the proper return code.
   Progress is defined as a change in either strm->avail_in or strm->avail_out.
   When there is a window, goto inf_leave will update the window with the last
   output written.  If a goto inf_leave occurs in the middle of decompression
   and there is no window currently, goto inf_leave will create one and copy
   output to the window for the next call of inflate().

   In this implementation, the flush parameter of inflate() only affects the
   return code (per zlib.h).  inflate() always writes as much as possible to
   strm->next_out, given the space available and the provided input--the effect
   documented in zlib.h of Z_SYNC_FLUSH.  Furthermore, inflate() always defers
   the allocation of and copying into a sliding window until necessary, which
   provides the effect documented in zlib.h for Z_FINISH when the entire input
   stream available.  So the only thing the flush parameter actually does is:
   when flush is set to Z_FINISH, inflate() cannot return Z_OK.  Instead it
   will return Z_BUF_ERROR if it has not reached the end of the stream.
 */

صحیح ZEXPORT inflate(z_streamp strm, صحیح flush) {
    ساخت inflate_state FAR *state;
    z_const غیرعلامتی حرف FAR *اگلا;    /* next input */
    غیرعلامتی حرف FAR *put;     /* next output */
    غیرعلامتی have, بایاں;        /* available input and output */
    غیرعلامتی طویل hold;         /* bit buffer */
    غیرعلامتی بٹ;              /* bits in bit buffer */
    غیرعلامتی اندر, برآمد;           /* save starting available input and output */
    غیرعلامتی copy;              /* number of stored or match bytes to copy */
    غیرعلامتی حرف FAR *from;    /* where to copy match bytes from */
    رمز یہاں;                  /* current decoding table entry */
    رمز آخری;                  /* parent table entry */
    غیرعلامتی len;               /* length to copy for repeats, bits to drop */
    صحیح ret;                    /* return code */
#اگر_تعریف_شدہ GUNZIP
    غیرعلامتی حرف hbuf[4];      /* buffer for gzip header crc calculation */
#اگر_ختم
    ساکن ثابت غیرعلامتی مختصر ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    اگر (inflateStateCheck(strm) || strm->اگلا_برآمد == Z_NULL ||
        (strm->اگلا_اندر == Z_NULL && strm->avail_اندر != 0))
        واپس Z_دھارا_ERROR;

    state = (ساخت inflate_state FAR *)strm->state;
    اگر (state->منوال == TYPE) state->منوال = TYPEDO;      /* skip check */
    بھرو_2();
    اندر = have;
    برآمد = بایاں;
    ret = Z_OK;
    برائے (;;)
        انتخاب (state->منوال) {
        حالت سر_2:
            اگر (state->wrap == 0) {
                state->منوال = TYPEDO;
                توڑو;
            }
            NEEDBITS(16);
#اگر_تعریف_شدہ GUNZIP
            اگر ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                اگر (state->wbits == 0)
                    state->wbits = 15;
                state->check = چکری_جانچ_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->منوال = FLAGS;
                توڑو;
            }
            اگر (state->سر != Z_NULL)
                state->سر->مکمل_2 = -1;
            اگر (!(state->wrap & 1) ||   /* check if zlib header allowed */
#ورنہ
            اگر (
#اگر_ختم
                ((بٹ_2(8) << 8) + (hold >> 8)) % 31) {
                strm->پیغام = (z_const حرف *)"incorrect header check";
                state->منوال = BAD;
                توڑو;
            }
            اگر (بٹ_2(4) != Z_DEFLATED) {
                strm->پیغام = (z_const حرف *)"unknown compression method";
                state->منوال = BAD;
                توڑو;
            }
            DROPBITS(4);
            len = بٹ_2(4) + 8;
            اگر (state->wbits == 0)
                state->wbits = len;
            اگر (len > 15 || len > state->wbits) {
                strm->پیغام = (z_const حرف *)"invalid window size";
                state->منوال = BAD;
                توڑو;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->منوال = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            توڑو;
#اگر_تعریف_شدہ GUNZIP
        حالت FLAGS:
            NEEDBITS(16);
            state->flags = (صحیح)(hold);
            اگر ((state->flags & 0xff) != Z_DEFLATED) {
                strm->پیغام = (z_const حرف *)"unknown compression method";
                state->منوال = BAD;
                توڑو;
            }
            اگر (state->flags & 0xe000) {
                strm->پیغام = (z_const حرف *)"unknown header flags set";
                state->منوال = BAD;
                توڑو;
            }
            اگر (state->سر != Z_NULL)
                state->سر->متن = (صحیح)((hold >> 8) & 1);
            اگر ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->منوال = وقت;
                /* fallthrough */
        حالت وقت:
            NEEDBITS(32);
            اگر (state->سر != Z_NULL)
                state->سر->time = hold;
            اگر ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->منوال = OS;
                /* fallthrough */
        حالت OS:
            NEEDBITS(16);
            اگر (state->سر != Z_NULL) {
                state->سر->xflags = (صحیح)(hold & 0xff);
                state->سر->os = (صحیح)(hold >> 8);
            }
            اگر ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->منوال = EXLEN;
                /* fallthrough */
        حالت EXLEN:
            اگر (state->flags & 0x0400) {
                NEEDBITS(16);
                state->لمبائی = (غیرعلامتی)(hold);
                اگر (state->سر != Z_NULL)
                    state->سر->extra_len = (غیرعلامتی)hold;
                اگر ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            ورنہ اگر (state->سر != Z_NULL)
                state->سر->extra = Z_NULL;
            state->منوال = EXTRA;
                /* fallthrough */
        حالت EXTRA:
            اگر (state->flags & 0x0400) {
                copy = state->لمبائی;
                اگر (copy > have) copy = have;
                اگر (copy) {
                    اگر (state->سر != Z_NULL &&
                        state->سر->extra != Z_NULL &&
                        (len = state->سر->extra_len - state->لمبائی) <
                            state->سر->extra_max) {
                        zmemcpy(state->سر->extra + len, اگلا,
                                len + copy > state->سر->extra_max ?
                                state->سر->extra_max - len : copy);
                    }
                    اگر ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = چکری_جانچ_32_2(state->check, اگلا, copy);
                    have -= copy;
                    اگلا += copy;
                    state->لمبائی -= copy;
                }
                اگر (state->لمبائی) جاؤ inf_leave;
            }
            state->لمبائی = 0;
            state->منوال = NAME;
                /* fallthrough */
        حالت NAME:
            اگر (state->flags & 0x0800) {
                اگر (have == 0) جاؤ inf_leave;
                copy = 0;
                کرو {
                    len = (غیرعلامتی)(اگلا[copy++]);
                    اگر (state->سر != Z_NULL &&
                            state->سر->name != Z_NULL &&
                            state->لمبائی < state->سر->name_max)
                        state->سر->name[state->لمبائی++] = (Bytef)len;
                } جب (len && copy < have);
                اگر ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = چکری_جانچ_32_2(state->check, اگلا, copy);
                have -= copy;
                اگلا += copy;
                اگر (len) جاؤ inf_leave;
            }
            ورنہ اگر (state->سر != Z_NULL)
                state->سر->name = Z_NULL;
            state->لمبائی = 0;
            state->منوال = COMMENT;
                /* fallthrough */
        حالت COMMENT:
            اگر (state->flags & 0x1000) {
                اگر (have == 0) جاؤ inf_leave;
                copy = 0;
                کرو {
                    len = (غیرعلامتی)(اگلا[copy++]);
                    اگر (state->سر != Z_NULL &&
                            state->سر->comment != Z_NULL &&
                            state->لمبائی < state->سر->comm_max)
                        state->سر->comment[state->لمبائی++] = (Bytef)len;
                } جب (len && copy < have);
                اگر ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = چکری_جانچ_32_2(state->check, اگلا, copy);
                have -= copy;
                اگلا += copy;
                اگر (len) جاؤ inf_leave;
            }
            ورنہ اگر (state->سر != Z_NULL)
                state->سر->comment = Z_NULL;
            state->منوال = HCRC;
                /* fallthrough */
        حالت HCRC:
            اگر (state->flags & 0x0200) {
                NEEDBITS(16);
                اگر ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->پیغام = (z_const حرف *)"header crc mismatch";
                    state->منوال = BAD;
                    توڑو;
                }
                INITBITS();
            }
            اگر (state->سر != Z_NULL) {
                state->سر->hcrc = (صحیح)((state->flags >> 9) & 1);
                state->سر->مکمل_2 = 1;
            }
            strm->adler = state->check = چکری_جانچ_32_2(0L, Z_NULL, 0);
            state->منوال = TYPE;
            توڑو;
#اگر_ختم
        حالت DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->منوال = DICT;
                /* fallthrough */
        حالت DICT:
            اگر (state->havedict == 0) {
                RESTORE();
                واپس Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->منوال = TYPE;
                /* fallthrough */
        حالت TYPE:
            اگر (flush == Z_بلاک || flush == Z_TREES) جاؤ inf_leave;
                /* fallthrough */
        حالت TYPEDO:
            اگر (state->آخری) {
                BYTEBITS();
                state->منوال = CHECK;
                توڑو;
            }
            NEEDBITS(3);
            state->آخری = بٹ_2(1);
            DROPBITS(1);
            انتخاب (بٹ_2(2)) {
            حالت 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = STORED;
                توڑو;
            حالت 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = LEN_;             /* decode codes */
                اگر (flush == Z_TREES) {
                    DROPBITS(2);
                    جاؤ inf_leave;
                }
                توڑو;
            حالت 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = جدول;
                توڑو;
            طےشدہ:
                strm->پیغام = (z_const حرف *)"invalid block type";
                state->منوال = BAD;
            }
            DROPBITS(2);
            توڑو;
        حالت STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            اگر ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                state->منوال = BAD;
                توڑو;
            }
            state->لمبائی = (غیرعلامتی)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->لمبائی));
            INITBITS();
            state->منوال = COPY_;
            اگر (flush == Z_TREES) جاؤ inf_leave;
                /* fallthrough */
        حالت COPY_:
            state->منوال = COPY;
                /* fallthrough */
        حالت COPY:
            copy = state->لمبائی;
            اگر (copy) {
                اگر (copy > have) copy = have;
                اگر (copy > بایاں) copy = بایاں;
                اگر (copy == 0) جاؤ inf_leave;
                zmemcpy(put, اگلا, copy);
                have -= copy;
                اگلا += copy;
                بایاں -= copy;
                put += copy;
                state->لمبائی -= copy;
                توڑو;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->منوال = TYPE;
            توڑو;
        حالت جدول:
            NEEDBITS(14);
            state->nlen = بٹ_2(5) + 257;
            DROPBITS(5);
            state->ndist = بٹ_2(5) + 1;
            DROPBITS(5);
            state->ncode = بٹ_2(4) + 4;
            DROPBITS(4);
#اگر_غیر_تعریف_شدہ PKZIP_BUG_WORKAROUND
            اگر (state->nlen > 286 || state->ndist > 30) {
                strm->پیغام = (z_const حرف *)
                    "too many length or distance symbols";
                state->منوال = BAD;
                توڑو;
            }
#اگر_ختم
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->منوال = LENLENS;
                /* fallthrough */
        حالت LENLENS:
            جب (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (غیرعلامتی مختصر)بٹ_2(3);
                DROPBITS(3);
            }
            جب (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->اگلا = state->codes;
            state->lencode = state->distcode = (ثابت رمز FAR *)(state->اگلا);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->اگلا),
                                &(state->lenbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                state->منوال = BAD;
                توڑو;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->منوال = CODELENS;
                /* fallthrough */
        حالت CODELENS:
            جب (state->have < state->nlen + state->ndist) {
                برائے (;;) {
                    یہاں = state->lencode[بٹ_2(state->lenbits)];
                    اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                اگر (یہاں.val < 16) {
                    DROPBITS(یہاں.بٹ);
                    state->lens[state->have++] = یہاں.val;
                }
                ورنہ {
                    اگر (یہاں.val == 16) {
                        NEEDBITS(یہاں.بٹ + 2);
                        DROPBITS(یہاں.بٹ);
                        اگر (state->have == 0) {
                            strm->پیغام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->منوال = BAD;
                            توڑو;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + بٹ_2(2);
                        DROPBITS(2);
                    }
                    ورنہ اگر (یہاں.val == 17) {
                        NEEDBITS(یہاں.بٹ + 3);
                        DROPBITS(یہاں.بٹ);
                        len = 0;
                        copy = 3 + بٹ_2(3);
                        DROPBITS(3);
                    }
                    ورنہ {
                        NEEDBITS(یہاں.بٹ + 7);
                        DROPBITS(یہاں.بٹ);
                        len = 0;
                        copy = 11 + بٹ_2(7);
                        DROPBITS(7);
                    }
                    اگر (state->have + copy > state->nlen + state->ndist) {
                        strm->پیغام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->منوال = BAD;
                        توڑو;
                    }
                    جب (copy--)
                        state->lens[state->have++] = (غیرعلامتی مختصر)len;
                }
            }

            /* handle error breaks in while */
            اگر (state->منوال == BAD) توڑو;

            /* check for end-of-block code (better have one) */
            اگر (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->منوال = BAD;
                توڑو;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->اگلا = state->codes;
            state->lencode = (ثابت رمز FAR *)(state->اگلا);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->اگلا),
                                &(state->lenbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                state->منوال = BAD;
                توڑو;
            }
            state->distcode = (ثابت رمز FAR *)(state->اگلا);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->اگلا), &(state->distbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                state->منوال = BAD;
                توڑو;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->منوال = LEN_;
            اگر (flush == Z_TREES) جاؤ inf_leave;
                /* fallthrough */
        حالت LEN_:
            state->منوال = LEN;
                /* fallthrough */
        حالت LEN:
            اگر (have >= 6 && بایاں >= 258) {
                RESTORE();
                inflate_fast(strm, برآمد);
                بھرو_2();
                اگر (state->منوال == TYPE)
                    state->عقب = -1;
                توڑو;
            }
            state->عقب = 0;
            برائے (;;) {
                یہاں = state->lencode[بٹ_2(state->lenbits)];
                اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                PULLBYTE();
            }
            اگر (یہاں.op && (یہاں.op & 0xf0) == 0) {
                آخری = یہاں;
                برائے (;;) {
                    یہاں = state->lencode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    اگر ((غیرعلامتی)(آخری.بٹ + یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
                state->عقب += آخری.بٹ;
            }
            DROPBITS(یہاں.بٹ);
            state->عقب += یہاں.بٹ;
            state->لمبائی = (غیرعلامتی)یہاں.val;
            اگر ((صحیح)(یہاں.op) == 0) {
                Tracevv((stderr, یہاں.val >= 0x20 && یہاں.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", یہاں.val));
                state->منوال = LIT;
                توڑو;
            }
            اگر (یہاں.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->عقب = -1;
                state->منوال = TYPE;
                توڑو;
            }
            اگر (یہاں.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                state->منوال = BAD;
                توڑو;
            }
            state->extra = (غیرعلامتی)(یہاں.op) & 15;
            state->منوال = LENEXT;
                /* fallthrough */
        حالت LENEXT:
            اگر (state->extra) {
                NEEDBITS(state->extra);
                state->لمبائی += بٹ_2(state->extra);
                DROPBITS(state->extra);
                state->عقب += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->لمبائی));
            state->was = state->لمبائی;
            state->منوال = DIST;
                /* fallthrough */
        حالت DIST:
            برائے (;;) {
                یہاں = state->distcode[بٹ_2(state->distbits)];
                اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                PULLBYTE();
            }
            اگر ((یہاں.op & 0xf0) == 0) {
                آخری = یہاں;
                برائے (;;) {
                    یہاں = state->distcode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    اگر ((غیرعلامتی)(آخری.بٹ + یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
                state->عقب += آخری.بٹ;
            }
            DROPBITS(یہاں.بٹ);
            state->عقب += یہاں.بٹ;
            اگر (یہاں.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->منوال = BAD;
                توڑو;
            }
            state->انحراف = (غیرعلامتی)یہاں.val;
            state->extra = (غیرعلامتی)(یہاں.op) & 15;
            state->منوال = DISTEXT;
                /* fallthrough */
        حالت DISTEXT:
            اگر (state->extra) {
                NEEDBITS(state->extra);
                state->انحراف += بٹ_2(state->extra);
                DROPBITS(state->extra);
                state->عقب += state->extra;
            }
#اگر_تعریف_شدہ INFLATE_STRICT
            اگر (state->انحراف > state->dmax) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                state->منوال = BAD;
                توڑو;
            }
#اگر_ختم
            Tracevv((stderr, "inflate:         distance %u\n", state->انحراف));
            state->منوال = مطابقت_2;
                /* fallthrough */
        حالت مطابقت_2:
            اگر (بایاں == 0) جاؤ inf_leave;
            copy = برآمد - بایاں;
            اگر (state->انحراف > copy) {         /* copy from window */
                copy = state->انحراف - copy;
                اگر (copy > state->whave) {
                    اگر (state->sane) {
                        strm->پیغام = (z_const حرف *)
                            "invalid distance too far back";
                        state->منوال = BAD;
                        توڑو;
                    }
#اگر_تعریف_شدہ INFLATE_ALLOW_INVALID_فاصلہ_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    اگر (copy > state->لمبائی) copy = state->لمبائی;
                    اگر (copy > بایاں) copy = بایاں;
                    بایاں -= copy;
                    state->لمبائی -= copy;
                    کرو {
                        *put++ = 0;
                    } جب (--copy);
                    اگر (state->لمبائی == 0) state->منوال = LEN;
                    توڑو;
#اگر_ختم
                }
                اگر (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->کھڑکی + (state->wsize - copy);
                }
                ورنہ
                    from = state->کھڑکی + (state->wnext - copy);
                اگر (copy > state->لمبائی) copy = state->لمبائی;
            }
            ورنہ {                              /* copy from output */
                from = put - state->انحراف;
                copy = state->لمبائی;
            }
            اگر (copy > بایاں) copy = بایاں;
            بایاں -= copy;
            state->لمبائی -= copy;
            کرو {
                *put++ = *from++;
            } جب (--copy);
            اگر (state->لمبائی == 0) state->منوال = LEN;
            توڑو;
        حالت LIT:
            اگر (بایاں == 0) جاؤ inf_leave;
            *put++ = (غیرعلامتی حرف)(state->لمبائی);
            بایاں--;
            state->منوال = LEN;
            توڑو;
        حالت CHECK:
            اگر (state->wrap) {
                NEEDBITS(32);
                برآمد -= بایاں;
                strm->کل_برآمد += برآمد;
                state->کل += برآمد;
                اگر ((state->wrap & 4) && برآمد)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - برآمد, برآمد);
                برآمد = بایاں;
                اگر ((state->wrap & 4) && (
#اگر_تعریف_شدہ GUNZIP
                     state->flags ? hold :
#اگر_ختم
                     ZSWAP32(hold)) != state->check) {
                    strm->پیغام = (z_const حرف *)"incorrect data check";
                    state->منوال = BAD;
                    توڑو;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#اگر_تعریف_شدہ GUNZIP
            state->منوال = لمبائی_2;
                /* fallthrough */
        حالت لمبائی_2:
            اگر (state->wrap && state->flags) {
                NEEDBITS(32);
                اگر ((state->wrap & 4) && hold != (state->کل & 0xffffffff)) {
                    strm->پیغام = (z_const حرف *)"incorrect length check";
                    state->منوال = BAD;
                    توڑو;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#اگر_ختم
            state->منوال = مکمل;
                /* fallthrough */
        حالت مکمل:
            ret = Z_دھارا_انتہا;
            جاؤ inf_leave;
        حالت BAD:
            ret = Z_ڈیٹا_ERROR;
            جاؤ inf_leave;
        حالت MEM:
            واپس Z_MEM_ERROR;
        حالت SYNC:
                /* fallthrough */
        طےشدہ:
            واپس Z_دھارا_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    اگر (state->wsize || (برآمد != strm->avail_برآمد && state->منوال < BAD &&
            (state->منوال < CHECK || flush != Z_اختتام)))
        اگر (updatewindow(strm, strm->اگلا_برآمد, برآمد - strm->avail_برآمد)) {
            state->منوال = MEM;
            واپس Z_MEM_ERROR;
        }
    اندر -= strm->avail_اندر;
    برآمد -= strm->avail_برآمد;
    strm->کل_اندر += اندر;
    strm->کل_برآمد += برآمد;
    state->کل += برآمد;
    اگر ((state->wrap & 4) && برآمد)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->اگلا_برآمد - برآمد, برآمد);
    strm->ڈیٹا_type = (صحیح)state->بٹ + (state->آخری ? 64 : 0) +
                      (state->منوال == TYPE ? 128 : 0) +
                      (state->منوال == LEN_ || state->منوال == COPY_ ? 256 : 0);
    اگر (((اندر == 0 && برآمد == 0) || flush == Z_اختتام) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    واپس ret;
}

صحیح ZEXPORT inflate_انتہا(z_streamp strm) {
    ساخت inflate_state FAR *state;
    اگر (inflateStateCheck(strm))
        واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    اگر (state->کھڑکی != Z_NULL) ZFREE(strm, state->کھڑکی);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    واپس Z_OK;
}

صحیح ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_لمبائی) {
    ساخت inflate_state FAR *state;

    /* check state */
    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;

    /* copy dictionary */
    اگر (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->کھڑکی + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->کھڑکی, state->wnext);
    }
    اگر (dict_لمبائی != Z_NULL)
        *dict_لمبائی = state->whave;
    واپس Z_OK;
}

صحیح ZEXPORT inflate_سیٹ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt dict_لمبائی) {
    ساخت inflate_state FAR *state;
    غیرعلامتی طویل dictid;
    صحیح ret;

    /* check state */
    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    اگر (state->wrap != 0 && state->منوال != DICT)
        واپس Z_دھارا_ERROR;

    /* check for correct dictionary identifier */
    اگر (state->منوال == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_لمبائی);
        اگر (dictid != state->check)
            واپس Z_ڈیٹا_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_لمبائی, dict_لمبائی);
    اگر (ret) {
        state->منوال = MEM;
        واپس Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    واپس Z_OK;
}

صحیح ZEXPORT inflate_Get_سرخی(z_streamp strm, gz_headerp سر) {
    ساخت inflate_state FAR *state;

    /* check state */
    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    اگر ((state->wrap & 2) == 0) واپس Z_دھارا_ERROR;

    /* save header structure */
    state->سر = سر;
    سر->مکمل_2 = 0;
    واپس Z_OK;
}

/*
   Search buf[0..len-1] for the pattern: 0, 0, 0xff, 0xff.  Return when found
   or when out of input.  When called, *have is the number of pattern bytes
   found in order so far, in 0..3.  On return *have is updated to the new
   state.  If on return *have equals four, then the pattern was found and the
   return value is how many bytes were read including the last byte of the
   pattern.  If *have is less than four, then the pattern has not been found
   yet and the return value is len.  In the latter case, syncsearch() can be
   called again with more data and the *have state.  *have is initialized to
   zero for the first call.
 */
local غیرعلامتی syncsearch(غیرعلامتی FAR *have, ثابت غیرعلامتی حرف FAR *buf,
                          غیرعلامتی len) {
    غیرعلامتی got;
    غیرعلامتی اگلا;

    got = *have;
    اگلا = 0;
    جب (اگلا < len && got < 4) {
        اگر ((صحیح)(buf[اگلا]) == (got < 2 ? 0 : 0xff))
            got++;
        ورنہ اگر (buf[اگلا])
            got = 0;
        ورنہ
            got = 4 - got;
        اگلا++;
    }
    *have = got;
    واپس اگلا;
}

صحیح ZEXPORT inflateSync(z_streamp strm) {
    غیرعلامتی len;               /* number of bytes to look at or looked at */
    صحیح flags;                  /* temporary to save header status */
    غیرعلامتی طویل اندر, برآمد;      /* temporary to save total_in and total_out */
    غیرعلامتی حرف buf[4];       /* to restore bit buffer to byte string */
    ساخت inflate_state FAR *state;

    /* check parameters */
    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    اگر (strm->avail_اندر == 0 && state->بٹ < 8) واپس Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    اگر (state->منوال != SYNC) {
        state->منوال = SYNC;
        state->hold >>= state->بٹ & 7;
        state->بٹ -= state->بٹ & 7;
        len = 0;
        جب (state->بٹ >= 8) {
            buf[len++] = (غیرعلامتی حرف)(state->hold);
            state->hold >>= 8;
            state->بٹ -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->اگلا_اندر, strm->avail_اندر);
    strm->avail_اندر -= len;
    strm->اگلا_اندر += len;
    strm->کل_اندر += len;

    /* return no joy or set up to restart inflate() on a new block */
    اگر (state->have != 4) واپس Z_ڈیٹا_ERROR;
    اگر (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    ورنہ
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    اندر = strm->کل_اندر;  برآمد = strm->کل_برآمد;
    inflateReset(strm);
    strm->کل_اندر = اندر;  strm->کل_برآمد = برآمد;
    state->flags = flags;
    state->منوال = TYPE;
    واپس Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
صحیح ZEXPORT inflate_Sync_نقطہ(z_streamp strm) {
    ساخت inflate_state FAR *state;

    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    واپس state->منوال == STORED && state->بٹ == 0;
}

صحیح ZEXPORT inflateCopy(z_streamp dest, z_streamp ماخذ) {
    ساخت inflate_state FAR *state;
    ساخت inflate_state FAR *copy;
    غیرعلامتی حرف FAR *کھڑکی;

    /* check input */
    اگر (inflateStateCheck(ماخذ) || dest == Z_NULL)
        واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)ماخذ->state;

    /* allocate space */
    copy = (ساخت inflate_state FAR *)
           ZALLOC(ماخذ, 1, حجم(ساخت inflate_state));
    اگر (copy == Z_NULL) واپس Z_MEM_ERROR;
    zmemzero(copy, حجم(ساخت inflate_state));
    کھڑکی = Z_NULL;
    اگر (state->کھڑکی != Z_NULL) {
        کھڑکی = (غیرعلامتی حرف FAR *)
                 ZALLOC(ماخذ, 1U << state->wbits, حجم(غیرعلامتی حرف));
        اگر (کھڑکی == Z_NULL) {
            ZFREE(ماخذ, copy);
            واپس Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, ماخذ, حجم(z_دھارا));
    zmemcpy(copy, state, حجم(ساخت inflate_state));
    copy->strm = dest;
    اگر (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->اگلا = copy->codes + (state->اگلا - state->codes);
    اگر (کھڑکی != Z_NULL)
        zmemcpy(کھڑکی, state->کھڑکی, state->whave);
    copy->کھڑکی = کھڑکی;
    dest->state = (ساخت اندرونی_state FAR *)copy;
    واپس Z_OK;
}

صحیح ZEXPORT inflateUndermine(z_streamp strm, صحیح subvert) {
    ساخت inflate_state FAR *state;

    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
#اگر_تعریف_شدہ INFLATE_ALLOW_INVALID_فاصلہ_TOOFAR_ARRR
    state->sane = !subvert;
    واپس Z_OK;
#ورنہ
    (خالی)subvert;
    state->sane = 1;
    واپس Z_ڈیٹا_ERROR;
#اگر_ختم
}

صحیح ZEXPORT inflateValidate(z_streamp strm, صحیح check) {
    ساخت inflate_state FAR *state;

    اگر (inflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;
    اگر (check && state->wrap)
        state->wrap |= 4;
    ورنہ
        state->wrap &= ~4;
    واپس Z_OK;
}

طویل ZEXPORT inflateMark(z_streamp strm) {
    ساخت inflate_state FAR *state;

    اگر (inflateStateCheck(strm))
        واپس -(1L << 16);
    state = (ساخت inflate_state FAR *)strm->state;
    واپس (طویل)(((غیرعلامتی طویل)((طویل)state->عقب)) << 16) +
        (state->منوال == COPY ? state->لمبائی :
            (state->منوال == مطابقت_2 ? state->was - state->لمبائی : 0));
}

غیرعلامتی طویل ZEXPORT inflate_Codes_مستعمل(z_streamp strm) {
    ساخت inflate_state FAR *state;
    اگر (inflateStateCheck(strm)) واپس (غیرعلامتی طویل)-1;
    state = (ساخت inflate_state FAR *)strm->state;
    واپس (غیرعلامتی طویل)(state->اگلا - state->codes);
}
