/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "zutil.h"
#अन्तर्भाव "inftrees.h"
#अन्तर्भाव "inflate.h"
#अन्तर्भाव "inffast.h"

#यदि_परिभाषितम् ASMINF
#  pragma सन्देशः_2("Assembler code may have bugs -- use at your own risk")
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
शून्य ZLIB_आन्तरिकम् inflate_fast(z_streamp strm, अचिह्नित start) {
    संरचना inflate_state FAR *state;
    z_const अचिह्नित वर्ण FAR *अन्तर्;      /* local strm->next_in */
    z_const अचिह्नित वर्ण FAR *चरमः;    /* have enough input while in < last */
    अचिह्नित वर्ण FAR *बहिर्गमनम्;     /* local strm->next_out */
    अचिह्नित वर्ण FAR *beg;     /* inflate()'s initial strm->next_out */
    अचिह्नित वर्ण FAR *अन्तः_2;     /* while out < end, enough space available */
#यदि_परिभाषितम् INFLATE_STRICT
    अचिह्नित dmax;              /* maximum distance from zlib header */
#यदि_अन्तः
    अचिह्नित wsize;             /* window size or zero if not using window */
    अचिह्नित whave;             /* valid bytes in the window */
    अचिह्नित wnext;             /* window write index */
    अचिह्नित वर्ण FAR *गवाक्षः;  /* allocated sliding window, if wsize != 0 */
    अचिह्नित दीर्घ hold;         /* local strm->hold */
    अचिह्नित द्वयङ्काः;              /* local strm->bits */
    संहिता ध्रुव FAR *lcode;      /* local strm->lencode */
    संहिता ध्रुव FAR *dcode;      /* local strm->distcode */
    अचिह्नित lmask;             /* mask for first level of length codes */
    अचिह्नित dmask;             /* mask for first level of distance codes */
    संहिता ध्रुव *अत्र;           /* retrieved table entry */
    अचिह्नित op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    अचिह्नित len;               /* match length, unused bytes */
    अचिह्नित dist;              /* match distance */
    अचिह्नित वर्ण FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (संरचना inflate_state FAR *)strm->state;
    अन्तर् = strm->अग्र_अन्तर्;
    चरमः = अन्तर् + (strm->avail_अन्तर् - 5);
    बहिर्गमनम् = strm->अग्र_बहिर्गमनम्;
    beg = बहिर्गमनम् - (start - strm->avail_बहिर्गमनम्);
    अन्तः_2 = बहिर्गमनम् + (strm->avail_बहिर्गमनम् - 257);
#यदि_परिभाषितम् INFLATE_STRICT
    dmax = state->dmax;
#यदि_अन्तः
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    गवाक्षः = state->गवाक्षः;
    hold = state->hold;
    द्वयङ्काः = state->द्वयङ्काः;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    कुरु {
        यदि (द्वयङ्काः < 15) {
            hold += (अचिह्नित दीर्घ)(*अन्तर्++) << द्वयङ्काः;
            द्वयङ्काः += 8;
            hold += (अचिह्नित दीर्घ)(*अन्तर्++) << द्वयङ्काः;
            द्वयङ्काः += 8;
        }
        अत्र = lcode + (hold & lmask);
      dolen:
        op = (अचिह्नित)(अत्र->द्वयङ्काः);
        hold >>= op;
        द्वयङ्काः -= op;
        op = (अचिह्नित)(अत्र->op);
        यदि (op == 0) {                          /* literal */
            Tracevv((stderr, अत्र->val >= 0x20 && अत्र->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", अत्र->val));
            *बहिर्गमनम्++ = (अचिह्नित वर्ण)(अत्र->val);
        }
        अन्यथा यदि (op & 16) {                     /* length base */
            len = (अचिह्नित)(अत्र->val);
            op &= 15;                           /* number of extra bits */
            यदि (op) {
                यदि (द्वयङ्काः < op) {
                    hold += (अचिह्नित दीर्घ)(*अन्तर्++) << द्वयङ्काः;
                    द्वयङ्काः += 8;
                }
                len += (अचिह्नित)hold & ((1U << op) - 1);
                hold >>= op;
                द्वयङ्काः -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            यदि (द्वयङ्काः < 15) {
                hold += (अचिह्नित दीर्घ)(*अन्तर्++) << द्वयङ्काः;
                द्वयङ्काः += 8;
                hold += (अचिह्नित दीर्घ)(*अन्तर्++) << द्वयङ्काः;
                द्वयङ्काः += 8;
            }
            अत्र = dcode + (hold & dmask);
          dodist:
            op = (अचिह्नित)(अत्र->द्वयङ्काः);
            hold >>= op;
            द्वयङ्काः -= op;
            op = (अचिह्नित)(अत्र->op);
            यदि (op & 16) {                      /* distance base */
                dist = (अचिह्नित)(अत्र->val);
                op &= 15;                       /* number of extra bits */
                यदि (द्वयङ्काः < op) {
                    hold += (अचिह्नित दीर्घ)(*अन्तर्++) << द्वयङ्काः;
                    द्वयङ्काः += 8;
                    यदि (द्वयङ्काः < op) {
                        hold += (अचिह्नित दीर्घ)(*अन्तर्++) << द्वयङ्काः;
                        द्वयङ्काः += 8;
                    }
                }
                dist += (अचिह्नित)hold & ((1U << op) - 1);
#यदि_परिभाषितम् INFLATE_STRICT
                यदि (dist > dmax) {
                    strm->सन्देशः = (z_const वर्ण *)
                        "invalid distance too far back";
                    state->बहुलकम् = BAD;
                    भङ्ग;
                }
#यदि_अन्तः
                hold >>= op;
                द्वयङ्काः -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (अचिह्नित)(बहिर्गमनम् - beg);     /* max distance in output */
                यदि (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    यदि (op > whave) {
                        यदि (state->sane) {
                            strm->सन्देशः = (z_const वर्ण *)
                                "invalid distance too far back";
                            state->बहुलकम् = BAD;
                            भङ्ग;
                        }
#यदि_परिभाषितम् INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        यदि (len <= op - whave) {
                            कुरु {
                                *बहिर्गमनम्++ = 0;
                            } यावत् (--len);
                            अनुवर्त;
                        }
                        len -= op - whave;
                        कुरु {
                            *बहिर्गमनम्++ = 0;
                        } यावत् (--op > whave);
                        यदि (op == 0) {
                            from = बहिर्गमनम् - dist;
                            कुरु {
                                *बहिर्गमनम्++ = *from++;
                            } यावत् (--len);
                            अनुवर्त;
                        }
#यदि_अन्तः
                    }
                    from = गवाक्षः;
                    यदि (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        यदि (op < len) {         /* some from window */
                            len -= op;
                            कुरु {
                                *बहिर्गमनम्++ = *from++;
                            } यावत् (--op);
                            from = बहिर्गमनम् - dist;  /* rest from output */
                        }
                    }
                    अन्यथा यदि (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        यदि (op < len) {         /* some from end of window */
                            len -= op;
                            कुरु {
                                *बहिर्गमनम्++ = *from++;
                            } यावत् (--op);
                            from = गवाक्षः;
                            यदि (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                कुरु {
                                    *बहिर्गमनम्++ = *from++;
                                } यावत् (--op);
                                from = बहिर्गमनम् - dist;      /* rest from output */
                            }
                        }
                    }
                    अन्यथा {                      /* contiguous in window */
                        from += wnext - op;
                        यदि (op < len) {         /* some from window */
                            len -= op;
                            कुरु {
                                *बहिर्गमनम्++ = *from++;
                            } यावत् (--op);
                            from = बहिर्गमनम् - dist;  /* rest from output */
                        }
                    }
                    यावत् (len > 2) {
                        *बहिर्गमनम्++ = *from++;
                        *बहिर्गमनम्++ = *from++;
                        *बहिर्गमनम्++ = *from++;
                        len -= 3;
                    }
                    यदि (len) {
                        *बहिर्गमनम्++ = *from++;
                        यदि (len > 1)
                            *बहिर्गमनम्++ = *from++;
                    }
                }
                अन्यथा {
                    from = बहिर्गमनम् - dist;          /* copy direct from output */
                    कुरु {                        /* minimum length is three */
                        *बहिर्गमनम्++ = *from++;
                        *बहिर्गमनम्++ = *from++;
                        *बहिर्गमनम्++ = *from++;
                        len -= 3;
                    } यावत् (len > 2);
                    यदि (len) {
                        *बहिर्गमनम्++ = *from++;
                        यदि (len > 1)
                            *बहिर्गमनम्++ = *from++;
                    }
                }
            }
            अन्यथा यदि ((op & 64) == 0) {          /* 2nd level distance code */
                अत्र = dcode + अत्र->val + (hold & ((1U << op) - 1));
                गच्छ dodist;
            }
            अन्यथा {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance code";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
        }
        अन्यथा यदि ((op & 64) == 0) {              /* 2nd level length code */
            अत्र = lcode + अत्र->val + (hold & ((1U << op) - 1));
            गच्छ dolen;
        }
        अन्यथा यदि (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->बहुलकम् = TYPE;
            भङ्ग;
        }
        अन्यथा {
            strm->सन्देशः = (z_const वर्ण *)"invalid literal/length code";
            state->बहुलकम् = BAD;
            भङ्ग;
        }
    } यावत् (अन्तर् < चरमः && बहिर्गमनम् < अन्तः_2);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = द्वयङ्काः >> 3;
    अन्तर् -= len;
    द्वयङ्काः -= len << 3;
    hold &= (1U << द्वयङ्काः) - 1;

    /* update state and return */
    strm->अग्र_अन्तर् = अन्तर्;
    strm->अग्र_बहिर्गमनम् = बहिर्गमनम्;
    strm->avail_अन्तर् = (अचिह्नित)(अन्तर् < चरमः ? 5 + (चरमः - अन्तर्) : 5 - (अन्तर् - चरमः));
    strm->avail_बहिर्गमनम् = (अचिह्नित)(बहिर्गमनम् < अन्तः_2 ?
                                 257 + (अन्तः_2 - बहिर्गमनम्) : 257 - (बहिर्गमनम् - अन्तः_2));
    state->hold = hold;
    state->द्वयङ्काः = द्वयङ्काः;
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

#यदि_अन्तः /* !ASMINF */
