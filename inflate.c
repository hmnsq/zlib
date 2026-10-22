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

#অন্তর্ভাব "zutil.h"
#অন্তর্ভাব "inftrees.h"
#অন্তর্ভাব "inflate.h"
#অন্তর্ভাব "inffast.h"

local পূর্ণসংখ্যা inflateStateCheck(z_streamp strm) {
    কাঠামো inflate_state FAR *state;
    যদি (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        ফেরত 1;
    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি (state == Z_NULL || state->strm != strm ||
        state->mode < শীর্ষ_2 || state->mode > SYNC)
        ফেরত 1;
    ফেরত 0;
}

পূর্ণসংখ্যা ZEXPORT inflateResetKeep(z_streamp strm) {
    কাঠামো inflate_state FAR *state;

    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    strm->যোগ_in = strm->যোগ_out = state->যোগ_2 = 0;
    strm->বার্তা = Z_NULL;
    strm->তথ্য_type = 0;
    যদি (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->mode = শীর্ষ_2;
    state->last = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->শীর্ষ = Z_NULL;
    state->hold = 0;
    state->bits = 0;
    state->lencode = state->distcode = state->পরবর্তী = state->codes;
    state->sane = 1;
    state->back = -1;
    Tracev((stderr, "inflate: reset\n"));
    ফেরত Z_OK;
}

পূর্ণসংখ্যা ZEXPORT inflateReset(z_streamp strm) {
    কাঠামো inflate_state FAR *state;

    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    ফেরত inflateResetKeep(strm);
}

পূর্ণসংখ্যা ZEXPORT inflateReset2(z_streamp strm, পূর্ণসংখ্যা windowBits) {
    পূর্ণসংখ্যা wrap;
    কাঠামো inflate_state FAR *state;

    /* get the state */
    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    যদি (windowBits < 0) {
        যদি (windowBits < -15)
            ফেরত Z_STREAM_ERROR;
        wrap = 0;
        windowBits = -windowBits;
    }
    অন্যথা {
        wrap = (windowBits >> 4) + 5;
#ifdef GUNZIP
        যদি (windowBits < 48)
            windowBits &= 15;
#endif
    }

    /* set number of window bits, free window if different */
    যদি (windowBits && (windowBits < 8 || windowBits > 15))
        ফেরত Z_STREAM_ERROR;
    যদি (state->window != Z_NULL && state->wbits != (unsigned)windowBits) {
        ZFREE(strm, state->window);
        state->window = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (unsigned)windowBits;
    ফেরত inflateReset(strm);
}

পূর্ণসংখ্যা ZEXPORT inflateInit2_(z_streamp strm, পূর্ণসংখ্যা windowBits,
                          ধ্রুবক অক্ষর *version, পূর্ণসংখ্যা stream_আকার) {
    পূর্ণসংখ্যা ret;
    কাঠামো inflate_state FAR *state;

    যদি (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_আকার != (পূর্ণসংখ্যা)(আকার(z_stream)))
        ফেরত Z_VERSION_ERROR;
    যদি (strm == Z_NULL) ফেরত Z_STREAM_ERROR;
    strm->বার্তা = Z_NULL;                 /* in case we return an error */
    যদি (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        ফেরত Z_STREAM_ERROR;
#অন্যথা
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    যদি (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        ফেরত Z_STREAM_ERROR;
#অন্যথা
        strm->zfree = zcfree;
#endif
    state = (কাঠামো inflate_state FAR *)
            ZALLOC(strm, 1, আকার(কাঠামো inflate_state));
    যদি (state == Z_NULL) ফেরত Z_MEM_ERROR;
    zmemzero(state, আকার(কাঠামো inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (কাঠামো internal_state FAR *)state;
    state->strm = strm;
    state->window = Z_NULL;
    state->mode = শীর্ষ_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, windowBits);
    যদি (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    ফেরত ret;
}

পূর্ণসংখ্যা ZEXPORT inflateInit_(z_streamp strm, ধ্রুবক অক্ষর *version,
                         পূর্ণসংখ্যা stream_আকার) {
    ফেরত inflateInit2_(strm, DEF_WBITS, version, stream_আকার);
}

পূর্ণসংখ্যা ZEXPORT inflate_মৌলিক(z_streamp strm, পূর্ণসংখ্যা bits, পূর্ণসংখ্যা মান) {
    কাঠামো inflate_state FAR *state;

    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    যদি (bits == 0)
        ফেরত Z_OK;
    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি (bits < 0) {
        state->hold = 0;
        state->bits = 0;
        ফেরত Z_OK;
    }
    যদি (bits > 16 || state->bits + (uInt)bits > 32) ফেরত Z_STREAM_ERROR;
    মান &= (1L << bits) - 1;
    state->hold += (unsigned long)মান << state->bits;
    state->bits += (uInt)bits;
    ফেরত Z_OK;
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
local পূর্ণসংখ্যা updatewindow(z_streamp strm, ধ্রুবক Bytef *end, unsigned copy) {
    কাঠামো inflate_state FAR *state;
    unsigned dist;

    state = (কাঠামো inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    যদি (state->window == Z_NULL) {
        state->window = (unsigned অক্ষর FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               আকার(unsigned অক্ষর));
        যদি (state->window == Z_NULL) ফেরত 1;
    }

    /* if window not in use yet, initialize */
    যদি (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    যদি (copy >= state->wsize) {
        zmemcpy(state->window, end - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    অন্যথা {
        dist = state->wsize - state->wnext;
        যদি (dist > copy) dist = copy;
        zmemcpy(state->window + state->wnext, end - copy, dist);
        copy -= dist;
        যদি (copy) {
            zmemcpy(state->window, end - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        অন্যথা {
            state->wnext += dist;
            যদি (state->wnext == state->wsize) state->wnext = 0;
            যদি (state->whave < state->wsize) state->whave += dist;
        }
    }
    ফেরত 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#ifdef GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? crc32(check, buf, len) : adler32(check, buf, len))
#অন্যথা
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#endif

/* check macros for header crc */
#ifdef GUNZIP
#  define CRC2(check, word) \
    করো { \
        hbuf[0] = (unsigned অক্ষর)(word); \
        hbuf[1] = (unsigned অক্ষর)((word) >> 8); \
        check = crc32(check, hbuf, 2); \
    } যখন (0)

#  define CRC4(check, word) \
    করো { \
        hbuf[0] = (unsigned অক্ষর)(word); \
        hbuf[1] = (unsigned অক্ষর)((word) >> 8); \
        hbuf[2] = (unsigned অক্ষর)((word) >> 16); \
        hbuf[3] = (unsigned অক্ষর)((word) >> 24); \
        check = crc32(check, hbuf, 4); \
    } যখন (0)
#endif

/* Load registers with state in inflate() for speed */
#define LOAD() \
    করো { \
        put = strm->পরবর্তী_out; \
        বাম = strm->avail_out; \
        পরবর্তী = strm->পরবর্তী_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } যখন (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
    করো { \
        strm->পরবর্তী_out = put; \
        strm->avail_out = বাম; \
        strm->পরবর্তী_in = পরবর্তী; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } যখন (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    করো { \
        hold = 0; \
        bits = 0; \
    } যখন (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#define PULLBYTE() \
    করো { \
        যদি (have == 0) যাও inf_leave; \
        have--; \
        hold += (unsigned long)(*পরবর্তী++) << bits; \
        bits += 8; \
    } যখন (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#define NEEDBITS(n) \
    করো { \
        যখন (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } যখন (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    করো { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } যখন (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    করো { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } যখন (0)

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

পূর্ণসংখ্যা ZEXPORT inflate(z_streamp strm, পূর্ণসংখ্যা flush) {
    কাঠামো inflate_state FAR *state;
    z_const unsigned অক্ষর FAR *পরবর্তী;    /* next input */
    unsigned অক্ষর FAR *put;     /* next output */
    unsigned have, বাম;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned in, out;           /* save starting available input and output */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned অক্ষর FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    পূর্ণসংখ্যা ret;                    /* return code */
#ifdef GUNZIP
    unsigned অক্ষর hbuf[4];      /* buffer for gzip header crc calculation */
#endif
    স্থির ধ্রুবক unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    যদি (inflateStateCheck(strm) || strm->পরবর্তী_out == Z_NULL ||
        (strm->পরবর্তী_in == Z_NULL && strm->avail_in != 0))
        ফেরত Z_STREAM_ERROR;

    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি (state->mode == TYPE) state->mode = TYPEDO;      /* skip check */
    LOAD();
    in = have;
    out = বাম;
    ret = Z_OK;
    জন্য (;;)
        নির্বাচন (state->mode) {
        ক্ষেত্র শীর্ষ_2:
            যদি (state->wrap == 0) {
                state->mode = TYPEDO;
                বিরতি;
            }
            NEEDBITS(16);
#ifdef GUNZIP
            যদি ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                যদি (state->wbits == 0)
                    state->wbits = 15;
                state->check = crc32(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->mode = FLAGS;
                বিরতি;
            }
            যদি (state->শীর্ষ != Z_NULL)
                state->শীর্ষ->done = -1;
            যদি (!(state->wrap & 1) ||   /* check if zlib header allowed */
#অন্যথা
            যদি (
#endif
                ((BITS(8) << 8) + (hold >> 8)) % 31) {
                strm->বার্তা = (z_const অক্ষর *)"incorrect header check";
                state->mode = BAD;
                বিরতি;
            }
            যদি (BITS(4) != Z_DEFLATED) {
                strm->বার্তা = (z_const অক্ষর *)"unknown compression method";
                state->mode = BAD;
                বিরতি;
            }
            DROPBITS(4);
            len = BITS(4) + 8;
            যদি (state->wbits == 0)
                state->wbits = len;
            যদি (len > 15 || len > state->wbits) {
                strm->বার্তা = (z_const অক্ষর *)"invalid window size";
                state->mode = BAD;
                বিরতি;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            বিরতি;
#ifdef GUNZIP
        ক্ষেত্র FLAGS:
            NEEDBITS(16);
            state->flags = (পূর্ণসংখ্যা)(hold);
            যদি ((state->flags & 0xff) != Z_DEFLATED) {
                strm->বার্তা = (z_const অক্ষর *)"unknown compression method";
                state->mode = BAD;
                বিরতি;
            }
            যদি (state->flags & 0xe000) {
                strm->বার্তা = (z_const অক্ষর *)"unknown header flags set";
                state->mode = BAD;
                বিরতি;
            }
            যদি (state->শীর্ষ != Z_NULL)
                state->শীর্ষ->text = (পূর্ণসংখ্যা)((hold >> 8) & 1);
            যদি ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = TIME;
                /* fallthrough */
        ক্ষেত্র TIME:
            NEEDBITS(32);
            যদি (state->শীর্ষ != Z_NULL)
                state->শীর্ষ->time = hold;
            যদি ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->mode = OS;
                /* fallthrough */
        ক্ষেত্র OS:
            NEEDBITS(16);
            যদি (state->শীর্ষ != Z_NULL) {
                state->শীর্ষ->xflags = (পূর্ণসংখ্যা)(hold & 0xff);
                state->শীর্ষ->os = (পূর্ণসংখ্যা)(hold >> 8);
            }
            যদি ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = EXLEN;
                /* fallthrough */
        ক্ষেত্র EXLEN:
            যদি (state->flags & 0x0400) {
                NEEDBITS(16);
                state->দৈর্ঘ্য = (unsigned)(hold);
                যদি (state->শীর্ষ != Z_NULL)
                    state->শীর্ষ->extra_len = (unsigned)hold;
                যদি ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            অন্যথা যদি (state->শীর্ষ != Z_NULL)
                state->শীর্ষ->extra = Z_NULL;
            state->mode = EXTRA;
                /* fallthrough */
        ক্ষেত্র EXTRA:
            যদি (state->flags & 0x0400) {
                copy = state->দৈর্ঘ্য;
                যদি (copy > have) copy = have;
                যদি (copy) {
                    যদি (state->শীর্ষ != Z_NULL &&
                        state->শীর্ষ->extra != Z_NULL &&
                        (len = state->শীর্ষ->extra_len - state->দৈর্ঘ্য) <
                            state->শীর্ষ->extra_max) {
                        zmemcpy(state->শীর্ষ->extra + len, পরবর্তী,
                                len + copy > state->শীর্ষ->extra_max ?
                                state->শীর্ষ->extra_max - len : copy);
                    }
                    যদি ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = crc32(state->check, পরবর্তী, copy);
                    have -= copy;
                    পরবর্তী += copy;
                    state->দৈর্ঘ্য -= copy;
                }
                যদি (state->দৈর্ঘ্য) যাও inf_leave;
            }
            state->দৈর্ঘ্য = 0;
            state->mode = NAME;
                /* fallthrough */
        ক্ষেত্র NAME:
            যদি (state->flags & 0x0800) {
                যদি (have == 0) যাও inf_leave;
                copy = 0;
                করো {
                    len = (unsigned)(পরবর্তী[copy++]);
                    যদি (state->শীর্ষ != Z_NULL &&
                            state->শীর্ষ->name != Z_NULL &&
                            state->দৈর্ঘ্য < state->শীর্ষ->name_max)
                        state->শীর্ষ->name[state->দৈর্ঘ্য++] = (Bytef)len;
                } যখন (len && copy < have);
                যদি ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, পরবর্তী, copy);
                have -= copy;
                পরবর্তী += copy;
                যদি (len) যাও inf_leave;
            }
            অন্যথা যদি (state->শীর্ষ != Z_NULL)
                state->শীর্ষ->name = Z_NULL;
            state->দৈর্ঘ্য = 0;
            state->mode = COMMENT;
                /* fallthrough */
        ক্ষেত্র COMMENT:
            যদি (state->flags & 0x1000) {
                যদি (have == 0) যাও inf_leave;
                copy = 0;
                করো {
                    len = (unsigned)(পরবর্তী[copy++]);
                    যদি (state->শীর্ষ != Z_NULL &&
                            state->শীর্ষ->comment != Z_NULL &&
                            state->দৈর্ঘ্য < state->শীর্ষ->comm_max)
                        state->শীর্ষ->comment[state->দৈর্ঘ্য++] = (Bytef)len;
                } যখন (len && copy < have);
                যদি ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, পরবর্তী, copy);
                have -= copy;
                পরবর্তী += copy;
                যদি (len) যাও inf_leave;
            }
            অন্যথা যদি (state->শীর্ষ != Z_NULL)
                state->শীর্ষ->comment = Z_NULL;
            state->mode = HCRC;
                /* fallthrough */
        ক্ষেত্র HCRC:
            যদি (state->flags & 0x0200) {
                NEEDBITS(16);
                যদি ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->বার্তা = (z_const অক্ষর *)"header crc mismatch";
                    state->mode = BAD;
                    বিরতি;
                }
                INITBITS();
            }
            যদি (state->শীর্ষ != Z_NULL) {
                state->শীর্ষ->hcrc = (পূর্ণসংখ্যা)((state->flags >> 9) & 1);
                state->শীর্ষ->done = 1;
            }
            strm->adler = state->check = crc32(0L, Z_NULL, 0);
            state->mode = TYPE;
            বিরতি;
#endif
        ক্ষেত্র DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->mode = DICT;
                /* fallthrough */
        ক্ষেত্র DICT:
            যদি (state->havedict == 0) {
                RESTORE();
                ফেরত Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = TYPE;
                /* fallthrough */
        ক্ষেত্র TYPE:
            যদি (flush == Z_BLOCK || flush == Z_TREES) যাও inf_leave;
                /* fallthrough */
        ক্ষেত্র TYPEDO:
            যদি (state->last) {
                BYTEBITS();
                state->mode = CHECK;
                বিরতি;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            নির্বাচন (BITS(2)) {
            ক্ষেত্র 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                বিরতি;
            ক্ষেত্র 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN_;             /* decode codes */
                যদি (flush == Z_TREES) {
                    DROPBITS(2);
                    যাও inf_leave;
                }
                বিরতি;
            ক্ষেত্র 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                বিরতি;
            পূর্বনির্ধারিত:
                strm->বার্তা = (z_const অক্ষর *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            বিরতি;
        ক্ষেত্র STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            যদি ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->বার্তা = (z_const অক্ষর *)"invalid stored block lengths";
                state->mode = BAD;
                বিরতি;
            }
            state->দৈর্ঘ্য = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->দৈর্ঘ্য));
            INITBITS();
            state->mode = COPY_;
            যদি (flush == Z_TREES) যাও inf_leave;
                /* fallthrough */
        ক্ষেত্র COPY_:
            state->mode = COPY;
                /* fallthrough */
        ক্ষেত্র COPY:
            copy = state->দৈর্ঘ্য;
            যদি (copy) {
                যদি (copy > have) copy = have;
                যদি (copy > বাম) copy = বাম;
                যদি (copy == 0) যাও inf_leave;
                zmemcpy(put, পরবর্তী, copy);
                have -= copy;
                পরবর্তী += copy;
                বাম -= copy;
                put += copy;
                state->দৈর্ঘ্য -= copy;
                বিরতি;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            বিরতি;
        ক্ষেত্র TABLE:
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            যদি (state->nlen > 286 || state->ndist > 30) {
                strm->বার্তা = (z_const অক্ষর *)
                    "too many length or distance symbols";
                state->mode = BAD;
                বিরতি;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->mode = LENLENS;
                /* fallthrough */
        ক্ষেত্র LENLENS:
            যখন (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            যখন (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->পরবর্তী = state->codes;
            state->lencode = state->distcode = (ধ্রুবক code FAR *)(state->পরবর্তী);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->পরবর্তী),
                                &(state->lenbits), state->work);
            যদি (ret) {
                strm->বার্তা = (z_const অক্ষর *)"invalid code lengths set";
                state->mode = BAD;
                বিরতি;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->mode = CODELENS;
                /* fallthrough */
        ক্ষেত্র CODELENS:
            যখন (state->have < state->nlen + state->ndist) {
                জন্য (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    যদি ((unsigned)(here.bits) <= bits) বিরতি;
                    PULLBYTE();
                }
                যদি (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                অন্যথা {
                    যদি (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        যদি (state->have == 0) {
                            strm->বার্তা = (z_const অক্ষর *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            বিরতি;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    অন্যথা যদি (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    অন্যথা {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    যদি (state->have + copy > state->nlen + state->ndist) {
                        strm->বার্তা = (z_const অক্ষর *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        বিরতি;
                    }
                    যখন (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            যদি (state->mode == BAD) বিরতি;

            /* check for end-of-block code (better have one) */
            যদি (state->lens[256] == 0) {
                strm->বার্তা = (z_const অক্ষর *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                বিরতি;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->পরবর্তী = state->codes;
            state->lencode = (ধ্রুবক code FAR *)(state->পরবর্তী);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->পরবর্তী),
                                &(state->lenbits), state->work);
            যদি (ret) {
                strm->বার্তা = (z_const অক্ষর *)"invalid literal/lengths set";
                state->mode = BAD;
                বিরতি;
            }
            state->distcode = (ধ্রুবক code FAR *)(state->পরবর্তী);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->পরবর্তী), &(state->distbits), state->work);
            যদি (ret) {
                strm->বার্তা = (z_const অক্ষর *)"invalid distances set";
                state->mode = BAD;
                বিরতি;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN_;
            যদি (flush == Z_TREES) যাও inf_leave;
                /* fallthrough */
        ক্ষেত্র LEN_:
            state->mode = LEN;
                /* fallthrough */
        ক্ষেত্র LEN:
            যদি (have >= 6 && বাম >= 258) {
                RESTORE();
                inflate_fast(strm, out);
                LOAD();
                যদি (state->mode == TYPE)
                    state->back = -1;
                বিরতি;
            }
            state->back = 0;
            জন্য (;;) {
                here = state->lencode[BITS(state->lenbits)];
                যদি ((unsigned)(here.bits) <= bits) বিরতি;
                PULLBYTE();
            }
            যদি (here.op && (here.op & 0xf0) == 0) {
                last = here;
                জন্য (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    যদি ((unsigned)(last.bits + here.bits) <= bits) বিরতি;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            state->দৈর্ঘ্য = (unsigned)here.val;
            যদি ((পূর্ণসংখ্যা)(here.op) == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                state->mode = LIT;
                বিরতি;
            }
            যদি (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->back = -1;
                state->mode = TYPE;
                বিরতি;
            }
            যদি (here.op & 64) {
                strm->বার্তা = (z_const অক্ষর *)"invalid literal/length code";
                state->mode = BAD;
                বিরতি;
            }
            state->extra = (unsigned)(here.op) & 15;
            state->mode = LENEXT;
                /* fallthrough */
        ক্ষেত্র LENEXT:
            যদি (state->extra) {
                NEEDBITS(state->extra);
                state->দৈর্ঘ্য += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->দৈর্ঘ্য));
            state->was = state->দৈর্ঘ্য;
            state->mode = DIST;
                /* fallthrough */
        ক্ষেত্র DIST:
            জন্য (;;) {
                here = state->distcode[BITS(state->distbits)];
                যদি ((unsigned)(here.bits) <= bits) বিরতি;
                PULLBYTE();
            }
            যদি ((here.op & 0xf0) == 0) {
                last = here;
                জন্য (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    যদি ((unsigned)(last.bits + here.bits) <= bits) বিরতি;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            যদি (here.op & 64) {
                strm->বার্তা = (z_const অক্ষর *)"invalid distance code";
                state->mode = BAD;
                বিরতি;
            }
            state->offset = (unsigned)here.val;
            state->extra = (unsigned)(here.op) & 15;
            state->mode = DISTEXT;
                /* fallthrough */
        ক্ষেত্র DISTEXT:
            যদি (state->extra) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
#ifdef INFLATE_STRICT
            যদি (state->offset > state->dmax) {
                strm->বার্তা = (z_const অক্ষর *)"invalid distance too far back";
                state->mode = BAD;
                বিরতি;
            }
#endif
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));
            state->mode = MATCH;
                /* fallthrough */
        ক্ষেত্র MATCH:
            যদি (বাম == 0) যাও inf_leave;
            copy = out - বাম;
            যদি (state->offset > copy) {         /* copy from window */
                copy = state->offset - copy;
                যদি (copy > state->whave) {
                    যদি (state->sane) {
                        strm->বার্তা = (z_const অক্ষর *)
                            "invalid distance too far back";
                        state->mode = BAD;
                        বিরতি;
                    }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    যদি (copy > state->দৈর্ঘ্য) copy = state->দৈর্ঘ্য;
                    যদি (copy > বাম) copy = বাম;
                    বাম -= copy;
                    state->দৈর্ঘ্য -= copy;
                    করো {
                        *put++ = 0;
                    } যখন (--copy);
                    যদি (state->দৈর্ঘ্য == 0) state->mode = LEN;
                    বিরতি;
#endif
                }
                যদি (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->window + (state->wsize - copy);
                }
                অন্যথা
                    from = state->window + (state->wnext - copy);
                যদি (copy > state->দৈর্ঘ্য) copy = state->দৈর্ঘ্য;
            }
            অন্যথা {                              /* copy from output */
                from = put - state->offset;
                copy = state->দৈর্ঘ্য;
            }
            যদি (copy > বাম) copy = বাম;
            বাম -= copy;
            state->দৈর্ঘ্য -= copy;
            করো {
                *put++ = *from++;
            } যখন (--copy);
            যদি (state->দৈর্ঘ্য == 0) state->mode = LEN;
            বিরতি;
        ক্ষেত্র LIT:
            যদি (বাম == 0) যাও inf_leave;
            *put++ = (unsigned অক্ষর)(state->দৈর্ঘ্য);
            বাম--;
            state->mode = LEN;
            বিরতি;
        ক্ষেত্র CHECK:
            যদি (state->wrap) {
                NEEDBITS(32);
                out -= বাম;
                strm->যোগ_out += out;
                state->যোগ_2 += out;
                যদি ((state->wrap & 4) && out)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - out, out);
                out = বাম;
                যদি ((state->wrap & 4) && (
#ifdef GUNZIP
                     state->flags ? hold :
#endif
                     ZSWAP32(hold)) != state->check) {
                    strm->বার্তা = (z_const অক্ষর *)"incorrect data check";
                    state->mode = BAD;
                    বিরতি;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#ifdef GUNZIP
            state->mode = দৈর্ঘ্য_2;
                /* fallthrough */
        ক্ষেত্র দৈর্ঘ্য_2:
            যদি (state->wrap && state->flags) {
                NEEDBITS(32);
                যদি ((state->wrap & 4) && hold != (state->যোগ_2 & 0xffffffff)) {
                    strm->বার্তা = (z_const অক্ষর *)"incorrect length check";
                    state->mode = BAD;
                    বিরতি;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#endif
            state->mode = DONE;
                /* fallthrough */
        ক্ষেত্র DONE:
            ret = Z_STREAM_END;
            যাও inf_leave;
        ক্ষেত্র BAD:
            ret = Z_তথ্য_ERROR;
            যাও inf_leave;
        ক্ষেত্র MEM:
            ফেরত Z_MEM_ERROR;
        ক্ষেত্র SYNC:
                /* fallthrough */
        পূর্বনির্ধারিত:
            ফেরত Z_STREAM_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    যদি (state->wsize || (out != strm->avail_out && state->mode < BAD &&
            (state->mode < CHECK || flush != Z_FINISH)))
        যদি (updatewindow(strm, strm->পরবর্তী_out, out - strm->avail_out)) {
            state->mode = MEM;
            ফেরত Z_MEM_ERROR;
        }
    in -= strm->avail_in;
    out -= strm->avail_out;
    strm->যোগ_in += in;
    strm->যোগ_out += out;
    state->যোগ_2 += out;
    যদি ((state->wrap & 4) && out)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->পরবর্তী_out - out, out);
    strm->তথ্য_type = (পূর্ণসংখ্যা)state->bits + (state->last ? 64 : 0) +
                      (state->mode == TYPE ? 128 : 0) +
                      (state->mode == LEN_ || state->mode == COPY_ ? 256 : 0);
    যদি (((in == 0 && out == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    ফেরত ret;
}

পূর্ণসংখ্যা ZEXPORT inflateEnd(z_streamp strm) {
    কাঠামো inflate_state FAR *state;
    যদি (inflateStateCheck(strm))
        ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি (state->window != Z_NULL) ZFREE(strm, state->window);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    ফেরত Z_OK;
}

পূর্ণসংখ্যা ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_দৈর্ঘ্য) {
    কাঠামো inflate_state FAR *state;

    /* check state */
    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;

    /* copy dictionary */
    যদি (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->window + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->window, state->wnext);
    }
    যদি (dict_দৈর্ঘ্য != Z_NULL)
        *dict_দৈর্ঘ্য = state->whave;
    ফেরত Z_OK;
}

পূর্ণসংখ্যা ZEXPORT inflateSetDictionary(z_streamp strm, ধ্রুবক Bytef *dictionary,
                                 uInt dict_দৈর্ঘ্য) {
    কাঠামো inflate_state FAR *state;
    unsigned long dictid;
    পূর্ণসংখ্যা ret;

    /* check state */
    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি (state->wrap != 0 && state->mode != DICT)
        ফেরত Z_STREAM_ERROR;

    /* check for correct dictionary identifier */
    যদি (state->mode == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_দৈর্ঘ্য);
        যদি (dictid != state->check)
            ফেরত Z_তথ্য_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_দৈর্ঘ্য, dict_দৈর্ঘ্য);
    যদি (ret) {
        state->mode = MEM;
        ফেরত Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    ফেরত Z_OK;
}

পূর্ণসংখ্যা ZEXPORT inflateGetHeader(z_streamp strm, gz_headerp শীর্ষ) {
    কাঠামো inflate_state FAR *state;

    /* check state */
    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি ((state->wrap & 2) == 0) ফেরত Z_STREAM_ERROR;

    /* save header structure */
    state->শীর্ষ = শীর্ষ;
    শীর্ষ->done = 0;
    ফেরত Z_OK;
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
local unsigned syncsearch(unsigned FAR *have, ধ্রুবক unsigned অক্ষর FAR *buf,
                          unsigned len) {
    unsigned got;
    unsigned পরবর্তী;

    got = *have;
    পরবর্তী = 0;
    যখন (পরবর্তী < len && got < 4) {
        যদি ((পূর্ণসংখ্যা)(buf[পরবর্তী]) == (got < 2 ? 0 : 0xff))
            got++;
        অন্যথা যদি (buf[পরবর্তী])
            got = 0;
        অন্যথা
            got = 4 - got;
        পরবর্তী++;
    }
    *have = got;
    ফেরত পরবর্তী;
}

পূর্ণসংখ্যা ZEXPORT inflateSync(z_streamp strm) {
    unsigned len;               /* number of bytes to look at or looked at */
    পূর্ণসংখ্যা flags;                  /* temporary to save header status */
    unsigned long in, out;      /* temporary to save total_in and total_out */
    unsigned অক্ষর buf[4];       /* to restore bit buffer to byte string */
    কাঠামো inflate_state FAR *state;

    /* check parameters */
    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি (strm->avail_in == 0 && state->bits < 8) ফেরত Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    যদি (state->mode != SYNC) {
        state->mode = SYNC;
        state->hold >>= state->bits & 7;
        state->bits -= state->bits & 7;
        len = 0;
        যখন (state->bits >= 8) {
            buf[len++] = (unsigned অক্ষর)(state->hold);
            state->hold >>= 8;
            state->bits -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->পরবর্তী_in, strm->avail_in);
    strm->avail_in -= len;
    strm->পরবর্তী_in += len;
    strm->যোগ_in += len;

    /* return no joy or set up to restart inflate() on a new block */
    যদি (state->have != 4) ফেরত Z_তথ্য_ERROR;
    যদি (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    অন্যথা
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    in = strm->যোগ_in;  out = strm->যোগ_out;
    inflateReset(strm);
    strm->যোগ_in = in;  strm->যোগ_out = out;
    state->flags = flags;
    state->mode = TYPE;
    ফেরত Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
পূর্ণসংখ্যা ZEXPORT inflateSyncPoint(z_streamp strm) {
    কাঠামো inflate_state FAR *state;

    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    ফেরত state->mode == STORED && state->bits == 0;
}

পূর্ণসংখ্যা ZEXPORT inflateCopy(z_streamp dest, z_streamp source) {
    কাঠামো inflate_state FAR *state;
    কাঠামো inflate_state FAR *copy;
    unsigned অক্ষর FAR *window;

    /* check input */
    যদি (inflateStateCheck(source) || dest == Z_NULL)
        ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)source->state;

    /* allocate space */
    copy = (কাঠামো inflate_state FAR *)
           ZALLOC(source, 1, আকার(কাঠামো inflate_state));
    যদি (copy == Z_NULL) ফেরত Z_MEM_ERROR;
    zmemzero(copy, আকার(কাঠামো inflate_state));
    window = Z_NULL;
    যদি (state->window != Z_NULL) {
        window = (unsigned অক্ষর FAR *)
                 ZALLOC(source, 1U << state->wbits, আকার(unsigned অক্ষর));
        যদি (window == Z_NULL) {
            ZFREE(source, copy);
            ফেরত Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, source, আকার(z_stream));
    zmemcpy(copy, state, আকার(কাঠামো inflate_state));
    copy->strm = dest;
    যদি (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->পরবর্তী = copy->codes + (state->পরবর্তী - state->codes);
    যদি (window != Z_NULL)
        zmemcpy(window, state->window, state->whave);
    copy->window = window;
    dest->state = (কাঠামো internal_state FAR *)copy;
    ফেরত Z_OK;
}

পূর্ণসংখ্যা ZEXPORT inflateUndermine(z_streamp strm, পূর্ণসংখ্যা subvert) {
    কাঠামো inflate_state FAR *state;

    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
    state->sane = !subvert;
    ফেরত Z_OK;
#অন্যথা
    (শূন্য)subvert;
    state->sane = 1;
    ফেরত Z_তথ্য_ERROR;
#endif
}

পূর্ণসংখ্যা ZEXPORT inflateValidate(z_streamp strm, পূর্ণসংখ্যা check) {
    কাঠামো inflate_state FAR *state;

    যদি (inflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    state = (কাঠামো inflate_state FAR *)strm->state;
    যদি (check && state->wrap)
        state->wrap |= 4;
    অন্যথা
        state->wrap &= ~4;
    ফেরত Z_OK;
}

long ZEXPORT inflateMark(z_streamp strm) {
    কাঠামো inflate_state FAR *state;

    যদি (inflateStateCheck(strm))
        ফেরত -(1L << 16);
    state = (কাঠামো inflate_state FAR *)strm->state;
    ফেরত (long)(((unsigned long)((long)state->back)) << 16) +
        (state->mode == COPY ? state->দৈর্ঘ্য :
            (state->mode == MATCH ? state->was - state->দৈর্ঘ্য : 0));
}

unsigned long ZEXPORT inflateCodesUsed(z_streamp strm) {
    কাঠামো inflate_state FAR *state;
    যদি (inflateStateCheck(strm)) ফেরত (unsigned long)-1;
    state = (কাঠামো inflate_state FAR *)strm->state;
    ফেরত (unsigned long)(state->পরবর্তী - state->codes);
}
