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
صحيح ZEXPORT inflate_پشت_Init_2(z_streamp strm, صحيح پنجره_بیتها,
                             بیعلامت حرف FAR *پنجره, ثابت حرف *نسخه,
                             صحيح جریان_اندازه) {
    بنية inflate_state FAR *state;

    إذا (نسخه == Z_NULL || نسخه[0] != ZLIB_نسخه[0] ||
        جریان_اندازه != (صحيح)(حجم(z_جریان)))
        أرجع Z_نسخه_ERROR;
    إذا (strm == Z_NULL || پنجره == Z_NULL ||
        پنجره_بیتها < 8 || پنجره_بیتها > 15)
        أرجع Z_جریان_ERROR;
    strm->پیام = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#اگر_تعریف_شده Z_SOLO
        أرجع Z_جریان_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#پایان_اگر
    }
    إذا (strm->zfree == (free_func)0)
#اگر_تعریف_شده Z_SOLO
        أرجع Z_جریان_ERROR;
#وإلا
        strm->zfree = zcfree;
#پایان_اگر
    state = (بنية inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية داخلی_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)پنجره_بیتها;
    state->wsize = 1U << پنجره_بیتها;
    state->پنجره = پنجره;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    أرجع Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#تعریف_کن بارگیری_2() \
    افعل { \
        put = strm->بعدی_بیرون; \
        چپ = strm->avail_بیرون; \
        بعدی = strm->بعدی_درون; \
        have = strm->avail_درون; \
        hold = state->hold; \
        بیتها = state->بیتها; \
    } طالما (0)

/* Set state from registers for inflate_fast() */
#تعریف_کن RESTORE() \
    افعل { \
        strm->بعدی_بیرون = put; \
        strm->avail_بیرون = چپ; \
        strm->بعدی_درون = بعدی; \
        strm->avail_درون = have; \
        state->hold = hold; \
        state->بیتها = بیتها; \
    } طالما (0)

