/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ଅନ୍ତର୍ଭାଵ "zutil.h"
#ଅନ୍ତର୍ଭାଵ "infback9.h"
#ଅନ୍ତର୍ଭାଵ "inftree9.h"
#ଅନ୍ତର୍ଭାଵ "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned ଅକ୍ଷର FAR *window,
                              ଅଚଳ ଅକ୍ଷର *version, ପୂର୍ଣ୍ଣସଂଖ୍ୟା stream_size) {
    ସଂରଚନା inflate_state FAR *state;

    ଯଦି (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ଆକାର(z_stream)))
        ଫେରନ୍ତୁ Z_VERSION_ERROR;
    ଯଦି (strm == Z_NULL || window == Z_NULL)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    strm->ସନ୍ଦେଶ = Z_NULL;                 /* in case we return an error */
    ଯଦି (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    ଯଦି (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (ସଂରଚନା inflate_state FAR *)ZALLOC(strm, 1,
                                               ଆକାର(ସଂରଚନା inflate_state));
    ଯଦି (state == Z_NULL) ଫେରନ୍ତୁ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    ଫେରନ୍ତୁ Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#ଅନ୍ତର୍ଭାଵ <stdio.h>

ଶୂନ୍ୟ makefixed9(ଶୂନ୍ୟ) {
    unsigned sym, bits, ତଳ, size;
    code *next, *lenfix, *distfix;
    ସଂରଚନା inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    ଯେତେବେଳେ (sym < 144) state.lens[sym++] = 8;
    ଯେତେବେଳେ (sym < 256) state.lens[sym++] = 9;
    ଯେତେବେଳେ (sym < 280) state.lens[sym++] = 7;
    ଯେତେବେଳେ (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    ଯେତେବେଳେ (sym < 32) state.lens[sym++] = 5;
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
    ତଳ = 0;
    ପାଇଁ (;;) {
        ଯଦି ((ତଳ % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[ତଳ].op, lenfix[ତଳ].bits,
               lenfix[ତଳ].val);
        ଯଦି (++ତଳ == size) ଭାଙ୍ଗ;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    ତଳ = 0;
    ପାଇଁ (;;) {
        ଯଦି ((ତଳ % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[ତଳ].op, distfix[ତଳ].bits,
               distfix[ତଳ].val);
        ଯଦି (++ତଳ == size) ଭାଙ୍ଗ;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    କର { \
        hold = 0; \
        bits = 0; \
    } ଯେତେବେଳେ (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    କର { \
        ଯଦି (have == 0) { \
            have = in(in_desc, &next); \
            ଯଦି (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                ଯାଅ inf_leave; \
            } \
        } \
    } ଯେତେବେଳେ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    କର { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } ଯେତେବେଳେ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    କର { \
        ଯେତେବେଳେ (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } ଯେତେବେଳେ (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    କର { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } ଯେତେବେଳେ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    କର { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } ଯେତେବେଳେ (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    କର { \
        ଯଦି (ବାମ == 0) { \
            put = window; \
            ବାମ = WSIZE; \
            wrap = 1; \
            ଯଦି (out(out_desc, put, (unsigned)ବାମ)) { \
                ret = Z_BUF_ERROR; \
                ଯାଅ inf_leave; \
            } \
        } \
    } ଯେତେବେଳେ (0)

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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, ଶୂନ୍ୟ FAR *in_desc,
                         out_func out, ଶୂନ୍ୟ FAR *out_desc) {
    ସଂରଚନା inflate_state FAR *state;
    z_const unsigned ଅକ୍ଷର FAR *next;    /* next input */
    unsigned ଅକ୍ଷର FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long ବାମ;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastblock;              /* true if processing last block */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrap;                   /* true if the window has wrapped */
    unsigned ଅକ୍ଷର FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned ଅକ୍ଷର FAR *from;    /* where to copy match bytes from */
    code ଅଚଳ FAR *lencode;    /* starting table for length/literal codes */
    code ଅଚଳ FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;                    /* return code */
    ସ୍ଥିର ଅଚଳ unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ଅନ୍ତର୍ଭାଵ "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    ଯଦି (strm == Z_NULL || strm->state == Z_NULL)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (ସଂରଚନା inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->ସନ୍ଦେଶ = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    ବାମ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    ପାଇଁ (;;)
        ବାଛ (mode) {
        ସ୍ଥିତି TYPE:
            /* determine and dispatch block type */
            ଯଦି (lastblock) {
                BYTEBITS();
                mode = DONE;
                ଭାଙ୍ଗ;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            ବାଛ (BITS(2)) {
            ସ୍ଥିତି 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 3:
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ଯଦି ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid stored block lengths";
                mode = BAD;
                ଭାଙ୍ଗ;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            ଯେତେବେଳେ (length != 0) {
                copy = length;
                PULL();
                ROOM();
                ଯଦି (copy > have) copy = have;
                ଯଦି (copy > ବାମ) copy = ବାମ;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                ବାମ -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            ଭାଙ୍ଗ;

        ସ୍ଥିତି TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            ଯଦି (state->nlen > 286) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"too many length symbols";
                mode = BAD;
                ଭାଙ୍ଗ;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            ଯେତେବେଳେ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            ଯେତେବେଳେ (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code ଅଚଳ FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            ଯଦି (ret) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid code lengths set";
                mode = BAD;
                ଭାଙ୍ଗ;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ଯେତେବେଳେ (state->have < state->nlen + state->ndist) {
                ପାଇଁ (;;) {
                    here = lencode[BITS(lenbits)];
                    ଯଦି ((unsigned)(here.bits) <= bits) ଭାଙ୍ଗ;
                    PULLBYTE();
                }
                ଯଦି (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                ଅନ୍ୟଥା {
                    ଯଦି (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        ଯଦି (state->have == 0) {
                            strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid bit length repeat";
                            mode = BAD;
                            ଭାଙ୍ଗ;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    ଅନ୍ୟଥା ଯଦି (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    ଅନ୍ୟଥା {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    ଯଦି (state->have + copy > state->nlen + state->ndist) {
                        strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid bit length repeat";
                        mode = BAD;
                        ଭାଙ୍ଗ;
                    }
                    ଯେତେବେଳେ (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            ଯଦି (mode == BAD) ଭାଙ୍ଗ;

            /* check for end-of-block code (better have one) */
            ଯଦି (state->lens[256] == 0) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid code -- missing end-of-block";
                mode = BAD;
                ଭାଙ୍ଗ;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code ଅଚଳ FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            ଯଦି (ret) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid literal/lengths set";
                mode = BAD;
                ଭାଙ୍ଗ;
            }
            distcode = (code ଅଚଳ FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            ଯଦି (ret) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid distances set";
                mode = BAD;
                ଭାଙ୍ଗ;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        ସ୍ଥିତି LEN:
            /* get a literal, length, or end-of-block code */
            ପାଇଁ (;;) {
                here = lencode[BITS(lenbits)];
                ଯଦି ((unsigned)(here.bits) <= bits) ଭାଙ୍ଗ;
                PULLBYTE();
            }
            ଯଦି (here.op && (here.op & 0xf0) == 0) {
                last = here;
                ପାଇଁ (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ଯଦି ((unsigned)(last.bits + here.bits) <= bits) ଭାଙ୍ଗ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            ଯଦି (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned ଅକ୍ଷର)(length);
                ବାମ--;
                mode = LEN;
                ଭାଙ୍ଗ;
            }

            /* process end of block */
            ଯଦି (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                ଭାଙ୍ଗ;
            }

            /* invalid code */
            ଯଦି (here.op & 64) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid literal/length code";
                mode = BAD;
                ଭାଙ୍ଗ;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            ଯଦି (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            ପାଇଁ (;;) {
                here = distcode[BITS(distbits)];
                ଯଦି ((unsigned)(here.bits) <= bits) ଭାଙ୍ଗ;
                PULLBYTE();
            }
            ଯଦି ((here.op & 0xf0) == 0) {
                last = here;
                ପାଇଁ (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ଯଦି ((unsigned)(last.bits + here.bits) <= bits) ଭାଙ୍ଗ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            ଯଦି (here.op & 64) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid distance code";
                mode = BAD;
                ଭାଙ୍ଗ;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            ଯଦି (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            ଯଦି (offset > WSIZE - (wrap ? 0: ବାମ)) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid distance too far back";
                mode = BAD;
                ଭାଙ୍ଗ;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            କର {
                ROOM();
                copy = WSIZE - offset;
                ଯଦି (copy < ବାମ) {
                    from = put + copy;
                    copy = ବାମ - copy;
                }
                ଅନ୍ୟଥା {
                    from = put - offset;
                    copy = ବାମ;
                }
                ଯଦି (copy > length) copy = length;
                length -= copy;
                ବାମ -= copy;
                କର {
                    *put++ = *from++;
                } ଯେତେବେଳେ (--copy);
            } ଯେତେବେଳେ (length != 0);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            ଯଦି (ବାମ < WSIZE) {
                ଯଦି (out(out_desc, window, (unsigned)(WSIZE - ବାମ)))
                    ret = Z_BUF_ERROR;
            }
            ଯାଅ inf_leave;

        ସ୍ଥିତି BAD:
            ret = Z_DATA_ERROR;
            ଯାଅ inf_leave;

        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            ଯାଅ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT inflateBack9End(z_stream FAR *strm) {
    ଯଦି (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ଫେରନ୍ତୁ Z_OK;
}
