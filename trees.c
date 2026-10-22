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

#ضمن "deflate.h"

#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
#  include <ctype.h>
#جيڪڏهن_پورو

/* ===========================================================================
 * Constants
 */

#وصف_ڪريو MAX_BL_بٽ 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#وصف_ڪريو پڇاڙي_بلاڪ 256
/* end of block literal code */

#وصف_ڪريو REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#وصف_ڪريو REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#وصف_ڪريو REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ثابت صحيح extra_lbits[ڊيگهائي_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local ثابت صحيح extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local ثابت صحيح extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local ثابت uch bl_ترتيب[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#وصف_ڪريو DIST_ڪوڊ_LEN  512 /* see definition of array dist_code below */

#إذا defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_معلومات static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_معلومات static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch dist_ڪوڊ[DIST_ڪوڊ_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch ڊيگهائي_ڪوڊ[MAX_ميلاپ-MIN_ميلاپ+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local صحيح بنياد_ڊيگهائي[ڊيگهائي_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local صحيح بنياد_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#وإلا
#  include "trees.h"
#جيڪڏهن_پورو /* defined(GEN_TREES_H) || !defined(STDC) */

بنية static_وڻ_desc_s {
    ثابت ct_معلومات *static_وڻ;  /* static tree or NULL */
    ثابت intf *extra_بٽ;      /* extra bits for each code or NULL */
    صحيح     extra_بنياد;          /* base index for extra_bits */
    صحيح     elems;               /* max number of elements in the tree */
    صحيح     max_ڊيگهائي;          /* max bit length for the codes */
};

#جيڪڏهن_وصف_ٿيل NO_INIT_GLOBAL_اشارا
#  define TCONST
#وإلا
#  define TCONST ثابت
#جيڪڏهن_پورو

local TCONST static_وڻ_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_بٽ};

local TCONST static_وڻ_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_بٽ};

