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

#ಅನ್ತರ್ಭಾವ <stddef.h>             /* for NULL */
#ಅನ್ತರ್ಭಾವ <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#ಅನ್ತರ್ಭಾವ "blast.h"              /* prototype for blast() */

#ವ್ಯಾಖ್ಯಾನಿಸು MAXBITS 13              /* maximum code length */
#ವ್ಯಾಖ್ಯಾನಿಸು MAXWIN 4096             /* maximum window size */

/* input and output state */
ರಚನೆ state {
    /* input state */
    blast_ಒಳ infun;             /* input function provided by user */
    ಶೂನ್ಯ *inhow;                /* opaque information passed to infun() */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ;          /* next input location */
    ಚಿಹ್ನೆರಹಿತ ಎಡ;              /* available input at in */
    ಪೂರ್ಣಾಂಕ bitbuf;                 /* bit buffer */
    ಪೂರ್ಣಾಂಕ bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;

    /* output state */
    blast_ಹೊರಹೋಗು outfun;           /* output function provided by user */
    ಶೂನ್ಯ *outhow;               /* opaque information passed to outfun() */
    ಚಿಹ್ನೆರಹಿತ ಮುಂದಿನ;              /* index of next write location in out[] */
    ಪೂರ್ಣಾಂಕ ಮೊದಲ;                  /* true to check distances (for first 4K) */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಹೊರಹೋಗು[MAXWIN];  /* output buffer and sliding window */
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
local ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು(ರಚನೆ state *s, ಪೂರ್ಣಾಂಕ need)
{
    ಪೂರ್ಣಾಂಕ val;            /* bit accumulator */

    /* load at least need bits into val */
    val = s->bitbuf;
    ಆಗ (s->bitcnt < need) {
        ಆದರೆ (s->ಎಡ == 0) {
            s->ಎಡ = s->infun(s->inhow, &(s->ಒಳ));
            ಆದರೆ (s->ಎಡ == 0) longjmp(s->env, 1);       /* out of input */
        }
        val |= (ಪೂರ್ಣಾಂಕ)(*(s->ಒಳ)++) << s->bitcnt;          /* load eight bits */
        s->ಎಡ--;
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = val >> need;
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    ಹಿಂತಿರುಗಿ val & ((1 << need) - 1);
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
ರಚನೆ huffman {
    ಗಿಡ್ಡ *ಎಣಿಕೆ_2;       /* number of symbols of each length */
    ಗಿಡ್ಡ *ಚಿಹ್ನೆ;      /* canonically ordered symbols */
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
local ಪೂರ್ಣಾಂಕ decode(ರಚನೆ state *s, ರಚನೆ huffman *h)
{
    ಪೂರ್ಣಾಂಕ len;            /* current number of bits in code */
    ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2;           /* len bits being decoded */
    ಪೂರ್ಣಾಂಕ ಮೊದಲ;          /* first code of length len */
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;          /* number of codes of length len */
    ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ;          /* index of first code of length len in symbol table */
    ಪೂರ್ಣಾಂಕ bitbuf;         /* bits from stream */
    ಪೂರ್ಣಾಂಕ ಎಡ;           /* bits left in next or left to process */
    ಗಿಡ್ಡ *ಮುಂದಿನ;        /* next number of codes */

    bitbuf = s->bitbuf;
    ಎಡ = s->bitcnt;
    ಸಂಕೇತ_2 = ಮೊದಲ = ಸೂಚ್ಯಂಕ = 0;
    len = 1;
    ಮುಂದಿನ = h->ಎಣಿಕೆ_2 + 1;
    ಆಗ (1) {
        ಆಗ (ಎಡ--) {
            ಸಂಕೇತ_2 |= (bitbuf & 1) ^ 1;   /* invert code */
            bitbuf >>= 1;
            ಎಣಿಕೆ_2 = *ಮುಂದಿನ++;
            ಆದರೆ (ಸಂಕೇತ_2 < ಮೊದಲ + ಎಣಿಕೆ_2) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                ಹಿಂತಿರುಗಿ h->ಚಿಹ್ನೆ[ಸೂಚ್ಯಂಕ + (ಸಂಕೇತ_2 - ಮೊದಲ)];
            }
            ಸೂಚ್ಯಂಕ += ಎಣಿಕೆ_2;             /* else update for next length */
            ಮೊದಲ += ಎಣಿಕೆ_2;
            ಮೊದಲ <<= 1;
            ಸಂಕೇತ_2 <<= 1;
            len++;
        }
        ಎಡ = (MAXBITS+1) - len;
        ಆದರೆ (ಎಡ == 0) ನಿಲ್ಲಿಸು;
        ಆದರೆ (s->ಎಡ == 0) {
            s->ಎಡ = s->infun(s->inhow, &(s->ಒಳ));
            ಆದರೆ (s->ಎಡ == 0) longjmp(s->env, 1);       /* out of input */
        }
        bitbuf = *(s->ಒಳ)++;
        s->ಎಡ--;
        ಆದರೆ (ಎಡ > 8) ಎಡ = 8;
    }
    ಹಿಂತಿರುಗಿ -9;                          /* ran out of codes */
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
local ಪೂರ್ಣಾಂಕ construct(ರಚನೆ huffman *h, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *rep, ಪೂರ್ಣಾಂಕ n)
{
    ಪೂರ್ಣಾಂಕ ಚಿಹ್ನೆ;         /* current symbol when stepping through length[] */
    ಪೂರ್ಣಾಂಕ len;            /* current length when stepping through h->count[] */
    ಪೂರ್ಣಾಂಕ ಎಡ;           /* number of possible codes left of current length */
    ಗಿಡ್ಡ offs[MAXBITS+1];      /* offsets in symbol table for each length */
    ಗಿಡ್ಡ ಉದ್ದಳತೆ[256];  /* code lengths */

    /* convert compact repeat counts into symbol bit length list */
    ಚಿಹ್ನೆ = 0;
    ಮಾಡು {
        len = *rep++;
        ಎಡ = (len >> 4) + 1;
        len &= 15;
        ಮಾಡು {
            ಉದ್ದಳತೆ[ಚಿಹ್ನೆ++] = len;
        } ಆಗ (--ಎಡ);
    } ಆಗ (--n);
    n = ಚಿಹ್ನೆ;

    /* count number of codes of each length */
    ಗಾಗಿ (len = 0; len <= MAXBITS; len++)
        h->ಎಣಿಕೆ_2[len] = 0;
    ಗಾಗಿ (ಚಿಹ್ನೆ = 0; ಚಿಹ್ನೆ < n; ಚಿಹ್ನೆ++)
        (h->ಎಣಿಕೆ_2[ಉದ್ದಳತೆ[ಚಿಹ್ನೆ]])++;   /* assumes lengths are within bounds */
    ಆದರೆ (h->ಎಣಿಕೆ_2[0] == n)               /* no codes! */
        ಹಿಂತಿರುಗಿ 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    ಎಡ = 1;                           /* one possible code of zero length */
    ಗಾಗಿ (len = 1; len <= MAXBITS; len++) {
        ಎಡ <<= 1;                     /* one more bit, double codes left */
        ಎಡ -= h->ಎಣಿಕೆ_2[len];          /* deduct count from possible codes */
        ಆದರೆ (ಎಡ < 0) ಹಿಂತಿರುಗಿ ಎಡ;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    ಗಾಗಿ (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->ಎಣಿಕೆ_2[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    ಗಾಗಿ (ಚಿಹ್ನೆ = 0; ಚಿಹ್ನೆ < n; ಚಿಹ್ನೆ++)
        ಆದರೆ (ಉದ್ದಳತೆ[ಚಿಹ್ನೆ] != 0)
            h->ಚಿಹ್ನೆ[offs[ಉದ್ದಳತೆ[ಚಿಹ್ನೆ]]++] = ಚಿಹ್ನೆ;

    /* return zero for complete set, positive for incomplete set */
    ಹಿಂತಿರುಗಿ ಎಡ;
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
local ಪೂರ್ಣಾಂಕ decomp(ರಚನೆ state *s)
{
    ಪೂರ್ಣಾಂಕ lit;            /* true if literals are coded */
    ಪೂರ್ಣಾಂಕ dict;           /* log2(dictionary size) - 6 */
    ಪೂರ್ಣಾಂಕ ಚಿಹ್ನೆ;         /* decoded symbol, extra bits for distance */
    ಪೂರ್ಣಾಂಕ len;            /* length for copy */
    ಚಿಹ್ನೆರಹಿತ dist;      /* distance for copy */
    ಪೂರ್ಣಾಂಕ copy;           /* copy counter */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *from, *ಗಮ್ಯ;   /* copy pointers */
    ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ virgin = 1;                              /* build tables once */
    ಸ್ಥಿರ ಗಿಡ್ಡ litcnt[MAXBITS+1], litsym[256];        /* litcode memory */
    ಸ್ಥಿರ ಗಿಡ್ಡ lencnt[MAXBITS+1], lensym[16];         /* lencode memory */
    ಸ್ಥಿರ ಗಿಡ್ಡ distcnt[MAXBITS+1], distsym[64];       /* distcode memory */
    ಸ್ಥಿರ ರಚನೆ huffman litcode = {litcnt, litsym};   /* length code */
    ಸ್ಥಿರ ರಚನೆ huffman lencode = {lencnt, lensym};   /* length code */
    ಸ್ಥಿರ ರಚನೆ huffman distcode = {distcnt, distsym};/* distance code */
        /* bit lengths of literal codes */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};
        /* bit lengths of length codes 0..15 */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ lenlen[] = {2, 35, 36, 53, 38, 23};
        /* bit lengths of distance codes 0..63 */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ distlen[] = {2, 20, 53, 230, 247, 151, 248};
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಗಿಡ್ಡ ತಳ_2[16] = {     /* base for length codes */
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ extra[16] = {     /* extra bits for length codes */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

    /* set up decoding tables (once--might not be thread-safe) */
    ಆದರೆ (virgin) {
        construct(&litcode, litlen, ಗಾತ್ರ(litlen));
        construct(&lencode, lenlen, ಗಾತ್ರ(lenlen));
        construct(&distcode, distlen, ಗಾತ್ರ(distlen));
        virgin = 0;
    }

    /* read header */
    lit = ಬಿಟ್ಟುಗಳು(s, 8);
    ಆದರೆ (lit > 1) ಹಿಂತಿರುಗಿ -1;
    dict = ಬಿಟ್ಟುಗಳು(s, 8);
    ಆದರೆ (dict < 4 || dict > 6) ಹಿಂತಿರುಗಿ -2;

    /* decode literals and length/distance pairs */
    ಮಾಡು {
        ಆದರೆ (ಬಿಟ್ಟುಗಳು(s, 1)) {
            /* get length */
            ಚಿಹ್ನೆ = decode(s, &lencode);
            len = ತಳ_2[ಚಿಹ್ನೆ] + ಬಿಟ್ಟುಗಳು(s, extra[ಚಿಹ್ನೆ]);
            ಆದರೆ (len == 519) ನಿಲ್ಲಿಸು;              /* end code */

            /* get distance */
            ಚಿಹ್ನೆ = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << ಚಿಹ್ನೆ;
            dist += ಬಿಟ್ಟುಗಳು(s, ಚಿಹ್ನೆ);
            dist++;
            ಆದರೆ (s->ಮೊದಲ && dist > s->ಮುಂದಿನ)
                ಹಿಂತಿರುಗಿ -3;              /* distance too far back */

            /* copy length bytes from distance bytes back */
            ಮಾಡು {
                ಗಮ್ಯ = s->ಹೊರಹೋಗು + s->ಮುಂದಿನ;
                from = ಗಮ್ಯ - dist;
                copy = MAXWIN;
                ಆದರೆ (s->ಮುಂದಿನ < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->ಮುಂದಿನ;
                ಆದರೆ (copy > len) copy = len;
                len -= copy;
                s->ಮುಂದಿನ += copy;
                ಮಾಡು {
                    *ಗಮ್ಯ++ = *from++;
                } ಆಗ (--copy);
                ಆದರೆ (s->ಮುಂದಿನ == MAXWIN) {
                    ಆದರೆ (s->outfun(s->outhow, s->ಹೊರಹೋಗು, s->ಮುಂದಿನ)) ಹಿಂತಿರುಗಿ 1;
                    s->ಮುಂದಿನ = 0;
                    s->ಮೊದಲ = 0;
                }
            } ಆಗ (len != 0);
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            /* get literal and write it */
            ಚಿಹ್ನೆ = lit ? decode(s, &litcode) : ಬಿಟ್ಟುಗಳು(s, 8);
            s->ಹೊರಹೋಗು[s->ಮುಂದಿನ++] = ಚಿಹ್ನೆ;
            ಆದರೆ (s->ಮುಂದಿನ == MAXWIN) {
                ಆದರೆ (s->outfun(s->outhow, s->ಹೊರಹೋಗು, s->ಮುಂದಿನ)) ಹಿಂತಿರುಗಿ 1;
                s->ಮುಂದಿನ = 0;
                s->ಮೊದಲ = 0;
            }
        }
    } ಆಗ (1);
    ಹಿಂತಿರುಗಿ 0;
}

/* See comments in blast.h */
ಪೂರ್ಣಾಂಕ blast(blast_ಒಳ infun, ಶೂನ್ಯ *inhow, blast_ಹೊರಹೋಗು outfun, ಶೂನ್ಯ *outhow,
          ಚಿಹ್ನೆರಹಿತ *ಎಡ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **ಒಳ)
{
    ರಚನೆ state s;             /* input/output state */
    ಪೂರ್ಣಾಂಕ err;                    /* return value */

    /* initialize input state */
    s.infun = infun;
    s.inhow = inhow;
    ಆದರೆ (ಎಡ != ಖಾಲಿ && *ಎಡ) {
        s.ಎಡ = *ಎಡ;
        s.ಒಳ = *ಒಳ;
    }
    ಇಲ್ಲದಿದ್ದರೆ
        s.ಎಡ = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* initialize output state */
    s.outfun = outfun;
    s.outhow = outhow;
    s.ಮುಂದಿನ = 0;
    s.ಮೊದಲ = 1;

    /* return if bits() or decode() tries to read past available input */
    ಆದರೆ (setjmp(s.env) != 0)             /* if came back here via longjmp(), */
        err = 2;                        /*  then skip decomp(), return error */
    ಇಲ್ಲದಿದ್ದರೆ
        err = decomp(&s);               /* decompress */

    /* return unused input */
    ಆದರೆ (ಎಡ != ಖಾಲಿ)
        *ಎಡ = s.ಎಡ;
    ಆದರೆ (ಒಳ != ಖಾಲಿ)
        *ಒಳ = s.ಎಡ ? s.ಒಳ : ಖಾಲಿ;

    /* write any leftover output and update the error code if needed */
    ಆದರೆ (err != 1 && s.ಮುಂದಿನ && s.outfun(s.outhow, s.ಹೊರಹೋಗು, s.ಮುಂದಿನ) && err == 0)
        err = 1;
    ಹಿಂತಿರುಗಿ err;
}
