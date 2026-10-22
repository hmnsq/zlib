/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#అన్తర్భావ "zutil.h"
#అన్తర్భావ "inftrees.h"
#అన్తర్భావ "inflate.h"
#అన్తర్భావ "inffast.h"

#నిర్వచితమైతే ASMINF
#  pragma సందేశం_2("Assembler code may have bugs -- use at your own risk")
#లేకపోతే

/*
   Decode literal, length, and distance codes and write out the resulting
   literal and match bytes until either not enough input or output is
   available, an end-of-block is encountered, or a data error is encountered.
   When large enough input and output buffers are supplied to inflate(), for
   example, a 16K input buffer and a 64K output buffer, more than 95% of the
   inflate execution time is spent in this routine.

   Entry assumptions:

        state->mode == LEN
        strm->avail_in >= 6
        strm->avail_out >= 258
        start >= strm->avail_out
        state->bits < 8

   On return, state->mode is one of:

        LEN -- ran out of enough output space or enough available input
        TYPE -- reached end of block code, inflate() to interpret next block
        BAD -- error in block data

   Notes:

    - The maximum input bits used by a length/distance pair is 15 bits for the
      length code, 5 bits for the length extra, 15 bits for the distance code,
      and 13 bits for the distance extra.  This totals 48 bits, or six bytes.
      Therefore if strm->avail_in >= 6, then there is enough input to avoid
      checking for available input while decoding.

    - The maximum bytes that a single length/distance pair can output is 258
      bytes, which is the maximum length that can be coded.  inflate_fast()
      requires strm->avail_out >= 258 for each loop to avoid checking for
      output space.
 */
