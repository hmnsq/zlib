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
صحيح ZEXPORT inflate_شا_Init_2(z_streamp strm, صحيح کړکۍ_بټونه,
                             بینښه حرف FAR *کړکۍ, ثابت حرف *نسخه,
                             صحيح بهیر_اندازه) {
    بنية inflate_state FAR *state;

    إذا (نسخه == Z_NULL || نسخه[0] != ZLIB_نسخه[0] ||
        بهیر_اندازه != (صحيح)(حجم(z_بهیر)))
        أرجع Z_نسخه_ERROR;
    إذا (strm == Z_NULL || کړکۍ == Z_NULL ||
        کړکۍ_بټونه < 8 || کړکۍ_بټونه > 15)
        أرجع Z_بهیر_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#که_تعریف_شوی Z_SOLO
        أرجع Z_بهیر_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#د_که_پای
    }
    إذا (strm->zfree == (free_func)0)
#که_تعریف_شوی Z_SOLO
        أرجع Z_بهیر_ERROR;
#وإلا
        strm->zfree = zcfree;
#د_که_پای
    state = (بنية inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية دننی_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)کړکۍ_بټونه;
    state->wsize = 1U << کړکۍ_بټونه;
    state->کړکۍ = کړکۍ;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    أرجع Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#تعریف_کړه بارول_2() \
    افعل { \
        put = strm->بل_بهر; \
        کیڼ = strm->avail_بهر; \
        بل = strm->بل_دننه; \
        have = strm->avail_دننه; \
        hold = state->hold; \
        بټونه = state->بټونه; \
    } طالما (0)

/* Set state from registers for inflate_fast() */
#تعریف_کړه RESTORE() \
    افعل { \
        strm->بل_بهر = put; \
        strm->avail_بهر = کیڼ; \
        strm->بل_دننه = بل; \
        strm->avail_دننه = have; \
        state->hold = hold; \
        state->بټونه = بټونه; \
    } طالما (0)

