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
        state->बहुलकम् < शीर्ष_2 || state->बहुलकम् > SYNC)
        प्रत्यागम 1;
    प्रत्यागम 0;
}

पूर्णाङ्क ZEXPORT inflateResetKeep(z_streamp strm) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    strm->योग_अन्तर् = strm->योग_बहिर्गमनम् = state->योग_2 = 0;
    strm->सन्देशः = Z_NULL;
    strm->दत्त_type = 0;
    यदि (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->बहुलकम् = शीर्ष_2;
    state->चरमः = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->शीर्ष = Z_NULL;
    state->hold = 0;
    state->द्वयङ्काः = 0;
    state->lencode = state->distcode = state->अग्र = state->codes;
    state->sane = 1;
    state->पृष्ठभागः = -1;
    Tracev((stderr, "inflate: reset\n"));
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateReset(z_streamp strm) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    प्रत्यागम inflateResetKeep(strm);
}

पूर्णाङ्क ZEXPORT inflateReset2(z_streamp strm, पूर्णाङ्क गवाक्षः_द्वयङ्काः) {
    पूर्णाङ्क wrap;
    संरचना inflate_state FAR *state;

    /* get the state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    यदि (गवाक्षः_द्वयङ्काः < 0) {
        यदि (गवाक्षः_द्वयङ्काः < -15)
            प्रत्यागम Z_प्रवाहः_ERROR;
        wrap = 0;
        गवाक्षः_द्वयङ्काः = -गवाक्षः_द्वयङ्काः;
    }
    अन्यथा {
        wrap = (गवाक्षः_द्वयङ्काः >> 4) + 5;
#यदि_परिभाषितम् GUNZIP
        यदि (गवाक्षः_द्वयङ्काः < 48)
            गवाक्षः_द्वयङ्काः &= 15;
#यदि_अन्तः
    }

    /* set number of window bits, free window if different */
    यदि (गवाक्षः_द्वयङ्काः && (गवाक्षः_द्वयङ्काः < 8 || गवाक्षः_द्वयङ्काः > 15))
        प्रत्यागम Z_प्रवाहः_ERROR;
    यदि (state->गवाक्षः != Z_NULL && state->wbits != (अचिह्नित)गवाक्षः_द्वयङ्काः) {
        ZFREE(strm, state->गवाक्षः);
        state->गवाक्षः = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (अचिह्नित)गवाक्षः_द्वयङ्काः;
    प्रत्यागम inflateReset(strm);
}

पूर्णाङ्क ZEXPORT inflateInit2_(z_streamp strm, पूर्णाङ्क गवाक्षः_द्वयङ्काः,
                          ध्रुव वर्ण *संस्करणम्, पूर्णाङ्क प्रवाहः_आकार) {
    पूर्णाङ्क ret;
    संरचना inflate_state FAR *state;

    यदि (संस्करणम् == Z_NULL || संस्करणम्[0] != ZLIB_संस्करणम्[0] ||
        प्रवाहः_आकार != (पूर्णाङ्क)(परिमाण(z_प्रवाहः)))
        प्रत्यागम Z_संस्करणम्_ERROR;
    यदि (strm == Z_NULL) प्रत्यागम Z_प्रवाहः_ERROR;
    strm->सन्देशः = Z_NULL;                 /* in case we return an error */
    यदि (strm->zalloc == (alloc_func)0) {
#यदि_परिभाषितम् Z_SOLO
        प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#यदि_अन्तः
    }
    यदि (strm->zfree == (free_func)0)
#यदि_परिभाषितम् Z_SOLO
        प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#यदि_अन्तः
    state = (संरचना inflate_state FAR *)
            ZALLOC(strm, 1, परिमाण(संरचना inflate_state));
    यदि (state == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(state, परिमाण(संरचना inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (संरचना आन्तरिकम्_state FAR *)state;
    state->strm = strm;
    state->गवाक्षः = Z_NULL;
    state->बहुलकम् = शीर्ष_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, गवाक्षः_द्वयङ्काः);
    यदि (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    प्रत्यागम ret;
}

पूर्णाङ्क ZEXPORT inflateInit_(z_streamp strm, ध्रुव वर्ण *संस्करणम्,
                         पूर्णाङ्क प्रवाहः_आकार) {
    प्रत्यागम inflateInit2_(strm, DEF_WBITS, संस्करणम्, प्रवाहः_आकार);
}

पूर्णाङ्क ZEXPORT inflate_अविभाज्यः(z_streamp strm, पूर्णाङ्क द्वयङ्काः, पूर्णाङ्क मानम्) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    यदि (द्वयङ्काः == 0)
        प्रत्यागम Z_OK;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (द्वयङ्काः < 0) {
        state->hold = 0;
        state->द्वयङ्काः = 0;
        प्रत्यागम Z_OK;
    }
    यदि (द्वयङ्काः > 16 || state->द्वयङ्काः + (uInt)द्वयङ्काः > 32) प्रत्यागम Z_प्रवाहः_ERROR;
    मानम् &= (1L << द्वयङ्काः) - 1;
    state->hold += (अचिह्नित दीर्घ)मानम् << state->द्वयङ्काः;
    state->द्वयङ्काः += (uInt)द्वयङ्काः;
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
local पूर्णाङ्क updatewindow(z_streamp strm, ध्रुव Bytef *अन्तः_2, अचिह्नित copy) {
    संरचना inflate_state FAR *state;
    अचिह्नित dist;

    state = (संरचना inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    यदि (state->गवाक्षः == Z_NULL) {
        state->गवाक्षः = (अचिह्नित वर्ण FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               परिमाण(अचिह्नित वर्ण));
        यदि (state->गवाक्षः == Z_NULL) प्रत्यागम 1;
    }

    /* if window not in use yet, initialize */
    यदि (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    यदि (copy >= state->wsize) {
        zmemcpy(state->गवाक्षः, अन्तः_2 - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    अन्यथा {
        dist = state->wsize - state->wnext;
        यदि (dist > copy) dist = copy;
        zmemcpy(state->गवाक्षः + state->wnext, अन्तः_2 - copy, dist);
        copy -= dist;
        यदि (copy) {
            zmemcpy(state->गवाक्षः, अन्तः_2 - copy, copy);
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
#यदि_परिभाषितम् GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? चक्रीयपरीक्षा_32_2(check, buf, len) : adler32(check, buf, len))
#अन्यथा
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#यदि_अन्तः

/* check macros for header crc */
#यदि_परिभाषितम् GUNZIP
#  define CRC2(check, word) \
    कुरु { \
        hbuf[0] = (अचिह्नित वर्ण)(word); \
        hbuf[1] = (अचिह्नित वर्ण)((word) >> 8); \
        check = चक्रीयपरीक्षा_32_2(check, hbuf, 2); \
    } यावत् (0)

#  define CRC4(check, word) \
    कुरु { \
        hbuf[0] = (अचिह्नित वर्ण)(word); \
        hbuf[1] = (अचिह्नित वर्ण)((word) >> 8); \
        hbuf[2] = (अचिह्नित वर्ण)((word) >> 16); \
        hbuf[3] = (अचिह्नित वर्ण)((word) >> 24); \
        check = चक्रीयपरीक्षा_32_2(check, hbuf, 4); \
    } यावत् (0)
#यदि_अन्तः

/* Load registers with state in inflate() for speed */
#परिभाषय आरोपय_2() \
    कुरु { \
        put = strm->अग्र_बहिर्गमनम्; \
        वामः = strm->avail_बहिर्गमनम्; \
        अग्र = strm->अग्र_अन्तर्; \
        have = strm->avail_अन्तर्; \
        hold = state->hold; \
        द्वयङ्काः = state->द्वयङ्काः; \
    } यावत् (0)

/* Restore state from registers in inflate() */
#परिभाषय RESTORE() \
    कुरु { \
        strm->अग्र_बहिर्गमनम् = put; \
        strm->avail_बहिर्गमनम् = वामः; \
        strm->अग्र_अन्तर् = अग्र; \
        strm->avail_अन्तर् = have; \
        state->hold = hold; \
        state->द्वयङ्काः = द्वयङ्काः; \
    } यावत् (0)

/* Clear the input bit accumulator */
#परिभाषय INITBITS() \
    कुरु { \
        hold = 0; \
        द्वयङ्काः = 0; \
    } यावत् (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#परिभाषय PULLBYTE() \
    कुरु { \
        यदि (have == 0) गच्छ inf_leave; \
        have--; \
        hold += (अचिह्नित दीर्घ)(*अग्र++) << द्वयङ्काः; \
        द्वयङ्काः += 8; \
    } यावत् (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#परिभाषय NEEDBITS(n) \
    कुरु { \
        यावत् (द्वयङ्काः < (अचिह्नित)(n)) \
            PULLBYTE(); \
    } यावत् (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#परिभाषय द्वयङ्काः_2(n) \
    ((अचिह्नित)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#परिभाषय DROPBITS(n) \
    कुरु { \
        hold >>= (n); \
        द्वयङ्काः -= (अचिह्नित)(n); \
    } यावत् (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#परिभाषय BYTEBITS() \
    कुरु { \
        hold >>= द्वयङ्काः & 7; \
        द्वयङ्काः -= द्वयङ्काः & 7; \
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
    z_const अचिह्नित वर्ण FAR *अग्र;    /* next input */
    अचिह्नित वर्ण FAR *put;     /* next output */
    अचिह्नित have, वामः;        /* available input and output */
    अचिह्नित दीर्घ hold;         /* bit buffer */
    अचिह्नित द्वयङ्काः;              /* bits in bit buffer */
    अचिह्नित अन्तर्, बहिर्गमनम्;           /* save starting available input and output */
    अचिह्नित copy;              /* number of stored or match bytes to copy */
    अचिह्नित वर्ण FAR *from;    /* where to copy match bytes from */
    संहिता अत्र;                  /* current decoding table entry */
    संहिता चरमः;                  /* parent table entry */
    अचिह्नित len;               /* length to copy for repeats, bits to drop */
    पूर्णाङ्क ret;                    /* return code */
#यदि_परिभाषितम् GUNZIP
    अचिह्नित वर्ण hbuf[4];      /* buffer for gzip header crc calculation */
#यदि_अन्तः
    स्थिर ध्रुव अचिह्नित ह्रस्व क्रमः[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    यदि (inflateStateCheck(strm) || strm->अग्र_बहिर्गमनम् == Z_NULL ||
        (strm->अग्र_अन्तर् == Z_NULL && strm->avail_अन्तर् != 0))
        प्रत्यागम Z_प्रवाहः_ERROR;

    state = (संरचना inflate_state FAR *)strm->state;
    यदि (state->बहुलकम् == TYPE) state->बहुलकम् = TYPEDO;      /* skip check */
    आरोपय_2();
    अन्तर् = have;
    बहिर्गमनम् = वामः;
    ret = Z_OK;
    प्रति (;;)
        विकल्प (state->बहुलकम्) {
        पक्ष शीर्ष_2:
            यदि (state->wrap == 0) {
                state->बहुलकम् = TYPEDO;
                भङ्ग;
            }
            NEEDBITS(16);
#यदि_परिभाषितम् GUNZIP
            यदि ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                यदि (state->wbits == 0)
                    state->wbits = 15;
                state->check = चक्रीयपरीक्षा_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->बहुलकम् = FLAGS;
                भङ्ग;
            }
            यदि (state->शीर्ष != Z_NULL)
                state->शीर्ष->समाप्तम्_2 = -1;
            यदि (!(state->wrap & 1) ||   /* check if zlib header allowed */
#अन्यथा
            यदि (
#यदि_अन्तः
                ((द्वयङ्काः_2(8) << 8) + (hold >> 8)) % 31) {
                strm->सन्देशः = (z_const वर्ण *)"incorrect header check";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            यदि (द्वयङ्काः_2(4) != Z_DEFLATED) {
                strm->सन्देशः = (z_const वर्ण *)"unknown compression method";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            DROPBITS(4);
            len = द्वयङ्काः_2(4) + 8;
            यदि (state->wbits == 0)
                state->wbits = len;
            यदि (len > 15 || len > state->wbits) {
                strm->सन्देशः = (z_const वर्ण *)"invalid window size";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->बहुलकम् = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            भङ्ग;
#यदि_परिभाषितम् GUNZIP
        पक्ष FLAGS:
            NEEDBITS(16);
            state->flags = (पूर्णाङ्क)(hold);
            यदि ((state->flags & 0xff) != Z_DEFLATED) {
                strm->सन्देशः = (z_const वर्ण *)"unknown compression method";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            यदि (state->flags & 0xe000) {
                strm->सन्देशः = (z_const वर्ण *)"unknown header flags set";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            यदि (state->शीर्ष != Z_NULL)
                state->शीर्ष->text = (पूर्णाङ्क)((hold >> 8) & 1);
            यदि ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->बहुलकम् = TIME;
                /* fallthrough */
        पक्ष TIME:
            NEEDBITS(32);
            यदि (state->शीर्ष != Z_NULL)
                state->शीर्ष->time = hold;
            यदि ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->बहुलकम् = OS;
                /* fallthrough */
        पक्ष OS:
            NEEDBITS(16);
            यदि (state->शीर्ष != Z_NULL) {
                state->शीर्ष->xflags = (पूर्णाङ्क)(hold & 0xff);
                state->शीर्ष->os = (पूर्णाङ्क)(hold >> 8);
            }
            यदि ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->बहुलकम् = EXLEN;
                /* fallthrough */
        पक्ष EXLEN:
            यदि (state->flags & 0x0400) {
                NEEDBITS(16);
                state->दीर्घता = (अचिह्नित)(hold);
                यदि (state->शीर्ष != Z_NULL)
                    state->शीर्ष->extra_len = (अचिह्नित)hold;
                यदि ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            अन्यथा यदि (state->शीर्ष != Z_NULL)
                state->शीर्ष->extra = Z_NULL;
            state->बहुलकम् = EXTRA;
                /* fallthrough */
        पक्ष EXTRA:
            यदि (state->flags & 0x0400) {
                copy = state->दीर्घता;
                यदि (copy > have) copy = have;
                यदि (copy) {
                    यदि (state->शीर्ष != Z_NULL &&
                        state->शीर्ष->extra != Z_NULL &&
                        (len = state->शीर्ष->extra_len - state->दीर्घता) <
                            state->शीर्ष->extra_max) {
                        zmemcpy(state->शीर्ष->extra + len, अग्र,
                                len + copy > state->शीर्ष->extra_max ?
                                state->शीर्ष->extra_max - len : copy);
                    }
                    यदि ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = चक्रीयपरीक्षा_32_2(state->check, अग्र, copy);
                    have -= copy;
                    अग्र += copy;
                    state->दीर्घता -= copy;
                }
                यदि (state->दीर्घता) गच्छ inf_leave;
            }
            state->दीर्घता = 0;
            state->बहुलकम् = NAME;
                /* fallthrough */
        पक्ष NAME:
            यदि (state->flags & 0x0800) {
                यदि (have == 0) गच्छ inf_leave;
                copy = 0;
                कुरु {
                    len = (अचिह्नित)(अग्र[copy++]);
                    यदि (state->शीर्ष != Z_NULL &&
                            state->शीर्ष->name != Z_NULL &&
                            state->दीर्घता < state->शीर्ष->name_max)
                        state->शीर्ष->name[state->दीर्घता++] = (Bytef)len;
                } यावत् (len && copy < have);
                यदि ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = चक्रीयपरीक्षा_32_2(state->check, अग्र, copy);
                have -= copy;
                अग्र += copy;
                यदि (len) गच्छ inf_leave;
            }
            अन्यथा यदि (state->शीर्ष != Z_NULL)
                state->शीर्ष->name = Z_NULL;
            state->दीर्घता = 0;
            state->बहुलकम् = COMMENT;
                /* fallthrough */
        पक्ष COMMENT:
            यदि (state->flags & 0x1000) {
                यदि (have == 0) गच्छ inf_leave;
                copy = 0;
                कुरु {
                    len = (अचिह्नित)(अग्र[copy++]);
                    यदि (state->शीर्ष != Z_NULL &&
                            state->शीर्ष->comment != Z_NULL &&
                            state->दीर्घता < state->शीर्ष->comm_max)
                        state->शीर्ष->comment[state->दीर्घता++] = (Bytef)len;
                } यावत् (len && copy < have);
                यदि ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = चक्रीयपरीक्षा_32_2(state->check, अग्र, copy);
                have -= copy;
                अग्र += copy;
                यदि (len) गच्छ inf_leave;
            }
            अन्यथा यदि (state->शीर्ष != Z_NULL)
                state->शीर्ष->comment = Z_NULL;
            state->बहुलकम् = HCRC;
                /* fallthrough */
        पक्ष HCRC:
            यदि (state->flags & 0x0200) {
                NEEDBITS(16);
                यदि ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->सन्देशः = (z_const वर्ण *)"header crc mismatch";
                    state->बहुलकम् = BAD;
                    भङ्ग;
                }
                INITBITS();
            }
            यदि (state->शीर्ष != Z_NULL) {
                state->शीर्ष->hcrc = (पूर्णाङ्क)((state->flags >> 9) & 1);
                state->शीर्ष->समाप्तम्_2 = 1;
            }
            strm->adler = state->check = चक्रीयपरीक्षा_32_2(0L, Z_NULL, 0);
            state->बहुलकम् = TYPE;
            भङ्ग;
#यदि_अन्तः
        पक्ष DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->बहुलकम् = DICT;
                /* fallthrough */
        पक्ष DICT:
            यदि (state->havedict == 0) {
                RESTORE();
                प्रत्यागम Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->बहुलकम् = TYPE;
                /* fallthrough */
        पक्ष TYPE:
            यदि (flush == Z_खण्डम् || flush == Z_TREES) गच्छ inf_leave;
                /* fallthrough */
        पक्ष TYPEDO:
            यदि (state->चरमः) {
                BYTEBITS();
                state->बहुलकम् = CHECK;
                भङ्ग;
            }
            NEEDBITS(3);
            state->चरमः = द्वयङ्काः_2(1);
            DROPBITS(1);
            विकल्प (द्वयङ्काः_2(2)) {
            पक्ष 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->चरमः ? " (last)" : ""));
                state->बहुलकम् = STORED;
                भङ्ग;
            पक्ष 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->चरमः ? " (last)" : ""));
                state->बहुलकम् = LEN_;             /* decode codes */
                यदि (flush == Z_TREES) {
                    DROPBITS(2);
                    गच्छ inf_leave;
                }
                भङ्ग;
            पक्ष 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->चरमः ? " (last)" : ""));
                state->बहुलकम् = सारणी;
                भङ्ग;
            स्वभाव:
                strm->सन्देशः = (z_const वर्ण *)"invalid block type";
                state->बहुलकम् = BAD;
            }
            DROPBITS(2);
            भङ्ग;
        पक्ष STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            यदि ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid stored block lengths";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->दीर्घता = (अचिह्नित)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->दीर्घता));
            INITBITS();
            state->बहुलकम् = COPY_;
            यदि (flush == Z_TREES) गच्छ inf_leave;
                /* fallthrough */
        पक्ष COPY_:
            state->बहुलकम् = COPY;
                /* fallthrough */
        पक्ष COPY:
            copy = state->दीर्घता;
            यदि (copy) {
                यदि (copy > have) copy = have;
                यदि (copy > वामः) copy = वामः;
                यदि (copy == 0) गच्छ inf_leave;
                zmemcpy(put, अग्र, copy);
                have -= copy;
                अग्र += copy;
                वामः -= copy;
                put += copy;
                state->दीर्घता -= copy;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->बहुलकम् = TYPE;
            भङ्ग;
        पक्ष सारणी:
            NEEDBITS(14);
            state->nlen = द्वयङ्काः_2(5) + 257;
            DROPBITS(5);
            state->ndist = द्वयङ्काः_2(5) + 1;
            DROPBITS(5);
            state->ncode = द्वयङ्काः_2(4) + 4;
            DROPBITS(4);
#यदि_अपरिभाषितम् PKZIP_BUG_WORKAROUND
            यदि (state->nlen > 286 || state->ndist > 30) {
                strm->सन्देशः = (z_const वर्ण *)
                    "too many length or distance symbols";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
#यदि_अन्तः
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->बहुलकम् = LENLENS;
                /* fallthrough */
        पक्ष LENLENS:
            यावत् (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[क्रमः[state->have++]] = (अचिह्नित ह्रस्व)द्वयङ्काः_2(3);
                DROPBITS(3);
            }
            यावत् (state->have < 19)
                state->lens[क्रमः[state->have++]] = 0;
            state->अग्र = state->codes;
            state->lencode = state->distcode = (ध्रुव संहिता FAR *)(state->अग्र);
            state->lenbits = 7;
            ret = inflate_सारणी(CODES, state->lens, 19, &(state->अग्र),
                                &(state->lenbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid code lengths set";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->बहुलकम् = CODELENS;
                /* fallthrough */
        पक्ष CODELENS:
            यावत् (state->have < state->nlen + state->ndist) {
                प्रति (;;) {
                    अत्र = state->lencode[द्वयङ्काः_2(state->lenbits)];
                    यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                यदि (अत्र.val < 16) {
                    DROPBITS(अत्र.द्वयङ्काः);
                    state->lens[state->have++] = अत्र.val;
                }
                अन्यथा {
                    यदि (अत्र.val == 16) {
                        NEEDBITS(अत्र.द्वयङ्काः + 2);
                        DROPBITS(अत्र.द्वयङ्काः);
                        यदि (state->have == 0) {
                            strm->सन्देशः = (z_const वर्ण *)
                                "invalid bit length repeat";
                            state->बहुलकम् = BAD;
                            भङ्ग;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + द्वयङ्काः_2(2);
                        DROPBITS(2);
                    }
                    अन्यथा यदि (अत्र.val == 17) {
                        NEEDBITS(अत्र.द्वयङ्काः + 3);
                        DROPBITS(अत्र.द्वयङ्काः);
                        len = 0;
                        copy = 3 + द्वयङ्काः_2(3);
                        DROPBITS(3);
                    }
                    अन्यथा {
                        NEEDBITS(अत्र.द्वयङ्काः + 7);
                        DROPBITS(अत्र.द्वयङ्काः);
                        len = 0;
                        copy = 11 + द्वयङ्काः_2(7);
                        DROPBITS(7);
                    }
                    यदि (state->have + copy > state->nlen + state->ndist) {
                        strm->सन्देशः = (z_const वर्ण *)
                            "invalid bit length repeat";
                        state->बहुलकम् = BAD;
                        भङ्ग;
                    }
                    यावत् (copy--)
                        state->lens[state->have++] = (अचिह्नित ह्रस्व)len;
                }
            }

            /* handle error breaks in while */
            यदि (state->बहुलकम् == BAD) भङ्ग;

            /* check for end-of-block code (better have one) */
            यदि (state->lens[256] == 0) {
                strm->सन्देशः = (z_const वर्ण *)
                    "invalid code -- missing end-of-block";
                state->बहुलकम् = BAD;
                भङ्ग;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->अग्र = state->codes;
            state->lencode = (ध्रुव संहिता FAR *)(state->अग्र);
            state->lenbits = 9;
            ret = inflate_सारणी(LENS, state->lens, state->nlen, &(state->अग्र),
                                &(state->lenbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/lengths set";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->distcode = (ध्रुव संहिता FAR *)(state->अग्र);
            state->distbits = 6;
            ret = inflate_सारणी(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->अग्र), &(state->distbits), state->work);
            यदि (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distances set";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->बहुलकम् = LEN_;
            यदि (flush == Z_TREES) गच्छ inf_leave;
                /* fallthrough */
        पक्ष LEN_:
            state->बहुलकम् = LEN;
                /* fallthrough */
        पक्ष LEN:
            यदि (have >= 6 && वामः >= 258) {
                RESTORE();
                inflate_fast(strm, बहिर्गमनम्);
                आरोपय_2();
                यदि (state->बहुलकम् == TYPE)
                    state->पृष्ठभागः = -1;
                भङ्ग;
            }
            state->पृष्ठभागः = 0;
            प्रति (;;) {
                अत्र = state->lencode[द्वयङ्काः_2(state->lenbits)];
                यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                PULLBYTE();
            }
            यदि (अत्र.op && (अत्र.op & 0xf0) == 0) {
                चरमः = अत्र;
                प्रति (;;) {
                    अत्र = state->lencode[चरमः.val +
                            (द्वयङ्काः_2(चरमः.द्वयङ्काः + चरमः.op) >> चरमः.द्वयङ्काः)];
                    यदि ((अचिह्नित)(चरमः.द्वयङ्काः + अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(चरमः.द्वयङ्काः);
                state->पृष्ठभागः += चरमः.द्वयङ्काः;
            }
            DROPBITS(अत्र.द्वयङ्काः);
            state->पृष्ठभागः += अत्र.द्वयङ्काः;
            state->दीर्घता = (अचिह्नित)अत्र.val;
            यदि ((पूर्णाङ्क)(अत्र.op) == 0) {
                Tracevv((stderr, अत्र.val >= 0x20 && अत्र.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", अत्र.val));
                state->बहुलकम् = LIT;
                भङ्ग;
            }
            यदि (अत्र.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->पृष्ठभागः = -1;
                state->बहुलकम् = TYPE;
                भङ्ग;
            }
            यदि (अत्र.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/length code";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->extra = (अचिह्नित)(अत्र.op) & 15;
            state->बहुलकम् = LENEXT;
                /* fallthrough */
        पक्ष LENEXT:
            यदि (state->extra) {
                NEEDBITS(state->extra);
                state->दीर्घता += द्वयङ्काः_2(state->extra);
                DROPBITS(state->extra);
                state->पृष्ठभागः += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->दीर्घता));
            state->was = state->दीर्घता;
            state->बहुलकम् = DIST;
                /* fallthrough */
        पक्ष DIST:
            प्रति (;;) {
                अत्र = state->distcode[द्वयङ्काः_2(state->distbits)];
                यदि ((अचिह्नित)(अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                PULLBYTE();
            }
            यदि ((अत्र.op & 0xf0) == 0) {
                चरमः = अत्र;
                प्रति (;;) {
                    अत्र = state->distcode[चरमः.val +
                            (द्वयङ्काः_2(चरमः.द्वयङ्काः + चरमः.op) >> चरमः.द्वयङ्काः)];
                    यदि ((अचिह्नित)(चरमः.द्वयङ्काः + अत्र.द्वयङ्काः) <= द्वयङ्काः) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(चरमः.द्वयङ्काः);
                state->पृष्ठभागः += चरमः.द्वयङ्काः;
            }
            DROPBITS(अत्र.द्वयङ्काः);
            state->पृष्ठभागः += अत्र.द्वयङ्काः;
            यदि (अत्र.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance code";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
            state->विस्थापनम् = (अचिह्नित)अत्र.val;
            state->extra = (अचिह्नित)(अत्र.op) & 15;
            state->बहुलकम् = DISTEXT;
                /* fallthrough */
        पक्ष DISTEXT:
            यदि (state->extra) {
                NEEDBITS(state->extra);
                state->विस्थापनम् += द्वयङ्काः_2(state->extra);
                DROPBITS(state->extra);
                state->पृष्ठभागः += state->extra;
            }
#यदि_परिभाषितम् INFLATE_STRICT
            यदि (state->विस्थापनम् > state->dmax) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance too far back";
                state->बहुलकम् = BAD;
                भङ्ग;
            }
#यदि_अन्तः
            Tracevv((stderr, "inflate:         distance %u\n", state->विस्थापनम्));
            state->बहुलकम् = मेलनम्_2;
                /* fallthrough */
        पक्ष मेलनम्_2:
            यदि (वामः == 0) गच्छ inf_leave;
            copy = बहिर्गमनम् - वामः;
            यदि (state->विस्थापनम् > copy) {         /* copy from window */
                copy = state->विस्थापनम् - copy;
                यदि (copy > state->whave) {
                    यदि (state->sane) {
                        strm->सन्देशः = (z_const वर्ण *)
                            "invalid distance too far back";
                        state->बहुलकम् = BAD;
                        भङ्ग;
                    }
#यदि_परिभाषितम् INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    यदि (copy > state->दीर्घता) copy = state->दीर्घता;
                    यदि (copy > वामः) copy = वामः;
                    वामः -= copy;
                    state->दीर्घता -= copy;
                    कुरु {
                        *put++ = 0;
                    } यावत् (--copy);
                    यदि (state->दीर्घता == 0) state->बहुलकम् = LEN;
                    भङ्ग;
#यदि_अन्तः
                }
                यदि (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->गवाक्षः + (state->wsize - copy);
                }
                अन्यथा
                    from = state->गवाक्षः + (state->wnext - copy);
                यदि (copy > state->दीर्घता) copy = state->दीर्घता;
            }
            अन्यथा {                              /* copy from output */
                from = put - state->विस्थापनम्;
                copy = state->दीर्घता;
            }
            यदि (copy > वामः) copy = वामः;
            वामः -= copy;
            state->दीर्घता -= copy;
            कुरु {
                *put++ = *from++;
            } यावत् (--copy);
            यदि (state->दीर्घता == 0) state->बहुलकम् = LEN;
            भङ्ग;
        पक्ष LIT:
            यदि (वामः == 0) गच्छ inf_leave;
            *put++ = (अचिह्नित वर्ण)(state->दीर्घता);
            वामः--;
            state->बहुलकम् = LEN;
            भङ्ग;
        पक्ष CHECK:
            यदि (state->wrap) {
                NEEDBITS(32);
                बहिर्गमनम् -= वामः;
                strm->योग_बहिर्गमनम् += बहिर्गमनम्;
                state->योग_2 += बहिर्गमनम्;
                यदि ((state->wrap & 4) && बहिर्गमनम्)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - बहिर्गमनम्, बहिर्गमनम्);
                बहिर्गमनम् = वामः;
                यदि ((state->wrap & 4) && (
#यदि_परिभाषितम् GUNZIP
                     state->flags ? hold :
#यदि_अन्तः
                     ZSWAP32(hold)) != state->check) {
                    strm->सन्देशः = (z_const वर्ण *)"incorrect data check";
                    state->बहुलकम् = BAD;
                    भङ्ग;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#यदि_परिभाषितम् GUNZIP
            state->बहुलकम् = दीर्घता_2;
                /* fallthrough */
        पक्ष दीर्घता_2:
            यदि (state->wrap && state->flags) {
                NEEDBITS(32);
                यदि ((state->wrap & 4) && hold != (state->योग_2 & 0xffffffff)) {
                    strm->सन्देशः = (z_const वर्ण *)"incorrect length check";
                    state->बहुलकम् = BAD;
                    भङ्ग;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#यदि_अन्तः
            state->बहुलकम् = समाप्तम्;
                /* fallthrough */
        पक्ष समाप्तम्:
            ret = Z_प्रवाहः_अन्तः;
            गच्छ inf_leave;
        पक्ष BAD:
            ret = Z_दत्त_ERROR;
            गच्छ inf_leave;
        पक्ष MEM:
            प्रत्यागम Z_MEM_ERROR;
        पक्ष SYNC:
                /* fallthrough */
        स्वभाव:
            प्रत्यागम Z_प्रवाहः_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    यदि (state->wsize || (बहिर्गमनम् != strm->avail_बहिर्गमनम् && state->बहुलकम् < BAD &&
            (state->बहुलकम् < CHECK || flush != Z_FINISH)))
        यदि (updatewindow(strm, strm->अग्र_बहिर्गमनम्, बहिर्गमनम् - strm->avail_बहिर्गमनम्)) {
            state->बहुलकम् = MEM;
            प्रत्यागम Z_MEM_ERROR;
        }
    अन्तर् -= strm->avail_अन्तर्;
    बहिर्गमनम् -= strm->avail_बहिर्गमनम्;
    strm->योग_अन्तर् += अन्तर्;
    strm->योग_बहिर्गमनम् += बहिर्गमनम्;
    state->योग_2 += बहिर्गमनम्;
    यदि ((state->wrap & 4) && बहिर्गमनम्)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->अग्र_बहिर्गमनम् - बहिर्गमनम्, बहिर्गमनम्);
    strm->दत्त_type = (पूर्णाङ्क)state->द्वयङ्काः + (state->चरमः ? 64 : 0) +
                      (state->बहुलकम् == TYPE ? 128 : 0) +
                      (state->बहुलकम् == LEN_ || state->बहुलकम् == COPY_ ? 256 : 0);
    यदि (((अन्तर् == 0 && बहिर्गमनम् == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    प्रत्यागम ret;
}

पूर्णाङ्क ZEXPORT inflate_अन्तः(z_streamp strm) {
    संरचना inflate_state FAR *state;
    यदि (inflateStateCheck(strm))
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (state->गवाक्षः != Z_NULL) ZFREE(strm, state->गवाक्षः);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_दीर्घता) {
    संरचना inflate_state FAR *state;

    /* check state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* copy dictionary */
    यदि (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->गवाक्षः + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->गवाक्षः, state->wnext);
    }
    यदि (dict_दीर्घता != Z_NULL)
        *dict_दीर्घता = state->whave;
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflate_समुच्चयः_Dictionary(z_streamp strm, ध्रुव Bytef *dictionary,
                                 uInt dict_दीर्घता) {
    संरचना inflate_state FAR *state;
    अचिह्नित दीर्घ dictid;
    पूर्णाङ्क ret;

    /* check state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (state->wrap != 0 && state->बहुलकम् != DICT)
        प्रत्यागम Z_प्रवाहः_ERROR;

    /* check for correct dictionary identifier */
    यदि (state->बहुलकम् == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_दीर्घता);
        यदि (dictid != state->check)
            प्रत्यागम Z_दत्त_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_दीर्घता, dict_दीर्घता);
    यदि (ret) {
        state->बहुलकम् = MEM;
        प्रत्यागम Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflate_Get_शीर्षकम्(z_streamp strm, gz_headerp शीर्ष) {
    संरचना inflate_state FAR *state;

    /* check state */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि ((state->wrap & 2) == 0) प्रत्यागम Z_प्रवाहः_ERROR;

    /* save header structure */
    state->शीर्ष = शीर्ष;
    शीर्ष->समाप्तम्_2 = 0;
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
local अचिह्नित syncsearch(अचिह्नित FAR *have, ध्रुव अचिह्नित वर्ण FAR *buf,
                          अचिह्नित len) {
    अचिह्नित got;
    अचिह्नित अग्र;

    got = *have;
    अग्र = 0;
    यावत् (अग्र < len && got < 4) {
        यदि ((पूर्णाङ्क)(buf[अग्र]) == (got < 2 ? 0 : 0xff))
            got++;
        अन्यथा यदि (buf[अग्र])
            got = 0;
        अन्यथा
            got = 4 - got;
        अग्र++;
    }
    *have = got;
    प्रत्यागम अग्र;
}

पूर्णाङ्क ZEXPORT inflateSync(z_streamp strm) {
    अचिह्नित len;               /* number of bytes to look at or looked at */
    पूर्णाङ्क flags;                  /* temporary to save header status */
    अचिह्नित दीर्घ अन्तर्, बहिर्गमनम्;      /* temporary to save total_in and total_out */
    अचिह्नित वर्ण buf[4];       /* to restore bit buffer to byte string */
    संरचना inflate_state FAR *state;

    /* check parameters */
    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (strm->avail_अन्तर् == 0 && state->द्वयङ्काः < 8) प्रत्यागम Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    यदि (state->बहुलकम् != SYNC) {
        state->बहुलकम् = SYNC;
        state->hold >>= state->द्वयङ्काः & 7;
        state->द्वयङ्काः -= state->द्वयङ्काः & 7;
        len = 0;
        यावत् (state->द्वयङ्काः >= 8) {
            buf[len++] = (अचिह्नित वर्ण)(state->hold);
            state->hold >>= 8;
            state->द्वयङ्काः -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->अग्र_अन्तर्, strm->avail_अन्तर्);
    strm->avail_अन्तर् -= len;
    strm->अग्र_अन्तर् += len;
    strm->योग_अन्तर् += len;

    /* return no joy or set up to restart inflate() on a new block */
    यदि (state->have != 4) प्रत्यागम Z_दत्त_ERROR;
    यदि (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    अन्यथा
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    अन्तर् = strm->योग_अन्तर्;  बहिर्गमनम् = strm->योग_बहिर्गमनम्;
    inflateReset(strm);
    strm->योग_अन्तर् = अन्तर्;  strm->योग_बहिर्गमनम् = बहिर्गमनम्;
    state->flags = flags;
    state->बहुलकम् = TYPE;
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

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    प्रत्यागम state->बहुलकम् == STORED && state->द्वयङ्काः == 0;
}

पूर्णाङ्क ZEXPORT inflateCopy(z_streamp dest, z_streamp source) {
    संरचना inflate_state FAR *state;
    संरचना inflate_state FAR *copy;
    अचिह्नित वर्ण FAR *गवाक्षः;

    /* check input */
    यदि (inflateStateCheck(source) || dest == Z_NULL)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)source->state;

    /* allocate space */
    copy = (संरचना inflate_state FAR *)
           ZALLOC(source, 1, परिमाण(संरचना inflate_state));
    यदि (copy == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(copy, परिमाण(संरचना inflate_state));
    गवाक्षः = Z_NULL;
    यदि (state->गवाक्षः != Z_NULL) {
        गवाक्षः = (अचिह्नित वर्ण FAR *)
                 ZALLOC(source, 1U << state->wbits, परिमाण(अचिह्नित वर्ण));
        यदि (गवाक्षः == Z_NULL) {
            ZFREE(source, copy);
            प्रत्यागम Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, source, परिमाण(z_प्रवाहः));
    zmemcpy(copy, state, परिमाण(संरचना inflate_state));
    copy->strm = dest;
    यदि (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->अग्र = copy->codes + (state->अग्र - state->codes);
    यदि (गवाक्षः != Z_NULL)
        zmemcpy(गवाक्षः, state->गवाक्षः, state->whave);
    copy->गवाक्षः = गवाक्षः;
    dest->state = (संरचना आन्तरिकम्_state FAR *)copy;
    प्रत्यागम Z_OK;
}

पूर्णाङ्क ZEXPORT inflateUndermine(z_streamp strm, पूर्णाङ्क subvert) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
#यदि_परिभाषितम् INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
    state->sane = !subvert;
    प्रत्यागम Z_OK;
#अन्यथा
    (शून्य)subvert;
    state->sane = 1;
    प्रत्यागम Z_दत्त_ERROR;
#यदि_अन्तः
}

पूर्णाङ्क ZEXPORT inflateValidate(z_streamp strm, पूर्णाङ्क check) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    यदि (check && state->wrap)
        state->wrap |= 4;
    अन्यथा
        state->wrap &= ~4;
    प्रत्यागम Z_OK;
}

दीर्घ ZEXPORT inflateMark(z_streamp strm) {
    संरचना inflate_state FAR *state;

    यदि (inflateStateCheck(strm))
        प्रत्यागम -(1L << 16);
    state = (संरचना inflate_state FAR *)strm->state;
    प्रत्यागम (दीर्घ)(((अचिह्नित दीर्घ)((दीर्घ)state->पृष्ठभागः)) << 16) +
        (state->बहुलकम् == COPY ? state->दीर्घता :
            (state->बहुलकम् == मेलनम्_2 ? state->was - state->दीर्घता : 0));
}

अचिह्नित दीर्घ ZEXPORT inflate_Codes_प्रयुक्तम्(z_streamp strm) {
    संरचना inflate_state FAR *state;
    यदि (inflateStateCheck(strm)) प्रत्यागम (अचिह्नित दीर्घ)-1;
    state = (संरचना inflate_state FAR *)strm->state;
    प्रत्यागम (अचिह्नित दीर्घ)(state->अग्र - state->codes);
}
