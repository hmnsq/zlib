/* inflate.c -- zlib decompression
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * Change history:
 *
 * 1.2.beta0    24 Nov 2002
 * - First version -- complete rewrite of inflate to simplify code, avoid
 *   creation of window when not needed, minimize use of window when it is
 *   needed, make inffast.c even faster, implement gzip decoding, and to
 *   improve code readability and style over the previous zlib inflate code
 *
 * 1.2.beta1    25 Nov 2002
 * - Use pointers for available input and output checking in inffast.c
 * - Remove input and output counters in inffast.c
 * - Change inffast.c entry and loop from avail_in >= 7 to >= 6
 * - Remove unnecessary second byte pull from length extra in inffast.c
 * - Unroll direct copy to three copies per loop in inffast.c
 *
 * 1.2.beta2    4 Dec 2002
 * - Change external routine names to reduce potential conflicts
 * - Correct filename to inffixed.h for fixed tables in inflate.c
 * - Make hbuf[] unsigned char to match parameter type in inflate.c
 * - Change strm->next_out[-state->offset] to *(strm->next_out - state->offset)
 *   to avoid negation problem on Alphas (64 bit) in inflate.c
 *
 * 1.2.beta3    22 Dec 2002
 * - Add comments on state->bits assertion in inffast.c
 * - Add comments on op field in inftrees.h
 * - Fix bug in reuse of allocated window after inflateReset()
 * - Remove bit fields--back to byte structure for speed
 * - Remove distance extra == 0 check in inflate_fast()--only helps for lengths
 * - Change post-increments to pre-increments in inflate_fast(), PPC biased?
 * - Add compile time option, POSTINC, to use post-increments instead (Intel?)
 * - Make MATCH copy in inflate() much faster for when inflate_fast() not used
 * - Use local copies of stream next and avail values, as well as local bit
 *   buffer and bit count in inflate()--for speed when inflate_fast() not used
 *
 * 1.2.beta4    1 Jan 2003
 * - Split ptr - 257 statements in inflate_table() to avoid compiler warnings
 * - Move a comment on output buffer sizes from inffast.c to inflate.c
 * - Add comments in inffast.c to introduce the inflate_fast() routine
 * - Rearrange window copies in inflate_fast() for speed and simplification
 * - Unroll last copy for window match in inflate_fast()
 * - Use local copies of window variables in inflate_fast() for speed
 * - Pull out common wnext == 0 case for speed in inflate_fast()
 * - Make op and len in inflate_fast() unsigned for consistency
 * - Add FAR to lcode and dcode declarations in inflate_fast()
 * - Simplified bad distance check in inflate_fast()
 * - Added inflateBackInit(), inflateBack(), and inflateBackEnd() in new
 *   source file infback.c to provide a call-back interface to inflate for
 *   programs like gzip and unzip -- uses window as output buffer to avoid
 *   window copying
 *
 * 1.2.beta5    1 Jan 2003
 * - Improved inflateBack() interface to allow the caller to provide initial
 *   input in strm.
 * - Fixed stored blocks bug in inflateBack()
 *
 * 1.2.beta6    4 Jan 2003
 * - Added comments in inffast.c on effectiveness of POSTINC
 * - Typecasting all around to reduce compiler warnings
 * - Changed loops from while (1) or do {} while (1) to for (;;), again to
 *   make compilers happy
 * - Changed type of window in inflateBackInit() to unsigned char *
 *
 * 1.2.beta7    27 Jan 2003
 * - Changed many types to unsigned or unsigned short to avoid warnings
 * - Added inflateCopy() function
 *
 * 1.2.0        9 Mar 2003
 * - Changed inflateBack() interface to provide separate opaque descriptors
 *   for the in() and out() functions
 * - Changed inflateBack() argument and in_func typedef to swap the length
 *   and buffer address return values for the input function
 * - Check next_in and next_out for Z_NULL on entry to inflate()
 *
 * The history for versions after 1.2.0 are in ChangeLog in zlib distribution.
 */

#अन्तर्भाव "zutil.h"
#अन्तर्भाव "inftrees.h"
#अन्तर्भाव "inflate.h"
#अन्तर्भाव "inffast.h"

