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

#अन्तर्भाव "deflate.h"

#यदि_परिभाषितम् ZLIB_DEBUG
#  include <ctype.h>
#यदि_अन्तः

/* ===========================================================================
 * Constants
 */

#परिभाषय MAX_BL_द्वयङ्काः 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#परिभाषय अन्तः_खण्डम् 256
/* end of block literal code */

#परिभाषय REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#परिभाषय REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#परिभाषय REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ध्रुव पूर्णाङ्क extra_lbits[दीर्घता_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local ध्रुव पूर्णाङ्क extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local ध्रुव पूर्णाङ्क extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local ध्रुव uch bl_क्रमः[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#परिभाषय DIST_संहिता_LEN  512 /* see definition of array dist_code below */

#यदि defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_दत्त static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_दत्त static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch dist_संहिता[DIST_संहिता_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch दीर्घता_संहिता[MAX_मेलनम्-MIN_मेलनम्+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local पूर्णाङ्क आधारः_दीर्घता[दीर्घता_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local पूर्णाङ्क आधारः_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#अन्यथा
#  include "trees.h"
#यदि_अन्तः /* defined(GEN_TREES_H) || !defined(STDC) */

संरचना static_वृक्षः_desc_s {
    ध्रुव ct_दत्त *static_वृक्षः;  /* static tree or NULL */
    ध्रुव intf *extra_द्वयङ्काः;      /* extra bits for each code or NULL */
    पूर्णाङ्क     extra_आधारः;          /* base index for extra_bits */
    पूर्णाङ्क     elems;               /* max number of elements in the tree */
    पूर्णाङ्क     max_दीर्घता;          /* max bit length for the codes */
};

#यदि_परिभाषितम् NO_INIT_GLOBAL_सूचकाः
#  define TCONST
#अन्यथा
#  define TCONST ध्रुव
#यदि_अन्तः

local TCONST static_वृक्षः_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_द्वयङ्काः};

local TCONST static_वृक्षः_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_द्वयङ्काः};

