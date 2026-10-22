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

#అన్తర్భావ "zutil.h"
#అన్తర్భావ "inftrees.h"
#అన్తర్భావ "inflate.h"
#అన్తర్భావ "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
పూర్ణాంకం ZEXPORT inflate_వెనుక_Init_2(z_streamp strm, పూర్ణాంకం కిటికీ_బిట్లు,
                             గుర్తులేని అక్షరం FAR *కిటికీ, స్థిరరాశి అక్షరం *సంస్కరణ,
                             పూర్ణాంకం ప్రవాహం_కొలత) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (సంస్కరణ == Z_NULL || సంస్కరణ[0] != ZLIB_సంస్కరణ[0] ||
        ప్రవాహం_కొలత != (పూర్ణాంకం)(పరిమాణం(z_ప్రవాహం)))
        తిరిగి Z_సంస్కరణ_ERROR;
    ఒకవేళ (strm == Z_NULL || కిటికీ == Z_NULL ||
        కిటికీ_బిట్లు < 8 || కిటికీ_బిట్లు > 15)
        తిరిగి Z_ప్రవాహం_ERROR;
    strm->సందేశం = Z_NULL;                 /* in case we return an error */
    ఒకవేళ (strm->zalloc == (alloc_func)0) {
#నిర్వచితమైతే Z_SOLO
        తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#అయితే_ముగింపు
    }
    ఒకవేళ (strm->zfree == (free_func)0)
#నిర్వచితమైతే Z_SOLO
        తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
        strm->zfree = zcfree;
#అయితే_ముగింపు
    state = (నిర్మాణం inflate_state FAR *)ZALLOC(strm, 1,
                                               పరిమాణం(నిర్మాణం inflate_state));
    ఒకవేళ (state == Z_NULL) తిరిగి Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (నిర్మాణం అంతర్గత_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)కిటికీ_బిట్లు;
    state->wsize = 1U << కిటికీ_బిట్లు;
    state->కిటికీ = కిటికీ;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    తిరిగి Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#నిర్వచించు ఎక్కించు_2() \
    చేయి { \
        put = strm->తదుపరి_బహిర్గతం; \
        ఎడమ = strm->avail_బహిర్గతం; \
        తదుపరి = strm->తదుపరి_లోపల; \
        have = strm->avail_లోపల; \
        hold = state->hold; \
        బిట్లు = state->బిట్లు; \
    } అయితే (0)

/* Set state from registers for inflate_fast() */
#నిర్వచించు RESTORE() \
    చేయి { \
        strm->తదుపరి_బహిర్గతం = put; \
        strm->avail_బహిర్గతం = ఎడమ; \
        strm->తదుపరి_లోపల = తదుపరి; \
        strm->avail_లోపల = have; \
        state->hold = hold; \
        state->బిట్లు = బిట్లు; \
    } అయితే (0)

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

