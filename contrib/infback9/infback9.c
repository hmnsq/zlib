/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "zutil.h"
#include "infback9.h"
#include "inftree9.h"
#include "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
puur_naa_nka ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned var_na FAR *window,
                              const var_na *version, puur_naa_nka stream_size) {
    sa_nracanaa inflate_state FAR *state;

    ꦪꦼꦤ꧀ (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (puur_naa_nka)(parimaa_na(z_stream)))
        ꦮꦭꦶ Z_VERSION_ERROR;
    ꦪꦼꦤ꧀ (strm == Z_NULL || window == Z_NULL)
        ꦮꦭꦶ Z_STREAM_ERROR;
    strm->msg = Z_NULL;                 /* in case we return an error */
    ꦪꦼꦤ꧀ (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    ꦪꦼꦤ꧀ (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (sa_nracanaa inflate_state FAR *)ZALLOC(strm, 1,
                                               parimaa_na(sa_nracanaa inflate_state));
    ꦪꦼꦤ꧀ (state == Z_NULL) ꦮꦭꦶ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    ꦮꦭꦶ Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#include <stdio.h>

void makefixed9(void) {
    unsigned sym, bits, low, size;
    code *next, *lenfix, *distfix;
    sa_nracanaa inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    ꦱꦼꦢꦼꦭ (sym < 144) state.lens[sym++] = 8;
    ꦱꦼꦢꦼꦭ (sym < 256) state.lens[sym++] = 9;
    ꦱꦼꦢꦼꦭ (sym < 280) state.lens[sym++] = 7;
    ꦱꦼꦢꦼꦭ (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    ꦱꦼꦢꦼꦭ (sym < 32) state.lens[sym++] = 5;
    distfix = next;
    bits = 5;
    inflate_table9(DISTS, state.lens, 32, &(next), &(bits), state.work);

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
    size = 1U << 9;
    printf("    static const code lenfix[%u] = {", size);
    low = 0;
    ꦏꦁꦒꦺꦴ (;;) {
        ꦪꦼꦤ꧀ ((low % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[low].op, lenfix[low].bits,
               lenfix[low].val);
        ꦪꦼꦤ꧀ (++low == size) bha_nga;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    low = 0;
    ꦏꦁꦒꦺꦴ (;;) {
        ꦪꦼꦤ꧀ ((low % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[low].op, distfix[low].bits,
               distfix[low].val);
        ꦪꦼꦤ꧀ (++low == size) bha_nga;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    do { \
        hold = 0; \
        bits = 0; \
    } ꦱꦼꦢꦼꦭ (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    do { \
        ꦪꦼꦤ꧀ (have == 0) { \
            have = in(in_desc, &next); \
            ꦪꦼꦤ꧀ (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                goto inf_leave; \
            } \
        } \
    } ꦱꦼꦢꦼꦭ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    do { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } ꦱꦼꦢꦼꦭ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    do { \
        ꦱꦼꦢꦼꦭ (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } ꦱꦼꦢꦼꦭ (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } ꦱꦼꦢꦼꦭ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    do { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } ꦱꦼꦢꦼꦭ (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    do { \
        ꦪꦼꦤ꧀ (left == 0) { \
            put = window; \
            left = WSIZE; \
            wrap = 1; \
            ꦪꦼꦤ꧀ (out(out_desc, put, (unsigned)left)) { \
                ret = Z_BUF_ERROR; \
                goto inf_leave; \
            } \
        } \
    } ꦱꦼꦢꦼꦭ (0)

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
puur_naa_nka ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, void FAR *in_desc,
                         out_func out, void FAR *out_desc) {
    sa_nracanaa inflate_state FAR *state;
    z_const unsigned var_na FAR *next;    /* next input */
    unsigned var_na FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long left;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    puur_naa_nka lastblock;              /* true if processing last block */
    puur_naa_nka wrap;                   /* true if the window has wrapped */
    unsigned var_na FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned var_na FAR *from;    /* where to copy match bytes from */
    code const FAR *lencode;    /* starting table for length/literal codes */
    code const FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    puur_naa_nka ret;                    /* return code */
    static const unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#include "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    ꦪꦼꦤ꧀ (strm == Z_NULL || strm->state == Z_NULL)
        ꦮꦭꦶ Z_STREAM_ERROR;
    state = (sa_nracanaa inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->msg = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    left = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    ꦏꦁꦒꦺꦴ (;;)
        switch (mode) {
        pak_sa TYPE:
            /* determine and dispatch block type */
            ꦪꦼꦤ꧀ (lastblock) {
                BYTEBITS();
                mode = DONE;
                bha_nga;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            switch (BITS(2)) {
            pak_sa 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                bha_nga;
            pak_sa 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                bha_nga;
            pak_sa 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                bha_nga;
            pak_sa 3:
                strm->msg = (z_const var_na *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            bha_nga;

        pak_sa STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ꦪꦼꦤ꧀ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->msg = (z_const var_na *)"invalid stored block lengths";
                mode = BAD;
                bha_nga;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            ꦱꦼꦢꦼꦭ (length != 0) {
                copy = length;
                PULL();
                ROOM();
                ꦪꦼꦤ꧀ (copy > have) copy = have;
                ꦪꦼꦤ꧀ (copy > left) copy = left;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                left -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
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
            ꦪꦼꦤ꧀ (state->nlen > 286) {
                strm->msg = (z_const var_na *)"too many length symbols";
                mode = BAD;
                bha_nga;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            ꦱꦼꦢꦼꦭ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            ꦱꦼꦢꦼꦭ (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code const FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            ꦪꦼꦤ꧀ (ret) {
                strm->msg = (z_const var_na *)"invalid code lengths set";
                mode = BAD;
                bha_nga;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ꦱꦼꦢꦼꦭ (state->have < state->nlen + state->ndist) {
                ꦏꦁꦒꦺꦴ (;;) {
                    here = lencode[BITS(lenbits)];
                    ꦪꦼꦤ꧀ ((unsigned)(here.bits) <= bits) bha_nga;
                    PULLBYTE();
                }
                ꦪꦼꦤ꧀ (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                ꦭꦶꦪ {
                    ꦪꦼꦤ꧀ (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        ꦪꦼꦤ꧀ (state->have == 0) {
                            strm->msg = (z_const var_na *)"invalid bit length repeat";
                            mode = BAD;
                            bha_nga;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    ꦭꦶꦪ ꦪꦼꦤ꧀ (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    ꦭꦶꦪ {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    ꦪꦼꦤ꧀ (state->have + copy > state->nlen + state->ndist) {
                        strm->msg = (z_const var_na *)"invalid bit length repeat";
                        mode = BAD;
                        bha_nga;
                    }
                    ꦱꦼꦢꦼꦭ (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            ꦪꦼꦤ꧀ (mode == BAD) bha_nga;

            /* check for end-of-block code (better have one) */
            ꦪꦼꦤ꧀ (state->lens[256] == 0) {
                strm->msg = (z_const var_na *)"invalid code -- missing end-of-block";
                mode = BAD;
                bha_nga;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code const FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            ꦪꦼꦤ꧀ (ret) {
                strm->msg = (z_const var_na *)"invalid literal/lengths set";
                mode = BAD;
                bha_nga;
            }
            distcode = (code const FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            ꦪꦼꦤ꧀ (ret) {
                strm->msg = (z_const var_na *)"invalid distances set";
                mode = BAD;
                bha_nga;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        pak_sa LEN:
            /* get a literal, length, or end-of-block code */
            ꦏꦁꦒꦺꦴ (;;) {
                here = lencode[BITS(lenbits)];
                ꦪꦼꦤ꧀ ((unsigned)(here.bits) <= bits) bha_nga;
                PULLBYTE();
            }
            ꦪꦼꦤ꧀ (here.op && (here.op & 0xf0) == 0) {
                last = here;
                ꦏꦁꦒꦺꦴ (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ꦪꦼꦤ꧀ ((unsigned)(last.bits + here.bits) <= bits) bha_nga;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            ꦪꦼꦤ꧀ (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned var_na)(length);
                left--;
                mode = LEN;
                bha_nga;
            }

            /* process end of block */
            ꦪꦼꦤ꧀ (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                bha_nga;
            }

            /* invalid code */
            ꦪꦼꦤ꧀ (here.op & 64) {
                strm->msg = (z_const var_na *)"invalid literal/length code";
                mode = BAD;
                bha_nga;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            ꦪꦼꦤ꧀ (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            ꦏꦁꦒꦺꦴ (;;) {
                here = distcode[BITS(distbits)];
                ꦪꦼꦤ꧀ ((unsigned)(here.bits) <= bits) bha_nga;
                PULLBYTE();
            }
            ꦪꦼꦤ꧀ ((here.op & 0xf0) == 0) {
                last = here;
                ꦏꦁꦒꦺꦴ (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ꦪꦼꦤ꧀ ((unsigned)(last.bits + here.bits) <= bits) bha_nga;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            ꦪꦼꦤ꧀ (here.op & 64) {
                strm->msg = (z_const var_na *)"invalid distance code";
                mode = BAD;
                bha_nga;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            ꦪꦼꦤ꧀ (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            ꦪꦼꦤ꧀ (offset > WSIZE - (wrap ? 0: left)) {
                strm->msg = (z_const var_na *)"invalid distance too far back";
                mode = BAD;
                bha_nga;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            do {
                ROOM();
                copy = WSIZE - offset;
                ꦪꦼꦤ꧀ (copy < left) {
                    from = put + copy;
                    copy = left - copy;
                }
                ꦭꦶꦪ {
                    from = put - offset;
                    copy = left;
                }
                ꦪꦼꦤ꧀ (copy > length) copy = length;
                length -= copy;
                left -= copy;
                do {
                    *put++ = *from++;
                } ꦱꦼꦢꦼꦭ (--copy);
            } ꦱꦼꦢꦼꦭ (length != 0);
            bha_nga;

        pak_sa DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            ꦪꦼꦤ꧀ (left < WSIZE) {
                ꦪꦼꦤ꧀ (out(out_desc, window, (unsigned)(WSIZE - left)))
                    ret = Z_BUF_ERROR;
            }
            goto inf_leave;

        pak_sa BAD:
            ret = Z_DATA_ERROR;
            goto inf_leave;

        default:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            goto inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    ꦮꦭꦶ ret;
}

puur_naa_nka ZEXPORT inflateBack9End(z_stream FAR *strm) {
    ꦪꦼꦤ꧀ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ꦮꦭꦶ Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ꦮꦭꦶ Z_OK;
}
