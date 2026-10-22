/* blast.c
 * Copyright (C) 2003, 2012, 2013 Mark Adler
 * For conditions of distribution and use, see copyright notice in blast.h
 * version 1.3, 24 Aug 2013
 *
 * blast.c decompresses data compressed by the PKWare Compression Library.
 * This function provides functionality similar to the explode() function of
 * the PKWare library, hence the name "blast".
 *
 * This decompressor is based on the excellent format description provided by
 * Ben Rudiak-Gould in comp.compression on August 13, 2001.  Interestingly, the
 * example Ben provided in the post is incorrect.  The distance 110001 should
 * instead be 111000.  When corrected, the example byte stream becomes:
 *
 *    00 04 82 24 25 8f 80 7f
 *
 * which decompresses to "AIAIAIAIAIAIA" (without the quotes).
 */

/*
 * Change history:
 *
 * 1.0  12 Feb 2003     - First version
 * 1.1  16 Feb 2003     - Fixed distance check for > 4 GB uncompressed data
 * 1.2  24 Oct 2012     - Add note about using binary mode in stdio
 *                      - Fix comparisons of differently signed integers
 * 1.3  24 Aug 2013     - Return unused input from blast()
 *                      - Fix test code to correctly report unused input
 *                      - Enable the provision of initial input to blast()
 */

#ضمن <stddef.h>             /* for NULL */
#ضمن <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#ضمن "blast.h"              /* prototype for blast() */

#تعریف_کن MAXBITS 13              /* maximum code length */
#تعریف_کن MAXWIN 4096             /* maximum window size */

/* input and output state */
بنية state {
    /* input state */
    blast_درون infun;             /* input function provided by user */
    فراغ *inhow;                /* opaque information passed to infun() */
    بیعلامت حرف *درون;          /* next input location */
    بیعلامت چپ;              /* available input at in */
    صحيح bitbuf;                 /* bit buffer */
    صحيح bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;

    /* output state */
    blast_بیرون outfun;           /* output function provided by user */
    فراغ *outhow;               /* opaque information passed to outfun() */
    بیعلامت بعدی;              /* index of next write location in out[] */
    صحيح اولی;                  /* true to check distances (for first 4K) */
    بیعلامت حرف بیرون[MAXWIN];  /* output buffer and sliding window */
};

/*
 * Return need bits from the input stream.  This always leaves less than
 * eight bits in the buffer.  bits() works properly for need == 0.
 *
 * Format notes:
 *
 * - Bits are stored in bytes from the least significant bit to the most
 *   significant bit.  Therefore bits are dropped from the bottom of the bit
 *   buffer, using shift right, and new bytes are appended to the top of the
 *   bit buffer, using shift left.
 */