/* Return the low n bits of the bit accumulator (n < 16) */
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
            put = state->కిటికీ; \
            ఎడమ = state->wsize; \
            state->whave = ఎడమ; \
            ఒకవేళ (బహిర్గతం(బహిర్గతం_desc, put, ఎడమ)) { \
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
పూర్ణాంకం ZEXPORT inflate_వెనుక(z_streamp strm, లోపల_func లోపల, శూన్య FAR *లోపల_desc,
                        బహిర్గతం_func బహిర్గతం, శూన్య FAR *బహిర్గతం_desc) {
    నిర్మాణం inflate_state FAR *state;
    z_const గుర్తులేని అక్షరం FAR *తదుపరి;    /* next input */
    గుర్తులేని అక్షరం FAR *put;     /* next output */
    గుర్తులేని have, ఎడమ;        /* available input and output */
    గుర్తులేని దీర్ఘ hold;         /* bit buffer */
    గుర్తులేని బిట్లు;              /* bits in bit buffer */
    గుర్తులేని copy;              /* number of stored or match bytes to copy */
    గుర్తులేని అక్షరం FAR *from;    /* where to copy match bytes from */
    సంకేతం_2 ఇక్కడ;                  /* current decoding table entry */
    సంకేతం_2 చివరి;                  /* parent table entry */
    గుర్తులేని len;               /* length to copy for repeats, bits to drop */
    పూర్ణాంకం ret;                    /* return code */
    స్థిర స్థిరరాశి గుర్తులేని పొట్టి క్రమం[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    ఒకవేళ (strm == Z_NULL || strm->state == Z_NULL)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->సందేశం = Z_NULL;
    state->బహుళకం = TYPE;
    state->చివరి = 0;
    state->whave = 0;
    తదుపరి = strm->తదుపరి_లోపల;
    have = తదుపరి != Z_NULL ? strm->avail_లోపల : 0;
    hold = 0;
    బిట్లు = 0;
    put = state->కిటికీ;
    ఎడమ = state->wsize;

    /* Inflate until end of block marked as last */
    కొరకు (;;)
        ఎంపిక (state->బహుళకం) {
        స్థితి TYPE:
            /* determine and dispatch block type */
            ఒకవేళ (state->చివరి) {
                BYTEBITS();
                state->బహుళకం = పూర్తి;
                ఆపు;
            }
            NEEDBITS(3);
            state->చివరి = బిట్లు_2(1);
            DROPBITS(1);
            ఎంపిక (బిట్లు_2(2)) {
            స్థితి 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->చివరి ? " (last)" : ""));
                state->బహుళకం = STORED;
                ఆపు;
            స్థితి 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->చివరి ? " (last)" : ""));
                state->బహుళకం = LEN;              /* decode codes */
                ఆపు;
            స్థితి 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->చివరి ? " (last)" : ""));
                state->బహుళకం = పట్టిక;
                ఆపు;
            అప్రమేయ:
                strm->సందేశం = (z_const అక్షరం *)"invalid block type";
                state->బహుళకం = BAD;
            }
            DROPBITS(2);
            ఆపు;

        స్థితి STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ఒకవేళ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->సందేశం = (z_const అక్షరం *)"invalid stored block lengths";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->నిడివి = (గుర్తులేని)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->నిడివి));
            INITBITS();

            /* copy stored block from input to output */
            అయితే (state->నిడివి != 0) {
                copy = state->నిడివి;
                PULL();
                స్థలం();
                ఒకవేళ (copy > have) copy = have;
                ఒకవేళ (copy > ఎడమ) copy = ఎడమ;
                zmemcpy(put, తదుపరి, copy);
                have -= copy;
                తదుపరి += copy;
                ఎడమ -= copy;
                put += copy;
                state->నిడివి -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->బహుళకం = TYPE;
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
#నిర్వచితం_కాకపోతే PKZIP_BUG_WORKAROUND
            ఒకవేళ (state->nlen > 286 || state->ndist > 30) {
                strm->సందేశం = (z_const అక్షరం *)
                    "too many length or distance symbols";
                state->బహుళకం = BAD;
                ఆపు;
            }
#అయితే_ముగింపు
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
            state->lencode = (సంకేతం_2 స్థిరరాశి FAR *)(state->తదుపరి);
            state->lenbits = 7;
            ret = inflate_పట్టిక(CODES, state->lens, 19, &(state->తదుపరి),
                                &(state->lenbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid code lengths set";
                state->బహుళకం = BAD;
                ఆపు;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            అయితే (state->have < state->nlen + state->ndist) {
                కొరకు (;;) {
                    ఇక్కడ = state->lencode[బిట్లు_2(state->lenbits)];
                    ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                ఒకవేళ (ఇక్కడ.val < 16) {
                    DROPBITS(ఇక్కడ.బిట్లు);
                    state->lens[state->have++] = ఇక్కడ.val;
                }
                లేకపోతే {
                    ఒకవేళ (ఇక్కడ.val == 16) {
                        NEEDBITS(ఇక్కడ.బిట్లు + 2);
                        DROPBITS(ఇక్కడ.బిట్లు);
                        ఒకవేళ (state->have == 0) {
                            strm->సందేశం = (z_const అక్షరం *)
                                "invalid bit length repeat";
                            state->బహుళకం = BAD;
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
                        strm->సందేశం = (z_const అక్షరం *)
                            "invalid bit length repeat";
                        state->బహుళకం = BAD;
                        ఆపు;
                    }
                    అయితే (copy--)
                        state->lens[state->have++] = (గుర్తులేని పొట్టి)len;
                }
            }

            /* handle error breaks in while */
            ఒకవేళ (state->బహుళకం == BAD) ఆపు;

            /* check for end-of-block code (better have one) */
            ఒకవేళ (state->lens[256] == 0) {
                strm->సందేశం = (z_const అక్షరం *)
                    "invalid code -- missing end-of-block";
                state->బహుళకం = BAD;
                ఆపు;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->తదుపరి = state->codes;
            state->lencode = (సంకేతం_2 స్థిరరాశి FAR *)(state->తదుపరి);
            state->lenbits = 9;
            ret = inflate_పట్టిక(LENS, state->lens, state->nlen, &(state->తదుపరి),
                                &(state->lenbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid literal/lengths set";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->distcode = (సంకేతం_2 స్థిరరాశి FAR *)(state->తదుపరి);
            state->distbits = 6;
            ret = inflate_పట్టిక(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->తదుపరి), &(state->distbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distances set";
                state->బహుళకం = BAD;
                ఆపు;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->బహుళకం = LEN;
                /* fallthrough */

        స్థితి LEN:
            /* use inflate_fast() if we have enough input and output */
            ఒకవేళ (have >= 6 && ఎడమ >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                ఎక్కించు_2();
                ఆపు;
            }

            /* get a literal, length, or end-of-block code */
            కొరకు (;;) {
                ఇక్కడ = state->lencode[బిట్లు_2(state->lenbits)];
                ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                PULLBYTE();
            }
            ఒకవేళ (ఇక్కడ.op && (ఇక్కడ.op & 0xf0) == 0) {
                చివరి = ఇక్కడ;
                కొరకు (;;) {
                    ఇక్కడ = state->lencode[చివరి.val +
                            (బిట్లు_2(చివరి.బిట్లు + చివరి.op) >> చివరి.బిట్లు)];
                    ఒకవేళ ((గుర్తులేని)(చివరి.బిట్లు + ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                DROPBITS(చివరి.బిట్లు);
            }
            DROPBITS(ఇక్కడ.బిట్లు);
            state->నిడివి = (గుర్తులేని)ఇక్కడ.val;

            /* process literal */
            ఒకవేళ (ఇక్కడ.op == 0) {
                Tracevv((stderr, ఇక్కడ.val >= 0x20 && ఇక్కడ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ఇక్కడ.val));
                స్థలం();
                *put++ = (గుర్తులేని అక్షరం)(state->నిడివి);
                ఎడమ--;
                state->బహుళకం = LEN;
                ఆపు;
            }

            /* process end of block */
            ఒకవేళ (ఇక్కడ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->బహుళకం = TYPE;
                ఆపు;
            }

            /* invalid code */
            ఒకవేళ (ఇక్కడ.op & 64) {
                strm->సందేశం = (z_const అక్షరం *)"invalid literal/length code";
                state->బహుళకం = BAD;
                ఆపు;
            }

            /* length code -- get extra bits, if any */
            state->extra = (గుర్తులేని)(ఇక్కడ.op) & 15;
            ఒకవేళ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->నిడివి += బిట్లు_2(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->నిడివి));

            /* get distance code */
            కొరకు (;;) {
                ఇక్కడ = state->distcode[బిట్లు_2(state->distbits)];
                ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                PULLBYTE();
            }
            ఒకవేళ ((ఇక్కడ.op & 0xf0) == 0) {
                చివరి = ఇక్కడ;
                కొరకు (;;) {
                    ఇక్కడ = state->distcode[చివరి.val +
                            (బిట్లు_2(చివరి.బిట్లు + చివరి.op) >> చివరి.బిట్లు)];
                    ఒకవేళ ((గుర్తులేని)(చివరి.బిట్లు + ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                DROPBITS(చివరి.బిట్లు);
            }
            DROPBITS(ఇక్కడ.బిట్లు);
            ఒకవేళ (ఇక్కడ.op & 64) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distance code";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->స్థానభ్రంశం = (గుర్తులేని)ఇక్కడ.val;

            /* get distance extra bits, if any */
            state->extra = (గుర్తులేని)(ఇక్కడ.op) & 15;
            ఒకవేళ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->స్థానభ్రంశం += బిట్లు_2(state->extra);
                DROPBITS(state->extra);
            }
            ఒకవేళ (state->స్థానభ్రంశం > state->wsize - (state->whave < state->wsize ?
                                                ఎడమ : 0)) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distance too far back";
                state->బహుళకం = BAD;
                ఆపు;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->స్థానభ్రంశం));

            /* copy match from window to output */
            చేయి {
                స్థలం();
                copy = state->wsize - state->స్థానభ్రంశం;
                ఒకవేళ (copy < ఎడమ) {
                    from = put + copy;
                    copy = ఎడమ - copy;
                }
                లేకపోతే {
                    from = put - state->స్థానభ్రంశం;
                    copy = ఎడమ;
                }
                ఒకవేళ (copy > state->నిడివి) copy = state->నిడివి;
                state->నిడివి -= copy;
                ఎడమ -= copy;
                చేయి {
                    *put++ = *from++;
                } అయితే (--copy);
            } అయితే (state->నిడివి != 0);
            ఆపు;

        స్థితి పూర్తి:
            /* inflate stream terminated properly */
            ret = Z_ప్రవాహం_చివర;
            వెళ్ళు inf_leave;

        స్థితి BAD:
            ret = Z_దత్తాంశం_ERROR;
            వెళ్ళు inf_leave;

        అప్రమేయ:
            /* can't happen, but makes compilers happy */
            ret = Z_ప్రవాహం_ERROR;
            వెళ్ళు inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    ఒకవేళ (ఎడమ < state->wsize) {
        ఒకవేళ (బహిర్గతం(బహిర్గతం_desc, state->కిటికీ, state->wsize - ఎడమ) &&
            ret == Z_ప్రవాహం_చివర)
            ret = Z_BUF_ERROR;
    }
    strm->తదుపరి_లోపల = తదుపరి;
    strm->avail_లోపల = have;
    తిరిగి ret;
}

పూర్ణాంకం ZEXPORT inflate_వెనుక_చివర(z_streamp strm) {
    ఒకవేళ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        తిరిగి Z_ప్రవాహం_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    తిరిగి Z_OK;
}
