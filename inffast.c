/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "zutil.h"
#شامل "inftrees.h"
#شامل "inflate.h"
#شامل "inffast.h"

#اگر_تعریف_شدہ ASMINF
#  pragma پیغام_2("Assembler code may have bugs -- use at your own risk")
#ورنہ

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
خالی ZLIB_اندرونی inflate_fast(z_streamp strm, غیرعلامتی آغاز) {
    ساخت inflate_state FAR *state;
    z_const غیرعلامتی حرف FAR *اندر;      /* local strm->next_in */
    z_const غیرعلامتی حرف FAR *آخری;    /* have enough input while in < last */
    غیرعلامتی حرف FAR *برآمد;     /* local strm->next_out */
    غیرعلامتی حرف FAR *beg;     /* inflate()'s initial strm->next_out */
    غیرعلامتی حرف FAR *انتہا;     /* while out < end, enough space available */
#اگر_تعریف_شدہ INFLATE_STRICT
    غیرعلامتی dmax;              /* maximum distance from zlib header */
#اگر_ختم
    غیرعلامتی wsize;             /* window size or zero if not using window */
    غیرعلامتی whave;             /* valid bytes in the window */
    غیرعلامتی wnext;             /* window write index */
    غیرعلامتی حرف FAR *کھڑکی;  /* allocated sliding window, if wsize != 0 */
    غیرعلامتی طویل hold;         /* local strm->hold */
    غیرعلامتی بٹ;              /* local strm->bits */
    رمز ثابت FAR *lcode;      /* local strm->lencode */
    رمز ثابت FAR *dcode;      /* local strm->distcode */
    غیرعلامتی lmask;             /* mask for first level of length codes */
    غیرعلامتی dmask;             /* mask for first level of distance codes */
    رمز ثابت *یہاں;           /* retrieved table entry */
    غیرعلامتی op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    غیرعلامتی len;               /* match length, unused bytes */
    غیرعلامتی dist;              /* match distance */
    غیرعلامتی حرف FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (ساخت inflate_state FAR *)strm->state;
    اندر = strm->اگلا_اندر;
    آخری = اندر + (strm->avail_اندر - 5);
    برآمد = strm->اگلا_برآمد;
    beg = برآمد - (آغاز - strm->avail_برآمد);
    انتہا = برآمد + (strm->avail_برآمد - 257);
#اگر_تعریف_شدہ INFLATE_STRICT
    dmax = state->dmax;
#اگر_ختم
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    کھڑکی = state->کھڑکی;
    hold = state->hold;
    بٹ = state->بٹ;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    کرو {
        اگر (بٹ < 15) {
            hold += (غیرعلامتی طویل)(*اندر++) << بٹ;
            بٹ += 8;
            hold += (غیرعلامتی طویل)(*اندر++) << بٹ;
            بٹ += 8;
        }
        یہاں = lcode + (hold & lmask);
      dolen:
        op = (غیرعلامتی)(یہاں->بٹ);
        hold >>= op;
        بٹ -= op;
        op = (غیرعلامتی)(یہاں->op);
        اگر (op == 0) {                          /* literal */
            Tracevv((stderr, یہاں->val >= 0x20 && یہاں->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", یہاں->val));
            *برآمد++ = (غیرعلامتی حرف)(یہاں->val);
        }
        ورنہ اگر (op & 16) {                     /* length base */
            len = (غیرعلامتی)(یہاں->val);
            op &= 15;                           /* number of extra bits */
            اگر (op) {
                اگر (بٹ < op) {
                    hold += (غیرعلامتی طویل)(*اندر++) << بٹ;
                    بٹ += 8;
                }
                len += (غیرعلامتی)hold & ((1U << op) - 1);
                hold >>= op;
                بٹ -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            اگر (بٹ < 15) {
                hold += (غیرعلامتی طویل)(*اندر++) << بٹ;
                بٹ += 8;
                hold += (غیرعلامتی طویل)(*اندر++) << بٹ;
                بٹ += 8;
            }
            یہاں = dcode + (hold & dmask);
          dodist:
            op = (غیرعلامتی)(یہاں->بٹ);
            hold >>= op;
            بٹ -= op;
            op = (غیرعلامتی)(یہاں->op);
            اگر (op & 16) {                      /* distance base */
                dist = (غیرعلامتی)(یہاں->val);
                op &= 15;                       /* number of extra bits */
                اگر (بٹ < op) {
                    hold += (غیرعلامتی طویل)(*اندر++) << بٹ;
                    بٹ += 8;
                    اگر (بٹ < op) {
                        hold += (غیرعلامتی طویل)(*اندر++) << بٹ;
                        بٹ += 8;
                    }
                }
                dist += (غیرعلامتی)hold & ((1U << op) - 1);
#اگر_تعریف_شدہ INFLATE_STRICT
                اگر (dist > dmax) {
                    strm->پیغام = (z_const حرف *)
                        "invalid distance too far back";
                    state->منوال = BAD;
                    توڑو;
                }
#اگر_ختم
                hold >>= op;
                بٹ -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (غیرعلامتی)(برآمد - beg);     /* max distance in output */
                اگر (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    اگر (op > whave) {
                        اگر (state->sane) {
                            strm->پیغام = (z_const حرف *)
                                "invalid distance too far back";
                            state->منوال = BAD;
                            توڑو;
                        }
#اگر_تعریف_شدہ INFLATE_ALLOW_INVALID_فاصلہ_TOOFAR_ARRR
                        اگر (len <= op - whave) {
                            کرو {
                                *برآمد++ = 0;
                            } جب (--len);
                            جاری;
                        }
                        len -= op - whave;
                        کرو {
                            *برآمد++ = 0;
                        } جب (--op > whave);
                        اگر (op == 0) {
                            from = برآمد - dist;
                            کرو {
                                *برآمد++ = *from++;
                            } جب (--len);
                            جاری;
                        }
#اگر_ختم
                    }
                    from = کھڑکی;
                    اگر (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        اگر (op < len) {         /* some from window */
                            len -= op;
                            کرو {
                                *برآمد++ = *from++;
                            } جب (--op);
                            from = برآمد - dist;  /* rest from output */
                        }
                    }
                    ورنہ اگر (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        اگر (op < len) {         /* some from end of window */
                            len -= op;
                            کرو {
                                *برآمد++ = *from++;
                            } جب (--op);
                            from = کھڑکی;
                            اگر (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                کرو {
                                    *برآمد++ = *from++;
                                } جب (--op);
                                from = برآمد - dist;      /* rest from output */
                            }
                        }
                    }
                    ورنہ {                      /* contiguous in window */
                        from += wnext - op;
                        اگر (op < len) {         /* some from window */
                            len -= op;
                            کرو {
                                *برآمد++ = *from++;
                            } جب (--op);
                            from = برآمد - dist;  /* rest from output */
                        }
                    }
                    جب (len > 2) {
                        *برآمد++ = *from++;
                        *برآمد++ = *from++;
                        *برآمد++ = *from++;
                        len -= 3;
                    }
                    اگر (len) {
                        *برآمد++ = *from++;
                        اگر (len > 1)
                            *برآمد++ = *from++;
                    }
                }
                ورنہ {
                    from = برآمد - dist;          /* copy direct from output */
                    کرو {                        /* minimum length is three */
                        *برآمد++ = *from++;
                        *برآمد++ = *from++;
                        *برآمد++ = *from++;
                        len -= 3;
                    } جب (len > 2);
                    اگر (len) {
                        *برآمد++ = *from++;
                        اگر (len > 1)
                            *برآمد++ = *from++;
                    }
                }
            }
            ورنہ اگر ((op & 64) == 0) {          /* 2nd level distance code */
                یہاں = dcode + یہاں->val + (hold & ((1U << op) - 1));
                جاؤ dodist;
            }
            ورنہ {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->منوال = BAD;
                توڑو;
            }
        }
        ورنہ اگر ((op & 64) == 0) {              /* 2nd level length code */
            یہاں = lcode + یہاں->val + (hold & ((1U << op) - 1));
            جاؤ dolen;
        }
        ورنہ اگر (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->منوال = TYPE;
            توڑو;
        }
        ورنہ {
            strm->پیغام = (z_const حرف *)"invalid literal/length code";
            state->منوال = BAD;
            توڑو;
        }
    } جب (اندر < آخری && برآمد < انتہا);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = بٹ >> 3;
    اندر -= len;
    بٹ -= len << 3;
    hold &= (1U << بٹ) - 1;

    /* update state and return */
    strm->اگلا_اندر = اندر;
    strm->اگلا_برآمد = برآمد;
    strm->avail_اندر = (غیرعلامتی)(اندر < آخری ? 5 + (آخری - اندر) : 5 - (اندر - آخری));
    strm->avail_برآمد = (غیرعلامتی)(برآمد < انتہا ?
                                 257 + (انتہا - برآمد) : 257 - (برآمد - انتہا));
    state->hold = hold;
    state->بٹ = بٹ;
    واپس;
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

#اگر_ختم /* !ASMINF */
