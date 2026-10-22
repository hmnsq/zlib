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

#ଅନ୍ତର୍ଭାଵ "deflate.h"

#ifdef ZLIB_DEBUG
#  include <ctype.h>
#endif

/* ===========================================================================
 * Constants
 */

#define MAX_BL_BITS 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#define END_BLOCK 256
/* end of block literal code */

#define REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#define REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#define REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_lbits[LENGTH_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local ଅଚଳ uch bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#define DIST_CODE_LEN  512 /* see definition of array dist_code below */

#ଯଦି defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_data static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_data static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch _dist_code[DIST_CODE_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch _length_code[MAX_MATCH-MIN_MATCH+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା base_length[LENGTH_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା base_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#ଅନ୍ୟଥା
#  include "trees.h"
#endif /* defined(GEN_TREES_H) || !defined(STDC) */

ସଂରଚନା static_ବୃକ୍ଷ_desc_s {
    ଅଚଳ ct_data *static_ବୃକ୍ଷ;  /* static tree or NULL */
    ଅଚଳ intf *extra_bits;      /* extra bits for each code or NULL */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା     extra_base;          /* base index for extra_bits */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା     elems;               /* max number of elements in the tree */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା     max_length;          /* max bit length for the codes */
};

#ifdef NO_INIT_GLOBAL_POINTERS
#  define TCONST
#ଅନ୍ୟଥା
#  define TCONST ଅଚଳ
#endif

local TCONST static_ବୃକ୍ଷ_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS};

local TCONST static_ବୃକ୍ଷ_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS};

