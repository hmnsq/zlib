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

#شامل "zutil.h"
#شامل "inftrees.h"
#شامل "inflate.h"
#شامل "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
صحیح ZEXPORT inflate_عقب_Init_2(z_streamp strm, صحیح کھڑکی_بٹ,
                             غیرعلامتی حرف FAR *کھڑکی, ثابت حرف *نسخہ,
                             صحیح دھارا_سائز) {
    ساخت inflate_state FAR *state;

    اگر (نسخہ == Z_NULL || نسخہ[0] != ZLIB_نسخہ[0] ||
        دھارا_سائز != (صحیح)(حجم(z_دھارا)))
        واپس Z_نسخہ_ERROR;
    اگر (strm == Z_NULL || کھڑکی == Z_NULL ||
        کھڑکی_بٹ < 8 || کھڑکی_بٹ > 15)
        واپس Z_دھارا_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    اگر (strm->zalloc == (alloc_func)0) {
#اگر_تعریف_شدہ Z_SOLO
        واپس Z_دھارا_ERROR;
#ورنہ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#اگر_ختم
    }
    اگر (strm->zfree == (free_func)0)
#اگر_تعریف_شدہ Z_SOLO
        واپس Z_دھارا_ERROR;
#ورنہ
        strm->zfree = zcfree;
#اگر_ختم
    state = (ساخت inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(ساخت inflate_state));
    اگر (state == Z_NULL) واپس Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ساخت اندرونی_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)کھڑکی_بٹ;
    state->wsize = 1U << کھڑکی_بٹ;
    state->کھڑکی = کھڑکی;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    واپس Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#تعریف_کرو بھرو_2() \
    کرو { \
        put = strm->اگلا_برآمد; \
        بایاں = strm->avail_برآمد; \
        اگلا = strm->اگلا_اندر; \
        have = strm->avail_اندر; \
        hold = state->hold; \
        بٹ = state->بٹ; \
    } جب (0)

/* Set state from registers for inflate_fast() */
#تعریف_کرو RESTORE() \
    کرو { \
        strm->اگلا_برآمد = put; \
        strm->avail_برآمد = بایاں; \
        strm->اگلا_اندر = اگلا; \
        strm->avail_اندر = have; \
        state->hold = hold; \
        state->بٹ = بٹ; \
    } جب (0)