/* Clear the input bit accumulator */
#تعریف_کړه INITBITS() \
    افعل { \
        hold = 0; \
        بټونه = 0; \
    } طالما (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#تعریف_کړه PULL() \
    افعل { \
        إذا (have == 0) { \
            have = دننه(دننه_desc, &بل); \
            إذا (have == 0) { \
                بل = Z_NULL; \
                ret = Z_BUF_ERROR; \
                اذهب inf_leave; \
            } \
        } \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#تعریف_کړه PULLBYTE() \
    افعل { \
        PULL(); \
        have--; \
        hold += (بینښه اوږد)(*بل++) << بټونه; \
        بټونه += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#تعریف_کړه NEEDBITS(n) \
    افعل { \
        طالما (بټونه < (بینښه)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کړه بټونه_2(n) \
    ((بینښه)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کړه DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بټونه -= (بینښه)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کړه BYTEBITS() \
    افعل { \
        hold >>= بټونه & 7; \
        بټونه -= بټونه & 7; \
    } طالما (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#تعریف_کړه ځای() \
    افعل { \
        إذا (کیڼ == 0) { \
            put = state->کړکۍ; \
            کیڼ = state->wsize; \
            state->whave = کیڼ; \
            إذا (بهر(بهر_desc, put, کیڼ)) { \
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
صحيح ZEXPORT inflate_شا(z_streamp strm, دننه_func دننه, فراغ FAR *دننه_desc,
                        بهر_func بهر, فراغ FAR *بهر_desc) {
    بنية inflate_state FAR *state;
    z_const بینښه حرف FAR *بل;    /* next input */
    بینښه حرف FAR *put;     /* next output */
    بینښه have, کیڼ;        /* available input and output */
    بینښه اوږد hold;         /* bit buffer */
    بینښه بټونه;              /* bits in bit buffer */
    بینښه copy;              /* number of stored or match bytes to copy */
    بینښه حرف FAR *from;    /* where to copy match bytes from */
    کوډ دلته;                  /* current decoding table entry */
    کوډ وروستی;                  /* parent table entry */
    بینښه len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بینښه لنډ ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیغام = Z_NULL;
    state->ډېرپېښ = TYPE;
    state->وروستی = 0;
    state->whave = 0;
    بل = strm->بل_دننه;
    have = بل != Z_NULL ? strm->avail_دننه : 0;
    hold = 0;
    بټونه = 0;
    put = state->کړکۍ;
    کیڼ = state->wsize;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (state->ډېرپېښ) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (state->وروستی) {
                BYTEBITS();
                state->ډېرپېښ = بشپړ;
                اكسر;
            }
            NEEDBITS(3);
            state->وروستی = بټونه_2(1);
            DROPBITS(1);
            بدل (بټونه_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->وروستی ? " (last)" : ""));
                state->ډېرپېښ = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->وروستی ? " (last)" : ""));
                state->ډېرپېښ = LEN;              /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->وروستی ? " (last)" : ""));
                state->ډېرپېښ = جدول;
                اكسر;
            افتراضي:
                strm->پیغام = (z_const حرف *)"invalid block type";
                state->ډېرپېښ = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->اوږدتیا = (بینښه)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->اوږدتیا));
            INITBITS();

            /* copy stored block from input to output */
            طالما (state->اوږدتیا != 0) {
                copy = state->اوږدتیا;
                PULL();
                ځای();
                إذا (copy > have) copy = have;
                إذا (copy > کیڼ) copy = کیڼ;
                zmemcpy(put, بل, copy);
                have -= copy;
                بل += copy;
                کیڼ -= copy;
                put += copy;
                state->اوږدتیا -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->ډېرپېښ = TYPE;
            اكسر;

        حالة جدول:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = بټونه_2(5) + 257;
            DROPBITS(5);
            state->ndist = بټونه_2(5) + 1;
            DROPBITS(5);
            state->ncode = بټونه_2(4) + 4;
            DROPBITS(4);
#که_نه_تعریف_شوی PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->پیغام = (z_const حرف *)
                    "too many length or distance symbols";
                state->ډېرپېښ = BAD;
                اكسر;
            }
#د_که_پای
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (بینښه لنډ)بټونه_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->بل = state->codes;
            state->lencode = (کوډ ثابت FAR *)(state->بل);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->بل),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    دلته = state->lencode[بټونه_2(state->lenbits)];
                    إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                إذا (دلته.val < 16) {
                    DROPBITS(دلته.بټونه);
                    state->lens[state->have++] = دلته.val;
                }
                وإلا {
                    إذا (دلته.val == 16) {
                        NEEDBITS(دلته.بټونه + 2);
                        DROPBITS(دلته.بټونه);
                        إذا (state->have == 0) {
                            strm->پیغام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->ډېرپېښ = BAD;
                            اكسر;
                        }
                        len = (بینښه)(state->lens[state->have - 1]);
                        copy = 3 + بټونه_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (دلته.val == 17) {
                        NEEDBITS(دلته.بټونه + 3);
                        DROPBITS(دلته.بټونه);
                        len = 0;
                        copy = 3 + بټونه_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(دلته.بټونه + 7);
                        DROPBITS(دلته.بټونه);
                        len = 0;
                        copy = 11 + بټونه_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->پیغام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->ډېرپېښ = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بینښه لنډ)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->ډېرپېښ == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->ډېرپېښ = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->بل = state->codes;
            state->lencode = (کوډ ثابت FAR *)(state->بل);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->بل),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->distcode = (کوډ ثابت FAR *)(state->بل);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->بل), &(state->distbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->ډېرپېښ = LEN;
                /* fallthrough */

        حالة LEN:
            /* use inflate_fast() if we have enough input and output */
            إذا (have >= 6 && کیڼ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                بارول_2();
                اكسر;
            }

            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                دلته = state->lencode[بټونه_2(state->lenbits)];
                إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                PULLBYTE();
            }
            إذا (دلته.op && (دلته.op & 0xf0) == 0) {
                وروستی = دلته;
                لكل (;;) {
                    دلته = state->lencode[وروستی.val +
                            (بټونه_2(وروستی.بټونه + وروستی.op) >> وروستی.بټونه)];
                    إذا ((بینښه)(وروستی.بټونه + دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                DROPBITS(وروستی.بټونه);
            }
            DROPBITS(دلته.بټونه);
            state->اوږدتیا = (بینښه)دلته.val;

            /* process literal */
            إذا (دلته.op == 0) {
                Tracevv((stderr, دلته.val >= 0x20 && دلته.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", دلته.val));
                ځای();
                *put++ = (بینښه حرف)(state->اوږدتیا);
                کیڼ--;
                state->ډېرپېښ = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (دلته.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->ډېرپېښ = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (دلته.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                state->ډېرپېښ = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            state->extra = (بینښه)(دلته.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->اوږدتیا += بټونه_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->اوږدتیا));

            /* get distance code */
            لكل (;;) {
                دلته = state->distcode[بټونه_2(state->distbits)];
                إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                PULLBYTE();
            }
            إذا ((دلته.op & 0xf0) == 0) {
                وروستی = دلته;
                لكل (;;) {
                    دلته = state->distcode[وروستی.val +
                            (بټونه_2(وروستی.بټونه + وروستی.op) >> وروستی.بټونه)];
                    إذا ((بینښه)(وروستی.بټونه + دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                DROPBITS(وروستی.بټونه);
            }
            DROPBITS(دلته.بټونه);
            إذا (دلته.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            state->دننه_واټن = (بینښه)دلته.val;

            /* get distance extra bits, if any */
            state->extra = (بینښه)(دلته.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->دننه_واټن += بټونه_2(state->extra);
                DROPBITS(state->extra);
            }
            إذا (state->دننه_واټن > state->wsize - (state->whave < state->wsize ?
                                                کیڼ : 0)) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                state->ډېرپېښ = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->دننه_واټن));

            /* copy match from window to output */
            افعل {
                ځای();
                copy = state->wsize - state->دننه_واټن;
                إذا (copy < کیڼ) {
                    from = put + copy;
                    copy = کیڼ - copy;
                }
                وإلا {
                    from = put - state->دننه_واټن;
                    copy = کیڼ;
                }
                إذا (copy > state->اوږدتیا) copy = state->اوږدتیا;
                state->اوږدتیا -= copy;
                کیڼ -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (state->اوږدتیا != 0);
            اكسر;

        حالة بشپړ:
            /* inflate stream terminated properly */
            ret = Z_بهیر_پای_ټکی;
            اذهب inf_leave;

        حالة BAD:
            ret = Z_معلومات_ERROR;
            اذهب inf_leave;

        افتراضي:
            /* can't happen, but makes compilers happy */
            ret = Z_بهیر_ERROR;
            اذهب inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    إذا (کیڼ < state->wsize) {
        إذا (بهر(بهر_desc, state->کړکۍ, state->wsize - کیڼ) &&
            ret == Z_بهیر_پای_ټکی)
            ret = Z_BUF_ERROR;
    }
    strm->بل_دننه = بل;
    strm->avail_دننه = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_شا_پای_ټکی(z_streamp strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_بهیر_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
