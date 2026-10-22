/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "zutil.h"
#अन्तर्भाव "infback9.h"
#अन्तर्भाव "inftree9.h"
#अन्तर्भाव "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
पूर्णांक ZEXPORT inflateBack9Init_(z_stream FAR *strm, अचिन्हित अक्षर FAR *window,
                              स्थिरांक अक्षर *version, पूर्णांक stream_size) {
    संरचना inflate_state FAR *state;

    जर (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (पूर्णांक)(आकार(z_stream)))
        परत Z_VERSION_ERROR;
    जर (strm == Z_NULL || window == Z_NULL)
        परत Z_STREAM_ERROR;
    strm->संदेश = Z_NULL;                 /* in case we return an error */
    जर (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    जर (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (संरचना inflate_state FAR *)ZALLOC(strm, 1,
                                               आकार(संरचना inflate_state));
    जर (state == Z_NULL) परत Z_MEM_ERROR;
    Tracev((मानक_त्रुटि, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    परत Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#अन्तर्भाव <stdio.h>

शून्य makefixed9(शून्य) {
    अचिन्हित sym, bits, खालचा, size;
    code *next, *lenfix, *distfix;
    संरचना inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    पर्यंत (sym < 144) state.lens[sym++] = 8;
    पर्यंत (sym < 256) state.lens[sym++] = 9;
    पर्यंत (sym < 280) state.lens[sym++] = 7;
    पर्यंत (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    पर्यंत (sym < 32) state.lens[sym++] = 5;
    distfix = next;
    bits = 5;
    inflate_table9(DISTS, state.lens, 32, &(next), &(bits), state.work);

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
    size = 1U << 9;
    म_लिखो("    static const code lenfix[%u] = {", size);
    खालचा = 0;
    साठी (;;) {
        जर ((खालचा % 6) == 0) म_लिखो("\n        ");
        म_लिखो("{%u,%u,%d}", lenfix[खालचा].op, lenfix[खालचा].bits,
               lenfix[खालचा].val);
        जर (++खालचा == size) थांबा;
        अक्षर_दो(',');
    }
    माला_दो("\n    };");
    size = 1U << 5;
    म_लिखो("\n    static const code distfix[%u] = {", size);
    खालचा = 0;
    साठी (;;) {
        जर ((खालचा % 5) == 0) म_लिखो("\n        ");
        म_लिखो("{%u,%u,%d}", distfix[खालचा].op, distfix[खालचा].bits,
               distfix[खालचा].val);
        जर (++खालचा == size) थांबा;
        अक्षर_दो(',');
    }
    माला_दो("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = window; \
            डावा = WSIZE; \
            wrap = 1; \
            जर (out(out_desc, put, (अचिन्हित)डावा)) { \
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
पूर्णांक ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, शून्य FAR *in_desc,
                         out_func out, शून्य FAR *out_desc) {
    संरचना inflate_state FAR *state;
    z_const अचिन्हित अक्षर FAR *next;    /* next input */
    अचिन्हित अक्षर FAR *put;     /* next output */
    अचिन्हित have;              /* available input */
    अचिन्हित दीर्घ डावा;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    पूर्णांक lastblock;              /* true if processing last block */
    पूर्णांक wrap;                   /* true if the window has wrapped */
    अचिन्हित अक्षर FAR *window;  /* allocated sliding window, if needed */
    अचिन्हित दीर्घ hold;         /* bit buffer */
    अचिन्हित bits;              /* bits in bit buffer */
    अचिन्हित extra;             /* extra bits needed */
    अचिन्हित दीर्घ length;       /* literal or length of data to copy */
    अचिन्हित दीर्घ offset;       /* distance back to copy string from */
    अचिन्हित दीर्घ copy;         /* number of stored or match bytes to copy */
    अचिन्हित अक्षर FAR *from;    /* where to copy match bytes from */
    code स्थिरांक FAR *lencode;    /* starting table for length/literal codes */
    code स्थिरांक FAR *distcode;   /* starting table for distance codes */
    अचिन्हित lenbits;           /* index bits for lencode */
    अचिन्हित distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    अचिन्हित len;               /* length to copy for repeats, bits to drop */
    पूर्णांक ret;                    /* return code */
    स्थिर स्थिरांक अचिन्हित लघु order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#अन्तर्भाव "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    जर (strm == Z_NULL || strm->state == Z_NULL)
        परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->संदेश = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    डावा = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    साठी (;;)
        निवड (mode) {
        स्थिती TYPE:
            /* determine and dispatch block type */
            जर (lastblock) {
                BYTEBITS();
                mode = DONE;
                थांबा;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            निवड (BITS(2)) {
            स्थिती 0:                             /* stored block */
                Tracev((मानक_त्रुटि, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                थांबा;
            स्थिती 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((मानक_त्रुटि, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                थांबा;
            स्थिती 2:                             /* dynamic block */
                Tracev((मानक_त्रुटि, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                थांबा;
            स्थिती 3:
                strm->संदेश = (z_const अक्षर *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            थांबा;

        स्थिती STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            जर ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->संदेश = (z_const अक्षर *)"invalid stored block lengths";
                mode = BAD;
                थांबा;
            }
            length = (अचिन्हित)hold & 0xffff;
            Tracev((मानक_त्रुटि, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            पर्यंत (length != 0) {
                copy = length;
                PULL();
                ROOM();
                जर (copy > have) copy = have;
                जर (copy > डावा) copy = डावा;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                डावा -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((मानक_त्रुटि, "inflate:       stored end\n"));
            mode = TYPE;
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
            जर (state->nlen > 286) {
                strm->संदेश = (z_const अक्षर *)"too many length symbols";
                mode = BAD;
                थांबा;
            }
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
            lencode = (code स्थिरांक FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid code lengths set";
                mode = BAD;
                थांबा;
            }
            Tracev((मानक_त्रुटि, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            पर्यंत (state->have < state->nlen + state->ndist) {
                साठी (;;) {
                    here = lencode[BITS(lenbits)];
                    जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                जर (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अन्यथा {
                    जर (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        जर (state->have == 0) {
                            strm->संदेश = (z_const अक्षर *)"invalid bit length repeat";
                            mode = BAD;
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
                        strm->संदेश = (z_const अक्षर *)"invalid bit length repeat";
                        mode = BAD;
                        थांबा;
                    }
                    पर्यंत (copy--)
                        state->lens[state->have++] = (अचिन्हित लघु)len;
                }
            }

            /* handle error breaks in while */
            जर (mode == BAD) थांबा;

            /* check for end-of-block code (better have one) */
            जर (state->lens[256] == 0) {
                strm->संदेश = (z_const अक्षर *)"invalid code -- missing end-of-block";
                mode = BAD;
                थांबा;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code स्थिरांक FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/lengths set";
                mode = BAD;
                थांबा;
            }
            distcode = (code स्थिरांक FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid distances set";
                mode = BAD;
                थांबा;
            }
            Tracev((मानक_त्रुटि, "inflate:       codes ok\n"));
            mode = LEN;

        स्थिती LEN:
            /* get a literal, length, or end-of-block code */
            साठी (;;) {
                here = lencode[BITS(lenbits)];
                जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                PULLBYTE();
            }
            जर (here.op && (here.op & 0xf0) == 0) {
                last = here;
                साठी (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    जर ((अचिन्हित)(last.bits + here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (अचिन्हित)here.val;

            /* process literal */
            जर (here.op == 0) {
                Tracevv((मानक_त्रुटि, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (अचिन्हित अक्षर)(length);
                डावा--;
                mode = LEN;
                थांबा;
            }

            /* process end of block */
            जर (here.op & 32) {
                Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
                mode = TYPE;
                थांबा;
            }

            /* invalid code */
            जर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/length code";
                mode = BAD;
                थांबा;
            }

            /* length code -- get extra bits, if any */
            extra = (अचिन्हित)(here.op) & 31;
            जर (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %lu\n", length));

            /* get distance code */
            साठी (;;) {
                here = distcode[BITS(distbits)];
                जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                PULLBYTE();
            }
            जर ((here.op & 0xf0) == 0) {
                last = here;
                साठी (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    जर ((अचिन्हित)(last.bits + here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            जर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid distance code";
                mode = BAD;
                थांबा;
            }
            offset = (अचिन्हित)here.val;

            /* get distance extra bits, if any */
            extra = (अचिन्हित)(here.op) & 15;
            जर (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            जर (offset > WSIZE - (wrap ? 0: डावा)) {
                strm->संदेश = (z_const अक्षर *)"invalid distance too far back";
                mode = BAD;
                थांबा;
            }
            Tracevv((मानक_त्रुटि, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            करा {
                ROOM();
                copy = WSIZE - offset;
                जर (copy < डावा) {
                    from = put + copy;
                    copy = डावा - copy;
                }
                अन्यथा {
                    from = put - offset;
                    copy = डावा;
                }
                जर (copy > length) copy = length;
                length -= copy;
                डावा -= copy;
                करा {
                    *put++ = *from++;
                } पर्यंत (--copy);
            } पर्यंत (length != 0);
            थांबा;

        स्थिती DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            जर (डावा < WSIZE) {
                जर (out(out_desc, window, (अचिन्हित)(WSIZE - डावा)))
                    ret = Z_BUF_ERROR;
            }
            जा inf_leave;

        स्थिती BAD:
            ret = Z_DATA_ERROR;
            जा inf_leave;

        पूर्वनिर्धारित:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            जा inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    परत ret;
}

पूर्णांक ZEXPORT inflateBack9End(z_stream FAR *strm) {
    जर (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        परत Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((मानक_त्रुटि, "inflate: end\n"));
    परत Z_OK;
}