/* Clear the input bit accumulator */
#تعریف_کرو INITBITS() \
    کرو { \
        hold = 0; \
        بٹ = 0; \
    } جب (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#تعریف_کرو PULL() \
    کرو { \
        اگر (have == 0) { \
            have = اندر(اندر_desc, &اگلا); \
            اگر (have == 0) { \
                اگلا = Z_NULL; \
                ret = Z_BUF_ERROR; \
                جاؤ inf_leave; \
            } \
        } \
    } جب (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#تعریف_کرو PULLBYTE() \
    کرو { \
        PULL(); \
        have--; \
        hold += (غیرعلامتی طویل)(*اگلا++) << بٹ; \
        بٹ += 8; \
    } جب (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#تعریف_کرو NEEDBITS(n) \
    کرو { \
        جب (بٹ < (غیرعلامتی)(n)) \
            PULLBYTE(); \
    } جب (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#تعریف_کرو بٹ_2(n) \
    ((غیرعلامتی)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#تعریف_کرو DROPBITS(n) \
    کرو { \
        hold >>= (n); \
        بٹ -= (غیرعلامتی)(n); \
    } جب (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#تعریف_کرو BYTEBITS() \
    کرو { \
        hold >>= بٹ & 7; \
        بٹ -= بٹ & 7; \
    } جب (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#تعریف_کرو گنجائش() \
    کرو { \
        اگر (بایاں == 0) { \
            put = state->کھڑکی; \
            بایاں = state->wsize; \
            state->whave = بایاں; \
            اگر (برآمد(برآمد_desc, put, بایاں)) { \
                ret = Z_BUF_ERROR; \
                جاؤ inf_leave; \
            } \
        } \
    } جب (0)

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
صحیح ZEXPORT inflate_عقب(z_streamp strm, اندر_func اندر, خالی FAR *اندر_desc,
                        برآمد_func برآمد, خالی FAR *برآمد_desc) {
    ساخت inflate_state FAR *state;
    z_const غیرعلامتی حرف FAR *اگلا;    /* next input */
    غیرعلامتی حرف FAR *put;     /* next output */
    غیرعلامتی have, بایاں;        /* available input and output */
    غیرعلامتی طویل hold;         /* bit buffer */
    غیرعلامتی بٹ;              /* bits in bit buffer */
    غیرعلامتی copy;              /* number of stored or match bytes to copy */
    غیرعلامتی حرف FAR *from;    /* where to copy match bytes from */
    رمز یہاں;                  /* current decoding table entry */
    رمز آخری;                  /* parent table entry */
    غیرعلامتی len;               /* length to copy for repeats, bits to drop */
    صحیح ret;                    /* return code */
    ساکن ثابت غیرعلامتی مختصر ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    اگر (strm == Z_NULL || strm->state == Z_NULL)
        واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیغام = Z_NULL;
    state->منوال = TYPE;
    state->آخری = 0;
    state->whave = 0;
    اگلا = strm->اگلا_اندر;
    have = اگلا != Z_NULL ? strm->avail_اندر : 0;
    hold = 0;
    بٹ = 0;
    put = state->کھڑکی;
    بایاں = state->wsize;

    /* Inflate until end of block marked as last */
    برائے (;;)
        انتخاب (state->منوال) {
        حالت TYPE:
            /* determine and dispatch block type */
            اگر (state->آخری) {
                BYTEBITS();
                state->منوال = مکمل;
                توڑو;
            }
            NEEDBITS(3);
            state->آخری = بٹ_2(1);
            DROPBITS(1);
            انتخاب (بٹ_2(2)) {
            حالت 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = STORED;
                توڑو;
            حالت 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = LEN;              /* decode codes */
                توڑو;
            حالت 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخری ? " (last)" : ""));
                state->منوال = جدول;
                توڑو;
            طےشدہ:
                strm->پیغام = (z_const حرف *)"invalid block type";
                state->منوال = BAD;
            }
            DROPBITS(2);
            توڑو;

        حالت STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            اگر ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                state->منوال = BAD;
                توڑو;
            }
            state->لمبائی = (غیرعلامتی)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->لمبائی));
            INITBITS();

            /* copy stored block from input to output */
            جب (state->لمبائی != 0) {
                copy = state->لمبائی;
                PULL();
                گنجائش();
                اگر (copy > have) copy = have;
                اگر (copy > بایاں) copy = بایاں;
                zmemcpy(put, اگلا, copy);
                have -= copy;
                اگلا += copy;
                بایاں -= copy;
                put += copy;
                state->لمبائی -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->منوال = TYPE;
            توڑو;

        حالت جدول:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = بٹ_2(5) + 257;
            DROPBITS(5);
            state->ndist = بٹ_2(5) + 1;
            DROPBITS(5);
            state->ncode = بٹ_2(4) + 4;
            DROPBITS(4);
#اگر_غیر_تعریف_شدہ PKZIP_BUG_WORKAROUND
            اگر (state->nlen > 286 || state->ndist > 30) {
                strm->پیغام = (z_const حرف *)
                    "too many length or distance symbols";
                state->منوال = BAD;
                توڑو;
            }
#اگر_ختم
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            جب (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتیب[state->have++]] = (غیرعلامتی مختصر)بٹ_2(3);
                DROPBITS(3);
            }
            جب (state->have < 19)
                state->lens[ترتیب[state->have++]] = 0;
            state->اگلا = state->codes;
            state->lencode = (رمز ثابت FAR *)(state->اگلا);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->اگلا),
                                &(state->lenbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                state->منوال = BAD;
                توڑو;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            جب (state->have < state->nlen + state->ndist) {
                برائے (;;) {
                    یہاں = state->lencode[بٹ_2(state->lenbits)];
                    اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                اگر (یہاں.val < 16) {
                    DROPBITS(یہاں.بٹ);
                    state->lens[state->have++] = یہاں.val;
                }
                ورنہ {
                    اگر (یہاں.val == 16) {
                        NEEDBITS(یہاں.بٹ + 2);
                        DROPBITS(یہاں.بٹ);
                        اگر (state->have == 0) {
                            strm->پیغام = (z_const حرف *)
                                "invalid bit length repeat";
                            state->منوال = BAD;
                            توڑو;
                        }
                        len = (غیرعلامتی)(state->lens[state->have - 1]);
                        copy = 3 + بٹ_2(2);
                        DROPBITS(2);
                    }
                    ورنہ اگر (یہاں.val == 17) {
                        NEEDBITS(یہاں.بٹ + 3);
                        DROPBITS(یہاں.بٹ);
                        len = 0;
                        copy = 3 + بٹ_2(3);
                        DROPBITS(3);
                    }
                    ورنہ {
                        NEEDBITS(یہاں.بٹ + 7);
                        DROPBITS(یہاں.بٹ);
                        len = 0;
                        copy = 11 + بٹ_2(7);
                        DROPBITS(7);
                    }
                    اگر (state->have + copy > state->nlen + state->ndist) {
                        strm->پیغام = (z_const حرف *)
                            "invalid bit length repeat";
                        state->منوال = BAD;
                        توڑو;
                    }
                    جب (copy--)
                        state->lens[state->have++] = (غیرعلامتی مختصر)len;
                }
            }

            /* handle error breaks in while */
            اگر (state->منوال == BAD) توڑو;

            /* check for end-of-block code (better have one) */
            اگر (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->منوال = BAD;
                توڑو;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->اگلا = state->codes;
            state->lencode = (رمز ثابت FAR *)(state->اگلا);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->اگلا),
                                &(state->lenbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                state->منوال = BAD;
                توڑو;
            }
            state->distcode = (رمز ثابت FAR *)(state->اگلا);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->اگلا), &(state->distbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                state->منوال = BAD;
                توڑو;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->منوال = LEN;
                /* fallthrough */

        حالت LEN:
            /* use inflate_fast() if we have enough input and output */
            اگر (have >= 6 && بایاں >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                بھرو_2();
                توڑو;
            }

            /* get a literal, length, or end-of-block code */
            برائے (;;) {
                یہاں = state->lencode[بٹ_2(state->lenbits)];
                اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                PULLBYTE();
            }
            اگر (یہاں.op && (یہاں.op & 0xf0) == 0) {
                آخری = یہاں;
                برائے (;;) {
                    یہاں = state->lencode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    اگر ((غیرعلامتی)(آخری.بٹ + یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(یہاں.بٹ);
            state->لمبائی = (غیرعلامتی)یہاں.val;

            /* process literal */
            اگر (یہاں.op == 0) {
                Tracevv((stderr, یہاں.val >= 0x20 && یہاں.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", یہاں.val));
                گنجائش();
                *put++ = (غیرعلامتی حرف)(state->لمبائی);
                بایاں--;
                state->منوال = LEN;
                توڑو;
            }

            /* process end of block */
            اگر (یہاں.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->منوال = TYPE;
                توڑو;
            }

            /* invalid code */
            اگر (یہاں.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                state->منوال = BAD;
                توڑو;
            }

            /* length code -- get extra bits, if any */
            state->extra = (غیرعلامتی)(یہاں.op) & 15;
            اگر (state->extra != 0) {
                NEEDBITS(state->extra);
                state->لمبائی += بٹ_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->لمبائی));

            /* get distance code */
            برائے (;;) {
                یہاں = state->distcode[بٹ_2(state->distbits)];
                اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                PULLBYTE();
            }
            اگر ((یہاں.op & 0xf0) == 0) {
                آخری = یہاں;
                برائے (;;) {
                    یہاں = state->distcode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    اگر ((غیرعلامتی)(آخری.بٹ + یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(یہاں.بٹ);
            اگر (یہاں.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->منوال = BAD;
                توڑو;
            }
            state->انحراف = (غیرعلامتی)یہاں.val;

            /* get distance extra bits, if any */
            state->extra = (غیرعلامتی)(یہاں.op) & 15;
            اگر (state->extra != 0) {
                NEEDBITS(state->extra);
                state->انحراف += بٹ_2(state->extra);
                DROPBITS(state->extra);
            }
            اگر (state->انحراف > state->wsize - (state->whave < state->wsize ?
                                                بایاں : 0)) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                state->منوال = BAD;
                توڑو;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->انحراف));

            /* copy match from window to output */
            کرو {
                گنجائش();
                copy = state->wsize - state->انحراف;
                اگر (copy < بایاں) {
                    from = put + copy;
                    copy = بایاں - copy;
                }
                ورنہ {
                    from = put - state->انحراف;
                    copy = بایاں;
                }
                اگر (copy > state->لمبائی) copy = state->لمبائی;
                state->لمبائی -= copy;
                بایاں -= copy;
                کرو {
                    *put++ = *from++;
                } جب (--copy);
            } جب (state->لمبائی != 0);
            توڑو;

        حالت مکمل:
            /* inflate stream terminated properly */
            ret = Z_دھارا_انتہا;
            جاؤ inf_leave;

        حالت BAD:
            ret = Z_ڈیٹا_ERROR;
            جاؤ inf_leave;

        طےشدہ:
            /* can't happen, but makes compilers happy */
            ret = Z_دھارا_ERROR;
            جاؤ inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    اگر (بایاں < state->wsize) {
        اگر (برآمد(برآمد_desc, state->کھڑکی, state->wsize - بایاں) &&
            ret == Z_دھارا_انتہا)
            ret = Z_BUF_ERROR;
    }
    strm->اگلا_اندر = اگلا;
    strm->avail_اندر = have;
    واپس ret;
}

صحیح ZEXPORT inflate_عقب_انتہا(z_streamp strm) {
    اگر (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        واپس Z_دھارا_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    واپس Z_OK;
}
