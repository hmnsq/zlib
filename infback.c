/* infback.c -- inflate using a call-back interface
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
   This code is largely copied from inflate.c.  Normally either infback.o or
   inflate.o would be linked into an application--not both.  The interface
   with inffast.c is retained so that optimized assembler-coded versions of
   inflate_fast() can be used with either inflate.c or infback.c.
 */

#ಅನ್ತರ್ಭಾವ "zutil.h"
#ಅನ್ತರ್ಭಾವ "inftrees.h"
#ಅನ್ತರ್ಭಾವ "inflate.h"
#ಅನ್ತರ್ಭಾವ "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
ಪೂರ್ಣಾಂಕ ZEXPORT inflate_ಹಿಂಭಾಗ_Init_2(z_streamp strm, ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,
                             ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ,
                             ಪೂರ್ಣಾಂಕ ಹರಿವು_ಗಾತ್ರ) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (ಆವೃತ್ತಿ == Z_NULL || ಆವೃತ್ತಿ[0] != ZLIB_ಆವೃತ್ತಿ[0] ||
        ಹರಿವು_ಗಾತ್ರ != (ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(z_ಹರಿವು)))
        ಹಿಂತಿರುಗಿ Z_ಆವೃತ್ತಿ_ERROR;
    ಆದರೆ (strm == Z_NULL || ಕಿಟಕಿ == Z_NULL ||
        ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < 8 || ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು > 15)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
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
    state = (ರಚನೆ inflate_state FAR *)ZALLOC(strm, 1,
                                               ಗಾತ್ರ(ರಚನೆ inflate_state));
    ಆದರೆ (state == Z_NULL) ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ರಚನೆ ಆಂತರಿಕ_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು;
    state->wsize = 1U << ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು;
    state->ಕಿಟಕಿ = ಕಿಟಕಿ;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#ವ್ಯಾಖ್ಯಾನಿಸು ತುಂಬು_2() \
    ಮಾಡು { \
        put = strm->ಮುಂದಿನ_ಹೊರಹೋಗು; \
        ಎಡ = strm->avail_ಹೊರಹೋಗು; \
        ಮುಂದಿನ = strm->ಮುಂದಿನ_ಒಳ; \
        have = strm->avail_ಒಳ; \
        hold = state->hold; \
        ಬಿಟ್ಟುಗಳು = state->ಬಿಟ್ಟುಗಳು; \
    } ಆಗ (0)

/* Set state from registers for inflate_fast() */
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

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#ವ್ಯಾಖ್ಯಾನಿಸು PULL() \
    ಮಾಡು { \
        ಆದರೆ (have == 0) { \
            have = ಒಳ(ಒಳ_desc, &ಮುಂದಿನ); \
            ಆದರೆ (have == 0) { \
                ಮುಂದಿನ = Z_NULL; \
                ret = Z_BUF_ERROR; \
                ಹೋಗು inf_leave; \
            } \
        } \
    } ಆಗ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#ವ್ಯಾಖ್ಯಾನಿಸು PULLBYTE() \
    ಮಾಡು { \
        PULL(); \
        have--; \
        hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಮುಂದಿನ++) << ಬಿಟ್ಟುಗಳು; \
        ಬಿಟ್ಟುಗಳು += 8; \
    } ಆಗ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
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

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಥಳ() \
    ಮಾಡು { \
        ಆದರೆ (ಎಡ == 0) { \
            put = state->ಕಿಟಕಿ; \
            ಎಡ = state->wsize; \
            state->whave = ಎಡ; \
            ಆದರೆ (ಹೊರಹೋಗು(ಹೊರಹೋಗು_desc, put, ಎಡ)) { \
                ret = Z_BUF_ERROR; \
                ಹೋಗು inf_leave; \
            } \
        } \
    } ಆಗ (0)

/*
   strm provides the memory allocation functions and window buffer on input,
   and provides information on the unused input on return.  For Z_DATA_ERROR
   returns, strm will also provide an error message.

   in() and out() are the call-back input and output functions.  When
   inflateBack() needs more input, it calls in().  When inflateBack() has
   filled the window with output, or when it completes with data in the
   window, it calls out() to write out the data.  The application must not
   change the provided input until in() is called again or inflateBack()
   returns.  The application must not change the window/output buffer until
   inflateBack() returns.

   in() and out() are called with a descriptor parameter provided in the
   inflateBack() call.  This parameter can be a structure that provides the
   information required to do the read or write, as well as accumulated
   information on the input and output such as totals and check values.

   in() should return zero on failure.  out() should return non-zero on
   failure.  If either in() or out() fails, than inflateBack() returns a
   Z_BUF_ERROR.  strm->next_in can be checked for Z_NULL to see whether it
   was in() or out() that caused in the error.  Otherwise,  inflateBack()
   returns Z_STREAM_END on success, Z_DATA_ERROR for an deflate format
   error, or Z_MEM_ERROR if it could not allocate memory for the state.
   inflateBack() can also return Z_STREAM_ERROR if the input parameters
   are not correct, i.e. strm is Z_NULL or the state was not initialized.
 */
