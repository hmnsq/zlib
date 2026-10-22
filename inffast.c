/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#समावेश "zutil.h"
#समावेश "inftrees.h"
#समावेश "inflate.h"
#समावेश "inffast.h"

#अगर_घोषित ASMINF
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
व्योम ZLIB_INTERNAL inflate_fast(z_streamp strm, अचिन्हित start) {
    काष्ठा inflate_state FAR *state;
    z_const अचिन्हित अक्षर FAR *in;      /* local strm->next_in */
    z_const अचिन्हित अक्षर FAR *last;    /* have enough input while in < last */
    अचिन्हित अक्षर FAR *out;     /* local strm->next_out */
    अचिन्हित अक्षर FAR *beg;     /* inflate()'s initial strm->next_out */
    अचिन्हित अक्षर FAR *end;     /* while out < end, enough space available */
#अगर_घोषित INFLATE_STRICT
    अचिन्हित dmax;              /* maximum distance from zlib header */
#पूर्ण_अगर
    अचिन्हित wsize;             /* window size or zero if not using window */
    अचिन्हित whave;             /* valid bytes in the window */
    अचिन्हित wnext;             /* window write index */
    अचिन्हित अक्षर FAR *window;  /* allocated sliding window, if wsize != 0 */
    अचिन्हित दीर्घ hold;         /* local strm->hold */
    अचिन्हित bits;              /* local strm->bits */
    code स्थिर FAR *lcode;      /* local strm->lencode */
    code स्थिर FAR *dcode;      /* local strm->distcode */
    अचिन्हित lmask;             /* mask for first level of length codes */
    अचिन्हित dmask;             /* mask for first level of distance codes */
    code स्थिर *here;           /* retrieved table entry */
    अचिन्हित op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    अचिन्हित len;               /* match length, unused bytes */
    अचिन्हित dist;              /* match distance */
    अचिन्हित अक्षर FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (काष्ठा inflate_state FAR *)strm->state;
    in = strm->अग्र_in;
    last = in + (strm->avail_in - 5);
    out = strm->अग्र_out;
    beg = out - (start - strm->avail_out);
    end = out + (strm->avail_out - 257);
#अगर_घोषित INFLATE_STRICT
    dmax = state->dmax;
#पूर्ण_अगर
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
    करो {
        अगर (bits < 15) {
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
        अगर (op == 0) {                          /* literal */
            Tracevv((मानक_त्रुटि, here->val >= 0x20 && here->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", here->val));
            *out++ = (अचिन्हित अक्षर)(here->val);
        }
        अन्यथा अगर (op & 16) {                     /* length base */
            len = (अचिन्हित)(here->val);
            op &= 15;                           /* number of extra bits */
            अगर (op) {
                अगर (bits < op) {
                    hold += (अचिन्हित दीर्घ)(*in++) << bits;
                    bits += 8;
                }
                len += (अचिन्हित)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %u\n", len));
            अगर (bits < 15) {
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
            अगर (op & 16) {                      /* distance base */
                dist = (अचिन्हित)(here->val);
                op &= 15;                       /* number of extra bits */
                अगर (bits < op) {
                    hold += (अचिन्हित दीर्घ)(*in++) << bits;
                    bits += 8;
                    अगर (bits < op) {
                        hold += (अचिन्हित दीर्घ)(*in++) << bits;
                        bits += 8;
                    }
                }
                dist += (अचिन्हित)hold & ((1U << op) - 1);
#अगर_घोषित INFLATE_STRICT
                अगर (dist > dmax) {
                    strm->संदेश = (z_const अक्षर *)
                        "invalid distance too far back";
                    state->mode = BAD;
                    अवरोध;
                }
#पूर्ण_अगर
                hold >>= op;
                bits -= op;
                Tracevv((मानक_त्रुटि, "inflate:         distance %u\n", dist));
                op = (अचिन्हित)(out - beg);     /* max distance in output */
                अगर (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    अगर (op > whave) {
                        अगर (state->sane) {
                            strm->संदेश = (z_const अक्षर *)
                                "invalid distance too far back";
                            state->mode = BAD;
                            अवरोध;
                        }
#अगर_घोषित INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        अगर (len <= op - whave) {
                            करो {
                                *out++ = 0;
                            } जबतक (--len);
                            जारी;
                        }
                        len -= op - whave;
                        करो {
                            *out++ = 0;
                        } जबतक (--op > whave);
                        अगर (op == 0) {
                            from = out - dist;
                            करो {
                                *out++ = *from++;
                            } जबतक (--len);
                            जारी;
                        }
#पूर्ण_अगर
                    }
                    from = window;
                    अगर (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        अगर (op < len) {         /* some from window */
                            len -= op;
                            करो {
                                *out++ = *from++;
                            } जबतक (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    अन्यथा अगर (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        अगर (op < len) {         /* some from end of window */
                            len -= op;
                            करो {
                                *out++ = *from++;
                            } जबतक (--op);
                            from = window;
                            अगर (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                करो {
                                    *out++ = *from++;
                                } जबतक (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    }
                    अन्यथा {                      /* contiguous in window */
                        from += wnext - op;
                        अगर (op < len) {         /* some from window */
                            len -= op;
                            करो {
                                *out++ = *from++;
                            } जबतक (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    जबतक (len > 2) {
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    }
                    अगर (len) {
                        *out++ = *from++;
                        अगर (len > 1)
                            *out++ = *from++;
                    }
                }
                अन्यथा {
                    from = out - dist;          /* copy direct from output */
                    करो {                        /* minimum length is three */
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    } जबतक (len > 2);
                    अगर (len) {
                        *out++ = *from++;
                        अगर (len > 1)
                            *out++ = *from++;
                    }
                }
            }
            अन्यथा अगर ((op & 64) == 0) {          /* 2nd level distance code */
                here = dcode + here->val + (hold & ((1U << op) - 1));
                जाओ dodist;
            }
            अन्यथा {
                strm->संदेश = (z_const अक्षर *)"invalid distance code";
                state->mode = BAD;
                अवरोध;
            }
        }
        अन्यथा अगर ((op & 64) == 0) {              /* 2nd level length code */
            here = lcode + here->val + (hold & ((1U << op) - 1));
            जाओ dolen;
        }
        अन्यथा अगर (op & 32) {                     /* end-of-block */
            Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
            state->mode = TYPE;
            अवरोध;
        }
        अन्यथा {
            strm->संदेश = (z_const अक्षर *)"invalid literal/length code";
            state->mode = BAD;
            अवरोध;
        }
    } जबतक (in < last && out < end);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    /* update state and return */
    strm->अग्र_in = in;
    strm->अग्र_out = out;
    strm->avail_in = (अचिन्हित)(in < last ? 5 + (last - in) : 5 - (in - last));
    strm->avail_out = (अचिन्हित)(out < end ?
                                 257 + (end - out) : 257 - (out - end));
    state->hold = hold;
    state->bits = bits;
    वापस;
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

#पूर्ण_अगर /* !ASMINF */
