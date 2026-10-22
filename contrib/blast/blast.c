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

#અન્તર્ભાવ <stddef.h>             /* for NULL */
#અન્તર્ભાવ <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#અન્તર્ભાવ "blast.h"              /* prototype for blast() */

#define MAXBITS 13              /* maximum code length */
#define MAXWIN 4096             /* maximum window size */

/* input and output state */
સંરચના state {
    /* input state */
    blast_in infun;             /* input function provided by user */
    શૂન્ય *inhow;                /* opaque information passed to infun() */
    unsigned અક્ષર *in;          /* next input location */
    unsigned ડાબું;              /* available input at in */
    પૂર્ણાંક bitbuf;                 /* bit buffer */
    પૂર્ણાંક bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;

    /* output state */
    blast_out outfun;           /* output function provided by user */
    શૂન્ય *outhow;               /* opaque information passed to outfun() */
    unsigned next;              /* index of next write location in out[] */
    પૂર્ણાંક first;                  /* true to check distances (for first 4K) */
    unsigned અક્ષર out[MAXWIN];  /* output buffer and sliding window */
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
local પૂર્ણાંક bits(સંરચના state *s, પૂર્ણાંક need)
{
    પૂર્ણાંક val;            /* bit accumulator */

    /* load at least need bits into val */
    val = s->bitbuf;
    જ્યારે (s->bitcnt < need) {
        જો (s->ડાબું == 0) {
            s->ડાબું = s->infun(s->inhow, &(s->in));
            જો (s->ડાબું == 0) longjmp(s->env, 1);       /* out of input */
        }
        val |= (પૂર્ણાંક)(*(s->in)++) << s->bitcnt;          /* load eight bits */
        s->ડાબું--;
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = val >> need;
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    પાછા val & ((1 << need) - 1);
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
સંરચના huffman {
    short *ગણતરી;       /* number of symbols of each length */
    short *પ્રતીક;      /* canonically ordered symbols */
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
local પૂર્ણાંક decode(સંરચના state *s, સંરચના huffman *h)
{
    પૂર્ણાંક len;            /* current number of bits in code */
    પૂર્ણાંક code;           /* len bits being decoded */
    પૂર્ણાંક first;          /* first code of length len */
    પૂર્ણાંક ગણતરી;          /* number of codes of length len */
    પૂર્ણાંક index;          /* index of first code of length len in symbol table */
    પૂર્ણાંક bitbuf;         /* bits from stream */
    પૂર્ણાંક ડાબું;           /* bits left in next or left to process */
    short *next;        /* next number of codes */

    bitbuf = s->bitbuf;
    ડાબું = s->bitcnt;
    code = first = index = 0;
    len = 1;
    next = h->ગણતરી + 1;
    જ્યારે (1) {
        જ્યારે (ડાબું--) {
            code |= (bitbuf & 1) ^ 1;   /* invert code */
            bitbuf >>= 1;
            ગણતરી = *next++;
            જો (code < first + ગણતરી) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                પાછા h->પ્રતીક[index + (code - first)];
            }
            index += ગણતરી;             /* else update for next length */
            first += ગણતરી;
            first <<= 1;
            code <<= 1;
            len++;
        }
        ડાબું = (MAXBITS+1) - len;
        જો (ડાબું == 0) તોડો;
        જો (s->ડાબું == 0) {
            s->ડાબું = s->infun(s->inhow, &(s->in));
            જો (s->ડાબું == 0) longjmp(s->env, 1);       /* out of input */
        }
        bitbuf = *(s->in)++;
        s->ડાબું--;
        જો (ડાબું > 8) ડાબું = 8;
    }
    પાછા -9;                          /* ran out of codes */
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
local પૂર્ણાંક construct(સંરચના huffman *h, અચલ unsigned અક્ષર *rep, પૂર્ણાંક n)
{
    પૂર્ણાંક પ્રતીક;         /* current symbol when stepping through length[] */
    પૂર્ણાંક len;            /* current length when stepping through h->count[] */
    પૂર્ણાંક ડાબું;           /* number of possible codes left of current length */
    short offs[MAXBITS+1];      /* offsets in symbol table for each length */
    short length[256];  /* code lengths */

    /* convert compact repeat counts into symbol bit length list */
    પ્રતીક = 0;
    કરો {
        len = *rep++;
        ડાબું = (len >> 4) + 1;
        len &= 15;
        કરો {
            length[પ્રતીક++] = len;
        } જ્યારે (--ડાબું);
    } જ્યારે (--n);
    n = પ્રતીક;

    /* count number of codes of each length */
    માટે (len = 0; len <= MAXBITS; len++)
        h->ગણતરી[len] = 0;
    માટે (પ્રતીક = 0; પ્રતીક < n; પ્રતીક++)
        (h->ગણતરી[length[પ્રતીક]])++;   /* assumes lengths are within bounds */
    જો (h->ગણતરી[0] == n)               /* no codes! */
        પાછા 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    ડાબું = 1;                           /* one possible code of zero length */
    માટે (len = 1; len <= MAXBITS; len++) {
        ડાબું <<= 1;                     /* one more bit, double codes left */
        ડાબું -= h->ગણતરી[len];          /* deduct count from possible codes */
        જો (ડાબું < 0) પાછા ડાબું;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    માટે (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->ગણતરી[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    માટે (પ્રતીક = 0; પ્રતીક < n; પ્રતીક++)
        જો (length[પ્રતીક] != 0)
            h->પ્રતીક[offs[length[પ્રતીક]]++] = પ્રતીક;

    /* return zero for complete set, positive for incomplete set */
    પાછા ડાબું;
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
local પૂર્ણાંક decomp(સંરચના state *s)
{
    પૂર્ણાંક lit;            /* true if literals are coded */
    પૂર્ણાંક dict;           /* log2(dictionary size) - 6 */
    પૂર્ણાંક પ્રતીક;         /* decoded symbol, extra bits for distance */
    પૂર્ણાંક len;            /* length for copy */
    unsigned dist;      /* distance for copy */
    પૂર્ણાંક copy;           /* copy counter */
    unsigned અક્ષર *from, *to;   /* copy pointers */
    સ્થિર પૂર્ણાંક virgin = 1;                              /* build tables once */
    સ્થિર short litcnt[MAXBITS+1], litsym[256];        /* litcode memory */
    સ્થિર short lencnt[MAXBITS+1], lensym[16];         /* lencode memory */
    સ્થિર short distcnt[MAXBITS+1], distsym[64];       /* distcode memory */
    સ્થિર સંરચના huffman litcode = {litcnt, litsym};   /* length code */
    સ્થિર સંરચના huffman lencode = {lencnt, lensym};   /* length code */
    સ્થિર સંરચના huffman distcode = {distcnt, distsym};/* distance code */
        /* bit lengths of literal codes */
    સ્થિર અચલ unsigned અક્ષર litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};
        /* bit lengths of length codes 0..15 */
    સ્થિર અચલ unsigned અક્ષર lenlen[] = {2, 35, 36, 53, 38, 23};
        /* bit lengths of distance codes 0..63 */
    સ્થિર અચલ unsigned અક્ષર distlen[] = {2, 20, 53, 230, 247, 151, 248};
    સ્થિર અચલ short base[16] = {     /* base for length codes */
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    સ્થિર અચલ અક્ષર extra[16] = {     /* extra bits for length codes */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

    /* set up decoding tables (once--might not be thread-safe) */
    જો (virgin) {
        construct(&litcode, litlen, કદ(litlen));
        construct(&lencode, lenlen, કદ(lenlen));
        construct(&distcode, distlen, કદ(distlen));
        virgin = 0;
    }

    /* read header */
    lit = bits(s, 8);
    જો (lit > 1) પાછા -1;
    dict = bits(s, 8);
    જો (dict < 4 || dict > 6) પાછા -2;

    /* decode literals and length/distance pairs */
    કરો {
        જો (bits(s, 1)) {
            /* get length */
            પ્રતીક = decode(s, &lencode);
            len = base[પ્રતીક] + bits(s, extra[પ્રતીક]);
            જો (len == 519) તોડો;              /* end code */

            /* get distance */
            પ્રતીક = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << પ્રતીક;
            dist += bits(s, પ્રતીક);
            dist++;
            જો (s->first && dist > s->next)
                પાછા -3;              /* distance too far back */

            /* copy length bytes from distance bytes back */
            કરો {
                to = s->out + s->next;
                from = to - dist;
                copy = MAXWIN;
                જો (s->next < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->next;
                જો (copy > len) copy = len;
                len -= copy;
                s->next += copy;
                કરો {
                    *to++ = *from++;
                } જ્યારે (--copy);
                જો (s->next == MAXWIN) {
                    જો (s->outfun(s->outhow, s->out, s->next)) પાછા 1;
                    s->next = 0;
                    s->first = 0;
                }
            } જ્યારે (len != 0);
        }
        નહીંતર {
            /* get literal and write it */
            પ્રતીક = lit ? decode(s, &litcode) : bits(s, 8);
            s->out[s->next++] = પ્રતીક;
            જો (s->next == MAXWIN) {
                જો (s->outfun(s->outhow, s->out, s->next)) પાછા 1;
                s->next = 0;
                s->first = 0;
            }
        }
    } જ્યારે (1);
    પાછા 0;
}

/* See comments in blast.h */
પૂર્ણાંક blast(blast_in infun, શૂન્ય *inhow, blast_out outfun, શૂન્ય *outhow,
          unsigned *ડાબું, unsigned અક્ષર **in)
{
    સંરચના state s;             /* input/output state */
    પૂર્ણાંક err;                    /* return value */

    /* initialize input state */
    s.infun = infun;
    s.inhow = inhow;
    જો (ડાબું != ખાલી && *ડાબું) {
        s.ડાબું = *ડાબું;
        s.in = *in;
    }
    નહીંતર
        s.ડાબું = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* initialize output state */
    s.outfun = outfun;
    s.outhow = outhow;
    s.next = 0;
    s.first = 1;

    /* return if bits() or decode() tries to read past available input */
    જો (setjmp(s.env) != 0)             /* if came back here via longjmp(), */
        err = 2;                        /*  then skip decomp(), return error */
    નહીંતર
        err = decomp(&s);               /* decompress */

    /* return unused input */
    જો (ડાબું != ખાલી)
        *ડાબું = s.ડાબું;
    જો (in != ખાલી)
        *in = s.ડાબું ? s.in : ખાલી;

    /* write any leftover output and update the error code if needed */
    જો (err != 1 && s.next && s.outfun(s.outhow, s.out, s.next) && err == 0)
        err = 1;
    પાછા err;
}
