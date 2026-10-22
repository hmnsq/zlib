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

#समावेश "deflate.h"

#अगर_घोषित ZLIB_DEBUG
#  include <ctype.h>
#पूर्ण_अगर

/* ===========================================================================
 * Constants
 */

#घोषणा MAX_BL_BITS 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#घोषणा END_BLOCK 256
/* end of block literal code */

#घोषणा REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#घोषणा REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#घोषणा REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local स्थिर पूर्णांक extra_lbits[लंबाई_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local स्थिर पूर्णांक extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local स्थिर पूर्णांक extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local स्थिर uch bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#घोषणा DIST_CODE_LEN  512 /* see definition of array dist_code below */

#अगर defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_तथ्य static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_तथ्य static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch _dist_code[DIST_CODE_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch लंबाई_code[MAX_MATCH-MIN_MATCH+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local पूर्णांक base_लंबाई[लंबाई_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local पूर्णांक base_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#अन्यथा
#  include "trees.h"
#पूर्ण_अगर /* defined(GEN_TREES_H) || !defined(STDC) */

काष्ठा static_वृक्ष_desc_s {
    स्थिर ct_तथ्य *static_वृक्ष;  /* static tree or NULL */
    स्थिर intf *extra_bits;      /* extra bits for each code or NULL */
    पूर्णांक     extra_base;          /* base index for extra_bits */
    पूर्णांक     elems;               /* max number of elements in the tree */
    पूर्णांक     max_लंबाई;          /* max bit length for the codes */
};

#अगर_घोषित NO_INIT_GLOBAL_POINTERS
#  define TCONST
#अन्यथा
#  define TCONST स्थिर
#पूर्ण_अगर

local TCONST static_वृक्ष_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS};

local TCONST static_वृक्ष_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS};

