/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "infback9.h"
#ضمن "inftree9.h"
#ضمن "inflate9.h"

#تعریف_کن WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
صحيح ZEXPORT inflateBack9Init_(z_جریان FAR *strm, بیعلامت حرف FAR *پنجره,
                              ثابت حرف *نسخه, صحيح جریان_اندازه) {
    بنية inflate_state FAR *state;

    إذا (نسخه == Z_NULL || نسخه[0] != ZLIB_نسخه[0] ||
        جریان_اندازه != (صحيح)(حجم(z_جریان)))
        أرجع Z_نسخه_ERROR;
    إذا (strm == Z_NULL || پنجره == Z_NULL)
        أرجع Z_جریان_ERROR;
    strm->پیام = Z_NULL;                 /* in case we return an error */
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
    state->پنجره = پنجره;
    أرجع Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#اگر_تعریف_شده MAKEFIXED
#ضمن <stdio.h>

فراغ makefixed9(فراغ) {
    بیعلامت sym, بیتها, کم, اندازه;
    کد *بعدی, *lenfix, *distfix;
    بنية inflate_state state;
    کد fixed[544];

    /* literal/length table */
    sym = 0;
    طالما (sym < 144) state.lens[sym++] = 8;
    طالما (sym < 256) state.lens[sym++] = 9;
    طالما (sym < 280) state.lens[sym++] = 7;
    طالما (sym < 288) state.lens[sym++] = 8;
    بعدی = fixed;
    lenfix = بعدی;
    بیتها = 9;
    inflate_table9(LENS, state.lens, 288, &(بعدی), &(بیتها), state.work);

    /* distance table */
    sym = 0;
    طالما (sym < 32) state.lens[sym++] = 5;
    distfix = بعدی;
    بیتها = 5;
    inflate_table9(DISTS, state.lens, 32, &(بعدی), &(بیتها), state.work);

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
    اندازه = 1U << 9;
    چاپ_کن("    static const code lenfix[%u] = {", اندازه);
    کم = 0;
    لكل (;;) {
        إذا ((کم % 6) == 0) چاپ_کن("\n        ");
        چاپ_کن("{%u,%u,%d}", lenfix[کم].op, lenfix[کم].بیتها,
               lenfix[کم].val);
        إذا (++کم == اندازه) اكسر;
        حرف_چاپ_کن(',');
    }
    puts("\n    };");
    اندازه = 1U << 5;
    چاپ_کن("\n    static const code distfix[%u] = {", اندازه);
    کم = 0;
    لكل (;;) {
        إذا ((کم % 5) == 0) چاپ_کن("\n        ");
        چاپ_کن("{%u,%u,%d}", distfix[کم].op, distfix[کم].بیتها,
               distfix[کم].val);
        إذا (++کم == اندازه) اكسر;
        حرف_چاپ_کن(',');
    }
    puts("\n    };");
}
#پایان_اگر /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = پنجره; \
            چپ = WSIZE; \
            wrap = 1; \
            إذا (بیرون(بیرون_desc, put, (بیعلامت)چپ)) { \
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
صحيح ZEXPORT inflateBack9(z_جریان FAR *strm, درون_func درون, فراغ FAR *درون_desc,
                         بیرون_func بیرون, فراغ FAR *بیرون_desc) {
    بنية inflate_state FAR *state;
    z_const بیعلامت حرف FAR *بعدی;    /* next input */
    بیعلامت حرف FAR *put;     /* next output */
    بیعلامت have;              /* available input */
    بیعلامت دراز چپ;         /* available output */
    inflate_نما نما;          /* current inflate mode */
    صحيح lastblock;              /* true if processing last block */
    صحيح wrap;                   /* true if the window has wrapped */
    بیعلامت حرف FAR *پنجره;  /* allocated sliding window, if needed */
    بیعلامت دراز hold;         /* bit buffer */
    بیعلامت بیتها;              /* bits in bit buffer */
    بیعلامت extra;             /* extra bits needed */
    بیعلامت دراز درازی;       /* literal or length of data to copy */
    بیعلامت دراز جابه_جایی;       /* distance back to copy string from */
    بیعلامت دراز copy;         /* number of stored or match bytes to copy */
    بیعلامت حرف FAR *from;    /* where to copy match bytes from */
    کد ثابت FAR *lencode;    /* starting table for length/literal codes */
    کد ثابت FAR *distcode;   /* starting table for distance codes */
    بیعلامت lenbits;           /* index bits for lencode */
    بیعلامت distbits;          /* index bits for distcode */
    کد اینجا;                  /* current decoding table entry */
    کد آخرین;                  /* parent table entry */
    بیعلامت len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بیعلامت کوتاه ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ضمن "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_جریان_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیام = Z_NULL;
    نما = TYPE;
    lastblock = 0;
    wrap = 0;
    پنجره = state->پنجره;
    بعدی = strm->بعدی_درون;
    have = بعدی != Z_NULL ? strm->avail_درون : 0;
    hold = 0;
    بیتها = 0;
    put = پنجره;
    چپ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (نما) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (lastblock) {
                BYTEBITS();
                نما = انجامشده;
                اكسر;
            }
            NEEDBITS(3);
            lastblock = بیتها_2(1);
            DROPBITS(1);
            بدل (بیتها_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                نما = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                نما = LEN;                     /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                نما = جدول;
                اكسر;
            حالة 3:
                strm->پیام = (z_const حرف *)"invalid block type";
                نما = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیام = (z_const حرف *)"invalid stored block lengths";
                نما = BAD;
                اكسر;
            }
            درازی = (بیعلامت)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    درازی));
            INITBITS();

            /* copy stored block from input to output */
            طالما (درازی != 0) {
                copy = درازی;
                PULL();
                جا();
                إذا (copy > have) copy = have;
                إذا (copy > چپ) copy = چپ;
                zmemcpy(put, بعدی, copy);
                have -= copy;
                بعدی += copy;
                چپ -= copy;
                put += copy;
                درازی -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            نما = TYPE;
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
            إذا (state->nlen > 286) {
                strm->پیام = (z_const حرف *)"too many length symbols";
                نما = BAD;
                اكسر;
            }
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
            lencode = (کد ثابت FAR *)(state->بعدی);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->بعدی),
                                &(lenbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid code lengths set";
                نما = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    اینجا = lencode[بیتها_2(lenbits)];
                    إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                إذا (اینجا.val < 16) {
                    NEEDBITS(اینجا.بیتها);
                    DROPBITS(اینجا.بیتها);
                    state->lens[state->have++] = اینجا.val;
                }
                وإلا {
                    إذا (اینجا.val == 16) {
                        NEEDBITS(اینجا.بیتها + 2);
                        DROPBITS(اینجا.بیتها);
                        إذا (state->have == 0) {
                            strm->پیام = (z_const حرف *)"invalid bit length repeat";
                            نما = BAD;
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
                        strm->پیام = (z_const حرف *)"invalid bit length repeat";
                        نما = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بیعلامت کوتاه)len;
                }
            }

            /* handle error breaks in while */
            إذا (نما == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیام = (z_const حرف *)"invalid code -- missing end-of-block";
                نما = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->بعدی = state->codes;
            lencode = (کد ثابت FAR *)(state->بعدی);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->بعدی), &(lenbits), state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid literal/lengths set";
                نما = BAD;
                اكسر;
            }
            distcode = (کد ثابت FAR *)(state->بعدی);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->بعدی), &(distbits),
                            state->work);
            إذا (ret) {
                strm->پیام = (z_const حرف *)"invalid distances set";
                نما = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            نما = LEN;

        حالة LEN:
            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                اینجا = lencode[بیتها_2(lenbits)];
                إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                PULLBYTE();
            }
            إذا (اینجا.op && (اینجا.op & 0xf0) == 0) {
                آخرین = اینجا;
                لكل (;;) {
                    اینجا = lencode[آخرین.val +
                            (بیتها_2(آخرین.بیتها + آخرین.op) >> آخرین.بیتها)];
                    إذا ((بیعلامت)(آخرین.بیتها + اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخرین.بیتها);
            }
            DROPBITS(اینجا.بیتها);
            درازی = (بیعلامت)اینجا.val;

            /* process literal */
            إذا (اینجا.op == 0) {
                Tracevv((stderr, اینجا.val >= 0x20 && اینجا.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", اینجا.val));
                جا();
                *put++ = (بیعلامت حرف)(درازی);
                چپ--;
                نما = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (اینجا.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                نما = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (اینجا.op & 64) {
                strm->پیام = (z_const حرف *)"invalid literal/length code";
                نما = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            extra = (بیعلامت)(اینجا.op) & 31;
            إذا (extra != 0) {
                NEEDBITS(extra);
                درازی += بیتها_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", درازی));

            /* get distance code */
            لكل (;;) {
                اینجا = distcode[بیتها_2(distbits)];
                إذا ((بیعلامت)(اینجا.بیتها) <= بیتها) اكسر;
                PULLBYTE();
            }
            إذا ((اینجا.op & 0xf0) == 0) {
                آخرین = اینجا;
                لكل (;;) {
                    اینجا = distcode[آخرین.val +
                            (بیتها_2(آخرین.بیتها + آخرین.op) >> آخرین.بیتها)];
                    إذا ((بیعلامت)(آخرین.بیتها + اینجا.بیتها) <= بیتها) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخرین.بیتها);
            }
            DROPBITS(اینجا.بیتها);
            إذا (اینجا.op & 64) {
                strm->پیام = (z_const حرف *)"invalid distance code";
                نما = BAD;
                اكسر;
            }
            جابه_جایی = (بیعلامت)اینجا.val;

            /* get distance extra bits, if any */
            extra = (بیعلامت)(اینجا.op) & 15;
            إذا (extra != 0) {
                NEEDBITS(extra);
                جابه_جایی += بیتها_2(extra);
                DROPBITS(extra);
            }
            إذا (جابه_جایی > WSIZE - (wrap ? 0: چپ)) {
                strm->پیام = (z_const حرف *)"invalid distance too far back";
                نما = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", جابه_جایی));

            /* copy match from window to output */
            افعل {
                جا();
                copy = WSIZE - جابه_جایی;
                إذا (copy < چپ) {
                    from = put + copy;
                    copy = چپ - copy;
                }
                وإلا {
                    from = put - جابه_جایی;
                    copy = چپ;
                }
                إذا (copy > درازی) copy = درازی;
                درازی -= copy;
                چپ -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (درازی != 0);
            اكسر;

        حالة انجامشده:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_جریان_انتها;
            إذا (چپ < WSIZE) {
                إذا (بیرون(بیرون_desc, پنجره, (بیعلامت)(WSIZE - چپ)))
                    ret = Z_BUF_ERROR;
            }
            اذهب inf_leave;

        حالة BAD:
            ret = Z_داده_ERROR;
            اذهب inf_leave;

        افتراضي:                /* can't happen, but makes compilers happy */
            ret = Z_جریان_ERROR;
            اذهب inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->بعدی_درون = بعدی;
    strm->avail_درون = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_Back9_انتها(z_جریان FAR *strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_جریان_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
