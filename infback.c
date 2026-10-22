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

#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
puur_naa_nka ZEXPORT inflateBackInit_(z_streamp strm, puur_naa_nka windowBits,
                             unsigned var_na FAR *window, const var_na *version,
                             puur_naa_nka stream_size) {
    sa_nracanaa inflate_state FAR *state;

    ᰀᰧ (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (puur_naa_nka)(parimaa_na(z_stream)))
        ᰀᰧᰌ Z_VERSION_ERROR;
    ᰀᰧ (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        ᰀᰧᰌ Z_STREAM_ERROR;
    strm->msg = Z_NULL;                 /* in case we return an error */
    ᰀᰧ (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        ᰀᰧᰌ Z_STREAM_ERROR;
#ᰀᰧᰀ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    ᰀᰧ (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        ᰀᰧᰌ Z_STREAM_ERROR;
#ᰀᰧᰀ
        strm->zfree = zcfree;
#endif
    state = (sa_nracanaa inflate_state FAR *)ZALLOC(strm, 1,
                                               parimaa_na(sa_nracanaa inflate_state));
    ᰀᰧ (state == Z_NULL) ᰀᰧᰌ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (sa_nracanaa internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    ᰀᰧᰌ Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    do { \
        put = strm->next_out; \
        left = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } ᰀᰬ (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    do { \
        strm->next_out = put; \
        strm->avail_out = left; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } ᰀᰬ (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    do { \
        hold = 0; \
        bits = 0; \
    } ᰀᰬ (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    do { \
        ᰀᰧ (have == 0) { \
            have = in(in_desc, &next); \
            ᰀᰧ (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                goto inf_leave; \
            } \
        } \
    } ᰀᰬ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    do { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } ᰀᰬ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    do { \
        ᰀᰬ (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } ᰀᰬ (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } ᰀᰬ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    do { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } ᰀᰬ (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    do { \
        ᰀᰧ (left == 0) { \
            put = state->window; \
            left = state->wsize; \
            state->whave = left; \
            ᰀᰧ (out(out_desc, put, left)) { \
                ret = Z_BUF_ERROR; \
                goto inf_leave; \
            } \
        } \
    } ᰀᰬ (0)

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
puur_naa_nka ZEXPORT inflateBack(z_streamp strm, in_func in, void FAR *in_desc,
                        out_func out, void FAR *out_desc) {
    sa_nracanaa inflate_state FAR *state;
    z_const unsigned var_na FAR *next;    /* next input */
    unsigned var_na FAR *put;     /* next output */
    unsigned have, left;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned var_na FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    puur_naa_nka ret;                    /* return code */
    static const unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    ᰀᰧ (strm == Z_NULL || strm->state == Z_NULL)
        ᰀᰧᰌ Z_STREAM_ERROR;
    state = (sa_nracanaa inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->msg = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    left = state->wsize;

    /* Inflate until end of block marked as last */
    for (;;)
        switch (state->mode) {
        pak_sa TYPE:
            /* determine and dispatch block type */
            ᰀᰧ (state->last) {
                BYTEBITS();
                state->mode = DONE;
                bha_nga;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            switch (BITS(2)) {
            pak_sa 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                bha_nga;
            pak_sa 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                bha_nga;
            pak_sa 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                bha_nga;
            default:
                strm->msg = (z_const var_na *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            bha_nga;

        pak_sa STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ᰀᰧ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->msg = (z_const var_na *)"invalid stored block lengths";
                state->mode = BAD;
                bha_nga;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            ᰀᰬ (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                ᰀᰧ (copy > have) copy = have;
                ᰀᰧ (copy > left) copy = left;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                left -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            bha_nga;

        pak_sa TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            ᰀᰧ (state->nlen > 286 || state->ndist > 30) {
                strm->msg = (z_const var_na *)
                    "too many length or distance symbols";
                state->mode = BAD;
                bha_nga;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            ᰀᰬ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            ᰀᰬ (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code const FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            ᰀᰧ (ret) {
                strm->msg = (z_const var_na *)"invalid code lengths set";
                state->mode = BAD;
                bha_nga;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ᰀᰬ (state->have < state->nlen + state->ndist) {
                for (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    ᰀᰧ ((unsigned)(here.bits) <= bits) bha_nga;
                    PULLBYTE();
                }
                ᰀᰧ (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                ᰀᰧᰀ {
                    ᰀᰧ (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        ᰀᰧ (state->have == 0) {
                            strm->msg = (z_const var_na *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            bha_nga;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    ᰀᰧᰀ ᰀᰧ (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    ᰀᰧᰀ {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    ᰀᰧ (state->have + copy > state->nlen + state->ndist) {
                        strm->msg = (z_const var_na *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        bha_nga;
                    }
                    ᰀᰬ (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            ᰀᰧ (state->mode == BAD) bha_nga;

            /* check for end-of-block code (better have one) */
            ᰀᰧ (state->lens[256] == 0) {
                strm->msg = (z_const var_na *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                bha_nga;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code const FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            ᰀᰧ (ret) {
                strm->msg = (z_const var_na *)"invalid literal/lengths set";
                state->mode = BAD;
                bha_nga;
            }
            state->distcode = (code const FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            ᰀᰧ (ret) {
                strm->msg = (z_const var_na *)"invalid distances set";
                state->mode = BAD;
                bha_nga;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        pak_sa LEN:
            /* use inflate_fast() if we have enough input and output */
            ᰀᰧ (have >= 6 && left >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                bha_nga;
            }

            /* get a literal, length, or end-of-block code */
            for (;;) {
                here = state->lencode[BITS(state->lenbits)];
                ᰀᰧ ((unsigned)(here.bits) <= bits) bha_nga;
                PULLBYTE();
            }
            ᰀᰧ (here.op && (here.op & 0xf0) == 0) {
                last = here;
                for (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ᰀᰧ ((unsigned)(last.bits + here.bits) <= bits) bha_nga;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            ᰀᰧ (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned var_na)(state->length);
                left--;
                state->mode = LEN;
                bha_nga;
            }

            /* process end of block */
            ᰀᰧ (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                bha_nga;
            }

            /* invalid code */
            ᰀᰧ (here.op & 64) {
                strm->msg = (z_const var_na *)"invalid literal/length code";
                state->mode = BAD;
                bha_nga;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            ᰀᰧ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            for (;;) {
                here = state->distcode[BITS(state->distbits)];
                ᰀᰧ ((unsigned)(here.bits) <= bits) bha_nga;
                PULLBYTE();
            }
            ᰀᰧ ((here.op & 0xf0) == 0) {
                last = here;
                for (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ᰀᰧ ((unsigned)(last.bits + here.bits) <= bits) bha_nga;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            ᰀᰧ (here.op & 64) {
                strm->msg = (z_const var_na *)"invalid distance code";
                state->mode = BAD;
                bha_nga;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            ᰀᰧ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            ᰀᰧ (state->offset > state->wsize - (state->whave < state->wsize ?
                                                left : 0)) {
                strm->msg = (z_const var_na *)"invalid distance too far back";
                state->mode = BAD;
                bha_nga;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            do {
                ROOM();
                copy = state->wsize - state->offset;
                ᰀᰧ (copy < left) {
                    from = put + copy;
                    copy = left - copy;
                }
                ᰀᰧᰀ {
                    from = put - state->offset;
                    copy = left;
                }
                ᰀᰧ (copy > state->length) copy = state->length;
                state->length -= copy;
                left -= copy;
                do {
                    *put++ = *from++;
                } ᰀᰬ (--copy);
            } ᰀᰬ (state->length != 0);
            bha_nga;

        pak_sa DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            goto inf_leave;

        pak_sa BAD:
            ret = Z_DATA_ERROR;
            goto inf_leave;

        default:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            goto inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    ᰀᰧ (left < state->wsize) {
        ᰀᰧ (out(out_desc, state->window, state->wsize - left) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    ᰀᰧᰌ ret;
}

puur_naa_nka ZEXPORT inflateBackEnd(z_streamp strm) {
    ᰀᰧ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ᰀᰧᰌ Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ᰀᰧᰌ Z_OK;
}
