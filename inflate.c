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

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

local صحيح inflateStateCheck(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        أرجع 1;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state == Z_NULL || state->strm != strm ||
        state->منوال < سر_2 || state->منوال > SYNC)
        أرجع 1;
    أرجع 0;
}

صحيح ZEXPORT inflateResetKeep(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    strm->کل_وچ = strm->کل_باہرلا = state->کل = 0;
    strm->پیغام = Z_NULL;
    strm->انکڑے_type = 0;
    إذا (state->wrap)        /* to support ill-conceived Java test suite */
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
    state->پٹھ = -1;
    Tracev((stderr, "inflate: reset\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateReset(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    أرجع inflateResetKeep(strm);
}

صحيح ZEXPORT inflateReset2(z_streamp strm, صحيح کھڑکی_بٹ) {
    صحيح wrap;
    بنية inflate_state FAR *state;

    /* get the state */
    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    إذا (کھڑکی_بٹ < 0) {
        إذا (کھڑکی_بٹ < -15)
            أرجع Z_دھارا_ERROR;
        wrap = 0;
        کھڑکی_بٹ = -کھڑکی_بٹ;
    }
    وإلا {
        wrap = (کھڑکی_بٹ >> 4) + 5;
#جے_تعریف_شدہ GUNZIP
        إذا (کھڑکی_بٹ < 48)
            کھڑکی_بٹ &= 15;
#جے_انت
    }

    /* set number of window bits, free window if different */
    إذا (کھڑکی_بٹ && (کھڑکی_بٹ < 8 || کھڑکی_بٹ > 15))
        أرجع Z_دھارا_ERROR;
    إذا (state->کھڑکی != Z_NULL && state->wbits != (بنانشانی)کھڑکی_بٹ) {
        ZFREE(strm, state->کھڑکی);
        state->کھڑکی = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (بنانشانی)کھڑکی_بٹ;
    أرجع inflateReset(strm);
}

صحيح ZEXPORT inflateInit2_(z_streamp strm, صحيح کھڑکی_بٹ,
                          ثابت حرف *نسخہ, صحيح دھارا_ناپ) {
    صحيح ret;
    بنية inflate_state FAR *state;

    إذا (نسخہ == Z_NULL || نسخہ[0] != ZLIB_نسخہ[0] ||
        دھارا_ناپ != (صحيح)(حجم(z_دھارا)))
        أرجع Z_نسخہ_ERROR;
    إذا (strm == Z_NULL) أرجع Z_دھارا_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#جے_تعریف_شدہ Z_SOLO
        أرجع Z_دھارا_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#جے_انت
    }
    إذا (strm->zfree == (free_func)0)
#جے_تعریف_شدہ Z_SOLO
        أرجع Z_دھارا_ERROR;
#وإلا
        strm->zfree = zcfree;
#جے_انت
    state = (بنية inflate_state FAR *)
            ZALLOC(strm, 1, حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(state, حجم(بنية inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية اندرونی_state FAR *)state;
    state->strm = strm;
    state->کھڑکی = Z_NULL;
    state->منوال = سر_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, کھڑکی_بٹ);
    إذا (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    أرجع ret;
}

صحيح ZEXPORT inflateInit_(z_streamp strm, ثابت حرف *نسخہ,
                         صحيح دھارا_ناپ) {
    أرجع inflateInit2_(strm, DEF_WBITS, نسخہ, دھارا_ناپ);
}

صحيح ZEXPORT inflate_مفرد(z_streamp strm, صحيح بٹ, صحيح قدر) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    إذا (بٹ == 0)
        أرجع Z_OK;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (بٹ < 0) {
        state->hold = 0;
        state->بٹ = 0;
        أرجع Z_OK;
    }
    إذا (بٹ > 16 || state->بٹ + (uInt)بٹ > 32) أرجع Z_دھارا_ERROR;
    قدر &= (1L << بٹ) - 1;
    state->hold += (بنانشانی لما)قدر << state->بٹ;
    state->بٹ += (uInt)بٹ;
    أرجع Z_OK;
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
local صحيح updatewindow(z_streamp strm, ثابت Bytef *سرا, بنانشانی copy) {
    بنية inflate_state FAR *state;
    بنانشانی dist;

    state = (بنية inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    إذا (state->کھڑکی == Z_NULL) {
        state->کھڑکی = (بنانشانی حرف FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               حجم(بنانشانی حرف));
        إذا (state->کھڑکی == Z_NULL) أرجع 1;
    }

    /* if window not in use yet, initialize */
    إذا (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    إذا (copy >= state->wsize) {
        zmemcpy(state->کھڑکی, سرا - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    وإلا {
        dist = state->wsize - state->wnext;
        إذا (dist > copy) dist = copy;
        zmemcpy(state->کھڑکی + state->wnext, سرا - copy, dist);
        copy -= dist;
        إذا (copy) {
            zmemcpy(state->کھڑکی, سرا - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        وإلا {
            state->wnext += dist;
            إذا (state->wnext == state->wsize) state->wnext = 0;
            إذا (state->whave < state->wsize) state->whave += dist;
        }
    }
    أرجع 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#جے_تعریف_شدہ GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? چکری_جانچ_32_2(check, buf, len) : adler32(check, buf, len))
#وإلا
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#جے_انت

/* check macros for header crc */
#جے_تعریف_شدہ GUNZIP
#  define CRC2(check, لفظ) \
    افعل { \
        hbuf[0] = (بنانشانی حرف)(لفظ); \
        hbuf[1] = (بنانشانی حرف)((لفظ) >> 8); \
        check = چکری_جانچ_32_2(check, hbuf, 2); \
    } طالما (0)

#  define CRC4(check, لفظ) \
    افعل { \
        hbuf[0] = (بنانشانی حرف)(لفظ); \
        hbuf[1] = (بنانشانی حرف)((لفظ) >> 8); \
        hbuf[2] = (بنانشانی حرف)((لفظ) >> 16); \
        hbuf[3] = (بنانشانی حرف)((لفظ) >> 24); \
        check = چکری_جانچ_32_2(check, hbuf, 4); \
    } طالما (0)
#جے_انت

/* Load registers with state in inflate() for speed */
#تعریف_کرو لدو_2() \
    افعل { \
        put = strm->اگلا_باہرلا; \
        بایاں = strm->avail_باہرلا; \
        اگلا = strm->اگلا_وچ; \
        have = strm->avail_وچ; \
        hold = state->hold; \
        بٹ = state->بٹ; \
    } طالما (0)

/* Restore state from registers in inflate() */
#تعریف_کرو RESTORE() \
    افعل { \
        strm->اگلا_باہرلا = put; \
        strm->avail_باہرلا = بایاں; \
        strm->اگلا_وچ = اگلا; \
        strm->avail_وچ = have; \
        state->hold = hold; \
        state->بٹ = بٹ; \
    } طالما (0)

/* Clear the input bit accumulator */
#تعریف_کرو INITBITS() \
    افعل { \
        hold = 0; \
        بٹ = 0; \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#تعریف_کرو PULLBYTE() \
    افعل { \
        إذا (have == 0) اذهب inf_leave; \
        have--; \
        hold += (بنانشانی لما)(*اگلا++) << بٹ; \
        بٹ += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#تعریف_کرو NEEDBITS(n) \
    افعل { \
        طالما (بٹ < (بنانشانی)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کرو بٹ_2(n) \
    ((بنانشانی)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کرو DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بٹ -= (بنانشانی)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کرو BYTEBITS() \
    افعل { \
        hold >>= بٹ & 7; \
        بٹ -= بٹ & 7; \
    } طالما (0)

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

صحيح ZEXPORT inflate(z_streamp strm, صحيح flush) {
    بنية inflate_state FAR *state;
    z_const بنانشانی حرف FAR *اگلا;    /* next input */
    بنانشانی حرف FAR *put;     /* next output */
    بنانشانی have, بایاں;        /* available input and output */
    بنانشانی لما hold;         /* bit buffer */
    بنانشانی بٹ;              /* bits in bit buffer */
    بنانشانی وچ, باہرلا;           /* save starting available input and output */
    بنانشانی copy;              /* number of stored or match bytes to copy */
    بنانشانی حرف FAR *from;    /* where to copy match bytes from */
    کوڈ ایتھے;                  /* current decoding table entry */
    کوڈ آخری;                  /* parent table entry */
    بنانشانی len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
#جے_تعریف_شدہ GUNZIP
    بنانشانی حرف hbuf[4];      /* buffer for gzip header crc calculation */
#جے_انت
    ساكن ثابت بنانشانی چھوٹا ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    إذا (inflateStateCheck(strm) || strm->اگلا_باہرلا == Z_NULL ||
        (strm->اگلا_وچ == Z_NULL && strm->avail_وچ != 0))
        أرجع Z_دھارا_ERROR;

    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->منوال == TYPE) state->منوال = TYPEDO;      /* skip check */
    لدو_2();
    وچ = have;
    باہرلا = بایاں;
    ret = Z_OK;
    لكل (;;)
        بدل (state->منوال) {
        حالة سر_2:
            إذا (state->wrap == 0) {
                state->منوال = TYPEDO;
                اكسر;
            }
            NEEDBITS(16);
#جے_تعریف_شدہ GUNZIP
            إذا ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                إذا (state->wbits == 0)
                    state->wbits = 15;
                state->check = چکری_جانچ_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->منوال = FLAGS;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->پورا_2 = -1;
            إذا (!(state->wrap & 1) ||   /* check if zlib header allowed */
#وإلا
            إذا (
#جے_انت
                ((بٹ_2(8) << 8) + (hold >> 8)) % 31) {
                strm->پیغام = (z_const حرف *)"incorrect header check";
                state->منوال = BAD;
                اكسر;
            }
            إذا (بٹ_2(4) != Z_DEFLATED) {
                strm->پیغام = (z_const حرف *)"unknown compression method";
                state->منوال = BAD;
                اكسر;
            }
            DROPBITS(4);
            len = بٹ_2(4) + 8;
            إذا (state->wbits == 0)
                state->wbits = len;
            إذا (len > 15 || len > state->wbits) {
                strm->پیغام = (z_const حرف *)"invalid window size";
                state->منوال = BAD;
                اكسر;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->منوال = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            اكسر;
#جے_تعریف_شدہ GUNZIP
        حالة FLAGS:
            NEEDBITS(16);
            state->flags = (صحيح)(hold);
            إذا ((state->flags & 0xff) != Z_DEFLATED) {
                strm->پیغام = (z_const حرف *)"unknown compression method";
                state->منوال = BAD;
                اكسر;
            }
            إذا (state->flags & 0xe000) {
                strm->پیغام = (z_const حرف *)"unknown header flags set";
                state->منوال = BAD;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->متن = (صحيح)((hold >> 8) & 1);
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->منوال = ویلا;
                /* fallthrough */
        حالة ویلا:
            NEEDBITS(32);
            إذا (state->سر != Z_NULL)
                state->سر->time = hold;
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->منوال = OS;
                /* fallthrough */
        حالة OS:
            NEEDBITS(16);
            إذا (state->سر != Z_NULL) {
                state->سر->xflags = (صحيح)(hold & 0xff);
                state->سر->os = (صحيح)(hold >> 8);
            }
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->منوال = EXLEN;
                /* fallthrough */
        حالة EXLEN:
            إذا (state->flags & 0x0400) {
                NEEDBITS(16);
                state->لمائی = (بنانشانی)(hold);
                إذا (state->سر != Z_NULL)
                    state->سر->extra_len = (بنانشانی)hold;
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->extra = Z_NULL;
            state->منوال = EXTRA;
                /* fallthrough */
        حالة EXTRA:
            إذا (state->flags & 0x0400) {
                copy = state->لمائی;
                إذا (copy > have) copy = have;
                إذا (copy) {
                    إذا (state->سر != Z_NULL &&
                        state->سر->extra != Z_NULL &&
                        (len = state->سر->extra_len - state->لمائی) <
                            state->سر->extra_max) {
                        zmemcpy(state->سر->extra + len, اگلا,
                                len + copy > state->سر->extra_max ?
                                state->سر->extra_max - len : copy);
                    }
                    إذا ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = چکری_جانچ_32_2(state->check, اگلا, copy);
                    have -= copy;
                    اگلا += copy;
                    state->لمائی -= copy;
                }
                إذا (state->لمائی) اذهب inf_leave;
            }
            state->لمائی = 0;
            state->منوال = NAME;
                /* fallthrough */
        حالة NAME:
            إذا (state->flags & 0x0800) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بنانشانی)(اگلا[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->name != Z_NULL &&
                            state->لمائی < state->سر->name_max)
                        state->سر->name[state->لمائی++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = چکری_جانچ_32_2(state->check, اگلا, copy);
                have -= copy;
                اگلا += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->name = Z_NULL;
            state->لمائی = 0;
            state->منوال = COMMENT;
                /* fallthrough */
        حالة COMMENT:
            إذا (state->flags & 0x1000) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بنانشانی)(اگلا[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->comment != Z_NULL &&
                            state->لمائی < state->سر->comm_max)
                        state->سر->comment[state->لمائی++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = چکری_جانچ_32_2(state->check, اگلا, copy);
                have -= copy;
                اگلا += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->comment = Z_NULL;
            state->منوال = HCRC;
                /* fallthrough */
        حالة HCRC:
            إذا (state->flags & 0x0200) {
                NEEDBITS(16);
                إذا ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->پیغام = (z_const حرف *)"header crc mismatch";
                    state->منوال = BAD;
                    اكسر;
                }
                INITBITS();
            }
            إذا (state->سر != Z_NULL) {
                state->سر->hcrc = (صحيح)((state->flags >> 9) & 1);
                state->سر->پورا_2 = 1;
            }
            strm->adler = state->check = چکری_جانچ_32_2(0L, Z_NULL, 0);
            state->منوال = TYPE;
            اكسر;
#جے_انت
        حالة DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->منوال = DICT;
                /* fallthrough */
        حالة DICT:
            إذا (state->havedict == 0) {
                RESTORE();
                أرجع Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->منوال = TYPE;
                /* fallthrough */
        حالة TYPE:
            إذا (flush == Z_بلاک || flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة TYPEDO:
            إذا (state->آخری) {
                BYTEBITS();
                state->منوال = CHECK;
                اكسر;
            }
            NEEDBITS(3);
            state->آخری = بٹ_2(1);
            DROPBITS(1);
            بدل (بٹ_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = LEN_;             /* decode codes */
                إذا (flush == Z_TREES) {
                    DROPBITS(2);
                    اذهب inf_leave;
                }
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = جدول;
                اكسر;
            افتراضي:
                strm->پیغام = (z_const حرف *)"invalid block type";
                state->منوال = BAD;
            }
            DROPBITS(2);
            اكسر;
        حالة STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                state->منوال = BAD;
                اكسر;
            }
            state->لمائی = (بنانشانی)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->لمائی));
            INITBITS();
            state->منوال = COPY_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة COPY_:
            state->منوال = COPY;
                /* fallthrough */
        حالة COPY:
            copy = state->لمائی;
            إذا (copy) {
                إذا (copy > have) copy = have;
                إذا (copy > بایاں) copy = بایاں;
                إذا (copy == 0) اذهب inf_leave;
                zmemcpy(put, اگلا, copy);
                have -= copy;
                اگلا += copy;
                بایاں -= copy;
                put += copy;
                state->لمائی -= copy;
                اكسر;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->منوال = TYPE;
            اكسر;
        حالة جدول:
            NEEDBITS(14);
            state->nlen = بٹ_2(5) + 257;
            DROPBITS(5);
            state->ndist = بٹ_2(5) + 1;
            DROPBITS(5);
            state->ncode = بٹ_2(4) + 4;
            DROPBITS(4);
#جے_بنا_تعریف PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->پیغام = (z_const حرف *)
                    "too many length or distance symbols";
                state->منوال = BAD;
                اكسر;
            }
#جے_انت
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->منوال = LENLENS;
                /* fallthrough */
        حالة LENLENS:
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (بنانشانی چھوٹا)بٹ_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->اگلا = state->codes;
            state->lencode = state->distcode = (ثابت کوڈ FAR *)(state->اگلا);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->اگلا),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                state->منوال = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->منوال = CODELENS;
                /* fallthrough */
        حالة CODELENS:
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    ایتھے = state->lencode[بٹ_2(state->lenbits)];
                    إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                إذا (ایتھے.val < 16) {
                    DROPBITS(ایتھے.بٹ);
                    state->lens[state->have++] = ایتھے.val;
                }
                وإلا {
                    إذا (ایتھے.val == 16) {
                        NEEDBITS(ایتھے.بٹ + 2);
                        DROPBITS(ایتھے.بٹ);
                        إذا (state->have == 0) {
                            strm->پیغام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->منوال = BAD;
                            اكسر;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + بٹ_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (ایتھے.val == 17) {
                        NEEDBITS(ایتھے.بٹ + 3);
                        DROPBITS(ایتھے.بٹ);
                        len = 0;
                        copy = 3 + بٹ_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(ایتھے.بٹ + 7);
                        DROPBITS(ایتھے.بٹ);
                        len = 0;
                        copy = 11 + بٹ_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->پیغام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->منوال = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بنانشانی چھوٹا)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->منوال == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->منوال = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->اگلا = state->codes;
            state->lencode = (ثابت کوڈ FAR *)(state->اگلا);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->اگلا),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                state->منوال = BAD;
                اكسر;
            }
            state->distcode = (ثابت کوڈ FAR *)(state->اگلا);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->اگلا), &(state->distbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                state->منوال = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->منوال = LEN_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة LEN_:
            state->منوال = LEN;
                /* fallthrough */
        حالة LEN:
            إذا (have >= 6 && بایاں >= 258) {
                RESTORE();
                inflate_fast(strm, باہرلا);
                لدو_2();
                إذا (state->منوال == TYPE)
                    state->پٹھ = -1;
                اكسر;
            }
            state->پٹھ = 0;
            لكل (;;) {
                ایتھے = state->lencode[بٹ_2(state->lenbits)];
                إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                PULLBYTE();
            }
            إذا (ایتھے.op && (ایتھے.op & 0xf0) == 0) {
                آخری = ایتھے;
                لكل (;;) {
                    ایتھے = state->lencode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    إذا ((بنانشانی)(آخری.بٹ + ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
                state->پٹھ += آخری.بٹ;
            }
            DROPBITS(ایتھے.بٹ);
            state->پٹھ += ایتھے.بٹ;
            state->لمائی = (بنانشانی)ایتھے.val;
            إذا ((صحيح)(ایتھے.op) == 0) {
                Tracevv((stderr, ایتھے.val >= 0x20 && ایتھے.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ایتھے.val));
                state->منوال = LIT;
                اكسر;
            }
            إذا (ایتھے.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->پٹھ = -1;
                state->منوال = TYPE;
                اكسر;
            }
            إذا (ایتھے.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                state->منوال = BAD;
                اكسر;
            }
            state->extra = (بنانشانی)(ایتھے.op) & 15;
            state->منوال = LENEXT;
                /* fallthrough */
        حالة LENEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->لمائی += بٹ_2(state->extra);
                DROPBITS(state->extra);
                state->پٹھ += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->لمائی));
            state->was = state->لمائی;
            state->منوال = DIST;
                /* fallthrough */
        حالة DIST:
            لكل (;;) {
                ایتھے = state->distcode[بٹ_2(state->distbits)];
                إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                PULLBYTE();
            }
            إذا ((ایتھے.op & 0xf0) == 0) {
                آخری = ایتھے;
                لكل (;;) {
                    ایتھے = state->distcode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    إذا ((بنانشانی)(آخری.بٹ + ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
                state->پٹھ += آخری.بٹ;
            }
            DROPBITS(ایتھے.بٹ);
            state->پٹھ += ایتھے.بٹ;
            إذا (ایتھے.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->منوال = BAD;
                اكسر;
            }
            state->وتھ = (بنانشانی)ایتھے.val;
            state->extra = (بنانشانی)(ایتھے.op) & 15;
            state->منوال = DISTEXT;
                /* fallthrough */
        حالة DISTEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->وتھ += بٹ_2(state->extra);
                DROPBITS(state->extra);
                state->پٹھ += state->extra;
            }
#جے_تعریف_شدہ INFLATE_STRICT
            إذا (state->وتھ > state->dmax) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                state->منوال = BAD;
                اكسر;
            }
#جے_انت
            Tracevv((stderr, "inflate:         distance %u\n", state->وتھ));
            state->منوال = میل_2;
                /* fallthrough */
        حالة میل_2:
            إذا (بایاں == 0) اذهب inf_leave;
            copy = باہرلا - بایاں;
            إذا (state->وتھ > copy) {         /* copy from window */
                copy = state->وتھ - copy;
                إذا (copy > state->whave) {
                    إذا (state->sane) {
                        strm->پیغام = (z_const حرف *)
                            "invalid distance too far back";
                        state->منوال = BAD;
                        اكسر;
                    }
#جے_تعریف_شدہ INFLATE_ALLOW_INVALID_فاصلہ_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    إذا (copy > state->لمائی) copy = state->لمائی;
                    إذا (copy > بایاں) copy = بایاں;
                    بایاں -= copy;
                    state->لمائی -= copy;
                    افعل {
                        *put++ = 0;
                    } طالما (--copy);
                    إذا (state->لمائی == 0) state->منوال = LEN;
                    اكسر;
#جے_انت
                }
                إذا (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->کھڑکی + (state->wsize - copy);
                }
                وإلا
                    from = state->کھڑکی + (state->wnext - copy);
                إذا (copy > state->لمائی) copy = state->لمائی;
            }
            وإلا {                              /* copy from output */
                from = put - state->وتھ;
                copy = state->لمائی;
            }
            إذا (copy > بایاں) copy = بایاں;
            بایاں -= copy;
            state->لمائی -= copy;
            افعل {
                *put++ = *from++;
            } طالما (--copy);
            إذا (state->لمائی == 0) state->منوال = LEN;
            اكسر;
        حالة LIT:
            إذا (بایاں == 0) اذهب inf_leave;
            *put++ = (بنانشانی حرف)(state->لمائی);
            بایاں--;
            state->منوال = LEN;
            اكسر;
        حالة CHECK:
            إذا (state->wrap) {
                NEEDBITS(32);
                باہرلا -= بایاں;
                strm->کل_باہرلا += باہرلا;
                state->کل += باہرلا;
                إذا ((state->wrap & 4) && باہرلا)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - باہرلا, باہرلا);
                باہرلا = بایاں;
                إذا ((state->wrap & 4) && (
#جے_تعریف_شدہ GUNZIP
                     state->flags ? hold :
#جے_انت
                     ZSWAP32(hold)) != state->check) {
                    strm->پیغام = (z_const حرف *)"incorrect data check";
                    state->منوال = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#جے_تعریف_شدہ GUNZIP
            state->منوال = لمائی_2;
                /* fallthrough */
        حالة لمائی_2:
            إذا (state->wrap && state->flags) {
                NEEDBITS(32);
                إذا ((state->wrap & 4) && hold != (state->کل & 0xffffffff)) {
                    strm->پیغام = (z_const حرف *)"incorrect length check";
                    state->منوال = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#جے_انت
            state->منوال = پورا;
                /* fallthrough */
        حالة پورا:
            ret = Z_دھارا_سرا;
            اذهب inf_leave;
        حالة BAD:
            ret = Z_انکڑے_ERROR;
            اذهب inf_leave;
        حالة MEM:
            أرجع Z_MEM_ERROR;
        حالة SYNC:
                /* fallthrough */
        افتراضي:
            أرجع Z_دھارا_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    إذا (state->wsize || (باہرلا != strm->avail_باہرلا && state->منوال < BAD &&
            (state->منوال < CHECK || flush != Z_انت)))
        إذا (updatewindow(strm, strm->اگلا_باہرلا, باہرلا - strm->avail_باہرلا)) {
            state->منوال = MEM;
            أرجع Z_MEM_ERROR;
        }
    وچ -= strm->avail_وچ;
    باہرلا -= strm->avail_باہرلا;
    strm->کل_وچ += وچ;
    strm->کل_باہرلا += باہرلا;
    state->کل += باہرلا;
    إذا ((state->wrap & 4) && باہرلا)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->اگلا_باہرلا - باہرلا, باہرلا);
    strm->انکڑے_type = (صحيح)state->بٹ + (state->آخری ? 64 : 0) +
                      (state->منوال == TYPE ? 128 : 0) +
                      (state->منوال == LEN_ || state->منوال == COPY_ ? 256 : 0);
    إذا (((وچ == 0 && باہرلا == 0) || flush == Z_انت) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    أرجع ret;
}

صحيح ZEXPORT inflate_سرا(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm))
        أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->کھڑکی != Z_NULL) ZFREE(strm, state->کھڑکی);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_لمائی) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* copy dictionary */
    إذا (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->کھڑکی + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->کھڑکی, state->wnext);
    }
    إذا (dict_لمائی != Z_NULL)
        *dict_لمائی = state->whave;
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_سیٹ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt dict_لمائی) {
    بنية inflate_state FAR *state;
    بنانشانی لما dictid;
    صحيح ret;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->wrap != 0 && state->منوال != DICT)
        أرجع Z_دھارا_ERROR;

    /* check for correct dictionary identifier */
    إذا (state->منوال == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_لمائی);
        إذا (dictid != state->check)
            أرجع Z_انکڑے_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_لمائی, dict_لمائی);
    إذا (ret) {
        state->منوال = MEM;
        أرجع Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_Get_سرخی(z_streamp strm, gz_headerp سر) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا ((state->wrap & 2) == 0) أرجع Z_دھارا_ERROR;

    /* save header structure */
    state->سر = سر;
    سر->پورا_2 = 0;
    أرجع Z_OK;
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
local بنانشانی syncsearch(بنانشانی FAR *have, ثابت بنانشانی حرف FAR *buf,
                          بنانشانی len) {
    بنانشانی got;
    بنانشانی اگلا;

    got = *have;
    اگلا = 0;
    طالما (اگلا < len && got < 4) {
        إذا ((صحيح)(buf[اگلا]) == (got < 2 ? 0 : 0xff))
            got++;
        وإلا إذا (buf[اگلا])
            got = 0;
        وإلا
            got = 4 - got;
        اگلا++;
    }
    *have = got;
    أرجع اگلا;
}

صحيح ZEXPORT inflateSync(z_streamp strm) {
    بنانشانی len;               /* number of bytes to look at or looked at */
    صحيح flags;                  /* temporary to save header status */
    بنانشانی لما وچ, باہرلا;      /* temporary to save total_in and total_out */
    بنانشانی حرف buf[4];       /* to restore bit buffer to byte string */
    بنية inflate_state FAR *state;

    /* check parameters */
    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (strm->avail_وچ == 0 && state->بٹ < 8) أرجع Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    إذا (state->منوال != SYNC) {
        state->منوال = SYNC;
        state->hold >>= state->بٹ & 7;
        state->بٹ -= state->بٹ & 7;
        len = 0;
        طالما (state->بٹ >= 8) {
            buf[len++] = (بنانشانی حرف)(state->hold);
            state->hold >>= 8;
            state->بٹ -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->اگلا_وچ, strm->avail_وچ);
    strm->avail_وچ -= len;
    strm->اگلا_وچ += len;
    strm->کل_وچ += len;

    /* return no joy or set up to restart inflate() on a new block */
    إذا (state->have != 4) أرجع Z_انکڑے_ERROR;
    إذا (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    وإلا
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    وچ = strm->کل_وچ;  باہرلا = strm->کل_باہرلا;
    inflateReset(strm);
    strm->کل_وچ = وچ;  strm->کل_باہرلا = باہرلا;
    state->flags = flags;
    state->منوال = TYPE;
    أرجع Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
صحيح ZEXPORT inflate_Sync_نقطہ(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع state->منوال == STORED && state->بٹ == 0;
}

صحيح ZEXPORT inflateCopy(z_streamp dest, z_streamp سروت) {
    بنية inflate_state FAR *state;
    بنية inflate_state FAR *copy;
    بنانشانی حرف FAR *کھڑکی;

    /* check input */
    إذا (inflateStateCheck(سروت) || dest == Z_NULL)
        أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)سروت->state;

    /* allocate space */
    copy = (بنية inflate_state FAR *)
           ZALLOC(سروت, 1, حجم(بنية inflate_state));
    إذا (copy == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(copy, حجم(بنية inflate_state));
    کھڑکی = Z_NULL;
    إذا (state->کھڑکی != Z_NULL) {
        کھڑکی = (بنانشانی حرف FAR *)
                 ZALLOC(سروت, 1U << state->wbits, حجم(بنانشانی حرف));
        إذا (کھڑکی == Z_NULL) {
            ZFREE(سروت, copy);
            أرجع Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, سروت, حجم(z_دھارا));
    zmemcpy(copy, state, حجم(بنية inflate_state));
    copy->strm = dest;
    إذا (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->اگلا = copy->codes + (state->اگلا - state->codes);
    إذا (کھڑکی != Z_NULL)
        zmemcpy(کھڑکی, state->کھڑکی, state->whave);
    copy->کھڑکی = کھڑکی;
    dest->state = (بنية اندرونی_state FAR *)copy;
    أرجع Z_OK;
}

صحيح ZEXPORT inflateUndermine(z_streamp strm, صحيح subvert) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
#جے_تعریف_شدہ INFLATE_ALLOW_INVALID_فاصلہ_TOOFAR_ARRR
    state->sane = !subvert;
    أرجع Z_OK;
#وإلا
    (فراغ)subvert;
    state->sane = 1;
    أرجع Z_انکڑے_ERROR;
#جے_انت
}

صحيح ZEXPORT inflateValidate(z_streamp strm, صحيح check) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (check && state->wrap)
        state->wrap |= 4;
    وإلا
        state->wrap &= ~4;
    أرجع Z_OK;
}

لما ZEXPORT inflateMark(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm))
        أرجع -(1L << 16);
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (لما)(((بنانشانی لما)((لما)state->پٹھ)) << 16) +
        (state->منوال == COPY ? state->لمائی :
            (state->منوال == میل_2 ? state->was - state->لمائی : 0));
}

بنانشانی لما ZEXPORT inflate_Codes_ورتیا(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm)) أرجع (بنانشانی لما)-1;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (بنانشانی لما)(state->اگلا - state->codes);
}
