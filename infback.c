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

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inftrees.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inflate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
ਪੂਰਨ ZEXPORT inflate_ਪਿੱਠ_Init_2(z_streamp strm, ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ, ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ,
                             ਪੂਰਨ ਧਾਰਾ_ਮਾਪ) {
    ਬਣਤਰ inflate_state FAR *state;

    ਜੇ (ਸੰਸਕਰਣ == Z_NULL || ਸੰਸਕਰਣ[0] != ZLIB_ਸੰਸਕਰਣ[0] ||
        ਧਾਰਾ_ਮਾਪ != (ਪੂਰਨ)(ਆਕਾਰ(z_ਧਾਰਾ)))
        ਵਾਪਸ Z_ਸੰਸਕਰਣ_ERROR;
    ਜੇ (strm == Z_NULL || ਖਿੜਕੀ == Z_NULL ||
        ਖਿੜਕੀ_ਬਿੱਟ < 8 || ਖਿੜਕੀ_ਬਿੱਟ > 15)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
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
    state = (ਬਣਤਰ inflate_state FAR *)ZALLOC(strm, 1,
                                               ਆਕਾਰ(ਬਣਤਰ inflate_state));
    ਜੇ (state == Z_NULL) ਵਾਪਸ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ਬਣਤਰ ਅੰਦਰੂਨੀ_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)ਖਿੜਕੀ_ਬਿੱਟ;
    state->wsize = 1U << ਖਿੜਕੀ_ਬਿੱਟ;
    state->ਖਿੜਕੀ = ਖਿੜਕੀ;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    ਵਾਪਸ Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਦੋ_2() \
    ਕਰੋ { \
        put = strm->ਅਗਲਾ_ਬਾਹਰਲਾ; \
        ਖੱਬਾ = strm->avail_ਬਾਹਰਲਾ; \
        ਅਗਲਾ = strm->ਅਗਲਾ_ਵਿੱਚ; \
        have = strm->avail_ਵਿੱਚ; \
        hold = state->hold; \
        ਬਿੱਟ = state->ਬਿੱਟ; \
    } ਜਦੋਂ (0)

