/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#అన్తర్భావ "zutil.h"
#అన్తర్భావ "infback9.h"
#అన్తర్భావ "inftree9.h"
#అన్తర్భావ "inflate9.h"

#నిర్వచించు WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
పూర్ణాంకం ZEXPORT inflateBack9Init_(z_ప్రవాహం FAR *strm, గుర్తులేని అక్షరం FAR *కిటికీ,
                              స్థిరరాశి అక్షరం *సంస్కరణ, పూర్ణాంకం ప్రవాహం_కొలత) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (సంస్కరణ == Z_NULL || సంస్కరణ[0] != ZLIB_సంస్కరణ[0] ||
        ప్రవాహం_కొలత != (పూర్ణాంకం)(పరిమాణం(z_ప్రవాహం)))
        తిరిగి Z_సంస్కరణ_ERROR;
    ఒకవేళ (strm == Z_NULL || కిటికీ == Z_NULL)
        తిరిగి Z_ప్రవాహం_ERROR;
    strm->సందేశం = Z_NULL;                 /* in case we return an error */
    ఒకవేళ (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    ఒకవేళ (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (నిర్మాణం inflate_state FAR *)ZALLOC(strm, 1,
                                               పరిమాణం(నిర్మాణం inflate_state));
    ఒకవేళ (state == Z_NULL) తిరిగి Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->కిటికీ = కిటికీ;
    తిరిగి Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#నిర్వచితమైతే MAKEFIXED
#అన్తర్భావ <stdio.h>

శూన్య makefixed9(శూన్య) {
    గుర్తులేని sym, బిట్లు, దిగువ, కొలత;
    సంకేతం_2 *తదుపరి, *lenfix, *distfix;
    నిర్మాణం inflate_state state;
    సంకేతం_2 fixed[544];

    /* literal/length table */
    sym = 0;
    అయితే (sym < 144) state.lens[sym++] = 8;
    అయితే (sym < 256) state.lens[sym++] = 9;
    అయితే (sym < 280) state.lens[sym++] = 7;
    అయితే (sym < 288) state.lens[sym++] = 8;
    తదుపరి = fixed;
    lenfix = తదుపరి;
    బిట్లు = 9;
    inflate_table9(LENS, state.lens, 288, &(తదుపరి), &(బిట్లు), state.work);

    /* distance table */
    sym = 0;
    అయితే (sym < 32) state.lens[sym++] = 5;
    distfix = తదుపరి;
    బిట్లు = 5;
    inflate_table9(DISTS, state.lens, 32, &(తదుపరి), &(బిట్లు), state.work);

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
    కొలత = 1U << 9;
    రాయి("    static const code lenfix[%u] = {", కొలత);
    దిగువ = 0;
    కొరకు (;;) {
        ఒకవేళ ((దిగువ % 6) == 0) రాయి("\n        ");
        రాయి("{%u,%u,%d}", lenfix[దిగువ].op, lenfix[దిగువ].బిట్లు,
               lenfix[దిగువ].val);
        ఒకవేళ (++దిగువ == కొలత) ఆపు;
        అక్షరం_రాయి(',');
    }
    puts("\n    };");
    కొలత = 1U << 5;
    రాయి("\n    static const code distfix[%u] = {", కొలత);
    దిగువ = 0;
    కొరకు (;;) {
        ఒకవేళ ((దిగువ % 5) == 0) రాయి("\n        ");
        రాయి("{%u,%u,%d}", distfix[దిగువ].op, distfix[దిగువ].బిట్లు,
               distfix[దిగువ].val);
        ఒకవేళ (++దిగువ == కొలత) ఆపు;
        అక్షరం_రాయి(',');
    }
    puts("\n    };");
}
#అయితే_ముగింపు /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#నిర్వచించు INITBITS() \
    చేయి { \
        hold = 0; \
        బిట్లు = 0; \
    } అయితే (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#నిర్వచించు PULL() \
    చేయి { \
        ఒకవేళ (have == 0) { \
            have = లోపల(లోపల_desc, &తదుపరి); \
            ఒకవేళ (have == 0) { \
                తదుపరి = Z_NULL; \
                ret = Z_BUF_ERROR; \
                వెళ్ళు inf_leave; \
            } \
        } \
    } అయితే (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#నిర్వచించు PULLBYTE() \
    చేయి { \
        PULL(); \
        have--; \
        hold += (గుర్తులేని దీర్ఘ)(*తదుపరి++) << బిట్లు; \
        బిట్లు += 8; \
    } అయితే (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#నిర్వచించు NEEDBITS(n) \
    చేయి { \
        అయితే (బిట్లు < (గుర్తులేని)(n)) \
            PULLBYTE(); \
    } అయితే (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#నిర్వచించు బిట్లు_2(n) \
    ((గుర్తులేని)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#నిర్వచించు DROPBITS(n) \
    చేయి { \
        hold >>= (n); \
        బిట్లు -= (గుర్తులేని)(n); \
    } అయితే (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#నిర్వచించు BYTEBITS() \
    చేయి { \
        hold >>= బిట్లు & 7; \
        బిట్లు -= బిట్లు & 7; \
    } అయితే (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#నిర్వచించు స్థలం() \
    చేయి { \
        ఒకవేళ (ఎడమ == 0) { \
            put = కిటికీ; \
            ఎడమ = WSIZE; \
            wrap = 1; \
            ఒకవేళ (బహిర్గతం(బహిర్గతం_desc, put, (గుర్తులేని)ఎడమ)) { \
                ret = Z_BUF_ERROR; \
                వెళ్ళు inf_leave; \
            } \
        } \
    } అయితే (0)

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
పూర్ణాంకం ZEXPORT inflateBack9(z_ప్రవాహం FAR *strm, లోపల_func లోపల, శూన్య FAR *లోపల_desc,
                         బహిర్గతం_func బహిర్గతం, శూన్య FAR *బహిర్గతం_desc) {
    నిర్మాణం inflate_state FAR *state;
    z_const గుర్తులేని అక్షరం FAR *తదుపరి;    /* next input */
    గుర్తులేని అక్షరం FAR *put;     /* next output */
    గుర్తులేని have;              /* available input */
    గుర్తులేని దీర్ఘ ఎడమ;         /* available output */
    inflate_బహుళకం బహుళకం;          /* current inflate mode */
    పూర్ణాంకం lastblock;              /* true if processing last block */
    పూర్ణాంకం wrap;                   /* true if the window has wrapped */
    గుర్తులేని అక్షరం FAR *కిటికీ;  /* allocated sliding window, if needed */
    గుర్తులేని దీర్ఘ hold;         /* bit buffer */
    గుర్తులేని బిట్లు;              /* bits in bit buffer */
    గుర్తులేని extra;             /* extra bits needed */
    గుర్తులేని దీర్ఘ నిడివి;       /* literal or length of data to copy */
    గుర్తులేని దీర్ఘ స్థానభ్రంశం;       /* distance back to copy string from */
    గుర్తులేని దీర్ఘ copy;         /* number of stored or match bytes to copy */
    గుర్తులేని అక్షరం FAR *from;    /* where to copy match bytes from */
    సంకేతం_2 స్థిరరాశి FAR *lencode;    /* starting table for length/literal codes */
    సంకేతం_2 స్థిరరాశి FAR *distcode;   /* starting table for distance codes */
    గుర్తులేని lenbits;           /* index bits for lencode */
    గుర్తులేని distbits;          /* index bits for distcode */
    సంకేతం_2 ఇక్కడ;                  /* current decoding table entry */
    సంకేతం_2 చివరి;                  /* parent table entry */
    గుర్తులేని len;               /* length to copy for repeats, bits to drop */
    పూర్ణాంకం ret;                    /* return code */
    స్థిర స్థిరరాశి గుర్తులేని పొట్టి క్రమం[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#అన్తర్భావ "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    ఒకవేళ (strm == Z_NULL || strm->state == Z_NULL)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->సందేశం = Z_NULL;
    బహుళకం = TYPE;
    lastblock = 0;
    wrap = 0;
    కిటికీ = state->కిటికీ;
    తదుపరి = strm->తదుపరి_లోపల;
    have = తదుపరి != Z_NULL ? strm->avail_లోపల : 0;
    hold = 0;
    బిట్లు = 0;
    put = కిటికీ;
    ఎడమ = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    కొరకు (;;)
        ఎంపిక (బహుళకం) {
        స్థితి TYPE:
            /* determine and dispatch block type */
            ఒకవేళ (lastblock) {
                BYTEBITS();
                బహుళకం = పూర్తి;
                ఆపు;
            }
            NEEDBITS(3);
            lastblock = బిట్లు_2(1);
            DROPBITS(1);
            ఎంపిక (బిట్లు_2(2)) {
            స్థితి 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                బహుళకం = STORED;
                ఆపు;
            స్థితి 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                బహుళకం = LEN;                     /* decode codes */
                ఆపు;
            స్థితి 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                బహుళకం = పట్టిక;
                ఆపు;
            స్థితి 3:
                strm->సందేశం = (z_const అక్షరం *)"invalid block type";
                బహుళకం = BAD;
            }
            DROPBITS(2);
            ఆపు;

        స్థితి STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ఒకవేళ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->సందేశం = (z_const అక్షరం *)"invalid stored block lengths";
                బహుళకం = BAD;
                ఆపు;
            }
            నిడివి = (గుర్తులేని)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    నిడివి));
            INITBITS();

            /* copy stored block from input to output */
            అయితే (నిడివి != 0) {
                copy = నిడివి;
                PULL();
                స్థలం();
                ఒకవేళ (copy > have) copy = have;
                ఒకవేళ (copy > ఎడమ) copy = ఎడమ;
                zmemcpy(put, తదుపరి, copy);
                have -= copy;
                తదుపరి += copy;
                ఎడమ -= copy;
                put += copy;
                నిడివి -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            బహుళకం = TYPE;
            ఆపు;

        స్థితి పట్టిక:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = బిట్లు_2(5) + 257;
            DROPBITS(5);
            state->ndist = బిట్లు_2(5) + 1;
            DROPBITS(5);
            state->ncode = బిట్లు_2(4) + 4;
            DROPBITS(4);
            ఒకవేళ (state->nlen > 286) {
                strm->సందేశం = (z_const అక్షరం *)"too many length symbols";
                బహుళకం = BAD;
                ఆపు;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            అయితే (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[క్రమం[state->have++]] = (గుర్తులేని పొట్టి)బిట్లు_2(3);
                DROPBITS(3);
            }
            అయితే (state->have < 19)
                state->lens[క్రమం[state->have++]] = 0;
            state->తదుపరి = state->codes;
            lencode = (సంకేతం_2 స్థిరరాశి FAR *)(state->తదుపరి);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->తదుపరి),
                                &(lenbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid code lengths set";
                బహుళకం = BAD;
                ఆపు;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            అయితే (state->have < state->nlen + state->ndist) {
                కొరకు (;;) {
                    ఇక్కడ = lencode[బిట్లు_2(lenbits)];
                    ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                ఒకవేళ (ఇక్కడ.val < 16) {
                    NEEDBITS(ఇక్కడ.బిట్లు);
                    DROPBITS(ఇక్కడ.బిట్లు);
                    state->lens[state->have++] = ఇక్కడ.val;
                }
                లేకపోతే {
                    ఒకవేళ (ఇక్కడ.val == 16) {
                        NEEDBITS(ఇక్కడ.బిట్లు + 2);
                        DROPBITS(ఇక్కడ.బిట్లు);
                        ఒకవేళ (state->have == 0) {
                            strm->సందేశం = (z_const అక్షరం *)"invalid bit length repeat";
                            బహుళకం = BAD;
                            ఆపు;
                        }
                        len = (గుర్తులేని)(state->lens[state->have - 1]);
                        copy = 3 + బిట్లు_2(2);
                        DROPBITS(2);
                    }
                    లేకపోతే ఒకవేళ (ఇక్కడ.val == 17) {
                        NEEDBITS(ఇక్కడ.బిట్లు + 3);
                        DROPBITS(ఇక్కడ.బిట్లు);
                        len = 0;
                        copy = 3 + బిట్లు_2(3);
                        DROPBITS(3);
                    }
                    లేకపోతే {
                        NEEDBITS(ఇక్కడ.బిట్లు + 7);
                        DROPBITS(ఇక్కడ.బిట్లు);
                        len = 0;
                        copy = 11 + బిట్లు_2(7);
                        DROPBITS(7);
                    }
                    ఒకవేళ (state->have + copy > state->nlen + state->ndist) {
                        strm->సందేశం = (z_const అక్షరం *)"invalid bit length repeat";
                        బహుళకం = BAD;
                        ఆపు;
                    }
                    అయితే (copy--)
                        state->lens[state->have++] = (గుర్తులేని పొట్టి)len;
                }
            }

            /* handle error breaks in while */
            ఒకవేళ (బహుళకం == BAD) ఆపు;

            /* check for end-of-block code (better have one) */
            ఒకవేళ (state->lens[256] == 0) {
                strm->సందేశం = (z_const అక్షరం *)"invalid code -- missing end-of-block";
                బహుళకం = BAD;
                ఆపు;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->తదుపరి = state->codes;
            lencode = (సంకేతం_2 స్థిరరాశి FAR *)(state->తదుపరి);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->తదుపరి), &(lenbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid literal/lengths set";
                బహుళకం = BAD;
                ఆపు;
            }
            distcode = (సంకేతం_2 స్థిరరాశి FAR *)(state->తదుపరి);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->తదుపరి), &(distbits),
                            state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distances set";
                బహుళకం = BAD;
                ఆపు;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            బహుళకం = LEN;

        స్థితి LEN:
            /* get a literal, length, or end-of-block code */
            కొరకు (;;) {
                ఇక్కడ = lencode[బిట్లు_2(lenbits)];
                ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                PULLBYTE();
            }
            ఒకవేళ (ఇక్కడ.op && (ఇక్కడ.op & 0xf0) == 0) {
                చివరి = ఇక్కడ;
                కొరకు (;;) {
                    ఇక్కడ = lencode[చివరి.val +
                            (బిట్లు_2(చివరి.బిట్లు + చివరి.op) >> చివరి.బిట్లు)];
                    ఒకవేళ ((గుర్తులేని)(చివరి.బిట్లు + ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                DROPBITS(చివరి.బిట్లు);
            }
            DROPBITS(ఇక్కడ.బిట్లు);
            నిడివి = (గుర్తులేని)ఇక్కడ.val;

            /* process literal */
            ఒకవేళ (ఇక్కడ.op == 0) {
                Tracevv((stderr, ఇక్కడ.val >= 0x20 && ఇక్కడ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ఇక్కడ.val));
                స్థలం();
                *put++ = (గుర్తులేని అక్షరం)(నిడివి);
                ఎడమ--;
                బహుళకం = LEN;
                ఆపు;
            }

            /* process end of block */
            ఒకవేళ (ఇక్కడ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                బహుళకం = TYPE;
                ఆపు;
            }

            /* invalid code */
            ఒకవేళ (ఇక్కడ.op & 64) {
                strm->సందేశం = (z_const అక్షరం *)"invalid literal/length code";
                బహుళకం = BAD;
                ఆపు;
            }

            /* length code -- get extra bits, if any */
            extra = (గుర్తులేని)(ఇక్కడ.op) & 31;
            ఒకవేళ (extra != 0) {
                NEEDBITS(extra);
                నిడివి += బిట్లు_2(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", నిడివి));

            /* get distance code */
            కొరకు (;;) {
                ఇక్కడ = distcode[బిట్లు_2(distbits)];
                ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                PULLBYTE();
            }
            ఒకవేళ ((ఇక్కడ.op & 0xf0) == 0) {
                చివరి = ఇక్కడ;
                కొరకు (;;) {
                    ఇక్కడ = distcode[చివరి.val +
                            (బిట్లు_2(చివరి.బిట్లు + చివరి.op) >> చివరి.బిట్లు)];
                    ఒకవేళ ((గుర్తులేని)(చివరి.బిట్లు + ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                DROPBITS(చివరి.బిట్లు);
            }
            DROPBITS(ఇక్కడ.బిట్లు);
            ఒకవేళ (ఇక్కడ.op & 64) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distance code";
                బహుళకం = BAD;
                ఆపు;
            }
            స్థానభ్రంశం = (గుర్తులేని)ఇక్కడ.val;

            /* get distance extra bits, if any */
            extra = (గుర్తులేని)(ఇక్కడ.op) & 15;
            ఒకవేళ (extra != 0) {
                NEEDBITS(extra);
                స్థానభ్రంశం += బిట్లు_2(extra);
                DROPBITS(extra);
            }
            ఒకవేళ (స్థానభ్రంశం > WSIZE - (wrap ? 0: ఎడమ)) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distance too far back";
                బహుళకం = BAD;
                ఆపు;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", స్థానభ్రంశం));

            /* copy match from window to output */
            చేయి {
                స్థలం();
                copy = WSIZE - స్థానభ్రంశం;
                ఒకవేళ (copy < ఎడమ) {
                    from = put + copy;
                    copy = ఎడమ - copy;
                }
                లేకపోతే {
                    from = put - స్థానభ్రంశం;
                    copy = ఎడమ;
                }
                ఒకవేళ (copy > నిడివి) copy = నిడివి;
                నిడివి -= copy;
                ఎడమ -= copy;
                చేయి {
                    *put++ = *from++;
                } అయితే (--copy);
            } అయితే (నిడివి != 0);
            ఆపు;

        స్థితి పూర్తి:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_ప్రవాహం_చివర;
            ఒకవేళ (ఎడమ < WSIZE) {
                ఒకవేళ (బహిర్గతం(బహిర్గతం_desc, కిటికీ, (గుర్తులేని)(WSIZE - ఎడమ)))
                    ret = Z_BUF_ERROR;
            }
            వెళ్ళు inf_leave;

        స్థితి BAD:
            ret = Z_దత్తాంశం_ERROR;
            వెళ్ళు inf_leave;

        అప్రమేయ:                /* can't happen, but makes compilers happy */
            ret = Z_ప్రవాహం_ERROR;
            వెళ్ళు inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->తదుపరి_లోపల = తదుపరి;
    strm->avail_లోపల = have;
    తిరిగి ret;
}

పూర్ణాంకం ZEXPORT inflate_Back9_చివర(z_ప్రవాహం FAR *strm) {
    ఒకవేళ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        తిరిగి Z_ప్రవాహం_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    తిరిగి Z_OK;
}