local TCONST static_वृक्ष_desc static_bl_desc =
{(स्थिर ct_तथ्य *)0, extra_blbits, 0,   BL_CODES, MAX_BL_BITS};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#घोषणा put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local अचिन्हित bi_reverse(अचिन्हित code, पूर्णांक len) {
    अचिन्हित res = 0;
    करो {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } जबतक (--len > 0);
    वापस res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local व्योम bi_flush(deflate_state *s) {
    अगर (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } अन्यथा अगर (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local व्योम bi_windup(deflate_state *s) {
    अगर (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } अन्यथा अगर (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_used = ((s->bi_valid - 1) & 7) + 1;
    s->bi_buf = 0;
    s->bi_valid = 0;
#अगर_घोषित ZLIB_DEBUG
    s->bits_sent = (s->bits_sent + 7) & ~(ulg)7;
#पूर्ण_अगर
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local व्योम gen_codes(ct_तथ्य *वृक्ष, पूर्णांक max_code, ushf *bl_गिनती) {
    ush अग्र_code[MAX_BITS+1]; /* next code value for each bit length */
    अचिन्हित code = 0;         /* running code value */
    पूर्णांक bits;                  /* bit index */
    पूर्णांक n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    क्रम (bits = 1; bits <= MAX_BITS; bits++) {
        code = (code + bl_गिनती[bits - 1]) << 1;
        अग्र_code[bits] = (ush)code;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (code + bl_गिनती[MAX_BITS] - 1 == (1 << MAX_BITS) - 1,
            "inconsistent bit counts");
    Tracev((मानक_त्रुटि,"\ngen_codes: max_code %d ", max_code));

    क्रम (n = 0;  n <= max_code; n++) {
        पूर्णांक len = वृक्ष[n].Len;
        अगर (len == 0) जारी;
        /* Now reverse the bits */
        वृक्ष[n].Code = (ush)bi_reverse(अग्र_code[len]++, len);

        Tracecv(वृक्ष != static_ltree, (मानक_त्रुटि,"\nn %3d %c l %2d c %4x (%x) ",
            n, (है_चित्र(n) ? n : ' '), len, वृक्ष[n].Code, अग्र_code[len] - 1));
    }
}

#अगर_घोषित GEN_TREES_H
local व्योम gen_trees_header(व्योम);
#पूर्ण_अगर

#अगर_अघोषित ZLIB_DEBUG
#  define send_code(s, c, वृक्ष) send_bits(s, वृक्ष[c].Code, वृक्ष[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#अन्यथा /* !ZLIB_DEBUG */
#  define send_code(s, c, वृक्ष) \
     { अगर (z_verbose>2) ख_लिखो(मानक_त्रुटि,"\ncd %3d ",(c)); \
       send_bits(s, वृक्ष[c].Code, वृक्ष[c].Len); }
#पूर्ण_अगर

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#अगर_घोषित ZLIB_DEBUG
local व्योम send_bits(deflate_state *s, पूर्णांक मूल्य, पूर्णांक लंबाई) {
    Tracevv((मानक_त्रुटि," l %2d v %4x ", लंबाई, मूल्य));
    Assert(लंबाई > 0 && लंबाई <= 15, "invalid length");
    s->bits_sent += (ulg)लंबाई;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    अगर (s->bi_valid > (पूर्णांक)Buf_आकार - लंबाई) {
        s->bi_buf |= (ush)मूल्य << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)मूल्य >> (Buf_आकार - s->bi_valid);
        s->bi_valid += लंबाई - Buf_आकार;
    } अन्यथा {
        s->bi_buf |= (ush)मूल्य << s->bi_valid;
        s->bi_valid += लंबाई;
    }
}
#अन्यथा /* !ZLIB_DEBUG */

#घोषणा send_bits(s, मूल्य, लंबाई) \
{ पूर्णांक len = लंबाई;\
  अगर (s->bi_valid > (पूर्णांक)Buf_आकार - len) {\
    पूर्णांक val = (पूर्णांक)मूल्य;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_आकार - s->bi_valid);\
    s->bi_valid += len - Buf_आकार;\
  } अन्यथा {\
    s->bi_buf |= (ush)(मूल्य) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#पूर्ण_अगर /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local व्योम tr_static_init(व्योम) {
#अगर defined(GEN_TREES_H) || !defined(STDC)
    अटल पूर्णांक static_init_done = 0;
    पूर्णांक n;        /* iterates over tree elements */
    पूर्णांक bits;     /* bit counter */
    पूर्णांक लंबाई;   /* length value */
    पूर्णांक code;     /* code value */
    पूर्णांक dist;     /* distance index */
    ush bl_गिनती[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */

    अगर (static_init_done) वापस;

    /* For some embedded targets, global variables are not initialized: */
#अगर_घोषित NO_INIT_GLOBAL_POINTERS
    static_l_desc.static_वृक्ष = static_ltree;
    static_l_desc.extra_bits = extra_lbits;
    static_d_desc.static_वृक्ष = static_dtree;
    static_d_desc.extra_bits = extra_dbits;
    static_bl_desc.extra_bits = extra_blbits;
#पूर्ण_अगर

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    लंबाई = 0;
    क्रम (code = 0; code < लंबाई_CODES-1; code++) {
        base_लंबाई[code] = लंबाई;
        क्रम (n = 0; n < (1 << extra_lbits[code]); n++) {
            लंबाई_code[लंबाई++] = (uch)code;
        }
    }
    Assert (लंबाई == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    लंबाई_code[लंबाई - 1] = (uch)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    क्रम (code = 0 ; code < 16; code++) {
        base_dist[code] = dist;
        क्रम (n = 0; n < (1 << extra_dbits[code]); n++) {
            _dist_code[dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    क्रम ( ; code < D_CODES; code++) {
        base_dist[code] = dist << 7;
        क्रम (n = 0; n < (1 << (extra_dbits[code] - 7)); n++) {
            _dist_code[256 + dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    क्रम (bits = 0; bits <= MAX_BITS; bits++) bl_गिनती[bits] = 0;
    n = 0;
    जबतक (n <= 143) static_ltree[n++].Len = 8, bl_गिनती[8]++;
    जबतक (n <= 255) static_ltree[n++].Len = 9, bl_गिनती[9]++;
    जबतक (n <= 279) static_ltree[n++].Len = 7, bl_गिनती[7]++;
    जबतक (n <= 287) static_ltree[n++].Len = 8, bl_गिनती[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_तथ्य *)static_ltree, L_CODES+1, bl_गिनती);

    /* The static distance tree is trivial: */
    क्रम (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].Code = bi_reverse((अचिन्हित)n, 5);
    }
    static_init_done = 1;

#  ifdef GEN_TREES_H
    gen_trees_header();
#  endif
#पूर्ण_अगर /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#अगर_घोषित GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, last, width) \
      ((i) == (last)? "\n};\n\n" :    \
       ((i) % (width) == (width) - 1 ? ",\n" : ", "))

व्योम gen_trees_header(व्योम) {
    खाता *header = ख_खोलो("trees.h", "w");
    पूर्णांक i;

    Assert (header != शून्य, "Can't open trees.h");
    ख_लिखो(header,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    ख_लिखो(header, "local const ct_data static_ltree[L_CODES+2] = {\n");
    क्रम (i = 0; i < L_CODES+2; i++) {
        ख_लिखो(header, "{{%3u},{%3u}}%s", static_ltree[i].Code,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    ख_लिखो(header, "local const ct_data static_dtree[D_CODES] = {\n");
    क्रम (i = 0; i < D_CODES; i++) {
        ख_लिखो(header, "{{%2u},{%2u}}%s", static_dtree[i].Code,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    ख_लिखो(header, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    क्रम (i = 0; i < DIST_CODE_LEN; i++) {
        ख_लिखो(header, "%2u%s", _dist_code[i],
                SEPARATOR(i, DIST_CODE_LEN-1, 20));
    }

    ख_लिखो(header,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    क्रम (i = 0; i < MAX_MATCH-MIN_MATCH+1; i++) {
        ख_लिखो(header, "%2u%s", लंबाई_code[i],
                SEPARATOR(i, MAX_MATCH-MIN_MATCH, 20));
    }

    ख_लिखो(header, "local const int base_length[LENGTH_CODES] = {\n");
    क्रम (i = 0; i < लंबाई_CODES; i++) {
        ख_लिखो(header, "%1u%s", base_लंबाई[i],
                SEPARATOR(i, लंबाई_CODES-1, 20));
    }

    ख_लिखो(header, "local const int base_dist[D_CODES] = {\n");
    क्रम (i = 0; i < D_CODES; i++) {
        ख_लिखो(header, "%5u%s", base_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    ख_बंद(header);
}
#पूर्ण_अगर /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local व्योम init_block(deflate_state *s) {
    पूर्णांक n; /* iterates over tree elements */

    /* Initialize the trees. */
    क्रम (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    क्रम (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    क्रम (n = 0; n < BL_CODES; n++) s->bl_वृक्ष[n].Freq = 0;

    s->dyn_ltree[END_BLOCK].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_अग्र = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
व्योम ZLIB_INTERNAL _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_वृक्ष = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_वृक्ष = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_वृक्ष = s->bl_वृक्ष;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->bi_used = 0;
#अगर_घोषित ZLIB_DEBUG
    s->compressed_len = 0L;
    s->bits_sent = 0L;
#पूर्ण_अगर

    /* Initialize the first block of the first file: */
    init_block(s);
}

#घोषणा SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#घोषणा pqremove(s, वृक्ष, शीर्ष_3) \
{\
    शीर्ष_3 = s->heap[SMALLEST]; \
    s->heap[SMALLEST] = s->heap[s->heap_len--]; \
    pqdownheap(s, वृक्ष, SMALLEST); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#घोषणा smaller(वृक्ष, n, m, depth) \
   (वृक्ष[n].Freq < वृक्ष[m].Freq || \
   (वृक्ष[n].Freq == वृक्ष[m].Freq && depth[n] <= depth[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local व्योम pqdownheap(deflate_state *s, ct_तथ्य *वृक्ष, पूर्णांक k) {
    पूर्णांक v = s->heap[k];
    पूर्णांक j = k << 1;  /* left son of k */
    जबतक (j <= s->heap_len) {
        /* Set j to the smallest of the two sons: */
        अगर (j < s->heap_len &&
            smaller(वृक्ष, s->heap[j + 1], s->heap[j], s->depth)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        अगर (smaller(वृक्ष, v, s->heap[j], s->depth)) अवरोध;

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
local व्योम gen_bitlen(deflate_state *s, वृक्ष_desc *desc) {
    ct_तथ्य *वृक्ष        = desc->dyn_वृक्ष;
    पूर्णांक max_code         = desc->max_code;
    स्थिर ct_तथ्य *stree = desc->stat_desc->static_वृक्ष;
    स्थिर intf *extra    = desc->stat_desc->extra_bits;
    पूर्णांक base             = desc->stat_desc->extra_base;
    पूर्णांक max_लंबाई       = desc->stat_desc->max_लंबाई;
    पूर्णांक h;              /* heap index */
    पूर्णांक n, m;           /* iterate over the tree elements */
    पूर्णांक bits;           /* bit length */
    पूर्णांक xbits;          /* extra bits */
    ush f;              /* frequency */
    पूर्णांक overflow = 0;   /* number of elements with bit length too large */

    क्रम (bits = 0; bits <= MAX_BITS; bits++) s->bl_गिनती[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    वृक्ष[s->heap[s->heap_max]].Len = 0; /* root of the heap */

    क्रम (h = s->heap_max + 1; h < HEAP_आकार; h++) {
        n = s->heap[h];
        bits = वृक्ष[वृक्ष[n].Dad].Len + 1;
        अगर (bits > max_लंबाई) bits = max_लंबाई, overflow++;
        वृक्ष[n].Len = (ush)bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        अगर (n > max_code) जारी; /* not a leaf node */

        s->bl_गिनती[bits]++;
        xbits = 0;
        अगर (n >= base) xbits = extra[n - base];
        f = वृक्ष[n].Freq;
        s->opt_len += (ulg)f * (अचिन्हित)(bits + xbits);
        अगर (stree) s->static_len += (ulg)f * (अचिन्हित)(stree[n].Len + xbits);
    }
    अगर (overflow == 0) वापस;

    Tracev((मानक_त्रुटि,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    करो {
        bits = max_लंबाई - 1;
        जबतक (s->bl_गिनती[bits] == 0) bits--;
        s->bl_गिनती[bits]--;        /* move one leaf down the tree */
        s->bl_गिनती[bits + 1] += 2; /* move one overflow item as its brother */
        s->bl_गिनती[max_लंबाई]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } जबतक (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    क्रम (bits = max_लंबाई; bits != 0; bits--) {
        n = s->bl_गिनती[bits];
        जबतक (n != 0) {
            m = s->heap[--h];
            अगर (m > max_code) जारी;
            अगर ((अचिन्हित) वृक्ष[m].Len != (अचिन्हित) bits) {
                Tracev((मानक_त्रुटि,"code %d bits %d->%d\n", m, वृक्ष[m].Len, bits));
                s->opt_len += ((ulg)bits - वृक्ष[m].Len) * वृक्ष[m].Freq;
                वृक्ष[m].Len = (ush)bits;
            }
            n--;
        }
    }
}

#अगर_घोषित DUMP_BL_वृक्ष
#  include <stdio.h>
#पूर्ण_अगर

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local व्योम build_वृक्ष(deflate_state *s, वृक्ष_desc *desc) {
    ct_तथ्य *वृक्ष         = desc->dyn_वृक्ष;
    स्थिर ct_तथ्य *stree  = desc->stat_desc->static_वृक्ष;
    पूर्णांक elems             = desc->stat_desc->elems;
    पूर्णांक n, m;          /* iterate over heap elements */
    पूर्णांक max_code = -1; /* largest code with non zero frequency */
    पूर्णांक गाँठ;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->heap_len = 0, s->heap_max = HEAP_आकार;

    क्रम (n = 0; n < elems; n++) {
        अगर (वृक्ष[n].Freq != 0) {
            s->heap[++(s->heap_len)] = max_code = n;
            s->depth[n] = 0;
        } अन्यथा {
            वृक्ष[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    जबतक (s->heap_len < 2) {
        गाँठ = s->heap[++(s->heap_len)] = (max_code < 2 ? ++max_code : 0);
        वृक्ष[गाँठ].Freq = 1;
        s->depth[गाँठ] = 0;
        s->opt_len--; अगर (stree) s->static_len -= stree[गाँठ].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    क्रम (n = s->heap_len/2; n >= 1; n--) pqdownheap(s, वृक्ष, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    गाँठ = elems;              /* next internal node of the tree */
    करो {
        pqremove(s, वृक्ष, n);  /* n = node of least frequency */
        m = s->heap[SMALLEST]; /* m = node of next least frequency */

        s->heap[--(s->heap_max)] = n; /* keep the nodes sorted by frequency */
        s->heap[--(s->heap_max)] = m;

        /* Create a new node father of n and m */
        वृक्ष[गाँठ].Freq = वृक्ष[n].Freq + वृक्ष[m].Freq;
        s->depth[गाँठ] = (uch)((s->depth[n] >= s->depth[m] ?
                                s->depth[n] : s->depth[m]) + 1);
        वृक्ष[n].Dad = वृक्ष[m].Dad = (ush)गाँठ;
#अगर_घोषित DUMP_BL_वृक्ष
        अगर (वृक्ष == s->bl_वृक्ष) {
            ख_लिखो(मानक_त्रुटि,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    गाँठ, वृक्ष[गाँठ].Freq, n, वृक्ष[n].Freq, m, वृक्ष[m].Freq);
        }
#पूर्ण_अगर
        /* and insert the new node in the heap */
        s->heap[SMALLEST] = गाँठ++;
        pqdownheap(s, वृक्ष, SMALLEST);

    } जबतक (s->heap_len >= 2);

    s->heap[--(s->heap_max)] = s->heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (वृक्ष_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_तथ्य *)वृक्ष, max_code, s->bl_गिनती);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local व्योम scan_वृक्ष(deflate_state *s, ct_तथ्य *वृक्ष, पूर्णांक max_code) {
    पूर्णांक n;                     /* iterates over all tree elements */
    पूर्णांक prevlen = -1;          /* last emitted length */
    पूर्णांक curlen;                /* length of current code */
    पूर्णांक nextlen = वृक्ष[0].Len; /* length of next code */
    पूर्णांक गिनती = 0;             /* repeat count of the current code */
    पूर्णांक max_गिनती = 7;         /* max repeat count */
    पूर्णांक min_गिनती = 4;         /* min repeat count */

    अगर (nextlen == 0) max_गिनती = 138, min_गिनती = 3;
    वृक्ष[max_code + 1].Len = (ush)0xffff; /* guard */

    क्रम (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = वृक्ष[n + 1].Len;
        अगर (++गिनती < max_गिनती && curlen == nextlen) {
            जारी;
        } अन्यथा अगर (गिनती < min_गिनती) {
            s->bl_वृक्ष[curlen].Freq += (ush)गिनती;
        } अन्यथा अगर (curlen != 0) {
            अगर (curlen != prevlen) s->bl_वृक्ष[curlen].Freq++;
            s->bl_वृक्ष[REP_3_6].Freq++;
        } अन्यथा अगर (गिनती <= 10) {
            s->bl_वृक्ष[REPZ_3_10].Freq++;
        } अन्यथा {
            s->bl_वृक्ष[REPZ_11_138].Freq++;
        }
        गिनती = 0; prevlen = curlen;
        अगर (nextlen == 0) {
            max_गिनती = 138, min_गिनती = 3;
        } अन्यथा अगर (curlen == nextlen) {
            max_गिनती = 6, min_गिनती = 3;
        } अन्यथा {
            max_गिनती = 7, min_गिनती = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local व्योम send_वृक्ष(deflate_state *s, ct_तथ्य *वृक्ष, पूर्णांक max_code) {
    पूर्णांक n;                     /* iterates over all tree elements */
    पूर्णांक prevlen = -1;          /* last emitted length */
    पूर्णांक curlen;                /* length of current code */
    पूर्णांक nextlen = वृक्ष[0].Len; /* length of next code */
    पूर्णांक गिनती = 0;             /* repeat count of the current code */
    पूर्णांक max_गिनती = 7;         /* max repeat count */
    पूर्णांक min_गिनती = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    अगर (nextlen == 0) max_गिनती = 138, min_गिनती = 3;

    क्रम (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = वृक्ष[n + 1].Len;
        अगर (++गिनती < max_गिनती && curlen == nextlen) {
            जारी;
        } अन्यथा अगर (गिनती < min_गिनती) {
            करो { send_code(s, curlen, s->bl_वृक्ष); } जबतक (--गिनती != 0);

        } अन्यथा अगर (curlen != 0) {
            अगर (curlen != prevlen) {
                send_code(s, curlen, s->bl_वृक्ष); गिनती--;
            }
            Assert(गिनती >= 3 && गिनती <= 6, " 3_6?");
            send_code(s, REP_3_6, s->bl_वृक्ष); send_bits(s, गिनती - 3, 2);

        } अन्यथा अगर (गिनती <= 10) {
            send_code(s, REPZ_3_10, s->bl_वृक्ष); send_bits(s, गिनती - 3, 3);

        } अन्यथा {
            send_code(s, REPZ_11_138, s->bl_वृक्ष); send_bits(s, गिनती - 11, 7);
        }
        गिनती = 0; prevlen = curlen;
        अगर (nextlen == 0) {
            max_गिनती = 138, min_गिनती = 3;
        } अन्यथा अगर (curlen == nextlen) {
            max_गिनती = 6, min_गिनती = 3;
        } अन्यथा {
            max_गिनती = 7, min_गिनती = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local पूर्णांक build_bl_वृक्ष(deflate_state *s) {
    पूर्णांक max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_वृक्ष(s, (ct_तथ्य *)s->dyn_ltree, s->l_desc.max_code);
    scan_वृक्ष(s, (ct_तथ्य *)s->dyn_dtree, s->d_desc.max_code);

    /* Build the bit length tree: */
    build_वृक्ष(s, (वृक्ष_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    क्रम (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        अगर (s->bl_वृक्ष[bl_order[max_blindex]].Len != 0) अवरोध;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((मानक_त्रुटि, "\ndyn trees: dyn %lu, stat %lu",
            s->opt_len, s->static_len));

    वापस max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local व्योम send_all_trees(deflate_state *s, पूर्णांक lcodes, पूर्णांक dcodes,
                          पूर्णांक blcodes) {
    पूर्णांक rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((मानक_त्रुटि, "\nbl counts: "));
    send_bits(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_bits(s, dcodes - 1,   5);
    send_bits(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    क्रम (rank = 0; rank < blcodes; rank++) {
        Tracev((मानक_त्रुटि, "\nbl code %2d ", bl_order[rank]));
        send_bits(s, s->bl_वृक्ष[bl_order[rank]].Len, 3);
    }
    Tracev((मानक_त्रुटि, "\nbl tree: sent %lu", s->bits_sent));

    send_वृक्ष(s, (ct_तथ्य *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((मानक_त्रुटि, "\nlit tree: sent %lu", s->bits_sent));

    send_वृक्ष(s, (ct_तथ्य *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((मानक_त्रुटि, "\ndist tree: sent %lu", s->bits_sent));
}

/* ===========================================================================
 * Send a stored block
 */
व्योम ZLIB_INTERNAL _tr_stored_block(deflate_state *s, charf *buf,
                                    ulg stored_len, पूर्णांक last) {
    send_bits(s, (STORED_BLOCK<<1) + last, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    अगर (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#अगर_घोषित ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->bits_sent += 2*16;
    s->bits_sent += stored_len << 3;
#पूर्ण_अगर
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
व्योम ZLIB_INTERNAL _tr_flush_bits(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
व्योम ZLIB_INTERNAL _tr_align(deflate_state *s) {
    send_bits(s, STATIC_TREES<<1, 3);
    send_code(s, END_BLOCK, static_ltree);
#अगर_घोषित ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#पूर्ण_अगर
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local व्योम compress_block(deflate_state *s, स्थिर ct_तथ्य *ltree,
                          स्थिर ct_तथ्य *dtree) {
    अचिन्हित dist;      /* distance of matched string */
    पूर्णांक lc;             /* match length or unmatched char (if dist == 0) */
    अचिन्हित sx = 0;    /* running index in symbol buffers */
    अचिन्हित code;      /* the code to send */
    पूर्णांक extra;          /* number of extra bits to send */

    अगर (s->sym_अग्र != 0) करो {
#अगर_घोषित LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#अन्यथा
        dist = s->sym_buf[sx++] & 0xff;
        dist += (अचिन्हित)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#पूर्ण_अगर
        अगर (dist == 0) {
            send_code(s, lc, ltree); /* send a literal byte */
            Tracecv(है_चित्र(lc), (मानक_त्रुटि," '%c' ", lc));
        } अन्यथा {
            /* Here, lc is the match length - MIN_MATCH */
            code = लंबाई_code[lc];
            send_code(s, code + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[code];
            अगर (extra != 0) {
                lc -= base_लंबाई[code];
                send_bits(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            code = d_code(dist);
            Assert (code < D_CODES, "bad d_code");

            send_code(s, code, dtree);       /* send the distance code */
            extra = extra_dbits[code];
            अगर (extra != 0) {
                dist -= (अचिन्हित)base_dist[code];
                send_bits(s, (पूर्णांक)dist, extra); /* send the extra bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#अगर_घोषित LIT_MEM
        Assert(s->pending < 2 * (s->lit_bufsize + sx), "pendingBuf overflow");
#अन्यथा
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#पूर्ण_अगर

    } जबतक (sx < s->sym_अग्र);

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
local पूर्णांक detect_तथ्य_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    अचिन्हित दीर्घ block_mask = 0xf3ffc07fUL;
    पूर्णांक n;

    /* Check for non-textual ("block-listed") bytes. */
    क्रम (n = 0; n <= 31; n++, block_mask >>= 1)
        अगर ((block_mask & 1) && (s->dyn_ltree[n].Freq != 0))
            वापस Z_द्विआधारी;

    /* Check for textual ("allow-listed") bytes. */
    अगर (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        वापस Z_TEXT;
    क्रम (n = 32; n < LITERALS; n++)
        अगर (s->dyn_ltree[n].Freq != 0)
            वापस Z_TEXT;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    वापस Z_द्विआधारी;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
व्योम ZLIB_INTERNAL _tr_flush_block(deflate_state *s, charf *buf,
                                   ulg stored_len, पूर्णांक last) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    पूर्णांक max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    अगर (s->level > 0) {

        /* Check if the file is binary or text */
        अगर (s->strm->तथ्य_type == Z_UNKNOWN)
            s->strm->तथ्य_type = detect_तथ्य_type(s);

        /* Construct the literal and distance trees */
        build_वृक्ष(s, (वृक्ष_desc *)(&(s->l_desc)));
        Tracev((मानक_त्रुटि, "\nlit data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));

        build_वृक्ष(s, (वृक्ष_desc *)(&(s->d_desc)));
        Tracev((मानक_त्रुटि, "\ndist data: dyn %lu, stat %lu", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_वृक्ष(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((मानक_त्रुटि, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_अग्र / 3));

#अगर_अघोषित FORCE_STATIC
        अगर (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#पूर्ण_अगर
            opt_lenb = static_lenb;

    } अन्यथा {
        Assert(buf != (अक्षर*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#अगर_घोषित FORCE_STORED
    अगर (buf != (अक्षर*)0) { /* force stored block */
#अन्यथा
    अगर (stored_len + 4 <= opt_lenb && buf != (अक्षर*)0) {
                       /* 4: two words for the lengths */
#पूर्ण_अगर
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        _tr_stored_block(s, buf, stored_len, last);

    } अन्यथा अगर (static_lenb == opt_lenb) {
        send_bits(s, (STATIC_TREES<<1) + last, 3);
        compress_block(s, (स्थिर ct_तथ्य *)static_ltree,
                       (स्थिर ct_तथ्य *)static_dtree);
#अगर_घोषित ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#पूर्ण_अगर
    } अन्यथा {
        send_bits(s, (DYN_TREES<<1) + last, 3);
        send_all_trees(s, s->l_desc.max_code + 1, s->d_desc.max_code + 1,
                       max_blindex + 1);
        compress_block(s, (स्थिर ct_तथ्य *)s->dyn_ltree,
                       (स्थिर ct_तथ्य *)s->dyn_dtree);
#अगर_घोषित ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#पूर्ण_अगर
    }
    Assert (s->compressed_len == s->bits_sent, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_block(s);

    अगर (last) {
        bi_windup(s);
#अगर_घोषित ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#पूर्ण_अगर
    }
    Tracev((मानक_त्रुटि,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*(ulg)last));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
पूर्णांक ZLIB_INTERNAL _tr_tally(deflate_state *s, अचिन्हित dist, अचिन्हित lc) {
#अगर_घोषित LIT_MEM
    s->d_buf[s->sym_अग्र] = (ush)dist;
    s->l_buf[s->sym_अग्र++] = (uch)lc;
#अन्यथा
    s->sym_buf[s->sym_अग्र++] = (uch)dist;
    s->sym_buf[s->sym_अग्र++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_अग्र++] = (uch)lc;
#पूर्ण_अगर
    अगर (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } अन्यथा {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_MATCH-MIN_MATCH) &&
               (ush)d_code(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[लंबाई_code[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_code(dist)].Freq++;
    }
    वापस (s->sym_अग्र == s->sym_end);
}
