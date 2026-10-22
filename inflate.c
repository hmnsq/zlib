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
        state->ډېرپېښ < سر_2 || state->ډېرپېښ > SYNC)
        أرجع 1;
    أرجع 0;
}

صحيح ZEXPORT inflateResetKeep(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    strm->ټول_دننه = strm->ټول_بهر = state->ټول = 0;
    strm->پیغام = Z_NULL;
    strm->معلومات_type = 0;
    إذا (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->ډېرپېښ = سر_2;
    state->وروستی = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->سر = Z_NULL;
    state->hold = 0;
    state->بټونه = 0;
    state->lencode = state->distcode = state->بل = state->codes;
    state->sane = 1;
    state->شا = -1;
    Tracev((stderr, "inflate: reset\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateReset(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    أرجع inflateResetKeep(strm);
}

صحيح ZEXPORT inflateReset2(z_streamp strm, صحيح کړکۍ_بټونه) {
    صحيح wrap;
    بنية inflate_state FAR *state;

    /* get the state */
    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    إذا (کړکۍ_بټونه < 0) {
        إذا (کړکۍ_بټونه < -15)
            أرجع Z_بهیر_ERROR;
        wrap = 0;
        کړکۍ_بټونه = -کړکۍ_بټونه;
    }
    وإلا {
        wrap = (کړکۍ_بټونه >> 4) + 5;
#که_تعریف_شوی GUNZIP
        إذا (کړکۍ_بټونه < 48)
            کړکۍ_بټونه &= 15;
#د_که_پای
    }

    /* set number of window bits, free window if different */
    إذا (کړکۍ_بټونه && (کړکۍ_بټونه < 8 || کړکۍ_بټونه > 15))
        أرجع Z_بهیر_ERROR;
    إذا (state->کړکۍ != Z_NULL && state->wbits != (بینښه)کړکۍ_بټونه) {
        ZFREE(strm, state->کړکۍ);
        state->کړکۍ = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (بینښه)کړکۍ_بټونه;
    أرجع inflateReset(strm);
}

صحيح ZEXPORT inflateInit2_(z_streamp strm, صحيح کړکۍ_بټونه,
                          ثابت حرف *نسخه, صحيح بهیر_اندازه) {
    صحيح ret;
    بنية inflate_state FAR *state;

    إذا (نسخه == Z_NULL || نسخه[0] != ZLIB_نسخه[0] ||
        بهیر_اندازه != (صحيح)(حجم(z_بهیر)))
        أرجع Z_نسخه_ERROR;
    إذا (strm == Z_NULL) أرجع Z_بهیر_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#که_تعریف_شوی Z_SOLO
        أرجع Z_بهیر_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#د_که_پای
    }
    إذا (strm->zfree == (free_func)0)
#که_تعریف_شوی Z_SOLO
        أرجع Z_بهیر_ERROR;
#وإلا
        strm->zfree = zcfree;
#د_که_پای
    state = (بنية inflate_state FAR *)
            ZALLOC(strm, 1, حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(state, حجم(بنية inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية دننی_state FAR *)state;
    state->strm = strm;
    state->کړکۍ = Z_NULL;
    state->ډېرپېښ = سر_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, کړکۍ_بټونه);
    إذا (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    أرجع ret;
}

صحيح ZEXPORT inflateInit_(z_streamp strm, ثابت حرف *نسخه,
                         صحيح بهیر_اندازه) {
    أرجع inflateInit2_(strm, DEF_WBITS, نسخه, بهیر_اندازه);
}

صحيح ZEXPORT inflate_اولي(z_streamp strm, صحيح بټونه, صحيح ارزښت) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    إذا (بټونه == 0)
        أرجع Z_OK;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (بټونه < 0) {
        state->hold = 0;
        state->بټونه = 0;
        أرجع Z_OK;
    }
    إذا (بټونه > 16 || state->بټونه + (uInt)بټونه > 32) أرجع Z_بهیر_ERROR;
    ارزښت &= (1L << بټونه) - 1;
    state->hold += (بینښه اوږد)ارزښت << state->بټونه;
    state->بټونه += (uInt)بټونه;
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
local صحيح updatewindow(z_streamp strm, ثابت Bytef *پای_ټکی, بینښه copy) {
    بنية inflate_state FAR *state;
    بینښه dist;

    state = (بنية inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    إذا (state->کړکۍ == Z_NULL) {
        state->کړکۍ = (بینښه حرف FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               حجم(بینښه حرف));
        إذا (state->کړکۍ == Z_NULL) أرجع 1;
    }

    /* if window not in use yet, initialize */
    إذا (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    إذا (copy >= state->wsize) {
        zmemcpy(state->کړکۍ, پای_ټکی - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    وإلا {
        dist = state->wsize - state->wnext;
        إذا (dist > copy) dist = copy;
        zmemcpy(state->کړکۍ + state->wnext, پای_ټکی - copy, dist);
        copy -= dist;
        إذا (copy) {
            zmemcpy(state->کړکۍ, پای_ټکی - copy, copy);
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
#که_تعریف_شوی GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? دوراني_ازموینه_32_2(check, buf, len) : adler32(check, buf, len))
#وإلا
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#د_که_پای

/* check macros for header crc */
#که_تعریف_شوی GUNZIP
#  define CRC2(check, کلمه) \
    افعل { \
        hbuf[0] = (بینښه حرف)(کلمه); \
        hbuf[1] = (بینښه حرف)((کلمه) >> 8); \
        check = دوراني_ازموینه_32_2(check, hbuf, 2); \
    } طالما (0)

#  define CRC4(check, کلمه) \
    افعل { \
        hbuf[0] = (بینښه حرف)(کلمه); \
        hbuf[1] = (بینښه حرف)((کلمه) >> 8); \
        hbuf[2] = (بینښه حرف)((کلمه) >> 16); \
        hbuf[3] = (بینښه حرف)((کلمه) >> 24); \
        check = دوراني_ازموینه_32_2(check, hbuf, 4); \
    } طالما (0)
#د_که_پای

/* Load registers with state in inflate() for speed */
#تعریف_کړه بارول_2() \
    افعل { \
        put = strm->بل_بهر; \
        کیڼ = strm->avail_بهر; \
        بل = strm->بل_دننه; \
        have = strm->avail_دننه; \
        hold = state->hold; \
        بټونه = state->بټونه; \
    } طالما (0)

/* Restore state from registers in inflate() */
#تعریف_کړه RESTORE() \
    افعل { \
        strm->بل_بهر = put; \
        strm->avail_بهر = کیڼ; \
        strm->بل_دننه = بل; \
        strm->avail_دننه = have; \
        state->hold = hold; \
        state->بټونه = بټونه; \
    } طالما (0)

/* Clear the input bit accumulator */
#تعریف_کړه INITBITS() \
    افعل { \
        hold = 0; \
        بټونه = 0; \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#تعریف_کړه PULLBYTE() \
    افعل { \
        إذا (have == 0) اذهب inf_leave; \
        have--; \
        hold += (بینښه اوږد)(*بل++) << بټونه; \
        بټونه += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#تعریف_کړه NEEDBITS(n) \
    افعل { \
        طالما (بټونه < (بینښه)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کړه بټونه_2(n) \
    ((بینښه)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کړه DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بټونه -= (بینښه)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کړه BYTEBITS() \
    افعل { \
        hold >>= بټونه & 7; \
        بټونه -= بټونه & 7; \
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
    z_const بینښه حرف FAR *بل;    /* next input */
    بینښه حرف FAR *put;     /* next output */
    بینښه have, کیڼ;        /* available input and output */
    بینښه اوږد hold;         /* bit buffer */
    بینښه بټونه;              /* bits in bit buffer */
    بینښه دننه, بهر;           /* save starting available input and output */
    بینښه copy;              /* number of stored or match bytes to copy */
    بینښه حرف FAR *from;    /* where to copy match bytes from */
    کوډ دلته;                  /* current decoding table entry */
    کوډ وروستی;                  /* parent table entry */
    بینښه len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
#که_تعریف_شوی GUNZIP
    بینښه حرف hbuf[4];      /* buffer for gzip header crc calculation */
#د_که_پای
    ساكن ثابت بینښه لنډ ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    إذا (inflateStateCheck(strm) || strm->بل_بهر == Z_NULL ||
        (strm->بل_دننه == Z_NULL && strm->avail_دننه != 0))
        أرجع Z_بهیر_ERROR;

    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->ډېرپېښ == TYPE) state->ډېرپېښ = TYPEDO;      /* skip check */
    بارول_2();
    دننه = have;
    بهر = کیڼ;
    ret = Z_OK;
    لكل (;;)
        بدل (state->ډېرپېښ) {
        حالة سر_2:
            إذا (state->wrap == 0) {
                state->ډېرپېښ = TYPEDO;
                اكسر;
            }
            NEEDBITS(16);
#که_تعریف_شوی GUNZIP
            إذا ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                إذا (state->wbits == 0)
                    state->wbits = 15;
                state->check = دوراني_ازموینه_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->ډېرپېښ = FLAGS;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->بشپړ_2 = -1;
            إذا (!(state->wrap & 1) ||   /* check if zlib header allowed */
#وإلا
            إذا (
#د_که_پای
                ((بټونه_2(8) << 8) + (hold >> 8)) % 31) {
                strm->پیغام = (z_const حرف *)"incorrect header check";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            إذا (بټونه_2(4) != Z_DEFLATED) {
                strm->پیغام = (z_const حرف *)"unknown compression method";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            DROPBITS(4);
            len = بټونه_2(4) + 8;
            إذا (state->wbits == 0)
                state->wbits = len;
            إذا (len > 15 || len > state->wbits) {
                strm->پیغام = (z_const حرف *)"invalid window size";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->ډېرپېښ = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            اكسر;
#که_تعریف_شوی GUNZIP
        حالة FLAGS:
            NEEDBITS(16);
            state->flags = (صحيح)(hold);
            إذا ((state->flags & 0xff) != Z_DEFLATED) {
                strm->پیغام = (z_const حرف *)"unknown compression method";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            إذا (state->flags & 0xe000) {
                strm->پیغام = (z_const حرف *)"unknown header flags set";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->متن = (صحيح)((hold >> 8) & 1);
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->ډېرپېښ = وخت;
                /* fallthrough */
        حالة وخت:
            NEEDBITS(32);
            إذا (state->سر != Z_NULL)
                state->سر->time = hold;
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->ډېرپېښ = OS;
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
            state->ډېرپېښ = EXLEN;
                /* fallthrough */
        حالة EXLEN:
            إذا (state->flags & 0x0400) {
                NEEDBITS(16);
                state->اوږدتیا = (بینښه)(hold);
                إذا (state->سر != Z_NULL)
                    state->سر->extra_len = (بینښه)hold;
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->extra = Z_NULL;
            state->ډېرپېښ = EXTRA;
                /* fallthrough */
        حالة EXTRA:
            إذا (state->flags & 0x0400) {
                copy = state->اوږدتیا;
                إذا (copy > have) copy = have;
                إذا (copy) {
                    إذا (state->سر != Z_NULL &&
                        state->سر->extra != Z_NULL &&
                        (len = state->سر->extra_len - state->اوږدتیا) <
                            state->سر->extra_max) {
                        zmemcpy(state->سر->extra + len, بل,
                                len + copy > state->سر->extra_max ?
                                state->سر->extra_max - len : copy);
                    }
                    إذا ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = دوراني_ازموینه_32_2(state->check, بل, copy);
                    have -= copy;
                    بل += copy;
                    state->اوږدتیا -= copy;
                }
                إذا (state->اوږدتیا) اذهب inf_leave;
            }
            state->اوږدتیا = 0;
            state->ډېرپېښ = NAME;
                /* fallthrough */
        حالة NAME:
            إذا (state->flags & 0x0800) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بینښه)(بل[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->name != Z_NULL &&
                            state->اوږدتیا < state->سر->name_max)
                        state->سر->name[state->اوږدتیا++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = دوراني_ازموینه_32_2(state->check, بل, copy);
                have -= copy;
                بل += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->name = Z_NULL;
            state->اوږدتیا = 0;
            state->ډېرپېښ = COMMENT;
                /* fallthrough */
        حالة COMMENT:
            إذا (state->flags & 0x1000) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بینښه)(بل[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->comment != Z_NULL &&
                            state->اوږدتیا < state->سر->comm_max)
                        state->سر->comment[state->اوږدتیا++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = دوراني_ازموینه_32_2(state->check, بل, copy);
                have -= copy;
                بل += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->comment = Z_NULL;
            state->ډېرپېښ = HCRC;
                /* fallthrough */
        حالة HCRC:
            إذا (state->flags & 0x0200) {
                NEEDBITS(16);
                إذا ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->پیغام = (z_const حرف *)"header crc mismatch";
                    state->ډېرپېښ = BAD;
                    اكسر;
                }
                INITBITS();
            }
            إذا (state->سر != Z_NULL) {
                state->سر->hcrc = (صحيح)((state->flags >> 9) & 1);
                state->سر->بشپړ_2 = 1;
            }
            strm->adler = state->check = دوراني_ازموینه_32_2(0L, Z_NULL, 0);
            state->ډېرپېښ = TYPE;
            اكسر;
#د_که_پای
        حالة DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->ډېرپېښ = DICT;
                /* fallthrough */
        حالة DICT:
            إذا (state->havedict == 0) {
                RESTORE();
                أرجع Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->ډېرپېښ = TYPE;
                /* fallthrough */
        حالة TYPE:
            إذا (flush == Z_بلاک || flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة TYPEDO:
            إذا (state->وروستی) {
                BYTEBITS();
                state->ډېرپېښ = CHECK;
                اكسر;
            }
            NEEDBITS(3);
            state->وروستی = بټونه_2(1);
            DROPBITS(1);
            بدل (بټونه_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->وروستی ? " (last)" : ""));
                state->ډېرپېښ = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->وروستی ? " (last)" : ""));
                state->ډېرپېښ = LEN_;             /* decode codes */
                إذا (flush == Z_TREES) {
                    DROPBITS(2);
                    اذهب inf_leave;
                }
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->وروستی ? " (last)" : ""));
                state->ډېرپېښ = جدول;
                اكسر;
            افتراضي:
                strm->پیغام = (z_const حرف *)"invalid block type";
                state->ډېرپېښ = BAD;
            }
            DROPBITS(2);
            اكسر;
        حالة STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->اوږدتیا = (بینښه)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->اوږدتیا));
            INITBITS();
            state->ډېرپېښ = COPY_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة COPY_:
            state->ډېرپېښ = COPY;
                /* fallthrough */
        حالة COPY:
            copy = state->اوږدتیا;
            إذا (copy) {
                إذا (copy > have) copy = have;
                إذا (copy > کیڼ) copy = کیڼ;
                إذا (copy == 0) اذهب inf_leave;
                zmemcpy(put, بل, copy);
                have -= copy;
                بل += copy;
                کیڼ -= copy;
                put += copy;
                state->اوږدتیا -= copy;
                اكسر;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->ډېرپېښ = TYPE;
            اكسر;
        حالة جدول:
            NEEDBITS(14);
            state->nlen = بټونه_2(5) + 257;
            DROPBITS(5);
            state->ndist = بټونه_2(5) + 1;
            DROPBITS(5);
            state->ncode = بټونه_2(4) + 4;
            DROPBITS(4);
#که_نه_تعریف_شوی PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->پیغام = (z_const حرف *)
                    "too many length or distance symbols";
                state->ډېرپېښ = BAD;
                اكسر;
            }
#د_که_پای
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->ډېرپېښ = LENLENS;
                /* fallthrough */
        حالة LENLENS:
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (بینښه لنډ)بټونه_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->بل = state->codes;
            state->lencode = state->distcode = (ثابت کوډ FAR *)(state->بل);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->بل),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->ډېرپېښ = CODELENS;
                /* fallthrough */
        حالة CODELENS:
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    دلته = state->lencode[بټونه_2(state->lenbits)];
                    إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                إذا (دلته.val < 16) {
                    DROPBITS(دلته.بټونه);
                    state->lens[state->have++] = دلته.val;
                }
                وإلا {
                    إذا (دلته.val == 16) {
                        NEEDBITS(دلته.بټونه + 2);
                        DROPBITS(دلته.بټونه);
                        إذا (state->have == 0) {
                            strm->پیغام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->ډېرپېښ = BAD;
                            اكسر;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + بټونه_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (دلته.val == 17) {
                        NEEDBITS(دلته.بټونه + 3);
                        DROPBITS(دلته.بټونه);
                        len = 0;
                        copy = 3 + بټونه_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(دلته.بټونه + 7);
                        DROPBITS(دلته.بټونه);
                        len = 0;
                        copy = 11 + بټونه_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->پیغام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->ډېرپېښ = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بینښه لنډ)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->ډېرپېښ == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->ډېرپېښ = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->بل = state->codes;
            state->lencode = (ثابت کوډ FAR *)(state->بل);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->بل),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->distcode = (ثابت کوډ FAR *)(state->بل);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->بل), &(state->distbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->ډېرپېښ = LEN_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة LEN_:
            state->ډېرپېښ = LEN;
                /* fallthrough */
        حالة LEN:
            إذا (have >= 6 && کیڼ >= 258) {
                RESTORE();
                inflate_fast(strm, بهر);
                بارول_2();
                إذا (state->ډېرپېښ == TYPE)
                    state->شا = -1;
                اكسر;
            }
            state->شا = 0;
            لكل (;;) {
                دلته = state->lencode[بټونه_2(state->lenbits)];
                إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                PULLBYTE();
            }
            إذا (دلته.op && (دلته.op & 0xf0) == 0) {
                وروستی = دلته;
                لكل (;;) {
                    دلته = state->lencode[وروستی.val +
                            (بټونه_2(وروستی.بټونه + وروستی.op) >> وروستی.بټونه)];
                    إذا ((بینښه)(وروستی.بټونه + دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                DROPBITS(وروستی.بټونه);
                state->شا += وروستی.بټونه;
            }
            DROPBITS(دلته.بټونه);
            state->شا += دلته.بټونه;
            state->اوږدتیا = (بینښه)دلته.val;
            إذا ((صحيح)(دلته.op) == 0) {
                Tracevv((stderr, دلته.val >= 0x20 && دلته.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", دلته.val));
                state->ډېرپېښ = LIT;
                اكسر;
            }
            إذا (دلته.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->شا = -1;
                state->ډېرپېښ = TYPE;
                اكسر;
            }
            إذا (دلته.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->extra = (بینښه)(دلته.op) & 15;
            state->ډېرپېښ = LENEXT;
                /* fallthrough */
        حالة LENEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->اوږدتیا += بټونه_2(state->extra);
                DROPBITS(state->extra);
                state->شا += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->اوږدتیا));
            state->was = state->اوږدتیا;
            state->ډېرپېښ = DIST;
                /* fallthrough */
        حالة DIST:
            لكل (;;) {
                دلته = state->distcode[بټونه_2(state->distbits)];
                إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                PULLBYTE();
            }
            إذا ((دلته.op & 0xf0) == 0) {
                وروستی = دلته;
                لكل (;;) {
                    دلته = state->distcode[وروستی.val +
                            (بټونه_2(وروستی.بټونه + وروستی.op) >> وروستی.بټونه)];
                    إذا ((بینښه)(وروستی.بټونه + دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                DROPBITS(وروستی.بټونه);
                state->شا += وروستی.بټونه;
            }
            DROPBITS(دلته.بټونه);
            state->شا += دلته.بټونه;
            إذا (دلته.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->دننه_واټن = (بینښه)دلته.val;
            state->extra = (بینښه)(دلته.op) & 15;
            state->ډېرپېښ = DISTEXT;
                /* fallthrough */
        حالة DISTEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->دننه_واټن += بټونه_2(state->extra);
                DROPBITS(state->extra);
                state->شا += state->extra;
            }
#که_تعریف_شوی INFLATE_STRICT
            إذا (state->دننه_واټن > state->dmax) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                state->ډېرپېښ = BAD;
                اكسر;
            }
#د_که_پای
            Tracevv((stderr, "inflate:         distance %u\n", state->دننه_واټن));
            state->ډېرپېښ = سمون_2;
                /* fallthrough */
        حالة سمون_2:
            إذا (کیڼ == 0) اذهب inf_leave;
            copy = بهر - کیڼ;
            إذا (state->دننه_واټن > copy) {         /* copy from window */
                copy = state->دننه_واټن - copy;
                إذا (copy > state->whave) {
                    إذا (state->sane) {
                        strm->پیغام = (z_const حرف *)
                            "invalid distance too far back";
                        state->ډېرپېښ = BAD;
                        اكسر;
                    }
#که_تعریف_شوی INFLATE_ALLOW_INVALID_واټن_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    إذا (copy > state->اوږدتیا) copy = state->اوږدتیا;
                    إذا (copy > کیڼ) copy = کیڼ;
                    کیڼ -= copy;
                    state->اوږدتیا -= copy;
                    افعل {
                        *put++ = 0;
                    } طالما (--copy);
                    إذا (state->اوږدتیا == 0) state->ډېرپېښ = LEN;
                    اكسر;
#د_که_پای
                }
                إذا (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->کړکۍ + (state->wsize - copy);
                }
                وإلا
                    from = state->کړکۍ + (state->wnext - copy);
                إذا (copy > state->اوږدتیا) copy = state->اوږدتیا;
            }
            وإلا {                              /* copy from output */
                from = put - state->دننه_واټن;
                copy = state->اوږدتیا;
            }
            إذا (copy > کیڼ) copy = کیڼ;
            کیڼ -= copy;
            state->اوږدتیا -= copy;
            افعل {
                *put++ = *from++;
            } طالما (--copy);
            إذا (state->اوږدتیا == 0) state->ډېرپېښ = LEN;
            اكسر;
        حالة LIT:
            إذا (کیڼ == 0) اذهب inf_leave;
            *put++ = (بینښه حرف)(state->اوږدتیا);
            کیڼ--;
            state->ډېرپېښ = LEN;
            اكسر;
        حالة CHECK:
            إذا (state->wrap) {
                NEEDBITS(32);
                بهر -= کیڼ;
                strm->ټول_بهر += بهر;
                state->ټول += بهر;
                إذا ((state->wrap & 4) && بهر)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - بهر, بهر);
                بهر = کیڼ;
                إذا ((state->wrap & 4) && (
#که_تعریف_شوی GUNZIP
                     state->flags ? hold :
#د_که_پای
                     ZSWAP32(hold)) != state->check) {
                    strm->پیغام = (z_const حرف *)"incorrect data check";
                    state->ډېرپېښ = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#که_تعریف_شوی GUNZIP
            state->ډېرپېښ = اوږدتیا_2;
                /* fallthrough */
        حالة اوږدتیا_2:
            إذا (state->wrap && state->flags) {
                NEEDBITS(32);
                إذا ((state->wrap & 4) && hold != (state->ټول & 0xffffffff)) {
                    strm->پیغام = (z_const حرف *)"incorrect length check";
                    state->ډېرپېښ = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#د_که_پای
            state->ډېرپېښ = بشپړ;
                /* fallthrough */
        حالة بشپړ:
            ret = Z_بهیر_پای_ټکی;
            اذهب inf_leave;
        حالة BAD:
            ret = Z_معلومات_ERROR;
            اذهب inf_leave;
        حالة MEM:
            أرجع Z_MEM_ERROR;
        حالة SYNC:
                /* fallthrough */
        افتراضي:
            أرجع Z_بهیر_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    إذا (state->wsize || (بهر != strm->avail_بهر && state->ډېرپېښ < BAD &&
            (state->ډېرپېښ < CHECK || flush != Z_پای)))
        إذا (updatewindow(strm, strm->بل_بهر, بهر - strm->avail_بهر)) {
            state->ډېرپېښ = MEM;
            أرجع Z_MEM_ERROR;
        }
    دننه -= strm->avail_دننه;
    بهر -= strm->avail_بهر;
    strm->ټول_دننه += دننه;
    strm->ټول_بهر += بهر;
    state->ټول += بهر;
    إذا ((state->wrap & 4) && بهر)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->بل_بهر - بهر, بهر);
    strm->معلومات_type = (صحيح)state->بټونه + (state->وروستی ? 64 : 0) +
                      (state->ډېرپېښ == TYPE ? 128 : 0) +
                      (state->ډېرپېښ == LEN_ || state->ډېرپېښ == COPY_ ? 256 : 0);
    إذا (((دننه == 0 && بهر == 0) || flush == Z_پای) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    أرجع ret;
}

صحيح ZEXPORT inflate_پای_ټکی(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm))
        أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->کړکۍ != Z_NULL) ZFREE(strm, state->کړکۍ);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_اوږدتیا) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* copy dictionary */
    إذا (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->کړکۍ + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->کړکۍ, state->wnext);
    }
    إذا (dict_اوږدتیا != Z_NULL)
        *dict_اوږدتیا = state->whave;
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_سېټ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt dict_اوږدتیا) {
    بنية inflate_state FAR *state;
    بینښه اوږد dictid;
    صحيح ret;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->wrap != 0 && state->ډېرپېښ != DICT)
        أرجع Z_بهیر_ERROR;

    /* check for correct dictionary identifier */
    إذا (state->ډېرپېښ == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_اوږدتیا);
        إذا (dictid != state->check)
            أرجع Z_معلومات_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_اوږدتیا, dict_اوږدتیا);
    إذا (ret) {
        state->ډېرپېښ = MEM;
        أرجع Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_Get_سرلیک(z_streamp strm, gz_headerp سر) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا ((state->wrap & 2) == 0) أرجع Z_بهیر_ERROR;

    /* save header structure */
    state->سر = سر;
    سر->بشپړ_2 = 0;
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
local بینښه syncsearch(بینښه FAR *have, ثابت بینښه حرف FAR *buf,
                          بینښه len) {
    بینښه got;
    بینښه بل;

    got = *have;
    بل = 0;
    طالما (بل < len && got < 4) {
        إذا ((صحيح)(buf[بل]) == (got < 2 ? 0 : 0xff))
            got++;
        وإلا إذا (buf[بل])
            got = 0;
        وإلا
            got = 4 - got;
        بل++;
    }
    *have = got;
    أرجع بل;
}

صحيح ZEXPORT inflateSync(z_streamp strm) {
    بینښه len;               /* number of bytes to look at or looked at */
    صحيح flags;                  /* temporary to save header status */
    بینښه اوږد دننه, بهر;      /* temporary to save total_in and total_out */
    بینښه حرف buf[4];       /* to restore bit buffer to byte string */
    بنية inflate_state FAR *state;

    /* check parameters */
    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (strm->avail_دننه == 0 && state->بټونه < 8) أرجع Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    إذا (state->ډېرپېښ != SYNC) {
        state->ډېرپېښ = SYNC;
        state->hold >>= state->بټونه & 7;
        state->بټونه -= state->بټونه & 7;
        len = 0;
        طالما (state->بټونه >= 8) {
            buf[len++] = (بینښه حرف)(state->hold);
            state->hold >>= 8;
            state->بټونه -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->بل_دننه, strm->avail_دننه);
    strm->avail_دننه -= len;
    strm->بل_دننه += len;
    strm->ټول_دننه += len;

    /* return no joy or set up to restart inflate() on a new block */
    إذا (state->have != 4) أرجع Z_معلومات_ERROR;
    إذا (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    وإلا
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    دننه = strm->ټول_دننه;  بهر = strm->ټول_بهر;
    inflateReset(strm);
    strm->ټول_دننه = دننه;  strm->ټول_بهر = بهر;
    state->flags = flags;
    state->ډېرپېښ = TYPE;
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
صحيح ZEXPORT inflate_Sync_ټکی(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع state->ډېرپېښ == STORED && state->بټونه == 0;
}

صحيح ZEXPORT inflateCopy(z_streamp dest, z_streamp سرچینه) {
    بنية inflate_state FAR *state;
    بنية inflate_state FAR *copy;
    بینښه حرف FAR *کړکۍ;

    /* check input */
    إذا (inflateStateCheck(سرچینه) || dest == Z_NULL)
        أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)سرچینه->state;

    /* allocate space */
    copy = (بنية inflate_state FAR *)
           ZALLOC(سرچینه, 1, حجم(بنية inflate_state));
    إذا (copy == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(copy, حجم(بنية inflate_state));
    کړکۍ = Z_NULL;
    إذا (state->کړکۍ != Z_NULL) {
        کړکۍ = (بینښه حرف FAR *)
                 ZALLOC(سرچینه, 1U << state->wbits, حجم(بینښه حرف));
        إذا (کړکۍ == Z_NULL) {
            ZFREE(سرچینه, copy);
            أرجع Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, سرچینه, حجم(z_بهیر));
    zmemcpy(copy, state, حجم(بنية inflate_state));
    copy->strm = dest;
    إذا (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->بل = copy->codes + (state->بل - state->codes);
    إذا (کړکۍ != Z_NULL)
        zmemcpy(کړکۍ, state->کړکۍ, state->whave);
    copy->کړکۍ = کړکۍ;
    dest->state = (بنية دننی_state FAR *)copy;
    أرجع Z_OK;
}

صحيح ZEXPORT inflateUndermine(z_streamp strm, صحيح subvert) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
#که_تعریف_شوی INFLATE_ALLOW_INVALID_واټن_TOOFAR_ARRR
    state->sane = !subvert;
    أرجع Z_OK;
#وإلا
    (فراغ)subvert;
    state->sane = 1;
    أرجع Z_معلومات_ERROR;
#د_که_پای
}

صحيح ZEXPORT inflateValidate(z_streamp strm, صحيح check) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (check && state->wrap)
        state->wrap |= 4;
    وإلا
        state->wrap &= ~4;
    أرجع Z_OK;
}

اوږد ZEXPORT inflateMark(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm))
        أرجع -(1L << 16);
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (اوږد)(((بینښه اوږد)((اوږد)state->شا)) << 16) +
        (state->ډېرپېښ == COPY ? state->اوږدتیا :
            (state->ډېرپېښ == سمون_2 ? state->was - state->اوږدتیا : 0));
}

بینښه اوږد ZEXPORT inflate_Codes_کارول_شوی(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm)) أرجع (بینښه اوږد)-1;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (بینښه اوږد)(state->بل - state->codes);
}
