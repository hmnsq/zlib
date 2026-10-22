/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#অন্তর্ভাব "zutil.h"
#অন্তর্ভাব "infback9.h"
#অন্তর্ভাব "inftree9.h"
#অন্তর্ভাব "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
পূর্ণসংখ্যা ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned অক্ষর FAR *window,
                              ধ্রুবক অক্ষর *version, পূর্ণসংখ্যা stream_আকার) {
    কাঠামো inflate_state FAR *state;

    যদি (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_আকার != (পূর্ণসংখ্যা)(আকার(z_stream)))
        ফেরত Z_VERSION_ERROR;
    যদি (strm == Z_NULL || window == Z_NULL)
        ফেরত Z_STREAM_ERROR;
    strm->বার্তা = Z_NULL;                 /* in case we return an error */
    যদি (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    যদি (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (কাঠামো inflate_state FAR *)ZALLOC(strm, 1,
                                               আকার(কাঠামো inflate_state));
    যদি (state == Z_NULL) ফেরত Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    ফেরত Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#অন্তর্ভাব <stdio.h>

শূন্য makefixed9(শূন্য) {
    unsigned sym, bits, নিচের, আকার_2;
    code *পরবর্তী, *lenfix, *distfix;
    কাঠামো inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    যখন (sym < 144) state.lens[sym++] = 8;
    যখন (sym < 256) state.lens[sym++] = 9;
    যখন (sym < 280) state.lens[sym++] = 7;
    যখন (sym < 288) state.lens[sym++] = 8;
    পরবর্তী = fixed;
    lenfix = পরবর্তী;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(পরবর্তী), &(bits), state.work);

    /* distance table */
    sym = 0;
    যখন (sym < 32) state.lens[sym++] = 5;
    distfix = পরবর্তী;
    bits = 5;
    inflate_table9(DISTS, state.lens, 32, &(পরবর্তী), &(bits), state.work);

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
    আকার_2 = 1U << 9;
    printf("    static const code lenfix[%u] = {", আকার_2);
    নিচের = 0;
    জন্য (;;) {
        যদি ((নিচের % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[নিচের].op, lenfix[নিচের].bits,
               lenfix[নিচের].val);
        যদি (++নিচের == আকার_2) বিরতি;
        putchar(',');
    }
    puts("\n    };");
    আকার_2 = 1U << 5;
    printf("\n    static const code distfix[%u] = {", আকার_2);
    নিচের = 0;
    জন্য (;;) {
        যদি ((নিচের % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[নিচের].op, distfix[নিচের].bits,
               distfix[নিচের].val);
        যদি (++নিচের == আকার_2) বিরতি;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    করো { \
        hold = 0; \
        bits = 0; \
    } যখন (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    করো { \
        যদি (have == 0) { \
            have = in(in_desc, &পরবর্তী); \
            যদি (have == 0) { \
                পরবর্তী = Z_NULL; \
                ret = Z_BUF_ERROR; \
                যাও inf_leave; \
            } \
        } \
    } যখন (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    করো { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*পরবর্তী++) << bits; \
        bits += 8; \
    } যখন (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    করো { \
        যখন (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } যখন (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    করো { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } যখন (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    করো { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } যখন (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    করো { \
        যদি (বাম == 0) { \
            put = window; \
            বাম = WSIZE; \
            wrap = 1; \
            যদি (out(out_desc, put, (unsigned)বাম)) { \
                ret = Z_BUF_ERROR; \
                যাও inf_leave; \
            } \
        } \
    } যখন (0)

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
পূর্ণসংখ্যা ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, শূন্য FAR *in_desc,
                         out_func out, শূন্য FAR *out_desc) {
    কাঠামো inflate_state FAR *state;
    z_const unsigned অক্ষর FAR *পরবর্তী;    /* next input */
    unsigned অক্ষর FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long বাম;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    পূর্ণসংখ্যা lastblock;              /* true if processing last block */
    পূর্ণসংখ্যা wrap;                   /* true if the window has wrapped */
    unsigned অক্ষর FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long দৈর্ঘ্য;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned অক্ষর FAR *from;    /* where to copy match bytes from */
    code ধ্রুবক FAR *lencode;    /* starting table for length/literal codes */
    code ধ্রুবক FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    পূর্ণসংখ্যা ret;                    /* return code */
    স্থির ধ্রুবক unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#অন্তর্ভাব "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    যদি (strm == Z_NULL || strm->state == Z_NULL)
        ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->বার্তা = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    পরবর্তী = strm->পরবর্তী_in;
    have = পরবর্তী != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    বাম = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    জন্য (;;)
        নির্বাচন (mode) {
        ক্ষেত্র TYPE:
            /* determine and dispatch block type */
            যদি (lastblock) {
                BYTEBITS();
                mode = DONE;
                বিরতি;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            নির্বাচন (BITS(2)) {
            ক্ষেত্র 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                বিরতি;
            ক্ষেত্র 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                বিরতি;
            ক্ষেত্র 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                বিরতি;
            ক্ষেত্র 3:
                strm->বার্তা = (z_const অক্ষর *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            বিরতি;

        ক্ষেত্র STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            যদি ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->বার্তা = (z_const অক্ষর *)"invalid stored block lengths";
                mode = BAD;
                বিরতি;
            }
            দৈর্ঘ্য = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    দৈর্ঘ্য));
            INITBITS();

            /* copy stored block from input to output */
            যখন (দৈর্ঘ্য != 0) {
                copy = দৈর্ঘ্য;
                PULL();
                ROOM();
                যদি (copy > have) copy = have;
                যদি (copy > বাম) copy = বাম;
                zmemcpy(put, পরবর্তী, copy);
                have -= copy;
                পরবর্তী += copy;
                বাম -= copy;
                put += copy;
                দৈর্ঘ্য -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            বিরতি;

        ক্ষেত্র TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            যদি (state->nlen > 286) {
                strm->বার্তা = (z_const অক্ষর *)"too many length symbols";
                mode = BAD;
                বিরতি;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            যখন (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            যখন (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->পরবর্তী = state->codes;
            lencode = (code ধ্রুবক FAR *)(state->পরবর্তী);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->পরবর্তী),
                                &(lenbits), state->work);
            যদি (ret) {
                strm->বার্তা = (z_const অক্ষর *)"invalid code lengths set";
                mode = BAD;
                বিরতি;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            যখন (state->have < state->nlen + state->ndist) {
                জন্য (;;) {
                    here = lencode[BITS(lenbits)];
                    যদি ((unsigned)(here.bits) <= bits) বিরতি;
                    PULLBYTE();
                }
                যদি (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                অন্যথা {
                    যদি (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        যদি (state->have == 0) {
                            strm->বার্তা = (z_const অক্ষর *)"invalid bit length repeat";
                            mode = BAD;
                            বিরতি;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    অন্যথা যদি (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    অন্যথা {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    যদি (state->have + copy > state->nlen + state->ndist) {
                        strm->বার্তা = (z_const অক্ষর *)"invalid bit length repeat";
                        mode = BAD;
                        বিরতি;
                    }
                    যখন (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            যদি (mode == BAD) বিরতি;

            /* check for end-of-block code (better have one) */
            যদি (state->lens[256] == 0) {
                strm->বার্তা = (z_const অক্ষর *)"invalid code -- missing end-of-block";
                mode = BAD;
                বিরতি;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->পরবর্তী = state->codes;
            lencode = (code ধ্রুবক FAR *)(state->পরবর্তী);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->পরবর্তী), &(lenbits), state->work);
            যদি (ret) {
                strm->বার্তা = (z_const অক্ষর *)"invalid literal/lengths set";
                mode = BAD;
                বিরতি;
            }
            distcode = (code ধ্রুবক FAR *)(state->পরবর্তী);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->পরবর্তী), &(distbits),
                            state->work);
            যদি (ret) {
                strm->বার্তা = (z_const অক্ষর *)"invalid distances set";
                mode = BAD;
                বিরতি;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        ক্ষেত্র LEN:
            /* get a literal, length, or end-of-block code */
            জন্য (;;) {
                here = lencode[BITS(lenbits)];
                যদি ((unsigned)(here.bits) <= bits) বিরতি;
                PULLBYTE();
            }
            যদি (here.op && (here.op & 0xf0) == 0) {
                last = here;
                জন্য (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    যদি ((unsigned)(last.bits + here.bits) <= bits) বিরতি;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            দৈর্ঘ্য = (unsigned)here.val;

            /* process literal */
            যদি (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned অক্ষর)(দৈর্ঘ্য);
                বাম--;
                mode = LEN;
                বিরতি;
            }

            /* process end of block */
            যদি (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                বিরতি;
            }

            /* invalid code */
            যদি (here.op & 64) {
                strm->বার্তা = (z_const অক্ষর *)"invalid literal/length code";
                mode = BAD;
                বিরতি;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            যদি (extra != 0) {
                NEEDBITS(extra);
                দৈর্ঘ্য += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", দৈর্ঘ্য));

            /* get distance code */
            জন্য (;;) {
                here = distcode[BITS(distbits)];
                যদি ((unsigned)(here.bits) <= bits) বিরতি;
                PULLBYTE();
            }
            যদি ((here.op & 0xf0) == 0) {
                last = here;
                জন্য (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    যদি ((unsigned)(last.bits + here.bits) <= bits) বিরতি;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            যদি (here.op & 64) {
                strm->বার্তা = (z_const অক্ষর *)"invalid distance code";
                mode = BAD;
                বিরতি;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            যদি (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            যদি (offset > WSIZE - (wrap ? 0: বাম)) {
                strm->বার্তা = (z_const অক্ষর *)"invalid distance too far back";
                mode = BAD;
                বিরতি;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            করো {
                ROOM();
                copy = WSIZE - offset;
                যদি (copy < বাম) {
                    from = put + copy;
                    copy = বাম - copy;
                }
                অন্যথা {
                    from = put - offset;
                    copy = বাম;
                }
                যদি (copy > দৈর্ঘ্য) copy = দৈর্ঘ্য;
                দৈর্ঘ্য -= copy;
                বাম -= copy;
                করো {
                    *put++ = *from++;
                } যখন (--copy);
            } যখন (দৈর্ঘ্য != 0);
            বিরতি;

        ক্ষেত্র DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            যদি (বাম < WSIZE) {
                যদি (out(out_desc, window, (unsigned)(WSIZE - বাম)))
                    ret = Z_BUF_ERROR;
            }
            যাও inf_leave;

        ক্ষেত্র BAD:
            ret = Z_তথ্য_ERROR;
            যাও inf_leave;

        পূর্বনির্ধারিত:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            যাও inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->পরবর্তী_in = পরবর্তী;
    strm->avail_in = have;
    ফেরত ret;
}

পূর্ণসংখ্যা ZEXPORT inflateBack9End(z_stream FAR *strm) {
    যদি (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ফেরত Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ফেরত Z_OK;
}
