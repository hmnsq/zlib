/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inftrees.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inflate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inffast.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ASMINF
#  pragma ਸੁਨੇਹਾ_2("Assembler code may have bugs -- use at your own risk")
#ਨਹੀਂਤਾਂ

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
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ inflate_fast(z_streamp strm, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ) {
    ਬਣਤਰ inflate_state FAR *state;
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਵਿੱਚ;      /* local strm->next_in */
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਆਖਰੀ;    /* have enough input while in < last */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਬਾਹਰਲਾ;     /* local strm->next_out */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *beg;     /* inflate()'s initial strm->next_out */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਸਿਰਾ;     /* while out < end, enough space available */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INFLATE_STRICT
    ਬਿਨਾਨਿਸ਼ਾਨੀ dmax;              /* maximum distance from zlib header */
#ਜੇ_ਅੰਤ
    ਬਿਨਾਨਿਸ਼ਾਨੀ wsize;             /* window size or zero if not using window */
    ਬਿਨਾਨਿਸ਼ਾਨੀ whave;             /* valid bytes in the window */
    ਬਿਨਾਨਿਸ਼ਾਨੀ wnext;             /* window write index */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ;  /* allocated sliding window, if wsize != 0 */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ hold;         /* local strm->hold */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ;              /* local strm->bits */
    ਕੋਡ ਅਚਲ FAR *lcode;      /* local strm->lencode */
    ਕੋਡ ਅਚਲ FAR *dcode;      /* local strm->distcode */
    ਬਿਨਾਨਿਸ਼ਾਨੀ lmask;             /* mask for first level of length codes */
    ਬਿਨਾਨਿਸ਼ਾਨੀ dmask;             /* mask for first level of distance codes */
    ਕੋਡ ਅਚਲ *ਇੱਥੇ;           /* retrieved table entry */
    ਬਿਨਾਨਿਸ਼ਾਨੀ op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;               /* match length, unused bytes */
    ਬਿਨਾਨਿਸ਼ਾਨੀ dist;              /* match distance */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (ਬਣਤਰ inflate_state FAR *)strm->state;
    ਵਿੱਚ = strm->ਅਗਲਾ_ਵਿੱਚ;
    ਆਖਰੀ = ਵਿੱਚ + (strm->avail_ਵਿੱਚ - 5);
    ਬਾਹਰਲਾ = strm->ਅਗਲਾ_ਬਾਹਰਲਾ;
    beg = ਬਾਹਰਲਾ - (ਸ਼ੁਰੂ - strm->avail_ਬਾਹਰਲਾ);
    ਸਿਰਾ = ਬਾਹਰਲਾ + (strm->avail_ਬਾਹਰਲਾ - 257);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INFLATE_STRICT
    dmax = state->dmax;
#ਜੇ_ਅੰਤ
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    ਖਿੜਕੀ = state->ਖਿੜਕੀ;
    hold = state->hold;
    ਬਿੱਟ = state->ਬਿੱਟ;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    ਕਰੋ {
        ਜੇ (ਬਿੱਟ < 15) {
            hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਵਿੱਚ++) << ਬਿੱਟ;
            ਬਿੱਟ += 8;
            hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਵਿੱਚ++) << ਬਿੱਟ;
            ਬਿੱਟ += 8;
        }
        ਇੱਥੇ = lcode + (hold & lmask);
      dolen:
        op = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ->ਬਿੱਟ);
        hold >>= op;
        ਬਿੱਟ -= op;
        op = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ->op);
        ਜੇ (op == 0) {                          /* literal */
            Tracevv((stderr, ਇੱਥੇ->val >= 0x20 && ਇੱਥੇ->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", ਇੱਥੇ->val));
            *ਬਾਹਰਲਾ++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(ਇੱਥੇ->val);
        }
        ਨਹੀਂਤਾਂ ਜੇ (op & 16) {                     /* length base */
            len = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ->val);
            op &= 15;                           /* number of extra bits */
            ਜੇ (op) {
                ਜੇ (ਬਿੱਟ < op) {
                    hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਵਿੱਚ++) << ਬਿੱਟ;
                    ਬਿੱਟ += 8;
                }
                len += (ਬਿਨਾਨਿਸ਼ਾਨੀ)hold & ((1U << op) - 1);
                hold >>= op;
                ਬਿੱਟ -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            ਜੇ (ਬਿੱਟ < 15) {
                hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਵਿੱਚ++) << ਬਿੱਟ;
                ਬਿੱਟ += 8;
                hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਵਿੱਚ++) << ਬਿੱਟ;
                ਬਿੱਟ += 8;
            }
            ਇੱਥੇ = dcode + (hold & dmask);
          dodist:
            op = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ->ਬਿੱਟ);
            hold >>= op;
            ਬਿੱਟ -= op;
            op = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ->op);
            ਜੇ (op & 16) {                      /* distance base */
                dist = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਇੱਥੇ->val);
                op &= 15;                       /* number of extra bits */
                ਜੇ (ਬਿੱਟ < op) {
                    hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਵਿੱਚ++) << ਬਿੱਟ;
                    ਬਿੱਟ += 8;
                    ਜੇ (ਬਿੱਟ < op) {
                        hold += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*ਵਿੱਚ++) << ਬਿੱਟ;
                        ਬਿੱਟ += 8;
                    }
                }
                dist += (ਬਿਨਾਨਿਸ਼ਾਨੀ)hold & ((1U << op) - 1);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INFLATE_STRICT
                ਜੇ (dist > dmax) {
                    strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                        "invalid distance too far back";
                    state->ਬਹੁਲਕ = BAD;
                    ਰੋਕੋ;
                }
