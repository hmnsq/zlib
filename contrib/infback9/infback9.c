/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "infback9.h"
#ضمن "inftree9.h"
#ضمن "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
صحيح ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned حرف FAR *window,
                              ثابت حرف *version, صحيح stream_size) {
    بنية inflate_state FAR *state;

    إذا (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (صحيح)(حجم(z_stream)))
        إرجاع Z_VERSION_ERROR;
    إذا (strm == Z_NULL || window == Z_NULL)
        إرجاع Z_STREAM_ERROR;
    strm->رسالة = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    إذا (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (بنية inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(بنية inflate_state));
    إذا (state == Z_NULL) إرجاع Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    إرجاع Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#ضمن <stdio.h>

فراغ makefixed9(فراغ) {
    unsigned sym, bits, أدنى, size;
    code *next, *lenfix, *distfix;
    بنية inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    بينما (sym < 144) state.lens[sym++] = 8;
    بينما (sym < 256) state.lens[sym++] = 9;
    بينما (sym < 280) state.lens[sym++] = 7;
    بينما (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    بينما (sym < 32) state.lens[sym++] = 5;
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
    أدنى = 0;
    لكل (;;) {
        إذا ((أدنى % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[أدنى].op, lenfix[أدنى].bits,
               lenfix[أدنى].val);
        إذا (++أدنى == size) اكسر;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    أدنى = 0;
    لكل (;;) {
        إذا ((أدنى % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[أدنى].op, distfix[أدنى].bits,
               distfix[أدنى].val);
        إذا (++أدنى == size) اكسر;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    افعل { \
        hold = 0; \
        bits = 0; \
    } بينما (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    افعل { \
        إذا (have == 0) { \
            have = in(in_desc, &next); \
            إذا (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                اذهب inf_leave; \
            } \
        } \
    } بينما (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    افعل { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } بينما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    افعل { \
        بينما (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } بينما (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } بينما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    افعل { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } بينما (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    افعل { \
        إذا (يسار == 0) { \
            put = window; \
            يسار = WSIZE; \
            wrap = 1; \
            إذا (out(out_desc, put, (unsigned)يسار)) { \
                ret = Z_BUF_ERROR; \
                اذهب inf_leave; \
            } \
        } \
    } بينما (0)

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
صحيح ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, فراغ FAR *in_desc,
                         out_func out, فراغ FAR *out_desc) {
    بنية inflate_state FAR *state;
    z_const unsigned حرف FAR *next;    /* next input */
    unsigned حرف FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long يسار;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    صحيح lastblock;              /* true if processing last block */
    صحيح wrap;                   /* true if the window has wrapped */
    unsigned حرف FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned حرف FAR *from;    /* where to copy match bytes from */
    code ثابت FAR *lencode;    /* starting table for length/literal codes */
    code ثابت FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ضمن "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        إرجاع Z_STREAM_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->رسالة = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    يسار = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (mode) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (lastblock) {
                BYTEBITS();
                mode = DONE;
                اكسر;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            بدل (BITS(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                اكسر;
            حالة 3:
                strm->رسالة = (z_const حرف *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->رسالة = (z_const حرف *)"invalid stored block lengths";
                mode = BAD;
                اكسر;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            بينما (length != 0) {
                copy = length;
                PULL();
                ROOM();
                إذا (copy > have) copy = have;
                إذا (copy > يسار) copy = يسار;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                يسار -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            اكسر;

        حالة TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            إذا (state->nlen > 286) {
                strm->رسالة = (z_const حرف *)"too many length symbols";
                mode = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            بينما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            بينما (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code ثابت FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            إذا (ret) {
                strm->رسالة = (z_const حرف *)"invalid code lengths set";
                mode = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            بينما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    here = lencode[BITS(lenbits)];
                    إذا ((unsigned)(here.bits) <= bits) اكسر;
                    PULLBYTE();
                }
                إذا (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                وإلا {
                    إذا (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        إذا (state->have == 0) {
                            strm->رسالة = (z_const حرف *)"invalid bit length repeat";
                            mode = BAD;
                            اكسر;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->رسالة = (z_const حرف *)"invalid bit length repeat";
                        mode = BAD;
                        اكسر;
                    }
                    بينما (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            إذا (mode == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->رسالة = (z_const حرف *)"invalid code -- missing end-of-block";
                mode = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code ثابت FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            إذا (ret) {
                strm->رسالة = (z_const حرف *)"invalid literal/lengths set";
                mode = BAD;
                اكسر;
            }
            distcode = (code ثابت FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            إذا (ret) {
                strm->رسالة = (z_const حرف *)"invalid distances set";
                mode = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        حالة LEN:
            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                here = lencode[BITS(lenbits)];
                إذا ((unsigned)(here.bits) <= bits) اكسر;
                PULLBYTE();
            }
            إذا (here.op && (here.op & 0xf0) == 0) {
                last = here;
                لكل (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    إذا ((unsigned)(last.bits + here.bits) <= bits) اكسر;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            إذا (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned حرف)(length);
                يسار--;
                mode = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (here.op & 64) {
                strm->رسالة = (z_const حرف *)"invalid literal/length code";
                mode = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            إذا (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            لكل (;;) {
                here = distcode[BITS(distbits)];
                إذا ((unsigned)(here.bits) <= bits) اكسر;
                PULLBYTE();
            }
            إذا ((here.op & 0xf0) == 0) {
                last = here;
                لكل (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    إذا ((unsigned)(last.bits + here.bits) <= bits) اكسر;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            إذا (here.op & 64) {
                strm->رسالة = (z_const حرف *)"invalid distance code";
                mode = BAD;
                اكسر;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            إذا (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            إذا (offset > WSIZE - (wrap ? 0: يسار)) {
                strm->رسالة = (z_const حرف *)"invalid distance too far back";
                mode = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            افعل {
                ROOM();
                copy = WSIZE - offset;
                إذا (copy < يسار) {
                    from = put + copy;
                    copy = يسار - copy;
                }
                وإلا {
                    from = put - offset;
                    copy = يسار;
                }
                إذا (copy > length) copy = length;
                length -= copy;
                يسار -= copy;
                افعل {
                    *put++ = *from++;
                } بينما (--copy);
            } بينما (length != 0);
            اكسر;

        حالة DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            إذا (يسار < WSIZE) {
                إذا (out(out_desc, window, (unsigned)(WSIZE - يسار)))
                    ret = Z_BUF_ERROR;
            }
            اذهب inf_leave;

        حالة BAD:
            ret = Z_DATA_ERROR;
            اذهب inf_leave;

        افتراضي:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            اذهب inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    إرجاع ret;
}

صحيح ZEXPORT inflateBack9End(z_stream FAR *strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        إرجاع Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    إرجاع Z_OK;
}
