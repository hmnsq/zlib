/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "zutil.h"
#अन्तर्भाव "infback9.h"
#अन्तर्भाव "inftree9.h"
#अन्तर्भाव "inflate9.h"

#परिभाषय WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
पूर्णाङ्क ZEXPORT inflateBack9Init_(z_प्रवाहः FAR *strm, अचिह्नित वर्ण FAR *गवाक्षः,
                              ध्रुव वर्ण *संस्करणम्, पूर्णाङ्क प्रवाहः_आकार) {
    संरचना inflate_state FAR *state;

    यदि (संस्करणम् == Z_NULL || संस्करणम्[0] != ZLIB_संस्करणम्[0] ||
        प्रवाहः_आकार != (पूर्णाङ्क)(परिमाण(z_प्रवाहः)))
        प्रत्यागम Z_संस्करणम्_ERROR;
    यदि (strm == Z_NULL || गवाक्षः == Z_NULL)
        प्रत्यागम Z_प्रवाहः_ERROR;
    strm->सन्देशः = Z_NULL;                 /* in case we return an error */
    यदि (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    यदि (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (संरचना inflate_state FAR *)ZALLOC(strm, 1,
                                               परिमाण(संरचना inflate_state));
    यदि (state == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->गवाक्षः = गवाक्षः;
    प्रत्यागम Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#यदि_परिभाषितम् MAKEFIXED
#अन्तर्भाव <stdio.h>

शून्य makefixed9(शून्य) {
    अचिह्नित sym, द्वयङ्काः, नीचः, आकार;
    संहिता *अग्र, *lenfix, *distfix;
    संरचना inflate_state state;
    संहिता fixed[544];

    /* literal/length table */
    sym = 0;
    यावत् (sym < 144) state.lens[sym++] = 8;
    यावत् (sym < 256) state.lens[sym++] = 9;
    यावत् (sym < 280) state.lens[sym++] = 7;
    यावत् (sym < 288) state.lens[sym++] = 8;
    अग्र = fixed;
    lenfix = अग्र;
    द्वयङ्काः = 9;
    inflate_table9(LENS, state.lens, 288, &(अग्र), &(द्वयङ्काः), state.work);

    /* distance table */
    sym = 0;
    यावत् (sym < 32) state.lens[sym++] = 5;
    distfix = अग्र;
    द्वयङ्काः = 5;
    inflate_table9(DISTS, state.lens, 32, &(अग्र), &(द्वयङ्काः), state.work);

    /* write tables */
    पङ्क्तिं_लिख("    /* inffix9.h -- table for decoding deflate64 fixed codes");
    पङ्क्तिं_लिख("     * Generated automatically by makefixed9().");
    पङ्क्तिं_लिख("     */");
    पङ्क्तिं_लिख("");
    पङ्क्तिं_लिख("    /* WARNING: this file should *not* be used by applications.");
    पङ्क्तिं_लिख("       It is part of the implementation of this library and is");
    पङ्क्तिं_लिख("       subject to change. Applications should only use zlib.h.");
    पङ्क्तिं_लिख("     */");
    पङ्क्तिं_लिख("");
    आकार = 1U << 9;
    प्रारूप_लिख("    static const code lenfix[%u] = {", आकार);
    नीचः = 0;
    प्रति (;;) {
        यदि ((नीचः % 6) == 0) प्रारूप_लिख("\n        ");
        प्रारूप_लिख("{%u,%u,%d}", lenfix[नीचः].op, lenfix[नीचः].द्वयङ्काः,
               lenfix[नीचः].val);
        यदि (++नीचः == आकार) भङ्ग;
        अक्षरं_लिख(',');
    }
    पङ्क्तिं_लिख("\n    };");
    आकार = 1U << 5;
    प्रारूप_लिख("\n    static const code distfix[%u] = {", आकार);
    नीचः = 0;
    प्रति (;;) {
        यदि ((नीचः % 5) == 0) प्रारूप_लिख("\n        ");
        प्रारूप_लिख("{%u,%u,%d}", distfix[नीचः].op, distfix[नीचः].द्वयङ्काः,
               distfix[नीचः].val);
        यदि (++नीचः == आकार) भङ्ग;
        अक्षरं_लिख(',');
    }
    पङ्क्तिं_लिख("\n    };");
}
#यदि_अन्तः /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#परिभाषय INITBITS() \
    कुरु { \
        hold = 0; \
        द्वयङ्काः = 0; \
    } यावत् (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#परिभाषय PULL() \
    कुरु { \
        यदि (have == 0) { \
            have = अन्तर्(अन्तर्_desc, &अग्र); \
            यदि (have == 0) { \
                अग्र = Z_NULL; \
                ret = Z_BUF_ERROR; \
                गच्छ inf_leave; \
            } \
        } \
    } यावत् (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#परिभाषय PULLBYTE() \
    कुरु { \
        PULL(); \
        have--; \
        hold += (अचिह्नित दीर्घ)(*अग्र++) << द्वयङ्काः; \
        द्वयङ्काः += 8; \
    } यावत् (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#परिभाषय NEEDBITS(n) \
    कुरु { \
        यावत् (द्वयङ्काः < (अचिह्नित)(n)) \
            PULLBYTE(); \
    } यावत् (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#परिभाषय द्वयङ्काः_2(n) \
    ((अचिह्नित)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#परिभाषय DROPBITS(n) \
    कुरु { \
        hold >>= (n); \
        द्वयङ्काः -= (अचिह्नित)(n); \
    } यावत् (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#परिभाषय BYTEBITS() \
    कुरु { \
        hold >>= द्वयङ्काः & 7; \
        द्वयङ्काः -= द्वयङ्काः & 7; \
    } यावत् (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#परिभाषय अवकाशः() \
    कुरु { \
        यदि (वामः == 0) { \
            put = गवाक्षः; \
            वामः = WSIZE; \
            wrap = 1; \
            यदि (बहिर्गमनम्(बहिर्गमनम्_desc, put, (अचिह्नित)वामः)) { \
                ret = Z_BUF_ERROR; \
                गच्छ inf_leave; \
            } \
        } \
    } यावत् (0)

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
पूर्णाङ्क ZEXPORT inflateBack9(z_प्रवाहः FAR *strm, अन्तर्_func अन्तर्, शून्य FAR *अन्तर्_desc,
                         बहिर्गमनम्_func बहिर्गमनम्, शून्य FAR *बहिर्गमनम्_desc) {
    संरचना inflate_state FAR *state;
    z_const अचिह्नित वर्ण FAR *अग्र;    /* next input */
    अचिह्नित वर्ण FAR *put;     /* next output */
    अचिह्नित have;              /* available input */
    अचिह्नित दीर्घ वामः;         /* available output */
    inflate_बहुलकम् बहुलकम्;          /* current inflate mode */
    पूर्णाङ्क lastblock;              /* true if processing last block */
    पूर्णाङ्क wrap;                   /* true if the window has wrapped */
    अचिह्नित वर्ण FAR *गवाक्षः;  /* allocated sliding window, if needed */
    अचिह्नित दीर्घ hold;         /* bit buffer */
    अचिह्नित द्वयङ्काः;              /* bits in bit buffer */
    अचिह्नित extra;             /* extra bits needed */
    अचिह्नित दीर्घ दीर्घता;       /* literal or length of data to copy */
    अचिह्नित दीर्घ विस्थापनम्;       /* distance back to copy string from */
    अचिह्नित दीर्घ copy;         /* number of stored or match bytes to copy */
    अचिह्नित वर्ण FAR *from;    /* where to copy match bytes from */
    संहिता ध्रुव FAR *lencode;    /* starting table for length/literal codes */
    संहिता ध्रुव FAR *distcode;   /* starting table for distance codes */
    अचिह्नित lenbits;           /* index bits for lencode */
    अचिह्नित distbits;          /* index bits for distcode */
    संहिता अत्र;                  /* current decoding table entry */
    संहिता चरमः;                  /* parent table entry */
    अचिह्नित len;               /* length to copy for repeats, bits to drop */
    पूर्णाङ्क ret;                    /* return code */
    स्थिर ध्रुव अचिह्नित ह्रस्व क्रमः[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#अन्तर्भाव "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    यदि (strm == Z_NULL || strm->state == Z_NULL)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->सन्देशः = Z_NULL;
    बहुलकम् = TYPE;
    lastblock = 0;
    wrap = 0;
    गवाक्षः = state->गवाक्षः;
    अग्र = strm->अग्र_अन्तर्;
    have = अग्र != Z_NULL ? strm->avail_अन्तर् : 0;
    hold = 0;
    द्वयङ्काः = 0;
    put = गवाक्षः;
    वामः = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    प्रति (;;)
        विकल्प (बहुलकम्) {
        पक्ष TYPE:
            /* determine and dispatch block type */
            यदि (lastblock) {
                BYTEBITS();
                बहुलकम् = समाप्तम्;
                भङ्ग;
            }
            NEEDBITS(3);
            lastblock = द्वयङ्काः_2(1);
            DROPBITS(1);
            विकल्प (द्वयङ्काः_2(2)) {
            पक्ष 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                बहुलकम् = STORED;
                भङ्ग;
            पक्ष 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                बहुलकम् = LEN;                     /* decode codes */
                भङ्ग;
            पक्ष 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                बहुलकम् = सारणी;
                भङ्ग;
            पक्ष 3:
                strm->सन्देशः = (z_const वर्ण *)"invalid block type";
                बहुलकम् = BAD;
            }
            DROPBITS(2);
            भङ्ग;

        पक्ष STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            यदि ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid stored block lengths";
                बहुलकम् = BAD;
                भङ्ग;
            }
            दीर्घता = (अचिह्नित)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    दीर्घता));
            INITBITS();

            /* copy stored block from input to output */
            यावत् (दीर्घता != 0) {
                copy = दीर्घता;
                PULL();
                अवकाशः();
                यदि (copy > have) copy = have;
                यदि (copy > वामः) copy = वामः;
                zmemcpy(put, अग्र, copy);
                have -= copy;
                अग्र += copy;
                वामः -= copy;
                put += copy;
                दीर्घता -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            बहुलकम् = TYPE;
            भङ्ग;

        पक्ष सारणी:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = द्वयङ्काः_2(5) + 257;
            DROPBITS(5);
            state->ndist = द्वयङ्काः_2(5) + 1;
            DROPBITS(5);
            state->ncode = द्वयङ्काः_2(4) + 4;
            DROPBITS(4);
            यदि (state->nlen > 286) {
                strm->सन्देशः = (z_const वर्ण *)"too many length symbols";
                बहुलकम् = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            यावत् (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[क्रमः[state->have++]] = (अचिह्नित ह्रस्व)द्वयङ्काः_2(3);
                DROPBITS(3);
            }
            यावत् (state->have < 19)
                state->lens[क्रमः[state->have++]] = 0;
            state->अग्र = state->codes;
            lencode = (संहिता ध्रुव FAR *)(state->अग्र);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->अग्र),
                                &(lenbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid code lengths set";
                बहुलकम् = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            यावत् (state->have < state->nlen + state->ndist) {
                प्रति (;;) {
                    अत्र = lencode[द्वयङ्काः_2(lenbits)];
                    यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                यदि (अत्र.val < 16) {
                    NEEDBITS(अत्र.द्वयङ्काः);
                    DROPBITS(अत्र.द्वयङ्काः);
                    state->lens[state->have++] = अत्र.val;
                }
                अन्यथा {
                    यदि (अत्र.val == 16) {
                        NEEDBITS(अत्र.द्वयङ्काः + 2);
                        DROPBITS(अत्र.द्वयङ्काः);
                        यदि (state->have == 0) {
                            strm->सन्देशः = (z_const वर्ण *)"invalid bit length repeat";
                            बहुलकम् = BAD;
                            भङ्ग;
                        }
                        len = (अचिह्नित)(state->lens[state->have - 1]);
                        copy = 3 + द्वयङ्काः_2(2);
                        DROPBITS(2);
                    }
                    अन्यथा यदि (अत्र.val == 17) {
                        NEEDBITS(अत्र.द्वयङ्काः + 3);
                        DROPBITS(अत्र.द्वयङ्काः);
                        len = 0;
                        copy = 3 + द्वयङ्काः_2(3);
                        DROPBITS(3);
                    }
                    अन्यथा {
                        NEEDBITS(अत्र.द्वयङ्काः + 7);
                        DROPBITS(अत्र.द्वयङ्काः);
                        len = 0;
                        copy = 11 + द्वयङ्काः_2(7);
                        DROPBITS(7);
                    }
                    यदि (state->have + copy > state->nlen + state->ndist) {
                        strm->सन्देशः = (z_const वर्ण *)"invalid bit length repeat";
                        बहुलकम् = BAD;
                        भङ्ग;
                    }
                    यावत् (copy--)
                        state->lens[state->have++] = (अचिह्नित ह्रस्व)len;
                }
            }

            /* handle error breaks in while */
            यदि (बहुलकम् == BAD) भङ्ग;

            /* check for end-of-block code (better have one) */
            यदि (state->lens[256] == 0) {
                strm->सन्देशः = (z_const वर्ण *)"invalid code -- missing end-of-block";
                बहुलकम् = BAD;
                भङ्ग;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->अग्र = state->codes;
            lencode = (संहिता ध्रुव FAR *)(state->अग्र);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->अग्र), &(lenbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/lengths set";
                बहुलकम् = BAD;
                भङ्ग;
            }
            distcode = (संहिता ध्रुव FAR *)(state->अग्र);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->अग्र), &(distbits),
                            state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distances set";
                बहुलकम् = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            बहुलकम् = LEN;

        पक्ष LEN:
            /* get a literal, length, or end-of-block code */
            प्रति (;;) {
                अत्र = lencode[द्वयङ्काः_2(lenbits)];
                यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                PULLBYTE();
            }
            यदि (अत्र.op && (अत्र.op & 0xf0) == 0) {
                चरमः = अत्र;
                प्रति (;;) {
                    अत्र = lencode[चरमः.val +
                            (द्वयङ्काः_2(चरमः.द्वयङ्काः + चरमः.op) >> चरमः.द्वयङ्काः)];
                    यदि ((अचिह्नित)(चरमः.द्वयङ्काः + अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(चरमः.द्वयङ्काः);
            }
            DROPBITS(अत्र.द्वयङ्काः);
            दीर्घता = (अचिह्नित)अत्र.val;

            /* process literal */
            यदि (अत्र.op == 0) {
                Tracevv((stderr, अत्र.val >= 0x20 && अत्र.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", अत्र.val));
                अवकाशः();
                *put++ = (अचिह्नित वर्ण)(दीर्घता);
                वामः--;
                बहुलकम् = LEN;
                भङ्ग;
            }

            /* process end of block */
            यदि (अत्र.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                बहुलकम् = TYPE;
                भङ्ग;
            }

            /* invalid code */
            यदि (अत्र.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/length code";
                बहुलकम् = BAD;
                भङ्ग;
            }

            /* length code -- get extra bits, if any */
            extra = (अचिह्नित)(अत्र.op) & 31;
            यदि (extra != 0) {
                NEEDBITS(extra);
                दीर्घता += द्वयङ्काः_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", दीर्घता));

            /* get distance code */
            प्रति (;;) {
                अत्र = distcode[द्वयङ्काः_2(distbits)];
                यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                PULLBYTE();
            }
            यदि ((अत्र.op & 0xf0) == 0) {
                चरमः = अत्र;
                प्रति (;;) {
                    अत्र = distcode[चरमः.val +
                            (द्वयङ्काः_2(चरमः.द्वयङ्काः + चरमः.op) >> चरमः.द्वयङ्काः)];
                    यदि ((अचिह्नित)(चरमः.द्वयङ्काः + अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(चरमः.द्वयङ्काः);
            }
            DROPBITS(अत्र.द्वयङ्काः);
            यदि (अत्र.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance code";
                बहुलकम् = BAD;
                भङ्ग;
            }
            विस्थापनम् = (अचिह्नित)अत्र.val;

            /* get distance extra bits, if any */
            extra = (अचिह्नित)(अत्र.op) & 15;
            यदि (extra != 0) {
                NEEDBITS(extra);
                विस्थापनम् += द्वयङ्काः_2(extra);
                DROPBITS(extra);
            }
            यदि (विस्थापनम् > WSIZE - (wrap ? 0: वामः)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance too far back";
                बहुलकम् = BAD;
                भङ्ग;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", विस्थापनम्));

            /* copy match from window to output */
            कुरु {
                अवकाशः();
                copy = WSIZE - विस्थापनम्;
                यदि (copy < वामः) {
                    from = put + copy;
                    copy = वामः - copy;
                }
                अन्यथा {
                    from = put - विस्थापनम्;
                    copy = वामः;
                }
                यदि (copy > दीर्घता) copy = दीर्घता;
                दीर्घता -= copy;
                वामः -= copy;
                कुरु {
                    *put++ = *from++;
                } यावत् (--copy);
            } यावत् (दीर्घता != 0);
            भङ्ग;

        पक्ष समाप्तम्:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_प्रवाहः_अन्तः;
            यदि (वामः < WSIZE) {
                यदि (बहिर्गमनम्(बहिर्गमनम्_desc, गवाक्षः, (अचिह्नित)(WSIZE - वामः)))
                    ret = Z_BUF_ERROR;
            }
            गच्छ inf_leave;

        पक्ष BAD:
            ret = Z_दत्त_ERROR;
            गच्छ inf_leave;

        स्वभाव:                /* can't happen, but makes compilers happy */
            ret = Z_प्रवाहः_ERROR;
            गच्छ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->अग्र_अन्तर् = अग्र;
    strm->avail_अन्तर् = have;
    प्रत्यागम ret;
}

पूर्णाङ्क ZEXPORT inflate_Back9_अन्तः(z_प्रवाहः FAR *strm) {
    यदि (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        प्रत्यागम Z_प्रवाहः_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    प्रत्यागम Z_OK;
}
