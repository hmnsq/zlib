/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "zutil.h"
#अन्तर्भाव "inftrees.h"
#अन्तर्भाव "inflate.h"
#अन्तर्भाव "inffast.h"

#ifdef ASMINF
#  pragma message("Assembler code may have bugs -- use at your own risk")
#अन्यथा

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
शून्य ZLIB_INTERNAL inflate_fast(z_streamp strm, अचिन्हित start) {
    संरचना inflate_state FAR *state;
    z_const अचिन्हित वर्ण FAR *in;      /* local strm->next_in */
    z_const अचिन्हित वर्ण FAR *last;    /* have enough input while in < last */
    अचिन्हित वर्ण FAR *out;     /* local strm->next_out */
    अचिन्हित वर्ण FAR *beg;     /* inflate()'s initial strm->next_out */
    अचिन्हित वर्ण FAR *end;     /* while out < end, enough space available */
#ifdef INFLATE_STRICT
    अचिन्हित dmax;              /* maximum distance from zlib header */
#endif
    अचिन्हित wsize;             /* window size or zero if not using window */
    अचिन्हित whave;             /* valid bytes in the window */
    अचिन्हित wnext;             /* window write index */
    अचिन्हित वर्ण FAR *window;  /* allocated sliding window, if wsize != 0 */
    अचिन्हित दीर्घ hold;         /* local strm->hold */
    अचिन्हित bits;              /* local strm->bits */
    code ध्रुव FAR *lcode;      /* local strm->lencode */
    code ध्रुव FAR *dcode;      /* local strm->distcode */
    अचिन्हित lmask;             /* mask for first level of length codes */
    अचिन्हित dmask;             /* mask for first level of distance codes */
    code ध्रुव *here;           /* retrieved table entry */
    अचिन्हित op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    अचिन्हित len;               /* match length, unused bytes */
    अचिन्हित dist;              /* match distance */
    अचिन्हित वर्ण FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (संरचना inflate_state FAR *)strm->state;
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
    कुरु {
        यदि (bits < 15) {
            hold += (अचिन्हित दीर्घ)(*in++) << bits;
            bits += 8;
            hold += (अचिन्हित दीर्घ)(*in++) << bits;
            bits += 8;
        }
        here = lcode + (hold & lmask);
      dolen:
        op = (अचिन्हित)(here->bits);
        hold >>= op;
        bits -= op;
        op = (अचिन्हित)(here->op);
        यदि (op == 0) {                          /* literal */
            Tracevv((मानक_त्रुटि, here->val >= 0x20 && here->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", here->val));
            *out++ = (अचिन्हित वर्ण)(here->val);
        }
        अन्यथा यदि (op & 16) {                     /* length base */
            len = (अचिन्हित)(here->val);
            op &= 15;                           /* number of extra bits */
            यदि (op) {
                यदि (bits < op) {
                    hold += (अचिन्हित दीर्घ)(*in++) << bits;
                    bits += 8;
                }
                len += (अचिन्हित)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %u\n", len));
            यदि (bits < 15) {
                hold += (अचिन्हित दीर्घ)(*in++) << bits;
                bits += 8;
                hold += (अचिन्हित दीर्घ)(*in++) << bits;
                bits += 8;
            }
            here = dcode + (hold & dmask);
          dodist:
            op = (अचिन्हित)(here->bits);
            hold >>= op;
            bits -= op;
            op = (अचिन्हित)(here->op);
            यदि (op & 16) {                      /* distance base */
                dist = (अचिन्हित)(here->val);
                op &= 15;                       /* number of extra bits */
                यदि (bits < op) {
                    hold += (अचिन्हित दीर्घ)(*in++) << bits;
                    bits += 8;
                    यदि (bits < op) {
                        hold += (अचिन्हित दीर्घ)(*in++) << bits;
                        bits += 8;
                    }
                }
                dist += (अचिन्हित)hold & ((1U << op) - 1);
#ifdef INFLATE_STRICT
                यदि (dist > dmax) {
                    strm->संदेश = (z_const वर्ण *)
                        "invalid distance too far back";
                    state->mode = BAD;
                    भङ्ग;
                }
#endif
                hold >>= op;
                bits -= op;
                Tracevv((मानक_त्रुटि, "inflate:         distance %u\n", dist));
                op = (अचिन्हित)(out - beg);     /* max distance in output */
                यदि (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    यदि (op > whave) {
                        यदि (state->sane) {
                            strm->संदेश = (z_const वर्ण *)
                                "invalid distance too far back";
                            state->mode = BAD;
                            भङ्ग;
                        }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        यदि (len <= op - whave) {
                            कुरु {
                                *out++ = 0;
                            } यावत् (--len);
                            अनुवर्त;
                        }
                        len -= op - whave;
                        कुरु {
                            *out++ = 0;
                        } यावत् (--op > whave);
                        यदि (op == 0) {
                            from = out - dist;
                            कुरु {
                                *out++ = *from++;
                            } यावत् (--len);
                            अनुवर्त;
                        }
#endif
                    }
                    from = window;
                    यदि (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        यदि (op < len) {         /* some from window */
                            len -= op;
                            कुरु {
                                *out++ = *from++;
                            } यावत् (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    अन्यथा यदि (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        यदि (op < len) {         /* some from end of window */
                            len -= op;
                            कुरु {
                                *out++ = *from++;
                            } यावत् (--op);
                            from = window;
                            यदि (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                कुरु {
                                    *out++ = *from++;
                                } यावत् (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    }
                    अन्यथा {                      /* contiguous in window */
                        from += wnext - op;
                        यदि (op < len) {         /* some from window */
                            len -= op;
                            कुरु {
                                *out++ = *from++;
                            } यावत् (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    यावत् (len > 2) {
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    }
                    यदि (len) {
                        *out++ = *from++;
                        यदि (len > 1)
                            *out++ = *from++;
                    }
                }
                अन्यथा {
                    from = out - dist;          /* copy direct from output */
                    कुरु {                        /* minimum length is three */
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    } यावत् (len > 2);
                    यदि (len) {
                        *out++ = *from++;
                        यदि (len > 1)
                            *out++ = *from++;
                    }
                }
            }
            अन्यथा यदि ((op & 64) == 0) {          /* 2nd level distance code */
                here = dcode + here->val + (hold & ((1U << op) - 1));
                गच्छ dodist;
            }
            अन्यथा {
                strm->संदेश = (z_const वर्ण *)"invalid distance code";
                state->mode = BAD;
                भङ्ग;
            }
        }
        अन्यथा यदि ((op & 64) == 0) {              /* 2nd level length code */
            here = lcode + here->val + (hold & ((1U << op) - 1));
            गच्छ dolen;
        }
        अन्यथा यदि (op & 32) {                     /* end-of-block */
            Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
            state->mode = TYPE;
            भङ्ग;
        }
        अन्यथा {
            strm->संदेश = (z_const वर्ण *)"invalid literal/length code";
            state->mode = BAD;
            भङ्ग;
        }
    } यावत् (in < last && out < end);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    /* update state and return */
    strm->next_in = in;
    strm->next_out = out;
    strm->avail_in = (अचिन्हित)(in < last ? 5 + (last - in) : 5 - (in - last));
    strm->avail_out = (अचिन्हित)(out < end ?
                                 257 + (end - out) : 257 - (out - end));
    state->hold = hold;
    state->bits = bits;
    प्रत्यागम;
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
