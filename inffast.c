/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "zutil.h"
#شامل "inftrees.h"
#شامل "inflate.h"
#شامل "inffast.h"

#ifdef ASMINF
#  pragma message("Assembler code may have bugs -- use at your own risk")
#وگرنه

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
تهی ZLIB_INTERNAL inflate_fast(z_streamp strm, unsigned start) {
    ساختار inflate_state FAR *state;
    z_const unsigned نویسه FAR *in;      /* local strm->next_in */
    z_const unsigned نویسه FAR *last;    /* have enough input while in < last */
    unsigned نویسه FAR *out;     /* local strm->next_out */
    unsigned نویسه FAR *beg;     /* inflate()'s initial strm->next_out */
    unsigned نویسه FAR *end;     /* while out < end, enough space available */
#ifdef INFLATE_STRICT
    unsigned dmax;              /* maximum distance from zlib header */
#endif
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned wnext;             /* window write index */
    unsigned نویسه FAR *window;  /* allocated sliding window, if wsize != 0 */
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
    unsigned نویسه FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (ساختار inflate_state FAR *)strm->state;
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
    انجامبده {
        اگر (bits < 15) {
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
        اگر (op == 0) {                          /* literal */
            Tracevv((stderr, here->val >= 0x20 && here->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", here->val));
            *out++ = (unsigned نویسه)(here->val);
        }
        وگرنه اگر (op & 16) {                     /* length base */
            len = (unsigned)(here->val);
            op &= 15;                           /* number of extra bits */
            اگر (op) {
                اگر (bits < op) {
                    hold += (unsigned long)(*in++) << bits;
                    bits += 8;
                }
                len += (unsigned)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            اگر (bits < 15) {
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
            اگر (op & 16) {                      /* distance base */
                dist = (unsigned)(here->val);
                op &= 15;                       /* number of extra bits */
                اگر (bits < op) {
                    hold += (unsigned long)(*in++) << bits;
                    bits += 8;
                    اگر (bits < op) {
                        hold += (unsigned long)(*in++) << bits;
                        bits += 8;
                    }
                }
                dist += (unsigned)hold & ((1U << op) - 1);
#ifdef INFLATE_STRICT
                اگر (dist > dmax) {
                    strm->پیام = (z_const نویسه *)
                        "invalid distance too far back";
                    state->mode = BAD;
                    بشکن;
                }
#endif
                hold >>= op;
                bits -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (unsigned)(out - beg);     /* max distance in output */
                اگر (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    اگر (op > whave) {
                        اگر (state->sane) {
                            strm->پیام = (z_const نویسه *)
                                "invalid distance too far back";
                            state->mode = BAD;
                            بشکن;
                        }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        اگر (len <= op - whave) {
                            انجامبده {
                                *out++ = 0;
                            } وقتی (--len);
                            ادامه;
                        }
                        len -= op - whave;
                        انجامبده {
                            *out++ = 0;
                        } وقتی (--op > whave);
                        اگر (op == 0) {
                            from = out - dist;
                            انجامبده {
                                *out++ = *from++;
                            } وقتی (--len);
                            ادامه;
                        }
#endif
                    }
                    from = window;
                    اگر (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        اگر (op < len) {         /* some from window */
                            len -= op;
                            انجامبده {
                                *out++ = *from++;
                            } وقتی (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    وگرنه اگر (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        اگر (op < len) {         /* some from end of window */
                            len -= op;
                            انجامبده {
                                *out++ = *from++;
                            } وقتی (--op);
                            from = window;
                            اگر (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                انجامبده {
                                    *out++ = *from++;
                                } وقتی (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    }
                    وگرنه {                      /* contiguous in window */
                        from += wnext - op;
                        اگر (op < len) {         /* some from window */
                            len -= op;
                            انجامبده {
                                *out++ = *from++;
                            } وقتی (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    وقتی (len > 2) {
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    }
                    اگر (len) {
                        *out++ = *from++;
                        اگر (len > 1)
                            *out++ = *from++;
                    }
                }
                وگرنه {
                    from = out - dist;          /* copy direct from output */
                    انجامبده {                        /* minimum length is three */
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    } وقتی (len > 2);
                    اگر (len) {
                        *out++ = *from++;
                        اگر (len > 1)
                            *out++ = *from++;
                    }
                }
            }
            وگرنه اگر ((op & 64) == 0) {          /* 2nd level distance code */
                here = dcode + here->val + (hold & ((1U << op) - 1));
                برو dodist;
            }
            وگرنه {
                strm->پیام = (z_const نویسه *)"invalid distance code";
                state->mode = BAD;
                بشکن;
            }
        }
        وگرنه اگر ((op & 64) == 0) {              /* 2nd level length code */
            here = lcode + here->val + (hold & ((1U << op) - 1));
            برو dolen;
        }
        وگرنه اگر (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->mode = TYPE;
            بشکن;
        }
        وگرنه {
            strm->پیام = (z_const نویسه *)"invalid literal/length code";
            state->mode = BAD;
            بشکن;
        }
    } وقتی (in < last && out < end);

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
    بازگشت;
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
