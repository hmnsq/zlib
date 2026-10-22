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
পূর্ণাঙ্ক ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned বর্ণ FAR *window,
                              ধ্রুব বর্ণ *version, পূর্ণাঙ্ক stream_size) {
    সংরচনা inflate_state FAR *state;

    যদি (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (পূর্ণাঙ্ক)(পরিমাণ(z_stream)))
        প্রত্যাগম Z_VERSION_ERROR;
    যদি (strm == Z_NULL || window == Z_NULL)
        প্রত্যাগম Z_STREAM_ERROR;
    strm->বার্তা = Z_NULL;                 /* in case we return an error */
    যদি (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    যদি (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (সংরচনা inflate_state FAR *)ZALLOC(strm, 1,
                                               পরিমাণ(সংরচনা inflate_state));
    যদি (state == Z_NULL) প্রত্যাগম Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    প্রত্যাগম Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#অন্তর্ভাব <stdio.h>

শূন্য makefixed9(শূন্য) {
    unsigned sym, bits, নিচের, size;
    code *next, *lenfix, *distfix;
    সংরচনা inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    যাবত্ (sym < 144) state.lens[sym++] = 8;
    যাবত্ (sym < 256) state.lens[sym++] = 9;
    যাবত্ (sym < 280) state.lens[sym++] = 7;
    যাবত্ (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    যাবত্ (sym < 32) state.lens[sym++] = 5;
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
    নিচের = 0;
    প্রতি (;;) {
        যদি ((নিচের % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[নিচের].op, lenfix[নিচের].bits,
               lenfix[নিচের].val);
        যদি (++নিচের == size) ভঙ্গ;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    নিচের = 0;
    প্রতি (;;) {
        যদি ((নিচের % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[নিচের].op, distfix[নিচের].bits,
               distfix[নিচের].val);
        যদি (++নিচের == size) ভঙ্গ;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    কুরু { \
        hold = 0; \
        bits = 0; \
    } যাবত্ (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    কুরু { \
        যদি (have == 0) { \
            have = in(in_desc, &next); \
            যদি (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                গচ্ছ inf_leave; \
            } \
        } \
    } যাবত্ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    কুরু { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } যাবত্ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    কুরু { \
        যাবত্ (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } যাবত্ (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    কুরু { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } যাবত্ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    কুরু { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } যাবত্ (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    কুরু { \
        যদি (বাম == 0) { \
            put = window; \
            বাম = WSIZE; \
            wrap = 1; \
            যদি (out(out_desc, put, (unsigned)বাম)) { \
                ret = Z_BUF_ERROR; \
                গচ্ছ inf_leave; \
            } \
        } \
    } যাবত্ (0)

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
পূর্ণাঙ্ক ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, শূন্য FAR *in_desc,
                         out_func out, শূন্য FAR *out_desc) {
    সংরচনা inflate_state FAR *state;
    z_const unsigned বর্ণ FAR *next;    /* next input */
    unsigned বর্ণ FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long বাম;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    পূর্ণাঙ্ক lastblock;              /* true if processing last block */
    পূর্ণাঙ্ক wrap;                   /* true if the window has wrapped */
    unsigned বর্ণ FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned বর্ণ FAR *from;    /* where to copy match bytes from */
    code ধ্রুব FAR *lencode;    /* starting table for length/literal codes */
    code ধ্রুব FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    পূর্ণাঙ্ক ret;                    /* return code */
    স্থির ধ্রুব unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#অন্তর্ভাব "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    যদি (strm == Z_NULL || strm->state == Z_NULL)
        প্রত্যাগম Z_STREAM_ERROR;
    state = (সংরচনা inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->বার্তা = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    বাম = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    প্রতি (;;)
        বিকল্প (mode) {
        পক্ষ TYPE:
            /* determine and dispatch block type */
            যদি (lastblock) {
                BYTEBITS();
                mode = DONE;
                ভঙ্গ;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            বিকল্প (BITS(2)) {
            পক্ষ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                ভঙ্গ;
            পক্ষ 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                ভঙ্গ;
            পক্ষ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                ভঙ্গ;
            পক্ষ 3:
                strm->বার্তা = (z_const বর্ণ *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            ভঙ্গ;

        পক্ষ STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            যদি ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->বার্তা = (z_const বর্ণ *)"invalid stored block lengths";
                mode = BAD;
                ভঙ্গ;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            যাবত্ (length != 0) {
                copy = length;
                PULL();
                ROOM();
                যদি (copy > have) copy = have;
                যদি (copy > বাম) copy = বাম;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                বাম -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            ভঙ্গ;

        পক্ষ TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            যদি (state->nlen > 286) {
                strm->বার্তা = (z_const বর্ণ *)"too many length symbols";
                mode = BAD;
                ভঙ্গ;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            যাবত্ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            যাবত্ (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code ধ্রুব FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            যদি (ret) {
                strm->বার্তা = (z_const বর্ণ *)"invalid code lengths set";
                mode = BAD;
                ভঙ্গ;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            যাবত্ (state->have < state->nlen + state->ndist) {
                প্রতি (;;) {
                    here = lencode[BITS(lenbits)];
                    যদি ((unsigned)(here.bits) <= bits) ভঙ্গ;
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
                            strm->বার্তা = (z_const বর্ণ *)"invalid bit length repeat";
                            mode = BAD;
                            ভঙ্গ;
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
                        strm->বার্তা = (z_const বর্ণ *)"invalid bit length repeat";
                        mode = BAD;
                        ভঙ্গ;
                    }
                    যাবত্ (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            যদি (mode == BAD) ভঙ্গ;

            /* check for end-of-block code (better have one) */
            যদি (state->lens[256] == 0) {
                strm->বার্তা = (z_const বর্ণ *)"invalid code -- missing end-of-block";
                mode = BAD;
                ভঙ্গ;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code ধ্রুব FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            যদি (ret) {
                strm->বার্তা = (z_const বর্ণ *)"invalid literal/lengths set";
                mode = BAD;
                ভঙ্গ;
            }
            distcode = (code ধ্রুব FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            যদি (ret) {
                strm->বার্তা = (z_const বর্ণ *)"invalid distances set";
                mode = BAD;
                ভঙ্গ;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        পক্ষ LEN:
            /* get a literal, length, or end-of-block code */
            প্রতি (;;) {
                here = lencode[BITS(lenbits)];
                যদি ((unsigned)(here.bits) <= bits) ভঙ্গ;
                PULLBYTE();
            }
            যদি (here.op && (here.op & 0xf0) == 0) {
                last = here;
                প্রতি (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    যদি ((unsigned)(last.bits + here.bits) <= bits) ভঙ্গ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            যদি (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned বর্ণ)(length);
                বাম--;
                mode = LEN;
                ভঙ্গ;
            }

            /* process end of block */
            যদি (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                ভঙ্গ;
            }

            /* invalid code */
            যদি (here.op & 64) {
                strm->বার্তা = (z_const বর্ণ *)"invalid literal/length code";
                mode = BAD;
                ভঙ্গ;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            যদি (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            প্রতি (;;) {
                here = distcode[BITS(distbits)];
                যদি ((unsigned)(here.bits) <= bits) ভঙ্গ;
                PULLBYTE();
            }
            যদি ((here.op & 0xf0) == 0) {
                last = here;
                প্রতি (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    যদি ((unsigned)(last.bits + here.bits) <= bits) ভঙ্গ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            যদি (here.op & 64) {
                strm->বার্তা = (z_const বর্ণ *)"invalid distance code";
                mode = BAD;
                ভঙ্গ;
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
                strm->বার্তা = (z_const বর্ণ *)"invalid distance too far back";
                mode = BAD;
                ভঙ্গ;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            কুরু {
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
                যদি (copy > length) copy = length;
                length -= copy;
                বাম -= copy;
                কুরু {
                    *put++ = *from++;
                } যাবত্ (--copy);
            } যাবত্ (length != 0);
            ভঙ্গ;

        পক্ষ DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            যদি (বাম < WSIZE) {
                যদি (out(out_desc, window, (unsigned)(WSIZE - বাম)))
                    ret = Z_BUF_ERROR;
            }
            গচ্ছ inf_leave;

        পক্ষ BAD:
            ret = Z_DATA_ERROR;
            গচ্ছ inf_leave;

        স্বভাব:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            গচ্ছ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    প্রত্যাগম ret;
}

পূর্ণাঙ্ক ZEXPORT inflateBack9End(z_stream FAR *strm) {
    যদি (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        প্রত্যাগম Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    প্রত্যাগম Z_OK;
}
