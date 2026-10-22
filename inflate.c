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

#అన్తర్భావ "zutil.h"
#అన్తర్భావ "inftrees.h"
#అన్తర్భావ "inflate.h"
#అన్తర్భావ "inffast.h"

local పూర్ణాంకం inflateStateCheck(z_streamp strm) {
    నిర్మాణం inflate_state FAR *state;
    ఒకవేళ (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        తిరిగి 1;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ (state == Z_NULL || state->strm != strm ||
        state->బహుళకం < తల_2 || state->బహుళకం > SYNC)
        తిరిగి 1;
    తిరిగి 0;
}

పూర్ణాంకం ZEXPORT inflateResetKeep(z_streamp strm) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    strm->మొత్తం_లోపల = strm->మొత్తం_బహిర్గతం = state->మొత్తం = 0;
    strm->సందేశం = Z_NULL;
    strm->దత్తాంశం_type = 0;
    ఒకవేళ (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->బహుళకం = తల_2;
    state->చివరి = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->తల = Z_NULL;
    state->hold = 0;
    state->బిట్లు = 0;
    state->lencode = state->distcode = state->తదుపరి = state->codes;
    state->sane = 1;
    state->వెనుక = -1;
    Tracev((stderr, "inflate: reset\n"));
    తిరిగి Z_OK;
}

పూర్ణాంకం ZEXPORT inflateReset(z_streamp strm) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    తిరిగి inflateResetKeep(strm);
}

