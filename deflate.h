/* deflate.h -- internal compression state
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#यदि_अपरिभाषितम् DEFLATE_H
#परिभाषय DEFLATE_H

#अन्तर्भाव "zutil.h"

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#यदि_अपरिभाषितम् NO_GZIP
#  define GZIP
#यदि_अन्तः

/* define LIT_MEM to slightly increase the speed of deflate (order 1% to 2%) at
   the cost of a larger memory footprint */
/* #define LIT_MEM */

/* ===========================================================================
 * Internal compression state.
 */

#परिभाषय दीर्घता_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#परिभाषय LITERALS  256
/* number of literal bytes 0..255 */

#परिभाषय L_CODES (LITERALS+1+दीर्घता_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#परिभाषय D_CODES   30
/* number of distance codes */

#परिभाषय BL_CODES  19
/* number of codes used to transfer the bit lengths */

#परिभाषय HEAP_आकार (2*L_CODES+1)
/* maximum heap size */

#परिभाषय MAX_द्वयङ्काः 15
/* All codes must not exceed MAX_BITS bits */

#परिभाषय Buf_आकार 16
/* size of bit buffer in bi_buf */

#परिभाषय INIT_STATE    42    /* zlib header -> BUSY_STATE */
#यदि_परिभाषितम् GZIP
#  define GZIP_STATE  57    /* gzip header -> BUSY_STATE | EXTRA_STATE */
#यदि_अन्तः
#परिभाषय EXTRA_STATE   69    /* gzip extra block -> NAME_STATE */
#परिभाषय NAME_STATE    73    /* gzip file name -> COMMENT_STATE */
#परिभाषय COMMENT_STATE 91    /* gzip comment -> HCRC_STATE */
#परिभाषय HCRC_STATE   103    /* gzip header CRC -> BUSY_STATE */
#परिभाषय BUSY_STATE   113    /* deflate -> FINISH_STATE */
#परिभाषय FINISH_STATE 666    /* stream complete */
/* Stream status */


