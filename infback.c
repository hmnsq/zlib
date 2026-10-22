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
صحيح ZEXPORT inflate_پٹھ_Init_2(z_streamp strm, صحيح کھڑکی_بٹ,
                             بنانشانی حرف FAR *کھڑکی, ثابت حرف *نسخہ,
                             صحيح دھارا_ناپ) {
    بنية inflate_state FAR *state;

    إذا (نسخہ == Z_NULL || نسخہ[0] != ZLIB_نسخہ[0] ||
        دھارا_ناپ != (صحيح)(حجم(z_دھارا)))
        أرجع Z_نسخہ_ERROR;
    إذا (strm == Z_NULL || کھڑکی == Z_NULL ||
        کھڑکی_بٹ < 8 || کھڑکی_بٹ > 15)
        أرجع Z_دھارا_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#جے_تعریف_شدہ Z_SOLO
        أرجع Z_دھارا_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#جے_انت
    }
    إذا (strm->zfree == (free_func)0)
#جے_تعریف_شدہ Z_SOLO
        أرجع Z_دھارا_ERROR;
#وإلا
        strm->zfree = zcfree;
#جے_انت
    state = (بنية inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية اندرونی_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)کھڑکی_بٹ;
    state->wsize = 1U << کھڑکی_بٹ;
    state->کھڑکی = کھڑکی;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    أرجع Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#تعریف_کرو لدو_2() \
    افعل { \
        put = strm->اگلا_باہرلا; \
        بایاں = strm->avail_باہرلا; \
        اگلا = strm->اگلا_وچ; \
        have = strm->avail_وچ; \
        hold = state->hold; \
        بٹ = state->بٹ; \
    } طالما (0)

/* Set state from registers for inflate_fast() */
#تعریف_کرو RESTORE() \
    افعل { \
        strm->اگلا_باہرلا = put; \
        strm->avail_باہرلا = بایاں; \
        strm->اگلا_وچ = اگلا; \
        strm->avail_وچ = have; \
        state->hold = hold; \
        state->بٹ = بٹ; \
    } طالما (0)

