/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

#اگر_تعریف_شده ASMINF
#  pragma پیام_2("Assembler code may have bugs -- use at your own risk")
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
فراغ ZLIB_داخلی inflate_fast(z_streamp strm, بیعلامت شروع) {
    بنية inflate_state FAR *state;
    z_const بیعلامت حرف FAR *درون;      /* local strm->next_in */
    z_const بیعلامت حرف FAR *آخرین;    /* have enough input while in < last */
    بیعلامت حرف FAR *بیرون;     /* local strm->next_out */
    بیعلامت حرف FAR *beg;     /* inflate()'s initial strm->next_out */
    بیعلامت حرف FAR *انتها;     /* while out < end, enough space available */
#اگر_تعریف_شده INFLATE_STRICT
    بیعلامت dmax;              /* maximum distance from zlib header */
#پایان_اگر
    بیعلامت wsize;             /* window size or zero if not using window */
    بیعلامت whave;             /* valid bytes in the window */
    بیعلامت wnext;             /* window write index */
    بیعلامت حرف FAR *پنجره;  /* allocated sliding window, if wsize != 0 */
    بیعلامت دراز hold;         /* local strm->hold */
    بیعلامت بیتها;              /* local strm->bits */
    کد ثابت FAR *lcode;      /* local strm->lencode */
    کد ثابت FAR *dcode;      /* local strm->distcode */
    بیعلامت lmask;             /* mask for first level of length codes */
    بیعلامت dmask;             /* mask for first level of distance codes */
    کد ثابت *اینجا;           /* retrieved table entry */
    بیعلامت op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    بیعلامت len;               /* match length, unused bytes */
    بیعلامت dist;              /* match distance */
    بیعلامت حرف FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (بنية inflate_state FAR *)strm->state;
    درون = strm->بعدی_درون;
    آخرین = درون + (strm->avail_درون - 5);
    بیرون = strm->بعدی_بیرون;
    beg = بیرون - (شروع - strm->avail_بیرون);
    انتها = بیرون + (strm->avail_بیرون - 257);
#اگر_تعریف_شده INFLATE_STRICT
    dmax = state->dmax;
#پایان_اگر
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    پنجره = state->پنجره;
    hold = state->hold;
    بیتها = state->بیتها;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    افعل {
        إذا (بیتها < 15) {
            hold += (بیعلامت دراز)(*درون++) << بیتها;
            بیتها += 8;
            hold += (بیعلامت دراز)(*درون++) << بیتها;
            بیتها += 8;
        }
        اینجا = lcode + (hold & lmask);
      dolen:
        op = (بیعلامت)(اینجا->بیتها);
        hold >>= op;
        بیتها -= op;
        op = (بیعلامت)(اینجا->op);
        إذا (op == 0) {                          /* literal */
            Tracevv((stderr, اینجا->val >= 0x20 && اینجا->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", اینجا->val));
            *بیرون++ = (بیعلامت حرف)(اینجا->val);
        }
        وإلا إذا (op & 16) {                     /* length base */
            len = (بیعلامت)(اینجا->val);
            op &= 15;                           /* number of extra bits */
            إذا (op) {
                إذا (بیتها < op) {
                    hold += (بیعلامت دراز)(*درون++) << بیتها;
                    بیتها += 8;
                }
                len += (بیعلامت)hold & ((1U << op) - 1);
                hold >>= op;
                بیتها -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            إذا (بیتها < 15) {
                hold += (بیعلامت دراز)(*درون++) << بیتها;
                بیتها += 8;
                hold += (بیعلامت دراز)(*درون++) << بیتها;
                بیتها += 8;
            }
            اینجا = dcode + (hold & dmask);
          dodist:
            op = (بیعلامت)(اینجا->بیتها);
            hold >>= op;
            بیتها -= op;
            op = (بیعلامت)(اینجا->op);
            إذا (op & 16) {                      /* distance base */
                dist = (بیعلامت)(اینجا->val);
                op &= 15;                       /* number of extra bits */
                إذا (بیتها < op) {
                    hold += (بیعلامت دراز)(*درون++) << بیتها;
                    بیتها += 8;
                    إذا (بیتها < op) {
                        hold += (بیعلامت دراز)(*درون++) << بیتها;
                        بیتها += 8;
                    }
                }
                dist += (بیعلامت)hold & ((1U << op) - 1);
#اگر_تعریف_شده INFLATE_STRICT
                إذا (dist > dmax) {
                    strm->پیام = (z_const حرف *)
                        "invalid distance too far back";
                    state->نما = BAD;
                    اكسر;
                }
#پایان_اگر
                hold >>= op;
                بیتها -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (بیعلامت)(بیرون - beg);     /* max distance in output */
                إذا (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    إذا (op > whave) {
                        إذا (state->sane) {
                            strm->پیام = (z_const حرف *)
                                "invalid distance too far back";
                            state->نما = BAD;
                            اكسر;
                        }
#اگر_تعریف_شده INFLATE_ALLOW_INVALID_فاصله_TOOFAR_ARRR
                        إذا (len <= op - whave) {
                            افعل {
                                *بیرون++ = 0;
                            } طالما (--len);
                            استمر;
                        }
                        len -= op - whave;
                        افعل {
                            *بیرون++ = 0;
                        } طالما (--op > whave);
                        إذا (op == 0) {
                            from = بیرون - dist;
                            افعل {
                                *بیرون++ = *from++;
                            } طالما (--len);
                            استمر;
                        }
#پایان_اگر
                    }
                    from = پنجره;
                    إذا (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *بیرون++ = *from++;
                            } طالما (--op);
                            from = بیرون - dist;  /* rest from output */
                        }
                    }
                    وإلا إذا (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        إذا (op < len) {         /* some from end of window */
                            len -= op;
                            افعل {
                                *بیرون++ = *from++;
                            } طالما (--op);
                            from = پنجره;
                            إذا (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                افعل {
                                    *بیرون++ = *from++;
                                } طالما (--op);
                                from = بیرون - dist;      /* rest from output */
                            }
                        }
                    }
                    وإلا {                      /* contiguous in window */
                        from += wnext - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *بیرون++ = *from++;
                            } طالما (--op);
                            from = بیرون - dist;  /* rest from output */
                        }
                    }
                    طالما (len > 2) {
                        *بیرون++ = *from++;
                        *بیرون++ = *from++;
                        *بیرون++ = *from++;
                        len -= 3;
                    }
                    إذا (len) {
                        *بیرون++ = *from++;
                        إذا (len > 1)
                            *بیرون++ = *from++;
                    }
                }
                وإلا {
                    from = بیرون - dist;          /* copy direct from output */
                    افعل {                        /* minimum length is three */
                        *بیرون++ = *from++;
                        *بیرون++ = *from++;
                        *بیرون++ = *from++;
                        len -= 3;
                    } طالما (len > 2);
                    إذا (len) {
                        *بیرون++ = *from++;
                        إذا (len > 1)
                            *بیرون++ = *from++;
                    }
                }
            }
            وإلا إذا ((op & 64) == 0) {          /* 2nd level distance code */
                اینجا = dcode + اینجا->val + (hold & ((1U << op) - 1));
                اذهب dodist;
            }
            وإلا {
                strm->پیام = (z_const حرف *)"invalid distance code";
                state->نما = BAD;
                اكسر;
            }
        }
        وإلا إذا ((op & 64) == 0) {              /* 2nd level length code */
            اینجا = lcode + اینجا->val + (hold & ((1U << op) - 1));
            اذهب dolen;
        }
        وإلا إذا (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->نما = TYPE;
            اكسر;
        }
        وإلا {
            strm->پیام = (z_const حرف *)"invalid literal/length code";
            state->نما = BAD;
            اكسر;
        }
    } طالما (درون < آخرین && بیرون < انتها);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = بیتها >> 3;
    درون -= len;
    بیتها -= len << 3;
    hold &= (1U << بیتها) - 1;

    /* update state and return */
    strm->بعدی_درون = درون;
    strm->بعدی_بیرون = بیرون;
    strm->avail_درون = (بیعلامت)(درون < آخرین ? 5 + (آخرین - درون) : 5 - (درون - آخرین));
    strm->avail_بیرون = (بیعلامت)(بیرون < انتها ?
                                 257 + (انتها - بیرون) : 257 - (بیرون - انتها));
    state->hold = hold;
    state->بیتها = بیتها;
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

#پایان_اگر /* !ASMINF */
