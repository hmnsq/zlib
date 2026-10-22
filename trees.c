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

#جے_تعریف_شدہ ZLIB_DEBUG
#  include <ctype.h>
#جے_انت

/* ===========================================================================
 * Constants
 */

#تعریف_کرو MAX_BL_بٹ 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#تعریف_کرو سرا_بلاک 256
/* end of block literal code */

#تعریف_کرو REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#تعریف_کرو REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#تعریف_کرو REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ثابت صحيح extra_lbits[لمائی_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local ثابت صحيح extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local ثابت صحيح extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local ثابت uch bl_ترتیب[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#تعریف_کرو DIST_کوڈ_LEN  512 /* see definition of array dist_code below */

#إذا defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_انکڑے static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_انکڑے static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch dist_کوڈ[DIST_کوڈ_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch لمائی_کوڈ[MAX_میل-MIN_میل+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local صحيح ادھار_لمائی[لمائی_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local صحيح ادھار_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#وإلا
#  include "trees.h"
#جے_انت /* defined(GEN_TREES_H) || !defined(STDC) */

بنية static_درخت_desc_s {
    ثابت ct_انکڑے *static_درخت;  /* static tree or NULL */
    ثابت intf *extra_بٹ;      /* extra bits for each code or NULL */
    صحيح     extra_ادھار;          /* base index for extra_bits */
    صحيح     elems;               /* max number of elements in the tree */
    صحيح     max_لمائی;          /* max bit length for the codes */
};

#جے_تعریف_شدہ NO_INIT_GLOBAL_سوچکاں
#  define TCONST
#وإلا
#  define TCONST ثابت
#جے_انت

local TCONST static_درخت_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_بٹ};

local TCONST static_درخت_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_بٹ};

local TCONST static_درخت_desc static_bl_desc =
{(ثابت ct_انکڑے *)0, extra_blbits, 0,   BL_CODES, MAX_BL_بٹ};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#تعریف_کرو put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local بنانشانی bi_الٹ(بنانشانی کوڈ, صحيح len) {
    بنانشانی res = 0;
    افعل {
        res |= کوڈ & 1;
        کوڈ >>= 1, res <<= 1;
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
    s->bi_ورتیا = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#جے_تعریف_شدہ ZLIB_DEBUG
    s->بٹ_گھلیا = (s->بٹ_گھلیا + 7) & ~(ulg)7;
#جے_انت
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local فراغ gen_codes(ct_انکڑے *درخت, صحيح max_کوڈ, ushf *bl_گنتی) {
    ush اگلا_کوڈ[MAX_بٹ+1]; /* next code value for each bit length */
    بنانشانی کوڈ = 0;         /* running code value */
    صحيح بٹ;                  /* bit index */
    صحيح n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    لكل (بٹ = 1; بٹ <= MAX_بٹ; بٹ++) {
        کوڈ = (کوڈ + bl_گنتی[بٹ - 1]) << 1;
        اگلا_کوڈ[بٹ] = (ush)کوڈ;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (کوڈ + bl_گنتی[MAX_بٹ] - 1 == (1 << MAX_بٹ) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_کوڈ));

    لكل (n = 0;  n <= max_کوڈ; n++) {
        صحيح len = درخت[n].Len;
        إذا (len == 0) استمر;
        /* Now reverse the bits */
        درخت[n].کوڈ_2 = (ush)bi_الٹ(اگلا_کوڈ[len]++, len);

        Tracecv(درخت != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, درخت[n].کوڈ_2, اگلا_کوڈ[len] - 1));
    }
}

#جے_تعریف_شدہ GEN_TREES_H
local فراغ gen_trees_سرخی(فراغ);
#جے_انت

