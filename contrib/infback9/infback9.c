/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "zutil.h"
#شامل "infback9.h"
#شامل "inftree9.h"
#شامل "inflate9.h"

#تعریف_کرو WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
صحیح ZEXPORT inflateBack9Init_(z_دھارا FAR *strm, غیرعلامتی حرف FAR *کھڑکی,
                              ثابت حرف *نسخہ, صحیح دھارا_سائز) {
    ساخت inflate_state FAR *state;

    اگر (نسخہ == Z_NULL || نسخہ[0] != ZLIB_نسخہ[0] ||
        دھارا_سائز != (صحیح)(حجم(z_دھارا)))
        واپس Z_نسخہ_ERROR;
    اگر (strm == Z_NULL || کھڑکی == Z_NULL)
        واپس Z_دھارا_ERROR;
    strm->پیغام = Z_NULL;                 /* in case we return an error */
    اگر (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    اگر (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (ساخت inflate_state FAR *)ZALLOC(strm, 1,
                                               حجم(ساخت inflate_state));
    اگر (state == Z_NULL) واپس Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->کھڑکی = کھڑکی;
    واپس Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#اگر_تعریف_شدہ MAKEFIXED
#شامل <stdio.h>

خالی makefixed9(خالی) {
    غیرعلامتی sym, بٹ, کم, سائز;
    رمز *اگلا, *lenfix, *distfix;
    ساخت inflate_state state;
    رمز fixed[544];

    /* literal/length table */
    sym = 0;
    جب (sym < 144) state.lens[sym++] = 8;
    جب (sym < 256) state.lens[sym++] = 9;
    جب (sym < 280) state.lens[sym++] = 7;
    جب (sym < 288) state.lens[sym++] = 8;
    اگلا = fixed;
    lenfix = اگلا;
    بٹ = 9;
    inflate_table9(LENS, state.lens, 288, &(اگلا), &(بٹ), state.work);

    /* distance table */
    sym = 0;
    جب (sym < 32) state.lens[sym++] = 5;
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
    سائز = 1U << 9;
    لکھو("    static const code lenfix[%u] = {", سائز);
    کم = 0;
    برائے (;;) {
        اگر ((کم % 6) == 0) لکھو("\n        ");
        لکھو("{%u,%u,%d}", lenfix[کم].op, lenfix[کم].بٹ,
               lenfix[کم].val);
        اگر (++کم == سائز) توڑو;
        حرف_لکھو(',');
    }
    puts("\n    };");
    سائز = 1U << 5;
    لکھو("\n    static const code distfix[%u] = {", سائز);
    کم = 0;
    برائے (;;) {
        اگر ((کم % 5) == 0) لکھو("\n        ");
        لکھو("{%u,%u,%d}", distfix[کم].op, distfix[کم].بٹ,
               distfix[کم].val);
        اگر (++کم == سائز) توڑو;
        حرف_لکھو(',');
    }
    puts("\n    };");
}
#اگر_ختم /* MAKEFIXED */

/* Macros for inflateBack(): */

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

/* Return the low n bits of the bit accumulator (n <= 16) */
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
            put = کھڑکی; \
            بایاں = WSIZE; \
            wrap = 1; \
            اگر (برآمد(برآمد_desc, put, (غیرعلامتی)بایاں)) { \
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
صحیح ZEXPORT inflateBack9(z_دھارا FAR *strm, اندر_func اندر, خالی FAR *اندر_desc,
                         برآمد_func برآمد, خالی FAR *برآمد_desc) {
    ساخت inflate_state FAR *state;
    z_const غیرعلامتی حرف FAR *اگلا;    /* next input */
    غیرعلامتی حرف FAR *put;     /* next output */
    غیرعلامتی have;              /* available input */
    غیرعلامتی طویل بایاں;         /* available output */
    inflate_منوال منوال;          /* current inflate mode */
    صحیح lastblock;              /* true if processing last block */
    صحیح wrap;                   /* true if the window has wrapped */
    غیرعلامتی حرف FAR *کھڑکی;  /* allocated sliding window, if needed */
    غیرعلامتی طویل hold;         /* bit buffer */
    غیرعلامتی بٹ;              /* bits in bit buffer */
    غیرعلامتی extra;             /* extra bits needed */
    غیرعلامتی طویل لمبائی;       /* literal or length of data to copy */
    غیرعلامتی طویل انحراف;       /* distance back to copy string from */
    غیرعلامتی طویل copy;         /* number of stored or match bytes to copy */
    غیرعلامتی حرف FAR *from;    /* where to copy match bytes from */
    رمز ثابت FAR *lencode;    /* starting table for length/literal codes */
    رمز ثابت FAR *distcode;   /* starting table for distance codes */
    غیرعلامتی lenbits;           /* index bits for lencode */
    غیرعلامتی distbits;          /* index bits for distcode */
    رمز یہاں;                  /* current decoding table entry */
    رمز آخری;                  /* parent table entry */
    غیرعلامتی len;               /* length to copy for repeats, bits to drop */
    صحیح ret;                    /* return code */
    ساکن ثابت غیرعلامتی مختصر ترتیب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#شامل "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    اگر (strm == Z_NULL || strm->state == Z_NULL)
        واپس Z_دھارا_ERROR;
    state = (ساخت inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->پیغام = Z_NULL;
    منوال = TYPE;
    lastblock = 0;
    wrap = 0;
    کھڑکی = state->کھڑکی;
    اگلا = strm->اگلا_اندر;
    have = اگلا != Z_NULL ? strm->avail_اندر : 0;
    hold = 0;
    بٹ = 0;
    put = کھڑکی;
    بایاں = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    برائے (;;)
        انتخاب (منوال) {
        حالت TYPE:
            /* determine and dispatch block type */
            اگر (lastblock) {
                BYTEBITS();
                منوال = مکمل;
                توڑو;
            }
            NEEDBITS(3);
            lastblock = بٹ_2(1);
            DROPBITS(1);
            انتخاب (بٹ_2(2)) {
            حالت 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                منوال = STORED;
                توڑو;
            حالت 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                منوال = LEN;                     /* decode codes */
                توڑو;
            حالت 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                منوال = جدول;
                توڑو;
            حالت 3:
                strm->پیغام = (z_const حرف *)"invalid block type";
                منوال = BAD;
            }
            DROPBITS(2);
            توڑو;

        حالت STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            اگر ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->پیغام = (z_const حرف *)"invalid stored block lengths";
                منوال = BAD;
                توڑو;
            }
            لمبائی = (غیرعلامتی)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    لمبائی));
            INITBITS();

            /* copy stored block from input to output */
            جب (لمبائی != 0) {
                copy = لمبائی;
                PULL();
                گنجائش();
                اگر (copy > have) copy = have;
                اگر (copy > بایاں) copy = بایاں;
                zmemcpy(put, اگلا, copy);
                have -= copy;
                اگلا += copy;
                بایاں -= copy;
                put += copy;
                لمبائی -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            منوال = TYPE;
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
            اگر (state->nlen > 286) {
                strm->پیغام = (z_const حرف *)"too many length symbols";
                منوال = BAD;
                توڑو;
            }
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
            lencode = (رمز ثابت FAR *)(state->اگلا);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->اگلا),
                                &(lenbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid code lengths set";
                منوال = BAD;
                توڑو;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            جب (state->have < state->nlen + state->ndist) {
                برائے (;;) {
                    یہاں = lencode[بٹ_2(lenbits)];
                    اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                اگر (یہاں.val < 16) {
                    NEEDBITS(یہاں.بٹ);
                    DROPBITS(یہاں.بٹ);
                    state->lens[state->have++] = یہاں.val;
                }
                ورنہ {
                    اگر (یہاں.val == 16) {
                        NEEDBITS(یہاں.بٹ + 2);
                        DROPBITS(یہاں.بٹ);
                        اگر (state->have == 0) {
                            strm->پیغام = (z_const حرف *)"invalid bit length repeat";
                            منوال = BAD;
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
                        strm->پیغام = (z_const حرف *)"invalid bit length repeat";
                        منوال = BAD;
                        توڑو;
                    }
                    جب (copy--)
                        state->lens[state->have++] = (غیرعلامتی مختصر)len;
                }
            }

            /* handle error breaks in while */
            اگر (منوال == BAD) توڑو;

            /* check for end-of-block code (better have one) */
            اگر (state->lens[256] == 0) {
                strm->پیغام = (z_const حرف *)"invalid code -- missing end-of-block";
                منوال = BAD;
                توڑو;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->اگلا = state->codes;
            lencode = (رمز ثابت FAR *)(state->اگلا);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->اگلا), &(lenbits), state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid literal/lengths set";
                منوال = BAD;
                توڑو;
            }
            distcode = (رمز ثابت FAR *)(state->اگلا);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->اگلا), &(distbits),
                            state->work);
            اگر (ret) {
                strm->پیغام = (z_const حرف *)"invalid distances set";
                منوال = BAD;
                توڑو;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            منوال = LEN;

        حالت LEN:
            /* get a literal, length, or end-of-block code */
            برائے (;;) {
                یہاں = lencode[بٹ_2(lenbits)];
                اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                PULLBYTE();
            }
            اگر (یہاں.op && (یہاں.op & 0xf0) == 0) {
                آخری = یہاں;
                برائے (;;) {
                    یہاں = lencode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    اگر ((غیرعلامتی)(آخری.بٹ + یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(یہاں.بٹ);
            لمبائی = (غیرعلامتی)یہاں.val;

            /* process literal */
            اگر (یہاں.op == 0) {
                Tracevv((stderr, یہاں.val >= 0x20 && یہاں.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", یہاں.val));
                گنجائش();
                *put++ = (غیرعلامتی حرف)(لمبائی);
                بایاں--;
                منوال = LEN;
                توڑو;
            }

            /* process end of block */
            اگر (یہاں.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                منوال = TYPE;
                توڑو;
            }

            /* invalid code */
            اگر (یہاں.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid literal/length code";
                منوال = BAD;
                توڑو;
            }

            /* length code -- get extra bits, if any */
            extra = (غیرعلامتی)(یہاں.op) & 31;
            اگر (extra != 0) {
                NEEDBITS(extra);
                لمبائی += بٹ_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", لمبائی));

            /* get distance code */
            برائے (;;) {
                یہاں = distcode[بٹ_2(distbits)];
                اگر ((غیرعلامتی)(یہاں.بٹ) <= بٹ) توڑو;
                PULLBYTE();
            }
            اگر ((یہاں.op & 0xf0) == 0) {
                آخری = یہاں;
                برائے (;;) {
                    یہاں = distcode[آخری.val +
                            (بٹ_2(آخری.بٹ + آخری.op) >> آخری.بٹ)];
                    اگر ((غیرعلامتی)(آخری.بٹ + یہاں.بٹ) <= بٹ) توڑو;
                    PULLBYTE();
                }
                DROPBITS(آخری.بٹ);
            }
            DROPBITS(یہاں.بٹ);
            اگر (یہاں.op & 64) {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                منوال = BAD;
                توڑو;
            }
            انحراف = (غیرعلامتی)یہاں.val;

            /* get distance extra bits, if any */
            extra = (غیرعلامتی)(یہاں.op) & 15;
            اگر (extra != 0) {
                NEEDBITS(extra);
                انحراف += بٹ_2(extra);
                DROPBITS(extra);
            }
            اگر (انحراف > WSIZE - (wrap ? 0: بایاں)) {
                strm->پیغام = (z_const حرف *)"invalid distance too far back";
                منوال = BAD;
                توڑو;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", انحراف));

            /* copy match from window to output */
            کرو {
                گنجائش();
                copy = WSIZE - انحراف;
                اگر (copy < بایاں) {
                    from = put + copy;
                    copy = بایاں - copy;
                }
                ورنہ {
                    from = put - انحراف;
                    copy = بایاں;
                }
                اگر (copy > لمبائی) copy = لمبائی;
                لمبائی -= copy;
                بایاں -= copy;
                کرو {
                    *put++ = *from++;
                } جب (--copy);
            } جب (لمبائی != 0);
            توڑو;

        حالت مکمل:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_دھارا_انتہا;
            اگر (بایاں < WSIZE) {
                اگر (برآمد(برآمد_desc, کھڑکی, (غیرعلامتی)(WSIZE - بایاں)))
                    ret = Z_BUF_ERROR;
            }
            جاؤ inf_leave;

        حالت BAD:
            ret = Z_ڈیٹا_ERROR;
            جاؤ inf_leave;

        طےشدہ:                /* can't happen, but makes compilers happy */
            ret = Z_دھارا_ERROR;
            جاؤ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->اگلا_اندر = اگلا;
    strm->avail_اندر = have;
    واپس ret;
}

صحیح ZEXPORT inflate_Back9_انتہا(z_دھارا FAR *strm) {
    اگر (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        واپس Z_دھارا_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    واپس Z_OK;
}
