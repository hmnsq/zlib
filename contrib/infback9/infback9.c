/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "infback9.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inftree9.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inflate9.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
ਪੂਰਨ ZEXPORT inflateBack9Init_(z_ਧਾਰਾ FAR *strm, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ,
                              ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ, ਪੂਰਨ ਧਾਰਾ_ਮਾਪ) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (ਸੰਸਕਰਣ == Z_NULL || ਸੰਸਕਰਣ[0] != ZLIB_ਸੰਸਕਰਣ[0] ||
        ਧਾਰਾ_ਮਾਪ != (ਪੂਰਨ)(ਆਕਾਰ(z_ਧਾਰਾ)))
        ਵਾਪਸ Z_ਸੰਸਕਰਣ_ERROR;
    ਜੇ (strm == Z_NULL || ਖਿੜਕੀ == Z_NULL)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    strm->ਸੁਨੇਹਾ = Z_NULL;                 /* in case we return an error */
    ਜੇ (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    ਜੇ (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (ਬਣਤਰ inflate_state FAR *)ZALLOC(strm, 1,
                                               ਆਕਾਰ(ਬਣਤਰ inflate_state));
    ਜੇ (state == Z_NULL) ਵਾਪਸ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->ਖਿੜਕੀ = ਖਿੜਕੀ;
    ਵਾਪਸ Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAKEFIXED
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

ਖਾਲੀ makefixed9(ਖਾਲੀ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ sym, ਬਿੱਟ, ਨੀਵਾਂ, ਮਾਪ;
    ਕੋਡ *ਅਗਲਾ, *lenfix, *distfix;
    ਬਣਤਰ inflate_state state;
    ਕੋਡ fixed[544];

    /* literal/length table */
    sym = 0;
    ਜਦੋਂ (sym < 144) state.lens[sym++] = 8;
    ਜਦੋਂ (sym < 256) state.lens[sym++] = 9;
    ਜਦੋਂ (sym < 280) state.lens[sym++] = 7;
    ਜਦੋਂ (sym < 288) state.lens[sym++] = 8;
    ਅਗਲਾ = fixed;
    lenfix = ਅਗਲਾ;
    ਬਿੱਟ = 9;
    inflate_table9(LENS, state.lens, 288, &(ਅਗਲਾ), &(ਬਿੱਟ), state.work);

    /* distance table */
    sym = 0;
    ਜਦੋਂ (sym < 32) state.lens[sym++] = 5;
    distfix = ਅਗਲਾ;
    ਬਿੱਟ = 5;
    inflate_table9(DISTS, state.lens, 32, &(ਅਗਲਾ), &(ਬਿੱਟ), state.work);

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
    ਮਾਪ = 1U << 9;
    ਲਿਖੋ("    static const code lenfix[%u] = {", ਮਾਪ);
    ਨੀਵਾਂ = 0;
    ਲਈ (;;) {
        ਜੇ ((ਨੀਵਾਂ % 6) == 0) ਲਿਖੋ("\n        ");
        ਲਿਖੋ("{%u,%u,%d}", lenfix[ਨੀਵਾਂ].op, lenfix[ਨੀਵਾਂ].ਬਿੱਟ,
               lenfix[ਨੀਵਾਂ].val);
        ਜੇ (++ਨੀਵਾਂ == ਮਾਪ) ਰੋਕੋ;
        ਅੱਖਰ_ਲਿਖੋ(',');
    }
    puts("\n    };");
    ਮਾਪ = 1U << 5;
    ਲਿਖੋ("\n    static const code distfix[%u] = {", ਮਾਪ);
    ਨੀਵਾਂ = 0;
    ਲਈ (;;) {
        ਜੇ ((ਨੀਵਾਂ % 5) == 0) ਲਿਖੋ("\n        ");
        ਲਿਖੋ("{%u,%u,%d}", distfix[ਨੀਵਾਂ].op, distfix[ਨੀਵਾਂ].ਬਿੱਟ,
               distfix[ਨੀਵਾਂ].val);
        ਜੇ (++ਨੀਵਾਂ == ਮਾਪ) ਰੋਕੋ;
        ਅੱਖਰ_ਲਿਖੋ(',');
    }
    puts("\n    };");
}
#ਜੇ_ਅੰਤ /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITBITS() \
    ਕਰੋ { \
        hold = 0; \
        ਬਿੱਟ = 0; \
    } ਜਦੋਂ (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PULL() \
    ਕਰੋ { \
        ਜੇ (have == 0) { \
            have = ਵਿੱਚ(ਵਿੱਚ_desc, &ਅਗਲਾ); \
            ਜੇ (have == 0) { \
                ਅਗਲਾ = Z_NULL; \
                ret = Z_BUF_ERROR; \
                ਜਾਓ inf_leave; \
            } \
        } \
    } ਜਦੋਂ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PULLBYTE() \
    ਕਰੋ { \
        PULL(); \
        have--; \
        hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਅਗਲਾ++) << ਬਿੱਟ; \
        ਬਿੱਟ += 8; \
    } ਜਦੋਂ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NEEDBITS(n) \
    ਕਰੋ { \
        ਜਦੋਂ (ਬਿੱਟ < (ਬਿਨਾਨਿਸ਼ਾਨੀ)(n)) \
            PULLBYTE(); \
    } ਜਦੋਂ (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
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

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗੁੰਜਾਇਸ਼() \
    ਕਰੋ { \
        ਜੇ (ਖੱਬਾ == 0) { \
            put = ਖਿੜਕੀ; \
            ਖੱਬਾ = WSIZE; \
            wrap = 1; \
            ਜੇ (ਬਾਹਰਲਾ(ਬਾਹਰਲਾ_desc, put, (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੱਬਾ)) { \
                ret = Z_BUF_ERROR; \
                ਜਾਓ inf_leave; \
            } \
        } \
    } ਜਦੋਂ (0)

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
ਪੂਰਨ ZEXPORT inflateBack9(z_ਧਾਰਾ FAR *strm, ਵਿੱਚ_func ਵਿੱਚ, ਖਾਲੀ FAR *ਵਿੱਚ_desc,
                         ਬਾਹਰਲਾ_func ਬਾਹਰਲਾ, ਖਾਲੀ FAR *ਬਾਹਰਲਾ_desc) {
    ਬਣਤਰ inflate_state FAR *state;
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਅਗਲਾ;    /* next input */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *put;     /* next output */
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;              /* available input */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਖੱਬਾ;         /* available output */
    inflate_ਬਹੁਲਕ ਬਹੁਲਕ;          /* current inflate mode */
    ਪੂਰਨ lastblock;              /* true if processing last block */
    ਪੂਰਨ wrap;                   /* true if the window has wrapped */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ;  /* allocated sliding window, if needed */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ hold;         /* bit buffer */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ;              /* bits in bit buffer */
    ਬਿਨਾਨਿਸ਼ਾਨੀ extra;             /* extra bits needed */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾਈ;       /* literal or length of data to copy */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਵਿੱਥ;       /* distance back to copy string from */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ copy;         /* number of stored or match bytes to copy */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *from;    /* where to copy match bytes from */
    ਕੋਡ ਅਚਲ FAR *lencode;    /* starting table for length/literal codes */
    ਕੋਡ ਅਚਲ FAR *distcode;   /* starting table for distance codes */
    ਬਿਨਾਨਿਸ਼ਾਨੀ lenbits;           /* index bits for lencode */
    ਬਿਨਾਨਿਸ਼ਾਨੀ distbits;          /* index bits for distcode */
    ਕੋਡ ਇੱਥੇ;                  /* current decoding table entry */
    ਕੋਡ ਆਖਰੀ;                  /* parent table entry */
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;               /* length to copy for repeats, bits to drop */
    ਪੂਰਨ ret;                    /* return code */
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਕ੍ਰਮ[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ਅਨ੍ਤਰ੍ਭਾਵ "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    ਜੇ (strm == Z_NULL || strm->state == Z_NULL)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->ਸੁਨੇਹਾ = Z_NULL;
    ਬਹੁਲਕ = TYPE;
    lastblock = 0;
    wrap = 0;
    ਖਿੜਕੀ = state->ਖਿੜਕੀ;
    ਅਗਲਾ = strm->ਅਗਲਾ_ਵਿੱਚ;
    have = ਅਗਲਾ != Z_NULL ? strm->avail_ਵਿੱਚ : 0;
    hold = 0;
    ਬਿੱਟ = 0;
    put = ਖਿੜਕੀ;
    ਖੱਬਾ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    ਲਈ (;;)
        ਚੋਣ (ਬਹੁਲਕ) {
        ਹਾਲਤ TYPE:
            /* determine and dispatch block type */
            ਜੇ (lastblock) {
                BYTEBITS();
                ਬਹੁਲਕ = ਪੂਰਾ;
                ਰੋਕੋ;
            }
            NEEDBITS(3);
            lastblock = ਬਿੱਟ_2(1);
            DROPBITS(1);
            ਚੋਣ (ਬਿੱਟ_2(2)) {
            ਹਾਲਤ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                ਬਹੁਲਕ = STORED;
                ਰੋਕੋ;
            ਹਾਲਤ 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                ਬਹੁਲਕ = LEN;                     /* decode codes */
                ਰੋਕੋ;
            ਹਾਲਤ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                ਬਹੁਲਕ = ਸਾਰਣੀ;
                ਰੋਕੋ;
            ਹਾਲਤ 3:
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid block type";
                ਬਹੁਲਕ = BAD;
            }
            DROPBITS(2);
            ਰੋਕੋ;

        ਹਾਲਤ STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ਜੇ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid stored block lengths";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            ਲੰਮਾਈ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    ਲੰਮਾਈ));
            INITBITS();

            /* copy stored block from input to output */
            ਜਦੋਂ (ਲੰਮਾਈ != 0) {
                copy = ਲੰਮਾਈ;
                PULL();
                ਗੁੰਜਾਇਸ਼();
                ਜੇ (copy > have) copy = have;
                ਜੇ (copy > ਖੱਬਾ) copy = ਖੱਬਾ;
                zmemcpy(put, ਅਗਲਾ, copy);
                have -= copy;
                ਅਗਲਾ += copy;
                ਖੱਬਾ -= copy;
                put += copy;
                ਲੰਮਾਈ -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            ਬਹੁਲਕ = TYPE;
            ਰੋਕੋ;

        ਹਾਲਤ ਸਾਰਣੀ:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = ਬਿੱਟ_2(5) + 257;
            DROPBITS(5);
            state->ndist = ਬਿੱਟ_2(5) + 1;
            DROPBITS(5);
            state->ncode = ਬਿੱਟ_2(4) + 4;
            DROPBITS(4);
            ਜੇ (state->nlen > 286) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"too many length symbols";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            ਜਦੋਂ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ਕ੍ਰਮ[state->have++]] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ)ਬਿੱਟ_2(3);
                DROPBITS(3);
            }
            ਜਦੋਂ (state->have < 19)
                state->lens[ਕ੍ਰਮ[state->have++]] = 0;
            state->ਅਗਲਾ = state->codes;
            lencode = (ਕੋਡ ਅਚਲ FAR *)(state->ਅਗਲਾ);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->ਅਗਲਾ),
                                &(lenbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid code lengths set";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ਜਦੋਂ (state->have < state->nlen + state->ndist) {
                ਲਈ (;;) {
                    ਇੱਥੇ = lencode[ਬਿੱਟ_2(lenbits)];
                    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                    PULLBYTE();
                }
                ਜੇ (ਇੱਥੇ.val < 16) {
                    NEEDBITS(ਇੱਥੇ.ਬਿੱਟ);
                    DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
                    state->lens[state->have++] = ਇੱਥੇ.val;
                }
                ਨਹੀਂਤਾਂ {
                    ਜੇ (ਇੱਥੇ.val == 16) {
                        NEEDBITS(ਇੱਥੇ.ਬਿੱਟ + 2);
                        DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
                        ਜੇ (state->have == 0) {
                            strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid bit length repeat";
                            ਬਹੁਲਕ = BAD;
                            ਰੋਕੋ;
                        }
                        len = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(state->lens[state->have - 1]);
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
                        strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid bit length repeat";
                        ਬਹੁਲਕ = BAD;
                        ਰੋਕੋ;
                    }
                    ਜਦੋਂ (copy--)
                        state->lens[state->have++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ)len;
                }
            }

            /* handle error breaks in while */
            ਜੇ (ਬਹੁਲਕ == BAD) ਰੋਕੋ;

            /* check for end-of-block code (better have one) */
            ਜੇ (state->lens[256] == 0) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid code -- missing end-of-block";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->ਅਗਲਾ = state->codes;
            lencode = (ਕੋਡ ਅਚਲ FAR *)(state->ਅਗਲਾ);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->ਅਗਲਾ), &(lenbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid literal/lengths set";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            distcode = (ਕੋਡ ਅਚਲ FAR *)(state->ਅਗਲਾ);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->ਅਗਲਾ), &(distbits),
                            state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distances set";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            ਬਹੁਲਕ = LEN;

        ਹਾਲਤ LEN:
            /* get a literal, length, or end-of-block code */
            ਲਈ (;;) {
                ਇੱਥੇ = lencode[ਬਿੱਟ_2(lenbits)];
                ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                PULLBYTE();
            }
            ਜੇ (ਇੱਥੇ.op && (ਇੱਥੇ.op & 0xf0) == 0) {
                ਆਖਰੀ = ਇੱਥੇ;
                ਲਈ (;;) {
                    ਇੱਥੇ = lencode[ਆਖਰੀ.val +
                            (ਬਿੱਟ_2(ਆਖਰੀ.ਬਿੱਟ + ਆਖਰੀ.op) >> ਆਖਰੀ.ਬਿੱਟ)];
                    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਆਖਰੀ.ਬਿੱਟ + ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                    PULLBYTE();
                }
                DROPBITS(ਆਖਰੀ.ਬਿੱਟ);
            }
            DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
            ਲੰਮਾਈ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਇੱਥੇ.val;

            /* process literal */
            ਜੇ (ਇੱਥੇ.op == 0) {
                Tracevv((stderr, ਇੱਥੇ.val >= 0x20 && ਇੱਥੇ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ਇੱਥੇ.val));
                ਗੁੰਜਾਇਸ਼();
                *put++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(ਲੰਮਾਈ);
                ਖੱਬਾ--;
                ਬਹੁਲਕ = LEN;
                ਰੋਕੋ;
            }

            /* process end of block */
            ਜੇ (ਇੱਥੇ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                ਬਹੁਲਕ = TYPE;
                ਰੋਕੋ;
            }

            /* invalid code */
            ਜੇ (ਇੱਥੇ.op & 64) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid literal/length code";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }

            /* length code -- get extra bits, if any */
            extra = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.op) & 31;
            ਜੇ (extra != 0) {
                NEEDBITS(extra);
                ਲੰਮਾਈ += ਬਿੱਟ_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", ਲੰਮਾਈ));

            /* get distance code */
            ਲਈ (;;) {
                ਇੱਥੇ = distcode[ਬਿੱਟ_2(distbits)];
                ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                PULLBYTE();
            }
            ਜੇ ((ਇੱਥੇ.op & 0xf0) == 0) {
                ਆਖਰੀ = ਇੱਥੇ;
                ਲਈ (;;) {
                    ਇੱਥੇ = distcode[ਆਖਰੀ.val +
                            (ਬਿੱਟ_2(ਆਖਰੀ.ਬਿੱਟ + ਆਖਰੀ.op) >> ਆਖਰੀ.ਬਿੱਟ)];
                    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਆਖਰੀ.ਬਿੱਟ + ਇੱਥੇ.ਬਿੱਟ) <= ਬਿੱਟ) ਰੋਕੋ;
                    PULLBYTE();
                }
                DROPBITS(ਆਖਰੀ.ਬਿੱਟ);
            }
            DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
            ਜੇ (ਇੱਥੇ.op & 64) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distance code";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            ਵਿੱਥ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਇੱਥੇ.val;

            /* get distance extra bits, if any */
            extra = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.op) & 15;
            ਜੇ (extra != 0) {
                NEEDBITS(extra);
                ਵਿੱਥ += ਬਿੱਟ_2(extra);
                DROPBITS(extra);
            }
            ਜੇ (ਵਿੱਥ > WSIZE - (wrap ? 0: ਖੱਬਾ)) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distance too far back";
                ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", ਵਿੱਥ));

            /* copy match from window to output */
            ਕਰੋ {
                ਗੁੰਜਾਇਸ਼();
                copy = WSIZE - ਵਿੱਥ;
                ਜੇ (copy < ਖੱਬਾ) {
                    from = put + copy;
                    copy = ਖੱਬਾ - copy;
                }
                ਨਹੀਂਤਾਂ {
                    from = put - ਵਿੱਥ;
                    copy = ਖੱਬਾ;
                }
                ਜੇ (copy > ਲੰਮਾਈ) copy = ਲੰਮਾਈ;
                ਲੰਮਾਈ -= copy;
                ਖੱਬਾ -= copy;
                ਕਰੋ {
                    *put++ = *from++;
                } ਜਦੋਂ (--copy);
            } ਜਦੋਂ (ਲੰਮਾਈ != 0);
            ਰੋਕੋ;

        ਹਾਲਤ ਪੂਰਾ:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_ਧਾਰਾ_ਸਿਰਾ;
            ਜੇ (ਖੱਬਾ < WSIZE) {
                ਜੇ (ਬਾਹਰਲਾ(ਬਾਹਰਲਾ_desc, ਖਿੜਕੀ, (ਬਿਨਾਨਿਸ਼ਾਨੀ)(WSIZE - ਖੱਬਾ)))
                    ret = Z_BUF_ERROR;
            }
            ਜਾਓ inf_leave;

        ਹਾਲਤ BAD:
            ret = Z_ਅੰਕੜੇ_ERROR;
            ਜਾਓ inf_leave;

        ਮੂਲ:                /* can't happen, but makes compilers happy */
            ret = Z_ਧਾਰਾ_ERROR;
            ਜਾਓ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->ਅਗਲਾ_ਵਿੱਚ = ਅਗਲਾ;
    strm->avail_ਵਿੱਚ = have;
    ਵਾਪਸ ret;
}

ਪੂਰਨ ZEXPORT inflate_Back9_ਸਿਰਾ(z_ਧਾਰਾ FAR *strm) {
    ਜੇ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ਵਾਪਸ Z_OK;
}
