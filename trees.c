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

#اگر_تعریف_شده ZLIB_DEBUG
#  include <ctype.h>
#پایان_اگر

/* ===========================================================================
 * Constants
 */

#تعریف_کن MAX_BL_بیتها 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#تعریف_کن انتها_بلوک 256
/* end of block literal code */

#تعریف_کن REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#تعریف_کن REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#تعریف_کن REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ثابت صحيح extra_lbits[درازی_CODES] /* extra bits for each length code */
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

#تعریف_کن DIST_کد_LEN  512 /* see definition of array dist_code below */

#إذا defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_داده static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_داده static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch dist_کد[DIST_کد_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch درازی_کد[MAX_تطابق-MIN_تطابق+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local صحيح پایه_درازی[درازی_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local صحيح پایه_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#وإلا
#  include "trees.h"
#پایان_اگر /* defined(GEN_TREES_H) || !defined(STDC) */

بنية static_درخت_desc_s {
    ثابت ct_داده *static_درخت;  /* static tree or NULL */
    ثابت intf *extra_بیتها;      /* extra bits for each code or NULL */
    صحيح     extra_پایه;          /* base index for extra_bits */
    صحيح     elems;               /* max number of elements in the tree */
    صحيح     max_درازی;          /* max bit length for the codes */
};

#اگر_تعریف_شده NO_INIT_GLOBAL_اشارهگرها
#  define TCONST
#وإلا
#  define TCONST ثابت
#پایان_اگر

local TCONST static_درخت_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_بیتها};

local TCONST static_درخت_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_بیتها};

local TCONST static_درخت_desc static_bl_desc =
{(ثابت ct_داده *)0, extra_blbits, 0,   BL_CODES, MAX_BL_بیتها};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#تعریف_کن put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local بیعلامت bi_معکوس(بیعلامت کد, صحيح len) {
    بیعلامت res = 0;
    افعل {
        res |= کد & 1;
        کد >>= 1, res <<= 1;
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
    s->bi_استفاده_شده = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#اگر_تعریف_شده ZLIB_DEBUG
    s->بیتها_فرستاده = (s->بیتها_فرستاده + 7) & ~(ulg)7;
#پایان_اگر
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local فراغ gen_codes(ct_داده *درخت, صحيح max_کد, ushf *bl_شمار) {
    ush بعدی_کد[MAX_بیتها+1]; /* next code value for each bit length */
    بیعلامت کد = 0;         /* running code value */
    صحيح بیتها;                  /* bit index */
    صحيح n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    لكل (بیتها = 1; بیتها <= MAX_بیتها; بیتها++) {
        کد = (کد + bl_شمار[بیتها - 1]) << 1;
        بعدی_کد[بیتها] = (ush)کد;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (کد + bl_شمار[MAX_بیتها] - 1 == (1 << MAX_بیتها) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_کد));

    لكل (n = 0;  n <= max_کد; n++) {
        صحيح len = درخت[n].Len;
        إذا (len == 0) استمر;
        /* Now reverse the bits */
        درخت[n].کد_2 = (ush)bi_معکوس(بعدی_کد[len]++, len);

        Tracecv(درخت != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, درخت[n].کد_2, بعدی_کد[len] - 1));
    }
}

#اگر_تعریف_شده GEN_TREES_H
local فراغ gen_trees_سرآیند(فراغ);
#پایان_اگر

#اگر_تعریف_نشده ZLIB_DEBUG
#  define send_کد(s, c, درخت) send_بیتها(s, درخت[c].کد_2, درخت[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#وإلا /* !ZLIB_DEBUG */
#  define send_کد(s, c, درخت) \
     { إذا (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_بیتها(s, درخت[c].کد_2, درخت[c].Len); }
#پایان_اگر

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#اگر_تعریف_شده ZLIB_DEBUG
local فراغ send_بیتها(deflate_state *s, صحيح مقدار, صحيح درازی) {
    Tracevv((stderr," l %2d v %4x ", درازی, مقدار));
    Assert(درازی > 0 && درازی <= 15, "invalid length");
    s->بیتها_فرستاده += (ulg)درازی;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    إذا (s->bi_valid > (صحيح)Buf_اندازه - درازی) {
        s->bi_buf |= (ush)مقدار << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)مقدار >> (Buf_اندازه - s->bi_valid);
        s->bi_valid += درازی - Buf_اندازه;
    } وإلا {
        s->bi_buf |= (ush)مقدار << s->bi_valid;
        s->bi_valid += درازی;
    }
}
#وإلا /* !ZLIB_DEBUG */