/* Clear the input bit accumulator */
#تعریف_کن INITBITS() \
    افعل { \
        hold = 0; \
        بیتها = 0; \
    } طالما (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#تعریف_کن PULL() \
    افعل { \
        إذا (have == 0) { \
            have = درون(درون_desc, &بعدی); \
            إذا (have == 0) { \
                بعدی = Z_NULL; \
                ret = Z_BUF_ERROR; \
                اذهب inf_leave; \
            } \
        } \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#تعریف_کن PULLBYTE() \
    افعل { \
        PULL(); \
        have--; \
        hold += (بیعلامت دراز)(*بعدی++) << بیتها; \
        بیتها += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#تعریف_کن NEEDBITS(n) \
    افعل { \
        طالما (بیتها < (بیعلامت)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کن بیتها_2(n) \
    ((بیعلامت)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کن DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بیتها -= (بیعلامت)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کن BYTEBITS() \
    افعل { \
        hold >>= بیتها & 7; \
        بیتها -= بیتها & 7; \
    } طالما (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#تعریف_کن جا() \
    افعل { \
        إذا (چپ == 0) { \
            put = state->پنجره; \
            چپ = state->wsize; \
            state->whave = چپ; \
            إذا (بیرون(بیرون_desc, put, چپ)) { \
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
صحيح ZEXPORT inflate_پشت(z_streamp strm, درون_func درون, فراغ FAR *درون_desc,
                        بیرون_func بیرون, فراغ FAR *بیرون_desc) {
    بنية inflate_state FAR *state;
    z_const بیعلامت حرف FAR *بعدی;    /* next input */
    بیعلامت حرف FAR *put;     /* next output */
    بیعلامت have, چپ;        /* available input and output */
    بیعلامت دراز hold;         /* bit buffer */
    بیعلامت بیتها;              /* bits in bit buffer */
    بیعلامت copy;              /* number of stored or match bytes to copy */
    بیعلامت حرف FAR *from;    /* where to copy match bytes from */
    کد اینجا;                  /* current decoding table entry */
    کد آخرین;                  /* parent table entry */
    بیعلامت len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بیعلامت کوتاه ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیام = Z_NULL;
    state->نما = TYPE;
    state->آخرین = 0;
    state->whave = 0;
    بعدی = strm->بعدی_درون;
    have = بعدی != Z_NULL ? strm->avail_درون : 0;
    hold = 0;
    بیتها = 0;
    put = state->پنجره;
    چپ = state->wsize;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (state->نما) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (state->آخرین) {
                BYTEBITS();
                state->نما = انجامشده;
                اكسر;
            }
            NEEDBITS(3);
            state->آخرین = بیتها_2(1);
            DROPBITS(1);
            بدل (بیتها_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخرین ? " (last)" : ""));
                state->نما = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخرین ? " (last)" : ""));
                state->نما = LEN;              /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخرین ? " (last)" : ""));
                state->نما = جدول;
                اكسر;
            افتراضي:
                strm->پیام = (z_const حرف *)"invalid block type";
                state->نما = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیام = (z_const حرف *)"invalid stored block lengths";
                state->نما = BAD;
                اكسر;
            }
            state->درازی = (بیعلامت)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->درازی));
            INITBITS();

            /* copy stored block from input to output */
            طالما (state->درازی != 0) {
                copy = state->درازی;
                PULL();
                جا();
                إذا (copy > have) copy = have;
                إذا (copy > چپ) copy = چپ;
                zmemcpy(put, بعدی, copy);
                have -= copy;
                بعدی += copy;
                چپ -= copy;
                put += copy;
                state->درازی -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->نما = TYPE;
            اكسر;

        حالة جدول:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = بیتها_2(5) + 257;
            DROPBITS(5);
            state->ndist = بیتها_2(5) + 1;
            DROPBITS(5);
            state->ncode = بیتها_2(4) + 4;
            DROPBITS(4);
#اگر_تعریف_نشده PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->پیام = (z_const حرف *)
                    "too many length or distance symbols";
                state->نما = BAD;
                اكسر;
            }
#پایان_اگر
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (بیعلامت کوتاه)بیتها_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->بعدی = state->codes;
            state->lencode = (کد ثابت FAR *)(state->بعدی);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->بعدی),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid code lengths set";
                state->نما = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    اینجا = state->lencode[بیتها_2(state->lenbits)];
                    إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                إذا (اینجا.val < 16) {
                    DROPBITS(اینجا.بیتها);
                    state->lens[state->have++] = اینجا.val;
                }
                وإلا {
                    إذا (اینجا.val == 16) {
                        NEEDBITS(اینجا.بیتها + 2);
                        DROPBITS(اینجا.بیتها);
                        إذا (state->have == 0) {
                            strm->پیام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->نما = BAD;
                            اكسر;
                        }
                        len = (بیعلامت)(state->lens[state->have - 1]);
                        copy = 3 + بیتها_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (اینجا.val == 17) {
                        NEEDBITS(اینجا.بیتها + 3);
                        DROPBITS(اینجا.بیتها);
                        len = 0;
                        copy = 3 + بیتها_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(اینجا.بیتها + 7);
                        DROPBITS(اینجا.بیتها);
                        len = 0;
                        copy = 11 + بیتها_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->پیام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->نما = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بیعلامت کوتاه)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->نما == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->نما = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->بعدی = state->codes;
            state->lencode = (کد ثابت FAR *)(state->بعدی);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->بعدی),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid literal/lengths set";
                state->نما = BAD;
                اكسر;
            }
            state->distcode = (کد ثابت FAR *)(state->بعدی);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->بعدی), &(state->distbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid distances set";
                state->نما = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->نما = LEN;
                /* fallthrough */

        حالة LEN:
            /* use inflate_fast() if we have enough input and output */
            إذا (have >= 6 && چپ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                بارگیری_2();
                اكسر;
            }

            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                اینجا = state->lencode[بیتها_2(state->lenbits)];
                إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                PULLBYTE();
            }
            إذا (اینجا.op && (اینجا.op & 0xf0) == 0) {
                آخرین = اینجا;
                لكل (;;) {
                    اینجا = state->lencode[آخرین.val +
                            (بیتها_2(آخرین.بیتها + آخرین.op) >> آخرین.بیتها)];
                    إذا ((بیعلامت)(آخرین.بیتها + اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخرین.بیتها);
            }
            DROPBITS(اینجا.بیتها);
            state->درازی = (بیعلامت)اینجا.val;

            /* process literal */
            إذا (اینجا.op == 0) {
                Tracevv((stderr, اینجا.val >= 0x20 && اینجا.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", اینجا.val));
                جا();
                *put++ = (بیعلامت حرف)(state->درازی);
                چپ--;
                state->نما = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (اینجا.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->نما = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (اینجا.op & 64) {
                strm->پیام = (z_const حرف *)"invalid literal/length code";
                state->نما = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            state->extra = (بیعلامت)(اینجا.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->درازی += بیتها_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->درازی));

            /* get distance code */
            لكل (;;) {
                اینجا = state->distcode[بیتها_2(state->distbits)];
                إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                PULLBYTE();
            }
            إذا ((اینجا.op & 0xf0) == 0) {
                آخرین = اینجا;
                لكل (;;) {
                    اینجا = state->distcode[آخرین.val +
                            (بیتها_2(آخرین.بیتها + آخرین.op) >> آخرین.بیتها)];
                    إذا ((بیعلامت)(آخرین.بیتها + اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخرین.بیتها);
            }
            DROPBITS(اینجا.بیتها);
            إذا (اینجا.op & 64) {
                strm->پیام = (z_const حرف *)"invalid distance code";
                state->نما = BAD;
                اكسر;
            }
            state->جابه_جایی = (بیعلامت)اینجا.val;

            /* get distance extra bits, if any */
            state->extra = (بیعلامت)(اینجا.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->جابه_جایی += بیتها_2(state->extra);
                DROPBITS(state->extra);
            }
            إذا (state->جابه_جایی > state->wsize - (state->whave < state->wsize ?
                                                چپ : 0)) {
                strm->پیام = (z_const حرف *)"invalid distance too far back";
                state->نما = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->جابه_جایی));

            /* copy match from window to output */
            افعل {
                جا();
                copy = state->wsize - state->جابه_جایی;
                إذا (copy < چپ) {
                    from = put + copy;
                    copy = چپ - copy;
                }
                وإلا {
                    from = put - state->جابه_جایی;
                    copy = چپ;
                }
                إذا (copy > state->درازی) copy = state->درازی;
                state->درازی -= copy;
                چپ -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (state->درازی != 0);
            اكسر;

        حالة انجامشده:
            /* inflate stream terminated properly */
            ret = Z_جریان_انتها;
            اذهب inf_leave;

        حالة BAD:
            ret = Z_داده_ERROR;
            اذهب inf_leave;

        افتراضي:
            /* can't happen, but makes compilers happy */
            ret = Z_جریان_ERROR;
            اذهب inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    إذا (چپ < state->wsize) {
        إذا (بیرون(بیرون_desc, state->پنجره, state->wsize - چپ) &&
            ret == Z_جریان_انتها)
            ret = Z_BUF_ERROR;
    }
    strm->بعدی_درون = بعدی;
    strm->avail_درون = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_پشت_انتها(z_streamp strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_جریان_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
