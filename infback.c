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

#ଅନ୍ତର୍ଭାଵ "zutil.h"
#ଅନ୍ତର୍ଭାଵ "inftrees.h"
#ଅନ୍ତର୍ଭାଵ "inflate.h"
#ଅନ୍ତର୍ଭାଵ "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT inflateBackInit_(z_streamp strm, ପୂର୍ଣ୍ଣସଂଖ୍ୟା windowBits,
                             unsigned ଅକ୍ଷର FAR *window, ଅଚଳ ଅକ୍ଷର *version,
                             ପୂର୍ଣ୍ଣସଂଖ୍ୟା stream_size) {
    ସଂରଚନା inflate_state FAR *state;

    ଯଦି (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ଆକାର(z_stream)))
        ଫେରନ୍ତୁ Z_VERSION_ERROR;
    ଯଦି (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    strm->ସନ୍ଦେଶ = Z_NULL;                 /* in case we return an error */
    ଯଦି (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
#ଅନ୍ୟଥା
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    ଯଦି (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
#ଅନ୍ୟଥା
        strm->zfree = zcfree;
#endif
    state = (ସଂରଚନା inflate_state FAR *)ZALLOC(strm, 1,
                                               ଆକାର(ସଂରଚନା inflate_state));
    ଯଦି (state == Z_NULL) ଫେରନ୍ତୁ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ସଂରଚନା internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    ଫେରନ୍ତୁ Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    କର { \
        put = strm->next_out; \
        ବାମ = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } ଯେତେବେଳେ (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    କର { \
        strm->next_out = put; \
        strm->avail_out = ବାମ; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } ଯେତେବେଳେ (0)

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

/* Return the low n bits of the bit accumulator (n < 16) */
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
            put = state->window; \
            ବାମ = state->wsize; \
            state->whave = ବାମ; \
            ଯଦି (out(out_desc, put, ବାମ)) { \
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT inflateBack(z_streamp strm, in_func in, ଶୂନ୍ୟ FAR *in_desc,
                        out_func out, ଶୂନ୍ୟ FAR *out_desc) {
    ସଂରଚନା inflate_state FAR *state;
    z_const unsigned ଅକ୍ଷର FAR *next;    /* next input */
    unsigned ଅକ୍ଷର FAR *put;     /* next output */
    unsigned have, ବାମ;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned ଅକ୍ଷର FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;                    /* return code */
    ସ୍ଥିର ଅଚଳ unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    ଯଦି (strm == Z_NULL || strm->state == Z_NULL)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (ସଂରଚନା inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->ସନ୍ଦେଶ = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    ବାମ = state->wsize;

    /* Inflate until end of block marked as last */
    ପାଇଁ (;;)
        ବାଛ (state->mode) {
        ସ୍ଥିତି TYPE:
            /* determine and dispatch block type */
            ଯଦି (state->last) {
                BYTEBITS();
                state->mode = DONE;
                ଭାଙ୍ଗ;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            ବାଛ (BITS(2)) {
            ସ୍ଥିତି 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ଯଦି ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid stored block lengths";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            ଯେତେବେଳେ (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                ଯଦି (copy > have) copy = have;
                ଯଦି (copy > ବାମ) copy = ବାମ;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                ବାମ -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
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
#ifndef PKZIP_BUG_WORKAROUND
            ଯଦି (state->nlen > 286 || state->ndist > 30) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)
                    "too many length or distance symbols";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }
#endif
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
            state->lencode = (code ଅଚଳ FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            ଯଦି (ret) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid code lengths set";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ଯେତେବେଳେ (state->have < state->nlen + state->ndist) {
                ପାଇଁ (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    ଯଦି ((unsigned)(here.bits) <= bits) ଭାଙ୍ଗ;
                    PULLBYTE();
                }
                ଯଦି (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                ଅନ୍ୟଥା {
                    ଯଦି (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        ଯଦି (state->have == 0) {
                            strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)
                                "invalid bit length repeat";
                            state->mode = BAD;
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
                        strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        ଭାଙ୍ଗ;
                    }
                    ଯେତେବେଳେ (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            ଯଦି (state->mode == BAD) ଭାଙ୍ଗ;

            /* check for end-of-block code (better have one) */
            ଯଦି (state->lens[256] == 0) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code ଅଚଳ FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            ଯଦି (ret) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid literal/lengths set";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }
            state->distcode = (code ଅଚଳ FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            ଯଦି (ret) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid distances set";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        ସ୍ଥିତି LEN:
            /* use inflate_fast() if we have enough input and output */
            ଯଦି (have >= 6 && ବାମ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                ଭାଙ୍ଗ;
            }

            /* get a literal, length, or end-of-block code */
            ପାଇଁ (;;) {
                here = state->lencode[BITS(state->lenbits)];
                ଯଦି ((unsigned)(here.bits) <= bits) ଭାଙ୍ଗ;
                PULLBYTE();
            }
            ଯଦି (here.op && (here.op & 0xf0) == 0) {
                last = here;
                ପାଇଁ (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ଯଦି ((unsigned)(last.bits + here.bits) <= bits) ଭାଙ୍ଗ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            ଯଦି (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned ଅକ୍ଷର)(state->length);
                ବାମ--;
                state->mode = LEN;
                ଭାଙ୍ଗ;
            }

            /* process end of block */
            ଯଦି (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                ଭାଙ୍ଗ;
            }

            /* invalid code */
            ଯଦି (here.op & 64) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid literal/length code";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            ଯଦି (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            ପାଇଁ (;;) {
                here = state->distcode[BITS(state->distbits)];
                ଯଦି ((unsigned)(here.bits) <= bits) ଭାଙ୍ଗ;
                PULLBYTE();
            }
            ଯଦି ((here.op & 0xf0) == 0) {
                last = here;
                ପାଇଁ (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    ଯଦି ((unsigned)(last.bits + here.bits) <= bits) ଭାଙ୍ଗ;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            ଯଦି (here.op & 64) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid distance code";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            ଯଦି (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            ଯଦି (state->offset > state->wsize - (state->whave < state->wsize ?
                                                ବାମ : 0)) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid distance too far back";
                state->mode = BAD;
                ଭାଙ୍ଗ;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            କର {
                ROOM();
                copy = state->wsize - state->offset;
                ଯଦି (copy < ବାମ) {
                    from = put + copy;
                    copy = ବାମ - copy;
                }
                ଅନ୍ୟଥା {
                    from = put - state->offset;
                    copy = ବାମ;
                }
                ଯଦି (copy > state->length) copy = state->length;
                state->length -= copy;
                ବାମ -= copy;
                କର {
                    *put++ = *from++;
                } ଯେତେବେଳେ (--copy);
            } ଯେତେବେଳେ (state->length != 0);
            ଭାଙ୍ଗ;

        ସ୍ଥିତି DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            ଯାଅ inf_leave;

        ସ୍ଥିତି BAD:
            ret = Z_DATA_ERROR;
            ଯାଅ inf_leave;

        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            ଯାଅ inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    ଯଦି (ବାମ < state->wsize) {
        ଯଦି (out(out_desc, state->window, state->wsize - ବାମ) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT inflateBackEnd(z_streamp strm) {
    ଯଦି (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ଫେରନ୍ତୁ Z_OK;
}
