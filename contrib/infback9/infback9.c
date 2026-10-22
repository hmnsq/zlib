/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "infback9.h"
#ضمن "inftree9.h"
#ضمن "inflate9.h"

#تعریف_کړه WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
صحيح ZEXPORT inflateBack9Init_(z_بهیر FAR *strm, بینښه حرف FAR *کړکۍ,
                              ثابت حرف *نسخه, صحيح بهیر_اندازه) {
    بنية inflate_state FAR *state;

    إذا (نسخه == Z_NULL || نسخه[0] != ZLIB_نسخه[0] ||
        بهیر_اندازه != (صحيح)(حجم(z_بهیر)))
        أرجع Z_نسخه_ERROR;
    إذا (strm == Z_NULL || کړکۍ == Z_NULL)
        أرجع Z_بهیر_ERROR;
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
    state->کړکۍ = کړکۍ;
    أرجع Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#که_تعریف_شوی MAKEFIXED
#ضمن <stdio.h>

فراغ makefixed9(فراغ) {
    بینښه sym, بټونه, ټیټ, اندازه;
    کوډ *بل, *lenfix, *distfix;
    بنية inflate_state state;
    کوډ fixed[544];

    /* literal/length table */
    sym = 0;
    طالما (sym < 144) state.lens[sym++] = 8;
    طالما (sym < 256) state.lens[sym++] = 9;
    طالما (sym < 280) state.lens[sym++] = 7;
    طالما (sym < 288) state.lens[sym++] = 8;
    بل = fixed;
    lenfix = بل;
    بټونه = 9;
    inflate_table9(LENS, state.lens, 288, &(بل), &(بټونه), state.work);

    /* distance table */
    sym = 0;
    طالما (sym < 32) state.lens[sym++] = 5;
    distfix = بل;
    بټونه = 5;
    inflate_table9(DISTS, state.lens, 32, &(بل), &(بټونه), state.work);

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
    چاپ_کړه("    static const code lenfix[%u] = {", اندازه);
    ټیټ = 0;
    لكل (;;) {
        إذا ((ټیټ % 6) == 0) چاپ_کړه("\n        ");
        چاپ_کړه("{%u,%u,%d}", lenfix[ټیټ].op, lenfix[ټیټ].بټونه,
               lenfix[ټیټ].val);
        إذا (++ټیټ == اندازه) اكسر;
        توری_چاپ_کړه(',');
    }
    puts("\n    };");
    اندازه = 1U << 5;
    چاپ_کړه("\n    static const code distfix[%u] = {", اندازه);
    ټیټ = 0;
    لكل (;;) {
        إذا ((ټیټ % 5) == 0) چاپ_کړه("\n        ");
        چاپ_کړه("{%u,%u,%d}", distfix[ټیټ].op, distfix[ټیټ].بټونه,
               distfix[ټیټ].val);
        إذا (++ټیټ == اندازه) اكسر;
        توری_چاپ_کړه(',');
    }
    puts("\n    };");
}
#د_که_پای /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = کړکۍ; \
            کیڼ = WSIZE; \
            wrap = 1; \
            إذا (بهر(بهر_desc, put, (بینښه)کیڼ)) { \
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
صحيح ZEXPORT inflateBack9(z_بهیر FAR *strm, دننه_func دننه, فراغ FAR *دننه_desc,
                         بهر_func بهر, فراغ FAR *بهر_desc) {
    بنية inflate_state FAR *state;
    z_const بینښه حرف FAR *بل;    /* next input */
    بینښه حرف FAR *put;     /* next output */
    بینښه have;              /* available input */
    بینښه اوږد کیڼ;         /* available output */
    inflate_ډېرپېښ ډېرپېښ;          /* current inflate mode */
    صحيح lastblock;              /* true if processing last block */
    صحيح wrap;                   /* true if the window has wrapped */
    بینښه حرف FAR *کړکۍ;  /* allocated sliding window, if needed */
    بینښه اوږد hold;         /* bit buffer */
    بینښه بټونه;              /* bits in bit buffer */
    بینښه extra;             /* extra bits needed */
    بینښه اوږد اوږدتیا;       /* literal or length of data to copy */
    بینښه اوږد دننه_واټن;       /* distance back to copy string from */
    بینښه اوږد copy;         /* number of stored or match bytes to copy */
    بینښه حرف FAR *from;    /* where to copy match bytes from */
    کوډ ثابت FAR *lencode;    /* starting table for length/literal codes */
    کوډ ثابت FAR *distcode;   /* starting table for distance codes */
    بینښه lenbits;           /* index bits for lencode */
    بینښه distbits;          /* index bits for distcode */
    کوډ دلته;                  /* current decoding table entry */
    کوډ وروستی;                  /* parent table entry */
    بینښه len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بینښه لنډ ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ضمن "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_بهیر_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیغام = Z_NULL;
    ډېرپېښ = TYPE;
    lastblock = 0;
    wrap = 0;
    کړکۍ = state->کړکۍ;
    بل = strm->بل_دننه;
    have = بل != Z_NULL ? strm->avail_دننه : 0;
    hold = 0;
    بټونه = 0;
    put = کړکۍ;
    کیڼ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (ډېرپېښ) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (lastblock) {
                BYTEBITS();
                ډېرپېښ = بشپړ;
                اكسر;
            }
            NEEDBITS(3);
            lastblock = بټونه_2(1);
            DROPBITS(1);
            بدل (بټونه_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                ډېرپېښ = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                ډېرپېښ = LEN;                     /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                ډېرپېښ = جدول;
                اكسر;
            حالة 3:
                strm->پیغام = (z_const حرف *)"invalid block type";
                ډېرپېښ = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                ډېرپېښ = BAD;
                اكسر;
            }
            اوږدتیا = (بینښه)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    اوږدتیا));
            INITBITS();

            /* copy stored block from input to output */
            طالما (اوږدتیا != 0) {
                copy = اوږدتیا;
                PULL();
                ځای();
                إذا (copy > have) copy = have;
                إذا (copy > کیڼ) copy = کیڼ;
                zmemcpy(put, بل, copy);
                have -= copy;
                بل += copy;
                کیڼ -= copy;
                put += copy;
                اوږدتیا -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            ډېرپېښ = TYPE;
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
            إذا (state->nlen > 286) {
                strm->پیغام = (z_const حرف *)"too many length symbols";
                ډېرپېښ = BAD;
                اكسر;
            }
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
            lencode = (کوډ ثابت FAR *)(state->بل);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->بل),
                                &(lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                ډېرپېښ = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    دلته = lencode[بټونه_2(lenbits)];
                    إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                إذا (دلته.val < 16) {
                    NEEDBITS(دلته.بټونه);
                    DROPBITS(دلته.بټونه);
                    state->lens[state->have++] = دلته.val;
                }
                وإلا {
                    إذا (دلته.val == 16) {
                        NEEDBITS(دلته.بټونه + 2);
                        DROPBITS(دلته.بټونه);
                        إذا (state->have == 0) {
                            strm->پیغام = (z_const حرف *)"invalid bit length repeat";
                            ډېرپېښ = BAD;
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
                        strm->پیغام = (z_const حرف *)"invalid bit length repeat";
                        ډېرپېښ = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بینښه لنډ)len;
                }
            }

            /* handle error breaks in while */
            إذا (ډېرپېښ == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)"invalid code -- missing end-of-block";
                ډېرپېښ = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->بل = state->codes;
            lencode = (کوډ ثابت FAR *)(state->بل);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->بل), &(lenbits), state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                ډېرپېښ = BAD;
                اكسر;
            }
            distcode = (کوډ ثابت FAR *)(state->بل);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->بل), &(distbits),
                            state->work);
            إذا (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                ډېرپېښ = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            ډېرپېښ = LEN;

        حالة LEN:
            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                دلته = lencode[بټونه_2(lenbits)];
                إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                PULLBYTE();
            }
            إذا (دلته.op && (دلته.op & 0xf0) == 0) {
                وروستی = دلته;
                لكل (;;) {
                    دلته = lencode[وروستی.val +
                            (بټونه_2(وروستی.بټونه + وروستی.op) >> وروستی.بټونه)];
                    إذا ((بینښه)(وروستی.بټونه + دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                DROPBITS(وروستی.بټونه);
            }
            DROPBITS(دلته.بټونه);
            اوږدتیا = (بینښه)دلته.val;

            /* process literal */
            إذا (دلته.op == 0) {
                Tracevv((stderr, دلته.val >= 0x20 && دلته.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", دلته.val));
                ځای();
                *put++ = (بینښه حرف)(اوږدتیا);
                کیڼ--;
                ډېرپېښ = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (دلته.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                ډېرپېښ = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (دلته.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                ډېرپېښ = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            extra = (بینښه)(دلته.op) & 31;
            إذا (extra != 0) {
                NEEDBITS(extra);
                اوږدتیا += بټونه_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", اوږدتیا));

            /* get distance code */
            لكل (;;) {
                دلته = distcode[بټونه_2(distbits)];
                إذا ((بینښه)(دلته.بټونه) <= بټونه) اكسر;
                PULLBYTE();
            }
            إذا ((دلته.op & 0xf0) == 0) {
                وروستی = دلته;
                لكل (;;) {
                    دلته = distcode[وروستی.val +
                            (بټونه_2(وروستی.بټونه + وروستی.op) >> وروستی.بټونه)];
                    إذا ((بینښه)(وروستی.بټونه + دلته.بټونه) <= بټونه) اكسر;
                    PULLBYTE();
                }
                DROPBITS(وروستی.بټونه);
            }
            DROPBITS(دلته.بټونه);
            إذا (دلته.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                ډېرپېښ = BAD;
                اكسر;
            }
            دننه_واټن = (بینښه)دلته.val;

            /* get distance extra bits, if any */
            extra = (بینښه)(دلته.op) & 15;
            إذا (extra != 0) {
                NEEDBITS(extra);
                دننه_واټن += بټونه_2(extra);
                DROPBITS(extra);
            }
            إذا (دننه_واټن > WSIZE - (wrap ? 0: کیڼ)) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                ډېرپېښ = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", دننه_واټن));

            /* copy match from window to output */
            افعل {
                ځای();
                copy = WSIZE - دننه_واټن;
                إذا (copy < کیڼ) {
                    from = put + copy;
                    copy = کیڼ - copy;
                }
                وإلا {
                    from = put - دننه_واټن;
                    copy = کیڼ;
                }
                إذا (copy > اوږدتیا) copy = اوږدتیا;
                اوږدتیا -= copy;
                کیڼ -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (اوږدتیا != 0);
            اكسر;

        حالة بشپړ:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_بهیر_پای_ټکی;
            إذا (کیڼ < WSIZE) {
                إذا (بهر(بهر_desc, کړکۍ, (بینښه)(WSIZE - کیڼ)))
                    ret = Z_BUF_ERROR;
            }
            اذهب inf_leave;

        حالة BAD:
            ret = Z_معلومات_ERROR;
            اذهب inf_leave;

        افتراضي:                /* can't happen, but makes compilers happy */
            ret = Z_بهیر_ERROR;
            اذهب inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->بل_دننه = بل;
    strm->avail_دننه = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_Back9_پای_ټکی(z_بهیر FAR *strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_بهیر_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
