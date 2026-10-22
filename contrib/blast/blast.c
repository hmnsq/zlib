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

#شامل <stddef.h>             /* for NULL */
#شامل <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#شامل "blast.h"              /* prototype for blast() */

#تعریف_کرو MAXBITS 13              /* maximum code length */
#تعریف_کرو MAXWIN 4096             /* maximum window size */

/* input and output state */
ساخت state {
    /* input state */
    blast_اندر infun;             /* input function provided by user */
    خالی *inhow;                /* opaque information passed to infun() */
    غیرعلامتی حرف *اندر;          /* next input location */
    غیرعلامتی بایاں;              /* available input at in */
    صحیح bitbuf;                 /* bit buffer */
    صحیح bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;

    /* output state */
    blast_برآمد outfun;           /* output function provided by user */
    خالی *outhow;               /* opaque information passed to outfun() */
    غیرعلامتی اگلا;              /* index of next write location in out[] */
    صحیح پہلا;                  /* true to check distances (for first 4K) */
    غیرعلامتی حرف برآمد[MAXWIN];  /* output buffer and sliding window */
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
local صحیح بٹ(ساخت state *s, صحیح need)
{
    صحیح val;            /* bit accumulator */

    /* load at least need bits into val */
    val = s->bitbuf;
    جب (s->bitcnt < need) {
        اگر (s->بایاں == 0) {
            s->بایاں = s->infun(s->inhow, &(s->اندر));
            اگر (s->بایاں == 0) longjmp(s->env, 1);       /* out of input */
        }
        val |= (صحیح)(*(s->اندر)++) << s->bitcnt;          /* load eight bits */
        s->بایاں--;
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = val >> need;
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    واپس val & ((1 << need) - 1);
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
ساخت huffman {
    مختصر *گنتی;       /* number of symbols of each length */
    مختصر *علامت;      /* canonically ordered symbols */
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
local صحیح decode(ساخت state *s, ساخت huffman *h)
{
    صحیح len;            /* current number of bits in code */
    صحیح رمز;           /* len bits being decoded */
    صحیح پہلا;          /* first code of length len */
    صحیح گنتی;          /* number of codes of length len */
    صحیح اشاریہ;          /* index of first code of length len in symbol table */
    صحیح bitbuf;         /* bits from stream */
    صحیح بایاں;           /* bits left in next or left to process */
    مختصر *اگلا;        /* next number of codes */

    bitbuf = s->bitbuf;
    بایاں = s->bitcnt;
    رمز = پہلا = اشاریہ = 0;
    len = 1;
    اگلا = h->گنتی + 1;
    جب (1) {
        جب (بایاں--) {
            رمز |= (bitbuf & 1) ^ 1;   /* invert code */
            bitbuf >>= 1;
            گنتی = *اگلا++;
            اگر (رمز < پہلا + گنتی) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                واپس h->علامت[اشاریہ + (رمز - پہلا)];
            }
            اشاریہ += گنتی;             /* else update for next length */
            پہلا += گنتی;
            پہلا <<= 1;
            رمز <<= 1;
            len++;
        }
        بایاں = (MAXBITS+1) - len;
        اگر (بایاں == 0) توڑو;
        اگر (s->بایاں == 0) {
            s->بایاں = s->infun(s->inhow, &(s->اندر));
            اگر (s->بایاں == 0) longjmp(s->env, 1);       /* out of input */
        }
        bitbuf = *(s->اندر)++;
        s->بایاں--;
        اگر (بایاں > 8) بایاں = 8;
    }
    واپس -9;                          /* ran out of codes */
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
local صحیح construct(ساخت huffman *h, ثابت غیرعلامتی حرف *rep, صحیح n)
{
    صحیح علامت;         /* current symbol when stepping through length[] */
    صحیح len;            /* current length when stepping through h->count[] */
    صحیح بایاں;           /* number of possible codes left of current length */
    مختصر offs[MAXBITS+1];      /* offsets in symbol table for each length */
    مختصر لمبائی[256];  /* code lengths */

    /* convert compact repeat counts into symbol bit length list */
    علامت = 0;
    کرو {
        len = *rep++;
        بایاں = (len >> 4) + 1;
        len &= 15;
        کرو {
            لمبائی[علامت++] = len;
        } جب (--بایاں);
    } جب (--n);
    n = علامت;

    /* count number of codes of each length */
    برائے (len = 0; len <= MAXBITS; len++)
        h->گنتی[len] = 0;
    برائے (علامت = 0; علامت < n; علامت++)
        (h->گنتی[لمبائی[علامت]])++;   /* assumes lengths are within bounds */
    اگر (h->گنتی[0] == n)               /* no codes! */
        واپس 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    بایاں = 1;                           /* one possible code of zero length */
    برائے (len = 1; len <= MAXBITS; len++) {
        بایاں <<= 1;                     /* one more bit, double codes left */
        بایاں -= h->گنتی[len];          /* deduct count from possible codes */
        اگر (بایاں < 0) واپس بایاں;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    برائے (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->گنتی[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    برائے (علامت = 0; علامت < n; علامت++)
        اگر (لمبائی[علامت] != 0)
            h->علامت[offs[لمبائی[علامت]]++] = علامت;

    /* return zero for complete set, positive for incomplete set */
    واپس بایاں;
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
local صحیح decomp(ساخت state *s)
{
    صحیح lit;            /* true if literals are coded */
    صحیح dict;           /* log2(dictionary size) - 6 */
    صحیح علامت;         /* decoded symbol, extra bits for distance */
    صحیح len;            /* length for copy */
    غیرعلامتی dist;      /* distance for copy */
    صحیح copy;           /* copy counter */
    غیرعلامتی حرف *from, *مقصد;   /* copy pointers */
    ساکن صحیح virgin = 1;                              /* build tables once */
    ساکن مختصر litcnt[MAXBITS+1], litsym[256];        /* litcode memory */
    ساکن مختصر lencnt[MAXBITS+1], lensym[16];         /* lencode memory */
    ساکن مختصر distcnt[MAXBITS+1], distsym[64];       /* distcode memory */
    ساکن ساخت huffman litcode = {litcnt, litsym};   /* length code */
    ساکن ساخت huffman lencode = {lencnt, lensym};   /* length code */
    ساکن ساخت huffman distcode = {distcnt, distsym};/* distance code */
        /* bit lengths of literal codes */
    ساکن ثابت غیرعلامتی حرف litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};
        /* bit lengths of length codes 0..15 */
    ساکن ثابت غیرعلامتی حرف lenlen[] = {2, 35, 36, 53, 38, 23};
        /* bit lengths of distance codes 0..63 */
    ساکن ثابت غیرعلامتی حرف distlen[] = {2, 20, 53, 230, 247, 151, 248};
    ساکن ثابت مختصر بنیاد_2[16] = {     /* base for length codes */
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    ساکن ثابت حرف extra[16] = {     /* extra bits for length codes */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

    /* set up decoding tables (once--might not be thread-safe) */
    اگر (virgin) {
        construct(&litcode, litlen, حجم(litlen));
        construct(&lencode, lenlen, حجم(lenlen));
        construct(&distcode, distlen, حجم(distlen));
        virgin = 0;
    }

    /* read header */
    lit = بٹ(s, 8);
    اگر (lit > 1) واپس -1;
    dict = بٹ(s, 8);
    اگر (dict < 4 || dict > 6) واپس -2;

    /* decode literals and length/distance pairs */
    کرو {
        اگر (بٹ(s, 1)) {
            /* get length */
            علامت = decode(s, &lencode);
            len = بنیاد_2[علامت] + بٹ(s, extra[علامت]);
            اگر (len == 519) توڑو;              /* end code */

            /* get distance */
            علامت = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << علامت;
            dist += بٹ(s, علامت);
            dist++;
            اگر (s->پہلا && dist > s->اگلا)
                واپس -3;              /* distance too far back */

            /* copy length bytes from distance bytes back */
            کرو {
                مقصد = s->برآمد + s->اگلا;
                from = مقصد - dist;
                copy = MAXWIN;
                اگر (s->اگلا < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->اگلا;
                اگر (copy > len) copy = len;
                len -= copy;
                s->اگلا += copy;
                کرو {
                    *مقصد++ = *from++;
                } جب (--copy);
                اگر (s->اگلا == MAXWIN) {
                    اگر (s->outfun(s->outhow, s->برآمد, s->اگلا)) واپس 1;
                    s->اگلا = 0;
                    s->پہلا = 0;
                }
            } جب (len != 0);
        }
        ورنہ {
            /* get literal and write it */
            علامت = lit ? decode(s, &litcode) : بٹ(s, 8);
            s->برآمد[s->اگلا++] = علامت;
            اگر (s->اگلا == MAXWIN) {
                اگر (s->outfun(s->outhow, s->برآمد, s->اگلا)) واپس 1;
                s->اگلا = 0;
                s->پہلا = 0;
            }
        }
    } جب (1);
    واپس 0;
}

/* See comments in blast.h */
صحیح blast(blast_اندر infun, خالی *inhow, blast_برآمد outfun, خالی *outhow,
          غیرعلامتی *بایاں, غیرعلامتی حرف **اندر)
{
    ساخت state s;             /* input/output state */
    صحیح err;                    /* return value */

    /* initialize input state */
    s.infun = infun;
    s.inhow = inhow;
    اگر (بایاں != تہی && *بایاں) {
        s.بایاں = *بایاں;
        s.اندر = *اندر;
    }
    ورنہ
        s.بایاں = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* initialize output state */
    s.outfun = outfun;
    s.outhow = outhow;
    s.اگلا = 0;
    s.پہلا = 1;

    /* return if bits() or decode() tries to read past available input */
    اگر (setjmp(s.env) != 0)             /* if came back here via longjmp(), */
        err = 2;                        /*  then skip decomp(), return error */
    ورنہ
        err = decomp(&s);               /* decompress */

    /* return unused input */
    اگر (بایاں != تہی)
        *بایاں = s.بایاں;
    اگر (اندر != تہی)
        *اندر = s.بایاں ? s.اندر : تہی;

    /* write any leftover output and update the error code if needed */
    اگر (err != 1 && s.اگلا && s.outfun(s.outhow, s.برآمد, s.اگلا) && err == 0)
        err = 1;
    واپس err;
}