/* Clear the input bit accumulator */
#تعریف_کرو INITBITS() \
    افعل { \
        hold = 0; \
        بٹ = 0; \
    } طالما (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#تعریف_کرو PULL() \
    افعل { \
        إذا (have == 0) { \
            have = وچ(وچ_desc, &اگلا); \
            إذا (have == 0) { \
                اگلا = Z_NULL; \
                ret = Z_BUF_ERROR; \
                اذهب inf_leave; \
            } \
        } \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#تعریف_کرو PULLBYTE() \
    افعل { \
        PULL(); \
        have--; \
        hold += (بنانشانی لما)(*اگلا++) << بٹ; \
        بٹ += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#تعریف_کرو NEEDBITS(n) \
    افعل { \
        طالما (بٹ < (بنانشانی)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کرو بٹ_2(n) \
    ((بنانشانی)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کرو DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بٹ -= (بنانشانی)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کرو BYTEBITS() \
    افعل { \
        hold >>= بٹ & 7; \
        بٹ -= بٹ & 7; \
    } طالما (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#تعریف_کرو گنجائش() \
    افعل { \
        إذا (بایاں == 0) { \
            put = state->کھڑکی; \
            بایاں = state->wsize; \
            state->whave = بایاں; \
            إذا (باہرلا(باہرلا_desc, put, بایاں)) { \
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
صحيح ZEXPORT inflate_پٹھ(z_streamp strm, وچ_func وچ, فراغ FAR *وچ_desc,
                        باہرلا_func باہرلا, فراغ FAR *باہرلا_desc) {
    بنية inflate_state FAR *state;
    z_const بنانشانی حرف FAR *اگلا;    /* next input */
    بنانشانی حرف FAR *put;     /* next output */
    بنانشانی have, بایاں;        /* available input and output */
    بنانشانی لما hold;         /* bit buffer */
    بنانشانی بٹ;              /* bits in bit buffer */
    بنانشانی copy;              /* number of stored or match bytes to copy */
    بنانشانی حرف FAR *from;    /* where to copy match bytes from */
    کوڈ ایتھے;                  /* current decoding table entry */
    کوڈ آخری;                  /* parent table entry */
    بنانشانی len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بنانشانی چھوٹا ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیغام = Z_NULL;
    state->منوال = TYPE;
    state->آخری = 0;
    state->whave = 0;
    اگلا = strm->اگلا_وچ;
    have = اگلا != Z_NULL ? strm->avail_وچ : 0;
    hold = 0;
    بٹ = 0;
    put = state->کھڑکی;
    بایاں = state->wsize;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (state->منوال) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (state->آخری) {
                BYTEBITS();
                state->منوال = پورا;
                اكسر;
            }
            NEEDBITS(3);
            state->آخری = بٹ_2(1);
            DROPBITS(1);
            بدل (بٹ_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = LEN;              /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = جدول;
                اكسر;
            افتراضي:
                strm->پیغام = (z_const حرف *)"invalid block type";
                state->منوال = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                state->منوال = BAD;
                اكسر;
            }
            state->لمائی = (بنانشانی)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->لمائی));
            INITBITS();

            /* copy stored block from input to output */
            طالما (state->لمائی != 0) {
                copy = state->لمائی;
                PULL();
                گنجائش();
                إذا (copy > have) copy = have;
                إذا (copy > بایاں) copy = بایاں;
                zmemcpy(put, اگلا, copy);
                have -= copy;
                اگلا += copy;
                بایاں -= copy;
                put += copy;
                state->لمائی -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->منوال = TYPE;
            اكسر;

        حالة جدول:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = بٹ_2(5) + 257;
            DROPBITS(5);
            state->ndist = بٹ_2(5) + 1;
            DROPBITS(5);
            state->ncode = بٹ_2(4) + 4;
            DROPBITS(4);
#جے_بنا_تعریف PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->پیغام = (z_const حرف *)
                    "too many length or distance symbols";
                state->منوال = BAD;
                اكسر;
            }
#جے_انت
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (بنانشانی چھوٹا)بٹ_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->اگلا = state->codes;
            state->lencode = (کوڈ ثابت FAR *)(state->اگلا);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->اگلا),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                state->منوال = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    ایتھے = state->lencode[بٹ_2(state->lenbits)];
                    إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                إذا (ایتھے.val < 16) {
                    DROPBITS(ایتھے.بٹ);
                    state->lens[state->have++] = ایتھے.val;
                }
                وإلا {
                    إذا (ایتھے.val == 16) {
                        NEEDBITS(ایتھے.بٹ + 2);
                        DROPBITS(ایتھے.بٹ);
                        إذا (state->have == 0) {
                            strm->پیغام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->منوال = BAD;
                            اكسر;
                        }
                        len = (بنانشانی)(state->lens[state->have - 1]);
                        copy = 3 + بٹ_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (ایتھے.val == 17) {
                        NEEDBITS(ایتھے.بٹ + 3);
                        DROPBITS(ایتھے.بٹ);
                        len = 0;
                        copy = 3 + بٹ_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(ایتھے.بٹ + 7);
                        DROPBITS(ایتھے.بٹ);
                        len = 0;
                        copy = 11 + بٹ_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->پیغام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->منوال = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بنانشانی چھوٹا)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->منوال == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->منوال = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->اگلا = state->codes;
            state->lencode = (کوڈ ثابت FAR *)(state->اگلا);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->اگلا),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                state->منوال = BAD;
                اكسر;
            }
            state->distcode = (کوڈ ثابت FAR *)(state->اگلا);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->اگلا), &(state->distbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                state->منوال = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->منوال = LEN;
                /* fallthrough */

        حالة LEN:
            /* use inflate_fast() if we have enough input and output */
            إذا (have >= 6 && بایاں >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                لدو_2();
                اكسر;
            }

            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                ایتھے = state->lencode[بٹ_2(state->lenbits)];
                إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                PULLBYTE();
            }
            إذا (ایتھے.op && (ایتھے.op & 0xf0) == 0) {
                آخری = ایتھے;
                لكل (;;) {
                    ایتھے = state->lencode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    إذا ((بنانشانی)(آخری.بٹ + ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(ایتھے.بٹ);
            state->لمائی = (بنانشانی)ایتھے.val;

            /* process literal */
            إذا (ایتھے.op == 0) {
                Tracevv((stderr, ایتھے.val >= 0x20 && ایتھے.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ایتھے.val));
                گنجائش();
                *put++ = (بنانشانی حرف)(state->لمائی);
                بایاں--;
                state->منوال = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (ایتھے.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->منوال = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (ایتھے.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                state->منوال = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            state->extra = (بنانشانی)(ایتھے.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->لمائی += بٹ_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->لمائی));

            /* get distance code */
            لكل (;;) {
                ایتھے = state->distcode[بٹ_2(state->distbits)];
                إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                PULLBYTE();
            }
            إذا ((ایتھے.op & 0xf0) == 0) {
                آخری = ایتھے;
                لكل (;;) {
                    ایتھے = state->distcode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    إذا ((بنانشانی)(آخری.بٹ + ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(ایتھے.بٹ);
            إذا (ایتھے.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->منوال = BAD;
                اكسر;
            }
            state->وتھ = (بنانشانی)ایتھے.val;

            /* get distance extra bits, if any */
            state->extra = (بنانشانی)(ایتھے.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->وتھ += بٹ_2(state->extra);
                DROPBITS(state->extra);
            }
            إذا (state->وتھ > state->wsize - (state->whave < state->wsize ?
                                                بایاں : 0)) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                state->منوال = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->وتھ));

            /* copy match from window to output */
            افعل {
                گنجائش();
                copy = state->wsize - state->وتھ;
                إذا (copy < بایاں) {
                    from = put + copy;
                    copy = بایاں - copy;
                }
                وإلا {
                    from = put - state->وتھ;
                    copy = بایاں;
                }
                إذا (copy > state->لمائی) copy = state->لمائی;
                state->لمائی -= copy;
                بایاں -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (state->لمائی != 0);
            اكسر;

        حالة پورا:
            /* inflate stream terminated properly */
            ret = Z_دھارا_سرا;
            اذهب inf_leave;

        حالة BAD:
            ret = Z_انکڑے_ERROR;
            اذهب inf_leave;

        افتراضي:
            /* can't happen, but makes compilers happy */
            ret = Z_دھارا_ERROR;
            اذهب inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    إذا (بایاں < state->wsize) {
        إذا (باہرلا(باہرلا_desc, state->کھڑکی, state->wsize - بایاں) &&
            ret == Z_دھارا_سرا)
            ret = Z_BUF_ERROR;
    }
    strm->اگلا_وچ = اگلا;
    strm->avail_وچ = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_پٹھ_سرا(z_streamp strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_دھارا_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
