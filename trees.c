/* trees.c -- output deflated data using Huffman coding
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * detect_data_type() function provided freely by Cosmin Truta, 2006
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 *  ALGORITHM
 *
 *      The "deflation" process uses several Huffman trees. The more
 *      common source values are represented by shorter bit sequences.
 *
 *      Each code tree is stored in a compressed form which is itself
 * a Huffman encoding of the lengths of all the code strings (in
 * ascending order by source values).  The actual code strings are
 * reconstructed from the lengths in the inflate process, as described
 * in the deflate specification.
 *
 *  REFERENCES
 *
 *      Deutsch, L.P.,"'Deflate' Compressed Data Format Specification".
 *      Available in ftp.uu.net:/pub/archiving/zip/doc/deflate-1.1.doc
 *
 *      Storer, James A.
 *          Data Compression:  Methods and Theory, pp. 49-50.
 *          Computer Science Press, 1988.  ISBN 0-7167-8156-5.
 *
 *      Sedgewick, R.
 *          Algorithms, p290.
 *          Addison-Wesley, 1983. ISBN 0-201-06672-6.
 */

/* @(#) $Id$ */

/* #define GEN_TREES_H */

#ਅਨ੍ਤਰ੍ਭਾਵ "deflate.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
#  include <ctype.h>
#ਜੇ_ਅੰਤ

