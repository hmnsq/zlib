/* infback.c -- inflate using a call-back interface
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
   This code is largely copied from inflate.c.  Normally either infback.o or
   inflate.o would be linked into an application--not both.  The interface
   with inffast.c is retained so that optimized assembler-coded versions of
   inflate_fast() can be used with either inflate.c or infback.c.
 */

#અન્તર્ભાવ "zutil.h"
#અન્તર્ભાવ "inftrees.h"
#અન્તર્ભાવ "inflate.h"
#અન્તર્ભાવ "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
પૂર્ણાંક ZEXPORT inflateBackInit_(z_streamp strm, પૂર્ણાંક windowBits,
                             unsigned અક્ષર FAR *window, અચલ અક્ષર *version,
                             પૂર્ણાંક stream_size) {
    સંરચના inflate_state FAR *state;

    જો (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (પૂર્ણાંક)(કદ(z_stream)))
        પાછા Z_VERSION_ERROR;
    જો (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        પાછા Z_STREAM_ERROR;
    strm->સંદેશ = Z_NULL;                 /* in case we return an error */
    જો (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        પાછા Z_STREAM_ERROR;
#નહીંતર
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    જો (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        પાછા Z_STREAM_ERROR;
#નહીંતર
        strm->zfree = zcfree;
#endif
    state = (સંરચના inflate_state FAR *)ZALLOC(strm, 1,
                                               કદ(સંરચના inflate_state));
    જો (state == Z_NULL) પાછા Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (સંરચના internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    પાછા Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    કરો { \
        put = strm->next_out; \
        ડાબું = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } જ્યારે (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    કરો { \
        strm->next_out = put; \
        strm->avail_out = ડાબું; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } જ્યારે (0)

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

/* Return the low n bits of the bit accumulator (n < 16) */
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
            put = state->window; \
            ડાબું = state->wsize; \
            state->whave = ડાબું; \
            જો (out(out_desc, put, ડાબું)) { \
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
પૂર્ણાંક ZEXPORT inflateBack(z_streamp strm, in_func in, શૂન્ય FAR *in_desc,
                        out_func out, શૂન્ય FAR *out_desc) {
    સંરચના inflate_state FAR *state;
    z_const unsigned અક્ષર FAR *next;    /* next input */
    unsigned અક્ષર FAR *put;     /* next output */
    unsigned have, ડાબું;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned અક્ષર FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    પૂર્ણાંક ret;                    /* return code */
    સ્થિર અચલ unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    જો (strm == Z_NULL || strm->state == Z_NULL)
        પાછા Z_STREAM_ERROR;
    state = (સંરચના inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->સંદેશ = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    ડાબું = state->wsize;

    /* Inflate until end of block marked as last */
    માટે (;;)
        પસંદગી (state->mode) {
        સ્થિતિ TYPE:
            /* determine and dispatch block type */
            જો (state->last) {
                BYTEBITS();
                state->mode = DONE;
                તોડો;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            પસંદગી (BITS(2)) {
            સ્થિતિ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                તોડો;
            સ્થિતિ 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                તોડો;
            સ્થિતિ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                તોડો;
            મૂળભૂત:
                strm->સંદેશ = (z_const અક્ષર *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            તોડો;

        સ્થિતિ STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            જો ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid stored block lengths";
                state->mode = BAD;
                તોડો;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            જ્યારે (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                જો (copy > have) copy = have;
                જો (copy > ડાબું) copy = ડાબું;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                ડાબું -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
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
#ifndef PKZIP_BUG_WORKAROUND
            જો (state->nlen > 286 || state->ndist > 30) {
                strm->સંદેશ = (z_const અક્ષર *)
                    "too many length or distance symbols";
                state->mode = BAD;
                તોડો;
            }
#endif
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
            state->lencode = (code અચલ FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            જો (ret) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid code lengths set";
                state->mode = BAD;
                તોડો;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            જ્યારે (state->have < state->nlen + state->ndist) {
                માટે (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    જો ((unsigned)(here.bits) <= bits) તોડો;
                    PULLBYTE();
                }
                જો (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                નહીંતર {
                    જો (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        જો (state->have == 0) {
                            strm->સંદેશ = (z_const અક્ષર *)
                                "invalid bit length repeat";
                            state->mode = BAD;
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
                        strm->સંદેશ = (z_const અક્ષર *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        તોડો;
                    }
                    જ્યારે (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            જો (state->mode == BAD) તોડો;

            /* check for end-of-block code (better have one) */
            જો (state->lens[256] == 0) {
                strm->સંદેશ = (z_const અક્ષર *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                તોડો;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code અચલ FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            જો (ret) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid literal/lengths set";
                state->mode = BAD;
                તોડો;
            }
            state->distcode = (code અચલ FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            જો (ret) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid distances set";
                state->mode = BAD;
                તોડો;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        સ્થિતિ LEN:
            /* use inflate_fast() if we have enough input and output */
            જો (have >= 6 && ડાબું >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                તોડો;
            }

            /* get a literal, length, or end-of-block code */
            માટે (;;) {
                here = state->lencode[BITS(state->lenbits)];
                જો ((unsigned)(here.bits) <= bits) તોડો;
                PULLBYTE();
            }
            જો (here.op && (here.op & 0xf0) == 0) {
                last = here;
                માટે (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    જો ((unsigned)(last.bits + here.bits) <= bits) તોડો;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            જો (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned અક્ષર)(state->length);
                ડાબું--;
                state->mode = LEN;
                તોડો;
            }

            /* process end of block */
            જો (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                તોડો;
            }

            /* invalid code */
            જો (here.op & 64) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid literal/length code";
                state->mode = BAD;
                તોડો;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            જો (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            માટે (;;) {
                here = state->distcode[BITS(state->distbits)];
                જો ((unsigned)(here.bits) <= bits) તોડો;
                PULLBYTE();
            }
            જો ((here.op & 0xf0) == 0) {
                last = here;
                માટે (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    જો ((unsigned)(last.bits + here.bits) <= bits) તોડો;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            જો (here.op & 64) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid distance code";
                state->mode = BAD;
                તોડો;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            જો (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            જો (state->offset > state->wsize - (state->whave < state->wsize ?
                                                ડાબું : 0)) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid distance too far back";
                state->mode = BAD;
                તોડો;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            કરો {
                ROOM();
                copy = state->wsize - state->offset;
                જો (copy < ડાબું) {
                    from = put + copy;
                    copy = ડાબું - copy;
                }
                નહીંતર {
                    from = put - state->offset;
                    copy = ડાબું;
                }
                જો (copy > state->length) copy = state->length;
                state->length -= copy;
                ડાબું -= copy;
                કરો {
                    *put++ = *from++;
                } જ્યારે (--copy);
            } જ્યારે (state->length != 0);
            તોડો;

        સ્થિતિ DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            જાઓ inf_leave;

        સ્થિતિ BAD:
            ret = Z_DATA_ERROR;
            જાઓ inf_leave;

        મૂળભૂત:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            જાઓ inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    જો (ડાબું < state->wsize) {
        જો (out(out_desc, state->window, state->wsize - ડાબું) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    પાછા ret;
}

પૂર્ણાંક ZEXPORT inflateBackEnd(z_streamp strm) {
    જો (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        પાછા Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    પાછા Z_OK;
}