పూర్ణాంకం ZEXPORT inflateReset2(z_streamp strm, పూర్ణాంకం కిటికీ_బిట్లు) {
    పూర్ణాంకం wrap;
    నిర్మాణం inflate_state FAR *state;

    /* get the state */
    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    ఒకవేళ (కిటికీ_బిట్లు < 0) {
        ఒకవేళ (కిటికీ_బిట్లు < -15)
            తిరిగి Z_ప్రవాహం_ERROR;
        wrap = 0;
        కిటికీ_బిట్లు = -కిటికీ_బిట్లు;
    }
    లేకపోతే {
        wrap = (కిటికీ_బిట్లు >> 4) + 5;
#నిర్వచితమైతే GUNZIP
        ఒకవేళ (కిటికీ_బిట్లు < 48)
            కిటికీ_బిట్లు &= 15;
#అయితే_ముగింపు
    }

    /* set number of window bits, free window if different */
    ఒకవేళ (కిటికీ_బిట్లు && (కిటికీ_బిట్లు < 8 || కిటికీ_బిట్లు > 15))
        తిరిగి Z_ప్రవాహం_ERROR;
    ఒకవేళ (state->కిటికీ != Z_NULL && state->wbits != (గుర్తులేని)కిటికీ_బిట్లు) {
        ZFREE(strm, state->కిటికీ);
        state->కిటికీ = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (గుర్తులేని)కిటికీ_బిట్లు;
    తిరిగి inflateReset(strm);
}

పూర్ణాంకం ZEXPORT inflateInit2_(z_streamp strm, పూర్ణాంకం కిటికీ_బిట్లు,
                          స్థిరరాశి అక్షరం *సంస్కరణ, పూర్ణాంకం ప్రవాహం_కొలత) {
    పూర్ణాంకం ret;
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (సంస్కరణ == Z_NULL || సంస్కరణ[0] != ZLIB_సంస్కరణ[0] ||
        ప్రవాహం_కొలత != (పూర్ణాంకం)(పరిమాణం(z_ప్రవాహం)))
        తిరిగి Z_సంస్కరణ_ERROR;
    ఒకవేళ (strm == Z_NULL) తిరిగి Z_ప్రవాహం_ERROR;
    strm->సందేశం = Z_NULL;                 /* in case we return an error */
    ఒకవేళ (strm->zalloc == (alloc_func)0) {
#నిర్వచితమైతే Z_SOLO
        తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#అయితే_ముగింపు
    }
    ఒకవేళ (strm->zfree == (free_func)0)
#నిర్వచితమైతే Z_SOLO
        తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
        strm->zfree = zcfree;
#అయితే_ముగింపు
    state = (నిర్మాణం inflate_state FAR *)
            ZALLOC(strm, 1, పరిమాణం(నిర్మాణం inflate_state));
    ఒకవేళ (state == Z_NULL) తిరిగి Z_MEM_ERROR;
    zmemzero(state, పరిమాణం(నిర్మాణం inflate_state));
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (నిర్మాణం అంతర్గత_state FAR *)state;
    state->strm = strm;
    state->కిటికీ = Z_NULL;
    state->బహుళకం = తల_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, కిటికీ_బిట్లు);
    ఒకవేళ (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    తిరిగి ret;
}

పూర్ణాంకం ZEXPORT inflateInit_(z_streamp strm, స్థిరరాశి అక్షరం *సంస్కరణ,
                         పూర్ణాంకం ప్రవాహం_కొలత) {
    తిరిగి inflateInit2_(strm, DEF_WBITS, సంస్కరణ, ప్రవాహం_కొలత);
}

పూర్ణాంకం ZEXPORT inflate_ప్రధానసంఖ్య(z_streamp strm, పూర్ణాంకం బిట్లు, పూర్ణాంకం విలువ) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    ఒకవేళ (బిట్లు == 0)
        తిరిగి Z_OK;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ (బిట్లు < 0) {
        state->hold = 0;
        state->బిట్లు = 0;
        తిరిగి Z_OK;
    }
    ఒకవేళ (బిట్లు > 16 || state->బిట్లు + (uInt)బిట్లు > 32) తిరిగి Z_ప్రవాహం_ERROR;
    విలువ &= (1L << బిట్లు) - 1;
    state->hold += (గుర్తులేని దీర్ఘ)విలువ << state->బిట్లు;
    state->బిట్లు += (uInt)బిట్లు;
    తిరిగి Z_OK;
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
local పూర్ణాంకం updatewindow(z_streamp strm, స్థిరరాశి Bytef *చివర, గుర్తులేని copy) {
    నిర్మాణం inflate_state FAR *state;
    గుర్తులేని dist;

    state = (నిర్మాణం inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    ఒకవేళ (state->కిటికీ == Z_NULL) {
        state->కిటికీ = (గుర్తులేని అక్షరం FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               పరిమాణం(గుర్తులేని అక్షరం));
        ఒకవేళ (state->కిటికీ == Z_NULL) తిరిగి 1;
    }

    /* if window not in use yet, initialize */
    ఒకవేళ (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    ఒకవేళ (copy >= state->wsize) {
        zmemcpy(state->కిటికీ, చివర - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    లేకపోతే {
        dist = state->wsize - state->wnext;
        ఒకవేళ (dist > copy) dist = copy;
        zmemcpy(state->కిటికీ + state->wnext, చివర - copy, dist);
        copy -= dist;
        ఒకవేళ (copy) {
            zmemcpy(state->కిటికీ, చివర - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        లేకపోతే {
            state->wnext += dist;
            ఒకవేళ (state->wnext == state->wsize) state->wnext = 0;
            ఒకవేళ (state->whave < state->wsize) state->whave += dist;
        }
    }
    తిరిగి 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#నిర్వచితమైతే GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? చక్రీయ_తనిఖీ_32_2(check, buf, len) : adler32(check, buf, len))
#లేకపోతే
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#అయితే_ముగింపు

/* check macros for header crc */
#నిర్వచితమైతే GUNZIP
#  define CRC2(check, మాట) \
    చేయి { \
        hbuf[0] = (గుర్తులేని అక్షరం)(మాట); \
        hbuf[1] = (గుర్తులేని అక్షరం)((మాట) >> 8); \
        check = చక్రీయ_తనిఖీ_32_2(check, hbuf, 2); \
    } అయితే (0)

#  define CRC4(check, మాట) \
    చేయి { \
        hbuf[0] = (గుర్తులేని అక్షరం)(మాట); \
        hbuf[1] = (గుర్తులేని అక్షరం)((మాట) >> 8); \
        hbuf[2] = (గుర్తులేని అక్షరం)((మాట) >> 16); \
        hbuf[3] = (గుర్తులేని అక్షరం)((మాట) >> 24); \
        check = చక్రీయ_తనిఖీ_32_2(check, hbuf, 4); \
    } అయితే (0)
#అయితే_ముగింపు

/* Load registers with state in inflate() for speed */
#నిర్వచించు ఎక్కించు_2() \
    చేయి { \
        put = strm->తదుపరి_బహిర్గతం; \
        ఎడమ = strm->avail_బహిర్గతం; \
        తదుపరి = strm->తదుపరి_లోపల; \
        have = strm->avail_లోపల; \
        hold = state->hold; \
        బిట్లు = state->బిట్లు; \
    } అయితే (0)

/* Restore state from registers in inflate() */
#నిర్వచించు RESTORE() \
    చేయి { \
        strm->తదుపరి_బహిర్గతం = put; \
        strm->avail_బహిర్గతం = ఎడమ; \
        strm->తదుపరి_లోపల = తదుపరి; \
        strm->avail_లోపల = have; \
        state->hold = hold; \
        state->బిట్లు = బిట్లు; \
    } అయితే (0)

/* Clear the input bit accumulator */
#నిర్వచించు INITBITS() \
    చేయి { \
        hold = 0; \
        బిట్లు = 0; \
    } అయితే (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#నిర్వచించు PULLBYTE() \
    చేయి { \
        ఒకవేళ (have == 0) వెళ్ళు inf_leave; \
        have--; \
        hold += (గుర్తులేని దీర్ఘ)(*తదుపరి++) << బిట్లు; \
        బిట్లు += 8; \
    } అయితే (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#నిర్వచించు NEEDBITS(n) \
    చేయి { \
        అయితే (బిట్లు < (గుర్తులేని)(n)) \
            PULLBYTE(); \
    } అయితే (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#నిర్వచించు బిట్లు_2(n) \
    ((గుర్తులేని)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#నిర్వచించు DROPBITS(n) \
    చేయి { \
        hold >>= (n); \
        బిట్లు -= (గుర్తులేని)(n); \
    } అయితే (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#నిర్వచించు BYTEBITS() \
    చేయి { \
        hold >>= బిట్లు & 7; \
        బిట్లు -= బిట్లు & 7; \
    } అయితే (0)

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

పూర్ణాంకం ZEXPORT inflate(z_streamp strm, పూర్ణాంకం flush) {
    నిర్మాణం inflate_state FAR *state;
    z_const గుర్తులేని అక్షరం FAR *తదుపరి;    /* next input */
    గుర్తులేని అక్షరం FAR *put;     /* next output */
    గుర్తులేని have, ఎడమ;        /* available input and output */
    గుర్తులేని దీర్ఘ hold;         /* bit buffer */
    గుర్తులేని బిట్లు;              /* bits in bit buffer */
    గుర్తులేని లోపల, బహిర్గతం;           /* save starting available input and output */
    గుర్తులేని copy;              /* number of stored or match bytes to copy */
    గుర్తులేని అక్షరం FAR *from;    /* where to copy match bytes from */
    సంకేతం_2 ఇక్కడ;                  /* current decoding table entry */
    సంకేతం_2 చివరి;                  /* parent table entry */
    గుర్తులేని len;               /* length to copy for repeats, bits to drop */
    పూర్ణాంకం ret;                    /* return code */
#నిర్వచితమైతే GUNZIP
    గుర్తులేని అక్షరం hbuf[4];      /* buffer for gzip header crc calculation */
#అయితే_ముగింపు
    స్థిర స్థిరరాశి గుర్తులేని పొట్టి క్రమం[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    ఒకవేళ (inflateStateCheck(strm) || strm->తదుపరి_బహిర్గతం == Z_NULL ||
        (strm->తదుపరి_లోపల == Z_NULL && strm->avail_లోపల != 0))
        తిరిగి Z_ప్రవాహం_ERROR;

    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ (state->బహుళకం == TYPE) state->బహుళకం = TYPEDO;      /* skip check */
    ఎక్కించు_2();
    లోపల = have;
    బహిర్గతం = ఎడమ;
    ret = Z_OK;
    కొరకు (;;)
        ఎంపిక (state->బహుళకం) {
        స్థితి తల_2:
            ఒకవేళ (state->wrap == 0) {
                state->బహుళకం = TYPEDO;
                ఆపు;
            }
            NEEDBITS(16);
#నిర్వచితమైతే GUNZIP
            ఒకవేళ ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                ఒకవేళ (state->wbits == 0)
                    state->wbits = 15;
                state->check = చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->బహుళకం = FLAGS;
                ఆపు;
            }
            ఒకవేళ (state->తల != Z_NULL)
                state->తల->పూర్తి_2 = -1;
            ఒకవేళ (!(state->wrap & 1) ||   /* check if zlib header allowed */
#లేకపోతే
            ఒకవేళ (
#అయితే_ముగింపు
                ((బిట్లు_2(8) << 8) + (hold >> 8)) % 31) {
                strm->సందేశం = (z_const అక్షరం *)"incorrect header check";
                state->బహుళకం = BAD;
                ఆపు;
            }
            ఒకవేళ (బిట్లు_2(4) != Z_DEFLATED) {
                strm->సందేశం = (z_const అక్షరం *)"unknown compression method";
                state->బహుళకం = BAD;
                ఆపు;
            }
            DROPBITS(4);
            len = బిట్లు_2(4) + 8;
            ఒకవేళ (state->wbits == 0)
                state->wbits = len;
            ఒకవేళ (len > 15 || len > state->wbits) {
                strm->సందేశం = (z_const అక్షరం *)"invalid window size";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->బహుళకం = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            ఆపు;
#నిర్వచితమైతే GUNZIP
        స్థితి FLAGS:
            NEEDBITS(16);
            state->flags = (పూర్ణాంకం)(hold);
            ఒకవేళ ((state->flags & 0xff) != Z_DEFLATED) {
                strm->సందేశం = (z_const అక్షరం *)"unknown compression method";
                state->బహుళకం = BAD;
                ఆపు;
            }
            ఒకవేళ (state->flags & 0xe000) {
                strm->సందేశం = (z_const అక్షరం *)"unknown header flags set";
                state->బహుళకం = BAD;
                ఆపు;
            }
            ఒకవేళ (state->తల != Z_NULL)
                state->తల->పాఠ్యం = (పూర్ణాంకం)((hold >> 8) & 1);
            ఒకవేళ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->బహుళకం = సమయం;
                /* fallthrough */
        స్థితి సమయం:
            NEEDBITS(32);
            ఒకవేళ (state->తల != Z_NULL)
                state->తల->time = hold;
            ఒకవేళ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->బహుళకం = OS;
                /* fallthrough */
        స్థితి OS:
            NEEDBITS(16);
            ఒకవేళ (state->తల != Z_NULL) {
                state->తల->xflags = (పూర్ణాంకం)(hold & 0xff);
                state->తల->os = (పూర్ణాంకం)(hold >> 8);
            }
            ఒకవేళ ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->బహుళకం = EXLEN;
                /* fallthrough */
        స్థితి EXLEN:
            ఒకవేళ (state->flags & 0x0400) {
                NEEDBITS(16);
                state->నిడివి = (గుర్తులేని)(hold);
                ఒకవేళ (state->తల != Z_NULL)
                    state->తల->extra_len = (గుర్తులేని)hold;
                ఒకవేళ ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            లేకపోతే ఒకవేళ (state->తల != Z_NULL)
                state->తల->extra = Z_NULL;
            state->బహుళకం = EXTRA;
                /* fallthrough */
        స్థితి EXTRA:
            ఒకవేళ (state->flags & 0x0400) {
                copy = state->నిడివి;
                ఒకవేళ (copy > have) copy = have;
                ఒకవేళ (copy) {
                    ఒకవేళ (state->తల != Z_NULL &&
                        state->తల->extra != Z_NULL &&
                        (len = state->తల->extra_len - state->నిడివి) <
                            state->తల->extra_max) {
                        zmemcpy(state->తల->extra + len, తదుపరి,
                                len + copy > state->తల->extra_max ?
                                state->తల->extra_max - len : copy);
                    }
                    ఒకవేళ ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = చక్రీయ_తనిఖీ_32_2(state->check, తదుపరి, copy);
                    have -= copy;
                    తదుపరి += copy;
                    state->నిడివి -= copy;
                }
                ఒకవేళ (state->నిడివి) వెళ్ళు inf_leave;
            }
            state->నిడివి = 0;
            state->బహుళకం = NAME;
                /* fallthrough */
        స్థితి NAME:
            ఒకవేళ (state->flags & 0x0800) {
                ఒకవేళ (have == 0) వెళ్ళు inf_leave;
                copy = 0;
                చేయి {
                    len = (గుర్తులేని)(తదుపరి[copy++]);
                    ఒకవేళ (state->తల != Z_NULL &&
                            state->తల->name != Z_NULL &&
                            state->నిడివి < state->తల->name_max)
                        state->తల->name[state->నిడివి++] = (Bytef)len;
                } అయితే (len && copy < have);
                ఒకవేళ ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = చక్రీయ_తనిఖీ_32_2(state->check, తదుపరి, copy);
                have -= copy;
                తదుపరి += copy;
                ఒకవేళ (len) వెళ్ళు inf_leave;
            }
            లేకపోతే ఒకవేళ (state->తల != Z_NULL)
                state->తల->name = Z_NULL;
            state->నిడివి = 0;
            state->బహుళకం = COMMENT;
                /* fallthrough */
        స్థితి COMMENT:
            ఒకవేళ (state->flags & 0x1000) {
                ఒకవేళ (have == 0) వెళ్ళు inf_leave;
                copy = 0;
                చేయి {
                    len = (గుర్తులేని)(తదుపరి[copy++]);
                    ఒకవేళ (state->తల != Z_NULL &&
                            state->తల->comment != Z_NULL &&
                            state->నిడివి < state->తల->comm_max)
                        state->తల->comment[state->నిడివి++] = (Bytef)len;
                } అయితే (len && copy < have);
                ఒకవేళ ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = చక్రీయ_తనిఖీ_32_2(state->check, తదుపరి, copy);
                have -= copy;
                తదుపరి += copy;
                ఒకవేళ (len) వెళ్ళు inf_leave;
            }
            లేకపోతే ఒకవేళ (state->తల != Z_NULL)
                state->తల->comment = Z_NULL;
            state->బహుళకం = HCRC;
                /* fallthrough */
        స్థితి HCRC:
            ఒకవేళ (state->flags & 0x0200) {
                NEEDBITS(16);
                ఒకవేళ ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->సందేశం = (z_const అక్షరం *)"header crc mismatch";
                    state->బహుళకం = BAD;
                    ఆపు;
                }
                INITBITS();
            }
            ఒకవేళ (state->తల != Z_NULL) {
                state->తల->hcrc = (పూర్ణాంకం)((state->flags >> 9) & 1);
                state->తల->పూర్తి_2 = 1;
            }
            strm->adler = state->check = చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0);
            state->బహుళకం = TYPE;
            ఆపు;
#అయితే_ముగింపు
        స్థితి DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->బహుళకం = DICT;
                /* fallthrough */
        స్థితి DICT:
            ఒకవేళ (state->havedict == 0) {
                RESTORE();
                తిరిగి Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->బహుళకం = TYPE;
                /* fallthrough */
        స్థితి TYPE:
            ఒకవేళ (flush == Z_ఖండం || flush == Z_TREES) వెళ్ళు inf_leave;
                /* fallthrough */
        స్థితి TYPEDO:
            ఒకవేళ (state->చివరి) {
                BYTEBITS();
                state->బహుళకం = CHECK;
                ఆపు;
            }
            NEEDBITS(3);
            state->చివరి = బిట్లు_2(1);
            DROPBITS(1);
            ఎంపిక (బిట్లు_2(2)) {
            స్థితి 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->చివరి ? " (last)" : ""));
                state->బహుళకం = STORED;
                ఆపు;
            స్థితి 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->చివరి ? " (last)" : ""));
                state->బహుళకం = LEN_;             /* decode codes */
                ఒకవేళ (flush == Z_TREES) {
                    DROPBITS(2);
                    వెళ్ళు inf_leave;
                }
                ఆపు;
            స్థితి 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->చివరి ? " (last)" : ""));
                state->బహుళకం = పట్టిక;
                ఆపు;
            అప్రమేయ:
                strm->సందేశం = (z_const అక్షరం *)"invalid block type";
                state->బహుళకం = BAD;
            }
            DROPBITS(2);
            ఆపు;
        స్థితి STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            ఒకవేళ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->సందేశం = (z_const అక్షరం *)"invalid stored block lengths";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->నిడివి = (గుర్తులేని)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->నిడివి));
            INITBITS();
            state->బహుళకం = COPY_;
            ఒకవేళ (flush == Z_TREES) వెళ్ళు inf_leave;
                /* fallthrough */
        స్థితి COPY_:
            state->బహుళకం = COPY;
                /* fallthrough */
        స్థితి COPY:
            copy = state->నిడివి;
            ఒకవేళ (copy) {
                ఒకవేళ (copy > have) copy = have;
                ఒకవేళ (copy > ఎడమ) copy = ఎడమ;
                ఒకవేళ (copy == 0) వెళ్ళు inf_leave;
                zmemcpy(put, తదుపరి, copy);
                have -= copy;
                తదుపరి += copy;
                ఎడమ -= copy;
                put += copy;
                state->నిడివి -= copy;
                ఆపు;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->బహుళకం = TYPE;
            ఆపు;
        స్థితి పట్టిక:
            NEEDBITS(14);
            state->nlen = బిట్లు_2(5) + 257;
            DROPBITS(5);
            state->ndist = బిట్లు_2(5) + 1;
            DROPBITS(5);
            state->ncode = బిట్లు_2(4) + 4;
            DROPBITS(4);
#నిర్వచితం_కాకపోతే PKZIP_BUG_WORKAROUND
            ఒకవేళ (state->nlen > 286 || state->ndist > 30) {
                strm->సందేశం = (z_const అక్షరం *)
                    "too many length or distance symbols";
                state->బహుళకం = BAD;
                ఆపు;
            }
#అయితే_ముగింపు
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->బహుళకం = LENLENS;
                /* fallthrough */
        స్థితి LENLENS:
            అయితే (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[క్రమం[state->have++]] = (గుర్తులేని పొట్టి)బిట్లు_2(3);
                DROPBITS(3);
            }
            అయితే (state->have < 19)
                state->lens[క్రమం[state->have++]] = 0;
            state->తదుపరి = state->codes;
            state->lencode = state->distcode = (స్థిరరాశి సంకేతం_2 FAR *)(state->తదుపరి);
            state->lenbits = 7;
            ret = inflate_పట్టిక(CODES, state->lens, 19, &(state->తదుపరి),
                                &(state->lenbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid code lengths set";
                state->బహుళకం = BAD;
                ఆపు;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->బహుళకం = CODELENS;
                /* fallthrough */
        స్థితి CODELENS:
            అయితే (state->have < state->nlen + state->ndist) {
                కొరకు (;;) {
                    ఇక్కడ = state->lencode[బిట్లు_2(state->lenbits)];
                    ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                ఒకవేళ (ఇక్కడ.val < 16) {
                    DROPBITS(ఇక్కడ.బిట్లు);
                    state->lens[state->have++] = ఇక్కడ.val;
                }
                లేకపోతే {
                    ఒకవేళ (ఇక్కడ.val == 16) {
                        NEEDBITS(ఇక్కడ.బిట్లు + 2);
                        DROPBITS(ఇక్కడ.బిట్లు);
                        ఒకవేళ (state->have == 0) {
                            strm->సందేశం = (z_const అక్షరం *)
                                "invalid bit length repeat";
                            state->బహుళకం = BAD;
                            ఆపు;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + బిట్లు_2(2);
                        DROPBITS(2);
                    }
                    లేకపోతే ఒకవేళ (ఇక్కడ.val == 17) {
                        NEEDBITS(ఇక్కడ.బిట్లు + 3);
                        DROPBITS(ఇక్కడ.బిట్లు);
                        len = 0;
                        copy = 3 + బిట్లు_2(3);
                        DROPBITS(3);
                    }
                    లేకపోతే {
                        NEEDBITS(ఇక్కడ.బిట్లు + 7);
                        DROPBITS(ఇక్కడ.బిట్లు);
                        len = 0;
                        copy = 11 + బిట్లు_2(7);
                        DROPBITS(7);
                    }
                    ఒకవేళ (state->have + copy > state->nlen + state->ndist) {
                        strm->సందేశం = (z_const అక్షరం *)
                            "invalid bit length repeat";
                        state->బహుళకం = BAD;
                        ఆపు;
                    }
                    అయితే (copy--)
                        state->lens[state->have++] = (గుర్తులేని పొట్టి)len;
                }
            }

            /* handle error breaks in while */
            ఒకవేళ (state->బహుళకం == BAD) ఆపు;

            /* check for end-of-block code (better have one) */
            ఒకవేళ (state->lens[256] == 0) {
                strm->సందేశం = (z_const అక్షరం *)
                    "invalid code -- missing end-of-block";
                state->బహుళకం = BAD;
                ఆపు;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->తదుపరి = state->codes;
            state->lencode = (స్థిరరాశి సంకేతం_2 FAR *)(state->తదుపరి);
            state->lenbits = 9;
            ret = inflate_పట్టిక(LENS, state->lens, state->nlen, &(state->తదుపరి),
                                &(state->lenbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid literal/lengths set";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->distcode = (స్థిరరాశి సంకేతం_2 FAR *)(state->తదుపరి);
            state->distbits = 6;
            ret = inflate_పట్టిక(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->తదుపరి), &(state->distbits), state->work);
            ఒకవేళ (ret) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distances set";
                state->బహుళకం = BAD;
                ఆపు;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->బహుళకం = LEN_;
            ఒకవేళ (flush == Z_TREES) వెళ్ళు inf_leave;
                /* fallthrough */
        స్థితి LEN_:
            state->బహుళకం = LEN;
                /* fallthrough */
        స్థితి LEN:
            ఒకవేళ (have >= 6 && ఎడమ >= 258) {
                RESTORE();
                inflate_fast(strm, బహిర్గతం);
                ఎక్కించు_2();
                ఒకవేళ (state->బహుళకం == TYPE)
                    state->వెనుక = -1;
                ఆపు;
            }
            state->వెనుక = 0;
            కొరకు (;;) {
                ఇక్కడ = state->lencode[బిట్లు_2(state->lenbits)];
                ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                PULLBYTE();
            }
            ఒకవేళ (ఇక్కడ.op && (ఇక్కడ.op & 0xf0) == 0) {
                చివరి = ఇక్కడ;
                కొరకు (;;) {
                    ఇక్కడ = state->lencode[చివరి.val +
                            (బిట్లు_2(చివరి.బిట్లు + చివరి.op) >> చివరి.బిట్లు)];
                    ఒకవేళ ((గుర్తులేని)(చివరి.బిట్లు + ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                DROPBITS(చివరి.బిట్లు);
                state->వెనుక += చివరి.బిట్లు;
            }
            DROPBITS(ఇక్కడ.బిట్లు);
            state->వెనుక += ఇక్కడ.బిట్లు;
            state->నిడివి = (గుర్తులేని)ఇక్కడ.val;
            ఒకవేళ ((పూర్ణాంకం)(ఇక్కడ.op) == 0) {
                Tracevv((stderr, ఇక్కడ.val >= 0x20 && ఇక్కడ.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", ఇక్కడ.val));
                state->బహుళకం = LIT;
                ఆపు;
            }
            ఒకవేళ (ఇక్కడ.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->వెనుక = -1;
                state->బహుళకం = TYPE;
                ఆపు;
            }
            ఒకవేళ (ఇక్కడ.op & 64) {
                strm->సందేశం = (z_const అక్షరం *)"invalid literal/length code";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->extra = (గుర్తులేని)(ఇక్కడ.op) & 15;
            state->బహుళకం = LENEXT;
                /* fallthrough */
        స్థితి LENEXT:
            ఒకవేళ (state->extra) {
                NEEDBITS(state->extra);
                state->నిడివి += బిట్లు_2(state->extra);
                DROPBITS(state->extra);
                state->వెనుక += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->నిడివి));
            state->was = state->నిడివి;
            state->బహుళకం = DIST;
                /* fallthrough */
        స్థితి DIST:
            కొరకు (;;) {
                ఇక్కడ = state->distcode[బిట్లు_2(state->distbits)];
                ఒకవేళ ((గుర్తులేని)(ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                PULLBYTE();
            }
            ఒకవేళ ((ఇక్కడ.op & 0xf0) == 0) {
                చివరి = ఇక్కడ;
                కొరకు (;;) {
                    ఇక్కడ = state->distcode[చివరి.val +
                            (బిట్లు_2(చివరి.బిట్లు + చివరి.op) >> చివరి.బిట్లు)];
                    ఒకవేళ ((గుర్తులేని)(చివరి.బిట్లు + ఇక్కడ.బిట్లు) <= బిట్లు) ఆపు;
                    PULLBYTE();
                }
                DROPBITS(చివరి.బిట్లు);
                state->వెనుక += చివరి.బిట్లు;
            }
            DROPBITS(ఇక్కడ.బిట్లు);
            state->వెనుక += ఇక్కడ.బిట్లు;
            ఒకవేళ (ఇక్కడ.op & 64) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distance code";
                state->బహుళకం = BAD;
                ఆపు;
            }
            state->స్థానభ్రంశం = (గుర్తులేని)ఇక్కడ.val;
            state->extra = (గుర్తులేని)(ఇక్కడ.op) & 15;
            state->బహుళకం = DISTEXT;
                /* fallthrough */
        స్థితి DISTEXT:
            ఒకవేళ (state->extra) {
                NEEDBITS(state->extra);
                state->స్థానభ్రంశం += బిట్లు_2(state->extra);
                DROPBITS(state->extra);
                state->వెనుక += state->extra;
            }
#నిర్వచితమైతే INFLATE_STRICT
            ఒకవేళ (state->స్థానభ్రంశం > state->dmax) {
                strm->సందేశం = (z_const అక్షరం *)"invalid distance too far back";
                state->బహుళకం = BAD;
                ఆపు;
            }
#అయితే_ముగింపు
            Tracevv((stderr, "inflate:         distance %u\n", state->స్థానభ్రంశం));
            state->బహుళకం = సరిపోలిక_2;
                /* fallthrough */
        స్థితి సరిపోలిక_2:
            ఒకవేళ (ఎడమ == 0) వెళ్ళు inf_leave;
            copy = బహిర్గతం - ఎడమ;
            ఒకవేళ (state->స్థానభ్రంశం > copy) {         /* copy from window */
                copy = state->స్థానభ్రంశం - copy;
                ఒకవేళ (copy > state->whave) {
                    ఒకవేళ (state->sane) {
                        strm->సందేశం = (z_const అక్షరం *)
                            "invalid distance too far back";
                        state->బహుళకం = BAD;
                        ఆపు;
                    }
#నిర్వచితమైతే INFLATE_ALLOW_INVALID_దూరం_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    ఒకవేళ (copy > state->నిడివి) copy = state->నిడివి;
                    ఒకవేళ (copy > ఎడమ) copy = ఎడమ;
                    ఎడమ -= copy;
                    state->నిడివి -= copy;
                    చేయి {
                        *put++ = 0;
                    } అయితే (--copy);
                    ఒకవేళ (state->నిడివి == 0) state->బహుళకం = LEN;
                    ఆపు;
#అయితే_ముగింపు
                }
                ఒకవేళ (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->కిటికీ + (state->wsize - copy);
                }
                లేకపోతే
                    from = state->కిటికీ + (state->wnext - copy);
                ఒకవేళ (copy > state->నిడివి) copy = state->నిడివి;
            }
            లేకపోతే {                              /* copy from output */
                from = put - state->స్థానభ్రంశం;
                copy = state->నిడివి;
            }
            ఒకవేళ (copy > ఎడమ) copy = ఎడమ;
            ఎడమ -= copy;
            state->నిడివి -= copy;
            చేయి {
                *put++ = *from++;
            } అయితే (--copy);
            ఒకవేళ (state->నిడివి == 0) state->బహుళకం = LEN;
            ఆపు;
        స్థితి LIT:
            ఒకవేళ (ఎడమ == 0) వెళ్ళు inf_leave;
            *put++ = (గుర్తులేని అక్షరం)(state->నిడివి);
            ఎడమ--;
            state->బహుళకం = LEN;
            ఆపు;
        స్థితి CHECK:
            ఒకవేళ (state->wrap) {
                NEEDBITS(32);
                బహిర్గతం -= ఎడమ;
                strm->మొత్తం_బహిర్గతం += బహిర్గతం;
                state->మొత్తం += బహిర్గతం;
                ఒకవేళ ((state->wrap & 4) && బహిర్గతం)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - బహిర్గతం, బహిర్గతం);
                బహిర్గతం = ఎడమ;
                ఒకవేళ ((state->wrap & 4) && (
#నిర్వచితమైతే GUNZIP
                     state->flags ? hold :
#అయితే_ముగింపు
                     ZSWAP32(hold)) != state->check) {
                    strm->సందేశం = (z_const అక్షరం *)"incorrect data check";
                    state->బహుళకం = BAD;
                    ఆపు;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#నిర్వచితమైతే GUNZIP
            state->బహుళకం = నిడివి_2;
                /* fallthrough */
        స్థితి నిడివి_2:
            ఒకవేళ (state->wrap && state->flags) {
                NEEDBITS(32);
                ఒకవేళ ((state->wrap & 4) && hold != (state->మొత్తం & 0xffffffff)) {
                    strm->సందేశం = (z_const అక్షరం *)"incorrect length check";
                    state->బహుళకం = BAD;
                    ఆపు;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#అయితే_ముగింపు
            state->బహుళకం = పూర్తి;
                /* fallthrough */
        స్థితి పూర్తి:
            ret = Z_ప్రవాహం_చివర;
            వెళ్ళు inf_leave;
        స్థితి BAD:
            ret = Z_దత్తాంశం_ERROR;
            వెళ్ళు inf_leave;
        స్థితి MEM:
            తిరిగి Z_MEM_ERROR;
        స్థితి SYNC:
                /* fallthrough */
        అప్రమేయ:
            తిరిగి Z_ప్రవాహం_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    ఒకవేళ (state->wsize || (బహిర్గతం != strm->avail_బహిర్గతం && state->బహుళకం < BAD &&
            (state->బహుళకం < CHECK || flush != Z_ముగింపు)))
        ఒకవేళ (updatewindow(strm, strm->తదుపరి_బహిర్గతం, బహిర్గతం - strm->avail_బహిర్గతం)) {
            state->బహుళకం = MEM;
            తిరిగి Z_MEM_ERROR;
        }
    లోపల -= strm->avail_లోపల;
    బహిర్గతం -= strm->avail_బహిర్గతం;
    strm->మొత్తం_లోపల += లోపల;
    strm->మొత్తం_బహిర్గతం += బహిర్గతం;
    state->మొత్తం += బహిర్గతం;
    ఒకవేళ ((state->wrap & 4) && బహిర్గతం)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->తదుపరి_బహిర్గతం - బహిర్గతం, బహిర్గతం);
    strm->దత్తాంశం_type = (పూర్ణాంకం)state->బిట్లు + (state->చివరి ? 64 : 0) +
                      (state->బహుళకం == TYPE ? 128 : 0) +
                      (state->బహుళకం == LEN_ || state->బహుళకం == COPY_ ? 256 : 0);
    ఒకవేళ (((లోపల == 0 && బహిర్గతం == 0) || flush == Z_ముగింపు) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    తిరిగి ret;
}

పూర్ణాంకం ZEXPORT inflate_చివర(z_streamp strm) {
    నిర్మాణం inflate_state FAR *state;
    ఒకవేళ (inflateStateCheck(strm))
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ (state->కిటికీ != Z_NULL) ZFREE(strm, state->కిటికీ);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    తిరిగి Z_OK;
}

పూర్ణాంకం ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_నిడివి) {
    నిర్మాణం inflate_state FAR *state;

    /* check state */
    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;

    /* copy dictionary */
    ఒకవేళ (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->కిటికీ + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->కిటికీ, state->wnext);
    }
    ఒకవేళ (dict_నిడివి != Z_NULL)
        *dict_నిడివి = state->whave;
    తిరిగి Z_OK;
}

పూర్ణాంకం ZEXPORT inflate_సమితి_Dictionary(z_streamp strm, స్థిరరాశి Bytef *dictionary,
                                 uInt dict_నిడివి) {
    నిర్మాణం inflate_state FAR *state;
    గుర్తులేని దీర్ఘ dictid;
    పూర్ణాంకం ret;

    /* check state */
    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ (state->wrap != 0 && state->బహుళకం != DICT)
        తిరిగి Z_ప్రవాహం_ERROR;

    /* check for correct dictionary identifier */
    ఒకవేళ (state->బహుళకం == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_నిడివి);
        ఒకవేళ (dictid != state->check)
            తిరిగి Z_దత్తాంశం_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_నిడివి, dict_నిడివి);
    ఒకవేళ (ret) {
        state->బహుళకం = MEM;
        తిరిగి Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    తిరిగి Z_OK;
}

పూర్ణాంకం ZEXPORT inflate_Get_శీర్షిక(z_streamp strm, gz_headerp తల) {
    నిర్మాణం inflate_state FAR *state;

    /* check state */
    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ ((state->wrap & 2) == 0) తిరిగి Z_ప్రవాహం_ERROR;

    /* save header structure */
    state->తల = తల;
    తల->పూర్తి_2 = 0;
    తిరిగి Z_OK;
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
local గుర్తులేని syncsearch(గుర్తులేని FAR *have, స్థిరరాశి గుర్తులేని అక్షరం FAR *buf,
                          గుర్తులేని len) {
    గుర్తులేని got;
    గుర్తులేని తదుపరి;

    got = *have;
    తదుపరి = 0;
    అయితే (తదుపరి < len && got < 4) {
        ఒకవేళ ((పూర్ణాంకం)(buf[తదుపరి]) == (got < 2 ? 0 : 0xff))
            got++;
        లేకపోతే ఒకవేళ (buf[తదుపరి])
            got = 0;
        లేకపోతే
            got = 4 - got;
        తదుపరి++;
    }
    *have = got;
    తిరిగి తదుపరి;
}

పూర్ణాంకం ZEXPORT inflateSync(z_streamp strm) {
    గుర్తులేని len;               /* number of bytes to look at or looked at */
    పూర్ణాంకం flags;                  /* temporary to save header status */
    గుర్తులేని దీర్ఘ లోపల, బహిర్గతం;      /* temporary to save total_in and total_out */
    గుర్తులేని అక్షరం buf[4];       /* to restore bit buffer to byte string */
    నిర్మాణం inflate_state FAR *state;

    /* check parameters */
    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ (strm->avail_లోపల == 0 && state->బిట్లు < 8) తిరిగి Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    ఒకవేళ (state->బహుళకం != SYNC) {
        state->బహుళకం = SYNC;
        state->hold >>= state->బిట్లు & 7;
        state->బిట్లు -= state->బిట్లు & 7;
        len = 0;
        అయితే (state->బిట్లు >= 8) {
            buf[len++] = (గుర్తులేని అక్షరం)(state->hold);
            state->hold >>= 8;
            state->బిట్లు -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->తదుపరి_లోపల, strm->avail_లోపల);
    strm->avail_లోపల -= len;
    strm->తదుపరి_లోపల += len;
    strm->మొత్తం_లోపల += len;

    /* return no joy or set up to restart inflate() on a new block */
    ఒకవేళ (state->have != 4) తిరిగి Z_దత్తాంశం_ERROR;
    ఒకవేళ (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    లేకపోతే
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    లోపల = strm->మొత్తం_లోపల;  బహిర్గతం = strm->మొత్తం_బహిర్గతం;
    inflateReset(strm);
    strm->మొత్తం_లోపల = లోపల;  strm->మొత్తం_బహిర్గతం = బహిర్గతం;
    state->flags = flags;
    state->బహుళకం = TYPE;
    తిరిగి Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
పూర్ణాంకం ZEXPORT inflate_Sync_బిందువు(z_streamp strm) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    తిరిగి state->బహుళకం == STORED && state->బిట్లు == 0;
}

పూర్ణాంకం ZEXPORT inflateCopy(z_streamp dest, z_streamp ఆధారం) {
    నిర్మాణం inflate_state FAR *state;
    నిర్మాణం inflate_state FAR *copy;
    గుర్తులేని అక్షరం FAR *కిటికీ;

    /* check input */
    ఒకవేళ (inflateStateCheck(ఆధారం) || dest == Z_NULL)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)ఆధారం->state;

    /* allocate space */
    copy = (నిర్మాణం inflate_state FAR *)
           ZALLOC(ఆధారం, 1, పరిమాణం(నిర్మాణం inflate_state));
    ఒకవేళ (copy == Z_NULL) తిరిగి Z_MEM_ERROR;
    zmemzero(copy, పరిమాణం(నిర్మాణం inflate_state));
    కిటికీ = Z_NULL;
    ఒకవేళ (state->కిటికీ != Z_NULL) {
        కిటికీ = (గుర్తులేని అక్షరం FAR *)
                 ZALLOC(ఆధారం, 1U << state->wbits, పరిమాణం(గుర్తులేని అక్షరం));
        ఒకవేళ (కిటికీ == Z_NULL) {
            ZFREE(ఆధారం, copy);
            తిరిగి Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, ఆధారం, పరిమాణం(z_ప్రవాహం));
    zmemcpy(copy, state, పరిమాణం(నిర్మాణం inflate_state));
    copy->strm = dest;
    ఒకవేళ (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->తదుపరి = copy->codes + (state->తదుపరి - state->codes);
    ఒకవేళ (కిటికీ != Z_NULL)
        zmemcpy(కిటికీ, state->కిటికీ, state->whave);
    copy->కిటికీ = కిటికీ;
    dest->state = (నిర్మాణం అంతర్గత_state FAR *)copy;
    తిరిగి Z_OK;
}

పూర్ణాంకం ZEXPORT inflateUndermine(z_streamp strm, పూర్ణాంకం subvert) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
#నిర్వచితమైతే INFLATE_ALLOW_INVALID_దూరం_TOOFAR_ARRR
    state->sane = !subvert;
    తిరిగి Z_OK;
#లేకపోతే
    (శూన్య)subvert;
    state->sane = 1;
    తిరిగి Z_దత్తాంశం_ERROR;
#అయితే_ముగింపు
}

పూర్ణాంకం ZEXPORT inflateValidate(z_streamp strm, పూర్ణాంకం check) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (inflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    ఒకవేళ (check && state->wrap)
        state->wrap |= 4;
    లేకపోతే
        state->wrap &= ~4;
    తిరిగి Z_OK;
}

దీర్ఘ ZEXPORT inflateMark(z_streamp strm) {
    నిర్మాణం inflate_state FAR *state;

    ఒకవేళ (inflateStateCheck(strm))
        తిరిగి -(1L << 16);
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    తిరిగి (దీర్ఘ)(((గుర్తులేని దీర్ఘ)((దీర్ఘ)state->వెనుక)) << 16) +
        (state->బహుళకం == COPY ? state->నిడివి :
            (state->బహుళకం == సరిపోలిక_2 ? state->was - state->నిడివి : 0));
}

గుర్తులేని దీర్ఘ ZEXPORT inflate_Codes_వాడిన(z_streamp strm) {
    నిర్మాణం inflate_state FAR *state;
    ఒకవేళ (inflateStateCheck(strm)) తిరిగి (గుర్తులేని దీర్ఘ)-1;
    state = (నిర్మాణం inflate_state FAR *)strm->state;
    తిరిగి (గుర్తులేని దీర్ఘ)(state->తదుపరి - state->codes);
}
