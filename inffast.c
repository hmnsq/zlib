/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

#ifdef ASMINF
#  pragma message("Assembler code may have bugs -- use at your own risk")
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
فراغ ZLIB_INTERNAL inflate_fast(z_streamp strm, unsigned start) {
    بنية inflate_state FAR *state;
    z_const unsigned حرف FAR *in;      /* local strm->next_in */
    z_const unsigned حرف FAR *last;    /* have enough input while in < last */
    unsigned حرف FAR *out;     /* local strm->next_out */
    unsigned حرف FAR *beg;     /* inflate()'s initial strm->next_out */
    unsigned حرف FAR *end;     /* while out < end, enough space available */
#ifdef INFLATE_STRICT
    unsigned dmax;              /* maximum distance from zlib header */
#endif
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned wnext;             /* window write index */
    unsigned حرف FAR *window;  /* allocated sliding window, if wsize != 0 */
    unsigned long hold;         /* local strm->hold */
    unsigned bits;              /* local strm->bits */
    code ثابت FAR *lcode;      /* local strm->lencode */
    code ثابت FAR *dcode;      /* local strm->distcode */
    unsigned lmask;             /* mask for first level of length codes */
    unsigned dmask;             /* mask for first level of distance codes */
    code ثابت *here;           /* retrieved table entry */
    unsigned op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    unsigned len;               /* match length, unused bytes */
    unsigned dist;              /* match distance */
    unsigned حرف FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (بنية inflate_state FAR *)strm->state;
    in = strm->next_in;
    last = in + (strm->avail_in - 5);
    out = strm->next_out;
    beg = out - (start - strm->avail_out);
    end = out + (strm->avail_out - 257);
#ifdef INFLATE_STRICT
    dmax = state->dmax;
#endif
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    window = state->window;
    hold = state->hold;
    bits = state->bits;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    افعل {
        إذا (bits < 15) {
            hold += (unsigned long)(*in++) << bits;
            bits += 8;
            hold += (unsigned long)(*in++) << bits;
            bits += 8;
        }
        here = lcode + (hold & lmask);
      dolen:
        op = (unsigned)(here->bits);
        hold >>= op;
        bits -= op;
        op = (unsigned)(here->op);
        إذا (op == 0) {                          /* literal */
            Tracevv((stderr, here->val >= 0x20 && here->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", here->val));
            *out++ = (unsigned حرف)(here->val);
        }
        وإلا إذا (op & 16) {                     /* length base */
            len = (unsigned)(here->val);
            op &= 15;                           /* number of extra bits */
            إذا (op) {
                إذا (bits < op) {
                    hold += (unsigned long)(*in++) << bits;
                    bits += 8;
                }
                len += (unsigned)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            إذا (bits < 15) {
                hold += (unsigned long)(*in++) << bits;
                bits += 8;
                hold += (unsigned long)(*in++) << bits;
                bits += 8;
            }
            here = dcode + (hold & dmask);
          dodist:
            op = (unsigned)(here->bits);
            hold >>= op;
            bits -= op;
            op = (unsigned)(here->op);
            إذا (op & 16) {                      /* distance base */
                dist = (unsigned)(here->val);
                op &= 15;                       /* number of extra bits */
                إذا (bits < op) {
                    hold += (unsigned long)(*in++) << bits;
                    bits += 8;
                    إذا (bits < op) {
                        hold += (unsigned long)(*in++) << bits;
                        bits += 8;
                    }
                }
                dist += (unsigned)hold & ((1U << op) - 1);
#ifdef INFLATE_STRICT
                إذا (dist > dmax) {
                    strm->ڤسنن = (z_const حرف *)
                        "invalid distance too far back";
                    state->mode = BAD;
                    اكسر;
                }
#endif
                hold >>= op;
                bits -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (unsigned)(out - beg);     /* max distance in output */
                إذا (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    إذا (op > whave) {
                        إذا (state->sane) {
                            strm->ڤسنن = (z_const حرف *)
                                "invalid distance too far back";
                            state->mode = BAD;
                            اكسر;
                        }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        إذا (len <= op - whave) {
                            افعل {
                                *out++ = 0;
                            } طالما (--len);
                            استمر;
                        }
                        len -= op - whave;
                        افعل {
                            *out++ = 0;
                        } طالما (--op > whave);
                        إذا (op == 0) {
                            from = out - dist;
                            افعل {
                                *out++ = *from++;
                            } طالما (--len);
                            استمر;
                        }
#endif
                    }
                    from = window;
                    إذا (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *out++ = *from++;
                            } طالما (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    وإلا إذا (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        إذا (op < len) {         /* some from end of window */
                            len -= op;
                            افعل {
                                *out++ = *from++;
                            } طالما (--op);
                            from = window;
                            إذا (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                افعل {
                                    *out++ = *from++;
                                } طالما (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    }
                    وإلا {                      /* contiguous in window */
                        from += wnext - op;
                        إذا (op < len) {         /* some from window */
                            len -= op;
                            افعل {
                                *out++ = *from++;
                            } طالما (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    طالما (len > 2) {
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    }
                    إذا (len) {
                        *out++ = *from++;
                        إذا (len > 1)
                            *out++ = *from++;
                    }
                }
                وإلا {
                    from = out - dist;          /* copy direct from output */
                    افعل {                        /* minimum length is three */
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    } طالما (len > 2);
                    إذا (len) {
                        *out++ = *from++;
                        إذا (len > 1)
                            *out++ = *from++;
                    }
                }
            }
            وإلا إذا ((op & 64) == 0) {          /* 2nd level distance code */
                here = dcode + here->val + (hold & ((1U << op) - 1));
                اذهب dodist;
            }
            وإلا {
                strm->ڤسنن = (z_const حرف *)"invalid distance code";
                state->mode = BAD;
                اكسر;
            }
        }
        وإلا إذا ((op & 64) == 0) {              /* 2nd level length code */
            here = lcode + here->val + (hold & ((1U << op) - 1));
            اذهب dolen;
        }
        وإلا إذا (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->mode = TYPE;
            اكسر;
        }
        وإلا {
            strm->ڤسنن = (z_const حرف *)"invalid literal/length code";
            state->mode = BAD;
            اكسر;
        }
    } طالما (in < last && out < end);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    /* update state and return */
    strm->next_in = in;
    strm->next_out = out;
    strm->avail_in = (unsigned)(in < last ? 5 + (last - in) : 5 - (in - last));
    strm->avail_out = (unsigned)(out < end ?
                                 257 + (end - out) : 257 - (out - end));
    state->hold = hold;
    state->bits = bits;
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

#endif /* !ASMINF */
