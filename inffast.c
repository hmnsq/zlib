/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

#جيڪڏهن_وصف_ٿيل ASMINF
#  pragma پيغام("Assembler code may have bugs -- use at your own risk")
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
فراغ ZLIB_اندروني inflate_fast(z_streamp strm, بنانشان شروعات) {
    بنية inflate_state FAR *state;
    z_const بنانشان حرف FAR *اندريون;      /* local strm->next_in */
    z_const بنانشان حرف FAR *آخري;    /* have enough input while in < last */
    بنانشان حرف FAR *ٻاهريون;     /* local strm->next_out */
    بنانشان حرف FAR *beg;     /* inflate()'s initial strm->next_out */
    بنانشان حرف FAR *پڇاڙي;     /* while out < end, enough space available */
#جيڪڏهن_وصف_ٿيل INFLATE_STRICT
    بنانشان dmax;              /* maximum distance from zlib header */
#جيڪڏهن_پورو
    بنانشان wsize;             /* window size or zero if not using window */
    بنانشان whave;             /* valid bytes in the window */
    بنانشان wnext;             /* window write index */
    بنانشان حرف FAR *دري;  /* allocated sliding window, if wsize != 0 */
    بنانشان ڊگهو hold;         /* local strm->hold */
    بنانشان بٽ;              /* local strm->bits */
    ڪوڊ ثابت FAR *lcode;      /* local strm->lencode */
    ڪوڊ ثابت FAR *dcode;      /* local strm->distcode */
    بنانشان lmask;             /* mask for first level of length codes */
    بنانشان dmask;             /* mask for first level of distance codes */
    ڪوڊ ثابت *هتي;           /* retrieved table entry */
    بنانشان op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    بنانشان len;               /* match length, unused bytes */
    بنانشان dist;              /* match distance */
    بنانشان حرف FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (بنية inflate_state FAR *)strm->state;
    اندريون = strm->ايندڙ_اندريون;
    آخري = اندريون + (strm->avail_اندريون - 5);
    ٻاهريون = strm->ايندڙ_ٻاهريون;
    beg = ٻاهريون - (شروعات - strm->avail_ٻاهريون);
    پڇاڙي = ٻاهريون + (strm->avail_ٻاهريون - 257);
#جيڪڏهن_وصف_ٿيل INFLATE_STRICT
    dmax = state->dmax;
#جيڪڏهن_پورو
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    دري = state->دري;
    hold = state->hold;
    بٽ = state->بٽ;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    افعل {
        إذا (بٽ < 15) {
            hold += (بنانشان ڊگهو)(*اندريون++) << بٽ;
            بٽ += 8;
            hold += (بنانشان ڊگهو)(*اندريون++) << بٽ;
            بٽ += 8;
        }
        هتي = lcode + (hold & lmask);
      dolen:
        op = (بنانشان)(هتي->بٽ);
        hold >>= op;
        بٽ -= op;
        op = (بنانشان)(هتي->op);
        إذا (op == 0) {                          /* literal */
            Tracevv((stderr, هتي->val >= 0x20 && هتي->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", هتي->val));
            *ٻاهريون++ = (بنانشان حرف)(هتي->val);
        }
        وإلا إذا (op & 16) {                     /* length base */
            len = (بنانشان)(هتي->val);
            op &= 15;                           /* number of extra bits */
            إذا (op) {
                إذا (بٽ < op) {
                    hold += (بنانشان ڊگهو)(*اندريون++) << بٽ;
                    بٽ += 8;
                }
                len += (بنانشان)hold & ((1U << op) - 1);
                hold >>= op;
                بٽ -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            إذا (بٽ < 15) {
                hold += (بنانشان ڊگهو)(*اندريون++) << بٽ;
                بٽ += 8;
                hold += (بنانشان ڊگهو)(*اندريون++) << بٽ;
                بٽ += 8;
            }
            هتي = dcode + (hold & dmask);
          dodist:
            op = (بنانشان)(هتي->بٽ);
            hold >>= op;
            بٽ -= op;
            op = (بنانشان)(هتي->op);
            إذا (op & 16) {                      /* distance base */
                dist = (بنانشان)(هتي->val);
                op &= 15;                       /* number of extra bits */
                إذا (بٽ < op) {
                    hold += (بنانشان ڊگهو)(*اندريون++) << بٽ;
                    بٽ += 8;
                    إذا (بٽ < op) {
                        hold += (بنانشان ڊگهو)(*اندريون++) << بٽ;
                        بٽ += 8;
                    }
                }
                dist += (بنانشان)hold & ((1U << op) - 1);
#جيڪڏهن_وصف_ٿيل INFLATE_STRICT
                إذا (dist > dmax) {
                    strm->نياپو = (z_const حرف *)
                        "invalid distance too far back";
                    state->گهڻائي = BAD;
                    اكسر;
                }
#جيڪڏهن_پورو
                hold >>= op;
                بٽ -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (بنانشان)(ٻاهريون - beg);     /* max distance in output */
                إذا (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    إذا (op > whave) {
                        إذا (state->sane) {
                            strm->نياپو = (z_const حرف *)
                                "invalid distance too far back";
                            state->گهڻائي = BAD;
                            اكسر;
                        }
#جيڪڏهن_وصف_ٿيل INFLATE_ALLOW_INVALID_مفاصلو_TOOFAR_ARRR
                        إذا (len <= op - whave) {
                            افعل {
                                *ٻاهريون++ = 0;
                            } طالما (--len);
                            استمر;
                        }
                        len -= op - whave;
                        افعل {
                            *ٻاهريون++ = 0;
                        } طالما (--op > whave);
                        إذا (op == 0) {
                            from = ٻاهريون - dist;
                            افعل {
                                *ٻاهريون++ = *from++;
                            } طالما (--len);
                            استمر;
                        }
#جيڪڏهن_پورو
                    }
                    from = دري;
                    إذا (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *ٻاهريون++ = *from++;
                            } طالما (--op);
                            from = ٻاهريون - dist;  /* rest from output */
                        }
                    }
                    وإلا إذا (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        إذا (op < len) {         /* some from end of window */
                            len -= op;
                            افعل {
                                *ٻاهريون++ = *from++;
                            } طالما (--op);
                            from = دري;
                            إذا (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                افعل {
                                    *ٻاهريون++ = *from++;
                                } طالما (--op);
                                from = ٻاهريون - dist;      /* rest from output */
                            }
                        }
                    }
                    وإلا {                      /* contiguous in window */
                        from += wnext - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *ٻاهريون++ = *from++;
                            } طالما (--op);
                            from = ٻاهريون - dist;  /* rest from output */
                        }
                    }
                    طالما (len > 2) {
                        *ٻاهريون++ = *from++;
                        *ٻاهريون++ = *from++;
                        *ٻاهريون++ = *from++;
                        len -= 3;
                    }
                    إذا (len) {
                        *ٻاهريون++ = *from++;
                        إذا (len > 1)
                            *ٻاهريون++ = *from++;
                    }
                }
                وإلا {
                    from = ٻاهريون - dist;          /* copy direct from output */
                    افعل {                        /* minimum length is three */
                        *ٻاهريون++ = *from++;
                        *ٻاهريون++ = *from++;
                        *ٻاهريون++ = *from++;
                        len -= 3;
                    } طالما (len > 2);
                    إذا (len) {
                        *ٻاهريون++ = *from++;
                        إذا (len > 1)
                            *ٻاهريون++ = *from++;
                    }
                }
            }
            وإلا إذا ((op & 64) == 0) {          /* 2nd level distance code */
                هتي = dcode + هتي->val + (hold & ((1U << op) - 1));
                اذهب dodist;
            }
            وإلا {
                strm->نياپو = (z_const حرف *)"invalid distance code";
                state->گهڻائي = BAD;
                اكسر;
            }
        }
        وإلا إذا ((op & 64) == 0) {              /* 2nd level length code */
            هتي = lcode + هتي->val + (hold & ((1U << op) - 1));
            اذهب dolen;
        }
        وإلا إذا (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->گهڻائي = TYPE;
            اكسر;
        }
        وإلا {
            strm->نياپو = (z_const حرف *)"invalid literal/length code";
            state->گهڻائي = BAD;
            اكسر;
        }
    } طالما (اندريون < آخري && ٻاهريون < پڇاڙي);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = بٽ >> 3;
    اندريون -= len;
    بٽ -= len << 3;
    hold &= (1U << بٽ) - 1;

    /* update state and return */
    strm->ايندڙ_اندريون = اندريون;
    strm->ايندڙ_ٻاهريون = ٻاهريون;
    strm->avail_اندريون = (بنانشان)(اندريون < آخري ? 5 + (آخري - اندريون) : 5 - (اندريون - آخري));
    strm->avail_ٻاهريون = (بنانشان)(ٻاهريون < پڇاڙي ?
                                 257 + (پڇاڙي - ٻاهريون) : 257 - (ٻاهريون - پڇاڙي));
    state->hold = hold;
    state->بٽ = بٽ;
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

#جيڪڏهن_پورو /* !ASMINF */
