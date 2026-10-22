/* inffast.c -- fast decoding
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "zutil.h"
#ಅನ್ತರ್ಭಾವ "inftrees.h"
#ಅನ್ತರ್ಭಾವ "inflate.h"
#ಅನ್ತರ್ಭಾವ "inffast.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ASMINF
#  pragma ಸಂದೇಶ_2("Assembler code may have bugs -- use at your own risk")
#ಇಲ್ಲದಿದ್ದರೆ

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
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ inflate_fast(z_streamp strm, ಚಿಹ್ನೆರಹಿತ ಆರಂಭ) {
    ರಚನೆ inflate_state FAR *state;
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಒಳ;      /* local strm->next_in */
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕೊನೆಯ;    /* have enough input while in < last */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಹೊರಹೋಗು;     /* local strm->next_out */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *beg;     /* inflate()'s initial strm->next_out */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕೊನೆ;     /* while out < end, enough space available */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INFLATE_STRICT
    ಚಿಹ್ನೆರಹಿತ dmax;              /* maximum distance from zlib header */
#ಆದರೆ_ಅಂತ್ಯ
    ಚಿಹ್ನೆರಹಿತ wsize;             /* window size or zero if not using window */
    ಚಿಹ್ನೆರಹಿತ whave;             /* valid bytes in the window */
    ಚಿಹ್ನೆರಹಿತ wnext;             /* window write index */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ;  /* allocated sliding window, if wsize != 0 */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ hold;         /* local strm->hold */
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು;              /* local strm->bits */
    ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *lcode;      /* local strm->lencode */
    ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *dcode;      /* local strm->distcode */
    ಚಿಹ್ನೆರಹಿತ lmask;             /* mask for first level of length codes */
    ಚಿಹ್ನೆರಹಿತ dmask;             /* mask for first level of distance codes */
    ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ *ಇಲ್ಲಿ;           /* retrieved table entry */
    ಚಿಹ್ನೆರಹಿತ op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    ಚಿಹ್ನೆರಹಿತ len;               /* match length, unused bytes */
    ಚಿಹ್ನೆರಹಿತ dist;              /* match distance */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (ರಚನೆ inflate_state FAR *)strm->state;
    ಒಳ = strm->ಮುಂದಿನ_ಒಳ;
    ಕೊನೆಯ = ಒಳ + (strm->avail_ಒಳ - 5);
    ಹೊರಹೋಗು = strm->ಮುಂದಿನ_ಹೊರಹೋಗು;
    beg = ಹೊರಹೋಗು - (ಆರಂಭ - strm->avail_ಹೊರಹೋಗು);
    ಕೊನೆ = ಹೊರಹೋಗು + (strm->avail_ಹೊರಹೋಗು - 257);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INFLATE_STRICT
    dmax = state->dmax;
