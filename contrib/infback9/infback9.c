/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#અન્તર્ભાવ "zutil.h"
#અન્તર્ભાવ "infback9.h"
#અન્તર્ભાવ "inftree9.h"
#અન્તર્ભાવ "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
પૂર્ણાંક ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned અક્ષર FAR *window,
                              અચલ અક્ષર *version, પૂર્ણાંક stream_size) {
    સંરચના inflate_state FAR *state;

    જો (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (પૂર્ણાંક)(કદ(z_stream)))
        પાછા Z_VERSION_ERROR;
    જો (strm == Z_NULL || window == Z_NULL)
        પાછા Z_STREAM_ERROR;
    strm->સંદેશ = Z_NULL;                 /* in case we return an error */
    જો (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    જો (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (સંરચના inflate_state FAR *)ZALLOC(strm, 1,
                                               કદ(સંરચના inflate_state));
    જો (state == Z_NULL) પાછા Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    પાછા Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#અન્તર્ભાવ <stdio.h>

શૂન્ય makefixed9(શૂન્ય) {
    unsigned sym, bits, નીચલું, size;
    code *next, *lenfix, *distfix;
    સંરચના inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    જ્યારે (sym < 144) state.lens[sym++] = 8;
    જ્યારે (sym < 256) state.lens[sym++] = 9;
    જ્યારે (sym < 280) state.lens[sym++] = 7;
    જ્યારે (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    જ્યારે (sym < 32) state.lens[sym++] = 5;
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
    નીચલું = 0;
    માટે (;;) {
        જો ((નીચલું % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[નીચલું].op, lenfix[નીચલું].bits,
               lenfix[નીચલું].val);
        જો (++નીચલું == size) તોડો;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    નીચલું = 0;
    માટે (;;) {
        જો ((નીચલું % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[નીચલું].op, distfix[નીચલું].bits,
               distfix[નીચલું].val);
        જો (++નીચલું == size) તોડો;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    કરો { \
        hold = 0; \
        bits = 0; \
    } જ્યારે (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    કરો { \
        જો (have == 0) { \
            have = in(in_desc, &next); \
            જો (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                જાઓ inf_leave; \
            } \
        } \
    } જ્યારે (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    કરો { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } જ્યારે (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    કરો { \
        જ્યારે (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } જ્યારે (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    કરો { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } જ્યારે (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    કરો { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } જ્યારે (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    કરો { \
        જો (ડાબું == 0) { \
            put = window; \
            ડાબું = WSIZE; \
            wrap = 1; \
            જો (out(out_desc, put, (unsigned)ડાબું)) { \
                ret = Z_BUF_ERROR; \
                જાઓ inf_leave; \
            } \
        } \
    } જ્યારે (0)

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
પૂર્ણાંક ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, શૂન્ય FAR *in_desc,
                         out_func out, શૂન્ય FAR *out_desc) {
    સંરચના inflate_state FAR *state;
    z_const unsigned અક્ષર FAR *next;    /* next input */
    unsigned અક્ષર FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long ડાબું;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    પૂર્ણાંક lastblock;              /* true if processing last block */
    પૂર્ણાંક wrap;                   /* true if the window has wrapped */
    unsigned અક્ષર FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned અક્ષર FAR *from;    /* where to copy match bytes from */
    code અચલ FAR *lencode;    /* starting table for length/literal codes */
    code અચલ FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    પૂર્ણાંક ret;                    /* return code */
    સ્થિર અચલ unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#અન્તર્ભાવ "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    જો (strm == Z_NULL || strm->state == Z_NULL)
        પાછા Z_STREAM_ERROR;
    state = (સંરચના inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->સંદેશ = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    ડાબું = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    માટે (;;)
        પસંદગી (mode) {
        સ્થિતિ TYPE:
            /* determine and dispatch block type */
            જો (lastblock) {
                BYTEBITS();
                mode = DONE;
                તોડો;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            પસંદગી (BITS(2)) {
            સ્થિતિ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                તોડો;
            સ્થિતિ 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                તોડો;
            સ્થિતિ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                તોડો;
            સ્થિતિ 3:
                strm->સંદેશ = (z_const અક્ષર *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            તોડો;

        સ્થિતિ STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            જો ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid stored block lengths";
                mode = BAD;
                તોડો;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            જ્યારે (length != 0) {
                copy = length;
                PULL();
                ROOM();
                જો (copy > have) copy = have;
                જો (copy > ડાબું) copy = ડાબું;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                ડાબું -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            તોડો;

        સ્થિતિ TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            જો (state->nlen > 286) {
                strm->સંદેશ = (z_const અક્ષર *)"too many length symbols";
                mode = BAD;
                તોડો;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            જ્યારે (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            જ્યારે (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code અચલ FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            જો (ret) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid code lengths set";
                mode = BAD;
                તોડો;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            જ્યારે (state->have < state->nlen + state->ndist) {
                માટે (;;) {
                    here = lencode[BITS(lenbits)];
                    જો ((unsigned)(here.bits) <= bits) તોડો;
                    PULLBYTE();
                }
                જો (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                નહીંતર {
                    જો (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        જો (state->have == 0) {
                            strm->સંદેશ = (z_const અક્ષર *)"invalid bit length repeat";
                            mode = BAD;
                            તોડો;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    નહીંતર જો (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    નહીંતર {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    જો (state->have + copy > state->nlen + state->ndist) {
                        strm->સંદેશ = (z_const અક્ષર *)"invalid bit length repeat";
                        mode = BAD;
                        તોડો;
                    }
                    જ્યારે (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            જો (mode == BAD) તોડો;

            /* check for end-of-block code (better have one) */
            જો (state->lens[256] == 0) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid code -- missing end-of-block";
                mode = BAD;
                તોડો;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code અચલ FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            જો (ret) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid literal/lengths set";
                mode = BAD;
                તોડો;
            }
            distcode = (code અચલ FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            જો (ret) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid distances set";
                mode = BAD;
                તોડો;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        સ્થિતિ LEN:
            /* get a literal, length, or end-of-block code */
            માટે (;;) {
                here = lencode[BITS(lenbits)];
                જો ((unsigned)(here.bits) <= bits) તોડો;
                PULLBYTE();
            }
            જો (here.op && (here.op & 0xf0) == 0) {
                last = here;
                માટે (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    જો ((unsigned)(last.bits + here.bits) <= bits) તોડો;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            જો (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned અક્ષર)(length);
                ડાબું--;
                mode = LEN;
                તોડો;
            }

            /* process end of block */
            જો (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                તોડો;
            }

            /* invalid code */
            જો (here.op & 64) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid literal/length code";
                mode = BAD;
                તોડો;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            જો (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            માટે (;;) {
                here = distcode[BITS(distbits)];
                જો ((unsigned)(here.bits) <= bits) તોડો;
                PULLBYTE();
            }
            જો ((here.op & 0xf0) == 0) {
                last = here;
                માટે (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    જો ((unsigned)(last.bits + here.bits) <= bits) તોડો;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            જો (here.op & 64) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid distance code";
                mode = BAD;
                તોડો;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            જો (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            જો (offset > WSIZE - (wrap ? 0: ડાબું)) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid distance too far back";
                mode = BAD;
                તોડો;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            કરો {
                ROOM();
                copy = WSIZE - offset;
                જો (copy < ડાબું) {
                    from = put + copy;
                    copy = ડાબું - copy;
                }
                નહીંતર {
                    from = put - offset;
                    copy = ડાબું;
                }
                જો (copy > length) copy = length;
                length -= copy;
                ડાબું -= copy;
                કરો {
                    *put++ = *from++;
                } જ્યારે (--copy);
            } જ્યારે (length != 0);
            તોડો;

        સ્થિતિ DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            જો (ડાબું < WSIZE) {
                જો (out(out_desc, window, (unsigned)(WSIZE - ડાબું)))
                    ret = Z_BUF_ERROR;
            }
            જાઓ inf_leave;

        સ્થિતિ BAD:
            ret = Z_DATA_ERROR;
            જાઓ inf_leave;

        મૂળભૂત:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            જાઓ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    પાછા ret;
}

પૂર્ણાંક ZEXPORT inflateBack9End(z_stream FAR *strm) {
    જો (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        પાછા Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    પાછા Z_OK;
}