local TCONST static_वृक्षः_desc static_bl_desc =
{(ध्रुव ct_दत्त *)0, extra_blbits, 0,   BL_CODES, MAX_BL_द्वयङ्काः};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#परिभाषय put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local अचिह्नित bi_reverse(अचिह्नित संहिता, पूर्णाङ्क len) {
    अचिह्नित res = 0;
    कुरु {
        res |= संहिता & 1;
        संहिता >>= 1, res <<= 1;
    } यावत् (--len > 0);
    प्रत्यागम res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local शून्य bi_flush(deflate_state *s) {
    यदि (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } अन्यथा यदि (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local शून्य bi_windup(deflate_state *s) {
    यदि (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } अन्यथा यदि (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_प्रयुक्तम् = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#यदि_परिभाषितम् ZLIB_DEBUG
    s->द्वयङ्काः_प्रेषितम् = (s->द्वयङ्काः_प्रेषितम् + 7) & ~(ulg)7;
#यदि_अन्तः
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local शून्य gen_codes(ct_दत्त *वृक्षः, पूर्णाङ्क max_संहिता, ushf *bl_गणनम्) {
    ush अग्र_संहिता[MAX_द्वयङ्काः+1]; /* next code value for each bit length */
    अचिह्नित संहिता = 0;         /* running code value */
    पूर्णाङ्क द्वयङ्काः;                  /* bit index */
    पूर्णाङ्क n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    प्रति (द्वयङ्काः = 1; द्वयङ्काः <= MAX_द्वयङ्काः; द्वयङ्काः++) {
        संहिता = (संहिता + bl_गणनम्[द्वयङ्काः - 1]) << 1;
        अग्र_संहिता[द्वयङ्काः] = (ush)संहिता;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (संहिता + bl_गणनम्[MAX_द्वयङ्काः] - 1 == (1 << MAX_द्वयङ्काः) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_संहिता));

    प्रति (n = 0;  n <= max_संहिता; n++) {
        पूर्णाङ्क len = वृक्षः[n].Len;
        यदि (len == 0) अनुवर्त;
        /* Now reverse the bits */
        वृक्षः[n].संहिता_2 = (ush)bi_reverse(अग्र_संहिता[len]++, len);

        Tracecv(वृक्षः != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, वृक्षः[n].संहिता_2, अग्र_संहिता[len] - 1));
    }
}

#यदि_परिभाषितम् GEN_TREES_H
local शून्य gen_trees_शीर्षकम्(शून्य);
#यदि_अन्तः

#यदि_अपरिभाषितम् ZLIB_DEBUG
#  define send_संहिता(s, c, वृक्षः) send_द्वयङ्काः(s, वृक्षः[c].संहिता_2, वृक्षः[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#अन्यथा /* !ZLIB_DEBUG */
#  define send_संहिता(s, c, वृक्षः) \
     { यदि (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_द्वयङ्काः(s, वृक्षः[c].संहिता_2, वृक्षः[c].Len); }
#यदि_अन्तः

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#यदि_परिभाषितम् ZLIB_DEBUG
local शून्य send_द्वयङ्काः(deflate_state *s, पूर्णाङ्क मानम्, पूर्णाङ्क दीर्घता) {
    Tracevv((stderr," l %2d v %4x ", दीर्घता, मानम्));
    Assert(दीर्घता > 0 && दीर्घता <= 15, "invalid length");
    s->द्वयङ्काः_प्रेषितम् += (ulg)दीर्घता;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    यदि (s->bi_valid > (पूर्णाङ्क)Buf_आकार - दीर्घता) {
        s->bi_buf |= (ush)मानम् << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)मानम् >> (Buf_आकार - s->bi_valid);
        s->bi_valid += दीर्घता - Buf_आकार;
    } अन्यथा {
        s->bi_buf |= (ush)मानम् << s->bi_valid;
        s->bi_valid += दीर्घता;
    }
}
#अन्यथा /* !ZLIB_DEBUG */

#परिभाषय send_द्वयङ्काः(s, मानम्, दीर्घता) \
{ पूर्णाङ्क len = दीर्घता;\
  यदि (s->bi_valid > (पूर्णाङ्क)Buf_आकार - len) {\
    पूर्णाङ्क val = (पूर्णाङ्क)मानम्;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_आकार - s->bi_valid);\
    s->bi_valid += len - Buf_आकार;\
  } अन्यथा {\
    s->bi_buf |= (ush)(मानम्) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#यदि_अन्तः /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local शून्य tr_static_init(शून्य) {
#यदि defined(GEN_TREES_H) || !defined(STDC)
    स्थिर पूर्णाङ्क static_init_समाप्तम् = 0;
    पूर्णाङ्क n;        /* iterates over tree elements */
    पूर्णाङ्क द्वयङ्काः;     /* bit counter */
    पूर्णाङ्क दीर्घता;   /* length value */
    पूर्णाङ्क संहिता;     /* code value */
    पूर्णाङ्क dist;     /* distance index */
    ush bl_गणनम्[MAX_द्वयङ्काः+1];
    /* number of codes at each bit length for an optimal tree */

    यदि (static_init_समाप्तम्) प्रत्यागम;

    /* For some embedded targets, global variables are not initialized: */
#यदि_परिभाषितम् NO_INIT_GLOBAL_सूचकाः
    static_l_desc.static_वृक्षः = static_ltree;
    static_l_desc.extra_द्वयङ्काः = extra_lbits;
    static_d_desc.static_वृक्षः = static_dtree;
    static_d_desc.extra_द्वयङ्काः = extra_dbits;
    static_bl_desc.extra_द्वयङ्काः = extra_blbits;
#यदि_अन्तः

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    दीर्घता = 0;
    प्रति (संहिता = 0; संहिता < दीर्घता_CODES-1; संहिता++) {
        आधारः_दीर्घता[संहिता] = दीर्घता;
        प्रति (n = 0; n < (1 << extra_lbits[संहिता]); n++) {
            दीर्घता_संहिता[दीर्घता++] = (uch)संहिता;
        }
    }
    Assert (दीर्घता == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    दीर्घता_संहिता[दीर्घता - 1] = (uch)संहिता;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    प्रति (संहिता = 0 ; संहिता < 16; संहिता++) {
        आधारः_dist[संहिता] = dist;
        प्रति (n = 0; n < (1 << extra_dbits[संहिता]); n++) {
            dist_संहिता[dist++] = (uch)संहिता;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    प्रति ( ; संहिता < D_CODES; संहिता++) {
        आधारः_dist[संहिता] = dist << 7;
        प्रति (n = 0; n < (1 << (extra_dbits[संहिता] - 7)); n++) {
            dist_संहिता[256 + dist++] = (uch)संहिता;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    प्रति (द्वयङ्काः = 0; द्वयङ्काः <= MAX_द्वयङ्काः; द्वयङ्काः++) bl_गणनम्[द्वयङ्काः] = 0;
    n = 0;
    यावत् (n <= 143) static_ltree[n++].Len = 8, bl_गणनम्[8]++;
    यावत् (n <= 255) static_ltree[n++].Len = 9, bl_गणनम्[9]++;
    यावत् (n <= 279) static_ltree[n++].Len = 7, bl_गणनम्[7]++;
    यावत् (n <= 287) static_ltree[n++].Len = 8, bl_गणनम्[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_दत्त *)static_ltree, L_CODES+1, bl_गणनम्);

    /* The static distance tree is trivial: */
    प्रति (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].संहिता_2 = bi_reverse((अचिह्नित)n, 5);
    }
    static_init_समाप्तम् = 1;

#  ifdef GEN_TREES_H
    gen_trees_शीर्षकम्();
#  endif
#यदि_अन्तः /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#यदि_परिभाषितम् GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, चरमः, विस्तारः) \
      ((i) == (चरमः)? "\n};\n\n" :    \
       ((i) % (विस्तारः) == (विस्तारः) - 1 ? ",\n" : ", "))

शून्य gen_trees_शीर्षकम्(शून्य) {
    FILE *शीर्षकम् = fopen("trees.h", "w");
    पूर्णाङ्क i;

    Assert (शीर्षकम् != रिक्त, "Can't open trees.h");
    fprintf(शीर्षकम्,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(शीर्षकम्, "local const ct_data static_ltree[L_CODES+2] = {\n");
    प्रति (i = 0; i < L_CODES+2; i++) {
        fprintf(शीर्षकम्, "{{%3u},{%3u}}%s", static_ltree[i].संहिता_2,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(शीर्षकम्, "local const ct_data static_dtree[D_CODES] = {\n");
    प्रति (i = 0; i < D_CODES; i++) {
        fprintf(शीर्षकम्, "{{%2u},{%2u}}%s", static_dtree[i].संहिता_2,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(शीर्षकम्, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    प्रति (i = 0; i < DIST_संहिता_LEN; i++) {
        fprintf(शीर्षकम्, "%2u%s", dist_संहिता[i],
                SEPARATOR(i, DIST_संहिता_LEN-1, 20));
    }

    fprintf(शीर्षकम्,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    प्रति (i = 0; i < MAX_मेलनम्-MIN_मेलनम्+1; i++) {
        fprintf(शीर्षकम्, "%2u%s", दीर्घता_संहिता[i],
                SEPARATOR(i, MAX_मेलनम्-MIN_मेलनम्, 20));
    }

    fprintf(शीर्षकम्, "local const int base_length[LENGTH_CODES] = {\n");
    प्रति (i = 0; i < दीर्घता_CODES; i++) {
        fprintf(शीर्षकम्, "%1u%s", आधारः_दीर्घता[i],
                SEPARATOR(i, दीर्घता_CODES-1, 20));
    }

    fprintf(शीर्षकम्, "local const int base_dist[D_CODES] = {\n");
    प्रति (i = 0; i < D_CODES; i++) {
        fprintf(शीर्षकम्, "%5u%s", आधारः_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(शीर्षकम्);
}
#यदि_अन्तः /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local शून्य init_खण्डम्(deflate_state *s) {
    पूर्णाङ्क n; /* iterates over tree elements */

    /* Initialize the trees. */
    प्रति (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    प्रति (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    प्रति (n = 0; n < BL_CODES; n++) s->bl_वृक्षः[n].Freq = 0;

    s->dyn_ltree[अन्तः_खण्डम्].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_अग्र = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
शून्य ZLIB_आन्तरिकम् _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_वृक्षः = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_वृक्षः = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_वृक्षः = s->bl_वृक्षः;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_प्रयुक्तम् = 0;
#यदि_परिभाषितम् ZLIB_DEBUG
    s->compressed_len = 0L;
    s->द्वयङ्काः_प्रेषितम् = 0L;
#यदि_अन्तः

    /* Initialize the first block of the first file: */
    init_खण्डम्(s);
}

#परिभाषय SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#परिभाषय pqremove(s, वृक्षः, शिखरम्) \
{\
    शिखरम् = s->heap[SMALLEST]; \
    s->heap[SMALLEST] = s->heap[s->heap_len--]; \
    pqdownheap(s, वृक्षः, SMALLEST); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#परिभाषय smaller(वृक्षः, n, m, गभीरता) \
   (वृक्षः[n].Freq < वृक्षः[m].Freq || \
   (वृक्षः[n].Freq == वृक्षः[m].Freq && गभीरता[n] <= गभीरता[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local शून्य pqdownheap(deflate_state *s, ct_दत्त *वृक्षः, पूर्णाङ्क k) {
    पूर्णाङ्क v = s->heap[k];
    पूर्णाङ्क j = k << 1;  /* left son of k */
    यावत् (j <= s->heap_len) {
        /* Set j to the smallest of the two sons: */
        यदि (j < s->heap_len &&
            smaller(वृक्षः, s->heap[j + 1], s->heap[j], s->गभीरता)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        यदि (smaller(वृक्षः, v, s->heap[j], s->गभीरता)) भङ्ग;

        /* Exchange v with the smallest son */
        s->heap[k] = s->heap[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->heap[k] = v;
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
local शून्य gen_bitlen(deflate_state *s, वृक्षः_desc *desc) {
    ct_दत्त *वृक्षः        = desc->dyn_वृक्षः;
    पूर्णाङ्क max_संहिता         = desc->max_संहिता;
    ध्रुव ct_दत्त *stree = desc->stat_desc->static_वृक्षः;
    ध्रुव intf *extra    = desc->stat_desc->extra_द्वयङ्काः;
    पूर्णाङ्क आधारः_2             = desc->stat_desc->extra_आधारः;
    पूर्णाङ्क max_दीर्घता       = desc->stat_desc->max_दीर्घता;
    पूर्णाङ्क h;              /* heap index */
    पूर्णाङ्क n, m;           /* iterate over the tree elements */
    पूर्णाङ्क द्वयङ्काः;           /* bit length */
    पूर्णाङ्क xbits;          /* extra bits */
    ush f;              /* frequency */
    पूर्णाङ्क अतिप्रवाहः = 0;   /* number of elements with bit length too large */

    प्रति (द्वयङ्काः = 0; द्वयङ्काः <= MAX_द्वयङ्काः; द्वयङ्काः++) s->bl_गणनम्[द्वयङ्काः] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    वृक्षः[s->heap[s->heap_max]].Len = 0; /* root of the heap */

    प्रति (h = s->heap_max + 1; h < HEAP_आकार; h++) {
        n = s->heap[h];
        द्वयङ्काः = वृक्षः[वृक्षः[n].Dad].Len + 1;
        यदि (द्वयङ्काः > max_दीर्घता) द्वयङ्काः = max_दीर्घता, अतिप्रवाहः++;
        वृक्षः[n].Len = (ush)द्वयङ्काः;
        /* We overwrite tree[n].Dad which is no longer needed */

        यदि (n > max_संहिता) अनुवर्त; /* not a leaf node */

        s->bl_गणनम्[द्वयङ्काः]++;
        xbits = 0;
        यदि (n >= आधारः_2) xbits = extra[n - आधारः_2];
        f = वृक्षः[n].Freq;
        s->opt_len += (ulg)f * (अचिह्नित)(द्वयङ्काः + xbits);
        यदि (stree) s->static_len += (ulg)f * (अचिह्नित)(stree[n].Len + xbits);
    }
    यदि (अतिप्रवाहः == 0) प्रत्यागम;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    कुरु {
        द्वयङ्काः = max_दीर्घता - 1;
        यावत् (s->bl_गणनम्[द्वयङ्काः] == 0) द्वयङ्काः--;
        s->bl_गणनम्[द्वयङ्काः]--;        /* move one leaf down the tree */
        s->bl_गणनम्[द्वयङ्काः + 1] += 2; /* move one overflow item as its brother */
        s->bl_गणनम्[max_दीर्घता]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        अतिप्रवाहः -= 2;
    } यावत् (अतिप्रवाहः > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    प्रति (द्वयङ्काः = max_दीर्घता; द्वयङ्काः != 0; द्वयङ्काः--) {
        n = s->bl_गणनम्[द्वयङ्काः];
        यावत् (n != 0) {
            m = s->heap[--h];
            यदि (m > max_संहिता) अनुवर्त;
            यदि ((अचिह्नित) वृक्षः[m].Len != (अचिह्नित) द्वयङ्काः) {
                Tracev((stderr,"code %d bits %d->%d\n", m, वृक्षः[m].Len, द्वयङ्काः));
                s->opt_len += ((ulg)द्वयङ्काः - वृक्षः[m].Len) * वृक्षः[m].Freq;
                वृक्षः[m].Len = (ush)द्वयङ्काः;
            }
            n--;
        }
    }
}

#यदि_परिभाषितम् DUMP_BL_वृक्षः
#  include <stdio.h>
#यदि_अन्तः

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local शून्य build_वृक्षः(deflate_state *s, वृक्षः_desc *desc) {
    ct_दत्त *वृक्षः         = desc->dyn_वृक्षः;
    ध्रुव ct_दत्त *stree  = desc->stat_desc->static_वृक्षः;
    पूर्णाङ्क elems             = desc->stat_desc->elems;
    पूर्णाङ्क n, m;          /* iterate over heap elements */
    पूर्णाङ्क max_संहिता = -1; /* largest code with non zero frequency */
    पूर्णाङ्क ग्रन्थिः;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->heap_len = 0, s->heap_max = HEAP_आकार;

    प्रति (n = 0; n < elems; n++) {
        यदि (वृक्षः[n].Freq != 0) {
            s->heap[++(s->heap_len)] = max_संहिता = n;
            s->गभीरता[n] = 0;
        } अन्यथा {
            वृक्षः[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    यावत् (s->heap_len < 2) {
        ग्रन्थिः = s->heap[++(s->heap_len)] = (max_संहिता < 2 ? ++max_संहिता : 0);
        वृक्षः[ग्रन्थिः].Freq = 1;
        s->गभीरता[ग्रन्थिः] = 0;
        s->opt_len--; यदि (stree) s->static_len -= stree[ग्रन्थिः].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_संहिता = max_संहिता;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    प्रति (n = s->heap_len/2; n >= 1; n--) pqdownheap(s, वृक्षः, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    ग्रन्थिः = elems;              /* next internal node of the tree */
    कुरु {
        pqremove(s, वृक्षः, n);  /* n = node of least frequency */
        m = s->heap[SMALLEST]; /* m = node of next least frequency */

        s->heap[--(s->heap_max)] = n; /* keep the nodes sorted by frequency */
        s->heap[--(s->heap_max)] = m;

        /* Create a new node father of n and m */
        वृक्षः[ग्रन्थिः].Freq = वृक्षः[n].Freq + वृक्षः[m].Freq;
        s->गभीरता[ग्रन्थिः] = (uch)((s->गभीरता[n] >= s->गभीरता[m] ?
                                s->गभीरता[n] : s->गभीरता[m]) + 1);
        वृक्षः[n].Dad = वृक्षः[m].Dad = (ush)ग्रन्थिः;
#यदि_परिभाषितम् DUMP_BL_वृक्षः
        यदि (वृक्षः == s->bl_वृक्षः) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    ग्रन्थिः, वृक्षः[ग्रन्थिः].Freq, n, वृक्षः[n].Freq, m, वृक्षः[m].Freq);
        }
#यदि_अन्तः
        /* and insert the new node in the heap */
        s->heap[SMALLEST] = ग्रन्थिः++;
        pqdownheap(s, वृक्षः, SMALLEST);

    } यावत् (s->heap_len >= 2);

    s->heap[--(s->heap_max)] = s->heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (वृक्षः_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_दत्त *)वृक्षः, max_संहिता, s->bl_गणनम्);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local शून्य अवलोकनम्_वृक्षः(deflate_state *s, ct_दत्त *वृक्षः, पूर्णाङ्क max_संहिता) {
    पूर्णाङ्क n;                     /* iterates over all tree elements */
    पूर्णाङ्क prevlen = -1;          /* last emitted length */
    पूर्णाङ्क curlen;                /* length of current code */
    पूर्णाङ्क nextlen = वृक्षः[0].Len; /* length of next code */
    पूर्णाङ्क गणनम् = 0;             /* repeat count of the current code */
    पूर्णाङ्क max_गणनम् = 7;         /* max repeat count */
    पूर्णाङ्क min_गणनम् = 4;         /* min repeat count */

    यदि (nextlen == 0) max_गणनम् = 138, min_गणनम् = 3;
    वृक्षः[max_संहिता + 1].Len = (ush)0xffff; /* guard */

    प्रति (n = 0; n <= max_संहिता; n++) {
        curlen = nextlen; nextlen = वृक्षः[n + 1].Len;
        यदि (++गणनम् < max_गणनम् && curlen == nextlen) {
            अनुवर्त;
        } अन्यथा यदि (गणनम् < min_गणनम्) {
            s->bl_वृक्षः[curlen].Freq += (ush)गणनम्;
        } अन्यथा यदि (curlen != 0) {
            यदि (curlen != prevlen) s->bl_वृक्षः[curlen].Freq++;
            s->bl_वृक्षः[REP_3_6].Freq++;
        } अन्यथा यदि (गणनम् <= 10) {
            s->bl_वृक्षः[REPZ_3_10].Freq++;
        } अन्यथा {
            s->bl_वृक्षः[REPZ_11_138].Freq++;
        }
        गणनम् = 0; prevlen = curlen;
        यदि (nextlen == 0) {
            max_गणनम् = 138, min_गणनम् = 3;
        } अन्यथा यदि (curlen == nextlen) {
            max_गणनम् = 6, min_गणनम् = 3;
        } अन्यथा {
            max_गणनम् = 7, min_गणनम् = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local शून्य send_वृक्षः(deflate_state *s, ct_दत्त *वृक्षः, पूर्णाङ्क max_संहिता) {
    पूर्णाङ्क n;                     /* iterates over all tree elements */
    पूर्णाङ्क prevlen = -1;          /* last emitted length */
    पूर्णाङ्क curlen;                /* length of current code */
    पूर्णाङ्क nextlen = वृक्षः[0].Len; /* length of next code */
    पूर्णाङ्क गणनम् = 0;             /* repeat count of the current code */
    पूर्णाङ्क max_गणनम् = 7;         /* max repeat count */
    पूर्णाङ्क min_गणनम् = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    यदि (nextlen == 0) max_गणनम् = 138, min_गणनम् = 3;

    प्रति (n = 0; n <= max_संहिता; n++) {
        curlen = nextlen; nextlen = वृक्षः[n + 1].Len;
        यदि (++गणनम् < max_गणनम् && curlen == nextlen) {
            अनुवर्त;
        } अन्यथा यदि (गणनम् < min_गणनम्) {
            कुरु { send_संहिता(s, curlen, s->bl_वृक्षः); } यावत् (--गणनम् != 0);

        } अन्यथा यदि (curlen != 0) {
            यदि (curlen != prevlen) {
                send_संहिता(s, curlen, s->bl_वृक्षः); गणनम्--;
            }
            Assert(गणनम् >= 3 && गणनम् <= 6, " 3_6?");
            send_संहिता(s, REP_3_6, s->bl_वृक्षः); send_द्वयङ्काः(s, गणनम् - 3, 2);

        } अन्यथा यदि (गणनम् <= 10) {
            send_संहिता(s, REPZ_3_10, s->bl_वृक्षः); send_द्वयङ्काः(s, गणनम् - 3, 3);

        } अन्यथा {
            send_संहिता(s, REPZ_11_138, s->bl_वृक्षः); send_द्वयङ्काः(s, गणनम् - 11, 7);
        }
        गणनम् = 0; prevlen = curlen;
        यदि (nextlen == 0) {
            max_गणनम् = 138, min_गणनम् = 3;
        } अन्यथा यदि (curlen == nextlen) {
            max_गणनम् = 6, min_गणनम् = 3;
        } अन्यथा {
            max_गणनम् = 7, min_गणनम् = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local पूर्णाङ्क build_bl_वृक्षः(deflate_state *s) {
    पूर्णाङ्क max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    अवलोकनम्_वृक्षः(s, (ct_दत्त *)s->dyn_ltree, s->l_desc.max_संहिता);
    अवलोकनम्_वृक्षः(s, (ct_दत्त *)s->dyn_dtree, s->d_desc.max_संहिता);

    /* Build the bit length tree: */
    build_वृक्षः(s, (वृक्षः_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    प्रति (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        यदि (s->bl_वृक्षः[bl_क्रमः[max_blindex]].Len != 0) भङ्ग;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((stderr, "\ndyn trees: dyn %lu, stat %lu",
            s->opt_len, s->static_len));

    प्रत्यागम max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local शून्य send_all_trees(deflate_state *s, पूर्णाङ्क lcodes, पूर्णाङ्क dcodes,
                          पूर्णाङ्क blcodes) {
    पूर्णाङ्क rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_द्वयङ्काः(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_द्वयङ्काः(s, dcodes - 1,   5);
    send_द्वयङ्काः(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    प्रति (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_क्रमः[rank]));
        send_द्वयङ्काः(s, s->bl_वृक्षः[bl_क्रमः[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->द्वयङ्काः_प्रेषितम्));

    send_वृक्षः(s, (ct_दत्त *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->द्वयङ्काः_प्रेषितम्));

    send_वृक्षः(s, (ct_दत्त *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->द्वयङ्काः_प्रेषितम्));
}

/* ===========================================================================
 * Send a stored block
 */
शून्य ZLIB_आन्तरिकम् tr_stored_खण्डम्(deflate_state *s, charf *buf,
                                    ulg stored_len, पूर्णाङ्क चरमः) {
    send_द्वयङ्काः(s, (STORED_खण्डम्<<1) + चरमः, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    यदि (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#यदि_परिभाषितम् ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->द्वयङ्काः_प्रेषितम् += 2*16;
    s->द्वयङ्काः_प्रेषितम् += stored_len << 3;
#यदि_अन्तः
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
शून्य ZLIB_आन्तरिकम् tr_flush_द्वयङ्काः(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
शून्य ZLIB_आन्तरिकम् _tr_align(deflate_state *s) {
    send_द्वयङ्काः(s, STATIC_TREES<<1, 3);
    send_संहिता(s, अन्तः_खण्डम्, static_ltree);
#यदि_परिभाषितम् ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#यदि_अन्तः
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local शून्य compress_खण्डम्(deflate_state *s, ध्रुव ct_दत्त *ltree,
                          ध्रुव ct_दत्त *dtree) {
    अचिह्नित dist;      /* distance of matched string */
    पूर्णाङ्क lc;             /* match length or unmatched char (if dist == 0) */
    अचिह्नित sx = 0;    /* running index in symbol buffers */
    अचिह्नित संहिता;      /* the code to send */
    पूर्णाङ्क extra;          /* number of extra bits to send */

    यदि (s->sym_अग्र != 0) कुरु {
#यदि_परिभाषितम् LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#अन्यथा
        dist = s->sym_buf[sx++] & 0xff;
        dist += (अचिह्नित)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#यदि_अन्तः
        यदि (dist == 0) {
            send_संहिता(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } अन्यथा {
            /* Here, lc is the match length - MIN_MATCH */
            संहिता = दीर्घता_संहिता[lc];
            send_संहिता(s, संहिता + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[संहिता];
            यदि (extra != 0) {
                lc -= आधारः_दीर्घता[संहिता];
                send_द्वयङ्काः(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            संहिता = d_संहिता(dist);
            Assert (संहिता < D_CODES, "bad d_code");

            send_संहिता(s, संहिता, dtree);       /* send the distance code */
            extra = extra_dbits[संहिता];
            यदि (extra != 0) {
                dist -= (अचिह्नित)आधारः_dist[संहिता];
                send_द्वयङ्काः(s, (पूर्णाङ्क)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#यदि_परिभाषितम् LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#अन्यथा
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#यदि_अन्तः

    } यावत् (sx < s->sym_अग्र);

    send_संहिता(s, अन्तः_खण्डम्, ltree);
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
local पूर्णाङ्क detect_दत्त_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    अचिह्नित दीर्घ खण्डम्_आवरणम् = 0xf3ffc07fUL;
    पूर्णाङ्क n;

    /* Check for non-textual ("block-listed") bytes. */
    प्रति (n = 0; n <= 31; n++, खण्डम्_आवरणम् >>= 1)
        यदि ((खण्डम्_आवरणम् & 1) && (s->dyn_ltree[n].Freq != 0))
            प्रत्यागम Z_द्विमान;

    /* Check for textual ("allow-listed") bytes. */
    यदि (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        प्रत्यागम Z_TEXT;
    प्रति (n = 32; n < LITERALS; n++)
        यदि (s->dyn_ltree[n].Freq != 0)
            प्रत्यागम Z_TEXT;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    प्रत्यागम Z_द्विमान;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
शून्य ZLIB_आन्तरिकम् tr_flush_खण्डम्(deflate_state *s, charf *buf,
                                   ulg stored_len, पूर्णाङ्क चरमः) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    पूर्णाङ्क max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    यदि (s->level > 0) {

        /* Check if the file is binary or text */
        यदि (s->strm->दत्त_type == Z_UNKNOWN)
            s->strm->दत्त_type = detect_दत्त_type(s);

        /* Construct the literal and distance trees */
        build_वृक्षः(s, (वृक्षः_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_वृक्षः(s, (वृक्षः_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_वृक्षः(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_अग्र / 3));

#यदि_अपरिभाषितम् FORCE_STATIC
        यदि (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#यदि_अन्तः
            opt_lenb = static_lenb;

    } अन्यथा {
        Assert(buf != (वर्ण*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#यदि_परिभाषितम् FORCE_STORED
    यदि (buf != (वर्ण*)0) { /* force stored block */
#अन्यथा
    यदि (stored_len + 4 <= opt_lenb && buf != (वर्ण*)0) {
                       /* 4: two words for the lengths */
#यदि_अन्तः
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        tr_stored_खण्डम्(s, buf, stored_len, चरमः);

    } अन्यथा यदि (static_lenb == opt_lenb) {
        send_द्वयङ्काः(s, (STATIC_TREES<<1) + चरमः, 3);
        compress_खण्डम्(s, (ध्रुव ct_दत्त *)static_ltree,
                       (ध्रुव ct_दत्त *)static_dtree);
#यदि_परिभाषितम् ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#यदि_अन्तः
    } अन्यथा {
        send_द्वयङ्काः(s, (DYN_TREES<<1) + चरमः, 3);
        send_all_trees(s, s->l_desc.max_संहिता + 1, s->d_desc.max_संहिता + 1,
                       max_blindex + 1);
        compress_खण्डम्(s, (ध्रुव ct_दत्त *)s->dyn_ltree,
                       (ध्रुव ct_दत्त *)s->dyn_dtree);
#यदि_परिभाषितम् ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#यदि_अन्तः
    }
    Assert (s->compressed_len == s->द्वयङ्काः_प्रेषितम्, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_खण्डम्(s);

    यदि (चरमः) {
        bi_windup(s);
#यदि_परिभाषितम् ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#यदि_अन्तः
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)चरमः));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
पूर्णाङ्क ZLIB_आन्तरिकम् _tr_tally(deflate_state *s, अचिह्नित dist, अचिह्नित lc) {
#यदि_परिभाषितम् LIT_MEM
    s->d_buf[s->sym_अग्र] = (ush)dist;
    s->l_buf[s->sym_अग्र++] = (uch)lc;
#अन्यथा
    s->sym_buf[s->sym_अग्र++] = (uch)dist;
    s->sym_buf[s->sym_अग्र++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_अग्र++] = (uch)lc;
#यदि_अन्तः
    यदि (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } अन्यथा {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_मेलनम्-MIN_मेलनम्) &&
               (ush)d_संहिता(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[दीर्घता_संहिता[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_संहिता(dist)].Freq++;
    }
    प्रत्यागम (s->sym_अग्र == s->sym_अन्तः);
}