/* ===========================================================================
 * Constants
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_BL_ਬਿੱਟ 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿਰਾ_ਖੰਡ 256
/* end of block literal code */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ਅਚਲ ਪੂਰਨ extra_lbits[ਲੰਮਾਈ_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local ਅਚਲ ਪੂਰਨ extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local ਅਚਲ ਪੂਰਨ extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local ਅਚਲ uch bl_ਕ੍ਰਮ[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIST_ਕੋਡ_LEN  512 /* see definition of array dist_code below */

#ਜੇ defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_ਅੰਕੜੇ static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_ਅੰਕੜੇ static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch dist_ਕੋਡ[DIST_ਕੋਡ_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch ਲੰਮਾਈ_ਕੋਡ[MAX_ਮੇਲ-MIN_ਮੇਲ+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local ਪੂਰਨ ਅਧਾਰ_ਲੰਮਾਈ[ਲੰਮਾਈ_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local ਪੂਰਨ ਅਧਾਰ_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#ਨਹੀਂਤਾਂ
#  include "trees.h"
#ਜੇ_ਅੰਤ /* defined(GEN_TREES_H) || !defined(STDC) */

ਬਣਤਰ static_ਰੁੱਖ_desc_s {
    ਅਚਲ ct_ਅੰਕੜੇ *static_ਰੁੱਖ;  /* static tree or NULL */
    ਅਚਲ intf *extra_ਬਿੱਟ;      /* extra bits for each code or NULL */
    ਪੂਰਨ     extra_ਅਧਾਰ;          /* base index for extra_bits */
    ਪੂਰਨ     elems;               /* max number of elements in the tree */
    ਪੂਰਨ     max_ਲੰਮਾਈ;          /* max bit length for the codes */
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_INIT_GLOBAL_ਸੂਚਕ_ਸਾਰੇ
#  define TCONST
#ਨਹੀਂਤਾਂ
#  define TCONST ਅਚਲ
#ਜੇ_ਅੰਤ

local TCONST static_ਰੁੱਖ_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_ਬਿੱਟ};

local TCONST static_ਰੁੱਖ_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_ਬਿੱਟ};

local TCONST static_ਰੁੱਖ_desc static_bl_desc =
{(ਅਚਲ ct_ਅੰਕੜੇ *)0, extra_blbits, 0,   BL_CODES, MAX_BL_ਬਿੱਟ};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local ਬਿਨਾਨਿਸ਼ਾਨੀ bi_ਉਲਟ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੋਡ, ਪੂਰਨ len) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ res = 0;
    ਕਰੋ {
        res |= ਕੋਡ & 1;
        ਕੋਡ >>= 1, res <<= 1;
    } ਜਦੋਂ (--len > 0);
    ਵਾਪਸ res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local ਖਾਲੀ bi_flush(deflate_state *s) {
    ਜੇ (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } ਨਹੀਂਤਾਂ ਜੇ (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local ਖਾਲੀ bi_windup(deflate_state *s) {
    ਜੇ (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } ਨਹੀਂਤਾਂ ਜੇ (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_ਵਰਤਿਆ = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
    s->ਬਿੱਟ_ਭੇਜਿਆ = (s->ਬਿੱਟ_ਭੇਜਿਆ + 7) & ~(ulg)7;
#ਜੇ_ਅੰਤ
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local ਖਾਲੀ gen_codes(ct_ਅੰਕੜੇ *ਰੁੱਖ, ਪੂਰਨ max_ਕੋਡ, ushf *bl_ਗਿਣਤੀ) {
    ush ਅਗਲਾ_ਕੋਡ[MAX_ਬਿੱਟ+1]; /* next code value for each bit length */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੋਡ = 0;         /* running code value */
    ਪੂਰਨ ਬਿੱਟ;                  /* bit index */
    ਪੂਰਨ n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    ਲਈ (ਬਿੱਟ = 1; ਬਿੱਟ <= MAX_ਬਿੱਟ; ਬਿੱਟ++) {
        ਕੋਡ = (ਕੋਡ + bl_ਗਿਣਤੀ[ਬਿੱਟ - 1]) << 1;
        ਅਗਲਾ_ਕੋਡ[ਬਿੱਟ] = (ush)ਕੋਡ;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (ਕੋਡ + bl_ਗਿਣਤੀ[MAX_ਬਿੱਟ] - 1 == (1 << MAX_ਬਿੱਟ) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_ਕੋਡ));

    ਲਈ (n = 0;  n <= max_ਕੋਡ; n++) {
        ਪੂਰਨ len = ਰੁੱਖ[n].Len;
        ਜੇ (len == 0) ਜਾਰੀ;
        /* Now reverse the bits */
        ਰੁੱਖ[n].ਕੋਡ_2 = (ush)bi_ਉਲਟ(ਅਗਲਾ_ਕੋਡ[len]++, len);

        Tracecv(ਰੁੱਖ != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, ਰੁੱਖ[n].ਕੋਡ_2, ਅਗਲਾ_ਕੋਡ[len] - 1));
    }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GEN_TREES_H
local ਖਾਲੀ gen_trees_ਸਿਰਲੇਖ(ਖਾਲੀ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
#  define send_ਕੋਡ(s, c, ਰੁੱਖ) send_ਬਿੱਟ(s, ਰੁੱਖ[c].ਕੋਡ_2, ਰੁੱਖ[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#ਨਹੀਂਤਾਂ /* !ZLIB_DEBUG */
#  define send_ਕੋਡ(s, c, ਰੁੱਖ) \
     { ਜੇ (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_ਬਿੱਟ(s, ਰੁੱਖ[c].ਕੋਡ_2, ਰੁੱਖ[c].Len); }
#ਜੇ_ਅੰਤ

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
local ਖਾਲੀ send_ਬਿੱਟ(deflate_state *s, ਪੂਰਨ ਮੁੱਲ, ਪੂਰਨ ਲੰਮਾਈ) {
    Tracevv((stderr," l %2d v %4x ", ਲੰਮਾਈ, ਮੁੱਲ));
    Assert(ਲੰਮਾਈ > 0 && ਲੰਮਾਈ <= 15, "invalid length");
    s->ਬਿੱਟ_ਭੇਜਿਆ += (ulg)ਲੰਮਾਈ;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    ਜੇ (s->bi_valid > (ਪੂਰਨ)Buf_ਮਾਪ - ਲੰਮਾਈ) {
        s->bi_buf |= (ush)ਮੁੱਲ << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)ਮੁੱਲ >> (Buf_ਮਾਪ - s->bi_valid);
        s->bi_valid += ਲੰਮਾਈ - Buf_ਮਾਪ;
    } ਨਹੀਂਤਾਂ {
        s->bi_buf |= (ush)ਮੁੱਲ << s->bi_valid;
        s->bi_valid += ਲੰਮਾਈ;
    }
}
#ਨਹੀਂਤਾਂ /* !ZLIB_DEBUG */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ send_ਬਿੱਟ(s, ਮੁੱਲ, ਲੰਮਾਈ) \
{ ਪੂਰਨ len = ਲੰਮਾਈ;\
  ਜੇ (s->bi_valid > (ਪੂਰਨ)Buf_ਮਾਪ - len) {\
    ਪੂਰਨ val = (ਪੂਰਨ)ਮੁੱਲ;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_ਮਾਪ - s->bi_valid);\
    s->bi_valid += len - Buf_ਮਾਪ;\
  } ਨਹੀਂਤਾਂ {\
    s->bi_buf |= (ush)(ਮੁੱਲ) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#ਜੇ_ਅੰਤ /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local ਖਾਲੀ tr_static_init(ਖਾਲੀ) {
#ਜੇ defined(GEN_TREES_H) || !defined(STDC)
    ਸਥਿਰ ਪੂਰਨ static_init_ਪੂਰਾ = 0;
    ਪੂਰਨ n;        /* iterates over tree elements */
    ਪੂਰਨ ਬਿੱਟ;     /* bit counter */
    ਪੂਰਨ ਲੰਮਾਈ;   /* length value */
    ਪੂਰਨ ਕੋਡ;     /* code value */
    ਪੂਰਨ dist;     /* distance index */
    ush bl_ਗਿਣਤੀ[MAX_ਬਿੱਟ+1];
    /* number of codes at each bit length for an optimal tree */

    ਜੇ (static_init_ਪੂਰਾ) ਵਾਪਸ;

    /* For some embedded targets, global variables are not initialized: */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_INIT_GLOBAL_ਸੂਚਕ_ਸਾਰੇ
    static_l_desc.static_ਰੁੱਖ = static_ltree;
    static_l_desc.extra_ਬਿੱਟ = extra_lbits;
    static_d_desc.static_ਰੁੱਖ = static_dtree;
    static_d_desc.extra_ਬਿੱਟ = extra_dbits;
    static_bl_desc.extra_ਬਿੱਟ = extra_blbits;
#ਜੇ_ਅੰਤ

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    ਲੰਮਾਈ = 0;
    ਲਈ (ਕੋਡ = 0; ਕੋਡ < ਲੰਮਾਈ_CODES-1; ਕੋਡ++) {
        ਅਧਾਰ_ਲੰਮਾਈ[ਕੋਡ] = ਲੰਮਾਈ;
        ਲਈ (n = 0; n < (1 << extra_lbits[ਕੋਡ]); n++) {
            ਲੰਮਾਈ_ਕੋਡ[ਲੰਮਾਈ++] = (uch)ਕੋਡ;
        }
    }
    Assert (ਲੰਮਾਈ == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    ਲੰਮਾਈ_ਕੋਡ[ਲੰਮਾਈ - 1] = (uch)ਕੋਡ;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    ਲਈ (ਕੋਡ = 0 ; ਕੋਡ < 16; ਕੋਡ++) {
        ਅਧਾਰ_dist[ਕੋਡ] = dist;
        ਲਈ (n = 0; n < (1 << extra_dbits[ਕੋਡ]); n++) {
            dist_ਕੋਡ[dist++] = (uch)ਕੋਡ;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    ਲਈ ( ; ਕੋਡ < D_CODES; ਕੋਡ++) {
        ਅਧਾਰ_dist[ਕੋਡ] = dist << 7;
        ਲਈ (n = 0; n < (1 << (extra_dbits[ਕੋਡ] - 7)); n++) {
            dist_ਕੋਡ[256 + dist++] = (uch)ਕੋਡ;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    ਲਈ (ਬਿੱਟ = 0; ਬਿੱਟ <= MAX_ਬਿੱਟ; ਬਿੱਟ++) bl_ਗਿਣਤੀ[ਬਿੱਟ] = 0;
    n = 0;
    ਜਦੋਂ (n <= 143) static_ltree[n++].Len = 8, bl_ਗਿਣਤੀ[8]++;
    ਜਦੋਂ (n <= 255) static_ltree[n++].Len = 9, bl_ਗਿਣਤੀ[9]++;
    ਜਦੋਂ (n <= 279) static_ltree[n++].Len = 7, bl_ਗਿਣਤੀ[7]++;
    ਜਦੋਂ (n <= 287) static_ltree[n++].Len = 8, bl_ਗਿਣਤੀ[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_ਅੰਕੜੇ *)static_ltree, L_CODES+1, bl_ਗਿਣਤੀ);

    /* The static distance tree is trivial: */
    ਲਈ (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].ਕੋਡ_2 = bi_ਉਲਟ((ਬਿਨਾਨਿਸ਼ਾਨੀ)n, 5);
    }
    static_init_ਪੂਰਾ = 1;

#  ifdef GEN_TREES_H
    gen_trees_ਸਿਰਲੇਖ();
#  endif
#ਜੇ_ਅੰਤ /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, ਆਖਰੀ, ਚੌੜਾਈ) \
      ((i) == (ਆਖਰੀ)? "\n};\n\n" :    \
       ((i) % (ਚੌੜਾਈ) == (ਚੌੜਾਈ) - 1 ? ",\n" : ", "))

ਖਾਲੀ gen_trees_ਸਿਰਲੇਖ(ਖਾਲੀ) {
    FILE *ਸਿਰਲੇਖ = fopen("trees.h", "w");
    ਪੂਰਨ i;

    Assert (ਸਿਰਲੇਖ != ਨਿਰਾ, "Can't open trees.h");
    fprintf(ਸਿਰਲੇਖ,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(ਸਿਰਲੇਖ, "local const ct_data static_ltree[L_CODES+2] = {\n");
    ਲਈ (i = 0; i < L_CODES+2; i++) {
        fprintf(ਸਿਰਲੇਖ, "{{%3u},{%3u}}%s", static_ltree[i].ਕੋਡ_2,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(ਸਿਰਲੇਖ, "local const ct_data static_dtree[D_CODES] = {\n");
    ਲਈ (i = 0; i < D_CODES; i++) {
        fprintf(ਸਿਰਲੇਖ, "{{%2u},{%2u}}%s", static_dtree[i].ਕੋਡ_2,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(ਸਿਰਲੇਖ, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    ਲਈ (i = 0; i < DIST_ਕੋਡ_LEN; i++) {
        fprintf(ਸਿਰਲੇਖ, "%2u%s", dist_ਕੋਡ[i],
                SEPARATOR(i, DIST_ਕੋਡ_LEN-1, 20));
    }

    fprintf(ਸਿਰਲੇਖ,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    ਲਈ (i = 0; i < MAX_ਮੇਲ-MIN_ਮੇਲ+1; i++) {
        fprintf(ਸਿਰਲੇਖ, "%2u%s", ਲੰਮਾਈ_ਕੋਡ[i],
                SEPARATOR(i, MAX_ਮੇਲ-MIN_ਮੇਲ, 20));
    }

    fprintf(ਸਿਰਲੇਖ, "local const int base_length[LENGTH_CODES] = {\n");
    ਲਈ (i = 0; i < ਲੰਮਾਈ_CODES; i++) {
        fprintf(ਸਿਰਲੇਖ, "%1u%s", ਅਧਾਰ_ਲੰਮਾਈ[i],
                SEPARATOR(i, ਲੰਮਾਈ_CODES-1, 20));
    }

    fprintf(ਸਿਰਲੇਖ, "local const int base_dist[D_CODES] = {\n");
    ਲਈ (i = 0; i < D_CODES; i++) {
        fprintf(ਸਿਰਲੇਖ, "%5u%s", ਅਧਾਰ_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(ਸਿਰਲੇਖ);
}
#ਜੇ_ਅੰਤ /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local ਖਾਲੀ init_ਖੰਡ(deflate_state *s) {
    ਪੂਰਨ n; /* iterates over tree elements */

    /* Initialize the trees. */
    ਲਈ (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    ਲਈ (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    ਲਈ (n = 0; n < BL_CODES; n++) s->bl_ਰੁੱਖ[n].Freq = 0;

    s->dyn_ltree[ਸਿਰਾ_ਖੰਡ].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_ਅਗਲਾ = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_ਰੁੱਖ = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_ਰੁੱਖ = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_ਰੁੱਖ = s->bl_ਰੁੱਖ;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_ਵਰਤਿਆ = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
    s->compressed_len = 0L;
    s->ਬਿੱਟ_ਭੇਜਿਆ = 0L;
#ਜੇ_ਅੰਤ

    /* Initialize the first block of the first file: */
    init_ਖੰਡ(s);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਭ_ਤੋਂ_ਛੋਟਾ 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pqremove(s, ਰੁੱਖ, ਸਿਖਰ) \
{\
    ਸਿਖਰ = s->ਢੇਰ[ਸਭ_ਤੋਂ_ਛੋਟਾ]; \
    s->ਢੇਰ[ਸਭ_ਤੋਂ_ਛੋਟਾ] = s->ਢੇਰ[s->ਢੇਰ_len--]; \
    pqdownheap(s, ਰੁੱਖ, ਸਭ_ਤੋਂ_ਛੋਟਾ); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ smaller(ਰੁੱਖ, n, m, ਡੂੰਘਾਈ) \
   (ਰੁੱਖ[n].Freq < ਰੁੱਖ[m].Freq || \
   (ਰੁੱਖ[n].Freq == ਰੁੱਖ[m].Freq && ਡੂੰਘਾਈ[n] <= ਡੂੰਘਾਈ[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local ਖਾਲੀ pqdownheap(deflate_state *s, ct_ਅੰਕੜੇ *ਰੁੱਖ, ਪੂਰਨ k) {
    ਪੂਰਨ v = s->ਢੇਰ[k];
    ਪੂਰਨ j = k << 1;  /* left son of k */
    ਜਦੋਂ (j <= s->ਢੇਰ_len) {
        /* Set j to the smallest of the two sons: */
        ਜੇ (j < s->ਢੇਰ_len &&
            smaller(ਰੁੱਖ, s->ਢੇਰ[j + 1], s->ਢੇਰ[j], s->ਡੂੰਘਾਈ)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        ਜੇ (smaller(ਰੁੱਖ, v, s->ਢੇਰ[j], s->ਡੂੰਘਾਈ)) ਰੋਕੋ;

        /* Exchange v with the smallest son */
        s->ਢੇਰ[k] = s->ਢੇਰ[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->ਢੇਰ[k] = v;
}

/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
local ਖਾਲੀ gen_bitlen(deflate_state *s, ਰੁੱਖ_desc *desc) {
    ct_ਅੰਕੜੇ *ਰੁੱਖ        = desc->dyn_ਰੁੱਖ;
    ਪੂਰਨ max_ਕੋਡ         = desc->max_ਕੋਡ;
    ਅਚਲ ct_ਅੰਕੜੇ *stree = desc->stat_desc->static_ਰੁੱਖ;
    ਅਚਲ intf *extra    = desc->stat_desc->extra_ਬਿੱਟ;
    ਪੂਰਨ ਅਧਾਰ_2             = desc->stat_desc->extra_ਅਧਾਰ;
    ਪੂਰਨ max_ਲੰਮਾਈ       = desc->stat_desc->max_ਲੰਮਾਈ;
    ਪੂਰਨ h;              /* heap index */
    ਪੂਰਨ n, m;           /* iterate over the tree elements */
    ਪੂਰਨ ਬਿੱਟ;           /* bit length */
    ਪੂਰਨ xbits;          /* extra bits */
    ush f;              /* frequency */
    ਪੂਰਨ ਉਛਾਲ = 0;   /* number of elements with bit length too large */

    ਲਈ (ਬਿੱਟ = 0; ਬਿੱਟ <= MAX_ਬਿੱਟ; ਬਿੱਟ++) s->bl_ਗਿਣਤੀ[ਬਿੱਟ] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    ਰੁੱਖ[s->ਢੇਰ[s->ਢੇਰ_max]].Len = 0; /* root of the heap */

    ਲਈ (h = s->ਢੇਰ_max + 1; h < ਢੇਰ_ਮਾਪ; h++) {
        n = s->ਢੇਰ[h];
        ਬਿੱਟ = ਰੁੱਖ[ਰੁੱਖ[n].Dad].Len + 1;
        ਜੇ (ਬਿੱਟ > max_ਲੰਮਾਈ) ਬਿੱਟ = max_ਲੰਮਾਈ, ਉਛਾਲ++;
        ਰੁੱਖ[n].Len = (ush)ਬਿੱਟ;
        /* We overwrite tree[n].Dad which is no longer needed */

        ਜੇ (n > max_ਕੋਡ) ਜਾਰੀ; /* not a leaf node */

        s->bl_ਗਿਣਤੀ[ਬਿੱਟ]++;
        xbits = 0;
        ਜੇ (n >= ਅਧਾਰ_2) xbits = extra[n - ਅਧਾਰ_2];
        f = ਰੁੱਖ[n].Freq;
        s->opt_len += (ulg)f * (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਬਿੱਟ + xbits);
        ਜੇ (stree) s->static_len += (ulg)f * (ਬਿਨਾਨਿਸ਼ਾਨੀ)(stree[n].Len + xbits);
    }
    ਜੇ (ਉਛਾਲ == 0) ਵਾਪਸ;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    ਕਰੋ {
        ਬਿੱਟ = max_ਲੰਮਾਈ - 1;
        ਜਦੋਂ (s->bl_ਗਿਣਤੀ[ਬਿੱਟ] == 0) ਬਿੱਟ--;
        s->bl_ਗਿਣਤੀ[ਬਿੱਟ]--;        /* move one leaf down the tree */
        s->bl_ਗਿਣਤੀ[ਬਿੱਟ + 1] += 2; /* move one overflow item as its brother */
        s->bl_ਗਿਣਤੀ[max_ਲੰਮਾਈ]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        ਉਛਾਲ -= 2;
    } ਜਦੋਂ (ਉਛਾਲ > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    ਲਈ (ਬਿੱਟ = max_ਲੰਮਾਈ; ਬਿੱਟ != 0; ਬਿੱਟ--) {
        n = s->bl_ਗਿਣਤੀ[ਬਿੱਟ];
        ਜਦੋਂ (n != 0) {
            m = s->ਢੇਰ[--h];
            ਜੇ (m > max_ਕੋਡ) ਜਾਰੀ;
            ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ) ਰੁੱਖ[m].Len != (ਬਿਨਾਨਿਸ਼ਾਨੀ) ਬਿੱਟ) {
                Tracev((stderr,"code %d bits %d->%d\n", m, ਰੁੱਖ[m].Len, ਬਿੱਟ));
                s->opt_len += ((ulg)ਬਿੱਟ - ਰੁੱਖ[m].Len) * ਰੁੱਖ[m].Freq;
                ਰੁੱਖ[m].Len = (ush)ਬਿੱਟ;
            }
            n--;
        }
    }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DUMP_BL_ਰੁੱਖ
#  include <stdio.h>
#ਜੇ_ਅੰਤ

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local ਖਾਲੀ build_ਰੁੱਖ(deflate_state *s, ਰੁੱਖ_desc *desc) {
    ct_ਅੰਕੜੇ *ਰੁੱਖ         = desc->dyn_ਰੁੱਖ;
    ਅਚਲ ct_ਅੰਕੜੇ *stree  = desc->stat_desc->static_ਰੁੱਖ;
    ਪੂਰਨ elems             = desc->stat_desc->elems;
    ਪੂਰਨ n, m;          /* iterate over heap elements */
    ਪੂਰਨ max_ਕੋਡ = -1; /* largest code with non zero frequency */
    ਪੂਰਨ ਗੰਢ;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->ਢੇਰ_len = 0, s->ਢੇਰ_max = ਢੇਰ_ਮਾਪ;

    ਲਈ (n = 0; n < elems; n++) {
        ਜੇ (ਰੁੱਖ[n].Freq != 0) {
            s->ਢੇਰ[++(s->ਢੇਰ_len)] = max_ਕੋਡ = n;
            s->ਡੂੰਘਾਈ[n] = 0;
        } ਨਹੀਂਤਾਂ {
            ਰੁੱਖ[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    ਜਦੋਂ (s->ਢੇਰ_len < 2) {
        ਗੰਢ = s->ਢੇਰ[++(s->ਢੇਰ_len)] = (max_ਕੋਡ < 2 ? ++max_ਕੋਡ : 0);
        ਰੁੱਖ[ਗੰਢ].Freq = 1;
        s->ਡੂੰਘਾਈ[ਗੰਢ] = 0;
        s->opt_len--; ਜੇ (stree) s->static_len -= stree[ਗੰਢ].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_ਕੋਡ = max_ਕੋਡ;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    ਲਈ (n = s->ਢੇਰ_len/2; n >= 1; n--) pqdownheap(s, ਰੁੱਖ, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    ਗੰਢ = elems;              /* next internal node of the tree */
    ਕਰੋ {
        pqremove(s, ਰੁੱਖ, n);  /* n = node of least frequency */
        m = s->ਢੇਰ[ਸਭ_ਤੋਂ_ਛੋਟਾ]; /* m = node of next least frequency */

        s->ਢੇਰ[--(s->ਢੇਰ_max)] = n; /* keep the nodes sorted by frequency */
        s->ਢੇਰ[--(s->ਢੇਰ_max)] = m;

        /* Create a new node father of n and m */
        ਰੁੱਖ[ਗੰਢ].Freq = ਰੁੱਖ[n].Freq + ਰੁੱਖ[m].Freq;
        s->ਡੂੰਘਾਈ[ਗੰਢ] = (uch)((s->ਡੂੰਘਾਈ[n] >= s->ਡੂੰਘਾਈ[m] ?
                                s->ਡੂੰਘਾਈ[n] : s->ਡੂੰਘਾਈ[m]) + 1);
        ਰੁੱਖ[n].Dad = ਰੁੱਖ[m].Dad = (ush)ਗੰਢ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DUMP_BL_ਰੁੱਖ
        ਜੇ (ਰੁੱਖ == s->bl_ਰੁੱਖ) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    ਗੰਢ, ਰੁੱਖ[ਗੰਢ].Freq, n, ਰੁੱਖ[n].Freq, m, ਰੁੱਖ[m].Freq);
        }
#ਜੇ_ਅੰਤ
        /* and insert the new node in the heap */
        s->ਢੇਰ[ਸਭ_ਤੋਂ_ਛੋਟਾ] = ਗੰਢ++;
        pqdownheap(s, ਰੁੱਖ, ਸਭ_ਤੋਂ_ਛੋਟਾ);

    } ਜਦੋਂ (s->ਢੇਰ_len >= 2);

    s->ਢੇਰ[--(s->ਢੇਰ_max)] = s->ਢੇਰ[ਸਭ_ਤੋਂ_ਛੋਟਾ];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (ਰੁੱਖ_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_ਅੰਕੜੇ *)ਰੁੱਖ, max_ਕੋਡ, s->bl_ਗਿਣਤੀ);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local ਖਾਲੀ ਜਾਂਚ_ਰੁੱਖ(deflate_state *s, ct_ਅੰਕੜੇ *ਰੁੱਖ, ਪੂਰਨ max_ਕੋਡ) {
    ਪੂਰਨ n;                     /* iterates over all tree elements */
    ਪੂਰਨ prevlen = -1;          /* last emitted length */
    ਪੂਰਨ curlen;                /* length of current code */
    ਪੂਰਨ nextlen = ਰੁੱਖ[0].Len; /* length of next code */
    ਪੂਰਨ ਗਿਣਤੀ_2 = 0;             /* repeat count of the current code */
    ਪੂਰਨ max_ਗਿਣਤੀ = 7;         /* max repeat count */
    ਪੂਰਨ min_ਗਿਣਤੀ = 4;         /* min repeat count */

    ਜੇ (nextlen == 0) max_ਗਿਣਤੀ = 138, min_ਗਿਣਤੀ = 3;
    ਰੁੱਖ[max_ਕੋਡ + 1].Len = (ush)0xffff; /* guard */

    ਲਈ (n = 0; n <= max_ਕੋਡ; n++) {
        curlen = nextlen; nextlen = ਰੁੱਖ[n + 1].Len;
        ਜੇ (++ਗਿਣਤੀ_2 < max_ਗਿਣਤੀ && curlen == nextlen) {
            ਜਾਰੀ;
        } ਨਹੀਂਤਾਂ ਜੇ (ਗਿਣਤੀ_2 < min_ਗਿਣਤੀ) {
            s->bl_ਰੁੱਖ[curlen].Freq += (ush)ਗਿਣਤੀ_2;
        } ਨਹੀਂਤਾਂ ਜੇ (curlen != 0) {
            ਜੇ (curlen != prevlen) s->bl_ਰੁੱਖ[curlen].Freq++;
            s->bl_ਰੁੱਖ[REP_3_6].Freq++;
        } ਨਹੀਂਤਾਂ ਜੇ (ਗਿਣਤੀ_2 <= 10) {
            s->bl_ਰੁੱਖ[REPZ_3_10].Freq++;
        } ਨਹੀਂਤਾਂ {
            s->bl_ਰੁੱਖ[REPZ_11_138].Freq++;
        }
        ਗਿਣਤੀ_2 = 0; prevlen = curlen;
        ਜੇ (nextlen == 0) {
            max_ਗਿਣਤੀ = 138, min_ਗਿਣਤੀ = 3;
        } ਨਹੀਂਤਾਂ ਜੇ (curlen == nextlen) {
            max_ਗਿਣਤੀ = 6, min_ਗਿਣਤੀ = 3;
        } ਨਹੀਂਤਾਂ {
            max_ਗਿਣਤੀ = 7, min_ਗਿਣਤੀ = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local ਖਾਲੀ send_ਰੁੱਖ(deflate_state *s, ct_ਅੰਕੜੇ *ਰੁੱਖ, ਪੂਰਨ max_ਕੋਡ) {
    ਪੂਰਨ n;                     /* iterates over all tree elements */
    ਪੂਰਨ prevlen = -1;          /* last emitted length */
    ਪੂਰਨ curlen;                /* length of current code */
    ਪੂਰਨ nextlen = ਰੁੱਖ[0].Len; /* length of next code */
    ਪੂਰਨ ਗਿਣਤੀ_2 = 0;             /* repeat count of the current code */
    ਪੂਰਨ max_ਗਿਣਤੀ = 7;         /* max repeat count */
    ਪੂਰਨ min_ਗਿਣਤੀ = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    ਜੇ (nextlen == 0) max_ਗਿਣਤੀ = 138, min_ਗਿਣਤੀ = 3;

    ਲਈ (n = 0; n <= max_ਕੋਡ; n++) {
        curlen = nextlen; nextlen = ਰੁੱਖ[n + 1].Len;
        ਜੇ (++ਗਿਣਤੀ_2 < max_ਗਿਣਤੀ && curlen == nextlen) {
            ਜਾਰੀ;
        } ਨਹੀਂਤਾਂ ਜੇ (ਗਿਣਤੀ_2 < min_ਗਿਣਤੀ) {
            ਕਰੋ { send_ਕੋਡ(s, curlen, s->bl_ਰੁੱਖ); } ਜਦੋਂ (--ਗਿਣਤੀ_2 != 0);

        } ਨਹੀਂਤਾਂ ਜੇ (curlen != 0) {
            ਜੇ (curlen != prevlen) {
                send_ਕੋਡ(s, curlen, s->bl_ਰੁੱਖ); ਗਿਣਤੀ_2--;
            }
            Assert(ਗਿਣਤੀ_2 >= 3 && ਗਿਣਤੀ_2 <= 6, " 3_6?");
            send_ਕੋਡ(s, REP_3_6, s->bl_ਰੁੱਖ); send_ਬਿੱਟ(s, ਗਿਣਤੀ_2 - 3, 2);

        } ਨਹੀਂਤਾਂ ਜੇ (ਗਿਣਤੀ_2 <= 10) {
            send_ਕੋਡ(s, REPZ_3_10, s->bl_ਰੁੱਖ); send_ਬਿੱਟ(s, ਗਿਣਤੀ_2 - 3, 3);

        } ਨਹੀਂਤਾਂ {
            send_ਕੋਡ(s, REPZ_11_138, s->bl_ਰੁੱਖ); send_ਬਿੱਟ(s, ਗਿਣਤੀ_2 - 11, 7);
        }
        ਗਿਣਤੀ_2 = 0; prevlen = curlen;
        ਜੇ (nextlen == 0) {
            max_ਗਿਣਤੀ = 138, min_ਗਿਣਤੀ = 3;
        } ਨਹੀਂਤਾਂ ਜੇ (curlen == nextlen) {
            max_ਗਿਣਤੀ = 6, min_ਗਿਣਤੀ = 3;
        } ਨਹੀਂਤਾਂ {
            max_ਗਿਣਤੀ = 7, min_ਗਿਣਤੀ = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local ਪੂਰਨ build_bl_ਰੁੱਖ(deflate_state *s) {
    ਪੂਰਨ max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    ਜਾਂਚ_ਰੁੱਖ(s, (ct_ਅੰਕੜੇ *)s->dyn_ltree, s->l_desc.max_ਕੋਡ);
    ਜਾਂਚ_ਰੁੱਖ(s, (ct_ਅੰਕੜੇ *)s->dyn_dtree, s->d_desc.max_ਕੋਡ);

    /* Build the bit length tree: */
    build_ਰੁੱਖ(s, (ਰੁੱਖ_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    ਲਈ (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        ਜੇ (s->bl_ਰੁੱਖ[bl_ਕ੍ਰਮ[max_blindex]].Len != 0) ਰੋਕੋ;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((stderr, "\ndyn trees: dyn %lu, stat %lu",
            s->opt_len, s->static_len));

    ਵਾਪਸ max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local ਖਾਲੀ send_all_trees(deflate_state *s, ਪੂਰਨ lcodes, ਪੂਰਨ dcodes,
                          ਪੂਰਨ blcodes) {
    ਪੂਰਨ rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_ਬਿੱਟ(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_ਬਿੱਟ(s, dcodes - 1,   5);
    send_ਬਿੱਟ(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    ਲਈ (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_ਕ੍ਰਮ[rank]));
        send_ਬਿੱਟ(s, s->bl_ਰੁੱਖ[bl_ਕ੍ਰਮ[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->ਬਿੱਟ_ਭੇਜਿਆ));

    send_ਰੁੱਖ(s, (ct_ਅੰਕੜੇ *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->ਬਿੱਟ_ਭੇਜਿਆ));

    send_ਰੁੱਖ(s, (ct_ਅੰਕੜੇ *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->ਬਿੱਟ_ਭੇਜਿਆ));
}

/* ===========================================================================
 * Send a stored block
 */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ tr_stored_ਖੰਡ(deflate_state *s, charf *buf,
                                    ulg stored_len, ਪੂਰਨ ਆਖਰੀ) {
    send_ਬਿੱਟ(s, (STORED_ਖੰਡ<<1) + ਆਖਰੀ, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    ਜੇ (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->ਬਿੱਟ_ਭੇਜਿਆ += 2*16;
    s->ਬਿੱਟ_ਭੇਜਿਆ += stored_len << 3;
#ਜੇ_ਅੰਤ
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ tr_flush_ਬਿੱਟ(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ _tr_align(deflate_state *s) {
    send_ਬਿੱਟ(s, STATIC_TREES<<1, 3);
    send_ਕੋਡ(s, ਸਿਰਾ_ਖੰਡ, static_ltree);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#ਜੇ_ਅੰਤ
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local ਖਾਲੀ compress_ਖੰਡ(deflate_state *s, ਅਚਲ ct_ਅੰਕੜੇ *ltree,
                          ਅਚਲ ct_ਅੰਕੜੇ *dtree) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ dist;      /* distance of matched string */
    ਪੂਰਨ lc;             /* match length or unmatched char (if dist == 0) */
    ਬਿਨਾਨਿਸ਼ਾਨੀ sx = 0;    /* running index in symbol buffers */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੋਡ;      /* the code to send */
    ਪੂਰਨ extra;          /* number of extra bits to send */

    ਜੇ (s->sym_ਅਗਲਾ != 0) ਕਰੋ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#ਨਹੀਂਤਾਂ
        dist = s->sym_buf[sx++] & 0xff;
        dist += (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#ਜੇ_ਅੰਤ
        ਜੇ (dist == 0) {
            send_ਕੋਡ(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } ਨਹੀਂਤਾਂ {
            /* Here, lc is the match length - MIN_MATCH */
            ਕੋਡ = ਲੰਮਾਈ_ਕੋਡ[lc];
            send_ਕੋਡ(s, ਕੋਡ + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[ਕੋਡ];
            ਜੇ (extra != 0) {
                lc -= ਅਧਾਰ_ਲੰਮਾਈ[ਕੋਡ];
                send_ਬਿੱਟ(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            ਕੋਡ = d_ਕੋਡ(dist);
            Assert (ਕੋਡ < D_CODES, "bad d_code");

            send_ਕੋਡ(s, ਕੋਡ, dtree);       /* send the distance code */
            extra = extra_dbits[ਕੋਡ];
            ਜੇ (extra != 0) {
                dist -= (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਅਧਾਰ_dist[ਕੋਡ];
                send_ਬਿੱਟ(s, (ਪੂਰਨ)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#ਨਹੀਂਤਾਂ
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#ਜੇ_ਅੰਤ

    } ਜਦੋਂ (sx < s->sym_ਅਗਲਾ);

    send_ਕੋਡ(s, ਸਿਰਾ_ਖੰਡ, ltree);
}

/* ===========================================================================
 * Check if the data type is TEXT or BINARY, using the following algorithm:
 * - TEXT if the two conditions below are satisfied:
 *    a) There are no non-portable control characters belonging to the
 *       "block list" (0..6, 14..25, 28..31).
 *    b) There is at least one printable character belonging to the
 *       "allow list" (9 {TAB}, 10 {LF}, 13 {CR}, 32..255).
 * - BINARY otherwise.
 * - The following partially-portable control characters form a
 *   "gray list" that is ignored in this detection algorithm:
 *   (7 {BEL}, 8 {BS}, 11 {VT}, 12 {FF}, 26 {SUB}, 27 {ESC}).
 * IN assertion: the fields Freq of dyn_ltree are set.
 */
local ਪੂਰਨ detect_ਅੰਕੜੇ_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਖੰਡ_ਨਕਾਬ = 0xf3ffc07fUL;
    ਪੂਰਨ n;

    /* Check for non-textual ("block-listed") bytes. */
    ਲਈ (n = 0; n <= 31; n++, ਖੰਡ_ਨਕਾਬ >>= 1)
        ਜੇ ((ਖੰਡ_ਨਕਾਬ & 1) && (s->dyn_ltree[n].Freq != 0))
            ਵਾਪਸ Z_ਦੁਆਧਾਰੀ;

    /* Check for textual ("allow-listed") bytes. */
    ਜੇ (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        ਵਾਪਸ Z_ਪਾਠ;
    ਲਈ (n = 32; n < LITERALS; n++)
        ਜੇ (s->dyn_ltree[n].Freq != 0)
            ਵਾਪਸ Z_ਪਾਠ;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    ਵਾਪਸ Z_ਦੁਆਧਾਰੀ;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ tr_flush_ਖੰਡ(deflate_state *s, charf *buf,
                                   ulg stored_len, ਪੂਰਨ ਆਖਰੀ) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    ਪੂਰਨ max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    ਜੇ (s->level > 0) {

        /* Check if the file is binary or text */
        ਜੇ (s->strm->ਅੰਕੜੇ_type == Z_UNKNOWN)
            s->strm->ਅੰਕੜੇ_type = detect_ਅੰਕੜੇ_type(s);

        /* Construct the literal and distance trees */
        build_ਰੁੱਖ(s, (ਰੁੱਖ_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_ਰੁੱਖ(s, (ਰੁੱਖ_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_ਰੁੱਖ(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_ਅਗਲਾ / 3));

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FORCE_STATIC
        ਜੇ (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#ਜੇ_ਅੰਤ
            opt_lenb = static_lenb;

    } ਨਹੀਂਤਾਂ {
        Assert(buf != (ਅੱਖਰ*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FORCE_STORED
    ਜੇ (buf != (ਅੱਖਰ*)0) { /* force stored block */
#ਨਹੀਂਤਾਂ
    ਜੇ (stored_len + 4 <= opt_lenb && buf != (ਅੱਖਰ*)0) {
                       /* 4: two words for the lengths */
#ਜੇ_ਅੰਤ
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_ਖੰਡ(s, buf, stored_len, ਆਖਰੀ);

    } ਨਹੀਂਤਾਂ ਜੇ (static_lenb == opt_lenb) {
        send_ਬਿੱਟ(s, (STATIC_TREES<<1) + ਆਖਰੀ, 3);
        compress_ਖੰਡ(s, (ਅਚਲ ct_ਅੰਕੜੇ *)static_ltree,
                       (ਅਚਲ ct_ਅੰਕੜੇ *)static_dtree);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#ਜੇ_ਅੰਤ
    } ਨਹੀਂਤਾਂ {
        send_ਬਿੱਟ(s, (DYN_TREES<<1) + ਆਖਰੀ, 3);
        send_all_trees(s, s->l_desc.max_ਕੋਡ + 1, s->d_desc.max_ਕੋਡ + 1,
                       max_blindex + 1);
        compress_ਖੰਡ(s, (ਅਚਲ ct_ਅੰਕੜੇ *)s->dyn_ltree,
                       (ਅਚਲ ct_ਅੰਕੜੇ *)s->dyn_dtree);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#ਜੇ_ਅੰਤ
    }
    Assert (s->compressed_len == s->ਬਿੱਟ_ਭੇਜਿਆ, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_ਖੰਡ(s);

    ਜੇ (ਆਖਰੀ) {
        bi_windup(s);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#ਜੇ_ਅੰਤ
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)ਆਖਰੀ));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
ਪੂਰਨ ZLIB_ਅੰਦਰੂਨੀ _tr_tally(deflate_state *s, ਬਿਨਾਨਿਸ਼ਾਨੀ dist, ਬਿਨਾਨਿਸ਼ਾਨੀ lc) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
    s->d_buf[s->sym_ਅਗਲਾ] = (ush)dist;
    s->l_buf[s->sym_ਅਗਲਾ++] = (uch)lc;
#ਨਹੀਂਤਾਂ
    s->sym_buf[s->sym_ਅਗਲਾ++] = (uch)dist;
    s->sym_buf[s->sym_ਅਗਲਾ++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_ਅਗਲਾ++] = (uch)lc;
#ਜੇ_ਅੰਤ
    ਜੇ (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } ਨਹੀਂਤਾਂ {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_ਮੇਲ-MIN_ਮੇਲ) &&
               (ush)d_ਕੋਡ(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[ਲੰਮਾਈ_ਕੋਡ[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_ਕੋਡ(dist)].Freq++;
    }
    ਵਾਪਸ (s->sym_ਅਗਲਾ == s->sym_ਸਿਰਾ);
}