/* Set state from registers for inflate_fast() */
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

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗੁੰਜਾਇਸ਼() \
    ਕਰੋ { \
        ਜੇ (ਖੱਬਾ == 0) { \
            put = state->ਖਿੜਕੀ; \
            ਖੱਬਾ = state->wsize; \
            state->whave = ਖੱਬਾ; \
            ਜੇ (ਬਾਹਰਲਾ(ਬਾਹਰਲਾ_desc, put, ਖੱਬਾ)) { \
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
ਪੂਰਨ ZEXPORT inflate_ਪਿੱਠ(z_streamp strm, ਵਿੱਚ_func ਵਿੱਚ, ਖਾਲੀ FAR *ਵਿੱਚ_desc,
                        ਬਾਹਰਲਾ_func ਬਾਹਰਲਾ, ਖਾਲੀ FAR *ਬਾਹਰਲਾ_desc) {
    ਬਣਤਰ inflate_state FAR *state;
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਅਗਲਾ;    /* next input */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *put;     /* next output */
    ਬਿਨਾਨਿਸ਼ਾਨੀ have, ਖੱਬਾ;        /* available input and output */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ hold;         /* bit buffer */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ;              /* bits in bit buffer */
    ਬਿਨਾਨਿਸ਼ਾਨੀ copy;              /* number of stored or match bytes to copy */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *from;    /* where to copy match bytes from */
    ਕੋਡ ਇੱਥੇ;                  /* current decoding table entry */
    ਕੋਡ ਆਖਰੀ;                  /* parent table entry */
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;               /* length to copy for repeats, bits to drop */
    ਪੂਰਨ ret;                    /* return code */
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਕ੍ਰਮ[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    ਜੇ (strm == Z_NULL || strm->state == Z_NULL)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->ਸੁਨੇਹਾ = Z_NULL;
    state->ਬਹੁਲਕ = TYPE;
    state->ਆਖਰੀ = 0;
    state->whave = 0;
    ਅਗਲਾ = strm->ਅਗਲਾ_ਵਿੱਚ;
    have = ਅਗਲਾ != Z_NULL ? strm->avail_ਵਿੱਚ : 0;
    hold = 0;
    ਬਿੱਟ = 0;
    put = state->ਖਿੜਕੀ;
    ਖੱਬਾ = state->wsize;

    /* Inflate until end of block marked as last */
    ਲਈ (;;)
        ਚੋਣ (state->ਬਹੁਲਕ) {
        ਹਾਲਤ TYPE:
            /* determine and dispatch block type */
            ਜੇ (state->ਆਖਰੀ) {
                BYTEBITS();
                state->ਬਹੁਲਕ = ਪੂਰਾ;
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
                state->ਬਹੁਲਕ = LEN;              /* decode codes */
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
            /* get and verify stored block length */
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

            /* copy stored block from input to output */
            ਜਦੋਂ (state->ਲੰਮਾਈ != 0) {
                copy = state->ਲੰਮਾਈ;
                PULL();
                ਗੁੰਜਾਇਸ਼();
                ਜੇ (copy > have) copy = have;
                ਜੇ (copy > ਖੱਬਾ) copy = ਖੱਬਾ;
                zmemcpy(put, ਅਗਲਾ, copy);
                have -= copy;
                ਅਗਲਾ += copy;
                ਖੱਬਾ -= copy;
                put += copy;
                state->ਲੰਮਾਈ -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->ਬਹੁਲਕ = TYPE;
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ PKZIP_BUG_WORKAROUND
            ਜੇ (state->nlen > 286 || state->ndist > 30) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                    "too many length or distance symbols";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
#ਜੇ_ਅੰਤ
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
            state->lencode = (ਕੋਡ ਅਚਲ FAR *)(state->ਅਗਲਾ);
            state->lenbits = 7;
            ret = inflate_ਸਾਰਣੀ(CODES, state->lens, 19, &(state->ਅਗਲਾ),
                                &(state->lenbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid code lengths set";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
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
            state->lencode = (ਕੋਡ ਅਚਲ FAR *)(state->ਅਗਲਾ);
            state->lenbits = 9;
            ret = inflate_ਸਾਰਣੀ(LENS, state->lens, state->nlen, &(state->ਅਗਲਾ),
                                &(state->lenbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid literal/lengths set";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            state->distcode = (ਕੋਡ ਅਚਲ FAR *)(state->ਅਗਲਾ);
            state->distbits = 6;
            ret = inflate_ਸਾਰਣੀ(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->ਅਗਲਾ), &(state->distbits), state->work);
            ਜੇ (ret) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distances set";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->ਬਹੁਲਕ = LEN;
                /* fallthrough */

        ਹਾਲਤ LEN:
            /* use inflate_fast() if we have enough input and output */
            ਜੇ (have >= 6 && ਖੱਬਾ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                ਲੱਦੋ_2();
                ਰੋਕੋ;
            }

            /* get a literal, length, or end-of-block code */
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
            }
            DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
            state->ਲੰਮਾਈ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਇੱਥੇ.val;

            /* process literal */
            ਜੇ (ਇੱਥੇ.op == 0) {
                Tracevv((stderr, ਇੱਥੇ.val >= 0x20 && ਇੱਥੇ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ਇੱਥੇ.val));
                ਗੁੰਜਾਇਸ਼();
                *put++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(state->ਲੰਮਾਈ);
                ਖੱਬਾ--;
                state->ਬਹੁਲਕ = LEN;
                ਰੋਕੋ;
            }

            /* process end of block */
            ਜੇ (ਇੱਥੇ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->ਬਹੁਲਕ = TYPE;
                ਰੋਕੋ;
            }

            /* invalid code */
            ਜੇ (ਇੱਥੇ.op & 64) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid literal/length code";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }

            /* length code -- get extra bits, if any */
            state->extra = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.op) & 15;
            ਜੇ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->ਲੰਮਾਈ += ਬਿੱਟ_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->ਲੰਮਾਈ));

            /* get distance code */
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
            }
            DROPBITS(ਇੱਥੇ.ਬਿੱਟ);
            ਜੇ (ਇੱਥੇ.op & 64) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distance code";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            state->ਵਿੱਥ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਇੱਥੇ.val;

            /* get distance extra bits, if any */
            state->extra = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ.op) & 15;
            ਜੇ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->ਵਿੱਥ += ਬਿੱਟ_2(state->extra);
                DROPBITS(state->extra);
            }
            ਜੇ (state->ਵਿੱਥ > state->wsize - (state->whave < state->wsize ?
                                                ਖੱਬਾ : 0)) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distance too far back";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->ਵਿੱਥ));

            /* copy match from window to output */
            ਕਰੋ {
                ਗੁੰਜਾਇਸ਼();
                copy = state->wsize - state->ਵਿੱਥ;
                ਜੇ (copy < ਖੱਬਾ) {
                    from = put + copy;
                    copy = ਖੱਬਾ - copy;
                }
                ਨਹੀਂਤਾਂ {
                    from = put - state->ਵਿੱਥ;
                    copy = ਖੱਬਾ;
                }
                ਜੇ (copy > state->ਲੰਮਾਈ) copy = state->ਲੰਮਾਈ;
                state->ਲੰਮਾਈ -= copy;
                ਖੱਬਾ -= copy;
                ਕਰੋ {
                    *put++ = *from++;
                } ਜਦੋਂ (--copy);
            } ਜਦੋਂ (state->ਲੰਮਾਈ != 0);
            ਰੋਕੋ;

        ਹਾਲਤ ਪੂਰਾ:
            /* inflate stream terminated properly */
            ret = Z_ਧਾਰਾ_ਸਿਰਾ;
            ਜਾਓ inf_leave;

        ਹਾਲਤ BAD:
            ret = Z_ਅੰਕੜੇ_ERROR;
            ਜਾਓ inf_leave;

        ਮੂਲ:
            /* can't happen, but makes compilers happy */
            ret = Z_ਧਾਰਾ_ERROR;
            ਜਾਓ inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    ਜੇ (ਖੱਬਾ < state->wsize) {
        ਜੇ (ਬਾਹਰਲਾ(ਬਾਹਰਲਾ_desc, state->ਖਿੜਕੀ, state->wsize - ਖੱਬਾ) &&
            ret == Z_ਧਾਰਾ_ਸਿਰਾ)
            ret = Z_BUF_ERROR;
    }
    strm->ਅਗਲਾ_ਵਿੱਚ = ਅਗਲਾ;
    strm->avail_ਵਿੱਚ = have;
    ਵਾਪਸ ret;
}

ਪੂਰਨ ZEXPORT inflate_ਪਿੱਠ_ਸਿਰਾ(z_streamp strm) {
    ਜੇ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ਵਾਪਸ Z_OK;
}
