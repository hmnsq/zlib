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

local पूर्णांक inflateStateCheck(z_streamp strm) {
    संरचना inflate_state FAR *state;
    जर (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        परत 1;
    state = (संरचना inflate_state FAR *)strm->state;
    जर (state == Z_NULL || state->strm != strm ||
        state->mode < HEAD || state->mode > SYNC)
        परत 1;
    परत 0;
}

पूर्णांक ZEXPORT inflateResetKeep(z_streamp strm) {
    संरचना inflate_state FAR *state;

    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    strm->total_in = strm->total_out = state->total = 0;
    strm->संदेश = Z_NULL;
    strm->data_type = 0;
    जर (state->wrap)        /* to support ill-conceived Java test suite */
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
    Tracev((मानक_त्रुटि, "inflate: reset\n"));
    परत Z_OK;
}

पूर्णांक ZEXPORT inflateReset(z_streamp strm) {
    संरचना inflate_state FAR *state;

    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    परत inflateResetKeep(strm);
}

पूर्णांक ZEXPORT inflateReset2(z_streamp strm, पूर्णांक windowBits) {
    पूर्णांक wrap;
    संरचना inflate_state FAR *state;

    /* get the state */
    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    जर (windowBits < 0) {
        जर (windowBits < -15)
            परत Z_STREAM_ERROR;
        wrap = 0;
        windowBits = -windowBits;
    }
    अन्यथा {
        wrap = (windowBits >> 4) + 5;
#ifdef GUNZIP
        जर (windowBits < 48)
            windowBits &= 15;
#endif
    }

    /* set number of window bits, free window if different */
    जर (windowBits && (windowBits < 8 || windowBits > 15))
        परत Z_STREAM_ERROR;
    जर (state->window != Z_NULL && state->wbits != (अचिन्हित)windowBits) {
        ZFREE(strm, state->window);
        state->window = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (अचिन्हित)windowBits;
    परत inflateReset(strm);
}

पूर्णांक ZEXPORT inflateInit2_(z_streamp strm, पूर्णांक windowBits,
                          स्थिरांक अक्षर *version, पूर्णांक stream_size) {
    पूर्णांक ret;
    संरचना inflate_state FAR *state;

    जर (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (पूर्णांक)(आकार(z_stream)))
        परत Z_VERSION_ERROR;
    जर (strm == Z_NULL) परत Z_STREAM_ERROR;
    strm->संदेश = Z_NULL;                 /* in case we return an error */
    जर (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        परत Z_STREAM_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    जर (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        परत Z_STREAM_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#endif
    state = (संरचना inflate_state FAR *)
            ZALLOC(strm, 1, आकार(संरचना inflate_state));
    जर (state == Z_NULL) परत Z_MEM_ERROR;
    zmemzero(state, आकार(संरचना inflate_state));
    Tracev((मानक_त्रुटि, "inflate: allocated\n"));
    strm->state = (संरचना internal_state FAR *)state;
    state->strm = strm;
    state->window = Z_NULL;
    state->mode = HEAD;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, windowBits);
    जर (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    परत ret;
}

पूर्णांक ZEXPORT inflateInit_(z_streamp strm, स्थिरांक अक्षर *version,
                         पूर्णांक stream_size) {
    परत inflateInit2_(strm, DEF_WBITS, version, stream_size);
}

पूर्णांक ZEXPORT inflate_मूळसंख्या(z_streamp strm, पूर्णांक bits, पूर्णांक मूल्य) {
    संरचना inflate_state FAR *state;

    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    जर (bits == 0)
        परत Z_OK;
    state = (संरचना inflate_state FAR *)strm->state;
    जर (bits < 0) {
        state->hold = 0;
        state->bits = 0;
        परत Z_OK;
    }
    जर (bits > 16 || state->bits + (uInt)bits > 32) परत Z_STREAM_ERROR;
    मूल्य &= (1L << bits) - 1;
    state->hold += (अचिन्हित दीर्घ)मूल्य << state->bits;
    state->bits += (uInt)bits;
    परत Z_OK;
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
local पूर्णांक updatewindow(z_streamp strm, स्थिरांक Bytef *end, अचिन्हित copy) {
    संरचना inflate_state FAR *state;
    अचिन्हित dist;

    state = (संरचना inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    जर (state->window == Z_NULL) {
        state->window = (अचिन्हित अक्षर FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               आकार(अचिन्हित अक्षर));
        जर (state->window == Z_NULL) परत 1;
    }

    /* if window not in use yet, initialize */
    जर (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    जर (copy >= state->wsize) {
        zmemcpy(state->window, end - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    अन्यथा {
        dist = state->wsize - state->wnext;
        जर (dist > copy) dist = copy;
        zmemcpy(state->window + state->wnext, end - copy, dist);
        copy -= dist;
        जर (copy) {
            zmemcpy(state->window, end - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        अन्यथा {
            state->wnext += dist;
            जर (state->wnext == state->wsize) state->wnext = 0;
            जर (state->whave < state->wsize) state->whave += dist;
        }
    }
    परत 0;
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
    करा { \
        hbuf[0] = (अचिन्हित अक्षर)(word); \
        hbuf[1] = (अचिन्हित अक्षर)((word) >> 8); \
        check = crc32(check, hbuf, 2); \
    } पर्यंत (0)

#  define CRC4(check, word) \
    करा { \
        hbuf[0] = (अचिन्हित अक्षर)(word); \
        hbuf[1] = (अचिन्हित अक्षर)((word) >> 8); \
        hbuf[2] = (अचिन्हित अक्षर)((word) >> 16); \
        hbuf[3] = (अचिन्हित अक्षर)((word) >> 24); \
        check = crc32(check, hbuf, 4); \
    } पर्यंत (0)
#endif

/* Load registers with state in inflate() for speed */
#define LOAD() \
    करा { \
        put = strm->next_out; \
        डावा = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } पर्यंत (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
    करा { \
        strm->next_out = put; \
        strm->avail_out = डावा; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } पर्यंत (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    करा { \
        hold = 0; \
        bits = 0; \
    } पर्यंत (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#define PULLBYTE() \
    करा { \
        जर (have == 0) जा inf_leave; \
        have--; \
        hold += (अचिन्हित दीर्घ)(*next++) << bits; \
        bits += 8; \
    } पर्यंत (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#define NEEDBITS(n) \
    करा { \
        पर्यंत (bits < (अचिन्हित)(n)) \
            PULLBYTE(); \
    } पर्यंत (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((अचिन्हित)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    करा { \
        hold >>= (n); \
        bits -= (अचिन्हित)(n); \
    } पर्यंत (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    करा { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } पर्यंत (0)

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
    संरचना inflate_state FAR *state;
    z_const अचिन्हित अक्षर FAR *next;    /* next input */
    अचिन्हित अक्षर FAR *put;     /* next output */
    अचिन्हित have, डावा;        /* available input and output */
    अचिन्हित दीर्घ hold;         /* bit buffer */
    अचिन्हित bits;              /* bits in bit buffer */
    अचिन्हित in, out;           /* save starting available input and output */
    अचिन्हित copy;              /* number of stored or match bytes to copy */
    अचिन्हित अक्षर FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    अचिन्हित len;               /* length to copy for repeats, bits to drop */
    पूर्णांक ret;                    /* return code */
#ifdef GUNZIP
    अचिन्हित अक्षर hbuf[4];      /* buffer for gzip header crc calculation */
#endif
    स्थिर स्थिरांक अचिन्हित लघु order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    जर (inflateStateCheck(strm) || strm->next_out == Z_NULL ||
        (strm->next_in == Z_NULL && strm->avail_in != 0))
        परत Z_STREAM_ERROR;

    state = (संरचना inflate_state FAR *)strm->state;
    जर (state->mode == TYPE) state->mode = TYPEDO;      /* skip check */
    LOAD();
    in = have;
    out = डावा;
    ret = Z_OK;
    साठी (;;)
        निवड (state->mode) {
        स्थिती HEAD:
            जर (state->wrap == 0) {
                state->mode = TYPEDO;
                थांबा;
            }
            NEEDBITS(16);
#ifdef GUNZIP
            जर ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                जर (state->wbits == 0)
                    state->wbits = 15;
                state->check = crc32(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->mode = FLAGS;
                थांबा;
            }
            जर (state->head != Z_NULL)
                state->head->done = -1;
            जर (!(state->wrap & 1) ||   /* check if zlib header allowed */
#अन्यथा
            जर (
#endif
                ((BITS(8) << 8) + (hold >> 8)) % 31) {
                strm->संदेश = (z_const अक्षर *)"incorrect header check";
                state->mode = BAD;
                थांबा;
            }
            जर (BITS(4) != Z_DEFLATED) {
                strm->संदेश = (z_const अक्षर *)"unknown compression method";
                state->mode = BAD;
                थांबा;
            }
            DROPBITS(4);
            len = BITS(4) + 8;
            जर (state->wbits == 0)
                state->wbits = len;
            जर (len > 15 || len > state->wbits) {
                strm->संदेश = (z_const अक्षर *)"invalid window size";
                state->mode = BAD;
                थांबा;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((मानक_त्रुटि, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            थांबा;
#ifdef GUNZIP
        स्थिती FLAGS:
            NEEDBITS(16);
            state->flags = (पूर्णांक)(hold);
            जर ((state->flags & 0xff) != Z_DEFLATED) {
                strm->संदेश = (z_const अक्षर *)"unknown compression method";
                state->mode = BAD;
                थांबा;
            }
            जर (state->flags & 0xe000) {
                strm->संदेश = (z_const अक्षर *)"unknown header flags set";
                state->mode = BAD;
                थांबा;
            }
            जर (state->head != Z_NULL)
                state->head->text = (पूर्णांक)((hold >> 8) & 1);
            जर ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = TIME;
                /* fallthrough */
        स्थिती TIME:
            NEEDBITS(32);
            जर (state->head != Z_NULL)
                state->head->समय = hold;
            जर ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->mode = OS;
                /* fallthrough */
        स्थिती OS:
            NEEDBITS(16);
            जर (state->head != Z_NULL) {
                state->head->xflags = (पूर्णांक)(hold & 0xff);
                state->head->os = (पूर्णांक)(hold >> 8);
            }
            जर ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = EXLEN;
                /* fallthrough */
        स्थिती EXLEN:
            जर (state->flags & 0x0400) {
                NEEDBITS(16);
                state->length = (अचिन्हित)(hold);
                जर (state->head != Z_NULL)
                    state->head->extra_len = (अचिन्हित)hold;
                जर ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            अन्यथा जर (state->head != Z_NULL)
                state->head->extra = Z_NULL;
            state->mode = EXTRA;
                /* fallthrough */
        स्थिती EXTRA:
            जर (state->flags & 0x0400) {
                copy = state->length;
                जर (copy > have) copy = have;
                जर (copy) {
                    जर (state->head != Z_NULL &&
                        state->head->extra != Z_NULL &&
                        (len = state->head->extra_len - state->length) <
                            state->head->extra_max) {
                        zmemcpy(state->head->extra + len, next,
                                len + copy > state->head->extra_max ?
                                state->head->extra_max - len : copy);
                    }
                    जर ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = crc32(state->check, next, copy);
                    have -= copy;
                    next += copy;
                    state->length -= copy;
                }
                जर (state->length) जा inf_leave;
            }
            state->length = 0;
            state->mode = NAME;
                /* fallthrough */
        स्थिती NAME:
            जर (state->flags & 0x0800) {
                जर (have == 0) जा inf_leave;
                copy = 0;
                करा {
                    len = (अचिन्हित)(next[copy++]);
                    जर (state->head != Z_NULL &&
                            state->head->name != Z_NULL &&
                            state->length < state->head->name_max)
                        state->head->name[state->length++] = (Bytef)len;
                } पर्यंत (len && copy < have);
                जर ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                जर (len) जा inf_leave;
            }
            अन्यथा जर (state->head != Z_NULL)
                state->head->name = Z_NULL;
            state->length = 0;
            state->mode = COMMENT;
                /* fallthrough */
        स्थिती COMMENT:
            जर (state->flags & 0x1000) {
                जर (have == 0) जा inf_leave;
                copy = 0;
                करा {
                    len = (अचिन्हित)(next[copy++]);
                    जर (state->head != Z_NULL &&
                            state->head->comment != Z_NULL &&
                            state->length < state->head->comm_max)
                        state->head->comment[state->length++] = (Bytef)len;
                } पर्यंत (len && copy < have);
                जर ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                जर (len) जा inf_leave;
            }
            अन्यथा जर (state->head != Z_NULL)
                state->head->comment = Z_NULL;
            state->mode = HCRC;
                /* fallthrough */
        स्थिती HCRC:
            जर (state->flags & 0x0200) {
                NEEDBITS(16);
                जर ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->संदेश = (z_const अक्षर *)"header crc mismatch";
                    state->mode = BAD;
                    थांबा;
                }
                INITBITS();
            }
            जर (state->head != Z_NULL) {
                state->head->hcrc = (पूर्णांक)((state->flags >> 9) & 1);
                state->head->done = 1;
            }
            strm->adler = state->check = crc32(0L, Z_NULL, 0);
            state->mode = TYPE;
            थांबा;
#endif
        स्थिती DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->mode = DICT;
                /* fallthrough */
        स्थिती DICT:
            जर (state->havedict == 0) {
                RESTORE();
                परत Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = TYPE;
                /* fallthrough */
        स्थिती TYPE:
            जर (flush == Z_BLOCK || flush == Z_TREES) जा inf_leave;
                /* fallthrough */
        स्थिती TYPEDO:
            जर (state->last) {
                BYTEBITS();
                state->mode = CHECK;
                थांबा;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            निवड (BITS(2)) {
            स्थिती 0:                             /* stored block */
                Tracev((मानक_त्रुटि, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                थांबा;
            स्थिती 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((मानक_त्रुटि, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN_;             /* decode codes */
                जर (flush == Z_TREES) {
                    DROPBITS(2);
                    जा inf_leave;
                }
                थांबा;
            स्थिती 2:                             /* dynamic block */
                Tracev((मानक_त्रुटि, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                थांबा;
            पूर्वनिर्धारित:
                strm->संदेश = (z_const अक्षर *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            थांबा;
        स्थिती STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            जर ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->संदेश = (z_const अक्षर *)"invalid stored block lengths";
                state->mode = BAD;
                थांबा;
            }
            state->length = (अचिन्हित)hold & 0xffff;
            Tracev((मानक_त्रुटि, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();
            state->mode = COPY_;
            जर (flush == Z_TREES) जा inf_leave;
                /* fallthrough */
        स्थिती COPY_:
            state->mode = COPY;
                /* fallthrough */
        स्थिती COPY:
            copy = state->length;
            जर (copy) {
                जर (copy > have) copy = have;
                जर (copy > डावा) copy = डावा;
                जर (copy == 0) जा inf_leave;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                डावा -= copy;
                put += copy;
                state->length -= copy;
                थांबा;
            }
            Tracev((मानक_त्रुटि, "inflate:       stored end\n"));
            state->mode = TYPE;
            थांबा;
        स्थिती TABLE:
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            जर (state->nlen > 286 || state->ndist > 30) {
                strm->संदेश = (z_const अक्षर *)
                    "too many length or distance symbols";
                state->mode = BAD;
                थांबा;
            }
#endif
            Tracev((मानक_त्रुटि, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->mode = LENLENS;
                /* fallthrough */
        स्थिती LENLENS:
            पर्यंत (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (अचिन्हित लघु)BITS(3);
                DROPBITS(3);
            }
            पर्यंत (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = state->distcode = (स्थिरांक code FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid code lengths set";
                state->mode = BAD;
                थांबा;
            }
            Tracev((मानक_त्रुटि, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->mode = CODELENS;
                /* fallthrough */
        स्थिती CODELENS:
            पर्यंत (state->have < state->nlen + state->ndist) {
                साठी (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                जर (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अन्यथा {
                    जर (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        जर (state->have == 0) {
                            strm->संदेश = (z_const अक्षर *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            थांबा;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    अन्यथा जर (here.val == 17) {
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
                    जर (state->have + copy > state->nlen + state->ndist) {
                        strm->संदेश = (z_const अक्षर *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        थांबा;
                    }
                    पर्यंत (copy--)
                        state->lens[state->have++] = (अचिन्हित लघु)len;
                }
            }

            /* handle error breaks in while */
            जर (state->mode == BAD) थांबा;

            /* check for end-of-block code (better have one) */
            जर (state->lens[256] == 0) {
                strm->संदेश = (z_const अक्षर *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                थांबा;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (स्थिरांक code FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/lengths set";
                state->mode = BAD;
                थांबा;
            }
            state->distcode = (स्थिरांक code FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            जर (ret) {
                strm->संदेश = (z_const अक्षर *)"invalid distances set";
                state->mode = BAD;
                थांबा;
            }
            Tracev((मानक_त्रुटि, "inflate:       codes ok\n"));
            state->mode = LEN_;
            जर (flush == Z_TREES) जा inf_leave;
                /* fallthrough */
        स्थिती LEN_:
            state->mode = LEN;
                /* fallthrough */
        स्थिती LEN:
            जर (have >= 6 && डावा >= 258) {
                RESTORE();
                inflate_fast(strm, out);
                LOAD();
                जर (state->mode == TYPE)
                    state->back = -1;
                थांबा;
            }
            state->back = 0;
            साठी (;;) {
                here = state->lencode[BITS(state->lenbits)];
                जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                PULLBYTE();
            }
            जर (here.op && (here.op & 0xf0) == 0) {
                last = here;
                साठी (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    जर ((अचिन्हित)(last.bits + here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            state->length = (अचिन्हित)here.val;
            जर ((पूर्णांक)(here.op) == 0) {
                Tracevv((मानक_त्रुटि, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                state->mode = LIT;
                थांबा;
            }
            जर (here.op & 32) {
                Tracevv((मानक_त्रुटि, "inflate:         end of block\n"));
                state->back = -1;
                state->mode = TYPE;
                थांबा;
            }
            जर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid literal/length code";
                state->mode = BAD;
                थांबा;
            }
            state->extra = (अचिन्हित)(here.op) & 15;
            state->mode = LENEXT;
                /* fallthrough */
        स्थिती LENEXT:
            जर (state->extra) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
            Tracevv((मानक_त्रुटि, "inflate:         length %u\n", state->length));
            state->was = state->length;
            state->mode = DIST;
                /* fallthrough */
        स्थिती DIST:
            साठी (;;) {
                here = state->distcode[BITS(state->distbits)];
                जर ((अचिन्हित)(here.bits) <= bits) थांबा;
                PULLBYTE();
            }
            जर ((here.op & 0xf0) == 0) {
                last = here;
                साठी (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    जर ((अचिन्हित)(last.bits + here.bits) <= bits) थांबा;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            जर (here.op & 64) {
                strm->संदेश = (z_const अक्षर *)"invalid distance code";
                state->mode = BAD;
                थांबा;
            }
            state->offset = (अचिन्हित)here.val;
            state->extra = (अचिन्हित)(here.op) & 15;
            state->mode = DISTEXT;
                /* fallthrough */
        स्थिती DISTEXT:
            जर (state->extra) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
#ifdef INFLATE_STRICT
            जर (state->offset > state->dmax) {
                strm->संदेश = (z_const अक्षर *)"invalid distance too far back";
                state->mode = BAD;
                थांबा;
            }
#endif
            Tracevv((मानक_त्रुटि, "inflate:         distance %u\n", state->offset));
            state->mode = MATCH;
                /* fallthrough */
        स्थिती MATCH:
            जर (डावा == 0) जा inf_leave;
            copy = out - डावा;
            जर (state->offset > copy) {         /* copy from window */
                copy = state->offset - copy;
                जर (copy > state->whave) {
                    जर (state->sane) {
                        strm->संदेश = (z_const अक्षर *)
                            "invalid distance too far back";
                        state->mode = BAD;
                        थांबा;
                    }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                    Trace((मानक_त्रुटि, "inflate.c too far\n"));
                    copy -= state->whave;
                    जर (copy > state->length) copy = state->length;
                    जर (copy > डावा) copy = डावा;
                    डावा -= copy;
                    state->length -= copy;
                    करा {
                        *put++ = 0;
                    } पर्यंत (--copy);
                    जर (state->length == 0) state->mode = LEN;
                    थांबा;
#endif
                }
                जर (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->window + (state->wsize - copy);
                }
                अन्यथा
                    from = state->window + (state->wnext - copy);
                जर (copy > state->length) copy = state->length;
            }
            अन्यथा {                              /* copy from output */
                from = put - state->offset;
                copy = state->length;
            }
            जर (copy > डावा) copy = डावा;
            डावा -= copy;
            state->length -= copy;
            करा {
                *put++ = *from++;
            } पर्यंत (--copy);
            जर (state->length == 0) state->mode = LEN;
            थांबा;
        स्थिती LIT:
            जर (डावा == 0) जा inf_leave;
            *put++ = (अचिन्हित अक्षर)(state->length);
            डावा--;
            state->mode = LEN;
            थांबा;
        स्थिती CHECK:
            जर (state->wrap) {
                NEEDBITS(32);
                out -= डावा;
                strm->total_out += out;
                state->total += out;
                जर ((state->wrap & 4) && out)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - out, out);
                out = डावा;
                जर ((state->wrap & 4) && (
#ifdef GUNZIP
                     state->flags ? hold :
#endif
                     ZSWAP32(hold)) != state->check) {
                    strm->संदेश = (z_const अक्षर *)"incorrect data check";
                    state->mode = BAD;
                    थांबा;
                }
                INITBITS();
                Tracev((मानक_त्रुटि, "inflate:   check matches trailer\n"));
            }
#ifdef GUNZIP
            state->mode = LENGTH;
                /* fallthrough */
        स्थिती LENGTH:
            जर (state->wrap && state->flags) {
                NEEDBITS(32);
                जर ((state->wrap & 4) && hold != (state->total & 0xffffffff)) {
                    strm->संदेश = (z_const अक्षर *)"incorrect length check";
                    state->mode = BAD;
                    थांबा;
                }
                INITBITS();
                Tracev((मानक_त्रुटि, "inflate:   length matches trailer\n"));
            }
#endif
            state->mode = DONE;
                /* fallthrough */
        स्थिती DONE:
            ret = Z_STREAM_END;
            जा inf_leave;
        स्थिती BAD:
            ret = Z_DATA_ERROR;
            जा inf_leave;
        स्थिती MEM:
            परत Z_MEM_ERROR;
        स्थिती SYNC:
                /* fallthrough */
        पूर्वनिर्धारित:
            परत Z_STREAM_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    जर (state->wsize || (out != strm->avail_out && state->mode < BAD &&
            (state->mode < CHECK || flush != Z_FINISH)))
        जर (updatewindow(strm, strm->next_out, out - strm->avail_out)) {
            state->mode = MEM;
            परत Z_MEM_ERROR;
        }
    in -= strm->avail_in;
    out -= strm->avail_out;
    strm->total_in += in;
    strm->total_out += out;
    state->total += out;
    जर ((state->wrap & 4) && out)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->next_out - out, out);
    strm->data_type = (पूर्णांक)state->bits + (state->last ? 64 : 0) +
                      (state->mode == TYPE ? 128 : 0) +
                      (state->mode == LEN_ || state->mode == COPY_ ? 256 : 0);
    जर (((in == 0 && out == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    परत ret;
}

पूर्णांक ZEXPORT inflateEnd(z_streamp strm) {
    संरचना inflate_state FAR *state;
    जर (inflateStateCheck(strm))
        परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    जर (state->window != Z_NULL) ZFREE(strm, state->window);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((मानक_त्रुटि, "inflate: end\n"));
    परत Z_OK;
}

पूर्णांक ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dictLength) {
    संरचना inflate_state FAR *state;

    /* check state */
    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* copy dictionary */
    जर (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->window + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->window, state->wnext);
    }
    जर (dictLength != Z_NULL)
        *dictLength = state->whave;
    परत Z_OK;
}

पूर्णांक ZEXPORT inflateSetDictionary(z_streamp strm, स्थिरांक Bytef *dictionary,
                                 uInt dictLength) {
    संरचना inflate_state FAR *state;
    अचिन्हित दीर्घ dictid;
    पूर्णांक ret;

    /* check state */
    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    जर (state->wrap != 0 && state->mode != DICT)
        परत Z_STREAM_ERROR;

    /* check for correct dictionary identifier */
    जर (state->mode == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dictLength);
        जर (dictid != state->check)
            परत Z_DATA_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dictLength, dictLength);
    जर (ret) {
        state->mode = MEM;
        परत Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((मानक_त्रुटि, "inflate:   dictionary set\n"));
    परत Z_OK;
}

पूर्णांक ZEXPORT inflateGetHeader(z_streamp strm, gz_headerp head) {
    संरचना inflate_state FAR *state;

    /* check state */
    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    जर ((state->wrap & 2) == 0) परत Z_STREAM_ERROR;

    /* save header structure */
    state->head = head;
    head->done = 0;
    परत Z_OK;
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
local अचिन्हित syncsearch(अचिन्हित FAR *have, स्थिरांक अचिन्हित अक्षर FAR *buf,
                          अचिन्हित len) {
    अचिन्हित got;
    अचिन्हित next;

    got = *have;
    next = 0;
    पर्यंत (next < len && got < 4) {
        जर ((पूर्णांक)(buf[next]) == (got < 2 ? 0 : 0xff))
            got++;
        अन्यथा जर (buf[next])
            got = 0;
        अन्यथा
            got = 4 - got;
        next++;
    }
    *have = got;
    परत next;
}

पूर्णांक ZEXPORT inflateSync(z_streamp strm) {
    अचिन्हित len;               /* number of bytes to look at or looked at */
    पूर्णांक flags;                  /* temporary to save header status */
    अचिन्हित दीर्घ in, out;      /* temporary to save total_in and total_out */
    अचिन्हित अक्षर buf[4];       /* to restore bit buffer to byte string */
    संरचना inflate_state FAR *state;

    /* check parameters */
    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    जर (strm->avail_in == 0 && state->bits < 8) परत Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    जर (state->mode != SYNC) {
        state->mode = SYNC;
        state->hold >>= state->bits & 7;
        state->bits -= state->bits & 7;
        len = 0;
        पर्यंत (state->bits >= 8) {
            buf[len++] = (अचिन्हित अक्षर)(state->hold);
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
    जर (state->have != 4) परत Z_DATA_ERROR;
    जर (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    अन्यथा
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    in = strm->total_in;  out = strm->total_out;
    inflateReset(strm);
    strm->total_in = in;  strm->total_out = out;
    state->flags = flags;
    state->mode = TYPE;
    परत Z_OK;
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
    संरचना inflate_state FAR *state;

    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    परत state->mode == STORED && state->bits == 0;
}

पूर्णांक ZEXPORT inflateCopy(z_streamp dest, z_streamp source) {
    संरचना inflate_state FAR *state;
    संरचना inflate_state FAR *copy;
    अचिन्हित अक्षर FAR *window;

    /* check input */
    जर (inflateStateCheck(source) || dest == Z_NULL)
        परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)source->state;

    /* allocate space */
    copy = (संरचना inflate_state FAR *)
           ZALLOC(source, 1, आकार(संरचना inflate_state));
    जर (copy == Z_NULL) परत Z_MEM_ERROR;
    zmemzero(copy, आकार(संरचना inflate_state));
    window = Z_NULL;
    जर (state->window != Z_NULL) {
        window = (अचिन्हित अक्षर FAR *)
                 ZALLOC(source, 1U << state->wbits, आकार(अचिन्हित अक्षर));
        जर (window == Z_NULL) {
            ZFREE(source, copy);
            परत Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy(dest, source, आकार(z_stream));
    zmemcpy(copy, state, आकार(संरचना inflate_state));
    copy->strm = dest;
    जर (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->next = copy->codes + (state->next - state->codes);
    जर (window != Z_NULL)
        zmemcpy(window, state->window, state->whave);
    copy->window = window;
    dest->state = (संरचना internal_state FAR *)copy;
    परत Z_OK;
}

पूर्णांक ZEXPORT inflateUndermine(z_streamp strm, पूर्णांक subvert) {
    संरचना inflate_state FAR *state;

    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
    state->sane = !subvert;
    परत Z_OK;
#अन्यथा
    (शून्य)subvert;
    state->sane = 1;
    परत Z_DATA_ERROR;
#endif
}

पूर्णांक ZEXPORT inflateValidate(z_streamp strm, पूर्णांक check) {
    संरचना inflate_state FAR *state;

    जर (inflateStateCheck(strm)) परत Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;
    जर (check && state->wrap)
        state->wrap |= 4;
    अन्यथा
        state->wrap &= ~4;
    परत Z_OK;
}

दीर्घ ZEXPORT inflateMark(z_streamp strm) {
    संरचना inflate_state FAR *state;

    जर (inflateStateCheck(strm))
        परत -(1L << 16);
    state = (संरचना inflate_state FAR *)strm->state;
    परत (दीर्घ)(((अचिन्हित दीर्घ)((दीर्घ)state->back)) << 16) +
        (state->mode == COPY ? state->length :
            (state->mode == MATCH ? state->was - state->length : 0));
}

अचिन्हित दीर्घ ZEXPORT inflateCodesUsed(z_streamp strm) {
    संरचना inflate_state FAR *state;
    जर (inflateStateCheck(strm)) परत (अचिन्हित दीर्घ)-1;
    state = (संरचना inflate_state FAR *)strm->state;
    परत (अचिन्हित दीर्घ)(state->next - state->codes);
}
