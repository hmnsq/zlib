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

#که_تعریف_شوی ZLIB_DEBUG
#  include <ctype.h>
#د_که_پای

/* ===========================================================================
 * Constants
 */

#تعریف_کړه MAX_BL_بټونه 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#تعریف_کړه پای_ټکی_بلاک 256
/* end of block literal code */

#تعریف_کړه REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#تعریف_کړه REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#تعریف_کړه REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ثابت صحيح extra_lbits[اوږدتیا_CODES] /* extra bits for each length code */
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

#تعریف_کړه DIST_کوډ_LEN  512 /* see definition of array dist_code below */

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

uch dist_کوډ[DIST_کوډ_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch اوږدتیا_کوډ[MAX_سمون-MIN_سمون+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local صحيح بنسټ_اوږدتیا[اوږدتیا_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local صحيح بنسټ_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#وإلا
#  include "trees.h"
#د_که_پای /* defined(GEN_TREES_H) || !defined(STDC) */

بنية static_ونه_desc_s {
    ثابت ct_معلومات *static_ونه;  /* static tree or NULL */
    ثابت intf *extra_بټونه;      /* extra bits for each code or NULL */
    صحيح     extra_بنسټ;          /* base index for extra_bits */
    صحيح     elems;               /* max number of elements in the tree */
    صحيح     max_اوږدتیا;          /* max bit length for the codes */
};

#که_تعریف_شوی NO_INIT_GLOBAL_اشارې
#  define TCONST
#وإلا
#  define TCONST ثابت
#د_که_پای

local TCONST static_ونه_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_بټونه};

local TCONST static_ونه_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_بټونه};

local TCONST static_ونه_desc static_bl_desc =
{(ثابت ct_معلومات *)0, extra_blbits, 0,   BL_CODES, MAX_BL_بټونه};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#تعریف_کړه put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local بینښه bi_سرچپه(بینښه کوډ, صحيح len) {
    بینښه res = 0;
    افعل {
        res |= کوډ & 1;
        کوډ >>= 1, res <<= 1;
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
    s->bi_کارول_شوی = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#که_تعریف_شوی ZLIB_DEBUG
    s->بټونه_لېږل_شوی = (s->بټونه_لېږل_شوی + 7) & ~(ulg)7;
#د_که_پای
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local فراغ gen_codes(ct_معلومات *ونه, صحيح max_کوډ, ushf *bl_شمېر) {
    ush بل_کوډ[MAX_بټونه+1]; /* next code value for each bit length */
    بینښه کوډ = 0;         /* running code value */
    صحيح بټونه;                  /* bit index */
    صحيح n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    لكل (بټونه = 1; بټونه <= MAX_بټونه; بټونه++) {
        کوډ = (کوډ + bl_شمېر[بټونه - 1]) << 1;
        بل_کوډ[بټونه] = (ush)کوډ;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (کوډ + bl_شمېر[MAX_بټونه] - 1 == (1 << MAX_بټونه) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_کوډ));

    لكل (n = 0;  n <= max_کوډ; n++) {
        صحيح len = ونه[n].Len;
        إذا (len == 0) استمر;
        /* Now reverse the bits */
        ونه[n].کوډ_2 = (ush)bi_سرچپه(بل_کوډ[len]++, len);

        Tracecv(ونه != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, ونه[n].کوډ_2, بل_کوډ[len] - 1));
    }
}

#که_تعریف_شوی GEN_TREES_H
local فراغ gen_trees_سرلیک(فراغ);
#د_که_پای

#که_نه_تعریف_شوی ZLIB_DEBUG
#  define send_کوډ(s, c, ونه) send_بټونه(s, ونه[c].کوډ_2, ونه[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#وإلا /* !ZLIB_DEBUG */
#  define send_کوډ(s, c, ونه) \
     { إذا (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_بټونه(s, ونه[c].کوډ_2, ونه[c].Len); }
#د_که_پای

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#که_تعریف_شوی ZLIB_DEBUG
local فراغ send_بټونه(deflate_state *s, صحيح ارزښت, صحيح اوږدتیا) {
    Tracevv((stderr," l %2d v %4x ", اوږدتیا, ارزښت));
    Assert(اوږدتیا > 0 && اوږدتیا <= 15, "invalid length");
    s->بټونه_لېږل_شوی += (ulg)اوږدتیا;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    إذا (s->bi_valid > (صحيح)Buf_اندازه - اوږدتیا) {
        s->bi_buf |= (ush)ارزښت << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)ارزښت >> (Buf_اندازه - s->bi_valid);
        s->bi_valid += اوږدتیا - Buf_اندازه;
    } وإلا {
        s->bi_buf |= (ush)ارزښت << s->bi_valid;
        s->bi_valid += اوږدتیا;
    }
}
#وإلا /* !ZLIB_DEBUG */

#تعریف_کړه send_بټونه(s, ارزښت, اوږدتیا) \
{ صحيح len = اوږدتیا;\
  إذا (s->bi_valid > (صحيح)Buf_اندازه - len) {\
    صحيح val = (صحيح)ارزښت;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_اندازه - s->bi_valid);\
    s->bi_valid += len - Buf_اندازه;\
  } وإلا {\
    s->bi_buf |= (ush)(ارزښت) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#د_که_پای /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local فراغ tr_static_init(فراغ) {
#إذا defined(GEN_TREES_H) || !defined(STDC)
    ساكن صحيح static_init_بشپړ = 0;
    صحيح n;        /* iterates over tree elements */
    صحيح بټونه;     /* bit counter */
    صحيح اوږدتیا;   /* length value */
    صحيح کوډ;     /* code value */
    صحيح dist;     /* distance index */
    ush bl_شمېر[MAX_بټونه+1];
    /* number of codes at each bit length for an optimal tree */

    إذا (static_init_بشپړ) أرجع;

    /* For some embedded targets, global variables are not initialized: */
#که_تعریف_شوی NO_INIT_GLOBAL_اشارې
    static_l_desc.static_ونه = static_ltree;
    static_l_desc.extra_بټونه = extra_lbits;
    static_d_desc.static_ونه = static_dtree;
    static_d_desc.extra_بټونه = extra_dbits;
    static_bl_desc.extra_بټونه = extra_blbits;
#د_که_پای

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    اوږدتیا = 0;
    لكل (کوډ = 0; کوډ < اوږدتیا_CODES-1; کوډ++) {
        بنسټ_اوږدتیا[کوډ] = اوږدتیا;
        لكل (n = 0; n < (1 << extra_lbits[کوډ]); n++) {
            اوږدتیا_کوډ[اوږدتیا++] = (uch)کوډ;
        }
    }
    Assert (اوږدتیا == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    اوږدتیا_کوډ[اوږدتیا - 1] = (uch)کوډ;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    لكل (کوډ = 0 ; کوډ < 16; کوډ++) {
        بنسټ_dist[کوډ] = dist;
        لكل (n = 0; n < (1 << extra_dbits[کوډ]); n++) {
            dist_کوډ[dist++] = (uch)کوډ;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    لكل ( ; کوډ < D_CODES; کوډ++) {
        بنسټ_dist[کوډ] = dist << 7;
        لكل (n = 0; n < (1 << (extra_dbits[کوډ] - 7)); n++) {
            dist_کوډ[256 + dist++] = (uch)کوډ;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    لكل (بټونه = 0; بټونه <= MAX_بټونه; بټونه++) bl_شمېر[بټونه] = 0;
    n = 0;
    طالما (n <= 143) static_ltree[n++].Len = 8, bl_شمېر[8]++;
    طالما (n <= 255) static_ltree[n++].Len = 9, bl_شمېر[9]++;
    طالما (n <= 279) static_ltree[n++].Len = 7, bl_شمېر[7]++;
    طالما (n <= 287) static_ltree[n++].Len = 8, bl_شمېر[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_معلومات *)static_ltree, L_CODES+1, bl_شمېر);

    /* The static distance tree is trivial: */
    لكل (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].کوډ_2 = bi_سرچپه((بینښه)n, 5);
    }
    static_init_بشپړ = 1;

#  ifdef GEN_TREES_H
    gen_trees_سرلیک();
#  endif
#د_که_پای /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#که_تعریف_شوی GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, وروستی, پلنوالی) \
      ((i) == (وروستی)? "\n};\n\n" :    \
       ((i) % (پلنوالی) == (پلنوالی) - 1 ? ",\n" : ", "))

فراغ gen_trees_سرلیک(فراغ) {
    FILE *سرلیک = fopen("trees.h", "w");
    صحيح i;

    Assert (سرلیک != عدم, "Can't open trees.h");
    fprintf(سرلیک,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(سرلیک, "local const ct_data static_ltree[L_CODES+2] = {\n");
    لكل (i = 0; i < L_CODES+2; i++) {
        fprintf(سرلیک, "{{%3u},{%3u}}%s", static_ltree[i].کوډ_2,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(سرلیک, "local const ct_data static_dtree[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرلیک, "{{%2u},{%2u}}%s", static_dtree[i].کوډ_2,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(سرلیک, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    لكل (i = 0; i < DIST_کوډ_LEN; i++) {
        fprintf(سرلیک, "%2u%s", dist_کوډ[i],
                SEPARATOR(i, DIST_کوډ_LEN-1, 20));
    }

    fprintf(سرلیک,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    لكل (i = 0; i < MAX_سمون-MIN_سمون+1; i++) {
        fprintf(سرلیک, "%2u%s", اوږدتیا_کوډ[i],
                SEPARATOR(i, MAX_سمون-MIN_سمون, 20));
    }

    fprintf(سرلیک, "local const int base_length[LENGTH_CODES] = {\n");
    لكل (i = 0; i < اوږدتیا_CODES; i++) {
        fprintf(سرلیک, "%1u%s", بنسټ_اوږدتیا[i],
                SEPARATOR(i, اوږدتیا_CODES-1, 20));
    }

    fprintf(سرلیک, "local const int base_dist[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرلیک, "%5u%s", بنسټ_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(سرلیک);
}
#د_که_پای /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local فراغ init_بلاک(deflate_state *s) {
    صحيح n; /* iterates over tree elements */

    /* Initialize the trees. */
    لكل (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    لكل (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    لكل (n = 0; n < BL_CODES; n++) s->bl_ونه[n].Freq = 0;

    s->dyn_ltree[پای_ټکی_بلاک].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_بل = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
فراغ ZLIB_دننی _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_ونه = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_ونه = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_ونه = s->bl_ونه;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_کارول_شوی = 0;
#که_تعریف_شوی ZLIB_DEBUG
    s->compressed_len = 0L;
    s->بټونه_لېږل_شوی = 0L;
#د_که_پای

    /* Initialize the first block of the first file: */
    init_بلاک(s);
}

#تعریف_کړه تر_ټولو_کوچنی 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#تعریف_کړه pqremove(s, ونه, سر_3) \
{\
    سر_3 = s->ډېرۍ[تر_ټولو_کوچنی]; \
    s->ډېرۍ[تر_ټولو_کوچنی] = s->ډېرۍ[s->ډېرۍ_len--]; \
    pqdownheap(s, ونه, تر_ټولو_کوچنی); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#تعریف_کړه smaller(ونه, n, m, ژوروالی) \
   (ونه[n].Freq < ونه[m].Freq || \
   (ونه[n].Freq == ونه[m].Freq && ژوروالی[n] <= ژوروالی[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local فراغ pqdownheap(deflate_state *s, ct_معلومات *ونه, صحيح k) {
    صحيح v = s->ډېرۍ[k];
    صحيح j = k << 1;  /* left son of k */
    طالما (j <= s->ډېرۍ_len) {
        /* Set j to the smallest of the two sons: */
        إذا (j < s->ډېرۍ_len &&
            smaller(ونه, s->ډېرۍ[j + 1], s->ډېرۍ[j], s->ژوروالی)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        إذا (smaller(ونه, v, s->ډېرۍ[j], s->ژوروالی)) اكسر;

        /* Exchange v with the smallest son */
        s->ډېرۍ[k] = s->ډېرۍ[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->ډېرۍ[k] = v;
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
local فراغ gen_bitlen(deflate_state *s, ونه_desc *desc) {
    ct_معلومات *ونه        = desc->dyn_ونه;
    صحيح max_کوډ         = desc->max_کوډ;
    ثابت ct_معلومات *stree = desc->stat_desc->static_ونه;
    ثابت intf *extra    = desc->stat_desc->extra_بټونه;
    صحيح بنسټ_2             = desc->stat_desc->extra_بنسټ;
    صحيح max_اوږدتیا       = desc->stat_desc->max_اوږدتیا;
    صحيح h;              /* heap index */
    صحيح n, m;           /* iterate over the tree elements */
    صحيح بټونه;           /* bit length */
    صحيح xbits;          /* extra bits */
    ush f;              /* frequency */
    صحيح ډکېدنه = 0;   /* number of elements with bit length too large */

    لكل (بټونه = 0; بټونه <= MAX_بټونه; بټونه++) s->bl_شمېر[بټونه] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    ونه[s->ډېرۍ[s->ډېرۍ_max]].Len = 0; /* root of the heap */

    لكل (h = s->ډېرۍ_max + 1; h < ډېرۍ_اندازه; h++) {
        n = s->ډېرۍ[h];
        بټونه = ونه[ونه[n].Dad].Len + 1;
        إذا (بټونه > max_اوږدتیا) بټونه = max_اوږدتیا, ډکېدنه++;
        ونه[n].Len = (ush)بټونه;
        /* We overwrite tree[n].Dad which is no longer needed */

        إذا (n > max_کوډ) استمر; /* not a leaf node */

        s->bl_شمېر[بټونه]++;
        xbits = 0;
        إذا (n >= بنسټ_2) xbits = extra[n - بنسټ_2];
        f = ونه[n].Freq;
        s->opt_len += (ulg)f * (بینښه)(بټونه + xbits);
        إذا (stree) s->static_len += (ulg)f * (بینښه)(stree[n].Len + xbits);
    }
    إذا (ډکېدنه == 0) أرجع;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    افعل {
        بټونه = max_اوږدتیا - 1;
        طالما (s->bl_شمېر[بټونه] == 0) بټونه--;
        s->bl_شمېر[بټونه]--;        /* move one leaf down the tree */
        s->bl_شمېر[بټونه + 1] += 2; /* move one overflow item as its brother */
        s->bl_شمېر[max_اوږدتیا]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        ډکېدنه -= 2;
    } طالما (ډکېدنه > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    لكل (بټونه = max_اوږدتیا; بټونه != 0; بټونه--) {
        n = s->bl_شمېر[بټونه];
        طالما (n != 0) {
            m = s->ډېرۍ[--h];
            إذا (m > max_کوډ) استمر;
            إذا ((بینښه) ونه[m].Len != (بینښه) بټونه) {
                Tracev((stderr,"code %d bits %d->%d\n", m, ونه[m].Len, بټونه));
                s->opt_len += ((ulg)بټونه - ونه[m].Len) * ونه[m].Freq;
                ونه[m].Len = (ush)بټونه;
            }
            n--;
        }
    }
}

#که_تعریف_شوی DUMP_BL_ونه
#  include <stdio.h>
#د_که_پای

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local فراغ build_ونه(deflate_state *s, ونه_desc *desc) {
    ct_معلومات *ونه         = desc->dyn_ونه;
    ثابت ct_معلومات *stree  = desc->stat_desc->static_ونه;
    صحيح elems             = desc->stat_desc->elems;
    صحيح n, m;          /* iterate over heap elements */
    صحيح max_کوډ = -1; /* largest code with non zero frequency */
    صحيح غوټه;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->ډېرۍ_len = 0, s->ډېرۍ_max = ډېرۍ_اندازه;

    لكل (n = 0; n < elems; n++) {
        إذا (ونه[n].Freq != 0) {
            s->ډېرۍ[++(s->ډېرۍ_len)] = max_کوډ = n;
            s->ژوروالی[n] = 0;
        } وإلا {
            ونه[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    طالما (s->ډېرۍ_len < 2) {
        غوټه = s->ډېرۍ[++(s->ډېرۍ_len)] = (max_کوډ < 2 ? ++max_کوډ : 0);
        ونه[غوټه].Freq = 1;
        s->ژوروالی[غوټه] = 0;
        s->opt_len--; إذا (stree) s->static_len -= stree[غوټه].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_کوډ = max_کوډ;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    لكل (n = s->ډېرۍ_len/2; n >= 1; n--) pqdownheap(s, ونه, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    غوټه = elems;              /* next internal node of the tree */
    افعل {
        pqremove(s, ونه, n);  /* n = node of least frequency */
        m = s->ډېرۍ[تر_ټولو_کوچنی]; /* m = node of next least frequency */

        s->ډېرۍ[--(s->ډېرۍ_max)] = n; /* keep the nodes sorted by frequency */
        s->ډېرۍ[--(s->ډېرۍ_max)] = m;

        /* Create a new node father of n and m */
        ونه[غوټه].Freq = ونه[n].Freq + ونه[m].Freq;
        s->ژوروالی[غوټه] = (uch)((s->ژوروالی[n] >= s->ژوروالی[m] ?
                                s->ژوروالی[n] : s->ژوروالی[m]) + 1);
        ونه[n].Dad = ونه[m].Dad = (ush)غوټه;
#که_تعریف_شوی DUMP_BL_ونه
        إذا (ونه == s->bl_ونه) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    غوټه, ونه[غوټه].Freq, n, ونه[n].Freq, m, ونه[m].Freq);
        }
#د_که_پای
        /* and insert the new node in the heap */
        s->ډېرۍ[تر_ټولو_کوچنی] = غوټه++;
        pqdownheap(s, ونه, تر_ټولو_کوچنی);

    } طالما (s->ډېرۍ_len >= 2);

    s->ډېرۍ[--(s->ډېرۍ_max)] = s->ډېرۍ[تر_ټولو_کوچنی];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (ونه_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_معلومات *)ونه, max_کوډ, s->bl_شمېر);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local فراغ لټون_ونه(deflate_state *s, ct_معلومات *ونه, صحيح max_کوډ) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = ونه[0].Len; /* length of next code */
    صحيح شمېر = 0;             /* repeat count of the current code */
    صحيح max_شمېر = 7;         /* max repeat count */
    صحيح min_شمېر = 4;         /* min repeat count */

    إذا (nextlen == 0) max_شمېر = 138, min_شمېر = 3;
    ونه[max_کوډ + 1].Len = (ush)0xffff; /* guard */

    لكل (n = 0; n <= max_کوډ; n++) {
        curlen = nextlen; nextlen = ونه[n + 1].Len;
        إذا (++شمېر < max_شمېر && curlen == nextlen) {
            استمر;
        } وإلا إذا (شمېر < min_شمېر) {
            s->bl_ونه[curlen].Freq += (ush)شمېر;
        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) s->bl_ونه[curlen].Freq++;
            s->bl_ونه[REP_3_6].Freq++;
        } وإلا إذا (شمېر <= 10) {
            s->bl_ونه[REPZ_3_10].Freq++;
        } وإلا {
            s->bl_ونه[REPZ_11_138].Freq++;
        }
        شمېر = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_شمېر = 138, min_شمېر = 3;
        } وإلا إذا (curlen == nextlen) {
            max_شمېر = 6, min_شمېر = 3;
        } وإلا {
            max_شمېر = 7, min_شمېر = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local فراغ send_ونه(deflate_state *s, ct_معلومات *ونه, صحيح max_کوډ) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = ونه[0].Len; /* length of next code */
    صحيح شمېر = 0;             /* repeat count of the current code */
    صحيح max_شمېر = 7;         /* max repeat count */
    صحيح min_شمېر = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    إذا (nextlen == 0) max_شمېر = 138, min_شمېر = 3;

    لكل (n = 0; n <= max_کوډ; n++) {
        curlen = nextlen; nextlen = ونه[n + 1].Len;
        إذا (++شمېر < max_شمېر && curlen == nextlen) {
            استمر;
        } وإلا إذا (شمېر < min_شمېر) {
            افعل { send_کوډ(s, curlen, s->bl_ونه); } طالما (--شمېر != 0);

        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) {
                send_کوډ(s, curlen, s->bl_ونه); شمېر--;
            }
            Assert(شمېر >= 3 && شمېر <= 6, " 3_6?");
            send_کوډ(s, REP_3_6, s->bl_ونه); send_بټونه(s, شمېر - 3, 2);

        } وإلا إذا (شمېر <= 10) {
            send_کوډ(s, REPZ_3_10, s->bl_ونه); send_بټونه(s, شمېر - 3, 3);

        } وإلا {
            send_کوډ(s, REPZ_11_138, s->bl_ونه); send_بټونه(s, شمېر - 11, 7);
        }
        شمېر = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_شمېر = 138, min_شمېر = 3;
        } وإلا إذا (curlen == nextlen) {
            max_شمېر = 6, min_شمېر = 3;
        } وإلا {
            max_شمېر = 7, min_شمېر = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local صحيح build_bl_ونه(deflate_state *s) {
    صحيح max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    لټون_ونه(s, (ct_معلومات *)s->dyn_ltree, s->l_desc.max_کوډ);
    لټون_ونه(s, (ct_معلومات *)s->dyn_dtree, s->d_desc.max_کوډ);

    /* Build the bit length tree: */
    build_ونه(s, (ونه_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    لكل (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        إذا (s->bl_ونه[bl_ترتیب[max_blindex]].Len != 0) اكسر;
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
    send_بټونه(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_بټونه(s, dcodes - 1,   5);
    send_بټونه(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    لكل (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_ترتیب[rank]));
        send_بټونه(s, s->bl_ونه[bl_ترتیب[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->بټونه_لېږل_شوی));

    send_ونه(s, (ct_معلومات *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->بټونه_لېږل_شوی));

    send_ونه(s, (ct_معلومات *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->بټونه_لېږل_شوی));
}

/* ===========================================================================
 * Send a stored block
 */
فراغ ZLIB_دننی tr_stored_بلاک(deflate_state *s, charf *buf,
                                    ulg stored_len, صحيح وروستی) {
    send_بټونه(s, (STORED_بلاک<<1) + وروستی, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    إذا (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#که_تعریف_شوی ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->بټونه_لېږل_شوی += 2*16;
    s->بټونه_لېږل_شوی += stored_len << 3;
#د_که_پای
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
فراغ ZLIB_دننی tr_flush_بټونه(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
فراغ ZLIB_دننی _tr_align(deflate_state *s) {
    send_بټونه(s, STATIC_TREES<<1, 3);
    send_کوډ(s, پای_ټکی_بلاک, static_ltree);
#که_تعریف_شوی ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#د_که_پای
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local فراغ compress_بلاک(deflate_state *s, ثابت ct_معلومات *ltree,
                          ثابت ct_معلومات *dtree) {
    بینښه dist;      /* distance of matched string */
    صحيح lc;             /* match length or unmatched char (if dist == 0) */
    بینښه sx = 0;    /* running index in symbol buffers */
    بینښه کوډ;      /* the code to send */
    صحيح extra;          /* number of extra bits to send */

    إذا (s->sym_بل != 0) افعل {
#که_تعریف_شوی LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#وإلا
        dist = s->sym_buf[sx++] & 0xff;
        dist += (بینښه)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#د_که_پای
        إذا (dist == 0) {
            send_کوډ(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } وإلا {
            /* Here, lc is the match length - MIN_MATCH */
            کوډ = اوږدتیا_کوډ[lc];
            send_کوډ(s, کوډ + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[کوډ];
            إذا (extra != 0) {
                lc -= بنسټ_اوږدتیا[کوډ];
                send_بټونه(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            کوډ = d_کوډ(dist);
            Assert (کوډ < D_CODES, "bad d_code");

            send_کوډ(s, کوډ, dtree);       /* send the distance code */
            extra = extra_dbits[کوډ];
            إذا (extra != 0) {
                dist -= (بینښه)بنسټ_dist[کوډ];
                send_بټونه(s, (صحيح)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#که_تعریف_شوی LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#وإلا
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#د_که_پای

    } طالما (sx < s->sym_بل);

    send_کوډ(s, پای_ټکی_بلاک, ltree);
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
    بینښه اوږد بلاک_ماسک = 0xf3ffc07fUL;
    صحيح n;

    /* Check for non-textual ("block-listed") bytes. */
    لكل (n = 0; n <= 31; n++, بلاک_ماسک >>= 1)
        إذا ((بلاک_ماسک & 1) && (s->dyn_ltree[n].Freq != 0))
            أرجع Z_دوګونی;

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
    أرجع Z_دوګونی;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
فراغ ZLIB_دننی tr_flush_بلاک(deflate_state *s, charf *buf,
                                   ulg stored_len, صحيح وروستی) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    صحيح max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    إذا (s->level > 0) {

        /* Check if the file is binary or text */
        إذا (s->strm->معلومات_type == Z_UNKNOWN)
            s->strm->معلومات_type = detect_معلومات_type(s);

        /* Construct the literal and distance trees */
        build_ونه(s, (ونه_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_ونه(s, (ونه_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_ونه(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_بل / 3));

#که_نه_تعریف_شوی FORCE_STATIC
        إذا (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#د_که_پای
            opt_lenb = static_lenb;

    } وإلا {
        Assert(buf != (حرف*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#که_تعریف_شوی FORCE_STORED
    إذا (buf != (حرف*)0) { /* force stored block */
#وإلا
    إذا (stored_len + 4 <= opt_lenb && buf != (حرف*)0) {
                       /* 4: two words for the lengths */
#د_که_پای
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_بلاک(s, buf, stored_len, وروستی);

    } وإلا إذا (static_lenb == opt_lenb) {
        send_بټونه(s, (STATIC_TREES<<1) + وروستی, 3);
        compress_بلاک(s, (ثابت ct_معلومات *)static_ltree,
                       (ثابت ct_معلومات *)static_dtree);
#که_تعریف_شوی ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#د_که_پای
    } وإلا {
        send_بټونه(s, (DYN_TREES<<1) + وروستی, 3);
        send_all_trees(s, s->l_desc.max_کوډ + 1, s->d_desc.max_کوډ + 1,
                       max_blindex + 1);
        compress_بلاک(s, (ثابت ct_معلومات *)s->dyn_ltree,
                       (ثابت ct_معلومات *)s->dyn_dtree);
#که_تعریف_شوی ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#د_که_پای
    }
    Assert (s->compressed_len == s->بټونه_لېږل_شوی, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_بلاک(s);

    إذا (وروستی) {
        bi_windup(s);
#که_تعریف_شوی ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#د_که_پای
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)وروستی));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
صحيح ZLIB_دننی _tr_tally(deflate_state *s, بینښه dist, بینښه lc) {
#که_تعریف_شوی LIT_MEM
    s->d_buf[s->sym_بل] = (ush)dist;
    s->l_buf[s->sym_بل++] = (uch)lc;
#وإلا
    s->sym_buf[s->sym_بل++] = (uch)dist;
    s->sym_buf[s->sym_بل++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_بل++] = (uch)lc;
#د_که_پای
    إذا (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } وإلا {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_سمون-MIN_سمون) &&
               (ush)d_کوډ(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[اوږدتیا_کوډ[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_کوډ(dist)].Freq++;
    }
    أرجع (s->sym_بل == s->sym_پای_ټکی);
}
