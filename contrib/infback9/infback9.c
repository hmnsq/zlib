/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "zutil.h"
#شامل "infback9.h"
#شامل "inftree9.h"
#شامل "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
صحیح ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned نویسه FAR *window,
                              ثابت نویسه *version, صحیح stream_size) {
    ساختار inflate_state FAR *state;

    اگر (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (صحیح)(اندازه(z_stream)))
        بازگشت Z_VERSION_ERROR;
    اگر (strm == Z_NULL || window == Z_NULL)
        بازگشت Z_STREAM_ERROR;
    strm->پیام = Z_NULL;                 /* in case we return an error */
    اگر (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    اگر (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (ساختار inflate_state FAR *)ZALLOC(strm, 1,
                                               اندازه(ساختار inflate_state));
    اگر (state == Z_NULL) بازگشت Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    بازگشت Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#شامل <stdio.h>

تهی makefixed9(تهی) {
    unsigned sym, bits, کم, size;
    code *next, *lenfix, *distfix;
    ساختار inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    وقتی (sym < 144) state.lens[sym++] = 8;
    وقتی (sym < 256) state.lens[sym++] = 9;
    وقتی (sym < 280) state.lens[sym++] = 7;
    وقتی (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    وقتی (sym < 32) state.lens[sym++] = 5;
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
    کم = 0;
    برای (;;) {
        اگر ((کم % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[کم].op, lenfix[کم].bits,
               lenfix[کم].val);
        اگر (++کم == size) بشکن;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    کم = 0;
    برای (;;) {
        اگر ((کم % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[کم].op, distfix[کم].bits,
               distfix[کم].val);
        اگر (++کم == size) بشکن;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    انجامبده { \
        hold = 0; \
        bits = 0; \
    } وقتی (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    انجامبده { \
        اگر (have == 0) { \
            have = in(in_desc, &next); \
            اگر (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                برو inf_leave; \
            } \
        } \
    } وقتی (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    انجامبده { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } وقتی (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    انجامبده { \
        وقتی (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } وقتی (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    انجامبده { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } وقتی (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    انجامبده { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } وقتی (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    انجامبده { \
        اگر (چپ == 0) { \
            put = window; \
            چپ = WSIZE; \
            wrap = 1; \
            اگر (out(out_desc, put, (unsigned)چپ)) { \
                ret = Z_BUF_ERROR; \
                برو inf_leave; \
            } \
        } \
    } وقتی (0)

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
صحیح ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, تهی FAR *in_desc,
                         out_func out, تهی FAR *out_desc) {
    ساختار inflate_state FAR *state;
    z_const unsigned نویسه FAR *next;    /* next input */
    unsigned نویسه FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long چپ;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    صحیح lastblock;              /* true if processing last block */
    صحیح wrap;                   /* true if the window has wrapped */
    unsigned نویسه FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned نویسه FAR *from;    /* where to copy match bytes from */
    code ثابت FAR *lencode;    /* starting table for length/literal codes */
    code ثابت FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    صحیح ret;                    /* return code */
    ایستا ثابت unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#شامل "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    اگر (strm == Z_NULL || strm->state == Z_NULL)
        بازگشت Z_STREAM_ERROR;
    state = (ساختار inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیام = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    چپ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    برای (;;)
        انتخاب (mode) {
        حالت TYPE:
            /* determine and dispatch block type */
            اگر (lastblock) {
                BYTEBITS();
                mode = DONE;
                بشکن;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            انتخاب (BITS(2)) {
            حالت 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                بشکن;
            حالت 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                بشکن;
            حالت 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                بشکن;
            حالت 3:
                strm->پیام = (z_const نویسه *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            بشکن;

        حالت STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            اگر ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیام = (z_const نویسه *)"invalid stored block lengths";
                mode = BAD;
                بشکن;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            وقتی (length != 0) {
                copy = length;
                PULL();
                ROOM();
                اگر (copy > have) copy = have;
                اگر (copy > چپ) copy = چپ;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                چپ -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            بشکن;

        حالت TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            اگر (state->nlen > 286) {
                strm->پیام = (z_const نویسه *)"too many length symbols";
                mode = BAD;
                بشکن;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            وقتی (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            وقتی (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code ثابت FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            اگر (ret) {
                strm->پیام = (z_const نویسه *)"invalid code lengths set";
                mode = BAD;
                بشکن;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            وقتی (state->have < state->nlen + state->ndist) {
                برای (;;) {
                    here = lencode[BITS(lenbits)];
                    اگر ((unsigned)(here.bits) <= bits) بشکن;
                    PULLBYTE();
                }
                اگر (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                وگرنه {
                    اگر (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        اگر (state->have == 0) {
                            strm->پیام = (z_const نویسه *)"invalid bit length repeat";
                            mode = BAD;
                            بشکن;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    وگرنه اگر (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    وگرنه {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    اگر (state->have + copy > state->nlen + state->ndist) {
                        strm->پیام = (z_const نویسه *)"invalid bit length repeat";
                        mode = BAD;
                        بشکن;
                    }
                    وقتی (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            اگر (mode == BAD) بشکن;

            /* check for end-of-block code (better have one) */
            اگر (state->lens[256] == 0) {
                strm->پیام = (z_const نویسه *)"invalid code -- missing end-of-block";
                mode = BAD;
                بشکن;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code ثابت FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            اگر (ret) {
                strm->پیام = (z_const نویسه *)"invalid literal/lengths set";
                mode = BAD;
                بشکن;
            }
            distcode = (code ثابت FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            اگر (ret) {
                strm->پیام = (z_const نویسه *)"invalid distances set";
                mode = BAD;
                بشکن;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        حالت LEN:
            /* get a literal, length, or end-of-block code */
            برای (;;) {
                here = lencode[BITS(lenbits)];
                اگر ((unsigned)(here.bits) <= bits) بشکن;
                PULLBYTE();
            }
            اگر (here.op && (here.op & 0xf0) == 0) {
                last = here;
                برای (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    اگر ((unsigned)(last.bits + here.bits) <= bits) بشکن;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            اگر (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned نویسه)(length);
                چپ--;
                mode = LEN;
                بشکن;
            }

            /* process end of block */
            اگر (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                بشکن;
            }

            /* invalid code */
            اگر (here.op & 64) {
                strm->پیام = (z_const نویسه *)"invalid literal/length code";
                mode = BAD;
                بشکن;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            اگر (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            برای (;;) {
                here = distcode[BITS(distbits)];
                اگر ((unsigned)(here.bits) <= bits) بشکن;
                PULLBYTE();
            }
            اگر ((here.op & 0xf0) == 0) {
                last = here;
                برای (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    اگر ((unsigned)(last.bits + here.bits) <= bits) بشکن;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            اگر (here.op & 64) {
                strm->پیام = (z_const نویسه *)"invalid distance code";
                mode = BAD;
                بشکن;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            اگر (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            اگر (offset > WSIZE - (wrap ? 0: چپ)) {
                strm->پیام = (z_const نویسه *)"invalid distance too far back";
                mode = BAD;
                بشکن;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            انجامبده {
                ROOM();
                copy = WSIZE - offset;
                اگر (copy < چپ) {
                    from = put + copy;
                    copy = چپ - copy;
                }
                وگرنه {
                    from = put - offset;
                    copy = چپ;
                }
                اگر (copy > length) copy = length;
                length -= copy;
                چپ -= copy;
                انجامبده {
                    *put++ = *from++;
                } وقتی (--copy);
            } وقتی (length != 0);
            بشکن;

        حالت DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            اگر (چپ < WSIZE) {
                اگر (out(out_desc, window, (unsigned)(WSIZE - چپ)))
                    ret = Z_BUF_ERROR;
            }
            برو inf_leave;

        حالت BAD:
            ret = Z_DATA_ERROR;
            برو inf_leave;

        پیشفرض:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            برو inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    بازگشت ret;
}

صحیح ZEXPORT inflateBack9End(z_stream FAR *strm) {
    اگر (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        بازگشت Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    بازگشت Z_OK;
}