local صحيح بیتها(بنية state *s, صحيح need)
{
    صحيح val;            /* bit accumulator */

    /* load at least need bits into val */
    val = s->bitbuf;
    طالما (s->bitcnt < need) {
        إذا (s->چپ == 0) {
            s->چپ = s->infun(s->inhow, &(s->درون));
            إذا (s->چپ == 0) longjmp(s->env, 1);       /* out of input */
        }
        val |= (صحيح)(*(s->درون)++) << s->bitcnt;          /* load eight bits */
        s->چپ--;
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = val >> need;
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    أرجع val & ((1 << need) - 1);
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
بنية huffman {
    کوتاه *شمار;       /* number of symbols of each length */
    کوتاه *نماد;      /* canonically ordered symbols */
};

/*
 * Decode a code from the stream s using huffman table h.  Return the symbol or
 * a negative value if there is an error.  If all of the lengths are zero, i.e.
 * an empty code, or if the code is incomplete and an invalid code is received,
 * then -9 is returned after reading MAXBITS bits.
 *
 * Format notes:
 *
 * - The codes as stored in the compressed data are bit-reversed relative to
 *   a simple integer ordering of codes of the same lengths.  Hence below the
 *   bits are pulled from the compressed data one at a time and used to
 *   build the code value reversed from what is in the stream in order to
 *   permit simple integer comparisons for decoding.
 *
 * - The first code for the shortest length is all ones.  Subsequent codes of
 *   the same length are simply integer decrements of the previous code.  When
 *   moving up a length, a one bit is appended to the code.  For a complete
 *   code, the last code of the longest length will be all zeros.  To support
 *   this ordering, the bits pulled during decoding are inverted to apply the
 *   more "natural" ordering starting with all zeros and incrementing.
 */
local صحيح decode(بنية state *s, بنية huffman *h)
{
    صحيح len;            /* current number of bits in code */
    صحيح کد;           /* len bits being decoded */
    صحيح اولی;          /* first code of length len */
    صحيح شمار;          /* number of codes of length len */
    صحيح شاخص;          /* index of first code of length len in symbol table */
    صحيح bitbuf;         /* bits from stream */
    صحيح چپ;           /* bits left in next or left to process */
    کوتاه *بعدی;        /* next number of codes */

    bitbuf = s->bitbuf;
    چپ = s->bitcnt;
    کد = اولی = شاخص = 0;
    len = 1;
    بعدی = h->شمار + 1;
    طالما (1) {
        طالما (چپ--) {
            کد |= (bitbuf & 1) ^ 1;   /* invert code */
            bitbuf >>= 1;
            شمار = *بعدی++;
            إذا (کد < اولی + شمار) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                أرجع h->نماد[شاخص + (کد - اولی)];
            }
            شاخص += شمار;             /* else update for next length */
            اولی += شمار;
            اولی <<= 1;
            کد <<= 1;
            len++;
        }
        چپ = (MAXBITS+1) - len;
        إذا (چپ == 0) اكسر;
        إذا (s->چپ == 0) {
            s->چپ = s->infun(s->inhow, &(s->درون));
            إذا (s->چپ == 0) longjmp(s->env, 1);       /* out of input */
        }
        bitbuf = *(s->درون)++;
        s->چپ--;
        إذا (چپ > 8) چپ = 8;
    }
    أرجع -9;                          /* ran out of codes */
}

/*
 * Given a list of repeated code lengths rep[0..n-1], where each byte is a
 * count (high four bits + 1) and a code length (low four bits), generate the
 * list of code lengths.  This compaction reduces the size of the object code.
 * Then given the list of code lengths length[0..n-1] representing a canonical
 * Huffman code for n symbols, construct the tables required to decode those
 * codes.  Those tables are the number of codes of each length, and the symbols
 * sorted by length, retaining their original order within each length.  The
 * return value is zero for a complete code set, negative for an over-
 * subscribed code set, and positive for an incomplete code set.  The tables
 * can be used if the return value is zero or positive, but they cannot be used
 * if the return value is negative.  If the return value is zero, it is not
 * possible for decode() using that table to return an error--any stream of
 * enough bits will resolve to a symbol.  If the return value is positive, then
 * it is possible for decode() using that table to return an error for received
 * codes past the end of the incomplete lengths.
 */
local صحيح construct(بنية huffman *h, ثابت بیعلامت حرف *rep, صحيح n)
{
    صحيح نماد;         /* current symbol when stepping through length[] */
    صحيح len;            /* current length when stepping through h->count[] */
    صحيح چپ;           /* number of possible codes left of current length */
    کوتاه offs[MAXBITS+1];      /* offsets in symbol table for each length */
    کوتاه درازی[256];  /* code lengths */

    /* convert compact repeat counts into symbol bit length list */
    نماد = 0;
    افعل {
        len = *rep++;
        چپ = (len >> 4) + 1;
        len &= 15;
        افعل {
            درازی[نماد++] = len;
        } طالما (--چپ);
    } طالما (--n);
    n = نماد;

    /* count number of codes of each length */
    لكل (len = 0; len <= MAXBITS; len++)
        h->شمار[len] = 0;
    لكل (نماد = 0; نماد < n; نماد++)
        (h->شمار[درازی[نماد]])++;   /* assumes lengths are within bounds */
    إذا (h->شمار[0] == n)               /* no codes! */
        أرجع 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    چپ = 1;                           /* one possible code of zero length */
    لكل (len = 1; len <= MAXBITS; len++) {
        چپ <<= 1;                     /* one more bit, double codes left */
        چپ -= h->شمار[len];          /* deduct count from possible codes */
        إذا (چپ < 0) أرجع چپ;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    لكل (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->شمار[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    لكل (نماد = 0; نماد < n; نماد++)
        إذا (درازی[نماد] != 0)
            h->نماد[offs[درازی[نماد]]++] = نماد;

    /* return zero for complete set, positive for incomplete set */
    أرجع چپ;
}

/*
 * Decode PKWare Compression Library stream.
 *
 * Format notes:
 *
 * - First byte is 0 if literals are uncoded or 1 if they are coded.  Second
 *   byte is 4, 5, or 6 for the number of extra bits in the distance code.
 *   This is the base-2 logarithm of the dictionary size minus six.
 *
 * - Compressed data is a combination of literals and length/distance pairs
 *   terminated by an end code.  Literals are either Huffman coded or
 *   uncoded bytes.  A length/distance pair is a coded length followed by a
 *   coded distance to represent a string that occurs earlier in the
 *   uncompressed data that occurs again at the current location.
 *
 * - A bit preceding a literal or length/distance pair indicates which comes
 *   next, 0 for literals, 1 for length/distance.
 *
 * - If literals are uncoded, then the next eight bits are the literal, in the
 *   normal bit order in the stream, i.e. no bit-reversal is needed. Similarly,
 *   no bit reversal is needed for either the length extra bits or the distance
 *   extra bits.
 *
 * - Literal bytes are simply written to the output.  A length/distance pair is
 *   an instruction to copy previously uncompressed bytes to the output.  The
 *   copy is from distance bytes back in the output stream, copying for length
 *   bytes.
 *
 * - Distances pointing before the beginning of the output data are not
 *   permitted.
 *
 * - Overlapped copies, where the length is greater than the distance, are
 *   allowed and common.  For example, a distance of one and a length of 518
 *   simply copies the last byte 518 times.  A distance of four and a length of
 *   twelve copies the last four bytes three times.  A simple forward copy
 *   ignoring whether the length is greater than the distance or not implements
 *   this correctly.
 */
local صحيح decomp(بنية state *s)
{
    صحيح lit;            /* true if literals are coded */
    صحيح dict;           /* log2(dictionary size) - 6 */
    صحيح نماد;         /* decoded symbol, extra bits for distance */
    صحيح len;            /* length for copy */
    بیعلامت dist;      /* distance for copy */
    صحيح copy;           /* copy counter */
    بیعلامت حرف *from, *مقصد;   /* copy pointers */
    ساكن صحيح virgin = 1;                              /* build tables once */
    ساكن کوتاه litcnt[MAXBITS+1], litsym[256];        /* litcode memory */
    ساكن کوتاه lencnt[MAXBITS+1], lensym[16];         /* lencode memory */
    ساكن کوتاه distcnt[MAXBITS+1], distsym[64];       /* distcode memory */
    ساكن بنية huffman litcode = {litcnt, litsym};   /* length code */
    ساكن بنية huffman lencode = {lencnt, lensym};   /* length code */
    ساكن بنية huffman distcode = {distcnt, distsym};/* distance code */
        /* bit lengths of literal codes */
    ساكن ثابت بیعلامت حرف litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};
        /* bit lengths of length codes 0..15 */
    ساكن ثابت بیعلامت حرف lenlen[] = {2, 35, 36, 53, 38, 23};
        /* bit lengths of distance codes 0..63 */
    ساكن ثابت بیعلامت حرف distlen[] = {2, 20, 53, 230, 247, 151, 248};
    ساكن ثابت کوتاه پایه_2[16] = {     /* base for length codes */
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    ساكن ثابت حرف extra[16] = {     /* extra bits for length codes */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

    /* set up decoding tables (once--might not be thread-safe) */
    إذا (virgin) {
        construct(&litcode, litlen, حجم(litlen));
        construct(&lencode, lenlen, حجم(lenlen));
        construct(&distcode, distlen, حجم(distlen));
        virgin = 0;
    }

    /* read header */
    lit = بیتها(s, 8);
    إذا (lit > 1) أرجع -1;
    dict = بیتها(s, 8);
    إذا (dict < 4 || dict > 6) أرجع -2;

    /* decode literals and length/distance pairs */
    افعل {
        إذا (بیتها(s, 1)) {
            /* get length */
            نماد = decode(s, &lencode);
            len = پایه_2[نماد] + بیتها(s, extra[نماد]);
            إذا (len == 519) اكسر;              /* end code */

            /* get distance */
            نماد = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << نماد;
            dist += بیتها(s, نماد);
            dist++;
            إذا (s->اولی && dist > s->بعدی)
                أرجع -3;              /* distance too far back */

            /* copy length bytes from distance bytes back */
            افعل {
                مقصد = s->بیرون + s->بعدی;
                from = مقصد - dist;
                copy = MAXWIN;
                إذا (s->بعدی < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->بعدی;
                إذا (copy > len) copy = len;
                len -= copy;
                s->بعدی += copy;
                افعل {
                    *مقصد++ = *from++;
                } طالما (--copy);
                إذا (s->بعدی == MAXWIN) {
                    إذا (s->outfun(s->outhow, s->بیرون, s->بعدی)) أرجع 1;
                    s->بعدی = 0;
                    s->اولی = 0;
                }
            } طالما (len != 0);
        }
        وإلا {
            /* get literal and write it */
            نماد = lit ? decode(s, &litcode) : بیتها(s, 8);
            s->بیرون[s->بعدی++] = نماد;
            إذا (s->بعدی == MAXWIN) {
                إذا (s->outfun(s->outhow, s->بیرون, s->بعدی)) أرجع 1;
                s->بعدی = 0;
                s->اولی = 0;
            }
        }
    } طالما (1);
    أرجع 0;
}

/* See comments in blast.h */
صحيح blast(blast_درون infun, فراغ *inhow, blast_بیرون outfun, فراغ *outhow,
          بیعلامت *چپ, بیعلامت حرف **درون)
{
    بنية state s;             /* input/output state */
    صحيح err;                    /* return value */

    /* initialize input state */
    s.infun = infun;
    s.inhow = inhow;
    إذا (چپ != عدم && *چپ) {
        s.چپ = *چپ;
        s.درون = *درون;
    }
    وإلا
        s.چپ = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* initialize output state */
    s.outfun = outfun;
    s.outhow = outhow;
    s.بعدی = 0;
    s.اولی = 1;

    /* return if bits() or decode() tries to read past available input */
    إذا (setjmp(s.env) != 0)             /* if came back here via longjmp(), */
        err = 2;                        /*  then skip decomp(), return error */
    وإلا
        err = decomp(&s);               /* decompress */

    /* return unused input */
    إذا (چپ != عدم)
        *چپ = s.چپ;
    إذا (درون != عدم)
        *درون = s.چپ ? s.درون : عدم;

    /* write any leftover output and update the error code if needed */
    إذا (err != 1 && s.بعدی && s.outfun(s.outhow, s.بیرون, s.بعدی) && err == 0)
        err = 1;
    أرجع err;
}