local पूर्णाङ्क inflateStateCheck(z_streamp strm) {
    संरचना inflate_state FAR *state;
    यदि (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        प्रत्यागम 1;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (state == Z_NULL || state->strm != strm ||
        state->mode < HEAD || state->mode > SYNC)
        प्रत्यागम 1;
    प्रत्यागम 0;
}

पूर्णाङ्क ZEXPORT inflateResetKeep(z_streamp strm) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    strm->total_in = strm->total_out = state->total = 0;
    strm->संदेश = Z_NULL;
    strm->data_type = 0;
    यदि (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->mode = HEAD;
    state->last = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->head = Z_NULL;
    state->hold = 0;
    state->bits = 0;
    state->lencode = state->distcode = state->next = state->codes;
    state->sane = 1;
    state->back = -1;
    Tracev((stderr, "inflate: reset\n"));
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateReset(z_streamp strm) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    प्रत्यागम inflateResetKeep(strm);
}

पूर्णाङ्क ZEXPORT inflateReset2(z_streamp strm, पूर्णाङ्क windowBits) {
    पूर्णाङ्क wrap;
    संरचना inflate_state FAR *state;

    /* get the state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    यदि (windowBits < 0) {
        यदि (windowBits < -15)
            प्रत्यागम Z_STREAM_ERROR;
        wrap = 0;
        windowBits = -windowBits;
    }
    अन्यथा {
        wrap = (windowBits >> 4) + 5;
#ifdef GUNZIP
        यदि (windowBits < 48)
            windowBits &= 15;
#endif
    }

    /* set number of window bits, free window if different */
    यदि (windowBits && (windowBits < 8 || windowBits > 15))
        प्रत्यागम Z_STREAM_ERROR;
    यदि (state->window != Z_NULL && state->wbits != (unsigned)windowBits) {
        ZFREE(strm, state->window);
        state->window = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (unsigned)windowBits;
    प्रत्यागम inflateReset(strm);
}

पूर्णाङ्क ZEXPORT inflateInit2_(z_streamp strm, पूर्णाङ्क windowBits,
                          ध्रुव वर्ण *version, पूर्णाङ्क stream_size) {
    पूर्णाङ्क ret;
    संरचना inflate_state FAR *state;

    यदि (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (पूर्णाङ्क)(परिमाण(z_stream)))
        प्रत्यागम Z_VERSION_ERROR;
    यदि (strm == Z_NULL) प्रत्यागम Z_STREAM_ERROR;
    strm->संदेश = Z_NULL;                 /* in case we return an error */
    यदि (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        प्रत्यागम Z_STREAM_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    यदि (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        प्रत्यागम Z_STREAM_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#endif
    state = (संरचना inflate_state FAR *)
            ZALLOC(strm, 1, परिमाण(संरचना inflate_state));
    यदि (state == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(state, परिमाण(संरचना inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (संरचना internal_state FAR *)state;
    state->strm = strm;
    state->window = Z_NULL;
    state->mode = HEAD;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, windowBits);
    यदि (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    प्रत्यागम ret;
}

पूर्णाङ्क ZEXPORT inflateInit_(z_streamp strm, ध्रुव वर्ण *version,
                         पूर्णाङ्क stream_size) {
    प्रत्यागम inflateInit2_(strm, DEF_WBITS, version, stream_size);
}

पूर्णाङ्क ZEXPORT inflate_मूळसंख्या(z_streamp strm, पूर्णाङ्क bits, पूर्णाङ्क मूल्य) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    यदि (bits == 0)
        प्रत्यागम Z_OK;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (bits < 0) {
        state->hold = 0;
        state->bits = 0;
        प्रत्यागम Z_OK;
    }
    यदि (bits > 16 || state->bits + (uInt)bits > 32) प्रत्यागम Z_STREAM_ERROR;
    मूल्य &= (1L << bits) - 1;
    state->hold += (unsigned long)मूल्य << state->bits;
    state->bits += (uInt)bits;
    प्रत्यागम Z_OK;
}

/*
   Update the window with the last wsize (normally 32K) bytes written before
   returning.  If window does not exist yet, create it.  This is only called
   when a window is already in use, or when output has been written during this
   inflate call, but the end of the deflate stream has not been reached yet.
   It is also called to create a window for dictionary data when a dictionary
   is loaded.

   Providing output buffers larger than 32K to inflate() should provide a speed
   advantage, since only the last 32K of output is copied to the sliding window
   upon return from inflate(), and since all distances after the first 32K of
   output will fall in the output data, making match copies simpler and faster.
   The advantage may be dependent on the size of the processor's data caches.
 */
local पूर्णाङ्क updatewindow(z_streamp strm, ध्रुव Bytef *end, unsigned copy) {
    संरचना inflate_state FAR *state;
    unsigned dist;

    state = (संरचना inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    यदि (state->window == Z_NULL) {
        state->window = (unsigned वर्ण FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               परिमाण(unsigned वर्ण));
        यदि (state->window == Z_NULL) प्रत्यागम 1;
    }

    /* if window not in use yet, initialize */
    यदि (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    यदि (copy >= state->wsize) {
        zmemcpy(state->window, end - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    अन्यथा {
        dist = state->wsize - state->wnext;
        यदि (dist > copy) dist = copy;
        zmemcpy(state->window + state->wnext, end - copy, dist);
        copy -= dist;
        यदि (copy) {
            zmemcpy(state->window, end - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        अन्यथा {
            state->wnext += dist;
            यदि (state->wnext == state->wsize) state->wnext = 0;
            यदि (state->whave < state->wsize) state->whave += dist;
        }
    }
    प्रत्यागम 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#ifdef GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? crc32(check, buf, len) : adler32(check, buf, len))
#अन्यथा
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#endif

/* check macros for header crc */
#ifdef GUNZIP
#  define CRC2(check, word) \
    कुरु { \
        hbuf[0] = (unsigned वर्ण)(word); \
        hbuf[1] = (unsigned वर्ण)((word) >> 8); \
        check = crc32(check, hbuf, 2); \
    } यावत् (0)

#  define CRC4(check, word) \
    कुरु { \
        hbuf[0] = (unsigned वर्ण)(word); \
        hbuf[1] = (unsigned वर्ण)((word) >> 8); \
        hbuf[2] = (unsigned वर्ण)((word) >> 16); \
        hbuf[3] = (unsigned वर्ण)((word) >> 24); \
        check = crc32(check, hbuf, 4); \
    } यावत् (0)
#endif

/* Load registers with state in inflate() for speed */
#define LOAD() \
    कुरु { \
        put = strm->next_out; \
        डावा = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } यावत् (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
    कुरु { \
        strm->next_out = put; \
        strm->avail_out = डावा; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } यावत् (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    कुरु { \
        hold = 0; \
        bits = 0; \
    } यावत् (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#define PULLBYTE() \
    कुरु { \
        यदि (have == 0) गच्छ inf_leave; \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } यावत् (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#define NEEDBITS(n) \
    कुरु { \
        यावत् (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } यावत् (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    कुरु { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } यावत् (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    कुरु { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } यावत् (0)

/*
   inflate() uses a state machine to process as much input data and generate as
   much output data as possible before returning.  The state machine is
   structured roughly as follows:

    for (;;) switch (state) {
    ...
    case STATEn:
        if (not enough input data or output space to make progress)
            return;
        ... make progress ...
        state = STATEm;
        break;
    ...
    }

   so when inflate() is called again, the same case is attempted again, and
   if the appropriate resources are provided, the machine proceeds to the
   next state.  The NEEDBITS() macro is usually the way the state evaluates
   whether it can proceed or should return.  NEEDBITS() does the return if
   the requested bits are not available.  The typical use of the BITS macros
   is:

        NEEDBITS(n);
        ... do something with BITS(n) ...
        DROPBITS(n);

   where NEEDBITS(n) either returns from inflate() if there isn't enough
   input left to load n bits into the accumulator, or it continues.  BITS(n)
   gives the low n bits in the accumulator.  When done, DROPBITS(n) drops
   the low n bits off the accumulator.  INITBITS() clears the accumulator
   and sets the number of available bits to zero.  BYTEBITS() discards just
   enough bits to put the accumulator on a byte boundary.  After BYTEBITS()
   and a NEEDBITS(8), then BITS(8) would return the next byte in the stream.

   NEEDBITS(n) uses PULLBYTE() to get an available byte of input, or to return
   if there is no input available.  The decoding of variable length codes uses
   PULLBYTE() directly in order to pull just enough bytes to decode the next
   code, and no more.

   Some states loop until they get enough input, making sure that enough
   state information is maintained to continue the loop where it left off
   if NEEDBITS() returns in the loop.  For example, want, need, and keep
   would all have to actually be part of the saved state in case NEEDBITS()
   returns:

    case STATEw:
        while (want < need) {
            NEEDBITS(n);
            keep[want++] = BITS(n);
            DROPBITS(n);
        }
        state = STATEx;
    case STATEx:

   As shown above, if the next state is also the next case, then the break
   is omitted.

   A state may also return if there is not enough output space available to
   complete that state.  Those states are copying stored data, writing a
   literal byte, and copying a matching string.

   When returning, a "goto inf_leave" is used to update the total counters,
   update the check value, and determine whether any progress has been made
   during that inflate() call in order to return the proper return code.
   Progress is defined as a change in either strm->avail_in or strm->avail_out.
   When there is a window, goto inf_leave will update the window with the last
   output written.  If a goto inf_leave occurs in the middle of decompression
   and there is no window currently, goto inf_leave will create one and copy
   output to the window for the next call of inflate().

   In this implementation, the flush parameter of inflate() only affects the
   return code (per zlib.h).  inflate() always writes as much as possible to
   strm->next_out, given the space available and the provided input--the effect
   documented in zlib.h of Z_SYNC_FLUSH.  Furthermore, inflate() always defers
   the allocation of and copying into a sliding window until necessary, which
   provides the effect documented in zlib.h for Z_FINISH when the entire input
   stream available.  So the only thing the flush parameter actually does is:
   when flush is set to Z_FINISH, inflate() cannot return Z_OK.  Instead it
   will return Z_BUF_ERROR if it has not reached the end of the stream.
 */

पूर्णाङ्क ZEXPORT inflate(z_streamp strm, पूर्णाङ्क flush) {
    संरचना inflate_state FAR *state;
    z_const unsigned वर्ण FAR *next;    /* next input */
    unsigned वर्ण FAR *put;     /* next output */
    unsigned have, डावा;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned in, out;           /* save starting available input and output */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned वर्ण FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    पूर्णाङ्क ret;                    /* return code */
#ifdef GUNZIP
    unsigned वर्ण hbuf[4];      /* buffer for gzip header crc calculation */
#endif
    स्थिर ध्रुव unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    यदि (inflateStateCheck(strm) || strm->next_out == Z_NULL ||
        (strm->next_in == Z_NULL && strm->avail_in != 0))
        प्रत्यागम Z_STREAM_ERROR;

    state = (संरचना inflate_state FAR *)strm->state;
    यदि (state->mode == TYPE) state->mode = TYPEDO;      /* skip check */
    LOAD();
    in = have;
    out = डावा;
    ret = Z_OK;
    प्रति (;;)
        विकल्प (state->mode) {
        पक्ष HEAD:
            यदि (state->wrap == 0) {
                state->mode = TYPEDO;
                भङ्ग;
            }
            NEEDBITS(16);
#ifdef GUNZIP
            यदि ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                यदि (state->wbits == 0)
                    state->wbits = 15;
                state->check = crc32(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->mode = FLAGS;
                भङ्ग;
            }
            यदि (state->head != Z_NULL)
                state->head->done = -1;
            यदि (!(state->wrap & 1) ||   /* check if zlib header allowed */
#अन्यथा
            यदि (
#endif
                ((BITS(8) << 8) + (hold >> 8)) % 31) {
                strm->संदेश = (z_const वर्ण *)"incorrect header check";
                state->mode = BAD;
                भङ्ग;
            }
            यदि (BITS(4) != Z_DEFLATED) {
                strm->संदेश = (z_const वर्ण *)"unknown compression method";
                state->mode = BAD;
                भङ्ग;
            }
            DROPBITS(4);
            len = BITS(4) + 8;
            यदि (state->wbits == 0)
                state->wbits = len;
            यदि (len > 15 || len > state->wbits) {
                strm->संदेश = (z_const वर्ण *)"invalid window size";
                state->mode = BAD;
                भङ्ग;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            भङ्ग;
#ifdef GUNZIP
        पक्ष FLAGS:
            NEEDBITS(16);
            state->flags = (पूर्णाङ्क)(hold);
            यदि ((state->flags & 0xff) != Z_DEFLATED) {
                strm->संदेश = (z_const वर्ण *)"unknown compression method";
                state->mode = BAD;
                भङ्ग;
            }
            यदि (state->flags & 0xe000) {
                strm->संदेश = (z_const वर्ण *)"unknown header flags set";
                state->mode = BAD;
                भङ्ग;
            }
            यदि (state->head != Z_NULL)
                state->head->text = (पूर्णाङ्क)((hold >> 8) & 1);
            यदि ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = TIME;
                /* fallthrough */
        पक्ष TIME:
            NEEDBITS(32);
            यदि (state->head != Z_NULL)
                state->head->time = hold;
            यदि ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->mode = OS;
                /* fallthrough */
        पक्ष OS:
            NEEDBITS(16);
            यदि (state->head != Z_NULL) {
                state->head->xflags = (पूर्णाङ्क)(hold & 0xff);
                state->head->os = (पूर्णाङ्क)(hold >> 8);
            }
            यदि ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = EXLEN;
                /* fallthrough */
        पक्ष EXLEN:
            यदि (state->flags & 0x0400) {
                NEEDBITS(16);
                state->length = (unsigned)(hold);
                यदि (state->head != Z_NULL)
                    state->head->extra_len = (unsigned)hold;
                यदि ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            अन्यथा यदि (state->head != Z_NULL)
                state->head->extra = Z_NULL;
            state->mode = EXTRA;
                /* fallthrough */
        पक्ष EXTRA:
            यदि (state->flags & 0x0400) {
                copy = state->length;
                यदि (copy > have) copy = have;
                यदि (copy) {
                    यदि (state->head != Z_NULL &&
                        state->head->extra != Z_NULL &&
                        (len = state->head->extra_len - state->length) <
                            state->head->extra_max) {
                        zmemcpy(state->head->extra + len, next,
                                len + copy > state->head->extra_max ?
                                state->head->extra_max - len : copy);
                    }
                    यदि ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = crc32(state->check, next, copy);
                    have -= copy;
                    next += copy;
                    state->length -= copy;
                }
                यदि (state->length) गच्छ inf_leave;
            }
            state->length = 0;
            state->mode = NAME;
                /* fallthrough */
        पक्ष NAME:
            यदि (state->flags & 0x0800) {
                यदि (have == 0) गच्छ inf_leave;
                copy = 0;
                कुरु {
                    len = (unsigned)(next[copy++]);
                    यदि (state->head != Z_NULL &&
                            state->head->name != Z_NULL &&
                            state->length < state->head->name_max)
                        state->head->name[state->length++] = (Bytef)len;
                } यावत् (len && copy < have);
                यदि ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                यदि (len) गच्छ inf_leave;
            }
            अन्यथा यदि (state->head != Z_NULL)
                state->head->name = Z_NULL;
            state->length = 0;
            state->mode = COMMENT;
                /* fallthrough */
        पक्ष COMMENT:
            यदि (state->flags & 0x1000) {
                यदि (have == 0) गच्छ inf_leave;
                copy = 0;
                कुरु {
                    len = (unsigned)(next[copy++]);
                    यदि (state->head != Z_NULL &&
                            state->head->comment != Z_NULL &&
                            state->length < state->head->comm_max)
                        state->head->comment[state->length++] = (Bytef)len;
                } यावत् (len && copy < have);
                यदि ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                यदि (len) गच्छ inf_leave;
            }
            अन्यथा यदि (state->head != Z_NULL)
                state->head->comment = Z_NULL;
            state->mode = HCRC;
                /* fallthrough */
        पक्ष HCRC:
            यदि (state->flags & 0x0200) {
                NEEDBITS(16);
                यदि ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->संदेश = (z_const वर्ण *)"header crc mismatch";
                    state->mode = BAD;
                    भङ्ग;
                }
                INITBITS();
            }
            यदि (state->head != Z_NULL) {
                state->head->hcrc = (पूर्णाङ्क)((state->flags >> 9) & 1);
                state->head->done = 1;
            }
            strm->adler = state->check = crc32(0L, Z_NULL, 0);
            state->mode = TYPE;
            भङ्ग;
#endif
        पक्ष DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->mode = DICT;
                /* fallthrough */
        पक्ष DICT:
            यदि (state->havedict == 0) {
                RESTORE();
                प्रत्यागम Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = TYPE;
                /* fallthrough */
        पक्ष TYPE:
            यदि (flush == Z_BLOCK || flush == Z_TREES) गच्छ inf_leave;
                /* fallthrough */
        पक्ष TYPEDO:
            यदि (state->last) {
                BYTEBITS();
                state->mode = CHECK;
                भङ्ग;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            विकल्प (BITS(2)) {
            पक्ष 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                भङ्ग;
            पक्ष 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN_;             /* decode codes */
                यदि (flush == Z_TREES) {
                    DROPBITS(2);
                    गच्छ inf_leave;
                }
                भङ्ग;
            पक्ष 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                भङ्ग;
            स्वभाव:
                strm->संदेश = (z_const वर्ण *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            भङ्ग;
        पक्ष STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            यदि ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->संदेश = (z_const वर्ण *)"invalid stored block lengths";
                state->mode = BAD;
                भङ्ग;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();
            state->mode = COPY_;
            यदि (flush == Z_TREES) गच्छ inf_leave;
                /* fallthrough */
        पक्ष COPY_:
            state->mode = COPY;
                /* fallthrough */
        पक्ष COPY:
            copy = state->length;
            यदि (copy) {
                यदि (copy > have) copy = have;
                यदि (copy > डावा) copy = डावा;
                यदि (copy == 0) गच्छ inf_leave;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                डावा -= copy;
                put += copy;
                state->length -= copy;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            भङ्ग;
        पक्ष TABLE:
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            यदि (state->nlen > 286 || state->ndist > 30) {
                strm->संदेश = (z_const वर्ण *)
                    "too many length or distance symbols";
                state->mode = BAD;
                भङ्ग;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->mode = LENLENS;
                /* fallthrough */
        पक्ष LENLENS:
            यावत् (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            यावत् (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = state->distcode = (ध्रुव code FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            यदि (ret) {
                strm->संदेश = (z_const वर्ण *)"invalid code lengths set";
                state->mode = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->mode = CODELENS;
                /* fallthrough */
        पक्ष CODELENS:
            यावत् (state->have < state->nlen + state->ndist) {
                प्रति (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    यदि ((unsigned)(here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                यदि (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अन्यथा {
                    यदि (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        यदि (state->have == 0) {
                            strm->संदेश = (z_const वर्ण *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            भङ्ग;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    अन्यथा यदि (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    अन्यथा {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    यदि (state->have + copy > state->nlen + state->ndist) {
                        strm->संदेश = (z_const वर्ण *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        भङ्ग;
                    }
                    यावत् (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            यदि (state->mode == BAD) भङ्ग;

            /* check for end-of-block code (better have one) */
            यदि (state->lens[256] == 0) {
                strm->संदेश = (z_const वर्ण *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                भङ्ग;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (ध्रुव code FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            यदि (ret) {
                strm->संदेश = (z_const वर्ण *)"invalid literal/lengths set";
                state->mode = BAD;
                भङ्ग;
            }
            state->distcode = (ध्रुव code FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            यदि (ret) {
                strm->संदेश = (z_const वर्ण *)"invalid distances set";
                state->mode = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN_;
            यदि (flush == Z_TREES) गच्छ inf_leave;
                /* fallthrough */
        पक्ष LEN_:
            state->mode = LEN;
                /* fallthrough */
        पक्ष LEN:
            यदि (have >= 6 && डावा >= 258) {
                RESTORE();
                inflate_fast(strm, out);
                LOAD();
                यदि (state->mode == TYPE)
                    state->back = -1;
                भङ्ग;
            }
            state->back = 0;
            प्रति (;;) {
                here = state->lencode[BITS(state->lenbits)];
                यदि ((unsigned)(here.bits) <= bits) भङ्ग;
                PULLBYTE();
            }
            यदि (here.op && (here.op & 0xf0) == 0) {
                last = here;
                प्रति (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    यदि ((unsigned)(last.bits + here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            state->length = (unsigned)here.val;
            यदि ((पूर्णाङ्क)(here.op) == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                state->mode = LIT;
                भङ्ग;
            }
            यदि (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->back = -1;
                state->mode = TYPE;
                भङ्ग;
            }
            यदि (here.op & 64) {
                strm->संदेश = (z_const वर्ण *)"invalid literal/length code";
                state->mode = BAD;
                भङ्ग;
            }
            state->extra = (unsigned)(here.op) & 15;
            state->mode = LENEXT;
                /* fallthrough */
        पक्ष LENEXT:
            यदि (state->extra) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));
            state->was = state->length;
            state->mode = DIST;
                /* fallthrough */
        पक्ष DIST:
            प्रति (;;) {
                here = state->distcode[BITS(state->distbits)];
                यदि ((unsigned)(here.bits) <= bits) भङ्ग;
                PULLBYTE();
            }
            यदि ((here.op & 0xf0) == 0) {
                last = here;
                प्रति (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    यदि ((unsigned)(last.bits + here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            यदि (here.op & 64) {
                strm->संदेश = (z_const वर्ण *)"invalid distance code";
                state->mode = BAD;
                भङ्ग;
            }
            state->offset = (unsigned)here.val;
            state->extra = (unsigned)(here.op) & 15;
            state->mode = DISTEXT;
                /* fallthrough */
        पक्ष DISTEXT:
            यदि (state->extra) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
#ifdef INFLATE_STRICT
            यदि (state->offset > state->dmax) {
                strm->संदेश = (z_const वर्ण *)"invalid distance too far back";
                state->mode = BAD;
                भङ्ग;
            }
#endif
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));
            state->mode = MATCH;
                /* fallthrough */
        पक्ष MATCH:
            यदि (डावा == 0) गच्छ inf_leave;
            copy = out - डावा;
            यदि (state->offset > copy) {         /* copy from window */
                copy = state->offset - copy;
                यदि (copy > state->whave) {
                    यदि (state->sane) {
                        strm->संदेश = (z_const वर्ण *)
                            "invalid distance too far back";
                        state->mode = BAD;
                        भङ्ग;
                    }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    यदि (copy > state->length) copy = state->length;
                    यदि (copy > डावा) copy = डावा;
                    डावा -= copy;
                    state->length -= copy;
                    कुरु {
                        *put++ = 0;
                    } यावत् (--copy);
                    यदि (state->length == 0) state->mode = LEN;
                    भङ्ग;
#endif
                }
                यदि (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->window + (state->wsize - copy);
                }
                अन्यथा
                    from = state->window + (state->wnext - copy);
                यदि (copy > state->length) copy = state->length;
            }
            अन्यथा {                              /* copy from output */
                from = put - state->offset;
                copy = state->length;
            }
            यदि (copy > डावा) copy = डावा;
            डावा -= copy;
            state->length -= copy;
            कुरु {
                *put++ = *from++;
            } यावत् (--copy);
            यदि (state->length == 0) state->mode = LEN;
            भङ्ग;
        पक्ष LIT:
            यदि (डावा == 0) गच्छ inf_leave;
            *put++ = (unsigned वर्ण)(state->length);
            डावा--;
            state->mode = LEN;
            भङ्ग;
        पक्ष CHECK:
            यदि (state->wrap) {
                NEEDBITS(32);
                out -= डावा;
                strm->total_out += out;
                state->total += out;
                यदि ((state->wrap & 4) && out)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - out, out);
                out = डावा;
                यदि ((state->wrap & 4) && (
#ifdef GUNZIP
                     state->flags ? hold :
#endif
                     ZSWAP32(hold)) != state->check) {
                    strm->संदेश = (z_const वर्ण *)"incorrect data check";
                    state->mode = BAD;
                    भङ्ग;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#ifdef GUNZIP
            state->mode = LENGTH;
                /* fallthrough */
        पक्ष LENGTH:
            यदि (state->wrap && state->flags) {
                NEEDBITS(32);
                यदि ((state->wrap & 4) && hold != (state->total & 0xffffffff)) {
                    strm->संदेश = (z_const वर्ण *)"incorrect length check";
                    state->mode = BAD;
                    भङ्ग;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#endif
            state->mode = DONE;
                /* fallthrough */
        पक्ष DONE:
            ret = Z_STREAM_END;
            गच्छ inf_leave;
        पक्ष BAD:
            ret = Z_DATA_ERROR;
            गच्छ inf_leave;
        पक्ष MEM:
            प्रत्यागम Z_MEM_ERROR;
        पक्ष SYNC:
                /* fallthrough */
        स्वभाव:
            प्रत्यागम Z_STREAM_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    यदि (state->wsize || (out != strm->avail_out && state->mode < BAD &&
            (state->mode < CHECK || flush != Z_FINISH)))
        यदि (updatewindow(strm, strm->next_out, out - strm->avail_out)) {
            state->mode = MEM;
            प्रत्यागम Z_MEM_ERROR;
        }
    in -= strm->avail_in;
    out -= strm->avail_out;
    strm->total_in += in;
    strm->total_out += out;
    state->total += out;
    यदि ((state->wrap & 4) && out)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->next_out - out, out);
    strm->data_type = (पूर्णाङ्क)state->bits + (state->last ? 64 : 0) +
                      (state->mode == TYPE ? 128 : 0) +
                      (state->mode == LEN_ || state->mode == COPY_ ? 256 : 0);
    यदि (((in == 0 && out == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    प्रत्यागम ret;
}

पूर्णाङ्क ZEXPORT inflateEnd(z_streamp strm) {
    संरचना inflate_state FAR *state;
    यदि (inflateStateCheck(strm))
        प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (state->window != Z_NULL) ZFREE(strm, state->window);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dictLength) {
    संरचना inflate_state FAR *state;

    /* check state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* copy dictionary */
    यदि (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->window + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->window, state->wnext);
    }
    यदि (dictLength != Z_NULL)
        *dictLength = state->whave;
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateSetDictionary(z_streamp strm, ध्रुव Bytef *dictionary,
                                 uInt dictLength) {
    संरचना inflate_state FAR *state;
    unsigned long dictid;
    पूर्णाङ्क ret;

    /* check state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (state->wrap != 0 && state->mode != DICT)
        प्रत्यागम Z_STREAM_ERROR;

    /* check for correct dictionary identifier */
    यदि (state->mode == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dictLength);
        यदि (dictid != state->check)
            प्रत्यागम Z_DATA_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dictLength, dictLength);
    यदि (ret) {
        state->mode = MEM;
        प्रत्यागम Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateGetHeader(z_streamp strm, gz_headerp head) {
    संरचना inflate_state FAR *state;

    /* check state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि ((state->wrap & 2) == 0) प्रत्यागम Z_STREAM_ERROR;

    /* save header structure */
    state->head = head;
    head->done = 0;
    प्रत्यागम Z_OK;
}

/*
   Search buf[0..len-1] for the pattern: 0, 0, 0xff, 0xff.  Return when found
   or when out of input.  When called, *have is the number of pattern bytes
   found in order so far, in 0..3.  On return *have is updated to the new
   state.  If on return *have equals four, then the pattern was found and the
   return value is how many bytes were read including the last byte of the
   pattern.  If *have is less than four, then the pattern has not been found
   yet and the return value is len.  In the latter case, syncsearch() can be
   called again with more data and the *have state.  *have is initialized to
   zero for the first call.
 */
local unsigned syncsearch(unsigned FAR *have, ध्रुव unsigned वर्ण FAR *buf,
                          unsigned len) {
    unsigned got;
    unsigned next;

    got = *have;
    next = 0;
    यावत् (next < len && got < 4) {
        यदि ((पूर्णाङ्क)(buf[next]) == (got < 2 ? 0 : 0xff))
            got++;
        अन्यथा यदि (buf[next])
            got = 0;
        अन्यथा
            got = 4 - got;
        next++;
    }
    *have = got;
    प्रत्यागम next;
}

पूर्णाङ्क ZEXPORT inflateSync(z_streamp strm) {
    unsigned len;               /* number of bytes to look at or looked at */
    पूर्णाङ्क flags;                  /* temporary to save header status */
    unsigned long in, out;      /* temporary to save total_in and total_out */
    unsigned वर्ण buf[4];       /* to restore bit buffer to byte string */
    संरचना inflate_state FAR *state;

    /* check parameters */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (strm->avail_in == 0 && state->bits < 8) प्रत्यागम Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    यदि (state->mode != SYNC) {
        state->mode = SYNC;
        state->hold >>= state->bits & 7;
        state->bits -= state->bits & 7;
        len = 0;
        यावत् (state->bits >= 8) {
            buf[len++] = (unsigned वर्ण)(state->hold);
            state->hold >>= 8;
            state->bits -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->next_in, strm->avail_in);
    strm->avail_in -= len;
    strm->next_in += len;
    strm->total_in += len;

    /* return no joy or set up to restart inflate() on a new block */
    यदि (state->have != 4) प्रत्यागम Z_DATA_ERROR;
    यदि (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    अन्यथा
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    in = strm->total_in;  out = strm->total_out;
    inflateReset(strm);
    strm->total_in = in;  strm->total_out = out;
    state->flags = flags;
    state->mode = TYPE;
    प्रत्यागम Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
पूर्णाङ्क ZEXPORT inflateSyncPoint(z_streamp strm) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    प्रत्यागम state->mode == STORED && state->bits == 0;
}

पूर्णाङ्क ZEXPORT inflateCopy(z_streamp dest, z_streamp source) {
    संरचना inflate_state FAR *state;
    संरचना inflate_state FAR *copy;
    unsigned वर्ण FAR *window;

    /* check input */
    यदि (inflateStateCheck(source) || dest == Z_NULL)
        प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)source->state;

    /* allocate space */
    copy = (संरचना inflate_state FAR *)
           ZALLOC(source, 1, परिमाण(संरचना inflate_state));
    यदि (copy == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(copy, परिमाण(संरचना inflate_state));
    window = Z_NULL;
    यदि (state->window != Z_NULL) {
        window = (unsigned वर्ण FAR *)
                 ZALLOC(source, 1U << state->wbits, परिमाण(unsigned वर्ण));
        यदि (window == Z_NULL) {
            ZFREE(source, copy);
            प्रत्यागम Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, source, परिमाण(z_stream));
    zmemcpy(copy, state, परिमाण(संरचना inflate_state));
    copy->strm = dest;
    यदि (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->next = copy->codes + (state->next - state->codes);
    यदि (window != Z_NULL)
        zmemcpy(window, state->window, state->whave);
    copy->window = window;
    dest->state = (संरचना internal_state FAR *)copy;
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateUndermine(z_streamp strm, पूर्णाङ्क subvert) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
    state->sane = !subvert;
    प्रत्यागम Z_OK;
#अन्यथा
    (शून्य)subvert;
    state->sane = 1;
    प्रत्यागम Z_DATA_ERROR;
#endif
}

पूर्णाङ्क ZEXPORT inflateValidate(z_streamp strm, पूर्णाङ्क check) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (check && state->wrap)
        state->wrap |= 4;
    अन्यथा
        state->wrap &= ~4;
    प्रत्यागम Z_OK;
}

long ZEXPORT inflateMark(z_streamp strm) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm))
        प्रत्यागम -(1L << 16);
    state = (संरचना inflate_state FAR *)strm->state;
    प्रत्यागम (long)(((unsigned long)((long)state->back)) << 16) +
        (state->mode == COPY ? state->length :
            (state->mode == MATCH ? state->was - state->length : 0));
}

unsigned long ZEXPORT inflateCodesUsed(z_streamp strm) {
    संरचना inflate_state FAR *state;
    यदि (inflateStateCheck(strm)) प्रत्यागम (unsigned long)-1;
    state = (संरचना inflate_state FAR *)strm->state;
    प्रत्यागम (unsigned long)(state->next - state->codes);
}