ಪೂರ್ಣಾಂಕ ZEXPORT inflate_ಹಿಂಭಾಗ(z_streamp strm, ಒಳ_func ಒಳ, ಶೂನ್ಯ FAR *ಒಳ_desc,
                        ಹೊರಹೋಗು_func ಹೊರಹೋಗು, ಶೂನ್ಯ FAR *ಹೊರಹೋಗು_desc) {
    ರಚನೆ inflate_state FAR *state;
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಮುಂದಿನ;    /* next input */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *put;     /* next output */
    ಚಿಹ್ನೆರಹಿತ have, ಎಡ;        /* available input and output */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ hold;         /* bit buffer */
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು;              /* bits in bit buffer */
    ಚಿಹ್ನೆರಹಿತ copy;              /* number of stored or match bytes to copy */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *from;    /* where to copy match bytes from */
    ಸಂಕೇತ_2 ಇಲ್ಲಿ;                  /* current decoding table entry */
    ಸಂಕೇತ_2 ಕೊನೆಯ;                  /* parent table entry */
    ಚಿಹ್ನೆರಹಿತ len;               /* length to copy for repeats, bits to drop */
    ಪೂರ್ಣಾಂಕ ret;                    /* return code */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಕ್ರಮ[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    ಆದರೆ (strm == Z_NULL || strm->state == Z_NULL)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->ಸಂದೇಶ = Z_NULL;
    state->ಬಹುಲಕ = TYPE;
    state->ಕೊನೆಯ = 0;
    state->whave = 0;
    ಮುಂದಿನ = strm->ಮುಂದಿನ_ಒಳ;
    have = ಮುಂದಿನ != Z_NULL ? strm->avail_ಒಳ : 0;
    hold = 0;
    ಬಿಟ್ಟುಗಳು = 0;
    put = state->ಕಿಟಕಿ;
    ಎಡ = state->wsize;

    /* Inflate until end of block marked as last */
    ಗಾಗಿ (;;)
        ಆಯ್ಕೆ (state->ಬಹುಲಕ) {
        ಸ್ಥಿತಿ TYPE:
            /* determine and dispatch block type */
            ಆದರೆ (state->ಕೊನೆಯ) {
                BYTEBITS();
                state->ಬಹುಲಕ = ಮುಗಿದ;
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
                state->ಬಹುಲಕ = LEN;              /* decode codes */
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
            /* get and verify stored block length */
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

            /* copy stored block from input to output */
            ಆಗ (state->ಉದ್ದಳತೆ != 0) {
                copy = state->ಉದ್ದಳತೆ;
                PULL();
                ಸ್ಥಳ();
                ಆದರೆ (copy > have) copy = have;
                ಆದರೆ (copy > ಎಡ) copy = ಎಡ;
                zmemcpy(put, ಮುಂದಿನ, copy);
                have -= copy;
                ಮುಂದಿನ += copy;
                ಎಡ -= copy;
                put += copy;
                state->ಉದ್ದಳತೆ -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->ಬಹುಲಕ = TYPE;
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ ಕೋಷ್ಟಕ:
            /* get dynamic table entries descriptor */
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

            /* get code length code lengths (not a typo) */
            state->have = 0;
            ಆಗ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ಕ್ರಮ[state->have++]] = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ)ಬಿಟ್ಟುಗಳು_2(3);
                DROPBITS(3);
            }
            ಆಗ (state->have < 19)
                state->lens[ಕ್ರಮ[state->have++]] = 0;
            state->ಮುಂದಿನ = state->codes;
            state->lencode = (ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *)(state->ಮುಂದಿನ);
            state->lenbits = 7;
            ret = inflate_ಕೋಷ್ಟಕ(CODES, state->lens, 19, &(state->ಮುಂದಿನ),
                                &(state->lenbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid code lengths set";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
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
                        len = (ಚಿಹ್ನೆರಹಿತ)(state->lens[state->have - 1]);
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
            state->lencode = (ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *)(state->ಮುಂದಿನ);
            state->lenbits = 9;
            ret = inflate_ಕೋಷ್ಟಕ(LENS, state->lens, state->nlen, &(state->ಮುಂದಿನ),
                                &(state->lenbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid literal/lengths set";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            state->distcode = (ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *)(state->ಮುಂದಿನ);
            state->distbits = 6;
            ret = inflate_ಕೋಷ್ಟಕ(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->ಮುಂದಿನ), &(state->distbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distances set";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->ಬಹುಲಕ = LEN;
                /* fallthrough */

        ಸ್ಥಿತಿ LEN:
            /* use inflate_fast() if we have enough input and output */
            ಆದರೆ (have >= 6 && ಎಡ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                ತುಂಬು_2();
                ನಿಲ್ಲಿಸು;
            }

            /* get a literal, length, or end-of-block code */
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
            }
            DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
            state->ಉದ್ದಳತೆ = (ಚಿಹ್ನೆರಹಿತ)ಇಲ್ಲಿ.val;

            /* process literal */
            ಆದರೆ (ಇಲ್ಲಿ.op == 0) {
                Tracevv((stderr, ಇಲ್ಲಿ.val >= 0x20 && ಇಲ್ಲಿ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ಇಲ್ಲಿ.val));
                ಸ್ಥಳ();
                *put++ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(state->ಉದ್ದಳತೆ);
                ಎಡ--;
                state->ಬಹುಲಕ = LEN;
                ನಿಲ್ಲಿಸು;
            }

            /* process end of block */
            ಆದರೆ (ಇಲ್ಲಿ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->ಬಹುಲಕ = TYPE;
                ನಿಲ್ಲಿಸು;
            }

            /* invalid code */
            ಆದರೆ (ಇಲ್ಲಿ.op & 64) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid literal/length code";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }

            /* length code -- get extra bits, if any */
            state->extra = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.op) & 15;
            ಆದರೆ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->ಉದ್ದಳತೆ += ಬಿಟ್ಟುಗಳು_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->ಉದ್ದಳತೆ));

            /* get distance code */
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
            }
            DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
            ಆದರೆ (ಇಲ್ಲಿ.op & 64) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distance code";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            state->ಸ್ಥಾನಾಂತರ = (ಚಿಹ್ನೆರಹಿತ)ಇಲ್ಲಿ.val;

            /* get distance extra bits, if any */
            state->extra = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.op) & 15;
            ಆದರೆ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->ಸ್ಥಾನಾಂತರ += ಬಿಟ್ಟುಗಳು_2(state->extra);
                DROPBITS(state->extra);
            }
            ಆದರೆ (state->ಸ್ಥಾನಾಂತರ > state->wsize - (state->whave < state->wsize ?
                                                ಎಡ : 0)) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distance too far back";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->ಸ್ಥಾನಾಂತರ));

            /* copy match from window to output */
            ಮಾಡು {
                ಸ್ಥಳ();
                copy = state->wsize - state->ಸ್ಥಾನಾಂತರ;
                ಆದರೆ (copy < ಎಡ) {
                    from = put + copy;
                    copy = ಎಡ - copy;
                }
                ಇಲ್ಲದಿದ್ದರೆ {
                    from = put - state->ಸ್ಥಾನಾಂತರ;
                    copy = ಎಡ;
                }
                ಆದರೆ (copy > state->ಉದ್ದಳತೆ) copy = state->ಉದ್ದಳತೆ;
                state->ಉದ್ದಳತೆ -= copy;
                ಎಡ -= copy;
                ಮಾಡು {
                    *put++ = *from++;
                } ಆಗ (--copy);
            } ಆಗ (state->ಉದ್ದಳತೆ != 0);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ ಮುಗಿದ:
            /* inflate stream terminated properly */
            ret = Z_ಹರಿವು_ಕೊನೆ;
            ಹೋಗು inf_leave;

        ಸ್ಥಿತಿ BAD:
            ret = Z_ದತ್ತಾಂಶ_ERROR;
            ಹೋಗು inf_leave;

        ಪೂರ್ವನಿಯೋಜಿತ:
            /* can't happen, but makes compilers happy */
            ret = Z_ಹರಿವು_ERROR;
            ಹೋಗು inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    ಆದರೆ (ಎಡ < state->wsize) {
        ಆದರೆ (ಹೊರಹೋಗು(ಹೊರಹೋಗು_desc, state->ಕಿಟಕಿ, state->wsize - ಎಡ) &&
            ret == Z_ಹರಿವು_ಕೊನೆ)
            ret = Z_BUF_ERROR;
    }
    strm->ಮುಂದಿನ_ಒಳ = ಮುಂದಿನ;
    strm->avail_ಒಳ = have;
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflate_ಹಿಂಭಾಗ_ಕೊನೆ(z_streamp strm) {
    ಆದರೆ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ಹಿಂತಿರುಗಿ Z_OK;
}
