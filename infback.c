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

#شامل "zutil.h"
#شامل "inftrees.h"
#شامل "inflate.h"
#شامل "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
صحیح ZEXPORT inflateBackInit_(z_streamp strm, صحیح windowBits,
                             unsigned نویسه FAR *window, ثابت نویسه *version,
                             صحیح stream_size) {
    ساختار inflate_state FAR *state;

    اگر (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (صحیح)(اندازه(z_stream)))
        بازگشت Z_VERSION_ERROR;
    اگر (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        بازگشت Z_STREAM_ERROR;
    strm->پیام = Z_NULL;                 /* in case we return an error */
    اگر (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        بازگشت Z_STREAM_ERROR;
#وگرنه
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    اگر (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        بازگشت Z_STREAM_ERROR;
#وگرنه
        strm->zfree = zcfree;
#endif
    state = (ساختار inflate_state FAR *)ZALLOC(strm, 1,
                                               اندازه(ساختار inflate_state));
    اگر (state == Z_NULL) بازگشت Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ساختار internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    بازگشت Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    انجامبده { \
        put = strm->next_out; \
        چپ = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } وقتی (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    انجامبده { \
        strm->next_out = put; \
        strm->avail_out = چپ; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } وقتی (0)

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

/* Return the low n bits of the bit accumulator (n < 16) */
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
            put = state->window; \
            چپ = state->wsize; \
            state->whave = چپ; \
            اگر (out(out_desc, put, چپ)) { \
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
صحیح ZEXPORT inflateBack(z_streamp strm, in_func in, تهی FAR *in_desc,
                        out_func out, تهی FAR *out_desc) {
    ساختار inflate_state FAR *state;
    z_const unsigned نویسه FAR *next;    /* next input */
    unsigned نویسه FAR *put;     /* next output */
    unsigned have, چپ;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned نویسه FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    صحیح ret;                    /* return code */
    ایستا ثابت unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    اگر (strm == Z_NULL || strm->state == Z_NULL)
        بازگشت Z_STREAM_ERROR;
    state = (ساختار inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیام = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    چپ = state->wsize;

    /* Inflate until end of block marked as last */
    برای (;;)
        انتخاب (state->mode) {
        حالت TYPE:
            /* determine and dispatch block type */
            اگر (state->last) {
                BYTEBITS();
                state->mode = DONE;
                بشکن;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            انتخاب (BITS(2)) {
            حالت 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                بشکن;
            حالت 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                بشکن;
            حالت 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                بشکن;
            پیشفرض:
                strm->پیام = (z_const نویسه *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            بشکن;

        حالت STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            اگر ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیام = (z_const نویسه *)"invalid stored block lengths";
                state->mode = BAD;
                بشکن;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            وقتی (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                اگر (copy > have) copy = have;
                اگر (copy > چپ) copy = چپ;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                چپ -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
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
#ifndef PKZIP_BUG_WORKAROUND
            اگر (state->nlen > 286 || state->ndist > 30) {
                strm->پیام = (z_const نویسه *)
                    "too many length or distance symbols";
                state->mode = BAD;
                بشکن;
            }
#endif
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
            state->lencode = (code ثابت FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            اگر (ret) {
                strm->پیام = (z_const نویسه *)"invalid code lengths set";
                state->mode = BAD;
                بشکن;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            وقتی (state->have < state->nlen + state->ndist) {
                برای (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    اگر ((unsigned)(here.bits) <= bits) بشکن;
                    PULLBYTE();
                }
                اگر (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                وگرنه {
                    اگر (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        اگر (state->have == 0) {
                            strm->پیام = (z_const نویسه *)
                                "invalid bit length repeat";
                            state->mode = BAD;
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
                        strm->پیام = (z_const نویسه *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        بشکن;
                    }
                    وقتی (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            اگر (state->mode == BAD) بشکن;

            /* check for end-of-block code (better have one) */
            اگر (state->lens[256] == 0) {
                strm->پیام = (z_const نویسه *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                بشکن;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code ثابت FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            اگر (ret) {
                strm->پیام = (z_const نویسه *)"invalid literal/lengths set";
                state->mode = BAD;
                بشکن;
            }
            state->distcode = (code ثابت FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            اگر (ret) {
                strm->پیام = (z_const نویسه *)"invalid distances set";
                state->mode = BAD;
                بشکن;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        حالت LEN:
            /* use inflate_fast() if we have enough input and output */
            اگر (have >= 6 && چپ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                بشکن;
            }

            /* get a literal, length, or end-of-block code */
            برای (;;) {
                here = state->lencode[BITS(state->lenbits)];
                اگر ((unsigned)(here.bits) <= bits) بشکن;
                PULLBYTE();
            }
            اگر (here.op && (here.op & 0xf0) == 0) {
                last = here;
                برای (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    اگر ((unsigned)(last.bits + here.bits) <= bits) بشکن;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            اگر (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned نویسه)(state->length);
                چپ--;
                state->mode = LEN;
                بشکن;
            }

            /* process end of block */
            اگر (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                بشکن;
            }

            /* invalid code */
            اگر (here.op & 64) {
                strm->پیام = (z_const نویسه *)"invalid literal/length code";
                state->mode = BAD;
                بشکن;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            اگر (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            برای (;;) {
                here = state->distcode[BITS(state->distbits)];
                اگر ((unsigned)(here.bits) <= bits) بشکن;
                PULLBYTE();
            }
            اگر ((here.op & 0xf0) == 0) {
                last = here;
                برای (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    اگر ((unsigned)(last.bits + here.bits) <= bits) بشکن;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            اگر (here.op & 64) {
                strm->پیام = (z_const نویسه *)"invalid distance code";
                state->mode = BAD;
                بشکن;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            اگر (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            اگر (state->offset > state->wsize - (state->whave < state->wsize ?
                                                چپ : 0)) {
                strm->پیام = (z_const نویسه *)"invalid distance too far back";
                state->mode = BAD;
                بشکن;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            انجامبده {
                ROOM();
                copy = state->wsize - state->offset;
                اگر (copy < چپ) {
                    from = put + copy;
                    copy = چپ - copy;
                }
                وگرنه {
                    from = put - state->offset;
                    copy = چپ;
                }
                اگر (copy > state->length) copy = state->length;
                state->length -= copy;
                چپ -= copy;
                انجامبده {
                    *put++ = *from++;
                } وقتی (--copy);
            } وقتی (state->length != 0);
            بشکن;

        حالت DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            برو inf_leave;

        حالت BAD:
            ret = Z_DATA_ERROR;
            برو inf_leave;

        پیشفرض:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            برو inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    اگر (چپ < state->wsize) {
        اگر (out(out_desc, state->window, state->wsize - چپ) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    بازگشت ret;
}

صحیح ZEXPORT inflateBackEnd(z_streamp strm) {
    اگر (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        بازگشت Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    بازگشت Z_OK;
}
