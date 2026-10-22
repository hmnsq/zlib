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

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inftrees.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inflate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inffast.h"

local ਪੂਰਨ inflateStateCheck(z_streamp strm) {
    ਬਣਤਰ inflate_state FAR *state;
    ਜੇ (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        ਵਾਪਸ 1;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ (state == Z_NULL || state->strm != strm ||
        state->ਬਹੁਲਕ < ਸਿਰ_2 || state->ਬਹੁਲਕ > SYNC)
        ਵਾਪਸ 1;
    ਵਾਪਸ 0;
}

ਪੂਰਨ ZEXPORT inflateResetKeep(z_streamp strm) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    strm->ਕੁੱਲ_ਵਿੱਚ = strm->ਕੁੱਲ_ਬਾਹਰਲਾ = state->ਕੁੱਲ = 0;
    strm->ਸੁਨੇਹਾ = Z_NULL;
    strm->ਅੰਕੜੇ_type = 0;
    ਜੇ (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->ਬਹੁਲਕ = ਸਿਰ_2;
    state->ਆਖਰੀ = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->ਸਿਰ = Z_NULL;
    state->hold = 0;
    state->ਬਿੱਟ = 0;
    state->lencode = state->distcode = state->ਅਗਲਾ = state->codes;
    state->sane = 1;
    state->ਪਿੱਠ = -1;
    Tracev((stderr, "inflate: reset\n"));
    ਵਾਪਸ Z_OK;
}

ਪੂਰਨ ZEXPORT inflateReset(z_streamp strm) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    ਵਾਪਸ inflateResetKeep(strm);
}

ਪੂਰਨ ZEXPORT inflateReset2(z_streamp strm, ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ) {
    ਪੂਰਨ wrap;
    ਬਣਤਰ inflate_state FAR *state;

    /* get the state */
    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ < 0) {
        ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ < -15)
            ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
        wrap = 0;
        ਖਿੜਕੀ_ਬਿੱਟ = -ਖਿੜਕੀ_ਬਿੱਟ;
    }
    ਨਹੀਂਤਾਂ {
        wrap = (ਖਿੜਕੀ_ਬਿੱਟ >> 4) + 5;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
        ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ < 48)
            ਖਿੜਕੀ_ਬਿੱਟ &= 15;
