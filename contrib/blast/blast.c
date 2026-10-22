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

#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>             /* for NULL */
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#ਅਨ੍ਤਰ੍ਭਾਵ "blast.h"              /* prototype for blast() */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXBITS 13              /* maximum code length */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXWIN 4096             /* maximum window size */

/* input and output state */
ਬਣਤਰ state {
    /* input state */
    blast_ਵਿੱਚ infun;             /* input function provided by user */
    ਖਾਲੀ *inhow;                /* opaque information passed to infun() */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ;          /* next input location */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ;              /* available input at in */
    ਪੂਰਨ bitbuf;                 /* bit buffer */
    ਪੂਰਨ bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;

    /* output state */
    blast_ਬਾਹਰਲਾ outfun;           /* output function provided by user */
    ਖਾਲੀ *outhow;               /* opaque information passed to outfun() */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅਗਲਾ;              /* index of next write location in out[] */
    ਪੂਰਨ ਪਹਿਲਾ;                  /* true to check distances (for first 4K) */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਬਾਹਰਲਾ[MAXWIN];  /* output buffer and sliding window */
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
local ਪੂਰਨ ਬਿੱਟ(ਬਣਤਰ state *s, ਪੂਰਨ need)
{
    ਪੂਰਨ val;            /* bit accumulator */

    /* load at least need bits into val */
    val = s->bitbuf;
    ਜਦੋਂ (s->bitcnt < need) {
        ਜੇ (s->ਖੱਬਾ == 0) {
            s->ਖੱਬਾ = s->infun(s->inhow, &(s->ਵਿੱਚ));
            ਜੇ (s->ਖੱਬਾ == 0) longjmp(s->env, 1);       /* out of input */
        }
        val |= (ਪੂਰਨ)(*(s->ਵਿੱਚ)++) << s->bitcnt;          /* load eight bits */
        s->ਖੱਬਾ--;
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = val >> need;
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    ਵਾਪਸ val & ((1 << need) - 1);
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
ਬਣਤਰ huffman {
    ਛੋਟਾ *ਗਿਣਤੀ_2;       /* number of symbols of each length */
    ਛੋਟਾ *ਚਿੰਨ੍ਹ;      /* canonically ordered symbols */
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
local ਪੂਰਨ decode(ਬਣਤਰ state *s, ਬਣਤਰ huffman *h)
{
    ਪੂਰਨ len;            /* current number of bits in code */
    ਪੂਰਨ ਕੋਡ;           /* len bits being decoded */
    ਪੂਰਨ ਪਹਿਲਾ;          /* first code of length len */
    ਪੂਰਨ ਗਿਣਤੀ_2;          /* number of codes of length len */
    ਪੂਰਨ ਸੂਚਕਾਂਕ;          /* index of first code of length len in symbol table */
    ਪੂਰਨ bitbuf;         /* bits from stream */
    ਪੂਰਨ ਖੱਬਾ;           /* bits left in next or left to process */
    ਛੋਟਾ *ਅਗਲਾ;        /* next number of codes */

    bitbuf = s->bitbuf;
    ਖੱਬਾ = s->bitcnt;
    ਕੋਡ = ਪਹਿਲਾ = ਸੂਚਕਾਂਕ = 0;
    len = 1;
    ਅਗਲਾ = h->ਗਿਣਤੀ_2 + 1;
    ਜਦੋਂ (1) {
        ਜਦੋਂ (ਖੱਬਾ--) {
            ਕੋਡ |= (bitbuf & 1) ^ 1;   /* invert code */
            bitbuf >>= 1;
            ਗਿਣਤੀ_2 = *ਅਗਲਾ++;
            ਜੇ (ਕੋਡ < ਪਹਿਲਾ + ਗਿਣਤੀ_2) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                ਵਾਪਸ h->ਚਿੰਨ੍ਹ[ਸੂਚਕਾਂਕ + (ਕੋਡ - ਪਹਿਲਾ)];
            }
            ਸੂਚਕਾਂਕ += ਗਿਣਤੀ_2;             /* else update for next length */
            ਪਹਿਲਾ += ਗਿਣਤੀ_2;
            ਪਹਿਲਾ <<= 1;
            ਕੋਡ <<= 1;
            len++;
        }
        ਖੱਬਾ = (MAXBITS+1) - len;
        ਜੇ (ਖੱਬਾ == 0) ਰੋਕੋ;
        ਜੇ (s->ਖੱਬਾ == 0) {
            s->ਖੱਬਾ = s->infun(s->inhow, &(s->ਵਿੱਚ));
            ਜੇ (s->ਖੱਬਾ == 0) longjmp(s->env, 1);       /* out of input */
        }
        bitbuf = *(s->ਵਿੱਚ)++;
        s->ਖੱਬਾ--;
        ਜੇ (ਖੱਬਾ > 8) ਖੱਬਾ = 8;
    }
    ਵਾਪਸ -9;                          /* ran out of codes */
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
local ਪੂਰਨ construct(ਬਣਤਰ huffman *h, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *rep, ਪੂਰਨ n)
{
    ਪੂਰਨ ਚਿੰਨ੍ਹ;         /* current symbol when stepping through length[] */
    ਪੂਰਨ len;            /* current length when stepping through h->count[] */
    ਪੂਰਨ ਖੱਬਾ;           /* number of possible codes left of current length */
    ਛੋਟਾ offs[MAXBITS+1];      /* offsets in symbol table for each length */
    ਛੋਟਾ ਲੰਮਾਈ[256];  /* code lengths */

    /* convert compact repeat counts into symbol bit length list */
    ਚਿੰਨ੍ਹ = 0;
    ਕਰੋ {
        len = *rep++;
        ਖੱਬਾ = (len >> 4) + 1;
        len &= 15;
        ਕਰੋ {
            ਲੰਮਾਈ[ਚਿੰਨ੍ਹ++] = len;
        } ਜਦੋਂ (--ਖੱਬਾ);
    } ਜਦੋਂ (--n);
    n = ਚਿੰਨ੍ਹ;

    /* count number of codes of each length */
    ਲਈ (len = 0; len <= MAXBITS; len++)
        h->ਗਿਣਤੀ_2[len] = 0;
    ਲਈ (ਚਿੰਨ੍ਹ = 0; ਚਿੰਨ੍ਹ < n; ਚਿੰਨ੍ਹ++)
        (h->ਗਿਣਤੀ_2[ਲੰਮਾਈ[ਚਿੰਨ੍ਹ]])++;   /* assumes lengths are within bounds */
    ਜੇ (h->ਗਿਣਤੀ_2[0] == n)               /* no codes! */
        ਵਾਪਸ 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    ਖੱਬਾ = 1;                           /* one possible code of zero length */
    ਲਈ (len = 1; len <= MAXBITS; len++) {
        ਖੱਬਾ <<= 1;                     /* one more bit, double codes left */
        ਖੱਬਾ -= h->ਗਿਣਤੀ_2[len];          /* deduct count from possible codes */
        ਜੇ (ਖੱਬਾ < 0) ਵਾਪਸ ਖੱਬਾ;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    ਲਈ (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->ਗਿਣਤੀ_2[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    ਲਈ (ਚਿੰਨ੍ਹ = 0; ਚਿੰਨ੍ਹ < n; ਚਿੰਨ੍ਹ++)
        ਜੇ (ਲੰਮਾਈ[ਚਿੰਨ੍ਹ] != 0)
            h->ਚਿੰਨ੍ਹ[offs[ਲੰਮਾਈ[ਚਿੰਨ੍ਹ]]++] = ਚਿੰਨ੍ਹ;

    /* return zero for complete set, positive for incomplete set */
    ਵਾਪਸ ਖੱਬਾ;
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
local ਪੂਰਨ decomp(ਬਣਤਰ state *s)
{
    ਪੂਰਨ lit;            /* true if literals are coded */
    ਪੂਰਨ dict;           /* log2(dictionary size) - 6 */
    ਪੂਰਨ ਚਿੰਨ੍ਹ;         /* decoded symbol, extra bits for distance */
    ਪੂਰਨ len;            /* length for copy */
    ਬਿਨਾਨਿਸ਼ਾਨੀ dist;      /* distance for copy */
    ਪੂਰਨ copy;           /* copy counter */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *from, *ਮੰਜ਼ਿਲ;   /* copy pointers */
    ਸਥਿਰ ਪੂਰਨ virgin = 1;                              /* build tables once */
    ਸਥਿਰ ਛੋਟਾ litcnt[MAXBITS+1], litsym[256];        /* litcode memory */
    ਸਥਿਰ ਛੋਟਾ lencnt[MAXBITS+1], lensym[16];         /* lencode memory */
    ਸਥਿਰ ਛੋਟਾ distcnt[MAXBITS+1], distsym[64];       /* distcode memory */
    ਸਥਿਰ ਬਣਤਰ huffman litcode = {litcnt, litsym};   /* length code */
    ਸਥਿਰ ਬਣਤਰ huffman lencode = {lencnt, lensym};   /* length code */
    ਸਥਿਰ ਬਣਤਰ huffman distcode = {distcnt, distsym};/* distance code */
        /* bit lengths of literal codes */
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};
        /* bit lengths of length codes 0..15 */
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ lenlen[] = {2, 35, 36, 53, 38, 23};
        /* bit lengths of distance codes 0..63 */
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ distlen[] = {2, 20, 53, 230, 247, 151, 248};
    ਸਥਿਰ ਅਚਲ ਛੋਟਾ ਅਧਾਰ_2[16] = {     /* base for length codes */
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ extra[16] = {     /* extra bits for length codes */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

    /* set up decoding tables (once--might not be thread-safe) */
    ਜੇ (virgin) {
        construct(&litcode, litlen, ਆਕਾਰ(litlen));
        construct(&lencode, lenlen, ਆਕਾਰ(lenlen));
        construct(&distcode, distlen, ਆਕਾਰ(distlen));
        virgin = 0;
    }

    /* read header */
    lit = ਬਿੱਟ(s, 8);
    ਜੇ (lit > 1) ਵਾਪਸ -1;
    dict = ਬਿੱਟ(s, 8);
    ਜੇ (dict < 4 || dict > 6) ਵਾਪਸ -2;

    /* decode literals and length/distance pairs */
    ਕਰੋ {
        ਜੇ (ਬਿੱਟ(s, 1)) {
            /* get length */
            ਚਿੰਨ੍ਹ = decode(s, &lencode);
            len = ਅਧਾਰ_2[ਚਿੰਨ੍ਹ] + ਬਿੱਟ(s, extra[ਚਿੰਨ੍ਹ]);
            ਜੇ (len == 519) ਰੋਕੋ;              /* end code */

            /* get distance */
            ਚਿੰਨ੍ਹ = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << ਚਿੰਨ੍ਹ;
            dist += ਬਿੱਟ(s, ਚਿੰਨ੍ਹ);
            dist++;
            ਜੇ (s->ਪਹਿਲਾ && dist > s->ਅਗਲਾ)
                ਵਾਪਸ -3;              /* distance too far back */

            /* copy length bytes from distance bytes back */
            ਕਰੋ {
                ਮੰਜ਼ਿਲ = s->ਬਾਹਰਲਾ + s->ਅਗਲਾ;
                from = ਮੰਜ਼ਿਲ - dist;
                copy = MAXWIN;
                ਜੇ (s->ਅਗਲਾ < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->ਅਗਲਾ;
                ਜੇ (copy > len) copy = len;
                len -= copy;
                s->ਅਗਲਾ += copy;
                ਕਰੋ {
                    *ਮੰਜ਼ਿਲ++ = *from++;
                } ਜਦੋਂ (--copy);
                ਜੇ (s->ਅਗਲਾ == MAXWIN) {
                    ਜੇ (s->outfun(s->outhow, s->ਬਾਹਰਲਾ, s->ਅਗਲਾ)) ਵਾਪਸ 1;
                    s->ਅਗਲਾ = 0;
                    s->ਪਹਿਲਾ = 0;
                }
            } ਜਦੋਂ (len != 0);
        }
        ਨਹੀਂਤਾਂ {
            /* get literal and write it */
            ਚਿੰਨ੍ਹ = lit ? decode(s, &litcode) : ਬਿੱਟ(s, 8);
            s->ਬਾਹਰਲਾ[s->ਅਗਲਾ++] = ਚਿੰਨ੍ਹ;
            ਜੇ (s->ਅਗਲਾ == MAXWIN) {
                ਜੇ (s->outfun(s->outhow, s->ਬਾਹਰਲਾ, s->ਅਗਲਾ)) ਵਾਪਸ 1;
                s->ਅਗਲਾ = 0;
                s->ਪਹਿਲਾ = 0;
            }
        }
    } ਜਦੋਂ (1);
    ਵਾਪਸ 0;
}

/* See comments in blast.h */
ਪੂਰਨ blast(blast_ਵਿੱਚ infun, ਖਾਲੀ *inhow, blast_ਬਾਹਰਲਾ outfun, ਖਾਲੀ *outhow,
          ਬਿਨਾਨਿਸ਼ਾਨੀ *ਖੱਬਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **ਵਿੱਚ)
{
    ਬਣਤਰ state s;             /* input/output state */
    ਪੂਰਨ err;                    /* return value */

    /* initialize input state */
    s.infun = infun;
    s.inhow = inhow;
    ਜੇ (ਖੱਬਾ != ਨਿਰਾ && *ਖੱਬਾ) {
        s.ਖੱਬਾ = *ਖੱਬਾ;
        s.ਵਿੱਚ = *ਵਿੱਚ;
    }
    ਨਹੀਂਤਾਂ
        s.ਖੱਬਾ = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* initialize output state */
    s.outfun = outfun;
    s.outhow = outhow;
    s.ਅਗਲਾ = 0;
    s.ਪਹਿਲਾ = 1;

    /* return if bits() or decode() tries to read past available input */
    ਜੇ (setjmp(s.env) != 0)             /* if came back here via longjmp(), */
        err = 2;                        /*  then skip decomp(), return error */
    ਨਹੀਂਤਾਂ
        err = decomp(&s);               /* decompress */

    /* return unused input */
    ਜੇ (ਖੱਬਾ != ਨਿਰਾ)
        *ਖੱਬਾ = s.ਖੱਬਾ;
    ਜੇ (ਵਿੱਚ != ਨਿਰਾ)
        *ਵਿੱਚ = s.ਖੱਬਾ ? s.ਵਿੱਚ : ਨਿਰਾ;

    /* write any leftover output and update the error code if needed */
    ਜੇ (err != 1 && s.ਅਗਲਾ && s.outfun(s.outhow, s.ਬਾਹਰਲਾ, s.ਅਗਲਾ) && err == 0)
        err = 1;
    ਵਾਪਸ err;
}
