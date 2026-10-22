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

#అన్తర్భావ "deflate.h"

#నిర్వచితమైతే ZLIB_DEBUG
#  include <ctype.h>
#అయితే_ముగింపు

/* ===========================================================================
 * Constants
 */

#నిర్వచించు MAX_BL_బిట్లు 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#నిర్వచించు చివర_ఖండం 256
/* end of block literal code */

#నిర్వచించు REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#నిర్వచించు REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#నిర్వచించు REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local స్థిరరాశి పూర్ణాంకం extra_lbits[నిడివి_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local స్థిరరాశి పూర్ణాంకం extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local స్థిరరాశి పూర్ణాంకం extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local స్థిరరాశి uch bl_క్రమం[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#నిర్వచించు DIST_సంకేతం_LEN  512 /* see definition of array dist_code below */

#ఒకవేళ defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_దత్తాంశం static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_దత్తాంశం static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch dist_సంకేతం[DIST_సంకేతం_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch నిడివి_సంకేతం[MAX_సరిపోలిక-MIN_సరిపోలిక+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local పూర్ణాంకం పునాది_నిడివి[నిడివి_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local పూర్ణాంకం పునాది_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#లేకపోతే
#  include "trees.h"
#అయితే_ముగింపు /* defined(GEN_TREES_H) || !defined(STDC) */

నిర్మాణం static_వృక్షం_desc_s {
    స్థిరరాశి ct_దత్తాంశం *static_వృక్షం;  /* static tree or NULL */
    స్థిరరాశి intf *extra_బిట్లు;      /* extra bits for each code or NULL */
    పూర్ణాంకం     extra_పునాది;          /* base index for extra_bits */
    పూర్ణాంకం     elems;               /* max number of elements in the tree */
    పూర్ణాంకం     max_నిడివి;          /* max bit length for the codes */
};

#నిర్వచితమైతే NO_INIT_GLOBAL_సూచికలు
#  define TCONST
#లేకపోతే
#  define TCONST స్థిరరాశి
#అయితే_ముగింపు

local TCONST static_వృక్షం_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_బిట్లు};

local TCONST static_వృక్షం_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_బిట్లు};

