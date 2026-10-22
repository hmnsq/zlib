/* deflate.h -- internal compression state
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#अगर_अघोषित DEFLATE_H
#घोषणा DEFLATE_H

#समावेश "zutil.h"

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#अगर_अघोषित NO_GZIP
#  define GZIP
#पूर्ण_अगर

/* define LIT_MEM to slightly increase the speed of deflate (order 1% to 2%) at
   the cost of a larger memory footprint */
/* #define LIT_MEM */

/* ===========================================================================
 * Internal compression state.
 */

#घोषणा लंबाई_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#घोषणा LITERALS  256
/* number of literal bytes 0..255 */

#घोषणा L_CODES (LITERALS+1+लंबाई_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#घोषणा D_CODES   30
/* number of distance codes */

#घोषणा BL_CODES  19
/* number of codes used to transfer the bit lengths */

#घोषणा HEAP_आकार (2*L_CODES+1)
/* maximum heap size */

#घोषणा MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#घोषणा Buf_आकार 16
/* size of bit buffer in bi_buf */

#घोषणा INIT_STATE    42    /* zlib header -> BUSY_STATE */
#अगर_घोषित GZIP
#  define GZIP_STATE  57    /* gzip header -> BUSY_STATE | EXTRA_STATE */
#पूर्ण_अगर
#घोषणा EXTRA_STATE   69    /* gzip extra block -> NAME_STATE */
#घोषणा NAME_STATE    73    /* gzip file name -> COMMENT_STATE */
#घोषणा COMMENT_STATE 91    /* gzip comment -> HCRC_STATE */
#घोषणा HCRC_STATE   103    /* gzip header CRC -> BUSY_STATE */
#घोषणा BUSY_STATE   113    /* deflate -> FINISH_STATE */
#घोषणा FINISH_STATE 666    /* stream complete */
/* Stream status */


