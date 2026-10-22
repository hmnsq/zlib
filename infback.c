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
पूर्णांक ZEXPORT inflateBackInit_(z_streamp strm, पूर्णांक windowBits,
                             अचिन्हित अक्षर FAR *window, स्थिरांक अक्षर *version,
                             पूर्णांक stream_size) {
    संरचना inflate_state FAR *state;

    जर (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (पूर्णांक)(आकार(z_stream)))
        परत Z_VERSION_ERROR;
    जर (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        परत Z_STREAM_ERROR;
    strm->संदेश = Z_NULL;                 /* in case we return an error */
    जर (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        परत Z_STREAM_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    जर (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        परत Z_STREAM_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#endif
    state = (संरचना inflate_state FAR *)ZALLOC(strm, 1,
                                               आकार(संरचना inflate_state));
    जर (state == Z_NULL) परत Z_MEM_ERROR;
    Tracev((मानक_त्रुटि, "inflate: allocated\n"));
    strm->state = (संरचना internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    परत Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    करा { \
        put = strm->next_out; \
        डावा = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } पर्यंत (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    करा { \
        strm->next_out = put; \
        strm->avail_out = डावा; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } पर्यंत (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    करा { \
        hold = 0; \
        bits = 0; \
    } पर्यंत (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    करा { \
        जर (have == 0) { \
            have = in(in_desc, &next); \
            जर (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                जा inf_leave; \
            } \
        } \
    } पर्यंत (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    करा { \
        PULL(); \
        have--; \
        hold += (अचिन्हित दीर्घ)(*next++) << bits; \
        bits += 8; \
    } पर्यंत (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    करा { \
        पर्यंत (bits < (अचिन्हित)(n)) \
            PULLBYTE(); \
    } पर्यंत (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((अचिन्हित)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    करा { \
        hold >>= (n); \
        bits -= (अचिन्हित)(n); \
    } पर्यंत (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    करा { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } पर्यंत (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    करा { \
        जर (डावा == 0) { \
            put = state->window; \
            डावा = state->wsize; \
            state->whave = डावा; \
            जर (out(out_desc, put, डावा)) { \
                ret = Z_BUF_ERROR; \
                जा inf_leave; \
            } \
        } \
    } पर्यंत (0)

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
पूर्णांक ZEXPORT inflateBack(z_streamp strm, in_func in, शून्य FAR *in_desc,
                        out_func out, शून्य FAR *out_desc) {
    संरचना inflate_state FAR *state;
    z_const अचिन्हित अक्षर FAR *next;    /* next input */
    अचिन्हित अक्षर FAR *put;     /* next output */
    अचिन्हित have, डावा;        /* available input and output */
    अचिन्हित दीर्घ hold;         /* bit buffer */
    अचिन्हित bits;              /* bits in bit buffer */
    अचिन्हित copy;              /* number of stored or match bytes to copy */
    अचिन्हित अक्षर FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    अचिन्हित len;               /* length to copy for repeats, bits to drop */
    पूर्णांक ret;                    /* return code */
    स्थिर स्थिरांक अचिन्हित लघु order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    जर (strm == Z_NULL || strm->state == Z_NULL)
        परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->संदेश = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    डावा = state->wsize;

    /* Inflate until end of block marked as last */
    साठी (;;)
        निवड (state->mode) {
        स्थिती TYPE:
            /* determine and dispatch block type */
            जर (state->last) {
                BYTEBITS();
                state->mode = DONE;
                थांबा;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            निवड (BITS(2)) {
            स्थिती 0:                             /* stored block */
                Tracev((मानक_त्रुटि, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                थांबा;
            स्थिती 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((मानक_त्रुटि, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                थांबा;
            स्थिती 2:                             /* dynamic block */
                Tracev((मानक_त्रुटि, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                थांबा;
            पूर्वनिर्धारित:
                strm->संदेश = (z_const अक्षर *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            थांबा;

        स्थिती STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            जर ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->संदेश = (z_const अक्षर *)"invalid stored block lengths";
                state->mode = BAD;
                थांबा;
            }
            state->length = (अचिन्हित)hold & 0xffff;
            Tracev((मानक_त्रुटि, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            पर्यंत (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                जर (copy > have) copy = have;
                जर (copy > डावा) copy = डावा;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                डावा -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((मानक_त्रुटि, "inflate:       stored end\n"));
            state->mode = TYPE;
            थांबा;

        स्थिती TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            जर (state->nlen > 286 || state->ndist > 30) {
                strm->संदेश = (z_const अक्षर *)
                    "too many length or distance symbols";
                state->mode = BAD;
                थांबा;
            }
#endif
            Tracev((मानक_त्रुटि, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            पर्यंत (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (अचिन्हित लघु)BITS(3);
                DROPBITS(3);
            }
            पर्यंत (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code स्थिरांक FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid code lengths set";
                state->mode = BAD;
                थांबा;
            }
            Tracev((मानक_त्रुटि, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            पर्यंत (state->have < state->nlen + state->ndist) {
                साठी (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                जर (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अन्यथा {
                    जर (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        जर (state->have == 0) {
                            strm->संदेश = (z_const अक्षर *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            थांबा;
                        }
                        len = (अचिन्हित)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    अन्यथा जर (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    अन्यथा {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    जर (state->have + copy > state->nlen + state->ndist) {
                        strm->संदेश = (z_const अक्षर *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        थांबा;
                    }
                    पर्यंत (copy--)
                        state->lens[state->have++] = (अचिन्हित लघु)len;
                }
            }

            /* handle error breaks in while */
            जर (state->mode == BAD) थांबा;

            /* check for end-of-block code (better have one) */
            जर (state->lens[256] == 0) {
                strm->संदेश = (z_const अक्षर *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                थांबा;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code स्थिरांक FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/lengths set";
                state->mode = BAD;
                थांबा;
            }
            state->distcode = (code स्थिरांक FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid distances set";
                state->mode = BAD;
                थांबा;
            }
            Tracev((मानक_त्रुटि, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        स्थिती LEN:
            /* use inflate_fast() if we have enough input and output */
            जर (have >= 6 && डावा >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                थांबा;
            }

            /* get a literal, length, or end-of-block code */
            साठी (;;) {
                here = state->lencode[BITS(state->lenbits)];
                जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                PULLBYTE();
            }
            जर (here.op && (here.op & 0xf0) == 0) {
                last = here;
                साठी (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    जर ((अचिन्हित)(last.bits + here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (अचिन्हित)here.val;

            /* process literal */
            जर (here.op == 0) {
                Tracevv((मानक_त्रुटि, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (अचिन्हित अक्षर)(state->length);
                डावा--;
                state->mode = LEN;
                थांबा;
            }

            /* process end of block */
            जर (here.op & 32) {
                Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
                state->mode = TYPE;
                थांबा;
            }

            /* invalid code */
            जर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/length code";
                state->mode = BAD;
                थांबा;
            }

            /* length code -- get extra bits, if any */
            state->extra = (अचिन्हित)(here.op) & 15;
            जर (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %u\n", state->length));

            /* get distance code */
            साठी (;;) {
                here = state->distcode[BITS(state->distbits)];
                जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                PULLBYTE();
            }
            जर ((here.op & 0xf0) == 0) {
                last = here;
                साठी (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    जर ((अचिन्हित)(last.bits + here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            जर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid distance code";
                state->mode = BAD;
                थांबा;
            }
            state->offset = (अचिन्हित)here.val;

            /* get distance extra bits, if any */
            state->extra = (अचिन्हित)(here.op) & 15;
            जर (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            जर (state->offset > state->wsize - (state->whave < state->wsize ?
                                                डावा : 0)) {
                strm->संदेश = (z_const अक्षर *)"invalid distance too far back";
                state->mode = BAD;
                थांबा;
            }
            Tracevv((मानक_त्रुटि, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            करा {
                ROOM();
                copy = state->wsize - state->offset;
                जर (copy < डावा) {
                    from = put + copy;
                    copy = डावा - copy;
                }
                अन्यथा {
                    from = put - state->offset;
                    copy = डावा;
                }
                जर (copy > state->length) copy = state->length;
                state->length -= copy;
                डावा -= copy;
                करा {
                    *put++ = *from++;
                } पर्यंत (--copy);
            } पर्यंत (state->length != 0);
            थांबा;

        स्थिती DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            जा inf_leave;

        स्थिती BAD:
            ret = Z_DATA_ERROR;
            जा inf_leave;

        पूर्वनिर्धारित:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            जा inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    जर (डावा < state->wsize) {
        जर (out(out_desc, state->window, state->wsize - डावा) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    परत ret;
}

पूर्णांक ZEXPORT inflateBackEnd(z_streamp strm) {
    जर (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        परत Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((मानक_त्रुटि, "inflate: end\n"));
    परत Z_OK;
}
