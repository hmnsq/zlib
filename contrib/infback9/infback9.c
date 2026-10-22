/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "zutil.h"
#ಅನ್ತರ್ಭಾವ "infback9.h"
#ಅನ್ತರ್ಭಾವ "inftree9.h"
#ಅನ್ತರ್ಭಾವ "inflate9.h"

#ವ್ಯಾಖ್ಯಾನಿಸು WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
ಪೂರ್ಣಾಂಕ ZEXPORT inflateBack9Init_(z_ಹರಿವು FAR *strm, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ,
                              ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ, ಪೂರ್ಣಾಂಕ ಹರಿವು_ಗಾತ್ರ) {
    ರಚನೆ inflate_state FAR *state;

    ಆದರೆ (ಆವೃತ್ತಿ == Z_NULL || ಆವೃತ್ತಿ[0] != ZLIB_ಆವೃತ್ತಿ[0] ||
        ಹರಿವು_ಗಾತ್ರ != (ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(z_ಹರಿವು)))
        ಹಿಂತಿರುಗಿ Z_ಆವೃತ್ತಿ_ERROR;
    ಆದರೆ (strm == Z_NULL || ಕಿಟಕಿ == Z_NULL)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    strm->ಸಂದೇಶ = Z_NULL;                 /* in case we return an error */
    ಆದರೆ (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    ಆದರೆ (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (ರಚನೆ inflate_state FAR *)ZALLOC(strm, 1,
                                               ಗಾತ್ರ(ರಚನೆ inflate_state));
    ಆದರೆ (state == Z_NULL) ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->ಕಿಟಕಿ = ಕಿಟಕಿ;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAKEFIXED
#ಅನ್ತರ್ಭಾವ <stdio.h>

ಶೂನ್ಯ makefixed9(ಶೂನ್ಯ) {
    ಚಿಹ್ನೆರಹಿತ sym, ಬಿಟ್ಟುಗಳು, ಕೆಳಗಿನ, ಗಾತ್ರ_2;
    ಸಂಕೇತ_2 *ಮುಂದಿನ, *lenfix, *distfix;
    ರಚನೆ inflate_state state;
    ಸಂಕೇತ_2 fixed[544];

    /* literal/length table */
    sym = 0;
    ಆಗ (sym < 144) state.lens[sym++] = 8;
    ಆಗ (sym < 256) state.lens[sym++] = 9;
    ಆಗ (sym < 280) state.lens[sym++] = 7;
    ಆಗ (sym < 288) state.lens[sym++] = 8;
    ಮುಂದಿನ = fixed;
    lenfix = ಮುಂದಿನ;
    ಬಿಟ್ಟುಗಳು = 9;
    inflate_table9(LENS, state.lens, 288, &(ಮುಂದಿನ), &(ಬಿಟ್ಟುಗಳು), state.work);

    /* distance table */
    sym = 0;
    ಆಗ (sym < 32) state.lens[sym++] = 5;
    distfix = ಮುಂದಿನ;
    ಬಿಟ್ಟುಗಳು = 5;
    inflate_table9(DISTS, state.lens, 32, &(ಮುಂದಿನ), &(ಬಿಟ್ಟುಗಳು), state.work);

    /* write tables */
    puts("    /* inffix9.h -- table for decoding deflate64 fixed codes");
    puts("     * Generated automatically by makefixed9().");
    puts("     */");
    puts("");
    puts("    /* WARNING: this file should *not* be used by applications.");
    puts("       It is part of the implementation of this library and is");
    puts("       subject to change. Applications should only use zlib.h.");
    puts("     */");
    puts("");
    ಗಾತ್ರ_2 = 1U << 9;
    ಬರೆ("    static const code lenfix[%u] = {", ಗಾತ್ರ_2);
    ಕೆಳಗಿನ = 0;
    ಗಾಗಿ (;;) {
        ಆದರೆ ((ಕೆಳಗಿನ % 6) == 0) ಬರೆ("\n        ");
        ಬರೆ("{%u,%u,%d}", lenfix[ಕೆಳಗಿನ].op, lenfix[ಕೆಳಗಿನ].ಬಿಟ್ಟುಗಳು,
               lenfix[ಕೆಳಗಿನ].val);
        ಆದರೆ (++ಕೆಳಗಿನ == ಗಾತ್ರ_2) ನಿಲ್ಲಿಸು;
        ಅಕ್ಷರ_ಬರೆ(',');
    }
    puts("\n    };");
    ಗಾತ್ರ_2 = 1U << 5;
    ಬರೆ("\n    static const code distfix[%u] = {", ಗಾತ್ರ_2);
    ಕೆಳಗಿನ = 0;
    ಗಾಗಿ (;;) {
        ಆದರೆ ((ಕೆಳಗಿನ % 5) == 0) ಬರೆ("\n        ");
        ಬರೆ("{%u,%u,%d}", distfix[ಕೆಳಗಿನ].op, distfix[ಕೆಳಗಿನ].ಬಿಟ್ಟುಗಳು,
               distfix[ಕೆಳಗಿನ].val);
        ಆದರೆ (++ಕೆಳಗಿನ == ಗಾತ್ರ_2) ನಿಲ್ಲಿಸು;
        ಅಕ್ಷರ_ಬರೆ(',');
    }
    puts("\n    };");
}
#ಆದರೆ_ಅಂತ್ಯ /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = ಕಿಟಕಿ; \
            ಎಡ = WSIZE; \
            wrap = 1; \
            ಆದರೆ (ಹೊರಹೋಗು(ಹೊರಹೋಗು_desc, put, (ಚಿಹ್ನೆರಹಿತ)ಎಡ)) { \
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
ಪೂರ್ಣಾಂಕ ZEXPORT inflateBack9(z_ಹರಿವು FAR *strm, ಒಳ_func ಒಳ, ಶೂನ್ಯ FAR *ಒಳ_desc,
                         ಹೊರಹೋಗು_func ಹೊರಹೋಗು, ಶೂನ್ಯ FAR *ಹೊರಹೋಗು_desc) {
    ರಚನೆ inflate_state FAR *state;
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಮುಂದಿನ;    /* next input */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *put;     /* next output */
    ಚಿಹ್ನೆರಹಿತ have;              /* available input */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಎಡ;         /* available output */
    inflate_ಬಹುಲಕ ಬಹುಲಕ;          /* current inflate mode */
    ಪೂರ್ಣಾಂಕ lastblock;              /* true if processing last block */
    ಪೂರ್ಣಾಂಕ wrap;                   /* true if the window has wrapped */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ;  /* allocated sliding window, if needed */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ hold;         /* bit buffer */
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು;              /* bits in bit buffer */
    ಚಿಹ್ನೆರಹಿತ extra;             /* extra bits needed */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಉದ್ದಳತೆ;       /* literal or length of data to copy */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಸ್ಥಾನಾಂತರ;       /* distance back to copy string from */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ copy;         /* number of stored or match bytes to copy */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *from;    /* where to copy match bytes from */
    ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *lencode;    /* starting table for length/literal codes */
    ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *distcode;   /* starting table for distance codes */
    ಚಿಹ್ನೆರಹಿತ lenbits;           /* index bits for lencode */
    ಚಿಹ್ನೆರಹಿತ distbits;          /* index bits for distcode */
    ಸಂಕೇತ_2 ಇಲ್ಲಿ;                  /* current decoding table entry */
    ಸಂಕೇತ_2 ಕೊನೆಯ;                  /* parent table entry */
    ಚಿಹ್ನೆರಹಿತ len;               /* length to copy for repeats, bits to drop */
    ಪೂರ್ಣಾಂಕ ret;                    /* return code */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಕ್ರಮ[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ಅನ್ತರ್ಭಾವ "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    ಆದರೆ (strm == Z_NULL || strm->state == Z_NULL)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (ರಚನೆ inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->ಸಂದೇಶ = Z_NULL;
    ಬಹುಲಕ = TYPE;
    lastblock = 0;
    wrap = 0;
    ಕಿಟಕಿ = state->ಕಿಟಕಿ;
    ಮುಂದಿನ = strm->ಮುಂದಿನ_ಒಳ;
    have = ಮುಂದಿನ != Z_NULL ? strm->avail_ಒಳ : 0;
    hold = 0;
    ಬಿಟ್ಟುಗಳು = 0;
    put = ಕಿಟಕಿ;
    ಎಡ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    ಗಾಗಿ (;;)
        ಆಯ್ಕೆ (ಬಹುಲಕ) {
        ಸ್ಥಿತಿ TYPE:
            /* determine and dispatch block type */
            ಆದರೆ (lastblock) {
                BYTEBITS();
                ಬಹುಲಕ = ಮುಗಿದ;
                ನಿಲ್ಲಿಸು;
            }
            NEEDBITS(3);
            lastblock = ಬಿಟ್ಟುಗಳು_2(1);
            DROPBITS(1);
            ಆಯ್ಕೆ (ಬಿಟ್ಟುಗಳು_2(2)) {
            ಸ್ಥಿತಿ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                ಬಹುಲಕ = STORED;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                ಬಹುಲಕ = LEN;                     /* decode codes */
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                ಬಹುಲಕ = ಕೋಷ್ಟಕ;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 3:
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid block type";
                ಬಹುಲಕ = BAD;
            }
            DROPBITS(2);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ಆದರೆ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid stored block lengths";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            ಉದ್ದಳತೆ = (ಚಿಹ್ನೆರಹಿತ)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    ಉದ್ದಳತೆ));
            INITBITS();

            /* copy stored block from input to output */
            ಆಗ (ಉದ್ದಳತೆ != 0) {
                copy = ಉದ್ದಳತೆ;
                PULL();
                ಸ್ಥಳ();
                ಆದರೆ (copy > have) copy = have;
                ಆದರೆ (copy > ಎಡ) copy = ಎಡ;
                zmemcpy(put, ಮುಂದಿನ, copy);
                have -= copy;
                ಮುಂದಿನ += copy;
                ಎಡ -= copy;
                put += copy;
                ಉದ್ದಳತೆ -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            ಬಹುಲಕ = TYPE;
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
            ಆದರೆ (state->nlen > 286) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"too many length symbols";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
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
            lencode = (ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *)(state->ಮುಂದಿನ);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->ಮುಂದಿನ),
                                &(lenbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid code lengths set";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ಆಗ (state->have < state->nlen + state->ndist) {
                ಗಾಗಿ (;;) {
                    ಇಲ್ಲಿ = lencode[ಬಿಟ್ಟುಗಳು_2(lenbits)];
                    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                    PULLBYTE();
                }
                ಆದರೆ (ಇಲ್ಲಿ.val < 16) {
                    NEEDBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
                    DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
                    state->lens[state->have++] = ಇಲ್ಲಿ.val;
                }
                ಇಲ್ಲದಿದ್ದರೆ {
                    ಆದರೆ (ಇಲ್ಲಿ.val == 16) {
                        NEEDBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು + 2);
                        DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
                        ಆದರೆ (state->have == 0) {
                            strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid bit length repeat";
                            ಬಹುಲಕ = BAD;
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
                        strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid bit length repeat";
                        ಬಹುಲಕ = BAD;
                        ನಿಲ್ಲಿಸು;
                    }
                    ಆಗ (copy--)
                        state->lens[state->have++] = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ)len;
                }
            }

            /* handle error breaks in while */
            ಆದರೆ (ಬಹುಲಕ == BAD) ನಿಲ್ಲಿಸು;

            /* check for end-of-block code (better have one) */
            ಆದರೆ (state->lens[256] == 0) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid code -- missing end-of-block";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->ಮುಂದಿನ = state->codes;
            lencode = (ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *)(state->ಮುಂದಿನ);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->ಮುಂದಿನ), &(lenbits), state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid literal/lengths set";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            distcode = (ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *)(state->ಮುಂದಿನ);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->ಮುಂದಿನ), &(distbits),
                            state->work);
            ಆದರೆ (ret) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distances set";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            ಬಹುಲಕ = LEN;

        ಸ್ಥಿತಿ LEN:
            /* get a literal, length, or end-of-block code */
            ಗಾಗಿ (;;) {
                ಇಲ್ಲಿ = lencode[ಬಿಟ್ಟುಗಳು_2(lenbits)];
                ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                PULLBYTE();
            }
            ಆದರೆ (ಇಲ್ಲಿ.op && (ಇಲ್ಲಿ.op & 0xf0) == 0) {
                ಕೊನೆಯ = ಇಲ್ಲಿ;
                ಗಾಗಿ (;;) {
                    ಇಲ್ಲಿ = lencode[ಕೊನೆಯ.val +
                            (ಬಿಟ್ಟುಗಳು_2(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಕೊನೆಯ.op) >> ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು)];
                    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                    PULLBYTE();
                }
                DROPBITS(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು);
            }
            DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
            ಉದ್ದಳತೆ = (ಚಿಹ್ನೆರಹಿತ)ಇಲ್ಲಿ.val;

            /* process literal */
            ಆದರೆ (ಇಲ್ಲಿ.op == 0) {
                Tracevv((stderr, ಇಲ್ಲಿ.val >= 0x20 && ಇಲ್ಲಿ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ಇಲ್ಲಿ.val));
                ಸ್ಥಳ();
                *put++ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(ಉದ್ದಳತೆ);
                ಎಡ--;
                ಬಹುಲಕ = LEN;
                ನಿಲ್ಲಿಸು;
            }

            /* process end of block */
            ಆದರೆ (ಇಲ್ಲಿ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                ಬಹುಲಕ = TYPE;
                ನಿಲ್ಲಿಸು;
            }

            /* invalid code */
            ಆದರೆ (ಇಲ್ಲಿ.op & 64) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid literal/length code";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }

            /* length code -- get extra bits, if any */
            extra = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.op) & 31;
            ಆದರೆ (extra != 0) {
                NEEDBITS(extra);
                ಉದ್ದಳತೆ += ಬಿಟ್ಟುಗಳು_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", ಉದ್ದಳತೆ));

            /* get distance code */
            ಗಾಗಿ (;;) {
                ಇಲ್ಲಿ = distcode[ಬಿಟ್ಟುಗಳು_2(distbits)];
                ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                PULLBYTE();
            }
            ಆದರೆ ((ಇಲ್ಲಿ.op & 0xf0) == 0) {
                ಕೊನೆಯ = ಇಲ್ಲಿ;
                ಗಾಗಿ (;;) {
                    ಇಲ್ಲಿ = distcode[ಕೊನೆಯ.val +
                            (ಬಿಟ್ಟುಗಳು_2(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಕೊನೆಯ.op) >> ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು)];
                    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು + ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು) <= ಬಿಟ್ಟುಗಳು) ನಿಲ್ಲಿಸು;
                    PULLBYTE();
                }
                DROPBITS(ಕೊನೆಯ.ಬಿಟ್ಟುಗಳು);
            }
            DROPBITS(ಇಲ್ಲಿ.ಬಿಟ್ಟುಗಳು);
            ಆದರೆ (ಇಲ್ಲಿ.op & 64) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distance code";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            ಸ್ಥಾನಾಂತರ = (ಚಿಹ್ನೆರಹಿತ)ಇಲ್ಲಿ.val;

            /* get distance extra bits, if any */
            extra = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ.op) & 15;
            ಆದರೆ (extra != 0) {
                NEEDBITS(extra);
                ಸ್ಥಾನಾಂತರ += ಬಿಟ್ಟುಗಳು_2(extra);
                DROPBITS(extra);
            }
            ಆದರೆ (ಸ್ಥಾನಾಂತರ > WSIZE - (wrap ? 0: ಎಡ)) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distance too far back";
                ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", ಸ್ಥಾನಾಂತರ));

            /* copy match from window to output */
            ಮಾಡು {
                ಸ್ಥಳ();
                copy = WSIZE - ಸ್ಥಾನಾಂತರ;
                ಆದರೆ (copy < ಎಡ) {
                    from = put + copy;
                    copy = ಎಡ - copy;
                }
                ಇಲ್ಲದಿದ್ದರೆ {
                    from = put - ಸ್ಥಾನಾಂತರ;
                    copy = ಎಡ;
                }
                ಆದರೆ (copy > ಉದ್ದಳತೆ) copy = ಉದ್ದಳತೆ;
                ಉದ್ದಳತೆ -= copy;
                ಎಡ -= copy;
                ಮಾಡು {
                    *put++ = *from++;
                } ಆಗ (--copy);
            } ಆಗ (ಉದ್ದಳತೆ != 0);
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ ಮುಗಿದ:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_ಹರಿವು_ಕೊನೆ;
            ಆದರೆ (ಎಡ < WSIZE) {
                ಆದರೆ (ಹೊರಹೋಗು(ಹೊರಹೋಗು_desc, ಕಿಟಕಿ, (ಚಿಹ್ನೆರಹಿತ)(WSIZE - ಎಡ)))
                    ret = Z_BUF_ERROR;
            }
            ಹೋಗು inf_leave;

        ಸ್ಥಿತಿ BAD:
            ret = Z_ದತ್ತಾಂಶ_ERROR;
            ಹೋಗು inf_leave;

        ಪೂರ್ವನಿಯೋಜಿತ:                /* can't happen, but makes compilers happy */
            ret = Z_ಹರಿವು_ERROR;
            ಹೋಗು inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->ಮುಂದಿನ_ಒಳ = ಮುಂದಿನ;
    strm->avail_ಒಳ = have;
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ ZEXPORT inflate_Back9_ಕೊನೆ(z_ಹರಿವು FAR *strm) {
    ಆದರೆ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ಹಿಂತಿರುಗಿ Z_OK;
}