శూన్య ZLIB_అంతర్గత inflate_fast(z_streamp strm, గుర్తులేని ప్రారంభం) {
    నిర్మాణం inflate_state FAR *state;
    z_const గుర్తులేని అక్షరం FAR *లోపల;      /* local strm->next_in */
    z_const గుర్తులేని అక్షరం FAR *చివరి;    /* have enough input while in < last */
    గుర్తులేని అక్షరం FAR *బహిర్గతం;     /* local strm->next_out */
    గుర్తులేని అక్షరం FAR *beg;     /* inflate()'s initial strm->next_out */
    గుర్తులేని అక్షరం FAR *చివర;     /* while out < end, enough space available */
#నిర్వచితమైతే INFLATE_STRICT
    గుర్తులేని dmax;              /* maximum distance from zlib header */
#అయితే_ముగింపు
    గుర్తులేని wsize;             /* window size or zero if not using window */
    గుర్తులేని whave;             /* valid bytes in the window */
    గుర్తులేని wnext;             /* window write index */
    గుర్తులేని అక్షరం FAR *కిటికీ;  /* allocated sliding window, if wsize != 0 */
    గుర్తులేని దీర్ఘ hold;         /* local strm->hold */
    గుర్తులేని బిట్లు;              /* local strm->bits */
    సంకేతం_2 స్థిరరాశి FAR *lcode;      /* local strm->lencode */
    సంకేతం_2 స్థిరరాశి FAR *dcode;      /* local strm->distcode */
    గుర్తులేని lmask;             /* mask for first level of length codes */
    గుర్తులేని dmask;             /* mask for first level of distance codes */
    సంకేతం_2 స్థిరరాశి *ఇక్కడ;           /* retrieved table entry */
    గుర్తులేని op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    గుర్తులేని len;               /* match length, unused bytes */
    గుర్తులేని dist;              /* match distance */
    గుర్తులేని అక్షరం FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    లోపల = strm->తదుపరి_లోపల;
    చివరి = లోపల + (strm->avail_లోపల - 5);
    బహిర్గతం = strm->తదుపరి_బహిర్గతం;
    beg = బహిర్గతం - (ప్రారంభం - strm->avail_బహిర్గతం);
    చివర = బహిర్గతం + (strm->avail_బహిర్గతం - 257);
#నిర్వచితమైతే INFLATE_STRICT
    dmax = state->dmax;
#అయితే_ముగింపు
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    కిటికీ = state->కిటికీ;
    hold = state->hold;
    బిట్లు = state->బిట్లు;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    చేయి {
        ఒకవేళ (బిట్లు < 15) {
            hold += (గుర్తులేని దీర్ఘ)(*లోపల++) << బిట్లు;
            బిట్లు += 8;
            hold += (గుర్తులేని దీర్ఘ)(*లోపల++) << బిట్లు;
            బిట్లు += 8;
        }
        ఇక్కడ = lcode + (hold & lmask);
      dolen:
        op = (గుర్తులేని)(ఇక్కడ->బిట్లు);
        hold >>= op;
        బిట్లు -= op;
        op = (గుర్తులేని)(ఇక్కడ->op);
        ఒకవేళ (op == 0) {                          /* literal */
            Tracevv((stderr, ఇక్కడ->val >= 0x20 && ఇక్కడ->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", ఇక్కడ->val));
            *బహిర్గతం++ = (గుర్తులేని అక్షరం)(ఇక్కడ->val);
        }
        లేకపోతే ఒకవేళ (op & 16) {                     /* length base */
            len = (గుర్తులేని)(ఇక్కడ->val);
            op &= 15;                           /* number of extra bits */
            ఒకవేళ (op) {
                ఒకవేళ (బిట్లు < op) {
                    hold += (గుర్తులేని దీర్ఘ)(*లోపల++) << బిట్లు;
                    బిట్లు += 8;
                }
                len += (గుర్తులేని)hold & ((1U << op) - 1);
                hold >>= op;
                బిట్లు -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            ఒకవేళ (బిట్లు < 15) {
                hold += (గుర్తులేని దీర్ఘ)(*లోపల++) << బిట్లు;
                బిట్లు += 8;
                hold += (గుర్తులేని దీర్ఘ)(*లోపల++) << బిట్లు;
                బిట్లు += 8;
            }
            ఇక్కడ = dcode + (hold & dmask);
          dodist:
            op = (గుర్తులేని)(ఇక్కడ->బిట్లు);
            hold >>= op;
            బిట్లు -= op;
            op = (గుర్తులేని)(ఇక్కడ->op);
            ఒకవేళ (op & 16) {                      /* distance base */
                dist = (గుర్తులేని)(ఇక్కడ->val);
                op &= 15;                       /* number of extra bits */
                ఒకవేళ (బిట్లు < op) {
                    hold += (గుర్తులేని దీర్ఘ)(*లోపల++) << బిట్లు;
                    బిట్లు += 8;
                    ఒకవేళ (బిట్లు < op) {
                        hold += (గుర్తులేని దీర్ఘ)(*లోపల++) << బిట్లు;
                        బిట్లు += 8;
                    }
                }
                dist += (గుర్తులేని)hold & ((1U << op) - 1);
#నిర్వచితమైతే INFLATE_STRICT
                ఒకవేళ (dist > dmax) {
                    strm->సందేశం = (z_const అక్షరం *)
                        "invalid distance too far back";
                    state->బహుళకం = BAD;
                    ఆపు;
                }
#అయితే_ముగింపు
                hold >>= op;
                బిట్లు -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (గుర్తులేని)(బహిర్గతం - beg);     /* max distance in output */
                ఒకవేళ (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    ఒకవేళ (op > whave) {
                        ఒకవేళ (state->sane) {
                            strm->సందేశం = (z_const అక్షరం *)
                                "invalid distance too far back";
                            state->బహుళకం = BAD;
                            ఆపు;
                        }
#నిర్వచితమైతే INFLATE_ALLOW_INVALID_దూరం_TOOFAR_ARRR
                        ఒకవేళ (len <= op - whave) {
                            చేయి {
                                *బహిర్గతం++ = 0;
                            } అయితే (--len);
                            కొనసాగు;
                        }
                        len -= op - whave;
                        చేయి {
                            *బహిర్గతం++ = 0;
                        } అయితే (--op > whave);
                        ఒకవేళ (op == 0) {
                            from = బహిర్గతం - dist;
                            చేయి {
                                *బహిర్గతం++ = *from++;
                            } అయితే (--len);
                            కొనసాగు;
                        }
#అయితే_ముగింపు
                    }
                    from = కిటికీ;
                    ఒకవేళ (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        ఒకవేళ (op < len) {         /* some from window */
                            len -= op;
                            చేయి {
                                *బహిర్గతం++ = *from++;
                            } అయితే (--op);
                            from = బహిర్గతం - dist;  /* rest from output */
                        }
                    }
                    లేకపోతే ఒకవేళ (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        ఒకవేళ (op < len) {         /* some from end of window */
                            len -= op;
                            చేయి {
                                *బహిర్గతం++ = *from++;
                            } అయితే (--op);
                            from = కిటికీ;
                            ఒకవేళ (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                చేయి {
                                    *బహిర్గతం++ = *from++;
                                } అయితే (--op);
                                from = బహిర్గతం - dist;      /* rest from output */
                            }
                        }
                    }
                    లేకపోతే {                      /* contiguous in window */
                        from += wnext - op;
                        ఒకవేళ (op < len) {         /* some from window */
                            len -= op;
                            చేయి {
                                *బహిర్గతం++ = *from++;
                            } అయితే (--op);
                            from = బహిర్గతం - dist;  /* rest from output */
                        }
                    }
                    అయితే (len > 2) {
                        *బహిర్గతం++ = *from++;
                        *బహిర్గతం++ = *from++;
                        *బహిర్గతం++ = *from++;
                        len -= 3;
                    }
                    ఒకవేళ (len) {
                        *బహిర్గతం++ = *from++;
                        ఒకవేళ (len > 1)
                            *బహిర్గతం++ = *from++;
                    }
                }
                లేకపోతే {
                    from = బహిర్గతం - dist;          /* copy direct from output */
                    చేయి {                        /* minimum length is three */
                        *బహిర్గతం++ = *from++;
                        *బహిర్గతం++ = *from++;
                        *బహిర్గతం++ = *from++;
                        len -= 3;
                    } అయితే (len > 2);
                    ఒకవేళ (len) {
                        *బహిర్గతం++ = *from++;
                        ఒకవేళ (len > 1)
                            *బహిర్గతం++ = *from++;
                    }
                }
            }
            లేకపోతే ఒకవేళ ((op & 64) == 0) {          /* 2nd level distance code */
                ఇక్కడ = dcode + ఇక్కడ->val + (hold & ((1U << op) - 1));
                వెళ్ళు dodist;
            }
            లేకపోతే {
                strm->సందేశం = (z_const అక్షరం *)"invalid distance code";
                state->బహుళకం = BAD;
                ఆపు;
            }
        }
        లేకపోతే ఒకవేళ ((op & 64) == 0) {              /* 2nd level length code */
            ఇక్కడ = lcode + ఇక్కడ->val + (hold & ((1U << op) - 1));
            వెళ్ళు dolen;
        }
        లేకపోతే ఒకవేళ (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->బహుళకం = TYPE;
            ఆపు;
        }
        లేకపోతే {
            strm->సందేశం = (z_const అక్షరం *)"invalid literal/length code";
            state->బహుళకం = BAD;
            ఆపు;
        }
    } అయితే (లోపల < చివరి && బహిర్గతం < చివర);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = బిట్లు >> 3;
    లోపల -= len;
    బిట్లు -= len << 3;
    hold &= (1U << బిట్లు) - 1;

    /* update state and return */
    strm->తదుపరి_లోపల = లోపల;
    strm->తదుపరి_బహిర్గతం = బహిర్గతం;
    strm->avail_లోపల = (గుర్తులేని)(లోపల < చివరి ? 5 + (చివరి - లోపల) : 5 - (లోపల - చివరి));
    strm->avail_బహిర్గతం = (గుర్తులేని)(బహిర్గతం < చివర ?
                                 257 + (చివర - బహిర్గతం) : 257 - (బహిర్గతం - చివర));
    state->hold = hold;
    state->బిట్లు = బిట్లు;
    తిరిగి;
}

/*
   inflate_fast() speedups that turned out slower (on a PowerPC G3 750CXe):
   - Using bit fields for code structure
   - Different op definition to avoid & for extra bits (do & for table bits)
   - Three separate decoding do-loops for direct, window, and wnext == 0
   - Special case for distance > 1 copies to do overlapped load and store copy
   - Explicit branch predictions (based on measured branch probabilities)
   - Deferring match copy and interspersed it with decoding subsequent codes
   - Swapping literal/length else
   - Swapping window/direct else
   - Larger unrolled copy loops (three is about right)
   - Moving len -= 3 statement into middle of loop
 */

#అయితే_ముగింపు /* !ASMINF */