#تعریف_کن send_بیتها(s, مقدار, درازی) \
{ صحيح len = درازی;\
  إذا (s->bi_valid > (صحيح)Buf_اندازه - len) {\
    صحيح val = (صحيح)مقدار;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_اندازه - s->bi_valid);\
    s->bi_valid += len - Buf_اندازه;\
  } وإلا {\
    s->bi_buf |= (ush)(مقدار) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#پایان_اگر /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local فراغ tr_static_init(فراغ) {
#إذا defined(GEN_TREES_H) || !defined(STDC)
    ساكن صحيح static_init_انجامشده = 0;
    صحيح n;        /* iterates over tree elements */
    صحيح بیتها;     /* bit counter */
    صحيح درازی;   /* length value */
    صحيح کد;     /* code value */
    صحيح dist;     /* distance index */
    ush bl_شمار[MAX_بیتها+1];
    /* number of codes at each bit length for an optimal tree */

    إذا (static_init_انجامشده) أرجع;

    /* For some embedded targets, global variables are not initialized: */
#اگر_تعریف_شده NO_INIT_GLOBAL_اشارهگرها
    static_l_desc.static_درخت = static_ltree;
    static_l_desc.extra_بیتها = extra_lbits;
    static_d_desc.static_درخت = static_dtree;
    static_d_desc.extra_بیتها = extra_dbits;
    static_bl_desc.extra_بیتها = extra_blbits;
#پایان_اگر

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    درازی = 0;
    لكل (کد = 0; کد < درازی_CODES-1; کد++) {
        پایه_درازی[کد] = درازی;
        لكل (n = 0; n < (1 << extra_lbits[کد]); n++) {
            درازی_کد[درازی++] = (uch)کد;
        }
    }
    Assert (درازی == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    درازی_کد[درازی - 1] = (uch)کد;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    لكل (کد = 0 ; کد < 16; کد++) {
        پایه_dist[کد] = dist;
        لكل (n = 0; n < (1 << extra_dbits[کد]); n++) {
            dist_کد[dist++] = (uch)کد;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    لكل ( ; کد < D_CODES; کد++) {
        پایه_dist[کد] = dist << 7;
        لكل (n = 0; n < (1 << (extra_dbits[کد] - 7)); n++) {
            dist_کد[256 + dist++] = (uch)کد;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    لكل (بیتها = 0; بیتها <= MAX_بیتها; بیتها++) bl_شمار[بیتها] = 0;
    n = 0;
    طالما (n <= 143) static_ltree[n++].Len = 8, bl_شمار[8]++;
    طالما (n <= 255) static_ltree[n++].Len = 9, bl_شمار[9]++;
    طالما (n <= 279) static_ltree[n++].Len = 7, bl_شمار[7]++;
    طالما (n <= 287) static_ltree[n++].Len = 8, bl_شمار[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_داده *)static_ltree, L_CODES+1, bl_شمار);

    /* The static distance tree is trivial: */
    لكل (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].کد_2 = bi_معکوس((بیعلامت)n, 5);
    }
    static_init_انجامشده = 1;

#  ifdef GEN_TREES_H
    gen_trees_سرآیند();
#  endif
#پایان_اگر /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#اگر_تعریف_شده GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, آخرین, عرض) \
      ((i) == (آخرین)? "\n};\n\n" :    \
       ((i) % (عرض) == (عرض) - 1 ? ",\n" : ", "))

فراغ gen_trees_سرآیند(فراغ) {
    FILE *سرآیند = fopen("trees.h", "w");
    صحيح i;

    Assert (سرآیند != عدم, "Can't open trees.h");
    fprintf(سرآیند,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(سرآیند, "local const ct_data static_ltree[L_CODES+2] = {\n");
    لكل (i = 0; i < L_CODES+2; i++) {
        fprintf(سرآیند, "{{%3u},{%3u}}%s", static_ltree[i].کد_2,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(سرآیند, "local const ct_data static_dtree[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرآیند, "{{%2u},{%2u}}%s", static_dtree[i].کد_2,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(سرآیند, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    لكل (i = 0; i < DIST_کد_LEN; i++) {
        fprintf(سرآیند, "%2u%s", dist_کد[i],
                SEPARATOR(i, DIST_کد_LEN-1, 20));
    }

    fprintf(سرآیند,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    لكل (i = 0; i < MAX_تطابق-MIN_تطابق+1; i++) {
        fprintf(سرآیند, "%2u%s", درازی_کد[i],
                SEPARATOR(i, MAX_تطابق-MIN_تطابق, 20));
    }

    fprintf(سرآیند, "local const int base_length[LENGTH_CODES] = {\n");
    لكل (i = 0; i < درازی_CODES; i++) {
        fprintf(سرآیند, "%1u%s", پایه_درازی[i],
                SEPARATOR(i, درازی_CODES-1, 20));
    }

    fprintf(سرآیند, "local const int base_dist[D_CODES] = {\n");
    لكل (i = 0; i < D_CODES; i++) {
        fprintf(سرآیند, "%5u%s", پایه_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(سرآیند);
}
#پایان_اگر /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local فراغ init_بلوک(deflate_state *s) {
    صحيح n; /* iterates over tree elements */

    /* Initialize the trees. */
    لكل (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    لكل (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    لكل (n = 0; n < BL_CODES; n++) s->bl_درخت[n].Freq = 0;

    s->dyn_ltree[انتها_بلوک].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_بعدی = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
فراغ ZLIB_داخلی _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_درخت = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_درخت = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_درخت = s->bl_درخت;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_استفاده_شده = 0;
#اگر_تعریف_شده ZLIB_DEBUG
    s->compressed_len = 0L;
    s->بیتها_فرستاده = 0L;
#پایان_اگر

    /* Initialize the first block of the first file: */
    init_بلوک(s);
}

#تعریف_کن کوچکترین 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#تعریف_کن pqremove(s, درخت, بالاترین) \
{\
    بالاترین = s->انباشته[کوچکترین]; \
    s->انباشته[کوچکترین] = s->انباشته[s->انباشته_len--]; \
    pqdownheap(s, درخت, کوچکترین); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#تعریف_کن smaller(درخت, n, m, عمق) \
   (درخت[n].Freq < درخت[m].Freq || \
   (درخت[n].Freq == درخت[m].Freq && عمق[n] <= عمق[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local فراغ pqdownheap(deflate_state *s, ct_داده *درخت, صحيح k) {
    صحيح v = s->انباشته[k];
    صحيح j = k << 1;  /* left son of k */
    طالما (j <= s->انباشته_len) {
        /* Set j to the smallest of the two sons: */
        إذا (j < s->انباشته_len &&
            smaller(درخت, s->انباشته[j + 1], s->انباشته[j], s->عمق)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        إذا (smaller(درخت, v, s->انباشته[j], s->عمق)) اكسر;

        /* Exchange v with the smallest son */
        s->انباشته[k] = s->انباشته[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->انباشته[k] = v;
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
    ct_داده *درخت        = desc->dyn_درخت;
    صحيح max_کد         = desc->max_کد;
    ثابت ct_داده *stree = desc->stat_desc->static_درخت;
    ثابت intf *extra    = desc->stat_desc->extra_بیتها;
    صحيح پایه_2             = desc->stat_desc->extra_پایه;
    صحيح max_درازی       = desc->stat_desc->max_درازی;
    صحيح h;              /* heap index */
    صحيح n, m;           /* iterate over the tree elements */
    صحيح بیتها;           /* bit length */
    صحيح xbits;          /* extra bits */
    ush f;              /* frequency */
    صحيح سرریز = 0;   /* number of elements with bit length too large */

    لكل (بیتها = 0; بیتها <= MAX_بیتها; بیتها++) s->bl_شمار[بیتها] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    درخت[s->انباشته[s->انباشته_max]].Len = 0; /* root of the heap */

    لكل (h = s->انباشته_max + 1; h < انباشته_اندازه; h++) {
        n = s->انباشته[h];
        بیتها = درخت[درخت[n].Dad].Len + 1;
        إذا (بیتها > max_درازی) بیتها = max_درازی, سرریز++;
        درخت[n].Len = (ush)بیتها;
        /* We overwrite tree[n].Dad which is no longer needed */

        إذا (n > max_کد) استمر; /* not a leaf node */

        s->bl_شمار[بیتها]++;
        xbits = 0;
        إذا (n >= پایه_2) xbits = extra[n - پایه_2];
        f = درخت[n].Freq;
        s->opt_len += (ulg)f * (بیعلامت)(بیتها + xbits);
        إذا (stree) s->static_len += (ulg)f * (بیعلامت)(stree[n].Len + xbits);
    }
    إذا (سرریز == 0) أرجع;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    افعل {
        بیتها = max_درازی - 1;
        طالما (s->bl_شمار[بیتها] == 0) بیتها--;
        s->bl_شمار[بیتها]--;        /* move one leaf down the tree */
        s->bl_شمار[بیتها + 1] += 2; /* move one overflow item as its brother */
        s->bl_شمار[max_درازی]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        سرریز -= 2;
    } طالما (سرریز > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    لكل (بیتها = max_درازی; بیتها != 0; بیتها--) {
        n = s->bl_شمار[بیتها];
        طالما (n != 0) {
            m = s->انباشته[--h];
            إذا (m > max_کد) استمر;
            إذا ((بیعلامت) درخت[m].Len != (بیعلامت) بیتها) {
                Tracev((stderr,"code %d bits %d->%d\n", m, درخت[m].Len, بیتها));
                s->opt_len += ((ulg)بیتها - درخت[m].Len) * درخت[m].Freq;
                درخت[m].Len = (ush)بیتها;
            }
            n--;
        }
    }
}

#اگر_تعریف_شده DUMP_BL_درخت
#  include <stdio.h>
#پایان_اگر

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local فراغ build_درخت(deflate_state *s, درخت_desc *desc) {
    ct_داده *درخت         = desc->dyn_درخت;
    ثابت ct_داده *stree  = desc->stat_desc->static_درخت;
    صحيح elems             = desc->stat_desc->elems;
    صحيح n, m;          /* iterate over heap elements */
    صحيح max_کد = -1; /* largest code with non zero frequency */
    صحيح گره;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->انباشته_len = 0, s->انباشته_max = انباشته_اندازه;

    لكل (n = 0; n < elems; n++) {
        إذا (درخت[n].Freq != 0) {
            s->انباشته[++(s->انباشته_len)] = max_کد = n;
            s->عمق[n] = 0;
        } وإلا {
            درخت[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    طالما (s->انباشته_len < 2) {
        گره = s->انباشته[++(s->انباشته_len)] = (max_کد < 2 ? ++max_کد : 0);
        درخت[گره].Freq = 1;
        s->عمق[گره] = 0;
        s->opt_len--; إذا (stree) s->static_len -= stree[گره].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_کد = max_کد;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    لكل (n = s->انباشته_len/2; n >= 1; n--) pqdownheap(s, درخت, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    گره = elems;              /* next internal node of the tree */
    افعل {
        pqremove(s, درخت, n);  /* n = node of least frequency */
        m = s->انباشته[کوچکترین]; /* m = node of next least frequency */

        s->انباشته[--(s->انباشته_max)] = n; /* keep the nodes sorted by frequency */
        s->انباشته[--(s->انباشته_max)] = m;

        /* Create a new node father of n and m */
        درخت[گره].Freq = درخت[n].Freq + درخت[m].Freq;
        s->عمق[گره] = (uch)((s->عمق[n] >= s->عمق[m] ?
                                s->عمق[n] : s->عمق[m]) + 1);
        درخت[n].Dad = درخت[m].Dad = (ush)گره;
#اگر_تعریف_شده DUMP_BL_درخت
        إذا (درخت == s->bl_درخت) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    گره, درخت[گره].Freq, n, درخت[n].Freq, m, درخت[m].Freq);
        }
#پایان_اگر
        /* and insert the new node in the heap */
        s->انباشته[کوچکترین] = گره++;
        pqdownheap(s, درخت, کوچکترین);

    } طالما (s->انباشته_len >= 2);

    s->انباشته[--(s->انباشته_max)] = s->انباشته[کوچکترین];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (درخت_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_داده *)درخت, max_کد, s->bl_شمار);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local فراغ پیمایش_درخت(deflate_state *s, ct_داده *درخت, صحيح max_کد) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = درخت[0].Len; /* length of next code */
    صحيح شمار = 0;             /* repeat count of the current code */
    صحيح max_شمار = 7;         /* max repeat count */
    صحيح min_شمار = 4;         /* min repeat count */

    إذا (nextlen == 0) max_شمار = 138, min_شمار = 3;
    درخت[max_کد + 1].Len = (ush)0xffff; /* guard */

    لكل (n = 0; n <= max_کد; n++) {
        curlen = nextlen; nextlen = درخت[n + 1].Len;
        إذا (++شمار < max_شمار && curlen == nextlen) {
            استمر;
        } وإلا إذا (شمار < min_شمار) {
            s->bl_درخت[curlen].Freq += (ush)شمار;
        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) s->bl_درخت[curlen].Freq++;
            s->bl_درخت[REP_3_6].Freq++;
        } وإلا إذا (شمار <= 10) {
            s->bl_درخت[REPZ_3_10].Freq++;
        } وإلا {
            s->bl_درخت[REPZ_11_138].Freq++;
        }
        شمار = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_شمار = 138, min_شمار = 3;
        } وإلا إذا (curlen == nextlen) {
            max_شمار = 6, min_شمار = 3;
        } وإلا {
            max_شمار = 7, min_شمار = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local فراغ send_درخت(deflate_state *s, ct_داده *درخت, صحيح max_کد) {
    صحيح n;                     /* iterates over all tree elements */
    صحيح prevlen = -1;          /* last emitted length */
    صحيح curlen;                /* length of current code */
    صحيح nextlen = درخت[0].Len; /* length of next code */
    صحيح شمار = 0;             /* repeat count of the current code */
    صحيح max_شمار = 7;         /* max repeat count */
    صحيح min_شمار = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    إذا (nextlen == 0) max_شمار = 138, min_شمار = 3;

    لكل (n = 0; n <= max_کد; n++) {
        curlen = nextlen; nextlen = درخت[n + 1].Len;
        إذا (++شمار < max_شمار && curlen == nextlen) {
            استمر;
        } وإلا إذا (شمار < min_شمار) {
            افعل { send_کد(s, curlen, s->bl_درخت); } طالما (--شمار != 0);

        } وإلا إذا (curlen != 0) {
            إذا (curlen != prevlen) {
                send_کد(s, curlen, s->bl_درخت); شمار--;
            }
            Assert(شمار >= 3 && شمار <= 6, " 3_6?");
            send_کد(s, REP_3_6, s->bl_درخت); send_بیتها(s, شمار - 3, 2);

        } وإلا إذا (شمار <= 10) {
            send_کد(s, REPZ_3_10, s->bl_درخت); send_بیتها(s, شمار - 3, 3);

        } وإلا {
            send_کد(s, REPZ_11_138, s->bl_درخت); send_بیتها(s, شمار - 11, 7);
        }
        شمار = 0; prevlen = curlen;
        إذا (nextlen == 0) {
            max_شمار = 138, min_شمار = 3;
        } وإلا إذا (curlen == nextlen) {
            max_شمار = 6, min_شمار = 3;
        } وإلا {
            max_شمار = 7, min_شمار = 4;
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
    پیمایش_درخت(s, (ct_داده *)s->dyn_ltree, s->l_desc.max_کد);
    پیمایش_درخت(s, (ct_داده *)s->dyn_dtree, s->d_desc.max_کد);

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
    send_بیتها(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_بیتها(s, dcodes - 1,   5);
    send_بیتها(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    لكل (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_ترتیب[rank]));
        send_بیتها(s, s->bl_درخت[bl_ترتیب[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->بیتها_فرستاده));

    send_درخت(s, (ct_داده *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->بیتها_فرستاده));

    send_درخت(s, (ct_داده *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->بیتها_فرستاده));
}

/* ===========================================================================
 * Send a stored block
 */
فراغ ZLIB_داخلی tr_stored_بلوک(deflate_state *s, charf *buf,
                                    ulg stored_len, صحيح آخرین) {
    send_بیتها(s, (STORED_بلوک<<1) + آخرین, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    إذا (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#اگر_تعریف_شده ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->بیتها_فرستاده += 2*16;
    s->بیتها_فرستاده += stored_len << 3;
#پایان_اگر
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
فراغ ZLIB_داخلی tr_flush_بیتها(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
فراغ ZLIB_داخلی _tr_align(deflate_state *s) {
    send_بیتها(s, STATIC_TREES<<1, 3);
    send_کد(s, انتها_بلوک, static_ltree);
#اگر_تعریف_شده ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#پایان_اگر
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local فراغ compress_بلوک(deflate_state *s, ثابت ct_داده *ltree,
                          ثابت ct_داده *dtree) {
    بیعلامت dist;      /* distance of matched string */
    صحيح lc;             /* match length or unmatched char (if dist == 0) */
    بیعلامت sx = 0;    /* running index in symbol buffers */
    بیعلامت کد;      /* the code to send */
    صحيح extra;          /* number of extra bits to send */

    إذا (s->sym_بعدی != 0) افعل {
#اگر_تعریف_شده LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#وإلا
        dist = s->sym_buf[sx++] & 0xff;
        dist += (بیعلامت)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#پایان_اگر
        إذا (dist == 0) {
            send_کد(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } وإلا {
            /* Here, lc is the match length - MIN_MATCH */
            کد = درازی_کد[lc];
            send_کد(s, کد + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[کد];
            إذا (extra != 0) {
                lc -= پایه_درازی[کد];
                send_بیتها(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            کد = d_کد(dist);
            Assert (کد < D_CODES, "bad d_code");

            send_کد(s, کد, dtree);       /* send the distance code */
            extra = extra_dbits[کد];
            إذا (extra != 0) {
                dist -= (بیعلامت)پایه_dist[کد];
                send_بیتها(s, (صحيح)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#اگر_تعریف_شده LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#وإلا
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#پایان_اگر

    } طالما (sx < s->sym_بعدی);

    send_کد(s, انتها_بلوک, ltree);
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
local صحيح detect_داده_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    بیعلامت دراز بلوک_ماسک = 0xf3ffc07fUL;
    صحيح n;

    /* Check for non-textual ("block-listed") bytes. */
    لكل (n = 0; n <= 31; n++, بلوک_ماسک >>= 1)
        إذا ((بلوک_ماسک & 1) && (s->dyn_ltree[n].Freq != 0))
            أرجع Z_دودویی;

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
    أرجع Z_دودویی;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
فراغ ZLIB_داخلی tr_flush_بلوک(deflate_state *s, charf *buf,
                                   ulg stored_len, صحيح آخرین) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    صحيح max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    إذا (s->level > 0) {

        /* Check if the file is binary or text */
        إذا (s->strm->داده_type == Z_UNKNOWN)
            s->strm->داده_type = detect_داده_type(s);

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
                s->sym_بعدی / 3));

#اگر_تعریف_نشده FORCE_STATIC
        إذا (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#پایان_اگر
            opt_lenb = static_lenb;

    } وإلا {
        Assert(buf != (حرف*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#اگر_تعریف_شده FORCE_STORED
    إذا (buf != (حرف*)0) { /* force stored block */
#وإلا
    إذا (stored_len + 4 <= opt_lenb && buf != (حرف*)0) {
                       /* 4: two words for the lengths */
#پایان_اگر
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_بلوک(s, buf, stored_len, آخرین);

    } وإلا إذا (static_lenb == opt_lenb) {
        send_بیتها(s, (STATIC_TREES<<1) + آخرین, 3);
        compress_بلوک(s, (ثابت ct_داده *)static_ltree,
                       (ثابت ct_داده *)static_dtree);
#اگر_تعریف_شده ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#پایان_اگر
    } وإلا {
        send_بیتها(s, (DYN_TREES<<1) + آخرین, 3);
        send_all_trees(s, s->l_desc.max_کد + 1, s->d_desc.max_کد + 1,
                       max_blindex + 1);
        compress_بلوک(s, (ثابت ct_داده *)s->dyn_ltree,
                       (ثابت ct_داده *)s->dyn_dtree);
#اگر_تعریف_شده ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#پایان_اگر
    }
    Assert (s->compressed_len == s->بیتها_فرستاده, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_بلوک(s);

    إذا (آخرین) {
        bi_windup(s);
#اگر_تعریف_شده ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#پایان_اگر
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)آخرین));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
صحيح ZLIB_داخلی _tr_tally(deflate_state *s, بیعلامت dist, بیعلامت lc) {
#اگر_تعریف_شده LIT_MEM
    s->d_buf[s->sym_بعدی] = (ush)dist;
    s->l_buf[s->sym_بعدی++] = (uch)lc;
#وإلا
    s->sym_buf[s->sym_بعدی++] = (uch)dist;
    s->sym_buf[s->sym_بعدی++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_بعدی++] = (uch)lc;
#پایان_اگر
    إذا (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } وإلا {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_تطابق-MIN_تطابق) &&
               (ush)d_کد(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[درازی_کد[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_کد(dist)].Freq++;
    }
    أرجع (s->sym_بعدی == s->sym_انتها);
}