#جے_بنا_تعریف ZLIB_DEBUG
#  define send_کوڈ(s, c, درخت) send_بٹ(s, درخت[c].کوڈ_2, درخت[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#وإلا /* !ZLIB_DEBUG */
#  define send_کوڈ(s, c, درخت) \
     { إذا (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_بٹ(s, درخت[c].کوڈ_2, درخت[c].Len); }
#جے_انت

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#جے_تعریف_شدہ ZLIB_DEBUG
local فراغ send_بٹ(deflate_state *s, صحيح قدر, صحيح لمائی) {
    Tracevv((stderr," l %2d v %4x ", لمائی, قدر));
    Assert(لمائی > 0 && لمائی <= 15, "invalid length");
    s->بٹ_گھلیا += (ulg)لمائی;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    إذا (s->bi_valid > (صحيح)Buf_ناپ - لمائی) {
        s->bi_buf |= (ush)قدر << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)قدر >> (Buf_ناپ - s->bi_valid);
        s->bi_valid += لمائی - Buf_ناپ;
    } وإلا {
        s->bi_buf |= (ush)قدر << s->bi_valid;
        s->bi_valid += لمائی;
    }
}
#وإلا /* !ZLIB_DEBUG */

#تعریف_کرو send_بٹ(s, قدر, لمائی) \
{ صحيح len = لمائی;\
  إذا (s->bi_valid > (صحيح)Buf_ناپ - len) {\
    صحيح val = (صحيح)قدر;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_ناپ - s->bi_valid);\
    s->bi_valid += len - Buf_ناپ;\
  } وإلا {\
    s->bi_buf |= (ush)(قدر) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#جے_انت /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local فراغ tr_static_init(فراغ) {
#إذا defined(GEN_TREES_H) || !defined(STDC)
    ساكن صحيح static_init_پورا = 0;
    صحيح n;        /* iterates over tree elements */
    صحيح بٹ;     /* bit counter */
    صحيح لمائی;   /* length value */
    صحيح کوڈ;     /* code value */
    صحيح dist;     /* distance index */
    ush bl_گنتی[MAX_بٹ+1];
    /* number of codes at each bit length for an optimal tree */

    إذا (static_init_پورا) أرجع;

    /* For some embedded targets, global variables are not initialized: */
#جے_تعریف_شدہ NO_INIT_GLOBAL_سوچکاں
    static_l_desc.static_درخت = static_ltree;
    static_l_desc.extra_بٹ = extra_lbits;
    static_d_desc.static_درخت = static_dtree;
    static_d_desc.extra_بٹ = extra_dbits;
    static_bl_desc.extra_بٹ = extra_blbits;
#جے_انت

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    لمائی = 0;
    لكل (کوڈ = 0; کوڈ < لمائی_CODES-1; کوڈ++) {
        ادھار_لمائی[کوڈ] = لمائی;
        لكل (n = 0; n < (1 << extra_lbits[کوڈ]); n++) {
            لمائی_کوڈ[لمائی++] = (uch)کوڈ;
        }
    }
    Assert (لمائی == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    لمائی_کوڈ[لمائی - 1] = (uch)کوڈ;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    لكل (کوڈ = 0 ; کوڈ < 16; کوڈ++) {
        ادھار_dist[کوڈ] = dist;
        لكل (n = 0; n < (1 << extra_dbits[کوڈ]); n++) {
            dist_کوڈ[dist++] = (uch)کوڈ;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    لكل ( ; کوڈ < D_CODES; کوڈ++) {
        ادھار_dist[کوڈ] = dist << 7;
        لكل (n = 0; n < (1 << (extra_dbits[کوڈ] - 7)); n++) {
            dist_کوڈ[256 + dist++] = (uch)کوڈ;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    لكل (بٹ = 0; بٹ <= MAX_بٹ; بٹ++) bl_گنتی[بٹ] = 0;
    n = 0;
    طالما (n <= 143) static_ltree[n++].Len = 8, bl_گنتی[8]++;
    طالما (n <= 255) static_ltree[n++].Len = 9, bl_گنتی[9]++;
    طالما (n <= 279) static_ltree[n++].Len = 7, bl_گنتی[7]++;
    طالما (n <= 287) static_ltree[n++].Len = 8, bl_گنتی[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_انکڑے *)static_ltree, L_CODES+1, bl_گنتی);

    /* The static distance tree is trivial: */
    لكل (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].کوڈ_2 = bi_الٹ((بنانشانی)n, 5);
    }
    static_init_پورا = 1;

#  ifdef GEN_TREES_H
    gen_trees_سرخی();
#  endif
#جے_انت /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#جے_تعریف_شدہ GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, آخری, چوڑائی) \
      ((i) == (آخری)? "\n};\n\n" :    \
       ((i) % (چوڑائی) == (چوڑائی) - 1 ? ",\n" : ", "))

فراغ gen_trees_سرخی(فراغ) {
    FILE *سرخی = fopen("trees.h", "w");
    صحيح i;

    Assert (سرخی != عدم, "Can't open trees.h");
    fprintf(سرخی,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(سرخی, "local const ct_data static_ltree[L_CODES+2] = {\n");
    لكل (i = 0; i < L_CODES+2; i++) {
        fprintf(سرخی, "{{%3u},{%3u}}%s", static_ltree[i].کوڈ_2,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(سرخی, "local const ct_data static_dtree[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرخی, "{{%2u},{%2u}}%s", static_dtree[i].کوڈ_2,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(سرخی, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    لكل (i = 0; i < DIST_کوڈ_LEN; i++) {
        fprintf(سرخی, "%2u%s", dist_کوڈ[i],
                SEPARATOR(i, DIST_کوڈ_LEN-1, 20));
    }

    fprintf(سرخی,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    لكل (i = 0; i < MAX_میل-MIN_میل+1; i++) {
        fprintf(سرخی, "%2u%s", لمائی_کوڈ[i],
                SEPARATOR(i, MAX_میل-MIN_میل, 20));
    }

    fprintf(سرخی, "local const int base_length[LENGTH_CODES] = {\n");
    لكل (i = 0; i < لمائی_CODES; i++) {
        fprintf(سرخی, "%1u%s", ادھار_لمائی[i],
                SEPARATOR(i, لمائی_CODES-1, 20));
    }

    fprintf(سرخی, "local const int base_dist[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرخی, "%5u%s", ادھار_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(سرخی);
}
#جے_انت /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local فراغ init_بلاک(deflate_state *s) {
    صحيح n; /* iterates over tree elements */

    /* Initialize the trees. */
    لكل (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    لكل (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    لكل (n = 0; n < BL_CODES; n++) s->bl_درخت[n].Freq = 0;

    s->dyn_ltree[سرا_بلاک].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_اگلا = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
فراغ ZLIB_اندرونی _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_درخت = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_درخت = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_درخت = s->bl_درخت;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_ورتیا = 0;
#جے_تعریف_شدہ ZLIB_DEBUG
    s->compressed_len = 0L;
    s->بٹ_گھلیا = 0L;
#جے_انت

    /* Initialize the first block of the first file: */
    init_بلاک(s);
}

#تعریف_کرو سبھ_توں_چھوٹا 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#تعریف_کرو pqremove(s, درخت, چوٹی) \
{\
    چوٹی = s->ڈھیر[سبھ_توں_چھوٹا]; \
    s->ڈھیر[سبھ_توں_چھوٹا] = s->ڈھیر[s->ڈھیر_len--]; \
    pqdownheap(s, درخت, سبھ_توں_چھوٹا); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#تعریف_کرو smaller(درخت, n, m, ڈونگھائی) \
   (درخت[n].Freq < درخت[m].Freq || \
   (درخت[n].Freq == درخت[m].Freq && ڈونگھائی[n] <= ڈونگھائی[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local فراغ pqdownheap(deflate_state *s, ct_انکڑے *درخت, صحيح k) {
    صحيح v = s->ڈھیر[k];
    صحيح j = k << 1;  /* left son of k */
    طالما (j <= s->ڈھیر_len) {
        /* Set j to the smallest of the two sons: */
        إذا (j < s->ڈھیر_len &&
            smaller(درخت, s->ڈھیر[j + 1], s->ڈھیر[j], s->ڈونگھائی)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        إذا (smaller(درخت, v, s->ڈھیر[j], s->ڈونگھائی)) اكسر;

        /* Exchange v with the smallest son */
        s->ڈھیر[k] = s->ڈھیر[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->ڈھیر[k] = v;
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
local فراغ gen_bitlen(deflate_state *s, درخت_desc *desc) {
    ct_انکڑے *درخت        = desc->dyn_درخت;
    صحيح max_کوڈ         = desc->max_کوڈ;
    ثابت ct_انکڑے *stree = desc->stat_desc->static_درخت;
    ثابت intf *extra    = desc->stat_desc->extra_بٹ;
    صحيح ادھار_2             = desc->stat_desc->extra_ادھار;
    صحيح max_لمائی       = desc->stat_desc->max_لمائی;
    صحيح h;              /* heap index */
    صحيح n, m;           /* iterate over the tree elements */
    صحيح بٹ;           /* bit length */
    صحيح xbits;          /* extra bits */
    ush f;              /* frequency */
    صحيح حد_توں_ودھ = 0;   /* number of elements with bit length too large */

    لكل (بٹ = 0; بٹ <= MAX_بٹ; بٹ++) s->bl_گنتی[بٹ] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    درخت[s->ڈھیر[s->ڈھیر_max]].Len = 0; /* root of the heap */

    لكل (h = s->ڈھیر_max + 1; h < ڈھیر_ناپ; h++) {
        n = s->ڈھیر[h];
        بٹ = درخت[درخت[n].Dad].Len + 1;
        إذا (بٹ > max_لمائی) بٹ = max_لمائی, حد_توں_ودھ++;
        درخت[n].Len = (ush)بٹ;
        /* We overwrite tree[n].Dad which is no longer needed */

        إذا (n > max_کوڈ) استمر; /* not a leaf node */

        s->bl_گنتی[بٹ]++;
        xbits = 0;
        إذا (n >= ادھار_2) xbits = extra[n - ادھار_2];
        f = درخت[n].Freq;
        s->opt_len += (ulg)f * (بنانشانی)(بٹ + xbits);
        إذا (stree) s->static_len += (ulg)f * (بنانشانی)(stree[n].Len + xbits);
    }
    إذا (حد_توں_ودھ == 0) أرجع;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    افعل {
        بٹ = max_لمائی - 1;
        طالما (s->bl_گنتی[بٹ] == 0) بٹ--;
        s->bl_گنتی[بٹ]--;        /* move one leaf down the tree */
        s->bl_گنتی[بٹ + 1] += 2; /* move one overflow item as its brother */
        s->bl_گنتی[max_لمائی]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        حد_توں_ودھ -= 2;
    } طالما (حد_توں_ودھ > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    لكل (بٹ = max_لمائی; بٹ != 0; بٹ--) {
        n = s->bl_گنتی[بٹ];
        طالما (n != 0) {
            m = s->ڈھیر[--h];
            إذا (m > max_کوڈ) استمر;
            إذا ((بنانشانی) درخت[m].Len != (بنانشانی) بٹ) {
                Tracev((stderr,"code %d bits %d->%d\n", m, درخت[m].Len, بٹ));
                s->opt_len += ((ulg)بٹ - درخت[m].Len) * درخت[m].Freq;
                درخت[m].Len = (ush)بٹ;
            }
            n--;
        }
    }
}

#جے_تعریف_شدہ DUMP_BL_درخت
#  include <stdio.h>
#جے_انت

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local فراغ build_درخت(deflate_state *s, درخت_desc *desc) {
    ct_انکڑے *درخت         = desc->dyn_درخت;
    ثابت ct_انکڑے *stree  = desc->stat_desc->static_درخت;
    صحيح elems             = desc->stat_desc->elems;
    صحيح n, m;          /* iterate over heap elements */
    صحيح max_کوڈ = -1; /* largest code with non zero frequency */
    صحيح گرہ;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->ڈھیر_len = 0, s->ڈھیر_max = ڈھیر_ناپ;

    لكل (n = 0; n < elems; n++) {
        إذا (درخت[n].Freq != 0) {
            s->ڈھیر[++(s->ڈھیر_len)] = max_کوڈ = n;
            s->ڈونگھائی[n] = 0;
        } وإلا {
            درخت[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    طالما (s->ڈھیر_len < 2) {
        گرہ = s->ڈھیر[++(s->ڈھیر_len)] = (max_کوڈ < 2 ? ++max_کوڈ : 0);
        درخت[گرہ].Freq = 1;
        s->ڈونگھائی[گرہ] = 0;
        s->opt_len--; إذا (stree) s->static_len -= stree[گرہ].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_کوڈ = max_کوڈ;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    لكل (n = s->ڈھیر_len/2; n >= 1; n--) pqdownheap(s, درخت, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    گرہ = elems;              /* next internal node of the tree */
    افعل {
        pqremove(s, درخت, n);  /* n = node of least frequency */
        m = s->ڈھیر[سبھ_توں_چھوٹا]; /* m = node of next least frequency */

        s->ڈھیر[--(s->ڈھیر_max)] = n; /* keep the nodes sorted by frequency */
        s->ڈھیر[--(s->ڈھیر_max)] = m;

        /* Create a new node father of n and m */
        درخت[گرہ].Freq = درخت[n].Freq + درخت[m].Freq;
        s->ڈونگھائی[گرہ] = (uch)((s->ڈونگھائی[n] >= s->ڈونگھائی[m] ?
                                s->ڈونگھائی[n] : s->ڈونگھائی[m]) + 1);
        درخت[n].Dad = درخت[m].Dad = (ush)گرہ;
#جے_تعریف_شدہ DUMP_BL_درخت
        إذا (درخت == s->bl_درخت) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    گرہ, درخت[گرہ].Freq, n, درخت[n].Freq, m, درخت[m].Freq);
        }
#جے_انت
        /* and insert the new node in the heap */
        s->ڈھیر[سبھ_توں_چھوٹا] = گرہ++;
        pqdownheap(s, درخت, سبھ_توں_چھوٹا);

    } طالما (s->ڈھیر_len >= 2);

    s->ڈھیر[--(s->ڈھیر_max)] = s->ڈھیر[سبھ_توں_چھوٹا];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (درخت_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_انکڑے *)درخت, max_کوڈ, s->bl_گنتی);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local فراغ جانچ_درخت(deflate_state *s, ct_انکڑے *درخت, صحيح max_کوڈ) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = درخت[0].Len; /* length of next code */
    صحيح گنتی = 0;             /* repeat count of the current code */
    صحيح max_گنتی = 7;         /* max repeat count */
    صحيح min_گنتی = 4;         /* min repeat count */

    إذا (nextlen == 0) max_گنتی = 138, min_گنتی = 3;
    درخت[max_کوڈ + 1].Len = (ush)0xffff; /* guard */

    لكل (n = 0; n <= max_کوڈ; n++) {
        curlen = nextlen; nextlen = درخت[n + 1].Len;
        إذا (++گنتی < max_گنتی && curlen == nextlen) {
            استمر;
        } وإلا إذا (گنتی < min_گنتی) {
            s->bl_درخت[curlen].Freq += (ush)گنتی;
        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) s->bl_درخت[curlen].Freq++;
            s->bl_درخت[REP_3_6].Freq++;
        } وإلا إذا (گنتی <= 10) {
            s->bl_درخت[REPZ_3_10].Freq++;
        } وإلا {
            s->bl_درخت[REPZ_11_138].Freq++;
        }
        گنتی = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_گنتی = 138, min_گنتی = 3;
        } وإلا إذا (curlen == nextlen) {
            max_گنتی = 6, min_گنتی = 3;
        } وإلا {
            max_گنتی = 7, min_گنتی = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local فراغ send_درخت(deflate_state *s, ct_انکڑے *درخت, صحيح max_کوڈ) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = درخت[0].Len; /* length of next code */
    صحيح گنتی = 0;             /* repeat count of the current code */
    صحيح max_گنتی = 7;         /* max repeat count */
    صحيح min_گنتی = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    إذا (nextlen == 0) max_گنتی = 138, min_گنتی = 3;

    لكل (n = 0; n <= max_کوڈ; n++) {
        curlen = nextlen; nextlen = درخت[n + 1].Len;
        إذا (++گنتی < max_گنتی && curlen == nextlen) {
            استمر;
        } وإلا إذا (گنتی < min_گنتی) {
            افعل { send_کوڈ(s, curlen, s->bl_درخت); } طالما (--گنتی != 0);

        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) {
                send_کوڈ(s, curlen, s->bl_درخت); گنتی--;
            }
            Assert(گنتی >= 3 && گنتی <= 6, " 3_6?");
            send_کوڈ(s, REP_3_6, s->bl_درخت); send_بٹ(s, گنتی - 3, 2);

        } وإلا إذا (گنتی <= 10) {
            send_کوڈ(s, REPZ_3_10, s->bl_درخت); send_بٹ(s, گنتی - 3, 3);

        } وإلا {
            send_کوڈ(s, REPZ_11_138, s->bl_درخت); send_بٹ(s, گنتی - 11, 7);
        }
        گنتی = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_گنتی = 138, min_گنتی = 3;
        } وإلا إذا (curlen == nextlen) {
            max_گنتی = 6, min_گنتی = 3;
        } وإلا {
            max_گنتی = 7, min_گنتی = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local صحيح build_bl_درخت(deflate_state *s) {
    صحيح max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    جانچ_درخت(s, (ct_انکڑے *)s->dyn_ltree, s->l_desc.max_کوڈ);
    جانچ_درخت(s, (ct_انکڑے *)s->dyn_dtree, s->d_desc.max_کوڈ);

    /* Build the bit length tree: */
    build_درخت(s, (درخت_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    لكل (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        إذا (s->bl_درخت[bl_ترتیب[max_blindex]].Len != 0) اكسر;
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
    send_بٹ(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_بٹ(s, dcodes - 1,   5);
    send_بٹ(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    لكل (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_ترتیب[rank]));
        send_بٹ(s, s->bl_درخت[bl_ترتیب[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->بٹ_گھلیا));

    send_درخت(s, (ct_انکڑے *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->بٹ_گھلیا));

    send_درخت(s, (ct_انکڑے *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->بٹ_گھلیا));
}

/* ===========================================================================
 * Send a stored block
 */
فراغ ZLIB_اندرونی tr_stored_بلاک(deflate_state *s, charf *buf,
                                    ulg stored_len, صحيح آخری) {
    send_بٹ(s, (STORED_بلاک<<1) + آخری, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    إذا (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#جے_تعریف_شدہ ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->بٹ_گھلیا += 2*16;
    s->بٹ_گھلیا += stored_len << 3;
#جے_انت
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
فراغ ZLIB_اندرونی tr_flush_بٹ(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
فراغ ZLIB_اندرونی _tr_align(deflate_state *s) {
    send_بٹ(s, STATIC_TREES<<1, 3);
    send_کوڈ(s, سرا_بلاک, static_ltree);
#جے_تعریف_شدہ ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#جے_انت
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local فراغ compress_بلاک(deflate_state *s, ثابت ct_انکڑے *ltree,
                          ثابت ct_انکڑے *dtree) {
    بنانشانی dist;      /* distance of matched string */
    صحيح lc;             /* match length or unmatched char (if dist == 0) */
    بنانشانی sx = 0;    /* running index in symbol buffers */
    بنانشانی کوڈ;      /* the code to send */
    صحيح extra;          /* number of extra bits to send */

    إذا (s->sym_اگلا != 0) افعل {
#جے_تعریف_شدہ LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#وإلا
        dist = s->sym_buf[sx++] & 0xff;
        dist += (بنانشانی)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#جے_انت
        إذا (dist == 0) {
            send_کوڈ(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } وإلا {
            /* Here, lc is the match length - MIN_MATCH */
            کوڈ = لمائی_کوڈ[lc];
            send_کوڈ(s, کوڈ + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[کوڈ];
            إذا (extra != 0) {
                lc -= ادھار_لمائی[کوڈ];
                send_بٹ(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            کوڈ = d_کوڈ(dist);
            Assert (کوڈ < D_CODES, "bad d_code");

            send_کوڈ(s, کوڈ, dtree);       /* send the distance code */
            extra = extra_dbits[کوڈ];
            إذا (extra != 0) {
                dist -= (بنانشانی)ادھار_dist[کوڈ];
                send_بٹ(s, (صحيح)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#جے_تعریف_شدہ LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#وإلا
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#جے_انت

    } طالما (sx < s->sym_اگلا);

    send_کوڈ(s, سرا_بلاک, ltree);
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
local صحيح detect_انکڑے_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    بنانشانی لما بلاک_نقاب = 0xf3ffc07fUL;
    صحيح n;

    /* Check for non-textual ("block-listed") bytes. */
    لكل (n = 0; n <= 31; n++, بلاک_نقاب >>= 1)
        إذا ((بلاک_نقاب & 1) && (s->dyn_ltree[n].Freq != 0))
            أرجع Z_ثنائی;

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
    أرجع Z_ثنائی;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
فراغ ZLIB_اندرونی tr_flush_بلاک(deflate_state *s, charf *buf,
                                   ulg stored_len, صحيح آخری) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    صحيح max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    إذا (s->level > 0) {

        /* Check if the file is binary or text */
        إذا (s->strm->انکڑے_type == Z_UNKNOWN)
            s->strm->انکڑے_type = detect_انکڑے_type(s);

        /* Construct the literal and distance trees */
        build_درخت(s, (درخت_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_درخت(s, (درخت_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_درخت(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_اگلا / 3));

#جے_بنا_تعریف FORCE_STATIC
        إذا (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#جے_انت
            opt_lenb = static_lenb;

    } وإلا {
        Assert(buf != (حرف*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#جے_تعریف_شدہ FORCE_STORED
    إذا (buf != (حرف*)0) { /* force stored block */
#وإلا
    إذا (stored_len + 4 <= opt_lenb && buf != (حرف*)0) {
                       /* 4: two words for the lengths */
#جے_انت
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_بلاک(s, buf, stored_len, آخری);

    } وإلا إذا (static_lenb == opt_lenb) {
        send_بٹ(s, (STATIC_TREES<<1) + آخری, 3);
        compress_بلاک(s, (ثابت ct_انکڑے *)static_ltree,
                       (ثابت ct_انکڑے *)static_dtree);
#جے_تعریف_شدہ ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#جے_انت
    } وإلا {
        send_بٹ(s, (DYN_TREES<<1) + آخری, 3);
        send_all_trees(s, s->l_desc.max_کوڈ + 1, s->d_desc.max_کوڈ + 1,
                       max_blindex + 1);
        compress_بلاک(s, (ثابت ct_انکڑے *)s->dyn_ltree,
                       (ثابت ct_انکڑے *)s->dyn_dtree);
#جے_تعریف_شدہ ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#جے_انت
    }
    Assert (s->compressed_len == s->بٹ_گھلیا, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_بلاک(s);

    إذا (آخری) {
        bi_windup(s);
#جے_تعریف_شدہ ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#جے_انت
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)آخری));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
صحيح ZLIB_اندرونی _tr_tally(deflate_state *s, بنانشانی dist, بنانشانی lc) {
#جے_تعریف_شدہ LIT_MEM
    s->d_buf[s->sym_اگلا] = (ush)dist;
    s->l_buf[s->sym_اگلا++] = (uch)lc;
#وإلا
    s->sym_buf[s->sym_اگلا++] = (uch)dist;
    s->sym_buf[s->sym_اگلا++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_اگلا++] = (uch)lc;
#جے_انت
    إذا (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } وإلا {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_میل-MIN_میل) &&
               (ush)d_کوڈ(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[لمائی_کوڈ[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_کوڈ(dist)].Freq++;
    }
    أرجع (s->sym_اگلا == s->sym_سرا);
}
