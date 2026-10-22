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

#समावेश "zutil.h"
#समावेश "inftrees.h"
#समावेश "inflate.h"
#समावेश "inffast.h"

local पूर्णांक inflateStateCheck(z_streamp strm) {
    काष्ठा inflate_state FAR *state;
    अगर (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        वापस 1;
    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर (state == Z_NULL || state->strm != strm ||
        state->mode < शीर्ष_2 || state->mode > SYNC)
        वापस 1;
    वापस 0;
}

पूर्णांक ZEXPORT inflateResetKeep(z_streamp strm) {
    काष्ठा inflate_state FAR *state;

    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    strm->योग_in = strm->योग_out = state->योग_2 = 0;
    strm->संदेश = Z_NULL;
    strm->तथ्य_type = 0;
    अगर (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->mode = शीर्ष_2;
    state->last = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->शीर्ष = Z_NULL;
    state->hold = 0;
    state->bits = 0;
    state->lencode = state->distcode = state->अग्र = state->codes;
    state->sane = 1;
    state->back = -1;
    Tracev((मानक_त्रुटि, "inflate: reset\n"));
    वापस Z_OK;
}

पूर्णांक ZEXPORT inflateReset(z_streamp strm) {
    काष्ठा inflate_state FAR *state;

    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    वापस inflateResetKeep(strm);
}

पूर्णांक ZEXPORT inflateReset2(z_streamp strm, पूर्णांक windowBits) {
    पूर्णांक wrap;
    काष्ठा inflate_state FAR *state;

    /* get the state */
    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    अगर (windowBits < 0) {
        अगर (windowBits < -15)
            वापस Z_STREAM_ERROR;
        wrap = 0;
        windowBits = -windowBits;
    }
    अन्यथा {
        wrap = (windowBits >> 4) + 5;
#अगर_घोषित GUNZIP
        अगर (windowBits < 48)
            windowBits &= 15;
#पूर्ण_अगर
    }

    /* set number of window bits, free window if different */
    अगर (windowBits && (windowBits < 8 || windowBits > 15))
        वापस Z_STREAM_ERROR;
    अगर (state->window != Z_NULL && state->wbits != (अचिन्हित)windowBits) {
        ZFREE(strm, state->window);
        state->window = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (अचिन्हित)windowBits;
    वापस inflateReset(strm);
}

पूर्णांक ZEXPORT inflateInit2_(z_streamp strm, पूर्णांक windowBits,
                          स्थिर अक्षर *version, पूर्णांक stream_आकार) {
    पूर्णांक ret;
    काष्ठा inflate_state FAR *state;

    अगर (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_आकार != (पूर्णांक)(माप(z_stream)))
        वापस Z_VERSION_ERROR;
    अगर (strm == Z_NULL) वापस Z_STREAM_ERROR;
    strm->संदेश = Z_NULL;                 /* in case we return an error */
    अगर (strm->zalloc == (alloc_func)0) {
#अगर_घोषित Z_SOLO
        वापस Z_STREAM_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#पूर्ण_अगर
    }
    अगर (strm->zfree == (free_func)0)
#अगर_घोषित Z_SOLO
        वापस Z_STREAM_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#पूर्ण_अगर
    state = (काष्ठा inflate_state FAR *)
            ZALLOC(strm, 1, माप(काष्ठा inflate_state));
    अगर (state == Z_NULL) वापस Z_MEM_ERROR;
    zmemzero(state, माप(काष्ठा inflate_state));
    Tracev((मानक_त्रुटि, "inflate: allocated\n"));
    strm->state = (काष्ठा internal_state FAR *)state;
    state->strm = strm;
    state->window = Z_NULL;
    state->mode = शीर्ष_2;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, windowBits);
    अगर (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    वापस ret;
}

पूर्णांक ZEXPORT inflateInit_(z_streamp strm, स्थिर अक्षर *version,
                         पूर्णांक stream_आकार) {
    वापस inflateInit2_(strm, DEF_WBITS, version, stream_आकार);
}

पूर्णांक ZEXPORT inflate_अभाज्य(z_streamp strm, पूर्णांक bits, पूर्णांक मूल्य) {
    काष्ठा inflate_state FAR *state;

    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    अगर (bits == 0)
        वापस Z_OK;
    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर (bits < 0) {
        state->hold = 0;
        state->bits = 0;
        वापस Z_OK;
    }
    अगर (bits > 16 || state->bits + (uInt)bits > 32) वापस Z_STREAM_ERROR;
    मूल्य &= (1L << bits) - 1;
    state->hold += (अचिन्हित दीर्घ)मूल्य << state->bits;
    state->bits += (uInt)bits;
    वापस Z_OK;
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
local पूर्णांक updatewindow(z_streamp strm, स्थिर Bytef *end, अचिन्हित copy) {
    काष्ठा inflate_state FAR *state;
    अचिन्हित dist;

    state = (काष्ठा inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    अगर (state->window == Z_NULL) {
        state->window = (अचिन्हित अक्षर FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               माप(अचिन्हित अक्षर));
        अगर (state->window == Z_NULL) वापस 1;
    }

    /* if window not in use yet, initialize */
    अगर (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    अगर (copy >= state->wsize) {
        zmemcpy(state->window, end - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    अन्यथा {
        dist = state->wsize - state->wnext;
        अगर (dist > copy) dist = copy;
        zmemcpy(state->window + state->wnext, end - copy, dist);
        copy -= dist;
        अगर (copy) {
            zmemcpy(state->window, end - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        अन्यथा {
            state->wnext += dist;
            अगर (state->wnext == state->wsize) state->wnext = 0;
            अगर (state->whave < state->wsize) state->whave += dist;
        }
    }
    वापस 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#अगर_घोषित GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? crc32(check, buf, len) : adler32(check, buf, len))
#अन्यथा
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#पूर्ण_अगर

/* check macros for header crc */
#अगर_घोषित GUNZIP
#  define CRC2(check, word) \
    करो { \
        hbuf[0] = (अचिन्हित अक्षर)(word); \
        hbuf[1] = (अचिन्हित अक्षर)((word) >> 8); \
        check = crc32(check, hbuf, 2); \
    } जबतक (0)

#  define CRC4(check, word) \
    करो { \
        hbuf[0] = (अचिन्हित अक्षर)(word); \
        hbuf[1] = (अचिन्हित अक्षर)((word) >> 8); \
        hbuf[2] = (अचिन्हित अक्षर)((word) >> 16); \
        hbuf[3] = (अचिन्हित अक्षर)((word) >> 24); \
        check = crc32(check, hbuf, 4); \
    } जबतक (0)
#पूर्ण_अगर

/* Load registers with state in inflate() for speed */
#घोषणा LOAD() \
    करो { \
        put = strm->अग्र_out; \
        बायाँ = strm->avail_out; \
        अग्र = strm->अग्र_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } जबतक (0)

/* Restore state from registers in inflate() */
#घोषणा RESTORE() \
    करो { \
        strm->अग्र_out = put; \
        strm->avail_out = बायाँ; \
        strm->अग्र_in = अग्र; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } जबतक (0)

/* Clear the input bit accumulator */
#घोषणा INITBITS() \
    करो { \
        hold = 0; \
        bits = 0; \
    } जबतक (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#घोषणा PULLBYTE() \
    करो { \
        अगर (have == 0) जाओ inf_leave; \
        have--; \
        hold += (अचिन्हित दीर्घ)(*अग्र++) << bits; \
        bits += 8; \
    } जबतक (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#घोषणा NEEDBITS(n) \
    करो { \
        जबतक (bits < (अचिन्हित)(n)) \
            PULLBYTE(); \
    } जबतक (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#घोषणा BITS(n) \
    ((अचिन्हित)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#घोषणा DROPBITS(n) \
    करो { \
        hold >>= (n); \
        bits -= (अचिन्हित)(n); \
    } जबतक (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#घोषणा BYTEBITS() \
    करो { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } जबतक (0)

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

पूर्णांक ZEXPORT inflate(z_streamp strm, पूर्णांक flush) {
    काष्ठा inflate_state FAR *state;
    z_const अचिन्हित अक्षर FAR *अग्र;    /* next input */
    अचिन्हित अक्षर FAR *put;     /* next output */
    अचिन्हित have, बायाँ;        /* available input and output */
    अचिन्हित दीर्घ hold;         /* bit buffer */
    अचिन्हित bits;              /* bits in bit buffer */
    अचिन्हित in, out;           /* save starting available input and output */
    अचिन्हित copy;              /* number of stored or match bytes to copy */
    अचिन्हित अक्षर FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    अचिन्हित len;               /* length to copy for repeats, bits to drop */
    पूर्णांक ret;                    /* return code */
#अगर_घोषित GUNZIP
    अचिन्हित अक्षर hbuf[4];      /* buffer for gzip header crc calculation */
#पूर्ण_अगर
    अटल स्थिर अचिन्हित लघु order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    अगर (inflateStateCheck(strm) || strm->अग्र_out == Z_NULL ||
        (strm->अग्र_in == Z_NULL && strm->avail_in != 0))
        वापस Z_STREAM_ERROR;

    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर (state->mode == TYPE) state->mode = TYPEDO;      /* skip check */
    LOAD();
    in = have;
    out = बायाँ;
    ret = Z_OK;
    क्रम (;;)
        चयन (state->mode) {
        हाल शीर्ष_2:
            अगर (state->wrap == 0) {
                state->mode = TYPEDO;
                अवरोध;
            }
            NEEDBITS(16);
#अगर_घोषित GUNZIP
            अगर ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                अगर (state->wbits == 0)
                    state->wbits = 15;
                state->check = crc32(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->mode = FLAGS;
                अवरोध;
            }
            अगर (state->शीर्ष != Z_NULL)
                state->शीर्ष->done = -1;
            अगर (!(state->wrap & 1) ||   /* check if zlib header allowed */
#अन्यथा
            अगर (
#पूर्ण_अगर
                ((BITS(8) << 8) + (hold >> 8)) % 31) {
                strm->संदेश = (z_const अक्षर *)"incorrect header check";
                state->mode = BAD;
                अवरोध;
            }
            अगर (BITS(4) != Z_DEFLATED) {
                strm->संदेश = (z_const अक्षर *)"unknown compression method";
                state->mode = BAD;
                अवरोध;
            }
            DROPBITS(4);
            len = BITS(4) + 8;
            अगर (state->wbits == 0)
                state->wbits = len;
            अगर (len > 15 || len > state->wbits) {
                strm->संदेश = (z_const अक्षर *)"invalid window size";
                state->mode = BAD;
                अवरोध;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((मानक_त्रुटि, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            अवरोध;
#अगर_घोषित GUNZIP
        हाल FLAGS:
            NEEDBITS(16);
            state->flags = (पूर्णांक)(hold);
            अगर ((state->flags & 0xff) != Z_DEFLATED) {
                strm->संदेश = (z_const अक्षर *)"unknown compression method";
                state->mode = BAD;
                अवरोध;
            }
            अगर (state->flags & 0xe000) {
                strm->संदेश = (z_const अक्षर *)"unknown header flags set";
                state->mode = BAD;
                अवरोध;
            }
            अगर (state->शीर्ष != Z_NULL)
                state->शीर्ष->text = (पूर्णांक)((hold >> 8) & 1);
            अगर ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = TIME;
                /* fallthrough */
        हाल TIME:
            NEEDBITS(32);
            अगर (state->शीर्ष != Z_NULL)
                state->शीर्ष->समय = hold;
            अगर ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->mode = OS;
                /* fallthrough */
        हाल OS:
            NEEDBITS(16);
            अगर (state->शीर्ष != Z_NULL) {
                state->शीर्ष->xflags = (पूर्णांक)(hold & 0xff);
                state->शीर्ष->os = (पूर्णांक)(hold >> 8);
            }
            अगर ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = EXLEN;
                /* fallthrough */
        हाल EXLEN:
            अगर (state->flags & 0x0400) {
                NEEDBITS(16);
                state->लंबाई = (अचिन्हित)(hold);
                अगर (state->शीर्ष != Z_NULL)
                    state->शीर्ष->extra_len = (अचिन्हित)hold;
                अगर ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            अन्यथा अगर (state->शीर्ष != Z_NULL)
                state->शीर्ष->extra = Z_NULL;
            state->mode = EXTRA;
                /* fallthrough */
        हाल EXTRA:
            अगर (state->flags & 0x0400) {
                copy = state->लंबाई;
                अगर (copy > have) copy = have;
                अगर (copy) {
                    अगर (state->शीर्ष != Z_NULL &&
                        state->शीर्ष->extra != Z_NULL &&
                        (len = state->शीर्ष->extra_len - state->लंबाई) <
                            state->शीर्ष->extra_max) {
                        zmemcpy(state->शीर्ष->extra + len, अग्र,
                                len + copy > state->शीर्ष->extra_max ?
                                state->शीर्ष->extra_max - len : copy);
                    }
                    अगर ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = crc32(state->check, अग्र, copy);
                    have -= copy;
                    अग्र += copy;
                    state->लंबाई -= copy;
                }
                अगर (state->लंबाई) जाओ inf_leave;
            }
            state->लंबाई = 0;
            state->mode = NAME;
                /* fallthrough */
        हाल NAME:
            अगर (state->flags & 0x0800) {
                अगर (have == 0) जाओ inf_leave;
                copy = 0;
                करो {
                    len = (अचिन्हित)(अग्र[copy++]);
                    अगर (state->शीर्ष != Z_NULL &&
                            state->शीर्ष->name != Z_NULL &&
                            state->लंबाई < state->शीर्ष->name_max)
                        state->शीर्ष->name[state->लंबाई++] = (Bytef)len;
                } जबतक (len && copy < have);
                अगर ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, अग्र, copy);
                have -= copy;
                अग्र += copy;
                अगर (len) जाओ inf_leave;
            }
            अन्यथा अगर (state->शीर्ष != Z_NULL)
                state->शीर्ष->name = Z_NULL;
            state->लंबाई = 0;
            state->mode = COMMENT;
                /* fallthrough */
        हाल COMMENT:
            अगर (state->flags & 0x1000) {
                अगर (have == 0) जाओ inf_leave;
                copy = 0;
                करो {
                    len = (अचिन्हित)(अग्र[copy++]);
                    अगर (state->शीर्ष != Z_NULL &&
                            state->शीर्ष->comment != Z_NULL &&
                            state->लंबाई < state->शीर्ष->comm_max)
                        state->शीर्ष->comment[state->लंबाई++] = (Bytef)len;
                } जबतक (len && copy < have);
                अगर ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, अग्र, copy);
                have -= copy;
                अग्र += copy;
                अगर (len) जाओ inf_leave;
            }
            अन्यथा अगर (state->शीर्ष != Z_NULL)
                state->शीर्ष->comment = Z_NULL;
            state->mode = HCRC;
                /* fallthrough */
        हाल HCRC:
            अगर (state->flags & 0x0200) {
                NEEDBITS(16);
                अगर ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->संदेश = (z_const अक्षर *)"header crc mismatch";
                    state->mode = BAD;
                    अवरोध;
                }
                INITBITS();
            }
            अगर (state->शीर्ष != Z_NULL) {
                state->शीर्ष->hcrc = (पूर्णांक)((state->flags >> 9) & 1);
                state->शीर्ष->done = 1;
            }
            strm->adler = state->check = crc32(0L, Z_NULL, 0);
            state->mode = TYPE;
            अवरोध;
#पूर्ण_अगर
        हाल DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->mode = DICT;
                /* fallthrough */
        हाल DICT:
            अगर (state->havedict == 0) {
                RESTORE();
                वापस Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = TYPE;
                /* fallthrough */
        हाल TYPE:
            अगर (flush == Z_BLOCK || flush == Z_TREES) जाओ inf_leave;
                /* fallthrough */
        हाल TYPEDO:
            अगर (state->last) {
                BYTEBITS();
                state->mode = CHECK;
                अवरोध;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            चयन (BITS(2)) {
            हाल 0:                             /* stored block */
                Tracev((मानक_त्रुटि, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                अवरोध;
            हाल 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((मानक_त्रुटि, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN_;             /* decode codes */
                अगर (flush == Z_TREES) {
                    DROPBITS(2);
                    जाओ inf_leave;
                }
                अवरोध;
            हाल 2:                             /* dynamic block */
                Tracev((मानक_त्रुटि, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                अवरोध;
            शेष:
                strm->संदेश = (z_const अक्षर *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            अवरोध;
        हाल STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            अगर ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->संदेश = (z_const अक्षर *)"invalid stored block lengths";
                state->mode = BAD;
                अवरोध;
            }
            state->लंबाई = (अचिन्हित)hold & 0xffff;
            Tracev((मानक_त्रुटि, "inflate:       stored length %u\n",
                    state->लंबाई));
            INITBITS();
            state->mode = COPY_;
            अगर (flush == Z_TREES) जाओ inf_leave;
                /* fallthrough */
        हाल COPY_:
            state->mode = COPY;
                /* fallthrough */
        हाल COPY:
            copy = state->लंबाई;
            अगर (copy) {
                अगर (copy > have) copy = have;
                अगर (copy > बायाँ) copy = बायाँ;
                अगर (copy == 0) जाओ inf_leave;
                zmemcpy(put, अग्र, copy);
                have -= copy;
                अग्र += copy;
                बायाँ -= copy;
                put += copy;
                state->लंबाई -= copy;
                अवरोध;
            }
            Tracev((मानक_त्रुटि, "inflate:       stored end\n"));
            state->mode = TYPE;
            अवरोध;
        हाल TABLE:
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#अगर_अघोषित PKZIP_BUG_WORKAROUND
            अगर (state->nlen > 286 || state->ndist > 30) {
                strm->संदेश = (z_const अक्षर *)
                    "too many length or distance symbols";
                state->mode = BAD;
                अवरोध;
            }
#पूर्ण_अगर
            Tracev((मानक_त्रुटि, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->mode = LENLENS;
                /* fallthrough */
        हाल LENLENS:
            जबतक (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (अचिन्हित लघु)BITS(3);
                DROPBITS(3);
            }
            जबतक (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->अग्र = state->codes;
            state->lencode = state->distcode = (स्थिर code FAR *)(state->अग्र);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->अग्र),
                                &(state->lenbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid code lengths set";
                state->mode = BAD;
                अवरोध;
            }
            Tracev((मानक_त्रुटि, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->mode = CODELENS;
                /* fallthrough */
        हाल CODELENS:
            जबतक (state->have < state->nlen + state->ndist) {
                क्रम (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                अगर (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अन्यथा {
                    अगर (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        अगर (state->have == 0) {
                            strm->संदेश = (z_const अक्षर *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            अवरोध;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    अन्यथा अगर (here.val == 17) {
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
                    अगर (state->have + copy > state->nlen + state->ndist) {
                        strm->संदेश = (z_const अक्षर *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        अवरोध;
                    }
                    जबतक (copy--)
                        state->lens[state->have++] = (अचिन्हित लघु)len;
                }
            }

            /* handle error breaks in while */
            अगर (state->mode == BAD) अवरोध;

            /* check for end-of-block code (better have one) */
            अगर (state->lens[256] == 0) {
                strm->संदेश = (z_const अक्षर *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                अवरोध;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->अग्र = state->codes;
            state->lencode = (स्थिर code FAR *)(state->अग्र);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->अग्र),
                                &(state->lenbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/lengths set";
                state->mode = BAD;
                अवरोध;
            }
            state->distcode = (स्थिर code FAR *)(state->अग्र);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->अग्र), &(state->distbits), state->work);
            अगर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid distances set";
                state->mode = BAD;
                अवरोध;
            }
            Tracev((मानक_त्रुटि, "inflate:       codes ok\n"));
            state->mode = LEN_;
            अगर (flush == Z_TREES) जाओ inf_leave;
                /* fallthrough */
        हाल LEN_:
            state->mode = LEN;
                /* fallthrough */
        हाल LEN:
            अगर (have >= 6 && बायाँ >= 258) {
                RESTORE();
                inflate_fast(strm, out);
                LOAD();
                अगर (state->mode == TYPE)
                    state->back = -1;
                अवरोध;
            }
            state->back = 0;
            क्रम (;;) {
                here = state->lencode[BITS(state->lenbits)];
                अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                PULLBYTE();
            }
            अगर (here.op && (here.op & 0xf0) == 0) {
                last = here;
                क्रम (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    अगर ((अचिन्हित)(last.bits + here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            state->लंबाई = (अचिन्हित)here.val;
            अगर ((पूर्णांक)(here.op) == 0) {
                Tracevv((मानक_त्रुटि, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                state->mode = LIT;
                अवरोध;
            }
            अगर (here.op & 32) {
                Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
                state->back = -1;
                state->mode = TYPE;
                अवरोध;
            }
            अगर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/length code";
                state->mode = BAD;
                अवरोध;
            }
            state->extra = (अचिन्हित)(here.op) & 15;
            state->mode = LENEXT;
                /* fallthrough */
        हाल LENEXT:
            अगर (state->extra) {
                NEEDBITS(state->extra);
                state->लंबाई += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %u\n", state->लंबाई));
            state->was = state->लंबाई;
            state->mode = DIST;
                /* fallthrough */
        हाल DIST:
            क्रम (;;) {
                here = state->distcode[BITS(state->distbits)];
                अगर ((अचिन्हित)(here.bits) <= bits) अवरोध;
                PULLBYTE();
            }
            अगर ((here.op & 0xf0) == 0) {
                last = here;
                क्रम (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    अगर ((अचिन्हित)(last.bits + here.bits) <= bits) अवरोध;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            अगर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid distance code";
                state->mode = BAD;
                अवरोध;
            }
            state->offset = (अचिन्हित)here.val;
            state->extra = (अचिन्हित)(here.op) & 15;
            state->mode = DISTEXT;
                /* fallthrough */
        हाल DISTEXT:
            अगर (state->extra) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
#अगर_घोषित INFLATE_STRICT
            अगर (state->offset > state->dmax) {
                strm->संदेश = (z_const अक्षर *)"invalid distance too far back";
                state->mode = BAD;
                अवरोध;
            }
#पूर्ण_अगर
            Tracevv((मानक_त्रुटि, "inflate:         distance %u\n", state->offset));
            state->mode = MATCH;
                /* fallthrough */
        हाल MATCH:
            अगर (बायाँ == 0) जाओ inf_leave;
            copy = out - बायाँ;
            अगर (state->offset > copy) {         /* copy from window */
                copy = state->offset - copy;
                अगर (copy > state->whave) {
                    अगर (state->sane) {
                        strm->संदेश = (z_const अक्षर *)
                            "invalid distance too far back";
                        state->mode = BAD;
                        अवरोध;
                    }
#अगर_घोषित INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                    Trace((मानक_त्रुटि, "inflate.c too far\n"));
                    copy -= state->whave;
                    अगर (copy > state->लंबाई) copy = state->लंबाई;
                    अगर (copy > बायाँ) copy = बायाँ;
                    बायाँ -= copy;
                    state->लंबाई -= copy;
                    करो {
                        *put++ = 0;
                    } जबतक (--copy);
                    अगर (state->लंबाई == 0) state->mode = LEN;
                    अवरोध;
#पूर्ण_अगर
                }
                अगर (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->window + (state->wsize - copy);
                }
                अन्यथा
                    from = state->window + (state->wnext - copy);
                अगर (copy > state->लंबाई) copy = state->लंबाई;
            }
            अन्यथा {                              /* copy from output */
                from = put - state->offset;
                copy = state->लंबाई;
            }
            अगर (copy > बायाँ) copy = बायाँ;
            बायाँ -= copy;
            state->लंबाई -= copy;
            करो {
                *put++ = *from++;
            } जबतक (--copy);
            अगर (state->लंबाई == 0) state->mode = LEN;
            अवरोध;
        हाल LIT:
            अगर (बायाँ == 0) जाओ inf_leave;
            *put++ = (अचिन्हित अक्षर)(state->लंबाई);
            बायाँ--;
            state->mode = LEN;
            अवरोध;
        हाल CHECK:
            अगर (state->wrap) {
                NEEDBITS(32);
                out -= बायाँ;
                strm->योग_out += out;
                state->योग_2 += out;
                अगर ((state->wrap & 4) && out)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - out, out);
                out = बायाँ;
                अगर ((state->wrap & 4) && (
#अगर_घोषित GUNZIP
                     state->flags ? hold :
#पूर्ण_अगर
                     ZSWAP32(hold)) != state->check) {
                    strm->संदेश = (z_const अक्षर *)"incorrect data check";
                    state->mode = BAD;
                    अवरोध;
                }
                INITBITS();
                Tracev((मानक_त्रुटि, "inflate:   check matches trailer\n"));
            }
#अगर_घोषित GUNZIP
            state->mode = लंबाई_2;
                /* fallthrough */
        हाल लंबाई_2:
            अगर (state->wrap && state->flags) {
                NEEDBITS(32);
                अगर ((state->wrap & 4) && hold != (state->योग_2 & 0xffffffff)) {
                    strm->संदेश = (z_const अक्षर *)"incorrect length check";
                    state->mode = BAD;
                    अवरोध;
                }
                INITBITS();
                Tracev((मानक_त्रुटि, "inflate:   length matches trailer\n"));
            }
#पूर्ण_अगर
            state->mode = DONE;
                /* fallthrough */
        हाल DONE:
            ret = Z_STREAM_END;
            जाओ inf_leave;
        हाल BAD:
            ret = Z_तथ्य_ERROR;
            जाओ inf_leave;
        हाल MEM:
            वापस Z_MEM_ERROR;
        हाल SYNC:
                /* fallthrough */
        शेष:
            वापस Z_STREAM_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    अगर (state->wsize || (out != strm->avail_out && state->mode < BAD &&
            (state->mode < CHECK || flush != Z_FINISH)))
        अगर (updatewindow(strm, strm->अग्र_out, out - strm->avail_out)) {
            state->mode = MEM;
            वापस Z_MEM_ERROR;
        }
    in -= strm->avail_in;
    out -= strm->avail_out;
    strm->योग_in += in;
    strm->योग_out += out;
    state->योग_2 += out;
    अगर ((state->wrap & 4) && out)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->अग्र_out - out, out);
    strm->तथ्य_type = (पूर्णांक)state->bits + (state->last ? 64 : 0) +
                      (state->mode == TYPE ? 128 : 0) +
                      (state->mode == LEN_ || state->mode == COPY_ ? 256 : 0);
    अगर (((in == 0 && out == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    वापस ret;
}

पूर्णांक ZEXPORT inflateEnd(z_streamp strm) {
    काष्ठा inflate_state FAR *state;
    अगर (inflateStateCheck(strm))
        वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर (state->window != Z_NULL) ZFREE(strm, state->window);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((मानक_त्रुटि, "inflate: end\n"));
    वापस Z_OK;
}

पूर्णांक ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_लंबाई) {
    काष्ठा inflate_state FAR *state;

    /* check state */
    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;

    /* copy dictionary */
    अगर (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->window + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->window, state->wnext);
    }
    अगर (dict_लंबाई != Z_NULL)
        *dict_लंबाई = state->whave;
    वापस Z_OK;
}

पूर्णांक ZEXPORT inflateSetDictionary(z_streamp strm, स्थिर Bytef *dictionary,
                                 uInt dict_लंबाई) {
    काष्ठा inflate_state FAR *state;
    अचिन्हित दीर्घ dictid;
    पूर्णांक ret;

    /* check state */
    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर (state->wrap != 0 && state->mode != DICT)
        वापस Z_STREAM_ERROR;

    /* check for correct dictionary identifier */
    अगर (state->mode == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dict_लंबाई);
        अगर (dictid != state->check)
            वापस Z_तथ्य_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dict_लंबाई, dict_लंबाई);
    अगर (ret) {
        state->mode = MEM;
        वापस Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((मानक_त्रुटि, "inflate:   dictionary set\n"));
    वापस Z_OK;
}

पूर्णांक ZEXPORT inflateGetHeader(z_streamp strm, gz_headerp शीर्ष) {
    काष्ठा inflate_state FAR *state;

    /* check state */
    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर ((state->wrap & 2) == 0) वापस Z_STREAM_ERROR;

    /* save header structure */
    state->शीर्ष = शीर्ष;
    शीर्ष->done = 0;
    वापस Z_OK;
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
local अचिन्हित syncsearch(अचिन्हित FAR *have, स्थिर अचिन्हित अक्षर FAR *buf,
                          अचिन्हित len) {
    अचिन्हित got;
    अचिन्हित अग्र;

    got = *have;
    अग्र = 0;
    जबतक (अग्र < len && got < 4) {
        अगर ((पूर्णांक)(buf[अग्र]) == (got < 2 ? 0 : 0xff))
            got++;
        अन्यथा अगर (buf[अग्र])
            got = 0;
        अन्यथा
            got = 4 - got;
        अग्र++;
    }
    *have = got;
    वापस अग्र;
}

पूर्णांक ZEXPORT inflateSync(z_streamp strm) {
    अचिन्हित len;               /* number of bytes to look at or looked at */
    पूर्णांक flags;                  /* temporary to save header status */
    अचिन्हित दीर्घ in, out;      /* temporary to save total_in and total_out */
    अचिन्हित अक्षर buf[4];       /* to restore bit buffer to byte string */
    काष्ठा inflate_state FAR *state;

    /* check parameters */
    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर (strm->avail_in == 0 && state->bits < 8) वापस Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    अगर (state->mode != SYNC) {
        state->mode = SYNC;
        state->hold >>= state->bits & 7;
        state->bits -= state->bits & 7;
        len = 0;
        जबतक (state->bits >= 8) {
            buf[len++] = (अचिन्हित अक्षर)(state->hold);
            state->hold >>= 8;
            state->bits -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->अग्र_in, strm->avail_in);
    strm->avail_in -= len;
    strm->अग्र_in += len;
    strm->योग_in += len;

    /* return no joy or set up to restart inflate() on a new block */
    अगर (state->have != 4) वापस Z_तथ्य_ERROR;
    अगर (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    अन्यथा
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    in = strm->योग_in;  out = strm->योग_out;
    inflateReset(strm);
    strm->योग_in = in;  strm->योग_out = out;
    state->flags = flags;
    state->mode = TYPE;
    वापस Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
पूर्णांक ZEXPORT inflateSyncPoint(z_streamp strm) {
    काष्ठा inflate_state FAR *state;

    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    वापस state->mode == STORED && state->bits == 0;
}

पूर्णांक ZEXPORT inflateCopy(z_streamp dest, z_streamp source) {
    काष्ठा inflate_state FAR *state;
    काष्ठा inflate_state FAR *copy;
    अचिन्हित अक्षर FAR *window;

    /* check input */
    अगर (inflateStateCheck(source) || dest == Z_NULL)
        वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)source->state;

    /* allocate space */
    copy = (काष्ठा inflate_state FAR *)
           ZALLOC(source, 1, माप(काष्ठा inflate_state));
    अगर (copy == Z_NULL) वापस Z_MEM_ERROR;
    zmemzero(copy, माप(काष्ठा inflate_state));
    window = Z_NULL;
    अगर (state->window != Z_NULL) {
        window = (अचिन्हित अक्षर FAR *)
                 ZALLOC(source, 1U << state->wbits, माप(अचिन्हित अक्षर));
        अगर (window == Z_NULL) {
            ZFREE(source, copy);
            वापस Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, source, माप(z_stream));
    zmemcpy(copy, state, माप(काष्ठा inflate_state));
    copy->strm = dest;
    अगर (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->अग्र = copy->codes + (state->अग्र - state->codes);
    अगर (window != Z_NULL)
        zmemcpy(window, state->window, state->whave);
    copy->window = window;
    dest->state = (काष्ठा internal_state FAR *)copy;
    वापस Z_OK;
}

पूर्णांक ZEXPORT inflateUndermine(z_streamp strm, पूर्णांक subvert) {
    काष्ठा inflate_state FAR *state;

    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
#अगर_घोषित INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
    state->sane = !subvert;
    वापस Z_OK;
#अन्यथा
    (व्योम)subvert;
    state->sane = 1;
    वापस Z_तथ्य_ERROR;
#पूर्ण_अगर
}

पूर्णांक ZEXPORT inflateValidate(z_streamp strm, पूर्णांक check) {
    काष्ठा inflate_state FAR *state;

    अगर (inflateStateCheck(strm)) वापस Z_STREAM_ERROR;
    state = (काष्ठा inflate_state FAR *)strm->state;
    अगर (check && state->wrap)
        state->wrap |= 4;
    अन्यथा
        state->wrap &= ~4;
    वापस Z_OK;
}

दीर्घ ZEXPORT inflateMark(z_streamp strm) {
    काष्ठा inflate_state FAR *state;

    अगर (inflateStateCheck(strm))
        वापस -(1L << 16);
    state = (काष्ठा inflate_state FAR *)strm->state;
    वापस (दीर्घ)(((अचिन्हित दीर्घ)((दीर्घ)state->back)) << 16) +
        (state->mode == COPY ? state->लंबाई :
            (state->mode == MATCH ? state->was - state->लंबाई : 0));
}

अचिन्हित दीर्घ ZEXPORT inflateCodesUsed(z_streamp strm) {
    काष्ठा inflate_state FAR *state;
    अगर (inflateStateCheck(strm)) वापस (अचिन्हित दीर्घ)-1;
    state = (काष्ठा inflate_state FAR *)strm->state;
    वापस (अचिन्हित दीर्घ)(state->अग्र - state->codes);
}
