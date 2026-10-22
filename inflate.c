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

#ಅನ್ತರ್ಭಾವ "zutil.h"
#ಅನ್ತರ್ಭಾವ "inftrees.h"
#ಅನ್ತರ್ಭಾವ "inflate.h"
#ಅನ್ತರ್ಭಾವ "inffast.h"

local ಪೂರ್ಣಾಂಕ inflateStateCheck(z_streamp strm) {
    ರಚನೆ inflate_state FAR *state;
    ಆದರೆ (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        ಹಿಂತಿರುಗಿ 1;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ (state == Z_NULL || state->strm != strm ||
        state->ಬಹುಲಕ < ತಲೆ_2 || state->ಬಹುಲಕ > SYNC)
        ಹಿಂತಿರುಗಿ 1;
    ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateResetKeep(z_streamp strm) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    strm->ಒಟ್ಟು_ಒಳ = strm->ಒಟ್ಟು_ಹೊರಹೋಗು = state->ಒಟ್ಟು = 0;
    strm->ಸಂದೇಶ = Z_NULL;
    strm->ದತ್ತಾಂಶ_type = 0;
    ಆದರೆ (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->ಬಹುಲಕ = ತಲೆ_2;
    state->ಕೊನೆಯ = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->ತಲೆ = Z_NULL;
    state->hold = 0;
    state->ಬಿಟ್ಟುಗಳು = 0;
    state->lencode = state->distcode = state->ಮುಂದಿನ = state->codes;
    state->sane = 1;
    state->ಹಿಂಭಾಗ = -1;
    Tracev((stderr, "inflate: reset\n"));
    ಹಿಂತಿರುಗಿ Z_OK;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateReset(z_streamp strm) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    ಹಿಂತಿರುಗಿ inflateResetKeep(strm);
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateReset2(z_streamp strm, ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು) {
    ಪೂರ್ಣಾಂಕ wrap;
    ರಚನೆ inflate_state FAR *state;

    /* get the state */
    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < 0) {
        ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < -15)
            ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
        wrap = 0;
        ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು = -ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
        wrap = (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು >> 4) + 5;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
        ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < 48)
            ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು &= 15;
#ಆದರೆ_ಅಂತ್ಯ
    }

    /* set number of window bits, free window if different */
    ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು && (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < 8 || ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು > 15))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ಆದರೆ (state->ಕಿಟಕಿ != Z_NULL && state->wbits != (ಚಿಹ್ನೆರಹಿತ)ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು) {
        ZFREE(strm, state->ಕಿಟಕಿ);
        state->ಕಿಟಕಿ = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (ಚಿಹ್ನೆರಹಿತ)ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು;
    ಹಿಂತಿರುಗಿ inflateReset(strm);
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateInit2_(z_streamp strm, ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,
                          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ, ಪೂರ್ಣಾಂಕ ಹರಿವು_ಗಾತ್ರ) {
    ಪೂರ್ಣಾಂಕ ret;
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (ಆವೃತ್ತಿ == Z_NULL || ಆವೃತ್ತಿ[0] != ZLIB_ಆವೃತ್ತಿ[0] ||
        ಹರಿವು_ಗಾತ್ರ != (ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(z_ಹರಿವು)))
        ಹಿಂತಿರುಗಿ Z_ಆವೃತ್ತಿ_ERROR;
    ಆದರೆ (strm == Z_NULL) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    strm->ಸಂದೇಶ = Z_NULL;                 /* in case we return an error */
    ಆದರೆ (strm->zalloc == (alloc_func)0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಆದರೆ (strm->zfree == (free_func)0)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
        strm->zfree = zcfree;
#ಆದರೆ_ಅಂತ್ಯ
    state = (ರಚನೆ inflate_state FAR *)
            ZALLOC(strm, 1, ಗಾತ್ರ(ರಚನೆ inflate_state));
    ಆದರೆ (state == Z_NULL) ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    zmemzero(state, ಗಾತ್ರ(ರಚನೆ inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ರಚನೆ ಆಂತರಿಕ_state FAR *)state;
    state->strm = strm;
    state->ಕಿಟಕಿ = Z_NULL;
    state->ಬಹುಲಕ = ತಲೆ_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು);
    ಆದರೆ (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateInit_(z_streamp strm, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ,
                         ಪೂರ್ಣಾಂಕ ಹರಿವು_ಗಾತ್ರ) {
    ಹಿಂತಿರುಗಿ inflateInit2_(strm, DEF_WBITS, ಆವೃತ್ತಿ, ಹರಿವು_ಗಾತ್ರ);
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflate_ಅವಿಭಾಜ್ಯ(z_streamp strm, ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ಆದರೆ (ಬಿಟ್ಟುಗಳು == 0)
        ಹಿಂತಿರುಗಿ Z_OK;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ (ಬಿಟ್ಟುಗಳು < 0) {
        state->hold = 0;
        state->ಬಿಟ್ಟುಗಳು = 0;
        ಹಿಂತಿರುಗಿ Z_OK;
    }
    ಆದರೆ (ಬಿಟ್ಟುಗಳು > 16 || state->ಬಿಟ್ಟುಗಳು + (uInt)ಬಿಟ್ಟುಗಳು > 32) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ಮೌಲ್ಯ &= (1L << ಬಿಟ್ಟುಗಳು) - 1;
    state->hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಮೌಲ್ಯ << state->ಬಿಟ್ಟುಗಳು;
    state->ಬಿಟ್ಟುಗಳು += (uInt)ಬಿಟ್ಟುಗಳು;
    ಹಿಂತಿರುಗಿ Z_OK;
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
local ಪೂರ್ಣಾಂಕ updatewindow(z_streamp strm, ಸ್ಥಿರಾಂಕ Bytef *ಕೊನೆ, ಚಿಹ್ನೆರಹಿತ copy) {
    ರಚನೆ inflate_state FAR *state;
    ಚಿಹ್ನೆರಹಿತ dist;

    state = (ರಚನೆ inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    ಆದರೆ (state->ಕಿಟಕಿ == Z_NULL) {
        state->ಕಿಟಕಿ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ));
        ಆದರೆ (state->ಕಿಟಕಿ == Z_NULL) ಹಿಂತಿರುಗಿ 1;
    }

    /* if window not in use yet, initialize */
    ಆದರೆ (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    ಆದರೆ (copy >= state->wsize) {
        zmemcpy(state->ಕಿಟಕಿ, ಕೊನೆ - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
        dist = state->wsize - state->wnext;
        ಆದರೆ (dist > copy) dist = copy;
        zmemcpy(state->ಕಿಟಕಿ + state->wnext, ಕೊನೆ - copy, dist);
        copy -= dist;
        ಆದರೆ (copy) {
            zmemcpy(state->ಕಿಟಕಿ, ಕೊನೆ - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            state->wnext += dist;
            ಆದರೆ (state->wnext == state->wsize) state->wnext = 0;
            ಆದರೆ (state->whave < state->wsize) state->whave += dist;
        }
    }
    ಹಿಂತಿರುಗಿ 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(check, buf, len) : adler32(check, buf, len))
#ಇಲ್ಲದಿದ್ದರೆ
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#ಆದರೆ_ಅಂತ್ಯ

/* check macros for header crc */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
#  define CRC2(check, ಶಬ್ದ) \
    ಮಾಡು { \
        hbuf[0] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(ಶಬ್ದ); \
        hbuf[1] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((ಶಬ್ದ) >> 8); \
        check = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(check, hbuf, 2); \
    } ಆಗ (0)

#  define CRC4(check, ಶಬ್ದ) \
    ಮಾಡು { \
        hbuf[0] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(ಶಬ್ದ); \
        hbuf[1] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((ಶಬ್ದ) >> 8); \
        hbuf[2] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((ಶಬ್ದ) >> 16); \
        hbuf[3] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((ಶಬ್ದ) >> 24); \
        check = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(check, hbuf, 4); \
    } ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ

/* Load registers with state in inflate() for speed */
#ವ್ಯಾಖ್ಯಾನಿಸು ತುಂಬು_2() \
    ಮಾಡು { \
        put = strm->ಮುಂದಿನ_ಹೊರಹೋಗು; \
        ಎಡ = strm->avail_ಹೊರಹೋಗು; \
        ಮುಂದಿನ = strm->ಮುಂದಿನ_ಒಳ; \
        have = strm->avail_ಒಳ; \
        hold = state->hold; \
        ಬಿಟ್ಟುಗಳು = state->ಬಿಟ್ಟುಗಳು; \
    } ಆಗ (0)

/* Restore state from registers in inflate() */
#ವ್ಯಾಖ್ಯಾನಿಸು RESTORE() \
    ಮಾಡು { \
        strm->ಮುಂದಿನ_ಹೊರಹೋಗು = put; \
        strm->avail_ಹೊರಹೋಗು = ಎಡ; \
        strm->ಮುಂದಿನ_ಒಳ = ಮುಂದಿನ; \
        strm->avail_ಒಳ = have; \
        state->hold = hold; \
        state->ಬಿಟ್ಟುಗಳು = ಬಿಟ್ಟುಗಳು; \
    } ಆಗ (0)

/* Clear the input bit accumulator */
#ವ್ಯಾಖ್ಯಾನಿಸು INITBITS() \
    ಮಾಡು { \
        hold = 0; \
        ಬಿಟ್ಟುಗಳು = 0; \
    } ಆಗ (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#ವ್ಯಾಖ್ಯಾನಿಸು PULLBYTE() \
    ಮಾಡು { \
        ಆದರೆ (have == 0) ಹೋಗು inf_leave; \
        have--; \
        hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಮುಂದಿನ++) << ಬಿಟ್ಟುಗಳು; \
        ಬಿಟ್ಟುಗಳು += 8; \
    } ಆಗ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#ವ್ಯಾಖ್ಯಾನಿಸು NEEDBITS(n) \
    ಮಾಡು { \
        ಆಗ (ಬಿಟ್ಟುಗಳು < (ಚಿಹ್ನೆರಹಿತ)(n)) \
            PULLBYTE(); \
    } ಆಗ (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಿಟ್ಟುಗಳು_2(n) \
    ((ಚಿಹ್ನೆರಹಿತ)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#ವ್ಯಾಖ್ಯಾನಿಸು DROPBITS(n) \
    ಮಾಡು { \
        hold >>= (n); \
        ಬಿಟ್ಟುಗಳು -= (ಚಿಹ್ನೆರಹಿತ)(n); \
    } ಆಗ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#ವ್ಯಾಖ್ಯಾನಿಸು BYTEBITS() \
    ಮಾಡು { \
        hold >>= ಬಿಟ್ಟುಗಳು & 7; \
        ಬಿಟ್ಟುಗಳು -= ಬಿಟ್ಟುಗಳು & 7; \
    } ಆಗ (0)

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

ಪೂರ್ಣಾಂಕ ZEXPORT inflate(z_streamp strm, ಪೂರ್ಣಾಂಕ flush) {
    ರಚನೆ inflate_state FAR *state;
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಮುಂದಿನ;    /* next input */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *put;     /* next output */
    ಚಿಹ್ನೆರಹಿತ have, ಎಡ;        /* available input and output */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ hold;         /* bit buffer */
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು;              /* bits in bit buffer */
    ಚಿಹ್ನೆರಹಿತ ಒಳ, ಹೊರಹೋಗು;           /* save starting available input and output */
    ಚಿಹ್ನೆರಹಿತ copy;              /* number of stored or match bytes to copy */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *from;    /* where to copy match bytes from */
    ಸಂಕೇತ_2 ಇಲ್ಲಿ;                  /* current decoding table entry */
    ಸಂಕೇತ_2 ಕೊನೆಯ;                  /* parent table entry */
    ಚಿಹ್ನೆರಹಿತ len;               /* length to copy for repeats, bits to drop */
    ಪೂರ್ಣಾಂಕ ret;                    /* return code */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ hbuf[4];      /* buffer for gzip header crc calculation */
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಕ್ರಮ[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    ಆದರೆ (inflateStateCheck(strm) || strm->ಮುಂದಿನ_ಹೊರಹೋಗು == Z_NULL ||
        (strm->ಮುಂದಿನ_ಒಳ == Z_NULL && strm->avail_ಒಳ != 0))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ (state->ಬಹುಲಕ == TYPE) state->ಬಹುಲಕ = TYPEDO;      /* skip check */
    ತುಂಬು_2();
    ಒಳ = have;
    ಹೊರಹೋಗು = ಎಡ;
    ret = Z_OK;
    ಗಾಗಿ (;;)
        ಆಯ್ಕೆ (state->ಬಹುಲಕ) {
        ಸ್ಥಿತಿ ತಲೆ_2:
            ಆದರೆ (state->wrap == 0) {
                state->ಬಹುಲಕ = TYPEDO;
                ನಿಲ್ಲಿಸು;
            }
            NEEDBITS(16);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
            ಆದರೆ ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                ಆದರೆ (state->wbits == 0)
                    state->wbits = 15;
                state->check = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->ಬಹುಲಕ = FLAGS;
                ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ (state->ತಲೆ != Z_NULL)
                state->ತಲೆ->ಮುಗಿದ_2 = -1;
            ಆದರೆ (!(state->wrap & 1) ||   /* check if zlib header allowed */
#ಇಲ್ಲದಿದ್ದರೆ
            ಆದರೆ (
#ಆದರೆ_ಅಂತ್ಯ
                ((ಬಿಟ್ಟುಗಳು_2(8) << 8) + (hold >> 8)) % 31) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"incorrect header check";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ (ಬಿಟ್ಟುಗಳು_2(4) != Z_DEFLATED) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"unknown compression method";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            DROPBITS(4);
            len = ಬಿಟ್ಟುಗಳು_2(4) + 8;
            ಆದರೆ (state->wbits == 0)
                state->wbits = len;
            ಆದರೆ (len > 15 || len > state->wbits) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid window size";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->ಬಹುಲಕ = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
        ಸ್ಥಿತಿ FLAGS:
            NEEDBITS(16);
            state->flags = (ಪೂರ್ಣಾಂಕ)(hold);
            ಆದರೆ ((state->flags & 0xff) != Z_DEFLATED) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"unknown compression method";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ (state->flags & 0xe000) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"unknown header flags set";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ (state->ತಲೆ != Z_NULL)
                state->ತಲೆ->ಪಠ್ಯ = (ಪೂರ್ಣಾಂಕ)((hold >> 8) & 1);
            ಆದರೆ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->ಬಹುಲಕ = ಸಮಯ;
                /* fallthrough */
        ಸ್ಥಿತಿ ಸಮಯ:
            NEEDBITS(32);
            ಆದರೆ (state->ತಲೆ != Z_NULL)
                state->ತಲೆ->time = hold;
            ಆದರೆ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->ಬಹುಲಕ = OS;
                /* fallthrough */
        ಸ್ಥಿತಿ OS:
            NEEDBITS(16);
            ಆದರೆ (state->ತಲೆ != Z_NULL) {
                state->ತಲೆ->xflags = (ಪೂರ್ಣಾಂಕ)(hold & 0xff);
                state->ತಲೆ->os = (ಪೂರ್ಣಾಂಕ)(hold >> 8);
            }
            ಆದರೆ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->ಬಹುಲಕ = EXLEN;
                /* fallthrough */
        ಸ್ಥಿತಿ EXLEN:
            ಆದರೆ (state->flags & 0x0400) {
                NEEDBITS(16);
                state->ಉದ್ದಳತೆ = (ಚಿಹ್ನೆರಹಿತ)(hold);
                ಆದರೆ (state->ತಲೆ != Z_NULL)
                    state->ತಲೆ->extra_len = (ಚಿಹ್ನೆರಹಿತ)hold;
                ಆದರೆ ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->ತಲೆ != Z_NULL)
                state->ತಲೆ->extra = Z_NULL;
            state->ಬಹುಲಕ = EXTRA;
                /* fallthrough */
        ಸ್ಥಿತಿ EXTRA:
            ಆದರೆ (state->flags & 0x0400) {
                copy = state->ಉದ್ದಳತೆ;
                ಆದರೆ (copy > have) copy = have;
                ಆದರೆ (copy) {
                    ಆದರೆ (state->ತಲೆ != Z_NULL &&
                        state->ತಲೆ->extra != Z_NULL &&
                        (len = state->ತಲೆ->extra_len - state->ಉದ್ದಳತೆ) <
                            state->ತಲೆ->extra_max) {
                        zmemcpy(state->ತಲೆ->extra + len, ಮುಂದಿನ,
                                len + copy > state->ತಲೆ->extra_max ?
                                state->ತಲೆ->extra_max - len : copy);
                    }
                    ಆದರೆ ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(state->check, ಮುಂದಿನ, copy);
                    have -= copy;
                    ಮುಂದಿನ += copy;
                    state->ಉದ್ದಳತೆ -= copy;
                }
                ಆದರೆ (state->ಉದ್ದಳತೆ) ಹೋಗು inf_leave;
            }
            state->ಉದ್ದಳತೆ = 0;
            state->ಬಹುಲಕ = NAME;
                /* fallthrough */
        ಸ್ಥಿತಿ NAME:
            ಆದರೆ (state->flags & 0x0800) {
                ಆದರೆ (have == 0) ಹೋಗು inf_leave;
                copy = 0;
                ಮಾಡು {
                    len = (ಚಿಹ್ನೆರಹಿತ)(ಮುಂದಿನ[copy++]);
                    ಆದರೆ (state->ತಲೆ != Z_NULL &&
                            state->ತಲೆ->name != Z_NULL &&
                            state->ಉದ್ದಳತೆ < state->ತಲೆ->name_max)
                        state->ತಲೆ->name[state->ಉದ್ದಳತೆ++] = (Bytef)len;
                } ಆಗ (len && copy < have);
                ಆದರೆ ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(state->check, ಮುಂದಿನ, copy);
                have -= copy;
                ಮುಂದಿನ += copy;
                ಆದರೆ (len) ಹೋಗು inf_leave;
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->ತಲೆ != Z_NULL)
                state->ತಲೆ->name = Z_NULL;
            state->ಉದ್ದಳತೆ = 0;
            state->ಬಹುಲಕ = COMMENT;
                /* fallthrough */
        ಸ್ಥಿತಿ COMMENT:
            ಆದರೆ (state->flags & 0x1000) {
                ಆದರೆ (have == 0) ಹೋಗು inf_leave;
                copy = 0;
                ಮಾಡು {
                    len = (ಚಿಹ್ನೆರಹಿತ)(ಮುಂದಿನ[copy++]);
                    ಆದರೆ (state->ತಲೆ != Z_NULL &&
                            state->ತಲೆ->comment != Z_NULL &&
                            state->ಉದ್ದಳತೆ < state->ತಲೆ->comm_max)
                        state->ತಲೆ->comment[state->ಉದ್ದಳತೆ++] = (Bytef)len;
                } ಆಗ (len && copy < have);
                ಆದರೆ ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(state->check, ಮುಂದಿನ, copy);
                have -= copy;
                ಮುಂದಿನ += copy;
                ಆದರೆ (len) ಹೋಗು inf_leave;
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->ತಲೆ != Z_NULL)
                state->ತಲೆ->comment = Z_NULL;
            state->ಬಹುಲಕ = HCRC;
                /* fallthrough */
        ಸ್ಥಿತಿ HCRC:
            ಆದರೆ (state->flags & 0x0200) {
                NEEDBITS(16);
                ಆದರೆ ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"header crc mismatch";
                    state->ಬಹುಲಕ = BAD;
                    ನಿಲ್ಲಿಸು;
                }
                INITBITS();
            }
            ಆದರೆ (state->ತಲೆ != Z_NULL) {
                state->ತಲೆ->hcrc = (ಪೂರ್ಣಾಂಕ)((state->flags >> 9) & 1);
                state->ತಲೆ->ಮುಗಿದ_2 = 1;
            }
            strm->adler = state->check = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0);
            state->ಬಹುಲಕ = TYPE;
            ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
        ಸ್ಥಿತಿ DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->ಬಹುಲಕ = DICT;
                /* fallthrough */
        ಸ್ಥಿತಿ DICT:
            ಆದರೆ (state->havedict == 0) {
                RESTORE();
                ಹಿಂತಿರುಗಿ Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->ಬಹುಲಕ = TYPE;
                /* fallthrough */
        ಸ್ಥಿತಿ TYPE:
            ಆದರೆ (flush == Z_ಖಂಡ || flush == Z_TREES) ಹೋಗು inf_leave;
                /* fallthrough */
        ಸ್ಥಿತಿ TYPEDO:
            ಆದರೆ (state->ಕೊನೆಯ) {
                BYTEBITS();
                state->ಬಹುಲಕ = CHECK;
                ನಿಲ್ಲಿಸು;
            }
            NEEDBITS(3);
            state->ಕೊನೆಯ = ಬಿಟ್ಟುಗಳು_2(1);
            DROPBITS(1);
            ಆಯ್ಕೆ (ಬಿಟ್ಟುಗಳು_2(2)) {
            ಸ್ಥಿತಿ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->ಕೊನೆಯ ? " (last)" : ""));
                state->ಬಹುಲಕ = STORED;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->ಕೊನೆಯ ? " (last)" : ""));
                state->ಬಹುಲಕ = LEN_;             /* decode codes */
                ಆದರೆ (flush == Z_TREES) {
                    DROPBITS(2);
                    ಹೋಗು inf_leave;
                }
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->ಕೊನೆಯ ? " (last)" : ""));
                state->ಬಹುಲಕ = ಕೋಷ್ಟಕ;
                ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid block type";
                state->ಬಹುಲಕ = BAD;
            }
            DROPBITS(2);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ಆದರೆ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid stored block lengths";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            state->ಉದ್ದಳತೆ = (ಚಿಹ್ನೆರಹಿತ)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->ಉದ್ದಳತೆ));
            INITBITS();
            state->ಬಹುಲಕ = COPY_;
            ಆದರೆ (flush == Z_TREES) ಹೋಗು inf_leave;
                /* fallthrough */
        ಸ್ಥಿತಿ COPY_:
            state->ಬಹುಲಕ = COPY;
                /* fallthrough */
        ಸ್ಥಿತಿ COPY:
            copy = state->ಉದ್ದಳತೆ;
            ಆದರೆ (copy) {
                ಆದರೆ (copy > have) copy = have;
                ಆದರೆ (copy > ಎಡ) copy = ಎಡ;
                ಆದರೆ (copy == 0) ಹೋಗು inf_leave;
                zmemcpy(put, ಮುಂದಿನ, copy);
                have -= copy;
                ಮುಂದಿನ += copy;
                ಎಡ -= copy;
                put += copy;
                state->ಉದ್ದಳತೆ -= copy;
                ನಿಲ್ಲಿಸು;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->ಬಹುಲಕ = TYPE;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ ಕೋಷ್ಟಕ:
            NEEDBITS(14);
            state->nlen = ಬಿಟ್ಟುಗಳು_2(5) + 257;
            DROPBITS(5);
            state->ndist = ಬಿಟ್ಟುಗಳು_2(5) + 1;
            DROPBITS(5);
            state->ncode = ಬಿಟ್ಟುಗಳು_2(4) + 4;
            DROPBITS(4);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ PKZIP_BUG_WORKAROUND
            ಆದರೆ (state->nlen > 286 || state->ndist > 30) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)
                    "too many length or distance symbols";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
#ಆದರೆ_ಅಂತ್ಯ
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->ಬಹುಲಕ = LENLENS;
                /* fallthrough */
        ಸ್ಥಿತಿ LENLENS:
            ಆಗ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ಕ್ರಮ[state->have++]] = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ)ಬಿಟ್ಟುಗಳು_2(3);
                DROPBITS(3);
            }
            ಆಗ (state->have < 19)
                state->lens[ಕ್ರಮ[state->have++]] = 0;
            state->ಮುಂದಿನ = state->codes;
            state->lencode = state->distcode = (ಸ್ಥಿರಾಂಕ ಸಂಕೇತ_2 FAR *)(state->ಮುಂದಿನ);
            state->lenbits = 7;
            ret = inflate_ಕೋಷ್ಟಕ(CODES, state->lens, 19, &(state->ಮುಂದಿನ),
                                &(state->lenbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid code lengths set";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->ಬಹುಲಕ = CODELENS;
                /* fallthrough */
        ಸ್ಥಿತಿ CODELENS:
            ಆಗ (state->have < state->nlen + state->ndist) {
                ಗಾಗಿ (;;) {
                    ಇಲ್ಲಿ = state->lencode[ಬಿಟ್ಟುಗಳು_2(state->lenbits)];
                    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                    PULLBYTE();
                }
                ಆದರೆ (ಇಲ್ಲಿ.val < 16) {
                    DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
                    state->lens[state->have++] = ಇಲ್ಲಿ.val;
                }
                ಇಲ್ಲದಿದ್ದರೆ {
                    ಆದರೆ (ಇಲ್ಲಿ.val == 16) {
                        NEEDBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು + 2);
                        DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
                        ಆದರೆ (state->have == 0) {
                            strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)
                                "invalid bit length repeat";
                            state->ಬಹುಲಕ = BAD;
                            ನಿಲ್ಲಿಸು;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + ಬಿಟ್ಟುಗಳು_2(2);
                        DROPBITS(2);
                    }
                    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಇಲ್ಲಿ.val == 17) {
                        NEEDBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು + 3);
                        DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
                        len = 0;
                        copy = 3 + ಬಿಟ್ಟುಗಳು_2(3);
                        DROPBITS(3);
                    }
                    ಇಲ್ಲದಿದ್ದರೆ {
                        NEEDBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು + 7);
                        DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
                        len = 0;
                        copy = 11 + ಬಿಟ್ಟುಗಳು_2(7);
                        DROPBITS(7);
                    }
                    ಆದರೆ (state->have + copy > state->nlen + state->ndist) {
                        strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)
                            "invalid bit length repeat";
                        state->ಬಹುಲಕ = BAD;
                        ನಿಲ್ಲಿಸು;
                    }
                    ಆಗ (copy--)
                        state->lens[state->have++] = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ)len;
                }
            }

            /* handle error breaks in while */
            ಆದರೆ (state->ಬಹುಲಕ == BAD) ನಿಲ್ಲಿಸು;

            /* check for end-of-block code (better have one) */
            ಆದರೆ (state->lens[256] == 0) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)
                    "invalid code -- missing end-of-block";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->ಮುಂದಿನ = state->codes;
            state->lencode = (ಸ್ಥಿರಾಂಕ ಸಂಕೇತ_2 FAR *)(state->ಮುಂದಿನ);
            state->lenbits = 9;
            ret = inflate_ಕೋಷ್ಟಕ(LENS, state->lens, state->nlen, &(state->ಮುಂದಿನ),
                                &(state->lenbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid literal/lengths set";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            state->distcode = (ಸ್ಥಿರಾಂಕ ಸಂಕೇತ_2 FAR *)(state->ಮುಂದಿನ);
            state->distbits = 6;
            ret = inflate_ಕೋಷ್ಟಕ(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->ಮುಂದಿನ), &(state->distbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distances set";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->ಬಹುಲಕ = LEN_;
            ಆದರೆ (flush == Z_TREES) ಹೋಗು inf_leave;
                /* fallthrough */
        ಸ್ಥಿತಿ LEN_:
            state->ಬಹುಲಕ = LEN;
                /* fallthrough */
        ಸ್ಥಿತಿ LEN:
            ಆದರೆ (have >= 6 && ಎಡ >= 258) {
                RESTORE();
                inflate_fast(strm, ಹೊರಹೋಗು);
                ತುಂಬು_2();
                ಆದರೆ (state->ಬಹುಲಕ == TYPE)
                    state->ಹಿಂಭಾಗ = -1;
                ನಿಲ್ಲಿಸು;
            }
            state->ಹಿಂಭಾಗ = 0;
            ಗಾಗಿ (;;) {
                ಇಲ್ಲಿ = state->lencode[ಬಿಟ್ಟುಗಳು_2(state->lenbits)];
                ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                PULLBYTE();
            }
            ಆದರೆ (ಇಲ್ಲಿ.op && (ಇಲ್ಲಿ.op & 0xf0) == 0) {
                ಕೊನೆಯ = ಇಲ್ಲಿ;
                ಗಾಗಿ (;;) {
                    ಇಲ್ಲಿ = state->lencode[ಕೊನೆಯ.val +
                            (ಬಿಟ್ಟುಗಳು_2(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಕೊನೆಯ.op) >> ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು)];
                    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                    PULLBYTE();
                }
                DROPBITS(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು);
                state->ಹಿಂಭಾಗ += ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು;
            }
            DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
            state->ಹಿಂಭಾಗ += ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು;
            state->ಉದ್ದಳತೆ = (ಚಿಹ್ನೆರಹಿತ)ಇಲ್ಲಿ.val;
            ಆದರೆ ((ಪೂರ್ಣಾಂಕ)(ಇಲ್ಲಿ.op) == 0) {
                Tracevv((stderr, ಇಲ್ಲಿ.val >= 0x20 && ಇಲ್ಲಿ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ಇಲ್ಲಿ.val));
                state->ಬಹುಲಕ = LIT;
                ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ (ಇಲ್ಲಿ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->ಹಿಂಭಾಗ = -1;
                state->ಬಹುಲಕ = TYPE;
                ನಿಲ್ಲಿಸು;
            }
            ಆದರೆ (ಇಲ್ಲಿ.op & 64) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid literal/length code";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            state->extra = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.op) & 15;
            state->ಬಹುಲಕ = LENEXT;
                /* fallthrough */
        ಸ್ಥಿತಿ LENEXT:
            ಆದರೆ (state->extra) {
                NEEDBITS(state->extra);
                state->ಉದ್ದಳತೆ += ಬಿಟ್ಟುಗಳು_2(state->extra);
                DROPBITS(state->extra);
                state->ಹಿಂಭಾಗ += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->ಉದ್ದಳತೆ));
            state->was = state->ಉದ್ದಳತೆ;
            state->ಬಹುಲಕ = DIST;
                /* fallthrough */
        ಸ್ಥಿತಿ DIST:
            ಗಾಗಿ (;;) {
                ಇಲ್ಲಿ = state->distcode[ಬಿಟ್ಟುಗಳು_2(state->distbits)];
                ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                PULLBYTE();
            }
            ಆದರೆ ((ಇಲ್ಲಿ.op & 0xf0) == 0) {
                ಕೊನೆಯ = ಇಲ್ಲಿ;
                ಗಾಗಿ (;;) {
                    ಇಲ್ಲಿ = state->distcode[ಕೊನೆಯ.val +
                            (ಬಿಟ್ಟುಗಳು_2(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಕೊನೆಯ.op) >> ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು)];
                    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                    PULLBYTE();
                }
                DROPBITS(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು);
                state->ಹಿಂಭಾಗ += ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು;
            }
            DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
            state->ಹಿಂಭಾಗ += ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು;
            ಆದರೆ (ಇಲ್ಲಿ.op & 64) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distance code";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            state->ಸ್ಥಾನಾಂತರ = (ಚಿಹ್ನೆರಹಿತ)ಇಲ್ಲಿ.val;
            state->extra = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.op) & 15;
            state->ಬಹುಲಕ = DISTEXT;
                /* fallthrough */
        ಸ್ಥಿತಿ DISTEXT:
            ಆದರೆ (state->extra) {
                NEEDBITS(state->extra);
                state->ಸ್ಥಾನಾಂತರ += ಬಿಟ್ಟುಗಳು_2(state->extra);
                DROPBITS(state->extra);
                state->ಹಿಂಭಾಗ += state->extra;
            }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INFLATE_STRICT
            ಆದರೆ (state->ಸ್ಥಾನಾಂತರ > state->dmax) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distance too far back";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
#ಆದರೆ_ಅಂತ್ಯ
            Tracevv((stderr, "inflate:         distance %u\n", state->ಸ್ಥಾನಾಂತರ));
            state->ಬಹುಲಕ = ಹೊಂದಿಕೆ_2;
                /* fallthrough */
        ಸ್ಥಿತಿ ಹೊಂದಿಕೆ_2:
            ಆದರೆ (ಎಡ == 0) ಹೋಗು inf_leave;
            copy = ಹೊರಹೋಗು - ಎಡ;
            ಆದರೆ (state->ಸ್ಥಾನಾಂತರ > copy) {         /* copy from window */
                copy = state->ಸ್ಥಾನಾಂತರ - copy;
                ಆದರೆ (copy > state->whave) {
                    ಆದರೆ (state->sane) {
                        strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)
                            "invalid distance too far back";
                        state->ಬಹುಲಕ = BAD;
                        ನಿಲ್ಲಿಸು;
                    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INFLATE_ALLOW_INVALID_ದೂರ_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    ಆದರೆ (copy > state->ಉದ್ದಳತೆ) copy = state->ಉದ್ದಳತೆ;
                    ಆದರೆ (copy > ಎಡ) copy = ಎಡ;
                    ಎಡ -= copy;
                    state->ಉದ್ದಳತೆ -= copy;
                    ಮಾಡು {
                        *put++ = 0;
                    } ಆಗ (--copy);
                    ಆದರೆ (state->ಉದ್ದಳತೆ == 0) state->ಬಹುಲಕ = LEN;
                    ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
                }
                ಆದರೆ (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->ಕಿಟಕಿ + (state->wsize - copy);
                }
                ಇಲ್ಲದಿದ್ದರೆ
                    from = state->ಕಿಟಕಿ + (state->wnext - copy);
                ಆದರೆ (copy > state->ಉದ್ದಳತೆ) copy = state->ಉದ್ದಳತೆ;
            }
            ಇಲ್ಲದಿದ್ದರೆ {                              /* copy from output */
                from = put - state->ಸ್ಥಾನಾಂತರ;
                copy = state->ಉದ್ದಳತೆ;
            }
            ಆದರೆ (copy > ಎಡ) copy = ಎಡ;
            ಎಡ -= copy;
            state->ಉದ್ದಳತೆ -= copy;
            ಮಾಡು {
                *put++ = *from++;
            } ಆಗ (--copy);
            ಆದರೆ (state->ಉದ್ದಳತೆ == 0) state->ಬಹುಲಕ = LEN;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ LIT:
            ಆದರೆ (ಎಡ == 0) ಹೋಗು inf_leave;
            *put++ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(state->ಉದ್ದಳತೆ);
            ಎಡ--;
            state->ಬಹುಲಕ = LEN;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ CHECK:
            ಆದರೆ (state->wrap) {
                NEEDBITS(32);
                ಹೊರಹೋಗು -= ಎಡ;
                strm->ಒಟ್ಟು_ಹೊರಹೋಗು += ಹೊರಹೋಗು;
                state->ಒಟ್ಟು += ಹೊರಹೋಗು;
                ಆದರೆ ((state->wrap & 4) && ಹೊರಹೋಗು)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - ಹೊರಹೋಗು, ಹೊರಹೋಗು);
                ಹೊರಹೋಗು = ಎಡ;
                ಆದರೆ ((state->wrap & 4) && (
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
                     state->flags ? hold :
#ಆದರೆ_ಅಂತ್ಯ
                     ZSWAP32(hold)) != state->check) {
                    strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"incorrect data check";
                    state->ಬಹುಲಕ = BAD;
                    ನಿಲ್ಲಿಸು;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GUNZIP
            state->ಬಹುಲಕ = ಉದ್ದಳತೆ_2;
                /* fallthrough */
        ಸ್ಥಿತಿ ಉದ್ದಳತೆ_2:
            ಆದರೆ (state->wrap && state->flags) {
                NEEDBITS(32);
                ಆದರೆ ((state->wrap & 4) && hold != (state->ಒಟ್ಟು & 0xffffffff)) {
                    strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"incorrect length check";
                    state->ಬಹುಲಕ = BAD;
                    ನಿಲ್ಲಿಸು;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#ಆದರೆ_ಅಂತ್ಯ
            state->ಬಹುಲಕ = ಮುಗಿದ;
                /* fallthrough */
        ಸ್ಥಿತಿ ಮುಗಿದ:
            ret = Z_ಹರಿವು_ಕೊನೆ;
            ಹೋಗು inf_leave;
        ಸ್ಥಿತಿ BAD:
            ret = Z_ದತ್ತಾಂಶ_ERROR;
            ಹೋಗು inf_leave;
        ಸ್ಥಿತಿ MEM:
            ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
        ಸ್ಥಿತಿ SYNC:
                /* fallthrough */
        ಪೂರ್ವನಿಯೋಜಿತ:
            ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    ಆದರೆ (state->wsize || (ಹೊರಹೋಗು != strm->avail_ಹೊರಹೋಗು && state->ಬಹುಲಕ < BAD &&
            (state->ಬಹುಲಕ < CHECK || flush != Z_ಮುಕ್ತಾಯ)))
        ಆದರೆ (updatewindow(strm, strm->ಮುಂದಿನ_ಹೊರಹೋಗು, ಹೊರಹೋಗು - strm->avail_ಹೊರಹೋಗು)) {
            state->ಬಹುಲಕ = MEM;
            ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
        }
    ಒಳ -= strm->avail_ಒಳ;
    ಹೊರಹೋಗು -= strm->avail_ಹೊರಹೋಗು;
    strm->ಒಟ್ಟು_ಒಳ += ಒಳ;
    strm->ಒಟ್ಟು_ಹೊರಹೋಗು += ಹೊರಹೋಗು;
    state->ಒಟ್ಟು += ಹೊರಹೋಗು;
    ಆದರೆ ((state->wrap & 4) && ಹೊರಹೋಗು)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->ಮುಂದಿನ_ಹೊರಹೋಗು - ಹೊರಹೋಗು, ಹೊರಹೋಗು);
    strm->ದತ್ತಾಂಶ_type = (ಪೂರ್ಣಾಂಕ)state->ಬಿಟ್ಟುಗಳು + (state->ಕೊನೆಯ ? 64 : 0) +
                      (state->ಬಹುಲಕ == TYPE ? 128 : 0) +
                      (state->ಬಹುಲಕ == LEN_ || state->ಬಹುಲಕ == COPY_ ? 256 : 0);
    ಆದರೆ (((ಒಳ == 0 && ಹೊರಹೋಗು == 0) || flush == Z_ಮುಕ್ತಾಯ) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflate_ಕೊನೆ(z_streamp strm) {
    ರಚನೆ inflate_state FAR *state;
    ಆದರೆ (inflateStateCheck(strm))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ (state->ಕಿಟಕಿ != Z_NULL) ZFREE(strm, state->ಕಿಟಕಿ);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ಹಿಂತಿರುಗಿ Z_OK;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_ಉದ್ದಳತೆ) {
    ರಚನೆ inflate_state FAR *state;

    /* check state */
    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;

    /* copy dictionary */
    ಆದರೆ (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->ಕಿಟಕಿ + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->ಕಿಟಕಿ, state->wnext);
    }
    ಆದರೆ (dict_ಉದ್ದಳತೆ != Z_NULL)
        *dict_ಉದ್ದಳತೆ = state->whave;
    ಹಿಂತಿರುಗಿ Z_OK;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflate_ಗಣ_Dictionary(z_streamp strm, ಸ್ಥಿರಾಂಕ Bytef *dictionary,
                                 uInt dict_ಉದ್ದಳತೆ) {
    ರಚನೆ inflate_state FAR *state;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ dictid;
    ಪೂರ್ಣಾಂಕ ret;

    /* check state */
    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ (state->wrap != 0 && state->ಬಹುಲಕ != DICT)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    /* check for correct dictionary identifier */
    ಆದರೆ (state->ಬಹುಲಕ == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_ಉದ್ದಳತೆ);
        ಆದರೆ (dictid != state->check)
            ಹಿಂತಿರುಗಿ Z_ದತ್ತಾಂಶ_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_ಉದ್ದಳತೆ, dict_ಉದ್ದಳತೆ);
    ಆದರೆ (ret) {
        state->ಬಹುಲಕ = MEM;
        ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    ಹಿಂತಿರುಗಿ Z_OK;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflate_Get_ಶೀರ್ಷಿಕೆ(z_streamp strm, gz_headerp ತಲೆ) {
    ರಚನೆ inflate_state FAR *state;

    /* check state */
    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ ((state->wrap & 2) == 0) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    /* save header structure */
    state->ತಲೆ = ತಲೆ;
    ತಲೆ->ಮುಗಿದ_2 = 0;
    ಹಿಂತಿರುಗಿ Z_OK;
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
local ಚಿಹ್ನೆರಹಿತ syncsearch(ಚಿಹ್ನೆರಹಿತ FAR *have, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *buf,
                          ಚಿಹ್ನೆರಹಿತ len) {
    ಚಿಹ್ನೆರಹಿತ got;
    ಚಿಹ್ನೆರಹಿತ ಮುಂದಿನ;

    got = *have;
    ಮುಂದಿನ = 0;
    ಆಗ (ಮುಂದಿನ < len && got < 4) {
        ಆದರೆ ((ಪೂರ್ಣಾಂಕ)(buf[ಮುಂದಿನ]) == (got < 2 ? 0 : 0xff))
            got++;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (buf[ಮುಂದಿನ])
            got = 0;
        ಇಲ್ಲದಿದ್ದರೆ
            got = 4 - got;
        ಮುಂದಿನ++;
    }
    *have = got;
    ಹಿಂತಿರುಗಿ ಮುಂದಿನ;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateSync(z_streamp strm) {
    ಚಿಹ್ನೆರಹಿತ len;               /* number of bytes to look at or looked at */
    ಪೂರ್ಣಾಂಕ flags;                  /* temporary to save header status */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಒಳ, ಹೊರಹೋಗು;      /* temporary to save total_in and total_out */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf[4];       /* to restore bit buffer to byte string */
    ರಚನೆ inflate_state FAR *state;

    /* check parameters */
    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ (strm->avail_ಒಳ == 0 && state->ಬಿಟ್ಟುಗಳು < 8) ಹಿಂತಿರುಗಿ Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    ಆದರೆ (state->ಬಹುಲಕ != SYNC) {
        state->ಬಹುಲಕ = SYNC;
        state->hold >>= state->ಬಿಟ್ಟುಗಳು & 7;
        state->ಬಿಟ್ಟುಗಳು -= state->ಬಿಟ್ಟುಗಳು & 7;
        len = 0;
        ಆಗ (state->ಬಿಟ್ಟುಗಳು >= 8) {
            buf[len++] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(state->hold);
            state->hold >>= 8;
            state->ಬಿಟ್ಟುಗಳು -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->ಮುಂದಿನ_ಒಳ, strm->avail_ಒಳ);
    strm->avail_ಒಳ -= len;
    strm->ಮುಂದಿನ_ಒಳ += len;
    strm->ಒಟ್ಟು_ಒಳ += len;

    /* return no joy or set up to restart inflate() on a new block */
    ಆದರೆ (state->have != 4) ಹಿಂತಿರುಗಿ Z_ದತ್ತಾಂಶ_ERROR;
    ಆದರೆ (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    ಇಲ್ಲದಿದ್ದರೆ
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    ಒಳ = strm->ಒಟ್ಟು_ಒಳ;  ಹೊರಹೋಗು = strm->ಒಟ್ಟು_ಹೊರಹೋಗು;
    inflateReset(strm);
    strm->ಒಟ್ಟು_ಒಳ = ಒಳ;  strm->ಒಟ್ಟು_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
    state->flags = flags;
    state->ಬಹುಲಕ = TYPE;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
ಪೂರ್ಣಾಂಕ ZEXPORT inflate_Sync_ಬಿಂದು(z_streamp strm) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಹಿಂತಿರುಗಿ state->ಬಹುಲಕ == STORED && state->ಬಿಟ್ಟುಗಳು == 0;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateCopy(z_streamp dest, z_streamp ಮೂಲ) {
    ರಚನೆ inflate_state FAR *state;
    ರಚನೆ inflate_state FAR *copy;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ;

    /* check input */
    ಆದರೆ (inflateStateCheck(ಮೂಲ) || dest == Z_NULL)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)ಮೂಲ->state;

    /* allocate space */
    copy = (ರಚನೆ inflate_state FAR *)
           ZALLOC(ಮೂಲ, 1, ಗಾತ್ರ(ರಚನೆ inflate_state));
    ಆದರೆ (copy == Z_NULL) ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    zmemzero(copy, ಗಾತ್ರ(ರಚನೆ inflate_state));
    ಕಿಟಕಿ = Z_NULL;
    ಆದರೆ (state->ಕಿಟಕಿ != Z_NULL) {
        ಕಿಟಕಿ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *)
                 ZALLOC(ಮೂಲ, 1U << state->wbits, ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ));
        ಆದರೆ (ಕಿಟಕಿ == Z_NULL) {
            ZFREE(ಮೂಲ, copy);
            ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, ಮೂಲ, ಗಾತ್ರ(z_ಹರಿವು));
    zmemcpy(copy, state, ಗಾತ್ರ(ರಚನೆ inflate_state));
    copy->strm = dest;
    ಆದರೆ (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->ಮುಂದಿನ = copy->codes + (state->ಮುಂದಿನ - state->codes);
    ಆದರೆ (ಕಿಟಕಿ != Z_NULL)
        zmemcpy(ಕಿಟಕಿ, state->ಕಿಟಕಿ, state->whave);
    copy->ಕಿಟಕಿ = ಕಿಟಕಿ;
    dest->state = (ರಚನೆ ಆಂತರಿಕ_state FAR *)copy;
    ಹಿಂತಿರುಗಿ Z_OK;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateUndermine(z_streamp strm, ಪೂರ್ಣಾಂಕ subvert) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INFLATE_ALLOW_INVALID_ದೂರ_TOOFAR_ARRR
    state->sane = !subvert;
    ಹಿಂತಿರುಗಿ Z_OK;
#ಇಲ್ಲದಿದ್ದರೆ
    (ಶೂನ್ಯ)subvert;
    state->sane = 1;
    ಹಿಂತಿರುಗಿ Z_ದತ್ತಾಂಶ_ERROR;
#ಆದರೆ_ಅಂತ್ಯ
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflateValidate(z_streamp strm, ಪೂರ್ಣಾಂಕ check) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಆದರೆ (check && state->wrap)
        state->wrap |= 4;
    ಇಲ್ಲದಿದ್ದರೆ
        state->wrap &= ~4;
    ಹಿಂತಿರುಗಿ Z_OK;
}

ದೀರ್ಘ ZEXPORT inflateMark(z_streamp strm) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (inflateStateCheck(strm))
        ಹಿಂತಿರುಗಿ -(1L << 16);
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಹಿಂತಿರುಗಿ (ದೀರ್ಘ)(((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)((ದೀರ್ಘ)state->ಹಿಂಭಾಗ)) << 16) +
        (state->ಬಹುಲಕ == COPY ? state->ಉದ್ದಳತೆ :
            (state->ಬಹುಲಕ == ಹೊಂದಿಕೆ_2 ? state->was - state->ಉದ್ದಳತೆ : 0));
}

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ZEXPORT inflate_Codes_ಬಳಸಿದ(z_streamp strm) {
    ರಚನೆ inflate_state FAR *state;
    ಆದರೆ (inflateStateCheck(strm)) ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)-1;
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(state->ಮುಂದಿನ - state->codes);
}
