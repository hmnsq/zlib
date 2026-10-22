/*
 * puff.c
 * Copyright (C) 2002-2013 Mark Adler
 * For conditions of distribution and use, see copyright notice in puff.h
 * version 2.3, 21 Jan 2013
 *
 * puff.c is a simple inflate written to be an unambiguous way to specify the
 * deflate format.  It is not written for speed but rather simplicity.  As a
 * side benefit, this code might actually be useful when small code is more
 * important than speed, such as bootstrap applications.  For typical deflate
 * data, zlib's inflate() is about four times as fast as puff().  zlib's
 * inflate compiles to around 20K on my machine, whereas puff.c compiles to
 * around 4K on my machine (a PowerPC using GNU cc).  If the faster decode()
 * function here is used, then puff() is only twice as slow as zlib's
 * inflate().
 *
 * All dynamically allocated memory comes from the stack.  The stack required
 * is less than 2K bytes.  This code is compatible with 16-bit int's and
 * assumes that long's are at least 32 bits.  puff.c uses the short data type,
 * assumed to be 16 bits, for arrays in order to conserve memory.  The code
 * works whether integers are stored big endian or little endian.
 *
 * In the comments below are "Format notes" that describe the inflate process
 * and document some of the less obvious aspects of the format.  This source
 * code is meant to supplement RFC 1951, which formally describes the deflate
 * format:
 *
 *    https://datatracker.ietf.org/doc/html/rfc1951
 */

/*
 * Change history:
 *
 * 1.0  10 Feb 2002     - First version
 * 1.1  17 Feb 2002     - Clarifications of some comments and notes
 *                      - Update puff() dest and source pointers on negative
 *                        errors to facilitate debugging deflators
 *                      - Remove longest from struct huffman -- not needed
 *                      - Simplify offs[] index in construct()
 *                      - Add input size and checking, using longjmp() to
 *                        maintain easy readability
 *                      - Use short data type for large arrays
 *                      - Use pointers instead of long to specify source and
 *                        destination sizes to avoid arbitrary 4 GB limits
 * 1.2  17 Mar 2002     - Add faster version of decode(), doubles speed (!),
 *                        but leave simple version for readability
 *                      - Make sure invalid distances detected if pointers
 *                        are 16 bits
 *                      - Fix fixed codes table error
 *                      - Provide a scanning mode for determining size of
 *                        uncompressed data
 * 1.3  20 Mar 2002     - Go back to lengths for puff() parameters [Gailly]
 *                      - Add a puff.h file for the interface
 *                      - Add braces in puff() for else do [Gailly]
 *                      - Use indexes instead of pointers for readability
 * 1.4  31 Mar 2002     - Simplify construct() code set check
 *                      - Fix some comments
 *                      - Add FIXLCODES #define
 * 1.5   6 Apr 2002     - Minor comment fixes
 * 1.6   7 Aug 2002     - Minor format changes
 * 1.7   3 Mar 2003     - Added test code for distribution
 *                      - Added zlib-like license
 * 1.8   9 Jan 2004     - Added some comments on no distance codes case
 * 1.9  21 Feb 2008     - Fix bug on 16-bit integer architectures [Pohland]
 *                      - Catch missing end-of-block symbol error
 * 2.0  25 Jul 2008     - Add #define to permit distance too far back
 *                      - Add option in TEST code for puff to write the data
 *                      - Add option in TEST code to skip input bytes
 *                      - Allow TEST code to read from piped stdin
 * 2.1   4 Apr 2010     - Avoid variable initialization for happier compilers
 *                      - Avoid unsigned comparisons for even happier compilers
 * 2.2  25 Apr 2010     - Fix bug in variable initializations [Oberhumer]
 *                      - Add const where appropriate [Oberhumer]
 *                      - Split if's and ?'s for coverage testing
 *                      - Break out test code to separate file
 *                      - Move NIL to puff.h
 *                      - Allow incomplete code only if single code length is 1
 *                      - Add full code coverage test to Makefile
 * 2.3  21 Jan 2013     - Check for invalid code length codes in dynamic blocks
 */

#ضمن <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */
#ضمن "puff.h"               /* prototype for puff() */

#تعریف_کن local ساكن            /* for local function definitions */

/*
 * Maximums for allocations and loops.  It is not useful to change these --
 * they are fixed by the deflate format.
 */
