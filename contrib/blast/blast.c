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

#അന്തര്ഭാവ <stddef.h>             /* for NULL */
#അന്തര്ഭാവ <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#അന്തര്ഭാവ "blast.h"              /* prototype for blast() */

#define MAXBITS 13              /* maximum code length */
#define MAXWIN 4096             /* maximum window size */

/* input and output state */
ഘടന state {
    /* input state */
    blast_in infun;             /* input function provided by user */
    ശൂന്യം *inhow;                /* opaque information passed to infun() */
    unsigned അക്ഷരം *in;          /* next input location */
    unsigned ഇടത്;              /* available input at in */
    പൂർണ്ണസംഖ്യ bitbuf;                 /* bit buffer */
    പൂർണ്ണസംഖ്യ bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;

    /* output state */
    blast_out outfun;           /* output function provided by user */
    ശൂന്യം *outhow;               /* opaque information passed to outfun() */
    unsigned next;              /* index of next write location in out[] */
    പൂർണ്ണസംഖ്യ first;                  /* true to check distances (for first 4K) */
    unsigned അക്ഷരം out[MAXWIN];  /* output buffer and sliding window */
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
local പൂർണ്ണസംഖ്യ bits(ഘടന state *s, പൂർണ്ണസംഖ്യ need)
{
    പൂർണ്ണസംഖ്യ val;            /* bit accumulator */

    /* load at least need bits into val */
    val = s->bitbuf;
    ആകുമ്പോൾ (s->bitcnt < need) {
        എങ്കിൽ (s->ഇടത് == 0) {
            s->ഇടത് = s->infun(s->inhow, &(s->in));
            എങ്കിൽ (s->ഇടത് == 0) longjmp(s->env, 1);       /* out of input */
        }
        val |= (പൂർണ്ണസംഖ്യ)(*(s->in)++) << s->bitcnt;          /* load eight bits */
        s->ഇടത്--;
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = val >> need;
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    തിരികെ val & ((1 << need) - 1);
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
ഘടന huffman {
    short *എണ്ണം;       /* number of symbols of each length */
    short *ചിഹ്നം;      /* canonically ordered symbols */
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
local പൂർണ്ണസംഖ്യ decode(ഘടന state *s, ഘടന huffman *h)
{
    പൂർണ്ണസംഖ്യ len;            /* current number of bits in code */
    പൂർണ്ണസംഖ്യ code;           /* len bits being decoded */
    പൂർണ്ണസംഖ്യ first;          /* first code of length len */
    പൂർണ്ണസംഖ്യ എണ്ണം;          /* number of codes of length len */
    പൂർണ്ണസംഖ്യ index;          /* index of first code of length len in symbol table */
    പൂർണ്ണസംഖ്യ bitbuf;         /* bits from stream */
    പൂർണ്ണസംഖ്യ ഇടത്;           /* bits left in next or left to process */
    short *next;        /* next number of codes */

    bitbuf = s->bitbuf;
    ഇടത് = s->bitcnt;
    code = first = index = 0;
    len = 1;
    next = h->എണ്ണം + 1;
    ആകുമ്പോൾ (1) {
        ആകുമ്പോൾ (ഇടത്--) {
            code |= (bitbuf & 1) ^ 1;   /* invert code */
            bitbuf >>= 1;
            എണ്ണം = *next++;
            എങ്കിൽ (code < first + എണ്ണം) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                തിരികെ h->ചിഹ്നം[index + (code - first)];
            }
            index += എണ്ണം;             /* else update for next length */
            first += എണ്ണം;
            first <<= 1;
            code <<= 1;
            len++;
        }
        ഇടത് = (MAXBITS+1) - len;
        എങ്കിൽ (ഇടത് == 0) നിർത്തുക;
        എങ്കിൽ (s->ഇടത് == 0) {
            s->ഇടത് = s->infun(s->inhow, &(s->in));
            എങ്കിൽ (s->ഇടത് == 0) longjmp(s->env, 1);       /* out of input */
        }
        bitbuf = *(s->in)++;
        s->ഇടത്--;
        എങ്കിൽ (ഇടത് > 8) ഇടത് = 8;
    }
    തിരികെ -9;                          /* ran out of codes */
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
local പൂർണ്ണസംഖ്യ construct(ഘടന huffman *h, സ്ഥിരം unsigned അക്ഷരം *rep, പൂർണ്ണസംഖ്യ n)
{
    പൂർണ്ണസംഖ്യ ചിഹ്നം;         /* current symbol when stepping through length[] */
    പൂർണ്ണസംഖ്യ len;            /* current length when stepping through h->count[] */
    പൂർണ്ണസംഖ്യ ഇടത്;           /* number of possible codes left of current length */
    short offs[MAXBITS+1];      /* offsets in symbol table for each length */
    short length[256];  /* code lengths */

    /* convert compact repeat counts into symbol bit length list */
    ചിഹ്നം = 0;
    ചെയ്യുക {
        len = *rep++;
        ഇടത് = (len >> 4) + 1;
        len &= 15;
        ചെയ്യുക {
            length[ചിഹ്നം++] = len;
        } ആകുമ്പോൾ (--ഇടത്);
    } ആകുമ്പോൾ (--n);
    n = ചിഹ്നം;

    /* count number of codes of each length */
    വേണ്ടി (len = 0; len <= MAXBITS; len++)
        h->എണ്ണം[len] = 0;
    വേണ്ടി (ചിഹ്നം = 0; ചിഹ്നം < n; ചിഹ്നം++)
        (h->എണ്ണം[length[ചിഹ്നം]])++;   /* assumes lengths are within bounds */
    എങ്കിൽ (h->എണ്ണം[0] == n)               /* no codes! */
        തിരികെ 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    ഇടത് = 1;                           /* one possible code of zero length */
    വേണ്ടി (len = 1; len <= MAXBITS; len++) {
        ഇടത് <<= 1;                     /* one more bit, double codes left */
        ഇടത് -= h->എണ്ണം[len];          /* deduct count from possible codes */
        എങ്കിൽ (ഇടത് < 0) തിരികെ ഇടത്;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    വേണ്ടി (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->എണ്ണം[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    വേണ്ടി (ചിഹ്നം = 0; ചിഹ്നം < n; ചിഹ്നം++)
        എങ്കിൽ (length[ചിഹ്നം] != 0)
            h->ചിഹ്നം[offs[length[ചിഹ്നം]]++] = ചിഹ്നം;

    /* return zero for complete set, positive for incomplete set */
    തിരികെ ഇടത്;
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
local പൂർണ്ണസംഖ്യ decomp(ഘടന state *s)
{
    പൂർണ്ണസംഖ്യ lit;            /* true if literals are coded */
    പൂർണ്ണസംഖ്യ dict;           /* log2(dictionary size) - 6 */
    പൂർണ്ണസംഖ്യ ചിഹ്നം;         /* decoded symbol, extra bits for distance */
    പൂർണ്ണസംഖ്യ len;            /* length for copy */
    unsigned dist;      /* distance for copy */
    പൂർണ്ണസംഖ്യ copy;           /* copy counter */
    unsigned അക്ഷരം *from, *to;   /* copy pointers */
    സ്ഥിതം പൂർണ്ണസംഖ്യ virgin = 1;                              /* build tables once */
    സ്ഥിതം short litcnt[MAXBITS+1], litsym[256];        /* litcode memory */
    സ്ഥിതം short lencnt[MAXBITS+1], lensym[16];         /* lencode memory */
    സ്ഥിതം short distcnt[MAXBITS+1], distsym[64];       /* distcode memory */
    സ്ഥിതം ഘടന huffman litcode = {litcnt, litsym};   /* length code */
    സ്ഥിതം ഘടന huffman lencode = {lencnt, lensym};   /* length code */
    സ്ഥിതം ഘടന huffman distcode = {distcnt, distsym};/* distance code */
        /* bit lengths of literal codes */
    സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം litlen[] = {
        11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
        9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
        7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
        8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
        44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
        44, 173};
        /* bit lengths of length codes 0..15 */
    സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം lenlen[] = {2, 35, 36, 53, 38, 23};
        /* bit lengths of distance codes 0..63 */
    സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം distlen[] = {2, 20, 53, 230, 247, 151, 248};
    സ്ഥിതം സ്ഥിരം short base[16] = {     /* base for length codes */
        3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264};
    സ്ഥിതം സ്ഥിരം അക്ഷരം extra[16] = {     /* extra bits for length codes */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

    /* set up decoding tables (once--might not be thread-safe) */
    എങ്കിൽ (virgin) {
        construct(&litcode, litlen, വലുപ്പം(litlen));
        construct(&lencode, lenlen, വലുപ്പം(lenlen));
        construct(&distcode, distlen, വലുപ്പം(distlen));
        virgin = 0;
    }

    /* read header */
    lit = bits(s, 8);
    എങ്കിൽ (lit > 1) തിരികെ -1;
    dict = bits(s, 8);
    എങ്കിൽ (dict < 4 || dict > 6) തിരികെ -2;

    /* decode literals and length/distance pairs */
    ചെയ്യുക {
        എങ്കിൽ (bits(s, 1)) {
            /* get length */
            ചിഹ്നം = decode(s, &lencode);
            len = base[ചിഹ്നം] + bits(s, extra[ചിഹ്നം]);
            എങ്കിൽ (len == 519) നിർത്തുക;              /* end code */

            /* get distance */
            ചിഹ്നം = len == 2 ? 2 : dict;
            dist = decode(s, &distcode) << ചിഹ്നം;
            dist += bits(s, ചിഹ്നം);
            dist++;
            എങ്കിൽ (s->first && dist > s->next)
                തിരികെ -3;              /* distance too far back */

            /* copy length bytes from distance bytes back */
            ചെയ്യുക {
                to = s->out + s->next;
                from = to - dist;
                copy = MAXWIN;
                എങ്കിൽ (s->next < dist) {
                    from += copy;
                    copy = dist;
                }
                copy -= s->next;
                എങ്കിൽ (copy > len) copy = len;
                len -= copy;
                s->next += copy;
                ചെയ്യുക {
                    *to++ = *from++;
                } ആകുമ്പോൾ (--copy);
                എങ്കിൽ (s->next == MAXWIN) {
                    എങ്കിൽ (s->outfun(s->outhow, s->out, s->next)) തിരികെ 1;
                    s->next = 0;
                    s->first = 0;
                }
            } ആകുമ്പോൾ (len != 0);
        }
        അല്ലെങ്കിൽ {
            /* get literal and write it */
            ചിഹ്നം = lit ? decode(s, &litcode) : bits(s, 8);
            s->out[s->next++] = ചിഹ്നം;
            എങ്കിൽ (s->next == MAXWIN) {
                എങ്കിൽ (s->outfun(s->outhow, s->out, s->next)) തിരികെ 1;
                s->next = 0;
                s->first = 0;
            }
        }
    } ആകുമ്പോൾ (1);
    തിരികെ 0;
}

/* See comments in blast.h */
പൂർണ്ണസംഖ്യ blast(blast_in infun, ശൂന്യം *inhow, blast_out outfun, ശൂന്യം *outhow,
          unsigned *ഇടത്, unsigned അക്ഷരം **in)
{
    ഘടന state s;             /* input/output state */
    പൂർണ്ണസംഖ്യ err;                    /* return value */

    /* initialize input state */
    s.infun = infun;
    s.inhow = inhow;
    എങ്കിൽ (ഇടത് != ശൂന്യ && *ഇടത്) {
        s.ഇടത് = *ഇടത്;
        s.in = *in;
    }
    അല്ലെങ്കിൽ
        s.ഇടത് = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* initialize output state */
    s.outfun = outfun;
    s.outhow = outhow;
    s.next = 0;
    s.first = 1;

    /* return if bits() or decode() tries to read past available input */
    എങ്കിൽ (setjmp(s.env) != 0)             /* if came back here via longjmp(), */
        err = 2;                        /*  then skip decomp(), return error */
    അല്ലെങ്കിൽ
        err = decomp(&s);               /* decompress */

    /* return unused input */
    എങ്കിൽ (ഇടത് != ശൂന്യ)
        *ഇടത് = s.ഇടത്;
    എങ്കിൽ (in != ശൂന്യ)
        *in = s.ഇടത് ? s.in : ശൂന്യ;

    /* write any leftover output and update the error code if needed */
    എങ്കിൽ (err != 1 && s.next && s.outfun(s.outhow, s.out, s.next) && err == 0)
        err = 1;
    തിരികെ err;
}
