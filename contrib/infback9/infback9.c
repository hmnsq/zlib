/* infback9.c -- inflate deflate64 data using a call-back interface
 * Copyright (C) 1995-2008 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "zutil.h"
#अन्तर्भाव "infback9.h"
#अन्तर्भाव "inftree9.h"
#अन्तर्भाव "inflate9.h"

#define WSIZE 65536UL

/*
   strm provides memory allocation functions in zalloc and zfree, or
   Z_NULL to use the library memory allocation functions.

   window is a user-supplied window and output buffer that is 64K bytes.
 */
पूर्णाङ्क ZEXPORT inflateBack9Init_(z_stream FAR *strm, unsigned वर्ण FAR *window,
                              ध्रुव वर्ण *version, पूर्णाङ्क stream_size) {
    संरचना inflate_state FAR *state;

    सचे (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (पूर्णाङ्क)(परिमाण(z_stream)))
        पटिनिवत्त Z_VERSION_ERROR;
    सचे (strm == Z_NULL || window == Z_NULL)
        पटिनिवत्त Z_STREAM_ERROR;
    strm->सन्देशः = Z_NULL;                 /* in case we return an error */
    सचे (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    सचे (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (संरचना inflate_state FAR *)ZALLOC(strm, 1,
                                               परिमाण(संरचना inflate_state));
    सचे (state == Z_NULL) पटिनिवत्त Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (voidpf)state;
    state->window = window;
    पटिनिवत्त Z_OK;
}

/*
   Build and output length and distance decoding tables for fixed code
   decoding.
 */
#ifdef MAKEFIXED
#अन्तर्भाव <stdio.h>

शून्य makefixed9(शून्य) {
    unsigned sym, bits, नीचः, size;
    code *next, *lenfix, *distfix;
    संरचना inflate_state state;
    code fixed[544];

    /* literal/length table */
    sym = 0;
    याव (sym < 144) state.lens[sym++] = 8;
    याव (sym < 256) state.lens[sym++] = 9;
    याव (sym < 280) state.lens[sym++] = 7;
    याव (sym < 288) state.lens[sym++] = 8;
    next = fixed;
    lenfix = next;
    bits = 9;
    inflate_table9(LENS, state.lens, 288, &(next), &(bits), state.work);

    /* distance table */
    sym = 0;
    याव (sym < 32) state.lens[sym++] = 5;
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
    नीचः = 0;
    अत्थाय (;;) {
        सचे ((नीचः % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", lenfix[नीचः].op, lenfix[नीचः].bits,
               lenfix[नीचः].val);
        सचे (++नीचः == size) भङ्ग;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    नीचः = 0;
    अत्थाय (;;) {
        सचे ((नीचः % 5) == 0) printf("\n        ");
        printf("{%u,%u,%d}", distfix[नीचः].op, distfix[नीचः].bits,
               distfix[नीचः].val);
        सचे (++नीचः == size) भङ्ग;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/* Macros for inflateBack(): */

/* Clear the input bit accumulator */
#define INITBITS() \
    करोहि { \
        hold = 0; \
        bits = 0; \
    } याव (0)

/* Assure that some input is available.  If input is requested, but denied,
   then return a Z_BUF_ERROR from inflateBack(). */
#define PULL() \
    करोहि { \
        सचे (have == 0) { \
            have = in(in_desc, &next); \
            सचे (have == 0) { \
                next = Z_NULL; \
                ret = Z_BUF_ERROR; \
                गच्छ inf_leave; \
            } \
        } \
    } याव (0)

/* Get a byte of input into the bit accumulator, or return from inflateBack()
   with an error if there is no input available. */
#define PULLBYTE() \
    करोहि { \
        PULL(); \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } याव (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflateBack() with
   an error. */
#define NEEDBITS(n) \
    करोहि { \
        याव (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } याव (0)

/* Return the low n bits of the bit accumulator (n <= 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    करोहि { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } याव (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    करोहि { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } याव (0)

/* Assure that some output space is available, by writing out the window
   if it's full.  If the write fails, return from inflateBack() with a
   Z_BUF_ERROR. */
#define ROOM() \
    करोहि { \
        सचे (वामः == 0) { \
            put = window; \
            वामः = WSIZE; \
            wrap = 1; \
            सचे (out(out_desc, put, (unsigned)वामः)) { \
                ret = Z_BUF_ERROR; \
                गच्छ inf_leave; \
            } \
        } \
    } याव (0)

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
पूर्णाङ्क ZEXPORT inflateBack9(z_stream FAR *strm, in_func in, शून्य FAR *in_desc,
                         out_func out, शून्य FAR *out_desc) {
    संरचना inflate_state FAR *state;
    z_const unsigned वर्ण FAR *next;    /* next input */
    unsigned वर्ण FAR *put;     /* next output */
    unsigned have;              /* available input */
    unsigned long वामः;         /* available output */
    inflate_mode mode;          /* current inflate mode */
    पूर्णाङ्क lastblock;              /* true if processing last block */
    पूर्णाङ्क wrap;                   /* true if the window has wrapped */
    unsigned वर्ण FAR *window;  /* allocated sliding window, if needed */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned extra;             /* extra bits needed */
    unsigned long length;       /* literal or length of data to copy */
    unsigned long offset;       /* distance back to copy string from */
    unsigned long copy;         /* number of stored or match bytes to copy */
    unsigned वर्ण FAR *from;    /* where to copy match bytes from */
    code ध्रुव FAR *lencode;    /* starting table for length/literal codes */
    code ध्रुव FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    पूर्णाङ्क ret;                    /* return code */
    स्थिर ध्रुव unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
#अन्तर्भाव "inffix9.h"

    /* Check that the strm exists and that the state was initialized */
    सचे (strm == Z_NULL || strm->state == Z_NULL)
        पटिनिवत्त Z_STREAM_ERROR;
    state = (संरचना inflate_state FAR *)strm->state;

    /* Reset the state */
    strm->सन्देशः = Z_NULL;
    mode = TYPE;
    lastblock = 0;
    wrap = 0;
    window = state->window;
    next = strm->next_in;
    have = next != Z_NULL ? strm->avail_in : 0;
    hold = 0;
    bits = 0;
    put = window;
    वामः = WSIZE;
    lencode = Z_NULL;
    distcode = Z_NULL;

    /* Inflate until end of block marked as last */
    अत्थाय (;;)
        विकल्प (mode) {
        पक्ष TYPE:
            /* determine and dispatch block type */
            सचे (lastblock) {
                BYTEBITS();
                mode = DONE;
                भङ्ग;
            }
            NEEDBITS(3);
            lastblock = BITS(1);
            DROPBITS(1);
            विकल्प (BITS(2)) {
            पक्ष 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        lastblock ? " (last)" : ""));
                mode = STORED;
                भङ्ग;
            पक्ष 1:                             /* fixed block */
                lencode = lenfix;
                lenbits = 9;
                distcode = distfix;
                distbits = 5;
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = LEN;                     /* decode codes */
                भङ्ग;
            पक्ष 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        lastblock ? " (last)" : ""));
                mode = TABLE;
                भङ्ग;
            पक्ष 3:
                strm->सन्देशः = (z_const वर्ण *)"invalid block type";
                mode = BAD;
            }
            DROPBITS(2);
            भङ्ग;

        पक्ष STORED:
            /* get and verify stored block length */
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            सचे ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid stored block lengths";
                mode = BAD;
                भङ्ग;
            }
            length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %lu\n",
                    length));
            INITBITS();

            /* copy stored block from input to output */
            याव (length != 0) {
                copy = length;
                PULL();
                ROOM();
                सचे (copy > have) copy = have;
                सचे (copy > वामः) copy = वामः;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                वामः -= copy;
                put += copy;
                length -= copy;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            mode = TYPE;
            भङ्ग;

        पक्ष TABLE:
            /* get dynamic table entries descriptor */
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
            सचे (state->nlen > 286) {
                strm->सन्देशः = (z_const वर्ण *)"too many length symbols";
                mode = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       table sizes ok\n"));

            /* get code length code lengths (not a typo) */
            state->have = 0;
            याव (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            याव (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            lencode = (code ध्रुव FAR *)(state->next);
            lenbits = 7;
            ret = inflate_table9(CODES, state->lens, 19, &(state->next),
                                &(lenbits), state->work);
            सचे (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid code lengths set";
                mode = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));

            /* get length and distance code code lengths */
            state->have = 0;
            याव (state->have < state->nlen + state->ndist) {
                अत्थाय (;;) {
                    here = lencode[BITS(lenbits)];
                    सचे ((unsigned)(here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                सचे (here.val < 16) {
                    NEEDBITS(here.bits);
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                अञ्ञथा {
                    सचे (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        सचे (state->have == 0) {
                            strm->सन्देशः = (z_const वर्ण *)"invalid bit length repeat";
                            mode = BAD;
                            भङ्ग;
                        }
                        len = (unsigned)(state->lens[state->have - 1]);
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    अञ्ञथा सचे (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    अञ्ञथा {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    सचे (state->have + copy > state->nlen + state->ndist) {
                        strm->सन्देशः = (z_const वर्ण *)"invalid bit length repeat";
                        mode = BAD;
                        भङ्ग;
                    }
                    याव (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            सचे (mode == BAD) भङ्ग;

            /* check for end-of-block code (better have one) */
            सचे (state->lens[256] == 0) {
                strm->सन्देशः = (z_const वर्ण *)"invalid code -- missing end-of-block";
                mode = BAD;
                भङ्ग;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftree9.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            lencode = (code ध्रुव FAR *)(state->next);
            lenbits = 9;
            ret = inflate_table9(LENS, state->lens, state->nlen,
                            &(state->next), &(lenbits), state->work);
            सचे (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/lengths set";
                mode = BAD;
                भङ्ग;
            }
            distcode = (code ध्रुव FAR *)(state->next);
            distbits = 6;
            ret = inflate_table9(DISTS, state->lens + state->nlen,
                            state->ndist, &(state->next), &(distbits),
                            state->work);
            सचे (ret) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distances set";
                mode = BAD;
                भङ्ग;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            mode = LEN;

        पक्ष LEN:
            /* get a literal, length, or end-of-block code */
            अत्थाय (;;) {
                here = lencode[BITS(lenbits)];
                सचे ((unsigned)(here.bits) <= bits) भङ्ग;
                PULLBYTE();
            }
            सचे (here.op && (here.op & 0xf0) == 0) {
                last = here;
                अत्थाय (;;) {
                    here = lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    सचे ((unsigned)(last.bits + here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            length = (unsigned)here.val;

            /* process literal */
            सचे (here.op == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                ROOM();
                *put++ = (unsigned वर्ण)(length);
                वामः--;
                mode = LEN;
                भङ्ग;
            }

            /* process end of block */
            सचे (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                mode = TYPE;
                भङ्ग;
            }

            /* invalid code */
            सचे (here.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid literal/length code";
                mode = BAD;
                भङ्ग;
            }

            /* length code -- get extra bits, if any */
            extra = (unsigned)(here.op) & 31;
            सचे (extra != 0) {
                NEEDBITS(extra);
                length += BITS(extra);
                DROPBITS(extra);
            }
            Tracevv((stderr, "inflate:         length %lu\n", length));

            /* get distance code */
            अत्थाय (;;) {
                here = distcode[BITS(distbits)];
                सचे ((unsigned)(here.bits) <= bits) भङ्ग;
                PULLBYTE();
            }
            सचे ((here.op & 0xf0) == 0) {
                last = here;
                अत्थाय (;;) {
                    here = distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    सचे ((unsigned)(last.bits + here.bits) <= bits) भङ्ग;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(here.bits);
            सचे (here.op & 64) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance code";
                mode = BAD;
                भङ्ग;
            }
            offset = (unsigned)here.val;

            /* get distance extra bits, if any */
            extra = (unsigned)(here.op) & 15;
            सचे (extra != 0) {
                NEEDBITS(extra);
                offset += BITS(extra);
                DROPBITS(extra);
            }
            सचे (offset > WSIZE - (wrap ? 0: वामः)) {
                strm->सन्देशः = (z_const वर्ण *)"invalid distance too far back";
                mode = BAD;
                भङ्ग;
            }
            Tracevv((stderr, "inflate:         distance %lu\n", offset));

            /* copy match from window to output */
            करोहि {
                ROOM();
                copy = WSIZE - offset;
                सचे (copy < वामः) {
                    from = put + copy;
                    copy = वामः - copy;
                }
                अञ्ञथा {
                    from = put - offset;
                    copy = वामः;
                }
                सचे (copy > length) copy = length;
                length -= copy;
                वामः -= copy;
                करोहि {
                    *put++ = *from++;
                } याव (--copy);
            } याव (length != 0);
            भङ्ग;

        पक्ष DONE:
            /* inflate stream terminated properly -- write leftover output */
            ret = Z_STREAM_END;
            सचे (वामः < WSIZE) {
                सचे (out(out_desc, window, (unsigned)(WSIZE - वामः)))
                    ret = Z_BUF_ERROR;
            }
            गच्छ inf_leave;

        पक्ष BAD:
            ret = Z_DATA_ERROR;
            गच्छ inf_leave;

        स्वभाव:                /* can't happen, but makes compilers happy */
            ret = Z_STREAM_ERROR;
            गच्छ inf_leave;
        }

    /* Return unused input */
  inf_leave:
    strm->next_in = next;
    strm->avail_in = have;
    पटिनिवत्त ret;
}

पूर्णाङ्क ZEXPORT inflateBack9End(z_stream FAR *strm) {
    सचे (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        पटिनिवत्त Z_STREAM_ERROR;
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    पटिनिवत्त Z_OK;
}
