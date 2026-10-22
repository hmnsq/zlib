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
صحيح ZEXPORT inflate_پٺ_Init_2(z_streamp strm, صحيح دري_بٽ,
                             بنانشان حرف FAR *دري, ثابت حرف *نسخو,
                             صحيح وهڪرو_سائيز) {
    بنية inflate_state FAR *state;

    إذا (نسخو == Z_NULL || نسخو[0] != ZLIB_نسخو[0] ||
        وهڪرو_سائيز != (صحيح)(حجم(z_وهڪرو)))
        أرجع Z_نسخو_ERROR;
    إذا (strm == Z_NULL || دري == Z_NULL ||
        دري_بٽ < 8 || دري_بٽ > 15)
        أرجع Z_وهڪرو_ERROR;
    strm->نياپو = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#جيڪڏهن_وصف_ٿيل Z_SOLO
        أرجع Z_وهڪرو_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#جيڪڏهن_پورو
    }
    إذا (strm->zfree == (free_func)0)
#جيڪڏهن_وصف_ٿيل Z_SOLO
        أرجع Z_وهڪرو_ERROR;
#وإلا
        strm->zfree = zcfree;
#جيڪڏهن_پورو
    state = (بنية inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية اندروني_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)دري_بٽ;
    state->wsize = 1U << دري_بٽ;
    state->دري = دري;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    أرجع Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#وصف_ڪريو ڀريو_2() \
    افعل { \
        put = strm->ايندڙ_ٻاهريون; \
        کاٻو = strm->avail_ٻاهريون; \
        ايندڙ = strm->ايندڙ_اندريون; \
        have = strm->avail_اندريون; \
        hold = state->hold; \
        بٽ = state->بٽ; \
    } طالما (0)

/* Set state from registers for inflate_fast() */
#وصف_ڪريو RESTORE() \
    افعل { \
        strm->ايندڙ_ٻاهريون = put; \
        strm->avail_ٻاهريون = کاٻو; \
        strm->ايندڙ_اندريون = ايندڙ; \
        strm->avail_اندريون = have; \
        state->hold = hold; \
        state->بٽ = بٽ; \
    } طالما (0)

