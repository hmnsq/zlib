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

#समावेश "zutil.h"
#समावेश "inftrees.h"
#समावेश "inflate.h"
#समावेश "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
पूर्णांक ZEXPORT inflateBackInit_(z_streamp strm, पूर्णांक windowBits,
                             अचिन्हित अक्षर FAR *window, स्थिर अक्षर *version,
                             पूर्णांक stream_आकार) {
    काष्ठा inflate_state FAR *state;

    अगर (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_आकार != (पूर्णांक)(माप(z_stream)))
        वापस Z_VERSION_ERROR;
    अगर (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        वापस Z_STREAM_ERROR;
    strm->संदेश = Z_NULL;                 /* in case we return an error */
    अगर (strm->zalloc == (alloc_func)0) {
#अगर_घोषित Z_SOLO
        वापस Z_STREAM_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#पूर्ण_अगर
    }
    अगर (strm->zfree == (free_func)0)
#अगर_घोषित Z_SOLO
        वापस Z_STREAM_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#पूर्ण_अगर
    state = (काष्ठा inflate_state FAR *)ZALLOC(strm, 1,
                                               माप(काष्ठा inflate_state));
    अगर (state == Z_NULL) वापस Z_MEM_ERROR;
    Tracev((मानक_त्रुटि, "inflate: allocated\n"));
    strm->state = (काष्ठा internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    वापस Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#घोषणा LOAD() \
    करो { \
        put = strm->अग्र_out; \
        बायाँ = strm->avail_out; \
        अग्र = strm->अग्र_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } जबतक (0)

/* Set state from registers for inflate_fast() */
#घोषणा RESTORE() \
    करो { \
        strm->अग्र_out = put; \
        strm->avail_out = बायाँ; \
        strm->अग्र_in = अग्र; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } जबतक (0)

/* Clear the input bit accumulator */
#घोषणा INITBITS() \
    करो { \
        hold = 0; \
        bits = 0; \
    } जबतक (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#घोषणा PULL() \
    करो { \
        अगर (have == 0) { \
            have = in(in_desc, &अग्र); \
            अगर (have == 0) { \
                अग्र = Z_NULL; \
                ret = Z_BUF_ERROR; \
                जाओ inf_leave; \
            } \
        } \
    } जबतक (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#घोषणा PULLBYTE() \
    करो { \
        PULL(); \
        have--; \
        hold += (अचिन्हित दीर्घ)(*अग्र++) << bits; \
        bits += 8; \
    } जबतक (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#घोषणा NEEDBITS(n) \
    करो { \
        जबतक (bits < (अचिन्हित)(n)) \
            PULLBYTE(); \
    } जबतक (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#घोषणा BITS(n) \
    ((अचिन्हित)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#घोषणा DROPBITS(n) \
    करो { \
        hold >>= (n); \
        bits -= (अचिन्हित)(n); \
    } जबतक (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#घोषणा BYTEBITS() \
    करो { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } जबतक (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#घोषणा ROOM() \
    करो { \
        अगर (बायाँ == 0) { \
            put = state->window; \
            बायाँ = state->wsize; \
            state->whave = बायाँ; \
            अगर (out(out_desc, put, बायाँ)) { \
                ret = Z_BUF_ERROR; \
                जाओ inf_leave; \
            } \
        } \
    } जबतक (0)

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
पूर्णांक ZEXPORT inflateBack(z_streamp strm, in_func in, व्योम FAR *in_desc,
                        out_func out, व्योम FAR *out_desc) {
    काष्ठा inflate_state FAR *state;
    z_const अचिन्हित अक्षर FAR *अग्र;    /* next input */
    अचिन्हित अक्षर FAR *put;     /* next output */
    अचिन्हित have, बायाँ;        /* available input and output */
    अचिन्हित दीर्घ hold;         /* bit buffer */
    अचिन्हित bits;              /* bits in bit buffer */
    अचिन्हित copy;              /* number of stored or match bytes to copy */
    अचिन्हित अक्षर FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    अचिन्हित len;               /* length to copy for repeats, bits to drop */
    पूर्णांक ret;                    /* return code */
    अटल स्थिर अचिन्हित लघु order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    अगर (strm == Z_NULL || strm->state == Z_NULL)
        वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->संदेश = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    अग्र = strm->अग्र_in;
    have = अग्र != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    बायाँ = state->wsize;

    /* Inflate until end of block marked as last */
    क्रम (;;)
        चयन (state->mode) {
        हाल TYPE:
            /* determine and dispatch block type */
            अगर (state->last) {
                BYTEBITS();
                state->mode = DONE;
                अवरोध;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            चयन (BITS(2)) {
            हाल 0:                             /* stored block */
                Tracev((मानक_त्रुटि, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                अवरोध;
            हाल 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((मानक_त्रुटि, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                अवरोध;
            हाल 2:                             /* dynamic block */
                Tracev((मानक_त्रुटि, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                अवरोध;
            शेष:
                strm->संदेश = (z_const अक्षर *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            अवरोध;

        हाल STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            अगर ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->संदेश = (z_const अक्षर *)"invalid stored block lengths";
                state->mode = BAD;
                अवरोध;
            }
            state->लंबाई = (अचिन्हित)hold & 0xffff;
            Tracev((मानक_त्रुटि, "inflate:       stored length %u\n",
                    state->लंबाई));
            INITBITS();

            /* copy stored block from input to output */
            जबतक (state->लंबाई != 0) {
                copy = state->लंबाई;
                PULL();
                ROOM();
                अगर (copy > have) copy = have;
                अगर (copy > बायाँ) copy = बायाँ;
                zmemcpy(put, अग्र, copy);
                have -= copy;
                अग्र += copy;
                बायाँ -= copy;
                put += copy;
                state->लंबाई -= copy;
            }
            Tracev((मानक_त्रुटि, "inflate:       stored end\n"));
            state->mode = TYPE;
            अवरोध;

        हाल TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#अगर_अघोषित PKZIP_BUG_WORKAROUND
            अगर (state->nlen > 286 || state->ndist > 30) {
                strm->संदेश = (z_const अक्षर *)
                    "too many length or distance symbols";
                state->mode = BAD;
                अवरोध;
            }
#पूर्ण_अगर
            Tracev((मानक_त्रुटि, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            जबतक (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (अचिन्हित लघु)BITS(3);
                DROPBITS(3);
            }
            जबतक (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->अग्र = state->codes;
            state->lencode = (code स्थिर FAR *)(state->अग्र);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->अग्र),
                                &(state->lenbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid code lengths set";
                state->mode = BAD;
                अवरोध;
            }
            Tracev((मानक_त्रुटि, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            जबतक (state->have < state->nlen + state->ndist) {
                क्रम (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                अगर (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अन्यथा {
                    अगर (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        अगर (state->have == 0) {
                            strm->संदेश = (z_const अक्षर *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            अवरोध;
                        }
                        len = (अचिन्हित)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    अन्यथा अगर (here.val == 17) {
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
                    अगर (state->have + copy > state->nlen + state->ndist) {
                        strm->संदेश = (z_const अक्षर *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        अवरोध;
                    }
                    जबतक (copy--)
                        state->lens[state->have++] = (अचिन्हित लघु)len;
                }
            }

            /* handle error breaks in while */
            अगर (state->mode == BAD) अवरोध;

            /* check for end-of-block code (better have one) */
            अगर (state->lens[256] == 0) {
                strm->संदेश = (z_const अक्षर *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                अवरोध;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->अग्र = state->codes;
            state->lencode = (code स्थिर FAR *)(state->अग्र);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->अग्र),
                                &(state->lenbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/lengths set";
                state->mode = BAD;
                अवरोध;
            }
            state->distcode = (code स्थिर FAR *)(state->अग्र);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->अग्र), &(state->distbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid distances set";
                state->mode = BAD;
                अवरोध;
            }
            Tracev((मानक_त्रुटि, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        हाल LEN:
            /* use inflate_fast() if we have enough input and output */
            अगर (have >= 6 && बायाँ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                अवरोध;
            }

            /* get a literal, length, or end-of-block code */
            क्रम (;;) {
                here = state->lencode[BITS(state->lenbits)];
                अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                PULLBYTE();
            }
            अगर (here.op && (here.op & 0xf0) == 0) {
                last = here;
                क्रम (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    अगर ((अचिन्हित)(last.bits + here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->लंबाई = (अचिन्हित)here.val;

            /* process literal */
            अगर (here.op == 0) {
                Tracevv((मानक_त्रुटि, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (अचिन्हित अक्षर)(state->लंबाई);
                बायाँ--;
                state->mode = LEN;
                अवरोध;
            }

            /* process end of block */
            अगर (here.op & 32) {
                Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
                state->mode = TYPE;
                अवरोध;
            }

            /* invalid code */
            अगर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/length code";
                state->mode = BAD;
                अवरोध;
            }

            /* length code -- get extra bits, if any */
            state->extra = (अचिन्हित)(here.op) & 15;
            अगर (state->extra != 0) {
                NEEDBITS(state->extra);
                state->लंबाई += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %u\n", state->लंबाई));

            /* get distance code */
            क्रम (;;) {
                here = state->distcode[BITS(state->distbits)];
                अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                PULLBYTE();
            }
            अगर ((here.op & 0xf0) == 0) {
                last = here;
                क्रम (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    अगर ((अचिन्हित)(last.bits + here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            अगर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid distance code";
                state->mode = BAD;
                अवरोध;
            }
            state->offset = (अचिन्हित)here.val;

            /* get distance extra bits, if any */
            state->extra = (अचिन्हित)(here.op) & 15;
            अगर (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            अगर (state->offset > state->wsize - (state->whave < state->wsize ?
                                                बायाँ : 0)) {
                strm->संदेश = (z_const अक्षर *)"invalid distance too far back";
                state->mode = BAD;
                अवरोध;
            }
            Tracevv((मानक_त्रुटि, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            करो {
                ROOM();
                copy = state->wsize - state->offset;
                अगर (copy < बायाँ) {
                    from = put + copy;
                    copy = बायाँ - copy;
                }
                अन्यथा {
                    from = put - state->offset;
                    copy = बायाँ;
                }
                अगर (copy > state->लंबाई) copy = state->लंबाई;
                state->लंबाई -= copy;
                बायाँ -= copy;
                करो {
                    *put++ = *from++;
                } जबतक (--copy);
            } जबतक (state->लंबाई != 0);
            अवरोध;

        हाल DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            जाओ inf_leave;

        हाल BAD:
            ret = Z_तथ्य_ERROR;
            जाओ inf_leave;

        शेष:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            जाओ inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    अगर (बायाँ < state->wsize) {
        अगर (out(out_desc, state->window, state->wsize - बायाँ) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->अग्र_in = अग्र;
    strm->avail_in = have;
    वापस ret;
}

पूर्णांक ZEXPORT inflateBackEnd(z_streamp strm) {
    अगर (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        वापस Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((मानक_त्रुटि, "inflate: end\n"));
    वापस Z_OK;
}