local TCONST static_وڻ_desc static_bl_desc =
{(ثابت ct_معلومات *)0, extra_blbits, 0,   BL_CODES, MAX_BL_بٽ};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#وصف_ڪريو put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local بنانشان bi_ابتو(بنانشان ڪوڊ, صحيح len) {
    بنانشان res = 0;
    افعل {
        res |= ڪوڊ & 1;
        ڪوڊ >>= 1, res <<= 1;
    } طالما (--len > 0);
    أرجع res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local فراغ bi_flush(deflate_state *s) {
    إذا (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } وإلا إذا (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local فراغ bi_windup(deflate_state *s) {
    إذا (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } وإلا إذا (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_استعمال_ٿيل = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
    s->بٽ_موڪليل = (s->بٽ_موڪليل + 7) & ~(ulg)7;
#جيڪڏهن_پورو
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local فراغ gen_codes(ct_معلومات *وڻ, صحيح max_ڪوڊ, ushf *bl_ڳڻپ) {
    ush ايندڙ_ڪوڊ[MAX_بٽ+1]; /* next code value for each bit length */
    بنانشان ڪوڊ = 0;         /* running code value */
    صحيح بٽ;                  /* bit index */
    صحيح n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    لكل (بٽ = 1; بٽ <= MAX_بٽ; بٽ++) {
        ڪوڊ = (ڪوڊ + bl_ڳڻپ[بٽ - 1]) << 1;
        ايندڙ_ڪوڊ[بٽ] = (ush)ڪوڊ;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (ڪوڊ + bl_ڳڻپ[MAX_بٽ] - 1 == (1 << MAX_بٽ) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_ڪوڊ));

    لكل (n = 0;  n <= max_ڪوڊ; n++) {
        صحيح len = وڻ[n].Len;
        إذا (len == 0) استمر;
        /* Now reverse the bits */
        وڻ[n].ڪوڊ_2 = (ush)bi_ابتو(ايندڙ_ڪوڊ[len]++, len);

        Tracecv(وڻ != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, وڻ[n].ڪوڊ_2, ايندڙ_ڪوڊ[len] - 1));
    }
}

#جيڪڏهن_وصف_ٿيل GEN_TREES_H
local فراغ gen_trees_سرو(فراغ);
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل ZLIB_DEBUG
#  define send_ڪوڊ(s, c, وڻ) send_بٽ(s, وڻ[c].ڪوڊ_2, وڻ[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#وإلا /* !ZLIB_DEBUG */
#  define send_ڪوڊ(s, c, وڻ) \
     { إذا (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_بٽ(s, وڻ[c].ڪوڊ_2, وڻ[c].Len); }
#جيڪڏهن_پورو

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
local فراغ send_بٽ(deflate_state *s, صحيح قدر, صحيح ڊيگهائي) {
    Tracevv((stderr," l %2d v %4x ", ڊيگهائي, قدر));
    Assert(ڊيگهائي > 0 && ڊيگهائي <= 15, "invalid length");
    s->بٽ_موڪليل += (ulg)ڊيگهائي;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    إذا (s->bi_valid > (صحيح)Buf_سائيز - ڊيگهائي) {
        s->bi_buf |= (ush)قدر << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)قدر >> (Buf_سائيز - s->bi_valid);
        s->bi_valid += ڊيگهائي - Buf_سائيز;
    } وإلا {
        s->bi_buf |= (ush)قدر << s->bi_valid;
        s->bi_valid += ڊيگهائي;
    }
}
#وإلا /* !ZLIB_DEBUG */

#وصف_ڪريو send_بٽ(s, قدر, ڊيگهائي) \
{ صحيح len = ڊيگهائي;\
  إذا (s->bi_valid > (صحيح)Buf_سائيز - len) {\
    صحيح val = (صحيح)قدر;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_سائيز - s->bi_valid);\
    s->bi_valid += len - Buf_سائيز;\
  } وإلا {\
    s->bi_buf |= (ush)(قدر) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#جيڪڏهن_پورو /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local فراغ tr_static_init(فراغ) {
#إذا defined(GEN_TREES_H) || !defined(STDC)
    ساكن صحيح static_init_پورو = 0;
    صحيح n;        /* iterates over tree elements */
    صحيح بٽ;     /* bit counter */
    صحيح ڊيگهائي;   /* length value */
    صحيح ڪوڊ;     /* code value */
    صحيح dist;     /* distance index */
    ush bl_ڳڻپ[MAX_بٽ+1];
    /* number of codes at each bit length for an optimal tree */

    إذا (static_init_پورو) أرجع;

    /* For some embedded targets, global variables are not initialized: */
#جيڪڏهن_وصف_ٿيل NO_INIT_GLOBAL_اشارا
    static_l_desc.static_وڻ = static_ltree;
    static_l_desc.extra_بٽ = extra_lbits;
    static_d_desc.static_وڻ = static_dtree;
    static_d_desc.extra_بٽ = extra_dbits;
    static_bl_desc.extra_بٽ = extra_blbits;
#جيڪڏهن_پورو

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    ڊيگهائي = 0;
    لكل (ڪوڊ = 0; ڪوڊ < ڊيگهائي_CODES-1; ڪوڊ++) {
        بنياد_ڊيگهائي[ڪوڊ] = ڊيگهائي;
        لكل (n = 0; n < (1 << extra_lbits[ڪوڊ]); n++) {
            ڊيگهائي_ڪوڊ[ڊيگهائي++] = (uch)ڪوڊ;
        }
    }
    Assert (ڊيگهائي == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    ڊيگهائي_ڪوڊ[ڊيگهائي - 1] = (uch)ڪوڊ;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    لكل (ڪوڊ = 0 ; ڪوڊ < 16; ڪوڊ++) {
        بنياد_dist[ڪوڊ] = dist;
        لكل (n = 0; n < (1 << extra_dbits[ڪوڊ]); n++) {
            dist_ڪوڊ[dist++] = (uch)ڪوڊ;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    لكل ( ; ڪوڊ < D_CODES; ڪوڊ++) {
        بنياد_dist[ڪوڊ] = dist << 7;
        لكل (n = 0; n < (1 << (extra_dbits[ڪوڊ] - 7)); n++) {
            dist_ڪوڊ[256 + dist++] = (uch)ڪوڊ;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    لكل (بٽ = 0; بٽ <= MAX_بٽ; بٽ++) bl_ڳڻپ[بٽ] = 0;
    n = 0;
    طالما (n <= 143) static_ltree[n++].Len = 8, bl_ڳڻپ[8]++;
    طالما (n <= 255) static_ltree[n++].Len = 9, bl_ڳڻپ[9]++;
    طالما (n <= 279) static_ltree[n++].Len = 7, bl_ڳڻپ[7]++;
    طالما (n <= 287) static_ltree[n++].Len = 8, bl_ڳڻپ[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_معلومات *)static_ltree, L_CODES+1, bl_ڳڻپ);

    /* The static distance tree is trivial: */
    لكل (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].ڪوڊ_2 = bi_ابتو((بنانشان)n, 5);
    }
    static_init_پورو = 1;

#  ifdef GEN_TREES_H
    gen_trees_سرو();
#  endif
#جيڪڏهن_پورو /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#جيڪڏهن_وصف_ٿيل GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, آخري, ويڪر) \
      ((i) == (آخري)? "\n};\n\n" :    \
       ((i) % (ويڪر) == (ويڪر) - 1 ? ",\n" : ", "))

فراغ gen_trees_سرو(فراغ) {
    FILE *سرو = fopen("trees.h", "w");
    صحيح i;

    Assert (سرو != عدم, "Can't open trees.h");
    fprintf(سرو,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(سرو, "local const ct_data static_ltree[L_CODES+2] = {\n");
    لكل (i = 0; i < L_CODES+2; i++) {
        fprintf(سرو, "{{%3u},{%3u}}%s", static_ltree[i].ڪوڊ_2,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(سرو, "local const ct_data static_dtree[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرو, "{{%2u},{%2u}}%s", static_dtree[i].ڪوڊ_2,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(سرو, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    لكل (i = 0; i < DIST_ڪوڊ_LEN; i++) {
        fprintf(سرو, "%2u%s", dist_ڪوڊ[i],
                SEPARATOR(i, DIST_ڪوڊ_LEN-1, 20));
    }

    fprintf(سرو,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    لكل (i = 0; i < MAX_ميلاپ-MIN_ميلاپ+1; i++) {
        fprintf(سرو, "%2u%s", ڊيگهائي_ڪوڊ[i],
                SEPARATOR(i, MAX_ميلاپ-MIN_ميلاپ, 20));
    }

    fprintf(سرو, "local const int base_length[LENGTH_CODES] = {\n");
    لكل (i = 0; i < ڊيگهائي_CODES; i++) {
        fprintf(سرو, "%1u%s", بنياد_ڊيگهائي[i],
                SEPARATOR(i, ڊيگهائي_CODES-1, 20));
    }

    fprintf(سرو, "local const int base_dist[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرو, "%5u%s", بنياد_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(سرو);
}
#جيڪڏهن_پورو /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local فراغ init_بلاڪ(deflate_state *s) {
    صحيح n; /* iterates over tree elements */

    /* Initialize the trees. */
    لكل (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    لكل (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    لكل (n = 0; n < BL_CODES; n++) s->bl_وڻ[n].Freq = 0;

    s->dyn_ltree[پڇاڙي_بلاڪ].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_ايندڙ = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
فراغ ZLIB_اندروني _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_وڻ = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_وڻ = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_وڻ = s->bl_وڻ;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_استعمال_ٿيل = 0;
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
    s->compressed_len = 0L;
    s->بٽ_موڪليل = 0L;
#جيڪڏهن_پورو

    /* Initialize the first block of the first file: */
    init_بلاڪ(s);
}

#وصف_ڪريو سڀ_کان_ننڍو 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#وصف_ڪريو pqremove(s, وڻ, چوٽي) \
{\
    چوٽي = s->ڍير[سڀ_کان_ننڍو]; \
    s->ڍير[سڀ_کان_ننڍو] = s->ڍير[s->ڍير_len--]; \
    pqdownheap(s, وڻ, سڀ_کان_ننڍو); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#وصف_ڪريو smaller(وڻ, n, m, اونهائي) \
   (وڻ[n].Freq < وڻ[m].Freq || \
   (وڻ[n].Freq == وڻ[m].Freq && اونهائي[n] <= اونهائي[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local فراغ pqdownheap(deflate_state *s, ct_معلومات *وڻ, صحيح k) {
    صحيح v = s->ڍير[k];
    صحيح j = k << 1;  /* left son of k */
    طالما (j <= s->ڍير_len) {
        /* Set j to the smallest of the two sons: */
        إذا (j < s->ڍير_len &&
            smaller(وڻ, s->ڍير[j + 1], s->ڍير[j], s->اونهائي)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        إذا (smaller(وڻ, v, s->ڍير[j], s->اونهائي)) اكسر;

        /* Exchange v with the smallest son */
        s->ڍير[k] = s->ڍير[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->ڍير[k] = v;
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
local فراغ gen_bitlen(deflate_state *s, وڻ_desc *desc) {
    ct_معلومات *وڻ        = desc->dyn_وڻ;
    صحيح max_ڪوڊ         = desc->max_ڪوڊ;
    ثابت ct_معلومات *stree = desc->stat_desc->static_وڻ;
    ثابت intf *extra    = desc->stat_desc->extra_بٽ;
    صحيح بنياد_2             = desc->stat_desc->extra_بنياد;
    صحيح max_ڊيگهائي       = desc->stat_desc->max_ڊيگهائي;
    صحيح h;              /* heap index */
    صحيح n, m;           /* iterate over the tree elements */
    صحيح بٽ;           /* bit length */
    صحيح xbits;          /* extra bits */
    ush f;              /* frequency */
    صحيح حد_کان_مٿي = 0;   /* number of elements with bit length too large */

    لكل (بٽ = 0; بٽ <= MAX_بٽ; بٽ++) s->bl_ڳڻپ[بٽ] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    وڻ[s->ڍير[s->ڍير_max]].Len = 0; /* root of the heap */

    لكل (h = s->ڍير_max + 1; h < ڍير_سائيز; h++) {
        n = s->ڍير[h];
        بٽ = وڻ[وڻ[n].Dad].Len + 1;
        إذا (بٽ > max_ڊيگهائي) بٽ = max_ڊيگهائي, حد_کان_مٿي++;
        وڻ[n].Len = (ush)بٽ;
        /* We overwrite tree[n].Dad which is no longer needed */

        إذا (n > max_ڪوڊ) استمر; /* not a leaf node */

        s->bl_ڳڻپ[بٽ]++;
        xbits = 0;
        إذا (n >= بنياد_2) xbits = extra[n - بنياد_2];
        f = وڻ[n].Freq;
        s->opt_len += (ulg)f * (بنانشان)(بٽ + xbits);
        إذا (stree) s->static_len += (ulg)f * (بنانشان)(stree[n].Len + xbits);
    }
    إذا (حد_کان_مٿي == 0) أرجع;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    افعل {
        بٽ = max_ڊيگهائي - 1;
        طالما (s->bl_ڳڻپ[بٽ] == 0) بٽ--;
        s->bl_ڳڻپ[بٽ]--;        /* move one leaf down the tree */
        s->bl_ڳڻپ[بٽ + 1] += 2; /* move one overflow item as its brother */
        s->bl_ڳڻپ[max_ڊيگهائي]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        حد_کان_مٿي -= 2;
    } طالما (حد_کان_مٿي > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    لكل (بٽ = max_ڊيگهائي; بٽ != 0; بٽ--) {
        n = s->bl_ڳڻپ[بٽ];
        طالما (n != 0) {
            m = s->ڍير[--h];
            إذا (m > max_ڪوڊ) استمر;
            إذا ((بنانشان) وڻ[m].Len != (بنانشان) بٽ) {
                Tracev((stderr,"code %d bits %d->%d\n", m, وڻ[m].Len, بٽ));
                s->opt_len += ((ulg)بٽ - وڻ[m].Len) * وڻ[m].Freq;
                وڻ[m].Len = (ush)بٽ;
            }
            n--;
        }
    }
}

#جيڪڏهن_وصف_ٿيل DUMP_BL_وڻ
#  include <stdio.h>
#جيڪڏهن_پورو

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local فراغ build_وڻ(deflate_state *s, وڻ_desc *desc) {
    ct_معلومات *وڻ         = desc->dyn_وڻ;
    ثابت ct_معلومات *stree  = desc->stat_desc->static_وڻ;
    صحيح elems             = desc->stat_desc->elems;
    صحيح n, m;          /* iterate over heap elements */
    صحيح max_ڪوڊ = -1; /* largest code with non zero frequency */
    صحيح ڳنڍ;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->ڍير_len = 0, s->ڍير_max = ڍير_سائيز;

    لكل (n = 0; n < elems; n++) {
        إذا (وڻ[n].Freq != 0) {
            s->ڍير[++(s->ڍير_len)] = max_ڪوڊ = n;
            s->اونهائي[n] = 0;
        } وإلا {
            وڻ[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    طالما (s->ڍير_len < 2) {
        ڳنڍ = s->ڍير[++(s->ڍير_len)] = (max_ڪوڊ < 2 ? ++max_ڪوڊ : 0);
        وڻ[ڳنڍ].Freq = 1;
        s->اونهائي[ڳنڍ] = 0;
        s->opt_len--; إذا (stree) s->static_len -= stree[ڳنڍ].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_ڪوڊ = max_ڪوڊ;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    لكل (n = s->ڍير_len/2; n >= 1; n--) pqdownheap(s, وڻ, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    ڳنڍ = elems;              /* next internal node of the tree */
    افعل {
        pqremove(s, وڻ, n);  /* n = node of least frequency */
        m = s->ڍير[سڀ_کان_ننڍو]; /* m = node of next least frequency */

        s->ڍير[--(s->ڍير_max)] = n; /* keep the nodes sorted by frequency */
        s->ڍير[--(s->ڍير_max)] = m;

        /* Create a new node father of n and m */
        وڻ[ڳنڍ].Freq = وڻ[n].Freq + وڻ[m].Freq;
        s->اونهائي[ڳنڍ] = (uch)((s->اونهائي[n] >= s->اونهائي[m] ?
                                s->اونهائي[n] : s->اونهائي[m]) + 1);
        وڻ[n].Dad = وڻ[m].Dad = (ush)ڳنڍ;
#جيڪڏهن_وصف_ٿيل DUMP_BL_وڻ
        إذا (وڻ == s->bl_وڻ) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    ڳنڍ, وڻ[ڳنڍ].Freq, n, وڻ[n].Freq, m, وڻ[m].Freq);
        }
#جيڪڏهن_پورو
        /* and insert the new node in the heap */
        s->ڍير[سڀ_کان_ننڍو] = ڳنڍ++;
        pqdownheap(s, وڻ, سڀ_کان_ننڍو);

    } طالما (s->ڍير_len >= 2);

    s->ڍير[--(s->ڍير_max)] = s->ڍير[سڀ_کان_ننڍو];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (وڻ_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_معلومات *)وڻ, max_ڪوڊ, s->bl_ڳڻپ);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local فراغ جاچ_وڻ(deflate_state *s, ct_معلومات *وڻ, صحيح max_ڪوڊ) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = وڻ[0].Len; /* length of next code */
    صحيح ڳڻپ = 0;             /* repeat count of the current code */
    صحيح max_ڳڻپ = 7;         /* max repeat count */
    صحيح min_ڳڻپ = 4;         /* min repeat count */

    إذا (nextlen == 0) max_ڳڻپ = 138, min_ڳڻپ = 3;
    وڻ[max_ڪوڊ + 1].Len = (ush)0xffff; /* guard */

    لكل (n = 0; n <= max_ڪوڊ; n++) {
        curlen = nextlen; nextlen = وڻ[n + 1].Len;
        إذا (++ڳڻپ < max_ڳڻپ && curlen == nextlen) {
            استمر;
        } وإلا إذا (ڳڻپ < min_ڳڻپ) {
            s->bl_وڻ[curlen].Freq += (ush)ڳڻپ;
        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) s->bl_وڻ[curlen].Freq++;
            s->bl_وڻ[REP_3_6].Freq++;
        } وإلا إذا (ڳڻپ <= 10) {
            s->bl_وڻ[REPZ_3_10].Freq++;
        } وإلا {
            s->bl_وڻ[REPZ_11_138].Freq++;
        }
        ڳڻپ = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_ڳڻپ = 138, min_ڳڻپ = 3;
        } وإلا إذا (curlen == nextlen) {
            max_ڳڻپ = 6, min_ڳڻپ = 3;
        } وإلا {
            max_ڳڻپ = 7, min_ڳڻپ = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local فراغ send_وڻ(deflate_state *s, ct_معلومات *وڻ, صحيح max_ڪوڊ) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = وڻ[0].Len; /* length of next code */
    صحيح ڳڻپ = 0;             /* repeat count of the current code */
    صحيح max_ڳڻپ = 7;         /* max repeat count */
    صحيح min_ڳڻپ = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    إذا (nextlen == 0) max_ڳڻپ = 138, min_ڳڻپ = 3;

    لكل (n = 0; n <= max_ڪوڊ; n++) {
        curlen = nextlen; nextlen = وڻ[n + 1].Len;
        إذا (++ڳڻپ < max_ڳڻپ && curlen == nextlen) {
            استمر;
        } وإلا إذا (ڳڻپ < min_ڳڻپ) {
            افعل { send_ڪوڊ(s, curlen, s->bl_وڻ); } طالما (--ڳڻپ != 0);

        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) {
                send_ڪوڊ(s, curlen, s->bl_وڻ); ڳڻپ--;
            }
            Assert(ڳڻپ >= 3 && ڳڻپ <= 6, " 3_6?");
            send_ڪوڊ(s, REP_3_6, s->bl_وڻ); send_بٽ(s, ڳڻپ - 3, 2);

        } وإلا إذا (ڳڻپ <= 10) {
            send_ڪوڊ(s, REPZ_3_10, s->bl_وڻ); send_بٽ(s, ڳڻپ - 3, 3);

        } وإلا {
            send_ڪوڊ(s, REPZ_11_138, s->bl_وڻ); send_بٽ(s, ڳڻپ - 11, 7);
        }
        ڳڻپ = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_ڳڻپ = 138, min_ڳڻپ = 3;
        } وإلا إذا (curlen == nextlen) {
            max_ڳڻپ = 6, min_ڳڻپ = 3;
        } وإلا {
            max_ڳڻپ = 7, min_ڳڻپ = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local صحيح build_bl_وڻ(deflate_state *s) {
    صحيح max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    جاچ_وڻ(s, (ct_معلومات *)s->dyn_ltree, s->l_desc.max_ڪوڊ);
    جاچ_وڻ(s, (ct_معلومات *)s->dyn_dtree, s->d_desc.max_ڪوڊ);

    /* Build the bit length tree: */
    build_وڻ(s, (وڻ_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    لكل (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        إذا (s->bl_وڻ[bl_ترتيب[max_blindex]].Len != 0) اكسر;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((stderr, "\ndyn trees: dyn %lu, stat %lu",
            s->opt_len, s->static_len));

    أرجع max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local فراغ send_all_trees(deflate_state *s, صحيح lcodes, صحيح dcodes,
                          صحيح blcodes) {
    صحيح rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_بٽ(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_بٽ(s, dcodes - 1,   5);
    send_بٽ(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    لكل (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_ترتيب[rank]));
        send_بٽ(s, s->bl_وڻ[bl_ترتيب[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->بٽ_موڪليل));

    send_وڻ(s, (ct_معلومات *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->بٽ_موڪليل));

    send_وڻ(s, (ct_معلومات *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->بٽ_موڪليل));
}

/* ===========================================================================
 * Send a stored block
 */
فراغ ZLIB_اندروني tr_stored_بلاڪ(deflate_state *s, charf *buf,
                                    ulg stored_len, صحيح آخري) {
    send_بٽ(s, (STORED_بلاڪ<<1) + آخري, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    إذا (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->بٽ_موڪليل += 2*16;
    s->بٽ_موڪليل += stored_len << 3;
#جيڪڏهن_پورو
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
فراغ ZLIB_اندروني tr_flush_بٽ(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
فراغ ZLIB_اندروني _tr_align(deflate_state *s) {
    send_بٽ(s, STATIC_TREES<<1, 3);
    send_ڪوڊ(s, پڇاڙي_بلاڪ, static_ltree);
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#جيڪڏهن_پورو
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local فراغ compress_بلاڪ(deflate_state *s, ثابت ct_معلومات *ltree,
                          ثابت ct_معلومات *dtree) {
    بنانشان dist;      /* distance of matched string */
    صحيح lc;             /* match length or unmatched char (if dist == 0) */
    بنانشان sx = 0;    /* running index in symbol buffers */
    بنانشان ڪوڊ;      /* the code to send */
    صحيح extra;          /* number of extra bits to send */

    إذا (s->sym_ايندڙ != 0) افعل {
#جيڪڏهن_وصف_ٿيل LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#وإلا
        dist = s->sym_buf[sx++] & 0xff;
        dist += (بنانشان)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#جيڪڏهن_پورو
        إذا (dist == 0) {
            send_ڪوڊ(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } وإلا {
            /* Here, lc is the match length - MIN_MATCH */
            ڪوڊ = ڊيگهائي_ڪوڊ[lc];
            send_ڪوڊ(s, ڪوڊ + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[ڪوڊ];
            إذا (extra != 0) {
                lc -= بنياد_ڊيگهائي[ڪوڊ];
                send_بٽ(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            ڪوڊ = d_ڪوڊ(dist);
            Assert (ڪوڊ < D_CODES, "bad d_code");

            send_ڪوڊ(s, ڪوڊ, dtree);       /* send the distance code */
            extra = extra_dbits[ڪوڊ];
            إذا (extra != 0) {
                dist -= (بنانشان)بنياد_dist[ڪوڊ];
                send_بٽ(s, (صحيح)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#جيڪڏهن_وصف_ٿيل LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#وإلا
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#جيڪڏهن_پورو

    } طالما (sx < s->sym_ايندڙ);

    send_ڪوڊ(s, پڇاڙي_بلاڪ, ltree);
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
local صحيح detect_معلومات_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    بنانشان ڊگهو بلاڪ_نقاب = 0xf3ffc07fUL;
    صحيح n;

    /* Check for non-textual ("block-listed") bytes. */
    لكل (n = 0; n <= 31; n++, بلاڪ_نقاب >>= 1)
        إذا ((بلاڪ_نقاب & 1) && (s->dyn_ltree[n].Freq != 0))
            أرجع Z_ٻٽو;

    /* Check for textual ("allow-listed") bytes. */
    إذا (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        أرجع Z_متن;
    لكل (n = 32; n < LITERALS; n++)
        إذا (s->dyn_ltree[n].Freq != 0)
            أرجع Z_متن;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    أرجع Z_ٻٽو;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
فراغ ZLIB_اندروني tr_flush_بلاڪ(deflate_state *s, charf *buf,
                                   ulg stored_len, صحيح آخري) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    صحيح max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    إذا (s->level > 0) {

        /* Check if the file is binary or text */
        إذا (s->strm->معلومات_type == Z_UNKNOWN)
            s->strm->معلومات_type = detect_معلومات_type(s);

        /* Construct the literal and distance trees */
        build_وڻ(s, (وڻ_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_وڻ(s, (وڻ_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_وڻ(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_ايندڙ / 3));

#جيڪڏهن_وصف_نه_ٿيل FORCE_STATIC
        إذا (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#جيڪڏهن_پورو
            opt_lenb = static_lenb;

    } وإلا {
        Assert(buf != (حرف*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#جيڪڏهن_وصف_ٿيل FORCE_STORED
    إذا (buf != (حرف*)0) { /* force stored block */
#وإلا
    إذا (stored_len + 4 <= opt_lenb && buf != (حرف*)0) {
                       /* 4: two words for the lengths */
#جيڪڏهن_پورو
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_بلاڪ(s, buf, stored_len, آخري);

    } وإلا إذا (static_lenb == opt_lenb) {
        send_بٽ(s, (STATIC_TREES<<1) + آخري, 3);
        compress_بلاڪ(s, (ثابت ct_معلومات *)static_ltree,
                       (ثابت ct_معلومات *)static_dtree);
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#جيڪڏهن_پورو
    } وإلا {
        send_بٽ(s, (DYN_TREES<<1) + آخري, 3);
        send_all_trees(s, s->l_desc.max_ڪوڊ + 1, s->d_desc.max_ڪوڊ + 1,
                       max_blindex + 1);
        compress_بلاڪ(s, (ثابت ct_معلومات *)s->dyn_ltree,
                       (ثابت ct_معلومات *)s->dyn_dtree);
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#جيڪڏهن_پورو
    }
    Assert (s->compressed_len == s->بٽ_موڪليل, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_بلاڪ(s);

    إذا (آخري) {
        bi_windup(s);
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#جيڪڏهن_پورو
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)آخري));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
صحيح ZLIB_اندروني _tr_tally(deflate_state *s, بنانشان dist, بنانشان lc) {
#جيڪڏهن_وصف_ٿيل LIT_MEM
    s->d_buf[s->sym_ايندڙ] = (ush)dist;
    s->l_buf[s->sym_ايندڙ++] = (uch)lc;
#وإلا
    s->sym_buf[s->sym_ايندڙ++] = (uch)dist;
    s->sym_buf[s->sym_ايندڙ++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_ايندڙ++] = (uch)lc;
#جيڪڏهن_پورو
    إذا (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } وإلا {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_ميلاپ-MIN_ميلاپ) &&
               (ush)d_ڪوڊ(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[ڊيگهائي_ڪوڊ[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_ڪوڊ(dist)].Freq++;
    }
    أرجع (s->sym_ايندڙ == s->sym_پڇاڙي);
}