#ਜੇ_ਅੰਤ
                hold >>= op;
                ਬਿੱਟ -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਬਾਹਰਲਾ - beg);     /* max distance in output */
                ਜੇ (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    ਜੇ (op > whave) {
                        ਜੇ (state->sane) {
                            strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)
                                "invalid distance too far back";
                            state->ਬਹੁਲਕ = BAD;
                            ਰੋਕੋ;
                        }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INFLATE_ALLOW_INVALID_ਦੂਰੀ_TOOFAR_ARRR
                        ਜੇ (len <= op - whave) {
                            ਕਰੋ {
                                *ਬਾਹਰਲਾ++ = 0;
                            } ਜਦੋਂ (--len);
                            ਜਾਰੀ;
                        }
                        len -= op - whave;
                        ਕਰੋ {
                            *ਬਾਹਰਲਾ++ = 0;
                        } ਜਦੋਂ (--op > whave);
                        ਜੇ (op == 0) {
                            from = ਬਾਹਰਲਾ - dist;
                            ਕਰੋ {
                                *ਬਾਹਰਲਾ++ = *from++;
                            } ਜਦੋਂ (--len);
                            ਜਾਰੀ;
                        }
#ਜੇ_ਅੰਤ
                    }
                    from = ਖਿੜਕੀ;
                    ਜੇ (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        ਜੇ (op < len) {         /* some from window */
                            len -= op;
                            ਕਰੋ {
                                *ਬਾਹਰਲਾ++ = *from++;
                            } ਜਦੋਂ (--op);
                            from = ਬਾਹਰਲਾ - dist;  /* rest from output */
                        }
                    }
                    ਨਹੀਂਤਾਂ ਜੇ (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        ਜੇ (op < len) {         /* some from end of window */
                            len -= op;
                            ਕਰੋ {
                                *ਬਾਹਰਲਾ++ = *from++;
                            } ਜਦੋਂ (--op);
                            from = ਖਿੜਕੀ;
                            ਜੇ (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                ਕਰੋ {
                                    *ਬਾਹਰਲਾ++ = *from++;
                                } ਜਦੋਂ (--op);
                                from = ਬਾਹਰਲਾ - dist;      /* rest from output */
                            }
                        }
                    }
                    ਨਹੀਂਤਾਂ {                      /* contiguous in window */
                        from += wnext - op;
                        ਜੇ (op < len) {         /* some from window */
                            len -= op;
                            ਕਰੋ {
                                *ਬਾਹਰਲਾ++ = *from++;
                            } ਜਦੋਂ (--op);
                            from = ਬਾਹਰਲਾ - dist;  /* rest from output */
                        }
                    }
                    ਜਦੋਂ (len > 2) {
                        *ਬਾਹਰਲਾ++ = *from++;
                        *ਬਾਹਰਲਾ++ = *from++;
                        *ਬਾਹਰਲਾ++ = *from++;
                        len -= 3;
                    }
                    ਜੇ (len) {
                        *ਬਾਹਰਲਾ++ = *from++;
                        ਜੇ (len > 1)
                            *ਬਾਹਰਲਾ++ = *from++;
                    }
                }
                ਨਹੀਂਤਾਂ {
                    from = ਬਾਹਰਲਾ - dist;          /* copy direct from output */
                    ਕਰੋ {                        /* minimum length is three */
                        *ਬਾਹਰਲਾ++ = *from++;
                        *ਬਾਹਰਲਾ++ = *from++;
                        *ਬਾਹਰਲਾ++ = *from++;
                        len -= 3;
                    } ਜਦੋਂ (len > 2);
                    ਜੇ (len) {
                        *ਬਾਹਰਲਾ++ = *from++;
                        ਜੇ (len > 1)
                            *ਬਾਹਰਲਾ++ = *from++;
                    }
                }
            }
            ਨਹੀਂਤਾਂ ਜੇ ((op & 64) == 0) {          /* 2nd level distance code */
                ਇੱਥੇ = dcode + ਇੱਥੇ->val + (hold & ((1U << op) - 1));
                ਜਾਓ dodist;
            }
            ਨਹੀਂਤਾਂ {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid distance code";
                state->ਬਹੁਲਕ = BAD;
                ਰੋਕੋ;
            }
        }
        ਨਹੀਂਤਾਂ ਜੇ ((op & 64) == 0) {              /* 2nd level length code */
            ਇੱਥੇ = lcode + ਇੱਥੇ->val + (hold & ((1U << op) - 1));
            ਜਾਓ dolen;
        }
        ਨਹੀਂਤਾਂ ਜੇ (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->ਬਹੁਲਕ = TYPE;
            ਰੋਕੋ;
        }
        ਨਹੀਂਤਾਂ {
            strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid literal/length code";
            state->ਬਹੁਲਕ = BAD;
            ਰੋਕੋ;
        }
    } ਜਦੋਂ (ਵਿੱਚ < ਆਖਰੀ && ਬਾਹਰਲਾ < ਸਿਰਾ);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = ਬਿੱਟ >> 3;
    ਵਿੱਚ -= len;
    ਬਿੱਟ -= len << 3;
    hold &= (1U << ਬਿੱਟ) - 1;

    /* update state and return */
    strm->ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;
    strm->ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
    strm->avail_ਵਿੱਚ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਵਿੱਚ < ਆਖਰੀ ? 5 + (ਆਖਰੀ - ਵਿੱਚ) : 5 - (ਵਿੱਚ - ਆਖਰੀ));
    strm->avail_ਬਾਹਰਲਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਬਾਹਰਲਾ < ਸਿਰਾ ?
                                 257 + (ਸਿਰਾ - ਬਾਹਰਲਾ) : 257 - (ਬਾਹਰਲਾ - ਸਿਰਾ));
    state->hold = hold;
    state->ਬਿੱਟ = ਬਿੱਟ;
    ਵਾਪਸ;
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

#ਜੇ_ਅੰਤ /* !ASMINF */