#ಆದರೆ_ಅಂತ್ಯ
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    ಕಿಟಕಿ = state->ಕಿಟಕಿ;
    hold = state->hold;
    ಬಿಟ್ಟುಗಳು = state->ಬಿಟ್ಟುಗಳು;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    ಮಾಡು {
        ಆದರೆ (ಬಿಟ್ಟುಗಳು < 15) {
            hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಒಳ++) << ಬಿಟ್ಟುಗಳು;
            ಬಿಟ್ಟುಗಳು += 8;
            hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಒಳ++) << ಬಿಟ್ಟುಗಳು;
            ಬಿಟ್ಟುಗಳು += 8;
        }
        ಇಲ್ಲಿ = lcode + (hold & lmask);
      dolen:
        op = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ->ಬಿಟ್ಟುಗಳು);
        hold >>= op;
        ಬಿಟ್ಟುಗಳು -= op;
        op = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ->op);
        ಆದರೆ (op == 0) {                          /* literal */
            Tracevv((stderr, ಇಲ್ಲಿ->val >= 0x20 && ಇಲ್ಲಿ->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", ಇಲ್ಲಿ->val));
            *ಹೊರಹೋಗು++ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(ಇಲ್ಲಿ->val);
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (op & 16) {                     /* length base */
            len = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ->val);
            op &= 15;                           /* number of extra bits */
            ಆದರೆ (op) {
                ಆದರೆ (ಬಿಟ್ಟುಗಳು < op) {
                    hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಒಳ++) << ಬಿಟ್ಟುಗಳು;
                    ಬಿಟ್ಟುಗಳು += 8;
                }
                len += (ಚಿಹ್ನೆರಹಿತ)hold & ((1U << op) - 1);
                hold >>= op;
                ಬಿಟ್ಟುಗಳು -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            ಆದರೆ (ಬಿಟ್ಟುಗಳು < 15) {
                hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಒಳ++) << ಬಿಟ್ಟುಗಳು;
                ಬಿಟ್ಟುಗಳು += 8;
                hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಒಳ++) << ಬಿಟ್ಟುಗಳು;
                ಬಿಟ್ಟುಗಳು += 8;
            }
            ಇಲ್ಲಿ = dcode + (hold & dmask);
          dodist:
            op = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ->ಬಿಟ್ಟುಗಳು);
            hold >>= op;
            ಬಿಟ್ಟುಗಳು -= op;
            op = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ->op);
            ಆದರೆ (op & 16) {                      /* distance base */
                dist = (ಚಿಹ್ನೆರಹಿತ)(ಇಲ್ಲಿ->val);
                op &= 15;                       /* number of extra bits */
                ಆದರೆ (ಬಿಟ್ಟುಗಳು < op) {
                    hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಒಳ++) << ಬಿಟ್ಟುಗಳು;
                    ಬಿಟ್ಟುಗಳು += 8;
                    ಆದರೆ (ಬಿಟ್ಟುಗಳು < op) {
                        hold += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*ಒಳ++) << ಬಿಟ್ಟುಗಳು;
                        ಬಿಟ್ಟುಗಳು += 8;
                    }
                }
                dist += (ಚಿಹ್ನೆರಹಿತ)hold & ((1U << op) - 1);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INFLATE_STRICT
                ಆದರೆ (dist > dmax) {
                    strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)
                        "invalid distance too far back";
                    state->ಬಹುಲಕ = BAD;
                    ನಿಲ್ಲಿಸು;
                }
