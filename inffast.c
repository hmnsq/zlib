/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

#که_تعریف_شوی ASMINF
#  pragma پیغام_2("Assembler code may have bugs -- use at your own risk")
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
فراغ ZLIB_دننی inflate_fast(z_streamp strm, بینښه پیل) {
    بنية inflate_state FAR *state;
    z_const بینښه حرف FAR *دننه;      /* local strm->next_in */
    z_const بینښه حرف FAR *وروستی;    /* have enough input while in < last */
    بینښه حرف FAR *بهر;     /* local strm->next_out */
    بینښه حرف FAR *beg;     /* inflate()'s initial strm->next_out */
    بینښه حرف FAR *پای_ټکی;     /* while out < end, enough space available */
#که_تعریف_شوی INFLATE_STRICT
    بینښه dmax;              /* maximum distance from zlib header */
#د_که_پای
    بینښه wsize;             /* window size or zero if not using window */
    بینښه whave;             /* valid bytes in the window */
    بینښه wnext;             /* window write index */
    بینښه حرف FAR *کړکۍ;  /* allocated sliding window, if wsize != 0 */
    بینښه اوږد hold;         /* local strm->hold */
    بینښه بټونه;              /* local strm->bits */
    کوډ ثابت FAR *lcode;      /* local strm->lencode */
    کوډ ثابت FAR *dcode;      /* local strm->distcode */
    بینښه lmask;             /* mask for first level of length codes */
    بینښه dmask;             /* mask for first level of distance codes */
    کوډ ثابت *دلته;           /* retrieved table entry */
    بینښه op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    بینښه len;               /* match length, unused bytes */
    بینښه dist;              /* match distance */
    بینښه حرف FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (بنية inflate_state FAR *)strm->state;
    دننه = strm->بل_دننه;
    وروستی = دننه + (strm->avail_دننه - 5);
    بهر = strm->بل_بهر;
    beg = بهر - (پیل - strm->avail_بهر);
    پای_ټکی = بهر + (strm->avail_بهر - 257);
#که_تعریف_شوی INFLATE_STRICT
    dmax = state->dmax;
#د_که_پای
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    کړکۍ = state->کړکۍ;
    hold = state->hold;
    بټونه = state->بټونه;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    افعل {
        إذا (بټونه < 15) {
            hold += (بینښه اوږد)(*دننه++) << بټونه;
            بټونه += 8;
            hold += (بینښه اوږد)(*دننه++) << بټونه;
            بټونه += 8;
        }
        دلته = lcode + (hold & lmask);
      dolen:
        op = (بینښه)(دلته->بټونه);
        hold >>= op;
        بټونه -= op;
        op = (بینښه)(دلته->op);
        إذا (op == 0) {                          /* literal */
            Tracevv((stderr, دلته->val >= 0x20 && دلته->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", دلته->val));
            *بهر++ = (بینښه حرف)(دلته->val);
        }
        وإلا إذا (op & 16) {                     /* length base */
            len = (بینښه)(دلته->val);
            op &= 15;                           /* number of extra bits */
            إذا (op) {
                إذا (بټونه < op) {
                    hold += (بینښه اوږد)(*دننه++) << بټونه;
                    بټونه += 8;
                }
                len += (بینښه)hold & ((1U << op) - 1);
                hold >>= op;
                بټونه -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            إذا (بټونه < 15) {
                hold += (بینښه اوږد)(*دننه++) << بټونه;
                بټونه += 8;
                hold += (بینښه اوږد)(*دننه++) << بټونه;
                بټونه += 8;
            }
            دلته = dcode + (hold & dmask);
          dodist:
            op = (بینښه)(دلته->بټونه);
            hold >>= op;
            بټونه -= op;
            op = (بینښه)(دلته->op);
            إذا (op & 16) {                      /* distance base */
                dist = (بینښه)(دلته->val);
                op &= 15;                       /* number of extra bits */
                إذا (بټونه < op) {
                    hold += (بینښه اوږد)(*دننه++) << بټونه;
                    بټونه += 8;
                    إذا (بټونه < op) {
                        hold += (بینښه اوږد)(*دننه++) << بټونه;
                        بټونه += 8;
                    }
                }
                dist += (بینښه)hold & ((1U << op) - 1);
#که_تعریف_شوی INFLATE_STRICT
                إذا (dist > dmax) {
                    strm->پیغام = (z_const حرف *)
                        "invalid distance too far back";
                    state->ډېرپېښ = BAD;
                    اكسر;
                }
#د_که_پای
                hold >>= op;
                بټونه -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (بینښه)(بهر - beg);     /* max distance in output */
                إذا (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    إذا (op > whave) {
                        إذا (state->sane) {
                            strm->پیغام = (z_const حرف *)
                                "invalid distance too far back";
                            state->ډېرپېښ = BAD;
                            اكسر;
                        }
#که_تعریف_شوی INFLATE_ALLOW_INVALID_واټن_TOOFAR_ARRR
                        إذا (len <= op - whave) {
                            افعل {
                                *بهر++ = 0;
                            } طالما (--len);
                            استمر;
                        }
                        len -= op - whave;
                        افعل {
                            *بهر++ = 0;
                        } طالما (--op > whave);
                        إذا (op == 0) {
                            from = بهر - dist;
                            افعل {
                                *بهر++ = *from++;
                            } طالما (--len);
                            استمر;
                        }
#د_که_پای
                    }
                    from = کړکۍ;
                    إذا (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *بهر++ = *from++;
                            } طالما (--op);
                            from = بهر - dist;  /* rest from output */
                        }
                    }
                    وإلا إذا (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        إذا (op < len) {         /* some from end of window */
                            len -= op;
                            افعل {
                                *بهر++ = *from++;
                            } طالما (--op);
                            from = کړکۍ;
                            إذا (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                افعل {
                                    *بهر++ = *from++;
                                } طالما (--op);
                                from = بهر - dist;      /* rest from output */
                            }
                        }
                    }
                    وإلا {                      /* contiguous in window */
                        from += wnext - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *بهر++ = *from++;
                            } طالما (--op);
                            from = بهر - dist;  /* rest from output */
                        }
                    }
                    طالما (len > 2) {
                        *بهر++ = *from++;
                        *بهر++ = *from++;
                        *بهر++ = *from++;
                        len -= 3;
                    }
                    إذا (len) {
                        *بهر++ = *from++;
                        إذا (len > 1)
                            *بهر++ = *from++;
                    }
                }
                وإلا {
                    from = بهر - dist;          /* copy direct from output */
                    افعل {                        /* minimum length is three */
                        *بهر++ = *from++;
                        *بهر++ = *from++;
                        *بهر++ = *from++;
                        len -= 3;
                    } طالما (len > 2);
                    إذا (len) {
                        *بهر++ = *from++;
                        إذا (len > 1)
                            *بهر++ = *from++;
                    }
                }
            }
            وإلا إذا ((op & 64) == 0) {          /* 2nd level distance code */
                دلته = dcode + دلته->val + (hold & ((1U << op) - 1));
                اذهب dodist;
            }
            وإلا {
                strm->پیغام = (z_const حرف *)"invalid distance code";
                state->ډېرپېښ = BAD;
                اكسر;
            }
        }
        وإلا إذا ((op & 64) == 0) {              /* 2nd level length code */
            دلته = lcode + دلته->val + (hold & ((1U << op) - 1));
            اذهب dolen;
        }
        وإلا إذا (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->ډېرپېښ = TYPE;
            اكسر;
        }
        وإلا {
            strm->پیغام = (z_const حرف *)"invalid literal/length code";
            state->ډېرپېښ = BAD;
            اكسر;
        }
    } طالما (دننه < وروستی && بهر < پای_ټکی);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = بټونه >> 3;
    دننه -= len;
    بټونه -= len << 3;
    hold &= (1U << بټونه) - 1;

    /* update state and return */
    strm->بل_دننه = دننه;
    strm->بل_بهر = بهر;
    strm->avail_دننه = (بینښه)(دننه < وروستی ? 5 + (وروستی - دننه) : 5 - (دننه - وروستی));
    strm->avail_بهر = (بینښه)(بهر < پای_ټکی ?
                                 257 + (پای_ټکی - بهر) : 257 - (بهر - پای_ټکی));
    state->hold = hold;
    state->بټونه = بټونه;
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

#د_که_پای /* !ASMINF */
