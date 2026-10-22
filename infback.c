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

#அந்தர்பாவ "zutil.h"
#அந்தர்பாவ "inftrees.h"
#அந்தர்பாவ "inflate.h"
#அந்தர்பாவ "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
முழு ZEXPORT inflateBackInit_(z_streamp strm, முழு windowBits,
                             unsigned எழுத்து FAR *window, மாறிலி எழுத்து *version,
                             முழு stream_size) {
    கட்டமைப்பு inflate_state FAR *state;

    எனில் (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (முழு)(அளவு(z_stream)))
        திரும்பு Z_VERSION_ERROR;
    எனில் (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        திரும்பு Z_STREAM_ERROR;
    strm->செய்தி = Z_NULL;                 /* in case we return an error */
    எனில் (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        திரும்பு Z_STREAM_ERROR;
#இல்லையெனில்
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    எனில் (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        திரும்பு Z_STREAM_ERROR;
#இல்லையெனில்
        strm->zfree = zcfree;
#endif
    state = (கட்டமைப்பு inflate_state FAR *)ZALLOC(strm, 1,
                                               அளவு(கட்டமைப்பு inflate_state));
    எனில் (state == Z_NULL) திரும்பு Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (கட்டமைப்பு internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    திரும்பு Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    செய் { \
        put = strm->next_out; \
        இடது = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } போது (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    செய் { \
        strm->next_out = put; \
        strm->avail_out = இடது; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } போது (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    செய் { \
        hold = 0; \
        bits = 0; \
    } போது (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    செய் { \
        எனில் (have == 0) { \
            have = in(in_desc, &next); \
            எனில் (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                செல் inf_leave; \
            } \
        } \
    } போது (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    செய் { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } போது (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    செய் { \
        போது (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } போது (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    செய் { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } போது (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    செய் { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } போது (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    செய் { \
        எனில் (இடது == 0) { \
            put = state->window; \
            இடது = state->wsize; \
            state->whave = இடது; \
            எனில் (out(out_desc, put, இடது)) { \
                ret = Z_BUF_ERROR; \
                செல் inf_leave; \
            } \
        } \
    } போது (0)

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
முழு ZEXPORT inflateBack(z_streamp strm, in_func in, வெறுமை FAR *in_desc,
                        out_func out, வெறுமை FAR *out_desc) {
    கட்டமைப்பு inflate_state FAR *state;
    z_const unsigned எழுத்து FAR *next;    /* next input */
    unsigned எழுத்து FAR *put;     /* next output */
    unsigned have, இடது;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned எழுத்து FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    முழு ret;                    /* return code */
    நிலையான மாறிலி unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    எனில் (strm == Z_NULL || strm->state == Z_NULL)
        திரும்பு Z_STREAM_ERROR;
    state = (கட்டமைப்பு inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->செய்தி = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    இடது = state->wsize;

    /* Inflate until end of block marked as last */
    க்கு (;;)
        தேர்வு (state->mode) {
        நிலை TYPE:
            /* determine and dispatch block type */
            எனில் (state->last) {
                BYTEBITS();
                state->mode = DONE;
                நிறுத்து;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            தேர்வு (BITS(2)) {
            நிலை 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                நிறுத்து;
            நிலை 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                நிறுத்து;
            நிலை 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                நிறுத்து;
            இயல்பு:
                strm->செய்தி = (z_const எழுத்து *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            நிறுத்து;

        நிலை STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            எனில் ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->செய்தி = (z_const எழுத்து *)"invalid stored block lengths";
                state->mode = BAD;
                நிறுத்து;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            போது (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                எனில் (copy > have) copy = have;
                எனில் (copy > இடது) copy = இடது;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                இடது -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            நிறுத்து;

        நிலை TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            எனில் (state->nlen > 286 || state->ndist > 30) {
                strm->செய்தி = (z_const எழுத்து *)
                    "too many length or distance symbols";
                state->mode = BAD;
                நிறுத்து;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            போது (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            போது (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code மாறிலி FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            எனில் (ret) {
                strm->செய்தி = (z_const எழுத்து *)"invalid code lengths set";
                state->mode = BAD;
                நிறுத்து;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            போது (state->have < state->nlen + state->ndist) {
                க்கு (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    எனில் ((unsigned)(here.bits) <= bits) நிறுத்து;
                    PULLBYTE();
                }
                எனில் (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                இல்லையெனில் {
                    எனில் (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        எனில் (state->have == 0) {
                            strm->செய்தி = (z_const எழுத்து *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            நிறுத்து;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    இல்லையெனில் எனில் (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    இல்லையெனில் {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    எனில் (state->have + copy > state->nlen + state->ndist) {
                        strm->செய்தி = (z_const எழுத்து *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        நிறுத்து;
                    }
                    போது (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            எனில் (state->mode == BAD) நிறுத்து;

            /* check for end-of-block code (better have one) */
            எனில் (state->lens[256] == 0) {
                strm->செய்தி = (z_const எழுத்து *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                நிறுத்து;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code மாறிலி FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            எனில் (ret) {
                strm->செய்தி = (z_const எழுத்து *)"invalid literal/lengths set";
                state->mode = BAD;
                நிறுத்து;
            }
            state->distcode = (code மாறிலி FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            எனில் (ret) {
                strm->செய்தி = (z_const எழுத்து *)"invalid distances set";
                state->mode = BAD;
                நிறுத்து;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        நிலை LEN:
            /* use inflate_fast() if we have enough input and output */
            எனில் (have >= 6 && இடது >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                நிறுத்து;
            }

            /* get a literal, length, or end-of-block code */
            க்கு (;;) {
                here = state->lencode[BITS(state->lenbits)];
                எனில் ((unsigned)(here.bits) <= bits) நிறுத்து;
                PULLBYTE();
            }
            எனில் (here.op && (here.op & 0xf0) == 0) {
                last = here;
                க்கு (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    எனில் ((unsigned)(last.bits + here.bits) <= bits) நிறுத்து;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            எனில் (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned எழுத்து)(state->length);
                இடது--;
                state->mode = LEN;
                நிறுத்து;
            }

            /* process end of block */
            எனில் (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                நிறுத்து;
            }

            /* invalid code */
            எனில் (here.op & 64) {
                strm->செய்தி = (z_const எழுத்து *)"invalid literal/length code";
                state->mode = BAD;
                நிறுத்து;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            எனில் (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            க்கு (;;) {
                here = state->distcode[BITS(state->distbits)];
                எனில் ((unsigned)(here.bits) <= bits) நிறுத்து;
                PULLBYTE();
            }
            எனில் ((here.op & 0xf0) == 0) {
                last = here;
                க்கு (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    எனில் ((unsigned)(last.bits + here.bits) <= bits) நிறுத்து;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            எனில் (here.op & 64) {
                strm->செய்தி = (z_const எழுத்து *)"invalid distance code";
                state->mode = BAD;
                நிறுத்து;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            எனில் (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            எனில் (state->offset > state->wsize - (state->whave < state->wsize ?
                                                இடது : 0)) {
                strm->செய்தி = (z_const எழுத்து *)"invalid distance too far back";
                state->mode = BAD;
                நிறுத்து;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            செய் {
                ROOM();
                copy = state->wsize - state->offset;
                எனில் (copy < இடது) {
                    from = put + copy;
                    copy = இடது - copy;
                }
                இல்லையெனில் {
                    from = put - state->offset;
                    copy = இடது;
                }
                எனில் (copy > state->length) copy = state->length;
                state->length -= copy;
                இடது -= copy;
                செய் {
                    *put++ = *from++;
                } போது (--copy);
            } போது (state->length != 0);
            நிறுத்து;

        நிலை DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            செல் inf_leave;

        நிலை BAD:
            ret = Z_DATA_ERROR;
            செல் inf_leave;

        இயல்பு:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            செல் inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    எனில் (இடது < state->wsize) {
        எனில் (out(out_desc, state->window, state->wsize - இடது) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    திரும்பு ret;
}

முழு ZEXPORT inflateBackEnd(z_streamp strm) {
    எனில் (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        திரும்பு Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    திரும்பு Z_OK;
}
