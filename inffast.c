/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

#جے_تعریف_شدہ ASMINF
#  pragma سنیہا("Assembler code may have bugs -- use at your own risk")
#وإلا

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
فراغ ZLIB_اندرونی inflate_fast(z_streamp strm, بنانشانی شروع) {
    بنية inflate_state FAR *state;
    z_const بنانشانی حرف FAR *وچ;      /* local strm->next_in */
    z_const بنانشانی حرف FAR *آخری;    /* have enough input while in < last */
    بنانشانی حرف FAR *باہرلا;     /* local strm->next_out */
    بنانشانی حرف FAR *beg;     /* inflate()'s initial strm->next_out */
    بنانشانی حرف FAR *سرا;     /* while out < end, enough space available */
#جے_تعریف_شدہ INFLATE_STRICT
    بنانشانی dmax;              /* maximum distance from zlib header */
#جے_انت
    بنانشانی wsize;             /* window size or zero if not using window */
    بنانشانی whave;             /* valid bytes in the window */
    بنانشانی wnext;             /* window write index */
    بنانشانی حرف FAR *کھڑکی;  /* allocated sliding window, if wsize != 0 */
    بنانشانی لما hold;         /* local strm->hold */
    بنانشانی بٹ;              /* local strm->bits */
    کوڈ ثابت FAR *lcode;      /* local strm->lencode */
    کوڈ ثابت FAR *dcode;      /* local strm->distcode */
    بنانشانی lmask;             /* mask for first level of length codes */
    بنانشانی dmask;             /* mask for first level of distance codes */
    کوڈ ثابت *ایتھے;           /* retrieved table entry */
    بنانشانی op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    بنانشانی len;               /* match length, unused bytes */
    بنانشانی dist;              /* match distance */
    بنانشانی حرف FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (بنية inflate_state FAR *)strm->state;
    وچ = strm->اگلا_وچ;
    آخری = وچ + (strm->avail_وچ - 5);
    باہرلا = strm->اگلا_باہرلا;
    beg = باہرلا - (شروع - strm->avail_باہرلا);
    سرا = باہرلا + (strm->avail_باہرلا - 257);
#جے_تعریف_شدہ INFLATE_STRICT
    dmax = state->dmax;
#جے_انت
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
    افعل {
        إذا (بٹ < 15) {
            hold += (بنانشانی لما)(*وچ++) << بٹ;
            بٹ += 8;
            hold += (بنانشانی لما)(*وچ++) << بٹ;
            بٹ += 8;
        }
        ایتھے = lcode + (hold & lmask);
      dolen:
        op = (بنانشانی)(ایتھے->بٹ);
        hold >>= op;
        بٹ -= op;
        op = (بنانشانی)(ایتھے->op);
        إذا (op == 0) {                          /* literal */
            Tracevv((stderr, ایتھے->val >= 0x20 && ایتھے->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", ایتھے->val));
            *باہرلا++ = (بنانشانی حرف)(ایتھے->val);
        }
        وإلا إذا (op & 16) {                     /* length base */
            len = (بنانشانی)(ایتھے->val);
            op &= 15;                           /* number of extra bits */
            إذا (op) {
                إذا (بٹ < op) {
                    hold += (بنانشانی لما)(*وچ++) << بٹ;
                    بٹ += 8;
                }
                len += (بنانشانی)hold & ((1U << op) - 1);
                hold >>= op;
                بٹ -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            إذا (بٹ < 15) {
                hold += (بنانشانی لما)(*وچ++) << بٹ;
                بٹ += 8;
                hold += (بنانشانی لما)(*وچ++) << بٹ;
                بٹ += 8;
            }
            ایتھے = dcode + (hold & dmask);
          dodist:
            op = (بنانشانی)(ایتھے->بٹ);
            hold >>= op;
            بٹ -= op;
            op = (بنانشانی)(ایتھے->op);
            إذا (op & 16) {                      /* distance base */
                dist = (بنانشانی)(ایتھے->val);
                op &= 15;                       /* number of extra bits */
                إذا (بٹ < op) {
                    hold += (بنانشانی لما)(*وچ++) << بٹ;
                    بٹ += 8;
                    إذا (بٹ < op) {
                        hold += (بنانشانی لما)(*وچ++) << بٹ;
                        بٹ += 8;
                    }
                }
                dist += (بنانشانی)hold & ((1U << op) - 1);
#جے_تعریف_شدہ INFLATE_STRICT
                إذا (dist > dmax) {
                    strm->پیغام = (z_const حرف *)
                        "invalid distance too far back";
                    state->منوال = BAD;
                    اكسر;
                }
#جے_انت
                hold >>= op;
                بٹ -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (بنانشانی)(باہرلا - beg);     /* max distance in output */
                إذا (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    إذا (op > whave) {
                        إذا (state->sane) {
                            strm->پیغام = (z_const حرف *)
                                "invalid distance too far back";
                            state->منوال = BAD;
                            اكسر;
                        }
#جے_تعریف_شدہ INFLATE_ALLOW_INVALID_فاصلہ_TOOFAR_ARRR
                        إذا (len <= op - whave) {
                            افعل {
                                *باہرلا++ = 0;
                            } طالما (--len);
                            استمر;
                        }
                        len -= op - whave;
                        افعل {
                            *باہرلا++ = 0;
                        } طالما (--op > whave);
                        إذا (op == 0) {
                            from = باہرلا - dist;
                            افعل {
                                *باہرلا++ = *from++;
                            } طالما (--len);
                            استمر;
                        }
#جے_انت
                    }
                    from = کھڑکی;
                    إذا (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *باہرلا++ = *from++;
                            } طالما (--op);
                            from = باہرلا - dist;  /* rest from output */
                        }
                    }
                    وإلا إذا (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        إذا (op < len) {         /* some from end of window */
                            len -= op;
                            افعل {
                                *باہرلا++ = *from++;
                            } طالما (--op);
                            from = کھڑکی;
                            إذا (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                افعل {
                                    *باہرلا++ = *from++;
                                } طالما (--op);
                                from = باہرلا - dist;      /* rest from output */
                            }
                        }
                    }
                    وإلا {                      /* contiguous in window */
                        from += wnext - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *باہرلا++ = *from++;
                            } طالما (--op);
                            from = باہرلا - dist;  /* rest from output */
                        }
                    }
                    طالما (len > 2) {
                        *باہرلا++ = *from++;
                        *باہرلا++ = *from++;
                        *باہرلا++ = *from++;
                        len -= 3;
                    }
                    إذا (len) {
                        *باہرلا++ = *from++;
                        إذا (len > 1)
                            *باہرلا++ = *from++;
                    }
                }
                وإلا {
                    from = باہرلا - dist;          /* copy direct from output */
                    افعل {                        /* minimum length is three */
                        *باہرلا++ = *from++;
                        *باہرلا++ = *from++;
                        *باہرلا++ = *from++;
                        len -= 3;
                    } طالما (len > 2);
                    إذا (len) {
                        *باہرلا++ = *from++;
                        إذا (len > 1)
                            *باہرلا++ = *from++;
                    }
                }
            }
            وإلا إذا ((op & 64) == 0) {          /* 2nd level distance code */
                ایتھے = dcode + ایتھے->val + (hold & ((1U << op) - 1));
                اذهب dodist;
            }
            وإلا {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->منوال = BAD;
                اكسر;
            }
        }
        وإلا إذا ((op & 64) == 0) {              /* 2nd level length code */
            ایتھے = lcode + ایتھے->val + (hold & ((1U << op) - 1));
            اذهب dolen;
        }
        وإلا إذا (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->منوال = TYPE;
            اكسر;
        }
        وإلا {
            strm->پیغام = (z_const حرف *)"invalid literal/length code";
            state->منوال = BAD;
            اكسر;
        }
    } طالما (وچ < آخری && باہرلا < سرا);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = بٹ >> 3;
    وچ -= len;
    بٹ -= len << 3;
    hold &= (1U << بٹ) - 1;

    /* update state and return */
    strm->اگلا_وچ = وچ;
    strm->اگلا_باہرلا = باہرلا;
    strm->avail_وچ = (بنانشانی)(وچ < آخری ? 5 + (آخری - وچ) : 5 - (وچ - آخری));
    strm->avail_باہرلا = (بنانشانی)(باہرلا < سرا ?
                                 257 + (سرا - باہرلا) : 257 - (باہرلا - سرا));
    state->hold = hold;
    state->بٹ = بٹ;
    أرجع;
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

#جے_انت /* !ASMINF */
