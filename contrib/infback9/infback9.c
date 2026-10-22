/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "infback9.h"
#ضمن "inftree9.h"
#ضمن "inflate9.h"

#وصف_ڪريو WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
صحيح ZEXPORT inflateBack9Init_(z_وهڪرو FAR *strm, بنانشان حرف FAR *دري,
                              ثابت حرف *نسخو, صحيح وهڪرو_سائيز) {
    بنية inflate_state FAR *state;

    إذا (نسخو == Z_NULL || نسخو[0] != ZLIB_نسخو[0] ||
        وهڪرو_سائيز != (صحيح)(حجم(z_وهڪرو)))
        أرجع Z_نسخو_ERROR;
    إذا (strm == Z_NULL || دري == Z_NULL)
        أرجع Z_وهڪرو_ERROR;
    strm->نياپو = Z_NULL;                 /* in case we return an error */
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
    state->دري = دري;
    أرجع Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#جيڪڏهن_وصف_ٿيل MAKEFIXED
#ضمن <stdio.h>

فراغ makefixed9(فراغ) {
    بنانشان sym, بٽ, هيٺيون, سائيز;
    ڪوڊ *ايندڙ, *lenfix, *distfix;
    بنية inflate_state state;
    ڪوڊ fixed[544];

    /* literal/length table */
    sym = 0;
    طالما (sym < 144) state.lens[sym++] = 8;
    طالما (sym < 256) state.lens[sym++] = 9;
    طالما (sym < 280) state.lens[sym++] = 7;
    طالما (sym < 288) state.lens[sym++] = 8;
    ايندڙ = fixed;
    lenfix = ايندڙ;
    بٽ = 9;
    inflate_table9(LENS, state.lens, 288, &(ايندڙ), &(بٽ), state.work);

    /* distance table */
    sym = 0;
    طالما (sym < 32) state.lens[sym++] = 5;
    distfix = ايندڙ;
    بٽ = 5;
    inflate_table9(DISTS, state.lens, 32, &(ايندڙ), &(بٽ), state.work);

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
    سائيز = 1U << 9;
    لکو("    static const code lenfix[%u] = {", سائيز);
    هيٺيون = 0;
    لكل (;;) {
        إذا ((هيٺيون % 6) == 0) لکو("\n        ");
        لکو("{%u,%u,%d}", lenfix[هيٺيون].op, lenfix[هيٺيون].بٽ,
               lenfix[هيٺيون].val);
        إذا (++هيٺيون == سائيز) اكسر;
        اکر_لکو(',');
    }
    puts("\n    };");
    سائيز = 1U << 5;
    لکو("\n    static const code distfix[%u] = {", سائيز);
    هيٺيون = 0;
    لكل (;;) {
        إذا ((هيٺيون % 5) == 0) لکو("\n        ");
        لکو("{%u,%u,%d}", distfix[هيٺيون].op, distfix[هيٺيون].بٽ,
               distfix[هيٺيون].val);
        إذا (++هيٺيون == سائيز) اكسر;
        اکر_لکو(',');
    }
    puts("\n    };");
}
#جيڪڏهن_پورو /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = دري; \
            کاٻو = WSIZE; \
            wrap = 1; \
            إذا (ٻاهريون(ٻاهريون_desc, put, (بنانشان)کاٻو)) { \
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
صحيح ZEXPORT inflateBack9(z_وهڪرو FAR *strm, اندريون_func اندريون, فراغ FAR *اندريون_desc,
                         ٻاهريون_func ٻاهريون, فراغ FAR *ٻاهريون_desc) {
    بنية inflate_state FAR *state;
    z_const بنانشان حرف FAR *ايندڙ;    /* next input */
    بنانشان حرف FAR *put;     /* next output */
    بنانشان have;              /* available input */
    بنانشان ڊگهو کاٻو;         /* available output */
    inflate_گهڻائي گهڻائي;          /* current inflate mode */
    صحيح lastblock;              /* true if processing last block */
    صحيح wrap;                   /* true if the window has wrapped */
    بنانشان حرف FAR *دري;  /* allocated sliding window, if needed */
    بنانشان ڊگهو hold;         /* bit buffer */
    بنانشان بٽ;              /* bits in bit buffer */
    بنانشان extra;             /* extra bits needed */
    بنانشان ڊگهو ڊيگهائي;       /* literal or length of data to copy */
    بنانشان ڊگهو وٿي;       /* distance back to copy string from */
    بنانشان ڊگهو copy;         /* number of stored or match bytes to copy */
    بنانشان حرف FAR *from;    /* where to copy match bytes from */
    ڪوڊ ثابت FAR *lencode;    /* starting table for length/literal codes */
    ڪوڊ ثابت FAR *distcode;   /* starting table for distance codes */
    بنانشان lenbits;           /* index bits for lencode */
    بنانشان distbits;          /* index bits for distcode */
    ڪوڊ هتي;                  /* current decoding table entry */
    ڪوڊ آخري;                  /* parent table entry */
    بنانشان len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
    ساكن ثابت بنانشان ننڍو ترتيب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#ضمن "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    إذا (strm == Z_NULL || strm->state == Z_NULL)
        أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->نياپو = Z_NULL;
    گهڻائي = TYPE;
    lastblock = 0;
    wrap = 0;
    دري = state->دري;
    ايندڙ = strm->ايندڙ_اندريون;
    have = ايندڙ != Z_NULL ? strm->avail_اندريون : 0;
    hold = 0;
    بٽ = 0;
    put = دري;
    کاٻو = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    لكل (;;)
        بدل (گهڻائي) {
        حالة TYPE:
            /* determine and dispatch block type */
            إذا (lastblock) {
                BYTEBITS();
                گهڻائي = پورو;
                اكسر;
            }
            NEEDBITS(3);
            lastblock = بٽ_2(1);
            DROPBITS(1);
            بدل (بٽ_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                گهڻائي = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                گهڻائي = LEN;                     /* decode codes */
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                گهڻائي = جدول;
                اكسر;
            حالة 3:
                strm->نياپو = (z_const حرف *)"invalid block type";
                گهڻائي = BAD;
            }
            DROPBITS(2);
            اكسر;

        حالة STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->نياپو = (z_const حرف *)"invalid stored block lengths";
                گهڻائي = BAD;
                اكسر;
            }
            ڊيگهائي = (بنانشان)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    ڊيگهائي));
            INITBITS();

            /* copy stored block from input to output */
            طالما (ڊيگهائي != 0) {
                copy = ڊيگهائي;
                PULL();
                گنجائش();
                إذا (copy > have) copy = have;
                إذا (copy > کاٻو) copy = کاٻو;
                zmemcpy(put, ايندڙ, copy);
                have -= copy;
                ايندڙ += copy;
                کاٻو -= copy;
                put += copy;
                ڊيگهائي -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            گهڻائي = TYPE;
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
            إذا (state->nlen > 286) {
                strm->نياپو = (z_const حرف *)"too many length symbols";
                گهڻائي = BAD;
                اكسر;
            }
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
            lencode = (ڪوڊ ثابت FAR *)(state->ايندڙ);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->ايندڙ),
                                &(lenbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid code lengths set";
                گهڻائي = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    هتي = lencode[بٽ_2(lenbits)];
                    إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                إذا (هتي.val < 16) {
                    NEEDBITS(هتي.بٽ);
                    DROPBITS(هتي.بٽ);
                    state->lens[state->have++] = هتي.val;
                }
                وإلا {
                    إذا (هتي.val == 16) {
                        NEEDBITS(هتي.بٽ + 2);
                        DROPBITS(هتي.بٽ);
                        إذا (state->have == 0) {
                            strm->نياپو = (z_const حرف *)"invalid bit length repeat";
                            گهڻائي = BAD;
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
                        strm->نياپو = (z_const حرف *)"invalid bit length repeat";
                        گهڻائي = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بنانشان ننڍو)len;
                }
            }

            /* handle error breaks in while */
            إذا (گهڻائي == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->نياپو = (z_const حرف *)"invalid code -- missing end-of-block";
                گهڻائي = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->ايندڙ = state->codes;
            lencode = (ڪوڊ ثابت FAR *)(state->ايندڙ);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->ايندڙ), &(lenbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid literal/lengths set";
                گهڻائي = BAD;
                اكسر;
            }
            distcode = (ڪوڊ ثابت FAR *)(state->ايندڙ);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->ايندڙ), &(distbits),
                            state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid distances set";
                گهڻائي = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            گهڻائي = LEN;

        حالة LEN:
            /* get a literal, length, or end-of-block code */
            لكل (;;) {
                هتي = lencode[بٽ_2(lenbits)];
                إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                PULLBYTE();
            }
            إذا (هتي.op && (هتي.op & 0xf0) == 0) {
                آخري = هتي;
                لكل (;;) {
                    هتي = lencode[آخري.val +
                            (بٽ_2(آخري.بٽ + آخري.op) >> آخري.بٽ)];
                    إذا ((بنانشان)(آخري.بٽ + هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخري.بٽ);
            }
            DROPBITS(هتي.بٽ);
            ڊيگهائي = (بنانشان)هتي.val;

            /* process literal */
            إذا (هتي.op == 0) {
                Tracevv((stderr, هتي.val >= 0x20 && هتي.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", هتي.val));
                گنجائش();
                *put++ = (بنانشان حرف)(ڊيگهائي);
                کاٻو--;
                گهڻائي = LEN;
                اكسر;
            }

            /* process end of block */
            إذا (هتي.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                گهڻائي = TYPE;
                اكسر;
            }

            /* invalid code */
            إذا (هتي.op & 64) {
                strm->نياپو = (z_const حرف *)"invalid literal/length code";
                گهڻائي = BAD;
                اكسر;
            }

            /* length code -- get extra bits, if any */
            extra = (بنانشان)(هتي.op) & 31;
            إذا (extra != 0) {
                NEEDBITS(extra);
                ڊيگهائي += بٽ_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", ڊيگهائي));

            /* get distance code */
            لكل (;;) {
                هتي = distcode[بٽ_2(distbits)];
                إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                PULLBYTE();
            }
            إذا ((هتي.op & 0xf0) == 0) {
                آخري = هتي;
                لكل (;;) {
                    هتي = distcode[آخري.val +
                            (بٽ_2(آخري.بٽ + آخري.op) >> آخري.بٽ)];
                    إذا ((بنانشان)(آخري.بٽ + هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخري.بٽ);
            }
            DROPBITS(هتي.بٽ);
            إذا (هتي.op & 64) {
                strm->نياپو = (z_const حرف *)"invalid distance code";
                گهڻائي = BAD;
                اكسر;
            }
            وٿي = (بنانشان)هتي.val;

            /* get distance extra bits, if any */
            extra = (بنانشان)(هتي.op) & 15;
            إذا (extra != 0) {
                NEEDBITS(extra);
                وٿي += بٽ_2(extra);
                DROPBITS(extra);
            }
            إذا (وٿي > WSIZE - (wrap ? 0: کاٻو)) {
                strm->نياپو = (z_const حرف *)"invalid distance too far back";
                گهڻائي = BAD;
                اكسر;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", وٿي));

            /* copy match from window to output */
            افعل {
                گنجائش();
                copy = WSIZE - وٿي;
                إذا (copy < کاٻو) {
                    from = put + copy;
                    copy = کاٻو - copy;
                }
                وإلا {
                    from = put - وٿي;
                    copy = کاٻو;
                }
                إذا (copy > ڊيگهائي) copy = ڊيگهائي;
                ڊيگهائي -= copy;
                کاٻو -= copy;
                افعل {
                    *put++ = *from++;
                } طالما (--copy);
            } طالما (ڊيگهائي != 0);
            اكسر;

        حالة پورو:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_وهڪرو_پڇاڙي;
            إذا (کاٻو < WSIZE) {
                إذا (ٻاهريون(ٻاهريون_desc, دري, (بنانشان)(WSIZE - کاٻو)))
                    ret = Z_BUF_ERROR;
            }
            اذهب inf_leave;

        حالة BAD:
            ret = Z_معلومات_ERROR;
            اذهب inf_leave;

        افتراضي:                /* can't happen, but makes compilers happy */
            ret = Z_وهڪرو_ERROR;
            اذهب inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->ايندڙ_اندريون = ايندڙ;
    strm->avail_اندريون = have;
    أرجع ret;
}

صحيح ZEXPORT inflate_Back9_پڇاڙي(z_وهڪرو FAR *strm) {
    إذا (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        أرجع Z_وهڪرو_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}