#ಆದರೆ_ಅಂತ್ಯ
                hold >>= op;
                ಬಿಟ್ಟುಗಳು -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (ಚಿಹ್ನೆರಹಿತ)(ಹೊರಹೋಗು - beg);     /* max distance in output */
                ಆದರೆ (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    ಆದರೆ (op > whave) {
                        ಆದರೆ (state->sane) {
                            strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)
                                "invalid distance too far back";
                            state->ಬಹುಲಕ = BAD;
                            ನಿಲ್ಲಿಸು;
                        }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INFLATE_ALLOW_INVALID_ದೂರ_TOOFAR_ARRR
                        ಆದರೆ (len <= op - whave) {
                            ಮಾಡು {
                                *ಹೊರಹೋಗು++ = 0;
                            } ಆಗ (--len);
                            ಮುಂದುವರಿ;
                        }
                        len -= op - whave;
                        ಮಾಡು {
                            *ಹೊರಹೋಗು++ = 0;
                        } ಆಗ (--op > whave);
                        ಆದರೆ (op == 0) {
                            from = ಹೊರಹೋಗು - dist;
                            ಮಾಡು {
                                *ಹೊರಹೋಗು++ = *from++;
                            } ಆಗ (--len);
                            ಮುಂದುವರಿ;
                        }
#ಆದರೆ_ಅಂತ್ಯ
                    }
                    from = ಕಿಟಕಿ;
                    ಆದರೆ (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        ಆದರೆ (op < len) {         /* some from window */
                            len -= op;
                            ಮಾಡು {
                                *ಹೊರಹೋಗು++ = *from++;
                            } ಆಗ (--op);
                            from = ಹೊರಹೋಗು - dist;  /* rest from output */
                        }
                    }
                    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        ಆದರೆ (op < len) {         /* some from end of window */
                            len -= op;
                            ಮಾಡು {
                                *ಹೊರಹೋಗು++ = *from++;
                            } ಆಗ (--op);
                            from = ಕಿಟಕಿ;
                            ಆದರೆ (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                ಮಾಡು {
                                    *ಹೊರಹೋಗು++ = *from++;
                                } ಆಗ (--op);
                                from = ಹೊರಹೋಗು - dist;      /* rest from output */
                            }
                        }
                    }
                    ಇಲ್ಲದಿದ್ದರೆ {                      /* contiguous in window */
                        from += wnext - op;
                        ಆದರೆ (op < len) {         /* some from window */
                            len -= op;
                            ಮಾಡು {
                                *ಹೊರಹೋಗು++ = *from++;
                            } ಆಗ (--op);
                            from = ಹೊರಹೋಗು - dist;  /* rest from output */
                        }
                    }
                    ಆಗ (len > 2) {
                        *ಹೊರಹೋಗು++ = *from++;
                        *ಹೊರಹೋಗು++ = *from++;
                        *ಹೊರಹೋಗು++ = *from++;
                        len -= 3;
                    }
                    ಆದರೆ (len) {
                        *ಹೊರಹೋಗು++ = *from++;
                        ಆದರೆ (len > 1)
                            *ಹೊರಹೋಗು++ = *from++;
                    }
                }
                ಇಲ್ಲದಿದ್ದರೆ {
                    from = ಹೊರಹೋಗು - dist;          /* copy direct from output */
                    ಮಾಡು {                        /* minimum length is three */
                        *ಹೊರಹೋಗು++ = *from++;
                        *ಹೊರಹೋಗು++ = *from++;
                        *ಹೊರಹೋಗು++ = *from++;
                        len -= 3;
                    } ಆಗ (len > 2);
                    ಆದರೆ (len) {
                        *ಹೊರಹೋಗು++ = *from++;
                        ಆದರೆ (len > 1)
                            *ಹೊರಹೋಗು++ = *from++;
                    }
                }
            }
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((op & 64) == 0) {          /* 2nd level distance code */
                ಇಲ್ಲಿ = dcode + ಇಲ್ಲಿ->val + (hold & ((1U << op) - 1));
                ಹೋಗು dodist;
            }
            ಇಲ್ಲದಿದ್ದರೆ {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid distance code";
                state->ಬಹುಲಕ = BAD;
                ನಿಲ್ಲಿಸು;
            }
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((op & 64) == 0) {              /* 2nd level length code */
            ಇಲ್ಲಿ = lcode + ಇಲ್ಲಿ->val + (hold & ((1U << op) - 1));
            ಹೋಗು dolen;
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->ಬಹುಲಕ = TYPE;
            ನಿಲ್ಲಿಸು;
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid literal/length code";
            state->ಬಹುಲಕ = BAD;
            ನಿಲ್ಲಿಸು;
        }
    } ಆಗ (ಒಳ < ಕೊನೆಯ && ಹೊರಹೋಗು < ಕೊನೆ);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = ಬಿಟ್ಟುಗಳು >> 3;
    ಒಳ -= len;
    ಬಿಟ್ಟುಗಳು -= len << 3;
    hold &= (1U << ಬಿಟ್ಟುಗಳು) - 1;

    /* update state and return */
    strm->ಮುಂದಿನ_ಒಳ = ಒಳ;
    strm->ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
    strm->avail_ಒಳ = (ಚಿಹ್ನೆರಹಿತ)(ಒಳ < ಕೊನೆಯ ? 5 + (ಕೊನೆಯ - ಒಳ) : 5 - (ಒಳ - ಕೊನೆಯ));
    strm->avail_ಹೊರಹೋಗು = (ಚಿಹ್ನೆರಹಿತ)(ಹೊರಹೋಗು < ಕೊನೆ ?
                                 257 + (ಕೊನೆ - ಹೊರಹೋಗು) : 257 - (ಹೊರಹೋಗು - ಕೊನೆ));
    state->hold = hold;
    state->ಬಿಟ್ಟುಗಳು = ಬಿಟ್ಟುಗಳು;
    ಹಿಂತಿರುಗಿ;
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

#ಆದರೆ_ಅಂತ್ಯ /* !ASMINF */