/* Data structure describing a single value and its code string. */
प्रकार काष्ठा ct_तथ्य_s {
    जोड़ {
        ush  freq;       /* frequency count */
        ush  code;       /* bit string */
    } fc;
    जोड़ {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_तथ्य;

#घोषणा Freq fc.freq
#घोषणा Code fc.code
#घोषणा Dad  dl.dad
#घोषणा Len  dl.len

प्रकार काष्ठा static_वृक्ष_desc_s  static_वृक्ष_desc;

प्रकार काष्ठा वृक्ष_desc_s {
    ct_तथ्य *dyn_वृक्ष;           /* the dynamic tree */
    पूर्णांक     max_code;            /* largest code with non zero frequency */
    स्थिर static_वृक्ष_desc *stat_desc;  /* the corresponding static tree */
} FAR वृक्ष_desc;

प्रकार ush Pos;
प्रकार Pos FAR Posf;
प्रकार अचिन्हित IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

प्रकार काष्ठा internal_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    पूर्णांक   status;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_आकार; /* size of pending_buf */
    Bytef *pending_out;  /* next pending byte to output to the stream */
    ulg   pending;       /* nb of bytes in the pending buffer */
    पूर्णांक   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    ulg   gzindex;       /* where in extra, name, or comment */
    Byte  method;        /* can only be DEFLATED */
    पूर्णांक   last_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_आकार;        /* LZ77 window size (32K by default) */
    uInt  w_bits;        /* log2(w_size)  (8..16) */
    uInt  w_mask;        /* w_size - 1 */

    Bytef *window;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg window_आकार;
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
    uInt  hash_bits;      /* log2(hash_size) */
    uInt  hash_mask;      /* hash_size-1 */

    uInt  hash_shift;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    दीर्घ block_start;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt match_लंबाई;           /* length of best match */
    IPos पूर्व_match;             /* previous match */
    पूर्णांक match_available;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt match_start;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt पूर्व_लंबाई;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_लंबाई;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_जोड़ो_लंबाई  max_lazy_match
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    पूर्णांक level;    /* compression level (1..9) */
    पूर्णांक strategy; /* favor or force Huffman coding*/

    uInt good_match;
    /* Use a faster search when the previous match is longer than this */

    पूर्णांक nice_match; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    काष्ठा ct_तथ्य_s dyn_ltree[HEAP_आकार];   /* literal and length tree */
    काष्ठा ct_तथ्य_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    काष्ठा ct_तथ्य_s bl_वृक्ष[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    काष्ठा वृक्ष_desc_s l_desc;               /* desc. for literal tree */
    काष्ठा वृक्ष_desc_s d_desc;               /* desc. for distance tree */
    काष्ठा वृक्ष_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_गिनती[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */

    पूर्णांक heap[2*L_CODES+1];      /* heap used to build the Huffman trees */
    पूर्णांक heap_len;               /* number of elements in the heap */
    पूर्णांक heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch depth[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

#अगर_घोषित LIT_MEM
#   define LIT_BUFS 5
    ushf *d_buf;          /* buffer for distances */
    uchf *l_buf;          /* buffer for literals/lengths */
#अन्यथा
#   define LIT_BUFS 4
    uchf *sym_buf;        /* buffer for distances and literals/lengths */
#पूर्ण_अगर

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
    uInt sym_end;       /* symbol table full when sym_next reaches this */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt जोड़ो;        /* bytes at end of window left to insert */

#अगर_घोषित ZLIB_DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg bits_sent;      /* bit length of compressed data sent mod 2^32 */
#पूर्ण_अगर

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    पूर्णांक bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
    पूर्णांक bi_used;
    /* Last number of used bits when going to a byte boundary.
     */

    ulg ऊपरी_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

    पूर्णांक slid;
    /* True if the hash table has been slid since it was cleared. */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#घोषणा put_byte(s, c) {s->pending_buf[s->pending++] = (Bytef)(c);}


#घोषणा MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#घोषणा MAX_DIST(s)  ((s)->w_आकार-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#घोषणा WIN_INIT MAX_MATCH
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
व्योम ZLIB_INTERNAL _tr_init(deflate_state *s);
पूर्णांक ZLIB_INTERNAL _tr_tally(deflate_state *s, अचिन्हित dist, अचिन्हित lc);
व्योम ZLIB_INTERNAL _tr_flush_block(deflate_state *s, charf *buf,
                                   ulg stored_len, पूर्णांक last);
व्योम ZLIB_INTERNAL _tr_flush_bits(deflate_state *s);
व्योम ZLIB_INTERNAL _tr_align(deflate_state *s);
व्योम ZLIB_INTERNAL _tr_stored_block(deflate_state *s, charf *buf,
                                    ulg stored_len, पूर्णांक last);

#घोषणा d_code(dist) \
   ((dist) < 256 ? _dist_code[dist] : _dist_code[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#अगर_अघोषित ZLIB_DEBUG
/* Inline versions of _tr_tally for speed: */

#अगर defined(GEN_TREES_H) || !defined(STDC)
  बाह्य uch ZLIB_INTERNAL लंबाई_code[];
  बाह्य uch ZLIB_INTERNAL _dist_code[];
#अन्यथा
  बाह्य स्थिर uch ZLIB_INTERNAL लंबाई_code[];
  बाह्य स्थिर uch ZLIB_INTERNAL _dist_code[];
#पूर्ण_अगर

#अगर_घोषित LIT_MEM
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->sym_अग्र] = 0; \
    s->l_buf[s->sym_अग्र++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_अग्र == s->sym_end); \
   }
# define _tr_tally_dist(s, distance, लंबाई, flush) \
  { uch len = (uch)(लंबाई); \
    ush dist = (ush)(distance); \
    s->d_buf[s->sym_अग्र] = dist; \
    s->l_buf[s->sym_अग्र++] = len; \
    dist--; \
    s->dyn_ltree[लंबाई_code[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_code(dist)].Freq++; \
    flush = (s->sym_अग्र == s->sym_end); \
  }
#अन्यथा
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->sym_buf[s->sym_अग्र++] = 0; \
    s->sym_buf[s->sym_अग्र++] = 0; \
    s->sym_buf[s->sym_अग्र++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_अग्र == s->sym_end); \
   }
# define _tr_tally_dist(s, distance, लंबाई, flush) \
  { uch len = (uch)(लंबाई); \
    ush dist = (ush)(distance); \
    s->sym_buf[s->sym_अग्र++] = (uch)dist; \
    s->sym_buf[s->sym_अग्र++] = (uch)(dist >> 8); \
    s->sym_buf[s->sym_अग्र++] = len; \
    dist--; \
    s->dyn_ltree[लंबाई_code[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_code(dist)].Freq++; \
    flush = (s->sym_अग्र == s->sym_end); \
  }
#पूर्ण_अगर
#अन्यथा
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, distance, लंबाई, flush) \
              flush = _tr_tally(s, distance, लंबाई)
#पूर्ण_अगर

#पूर्ण_अगर /* DEFLATE_H */
