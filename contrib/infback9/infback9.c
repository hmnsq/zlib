/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#அந்தர்பாவ "zutil.h"
#அந்தர்பாவ "infback9.h"
#அந்தர்பாவ "inftree9.h"
#அந்தர்பாவ "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
முழு ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned எழுத்து FAR *window,
                              மாறிலி எழுத்து *version, முழு stream_size) {
    கட்டமைப்பு inflate_state FAR *state;

    எனில் (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (முழு)(அளவு(z_stream)))
        திரும்பு Z_VERSION_ERROR;
    எனில் (strm == Z_NULL || window == Z_NULL)
        திரும்பு Z_STREAM_ERROR;
    strm->செய்தி = Z_NULL;                 /* in case we return an error */
    எனில் (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    எனில் (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (கட்டமைப்பு inflate_state FAR *)ZALLOC(strm, 1,
                                               அளவு(கட்டமைப்பு inflate_state));
    எனில் (state == Z_NULL) திரும்பு Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    திரும்பு Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#அந்தர்பாவ <stdio.h>

வெறுமை makefixed9(வெறுமை) {
    unsigned sym, bits, கீழ், size;
    code *next, *lenfix, *distfix;
    கட்டமைப்பு inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    போது (sym < 144) state.lens[sym++] = 8;
    போது (sym < 256) state.lens[sym++] = 9;
    போது (sym < 280) state.lens[sym++] = 7;
    போது (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    போது (sym < 32) state.lens[sym++] = 5;
    distfix = next;
    bits = 5;
    inflate_table9(DISTS, state.lens, 32, &(next), &(bits), state.work);

    /* write tables */
    puts("    /* inffix9.h -- table for decoding deflate64 fixed codes");
    puts("     * Generated automatically by makefixed9().");
    puts("     */");
    puts("");
    puts("    /* WARNING: this file should *not* be used by applications.");
    puts("       It is part of the implementation of this library and is");
    puts("       subject to change. Applications should only use zlib.h.");
    puts("     */");
    puts("");
    size = 1U << 9;
    printf("    static const code lenfix[%u] = {", size);
    கீழ் = 0;
    க்கு (;;) {
        எனில் ((கீழ் % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[கீழ்].op, lenfix[கீழ்].bits,
               lenfix[கீழ்].val);
        எனில் (++கீழ் == size) நிறுத்து;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    கீழ் = 0;
    க்கு (;;) {
        எனில் ((கீழ் % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[கீழ்].op, distfix[கீழ்].bits,
               distfix[கீழ்].val);
        எனில் (++கீழ் == size) நிறுத்து;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    செய் { \
        hold = 0; \
        bits = 0; \
    } போது (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    செய் { \
        எனில் (have == 0) { \
            have = in(in_desc, &next); \
            எனில் (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                செல் inf_leave; \
            } \
        } \
    } போது (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    செய் { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } போது (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    செய் { \
        போது (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } போது (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    செய் { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } போது (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    செய் { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } போது (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    செய் { \
        எனில் (இடது == 0) { \
            put = window; \
            இடது = WSIZE; \
            wrap = 1; \
            எனில் (out(out_desc, put, (unsigned)இடது)) { \
                ret = Z_BUF_ERROR; \
                செல் inf_leave; \
            } \
        } \
    } போது (0)

/*
   strm provides the memory allocation functions and window buffer on input,
   and provides information on the unused input on return.  For Z_DATA_ERROR
   returns, strm will also provide an error message.

   in() and out() are the call-back input and output functions.  When
   inflateBack() needs more input, it calls in().  When inflateBack() has
   filled the window with output, or when it completes with data in the
   window, it calls out() to write out the data.  The application must not
   change the provided input until in() is called again or inflateBack()
   returns.  The application must not change the window/output buffer until
   inflateBack() returns.

   in() and out() are called with a descriptor parameter provided in the
   inflateBack() call.  This parameter can be a structure that provides the
   information required to do the read or write, as well as accumulated
   information on the input and output such as totals and check values.

   in() should return zero on failure.  out() should return non-zero on
   failure.  If either in() or out() fails, than inflateBack() returns a
   Z_BUF_ERROR.  strm->next_in can be checked for Z_NULL to see whether it
   was in() or out() that caused in the error.  Otherwise,  inflateBack()
   returns Z_STREAM_END on success, Z_DATA_ERROR for an deflate format
   error, or Z_MEM_ERROR if it could not allocate memory for the state.
   inflateBack() can also return Z_STREAM_ERROR if the input parameters
   are not correct, i.e. strm is Z_NULL or the state was not initialized.
 */
முழு ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, வெறுமை FAR *in_desc,
                         out_func out, வெறுமை FAR *out_desc) {
    கட்டமைப்பு inflate_state FAR *state;
    z_const unsigned எழுத்து FAR *next;    /* next input */
    unsigned எழுத்து FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long இடது;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    முழு lastblock;              /* true if processing last block */
    முழு wrap;                   /* true if the window has wrapped */
    unsigned எழுத்து FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned எழுத்து FAR *from;    /* where to copy match bytes from */
    code மாறிலி FAR *lencode;    /* starting table for length/literal codes */
    code மாறிலி FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    முழு ret;                    /* return code */
    நிலையான மாறிலி unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#அந்தர்பாவ "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    எனில் (strm == Z_NULL || strm->state == Z_NULL)
        திரும்பு Z_STREAM_ERROR;
    state = (கட்டமைப்பு inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->செய்தி = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    இடது = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    க்கு (;;)
        தேர்வு (mode) {
        நிலை TYPE:
            /* determine and dispatch block type */
            எனில் (lastblock) {
                BYTEBITS();
                mode = DONE;
                நிறுத்து;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            தேர்வு (BITS(2)) {
            நிலை 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                நிறுத்து;
            நிலை 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                நிறுத்து;
            நிலை 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                நிறுத்து;
            நிலை 3:
                strm->செய்தி = (z_const எழுத்து *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            நிறுத்து;

        நிலை STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            எனில் ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->செய்தி = (z_const எழுத்து *)"invalid stored block lengths";
                mode = BAD;
                நிறுத்து;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            போது (length != 0) {
                copy = length;
                PULL();
                ROOM();
                எனில் (copy > have) copy = have;
                எனில் (copy > இடது) copy = இடது;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                இடது -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            நிறுத்து;

        நிலை TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            எனில் (state->nlen > 286) {
                strm->செய்தி = (z_const எழுத்து *)"too many length symbols";
                mode = BAD;
                நிறுத்து;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            போது (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            போது (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code மாறிலி FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            எனில் (ret) {
                strm->செய்தி = (z_const எழுத்து *)"invalid code lengths set";
                mode = BAD;
                நிறுத்து;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            போது (state->have < state->nlen + state->ndist) {
                க்கு (;;) {
                    here = lencode[BITS(lenbits)];
                    எனில் ((unsigned)(here.bits) <= bits) நிறுத்து;
                    PULLBYTE();
                }
                எனில் (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                இல்லையெனில் {
                    எனில் (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        எனில் (state->have == 0) {
                            strm->செய்தி = (z_const எழுத்து *)"invalid bit length repeat";
                            mode = BAD;
                            நிறுத்து;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    இல்லையெனில் எனில் (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    இல்லையெனில் {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    எனில் (state->have + copy > state->nlen + state->ndist) {
                        strm->செய்தி = (z_const எழுத்து *)"invalid bit length repeat";
                        mode = BAD;
                        நிறுத்து;
                    }
                    போது (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            எனில் (mode == BAD) நிறுத்து;

            /* check for end-of-block code (better have one) */
            எனில் (state->lens[256] == 0) {
                strm->செய்தி = (z_const எழுத்து *)"invalid code -- missing end-of-block";
                mode = BAD;
                நிறுத்து;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code மாறிலி FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            எனில் (ret) {
                strm->செய்தி = (z_const எழுத்து *)"invalid literal/lengths set";
                mode = BAD;
                நிறுத்து;
            }
            distcode = (code மாறிலி FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            எனில் (ret) {
                strm->செய்தி = (z_const எழுத்து *)"invalid distances set";
                mode = BAD;
                நிறுத்து;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        நிலை LEN:
            /* get a literal, length, or end-of-block code */
            க்கு (;;) {
                here = lencode[BITS(lenbits)];
                எனில் ((unsigned)(here.bits) <= bits) நிறுத்து;
                PULLBYTE();
            }
            எனில் (here.op && (here.op & 0xf0) == 0) {
                last = here;
                க்கு (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    எனில் ((unsigned)(last.bits + here.bits) <= bits) நிறுத்து;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            எனில் (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned எழுத்து)(length);
                இடது--;
                mode = LEN;
                நிறுத்து;
            }

            /* process end of block */
            எனில் (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                நிறுத்து;
            }

            /* invalid code */
            எனில் (here.op & 64) {
                strm->செய்தி = (z_const எழுத்து *)"invalid literal/length code";
                mode = BAD;
                நிறுத்து;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            எனில் (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            க்கு (;;) {
                here = distcode[BITS(distbits)];
                எனில் ((unsigned)(here.bits) <= bits) நிறுத்து;
                PULLBYTE();
            }
            எனில் ((here.op & 0xf0) == 0) {
                last = here;
                க்கு (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    எனில் ((unsigned)(last.bits + here.bits) <= bits) நிறுத்து;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            எனில் (here.op & 64) {
                strm->செய்தி = (z_const எழுத்து *)"invalid distance code";
                mode = BAD;
                நிறுத்து;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            எனில் (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            எனில் (offset > WSIZE - (wrap ? 0: இடது)) {
                strm->செய்தி = (z_const எழுத்து *)"invalid distance too far back";
                mode = BAD;
                நிறுத்து;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            செய் {
                ROOM();
                copy = WSIZE - offset;
                எனில் (copy < இடது) {
                    from = put + copy;
                    copy = இடது - copy;
                }
                இல்லையெனில் {
                    from = put - offset;
                    copy = இடது;
                }
                எனில் (copy > length) copy = length;
                length -= copy;
                இடது -= copy;
                செய் {
                    *put++ = *from++;
                } போது (--copy);
            } போது (length != 0);
            நிறுத்து;

        நிலை DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            எனில் (இடது < WSIZE) {
                எனில் (out(out_desc, window, (unsigned)(WSIZE - இடது)))
                    ret = Z_BUF_ERROR;
            }
            செல் inf_leave;

        நிலை BAD:
            ret = Z_DATA_ERROR;
            செல் inf_leave;

        இயல்பு:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            செல் inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    திரும்பு ret;
}

முழு ZEXPORT inflateBack9End(z_stream FAR *strm) {
    எனில் (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        திரும்பு Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    திரும்பு Z_OK;
}