local TCONST static_వృక్షం_desc static_bl_desc =
{(స్థిరరాశి ct_దత్తాంశం *)0, extra_blbits, 0,   BL_CODES, MAX_BL_బిట్లు};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#నిర్వచించు put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local గుర్తులేని bi_తిరగేయి(గుర్తులేని సంకేతం_2, పూర్ణాంకం len) {
    గుర్తులేని res = 0;
    చేయి {
        res |= సంకేతం_2 & 1;
        సంకేతం_2 >>= 1, res <<= 1;
    } అయితే (--len > 0);
    తిరిగి res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local శూన్య bi_flush(deflate_state *s) {
    ఒకవేళ (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } లేకపోతే ఒకవేళ (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local శూన్య bi_windup(deflate_state *s) {
    ఒకవేళ (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } లేకపోతే ఒకవేళ (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_వాడిన = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#నిర్వచితమైతే ZLIB_DEBUG
    s->బిట్లు_పంపిన = (s->బిట్లు_పంపిన + 7) & ~(ulg)7;
#అయితే_ముగింపు
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local శూన్య gen_codes(ct_దత్తాంశం *వృక్షం, పూర్ణాంకం max_సంకేతం, ushf *bl_లెక్క) {
    ush తదుపరి_సంకేతం[MAX_బిట్లు+1]; /* next code value for each bit length */
    గుర్తులేని సంకేతం_2 = 0;         /* running code value */
    పూర్ణాంకం బిట్లు;                  /* bit index */
    పూర్ణాంకం n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    కొరకు (బిట్లు = 1; బిట్లు <= MAX_బిట్లు; బిట్లు++) {
        సంకేతం_2 = (సంకేతం_2 + bl_లెక్క[బిట్లు - 1]) << 1;
        తదుపరి_సంకేతం[బిట్లు] = (ush)సంకేతం_2;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (సంకేతం_2 + bl_లెక్క[MAX_బిట్లు] - 1 == (1 << MAX_బిట్లు) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_సంకేతం));

    కొరకు (n = 0;  n <= max_సంకేతం; n++) {
        పూర్ణాంకం len = వృక్షం[n].Len;
        ఒకవేళ (len == 0) కొనసాగు;
        /* Now reverse the bits */
        వృక్షం[n].సంకేతం_3 = (ush)bi_తిరగేయి(తదుపరి_సంకేతం[len]++, len);

        Tracecv(వృక్షం != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, వృక్షం[n].సంకేతం_3, తదుపరి_సంకేతం[len] - 1));
    }
}

#నిర్వచితమైతే GEN_TREES_H
local శూన్య gen_trees_శీర్షిక(శూన్య);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే ZLIB_DEBUG
#  define send_సంకేతం(s, c, వృక్షం) send_బిట్లు(s, వృక్షం[c].సంకేతం_3, వృక్షం[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#లేకపోతే /* !ZLIB_DEBUG */
#  define send_సంకేతం(s, c, వృక్షం) \
     { ఒకవేళ (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_బిట్లు(s, వృక్షం[c].సంకేతం_3, వృక్షం[c].Len); }
#అయితే_ముగింపు

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#నిర్వచితమైతే ZLIB_DEBUG
local శూన్య send_బిట్లు(deflate_state *s, పూర్ణాంకం విలువ, పూర్ణాంకం నిడివి) {
    Tracevv((stderr," l %2d v %4x ", నిడివి, విలువ));
    Assert(నిడివి > 0 && నిడివి <= 15, "invalid length");
    s->బిట్లు_పంపిన += (ulg)నిడివి;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    ఒకవేళ (s->bi_valid > (పూర్ణాంకం)Buf_కొలత - నిడివి) {
        s->bi_buf |= (ush)విలువ << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)విలువ >> (Buf_కొలత - s->bi_valid);
        s->bi_valid += నిడివి - Buf_కొలత;
    } లేకపోతే {
        s->bi_buf |= (ush)విలువ << s->bi_valid;
        s->bi_valid += నిడివి;
    }
}
#లేకపోతే /* !ZLIB_DEBUG */

#నిర్వచించు send_బిట్లు(s, విలువ, నిడివి) \
{ పూర్ణాంకం len = నిడివి;\
  ఒకవేళ (s->bi_valid > (పూర్ణాంకం)Buf_కొలత - len) {\
    పూర్ణాంకం val = (పూర్ణాంకం)విలువ;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_కొలత - s->bi_valid);\
    s->bi_valid += len - Buf_కొలత;\
  } లేకపోతే {\
    s->bi_buf |= (ush)(విలువ) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#అయితే_ముగింపు /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local శూన్య tr_static_init(శూన్య) {
#ఒకవేళ defined(GEN_TREES_H) || !defined(STDC)
    స్థిర పూర్ణాంకం static_init_పూర్తి = 0;
    పూర్ణాంకం n;        /* iterates over tree elements */
    పూర్ణాంకం బిట్లు;     /* bit counter */
    పూర్ణాంకం నిడివి;   /* length value */
    పూర్ణాంకం సంకేతం_2;     /* code value */
    పూర్ణాంకం dist;     /* distance index */
    ush bl_లెక్క[MAX_బిట్లు+1];
    /* number of codes at each bit length for an optimal tree */

    ఒకవేళ (static_init_పూర్తి) తిరిగి;

    /* For some embedded targets, global variables are not initialized: */
#నిర్వచితమైతే NO_INIT_GLOBAL_సూచికలు
    static_l_desc.static_వృక్షం = static_ltree;
    static_l_desc.extra_బిట్లు = extra_lbits;
    static_d_desc.static_వృక్షం = static_dtree;
    static_d_desc.extra_బిట్లు = extra_dbits;
    static_bl_desc.extra_బిట్లు = extra_blbits;
#అయితే_ముగింపు

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    నిడివి = 0;
    కొరకు (సంకేతం_2 = 0; సంకేతం_2 < నిడివి_CODES-1; సంకేతం_2++) {
        పునాది_నిడివి[సంకేతం_2] = నిడివి;
        కొరకు (n = 0; n < (1 << extra_lbits[సంకేతం_2]); n++) {
            నిడివి_సంకేతం[నిడివి++] = (uch)సంకేతం_2;
        }
    }
    Assert (నిడివి == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    నిడివి_సంకేతం[నిడివి - 1] = (uch)సంకేతం_2;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    కొరకు (సంకేతం_2 = 0 ; సంకేతం_2 < 16; సంకేతం_2++) {
        పునాది_dist[సంకేతం_2] = dist;
        కొరకు (n = 0; n < (1 << extra_dbits[సంకేతం_2]); n++) {
            dist_సంకేతం[dist++] = (uch)సంకేతం_2;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    కొరకు ( ; సంకేతం_2 < D_CODES; సంకేతం_2++) {
        పునాది_dist[సంకేతం_2] = dist << 7;
        కొరకు (n = 0; n < (1 << (extra_dbits[సంకేతం_2] - 7)); n++) {
            dist_సంకేతం[256 + dist++] = (uch)సంకేతం_2;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    కొరకు (బిట్లు = 0; బిట్లు <= MAX_బిట్లు; బిట్లు++) bl_లెక్క[బిట్లు] = 0;
    n = 0;
    అయితే (n <= 143) static_ltree[n++].Len = 8, bl_లెక్క[8]++;
    అయితే (n <= 255) static_ltree[n++].Len = 9, bl_లెక్క[9]++;
    అయితే (n <= 279) static_ltree[n++].Len = 7, bl_లెక్క[7]++;
    అయితే (n <= 287) static_ltree[n++].Len = 8, bl_లెక్క[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_దత్తాంశం *)static_ltree, L_CODES+1, bl_లెక్క);

    /* The static distance tree is trivial: */
    కొరకు (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].సంకేతం_3 = bi_తిరగేయి((గుర్తులేని)n, 5);
    }
    static_init_పూర్తి = 1;

#  ifdef GEN_TREES_H
    gen_trees_శీర్షిక();
#  endif
#అయితే_ముగింపు /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#నిర్వచితమైతే GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, చివరి, వెడల్పు) \
      ((i) == (చివరి)? "\n};\n\n" :    \
       ((i) % (వెడల్పు) == (వెడల్పు) - 1 ? ",\n" : ", "))

శూన్య gen_trees_శీర్షిక(శూన్య) {
    FILE *శీర్షిక = fopen("trees.h", "w");
    పూర్ణాంకం i;

    Assert (శీర్షిక != ఖాళీ, "Can't open trees.h");
    fprintf(శీర్షిక,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(శీర్షిక, "local const ct_data static_ltree[L_CODES+2] = {\n");
    కొరకు (i = 0; i < L_CODES+2; i++) {
        fprintf(శీర్షిక, "{{%3u},{%3u}}%s", static_ltree[i].సంకేతం_3,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(శీర్షిక, "local const ct_data static_dtree[D_CODES] = {\n");
    కొరకు (i = 0; i < D_CODES; i++) {
        fprintf(శీర్షిక, "{{%2u},{%2u}}%s", static_dtree[i].సంకేతం_3,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(శీర్షిక, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    కొరకు (i = 0; i < DIST_సంకేతం_LEN; i++) {
        fprintf(శీర్షిక, "%2u%s", dist_సంకేతం[i],
                SEPARATOR(i, DIST_సంకేతం_LEN-1, 20));
    }

    fprintf(శీర్షిక,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    కొరకు (i = 0; i < MAX_సరిపోలిక-MIN_సరిపోలిక+1; i++) {
        fprintf(శీర్షిక, "%2u%s", నిడివి_సంకేతం[i],
                SEPARATOR(i, MAX_సరిపోలిక-MIN_సరిపోలిక, 20));
    }

    fprintf(శీర్షిక, "local const int base_length[LENGTH_CODES] = {\n");
    కొరకు (i = 0; i < నిడివి_CODES; i++) {
        fprintf(శీర్షిక, "%1u%s", పునాది_నిడివి[i],
                SEPARATOR(i, నిడివి_CODES-1, 20));
    }

    fprintf(శీర్షిక, "local const int base_dist[D_CODES] = {\n");
    కొరకు (i = 0; i < D_CODES; i++) {
        fprintf(శీర్షిక, "%5u%s", పునాది_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(శీర్షిక);
}
#అయితే_ముగింపు /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local శూన్య init_ఖండం(deflate_state *s) {
    పూర్ణాంకం n; /* iterates over tree elements */

    /* Initialize the trees. */
    కొరకు (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    కొరకు (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    కొరకు (n = 0; n < BL_CODES; n++) s->bl_వృక్షం[n].Freq = 0;

    s->dyn_ltree[చివర_ఖండం].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_తదుపరి = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
శూన్య ZLIB_అంతర్గత _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_వృక్షం = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_వృక్షం = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_వృక్షం = s->bl_వృక్షం;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_వాడిన = 0;
#నిర్వచితమైతే ZLIB_DEBUG
    s->compressed_len = 0L;
    s->బిట్లు_పంపిన = 0L;
#అయితే_ముగింపు

    /* Initialize the first block of the first file: */
    init_ఖండం(s);
}

#నిర్వచించు అతిచిన్న 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#నిర్వచించు pqremove(s, వృక్షం, శిఖరం) \
{\
    శిఖరం = s->కుప్ప[అతిచిన్న]; \
    s->కుప్ప[అతిచిన్న] = s->కుప్ప[s->కుప్ప_len--]; \
    pqdownheap(s, వృక్షం, అతిచిన్న); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#నిర్వచించు smaller(వృక్షం, n, m, లోతు) \
   (వృక్షం[n].Freq < వృక్షం[m].Freq || \
   (వృక్షం[n].Freq == వృక్షం[m].Freq && లోతు[n] <= లోతు[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local శూన్య pqdownheap(deflate_state *s, ct_దత్తాంశం *వృక్షం, పూర్ణాంకం k) {
    పూర్ణాంకం v = s->కుప్ప[k];
    పూర్ణాంకం j = k << 1;  /* left son of k */
    అయితే (j <= s->కుప్ప_len) {
        /* Set j to the smallest of the two sons: */
        ఒకవేళ (j < s->కుప్ప_len &&
            smaller(వృక్షం, s->కుప్ప[j + 1], s->కుప్ప[j], s->లోతు)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        ఒకవేళ (smaller(వృక్షం, v, s->కుప్ప[j], s->లోతు)) ఆపు;

        /* Exchange v with the smallest son */
        s->కుప్ప[k] = s->కుప్ప[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->కుప్ప[k] = v;
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
local శూన్య gen_bitlen(deflate_state *s, వృక్షం_desc *desc) {
    ct_దత్తాంశం *వృక్షం        = desc->dyn_వృక్షం;
    పూర్ణాంకం max_సంకేతం         = desc->max_సంకేతం;
    స్థిరరాశి ct_దత్తాంశం *stree = desc->stat_desc->static_వృక్షం;
    స్థిరరాశి intf *extra    = desc->stat_desc->extra_బిట్లు;
    పూర్ణాంకం పునాది_2             = desc->stat_desc->extra_పునాది;
    పూర్ణాంకం max_నిడివి       = desc->stat_desc->max_నిడివి;
    పూర్ణాంకం h;              /* heap index */
    పూర్ణాంకం n, m;           /* iterate over the tree elements */
    పూర్ణాంకం బిట్లు;           /* bit length */
    పూర్ణాంకం xbits;          /* extra bits */
    ush f;              /* frequency */
    పూర్ణాంకం అధికప్రవాహం = 0;   /* number of elements with bit length too large */

    కొరకు (బిట్లు = 0; బిట్లు <= MAX_బిట్లు; బిట్లు++) s->bl_లెక్క[బిట్లు] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    వృక్షం[s->కుప్ప[s->కుప్ప_max]].Len = 0; /* root of the heap */

    కొరకు (h = s->కుప్ప_max + 1; h < కుప్ప_కొలత; h++) {
        n = s->కుప్ప[h];
        బిట్లు = వృక్షం[వృక్షం[n].Dad].Len + 1;
        ఒకవేళ (బిట్లు > max_నిడివి) బిట్లు = max_నిడివి, అధికప్రవాహం++;
        వృక్షం[n].Len = (ush)బిట్లు;
        /* We overwrite tree[n].Dad which is no longer needed */

        ఒకవేళ (n > max_సంకేతం) కొనసాగు; /* not a leaf node */

        s->bl_లెక్క[బిట్లు]++;
        xbits = 0;
        ఒకవేళ (n >= పునాది_2) xbits = extra[n - పునాది_2];
        f = వృక్షం[n].Freq;
        s->opt_len += (ulg)f * (గుర్తులేని)(బిట్లు + xbits);
        ఒకవేళ (stree) s->static_len += (ulg)f * (గుర్తులేని)(stree[n].Len + xbits);
    }
    ఒకవేళ (అధికప్రవాహం == 0) తిరిగి;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    చేయి {
        బిట్లు = max_నిడివి - 1;
        అయితే (s->bl_లెక్క[బిట్లు] == 0) బిట్లు--;
        s->bl_లెక్క[బిట్లు]--;        /* move one leaf down the tree */
        s->bl_లెక్క[బిట్లు + 1] += 2; /* move one overflow item as its brother */
        s->bl_లెక్క[max_నిడివి]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        అధికప్రవాహం -= 2;
    } అయితే (అధికప్రవాహం > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    కొరకు (బిట్లు = max_నిడివి; బిట్లు != 0; బిట్లు--) {
        n = s->bl_లెక్క[బిట్లు];
        అయితే (n != 0) {
            m = s->కుప్ప[--h];
            ఒకవేళ (m > max_సంకేతం) కొనసాగు;
            ఒకవేళ ((గుర్తులేని) వృక్షం[m].Len != (గుర్తులేని) బిట్లు) {
                Tracev((stderr,"code %d bits %d->%d\n", m, వృక్షం[m].Len, బిట్లు));
                s->opt_len += ((ulg)బిట్లు - వృక్షం[m].Len) * వృక్షం[m].Freq;
                వృక్షం[m].Len = (ush)బిట్లు;
            }
            n--;
        }
    }
}

#నిర్వచితమైతే DUMP_BL_వృక్షం
#  include <stdio.h>
#అయితే_ముగింపు

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local శూన్య build_వృక్షం(deflate_state *s, వృక్షం_desc *desc) {
    ct_దత్తాంశం *వృక్షం         = desc->dyn_వృక్షం;
    స్థిరరాశి ct_దత్తాంశం *stree  = desc->stat_desc->static_వృక్షం;
    పూర్ణాంకం elems             = desc->stat_desc->elems;
    పూర్ణాంకం n, m;          /* iterate over heap elements */
    పూర్ణాంకం max_సంకేతం = -1; /* largest code with non zero frequency */
    పూర్ణాంకం కణుపు;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->కుప్ప_len = 0, s->కుప్ప_max = కుప్ప_కొలత;

    కొరకు (n = 0; n < elems; n++) {
        ఒకవేళ (వృక్షం[n].Freq != 0) {
            s->కుప్ప[++(s->కుప్ప_len)] = max_సంకేతం = n;
            s->లోతు[n] = 0;
        } లేకపోతే {
            వృక్షం[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    అయితే (s->కుప్ప_len < 2) {
        కణుపు = s->కుప్ప[++(s->కుప్ప_len)] = (max_సంకేతం < 2 ? ++max_సంకేతం : 0);
        వృక్షం[కణుపు].Freq = 1;
        s->లోతు[కణుపు] = 0;
        s->opt_len--; ఒకవేళ (stree) s->static_len -= stree[కణుపు].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_సంకేతం = max_సంకేతం;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    కొరకు (n = s->కుప్ప_len/2; n >= 1; n--) pqdownheap(s, వృక్షం, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    కణుపు = elems;              /* next internal node of the tree */
    చేయి {
        pqremove(s, వృక్షం, n);  /* n = node of least frequency */
        m = s->కుప్ప[అతిచిన్న]; /* m = node of next least frequency */

        s->కుప్ప[--(s->కుప్ప_max)] = n; /* keep the nodes sorted by frequency */
        s->కుప్ప[--(s->కుప్ప_max)] = m;

        /* Create a new node father of n and m */
        వృక్షం[కణుపు].Freq = వృక్షం[n].Freq + వృక్షం[m].Freq;
        s->లోతు[కణుపు] = (uch)((s->లోతు[n] >= s->లోతు[m] ?
                                s->లోతు[n] : s->లోతు[m]) + 1);
        వృక్షం[n].Dad = వృక్షం[m].Dad = (ush)కణుపు;
#నిర్వచితమైతే DUMP_BL_వృక్షం
        ఒకవేళ (వృక్షం == s->bl_వృక్షం) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    కణుపు, వృక్షం[కణుపు].Freq, n, వృక్షం[n].Freq, m, వృక్షం[m].Freq);
        }
#అయితే_ముగింపు
        /* and insert the new node in the heap */
        s->కుప్ప[అతిచిన్న] = కణుపు++;
        pqdownheap(s, వృక్షం, అతిచిన్న);

    } అయితే (s->కుప్ప_len >= 2);

    s->కుప్ప[--(s->కుప్ప_max)] = s->కుప్ప[అతిచిన్న];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (వృక్షం_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_దత్తాంశం *)వృక్షం, max_సంకేతం, s->bl_లెక్క);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local శూన్య పరిశీలన_వృక్షం(deflate_state *s, ct_దత్తాంశం *వృక్షం, పూర్ణాంకం max_సంకేతం) {
    పూర్ణాంకం n;                     /* iterates over all tree elements */
    పూర్ణాంకం prevlen = -1;          /* last emitted length */
    పూర్ణాంకం curlen;                /* length of current code */
    పూర్ణాంకం nextlen = వృక్షం[0].Len; /* length of next code */
    పూర్ణాంకం లెక్క = 0;             /* repeat count of the current code */
    పూర్ణాంకం max_లెక్క = 7;         /* max repeat count */
    పూర్ణాంకం min_లెక్క = 4;         /* min repeat count */

    ఒకవేళ (nextlen == 0) max_లెక్క = 138, min_లెక్క = 3;
    వృక్షం[max_సంకేతం + 1].Len = (ush)0xffff; /* guard */

    కొరకు (n = 0; n <= max_సంకేతం; n++) {
        curlen = nextlen; nextlen = వృక్షం[n + 1].Len;
        ఒకవేళ (++లెక్క < max_లెక్క && curlen == nextlen) {
            కొనసాగు;
        } లేకపోతే ఒకవేళ (లెక్క < min_లెక్క) {
            s->bl_వృక్షం[curlen].Freq += (ush)లెక్క;
        } లేకపోతే ఒకవేళ (curlen != 0) {
            ఒకవేళ (curlen != prevlen) s->bl_వృక్షం[curlen].Freq++;
            s->bl_వృక్షం[REP_3_6].Freq++;
        } లేకపోతే ఒకవేళ (లెక్క <= 10) {
            s->bl_వృక్షం[REPZ_3_10].Freq++;
        } లేకపోతే {
            s->bl_వృక్షం[REPZ_11_138].Freq++;
        }
        లెక్క = 0; prevlen = curlen;
        ఒకవేళ (nextlen == 0) {
            max_లెక్క = 138, min_లెక్క = 3;
        } లేకపోతే ఒకవేళ (curlen == nextlen) {
            max_లెక్క = 6, min_లెక్క = 3;
        } లేకపోతే {
            max_లెక్క = 7, min_లెక్క = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local శూన్య send_వృక్షం(deflate_state *s, ct_దత్తాంశం *వృక్షం, పూర్ణాంకం max_సంకేతం) {
    పూర్ణాంకం n;                     /* iterates over all tree elements */
    పూర్ణాంకం prevlen = -1;          /* last emitted length */
    పూర్ణాంకం curlen;                /* length of current code */
    పూర్ణాంకం nextlen = వృక్షం[0].Len; /* length of next code */
    పూర్ణాంకం లెక్క = 0;             /* repeat count of the current code */
    పూర్ణాంకం max_లెక్క = 7;         /* max repeat count */
    పూర్ణాంకం min_లెక్క = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    ఒకవేళ (nextlen == 0) max_లెక్క = 138, min_లెక్క = 3;

    కొరకు (n = 0; n <= max_సంకేతం; n++) {
        curlen = nextlen; nextlen = వృక్షం[n + 1].Len;
        ఒకవేళ (++లెక్క < max_లెక్క && curlen == nextlen) {
            కొనసాగు;
        } లేకపోతే ఒకవేళ (లెక్క < min_లెక్క) {
            చేయి { send_సంకేతం(s, curlen, s->bl_వృక్షం); } అయితే (--లెక్క != 0);

        } లేకపోతే ఒకవేళ (curlen != 0) {
            ఒకవేళ (curlen != prevlen) {
                send_సంకేతం(s, curlen, s->bl_వృక్షం); లెక్క--;
            }
            Assert(లెక్క >= 3 && లెక్క <= 6, " 3_6?");
            send_సంకేతం(s, REP_3_6, s->bl_వృక్షం); send_బిట్లు(s, లెక్క - 3, 2);

        } లేకపోతే ఒకవేళ (లెక్క <= 10) {
            send_సంకేతం(s, REPZ_3_10, s->bl_వృక్షం); send_బిట్లు(s, లెక్క - 3, 3);

        } లేకపోతే {
            send_సంకేతం(s, REPZ_11_138, s->bl_వృక్షం); send_బిట్లు(s, లెక్క - 11, 7);
        }
        లెక్క = 0; prevlen = curlen;
        ఒకవేళ (nextlen == 0) {
            max_లెక్క = 138, min_లెక్క = 3;
        } లేకపోతే ఒకవేళ (curlen == nextlen) {
            max_లెక్క = 6, min_లెక్క = 3;
        } లేకపోతే {
            max_లెక్క = 7, min_లెక్క = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local పూర్ణాంకం build_bl_వృక్షం(deflate_state *s) {
    పూర్ణాంకం max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    పరిశీలన_వృక్షం(s, (ct_దత్తాంశం *)s->dyn_ltree, s->l_desc.max_సంకేతం);
    పరిశీలన_వృక్షం(s, (ct_దత్తాంశం *)s->dyn_dtree, s->d_desc.max_సంకేతం);

    /* Build the bit length tree: */
    build_వృక్షం(s, (వృక్షం_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    కొరకు (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        ఒకవేళ (s->bl_వృక్షం[bl_క్రమం[max_blindex]].Len != 0) ఆపు;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((stderr, "\ndyn trees: dyn %lu, stat %lu",
            s->opt_len, s->static_len));

    తిరిగి max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local శూన్య send_all_trees(deflate_state *s, పూర్ణాంకం lcodes, పూర్ణాంకం dcodes,
                          పూర్ణాంకం blcodes) {
    పూర్ణాంకం rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_బిట్లు(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_బిట్లు(s, dcodes - 1,   5);
    send_బిట్లు(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    కొరకు (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_క్రమం[rank]));
        send_బిట్లు(s, s->bl_వృక్షం[bl_క్రమం[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->బిట్లు_పంపిన));

    send_వృక్షం(s, (ct_దత్తాంశం *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->బిట్లు_పంపిన));

    send_వృక్షం(s, (ct_దత్తాంశం *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->బిట్లు_పంపిన));
}

/* ===========================================================================
 * Send a stored block
 */
శూన్య ZLIB_అంతర్గత tr_stored_ఖండం(deflate_state *s, charf *buf,
                                    ulg stored_len, పూర్ణాంకం చివరి) {
    send_బిట్లు(s, (STORED_ఖండం<<1) + చివరి, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    ఒకవేళ (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#నిర్వచితమైతే ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->బిట్లు_పంపిన += 2*16;
    s->బిట్లు_పంపిన += stored_len << 3;
#అయితే_ముగింపు
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
శూన్య ZLIB_అంతర్గత tr_flush_బిట్లు(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
శూన్య ZLIB_అంతర్గత _tr_align(deflate_state *s) {
    send_బిట్లు(s, STATIC_TREES<<1, 3);
    send_సంకేతం(s, చివర_ఖండం, static_ltree);
#నిర్వచితమైతే ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#అయితే_ముగింపు
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local శూన్య compress_ఖండం(deflate_state *s, స్థిరరాశి ct_దత్తాంశం *ltree,
                          స్థిరరాశి ct_దత్తాంశం *dtree) {
    గుర్తులేని dist;      /* distance of matched string */
    పూర్ణాంకం lc;             /* match length or unmatched char (if dist == 0) */
    గుర్తులేని sx = 0;    /* running index in symbol buffers */
    గుర్తులేని సంకేతం_2;      /* the code to send */
    పూర్ణాంకం extra;          /* number of extra bits to send */

    ఒకవేళ (s->sym_తదుపరి != 0) చేయి {
#నిర్వచితమైతే LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#లేకపోతే
        dist = s->sym_buf[sx++] & 0xff;
        dist += (గుర్తులేని)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#అయితే_ముగింపు
        ఒకవేళ (dist == 0) {
            send_సంకేతం(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } లేకపోతే {
            /* Here, lc is the match length - MIN_MATCH */
            సంకేతం_2 = నిడివి_సంకేతం[lc];
            send_సంకేతం(s, సంకేతం_2 + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[సంకేతం_2];
            ఒకవేళ (extra != 0) {
                lc -= పునాది_నిడివి[సంకేతం_2];
                send_బిట్లు(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            సంకేతం_2 = d_సంకేతం(dist);
            Assert (సంకేతం_2 < D_CODES, "bad d_code");

            send_సంకేతం(s, సంకేతం_2, dtree);       /* send the distance code */
            extra = extra_dbits[సంకేతం_2];
            ఒకవేళ (extra != 0) {
                dist -= (గుర్తులేని)పునాది_dist[సంకేతం_2];
                send_బిట్లు(s, (పూర్ణాంకం)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#నిర్వచితమైతే LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#లేకపోతే
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#అయితే_ముగింపు

    } అయితే (sx < s->sym_తదుపరి);

    send_సంకేతం(s, చివర_ఖండం, ltree);
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
local పూర్ణాంకం detect_దత్తాంశం_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    గుర్తులేని దీర్ఘ ఖండం_ముసుగు = 0xf3ffc07fUL;
    పూర్ణాంకం n;

    /* Check for non-textual ("block-listed") bytes. */
    కొరకు (n = 0; n <= 31; n++, ఖండం_ముసుగు >>= 1)
        ఒకవేళ ((ఖండం_ముసుగు & 1) && (s->dyn_ltree[n].Freq != 0))
            తిరిగి Z_ద్విమాన;

    /* Check for textual ("allow-listed") bytes. */
    ఒకవేళ (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        తిరిగి Z_పాఠ్యం;
    కొరకు (n = 32; n < LITERALS; n++)
        ఒకవేళ (s->dyn_ltree[n].Freq != 0)
            తిరిగి Z_పాఠ్యం;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    తిరిగి Z_ద్విమాన;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
శూన్య ZLIB_అంతర్గత tr_flush_ఖండం(deflate_state *s, charf *buf,
                                   ulg stored_len, పూర్ణాంకం చివరి) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    పూర్ణాంకం max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    ఒకవేళ (s->level > 0) {

        /* Check if the file is binary or text */
        ఒకవేళ (s->strm->దత్తాంశం_type == Z_UNKNOWN)
            s->strm->దత్తాంశం_type = detect_దత్తాంశం_type(s);

        /* Construct the literal and distance trees */
        build_వృక్షం(s, (వృక్షం_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_వృక్షం(s, (వృక్షం_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_వృక్షం(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_తదుపరి / 3));

#నిర్వచితం_కాకపోతే FORCE_STATIC
        ఒకవేళ (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#అయితే_ముగింపు
            opt_lenb = static_lenb;

    } లేకపోతే {
        Assert(buf != (అక్షరం*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#నిర్వచితమైతే FORCE_STORED
    ఒకవేళ (buf != (అక్షరం*)0) { /* force stored block */
#లేకపోతే
    ఒకవేళ (stored_len + 4 <= opt_lenb && buf != (అక్షరం*)0) {
                       /* 4: two words for the lengths */
#అయితే_ముగింపు
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_ఖండం(s, buf, stored_len, చివరి);

    } లేకపోతే ఒకవేళ (static_lenb == opt_lenb) {
        send_బిట్లు(s, (STATIC_TREES<<1) + చివరి, 3);
        compress_ఖండం(s, (స్థిరరాశి ct_దత్తాంశం *)static_ltree,
                       (స్థిరరాశి ct_దత్తాంశం *)static_dtree);
#నిర్వచితమైతే ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#అయితే_ముగింపు
    } లేకపోతే {
        send_బిట్లు(s, (DYN_TREES<<1) + చివరి, 3);
        send_all_trees(s, s->l_desc.max_సంకేతం + 1, s->d_desc.max_సంకేతం + 1,
                       max_blindex + 1);
        compress_ఖండం(s, (స్థిరరాశి ct_దత్తాంశం *)s->dyn_ltree,
                       (స్థిరరాశి ct_దత్తాంశం *)s->dyn_dtree);
#నిర్వచితమైతే ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#అయితే_ముగింపు
    }
    Assert (s->compressed_len == s->బిట్లు_పంపిన, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_ఖండం(s);

    ఒకవేళ (చివరి) {
        bi_windup(s);
#నిర్వచితమైతే ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#అయితే_ముగింపు
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)చివరి));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
పూర్ణాంకం ZLIB_అంతర్గత _tr_tally(deflate_state *s, గుర్తులేని dist, గుర్తులేని lc) {
#నిర్వచితమైతే LIT_MEM
    s->d_buf[s->sym_తదుపరి] = (ush)dist;
    s->l_buf[s->sym_తదుపరి++] = (uch)lc;
#లేకపోతే
    s->sym_buf[s->sym_తదుపరి++] = (uch)dist;
    s->sym_buf[s->sym_తదుపరి++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_తదుపరి++] = (uch)lc;
#అయితే_ముగింపు
    ఒకవేళ (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } లేకపోతే {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_సరిపోలిక-MIN_సరిపోలిక) &&
               (ush)d_సంకేతం(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[నిడివి_సంకేతం[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_సంకేతం(dist)].Freq++;
    }
    తిరిగి (s->sym_తదుపరి == s->sym_చివర);
}
