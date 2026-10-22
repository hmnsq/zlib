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

#ಅನ್ತರ್ಭಾವ "deflate.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
#  include <ctype.h>
#ಆದರೆ_ಅಂತ್ಯ

/* ===========================================================================
 * Constants
 */

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_BL_ಬಿಟ್ಟುಗಳು 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#ವ್ಯಾಖ್ಯಾನಿಸು ಕೊನೆ_ಖಂಡ 256
/* end of block literal code */

#ವ್ಯಾಖ್ಯಾನಿಸು REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#ವ್ಯಾಖ್ಯಾನಿಸು REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#ವ್ಯಾಖ್ಯಾನಿಸು REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ extra_lbits[ಉದ್ದಳತೆ_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local ಸ್ಥಿರಾಂಕ uch bl_ಕ್ರಮ[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#ವ್ಯಾಖ್ಯಾನಿಸು DIST_ಸಂಕೇತ_LEN  512 /* see definition of array dist_code below */

#ಆದರೆ defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_ದತ್ತಾಂಶ static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_ದತ್ತಾಂಶ static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch dist_ಸಂಕೇತ[DIST_ಸಂಕೇತ_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch ಉದ್ದಳತೆ_ಸಂಕೇತ[MAX_ಹೊಂದಿಕೆ-MIN_ಹೊಂದಿಕೆ+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local ಪೂರ್ಣಾಂಕ ತಳ_ಉದ್ದಳತೆ[ಉದ್ದಳತೆ_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local ಪೂರ್ಣಾಂಕ ತಳ_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#ಇಲ್ಲದಿದ್ದರೆ
#  include "trees.h"
#ಆದರೆ_ಅಂತ್ಯ /* defined(GEN_TREES_H) || !defined(STDC) */

ರಚನೆ static_ವೃಕ್ಷ_desc_s {
    ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *static_ವೃಕ್ಷ;  /* static tree or NULL */
    ಸ್ಥಿರಾಂಕ intf *extra_ಬಿಟ್ಟುಗಳು;      /* extra bits for each code or NULL */
    ಪೂರ್ಣಾಂಕ     extra_ತಳ;          /* base index for extra_bits */
    ಪೂರ್ಣಾಂಕ     elems;               /* max number of elements in the tree */
    ಪೂರ್ಣಾಂಕ     max_ಉದ್ದಳತೆ;          /* max bit length for the codes */
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_INIT_GLOBAL_ಸೂಚಕಗಳು
#  define TCONST
#ಇಲ್ಲದಿದ್ದರೆ
#  define TCONST ಸ್ಥಿರಾಂಕ
#ಆದರೆ_ಅಂತ್ಯ

local TCONST static_ವೃಕ್ಷ_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_ಬಿಟ್ಟುಗಳು};

local TCONST static_ವೃಕ್ಷ_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_ಬಿಟ್ಟುಗಳು};

