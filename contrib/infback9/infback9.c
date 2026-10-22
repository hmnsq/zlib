/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#समावेश "zutil.h"
#समावेश "infback9.h"
#समावेश "inftree9.h"
#समावेश "inflate9.h"

#घोषणा WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
पूर्णांक ZEXPORT inflateBack9Init_(z_stream FAR *strm, अचिन्हित अक्षर FAR *window,
                              स्थिर अक्षर *version, पूर्णांक stream_आकार) {
    काष्ठा inflate_state FAR *state;

    अगर (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_आकार != (पूर्णांक)(माप(z_stream)))
        वापस Z_VERSION_ERROR;
    अगर (strm == Z_NULL || window == Z_NULL)
        वापस Z_STREAM_ERROR;
    strm->संदेश = Z_NULL;                 /* in case we return an error */
    अगर (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    अगर (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (काष्ठा inflate_state FAR *)ZALLOC(strm, 1,
                                               माप(काष्ठा inflate_state));
    अगर (state == Z_NULL) वापस Z_MEM_ERROR;
    Tracev((मानक_त्रुटि, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    वापस Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#अगर_घोषित MAKEFIXED
#समावेश <stdio.h>

व्योम makefixed9(व्योम) {
    अचिन्हित sym, bits, निचला, आकार;
    code *अग्र, *lenfix, *distfix;
    काष्ठा inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    जबतक (sym < 144) state.lens[sym++] = 8;
    जबतक (sym < 256) state.lens[sym++] = 9;
    जबतक (sym < 280) state.lens[sym++] = 7;
    जबतक (sym < 288) state.lens[sym++] = 8;
    अग्र = fixed;
    lenfix = अग्र;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(अग्र), &(bits), state.work);

    /* distance table */
    sym = 0;
    जबतक (sym < 32) state.lens[sym++] = 5;
    distfix = अग्र;
    bits = 5;
    inflate_table9(DISTS, state.lens, 32, &(अग्र), &(bits), state.work);

    /* write tables */
    माला_दो("    /* inffix9.h -- table for decoding deflate64 fixed codes");
    माला_दो("     * Generated automatically by makefixed9().");
    माला_दो("     */");
    माला_दो("");
    माला_दो("    /* WARNING: this file should *not* be used by applications.");
    माला_दो("       It is part of the implementation of this library and is");
    माला_दो("       subject to change. Applications should only use zlib.h.");
    माला_दो("     */");
    माला_दो("");
    आकार = 1U << 9;
    म_लिखो("    static const code lenfix[%u] = {", आकार);
    निचला = 0;
    क्रम (;;) {
        अगर ((निचला % 6) == 0) म_लिखो("\n        ");
        म_लिखो("{%u,%u,%d}", lenfix[निचला].op, lenfix[निचला].bits,
               lenfix[निचला].val);
        अगर (++निचला == आकार) अवरोध;
        अक्षर_दो(',');
    }
    माला_दो("\n    };");
    आकार = 1U << 5;
    म_लिखो("\n    static const code distfix[%u] = {", आकार);
    निचला = 0;
    क्रम (;;) {
        अगर ((निचला % 5) == 0) म_लिखो("\n        ");
        म_लिखो("{%u,%u,%d}", distfix[निचला].op, distfix[निचला].bits,
               distfix[निचला].val);
        अगर (++निचला == आकार) अवरोध;
        अक्षर_दो(',');
    }
    माला_दो("\n    };");
}
#पूर्ण_अगर /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = window; \
            बायाँ = WSIZE; \
            wrap = 1; \
            अगर (out(out_desc, put, (अचिन्हित)बायाँ)) { \
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
पूर्णांक ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, व्योम FAR *in_desc,
                         out_func out, व्योम FAR *out_desc) {
    काष्ठा inflate_state FAR *state;
    z_const अचिन्हित अक्षर FAR *अग्र;    /* next input */
    अचिन्हित अक्षर FAR *put;     /* next output */
    अचिन्हित have;              /* available input */
    अचिन्हित दीर्घ बायाँ;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    पूर्णांक lastblock;              /* true if processing last block */
    पूर्णांक wrap;                   /* true if the window has wrapped */
    अचिन्हित अक्षर FAR *window;  /* allocated sliding window, if needed */
    अचिन्हित दीर्घ hold;         /* bit buffer */
    अचिन्हित bits;              /* bits in bit buffer */
    अचिन्हित extra;             /* extra bits needed */
    अचिन्हित दीर्घ लंबाई;       /* literal or length of data to copy */
    अचिन्हित दीर्घ offset;       /* distance back to copy string from */
    अचिन्हित दीर्घ copy;         /* number of stored or match bytes to copy */
    अचिन्हित अक्षर FAR *from;    /* where to copy match bytes from */
    code स्थिर FAR *lencode;    /* starting table for length/literal codes */
    code स्थिर FAR *distcode;   /* starting table for distance codes */
    अचिन्हित lenbits;           /* index bits for lencode */
    अचिन्हित distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    अचिन्हित len;               /* length to copy for repeats, bits to drop */
    पूर्णांक ret;                    /* return code */
    अटल स्थिर अचिन्हित लघु order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#समावेश "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    अगर (strm == Z_NULL || strm->state == Z_NULL)
        वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->संदेश = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    अग्र = strm->अग्र_in;
    have = अग्र != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    बायाँ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    क्रम (;;)
        चयन (mode) {
        हाल TYPE:
            /* determine and dispatch block type */
            अगर (lastblock) {
                BYTEBITS();
                mode = DONE;
                अवरोध;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            चयन (BITS(2)) {
            हाल 0:                             /* stored block */
                Tracev((मानक_त्रुटि, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                अवरोध;
            हाल 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((मानक_त्रुटि, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                अवरोध;
            हाल 2:                             /* dynamic block */
                Tracev((मानक_त्रुटि, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                अवरोध;
            हाल 3:
                strm->संदेश = (z_const अक्षर *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            अवरोध;

        हाल STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            अगर ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->संदेश = (z_const अक्षर *)"invalid stored block lengths";
                mode = BAD;
                अवरोध;
            }
            लंबाई = (अचिन्हित)hold & 0xffff;
            Tracev((मानक_त्रुटि, "inflate:       stored length %lu\n",
                    लंबाई));
            INITBITS();

            /* copy stored block from input to output */
            जबतक (लंबाई != 0) {
                copy = लंबाई;
                PULL();
                ROOM();
                अगर (copy > have) copy = have;
                अगर (copy > बायाँ) copy = बायाँ;
                zmemcpy(put, अग्र, copy);
                have -= copy;
                अग्र += copy;
                बायाँ -= copy;
                put += copy;
                लंबाई -= copy;
            }
            Tracev((मानक_त्रुटि, "inflate:       stored end\n"));
            mode = TYPE;
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
            अगर (state->nlen > 286) {
                strm->संदेश = (z_const अक्षर *)"too many length symbols";
                mode = BAD;
                अवरोध;
            }
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
            lencode = (code स्थिर FAR *)(state->अग्र);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->अग्र),
                                &(lenbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid code lengths set";
                mode = BAD;
                अवरोध;
            }
            Tracev((मानक_त्रुटि, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            जबतक (state->have < state->nlen + state->ndist) {
                क्रम (;;) {
                    here = lencode[BITS(lenbits)];
                    अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                अगर (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अन्यथा {
                    अगर (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        अगर (state->have == 0) {
                            strm->संदेश = (z_const अक्षर *)"invalid bit length repeat";
                            mode = BAD;
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
                        strm->संदेश = (z_const अक्षर *)"invalid bit length repeat";
                        mode = BAD;
                        अवरोध;
                    }
                    जबतक (copy--)
                        state->lens[state->have++] = (अचिन्हित लघु)len;
                }
            }

            /* handle error breaks in while */
            अगर (mode == BAD) अवरोध;

            /* check for end-of-block code (better have one) */
            अगर (state->lens[256] == 0) {
                strm->संदेश = (z_const अक्षर *)"invalid code -- missing end-of-block";
                mode = BAD;
                अवरोध;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->अग्र = state->codes;
            lencode = (code स्थिर FAR *)(state->अग्र);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->अग्र), &(lenbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/lengths set";
                mode = BAD;
                अवरोध;
            }
            distcode = (code स्थिर FAR *)(state->अग्र);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->अग्र), &(distbits),
                            state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid distances set";
                mode = BAD;
                अवरोध;
            }
            Tracev((मानक_त्रुटि, "inflate:       codes ok\n"));
            mode = LEN;

        हाल LEN:
            /* get a literal, length, or end-of-block code */
            क्रम (;;) {
                here = lencode[BITS(lenbits)];
                अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                PULLBYTE();
            }
            अगर (here.op && (here.op & 0xf0) == 0) {
                last = here;
                क्रम (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    अगर ((अचिन्हित)(last.bits + here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            लंबाई = (अचिन्हित)here.val;

            /* process literal */
            अगर (here.op == 0) {
                Tracevv((मानक_त्रुटि, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (अचिन्हित अक्षर)(लंबाई);
                बायाँ--;
                mode = LEN;
                अवरोध;
            }

            /* process end of block */
            अगर (here.op & 32) {
                Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
                mode = TYPE;
                अवरोध;
            }

            /* invalid code */
            अगर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/length code";
                mode = BAD;
                अवरोध;
            }

            /* length code -- get extra bits, if any */
            extra = (अचिन्हित)(here.op) & 31;
            अगर (extra != 0) {
                NEEDBITS(extra);
                लंबाई += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %lu\n", लंबाई));

            /* get distance code */
            क्रम (;;) {
                here = distcode[BITS(distbits)];
                अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                PULLBYTE();
            }
            अगर ((here.op & 0xf0) == 0) {
                last = here;
                क्रम (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    अगर ((अचिन्हित)(last.bits + here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            अगर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid distance code";
                mode = BAD;
                अवरोध;
            }
            offset = (अचिन्हित)here.val;

            /* get distance extra bits, if any */
            extra = (अचिन्हित)(here.op) & 15;
            अगर (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            अगर (offset > WSIZE - (wrap ? 0: बायाँ)) {
                strm->संदेश = (z_const अक्षर *)"invalid distance too far back";
                mode = BAD;
                अवरोध;
            }
            Tracevv((मानक_त्रुटि, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            करो {
                ROOM();
                copy = WSIZE - offset;
                अगर (copy < बायाँ) {
                    from = put + copy;
                    copy = बायाँ - copy;
                }
                अन्यथा {
                    from = put - offset;
                    copy = बायाँ;
                }
                अगर (copy > लंबाई) copy = लंबाई;
                लंबाई -= copy;
                बायाँ -= copy;
                करो {
                    *put++ = *from++;
                } जबतक (--copy);
            } जबतक (लंबाई != 0);
            अवरोध;

        हाल DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            अगर (बायाँ < WSIZE) {
                अगर (out(out_desc, window, (अचिन्हित)(WSIZE - बायाँ)))
                    ret = Z_BUF_ERROR;
            }
            जाओ inf_leave;

        हाल BAD:
            ret = Z_तथ्य_ERROR;
            जाओ inf_leave;

        शेष:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            जाओ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->अग्र_in = अग्र;
    strm->avail_in = have;
    वापस ret;
}

पूर्णांक ZEXPORT inflateBack9End(z_stream FAR *strm) {
    अगर (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        वापस Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((मानक_त्रुटि, "inflate: end\n"));
    वापस Z_OK;
}
