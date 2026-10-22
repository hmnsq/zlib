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

#ضمن "zutil.h"
#ضمن "inftrees.h"
#ضمن "inflate.h"
#ضمن "inffast.h"

local صحيح inflateStateCheck(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        أرجع 1;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state == Z_NULL || state->strm != strm ||
        state->گهڻائي < سر_2 || state->گهڻائي > SYNC)
        أرجع 1;
    أرجع 0;
}

صحيح ZEXPORT inflateResetKeep(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    strm->ڪل_اندريون = strm->ڪل_ٻاهريون = state->ڪل = 0;
    strm->نياپو = Z_NULL;
    strm->معلومات_type = 0;
    إذا (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->گهڻائي = سر_2;
    state->آخري = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->سر = Z_NULL;
    state->hold = 0;
    state->بٽ = 0;
    state->lencode = state->distcode = state->ايندڙ = state->codes;
    state->sane = 1;
    state->پٺ = -1;
    Tracev((stderr, "inflate: reset\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateReset(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    أرجع inflateResetKeep(strm);
}

صحيح ZEXPORT inflateReset2(z_streamp strm, صحيح دري_بٽ) {
    صحيح wrap;
    بنية inflate_state FAR *state;

    /* get the state */
    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    إذا (دري_بٽ < 0) {
        إذا (دري_بٽ < -15)
            أرجع Z_وهڪرو_ERROR;
        wrap = 0;
        دري_بٽ = -دري_بٽ;
    }
    وإلا {
        wrap = (دري_بٽ >> 4) + 5;
#جيڪڏهن_وصف_ٿيل GUNZIP
        إذا (دري_بٽ < 48)
            دري_بٽ &= 15;
#جيڪڏهن_پورو
    }

    /* set number of window bits, free window if different */
    إذا (دري_بٽ && (دري_بٽ < 8 || دري_بٽ > 15))
        أرجع Z_وهڪرو_ERROR;
    إذا (state->دري != Z_NULL && state->wbits != (بنانشان)دري_بٽ) {
        ZFREE(strm, state->دري);
        state->دري = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (بنانشان)دري_بٽ;
    أرجع inflateReset(strm);
}

صحيح ZEXPORT inflateInit2_(z_streamp strm, صحيح دري_بٽ,
                          ثابت حرف *نسخو, صحيح وهڪرو_سائيز) {
    صحيح ret;
    بنية inflate_state FAR *state;

    إذا (نسخو == Z_NULL || نسخو[0] != ZLIB_نسخو[0] ||
        وهڪرو_سائيز != (صحيح)(حجم(z_وهڪرو)))
        أرجع Z_نسخو_ERROR;
    إذا (strm == Z_NULL) أرجع Z_وهڪرو_ERROR;
    strm->نياپو = Z_NULL;                 /* in case we return an error */
    إذا (strm->zalloc == (alloc_func)0) {
#جيڪڏهن_وصف_ٿيل Z_SOLO
        أرجع Z_وهڪرو_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#جيڪڏهن_پورو
    }
    إذا (strm->zfree == (free_func)0)
#جيڪڏهن_وصف_ٿيل Z_SOLO
        أرجع Z_وهڪرو_ERROR;
#وإلا
        strm->zfree = zcfree;
#جيڪڏهن_پورو
    state = (بنية inflate_state FAR *)
            ZALLOC(strm, 1, حجم(بنية inflate_state));
    إذا (state == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(state, حجم(بنية inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (بنية اندروني_state FAR *)state;
    state->strm = strm;
    state->دري = Z_NULL;
    state->گهڻائي = سر_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, دري_بٽ);
    إذا (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    أرجع ret;
}

صحيح ZEXPORT inflateInit_(z_streamp strm, ثابت حرف *نسخو,
                         صحيح وهڪرو_سائيز) {
    أرجع inflateInit2_(strm, DEF_WBITS, نسخو, وهڪرو_سائيز);
}

صحيح ZEXPORT inflate_مفرد(z_streamp strm, صحيح بٽ, صحيح قدر) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    إذا (بٽ == 0)
        أرجع Z_OK;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (بٽ < 0) {
        state->hold = 0;
        state->بٽ = 0;
        أرجع Z_OK;
    }
    إذا (بٽ > 16 || state->بٽ + (uInt)بٽ > 32) أرجع Z_وهڪرو_ERROR;
    قدر &= (1L << بٽ) - 1;
    state->hold += (بنانشان ڊگهو)قدر << state->بٽ;
    state->بٽ += (uInt)بٽ;
    أرجع Z_OK;
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
local صحيح updatewindow(z_streamp strm, ثابت Bytef *پڇاڙي, بنانشان copy) {
    بنية inflate_state FAR *state;
    بنانشان dist;

    state = (بنية inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    إذا (state->دري == Z_NULL) {
        state->دري = (بنانشان حرف FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               حجم(بنانشان حرف));
        إذا (state->دري == Z_NULL) أرجع 1;
    }

    /* if window not in use yet, initialize */
    إذا (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    إذا (copy >= state->wsize) {
        zmemcpy(state->دري, پڇاڙي - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    وإلا {
        dist = state->wsize - state->wnext;
        إذا (dist > copy) dist = copy;
        zmemcpy(state->دري + state->wnext, پڇاڙي - copy, dist);
        copy -= dist;
        إذا (copy) {
            zmemcpy(state->دري, پڇاڙي - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        وإلا {
            state->wnext += dist;
            إذا (state->wnext == state->wsize) state->wnext = 0;
            إذا (state->whave < state->wsize) state->whave += dist;
        }
    }
    أرجع 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#جيڪڏهن_وصف_ٿيل GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? چڪري_جاچ_32_2(check, buf, len) : adler32(check, buf, len))
#وإلا
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#جيڪڏهن_پورو

/* check macros for header crc */
#جيڪڏهن_وصف_ٿيل GUNZIP
#  define CRC2(check, لفظ) \
    افعل { \
        hbuf[0] = (بنانشان حرف)(لفظ); \
        hbuf[1] = (بنانشان حرف)((لفظ) >> 8); \
        check = چڪري_جاچ_32_2(check, hbuf, 2); \
    } طالما (0)

#  define CRC4(check, لفظ) \
    افعل { \
        hbuf[0] = (بنانشان حرف)(لفظ); \
        hbuf[1] = (بنانشان حرف)((لفظ) >> 8); \
        hbuf[2] = (بنانشان حرف)((لفظ) >> 16); \
        hbuf[3] = (بنانشان حرف)((لفظ) >> 24); \
        check = چڪري_جاچ_32_2(check, hbuf, 4); \
    } طالما (0)
#جيڪڏهن_پورو

/* Load registers with state in inflate() for speed */
#وصف_ڪريو ڀريو_2() \
    افعل { \
        put = strm->ايندڙ_ٻاهريون; \
        کاٻو = strm->avail_ٻاهريون; \
        ايندڙ = strm->ايندڙ_اندريون; \
        have = strm->avail_اندريون; \
        hold = state->hold; \
        بٽ = state->بٽ; \
    } طالما (0)

/* Restore state from registers in inflate() */
#وصف_ڪريو RESTORE() \
    افعل { \
        strm->ايندڙ_ٻاهريون = put; \
        strm->avail_ٻاهريون = کاٻو; \
        strm->ايندڙ_اندريون = ايندڙ; \
        strm->avail_اندريون = have; \
        state->hold = hold; \
        state->بٽ = بٽ; \
    } طالما (0)

/* Clear the input bit accumulator */
#وصف_ڪريو INITBITS() \
    افعل { \
        hold = 0; \
        بٽ = 0; \
    } طالما (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#وصف_ڪريو PULLBYTE() \
    افعل { \
        إذا (have == 0) اذهب inf_leave; \
        have--; \
        hold += (بنانشان ڊگهو)(*ايندڙ++) << بٽ; \
        بٽ += 8; \
    } طالما (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#وصف_ڪريو NEEDBITS(n) \
    افعل { \
        طالما (بٽ < (بنانشان)(n)) \
            PULLBYTE(); \
    } طالما (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#وصف_ڪريو بٽ_2(n) \
    ((بنانشان)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#وصف_ڪريو DROPBITS(n) \
    افعل { \
        hold >>= (n); \
        بٽ -= (بنانشان)(n); \
    } طالما (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#وصف_ڪريو BYTEBITS() \
    افعل { \
        hold >>= بٽ & 7; \
        بٽ -= بٽ & 7; \
    } طالما (0)

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

صحيح ZEXPORT inflate(z_streamp strm, صحيح flush) {
    بنية inflate_state FAR *state;
    z_const بنانشان حرف FAR *ايندڙ;    /* next input */
    بنانشان حرف FAR *put;     /* next output */
    بنانشان have, کاٻو;        /* available input and output */
    بنانشان ڊگهو hold;         /* bit buffer */
    بنانشان بٽ;              /* bits in bit buffer */
    بنانشان اندريون, ٻاهريون;           /* save starting available input and output */
    بنانشان copy;              /* number of stored or match bytes to copy */
    بنانشان حرف FAR *from;    /* where to copy match bytes from */
    ڪوڊ هتي;                  /* current decoding table entry */
    ڪوڊ آخري;                  /* parent table entry */
    بنانشان len;               /* length to copy for repeats, bits to drop */
    صحيح ret;                    /* return code */
#جيڪڏهن_وصف_ٿيل GUNZIP
    بنانشان حرف hbuf[4];      /* buffer for gzip header crc calculation */
#جيڪڏهن_پورو
    ساكن ثابت بنانشان ننڍو ترتيب[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    إذا (inflateStateCheck(strm) || strm->ايندڙ_ٻاهريون == Z_NULL ||
        (strm->ايندڙ_اندريون == Z_NULL && strm->avail_اندريون != 0))
        أرجع Z_وهڪرو_ERROR;

    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->گهڻائي == TYPE) state->گهڻائي = TYPEDO;      /* skip check */
    ڀريو_2();
    اندريون = have;
    ٻاهريون = کاٻو;
    ret = Z_OK;
    لكل (;;)
        بدل (state->گهڻائي) {
        حالة سر_2:
            إذا (state->wrap == 0) {
                state->گهڻائي = TYPEDO;
                اكسر;
            }
            NEEDBITS(16);
#جيڪڏهن_وصف_ٿيل GUNZIP
            إذا ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                إذا (state->wbits == 0)
                    state->wbits = 15;
                state->check = چڪري_جاچ_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->گهڻائي = FLAGS;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->پورو_2 = -1;
            إذا (!(state->wrap & 1) ||   /* check if zlib header allowed */
#وإلا
            إذا (
#جيڪڏهن_پورو
                ((بٽ_2(8) << 8) + (hold >> 8)) % 31) {
                strm->نياپو = (z_const حرف *)"incorrect header check";
                state->گهڻائي = BAD;
                اكسر;
            }
            إذا (بٽ_2(4) != Z_DEFLATED) {
                strm->نياپو = (z_const حرف *)"unknown compression method";
                state->گهڻائي = BAD;
                اكسر;
            }
            DROPBITS(4);
            len = بٽ_2(4) + 8;
            إذا (state->wbits == 0)
                state->wbits = len;
            إذا (len > 15 || len > state->wbits) {
                strm->نياپو = (z_const حرف *)"invalid window size";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->گهڻائي = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            اكسر;
#جيڪڏهن_وصف_ٿيل GUNZIP
        حالة FLAGS:
            NEEDBITS(16);
            state->flags = (صحيح)(hold);
            إذا ((state->flags & 0xff) != Z_DEFLATED) {
                strm->نياپو = (z_const حرف *)"unknown compression method";
                state->گهڻائي = BAD;
                اكسر;
            }
            إذا (state->flags & 0xe000) {
                strm->نياپو = (z_const حرف *)"unknown header flags set";
                state->گهڻائي = BAD;
                اكسر;
            }
            إذا (state->سر != Z_NULL)
                state->سر->متن = (صحيح)((hold >> 8) & 1);
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->گهڻائي = وقت;
                /* fallthrough */
        حالة وقت:
            NEEDBITS(32);
            إذا (state->سر != Z_NULL)
                state->سر->time = hold;
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->گهڻائي = OS;
                /* fallthrough */
        حالة OS:
            NEEDBITS(16);
            إذا (state->سر != Z_NULL) {
                state->سر->xflags = (صحيح)(hold & 0xff);
                state->سر->os = (صحيح)(hold >> 8);
            }
            إذا ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->گهڻائي = EXLEN;
                /* fallthrough */
        حالة EXLEN:
            إذا (state->flags & 0x0400) {
                NEEDBITS(16);
                state->ڊيگهائي = (بنانشان)(hold);
                إذا (state->سر != Z_NULL)
                    state->سر->extra_len = (بنانشان)hold;
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->extra = Z_NULL;
            state->گهڻائي = EXTRA;
                /* fallthrough */
        حالة EXTRA:
            إذا (state->flags & 0x0400) {
                copy = state->ڊيگهائي;
                إذا (copy > have) copy = have;
                إذا (copy) {
                    إذا (state->سر != Z_NULL &&
                        state->سر->extra != Z_NULL &&
                        (len = state->سر->extra_len - state->ڊيگهائي) <
                            state->سر->extra_max) {
                        zmemcpy(state->سر->extra + len, ايندڙ,
                                len + copy > state->سر->extra_max ?
                                state->سر->extra_max - len : copy);
                    }
                    إذا ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = چڪري_جاچ_32_2(state->check, ايندڙ, copy);
                    have -= copy;
                    ايندڙ += copy;
                    state->ڊيگهائي -= copy;
                }
                إذا (state->ڊيگهائي) اذهب inf_leave;
            }
            state->ڊيگهائي = 0;
            state->گهڻائي = NAME;
                /* fallthrough */
        حالة NAME:
            إذا (state->flags & 0x0800) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بنانشان)(ايندڙ[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->name != Z_NULL &&
                            state->ڊيگهائي < state->سر->name_max)
                        state->سر->name[state->ڊيگهائي++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = چڪري_جاچ_32_2(state->check, ايندڙ, copy);
                have -= copy;
                ايندڙ += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->name = Z_NULL;
            state->ڊيگهائي = 0;
            state->گهڻائي = COMMENT;
                /* fallthrough */
        حالة COMMENT:
            إذا (state->flags & 0x1000) {
                إذا (have == 0) اذهب inf_leave;
                copy = 0;
                افعل {
                    len = (بنانشان)(ايندڙ[copy++]);
                    إذا (state->سر != Z_NULL &&
                            state->سر->comment != Z_NULL &&
                            state->ڊيگهائي < state->سر->comm_max)
                        state->سر->comment[state->ڊيگهائي++] = (Bytef)len;
                } طالما (len && copy < have);
                إذا ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = چڪري_جاچ_32_2(state->check, ايندڙ, copy);
                have -= copy;
                ايندڙ += copy;
                إذا (len) اذهب inf_leave;
            }
            وإلا إذا (state->سر != Z_NULL)
                state->سر->comment = Z_NULL;
            state->گهڻائي = HCRC;
                /* fallthrough */
        حالة HCRC:
            إذا (state->flags & 0x0200) {
                NEEDBITS(16);
                إذا ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->نياپو = (z_const حرف *)"header crc mismatch";
                    state->گهڻائي = BAD;
                    اكسر;
                }
                INITBITS();
            }
            إذا (state->سر != Z_NULL) {
                state->سر->hcrc = (صحيح)((state->flags >> 9) & 1);
                state->سر->پورو_2 = 1;
            }
            strm->adler = state->check = چڪري_جاچ_32_2(0L, Z_NULL, 0);
            state->گهڻائي = TYPE;
            اكسر;
#جيڪڏهن_پورو
        حالة DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->گهڻائي = DICT;
                /* fallthrough */
        حالة DICT:
            إذا (state->havedict == 0) {
                RESTORE();
                أرجع Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->گهڻائي = TYPE;
                /* fallthrough */
        حالة TYPE:
            إذا (flush == Z_بلاڪ || flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة TYPEDO:
            إذا (state->آخري) {
                BYTEBITS();
                state->گهڻائي = CHECK;
                اكسر;
            }
            NEEDBITS(3);
            state->آخري = بٽ_2(1);
            DROPBITS(1);
            بدل (بٽ_2(2)) {
            حالة 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->آخري ? " (last)" : ""));
                state->گهڻائي = STORED;
                اكسر;
            حالة 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->آخري ? " (last)" : ""));
                state->گهڻائي = LEN_;             /* decode codes */
                إذا (flush == Z_TREES) {
                    DROPBITS(2);
                    اذهب inf_leave;
                }
                اكسر;
            حالة 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->آخري ? " (last)" : ""));
                state->گهڻائي = جدول;
                اكسر;
            افتراضي:
                strm->نياپو = (z_const حرف *)"invalid block type";
                state->گهڻائي = BAD;
            }
            DROPBITS(2);
            اكسر;
        حالة STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            إذا ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->نياپو = (z_const حرف *)"invalid stored block lengths";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->ڊيگهائي = (بنانشان)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->ڊيگهائي));
            INITBITS();
            state->گهڻائي = COPY_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة COPY_:
            state->گهڻائي = COPY;
                /* fallthrough */
        حالة COPY:
            copy = state->ڊيگهائي;
            إذا (copy) {
                إذا (copy > have) copy = have;
                إذا (copy > کاٻو) copy = کاٻو;
                إذا (copy == 0) اذهب inf_leave;
                zmemcpy(put, ايندڙ, copy);
                have -= copy;
                ايندڙ += copy;
                کاٻو -= copy;
                put += copy;
                state->ڊيگهائي -= copy;
                اكسر;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->گهڻائي = TYPE;
            اكسر;
        حالة جدول:
            NEEDBITS(14);
            state->nlen = بٽ_2(5) + 257;
            DROPBITS(5);
            state->ndist = بٽ_2(5) + 1;
            DROPBITS(5);
            state->ncode = بٽ_2(4) + 4;
            DROPBITS(4);
#جيڪڏهن_وصف_نه_ٿيل PKZIP_BUG_WORKAROUND
            إذا (state->nlen > 286 || state->ndist > 30) {
                strm->نياپو = (z_const حرف *)
                    "too many length or distance symbols";
                state->گهڻائي = BAD;
                اكسر;
            }
#جيڪڏهن_پورو
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->گهڻائي = LENLENS;
                /* fallthrough */
        حالة LENLENS:
            طالما (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[ترتيب[state->have++]] = (بنانشان ننڍو)بٽ_2(3);
                DROPBITS(3);
            }
            طالما (state->have < 19)
                state->lens[ترتيب[state->have++]] = 0;
            state->ايندڙ = state->codes;
            state->lencode = state->distcode = (ثابت ڪوڊ FAR *)(state->ايندڙ);
            state->lenbits = 7;
            ret = inflate_جدول(CODES, state->lens, 19, &(state->ايندڙ),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid code lengths set";
                state->گهڻائي = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->گهڻائي = CODELENS;
                /* fallthrough */
        حالة CODELENS:
            طالما (state->have < state->nlen + state->ndist) {
                لكل (;;) {
                    هتي = state->lencode[بٽ_2(state->lenbits)];
                    إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                إذا (هتي.val < 16) {
                    DROPBITS(هتي.بٽ);
                    state->lens[state->have++] = هتي.val;
                }
                وإلا {
                    إذا (هتي.val == 16) {
                        NEEDBITS(هتي.بٽ + 2);
                        DROPBITS(هتي.بٽ);
                        إذا (state->have == 0) {
                            strm->نياپو = (z_const حرف *)
                                "invalid bit length repeat";
                            state->گهڻائي = BAD;
                            اكسر;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + بٽ_2(2);
                        DROPBITS(2);
                    }
                    وإلا إذا (هتي.val == 17) {
                        NEEDBITS(هتي.بٽ + 3);
                        DROPBITS(هتي.بٽ);
                        len = 0;
                        copy = 3 + بٽ_2(3);
                        DROPBITS(3);
                    }
                    وإلا {
                        NEEDBITS(هتي.بٽ + 7);
                        DROPBITS(هتي.بٽ);
                        len = 0;
                        copy = 11 + بٽ_2(7);
                        DROPBITS(7);
                    }
                    إذا (state->have + copy > state->nlen + state->ndist) {
                        strm->نياپو = (z_const حرف *)
                            "invalid bit length repeat";
                        state->گهڻائي = BAD;
                        اكسر;
                    }
                    طالما (copy--)
                        state->lens[state->have++] = (بنانشان ننڍو)len;
                }
            }

            /* handle error breaks in while */
            إذا (state->گهڻائي == BAD) اكسر;

            /* check for end-of-block code (better have one) */
            إذا (state->lens[256] == 0) {
                strm->نياپو = (z_const حرف *)
                    "invalid code -- missing end-of-block";
                state->گهڻائي = BAD;
                اكسر;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->ايندڙ = state->codes;
            state->lencode = (ثابت ڪوڊ FAR *)(state->ايندڙ);
            state->lenbits = 9;
            ret = inflate_جدول(LENS, state->lens, state->nlen, &(state->ايندڙ),
                                &(state->lenbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid literal/lengths set";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->distcode = (ثابت ڪوڊ FAR *)(state->ايندڙ);
            state->distbits = 6;
            ret = inflate_جدول(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->ايندڙ), &(state->distbits), state->work);
            إذا (ret) {
                strm->نياپو = (z_const حرف *)"invalid distances set";
                state->گهڻائي = BAD;
                اكسر;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->گهڻائي = LEN_;
            إذا (flush == Z_TREES) اذهب inf_leave;
                /* fallthrough */
        حالة LEN_:
            state->گهڻائي = LEN;
                /* fallthrough */
        حالة LEN:
            إذا (have >= 6 && کاٻو >= 258) {
                RESTORE();
                inflate_fast(strm, ٻاهريون);
                ڀريو_2();
                إذا (state->گهڻائي == TYPE)
                    state->پٺ = -1;
                اكسر;
            }
            state->پٺ = 0;
            لكل (;;) {
                هتي = state->lencode[بٽ_2(state->lenbits)];
                إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                PULLBYTE();
            }
            إذا (هتي.op && (هتي.op & 0xf0) == 0) {
                آخري = هتي;
                لكل (;;) {
                    هتي = state->lencode[آخري.val +
                            (بٽ_2(آخري.بٽ + آخري.op) >> آخري.بٽ)];
                    إذا ((بنانشان)(آخري.بٽ + هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخري.بٽ);
                state->پٺ += آخري.بٽ;
            }
            DROPBITS(هتي.بٽ);
            state->پٺ += هتي.بٽ;
            state->ڊيگهائي = (بنانشان)هتي.val;
            إذا ((صحيح)(هتي.op) == 0) {
                Tracevv((stderr, هتي.val >= 0x20 && هتي.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", هتي.val));
                state->گهڻائي = LIT;
                اكسر;
            }
            إذا (هتي.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->پٺ = -1;
                state->گهڻائي = TYPE;
                اكسر;
            }
            إذا (هتي.op & 64) {
                strm->نياپو = (z_const حرف *)"invalid literal/length code";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->extra = (بنانشان)(هتي.op) & 15;
            state->گهڻائي = LENEXT;
                /* fallthrough */
        حالة LENEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->ڊيگهائي += بٽ_2(state->extra);
                DROPBITS(state->extra);
                state->پٺ += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->ڊيگهائي));
            state->was = state->ڊيگهائي;
            state->گهڻائي = DIST;
                /* fallthrough */
        حالة DIST:
            لكل (;;) {
                هتي = state->distcode[بٽ_2(state->distbits)];
                إذا ((بنانشان)(هتي.بٽ) <= بٽ) اكسر;
                PULLBYTE();
            }
            إذا ((هتي.op & 0xf0) == 0) {
                آخري = هتي;
                لكل (;;) {
                    هتي = state->distcode[آخري.val +
                            (بٽ_2(آخري.بٽ + آخري.op) >> آخري.بٽ)];
                    إذا ((بنانشان)(آخري.بٽ + هتي.بٽ) <= بٽ) اكسر;
                    PULLBYTE();
                }
                DROPBITS(آخري.بٽ);
                state->پٺ += آخري.بٽ;
            }
            DROPBITS(هتي.بٽ);
            state->پٺ += هتي.بٽ;
            إذا (هتي.op & 64) {
                strm->نياپو = (z_const حرف *)"invalid distance code";
                state->گهڻائي = BAD;
                اكسر;
            }
            state->وٿي = (بنانشان)هتي.val;
            state->extra = (بنانشان)(هتي.op) & 15;
            state->گهڻائي = DISTEXT;
                /* fallthrough */
        حالة DISTEXT:
            إذا (state->extra) {
                NEEDBITS(state->extra);
                state->وٿي += بٽ_2(state->extra);
                DROPBITS(state->extra);
                state->پٺ += state->extra;
            }
#جيڪڏهن_وصف_ٿيل INFLATE_STRICT
            إذا (state->وٿي > state->dmax) {
                strm->نياپو = (z_const حرف *)"invalid distance too far back";
                state->گهڻائي = BAD;
                اكسر;
            }
#جيڪڏهن_پورو
            Tracevv((stderr, "inflate:         distance %u\n", state->وٿي));
            state->گهڻائي = ميلاپ_2;
                /* fallthrough */
        حالة ميلاپ_2:
            إذا (کاٻو == 0) اذهب inf_leave;
            copy = ٻاهريون - کاٻو;
            إذا (state->وٿي > copy) {         /* copy from window */
                copy = state->وٿي - copy;
                إذا (copy > state->whave) {
                    إذا (state->sane) {
                        strm->نياپو = (z_const حرف *)
                            "invalid distance too far back";
                        state->گهڻائي = BAD;
                        اكسر;
                    }
#جيڪڏهن_وصف_ٿيل INFLATE_ALLOW_INVALID_مفاصلو_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    إذا (copy > state->ڊيگهائي) copy = state->ڊيگهائي;
                    إذا (copy > کاٻو) copy = کاٻو;
                    کاٻو -= copy;
                    state->ڊيگهائي -= copy;
                    افعل {
                        *put++ = 0;
                    } طالما (--copy);
                    إذا (state->ڊيگهائي == 0) state->گهڻائي = LEN;
                    اكسر;
#جيڪڏهن_پورو
                }
                إذا (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->دري + (state->wsize - copy);
                }
                وإلا
                    from = state->دري + (state->wnext - copy);
                إذا (copy > state->ڊيگهائي) copy = state->ڊيگهائي;
            }
            وإلا {                              /* copy from output */
                from = put - state->وٿي;
                copy = state->ڊيگهائي;
            }
            إذا (copy > کاٻو) copy = کاٻو;
            کاٻو -= copy;
            state->ڊيگهائي -= copy;
            افعل {
                *put++ = *from++;
            } طالما (--copy);
            إذا (state->ڊيگهائي == 0) state->گهڻائي = LEN;
            اكسر;
        حالة LIT:
            إذا (کاٻو == 0) اذهب inf_leave;
            *put++ = (بنانشان حرف)(state->ڊيگهائي);
            کاٻو--;
            state->گهڻائي = LEN;
            اكسر;
        حالة CHECK:
            إذا (state->wrap) {
                NEEDBITS(32);
                ٻاهريون -= کاٻو;
                strm->ڪل_ٻاهريون += ٻاهريون;
                state->ڪل += ٻاهريون;
                إذا ((state->wrap & 4) && ٻاهريون)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - ٻاهريون, ٻاهريون);
                ٻاهريون = کاٻو;
                إذا ((state->wrap & 4) && (
#جيڪڏهن_وصف_ٿيل GUNZIP
                     state->flags ? hold :
#جيڪڏهن_پورو
                     ZSWAP32(hold)) != state->check) {
                    strm->نياپو = (z_const حرف *)"incorrect data check";
                    state->گهڻائي = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#جيڪڏهن_وصف_ٿيل GUNZIP
            state->گهڻائي = ڊيگهائي_2;
                /* fallthrough */
        حالة ڊيگهائي_2:
            إذا (state->wrap && state->flags) {
                NEEDBITS(32);
                إذا ((state->wrap & 4) && hold != (state->ڪل & 0xffffffff)) {
                    strm->نياپو = (z_const حرف *)"incorrect length check";
                    state->گهڻائي = BAD;
                    اكسر;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#جيڪڏهن_پورو
            state->گهڻائي = پورو;
                /* fallthrough */
        حالة پورو:
            ret = Z_وهڪرو_پڇاڙي;
            اذهب inf_leave;
        حالة BAD:
            ret = Z_معلومات_ERROR;
            اذهب inf_leave;
        حالة MEM:
            أرجع Z_MEM_ERROR;
        حالة SYNC:
                /* fallthrough */
        افتراضي:
            أرجع Z_وهڪرو_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    إذا (state->wsize || (ٻاهريون != strm->avail_ٻاهريون && state->گهڻائي < BAD &&
            (state->گهڻائي < CHECK || flush != Z_پڄاڻي)))
        إذا (updatewindow(strm, strm->ايندڙ_ٻاهريون, ٻاهريون - strm->avail_ٻاهريون)) {
            state->گهڻائي = MEM;
            أرجع Z_MEM_ERROR;
        }
    اندريون -= strm->avail_اندريون;
    ٻاهريون -= strm->avail_ٻاهريون;
    strm->ڪل_اندريون += اندريون;
    strm->ڪل_ٻاهريون += ٻاهريون;
    state->ڪل += ٻاهريون;
    إذا ((state->wrap & 4) && ٻاهريون)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->ايندڙ_ٻاهريون - ٻاهريون, ٻاهريون);
    strm->معلومات_type = (صحيح)state->بٽ + (state->آخري ? 64 : 0) +
                      (state->گهڻائي == TYPE ? 128 : 0) +
                      (state->گهڻائي == LEN_ || state->گهڻائي == COPY_ ? 256 : 0);
    إذا (((اندريون == 0 && ٻاهريون == 0) || flush == Z_پڄاڻي) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    أرجع ret;
}

صحيح ZEXPORT inflate_پڇاڙي(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm))
        أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->دري != Z_NULL) ZFREE(strm, state->دري);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_ڊيگهائي) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;

    /* copy dictionary */
    إذا (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->دري + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->دري, state->wnext);
    }
    إذا (dict_ڊيگهائي != Z_NULL)
        *dict_ڊيگهائي = state->whave;
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_سيٽ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt dict_ڊيگهائي) {
    بنية inflate_state FAR *state;
    بنانشان ڊگهو dictid;
    صحيح ret;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (state->wrap != 0 && state->گهڻائي != DICT)
        أرجع Z_وهڪرو_ERROR;

    /* check for correct dictionary identifier */
    إذا (state->گهڻائي == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_ڊيگهائي);
        إذا (dictid != state->check)
            أرجع Z_معلومات_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_ڊيگهائي, dict_ڊيگهائي);
    إذا (ret) {
        state->گهڻائي = MEM;
        أرجع Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    أرجع Z_OK;
}

صحيح ZEXPORT inflate_Get_سرو(z_streamp strm, gz_headerp سر) {
    بنية inflate_state FAR *state;

    /* check state */
    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا ((state->wrap & 2) == 0) أرجع Z_وهڪرو_ERROR;

    /* save header structure */
    state->سر = سر;
    سر->پورو_2 = 0;
    أرجع Z_OK;
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
local بنانشان syncsearch(بنانشان FAR *have, ثابت بنانشان حرف FAR *buf,
                          بنانشان len) {
    بنانشان got;
    بنانشان ايندڙ;

    got = *have;
    ايندڙ = 0;
    طالما (ايندڙ < len && got < 4) {
        إذا ((صحيح)(buf[ايندڙ]) == (got < 2 ? 0 : 0xff))
            got++;
        وإلا إذا (buf[ايندڙ])
            got = 0;
        وإلا
            got = 4 - got;
        ايندڙ++;
    }
    *have = got;
    أرجع ايندڙ;
}

صحيح ZEXPORT inflateSync(z_streamp strm) {
    بنانشان len;               /* number of bytes to look at or looked at */
    صحيح flags;                  /* temporary to save header status */
    بنانشان ڊگهو اندريون, ٻاهريون;      /* temporary to save total_in and total_out */
    بنانشان حرف buf[4];       /* to restore bit buffer to byte string */
    بنية inflate_state FAR *state;

    /* check parameters */
    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (strm->avail_اندريون == 0 && state->بٽ < 8) أرجع Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    إذا (state->گهڻائي != SYNC) {
        state->گهڻائي = SYNC;
        state->hold >>= state->بٽ & 7;
        state->بٽ -= state->بٽ & 7;
        len = 0;
        طالما (state->بٽ >= 8) {
            buf[len++] = (بنانشان حرف)(state->hold);
            state->hold >>= 8;
            state->بٽ -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->ايندڙ_اندريون, strm->avail_اندريون);
    strm->avail_اندريون -= len;
    strm->ايندڙ_اندريون += len;
    strm->ڪل_اندريون += len;

    /* return no joy or set up to restart inflate() on a new block */
    إذا (state->have != 4) أرجع Z_معلومات_ERROR;
    إذا (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    وإلا
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    اندريون = strm->ڪل_اندريون;  ٻاهريون = strm->ڪل_ٻاهريون;
    inflateReset(strm);
    strm->ڪل_اندريون = اندريون;  strm->ڪل_ٻاهريون = ٻاهريون;
    state->flags = flags;
    state->گهڻائي = TYPE;
    أرجع Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
صحيح ZEXPORT inflate_Sync_نقطو(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع state->گهڻائي == STORED && state->بٽ == 0;
}

صحيح ZEXPORT inflateCopy(z_streamp dest, z_streamp ذريعو) {
    بنية inflate_state FAR *state;
    بنية inflate_state FAR *copy;
    بنانشان حرف FAR *دري;

    /* check input */
    إذا (inflateStateCheck(ذريعو) || dest == Z_NULL)
        أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)ذريعو->state;

    /* allocate space */
    copy = (بنية inflate_state FAR *)
           ZALLOC(ذريعو, 1, حجم(بنية inflate_state));
    إذا (copy == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(copy, حجم(بنية inflate_state));
    دري = Z_NULL;
    إذا (state->دري != Z_NULL) {
        دري = (بنانشان حرف FAR *)
                 ZALLOC(ذريعو, 1U << state->wbits, حجم(بنانشان حرف));
        إذا (دري == Z_NULL) {
            ZFREE(ذريعو, copy);
            أرجع Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, ذريعو, حجم(z_وهڪرو));
    zmemcpy(copy, state, حجم(بنية inflate_state));
    copy->strm = dest;
    إذا (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->ايندڙ = copy->codes + (state->ايندڙ - state->codes);
    إذا (دري != Z_NULL)
        zmemcpy(دري, state->دري, state->whave);
    copy->دري = دري;
    dest->state = (بنية اندروني_state FAR *)copy;
    أرجع Z_OK;
}

صحيح ZEXPORT inflateUndermine(z_streamp strm, صحيح subvert) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
#جيڪڏهن_وصف_ٿيل INFLATE_ALLOW_INVALID_مفاصلو_TOOFAR_ARRR
    state->sane = !subvert;
    أرجع Z_OK;
#وإلا
    (فراغ)subvert;
    state->sane = 1;
    أرجع Z_معلومات_ERROR;
#جيڪڏهن_پورو
}

صحيح ZEXPORT inflateValidate(z_streamp strm, صحيح check) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    state = (بنية inflate_state FAR *)strm->state;
    إذا (check && state->wrap)
        state->wrap |= 4;
    وإلا
        state->wrap &= ~4;
    أرجع Z_OK;
}

ڊگهو ZEXPORT inflateMark(z_streamp strm) {
    بنية inflate_state FAR *state;

    إذا (inflateStateCheck(strm))
        أرجع -(1L << 16);
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (ڊگهو)(((بنانشان ڊگهو)((ڊگهو)state->پٺ)) << 16) +
        (state->گهڻائي == COPY ? state->ڊيگهائي :
            (state->گهڻائي == ميلاپ_2 ? state->was - state->ڊيگهائي : 0));
}

بنانشان ڊگهو ZEXPORT inflate_Codes_استعمال_ٿيل(z_streamp strm) {
    بنية inflate_state FAR *state;
    إذا (inflateStateCheck(strm)) أرجع (بنانشان ڊگهو)-1;
    state = (بنية inflate_state FAR *)strm->state;
    أرجع (بنانشان ڊگهو)(state->ايندڙ - state->codes);
}
