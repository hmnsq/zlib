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

#अन्तर्भाव "zutil.h"
#अन्तर्भाव "inftrees.h"
#अन्तर्भाव "inflate.h"
#अन्तर्भाव "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
पूर्णाङ्क ZEXPORT inflateBackInit_(z_streamp strm, पूर्णाङ्क windowBits,
                             unsigned वर्ण FAR *window, ध्रुव वर्ण *version,
                             पूर्णाङ्क stream_size) {
    संरचना inflate_state FAR *state;

    सचे (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (पूर्णाङ्क)(परिमाण(z_stream)))
        पटिनिवत्त Z_VERSION_ERROR;
    सचे (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        पटिनिवत्त Z_STREAM_ERROR;
    strm->सन्देशः = Z_NULL;                 /* in case we return an error */
    सचे (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        पटिनिवत्त Z_STREAM_ERROR;
#अञ्ञथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    सचे (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        पटिनिवत्त Z_STREAM_ERROR;
#अञ्ञथा
        strm->zfree = zcfree;
#endif
    state = (संरचना inflate_state FAR *)ZALLOC(strm, 1,
                                               परिमाण(संरचना inflate_state));
    सचे (state == Z_NULL) पटिनिवत्त Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (संरचना internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    पटिनिवत्त Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    करोहि { \
        put = strm->next_out; \
        वामः = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } याव (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    करोहि { \
        strm->next_out = put; \
        strm->avail_out = वामः; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } याव (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    करोहि { \
        hold = 0; \
        bits = 0; \
    } याव (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    करोहि { \
        सचे (have == 0) { \
            have = in(in_desc, &next); \
            सचे (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                गच्छ inf_leave; \
            } \
        } \
    } याव (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    करोहि { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } याव (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    करोहि { \
        याव (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } याव (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    करोहि { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } याव (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    करोहि { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } याव (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    करोहि { \
        सचे (वामः == 0) { \
            put = state->window; \
            वामः = state->wsize; \
            state->whave = वामः; \
            सचे (out(out_desc, put, वामः)) { \
                ret = Z_BUF_ERROR; \
                गच्छ inf_leave; \
            } \
        } \
    } याव (0)

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
पूर्णाङ्क ZEXPORT inflateBack(z_streamp strm, in_func in, शून्य FAR *in_desc,
                        out_func out, शून्य FAR *out_desc) {
    संरचना inflate_state FAR *state;
    z_const unsigned वर्ण FAR *next;    /* next input */
    unsigned वर्ण FAR *put;     /* next output */
    unsigned have, वामः;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned वर्ण FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    पूर्णाङ्क ret;                    /* return code */
    स्थिर ध्रुव unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    सचे (strm == Z_NULL || strm->state == Z_NULL)
        पटिनिवत्त Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->सन्देशः = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    वामः = state->wsize;

    /* Inflate until end of block marked as last */
    अत्थाय (;;)
        विकल्प (state->mode) {
        पक्ष TYPE:
            /* determine and dispatch block type */
            सचे (state->last) {
                BYTEBITS();
                state->mode = DONE;
                भङ्ग;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            विकल्प (BITS(2)) {
            पक्ष 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                भङ्ग;
            पक्ष 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                भङ्ग;
            पक्ष 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                भङ्ग;
            स्वभाव:
                strm->सन्देशः = (z_const वर्ण *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            भङ्ग;

        पक्ष STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            सचे ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid stored block lengths";
                state->mode = BAD;
                भङ्ग;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            याव (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                सचे (copy > have) copy = have;
                सचे (copy > वामः) copy = वामः;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                वामः -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            भङ्ग;

        पक्ष TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            सचे (state->nlen > 286 || state->ndist > 30) {
                strm->सन्देशः = (z_const वर्ण *)
                    "too many length or distance symbols";
                state->mode = BAD;
                भङ्ग;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            याव (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            याव (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code ध्रुव FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            सचे (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid code lengths set";
                state->mode = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            याव (state->have < state->nlen + state->ndist) {
                अत्थाय (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    सचे ((unsigned)(here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                सचे (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अञ्ञथा {
                    सचे (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        सचे (state->have == 0) {
                            strm->सन्देशः = (z_const वर्ण *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            भङ्ग;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    अञ्ञथा सचे (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    अञ्ञथा {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    सचे (state->have + copy > state->nlen + state->ndist) {
                        strm->सन्देशः = (z_const वर्ण *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        भङ्ग;
                    }
                    याव (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            सचे (state->mode == BAD) भङ्ग;

            /* check for end-of-block code (better have one) */
            सचे (state->lens[256] == 0) {
                strm->सन्देशः = (z_const वर्ण *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                भङ्ग;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code ध्रुव FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            सचे (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/lengths set";
                state->mode = BAD;
                भङ्ग;
            }
            state->distcode = (code ध्रुव FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            सचे (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distances set";
                state->mode = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        पक्ष LEN:
            /* use inflate_fast() if we have enough input and output */
            सचे (have >= 6 && वामः >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                भङ्ग;
            }

            /* get a literal, length, or end-of-block code */
            अत्थाय (;;) {
                here = state->lencode[BITS(state->lenbits)];
                सचे ((unsigned)(here.bits) <= bits) भङ्ग;
                PULLBYTE();
            }
            सचे (here.op && (here.op & 0xf0) == 0) {
                last = here;
                अत्थाय (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    सचे ((unsigned)(last.bits + here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            सचे (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned वर्ण)(state->length);
                वामः--;
                state->mode = LEN;
                भङ्ग;
            }

            /* process end of block */
            सचे (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                भङ्ग;
            }

            /* invalid code */
            सचे (here.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/length code";
                state->mode = BAD;
                भङ्ग;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            सचे (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            अत्थाय (;;) {
                here = state->distcode[BITS(state->distbits)];
                सचे ((unsigned)(here.bits) <= bits) भङ्ग;
                PULLBYTE();
            }
            सचे ((here.op & 0xf0) == 0) {
                last = here;
                अत्थाय (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    सचे ((unsigned)(last.bits + here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            सचे (here.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance code";
                state->mode = BAD;
                भङ्ग;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            सचे (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            सचे (state->offset > state->wsize - (state->whave < state->wsize ?
                                                वामः : 0)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance too far back";
                state->mode = BAD;
                भङ्ग;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            करोहि {
                ROOM();
                copy = state->wsize - state->offset;
                सचे (copy < वामः) {
                    from = put + copy;
                    copy = वामः - copy;
                }
                अञ्ञथा {
                    from = put - state->offset;
                    copy = वामः;
                }
                सचे (copy > state->length) copy = state->length;
                state->length -= copy;
                वामः -= copy;
                करोहि {
                    *put++ = *from++;
                } याव (--copy);
            } याव (state->length != 0);
            भङ्ग;

        पक्ष DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            गच्छ inf_leave;

        पक्ष BAD:
            ret = Z_DATA_ERROR;
            गच्छ inf_leave;

        स्वभाव:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            गच्छ inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    सचे (वामः < state->wsize) {
        सचे (out(out_desc, state->window, state->wsize - वामः) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    पटिनिवत्त ret;
}

पूर्णाङ्क ZEXPORT inflateBackEnd(z_streamp strm) {
    सचे (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        पटिनिवत्त Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    पटिनिवत्त Z_OK;
}
