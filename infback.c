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
पूर्णाङ्क ZEXPORT inflate_पृष्ठभागः_Init_2(z_streamp strm, पूर्णाङ्क गवाक्षः_द्वयङ्काः,
                             अचिह्नित वर्ण FAR *गवाक्षः, ध्रुव वर्ण *संस्करणम्,
                             पूर्णाङ्क प्रवाहः_आकार) {
    संरचना inflate_state FAR *state;

    यदि (संस्करणम् == Z_NULL || संस्करणम्[0] != ZLIB_संस्करणम्[0] ||
        प्रवाहः_आकार != (पूर्णाङ्क)(परिमाण(z_प्रवाहः)))
        प्रत्यागम Z_संस्करणम्_ERROR;
    यदि (strm == Z_NULL || गवाक्षः == Z_NULL ||
        गवाक्षः_द्वयङ्काः < 8 || गवाक्षः_द्वयङ्काः > 15)
        प्रत्यागम Z_प्रवाहः_ERROR;
    strm->सन्देशः = Z_NULL;                 /* in case we return an error */
    यदि (strm->zalloc == (alloc_func)0) {
#यदि_परिभाषितम् Z_SOLO
        प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#यदि_अन्तः
    }
    यदि (strm->zfree == (free_func)0)
#यदि_परिभाषितम् Z_SOLO
        प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#यदि_अन्तः
    state = (संरचना inflate_state FAR *)ZALLOC(strm, 1,
                                               परिमाण(संरचना inflate_state));
    यदि (state == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (संरचना आन्तरिकम्_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)गवाक्षः_द्वयङ्काः;
    state->wsize = 1U << गवाक्षः_द्वयङ्काः;
    state->गवाक्षः = गवाक्षः;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    प्रत्यागम Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#परिभाषय आरोपय_2() \
    कुरु { \
        put = strm->अग्र_बहिर्गमनम्; \
        वामः = strm->avail_बहिर्गमनम्; \
        अग्र = strm->अग्र_अन्तर्; \
        have = strm->avail_अन्तर्; \
        hold = state->hold; \
        द्वयङ्काः = state->द्वयङ्काः; \
    } यावत् (0)

/* Set state from registers for inflate_fast() */
#परिभाषय RESTORE() \
    कुरु { \
        strm->अग्र_बहिर्गमनम् = put; \
        strm->avail_बहिर्गमनम् = वामः; \
        strm->अग्र_अन्तर् = अग्र; \
        strm->avail_अन्तर् = have; \
        state->hold = hold; \
        state->द्वयङ्काः = द्वयङ्काः; \
    } यावत् (0)

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

/* Return the low n bits of the bit accumulator (n < 16) */
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
            put = state->गवाक्षः; \
            वामः = state->wsize; \
            state->whave = वामः; \
            यदि (बहिर्गमनम्(बहिर्गमनम्_desc, put, वामः)) { \
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
पूर्णाङ्क ZEXPORT inflate_पृष्ठभागः(z_streamp strm, अन्तर्_func अन्तर्, शून्य FAR *अन्तर्_desc,
                        बहिर्गमनम्_func बहिर्गमनम्, शून्य FAR *बहिर्गमनम्_desc) {
    संरचना inflate_state FAR *state;
    z_const अचिह्नित वर्ण FAR *अग्र;    /* next input */
    अचिह्नित वर्ण FAR *put;     /* next output */
    अचिह्नित have, वामः;        /* available input and output */
    अचिह्नित दीर्घ hold;         /* bit buffer */
    अचिह्नित द्वयङ्काः;              /* bits in bit buffer */
    अचिह्नित copy;              /* number of stored or match bytes to copy */
    अचिह्नित वर्ण FAR *from;    /* where to copy match bytes from */
    संहिता अत्र;                  /* current decoding table entry */
    संहिता चरमः;                  /* parent table entry */
    अचिह्नित len;               /* length to copy for repeats, bits to drop */
    पूर्णाङ्क ret;                    /* return code */
    स्थिर ध्रुव अचिह्नित ह्रस्व क्रमः[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    यदि (strm == Z_NULL || strm->state == Z_NULL)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->सन्देशः = Z_NULL;
    state->बहुलकम् = TYPE;
    state->चरमः = 0;
    state->whave = 0;
    अग्र = strm->अग्र_अन्तर्;
    have = अग्र != Z_NULL ? strm->avail_अन्तर् : 0;
    hold = 0;
    द्वयङ्काः = 0;
    put = state->गवाक्षः;
    वामः = state->wsize;

    /* Inflate until end of block marked as last */
    प्रति (;;)
        विकल्प (state->बहुलकम्) {
        पक्ष TYPE:
            /* determine and dispatch block type */
            यदि (state->चरमः) {
                BYTEBITS();
                state->बहुलकम् = समाप्तम्;
                भङ्ग;
            }
            NEEDBITS(3);
            state->चरमः = द्वयङ्काः_2(1);
            DROPBITS(1);
            विकल्प (द्वयङ्काः_2(2)) {
            पक्ष 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->चरमः ? " (last)" : ""));
                state->बहुलकम् = STORED;
                भङ्ग;
            पक्ष 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->चरमः ? " (last)" : ""));
                state->बहुलकम् = LEN;              /* decode codes */
                भङ्ग;
            पक्ष 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->चरमः ? " (last)" : ""));
                state->बहुलकम् = सारणी;
                भङ्ग;
            स्वभाव:
                strm->सन्देशः = (z_const वर्ण *)"invalid block type";
                state->बहुलकम् = BAD;
            }
            DROPBITS(2);
            भङ्ग;

        पक्ष STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            यदि ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid stored block lengths";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->दीर्घता = (अचिह्नित)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->दीर्घता));
            INITBITS();

            /* copy stored block from input to output */
            यावत् (state->दीर्घता != 0) {
                copy = state->दीर्घता;
                PULL();
                अवकाशः();
                यदि (copy > have) copy = have;
                यदि (copy > वामः) copy = वामः;
                zmemcpy(put, अग्र, copy);
                have -= copy;
                अग्र += copy;
                वामः -= copy;
                put += copy;
                state->दीर्घता -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->बहुलकम् = TYPE;
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
#यदि_अपरिभाषितम् PKZIP_BUG_WORKAROUND
            यदि (state->nlen > 286 || state->ndist > 30) {
                strm->सन्देशः = (z_const वर्ण *)
                    "too many length or distance symbols";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
#यदि_अन्तः
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
            state->lencode = (संहिता ध्रुव FAR *)(state->अग्र);
            state->lenbits = 7;
            ret = inflate_सारणी(CODES, state->lens, 19, &(state->अग्र),
                                &(state->lenbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid code lengths set";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            यावत् (state->have < state->nlen + state->ndist) {
                प्रति (;;) {
                    अत्र = state->lencode[द्वयङ्काः_2(state->lenbits)];
                    यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                यदि (अत्र.val < 16) {
                    DROPBITS(अत्र.द्वयङ्काः);
                    state->lens[state->have++] = अत्र.val;
                }
                अन्यथा {
                    यदि (अत्र.val == 16) {
                        NEEDBITS(अत्र.द्वयङ्काः + 2);
                        DROPBITS(अत्र.द्वयङ्काः);
                        यदि (state->have == 0) {
                            strm->सन्देशः = (z_const वर्ण *)
                                "invalid bit length repeat";
                            state->बहुलकम् = BAD;
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
                        strm->सन्देशः = (z_const वर्ण *)
                            "invalid bit length repeat";
                        state->बहुलकम् = BAD;
                        भङ्ग;
                    }
                    यावत् (copy--)
                        state->lens[state->have++] = (अचिह्नित ह्रस्व)len;
                }
            }

            /* handle error breaks in while */
            यदि (state->बहुलकम् == BAD) भङ्ग;

            /* check for end-of-block code (better have one) */
            यदि (state->lens[256] == 0) {
                strm->सन्देशः = (z_const वर्ण *)
                    "invalid code -- missing end-of-block";
                state->बहुलकम् = BAD;
                भङ्ग;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->अग्र = state->codes;
            state->lencode = (संहिता ध्रुव FAR *)(state->अग्र);
            state->lenbits = 9;
            ret = inflate_सारणी(LENS, state->lens, state->nlen, &(state->अग्र),
                                &(state->lenbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/lengths set";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->distcode = (संहिता ध्रुव FAR *)(state->अग्र);
            state->distbits = 6;
            ret = inflate_सारणी(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->अग्र), &(state->distbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distances set";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->बहुलकम् = LEN;
                /* fallthrough */

        पक्ष LEN:
            /* use inflate_fast() if we have enough input and output */
            यदि (have >= 6 && वामः >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                आरोपय_2();
                भङ्ग;
            }

            /* get a literal, length, or end-of-block code */
            प्रति (;;) {
                अत्र = state->lencode[द्वयङ्काः_2(state->lenbits)];
                यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                PULLBYTE();
            }
            यदि (अत्र.op && (अत्र.op & 0xf0) == 0) {
                चरमः = अत्र;
                प्रति (;;) {
                    अत्र = state->lencode[चरमः.val +
                            (द्वयङ्काः_2(चरमः.द्वयङ्काः + चरमः.op) >> चरमः.द्वयङ्काः)];
                    यदि ((अचिह्नित)(चरमः.द्वयङ्काः + अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(चरमः.द्वयङ्काः);
            }
            DROPBITS(अत्र.द्वयङ्काः);
            state->दीर्घता = (अचिह्नित)अत्र.val;

            /* process literal */
            यदि (अत्र.op == 0) {
                Tracevv((stderr, अत्र.val >= 0x20 && अत्र.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", अत्र.val));
                अवकाशः();
                *put++ = (अचिह्नित वर्ण)(state->दीर्घता);
                वामः--;
                state->बहुलकम् = LEN;
                भङ्ग;
            }

            /* process end of block */
            यदि (अत्र.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->बहुलकम् = TYPE;
                भङ्ग;
            }

            /* invalid code */
            यदि (अत्र.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/length code";
                state->बहुलकम् = BAD;
                भङ्ग;
            }

            /* length code -- get extra bits, if any */
            state->extra = (अचिह्नित)(अत्र.op) & 15;
            यदि (state->extra != 0) {
                NEEDBITS(state->extra);
                state->दीर्घता += द्वयङ्काः_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->दीर्घता));

            /* get distance code */
            प्रति (;;) {
                अत्र = state->distcode[द्वयङ्काः_2(state->distbits)];
                यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                PULLBYTE();
            }
            यदि ((अत्र.op & 0xf0) == 0) {
                चरमः = अत्र;
                प्रति (;;) {
                    अत्र = state->distcode[चरमः.val +
                            (द्वयङ्काः_2(चरमः.द्वयङ्काः + चरमः.op) >> चरमः.द्वयङ्काः)];
                    यदि ((अचिह्नित)(चरमः.द्वयङ्काः + अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(चरमः.द्वयङ्काः);
            }
            DROPBITS(अत्र.द्वयङ्काः);
            यदि (अत्र.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance code";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->विस्थापनम् = (अचिह्नित)अत्र.val;

            /* get distance extra bits, if any */
            state->extra = (अचिह्नित)(अत्र.op) & 15;
            यदि (state->extra != 0) {
                NEEDBITS(state->extra);
                state->विस्थापनम् += द्वयङ्काः_2(state->extra);
                DROPBITS(state->extra);
            }
            यदि (state->विस्थापनम् > state->wsize - (state->whave < state->wsize ?
                                                वामः : 0)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance too far back";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->विस्थापनम्));

            /* copy match from window to output */
            कुरु {
                अवकाशः();
                copy = state->wsize - state->विस्थापनम्;
                यदि (copy < वामः) {
                    from = put + copy;
                    copy = वामः - copy;
                }
                अन्यथा {
                    from = put - state->विस्थापनम्;
                    copy = वामः;
                }
                यदि (copy > state->दीर्घता) copy = state->दीर्घता;
                state->दीर्घता -= copy;
                वामः -= copy;
                कुरु {
                    *put++ = *from++;
                } यावत् (--copy);
            } यावत् (state->दीर्घता != 0);
            भङ्ग;

        पक्ष समाप्तम्:
            /* inflate stream terminated properly */
            ret = Z_प्रवाहः_अन्तः;
            गच्छ inf_leave;

        पक्ष BAD:
            ret = Z_दत्त_ERROR;
            गच्छ inf_leave;

        स्वभाव:
            /* can't happen, but makes compilers happy */
            ret = Z_प्रवाहः_ERROR;
            गच्छ inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    यदि (वामः < state->wsize) {
        यदि (बहिर्गमनम्(बहिर्गमनम्_desc, state->गवाक्षः, state->wsize - वामः) &&
            ret == Z_प्रवाहः_अन्तः)
            ret = Z_BUF_ERROR;
    }
    strm->अग्र_अन्तर् = अग्र;
    strm->avail_अन्तर् = have;
    प्रत्यागम ret;
}

पूर्णाङ्क ZEXPORT inflate_पृष्ठभागः_अन्तः(z_streamp strm) {
    यदि (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        प्रत्यागम Z_प्रवाहः_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    प्रत्यागम Z_OK;
}
