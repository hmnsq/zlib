/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "zutil.h"
#ಅನ್ತರ್ಭಾವ "infback9.h"
#ಅನ್ತರ್ಭಾವ "inftree9.h"
#ಅನ್ತರ್ಭಾವ "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned ವರ್ಣ FAR *window,
                              ಧ್ರುವ ವರ್ಣ *version, ಪೂರ್ಣಾಙ್ಕ stream_size) {
    ಸಂರಚನಾ inflate_state FAR *state;

    ಯದಿ (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (ಪೂರ್ಣಾಙ್ಕ)(ಪರಿಮಾಣ(z_stream)))
        ಪ್ರತ್ಯಾಗಮ Z_VERSION_ERROR;
    ಯದಿ (strm == Z_NULL || window == Z_NULL)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    strm->ಸಂದೇಶ = Z_NULL;                 /* in case we return an error */
    ಯದಿ (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    ಯದಿ (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (ಸಂರಚನಾ inflate_state FAR *)ZALLOC(strm, 1,
                                               ಪರಿಮಾಣ(ಸಂರಚನಾ inflate_state));
    ಯದಿ (state == Z_NULL) ಪ್ರತ್ಯಾಗಮ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    ಪ್ರತ್ಯಾಗಮ Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#ಅನ್ತರ್ಭಾವ <stdio.h>

ಶೂನ್ಯ makefixed9(ಶೂನ್ಯ) {
    unsigned sym, bits, ಕೆಳಗಿನ, size;
    code *next, *lenfix, *distfix;
    ಸಂರಚನಾ inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    ಯಾವತ್ (sym < 144) state.lens[sym++] = 8;
    ಯಾವತ್ (sym < 256) state.lens[sym++] = 9;
    ಯಾವತ್ (sym < 280) state.lens[sym++] = 7;
    ಯಾವತ್ (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    ಯಾವತ್ (sym < 32) state.lens[sym++] = 5;
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
    ಕೆಳಗಿನ = 0;
    ಪ್ರತಿ (;;) {
        ಯದಿ ((ಕೆಳಗಿನ % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[ಕೆಳಗಿನ].op, lenfix[ಕೆಳಗಿನ].bits,
               lenfix[ಕೆಳಗಿನ].val);
        ಯದಿ (++ಕೆಳಗಿನ == size) ಭಙ್ಗ;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    ಕೆಳಗಿನ = 0;
    ಪ್ರತಿ (;;) {
        ಯದಿ ((ಕೆಳಗಿನ % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[ಕೆಳಗಿನ].op, distfix[ಕೆಳಗಿನ].bits,
               distfix[ಕೆಳಗಿನ].val);
        ಯದಿ (++ಕೆಳಗಿನ == size) ಭಙ್ಗ;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    ಕುರು { \
        hold = 0; \
        bits = 0; \
    } ಯಾವತ್ (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    ಕುರು { \
        ಯದಿ (have == 0) { \
            have = in(in_desc, &next); \
            ಯದಿ (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                ಗಚ್ಛ inf_leave; \
            } \
        } \
    } ಯಾವತ್ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    ಕುರು { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } ಯಾವತ್ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    ಕುರು { \
        ಯಾವತ್ (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } ಯಾವತ್ (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    ಕುರು { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } ಯಾವತ್ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    ಕುರು { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } ಯಾವತ್ (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    ಕುರು { \
        ಯದಿ (ಎಡ == 0) { \
            put = window; \
            ಎಡ = WSIZE; \
            wrap = 1; \
            ಯದಿ (out(out_desc, put, (unsigned)ಎಡ)) { \
                ret = Z_BUF_ERROR; \
                ಗಚ್ಛ inf_leave; \
            } \
        } \
    } ಯಾವತ್ (0)

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
ಪೂರ್ಣಾಙ್ಕ ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, ಶೂನ್ಯ FAR *in_desc,
                         out_func out, ಶೂನ್ಯ FAR *out_desc) {
    ಸಂರಚನಾ inflate_state FAR *state;
    z_const unsigned ವರ್ಣ FAR *next;    /* next input */
    unsigned ವರ್ಣ FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long ಎಡ;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    ಪೂರ್ಣಾಙ್ಕ lastblock;              /* true if processing last block */
    ಪೂರ್ಣಾಙ್ಕ wrap;                   /* true if the window has wrapped */
    unsigned ವರ್ಣ FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned ವರ್ಣ FAR *from;    /* where to copy match bytes from */
    code ಧ್ರುವ FAR *lencode;    /* starting table for length/literal codes */
    code ಧ್ರುವ FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    ಪೂರ್ಣಾಙ್ಕ ret;                    /* return code */
    ಸ್ಥಿರ ಧ್ರುವ unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ಅನ್ತರ್ಭಾವ "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    ಯದಿ (strm == Z_NULL || strm->state == Z_NULL)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    state = (ಸಂರಚನಾ inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->ಸಂದೇಶ = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    ಎಡ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    ಪ್ರತಿ (;;)
        ವಿಕಲ್ಪ (mode) {
        ಪಕ್ಷ TYPE:
            /* determine and dispatch block type */
            ಯದಿ (lastblock) {
                BYTEBITS();
                mode = DONE;
                ಭಙ್ಗ;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            ವಿಕಲ್ಪ (BITS(2)) {
            ಪಕ್ಷ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                ಭಙ್ಗ;
            ಪಕ್ಷ 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                ಭಙ್ಗ;
            ಪಕ್ಷ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 3:
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            ಭಙ್ಗ;

        ಪಕ್ಷ STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ಯದಿ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid stored block lengths";
                mode = BAD;
                ಭಙ್ಗ;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            ಯಾವತ್ (length != 0) {
                copy = length;
                PULL();
                ROOM();
                ಯದಿ (copy > have) copy = have;
                ಯದಿ (copy > ಎಡ) copy = ಎಡ;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                ಎಡ -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            ಭಙ್ಗ;

        ಪಕ್ಷ TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            ಯದಿ (state->nlen > 286) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"too many length symbols";
                mode = BAD;
                ಭಙ್ಗ;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            ಯಾವತ್ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            ಯಾವತ್ (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code ಧ್ರುವ FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            ಯದಿ (ret) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid code lengths set";
                mode = BAD;
                ಭಙ್ಗ;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ಯಾವತ್ (state->have < state->nlen + state->ndist) {
                ಪ್ರತಿ (;;) {
                    here = lencode[BITS(lenbits)];
                    ಯದಿ ((unsigned)(here.bits) <= bits) ಭಙ್ಗ;
                    PULLBYTE();
                }
                ಯದಿ (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                ಅನ್ಯಥಾ {
                    ಯದಿ (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        ಯದಿ (state->have == 0) {
                            strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid bit length repeat";
                            mode = BAD;
                            ಭಙ್ಗ;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    ಅನ್ಯಥಾ ಯದಿ (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    ಅನ್ಯಥಾ {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    ಯದಿ (state->have + copy > state->nlen + state->ndist) {
                        strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid bit length repeat";
                        mode = BAD;
                        ಭಙ್ಗ;
                    }
                    ಯಾವತ್ (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            ಯದಿ (mode == BAD) ಭಙ್ಗ;

            /* check for end-of-block code (better have one) */
            ಯದಿ (state->lens[256] == 0) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid code -- missing end-of-block";
                mode = BAD;
                ಭಙ್ಗ;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code ಧ್ರುವ FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            ಯದಿ (ret) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid literal/lengths set";
                mode = BAD;
                ಭಙ್ಗ;
            }
            distcode = (code ಧ್ರುವ FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            ಯದಿ (ret) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid distances set";
                mode = BAD;
                ಭಙ್ಗ;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        ಪಕ್ಷ LEN:
            /* get a literal, length, or end-of-block code */
            ಪ್ರತಿ (;;) {
                here = lencode[BITS(lenbits)];
                ಯದಿ ((unsigned)(here.bits) <= bits) ಭಙ್ಗ;
                PULLBYTE();
            }
            ಯದಿ (here.op && (here.op & 0xf0) == 0) {
                last = here;
                ಪ್ರತಿ (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ಯದಿ ((unsigned)(last.bits + here.bits) <= bits) ಭಙ್ಗ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            ಯದಿ (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned ವರ್ಣ)(length);
                ಎಡ--;
                mode = LEN;
                ಭಙ್ಗ;
            }

            /* process end of block */
            ಯದಿ (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                ಭಙ್ಗ;
            }

            /* invalid code */
            ಯದಿ (here.op & 64) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid literal/length code";
                mode = BAD;
                ಭಙ್ಗ;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            ಯದಿ (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            ಪ್ರತಿ (;;) {
                here = distcode[BITS(distbits)];
                ಯದಿ ((unsigned)(here.bits) <= bits) ಭಙ್ಗ;
                PULLBYTE();
            }
            ಯದಿ ((here.op & 0xf0) == 0) {
                last = here;
                ಪ್ರತಿ (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ಯದಿ ((unsigned)(last.bits + here.bits) <= bits) ಭಙ್ಗ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            ಯದಿ (here.op & 64) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid distance code";
                mode = BAD;
                ಭಙ್ಗ;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            ಯದಿ (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            ಯದಿ (offset > WSIZE - (wrap ? 0: ಎಡ)) {
                strm->ಸಂದೇಶ = (z_const ವರ್ಣ *)"invalid distance too far back";
                mode = BAD;
                ಭಙ್ಗ;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            ಕುರು {
                ROOM();
                copy = WSIZE - offset;
                ಯದಿ (copy < ಎಡ) {
                    from = put + copy;
                    copy = ಎಡ - copy;
                }
                ಅನ್ಯಥಾ {
                    from = put - offset;
                    copy = ಎಡ;
                }
                ಯದಿ (copy > length) copy = length;
                length -= copy;
                ಎಡ -= copy;
                ಕುರು {
                    *put++ = *from++;
                } ಯಾವತ್ (--copy);
            } ಯಾವತ್ (length != 0);
            ಭಙ್ಗ;

        ಪಕ್ಷ DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            ಯದಿ (ಎಡ < WSIZE) {
                ಯದಿ (out(out_desc, window, (unsigned)(WSIZE - ಎಡ)))
                    ret = Z_BUF_ERROR;
            }
            ಗಚ್ಛ inf_leave;

        ಪಕ್ಷ BAD:
            ret = Z_DATA_ERROR;
            ಗಚ್ಛ inf_leave;

        ಸ್ವಭಾವ:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            ಗಚ್ಛ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ ZEXPORT inflateBack9End(z_stream FAR *strm) {
    ಯದಿ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ಪ್ರತ್ಯಾಗಮ Z_OK;
}
