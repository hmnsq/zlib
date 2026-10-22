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

#అన్తర్భావ <stddef.h>             /* for NULL */
#అన్తర్భావ <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#అన్తర్భావ "blast.h"              /* prototype for blast() */

#నిర్వచించు MAXBITS 13              /* maximum code length */
#నిర్వచించు MAXWIN 4096             /* maximum window size */

/* input and output state */
నిర్మాణం state {
    /* input state */
    blast_లోపల infun;             /* input function provided by user */
    శూన్య *inhow;                /* opaque information passed to infun() */
    గుర్తులేని అక్షరం *లోపల;          /* next input location */
    గుర్తులేని ఎడమ;              /* available input at in */
    పూర్ణాంకం bitbuf;                 /* bit buffer */
    పూర్ణాంకం bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;

    /* output state */
    blast_బహిర్గతం outfun;           /* output function provided by user */
    శూన్య *outhow;               /* opaque information passed to outfun() */
    గుర్తులేని తదుపరి;              /* index of next write location in out[] */
    పూర్ణాంకం మొదటి;                  /* true to check distances (for first 4K) */
    గుర్తులేని అక్షరం బహిర్గతం[MAXWIN];  /* output buffer and sliding window */
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
local పూర్ణాంకం బిట్లు(నిర్మాణం state *s, పూర్ణాంకం need)
{
    పూర్ణాంకం val;            /* bit accumulator */

    /* load at least need bits into val */
    val = s->bitbuf;
    అయితే (s->bitcnt < need) {
        ఒకవేళ (s->ఎడమ == 0) {
            s->ఎడమ = s->infun(s->inhow, &(s->లోపల));
            ఒకవేళ (s->ఎడమ == 0) longjmp(s->env, 1);       /* out of input */
        }
        val |= (పూర్ణాంకం)(*(s->లోపల)++) << s->bitcnt;          /* load eight bits */
        s->ఎడమ--;
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = val >> need;
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    తిరిగి val & ((1 << need) - 1);
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
నిర్మాణం huffman {
    పొట్టి *లెక్క;       /* number of symbols of each length */
    పొట్టి *చిహ్నం;      /* canonically ordered symbols */
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
local పూర్ణాంకం decode(నిర్మాణం state *s, నిర్మాణం huffman *h)
{
    పూర్ణాంకం len;            /* current number of bits in code */
    పూర్ణాంకం సంకేతం_2;           /* len bits being decoded */
    పూర్ణాంకం మొదటి;          /* first code of length len */
    పూర్ణాంకం లెక్క;          /* number of codes of length len */
    పూర్ణాంకం సూచీ;          /* index of first code of length len in symbol table */
    పూర్ణాంకం bitbuf;         /* bits from stream */
    పూర్ణాంకం ఎడమ;           /* bits left in next or left to process */
    పొట్టి *తదుపరి;        /* next number of codes */

    bitbuf = s->bitbuf;
    ఎడమ = s->bitcnt;
    సంకేతం_2 = మొదటి = సూచీ = 0;
    len = 1;
    తదుపరి = h->లెక్క + 1;
    అయితే (1) {
        అయితే (ఎడమ--) {
            సంకేతం_2 |= (bitbuf & 1) ^ 1;   /* invert code */
            bitbuf >>= 1;
            లెక్క = *తదుపరి++;
            ఒకవేళ (సంకేతం_2 < మొదటి + లెక్క) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                తిరిగి h->చిహ్నం[సూచీ + (సంకేతం_2 - మొదటి)];
            }
            సూచీ += లెక్క;             /* else update for next length */
            మొదటి += లెక్క;
            మొదటి <<= 1;
            సంకేతం_2 <<= 1;
            len++;
        }
        ఎడమ = (MAXBITS+1) - len;
        ఒకవేళ (ఎడమ == 0) ఆపు;
        ఒకవేళ (s->ఎడమ == 0) {
            s->ఎడమ = s->infun(s->inhow, &(s->లోపల));
            ఒకవేళ (s->ఎడమ == 0) longjmp(s->env, 1);       /* out of input */
        }
        bitbuf = *(s->లోపల)++;
        s->ఎడమ--;
        ఒకవేళ (ఎడమ > 8) ఎడమ = 8;
    }
    తిరిగి -9;                          /* ran out of codes */
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
local పూర్ణాంకం construct(నిర్మాణం huffman *h, స్థిరరాశి గుర్తులేని అక్షరం *rep, పూర్ణాంకం n)
{
    పూర్ణాంకం చిహ్నం;         /* current symbol when stepping through length[] */
    పూర్ణాంకం len;            /* current length when stepping through h->count[] */
    పూర్ణాంకం ఎడమ;           /* number of possible codes left of current length */
    పొట్టి offs[MAXBITS+1];      /* offsets in symbol table for each length */
    పొట్టి నిడివి[256];  /* code lengths */

    /* convert compact repeat counts into symbol bit length list */
    చిహ్నం = 0;
    చేయి {
        len = *rep++;
        ఎడమ = (len >> 4) + 1;
        len &= 15;
        చేయి {
            నిడివి[చిహ్నం++] = len;
        } అయితే (--ఎడమ);
    } అయితే (--n);
    n = చిహ్నం;

    /* count number of codes of each length */
    కొరకు (len = 0; len <= MAXBITS; len++)
        h->లెక్క[len] = 0;
    కొరకు (చిహ్నం = 0; చిహ్నం < n; చిహ్నం++)
        (h->లెక్క[నిడివి[చిహ్నం]])++;   /* assumes lengths are within bounds */
    ఒకవేళ (h->లెక్క[0] == n)               /* no codes! */
        తిరిగి 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    ఎడమ = 1;                           /* one possible code of zero length */
    కొరకు (len = 1; len <= MAXBITS; len++) {
        ఎడమ <<= 1;                     /* one more bit, double codes left */
        ఎడమ -= h->లెక్క[len];          /* deduct count from possible codes */
        ఒకవేళ (ఎడమ < 0) తిరిగి ఎడమ;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    కొరకు (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->లెక్క[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    కొరకు (చిహ్నం = 0; చిహ్నం < n; చిహ్నం++)
        ఒకవేళ (నిడివి[చిహ్నం] != 0)
            h->చిహ్నం[offs[నిడివి[చిహ్నం]]++] = చిహ్నం;

    /* return zero for complete set, positive for incomplete set */
    తిరిగి ఎడమ;
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
local పూర్ణాంకం decomp(నిర్మాణం state *s)
{
    పూర్ణాంకం lit;            /* true if literals are coded */
    పూర్ణాంకం dict;           /* log2(dictionary size) - 6 */
    పూర్ణాంకం చిహ్నం;         /* decoded symbol, extra bits for distance */
    పూర్ణాంకం len;            /* length for copy */
    గుర్తులేని dist;      /* distance for copy */
    పూర్ణాంకం copy;           /* copy counter */
    గుర్తులేని అక్షరం *from, *గమ్యం;   /* copy pointers */
    స్థిర పూర్ణాంకం virgin = 1;                              /* build tables once */
    స్థిర పొట్టి litcnt[MAXBITS+1], litsym[256];        /* litcode memory */
    స్థిర పొట్టి lencnt[MAXBITS+1], lensym[16];         /* lencode memory */
    స్థిర పొట్టి distcnt[MAXBITS+1], distsym[64];       /* distcode memory */
    స్థిర నిర్మాణం huffman litcode = {litcnt, litsym};   /* length code */
    స్థిర నిర్మాణం huffman lencode = {lencnt, lensym};   /* length code */
    స్థిర నిర్మాణం huffman distcode = {distcnt, distsym};/* distance code */
        /* bit lengths of literal codes */
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};
        /* bit lengths of length codes 0..15 */
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం lenlen[] = {2, 35, 36, 53, 38, 23};
        /* bit lengths of distance codes 0..63 */
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం distlen[] = {2, 20, 53, 230, 247, 151, 248};
    స్థిర స్థిరరాశి పొట్టి పునాది_2[16] = {     /* base for length codes */
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    స్థిర స్థిరరాశి అక్షరం extra[16] = {     /* extra bits for length codes */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

    /* set up decoding tables (once--might not be thread-safe) */
    ఒకవేళ (virgin) {
        construct(&litcode, litlen, పరిమాణం(litlen));
        construct(&lencode, lenlen, పరిమాణం(lenlen));
        construct(&distcode, distlen, పరిమాణం(distlen));
        virgin = 0;
    }

    /* read header */
    lit = బిట్లు(s, 8);
    ఒకవేళ (lit > 1) తిరిగి -1;
    dict = బిట్లు(s, 8);
    ఒకవేళ (dict < 4 || dict > 6) తిరిగి -2;

    /* decode literals and length/distance pairs */
    చేయి {
        ఒకవేళ (బిట్లు(s, 1)) {
            /* get length */
            చిహ్నం = decode(s, &lencode);
            len = పునాది_2[చిహ్నం] + బిట్లు(s, extra[చిహ్నం]);
            ఒకవేళ (len == 519) ఆపు;              /* end code */

            /* get distance */
            చిహ్నం = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << చిహ్నం;
            dist += బిట్లు(s, చిహ్నం);
            dist++;
            ఒకవేళ (s->మొదటి && dist > s->తదుపరి)
                తిరిగి -3;              /* distance too far back */

            /* copy length bytes from distance bytes back */
            చేయి {
                గమ్యం = s->బహిర్గతం + s->తదుపరి;
                from = గమ్యం - dist;
                copy = MAXWIN;
                ఒకవేళ (s->తదుపరి < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->తదుపరి;
                ఒకవేళ (copy > len) copy = len;
                len -= copy;
                s->తదుపరి += copy;
                చేయి {
                    *గమ్యం++ = *from++;
                } అయితే (--copy);
                ఒకవేళ (s->తదుపరి == MAXWIN) {
                    ఒకవేళ (s->outfun(s->outhow, s->బహిర్గతం, s->తదుపరి)) తిరిగి 1;
                    s->తదుపరి = 0;
                    s->మొదటి = 0;
                }
            } అయితే (len != 0);
        }
        లేకపోతే {
            /* get literal and write it */
            చిహ్నం = lit ? decode(s, &litcode) : బిట్లు(s, 8);
            s->బహిర్గతం[s->తదుపరి++] = చిహ్నం;
            ఒకవేళ (s->తదుపరి == MAXWIN) {
                ఒకవేళ (s->outfun(s->outhow, s->బహిర్గతం, s->తదుపరి)) తిరిగి 1;
                s->తదుపరి = 0;
                s->మొదటి = 0;
            }
        }
    } అయితే (1);
    తిరిగి 0;
}

/* See comments in blast.h */
పూర్ణాంకం blast(blast_లోపల infun, శూన్య *inhow, blast_బహిర్గతం outfun, శూన్య *outhow,
          గుర్తులేని *ఎడమ, గుర్తులేని అక్షరం **లోపల)
{
    నిర్మాణం state s;             /* input/output state */
    పూర్ణాంకం err;                    /* return value */

    /* initialize input state */
    s.infun = infun;
    s.inhow = inhow;
    ఒకవేళ (ఎడమ != ఖాళీ && *ఎడమ) {
        s.ఎడమ = *ఎడమ;
        s.లోపల = *లోపల;
    }
    లేకపోతే
        s.ఎడమ = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* initialize output state */
    s.outfun = outfun;
    s.outhow = outhow;
    s.తదుపరి = 0;
    s.మొదటి = 1;

    /* return if bits() or decode() tries to read past available input */
    ఒకవేళ (setjmp(s.env) != 0)             /* if came back here via longjmp(), */
        err = 2;                        /*  then skip decomp(), return error */
    లేకపోతే
        err = decomp(&s);               /* decompress */

    /* return unused input */
    ఒకవేళ (ఎడమ != ఖాళీ)
        *ఎడమ = s.ఎడమ;
    ఒకవేళ (లోపల != ఖాళీ)
        *లోపల = s.ఎడమ ? s.లోపల : ఖాళీ;

    /* write any leftover output and update the error code if needed */
    ఒకవేళ (err != 1 && s.తదుపరి && s.outfun(s.outhow, s.బహిర్గతం, s.తదుపరి) && err == 0)
        err = 1;
    తిరిగి err;
}