#تعریف_کن MAXBITS 15              /* maximum bits in a code */
#تعریف_کن MAXLCODES 286           /* maximum number of literal/length codes */
#تعریف_کن MAXDCODES 30            /* maximum number of distance codes */
#تعریف_کن MAXCODES (MAXLCODES+MAXDCODES)  /* maximum codes lengths to read */
#تعریف_کن FIXLCODES 288           /* number of fixed literal/length codes */

/* input and output state */
بنية state {
    /* output state */
    بیعلامت حرف *بیرون;         /* output buffer */
    بیعلامت دراز outlen;       /* available space at out */
    بیعلامت دراز outcnt;       /* bytes written to out so far */

    /* input state */
    ثابت بیعلامت حرف *درون;    /* input buffer */
    بیعلامت دراز inlen;        /* available input at in */
    بیعلامت دراز incnt;        /* bytes read so far */
    صحيح bitbuf;                 /* bit buffer */
    صحيح bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;
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
    دراز val;           /* bit accumulator (can use up to 20 bits) */

    /* load at least need bits into val */
    val = s->bitbuf;
    طالما (s->bitcnt < need) {
        إذا (s->incnt == s->inlen)
            longjmp(s->env, 1);         /* out of input */
        val |= (دراز)(s->درون[s->incnt++]) << s->bitcnt;  /* load eight bits */
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = (صحيح)(val >> need);
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    أرجع (صحيح)(val & ((1L << need) - 1));
}

/*
 * Process a stored block.
 *
 * Format notes:
 *
 * - After the two-bit stored block type (00), the stored block length and
 *   stored bytes are byte-aligned for fast copying.  Therefore any leftover
 *   bits in the byte that has the last bit of the type, as many as seven, are
 *   discarded.  The value of the discarded bits are not defined and should not
 *   be checked against any expectation.
 *
 * - The second inverted copy of the stored block length does not have to be
 *   checked, but it's probably a good idea to do so anyway.
 *
 * - A stored block can have zero length.  This is sometimes used to byte-align
 *   subsets of the compressed data for random access or partial recovery.
 */
local صحيح stored(بنية state *s)
{
    بیعلامت len;       /* length of stored block */

    /* discard leftover bits from current byte (assumes s->bitcnt < 8) */
    s->bitbuf = 0;
    s->bitcnt = 0;

    /* get length and check against its one's complement */
    إذا (s->incnt + 4 > s->inlen)
        أرجع 2;                               /* not enough input */
    len = s->درون[s->incnt++];
    len |= s->درون[s->incnt++] << 8;
    إذا (s->درون[s->incnt++] != (~len & 0xff) ||
        s->درون[s->incnt++] != ((~len >> 8) & 0xff))
        أرجع -2;                              /* didn't match complement! */

    /* copy len bytes from in to out */
    إذا (s->incnt + len > s->inlen)
        أرجع 2;                               /* not enough input */
    إذا (s->بیرون != NIL) {
        إذا (s->outcnt + len > s->outlen)
            أرجع 1;                           /* not enough output space */
        طالما (len--)
            s->بیرون[s->outcnt++] = s->درون[s->incnt++];
    }
    وإلا {                                      /* just scanning */
        s->outcnt += len;
        s->incnt += len;
    }

    /* done with a valid stored block */
    أرجع 0;
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
 * then -10 is returned after reading MAXBITS bits.
 *
 * Format notes:
 *
 * - The codes as stored in the compressed data are bit-reversed relative to
 *   a simple integer ordering of codes of the same lengths.  Hence below the
 *   bits are pulled from the compressed data one at a time and used to
 *   build the code value reversed from what is in the stream in order to
 *   permit simple integer comparisons for decoding.  A table-based decoding
 *   scheme (as used in zlib) does not need to do this reversal.
 *
 * - The first code for the shortest length is all zeros.  Subsequent codes of
 *   the same length are simply integer increments of the previous code.  When
 *   moving up a length, a zero bit is appended to the code.  For a complete
 *   code, the last code of the longest length will be all ones.
 *
 * - Incomplete codes are handled by this decoder, since they are permitted
 *   in the deflate format.  See the format notes for fixed() and dynamic().
 */
#اگر_تعریف_شده SLOW
local صحيح decode(بنية state *s, ثابت بنية huffman *h)
{
    صحيح len;            /* current number of bits in code */
    صحيح کد;           /* len bits being decoded */
    صحيح اولی;          /* first code of length len */
    صحيح شمار;          /* number of codes of length len */
    صحيح شاخص;          /* index of first code of length len in symbol table */

    کد = اولی = شاخص = 0;
    لكل (len = 1; len <= MAXBITS; len++) {
        کد |= بیتها(s, 1);             /* get next bit */
        شمار = h->شمار[len];
        إذا (کد - شمار < اولی)       /* if length len, return symbol */
            أرجع h->نماد[شاخص + (کد - اولی)];
        شاخص += شمار;                 /* else update for next length */
        اولی += شمار;
        اولی <<= 1;
        کد <<= 1;
    }
    أرجع -10;                         /* ran out of codes */
}

/*
 * A faster version of decode() for real applications of this code.   It's not
 * as readable, but it makes puff() twice as fast.  And it only makes the code
 * a few percent larger.
 */
#وإلا /* !SLOW */
local صحيح decode(بنية state *s, ثابت بنية huffman *h)
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
            کد |= bitbuf & 1;
            bitbuf >>= 1;
            شمار = *بعدی++;
            إذا (کد - شمار < اولی) { /* if length len, return symbol */
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
        إذا (چپ == 0)
            اكسر;
        إذا (s->incnt == s->inlen)
            longjmp(s->env, 1);         /* out of input */
        bitbuf = s->درون[s->incnt++];
        إذا (چپ > 8)
            چپ = 8;
    }
    أرجع -10;                         /* ran out of codes */
}
#پایان_اگر /* SLOW */

/*
 * Given the list of code lengths length[0..n-1] representing a canonical
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
 *
 * Not used by decode(), but used for error checking, h->count[0] is the number
 * of the n symbols not in the code.  So n - h->count[0] is the number of
 * codes.  This is useful for checking for incomplete codes that have more than
 * one symbol, which is an error in a dynamic block.
 *
 * Assumption: for all i in 0..n-1, 0 <= length[i] <= MAXBITS
 * This is assured by the construction of the length arrays in dynamic() and
 * fixed() and is not verified by construct().
 *
 * Format notes:
 *
 * - Permitted and expected examples of incomplete codes are one of the fixed
 *   codes and any code with a single symbol which in deflate is coded as one
 *   bit instead of zero bits.  See the format notes for fixed() and dynamic().
 *
 * - Within a given code length, the symbols are kept in ascending order for
 *   the code bits definition.
 */
local صحيح construct(بنية huffman *h, ثابت کوتاه *درازی, صحيح n)
{
    صحيح نماد;         /* current symbol when stepping through length[] */
    صحيح len;            /* current length when stepping through h->count[] */
    صحيح چپ;           /* number of possible codes left of current length */
    کوتاه offs[MAXBITS+1];      /* offsets in symbol table for each length */

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
        إذا (چپ < 0)
            أرجع چپ;                /* over-subscribed--return negative */
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
 * Decode literal/length and distance codes until an end-of-block code.
 *
 * Format notes:
 *
 * - Compressed data that is after the block type if fixed or after the code
 *   description if dynamic is a combination of literals and length/distance
 *   pairs terminated by and end-of-block code.  Literals are simply Huffman
 *   coded bytes.  A length/distance pair is a coded length followed by a
 *   coded distance to represent a string that occurs earlier in the
 *   uncompressed data that occurs again at the current location.
 *
 * - Literals, lengths, and the end-of-block code are combined into a single
 *   code of up to 286 symbols.  They are 256 literals (0..255), 29 length
 *   symbols (257..285), and the end-of-block symbol (256).
 *
 * - There are 256 possible lengths (3..258), and so 29 symbols are not enough
 *   to represent all of those.  Lengths 3..10 and 258 are in fact represented
 *   by just a length symbol.  Lengths 11..257 are represented as a symbol and
 *   some number of extra bits that are added as an integer to the base length
 *   of the length symbol.  The number of extra bits is determined by the base
 *   length symbol.  These are in the static arrays below, lens[] for the base
 *   lengths and lext[] for the corresponding number of extra bits.
 *
 * - The reason that 258 gets its own symbol is that the longest length is used
 *   often in highly redundant files.  Note that 258 can also be coded as the
 *   base value 227 plus the maximum extra value of 31.  While a good deflate
 *   should never do this, it is not an error, and should be decoded properly.
 *
 * - If a length is decoded, including its extra bits if any, then it is
 *   followed a distance code.  There are up to 30 distance symbols.  Again
 *   there are many more possible distances (1..32768), so extra bits are added
 *   to a base value represented by the symbol.  The distances 1..4 get their
 *   own symbol, but the rest require extra bits.  The base distances and
 *   corresponding number of extra bits are below in the static arrays dist[]
 *   and dext[].
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
 *   allowed and common.  For example, a distance of one and a length of 258
 *   simply copies the last byte 258 times.  A distance of four and a length of
 *   twelve copies the last four bytes three times.  A simple forward copy
 *   ignoring whether the length is greater than the distance or not implements
 *   this correctly.  You should not use memcpy() since its behavior is not
 *   defined for overlapped arrays.  You should not use memmove() or bcopy()
 *   since though their behavior -is- defined for overlapping arrays, it is
 *   defined to do the wrong thing in this case.
 */
local صحيح codes(بنية state *s,
                ثابت بنية huffman *lencode,
                ثابت بنية huffman *distcode)
{
    صحيح نماد;         /* decoded symbol */
    صحيح len;            /* length for copy */
    بیعلامت dist;      /* distance for copy */
    ساكن ثابت کوتاه lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    ساكن ثابت کوتاه lext[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    ساكن ثابت کوتاه dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    ساكن ثابت کوتاه dext[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

    /* decode literals and length/distance pairs */
    افعل {
        نماد = decode(s, lencode);
        إذا (نماد < 0)
            أرجع نماد;              /* invalid symbol */
        إذا (نماد < 256) {             /* literal: symbol is the byte */
            /* write out the literal */
            إذا (s->بیرون != NIL) {
                إذا (s->outcnt == s->outlen)
                    أرجع 1;
                s->بیرون[s->outcnt] = نماد;
            }
            s->outcnt++;
        }
        وإلا إذا (نماد > 256) {        /*درازی*/
            /* get and compute length */
            نماد -= 257;
            إذا (نماد >= 29)
                أرجع -10;             /* invalid fixed code */
            len = lens[نماد] + بیتها(s, lext[نماد]);

            /* get and check distance */
            نماد = decode(s, distcode);
            إذا (نماد < 0)
                أرجع نماد;          /* invalid symbol */
            dist = dists[نماد] + بیتها(s, dext[نماد]);
#اگر_تعریف_نشده INFLATE_ALLOW_INVALID_فاصله_TOOFAR_ARRR
            إذا (dist > s->outcnt)
                أرجع -11;     /* distance too far back */
#پایان_اگر

            /* copy length bytes from distance bytes back */
            إذا (s->بیرون != NIL) {
                إذا (s->outcnt + len > s->outlen)
                    أرجع 1;
                طالما (len--) {
                    s->بیرون[s->outcnt] =
#اگر_تعریف_شده INFLATE_ALLOW_INVALID_فاصله_TOOFAR_ARRR
                        dist > s->outcnt ?
                            0 :
#پایان_اگر
                            s->بیرون[s->outcnt - dist];
                    s->outcnt++;
                }
            }
            وإلا
                s->outcnt += len;
        }
    } طالما (نماد != 256);            /* end of block symbol */

    /* done with a valid fixed or dynamic block */
    أرجع 0;
}

/*
 * Process a fixed codes block.
 *
 * Format notes:
 *
 * - This block type can be useful for compressing small amounts of data for
 *   which the size of the code descriptions in a dynamic block exceeds the
 *   benefit of custom codes for that block.  For fixed codes, no bits are
 *   spent on code descriptions.  Instead the code lengths for literal/length
 *   codes and distance codes are fixed.  The specific lengths for each symbol
 *   can be seen in the "for" loops below.
 *
 * - The literal/length code is complete, but has two symbols that are invalid
 *   and should result in an error if received.  This cannot be implemented
 *   simply as an incomplete code since those two symbols are in the "middle"
 *   of the code.  They are eight bits long and the longest literal/length\
 *   code is nine bits.  Therefore the code must be constructed with those
 *   symbols, and the invalid symbols must be detected after decoding.
 *
 * - The fixed distance codes also have two invalid symbols that should result
 *   in an error if received.  Since all of the distance codes are the same
 *   length, this can be implemented as an incomplete code.  Then the invalid
 *   codes are detected while decoding.
 */
local صحيح fixed(بنية state *s)
{
    ساكن صحيح virgin = 1;
    ساكن کوتاه lencnt[MAXBITS+1], lensym[FIXLCODES];
    ساكن کوتاه distcnt[MAXBITS+1], distsym[MAXDCODES];
    ساكن بنية huffman lencode, distcode;

    /* build fixed huffman tables if first call (may not be thread safe) */
    إذا (virgin) {
        صحيح نماد;
        کوتاه lengths[FIXLCODES];

        /* construct lencode and distcode */
        lencode.شمار = lencnt;
        lencode.نماد = lensym;
        distcode.شمار = distcnt;
        distcode.نماد = distsym;

        /* literal/length table */
        لكل (نماد = 0; نماد < 144; نماد++)
            lengths[نماد] = 8;
        لكل (; نماد < 256; نماد++)
            lengths[نماد] = 9;
        لكل (; نماد < 280; نماد++)
            lengths[نماد] = 7;
        لكل (; نماد < FIXLCODES; نماد++)
            lengths[نماد] = 8;
        construct(&lencode, lengths, FIXLCODES);

        /* distance table */
        لكل (نماد = 0; نماد < MAXDCODES; نماد++)
            lengths[نماد] = 5;
        construct(&distcode, lengths, MAXDCODES);

        /* do this just once */
        virgin = 0;
    }

    /* decode data until end-of-block code */
    أرجع codes(s, &lencode, &distcode);
}

/*
 * Process a dynamic codes block.
 *
 * Format notes:
 *
 * - A dynamic block starts with a description of the literal/length and
 *   distance codes for that block.  New dynamic blocks allow the compressor to
 *   rapidly adapt to changing data with new codes optimized for that data.
 *
 * - The codes used by the deflate format are "canonical", which means that
 *   the actual bits of the codes are generated in an unambiguous way simply
 *   from the number of bits in each code.  Therefore the code descriptions
 *   are simply a list of code lengths for each symbol.
 *
 * - The code lengths are stored in order for the symbols, so lengths are
 *   provided for each of the literal/length symbols, and for each of the
 *   distance symbols.
 *
 * - If a symbol is not used in the block, this is represented by a zero as the
 *   code length.  This does not mean a zero-length code, but rather that no
 *   code should be created for this symbol.  There is no way in the deflate
 *   format to represent a zero-length code.
 *
 * - The maximum number of bits in a code is 15, so the possible lengths for
 *   any code are 1..15.
 *
 * - The fact that a length of zero is not permitted for a code has an
 *   interesting consequence.  Normally if only one symbol is used for a given
 *   code, then in fact that code could be represented with zero bits.  However
 *   in deflate, that code has to be at least one bit.  So for example, if
 *   only a single distance base symbol appears in a block, then it will be
 *   represented by a single code of length one, in particular one 0 bit.  This
 *   is an incomplete code, since if a 1 bit is received, it has no meaning,
 *   and should result in an error.  So incomplete distance codes of one symbol
 *   should be permitted, and the receipt of invalid codes should be handled.
 *
 * - It is also possible to have a single literal/length code, but that code
 *   must be the end-of-block code, since every dynamic block has one.  This
 *   is not the most efficient way to create an empty block (an empty fixed
 *   block is fewer bits), but it is allowed by the format.  So incomplete
 *   literal/length codes of one symbol should also be permitted.
 *
 * - If there are only literal codes and no lengths, then there are no distance
 *   codes.  This is represented by one distance code with zero bits.
 *
 * - The list of up to 286 length/literal lengths and up to 30 distance lengths
 *   are themselves compressed using Huffman codes and run-length encoding.  In
 *   the list of code lengths, a 0 symbol means no code, a 1..15 symbol means
 *   that length, and the symbols 16, 17, and 18 are run-length instructions.
 *   Each of 16, 17, and 18 are followed by extra bits to define the length of
 *   the run.  16 copies the last length 3 to 6 times.  17 represents 3 to 10
 *   zero lengths, and 18 represents 11 to 138 zero lengths.  Unused symbols
 *   are common, hence the special coding for zero lengths.
 *
 * - The symbols for 0..18 are Huffman coded, and so that code must be
 *   described first.  This is simply a sequence of up to 19 three-bit values
 *   representing no code (0) or the code length for that symbol (1..7).
 *
 * - A dynamic block starts with three fixed-size counts from which is computed
 *   the number of literal/length code lengths, the number of distance code
 *   lengths, and the number of code length code lengths (ok, you come up with
 *   a better name!) in the code descriptions.  For the literal/length and
 *   distance codes, lengths after those provided are considered zero, i.e. no
 *   code.  The code length code lengths are received in a permuted order (see
 *   the order[] array below) to make a short code length code length list more
 *   likely.  As it turns out, very short and very long codes are less likely
 *   to be seen in a dynamic code description, hence what may appear initially
 *   to be a peculiar ordering.
 *
 * - Given the number of literal/length code lengths (nlen) and distance code
 *   lengths (ndist), then they are treated as one long list of nlen + ndist
 *   code lengths.  Therefore run-length coding can and often does cross the
 *   boundary between the two sets of lengths.
 *
 * - So to summarize, the code description at the start of a dynamic block is
 *   three counts for the number of code lengths for the literal/length codes,
 *   the distance codes, and the code length codes.  This is followed by the
 *   code length code lengths, three bits each.  This is used to construct the
 *   code length code which is used to read the remainder of the lengths.  Then
 *   the literal/length code lengths and distance lengths are read as a single
 *   set of lengths using the code length codes.  Codes are constructed from
 *   the resulting two sets of lengths, and then finally you can start
 *   decoding actual compressed data in the block.
 *
 * - For reference, a "typical" size for the code description in a dynamic
 *   block is around 80 bytes.
 */
local صحيح dynamic(بنية state *s)
{
    صحيح nlen, ndist, ncode;             /* number of lengths in descriptor */
    صحيح شاخص;                          /* index of lengths[] */
    صحيح err;                            /* construct() return value */
    کوتاه lengths[MAXCODES];            /* descriptor code lengths */
    کوتاه lencnt[MAXBITS+1], lensym[MAXLCODES];         /* lencode memory */
    کوتاه distcnt[MAXBITS+1], distsym[MAXDCODES];       /* distcode memory */
    بنية huffman lencode, distcode;   /* length and distance codes */
    ساكن ثابت کوتاه ترتیب[19] =      /* permutation of code length codes */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* construct lencode and distcode */
    lencode.شمار = lencnt;
    lencode.نماد = lensym;
    distcode.شمار = distcnt;
    distcode.نماد = distsym;

    /* get number of lengths in each table, check lengths */
    nlen = بیتها(s, 5) + 257;
    ndist = بیتها(s, 5) + 1;
    ncode = بیتها(s, 4) + 4;
    إذا (nlen > MAXLCODES || ndist > MAXDCODES)
        أرجع -3;                      /* bad counts */

    /* read code length code lengths (really), missing lengths are zero */
    لكل (شاخص = 0; شاخص < ncode; شاخص++)
        lengths[ترتیب[شاخص]] = بیتها(s, 3);
    لكل (; شاخص < 19; شاخص++)
        lengths[ترتیب[شاخص]] = 0;

    /* build huffman table for code lengths codes (use lencode temporarily) */
    err = construct(&lencode, lengths, 19);
    إذا (err != 0)               /* require complete code set here */
        أرجع -4;

    /* read length/literal and distance code length tables */
    شاخص = 0;
    طالما (شاخص < nlen + ndist) {
        صحيح نماد;             /* decoded value */
        صحيح len;                /* last length to repeat */

        نماد = decode(s, &lencode);
        إذا (نماد < 0)
            أرجع نماد;          /* invalid symbol */
        إذا (نماد < 16)                /* length in 0..15 */
            lengths[شاخص++] = نماد;
        وإلا {                          /* repeat instruction */
            len = 0;                    /* assume repeating zeros */
            إذا (نماد == 16) {         /* repeat last length 3..6 times */
                إذا (شاخص == 0)
                    أرجع -5;          /* no last length! */
                len = lengths[شاخص - 1];       /* last length */
                نماد = 3 + بیتها(s, 2);
            }
            وإلا إذا (نماد == 17)      /* repeat zero 3..10 times */
                نماد = 3 + بیتها(s, 3);
            وإلا                        /* == 18, repeat zero 11..138 times */
                نماد = 11 + بیتها(s, 7);
            إذا (شاخص + نماد > nlen + ndist)
                أرجع -6;              /* too many lengths! */
            طالما (نماد--)            /* repeat last or zero symbol times */
                lengths[شاخص++] = len;
        }
    }

    /* check for end-of-block code -- there better be one! */
    إذا (lengths[256] == 0)
        أرجع -9;

    /* build huffman table for literal/length codes */
    err = construct(&lencode, lengths, nlen);
    إذا (err && (err < 0 || nlen != lencode.شمار[0] + lencode.شمار[1]))
        أرجع -7;      /* incomplete code ok only for single length 1 code */

    /* build huffman table for distance codes */
    err = construct(&distcode, lengths + nlen, ndist);
    إذا (err && (err < 0 || ndist != distcode.شمار[0] + distcode.شمار[1]))
        أرجع -8;      /* incomplete code ok only for single length 1 code */

    /* decode data until end-of-block code */
    أرجع codes(s, &lencode, &distcode);
}

/*
 * Inflate source to dest.  On return, destlen and sourcelen are updated to the
 * size of the uncompressed data and the size of the deflate data respectively.
 * On success, the return value of puff() is zero.  If there is an error in the
 * source data, i.e. it is not in the deflate format, then a negative value is
 * returned.  If there is not enough input available or there is not enough
 * output space, then a positive error is returned.  In that case, destlen and
 * sourcelen are not updated to facilitate retrying from the beginning with the
 * provision of more input data or more output space.  In the case of invalid
 * inflate data (a negative error), the dest and source pointers are updated to
 * facilitate the debugging of deflators.
 *
 * puff() also has a mode to determine the size of the uncompressed output with
 * no output written.  For this dest must be (unsigned char *)0.  In this case,
 * the input value of *destlen is ignored, and on return *destlen is set to the
 * size of the uncompressed output.
 *
 * The return codes are:
 *
 *   2:  available inflate data did not terminate
 *   1:  output space exhausted before completing inflate
 *   0:  successful inflate
 *  -1:  invalid block type (type == 3)
 *  -2:  stored block length did not match one's complement
 *  -3:  dynamic block code description: too many length or distance codes
 *  -4:  dynamic block code description: code lengths codes incomplete
 *  -5:  dynamic block code description: repeat lengths with no first length
 *  -6:  dynamic block code description: repeat more than specified lengths
 *  -7:  dynamic block code description: invalid literal/length code lengths
 *  -8:  dynamic block code description: invalid distance code lengths
 *  -9:  dynamic block code description: missing end-of-block code
 * -10:  invalid literal/length or distance code in fixed or dynamic block
 * -11:  distance is too far back in fixed or dynamic block
 *
 * Format notes:
 *
 * - Three bits are read for each block to determine the kind of block and
 *   whether or not it is the last block.  Then the block is decoded and the
 *   process repeated if it was not the last block.
 *
 * - The leftover bits in the last byte of the deflate data after the last
 *   block (if it was a fixed or dynamic block) are undefined and have no
 *   expected values to check.
 */
صحيح puff(بیعلامت حرف *dest,           /* pointer to destination pointer */
         بیعلامت دراز *destlen,        /* amount of output space */
         ثابت بیعلامت حرف *منبع,   /* pointer to source data pointer */
         بیعلامت دراز *sourcelen)      /* amount of input available */
{
    بنية state s;             /* input/output state */
    صحيح آخرین, type;             /* block information */
    صحيح err;                    /* return value */

    /* initialize output state */
    s.بیرون = dest;
    s.outlen = *destlen;                /* ignored if dest is NIL */
    s.outcnt = 0;

    /* initialize input state */
    s.درون = منبع;
    s.inlen = *sourcelen;
    s.incnt = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* return if bits() or decode() tries to read past available input */
    إذا (setjmp(s.env) != 0)             /* if came back here via longjmp() */
        err = 2;                        /* then skip do-loop, return error */
    وإلا {
        /* process blocks until last block or error */
        افعل {
            آخرین = بیتها(&s, 1);         /* one if last block */
            type = بیتها(&s, 2);         /* block type 0..3 */
            err = type == 0 ?
                    stored(&s) :
                    (type == 1 ?
                        fixed(&s) :
                        (type == 2 ?
                            dynamic(&s) :
                            -1));       /* type == 3, invalid */
            إذا (err != 0)
                اكسر;                  /* return with error */
        } طالما (!آخرین);
    }

    /* update the lengths and return */
    إذا (err <= 0) {
        *destlen = s.outcnt;
        *sourcelen = s.incnt;
    }
    أرجع err;
}
