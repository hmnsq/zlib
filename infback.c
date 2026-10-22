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

#അന്തര്ഭാവ "zutil.h"
#അന്തര്ഭാവ "inftrees.h"
#അന്തര്ഭാവ "inflate.h"
#അന്തര്ഭാവ "inffast.h"

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   windowBits is in the range 8..15, and window is a user-supplied
   window and output buffer that is 2**windowBits bytes.
 */
പൂർണ്ണസംഖ്യ ZEXPORT inflateBackInit_(z_streamp strm, പൂർണ്ണസംഖ്യ windowBits,
                             unsigned അക്ഷരം FAR *window, സ്ഥിരം അക്ഷരം *version,
                             പൂർണ്ണസംഖ്യ stream_size) {
    ഘടന inflate_state FAR *state;

    എങ്കിൽ (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (പൂർണ്ണസംഖ്യ)(വലുപ്പം(z_stream)))
        തിരികെ Z_VERSION_ERROR;
    എങ്കിൽ (strm == Z_NULL || window == Z_NULL ||
        windowBits < 8 || windowBits > 15)
        തിരികെ Z_STREAM_ERROR;
    strm->സന്ദേശം = Z_NULL;                 /* in case we return an error */
    എങ്കിൽ (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        തിരികെ Z_STREAM_ERROR;
#അല്ലെങ്കിൽ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    എങ്കിൽ (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        തിരികെ Z_STREAM_ERROR;
#അല്ലെങ്കിൽ
        strm->zfree = zcfree;
#endif
    state = (ഘടന inflate_state FAR *)ZALLOC(strm, 1,
                                               വലുപ്പം(ഘടന inflate_state));
    എങ്കിൽ (state == Z_NULL) തിരികെ Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (ഘടന internal_state FAR *)state;
    state->dmax = 32768U;
    state->wbits = (uInt)windowBits;
    state->wsize = 1U << windowBits;
    state->window = window;
    state->wnext = 0;
    state->whave = 0;
    state->sane = 1;
    തിരികെ Z_OK;
}

/* Macros for inflateBack(): */

/* Load returned state from inflate_fast() */
#define LOAD() \
    ചെയ്യുക { \
        put = strm->next_out; \
        ഇടത് = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } ആകുമ്പോൾ (0)

/* Set state from registers for inflate_fast() */
#define RESTORE() \
    ചെയ്യുക { \
        strm->next_out = put; \
        strm->avail_out = ഇടത്; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } ആകുമ്പോൾ (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    ചെയ്യുക { \
        hold = 0; \
        bits = 0; \
    } ആകുമ്പോൾ (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    ചെയ്യുക { \
        എങ്കിൽ (have == 0) { \
            have = in(in_desc, &next); \
            എങ്കിൽ (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                പോകുക inf_leave; \
            } \
        } \
    } ആകുമ്പോൾ (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    ചെയ്യുക { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } ആകുമ്പോൾ (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    ചെയ്യുക { \
        ആകുമ്പോൾ (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } ആകുമ്പോൾ (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    ചെയ്യുക { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } ആകുമ്പോൾ (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    ചെയ്യുക { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } ആകുമ്പോൾ (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    ചെയ്യുക { \
        എങ്കിൽ (ഇടത് == 0) { \
            put = state->window; \
            ഇടത് = state->wsize; \
            state->whave = ഇടത്; \
            എങ്കിൽ (out(out_desc, put, ഇടത്)) { \
                ret = Z_BUF_ERROR; \
                പോകുക inf_leave; \
            } \
        } \
    } ആകുമ്പോൾ (0)

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
പൂർണ്ണസംഖ്യ ZEXPORT inflateBack(z_streamp strm, in_func in, ശൂന്യം FAR *in_desc,
                        out_func out, ശൂന്യം FAR *out_desc) {
    ഘടന inflate_state FAR *state;
    z_const unsigned അക്ഷരം FAR *next;    /* next input */
    unsigned അക്ഷരം FAR *put;     /* next output */
    unsigned have, ഇടത്;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned അക്ഷരം FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    പൂർണ്ണസംഖ്യ ret;                    /* return code */
    സ്ഥിതം സ്ഥിരം unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* Check that the strm exists and that the state was initialized */
    എങ്കിൽ (strm == Z_NULL || strm->state == Z_NULL)
        തിരികെ Z_STREAM_ERROR;
    state = (ഘടന inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->സന്ദേശം = Z_NULL;
    state->mode = TYPE;
    state->last = 0;
    state->whave = 0;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = state->window;
    ഇടത് = state->wsize;

    /* Inflate until end of block marked as last */
    വേണ്ടി (;;)
        തിരഞ്ഞെടുപ്പ് (state->mode) {
        അവസ്ഥ TYPE:
            /* determine and dispatch block type */
            എങ്കിൽ (state->last) {
                BYTEBITS();
                state->mode = DONE;
                നിർത്തുക;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            തിരഞ്ഞെടുപ്പ് (BITS(2)) {
            അവസ്ഥ 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                നിർത്തുക;
            അവസ്ഥ 1:                             /* fixed block */
                inflate_fixed(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                നിർത്തുക;
            അവസ്ഥ 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            നിർത്തുക;

        അവസ്ഥ STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            എങ്കിൽ ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid stored block lengths";
                state->mode = BAD;
                നിർത്തുക;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();

            /* copy stored block from input to output */
            ആകുമ്പോൾ (state->length != 0) {
                copy = state->length;
                PULL();
                ROOM();
                എങ്കിൽ (copy > have) copy = have;
                എങ്കിൽ (copy > ഇടത്) copy = ഇടത്;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                ഇടത് -= copy;
                put += copy;
                state->length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            നിർത്തുക;

        അവസ്ഥ TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            എങ്കിൽ (state->nlen > 286 || state->ndist > 30) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)
                    "too many length or distance symbols";
                state->mode = BAD;
                നിർത്തുക;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            ആകുമ്പോൾ (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            ആകുമ്പോൾ (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code സ്ഥിരം FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            എങ്കിൽ (ret) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid code lengths set";
                state->mode = BAD;
                നിർത്തുക;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            ആകുമ്പോൾ (state->have < state->nlen + state->ndist) {
                വേണ്ടി (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    എങ്കിൽ ((unsigned)(here.bits) <= bits) നിർത്തുക;
                    PULLBYTE();
                }
                എങ്കിൽ (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                അല്ലെങ്കിൽ {
                    എങ്കിൽ (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        എങ്കിൽ (state->have == 0) {
                            strm->സന്ദേശം = (z_const അക്ഷരം *)
                                "invalid bit length repeat";
                            state->mode = BAD;
                            നിർത്തുക;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    അല്ലെങ്കിൽ എങ്കിൽ (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    അല്ലെങ്കിൽ {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    എങ്കിൽ (state->have + copy > state->nlen + state->ndist) {
                        strm->സന്ദേശം = (z_const അക്ഷരം *)
                            "invalid bit length repeat";
                        state->mode = BAD;
                        നിർത്തുക;
                    }
                    ആകുമ്പോൾ (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            എങ്കിൽ (state->mode == BAD) നിർത്തുക;

            /* check for end-of-block code (better have one) */
            എങ്കിൽ (state->lens[256] == 0) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)
                    "invalid code -- missing end-of-block";
                state->mode = BAD;
                നിർത്തുക;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (code സ്ഥിരം FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            എങ്കിൽ (ret) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid literal/lengths set";
                state->mode = BAD;
                നിർത്തുക;
            }
            state->distcode = (code സ്ഥിരം FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            എങ്കിൽ (ret) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid distances set";
                state->mode = BAD;
                നിർത്തുക;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
                /* fallthrough */

        അവസ്ഥ LEN:
            /* use inflate_fast() if we have enough input and output */
            എങ്കിൽ (have >= 6 && ഇടത് >= 258) {
                RESTORE();
                inflate_fast(strm, state->wsize);
                LOAD();
                നിർത്തുക;
            }

            /* get a literal, length, or end-of-block code */
            വേണ്ടി (;;) {
                here = state->lencode[BITS(state->lenbits)];
                എങ്കിൽ ((unsigned)(here.bits) <= bits) നിർത്തുക;
                PULLBYTE();
            }
            എങ്കിൽ (here.op && (here.op & 0xf0) == 0) {
                last = here;
                വേണ്ടി (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    എങ്കിൽ ((unsigned)(last.bits + here.bits) <= bits) നിർത്തുക;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            state->length = (unsigned)here.val;

            /* process literal */
            എങ്കിൽ (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned അക്ഷരം)(state->length);
                ഇടത്--;
                state->mode = LEN;
                നിർത്തുക;
            }

            /* process end of block */
            എങ്കിൽ (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                നിർത്തുക;
            }

            /* invalid code */
            എങ്കിൽ (here.op & 64) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid literal/length code";
                state->mode = BAD;
                നിർത്തുക;
            }

            /* length code -- get extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            എങ്കിൽ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));

            /* get distance code */
            വേണ്ടി (;;) {
                here = state->distcode[BITS(state->distbits)];
                എങ്കിൽ ((unsigned)(here.bits) <= bits) നിർത്തുക;
                PULLBYTE();
            }
            എങ്കിൽ ((here.op & 0xf0) == 0) {
                last = here;
                വേണ്ടി (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    എങ്കിൽ ((unsigned)(last.bits + here.bits) <= bits) നിർത്തുക;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            എങ്കിൽ (here.op & 64) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid distance code";
                state->mode = BAD;
                നിർത്തുക;
            }
            state->offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            state->extra = (unsigned)(here.op) & 15;
            എങ്കിൽ (state->extra != 0) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
            എങ്കിൽ (state->offset > state->wsize - (state->whave < state->wsize ?
                                                ഇടത് : 0)) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid distance too far back";
                state->mode = BAD;
                നിർത്തുക;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));

            /* copy match from window to output */
            ചെയ്യുക {
                ROOM();
                copy = state->wsize - state->offset;
                എങ്കിൽ (copy < ഇടത്) {
                    from = put + copy;
                    copy = ഇടത് - copy;
                }
                അല്ലെങ്കിൽ {
                    from = put - state->offset;
                    copy = ഇടത്;
                }
                എങ്കിൽ (copy > state->length) copy = state->length;
                state->length -= copy;
                ഇടത് -= copy;
                ചെയ്യുക {
                    *put++ = *from++;
                } ആകുമ്പോൾ (--copy);
            } ആകുമ്പോൾ (state->length != 0);
            നിർത്തുക;

        അവസ്ഥ DONE:
            /* inflate stream terminated properly */
            ret = Z_STREAM_END;
            പോകുക inf_leave;

        അവസ്ഥ BAD:
            ret = Z_DATA_ERROR;
            പോകുക inf_leave;

        സ്ഥിരസ്ഥിതി:
            /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            പോകുക inf_leave;
        }

    /* Write leftover output and return unused input */
  inf_leave:
    എങ്കിൽ (ഇടത് < state->wsize) {
        എങ്കിൽ (out(out_desc, state->window, state->wsize - ഇടത്) &&
            ret == Z_STREAM_END)
            ret = Z_BUF_ERROR;
    }
    strm->next_in = next;
    strm->avail_in = have;
    തിരികെ ret;
}

പൂർണ്ണസംഖ്യ ZEXPORT inflateBackEnd(z_streamp strm) {
    എങ്കിൽ (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        തിരികെ Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    തിരികെ Z_OK;
}