/* Data structure describing a single value and its code string. */
प्रकारनिर्देश संरचना ct_दत्त_s {
    सङ्घ {
        ush  freq;       /* frequency count */
        ush  संहिता;       /* bit string */
    } fc;
    सङ्घ {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_दत्त;

#परिभाषय Freq fc.freq
#परिभाषय संहिता_2 fc.संहिता
#परिभाषय Dad  dl.dad
#परिभाषय Len  dl.len

प्रकारनिर्देश संरचना static_वृक्षः_desc_s  static_वृक्षः_desc;

प्रकारनिर्देश संरचना वृक्षः_desc_s {
    ct_दत्त *dyn_वृक्षः;           /* the dynamic tree */
    पूर्णाङ्क     max_संहिता;            /* largest code with non zero frequency */
    ध्रुव static_वृक्षः_desc *stat_desc;  /* the corresponding static tree */
} FAR वृक्षः_desc;

प्रकारनिर्देश ush Pos;
प्रकारनिर्देश Pos FAR Posf;
प्रकारनिर्देश अचिह्नित IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

प्रकारनिर्देश संरचना आन्तरिकम्_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    पूर्णाङ्क   स्थितिः;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_आकार; /* size of pending_buf */
    Bytef *pending_बहिर्गमनम्;  /* next pending byte to output to the stream */
    ulg   pending;       /* nb of bytes in the pending buffer */
    पूर्णाङ्क   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    ulg   gzindex;       /* where in extra, name, or comment */
    Byte  विधानम्;        /* can only be DEFLATED */
    पूर्णाङ्क   चरमः_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_आकार;        /* LZ77 window size (32K by default) */
    uInt  w_द्वयङ्काः;        /* log2(w_size)  (8..16) */
    uInt  w_आवरणम्;        /* w_size - 1 */

    Bytef *गवाक्षः;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg गवाक्षः_आकार;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *पूर्व;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *शीर्ष; /* Heads of the hash chains or NIL. */

    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_आकार;      /* number of elements in hash table */
    uInt  hash_द्वयङ्काः;      /* log2(hash_size) */
    uInt  hash_आवरणम्;      /* hash_size-1 */

    uInt  hash_सरणम्;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    दीर्घ खण्डम्_start;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt मेलनम्_दीर्घता;           /* length of best match */
    IPos पूर्व_मेलनम्;             /* previous match */
    पूर्णाङ्क मेलनम्_उपलब्धम्;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt मेलनम्_start;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt पूर्व_दीर्घता;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_दीर्घता;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_मेलनम्;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_निवेशय_दीर्घता  max_lazy_मेलनम्
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    पूर्णाङ्क level;    /* compression level (1..9) */
    पूर्णाङ्क strategy; /* favor or force Huffman coding*/

    uInt good_मेलनम्;
    /* Use a faster search when the previous match is longer than this */

    पूर्णाङ्क nice_मेलनम्; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    संरचना ct_दत्त_s dyn_ltree[HEAP_आकार];   /* literal and length tree */
    संरचना ct_दत्त_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    संरचना ct_दत्त_s bl_वृक्षः[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    संरचना वृक्षः_desc_s l_desc;               /* desc. for literal tree */
    संरचना वृक्षः_desc_s d_desc;               /* desc. for distance tree */
    संरचना वृक्षः_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_गणनम्[MAX_द्वयङ्काः+1];
    /* number of codes at each bit length for an optimal tree */

    पूर्णाङ्क heap[2*L_CODES+1];      /* heap used to build the Huffman trees */
    पूर्णाङ्क heap_len;               /* number of elements in the heap */
    पूर्णाङ्क heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch गभीरता[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

#यदि_परिभाषितम् LIT_MEM
#   define LIT_BUFS 5
    ushf *d_buf;          /* buffer for distances */
    uchf *l_buf;          /* buffer for literals/lengths */
#अन्यथा
#   define LIT_BUFS 4
    uchf *sym_buf;        /* buffer for distances and literals/lengths */
#यदि_अन्तः

    uInt  lit_bufsize;
    /* Size of match buffer for literals/lengths.  There are 4 reasons for
     * limiting lit_bufsize to 64K:
     *   - frequencies can be kept in 16 bit counters
     *   - if compression is not successful for the first block, all input
     *     data is still in the window so we can still emit a stored block even
     *     when input comes from standard input.  (This can also be done for
     *     all blocks if lit_bufsize is not greater than 32K.)
     *   - if compression is not successful for a file smaller than 64K, we can
     *     even emit a stored file instead of a stored block (saving 5 bytes).
     *     This is applicable only for zip (not gzip or zlib).
     *   - creating new Huffman trees less frequently may not provide fast
     *     adaptation to changes in the input data statistics. (Take for
     *     example a binary file with poorly compressible code followed by
     *     a highly compressible string table.) Smaller buffer sizes give
     *     fast adaptation but have of course the overhead of transmitting
     *     trees more frequently.
     *   - I can't count above 4
     */

    uInt sym_अग्र;      /* running index in symbol buffer */
    uInt sym_अन्तः;       /* symbol table full when sym_next reaches this */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt निवेशय;        /* bytes at end of window left to insert */

#यदि_परिभाषितम् ZLIB_DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg द्वयङ्काः_प्रेषितम्;      /* bit length of compressed data sent mod 2^32 */
#यदि_अन्तः

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    पूर्णाङ्क bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
    पूर्णाङ्क bi_प्रयुक्तम्;
    /* Last number of used bits when going to a byte boundary.
     */

    ulg उच्चः_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

    पूर्णाङ्क slid;
    /* True if the hash table has been slid since it was cleared. */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#परिभाषय put_byte(s, c) {s->pending_buf[s->pending++] = (Bytef)(c);}


#परिभाषय MIN_LOOKAHEAD (MAX_मेलनम्+MIN_मेलनम्+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#परिभाषय MAX_DIST(s)  ((s)->w_आकार-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#परिभाषय WIN_INIT MAX_मेलनम्
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
शून्य ZLIB_आन्तरिकम् _tr_init(deflate_state *s);
पूर्णाङ्क ZLIB_आन्तरिकम् _tr_tally(deflate_state *s, अचिह्नित dist, अचिह्नित lc);
शून्य ZLIB_आन्तरिकम् tr_flush_खण्डम्(deflate_state *s, charf *buf,
                                   ulg stored_len, पूर्णाङ्क चरमः);
शून्य ZLIB_आन्तरिकम् tr_flush_द्वयङ्काः(deflate_state *s);
शून्य ZLIB_आन्तरिकम् _tr_align(deflate_state *s);
शून्य ZLIB_आन्तरिकम् tr_stored_खण्डम्(deflate_state *s, charf *buf,
                                    ulg stored_len, पूर्णाङ्क चरमः);

#परिभाषय d_संहिता(dist) \
   ((dist) < 256 ? dist_संहिता[dist] : dist_संहिता[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#यदि_अपरिभाषितम् ZLIB_DEBUG
/* Inline versions of _tr_tally for speed: */

#यदि defined(GEN_TREES_H) || !defined(STDC)
  बाह्य uch ZLIB_आन्तरिकम् दीर्घता_संहिता[];
  बाह्य uch ZLIB_आन्तरिकम् dist_संहिता[];
#अन्यथा
  बाह्य ध्रुव uch ZLIB_आन्तरिकम् दीर्घता_संहिता[];
  बाह्य ध्रुव uch ZLIB_आन्तरिकम् dist_संहिता[];
#यदि_अन्तः

#यदि_परिभाषितम् LIT_MEM
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->sym_अग्र] = 0; \
    s->l_buf[s->sym_अग्र++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_अग्र == s->sym_अन्तः); \
   }
# define _tr_tally_dist(s, distance, दीर्घता, flush) \
  { uch len = (uch)(दीर्घता); \
    ush dist = (ush)(distance); \
    s->d_buf[s->sym_अग्र] = dist; \
    s->l_buf[s->sym_अग्र++] = len; \
    dist--; \
    s->dyn_ltree[दीर्घता_संहिता[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_संहिता(dist)].Freq++; \
    flush = (s->sym_अग्र == s->sym_अन्तः); \
  }
#अन्यथा
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->sym_buf[s->sym_अग्र++] = 0; \
    s->sym_buf[s->sym_अग्र++] = 0; \
    s->sym_buf[s->sym_अग्र++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_अग्र == s->sym_अन्तः); \
   }
# define _tr_tally_dist(s, distance, दीर्घता, flush) \
  { uch len = (uch)(दीर्घता); \
    ush dist = (ush)(distance); \
    s->sym_buf[s->sym_अग्र++] = (uch)dist; \
    s->sym_buf[s->sym_अग्र++] = (uch)(dist >> 8); \
    s->sym_buf[s->sym_अग्र++] = len; \
    dist--; \
    s->dyn_ltree[दीर्घता_संहिता[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_संहिता(dist)].Freq++; \
    flush = (s->sym_अग्र == s->sym_अन्तः); \
  }
#यदि_अन्तः
#अन्यथा
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, distance, दीर्घता, flush) \
              flush = _tr_tally(s, distance, दीर्घता)
#यदि_अन्तः

#यदि_अन्तः /* DEFLATE_H */
