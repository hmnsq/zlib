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

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
صحيح ZEXPORT inflateBackInit_(z_streamp strm, صحيح windowBits,
                             unsigned حرف FAR *window, ثابت حرف *version,
                             صحيح stream_size) {
    بنية inflate_state FAR *state;

    إذا (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (صحيح)(حجم(z_stream)))
        أرجع Z_VERSION_ERROR;
    إذا (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        أرجع Z_STREAM_ERROR;
    strm->رسالة = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        أرجع Z_STREAM_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    إذا (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        أرجع Z_STREAM_ERROR;
#وإلا
        strm->zfree = zcfree;
#endif
    state = (بنية inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    أرجع Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    افعل { \
        put = strm->next_out; \
        يسار = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } طالما (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    افعل { \
        strm->next_out = put; \
        strm->avail_out = يسار; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } طالما (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    افعل { \
        hold = 0; \
        bits = 0; \
    } طالما (0)

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
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    افعل { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    افعل { \
        طالما (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    افعل { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } طالما (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    افعل { \
        إذا (يسار == 0) { \
            put = state->window; \
            يسار = state->wsize; \
            state->whave = يسار; \
            إذا (out(out_desc, put, يسار)) { \
                ret = Z_BUF_ERROR; \
                اذهب inf_leave; \
            } \
        } \
    } طالما (0)

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
صحيح ZEXPORT inflateBack(z_streamp strm, in_func in, فراغ FAR *in_desc,
                        out_func out, فراغ FAR *out_desc) {
    بنية inflate_state FAR *state;
    z_const unsigned حرف FAR *next;    /* next input */
    unsigned حرف FAR *put;     /* next output */
    unsigned have, يسار;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned حرف FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_STREAM_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->رسالة = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    يسار = state->wsize;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (state->mode) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (state->last) {
                BYTEBITS();
                state->mode = DONE;
                اكسر;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            بدل (BITS(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                اكسر;
            افتراضي:
                strm->رسالة = (z_const حرف *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->رسالة = (z_const حرف *)"invalid stored block lengths";
                state->mode = BAD;
                اكسر;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            طالما (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                إذا (copy > have) copy = have;
                إذا (copy > يسار) copy = يسار;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                يسار -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
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
#ifndef PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->رسالة = (z_const حرف *)
                    "too many length or distance symbols";
                state->mode = BAD;
                اكسر;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code ثابت FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->رسالة = (z_const حرف *)"invalid code lengths set";
                state->mode = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    إذا ((unsigned)(here.bits) <= bits) اكسر;
                    PULLBYTE();
                }
                إذا (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                وإلا {
                    إذا (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        إذا (state->have == 0) {
                            strm->رسالة = (z_const حرف *)
                                "invalid bit length repeat";
                            state->mode = BAD;
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
                        strm->رسالة = (z_const حرف *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->mode == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->رسالة = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code ثابت FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->رسالة = (z_const حرف *)"invalid literal/lengths set";
                state->mode = BAD;
                اكسر;
            }
            state->distcode = (code ثابت FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            إذا (ret) {
                strm->رسالة = (z_const حرف *)"invalid distances set";
                state->mode = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        حالة LEN:
            /* use inflate_fast() if we have enough input and output */
            إذا (have >= 6 && يسار >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                اكسر;
            }

            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                here = state->lencode[BITS(state->lenbits)];
                إذا ((unsigned)(here.bits) <= bits) اكسر;
                PULLBYTE();
            }
            إذا (here.op && (here.op & 0xf0) == 0) {
                last = here;
                لكل (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    إذا ((unsigned)(last.bits + here.bits) <= bits) اكسر;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            إذا (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned حرف)(state->length);
                يسار--;
                state->mode = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (here.op & 64) {
                strm->رسالة = (z_const حرف *)"invalid literal/length code";
                state->mode = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            لكل (;;) {
                here = state->distcode[BITS(state->distbits)];
                إذا ((unsigned)(here.bits) <= bits) اكسر;
                PULLBYTE();
            }
            إذا ((here.op & 0xf0) == 0) {
                last = here;
                لكل (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    إذا ((unsigned)(last.bits + here.bits) <= bits) اكسر;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            إذا (here.op & 64) {
                strm->رسالة = (z_const حرف *)"invalid distance code";
                state->mode = BAD;
                اكسر;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            إذا (state->offset > state->wsize - (state->whave < state->wsize ?
                                                يسار : 0)) {
                strm->رسالة = (z_const حرف *)"invalid distance too far back";
                state->mode = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            افعل {
                ROOM();
                copy = state->wsize - state->offset;
                إذا (copy < يسار) {
                    from = put + copy;
                    copy = يسار - copy;
                }
                وإلا {
                    from = put - state->offset;
                    copy = يسار;
                }
                إذا (copy > state->length) copy = state->length;
                state->length -= copy;
                يسار -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (state->length != 0);
            اكسر;

        حالة DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            اذهب inf_leave;

        حالة BAD:
            ret = Z_DATA_ERROR;
            اذهب inf_leave;

        افتراضي:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            اذهب inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    إذا (يسار < state->wsize) {
        إذا (out(out_desc, state->window, state->wsize - يسار) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    أرجع ret;
}

صحيح ZEXPORT inflateBackEnd(z_streamp strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