local TCONST static_ವೃಕ್ಷ_desc static_bl_desc =
{(ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *)0, extra_blbits, 0,   BL_CODES, MAX_BL_ಬಿಟ್ಟುಗಳು};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local ಚಿಹ್ನೆರಹಿತ bi_ಹಿಮ್ಮುಖ(ಚಿಹ್ನೆರಹಿತ ಸಂಕೇತ_2, ಪೂರ್ಣಾಂಕ len) {
    ಚಿಹ್ನೆರಹಿತ res = 0;
    ಮಾಡು {
        res |= ಸಂಕೇತ_2 & 1;
        ಸಂಕೇತ_2 >>= 1, res <<= 1;
    } ಆಗ (--len > 0);
    ಹಿಂತಿರುಗಿ res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local ಶೂನ್ಯ bi_flush(deflate_state *s) {
    ಆದರೆ (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local ಶೂನ್ಯ bi_windup(deflate_state *s) {
    ಆದರೆ (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_ಬಳಸಿದ = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
    s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ = (s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ + 7) & ~(ulg)7;
#ಆದರೆ_ಅಂತ್ಯ
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local ಶೂನ್ಯ gen_codes(ct_ದತ್ತಾಂಶ *ವೃಕ್ಷ, ಪೂರ್ಣಾಂಕ max_ಸಂಕೇತ, ushf *bl_ಎಣಿಕೆ) {
    ush ಮುಂದಿನ_ಸಂಕೇತ[MAX_ಬಿಟ್ಟುಗಳು+1]; /* next code value for each bit length */
    ಚಿಹ್ನೆರಹಿತ ಸಂಕೇತ_2 = 0;         /* running code value */
    ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು;                  /* bit index */
    ಪೂರ್ಣಾಂಕ n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    ಗಾಗಿ (ಬಿಟ್ಟುಗಳು = 1; ಬಿಟ್ಟುಗಳು <= MAX_ಬಿಟ್ಟುಗಳು; ಬಿಟ್ಟುಗಳು++) {
        ಸಂಕೇತ_2 = (ಸಂಕೇತ_2 + bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು - 1]) << 1;
        ಮುಂದಿನ_ಸಂಕೇತ[ಬಿಟ್ಟುಗಳು] = (ush)ಸಂಕೇತ_2;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (ಸಂಕೇತ_2 + bl_ಎಣಿಕೆ[MAX_ಬಿಟ್ಟುಗಳು] - 1 == (1 << MAX_ಬಿಟ್ಟುಗಳು) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_ಸಂಕೇತ));

    ಗಾಗಿ (n = 0;  n <= max_ಸಂಕೇತ; n++) {
        ಪೂರ್ಣಾಂಕ len = ವೃಕ್ಷ[n].Len;
        ಆದರೆ (len == 0) ಮುಂದುವರಿ;
        /* Now reverse the bits */
        ವೃಕ್ಷ[n].ಸಂಕೇತ_3 = (ush)bi_ಹಿಮ್ಮುಖ(ಮುಂದಿನ_ಸಂಕೇತ[len]++, len);

        Tracecv(ವೃಕ್ಷ != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, ವೃಕ್ಷ[n].ಸಂಕೇತ_3, ಮುಂದಿನ_ಸಂಕೇತ[len] - 1));
    }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GEN_TREES_H
local ಶೂನ್ಯ gen_trees_ಶೀರ್ಷಿಕೆ(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZLIB_DEBUG
#  define send_ಸಂಕೇತ(s, c, ವೃಕ್ಷ) send_ಬಿಟ್ಟುಗಳು(s, ವೃಕ್ಷ[c].ಸಂಕೇತ_3, ವೃಕ್ಷ[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#ಇಲ್ಲದಿದ್ದರೆ /* !ZLIB_DEBUG */
#  define send_ಸಂಕೇತ(s, c, ವೃಕ್ಷ) \
     { ಆದರೆ (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_ಬಿಟ್ಟುಗಳು(s, ವೃಕ್ಷ[c].ಸಂಕೇತ_3, ವೃಕ್ಷ[c].Len); }
#ಆದರೆ_ಅಂತ್ಯ

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
local ಶೂನ್ಯ send_ಬಿಟ್ಟುಗಳು(deflate_state *s, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ) {
    Tracevv((stderr," l %2d v %4x ", ಉದ್ದಳತೆ, ಮೌಲ್ಯ));
    Assert(ಉದ್ದಳತೆ > 0 && ಉದ್ದಳತೆ <= 15, "invalid length");
    s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ += (ulg)ಉದ್ದಳತೆ;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    ಆದರೆ (s->bi_valid > (ಪೂರ್ಣಾಂಕ)Buf_ಗಾತ್ರ - ಉದ್ದಳತೆ) {
        s->bi_buf |= (ush)ಮೌಲ್ಯ << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)ಮೌಲ್ಯ >> (Buf_ಗಾತ್ರ - s->bi_valid);
        s->bi_valid += ಉದ್ದಳತೆ - Buf_ಗಾತ್ರ;
    } ಇಲ್ಲದಿದ್ದರೆ {
        s->bi_buf |= (ush)ಮೌಲ್ಯ << s->bi_valid;
        s->bi_valid += ಉದ್ದಳತೆ;
    }
}
#ಇಲ್ಲದಿದ್ದರೆ /* !ZLIB_DEBUG */

#ವ್ಯಾಖ್ಯಾನಿಸು send_ಬಿಟ್ಟುಗಳು(s, ಮೌಲ್ಯ, ಉದ್ದಳತೆ) \
{ ಪೂರ್ಣಾಂಕ len = ಉದ್ದಳತೆ;\
  ಆದರೆ (s->bi_valid > (ಪೂರ್ಣಾಂಕ)Buf_ಗಾತ್ರ - len) {\
    ಪೂರ್ಣಾಂಕ val = (ಪೂರ್ಣಾಂಕ)ಮೌಲ್ಯ;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_ಗಾತ್ರ - s->bi_valid);\
    s->bi_valid += len - Buf_ಗಾತ್ರ;\
  } ಇಲ್ಲದಿದ್ದರೆ {\
    s->bi_buf |= (ush)(ಮೌಲ್ಯ) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#ಆದರೆ_ಅಂತ್ಯ /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local ಶೂನ್ಯ tr_static_init(ಶೂನ್ಯ) {
#ಆದರೆ defined(GEN_TREES_H) || !defined(STDC)
    ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ static_init_ಮುಗಿದ = 0;
    ಪೂರ್ಣಾಂಕ n;        /* iterates over tree elements */
    ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು;     /* bit counter */
    ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ;   /* length value */
    ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2;     /* code value */
    ಪೂರ್ಣಾಂಕ dist;     /* distance index */
    ush bl_ಎಣಿಕೆ[MAX_ಬಿಟ್ಟುಗಳು+1];
    /* number of codes at each bit length for an optimal tree */

    ಆದರೆ (static_init_ಮುಗಿದ) ಹಿಂತಿರುಗಿ;

    /* For some embedded targets, global variables are not initialized: */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_INIT_GLOBAL_ಸೂಚಕಗಳು
    static_l_desc.static_ವೃಕ್ಷ = static_ltree;
    static_l_desc.extra_ಬಿಟ್ಟುಗಳು = extra_lbits;
    static_d_desc.static_ವೃಕ್ಷ = static_dtree;
    static_d_desc.extra_ಬಿಟ್ಟುಗಳು = extra_dbits;
    static_bl_desc.extra_ಬಿಟ್ಟುಗಳು = extra_blbits;
#ಆದರೆ_ಅಂತ್ಯ

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    ಉದ್ದಳತೆ = 0;
    ಗಾಗಿ (ಸಂಕೇತ_2 = 0; ಸಂಕೇತ_2 < ಉದ್ದಳತೆ_CODES-1; ಸಂಕೇತ_2++) {
        ತಳ_ಉದ್ದಳತೆ[ಸಂಕೇತ_2] = ಉದ್ದಳತೆ;
        ಗಾಗಿ (n = 0; n < (1 << extra_lbits[ಸಂಕೇತ_2]); n++) {
            ಉದ್ದಳತೆ_ಸಂಕೇತ[ಉದ್ದಳತೆ++] = (uch)ಸಂಕೇತ_2;
        }
    }
    Assert (ಉದ್ದಳತೆ == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    ಉದ್ದಳತೆ_ಸಂಕೇತ[ಉದ್ದಳತೆ - 1] = (uch)ಸಂಕೇತ_2;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    ಗಾಗಿ (ಸಂಕೇತ_2 = 0 ; ಸಂಕೇತ_2 < 16; ಸಂಕೇತ_2++) {
        ತಳ_dist[ಸಂಕೇತ_2] = dist;
        ಗಾಗಿ (n = 0; n < (1 << extra_dbits[ಸಂಕೇತ_2]); n++) {
            dist_ಸಂಕೇತ[dist++] = (uch)ಸಂಕೇತ_2;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    ಗಾಗಿ ( ; ಸಂಕೇತ_2 < D_CODES; ಸಂಕೇತ_2++) {
        ತಳ_dist[ಸಂಕೇತ_2] = dist << 7;
        ಗಾಗಿ (n = 0; n < (1 << (extra_dbits[ಸಂಕೇತ_2] - 7)); n++) {
            dist_ಸಂಕೇತ[256 + dist++] = (uch)ಸಂಕೇತ_2;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    ಗಾಗಿ (ಬಿಟ್ಟುಗಳು = 0; ಬಿಟ್ಟುಗಳು <= MAX_ಬಿಟ್ಟುಗಳು; ಬಿಟ್ಟುಗಳು++) bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು] = 0;
    n = 0;
    ಆಗ (n <= 143) static_ltree[n++].Len = 8, bl_ಎಣಿಕೆ[8]++;
    ಆಗ (n <= 255) static_ltree[n++].Len = 9, bl_ಎಣಿಕೆ[9]++;
    ಆಗ (n <= 279) static_ltree[n++].Len = 7, bl_ಎಣಿಕೆ[7]++;
    ಆಗ (n <= 287) static_ltree[n++].Len = 8, bl_ಎಣಿಕೆ[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_ದತ್ತಾಂಶ *)static_ltree, L_CODES+1, bl_ಎಣಿಕೆ);

    /* The static distance tree is trivial: */
    ಗಾಗಿ (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].ಸಂಕೇತ_3 = bi_ಹಿಮ್ಮುಖ((ಚಿಹ್ನೆರಹಿತ)n, 5);
    }
    static_init_ಮುಗಿದ = 1;

#  ifdef GEN_TREES_H
    gen_trees_ಶೀರ್ಷಿಕೆ();
#  endif
#ಆದರೆ_ಅಂತ್ಯ /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, ಕೊನೆಯ, ಅಗಲ) \
      ((i) == (ಕೊನೆಯ)? "\n};\n\n" :    \
       ((i) % (ಅಗಲ) == (ಅಗಲ) - 1 ? ",\n" : ", "))

ಶೂನ್ಯ gen_trees_ಶೀರ್ಷಿಕೆ(ಶೂನ್ಯ) {
    FILE *ಶೀರ್ಷಿಕೆ = fopen("trees.h", "w");
    ಪೂರ್ಣಾಂಕ i;

    Assert (ಶೀರ್ಷಿಕೆ != ಖಾಲಿ, "Can't open trees.h");
    fprintf(ಶೀರ್ಷಿಕೆ,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(ಶೀರ್ಷಿಕೆ, "local const ct_data static_ltree[L_CODES+2] = {\n");
    ಗಾಗಿ (i = 0; i < L_CODES+2; i++) {
        fprintf(ಶೀರ್ಷಿಕೆ, "{{%3u},{%3u}}%s", static_ltree[i].ಸಂಕೇತ_3,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(ಶೀರ್ಷಿಕೆ, "local const ct_data static_dtree[D_CODES] = {\n");
    ಗಾಗಿ (i = 0; i < D_CODES; i++) {
        fprintf(ಶೀರ್ಷಿಕೆ, "{{%2u},{%2u}}%s", static_dtree[i].ಸಂಕೇತ_3,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(ಶೀರ್ಷಿಕೆ, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    ಗಾಗಿ (i = 0; i < DIST_ಸಂಕೇತ_LEN; i++) {
        fprintf(ಶೀರ್ಷಿಕೆ, "%2u%s", dist_ಸಂಕೇತ[i],
                SEPARATOR(i, DIST_ಸಂಕೇತ_LEN-1, 20));
    }

    fprintf(ಶೀರ್ಷಿಕೆ,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    ಗಾಗಿ (i = 0; i < MAX_ಹೊಂದಿಕೆ-MIN_ಹೊಂದಿಕೆ+1; i++) {
        fprintf(ಶೀರ್ಷಿಕೆ, "%2u%s", ಉದ್ದಳತೆ_ಸಂಕೇತ[i],
                SEPARATOR(i, MAX_ಹೊಂದಿಕೆ-MIN_ಹೊಂದಿಕೆ, 20));
    }

    fprintf(ಶೀರ್ಷಿಕೆ, "local const int base_length[LENGTH_CODES] = {\n");
    ಗಾಗಿ (i = 0; i < ಉದ್ದಳತೆ_CODES; i++) {
        fprintf(ಶೀರ್ಷಿಕೆ, "%1u%s", ತಳ_ಉದ್ದಳತೆ[i],
                SEPARATOR(i, ಉದ್ದಳತೆ_CODES-1, 20));
    }

    fprintf(ಶೀರ್ಷಿಕೆ, "local const int base_dist[D_CODES] = {\n");
    ಗಾಗಿ (i = 0; i < D_CODES; i++) {
        fprintf(ಶೀರ್ಷಿಕೆ, "%5u%s", ತಳ_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(ಶೀರ್ಷಿಕೆ);
}
#ಆದರೆ_ಅಂತ್ಯ /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local ಶೂನ್ಯ init_ಖಂಡ(deflate_state *s) {
    ಪೂರ್ಣಾಂಕ n; /* iterates over tree elements */

    /* Initialize the trees. */
    ಗಾಗಿ (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    ಗಾಗಿ (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    ಗಾಗಿ (n = 0; n < BL_CODES; n++) s->bl_ವೃಕ್ಷ[n].Freq = 0;

    s->dyn_ltree[ಕೊನೆ_ಖಂಡ].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_ಮುಂದಿನ = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_ವೃಕ್ಷ = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_ವೃಕ್ಷ = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_ವೃಕ್ಷ = s->bl_ವೃಕ್ಷ;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_ಬಳಸಿದ = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
    s->compressed_len = 0L;
    s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ = 0L;
#ಆದರೆ_ಅಂತ್ಯ

    /* Initialize the first block of the first file: */
    init_ಖಂಡ(s);
}

#ವ್ಯಾಖ್ಯಾನಿಸು ಅತಿಚಿಕ್ಕ 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು pqremove(s, ವೃಕ್ಷ, ತುದಿ) \
{\
    ತುದಿ = s->ರಾಶಿ[ಅತಿಚಿಕ್ಕ]; \
    s->ರಾಶಿ[ಅತಿಚಿಕ್ಕ] = s->ರಾಶಿ[s->ರಾಶಿ_len--]; \
    pqdownheap(s, ವೃಕ್ಷ, ಅತಿಚಿಕ್ಕ); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು smaller(ವೃಕ್ಷ, n, m, ಆಳ) \
   (ವೃಕ್ಷ[n].Freq < ವೃಕ್ಷ[m].Freq || \
   (ವೃಕ್ಷ[n].Freq == ವೃಕ್ಷ[m].Freq && ಆಳ[n] <= ಆಳ[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local ಶೂನ್ಯ pqdownheap(deflate_state *s, ct_ದತ್ತಾಂಶ *ವೃಕ್ಷ, ಪೂರ್ಣಾಂಕ k) {
    ಪೂರ್ಣಾಂಕ v = s->ರಾಶಿ[k];
    ಪೂರ್ಣಾಂಕ j = k << 1;  /* left son of k */
    ಆಗ (j <= s->ರಾಶಿ_len) {
        /* Set j to the smallest of the two sons: */
        ಆದರೆ (j < s->ರಾಶಿ_len &&
            smaller(ವೃಕ್ಷ, s->ರಾಶಿ[j + 1], s->ರಾಶಿ[j], s->ಆಳ)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        ಆದರೆ (smaller(ವೃಕ್ಷ, v, s->ರಾಶಿ[j], s->ಆಳ)) ನಿಲ್ಲಿಸು;

        /* Exchange v with the smallest son */
        s->ರಾಶಿ[k] = s->ರಾಶಿ[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->ರಾಶಿ[k] = v;
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
local ಶೂನ್ಯ gen_bitlen(deflate_state *s, ವೃಕ್ಷ_desc *desc) {
    ct_ದತ್ತಾಂಶ *ವೃಕ್ಷ        = desc->dyn_ವೃಕ್ಷ;
    ಪೂರ್ಣಾಂಕ max_ಸಂಕೇತ         = desc->max_ಸಂಕೇತ;
    ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *stree = desc->stat_desc->static_ವೃಕ್ಷ;
    ಸ್ಥಿರಾಂಕ intf *extra    = desc->stat_desc->extra_ಬಿಟ್ಟುಗಳು;
    ಪೂರ್ಣಾಂಕ ತಳ_2             = desc->stat_desc->extra_ತಳ;
    ಪೂರ್ಣಾಂಕ max_ಉದ್ದಳತೆ       = desc->stat_desc->max_ಉದ್ದಳತೆ;
    ಪೂರ್ಣಾಂಕ h;              /* heap index */
    ಪೂರ್ಣಾಂಕ n, m;           /* iterate over the tree elements */
    ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು;           /* bit length */
    ಪೂರ್ಣಾಂಕ xbits;          /* extra bits */
    ush f;              /* frequency */
    ಪೂರ್ಣಾಂಕ ಅತಿಪ್ರವಾಹ = 0;   /* number of elements with bit length too large */

    ಗಾಗಿ (ಬಿಟ್ಟುಗಳು = 0; ಬಿಟ್ಟುಗಳು <= MAX_ಬಿಟ್ಟುಗಳು; ಬಿಟ್ಟುಗಳು++) s->bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    ವೃಕ್ಷ[s->ರಾಶಿ[s->ರಾಶಿ_max]].Len = 0; /* root of the heap */

    ಗಾಗಿ (h = s->ರಾಶಿ_max + 1; h < ರಾಶಿ_ಗಾತ್ರ; h++) {
        n = s->ರಾಶಿ[h];
        ಬಿಟ್ಟುಗಳು = ವೃಕ್ಷ[ವೃಕ್ಷ[n].Dad].Len + 1;
        ಆದರೆ (ಬಿಟ್ಟುಗಳು > max_ಉದ್ದಳತೆ) ಬಿಟ್ಟುಗಳು = max_ಉದ್ದಳತೆ, ಅತಿಪ್ರವಾಹ++;
        ವೃಕ್ಷ[n].Len = (ush)ಬಿಟ್ಟುಗಳು;
        /* We overwrite tree[n].Dad which is no longer needed */

        ಆದರೆ (n > max_ಸಂಕೇತ) ಮುಂದುವರಿ; /* not a leaf node */

        s->bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು]++;
        xbits = 0;
        ಆದರೆ (n >= ತಳ_2) xbits = extra[n - ತಳ_2];
        f = ವೃಕ್ಷ[n].Freq;
        s->opt_len += (ulg)f * (ಚಿಹ್ನೆರಹಿತ)(ಬಿಟ್ಟುಗಳು + xbits);
        ಆದರೆ (stree) s->static_len += (ulg)f * (ಚಿಹ್ನೆರಹಿತ)(stree[n].Len + xbits);
    }
    ಆದರೆ (ಅತಿಪ್ರವಾಹ == 0) ಹಿಂತಿರುಗಿ;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    ಮಾಡು {
        ಬಿಟ್ಟುಗಳು = max_ಉದ್ದಳತೆ - 1;
        ಆಗ (s->bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು] == 0) ಬಿಟ್ಟುಗಳು--;
        s->bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು]--;        /* move one leaf down the tree */
        s->bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು + 1] += 2; /* move one overflow item as its brother */
        s->bl_ಎಣಿಕೆ[max_ಉದ್ದಳತೆ]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        ಅತಿಪ್ರವಾಹ -= 2;
    } ಆಗ (ಅತಿಪ್ರವಾಹ > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    ಗಾಗಿ (ಬಿಟ್ಟುಗಳು = max_ಉದ್ದಳತೆ; ಬಿಟ್ಟುಗಳು != 0; ಬಿಟ್ಟುಗಳು--) {
        n = s->bl_ಎಣಿಕೆ[ಬಿಟ್ಟುಗಳು];
        ಆಗ (n != 0) {
            m = s->ರಾಶಿ[--h];
            ಆದರೆ (m > max_ಸಂಕೇತ) ಮುಂದುವರಿ;
            ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ) ವೃಕ್ಷ[m].Len != (ಚಿಹ್ನೆರಹಿತ) ಬಿಟ್ಟುಗಳು) {
                Tracev((stderr,"code %d bits %d->%d\n", m, ವೃಕ್ಷ[m].Len, ಬಿಟ್ಟುಗಳು));
                s->opt_len += ((ulg)ಬಿಟ್ಟುಗಳು - ವೃಕ್ಷ[m].Len) * ವೃಕ್ಷ[m].Freq;
                ವೃಕ್ಷ[m].Len = (ush)ಬಿಟ್ಟುಗಳು;
            }
            n--;
        }
    }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DUMP_BL_ವೃಕ್ಷ
#  include <stdio.h>
#ಆದರೆ_ಅಂತ್ಯ

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local ಶೂನ್ಯ build_ವೃಕ್ಷ(deflate_state *s, ವೃಕ್ಷ_desc *desc) {
    ct_ದತ್ತಾಂಶ *ವೃಕ್ಷ         = desc->dyn_ವೃಕ್ಷ;
    ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *stree  = desc->stat_desc->static_ವೃಕ್ಷ;
    ಪೂರ್ಣಾಂಕ elems             = desc->stat_desc->elems;
    ಪೂರ್ಣಾಂಕ n, m;          /* iterate over heap elements */
    ಪೂರ್ಣಾಂಕ max_ಸಂಕೇತ = -1; /* largest code with non zero frequency */
    ಪೂರ್ಣಾಂಕ ಗಂಟು;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->ರಾಶಿ_len = 0, s->ರಾಶಿ_max = ರಾಶಿ_ಗಾತ್ರ;

    ಗಾಗಿ (n = 0; n < elems; n++) {
        ಆದರೆ (ವೃಕ್ಷ[n].Freq != 0) {
            s->ರಾಶಿ[++(s->ರಾಶಿ_len)] = max_ಸಂಕೇತ = n;
            s->ಆಳ[n] = 0;
        } ಇಲ್ಲದಿದ್ದರೆ {
            ವೃಕ್ಷ[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    ಆಗ (s->ರಾಶಿ_len < 2) {
        ಗಂಟು = s->ರಾಶಿ[++(s->ರಾಶಿ_len)] = (max_ಸಂಕೇತ < 2 ? ++max_ಸಂಕೇತ : 0);
        ವೃಕ್ಷ[ಗಂಟು].Freq = 1;
        s->ಆಳ[ಗಂಟು] = 0;
        s->opt_len--; ಆದರೆ (stree) s->static_len -= stree[ಗಂಟು].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_ಸಂಕೇತ = max_ಸಂಕೇತ;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    ಗಾಗಿ (n = s->ರಾಶಿ_len/2; n >= 1; n--) pqdownheap(s, ವೃಕ್ಷ, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    ಗಂಟು = elems;              /* next internal node of the tree */
    ಮಾಡು {
        pqremove(s, ವೃಕ್ಷ, n);  /* n = node of least frequency */
        m = s->ರಾಶಿ[ಅತಿಚಿಕ್ಕ]; /* m = node of next least frequency */

        s->ರಾಶಿ[--(s->ರಾಶಿ_max)] = n; /* keep the nodes sorted by frequency */
        s->ರಾಶಿ[--(s->ರಾಶಿ_max)] = m;

        /* Create a new node father of n and m */
        ವೃಕ್ಷ[ಗಂಟು].Freq = ವೃಕ್ಷ[n].Freq + ವೃಕ್ಷ[m].Freq;
        s->ಆಳ[ಗಂಟು] = (uch)((s->ಆಳ[n] >= s->ಆಳ[m] ?
                                s->ಆಳ[n] : s->ಆಳ[m]) + 1);
        ವೃಕ್ಷ[n].Dad = ವೃಕ್ಷ[m].Dad = (ush)ಗಂಟು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DUMP_BL_ವೃಕ್ಷ
        ಆದರೆ (ವೃಕ್ಷ == s->bl_ವೃಕ್ಷ) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    ಗಂಟು, ವೃಕ್ಷ[ಗಂಟು].Freq, n, ವೃಕ್ಷ[n].Freq, m, ವೃಕ್ಷ[m].Freq);
        }
#ಆದರೆ_ಅಂತ್ಯ
        /* and insert the new node in the heap */
        s->ರಾಶಿ[ಅತಿಚಿಕ್ಕ] = ಗಂಟು++;
        pqdownheap(s, ವೃಕ್ಷ, ಅತಿಚಿಕ್ಕ);

    } ಆಗ (s->ರಾಶಿ_len >= 2);

    s->ರಾಶಿ[--(s->ರಾಶಿ_max)] = s->ರಾಶಿ[ಅತಿಚಿಕ್ಕ];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (ವೃಕ್ಷ_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_ದತ್ತಾಂಶ *)ವೃಕ್ಷ, max_ಸಂಕೇತ, s->bl_ಎಣಿಕೆ);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local ಶೂನ್ಯ ಪರಿಶೀಲನೆ_ವೃಕ್ಷ(deflate_state *s, ct_ದತ್ತಾಂಶ *ವೃಕ್ಷ, ಪೂರ್ಣಾಂಕ max_ಸಂಕೇತ) {
    ಪೂರ್ಣಾಂಕ n;                     /* iterates over all tree elements */
    ಪೂರ್ಣಾಂಕ prevlen = -1;          /* last emitted length */
    ಪೂರ್ಣಾಂಕ curlen;                /* length of current code */
    ಪೂರ್ಣಾಂಕ nextlen = ವೃಕ್ಷ[0].Len; /* length of next code */
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;             /* repeat count of the current code */
    ಪೂರ್ಣಾಂಕ max_ಎಣಿಕೆ = 7;         /* max repeat count */
    ಪೂರ್ಣಾಂಕ min_ಎಣಿಕೆ = 4;         /* min repeat count */

    ಆದರೆ (nextlen == 0) max_ಎಣಿಕೆ = 138, min_ಎಣಿಕೆ = 3;
    ವೃಕ್ಷ[max_ಸಂಕೇತ + 1].Len = (ush)0xffff; /* guard */

    ಗಾಗಿ (n = 0; n <= max_ಸಂಕೇತ; n++) {
        curlen = nextlen; nextlen = ವೃಕ್ಷ[n + 1].Len;
        ಆದರೆ (++ಎಣಿಕೆ_2 < max_ಎಣಿಕೆ && curlen == nextlen) {
            ಮುಂದುವರಿ;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಣಿಕೆ_2 < min_ಎಣಿಕೆ) {
            s->bl_ವೃಕ್ಷ[curlen].Freq += (ush)ಎಣಿಕೆ_2;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (curlen != 0) {
            ಆದರೆ (curlen != prevlen) s->bl_ವೃಕ್ಷ[curlen].Freq++;
            s->bl_ವೃಕ್ಷ[REP_3_6].Freq++;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಣಿಕೆ_2 <= 10) {
            s->bl_ವೃಕ್ಷ[REPZ_3_10].Freq++;
        } ಇಲ್ಲದಿದ್ದರೆ {
            s->bl_ವೃಕ್ಷ[REPZ_11_138].Freq++;
        }
        ಎಣಿಕೆ_2 = 0; prevlen = curlen;
        ಆದರೆ (nextlen == 0) {
            max_ಎಣಿಕೆ = 138, min_ಎಣಿಕೆ = 3;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (curlen == nextlen) {
            max_ಎಣಿಕೆ = 6, min_ಎಣಿಕೆ = 3;
        } ಇಲ್ಲದಿದ್ದರೆ {
            max_ಎಣಿಕೆ = 7, min_ಎಣಿಕೆ = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local ಶೂನ್ಯ send_ವೃಕ್ಷ(deflate_state *s, ct_ದತ್ತಾಂಶ *ವೃಕ್ಷ, ಪೂರ್ಣಾಂಕ max_ಸಂಕೇತ) {
    ಪೂರ್ಣಾಂಕ n;                     /* iterates over all tree elements */
    ಪೂರ್ಣಾಂಕ prevlen = -1;          /* last emitted length */
    ಪೂರ್ಣಾಂಕ curlen;                /* length of current code */
    ಪೂರ್ಣಾಂಕ nextlen = ವೃಕ್ಷ[0].Len; /* length of next code */
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;             /* repeat count of the current code */
    ಪೂರ್ಣಾಂಕ max_ಎಣಿಕೆ = 7;         /* max repeat count */
    ಪೂರ್ಣಾಂಕ min_ಎಣಿಕೆ = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    ಆದರೆ (nextlen == 0) max_ಎಣಿಕೆ = 138, min_ಎಣಿಕೆ = 3;

    ಗಾಗಿ (n = 0; n <= max_ಸಂಕೇತ; n++) {
        curlen = nextlen; nextlen = ವೃಕ್ಷ[n + 1].Len;
        ಆದರೆ (++ಎಣಿಕೆ_2 < max_ಎಣಿಕೆ && curlen == nextlen) {
            ಮುಂದುವರಿ;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಣಿಕೆ_2 < min_ಎಣಿಕೆ) {
            ಮಾಡು { send_ಸಂಕೇತ(s, curlen, s->bl_ವೃಕ್ಷ); } ಆಗ (--ಎಣಿಕೆ_2 != 0);

        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (curlen != 0) {
            ಆದರೆ (curlen != prevlen) {
                send_ಸಂಕೇತ(s, curlen, s->bl_ವೃಕ್ಷ); ಎಣಿಕೆ_2--;
            }
            Assert(ಎಣಿಕೆ_2 >= 3 && ಎಣಿಕೆ_2 <= 6, " 3_6?");
            send_ಸಂಕೇತ(s, REP_3_6, s->bl_ವೃಕ್ಷ); send_ಬಿಟ್ಟುಗಳು(s, ಎಣಿಕೆ_2 - 3, 2);

        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎಣಿಕೆ_2 <= 10) {
            send_ಸಂಕೇತ(s, REPZ_3_10, s->bl_ವೃಕ್ಷ); send_ಬಿಟ್ಟುಗಳು(s, ಎಣಿಕೆ_2 - 3, 3);

        } ಇಲ್ಲದಿದ್ದರೆ {
            send_ಸಂಕೇತ(s, REPZ_11_138, s->bl_ವೃಕ್ಷ); send_ಬಿಟ್ಟುಗಳು(s, ಎಣಿಕೆ_2 - 11, 7);
        }
        ಎಣಿಕೆ_2 = 0; prevlen = curlen;
        ಆದರೆ (nextlen == 0) {
            max_ಎಣಿಕೆ = 138, min_ಎಣಿಕೆ = 3;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (curlen == nextlen) {
            max_ಎಣಿಕೆ = 6, min_ಎಣಿಕೆ = 3;
        } ಇಲ್ಲದಿದ್ದರೆ {
            max_ಎಣಿಕೆ = 7, min_ಎಣಿಕೆ = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local ಪೂರ್ಣಾಂಕ build_bl_ವೃಕ್ಷ(deflate_state *s) {
    ಪೂರ್ಣಾಂಕ max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    ಪರಿಶೀಲನೆ_ವೃಕ್ಷ(s, (ct_ದತ್ತಾಂಶ *)s->dyn_ltree, s->l_desc.max_ಸಂಕೇತ);
    ಪರಿಶೀಲನೆ_ವೃಕ್ಷ(s, (ct_ದತ್ತಾಂಶ *)s->dyn_dtree, s->d_desc.max_ಸಂಕೇತ);

    /* Build the bit length tree: */
    build_ವೃಕ್ಷ(s, (ವೃಕ್ಷ_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    ಗಾಗಿ (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        ಆದರೆ (s->bl_ವೃಕ್ಷ[bl_ಕ್ರಮ[max_blindex]].Len != 0) ನಿಲ್ಲಿಸು;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((stderr, "\ndyn trees: dyn %lu, stat %lu",
            s->opt_len, s->static_len));

    ಹಿಂತಿರುಗಿ max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local ಶೂನ್ಯ send_all_trees(deflate_state *s, ಪೂರ್ಣಾಂಕ lcodes, ಪೂರ್ಣಾಂಕ dcodes,
                          ಪೂರ್ಣಾಂಕ blcodes) {
    ಪೂರ್ಣಾಂಕ rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_ಬಿಟ್ಟುಗಳು(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_ಬಿಟ್ಟುಗಳು(s, dcodes - 1,   5);
    send_ಬಿಟ್ಟುಗಳು(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    ಗಾಗಿ (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_ಕ್ರಮ[rank]));
        send_ಬಿಟ್ಟುಗಳು(s, s->bl_ವೃಕ್ಷ[bl_ಕ್ರಮ[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ));

    send_ವೃಕ್ಷ(s, (ct_ದತ್ತಾಂಶ *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ));

    send_ವೃಕ್ಷ(s, (ct_ದತ್ತಾಂಶ *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ));
}

/* ===========================================================================
 * Send a stored block
 */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ tr_stored_ಖಂಡ(deflate_state *s, charf *buf,
                                    ulg stored_len, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ) {
    send_ಬಿಟ್ಟುಗಳು(s, (STORED_ಖಂಡ<<1) + ಕೊನೆಯ, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    ಆದರೆ (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ += 2*16;
    s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ += stored_len << 3;
#ಆದರೆ_ಅಂತ್ಯ
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ tr_flush_ಬಿಟ್ಟುಗಳು(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ _tr_align(deflate_state *s) {
    send_ಬಿಟ್ಟುಗಳು(s, STATIC_TREES<<1, 3);
    send_ಸಂಕೇತ(s, ಕೊನೆ_ಖಂಡ, static_ltree);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#ಆದರೆ_ಅಂತ್ಯ
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local ಶೂನ್ಯ compress_ಖಂಡ(deflate_state *s, ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *ltree,
                          ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *dtree) {
    ಚಿಹ್ನೆರಹಿತ dist;      /* distance of matched string */
    ಪೂರ್ಣಾಂಕ lc;             /* match length or unmatched char (if dist == 0) */
    ಚಿಹ್ನೆರಹಿತ sx = 0;    /* running index in symbol buffers */
    ಚಿಹ್ನೆರಹಿತ ಸಂಕೇತ_2;      /* the code to send */
    ಪೂರ್ಣಾಂಕ extra;          /* number of extra bits to send */

    ಆದರೆ (s->sym_ಮುಂದಿನ != 0) ಮಾಡು {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#ಇಲ್ಲದಿದ್ದರೆ
        dist = s->sym_buf[sx++] & 0xff;
        dist += (ಚಿಹ್ನೆರಹಿತ)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#ಆದರೆ_ಅಂತ್ಯ
        ಆದರೆ (dist == 0) {
            send_ಸಂಕೇತ(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } ಇಲ್ಲದಿದ್ದರೆ {
            /* Here, lc is the match length - MIN_MATCH */
            ಸಂಕೇತ_2 = ಉದ್ದಳತೆ_ಸಂಕೇತ[lc];
            send_ಸಂಕೇತ(s, ಸಂಕೇತ_2 + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[ಸಂಕೇತ_2];
            ಆದರೆ (extra != 0) {
                lc -= ತಳ_ಉದ್ದಳತೆ[ಸಂಕೇತ_2];
                send_ಬಿಟ್ಟುಗಳು(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            ಸಂಕೇತ_2 = d_ಸಂಕೇತ(dist);
            Assert (ಸಂಕೇತ_2 < D_CODES, "bad d_code");

            send_ಸಂಕೇತ(s, ಸಂಕೇತ_2, dtree);       /* send the distance code */
            extra = extra_dbits[ಸಂಕೇತ_2];
            ಆದರೆ (extra != 0) {
                dist -= (ಚಿಹ್ನೆರಹಿತ)ತಳ_dist[ಸಂಕೇತ_2];
                send_ಬಿಟ್ಟುಗಳು(s, (ಪೂರ್ಣಾಂಕ)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#ಇಲ್ಲದಿದ್ದರೆ
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#ಆದರೆ_ಅಂತ್ಯ

    } ಆಗ (sx < s->sym_ಮುಂದಿನ);

    send_ಸಂಕೇತ(s, ಕೊನೆ_ಖಂಡ, ltree);
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
local ಪೂರ್ಣಾಂಕ detect_ದತ್ತಾಂಶ_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಖಂಡ_ಮುಖವಾಡ = 0xf3ffc07fUL;
    ಪೂರ್ಣಾಂಕ n;

    /* Check for non-textual ("block-listed") bytes. */
    ಗಾಗಿ (n = 0; n <= 31; n++, ಖಂಡ_ಮುಖವಾಡ >>= 1)
        ಆದರೆ ((ಖಂಡ_ಮುಖವಾಡ & 1) && (s->dyn_ltree[n].Freq != 0))
            ಹಿಂತಿರುಗಿ Z_ದ್ವಿಮಾನ;

    /* Check for textual ("allow-listed") bytes. */
    ಆದರೆ (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        ಹಿಂತಿರುಗಿ Z_ಪಠ್ಯ;
    ಗಾಗಿ (n = 32; n < LITERALS; n++)
        ಆದರೆ (s->dyn_ltree[n].Freq != 0)
            ಹಿಂತಿರುಗಿ Z_ಪಠ್ಯ;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    ಹಿಂತಿರುಗಿ Z_ದ್ವಿಮಾನ;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ tr_flush_ಖಂಡ(deflate_state *s, charf *buf,
                                   ulg stored_len, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    ಪೂರ್ಣಾಂಕ max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    ಆದರೆ (s->level > 0) {

        /* Check if the file is binary or text */
        ಆದರೆ (s->strm->ದತ್ತಾಂಶ_type == Z_UNKNOWN)
            s->strm->ದತ್ತಾಂಶ_type = detect_ದತ್ತಾಂಶ_type(s);

        /* Construct the literal and distance trees */
        build_ವೃಕ್ಷ(s, (ವೃಕ್ಷ_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_ವೃಕ್ಷ(s, (ವೃಕ್ಷ_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_ವೃಕ್ಷ(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_ಮುಂದಿನ / 3));

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FORCE_STATIC
        ಆದರೆ (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#ಆದರೆ_ಅಂತ್ಯ
            opt_lenb = static_lenb;

    } ಇಲ್ಲದಿದ್ದರೆ {
        Assert(buf != (ಅಕ್ಷರ*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FORCE_STORED
    ಆದರೆ (buf != (ಅಕ್ಷರ*)0) { /* force stored block */
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (stored_len + 4 <= opt_lenb && buf != (ಅಕ್ಷರ*)0) {
                       /* 4: two words for the lengths */
#ಆದರೆ_ಅಂತ್ಯ
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_ಖಂಡ(s, buf, stored_len, ಕೊನೆಯ);

    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (static_lenb == opt_lenb) {
        send_ಬಿಟ್ಟುಗಳು(s, (STATIC_TREES<<1) + ಕೊನೆಯ, 3);
        compress_ಖಂಡ(s, (ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *)static_ltree,
                       (ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *)static_dtree);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#ಆದರೆ_ಅಂತ್ಯ
    } ಇಲ್ಲದಿದ್ದರೆ {
        send_ಬಿಟ್ಟುಗಳು(s, (DYN_TREES<<1) + ಕೊನೆಯ, 3);
        send_all_trees(s, s->l_desc.max_ಸಂಕೇತ + 1, s->d_desc.max_ಸಂಕೇತ + 1,
                       max_blindex + 1);
        compress_ಖಂಡ(s, (ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *)s->dyn_ltree,
                       (ಸ್ಥಿರಾಂಕ ct_ದತ್ತಾಂಶ *)s->dyn_dtree);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#ಆದರೆ_ಅಂತ್ಯ
    }
    Assert (s->compressed_len == s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_ಖಂಡ(s);

    ಆದರೆ (ಕೊನೆಯ) {
        bi_windup(s);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#ಆದರೆ_ಅಂತ್ಯ
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)ಕೊನೆಯ));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
ಪೂರ್ಣಾಂಕ ZLIB_ಆಂತರಿಕ _tr_tally(deflate_state *s, ಚಿಹ್ನೆರಹಿತ dist, ಚಿಹ್ನೆರಹಿತ lc) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
    s->d_buf[s->sym_ಮುಂದಿನ] = (ush)dist;
    s->l_buf[s->sym_ಮುಂದಿನ++] = (uch)lc;
#ಇಲ್ಲದಿದ್ದರೆ
    s->sym_buf[s->sym_ಮುಂದಿನ++] = (uch)dist;
    s->sym_buf[s->sym_ಮುಂದಿನ++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_ಮುಂದಿನ++] = (uch)lc;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } ಇಲ್ಲದಿದ್ದರೆ {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_ಹೊಂದಿಕೆ-MIN_ಹೊಂದಿಕೆ) &&
               (ush)d_ಸಂಕೇತ(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[ಉದ್ದಳತೆ_ಸಂಕೇತ[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_ಸಂಕೇತ(dist)].Freq++;
    }
    ಹಿಂತಿರುಗಿ (s->sym_ಮುಂದಿನ == s->sym_ಕೊನೆ);
}