/* Clear the input bit accumulator */
#وصف_ڪريو INITBITS() \
    افعل { \
        hold = 0; \
        بٽ = 0; \
    } طالما (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#وصف_ڪريو PULL() \
    افعل { \
        إذا (have == 0) { \
            have = اندريون(اندريون_desc, &ايندڙ); \
            إذا (have == 0) { \
                ايندڙ = Z_NULL; \
                ret = Z_BUF_ERROR; \
                اذهب inf_leave; \
            } \
        } \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#وصف_ڪريو PULLBYTE() \
    افعل { \
        PULL(); \
        have--; \
        hold += (بنانشان ڊگهو)(*ايندڙ++) << بٽ; \
        بٽ += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#وصف_ڪريو NEEDBITS(n) \
    افعل { \
        طالما (بٽ < (بنانشان)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#وصف_ڪريو بٽ_2(n) \
    ((بنانشان)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#وصف_ڪريو DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بٽ -= (بنانشان)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#وصف_ڪريو BYTEBITS() \
    افعل { \
        hold >>= بٽ & 7; \
        بٽ -= بٽ & 7; \
    } طالما (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#وصف_ڪريو گنجائش() \
    افعل { \
        إذا (کاٻو == 0) { \
            put = state->دري; \
            کاٻو = state->wsize; \
            state->whave = کاٻو; \
            إذا (ٻاهريون(ٻاهريون_desc, put, کاٻو)) { \
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
صحيح ZEXPORT inflate_پٺ(z_streamp strm, اندريون_func اندريون, فراغ FAR *اندريون_desc,
                        ٻاهريون_func ٻاهريون, فراغ FAR *ٻاهريون_desc) {
    بنية inflate_state FAR *state;
    z_const بنانشان حرف FAR *ايندڙ;    /* next input */
    بنانشان حرف FAR *put;     /* next output */
    بنانشان have, کاٻو;        /* available input and output */
    بنانشان ڊگهو hold;         /* bit buffer */
    بنانشان بٽ;              /* bits in bit buffer */
    بنانشان copy;              /* number of stored or match bytes to copy */
    بنانشان حرف FAR *from;    /* where to copy match bytes from */
    ڪوڊ هتي;                  /* current decoding table entry */
    ڪوڊ آخري;                  /* parent table entry */
    بنانشان len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بنانشان ننڍو ترتيب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->نياپو = Z_NULL;
    state->گهڻائي = TYPE;
    state->آخري = 0;
    state->whave = 0;
    ايندڙ = strm->ايندڙ_اندريون;
    have = ايندڙ != Z_NULL ? strm->avail_اندريون : 0;
    hold = 0;
    بٽ = 0;
    put = state->دري;
    کاٻو = state->wsize;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (state->گهڻائي) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (state->آخري) {
                BYTEBITS();
                state->گهڻائي = پورو;
                اكسر;
            }
            NEEDBITS(3);
            state->آخري = بٽ_2(1);
            DROPBITS(1);
            بدل (بٽ_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخري ? " (last)" : ""));
                state->گهڻائي = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخري ? " (last)" : ""));
                state->گهڻائي = LEN;              /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخري ? " (last)" : ""));
                state->گهڻائي = جدول;
                اكسر;
            افتراضي:
                strm->نياپو = (z_const حرف *)"invalid block type";
                state->گهڻائي = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->نياپو = (z_const حرف *)"invalid stored block lengths";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->ڊيگهائي = (بنانشان)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->ڊيگهائي));
            INITBITS();

            /* copy stored block from input to output */
            طالما (state->ڊيگهائي != 0) {
                copy = state->ڊيگهائي;
                PULL();
                گنجائش();
                إذا (copy > have) copy = have;
                إذا (copy > کاٻو) copy = کاٻو;
                zmemcpy(put, ايندڙ, copy);
                have -= copy;
                ايندڙ += copy;
                کاٻو -= copy;
                put += copy;
                state->ڊيگهائي -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->گهڻائي = TYPE;
            اكسر;

        حالة جدول:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = بٽ_2(5) + 257;
            DROPBITS(5);
            state->ndist = بٽ_2(5) + 1;
            DROPBITS(5);
            state->ncode = بٽ_2(4) + 4;
            DROPBITS(4);
#جيڪڏهن_وصف_نه_ٿيل PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->نياپو = (z_const حرف *)
                    "too many length or distance symbols";
                state->گهڻائي = BAD;
                اكسر;
            }
#جيڪڏهن_پورو
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتيب[state->have++]] = (بنانشان ننڍو)بٽ_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتيب[state->have++]] = 0;
            state->ايندڙ = state->codes;
            state->lencode = (ڪوڊ ثابت FAR *)(state->ايندڙ);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->ايندڙ),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid code lengths set";
                state->گهڻائي = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    هتي = state->lencode[بٽ_2(state->lenbits)];
                    إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                إذا (هتي.val < 16) {
                    DROPBITS(هتي.بٽ);
                    state->lens[state->have++] = هتي.val;
                }
                وإلا {
                    إذا (هتي.val == 16) {
                        NEEDBITS(هتي.بٽ + 2);
                        DROPBITS(هتي.بٽ);
                        إذا (state->have == 0) {
                            strm->نياپو = (z_const حرف *)
                                "invalid bit length repeat";
                            state->گهڻائي = BAD;
                            اكسر;
                        }
                        len = (بنانشان)(state->lens[state->have - 1]);
                        copy = 3 + بٽ_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (هتي.val == 17) {
                        NEEDBITS(هتي.بٽ + 3);
                        DROPBITS(هتي.بٽ);
                        len = 0;
                        copy = 3 + بٽ_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(هتي.بٽ + 7);
                        DROPBITS(هتي.بٽ);
                        len = 0;
                        copy = 11 + بٽ_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->نياپو = (z_const حرف *)
                            "invalid bit length repeat";
                        state->گهڻائي = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بنانشان ننڍو)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->گهڻائي == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->نياپو = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->گهڻائي = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->ايندڙ = state->codes;
            state->lencode = (ڪوڊ ثابت FAR *)(state->ايندڙ);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->ايندڙ),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid literal/lengths set";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->distcode = (ڪوڊ ثابت FAR *)(state->ايندڙ);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->ايندڙ), &(state->distbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid distances set";
                state->گهڻائي = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->گهڻائي = LEN;
                /* fallthrough */

        حالة LEN:
            /* use inflate_fast() if we have enough input and output */
            إذا (have >= 6 && کاٻو >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                ڀريو_2();
                اكسر;
            }

            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                هتي = state->lencode[بٽ_2(state->lenbits)];
                إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                PULLBYTE();
            }
            إذا (هتي.op && (هتي.op & 0xf0) == 0) {
                آخري = هتي;
                لكل (;;) {
                    هتي = state->lencode[آخري.val +
                            (بٽ_2(آخري.بٽ + آخري.op) >> آخري.بٽ)];
                    إذا ((بنانشان)(آخري.بٽ + هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخري.بٽ);
            }
            DROPBITS(هتي.بٽ);
            state->ڊيگهائي = (بنانشان)هتي.val;

            /* process literal */
            إذا (هتي.op == 0) {
                Tracevv((stderr, هتي.val >= 0x20 && هتي.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", هتي.val));
                گنجائش();
                *put++ = (بنانشان حرف)(state->ڊيگهائي);
                کاٻو--;
                state->گهڻائي = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (هتي.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->گهڻائي = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (هتي.op & 64) {
                strm->نياپو = (z_const حرف *)"invalid literal/length code";
                state->گهڻائي = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            state->extra = (بنانشان)(هتي.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->ڊيگهائي += بٽ_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->ڊيگهائي));

            /* get distance code */
            لكل (;;) {
                هتي = state->distcode[بٽ_2(state->distbits)];
                إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                PULLBYTE();
            }
            إذا ((هتي.op & 0xf0) == 0) {
                آخري = هتي;
                لكل (;;) {
                    هتي = state->distcode[آخري.val +
                            (بٽ_2(آخري.بٽ + آخري.op) >> آخري.بٽ)];
                    إذا ((بنانشان)(آخري.بٽ + هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخري.بٽ);
            }
            DROPBITS(هتي.بٽ);
            إذا (هتي.op & 64) {
                strm->نياپو = (z_const حرف *)"invalid distance code";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->وٿي = (بنانشان)هتي.val;

            /* get distance extra bits, if any */
            state->extra = (بنانشان)(هتي.op) & 15;
            إذا (state->extra != 0) {
                NEEDBITS(state->extra);
                state->وٿي += بٽ_2(state->extra);
                DROPBITS(state->extra);
            }
            إذا (state->وٿي > state->wsize - (state->whave < state->wsize ?
                                                کاٻو : 0)) {
                strm->نياپو = (z_const حرف *)"invalid distance too far back";
                state->گهڻائي = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->وٿي));

            /* copy match from window to output */
            افعل {
                گنجائش();
                copy = state->wsize - state->وٿي;
                إذا (copy < کاٻو) {
                    from = put + copy;
                    copy = کاٻو - copy;
                }
                وإلا {
                    from = put - state->وٿي;
                    copy = کاٻو;
                }
                إذا (copy > state->ڊيگهائي) copy = state->ڊيگهائي;
                state->ڊيگهائي -= copy;
                کاٻو -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (state->ڊيگهائي != 0);
            اكسر;

        حالة پورو:
            /* inflate stream terminated properly */
            ret = Z_وهڪرو_پڇاڙي;
            اذهب inf_leave;

        حالة BAD:
            ret = Z_معلومات_ERROR;
            اذهب inf_leave;

        افتراضي:
            /* can't happen, but makes compilers happy */
            ret = Z_وهڪرو_ERROR;
            اذهب inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    إذا (کاٻو < state->wsize) {
        إذا (ٻاهريون(ٻاهريون_desc, state->دري, state->wsize - کاٻو) &&
            ret == Z_وهڪرو_پڇاڙي)
            ret = Z_BUF_ERROR;
    }
    strm->ايندڙ_اندريون = ايندڙ;
    strm->avail_اندريون = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_پٺ_پڇاڙي(z_streamp strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_وهڪرو_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