local TCONST static_ବୃକ୍ଷ_desc static_bl_desc =
{(ଅଚଳ ct_data *)0, extra_blbits, 0,   BL_CODES, MAX_BL_BITS};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#define put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local unsigned bi_reverse(unsigned code, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len) {
    unsigned res = 0;
    କର {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } ଯେତେବେଳେ (--len > 0);
    ଫେରନ୍ତୁ res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local ଶୂନ୍ୟ bi_flush(deflate_state *s) {
    ଯଦି (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } ଅନ୍ୟଥା ଯଦି (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local ଶୂନ୍ୟ bi_windup(deflate_state *s) {
    ଯଦି (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } ଅନ୍ୟଥା ଯଦି (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_used = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#ifdef ZLIB_DEBUG
    s->bits_sent = (s->bits_sent + 7) & ~(ulg)7;
#endif
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local ଶୂନ୍ୟ gen_codes(ct_data *ବୃକ୍ଷ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_code, ushf *bl_ଗଣନା) {
    ush next_code[MAX_BITS+1]; /* next code value for each bit length */
    unsigned code = 0;         /* running code value */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits;                  /* bit index */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    ପାଇଁ (bits = 1; bits <= MAX_BITS; bits++) {
        code = (code + bl_ଗଣନା[bits - 1]) << 1;
        next_code[bits] = (ush)code;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (code + bl_ଗଣନା[MAX_BITS] - 1 == (1 << MAX_BITS) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

    ପାଇଁ (n = 0;  n <= max_code; n++) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = ବୃକ୍ଷ[n].Len;
        ଯଦି (len == 0) ଜାରି;
        /* Now reverse the bits */
        ବୃକ୍ଷ[n].Code = (ush)bi_reverse(next_code[len]++, len);

        Tracecv(ବୃକ୍ଷ != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, ବୃକ୍ଷ[n].Code, next_code[len] - 1));
    }
}

#ifdef GEN_TREES_H
local ଶୂନ୍ୟ gen_trees_header(ଶୂନ୍ୟ);
#endif

#ifndef ZLIB_DEBUG
#  define send_code(s, c, ବୃକ୍ଷ) send_bits(s, ବୃକ୍ଷ[c].Code, ବୃକ୍ଷ[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#ଅନ୍ୟଥା /* !ZLIB_DEBUG */
#  define send_code(s, c, ବୃକ୍ଷ) \
     { ଯଦି (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_bits(s, ବୃକ୍ଷ[c].Code, ବୃକ୍ଷ[c].Len); }
#endif

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#ifdef ZLIB_DEBUG
local ଶୂନ୍ୟ send_bits(deflate_state *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା length) {
    Tracevv((stderr," l %2d v %4x ", length, ମୂଲ୍ୟ));
    Assert(length > 0 && length <= 15, "invalid length");
    s->bits_sent += (ulg)length;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    ଯଦି (s->bi_valid > (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)Buf_size - length) {
        s->bi_buf |= (ush)ମୂଲ୍ୟ << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)ମୂଲ୍ୟ >> (Buf_size - s->bi_valid);
        s->bi_valid += length - Buf_size;
    } ଅନ୍ୟଥା {
        s->bi_buf |= (ush)ମୂଲ୍ୟ << s->bi_valid;
        s->bi_valid += length;
    }
}
#ଅନ୍ୟଥା /* !ZLIB_DEBUG */

#define send_bits(s, ମୂଲ୍ୟ, length) \
{ ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = length;\
  ଯଦି (s->bi_valid > (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)Buf_size - len) {\
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା val = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ମୂଲ୍ୟ;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_size - s->bi_valid);\
    s->bi_valid += len - Buf_size;\
  } ଅନ୍ୟଥା {\
    s->bi_buf |= (ush)(ମୂଲ୍ୟ) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#endif /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local ଶୂନ୍ୟ tr_static_init(ଶୂନ୍ୟ) {
#ଯଦି defined(GEN_TREES_H) || !defined(STDC)
    ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା static_init_done = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;        /* iterates over tree elements */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits;     /* bit counter */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା length;   /* length value */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା code;     /* code value */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା dist;     /* distance index */
    ush bl_ଗଣନା[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */

    ଯଦି (static_init_done) ଫେରନ୍ତୁ;

    /* For some embedded targets, global variables are not initialized: */
#ifdef NO_INIT_GLOBAL_POINTERS
    static_l_desc.static_ବୃକ୍ଷ = static_ltree;
    static_l_desc.extra_bits = extra_lbits;
    static_d_desc.static_ବୃକ୍ଷ = static_dtree;
    static_d_desc.extra_bits = extra_dbits;
    static_bl_desc.extra_bits = extra_blbits;
#endif

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    ପାଇଁ (code = 0; code < LENGTH_CODES-1; code++) {
        base_length[code] = length;
        ପାଇଁ (n = 0; n < (1 << extra_lbits[code]); n++) {
            _length_code[length++] = (uch)code;
        }
    }
    Assert (length == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    _length_code[length - 1] = (uch)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    ପାଇଁ (code = 0 ; code < 16; code++) {
        base_dist[code] = dist;
        ପାଇଁ (n = 0; n < (1 << extra_dbits[code]); n++) {
            _dist_code[dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    ପାଇଁ ( ; code < D_CODES; code++) {
        base_dist[code] = dist << 7;
        ପାଇଁ (n = 0; n < (1 << (extra_dbits[code] - 7)); n++) {
            _dist_code[256 + dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    ପାଇଁ (bits = 0; bits <= MAX_BITS; bits++) bl_ଗଣନା[bits] = 0;
    n = 0;
    ଯେତେବେଳେ (n <= 143) static_ltree[n++].Len = 8, bl_ଗଣନା[8]++;
    ଯେତେବେଳେ (n <= 255) static_ltree[n++].Len = 9, bl_ଗଣନା[9]++;
    ଯେତେବେଳେ (n <= 279) static_ltree[n++].Len = 7, bl_ଗଣନା[7]++;
    ଯେତେବେଳେ (n <= 287) static_ltree[n++].Len = 8, bl_ଗଣନା[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_data *)static_ltree, L_CODES+1, bl_ଗଣନା);

    /* The static distance tree is trivial: */
    ପାଇଁ (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].Code = bi_reverse((unsigned)n, 5);
    }
    static_init_done = 1;

#  ifdef GEN_TREES_H
    gen_trees_header();
#  endif
#endif /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#ifdef GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, last, width) \
      ((i) == (last)? "\n};\n\n" :    \
       ((i) % (width) == (width) - 1 ? ",\n" : ", "))

ଶୂନ୍ୟ gen_trees_header(ଶୂନ୍ୟ) {
    FILE *header = fopen("trees.h", "w");
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    Assert (header != ଖାଲି, "Can't open trees.h");
    fprintf(header,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(header, "local const ct_data static_ltree[L_CODES+2] = {\n");
    ପାଇଁ (i = 0; i < L_CODES+2; i++) {
        fprintf(header, "{{%3u},{%3u}}%s", static_ltree[i].Code,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(header, "local const ct_data static_dtree[D_CODES] = {\n");
    ପାଇଁ (i = 0; i < D_CODES; i++) {
        fprintf(header, "{{%2u},{%2u}}%s", static_dtree[i].Code,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(header, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    ପାଇଁ (i = 0; i < DIST_CODE_LEN; i++) {
        fprintf(header, "%2u%s", _dist_code[i],
                SEPARATOR(i, DIST_CODE_LEN-1, 20));
    }

    fprintf(header,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    ପାଇଁ (i = 0; i < MAX_MATCH-MIN_MATCH+1; i++) {
        fprintf(header, "%2u%s", _length_code[i],
                SEPARATOR(i, MAX_MATCH-MIN_MATCH, 20));
    }

    fprintf(header, "local const int base_length[LENGTH_CODES] = {\n");
    ପାଇଁ (i = 0; i < LENGTH_CODES; i++) {
        fprintf(header, "%1u%s", base_length[i],
                SEPARATOR(i, LENGTH_CODES-1, 20));
    }

    fprintf(header, "local const int base_dist[D_CODES] = {\n");
    ପାଇଁ (i = 0; i < D_CODES; i++) {
        fprintf(header, "%5u%s", base_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(header);
}
#endif /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local ଶୂନ୍ୟ init_block(deflate_state *s) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n; /* iterates over tree elements */

    /* Initialize the trees. */
    ପାଇଁ (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    ପାଇଁ (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    ପାଇଁ (n = 0; n < BL_CODES; n++) s->bl_ବୃକ୍ଷ[n].Freq = 0;

    s->dyn_ltree[END_BLOCK].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_next = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
ଶୂନ୍ୟ ZLIB_INTERNAL _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_ବୃକ୍ଷ = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_ବୃକ୍ଷ = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_ବୃକ୍ଷ = s->bl_ବୃକ୍ଷ;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_used = 0;
#ifdef ZLIB_DEBUG
    s->compressed_len = 0L;
    s->bits_sent = 0L;
#endif

    /* Initialize the first block of the first file: */
    init_block(s);
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#define pqremove(s, ବୃକ୍ଷ, ଶୀର୍ଷ) \
{\
    ଶୀର୍ଷ = s->heap[SMALLEST]; \
    s->heap[SMALLEST] = s->heap[s->heap_len--]; \
    pqdownheap(s, ବୃକ୍ଷ, SMALLEST); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define smaller(ବୃକ୍ଷ, n, m, depth) \
   (ବୃକ୍ଷ[n].Freq < ବୃକ୍ଷ[m].Freq || \
   (ବୃକ୍ଷ[n].Freq == ବୃକ୍ଷ[m].Freq && depth[n] <= depth[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local ଶୂନ୍ୟ pqdownheap(deflate_state *s, ct_data *ବୃକ୍ଷ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା k) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = s->heap[k];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = k << 1;  /* left son of k */
    ଯେତେବେଳେ (j <= s->heap_len) {
        /* Set j to the smallest of the two sons: */
        ଯଦି (j < s->heap_len &&
            smaller(ବୃକ୍ଷ, s->heap[j + 1], s->heap[j], s->depth)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        ଯଦି (smaller(ବୃକ୍ଷ, v, s->heap[j], s->depth)) ଭାଙ୍ଗ;

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
local ଶୂନ୍ୟ gen_bitlen(deflate_state *s, ବୃକ୍ଷ_desc *desc) {
    ct_data *ବୃକ୍ଷ        = desc->dyn_ବୃକ୍ଷ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_code         = desc->max_code;
    ଅଚଳ ct_data *stree = desc->stat_desc->static_ବୃକ୍ଷ;
    ଅଚଳ intf *extra    = desc->stat_desc->extra_bits;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା base             = desc->stat_desc->extra_base;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_length       = desc->stat_desc->max_length;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା h;              /* heap index */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, m;           /* iterate over the tree elements */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits;           /* bit length */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା xbits;          /* extra bits */
    ush f;              /* frequency */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା overflow = 0;   /* number of elements with bit length too large */

    ପାଇଁ (bits = 0; bits <= MAX_BITS; bits++) s->bl_ଗଣନା[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    ବୃକ୍ଷ[s->heap[s->heap_max]].Len = 0; /* root of the heap */

    ପାଇଁ (h = s->heap_max + 1; h < HEAP_SIZE; h++) {
        n = s->heap[h];
        bits = ବୃକ୍ଷ[ବୃକ୍ଷ[n].Dad].Len + 1;
        ଯଦି (bits > max_length) bits = max_length, overflow++;
        ବୃକ୍ଷ[n].Len = (ush)bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        ଯଦି (n > max_code) ଜାରି; /* not a leaf node */

        s->bl_ଗଣନା[bits]++;
        xbits = 0;
        ଯଦି (n >= base) xbits = extra[n - base];
        f = ବୃକ୍ଷ[n].Freq;
        s->opt_len += (ulg)f * (unsigned)(bits + xbits);
        ଯଦି (stree) s->static_len += (ulg)f * (unsigned)(stree[n].Len + xbits);
    }
    ଯଦି (overflow == 0) ଫେରନ୍ତୁ;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    କର {
        bits = max_length - 1;
        ଯେତେବେଳେ (s->bl_ଗଣନା[bits] == 0) bits--;
        s->bl_ଗଣନା[bits]--;        /* move one leaf down the tree */
        s->bl_ଗଣନା[bits + 1] += 2; /* move one overflow item as its brother */
        s->bl_ଗଣନା[max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } ଯେତେବେଳେ (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    ପାଇଁ (bits = max_length; bits != 0; bits--) {
        n = s->bl_ଗଣନା[bits];
        ଯେତେବେଳେ (n != 0) {
            m = s->heap[--h];
            ଯଦି (m > max_code) ଜାରି;
            ଯଦି ((unsigned) ବୃକ୍ଷ[m].Len != (unsigned) bits) {
                Tracev((stderr,"code %d bits %d->%d\n", m, ବୃକ୍ଷ[m].Len, bits));
                s->opt_len += ((ulg)bits - ବୃକ୍ଷ[m].Len) * ବୃକ୍ଷ[m].Freq;
                ବୃକ୍ଷ[m].Len = (ush)bits;
            }
            n--;
        }
    }
}

#ifdef DUMP_BL_ବୃକ୍ଷ
#  include <stdio.h>
#endif

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local ଶୂନ୍ୟ build_ବୃକ୍ଷ(deflate_state *s, ବୃକ୍ଷ_desc *desc) {
    ct_data *ବୃକ୍ଷ         = desc->dyn_ବୃକ୍ଷ;
    ଅଚଳ ct_data *stree  = desc->stat_desc->static_ବୃକ୍ଷ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା elems             = desc->stat_desc->elems;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, m;          /* iterate over heap elements */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_code = -1; /* largest code with non zero frequency */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->heap_len = 0, s->heap_max = HEAP_SIZE;

    ପାଇଁ (n = 0; n < elems; n++) {
        ଯଦି (ବୃକ୍ଷ[n].Freq != 0) {
            s->heap[++(s->heap_len)] = max_code = n;
            s->depth[n] = 0;
        } ଅନ୍ୟଥା {
            ବୃକ୍ଷ[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    ଯେତେବେଳେ (s->heap_len < 2) {
        ଗଣ୍ଠି = s->heap[++(s->heap_len)] = (max_code < 2 ? ++max_code : 0);
        ବୃକ୍ଷ[ଗଣ୍ଠି].Freq = 1;
        s->depth[ଗଣ୍ଠି] = 0;
        s->opt_len--; ଯଦି (stree) s->static_len -= stree[ଗଣ୍ଠି].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    ପାଇଁ (n = s->heap_len/2; n >= 1; n--) pqdownheap(s, ବୃକ୍ଷ, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    ଗଣ୍ଠି = elems;              /* next internal node of the tree */
    କର {
        pqremove(s, ବୃକ୍ଷ, n);  /* n = node of least frequency */
        m = s->heap[SMALLEST]; /* m = node of next least frequency */

        s->heap[--(s->heap_max)] = n; /* keep the nodes sorted by frequency */
        s->heap[--(s->heap_max)] = m;

        /* Create a new node father of n and m */
        ବୃକ୍ଷ[ଗଣ୍ଠି].Freq = ବୃକ୍ଷ[n].Freq + ବୃକ୍ଷ[m].Freq;
        s->depth[ଗଣ୍ଠି] = (uch)((s->depth[n] >= s->depth[m] ?
                                s->depth[n] : s->depth[m]) + 1);
        ବୃକ୍ଷ[n].Dad = ବୃକ୍ଷ[m].Dad = (ush)ଗଣ୍ଠି;
#ifdef DUMP_BL_ବୃକ୍ଷ
        ଯଦି (ବୃକ୍ଷ == s->bl_ବୃକ୍ଷ) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    ଗଣ୍ଠି, ବୃକ୍ଷ[ଗଣ୍ଠି].Freq, n, ବୃକ୍ଷ[n].Freq, m, ବୃକ୍ଷ[m].Freq);
        }
#endif
        /* and insert the new node in the heap */
        s->heap[SMALLEST] = ଗଣ୍ଠି++;
        pqdownheap(s, ବୃକ୍ଷ, SMALLEST);

    } ଯେତେବେଳେ (s->heap_len >= 2);

    s->heap[--(s->heap_max)] = s->heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (ବୃକ୍ଷ_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_data *)ବୃକ୍ଷ, max_code, s->bl_ଗଣନା);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local ଶୂନ୍ୟ scan_ବୃକ୍ଷ(deflate_state *s, ct_data *ବୃକ୍ଷ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_code) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;                     /* iterates over all tree elements */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prevlen = -1;          /* last emitted length */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା curlen;                /* length of current code */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextlen = ବୃକ୍ଷ[0].Len; /* length of next code */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;             /* repeat count of the current code */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_ଗଣନା = 7;         /* max repeat count */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_ଗଣନା = 4;         /* min repeat count */

    ଯଦି (nextlen == 0) max_ଗଣନା = 138, min_ଗଣନା = 3;
    ବୃକ୍ଷ[max_code + 1].Len = (ush)0xffff; /* guard */

    ପାଇଁ (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = ବୃକ୍ଷ[n + 1].Len;
        ଯଦି (++ଗଣନା_2 < max_ଗଣନା && curlen == nextlen) {
            ଜାରି;
        } ଅନ୍ୟଥା ଯଦି (ଗଣନା_2 < min_ଗଣନା) {
            s->bl_ବୃକ୍ଷ[curlen].Freq += (ush)ଗଣନା_2;
        } ଅନ୍ୟଥା ଯଦି (curlen != 0) {
            ଯଦି (curlen != prevlen) s->bl_ବୃକ୍ଷ[curlen].Freq++;
            s->bl_ବୃକ୍ଷ[REP_3_6].Freq++;
        } ଅନ୍ୟଥା ଯଦି (ଗଣନା_2 <= 10) {
            s->bl_ବୃକ୍ଷ[REPZ_3_10].Freq++;
        } ଅନ୍ୟଥା {
            s->bl_ବୃକ୍ଷ[REPZ_11_138].Freq++;
        }
        ଗଣନା_2 = 0; prevlen = curlen;
        ଯଦି (nextlen == 0) {
            max_ଗଣନା = 138, min_ଗଣନା = 3;
        } ଅନ୍ୟଥା ଯଦି (curlen == nextlen) {
            max_ଗଣନା = 6, min_ଗଣନା = 3;
        } ଅନ୍ୟଥା {
            max_ଗଣନା = 7, min_ଗଣନା = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local ଶୂନ୍ୟ send_ବୃକ୍ଷ(deflate_state *s, ct_data *ବୃକ୍ଷ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_code) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;                     /* iterates over all tree elements */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prevlen = -1;          /* last emitted length */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା curlen;                /* length of current code */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextlen = ବୃକ୍ଷ[0].Len; /* length of next code */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;             /* repeat count of the current code */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_ଗଣନା = 7;         /* max repeat count */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_ଗଣନା = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    ଯଦି (nextlen == 0) max_ଗଣନା = 138, min_ଗଣନା = 3;

    ପାଇଁ (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = ବୃକ୍ଷ[n + 1].Len;
        ଯଦି (++ଗଣନା_2 < max_ଗଣନା && curlen == nextlen) {
            ଜାରି;
        } ଅନ୍ୟଥା ଯଦି (ଗଣନା_2 < min_ଗଣନା) {
            କର { send_code(s, curlen, s->bl_ବୃକ୍ଷ); } ଯେତେବେଳେ (--ଗଣନା_2 != 0);

        } ଅନ୍ୟଥା ଯଦି (curlen != 0) {
            ଯଦି (curlen != prevlen) {
                send_code(s, curlen, s->bl_ବୃକ୍ଷ); ଗଣନା_2--;
            }
            Assert(ଗଣନା_2 >= 3 && ଗଣନା_2 <= 6, " 3_6?");
            send_code(s, REP_3_6, s->bl_ବୃକ୍ଷ); send_bits(s, ଗଣନା_2 - 3, 2);

        } ଅନ୍ୟଥା ଯଦି (ଗଣନା_2 <= 10) {
            send_code(s, REPZ_3_10, s->bl_ବୃକ୍ଷ); send_bits(s, ଗଣନା_2 - 3, 3);

        } ଅନ୍ୟଥା {
            send_code(s, REPZ_11_138, s->bl_ବୃକ୍ଷ); send_bits(s, ଗଣନା_2 - 11, 7);
        }
        ଗଣନା_2 = 0; prevlen = curlen;
        ଯଦି (nextlen == 0) {
            max_ଗଣନା = 138, min_ଗଣନା = 3;
        } ଅନ୍ୟଥା ଯଦି (curlen == nextlen) {
            max_ଗଣନା = 6, min_ଗଣନା = 3;
        } ଅନ୍ୟଥା {
            max_ଗଣନା = 7, min_ଗଣନା = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା build_bl_ବୃକ୍ଷ(deflate_state *s) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_ବୃକ୍ଷ(s, (ct_data *)s->dyn_ltree, s->l_desc.max_code);
    scan_ବୃକ୍ଷ(s, (ct_data *)s->dyn_dtree, s->d_desc.max_code);

    /* Build the bit length tree: */
    build_ବୃକ୍ଷ(s, (ବୃକ୍ଷ_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    ପାଇଁ (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        ଯଦି (s->bl_ବୃକ୍ଷ[bl_order[max_blindex]].Len != 0) ଭାଙ୍ଗ;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((stderr, "\ndyn trees: dyn %lu, stat %lu",
            s->opt_len, s->static_len));

    ଫେରନ୍ତୁ max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local ଶୂନ୍ୟ send_all_trees(deflate_state *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lcodes, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dcodes,
                          ପୂର୍ଣ୍ଣସଂଖ୍ୟା blcodes) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_bits(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_bits(s, dcodes - 1,   5);
    send_bits(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    ପାଇଁ (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
        send_bits(s, s->bl_ବୃକ୍ଷ[bl_order[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %lu", s->bits_sent));

    send_ବୃକ୍ଷ(s, (ct_data *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %lu", s->bits_sent));

    send_ବୃକ୍ଷ(s, (ct_data *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %lu", s->bits_sent));
}

/* ===========================================================================
 * Send a stored block
 */
ଶୂନ୍ୟ ZLIB_INTERNAL _tr_stored_block(deflate_state *s, charf *buf,
                                    ulg stored_len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା last) {
    send_bits(s, (STORED_BLOCK<<1) + last, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    ଯଦି (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#ifdef ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->bits_sent += 2*16;
    s->bits_sent += stored_len << 3;
#endif
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
ଶୂନ୍ୟ ZLIB_INTERNAL _tr_flush_bits(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
ଶୂନ୍ୟ ZLIB_INTERNAL _tr_align(deflate_state *s) {
    send_bits(s, STATIC_TREES<<1, 3);
    send_code(s, END_BLOCK, static_ltree);
#ifdef ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#endif
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local ଶୂନ୍ୟ compress_block(deflate_state *s, ଅଚଳ ct_data *ltree,
                          ଅଚଳ ct_data *dtree) {
    unsigned dist;      /* distance of matched string */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lc;             /* match length or unmatched char (if dist == 0) */
    unsigned sx = 0;    /* running index in symbol buffers */
    unsigned code;      /* the code to send */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra;          /* number of extra bits to send */

    ଯଦି (s->sym_next != 0) କର {
#ifdef LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#ଅନ୍ୟଥା
        dist = s->sym_buf[sx++] & 0xff;
        dist += (unsigned)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#endif
        ଯଦି (dist == 0) {
            send_code(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } ଅନ୍ୟଥା {
            /* Here, lc is the match length - MIN_MATCH */
            code = _length_code[lc];
            send_code(s, code + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[code];
            ଯଦି (extra != 0) {
                lc -= base_length[code];
                send_bits(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            code = d_code(dist);
            Assert (code < D_CODES, "bad d_code");

            send_code(s, code, dtree);       /* send the distance code */
            extra = extra_dbits[code];
            ଯଦି (extra != 0) {
                dist -= (unsigned)base_dist[code];
                send_bits(s, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#ifdef LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#ଅନ୍ୟଥା
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#endif

    } ଯେତେବେଳେ (sx < s->sym_next);

    send_code(s, END_BLOCK, ltree);
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
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା detect_data_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    unsigned long block_mask = 0xf3ffc07fUL;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

    /* Check for non-textual ("block-listed") bytes. */
    ପାଇଁ (n = 0; n <= 31; n++, block_mask >>= 1)
        ଯଦି ((block_mask & 1) && (s->dyn_ltree[n].Freq != 0))
            ଫେରନ୍ତୁ Z_ଦ୍ୱିମିକ;

    /* Check for textual ("allow-listed") bytes. */
    ଯଦି (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        ଫେରନ୍ତୁ Z_TEXT;
    ପାଇଁ (n = 32; n < LITERALS; n++)
        ଯଦି (s->dyn_ltree[n].Freq != 0)
            ଫେରନ୍ତୁ Z_TEXT;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    ଫେରନ୍ତୁ Z_ଦ୍ୱିମିକ;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
ଶୂନ୍ୟ ZLIB_INTERNAL _tr_flush_block(deflate_state *s, charf *buf,
                                   ulg stored_len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା last) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    ଯଦି (s->level > 0) {

        /* Check if the file is binary or text */
        ଯଦି (s->strm->data_type == Z_UNKNOWN)
            s->strm->data_type = detect_data_type(s);

        /* Construct the literal and distance trees */
        build_ବୃକ୍ଷ(s, (ବୃକ୍ଷ_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_ବୃକ୍ଷ(s, (ବୃକ୍ଷ_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_ବୃକ୍ଷ(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_next / 3));

#ifndef FORCE_STATIC
        ଯଦି (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#endif
            opt_lenb = static_lenb;

    } ଅନ୍ୟଥା {
        Assert(buf != (ଅକ୍ଷର*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#ifdef FORCE_STORED
    ଯଦି (buf != (ଅକ୍ଷର*)0) { /* force stored block */
#ଅନ୍ୟଥା
    ଯଦି (stored_len + 4 <= opt_lenb && buf != (ଅକ୍ଷର*)0) {
                       /* 4: two words for the lengths */
#endif
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        _tr_stored_block(s, buf, stored_len, last);

    } ଅନ୍ୟଥା ଯଦି (static_lenb == opt_lenb) {
        send_bits(s, (STATIC_TREES<<1) + last, 3);
        compress_block(s, (ଅଚଳ ct_data *)static_ltree,
                       (ଅଚଳ ct_data *)static_dtree);
#ifdef ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#endif
    } ଅନ୍ୟଥା {
        send_bits(s, (DYN_TREES<<1) + last, 3);
        send_all_trees(s, s->l_desc.max_code + 1, s->d_desc.max_code + 1,
                       max_blindex + 1);
        compress_block(s, (ଅଚଳ ct_data *)s->dyn_ltree,
                       (ଅଚଳ ct_data *)s->dyn_dtree);
#ifdef ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#endif
    }
    Assert (s->compressed_len == s->bits_sent, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_block(s);

    ଯଦି (last) {
        bi_windup(s);
#ifdef ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#endif
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)last));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZLIB_INTERNAL _tr_tally(deflate_state *s, unsigned dist, unsigned lc) {
#ifdef LIT_MEM
    s->d_buf[s->sym_next] = (ush)dist;
    s->l_buf[s->sym_next++] = (uch)lc;
#ଅନ୍ୟଥା
    s->sym_buf[s->sym_next++] = (uch)dist;
    s->sym_buf[s->sym_next++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_next++] = (uch)lc;
#endif
    ଯଦି (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } ଅନ୍ୟଥା {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_MATCH-MIN_MATCH) &&
               (ush)d_code(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[_length_code[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_code(dist)].Freq++;
    }
    ଫେରନ୍ତୁ (s->sym_next == s->sym_end);
}