#ਜੇ_ਅੰਤ
    }

    /* set number of window bits, free window if different */
    ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ && (ਖਿੜਕੀ_ਬਿੱਟ < 8 || ਖਿੜਕੀ_ਬਿੱਟ > 15))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ਜੇ (state->ਖਿੜਕੀ != Z_NULL && state->wbits != (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖਿੜਕੀ_ਬਿੱਟ) {
        ZFREE(strm, state->ਖਿੜਕੀ);
        state->ਖਿੜਕੀ = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖਿੜਕੀ_ਬਿੱਟ;
    ਵਾਪਸ inflateReset(strm);
}

ਪੂਰਨ ZEXPORT inflateInit2_(z_streamp strm, ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,
                          ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ, ਪੂਰਨ ਧਾਰਾ_ਮਾਪ) {
    ਪੂਰਨ ret;
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (ਸੰਸਕਰਣ == Z_NULL || ਸੰਸਕਰਣ[0] != ZLIB_ਸੰਸਕਰਣ[0] ||
        ਧਾਰਾ_ਮਾਪ != (ਪੂਰਨ)(ਆਕਾਰ(z_ਧਾਰਾ)))
        ਵਾਪਸ Z_ਸੰਸਕਰਣ_ERROR;
    ਜੇ (strm == Z_NULL) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    strm->ਸੁਨੇਹਾ = Z_NULL;                 /* in case we return an error */
    ਜੇ (strm->zalloc == (alloc_func)0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
#ਨਹੀਂਤਾਂ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#ਜੇ_ਅੰਤ
    }
    ਜੇ (strm->zfree == (free_func)0)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
#ਨਹੀਂਤਾਂ
        strm->zfree = zcfree;
#ਜੇ_ਅੰਤ
    state = (ਬਣਤਰ inflate_state FAR *)
            ZALLOC(strm, 1, ਆਕਾਰ(ਬਣਤਰ inflate_state));
    ਜੇ (state == Z_NULL) ਵਾਪਸ Z_MEM_ERROR;
    zmemzero(state, ਆਕਾਰ(ਬਣਤਰ inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ਬਣਤਰ ਅੰਦਰੂਨੀ_state FAR *)state;
    state->strm = strm;
    state->ਖਿੜਕੀ = Z_NULL;
    state->ਬਹੁਲਕ = ਸਿਰ_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, ਖਿੜਕੀ_ਬਿੱਟ);
    ਜੇ (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    ਵਾਪਸ ret;
}

ਪੂਰਨ ZEXPORT inflateInit_(z_streamp strm, ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ,
                         ਪੂਰਨ ਧਾਰਾ_ਮਾਪ) {
    ਵਾਪਸ inflateInit2_(strm, DEF_WBITS, ਸੰਸਕਰਣ, ਧਾਰਾ_ਮਾਪ);
}

ਪੂਰਨ ZEXPORT inflate_ਅਭਾਜ(z_streamp strm, ਪੂਰਨ ਬਿੱਟ, ਪੂਰਨ ਮੁੱਲ) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ਜੇ (ਬਿੱਟ == 0)
        ਵਾਪਸ Z_OK;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ (ਬਿੱਟ < 0) {
        state->hold = 0;
        state->ਬਿੱਟ = 0;
        ਵਾਪਸ Z_OK;
    }
    ਜੇ (ਬਿੱਟ > 16 || state->ਬਿੱਟ + (uInt)ਬਿੱਟ > 32) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ਮੁੱਲ &= (1L << ਬਿੱਟ) - 1;
    state->hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਮੁੱਲ << state->ਬਿੱਟ;
    state->ਬਿੱਟ += (uInt)ਬਿੱਟ;
    ਵਾਪਸ Z_OK;
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
local ਪੂਰਨ updatewindow(z_streamp strm, ਅਚਲ Bytef *ਸਿਰਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ copy) {
    ਬਣਤਰ inflate_state FAR *state;
    ਬਿਨਾਨਿਸ਼ਾਨੀ dist;

    state = (ਬਣਤਰ inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    ਜੇ (state->ਖਿੜਕੀ == Z_NULL) {
        state->ਖਿੜਕੀ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ));
        ਜੇ (state->ਖਿੜਕੀ == Z_NULL) ਵਾਪਸ 1;
    }

    /* if window not in use yet, initialize */
    ਜੇ (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    ਜੇ (copy >= state->wsize) {
        zmemcpy(state->ਖਿੜਕੀ, ਸਿਰਾ - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    ਨਹੀਂਤਾਂ {
        dist = state->wsize - state->wnext;
        ਜੇ (dist > copy) dist = copy;
        zmemcpy(state->ਖਿੜਕੀ + state->wnext, ਸਿਰਾ - copy, dist);
        copy -= dist;
        ਜੇ (copy) {
            zmemcpy(state->ਖਿੜਕੀ, ਸਿਰਾ - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        ਨਹੀਂਤਾਂ {
            state->wnext += dist;
            ਜੇ (state->wnext == state->wsize) state->wnext = 0;
            ਜੇ (state->whave < state->wsize) state->whave += dist;
        }
    }
    ਵਾਪਸ 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? ਚੱਕਰੀ_ਜਾਂਚ_32_2(check, buf, len) : adler32(check, buf, len))
#ਨਹੀਂਤਾਂ
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#ਜੇ_ਅੰਤ

/* check macros for header crc */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
#  define CRC2(check, ਸ਼ਬਦ) \
    ਕਰੋ { \
        hbuf[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(ਸ਼ਬਦ); \
        hbuf[1] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((ਸ਼ਬਦ) >> 8); \
        check = ਚੱਕਰੀ_ਜਾਂਚ_32_2(check, hbuf, 2); \
    } ਜਦੋਂ (0)

#  define CRC4(check, ਸ਼ਬਦ) \
    ਕਰੋ { \
        hbuf[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(ਸ਼ਬਦ); \
        hbuf[1] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((ਸ਼ਬਦ) >> 8); \
        hbuf[2] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((ਸ਼ਬਦ) >> 16); \
        hbuf[3] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((ਸ਼ਬਦ) >> 24); \
        check = ਚੱਕਰੀ_ਜਾਂਚ_32_2(check, hbuf, 4); \
    } ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ

/* Load registers with state in inflate() for speed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਦੋ_2() \
    ਕਰੋ { \
        put = strm->ਅਗਲਾ_ਬਾਹਰਲਾ; \
        ਖੱਬਾ = strm->avail_ਬਾਹਰਲਾ; \
        ਅਗਲਾ = strm->ਅਗਲਾ_ਵਿੱਚ; \
        have = strm->avail_ਵਿੱਚ; \
        hold = state->hold; \
        ਬਿੱਟ = state->ਬਿੱਟ; \
    } ਜਦੋਂ (0)

/* Restore state from registers in inflate() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RESTORE() \
    ਕਰੋ { \
        strm->ਅਗਲਾ_ਬਾਹਰਲਾ = put; \
        strm->avail_ਬਾਹਰਲਾ = ਖੱਬਾ; \
        strm->ਅਗਲਾ_ਵਿੱਚ = ਅਗਲਾ; \
        strm->avail_ਵਿੱਚ = have; \
        state->hold = hold; \
        state->ਬਿੱਟ = ਬਿੱਟ; \
    } ਜਦੋਂ (0)

/* Clear the input bit accumulator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITBITS() \
    ਕਰੋ { \
        hold = 0; \
        ਬਿੱਟ = 0; \
    } ਜਦੋਂ (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PULLBYTE() \
    ਕਰੋ { \
        ਜੇ (have == 0) ਜਾਓ inf_leave; \
        have--; \
        hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਅਗਲਾ++) << ਬਿੱਟ; \
        ਬਿੱਟ += 8; \
    } ਜਦੋਂ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NEEDBITS(n) \
    ਕਰੋ { \
        ਜਦੋਂ (ਬਿੱਟ < (ਬਿਨਾਨਿਸ਼ਾਨੀ)(n)) \
            PULLBYTE(); \
    } ਜਦੋਂ (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਿੱਟ_2(n) \
    ((ਬਿਨਾਨਿਸ਼ਾਨੀ)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DROPBITS(n) \
    ਕਰੋ { \
        hold >>= (n); \
        ਬਿੱਟ -= (ਬਿਨਾਨਿਸ਼ਾਨੀ)(n); \
    } ਜਦੋਂ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BYTEBITS() \
    ਕਰੋ { \
        hold >>= ਬਿੱਟ & 7; \
        ਬਿੱਟ -= ਬਿੱਟ & 7; \
    } ਜਦੋਂ (0)

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

ਪੂਰਨ ZEXPORT inflate(z_streamp strm, ਪੂਰਨ flush) {
    ਬਣਤਰ inflate_state FAR *state;
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਅਗਲਾ;    /* next input */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *put;     /* next output */
    ਬਿਨਾਨਿਸ਼ਾਨੀ have, ਖੱਬਾ;        /* available input and output */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ hold;         /* bit buffer */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ;              /* bits in bit buffer */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਚ, ਬਾਹਰਲਾ;           /* save starting available input and output */
    ਬਿਨਾਨਿਸ਼ਾਨੀ copy;              /* number of stored or match bytes to copy */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *from;    /* where to copy match bytes from */
    ਕੋਡ ਇੱਥੇ;                  /* current decoding table entry */
    ਕੋਡ ਆਖਰੀ;                  /* parent table entry */
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;               /* length to copy for repeats, bits to drop */
    ਪੂਰਨ ret;                    /* return code */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ hbuf[4];      /* buffer for gzip header crc calculation */
#ਜੇ_ਅੰਤ
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਕ੍ਰਮ[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    ਜੇ (inflateStateCheck(strm) || strm->ਅਗਲਾ_ਬਾਹਰਲਾ == Z_NULL ||
        (strm->ਅਗਲਾ_ਵਿੱਚ == Z_NULL && strm->avail_ਵਿੱਚ != 0))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ (state->ਬਹੁਲਕ == TYPE) state->ਬਹੁਲਕ = TYPEDO;      /* skip check */
    ਲੱਦੋ_2();
    ਵਿੱਚ = have;
    ਬਾਹਰਲਾ = ਖੱਬਾ;
    ret = Z_OK;
    ਲਈ (;;)
        ਚੋਣ (state->ਬਹੁਲਕ) {
        ਹਾਲਤ ਸਿਰ_2:
            ਜੇ (state->wrap == 0) {
                state->ਬਹੁਲਕ = TYPEDO;
                ਰੋਕੋ;
            }
            NEEDBITS(16);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
            ਜੇ ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                ਜੇ (state->wbits == 0)
                    state->wbits = 15;
                state->check = ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->ਬਹੁਲਕ = FLAGS;
                ਰੋਕੋ;
            }
            ਜੇ (state->ਸਿਰ != Z_NULL)
                state->ਸਿਰ->ਪੂਰਾ_2 = -1;
            ਜੇ (!(state->wrap & 1) ||   /* check if zlib header allowed */
#ਨਹੀਂਤਾਂ
            ਜੇ (
#ਜੇ_ਅੰਤ
                ((ਬਿੱਟ_2(8) << 8) + (hold >> 8)) % 31) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"incorrect header check";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            ਜੇ (ਬਿੱਟ_2(4) != Z_DEFLATED) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"unknown compression method";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            DROPBITS(4);
            len = ਬਿੱਟ_2(4) + 8;
            ਜੇ (state->wbits == 0)
                state->wbits = len;
            ਜੇ (len > 15 || len > state->wbits) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid window size";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->ਬਹੁਲਕ = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
        ਹਾਲਤ FLAGS:
            NEEDBITS(16);
            state->flags = (ਪੂਰਨ)(hold);
            ਜੇ ((state->flags & 0xff) != Z_DEFLATED) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"unknown compression method";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            ਜੇ (state->flags & 0xe000) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"unknown header flags set";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            ਜੇ (state->ਸਿਰ != Z_NULL)
                state->ਸਿਰ->ਪਾਠ = (ਪੂਰਨ)((hold >> 8) & 1);
            ਜੇ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->ਬਹੁਲਕ = ਸਮਾਂ;
                /* fallthrough */
        ਹਾਲਤ ਸਮਾਂ:
            NEEDBITS(32);
            ਜੇ (state->ਸਿਰ != Z_NULL)
                state->ਸਿਰ->time = hold;
            ਜੇ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->ਬਹੁਲਕ = OS;
                /* fallthrough */
        ਹਾਲਤ OS:
            NEEDBITS(16);
            ਜੇ (state->ਸਿਰ != Z_NULL) {
                state->ਸਿਰ->xflags = (ਪੂਰਨ)(hold & 0xff);
                state->ਸਿਰ->os = (ਪੂਰਨ)(hold >> 8);
            }
            ਜੇ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->ਬਹੁਲਕ = EXLEN;
                /* fallthrough */
        ਹਾਲਤ EXLEN:
            ਜੇ (state->flags & 0x0400) {
                NEEDBITS(16);
                state->ਲੰਮਾਈ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(hold);
                ਜੇ (state->ਸਿਰ != Z_NULL)
                    state->ਸਿਰ->extra_len = (ਬਿਨਾਨਿਸ਼ਾਨੀ)hold;
                ਜੇ ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            ਨਹੀਂਤਾਂ ਜੇ (state->ਸਿਰ != Z_NULL)
                state->ਸਿਰ->extra = Z_NULL;
            state->ਬਹੁਲਕ = EXTRA;
                /* fallthrough */
        ਹਾਲਤ EXTRA:
            ਜੇ (state->flags & 0x0400) {
                copy = state->ਲੰਮਾਈ;
                ਜੇ (copy > have) copy = have;
                ਜੇ (copy) {
                    ਜੇ (state->ਸਿਰ != Z_NULL &&
                        state->ਸਿਰ->extra != Z_NULL &&
                        (len = state->ਸਿਰ->extra_len - state->ਲੰਮਾਈ) <
                            state->ਸਿਰ->extra_max) {
                        zmemcpy(state->ਸਿਰ->extra + len, ਅਗਲਾ,
                                len + copy > state->ਸਿਰ->extra_max ?
                                state->ਸਿਰ->extra_max - len : copy);
                    }
                    ਜੇ ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = ਚੱਕਰੀ_ਜਾਂਚ_32_2(state->check, ਅਗਲਾ, copy);
                    have -= copy;
                    ਅਗਲਾ += copy;
                    state->ਲੰਮਾਈ -= copy;
                }
                ਜੇ (state->ਲੰਮਾਈ) ਜਾਓ inf_leave;
            }
            state->ਲੰਮਾਈ = 0;
            state->ਬਹੁਲਕ = NAME;
                /* fallthrough */
        ਹਾਲਤ NAME:
            ਜੇ (state->flags & 0x0800) {
                ਜੇ (have == 0) ਜਾਓ inf_leave;
                copy = 0;
                ਕਰੋ {
                    len = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਅਗਲਾ[copy++]);
                    ਜੇ (state->ਸਿਰ != Z_NULL &&
                            state->ਸਿਰ->name != Z_NULL &&
                            state->ਲੰਮਾਈ < state->ਸਿਰ->name_max)
                        state->ਸਿਰ->name[state->ਲੰਮਾਈ++] = (Bytef)len;
                } ਜਦੋਂ (len && copy < have);
                ਜੇ ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = ਚੱਕਰੀ_ਜਾਂਚ_32_2(state->check, ਅਗਲਾ, copy);
                have -= copy;
                ਅਗਲਾ += copy;
                ਜੇ (len) ਜਾਓ inf_leave;
            }
            ਨਹੀਂਤਾਂ ਜੇ (state->ਸਿਰ != Z_NULL)
                state->ਸਿਰ->name = Z_NULL;
            state->ਲੰਮਾਈ = 0;
            state->ਬਹੁਲਕ = COMMENT;
                /* fallthrough */
        ਹਾਲਤ COMMENT:
            ਜੇ (state->flags & 0x1000) {
                ਜੇ (have == 0) ਜਾਓ inf_leave;
                copy = 0;
                ਕਰੋ {
                    len = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਅਗਲਾ[copy++]);
                    ਜੇ (state->ਸਿਰ != Z_NULL &&
                            state->ਸਿਰ->comment != Z_NULL &&
                            state->ਲੰਮਾਈ < state->ਸਿਰ->comm_max)
                        state->ਸਿਰ->comment[state->ਲੰਮਾਈ++] = (Bytef)len;
                } ਜਦੋਂ (len && copy < have);
                ਜੇ ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = ਚੱਕਰੀ_ਜਾਂਚ_32_2(state->check, ਅਗਲਾ, copy);
                have -= copy;
                ਅਗਲਾ += copy;
                ਜੇ (len) ਜਾਓ inf_leave;
            }
            ਨਹੀਂਤਾਂ ਜੇ (state->ਸਿਰ != Z_NULL)
                state->ਸਿਰ->comment = Z_NULL;
            state->ਬਹੁਲਕ = HCRC;
                /* fallthrough */
        ਹਾਲਤ HCRC:
            ਜੇ (state->flags & 0x0200) {
                NEEDBITS(16);
                ਜੇ ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"header crc mismatch";
                    state->ਬਹੁਲਕ = BAD;
                    ਰੋਕੋ;
                }
                INITBITS();
            }
            ਜੇ (state->ਸਿਰ != Z_NULL) {
                state->ਸਿਰ->hcrc = (ਪੂਰਨ)((state->flags >> 9) & 1);
                state->ਸਿਰ->ਪੂਰਾ_2 = 1;
            }
            strm->adler = state->check = ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0);
            state->ਬਹੁਲਕ = TYPE;
            ਰੋਕੋ;
#ਜੇ_ਅੰਤ
        ਹਾਲਤ DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->ਬਹੁਲਕ = DICT;
                /* fallthrough */
        ਹਾਲਤ DICT:
            ਜੇ (state->havedict == 0) {
                RESTORE();
                ਵਾਪਸ Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->ਬਹੁਲਕ = TYPE;
                /* fallthrough */
        ਹਾਲਤ TYPE:
            ਜੇ (flush == Z_ਖੰਡ || flush == Z_TREES) ਜਾਓ inf_leave;
                /* fallthrough */
        ਹਾਲਤ TYPEDO:
            ਜੇ (state->ਆਖਰੀ) {
                BYTEBITS();
                state->ਬਹੁਲਕ = CHECK;
                ਰੋਕੋ;
            }
            NEEDBITS(3);
            state->ਆਖਰੀ = ਬਿੱਟ_2(1);
            DROPBITS(1);
            ਚੋਣ (ਬਿੱਟ_2(2)) {
            ਹਾਲਤ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->ਆਖਰੀ ? " (last)" : ""));
                state->ਬਹੁਲਕ = STORED;
                ਰੋਕੋ;
            ਹਾਲਤ 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->ਆਖਰੀ ? " (last)" : ""));
                state->ਬਹੁਲਕ = LEN_;             /* decode codes */
                ਜੇ (flush == Z_TREES) {
                    DROPBITS(2);
                    ਜਾਓ inf_leave;
                }
                ਰੋਕੋ;
            ਹਾਲਤ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->ਆਖਰੀ ? " (last)" : ""));
                state->ਬਹੁਲਕ = ਸਾਰਣੀ;
                ਰੋਕੋ;
            ਮੂਲ:
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid block type";
                state->ਬਹੁਲਕ = BAD;
            }
            DROPBITS(2);
            ਰੋਕੋ;
        ਹਾਲਤ STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ਜੇ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid stored block lengths";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            state->ਲੰਮਾਈ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->ਲੰਮਾਈ));
            INITBITS();
            state->ਬਹੁਲਕ = COPY_;
            ਜੇ (flush == Z_TREES) ਜਾਓ inf_leave;
                /* fallthrough */
        ਹਾਲਤ COPY_:
            state->ਬਹੁਲਕ = COPY;
                /* fallthrough */
        ਹਾਲਤ COPY:
            copy = state->ਲੰਮਾਈ;
            ਜੇ (copy) {
                ਜੇ (copy > have) copy = have;
                ਜੇ (copy > ਖੱਬਾ) copy = ਖੱਬਾ;
                ਜੇ (copy == 0) ਜਾਓ inf_leave;
                zmemcpy(put, ਅਗਲਾ, copy);
                have -= copy;
                ਅਗਲਾ += copy;
                ਖੱਬਾ -= copy;
                put += copy;
                state->ਲੰਮਾਈ -= copy;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->ਬਹੁਲਕ = TYPE;
            ਰੋਕੋ;
        ਹਾਲਤ ਸਾਰਣੀ:
            NEEDBITS(14);
            state->nlen = ਬਿੱਟ_2(5) + 257;
            DROPBITS(5);
            state->ndist = ਬਿੱਟ_2(5) + 1;
            DROPBITS(5);
            state->ncode = ਬਿੱਟ_2(4) + 4;
            DROPBITS(4);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ PKZIP_BUG_WORKAROUND
            ਜੇ (state->nlen > 286 || state->ndist > 30) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                    "too many length or distance symbols";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
#ਜੇ_ਅੰਤ
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->ਬਹੁਲਕ = LENLENS;
                /* fallthrough */
        ਹਾਲਤ LENLENS:
            ਜਦੋਂ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ਕ੍ਰਮ[state->have++]] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ)ਬਿੱਟ_2(3);
                DROPBITS(3);
            }
            ਜਦੋਂ (state->have < 19)
                state->lens[ਕ੍ਰਮ[state->have++]] = 0;
            state->ਅਗਲਾ = state->codes;
            state->lencode = state->distcode = (ਅਚਲ ਕੋਡ FAR *)(state->ਅਗਲਾ);
            state->lenbits = 7;
            ret = inflate_ਸਾਰਣੀ(CODES, state->lens, 19, &(state->ਅਗਲਾ),
                                &(state->lenbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid code lengths set";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->ਬਹੁਲਕ = CODELENS;
                /* fallthrough */
        ਹਾਲਤ CODELENS:
            ਜਦੋਂ (state->have < state->nlen + state->ndist) {
                ਲਈ (;;) {
                    ਇੱਥੇ = state->lencode[ਬਿੱਟ_2(state->lenbits)];
                    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                    PULLBYTE();
                }
                ਜੇ (ਇੱਥੇ.val < 16) {
                    DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
                    state->lens[state->have++] = ਇੱਥੇ.val;
                }
                ਨਹੀਂਤਾਂ {
                    ਜੇ (ਇੱਥੇ.val == 16) {
                        NEEDBITS(ਇੱਥੇ.ਬਿੱਟ + 2);
                        DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
                        ਜੇ (state->have == 0) {
                            strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                                "invalid bit length repeat";
                            state->ਬਹੁਲਕ = BAD;
                            ਰੋਕੋ;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + ਬਿੱਟ_2(2);
                        DROPBITS(2);
                    }
                    ਨਹੀਂਤਾਂ ਜੇ (ਇੱਥੇ.val == 17) {
                        NEEDBITS(ਇੱਥੇ.ਬਿੱਟ + 3);
                        DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
                        len = 0;
                        copy = 3 + ਬਿੱਟ_2(3);
                        DROPBITS(3);
                    }
                    ਨਹੀਂਤਾਂ {
                        NEEDBITS(ਇੱਥੇ.ਬਿੱਟ + 7);
                        DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
                        len = 0;
                        copy = 11 + ਬਿੱਟ_2(7);
                        DROPBITS(7);
                    }
                    ਜੇ (state->have + copy > state->nlen + state->ndist) {
                        strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                            "invalid bit length repeat";
                        state->ਬਹੁਲਕ = BAD;
                        ਰੋਕੋ;
                    }
                    ਜਦੋਂ (copy--)
                        state->lens[state->have++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ)len;
                }
            }

            /* handle error breaks in while */
            ਜੇ (state->ਬਹੁਲਕ == BAD) ਰੋਕੋ;

            /* check for end-of-block code (better have one) */
            ਜੇ (state->lens[256] == 0) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                    "invalid code -- missing end-of-block";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->ਅਗਲਾ = state->codes;
            state->lencode = (ਅਚਲ ਕੋਡ FAR *)(state->ਅਗਲਾ);
            state->lenbits = 9;
            ret = inflate_ਸਾਰਣੀ(LENS, state->lens, state->nlen, &(state->ਅਗਲਾ),
                                &(state->lenbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid literal/lengths set";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            state->distcode = (ਅਚਲ ਕੋਡ FAR *)(state->ਅਗਲਾ);
            state->distbits = 6;
            ret = inflate_ਸਾਰਣੀ(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->ਅਗਲਾ), &(state->distbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distances set";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->ਬਹੁਲਕ = LEN_;
            ਜੇ (flush == Z_TREES) ਜਾਓ inf_leave;
                /* fallthrough */
        ਹਾਲਤ LEN_:
            state->ਬਹੁਲਕ = LEN;
                /* fallthrough */
        ਹਾਲਤ LEN:
            ਜੇ (have >= 6 && ਖੱਬਾ >= 258) {
                RESTORE();
                inflate_fast(strm, ਬਾਹਰਲਾ);
                ਲੱਦੋ_2();
                ਜੇ (state->ਬਹੁਲਕ == TYPE)
                    state->ਪਿੱਠ = -1;
                ਰੋਕੋ;
            }
            state->ਪਿੱਠ = 0;
            ਲਈ (;;) {
                ਇੱਥੇ = state->lencode[ਬਿੱਟ_2(state->lenbits)];
                ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                PULLBYTE();
            }
            ਜੇ (ਇੱਥੇ.op && (ਇੱਥੇ.op & 0xf0) == 0) {
                ਆਖਰੀ = ਇੱਥੇ;
                ਲਈ (;;) {
                    ਇੱਥੇ = state->lencode[ਆਖਰੀ.val +
                            (ਬਿੱਟ_2(ਆਖਰੀ.ਬਿੱਟ + ਆਖਰੀ.op) >> ਆਖਰੀ.ਬਿੱਟ)];
                    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਆਖਰੀ.ਬਿੱਟ + ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                    PULLBYTE();
                }
                DROPBITS(ਆਖਰੀ.ਬਿੱਟ);
                state->ਪਿੱਠ += ਆਖਰੀ.ਬਿੱਟ;
            }
            DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
            state->ਪਿੱਠ += ਇੱਥੇ.ਬਿੱਟ;
            state->ਲੰਮਾਈ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਇੱਥੇ.val;
            ਜੇ ((ਪੂਰਨ)(ਇੱਥੇ.op) == 0) {
                Tracevv((stderr, ਇੱਥੇ.val >= 0x20 && ਇੱਥੇ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ਇੱਥੇ.val));
                state->ਬਹੁਲਕ = LIT;
                ਰੋਕੋ;
            }
            ਜੇ (ਇੱਥੇ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->ਪਿੱਠ = -1;
                state->ਬਹੁਲਕ = TYPE;
                ਰੋਕੋ;
            }
            ਜੇ (ਇੱਥੇ.op & 64) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid literal/length code";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            state->extra = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.op) & 15;
            state->ਬਹੁਲਕ = LENEXT;
                /* fallthrough */
        ਹਾਲਤ LENEXT:
            ਜੇ (state->extra) {
                NEEDBITS(state->extra);
                state->ਲੰਮਾਈ += ਬਿੱਟ_2(state->extra);
                DROPBITS(state->extra);
                state->ਪਿੱਠ += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->ਲੰਮਾਈ));
            state->was = state->ਲੰਮਾਈ;
            state->ਬਹੁਲਕ = DIST;
                /* fallthrough */
        ਹਾਲਤ DIST:
            ਲਈ (;;) {
                ਇੱਥੇ = state->distcode[ਬਿੱਟ_2(state->distbits)];
                ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                PULLBYTE();
            }
            ਜੇ ((ਇੱਥੇ.op & 0xf0) == 0) {
                ਆਖਰੀ = ਇੱਥੇ;
                ਲਈ (;;) {
                    ਇੱਥੇ = state->distcode[ਆਖਰੀ.val +
                            (ਬਿੱਟ_2(ਆਖਰੀ.ਬਿੱਟ + ਆਖਰੀ.op) >> ਆਖਰੀ.ਬਿੱਟ)];
                    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਆਖਰੀ.ਬਿੱਟ + ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                    PULLBYTE();
                }
                DROPBITS(ਆਖਰੀ.ਬਿੱਟ);
                state->ਪਿੱਠ += ਆਖਰੀ.ਬਿੱਟ;
            }
            DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
            state->ਪਿੱਠ += ਇੱਥੇ.ਬਿੱਟ;
            ਜੇ (ਇੱਥੇ.op & 64) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distance code";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            state->ਵਿੱਥ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਇੱਥੇ.val;
            state->extra = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.op) & 15;
            state->ਬਹੁਲਕ = DISTEXT;
                /* fallthrough */
        ਹਾਲਤ DISTEXT:
            ਜੇ (state->extra) {
                NEEDBITS(state->extra);
                state->ਵਿੱਥ += ਬਿੱਟ_2(state->extra);
                DROPBITS(state->extra);
                state->ਪਿੱਠ += state->extra;
            }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INFLATE_STRICT
            ਜੇ (state->ਵਿੱਥ > state->dmax) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distance too far back";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
#ਜੇ_ਅੰਤ
            Tracevv((stderr, "inflate:         distance %u\n", state->ਵਿੱਥ));
            state->ਬਹੁਲਕ = ਮੇਲ_2;
                /* fallthrough */
        ਹਾਲਤ ਮੇਲ_2:
            ਜੇ (ਖੱਬਾ == 0) ਜਾਓ inf_leave;
            copy = ਬਾਹਰਲਾ - ਖੱਬਾ;
            ਜੇ (state->ਵਿੱਥ > copy) {         /* copy from window */
                copy = state->ਵਿੱਥ - copy;
                ਜੇ (copy > state->whave) {
                    ਜੇ (state->sane) {
                        strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                            "invalid distance too far back";
                        state->ਬਹੁਲਕ = BAD;
                        ਰੋਕੋ;
                    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INFLATE_ALLOW_INVALID_ਦੂਰੀ_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    ਜੇ (copy > state->ਲੰਮਾਈ) copy = state->ਲੰਮਾਈ;
                    ਜੇ (copy > ਖੱਬਾ) copy = ਖੱਬਾ;
                    ਖੱਬਾ -= copy;
                    state->ਲੰਮਾਈ -= copy;
                    ਕਰੋ {
                        *put++ = 0;
                    } ਜਦੋਂ (--copy);
                    ਜੇ (state->ਲੰਮਾਈ == 0) state->ਬਹੁਲਕ = LEN;
                    ਰੋਕੋ;
#ਜੇ_ਅੰਤ
                }
                ਜੇ (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->ਖਿੜਕੀ + (state->wsize - copy);
                }
                ਨਹੀਂਤਾਂ
                    from = state->ਖਿੜਕੀ + (state->wnext - copy);
                ਜੇ (copy > state->ਲੰਮਾਈ) copy = state->ਲੰਮਾਈ;
            }
            ਨਹੀਂਤਾਂ {                              /* copy from output */
                from = put - state->ਵਿੱਥ;
                copy = state->ਲੰਮਾਈ;
            }
            ਜੇ (copy > ਖੱਬਾ) copy = ਖੱਬਾ;
            ਖੱਬਾ -= copy;
            state->ਲੰਮਾਈ -= copy;
            ਕਰੋ {
                *put++ = *from++;
            } ਜਦੋਂ (--copy);
            ਜੇ (state->ਲੰਮਾਈ == 0) state->ਬਹੁਲਕ = LEN;
            ਰੋਕੋ;
        ਹਾਲਤ LIT:
            ਜੇ (ਖੱਬਾ == 0) ਜਾਓ inf_leave;
            *put++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(state->ਲੰਮਾਈ);
            ਖੱਬਾ--;
            state->ਬਹੁਲਕ = LEN;
            ਰੋਕੋ;
        ਹਾਲਤ CHECK:
            ਜੇ (state->wrap) {
                NEEDBITS(32);
                ਬਾਹਰਲਾ -= ਖੱਬਾ;
                strm->ਕੁੱਲ_ਬਾਹਰਲਾ += ਬਾਹਰਲਾ;
                state->ਕੁੱਲ += ਬਾਹਰਲਾ;
                ਜੇ ((state->wrap & 4) && ਬਾਹਰਲਾ)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - ਬਾਹਰਲਾ, ਬਾਹਰਲਾ);
                ਬਾਹਰਲਾ = ਖੱਬਾ;
                ਜੇ ((state->wrap & 4) && (
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
                     state->flags ? hold :
#ਜੇ_ਅੰਤ
                     ZSWAP32(hold)) != state->check) {
                    strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"incorrect data check";
                    state->ਬਹੁਲਕ = BAD;
                    ਰੋਕੋ;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GUNZIP
            state->ਬਹੁਲਕ = ਲੰਮਾਈ_2;
                /* fallthrough */
        ਹਾਲਤ ਲੰਮਾਈ_2:
            ਜੇ (state->wrap && state->flags) {
                NEEDBITS(32);
                ਜੇ ((state->wrap & 4) && hold != (state->ਕੁੱਲ & 0xffffffff)) {
                    strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"incorrect length check";
                    state->ਬਹੁਲਕ = BAD;
                    ਰੋਕੋ;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#ਜੇ_ਅੰਤ
            state->ਬਹੁਲਕ = ਪੂਰਾ;
                /* fallthrough */
        ਹਾਲਤ ਪੂਰਾ:
            ret = Z_ਧਾਰਾ_ਸਿਰਾ;
            ਜਾਓ inf_leave;
        ਹਾਲਤ BAD:
            ret = Z_ਅੰਕੜੇ_ERROR;
            ਜਾਓ inf_leave;
        ਹਾਲਤ MEM:
            ਵਾਪਸ Z_MEM_ERROR;
        ਹਾਲਤ SYNC:
                /* fallthrough */
        ਮੂਲ:
            ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    ਜੇ (state->wsize || (ਬਾਹਰਲਾ != strm->avail_ਬਾਹਰਲਾ && state->ਬਹੁਲਕ < BAD &&
            (state->ਬਹੁਲਕ < CHECK || flush != Z_ਅੰਤ)))
        ਜੇ (updatewindow(strm, strm->ਅਗਲਾ_ਬਾਹਰਲਾ, ਬਾਹਰਲਾ - strm->avail_ਬਾਹਰਲਾ)) {
            state->ਬਹੁਲਕ = MEM;
            ਵਾਪਸ Z_MEM_ERROR;
        }
    ਵਿੱਚ -= strm->avail_ਵਿੱਚ;
    ਬਾਹਰਲਾ -= strm->avail_ਬਾਹਰਲਾ;
    strm->ਕੁੱਲ_ਵਿੱਚ += ਵਿੱਚ;
    strm->ਕੁੱਲ_ਬਾਹਰਲਾ += ਬਾਹਰਲਾ;
    state->ਕੁੱਲ += ਬਾਹਰਲਾ;
    ਜੇ ((state->wrap & 4) && ਬਾਹਰਲਾ)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->ਅਗਲਾ_ਬਾਹਰਲਾ - ਬਾਹਰਲਾ, ਬਾਹਰਲਾ);
    strm->ਅੰਕੜੇ_type = (ਪੂਰਨ)state->ਬਿੱਟ + (state->ਆਖਰੀ ? 64 : 0) +
                      (state->ਬਹੁਲਕ == TYPE ? 128 : 0) +
                      (state->ਬਹੁਲਕ == LEN_ || state->ਬਹੁਲਕ == COPY_ ? 256 : 0);
    ਜੇ (((ਵਿੱਚ == 0 && ਬਾਹਰਲਾ == 0) || flush == Z_ਅੰਤ) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    ਵਾਪਸ ret;
}

ਪੂਰਨ ZEXPORT inflate_ਸਿਰਾ(z_streamp strm) {
    ਬਣਤਰ inflate_state FAR *state;
    ਜੇ (inflateStateCheck(strm))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ (state->ਖਿੜਕੀ != Z_NULL) ZFREE(strm, state->ਖਿੜਕੀ);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ਵਾਪਸ Z_OK;
}

ਪੂਰਨ ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_ਲੰਮਾਈ) {
    ਬਣਤਰ inflate_state FAR *state;

    /* check state */
    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;

    /* copy dictionary */
    ਜੇ (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->ਖਿੜਕੀ + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->ਖਿੜਕੀ, state->wnext);
    }
    ਜੇ (dict_ਲੰਮਾਈ != Z_NULL)
        *dict_ਲੰਮਾਈ = state->whave;
    ਵਾਪਸ Z_OK;
}

ਪੂਰਨ ZEXPORT inflate_ਸੈੱਟ_Dictionary(z_streamp strm, ਅਚਲ Bytef *dictionary,
                                 uInt dict_ਲੰਮਾਈ) {
    ਬਣਤਰ inflate_state FAR *state;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ dictid;
    ਪੂਰਨ ret;

    /* check state */
    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ (state->wrap != 0 && state->ਬਹੁਲਕ != DICT)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    /* check for correct dictionary identifier */
    ਜੇ (state->ਬਹੁਲਕ == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_ਲੰਮਾਈ);
        ਜੇ (dictid != state->check)
            ਵਾਪਸ Z_ਅੰਕੜੇ_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_ਲੰਮਾਈ, dict_ਲੰਮਾਈ);
    ਜੇ (ret) {
        state->ਬਹੁਲਕ = MEM;
        ਵਾਪਸ Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    ਵਾਪਸ Z_OK;
}

ਪੂਰਨ ZEXPORT inflate_Get_ਸਿਰਲੇਖ(z_streamp strm, gz_headerp ਸਿਰ) {
    ਬਣਤਰ inflate_state FAR *state;

    /* check state */
    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ ((state->wrap & 2) == 0) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    /* save header structure */
    state->ਸਿਰ = ਸਿਰ;
    ਸਿਰ->ਪੂਰਾ_2 = 0;
    ਵਾਪਸ Z_OK;
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
local ਬਿਨਾਨਿਸ਼ਾਨੀ syncsearch(ਬਿਨਾਨਿਸ਼ਾਨੀ FAR *have, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *buf,
                          ਬਿਨਾਨਿਸ਼ਾਨੀ len) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ got;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅਗਲਾ;

    got = *have;
    ਅਗਲਾ = 0;
    ਜਦੋਂ (ਅਗਲਾ < len && got < 4) {
        ਜੇ ((ਪੂਰਨ)(buf[ਅਗਲਾ]) == (got < 2 ? 0 : 0xff))
            got++;
        ਨਹੀਂਤਾਂ ਜੇ (buf[ਅਗਲਾ])
            got = 0;
        ਨਹੀਂਤਾਂ
            got = 4 - got;
        ਅਗਲਾ++;
    }
    *have = got;
    ਵਾਪਸ ਅਗਲਾ;
}

ਪੂਰਨ ZEXPORT inflateSync(z_streamp strm) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;               /* number of bytes to look at or looked at */
    ਪੂਰਨ flags;                  /* temporary to save header status */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਵਿੱਚ, ਬਾਹਰਲਾ;      /* temporary to save total_in and total_out */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[4];       /* to restore bit buffer to byte string */
    ਬਣਤਰ inflate_state FAR *state;

    /* check parameters */
    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ (strm->avail_ਵਿੱਚ == 0 && state->ਬਿੱਟ < 8) ਵਾਪਸ Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    ਜੇ (state->ਬਹੁਲਕ != SYNC) {
        state->ਬਹੁਲਕ = SYNC;
        state->hold >>= state->ਬਿੱਟ & 7;
        state->ਬਿੱਟ -= state->ਬਿੱਟ & 7;
        len = 0;
        ਜਦੋਂ (state->ਬਿੱਟ >= 8) {
            buf[len++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(state->hold);
            state->hold >>= 8;
            state->ਬਿੱਟ -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->ਅਗਲਾ_ਵਿੱਚ, strm->avail_ਵਿੱਚ);
    strm->avail_ਵਿੱਚ -= len;
    strm->ਅਗਲਾ_ਵਿੱਚ += len;
    strm->ਕੁੱਲ_ਵਿੱਚ += len;

    /* return no joy or set up to restart inflate() on a new block */
    ਜੇ (state->have != 4) ਵਾਪਸ Z_ਅੰਕੜੇ_ERROR;
    ਜੇ (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    ਨਹੀਂਤਾਂ
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    ਵਿੱਚ = strm->ਕੁੱਲ_ਵਿੱਚ;  ਬਾਹਰਲਾ = strm->ਕੁੱਲ_ਬਾਹਰਲਾ;
    inflateReset(strm);
    strm->ਕੁੱਲ_ਵਿੱਚ = ਵਿੱਚ;  strm->ਕੁੱਲ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
    state->flags = flags;
    state->ਬਹੁਲਕ = TYPE;
    ਵਾਪਸ Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
ਪੂਰਨ ZEXPORT inflate_Sync_ਬਿੰਦੂ(z_streamp strm) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਵਾਪਸ state->ਬਹੁਲਕ == STORED && state->ਬਿੱਟ == 0;
}

ਪੂਰਨ ZEXPORT inflateCopy(z_streamp dest, z_streamp ਸਰੋਤ) {
    ਬਣਤਰ inflate_state FAR *state;
    ਬਣਤਰ inflate_state FAR *copy;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ;

    /* check input */
    ਜੇ (inflateStateCheck(ਸਰੋਤ) || dest == Z_NULL)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)ਸਰੋਤ->state;

    /* allocate space */
    copy = (ਬਣਤਰ inflate_state FAR *)
           ZALLOC(ਸਰੋਤ, 1, ਆਕਾਰ(ਬਣਤਰ inflate_state));
    ਜੇ (copy == Z_NULL) ਵਾਪਸ Z_MEM_ERROR;
    zmemzero(copy, ਆਕਾਰ(ਬਣਤਰ inflate_state));
    ਖਿੜਕੀ = Z_NULL;
    ਜੇ (state->ਖਿੜਕੀ != Z_NULL) {
        ਖਿੜਕੀ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *)
                 ZALLOC(ਸਰੋਤ, 1U << state->wbits, ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ));
        ਜੇ (ਖਿੜਕੀ == Z_NULL) {
            ZFREE(ਸਰੋਤ, copy);
            ਵਾਪਸ Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, ਸਰੋਤ, ਆਕਾਰ(z_ਧਾਰਾ));
    zmemcpy(copy, state, ਆਕਾਰ(ਬਣਤਰ inflate_state));
    copy->strm = dest;
    ਜੇ (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->ਅਗਲਾ = copy->codes + (state->ਅਗਲਾ - state->codes);
    ਜੇ (ਖਿੜਕੀ != Z_NULL)
        zmemcpy(ਖਿੜਕੀ, state->ਖਿੜਕੀ, state->whave);
    copy->ਖਿੜਕੀ = ਖਿੜਕੀ;
    dest->state = (ਬਣਤਰ ਅੰਦਰੂਨੀ_state FAR *)copy;
    ਵਾਪਸ Z_OK;
}

ਪੂਰਨ ZEXPORT inflateUndermine(z_streamp strm, ਪੂਰਨ subvert) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INFLATE_ALLOW_INVALID_ਦੂਰੀ_TOOFAR_ARRR
    state->sane = !subvert;
    ਵਾਪਸ Z_OK;
#ਨਹੀਂਤਾਂ
    (ਖਾਲੀ)subvert;
    state->sane = 1;
    ਵਾਪਸ Z_ਅੰਕੜੇ_ERROR;
#ਜੇ_ਅੰਤ
}

ਪੂਰਨ ZEXPORT inflateValidate(z_streamp strm, ਪੂਰਨ check) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਜੇ (check && state->wrap)
        state->wrap |= 4;
    ਨਹੀਂਤਾਂ
        state->wrap &= ~4;
    ਵਾਪਸ Z_OK;
}

ਲੰਮਾ ZEXPORT inflateMark(z_streamp strm) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (inflateStateCheck(strm))
        ਵਾਪਸ -(1L << 16);
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਵਾਪਸ (ਲੰਮਾ)(((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)((ਲੰਮਾ)state->ਪਿੱਠ)) << 16) +
        (state->ਬਹੁਲਕ == COPY ? state->ਲੰਮਾਈ :
            (state->ਬਹੁਲਕ == ਮੇਲ_2 ? state->was - state->ਲੰਮਾਈ : 0));
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ZEXPORT inflate_Codes_ਵਰਤਿਆ(z_streamp strm) {
    ਬਣਤਰ inflate_state FAR *state;
    ਜੇ (inflateStateCheck(strm)) ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)-1;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(state->ਅਗਲਾ - state->codes);
}
