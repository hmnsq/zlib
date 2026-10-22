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
        state->نما < سر_2 || state->نما > SYNC)
        أرجع 1;
    أرجع 0;
}

صحيح ZEXPORT inflateResetKeep(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    strm->مجموع_درون = strm->مجموع_بیرون = state->مجموع = 0;
    strm->پیام = Z_NULL;
    strm->داده_type = 0;
    إذا (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->نما = سر_2;
    state->آخرین = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->سر = Z_NULL;
    state->hold = 0;
    state->بیتها = 0;
    state->lencode = state->distcode = state->بعدی = state->codes;
    state->sane = 1;
    state->پشت = -1;
    Tracev((stderr, "inflate: reset\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateReset(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    أرجع inflateResetKeep(strm);
}

صحيح ZEXPORT inflateReset2(z_streamp strm, صحيح پنجره_بیتها) {
    صحيح wrap;
    بنية inflate_state FAR *state;

    /* get the state */
    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    إذا (پنجره_بیتها < 0) {
        إذا (پنجره_بیتها < -15)
            أرجع Z_جریان_ERROR;
        wrap = 0;
        پنجره_بیتها = -پنجره_بیتها;
    }
    وإلا {
        wrap = (پنجره_بیتها >> 4) + 5;
#اگر_تعریف_شده GUNZIP
        إذا (پنجره_بیتها < 48)
            پنجره_بیتها &= 15;
#پایان_اگر
    }

    /* set number of window bits, free window if different */
    إذا (پنجره_بیتها && (پنجره_بیتها < 8 || پنجره_بیتها > 15))
        أرجع Z_جریان_ERROR;
    إذا (state->پنجره != Z_NULL && state->wbits != (بیعلامت)پنجره_بیتها) {
        ZFREE(strm, state->پنجره);
        state->پنجره = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (بیعلامت)پنجره_بیتها;
    أرجع inflateReset(strm);
}

صحيح ZEXPORT inflateInit2_(z_streamp strm, صحيح پنجره_بیتها,
                          ثابت حرف *نسخه, صحيح جریان_اندازه) {
    صحيح ret;
    بنية inflate_state FAR *state;

    إذا (نسخه == Z_NULL || نسخه[0] != ZLIB_نسخه[0] ||
        جریان_اندازه != (صحيح)(حجم(z_جریان)))
        أرجع Z_نسخه_ERROR;
    إذا (strm == Z_NULL) أرجع Z_جریان_ERROR;
    strm->پیام = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#اگر_تعریف_شده Z_SOLO
        أرجع Z_جریان_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#پایان_اگر
    }
    إذا (strm->zfree == (free_func)0)
#اگر_تعریف_شده Z_SOLO
        أرجع Z_جریان_ERROR;
#وإلا
        strm->zfree = zcfree;
#پایان_اگر
    state = (بنية inflate_state FAR *)
            ZALLOC(strm, 1, حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(state, حجم(بنية inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية داخلی_state FAR *)state;
    state->strm = strm;
    state->پنجره = Z_NULL;
    state->نما = سر_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, پنجره_بیتها);
    إذا (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    أرجع ret;
}

صحيح ZEXPORT inflateInit_(z_streamp strm, ثابت حرف *نسخه,
                         صحيح جریان_اندازه) {
    أرجع inflateInit2_(strm, DEF_WBITS, نسخه, جریان_اندازه);
}

صحيح ZEXPORT inflate_اول(z_streamp strm, صحيح بیتها, صحيح مقدار) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    إذا (بیتها == 0)
        أرجع Z_OK;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (بیتها < 0) {
        state->hold = 0;
        state->بیتها = 0;
        أرجع Z_OK;
    }
    إذا (بیتها > 16 || state->بیتها + (uInt)بیتها > 32) أرجع Z_جریان_ERROR;
    مقدار &= (1L << بیتها) - 1;
    state->hold += (بیعلامت دراز)مقدار << state->بیتها;
    state->بیتها += (uInt)بیتها;
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
local صحيح updatewindow(z_streamp strm, ثابت Bytef *انتها, بیعلامت copy) {
    بنية inflate_state FAR *state;
    بیعلامت dist;

    state = (بنية inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    إذا (state->پنجره == Z_NULL) {
        state->پنجره = (بیعلامت حرف FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               حجم(بیعلامت حرف));
        إذا (state->پنجره == Z_NULL) أرجع 1;
    }

    /* if window not in use yet, initialize */
    إذا (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    إذا (copy >= state->wsize) {
        zmemcpy(state->پنجره, انتها - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    وإلا {
        dist = state->wsize - state->wnext;
        إذا (dist > copy) dist = copy;
        zmemcpy(state->پنجره + state->wnext, انتها - copy, dist);
        copy -= dist;
        إذا (copy) {
            zmemcpy(state->پنجره, انتها - copy, copy);
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
#اگر_تعریف_شده GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? بررسی_چرخهای_32_2(check, buf, len) : adler32(check, buf, len))
#وإلا
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#پایان_اگر

/* check macros for header crc */
#اگر_تعریف_شده GUNZIP
#  define CRC2(check, کلمه) \
    افعل { \
        hbuf[0] = (بیعلامت حرف)(کلمه); \
        hbuf[1] = (بیعلامت حرف)((کلمه) >> 8); \
        check = بررسی_چرخهای_32_2(check, hbuf, 2); \
    } طالما (0)

#  define CRC4(check, کلمه) \
    افعل { \
        hbuf[0] = (بیعلامت حرف)(کلمه); \
        hbuf[1] = (بیعلامت حرف)((کلمه) >> 8); \
        hbuf[2] = (بیعلامت حرف)((کلمه) >> 16); \
        hbuf[3] = (بیعلامت حرف)((کلمه) >> 24); \
        check = بررسی_چرخهای_32_2(check, hbuf, 4); \
    } طالما (0)
#پایان_اگر

/* Load registers with state in inflate() for speed */
#تعریف_کن بارگیری_2() \
    افعل { \
        put = strm->بعدی_بیرون; \
        چپ = strm->avail_بیرون; \
        بعدی = strm->بعدی_درون; \
        have = strm->avail_درون; \
        hold = state->hold; \
        بیتها = state->بیتها; \
    } طالما (0)

/* Restore state from registers in inflate() */
#تعریف_کن RESTORE() \
    افعل { \
        strm->بعدی_بیرون = put; \
        strm->avail_بیرون = چپ; \
        strm->بعدی_درون = بعدی; \
        strm->avail_درون = have; \
        state->hold = hold; \
        state->بیتها = بیتها; \
    } طالما (0)

/* Clear the input bit accumulator */
#تعریف_کن INITBITS() \
    افعل { \
        hold = 0; \
        بیتها = 0; \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#تعریف_کن PULLBYTE() \
    افعل { \
        إذا (have == 0) اذهب inf_leave; \
        have--; \
        hold += (بیعلامت دراز)(*بعدی++) << بیتها; \
        بیتها += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#تعریف_کن NEEDBITS(n) \
    افعل { \
        طالما (بیتها < (بیعلامت)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کن بیتها_2(n) \
    ((بیعلامت)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کن DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بیتها -= (بیعلامت)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کن BYTEBITS() \
    افعل { \
        hold >>= بیتها & 7; \
        بیتها -= بیتها & 7; \
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
    z_const بیعلامت حرف FAR *بعدی;    /* next input */
    بیعلامت حرف FAR *put;     /* next output */
    بیعلامت have, چپ;        /* available input and output */
    بیعلامت دراز hold;         /* bit buffer */
    بیعلامت بیتها;              /* bits in bit buffer */
    بیعلامت درون, بیرون;           /* save starting available input and output */
    بیعلامت copy;              /* number of stored or match bytes to copy */
    بیعلامت حرف FAR *from;    /* where to copy match bytes from */
    کد اینجا;                  /* current decoding table entry */
    کد آخرین;                  /* parent table entry */
    بیعلامت len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
#اگر_تعریف_شده GUNZIP
    بیعلامت حرف hbuf[4];      /* buffer for gzip header crc calculation */
#پایان_اگر
    ساكن ثابت بیعلامت کوتاه ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    إذا (inflateStateCheck(strm) || strm->بعدی_بیرون == Z_NULL ||
        (strm->بعدی_درون == Z_NULL && strm->avail_درون != 0))
        أرجع Z_جریان_ERROR;

    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->نما == TYPE) state->نما = TYPEDO;      /* skip check */
    بارگیری_2();
    درون = have;
    بیرون = چپ;
    ret = Z_OK;
    لكل (;;)
        بدل (state->نما) {
        حالة سر_2:
            إذا (state->wrap == 0) {
                state->نما = TYPEDO;
                اكسر;
            }
            NEEDBITS(16);
#اگر_تعریف_شده GUNZIP
            إذا ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                إذا (state->wbits == 0)
                    state->wbits = 15;
                state->check = بررسی_چرخهای_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->نما = FLAGS;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->انجامشده_2 = -1;
            إذا (!(state->wrap & 1) ||   /* check if zlib header allowed */
#وإلا
            إذا (
#پایان_اگر
                ((بیتها_2(8) << 8) + (hold >> 8)) % 31) {
                strm->پیام = (z_const حرف *)"incorrect header check";
                state->نما = BAD;
                اكسر;
            }
            إذا (بیتها_2(4) != Z_DEFLATED) {
                strm->پیام = (z_const حرف *)"unknown compression method";
                state->نما = BAD;
                اكسر;
            }
            DROPBITS(4);
            len = بیتها_2(4) + 8;
            إذا (state->wbits == 0)
                state->wbits = len;
            إذا (len > 15 || len > state->wbits) {
                strm->پیام = (z_const حرف *)"invalid window size";
                state->نما = BAD;
                اكسر;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->نما = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            اكسر;
#اگر_تعریف_شده GUNZIP
        حالة FLAGS:
            NEEDBITS(16);
            state->flags = (صحيح)(hold);
            إذا ((state->flags & 0xff) != Z_DEFLATED) {
                strm->پیام = (z_const حرف *)"unknown compression method";
                state->نما = BAD;
                اكسر;
            }
            إذا (state->flags & 0xe000) {
                strm->پیام = (z_const حرف *)"unknown header flags set";
                state->نما = BAD;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->متن = (صحيح)((hold >> 8) & 1);
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->نما = زمان;
                /* fallthrough */
        حالة زمان:
            NEEDBITS(32);
            إذا (state->سر != Z_NULL)
                state->سر->time = hold;
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->نما = OS;
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
            state->نما = EXLEN;
                /* fallthrough */
        حالة EXLEN:
            إذا (state->flags & 0x0400) {
                NEEDBITS(16);
                state->درازی = (بیعلامت)(hold);
                إذا (state->سر != Z_NULL)
                    state->سر->extra_len = (بیعلامت)hold;
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->extra = Z_NULL;
            state->نما = EXTRA;
                /* fallthrough */
        حالة EXTRA:
            إذا (state->flags & 0x0400) {
                copy = state->درازی;
                إذا (copy > have) copy = have;
                إذا (copy) {
                    إذا (state->سر != Z_NULL &&
                        state->سر->extra != Z_NULL &&
                        (len = state->سر->extra_len - state->درازی) <
                            state->سر->extra_max) {
                        zmemcpy(state->سر->extra + len, بعدی,
                                len + copy > state->سر->extra_max ?
                                state->سر->extra_max - len : copy);
                    }
                    إذا ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = بررسی_چرخهای_32_2(state->check, بعدی, copy);
                    have -= copy;
                    بعدی += copy;
                    state->درازی -= copy;
                }
                إذا (state->درازی) اذهب inf_leave;
            }
            state->درازی = 0;
            state->نما = NAME;
                /* fallthrough */
        حالة NAME:
            إذا (state->flags & 0x0800) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بیعلامت)(بعدی[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->name != Z_NULL &&
                            state->درازی < state->سر->name_max)
                        state->سر->name[state->درازی++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = بررسی_چرخهای_32_2(state->check, بعدی, copy);
                have -= copy;
                بعدی += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->name = Z_NULL;
            state->درازی = 0;
            state->نما = COMMENT;
                /* fallthrough */
        حالة COMMENT:
            إذا (state->flags & 0x1000) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بیعلامت)(بعدی[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->comment != Z_NULL &&
                            state->درازی < state->سر->comm_max)
                        state->سر->comment[state->درازی++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = بررسی_چرخهای_32_2(state->check, بعدی, copy);
                have -= copy;
                بعدی += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->comment = Z_NULL;
            state->نما = HCRC;
                /* fallthrough */
        حالة HCRC:
            إذا (state->flags & 0x0200) {
                NEEDBITS(16);
                إذا ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->پیام = (z_const حرف *)"header crc mismatch";
                    state->نما = BAD;
                    اكسر;
                }
                INITBITS();
            }
            إذا (state->سر != Z_NULL) {
                state->سر->hcrc = (صحيح)((state->flags >> 9) & 1);
                state->سر->انجامشده_2 = 1;
            }
            strm->adler = state->check = بررسی_چرخهای_32_2(0L, Z_NULL, 0);
            state->نما = TYPE;
            اكسر;
#پایان_اگر
        حالة DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->نما = DICT;
                /* fallthrough */
        حالة DICT:
            إذا (state->havedict == 0) {
                RESTORE();
                أرجع Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->نما = TYPE;
                /* fallthrough */
        حالة TYPE:
            إذا (flush == Z_بلوک || flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة TYPEDO:
            إذا (state->آخرین) {
                BYTEBITS();
                state->نما = CHECK;
                اكسر;
            }
            NEEDBITS(3);
            state->آخرین = بیتها_2(1);
            DROPBITS(1);
            بدل (بیتها_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخرین ? " (last)" : ""));
                state->نما = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخرین ? " (last)" : ""));
                state->نما = LEN_;             /* decode codes */
                إذا (flush == Z_TREES) {
                    DROPBITS(2);
                    اذهب inf_leave;
                }
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخرین ? " (last)" : ""));
                state->نما = جدول;
                اكسر;
            افتراضي:
                strm->پیام = (z_const حرف *)"invalid block type";
                state->نما = BAD;
            }
            DROPBITS(2);
            اكسر;
        حالة STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیام = (z_const حرف *)"invalid stored block lengths";
                state->نما = BAD;
                اكسر;
            }
            state->درازی = (بیعلامت)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->درازی));
            INITBITS();
            state->نما = COPY_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة COPY_:
            state->نما = COPY;
                /* fallthrough */
        حالة COPY:
            copy = state->درازی;
            إذا (copy) {
                إذا (copy > have) copy = have;
                إذا (copy > چپ) copy = چپ;
                إذا (copy == 0) اذهب inf_leave;
                zmemcpy(put, بعدی, copy);
                have -= copy;
                بعدی += copy;
                چپ -= copy;
                put += copy;
                state->درازی -= copy;
                اكسر;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->نما = TYPE;
            اكسر;
        حالة جدول:
            NEEDBITS(14);
            state->nlen = بیتها_2(5) + 257;
            DROPBITS(5);
            state->ndist = بیتها_2(5) + 1;
            DROPBITS(5);
            state->ncode = بیتها_2(4) + 4;
            DROPBITS(4);
#اگر_تعریف_نشده PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->پیام = (z_const حرف *)
                    "too many length or distance symbols";
                state->نما = BAD;
                اكسر;
            }
#پایان_اگر
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->نما = LENLENS;
                /* fallthrough */
        حالة LENLENS:
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (بیعلامت کوتاه)بیتها_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->بعدی = state->codes;
            state->lencode = state->distcode = (ثابت کد FAR *)(state->بعدی);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->بعدی),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid code lengths set";
                state->نما = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->نما = CODELENS;
                /* fallthrough */
        حالة CODELENS:
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    اینجا = state->lencode[بیتها_2(state->lenbits)];
                    إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                إذا (اینجا.val < 16) {
                    DROPBITS(اینجا.بیتها);
                    state->lens[state->have++] = اینجا.val;
                }
                وإلا {
                    إذا (اینجا.val == 16) {
                        NEEDBITS(اینجا.بیتها + 2);
                        DROPBITS(اینجا.بیتها);
                        إذا (state->have == 0) {
                            strm->پیام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->نما = BAD;
                            اكسر;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + بیتها_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (اینجا.val == 17) {
                        NEEDBITS(اینجا.بیتها + 3);
                        DROPBITS(اینجا.بیتها);
                        len = 0;
                        copy = 3 + بیتها_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(اینجا.بیتها + 7);
                        DROPBITS(اینجا.بیتها);
                        len = 0;
                        copy = 11 + بیتها_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->پیام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->نما = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بیعلامت کوتاه)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->نما == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->نما = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->بعدی = state->codes;
            state->lencode = (ثابت کد FAR *)(state->بعدی);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->بعدی),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid literal/lengths set";
                state->نما = BAD;
                اكسر;
            }
            state->distcode = (ثابت کد FAR *)(state->بعدی);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->بعدی), &(state->distbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid distances set";
                state->نما = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->نما = LEN_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة LEN_:
            state->نما = LEN;
                /* fallthrough */
        حالة LEN:
            إذا (have >= 6 && چپ >= 258) {
                RESTORE();
                inflate_fast(strm, بیرون);
                بارگیری_2();
                إذا (state->نما == TYPE)
                    state->پشت = -1;
                اكسر;
            }
            state->پشت = 0;
            لكل (;;) {
                اینجا = state->lencode[بیتها_2(state->lenbits)];
                إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                PULLBYTE();
            }
            إذا (اینجا.op && (اینجا.op & 0xf0) == 0) {
                آخرین = اینجا;
                لكل (;;) {
                    اینجا = state->lencode[آخرین.val +
                            (بیتها_2(آخرین.بیتها + آخرین.op) >> آخرین.بیتها)];
                    إذا ((بیعلامت)(آخرین.بیتها + اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخرین.بیتها);
                state->پشت += آخرین.بیتها;
            }
            DROPBITS(اینجا.بیتها);
            state->پشت += اینجا.بیتها;
            state->درازی = (بیعلامت)اینجا.val;
            إذا ((صحيح)(اینجا.op) == 0) {
                Tracevv((stderr, اینجا.val >= 0x20 && اینجا.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", اینجا.val));
                state->نما = LIT;
                اكسر;
            }
            إذا (اینجا.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->پشت = -1;
                state->نما = TYPE;
                اكسر;
            }
            إذا (اینجا.op & 64) {
                strm->پیام = (z_const حرف *)"invalid literal/length code";
                state->نما = BAD;
                اكسر;
            }
            state->extra = (بیعلامت)(اینجا.op) & 15;
            state->نما = LENEXT;
                /* fallthrough */
        حالة LENEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->درازی += بیتها_2(state->extra);
                DROPBITS(state->extra);
                state->پشت += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->درازی));
            state->was = state->درازی;
            state->نما = DIST;
                /* fallthrough */
        حالة DIST:
            لكل (;;) {
                اینجا = state->distcode[بیتها_2(state->distbits)];
                إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                PULLBYTE();
            }
            إذا ((اینجا.op & 0xf0) == 0) {
                آخرین = اینجا;
                لكل (;;) {
                    اینجا = state->distcode[آخرین.val +
                            (بیتها_2(آخرین.بیتها + آخرین.op) >> آخرین.بیتها)];
                    إذا ((بیعلامت)(آخرین.بیتها + اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخرین.بیتها);
                state->پشت += آخرین.بیتها;
            }
            DROPBITS(اینجا.بیتها);
            state->پشت += اینجا.بیتها;
            إذا (اینجا.op & 64) {
                strm->پیام = (z_const حرف *)"invalid distance code";
                state->نما = BAD;
                اكسر;
            }
            state->جابه_جایی = (بیعلامت)اینجا.val;
            state->extra = (بیعلامت)(اینجا.op) & 15;
            state->نما = DISTEXT;
                /* fallthrough */
        حالة DISTEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->جابه_جایی += بیتها_2(state->extra);
                DROPBITS(state->extra);
                state->پشت += state->extra;
            }
#اگر_تعریف_شده INFLATE_STRICT
            إذا (state->جابه_جایی > state->dmax) {
                strm->پیام = (z_const حرف *)"invalid distance too far back";
                state->نما = BAD;
                اكسر;
            }
#پایان_اگر
            Tracevv((stderr, "inflate:         distance %u\n", state->جابه_جایی));
            state->نما = تطابق_2;
                /* fallthrough */
        حالة تطابق_2:
            إذا (چپ == 0) اذهب inf_leave;
            copy = بیرون - چپ;
            إذا (state->جابه_جایی > copy) {         /* copy from window */
                copy = state->جابه_جایی - copy;
                إذا (copy > state->whave) {
                    إذا (state->sane) {
                        strm->پیام = (z_const حرف *)
                            "invalid distance too far back";
                        state->نما = BAD;
                        اكسر;
                    }
#اگر_تعریف_شده INFLATE_ALLOW_INVALID_فاصله_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    إذا (copy > state->درازی) copy = state->درازی;
                    إذا (copy > چپ) copy = چپ;
                    چپ -= copy;
                    state->درازی -= copy;
                    افعل {
                        *put++ = 0;
                    } طالما (--copy);
                    إذا (state->درازی == 0) state->نما = LEN;
                    اكسر;
#پایان_اگر
                }
                إذا (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->پنجره + (state->wsize - copy);
                }
                وإلا
                    from = state->پنجره + (state->wnext - copy);
                إذا (copy > state->درازی) copy = state->درازی;
            }
            وإلا {                              /* copy from output */
                from = put - state->جابه_جایی;
                copy = state->درازی;
            }
            إذا (copy > چپ) copy = چپ;
            چپ -= copy;
            state->درازی -= copy;
            افعل {
                *put++ = *from++;
            } طالما (--copy);
            إذا (state->درازی == 0) state->نما = LEN;
            اكسر;
        حالة LIT:
            إذا (چپ == 0) اذهب inf_leave;
            *put++ = (بیعلامت حرف)(state->درازی);
            چپ--;
            state->نما = LEN;
            اكسر;
        حالة CHECK:
            إذا (state->wrap) {
                NEEDBITS(32);
                بیرون -= چپ;
                strm->مجموع_بیرون += بیرون;
                state->مجموع += بیرون;
                إذا ((state->wrap & 4) && بیرون)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - بیرون, بیرون);
                بیرون = چپ;
                إذا ((state->wrap & 4) && (
#اگر_تعریف_شده GUNZIP
                     state->flags ? hold :
#پایان_اگر
                     ZSWAP32(hold)) != state->check) {
                    strm->پیام = (z_const حرف *)"incorrect data check";
                    state->نما = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#اگر_تعریف_شده GUNZIP
            state->نما = درازی_2;
                /* fallthrough */
        حالة درازی_2:
            إذا (state->wrap && state->flags) {
                NEEDBITS(32);
                إذا ((state->wrap & 4) && hold != (state->مجموع & 0xffffffff)) {
                    strm->پیام = (z_const حرف *)"incorrect length check";
                    state->نما = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#پایان_اگر
            state->نما = انجامشده;
                /* fallthrough */
        حالة انجامشده:
            ret = Z_جریان_انتها;
            اذهب inf_leave;
        حالة BAD:
            ret = Z_داده_ERROR;
            اذهب inf_leave;
        حالة MEM:
            أرجع Z_MEM_ERROR;
        حالة SYNC:
                /* fallthrough */
        افتراضي:
            أرجع Z_جریان_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    إذا (state->wsize || (بیرون != strm->avail_بیرون && state->نما < BAD &&
            (state->نما < CHECK || flush != Z_پایان)))
        إذا (updatewindow(strm, strm->بعدی_بیرون, بیرون - strm->avail_بیرون)) {
            state->نما = MEM;
            أرجع Z_MEM_ERROR;
        }
    درون -= strm->avail_درون;
    بیرون -= strm->avail_بیرون;
    strm->مجموع_درون += درون;
    strm->مجموع_بیرون += بیرون;
    state->مجموع += بیرون;
    إذا ((state->wrap & 4) && بیرون)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->بعدی_بیرون - بیرون, بیرون);
    strm->داده_type = (صحيح)state->بیتها + (state->آخرین ? 64 : 0) +
                      (state->نما == TYPE ? 128 : 0) +
                      (state->نما == LEN_ || state->نما == COPY_ ? 256 : 0);
    إذا (((درون == 0 && بیرون == 0) || flush == Z_پایان) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    أرجع ret;
}

صحيح ZEXPORT inflate_انتها(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm))
        أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->پنجره != Z_NULL) ZFREE(strm, state->پنجره);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_درازی) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* copy dictionary */
    إذا (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->پنجره + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->پنجره, state->wnext);
    }
    إذا (dict_درازی != Z_NULL)
        *dict_درازی = state->whave;
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_مجموعه_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt dict_درازی) {
    بنية inflate_state FAR *state;
    بیعلامت دراز dictid;
    صحيح ret;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->wrap != 0 && state->نما != DICT)
        أرجع Z_جریان_ERROR;

    /* check for correct dictionary identifier */
    إذا (state->نما == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_درازی);
        إذا (dictid != state->check)
            أرجع Z_داده_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_درازی, dict_درازی);
    إذا (ret) {
        state->نما = MEM;
        أرجع Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_Get_سرآیند(z_streamp strm, gz_headerp سر) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا ((state->wrap & 2) == 0) أرجع Z_جریان_ERROR;

    /* save header structure */
    state->سر = سر;
    سر->انجامشده_2 = 0;
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
local بیعلامت syncsearch(بیعلامت FAR *have, ثابت بیعلامت حرف FAR *buf,
                          بیعلامت len) {
    بیعلامت got;
    بیعلامت بعدی;

    got = *have;
    بعدی = 0;
    طالما (بعدی < len && got < 4) {
        إذا ((صحيح)(buf[بعدی]) == (got < 2 ? 0 : 0xff))
            got++;
        وإلا إذا (buf[بعدی])
            got = 0;
        وإلا
            got = 4 - got;
        بعدی++;
    }
    *have = got;
    أرجع بعدی;
}

صحيح ZEXPORT inflateSync(z_streamp strm) {
    بیعلامت len;               /* number of bytes to look at or looked at */
    صحيح flags;                  /* temporary to save header status */
    بیعلامت دراز درون, بیرون;      /* temporary to save total_in and total_out */
    بیعلامت حرف buf[4];       /* to restore bit buffer to byte string */
    بنية inflate_state FAR *state;

    /* check parameters */
    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (strm->avail_درون == 0 && state->بیتها < 8) أرجع Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    إذا (state->نما != SYNC) {
        state->نما = SYNC;
        state->hold >>= state->بیتها & 7;
        state->بیتها -= state->بیتها & 7;
        len = 0;
        طالما (state->بیتها >= 8) {
            buf[len++] = (بیعلامت حرف)(state->hold);
            state->hold >>= 8;
            state->بیتها -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->بعدی_درون, strm->avail_درون);
    strm->avail_درون -= len;
    strm->بعدی_درون += len;
    strm->مجموع_درون += len;

    /* return no joy or set up to restart inflate() on a new block */
    إذا (state->have != 4) أرجع Z_داده_ERROR;
    إذا (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    وإلا
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    درون = strm->مجموع_درون;  بیرون = strm->مجموع_بیرون;
    inflateReset(strm);
    strm->مجموع_درون = درون;  strm->مجموع_بیرون = بیرون;
    state->flags = flags;
    state->نما = TYPE;
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
صحيح ZEXPORT inflate_Sync_نقطه(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع state->نما == STORED && state->بیتها == 0;
}

صحيح ZEXPORT inflateCopy(z_streamp dest, z_streamp منبع) {
    بنية inflate_state FAR *state;
    بنية inflate_state FAR *copy;
    بیعلامت حرف FAR *پنجره;

    /* check input */
    إذا (inflateStateCheck(منبع) || dest == Z_NULL)
        أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)منبع->state;

    /* allocate space */
    copy = (بنية inflate_state FAR *)
           ZALLOC(منبع, 1, حجم(بنية inflate_state));
    إذا (copy == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(copy, حجم(بنية inflate_state));
    پنجره = Z_NULL;
    إذا (state->پنجره != Z_NULL) {
        پنجره = (بیعلامت حرف FAR *)
                 ZALLOC(منبع, 1U << state->wbits, حجم(بیعلامت حرف));
        إذا (پنجره == Z_NULL) {
            ZFREE(منبع, copy);
            أرجع Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, منبع, حجم(z_جریان));
    zmemcpy(copy, state, حجم(بنية inflate_state));
    copy->strm = dest;
    إذا (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->بعدی = copy->codes + (state->بعدی - state->codes);
    إذا (پنجره != Z_NULL)
        zmemcpy(پنجره, state->پنجره, state->whave);
    copy->پنجره = پنجره;
    dest->state = (بنية داخلی_state FAR *)copy;
    أرجع Z_OK;
}

صحيح ZEXPORT inflateUndermine(z_streamp strm, صحيح subvert) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
#اگر_تعریف_شده INFLATE_ALLOW_INVALID_فاصله_TOOFAR_ARRR
    state->sane = !subvert;
    أرجع Z_OK;
#وإلا
    (فراغ)subvert;
    state->sane = 1;
    أرجع Z_داده_ERROR;
#پایان_اگر
}

صحيح ZEXPORT inflateValidate(z_streamp strm, صحيح check) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (check && state->wrap)
        state->wrap |= 4;
    وإلا
        state->wrap &= ~4;
    أرجع Z_OK;
}

دراز ZEXPORT inflateMark(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm))
        أرجع -(1L << 16);
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (دراز)(((بیعلامت دراز)((دراز)state->پشت)) << 16) +
        (state->نما == COPY ? state->درازی :
            (state->نما == تطابق_2 ? state->was - state->درازی : 0));
}

بیعلامت دراز ZEXPORT inflate_Codes_استفاده_شده(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm)) أرجع (بیعلامت دراز)-1;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (بیعلامت دراز)(state->بعدی - state->codes);
}
