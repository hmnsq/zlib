/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "infback9.h"
#ضمن "inftree9.h"
#ضمن "inflate9.h"

#تعریف_کرو WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
صحيح ZEXPORT inflateBack9Init_(z_دھارا FAR *strm, بنانشانی حرف FAR *کھڑکی,
                              ثابت حرف *نسخہ, صحيح دھارا_ناپ) {
    بنية inflate_state FAR *state;

    إذا (نسخہ == Z_NULL || نسخہ[0] != ZLIB_نسخہ[0] ||
        دھارا_ناپ != (صحيح)(حجم(z_دھارا)))
        أرجع Z_نسخہ_ERROR;
    إذا (strm == Z_NULL || کھڑکی == Z_NULL)
        أرجع Z_دھارا_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    إذا (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (بنية inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->کھڑکی = کھڑکی;
    أرجع Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#جے_تعریف_شدہ MAKEFIXED
#ضمن <stdio.h>

فراغ makefixed9(فراغ) {
    بنانشانی sym, بٹ, کم, ناپ;
    کوڈ *اگلا, *lenfix, *distfix;
    بنية inflate_state state;
    کوڈ fixed[544];

    /* literal/length table */
    sym = 0;
    طالما (sym < 144) state.lens[sym++] = 8;
    طالما (sym < 256) state.lens[sym++] = 9;
    طالما (sym < 280) state.lens[sym++] = 7;
    طالما (sym < 288) state.lens[sym++] = 8;
    اگلا = fixed;
    lenfix = اگلا;
    بٹ = 9;
    inflate_table9(LENS, state.lens, 288, &(اگلا), &(بٹ), state.work);

    /* distance table */
    sym = 0;
    طالما (sym < 32) state.lens[sym++] = 5;
    distfix = اگلا;
    بٹ = 5;
    inflate_table9(DISTS, state.lens, 32, &(اگلا), &(بٹ), state.work);

    /* write tables */
    puts("    /* inffix9.h -- table for decoding deflate64 fixed codes");
    puts("     * Generated automatically by makefixed9().");
    puts("     */");
    puts("");
    puts("    /* WARNING: this file should *not* be used by applications.");
    puts("       It is part of the implementation of this library and is");
    puts("       subject to change. Applications should only use zlib.h.");
    puts("     */");
    puts("");
    ناپ = 1U << 9;
    لکھو("    static const code lenfix[%u] = {", ناپ);
    کم = 0;
    لكل (;;) {
        إذا ((کم % 6) == 0) لکھو("\n        ");
        لکھو("{%u,%u,%d}", lenfix[کم].op, lenfix[کم].بٹ,
               lenfix[کم].val);
        إذا (++کم == ناپ) اكسر;
        حرف_لکھو(',');
    }
    puts("\n    };");
    ناپ = 1U << 5;
    لکھو("\n    static const code distfix[%u] = {", ناپ);
    کم = 0;
    لكل (;;) {
        إذا ((کم % 5) == 0) لکھو("\n        ");
        لکھو("{%u,%u,%d}", distfix[کم].op, distfix[کم].بٹ,
               distfix[کم].val);
        إذا (++کم == ناپ) اكسر;
        حرف_لکھو(',');
    }
    puts("\n    };");
}
#جے_انت /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = کھڑکی; \
            بایاں = WSIZE; \
            wrap = 1; \
            إذا (باہرلا(باہرلا_desc, put, (بنانشانی)بایاں)) { \
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
صحيح ZEXPORT inflateBack9(z_دھارا FAR *strm, وچ_func وچ, فراغ FAR *وچ_desc,
                         باہرلا_func باہرلا, فراغ FAR *باہرلا_desc) {
    بنية inflate_state FAR *state;
    z_const بنانشانی حرف FAR *اگلا;    /* next input */
    بنانشانی حرف FAR *put;     /* next output */
    بنانشانی have;              /* available input */
    بنانشانی لما بایاں;         /* available output */
    inflate_منوال منوال;          /* current inflate mode */
    صحيح lastblock;              /* true if processing last block */
    صحيح wrap;                   /* true if the window has wrapped */
    بنانشانی حرف FAR *کھڑکی;  /* allocated sliding window, if needed */
    بنانشانی لما hold;         /* bit buffer */
    بنانشانی بٹ;              /* bits in bit buffer */
    بنانشانی extra;             /* extra bits needed */
    بنانشانی لما لمائی;       /* literal or length of data to copy */
    بنانشانی لما وتھ;       /* distance back to copy string from */
    بنانشانی لما copy;         /* number of stored or match bytes to copy */
    بنانشانی حرف FAR *from;    /* where to copy match bytes from */
    کوڈ ثابت FAR *lencode;    /* starting table for length/literal codes */
    کوڈ ثابت FAR *distcode;   /* starting table for distance codes */
    بنانشانی lenbits;           /* index bits for lencode */
    بنانشانی distbits;          /* index bits for distcode */
    کوڈ ایتھے;                  /* current decoding table entry */
    کوڈ آخری;                  /* parent table entry */
    بنانشانی len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بنانشانی چھوٹا ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ضمن "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_دھارا_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیغام = Z_NULL;
    منوال = TYPE;
    lastblock = 0;
    wrap = 0;
    کھڑکی = state->کھڑکی;
    اگلا = strm->اگلا_وچ;
    have = اگلا != Z_NULL ? strm->avail_وچ : 0;
    hold = 0;
    بٹ = 0;
    put = کھڑکی;
    بایاں = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (منوال) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (lastblock) {
                BYTEBITS();
                منوال = پورا;
                اكسر;
            }
            NEEDBITS(3);
            lastblock = بٹ_2(1);
            DROPBITS(1);
            بدل (بٹ_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                منوال = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                منوال = LEN;                     /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                منوال = جدول;
                اكسر;
            حالة 3:
                strm->پیغام = (z_const حرف *)"invalid block type";
                منوال = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                منوال = BAD;
                اكسر;
            }
            لمائی = (بنانشانی)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    لمائی));
            INITBITS();

            /* copy stored block from input to output */
            طالما (لمائی != 0) {
                copy = لمائی;
                PULL();
                گنجائش();
                إذا (copy > have) copy = have;
                إذا (copy > بایاں) copy = بایاں;
                zmemcpy(put, اگلا, copy);
                have -= copy;
                اگلا += copy;
                بایاں -= copy;
                put += copy;
                لمائی -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            منوال = TYPE;
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
            إذا (state->nlen > 286) {
                strm->پیغام = (z_const حرف *)"too many length symbols";
                منوال = BAD;
                اكسر;
            }
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
            lencode = (کوڈ ثابت FAR *)(state->اگلا);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->اگلا),
                                &(lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                منوال = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    ایتھے = lencode[بٹ_2(lenbits)];
                    إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                إذا (ایتھے.val < 16) {
                    NEEDBITS(ایتھے.بٹ);
                    DROPBITS(ایتھے.بٹ);
                    state->lens[state->have++] = ایتھے.val;
                }
                وإلا {
                    إذا (ایتھے.val == 16) {
                        NEEDBITS(ایتھے.بٹ + 2);
                        DROPBITS(ایتھے.بٹ);
                        إذا (state->have == 0) {
                            strm->پیغام = (z_const حرف *)"invalid bit length repeat";
                            منوال = BAD;
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
                        strm->پیغام = (z_const حرف *)"invalid bit length repeat";
                        منوال = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بنانشانی چھوٹا)len;
                }
            }

            /* handle error breaks in while */
            إذا (منوال == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)"invalid code -- missing end-of-block";
                منوال = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->اگلا = state->codes;
            lencode = (کوڈ ثابت FAR *)(state->اگلا);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->اگلا), &(lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                منوال = BAD;
                اكسر;
            }
            distcode = (کوڈ ثابت FAR *)(state->اگلا);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->اگلا), &(distbits),
                            state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                منوال = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            منوال = LEN;

        حالة LEN:
            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                ایتھے = lencode[بٹ_2(lenbits)];
                إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                PULLBYTE();
            }
            إذا (ایتھے.op && (ایتھے.op & 0xf0) == 0) {
                آخری = ایتھے;
                لكل (;;) {
                    ایتھے = lencode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    إذا ((بنانشانی)(آخری.بٹ + ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(ایتھے.بٹ);
            لمائی = (بنانشانی)ایتھے.val;

            /* process literal */
            إذا (ایتھے.op == 0) {
                Tracevv((stderr, ایتھے.val >= 0x20 && ایتھے.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ایتھے.val));
                گنجائش();
                *put++ = (بنانشانی حرف)(لمائی);
                بایاں--;
                منوال = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (ایتھے.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                منوال = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (ایتھے.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                منوال = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            extra = (بنانشانی)(ایتھے.op) & 31;
            إذا (extra != 0) {
                NEEDBITS(extra);
                لمائی += بٹ_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", لمائی));

            /* get distance code */
            لكل (;;) {
                ایتھے = distcode[بٹ_2(distbits)];
                إذا ((بنانشانی)(ایتھے.بٹ) <= بٹ) اكسر;
                PULLBYTE();
            }
            إذا ((ایتھے.op & 0xf0) == 0) {
                آخری = ایتھے;
                لكل (;;) {
                    ایتھے = distcode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    إذا ((بنانشانی)(آخری.بٹ + ایتھے.بٹ) <= بٹ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(ایتھے.بٹ);
            إذا (ایتھے.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                منوال = BAD;
                اكسر;
            }
            وتھ = (بنانشانی)ایتھے.val;

            /* get distance extra bits, if any */
            extra = (بنانشانی)(ایتھے.op) & 15;
            إذا (extra != 0) {
                NEEDBITS(extra);
                وتھ += بٹ_2(extra);
                DROPBITS(extra);
            }
            إذا (وتھ > WSIZE - (wrap ? 0: بایاں)) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                منوال = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", وتھ));

            /* copy match from window to output */
            افعل {
                گنجائش();
                copy = WSIZE - وتھ;
                إذا (copy < بایاں) {
                    from = put + copy;
                    copy = بایاں - copy;
                }
                وإلا {
                    from = put - وتھ;
                    copy = بایاں;
                }
                إذا (copy > لمائی) copy = لمائی;
                لمائی -= copy;
                بایاں -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (لمائی != 0);
            اكسر;

        حالة پورا:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_دھارا_سرا;
            إذا (بایاں < WSIZE) {
                إذا (باہرلا(باہرلا_desc, کھڑکی, (بنانشانی)(WSIZE - بایاں)))
                    ret = Z_BUF_ERROR;
            }
            اذهب inf_leave;

        حالة BAD:
            ret = Z_انکڑے_ERROR;
            اذهب inf_leave;

        افتراضي:                /* can't happen, but makes compilers happy */
            ret = Z_دھارا_ERROR;
            اذهب inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->اگلا_وچ = اگلا;
    strm->avail_وچ = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_Back9_سرا(z_دھارا FAR *strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_دھارا_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
