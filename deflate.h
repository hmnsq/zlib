/* deflate.h -- internal compression state
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#جيڪڏهن_وصف_نه_ٿيل DEFLATE_H
#وصف_ڪريو DEFLATE_H

#ضمن "zutil.h"

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#جيڪڏهن_وصف_نه_ٿيل NO_GZIP
#  define GZIP
#جيڪڏهن_پورو

/* define LIT_MEM to slightly increase the speed of deflate (order 1% to 2%) at
   the cost of a larger memory footprint */
/* #define LIT_MEM */

/* ===========================================================================
 * Internal compression state.
 */

#وصف_ڪريو ڊيگهائي_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#وصف_ڪريو LITERALS  256
/* number of literal bytes 0..255 */

#وصف_ڪريو L_CODES (LITERALS+1+ڊيگهائي_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#وصف_ڪريو D_CODES   30
/* number of distance codes */

#وصف_ڪريو BL_CODES  19
/* number of codes used to transfer the bit lengths */

#وصف_ڪريو ڍير_سائيز (2*L_CODES+1)
/* maximum heap size */

#وصف_ڪريو MAX_بٽ 15
/* All codes must not exceed MAX_BITS bits */

#وصف_ڪريو Buf_سائيز 16
/* size of bit buffer in bi_buf */

#وصف_ڪريو INIT_STATE    42    /* zlib header -> BUSY_STATE */
#جيڪڏهن_وصف_ٿيل GZIP
#  define GZIP_STATE  57    /* gzip header -> BUSY_STATE | EXTRA_STATE */
#جيڪڏهن_پورو
#وصف_ڪريو EXTRA_STATE   69    /* gzip extra block -> NAME_STATE */
#وصف_ڪريو NAME_STATE    73    /* gzip file name -> COMMENT_STATE */
#وصف_ڪريو COMMENT_STATE 91    /* gzip comment -> HCRC_STATE */
#وصف_ڪريو HCRC_STATE   103    /* gzip header CRC -> BUSY_STATE */
#وصف_ڪريو BUSY_STATE   113    /* deflate -> FINISH_STATE */
#وصف_ڪريو پڄاڻي_STATE 666    /* stream complete */
/* Stream status */


/* Data structure describing a single value and its code string. */
تعريفنوع بنية ct_معلومات_s {
    اتحاد {
        ush  freq;       /* frequency count */
        ush  ڪوڊ;       /* bit string */
    } fc;
    اتحاد {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_معلومات;

#وصف_ڪريو Freq fc.freq
#وصف_ڪريو ڪوڊ_2 fc.ڪوڊ
#وصف_ڪريو Dad  dl.dad
#وصف_ڪريو Len  dl.len

تعريفنوع بنية static_وڻ_desc_s  static_وڻ_desc;

تعريفنوع بنية وڻ_desc_s {
    ct_معلومات *dyn_وڻ;           /* the dynamic tree */
    صحيح     max_ڪوڊ;            /* largest code with non zero frequency */
    ثابت static_وڻ_desc *stat_desc;  /* the corresponding static tree */
} FAR وڻ_desc;

تعريفنوع ush Pos;
تعريفنوع Pos FAR Posf;
تعريفنوع بنانشان IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

تعريفنوع بنية اندروني_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    صحيح   حالت;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_سائيز; /* size of pending_buf */
    Bytef *pending_ٻاهريون;  /* next pending byte to output to the stream */
    ulg   pending;       /* nb of bytes in the pending buffer */
    صحيح   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    ulg   gzindex;       /* where in extra, name, or comment */
    Byte  طريقو;        /* can only be DEFLATED */
    صحيح   آخري_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_سائيز;        /* LZ77 window size (32K by default) */
    uInt  w_بٽ;        /* log2(w_size)  (8..16) */
    uInt  w_نقاب;        /* w_size - 1 */

    Bytef *دري;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg دري_سائيز;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *سر; /* Heads of the hash chains or NIL. */

    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_سائيز;      /* number of elements in hash table */
    uInt  hash_بٽ;      /* log2(hash_size) */
    uInt  hash_نقاب;      /* hash_size-1 */

    uInt  hash_سرڪايو;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    ڊگهو بلاڪ_شروعات;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt ميلاپ_ڊيگهائي;           /* length of best match */
    IPos prev_ميلاپ;             /* previous match */
    صحيح ميلاپ_موجود;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt ميلاپ_شروعات;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt prev_ڊيگهائي;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_ڊيگهائي;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_ميلاپ;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_وجهو_ڊيگهائي  max_lazy_ميلاپ
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    صحيح level;    /* compression level (1..9) */
    صحيح strategy; /* favor or force Huffman coding*/

    uInt good_ميلاپ;
    /* Use a faster search when the previous match is longer than this */

    صحيح nice_ميلاپ; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    بنية ct_معلومات_s dyn_ltree[ڍير_سائيز];   /* literal and length tree */
    بنية ct_معلومات_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    بنية ct_معلومات_s bl_وڻ[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    بنية وڻ_desc_s l_desc;               /* desc. for literal tree */
    بنية وڻ_desc_s d_desc;               /* desc. for distance tree */
    بنية وڻ_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_ڳڻپ[MAX_بٽ+1];
    /* number of codes at each bit length for an optimal tree */

    صحيح ڍير[2*L_CODES+1];      /* heap used to build the Huffman trees */
    صحيح ڍير_len;               /* number of elements in the heap */
    صحيح ڍير_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch اونهائي[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

#جيڪڏهن_وصف_ٿيل LIT_MEM
#   define LIT_BUFS 5
    ushf *d_buf;          /* buffer for distances */
    uchf *l_buf;          /* buffer for literals/lengths */
#وإلا
#   define LIT_BUFS 4
    uchf *sym_buf;        /* buffer for distances and literals/lengths */
#جيڪڏهن_پورو

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

    uInt sym_ايندڙ;      /* running index in symbol buffer */
    uInt sym_پڇاڙي;       /* symbol table full when sym_next reaches this */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt وجهو;        /* bytes at end of window left to insert */

#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg بٽ_موڪليل;      /* bit length of compressed data sent mod 2^32 */
#جيڪڏهن_پورو

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    صحيح bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
    صحيح bi_استعمال_ٿيل;
    /* Last number of used bits when going to a byte boundary.
     */

    ulg مٿاهون_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

    صحيح slid;
    /* True if the hash table has been slid since it was cleared. */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#وصف_ڪريو put_byte(s, c) {s->pending_buf[s->pending++] = (Bytef)(c);}


#وصف_ڪريو MIN_LOOKAHEAD (MAX_ميلاپ+MIN_ميلاپ+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#وصف_ڪريو MAX_DIST(s)  ((s)->w_سائيز-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#وصف_ڪريو WIN_INIT MAX_ميلاپ
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
فراغ ZLIB_اندروني _tr_init(deflate_state *s);
صحيح ZLIB_اندروني _tr_tally(deflate_state *s, بنانشان dist, بنانشان lc);
فراغ ZLIB_اندروني tr_flush_بلاڪ(deflate_state *s, charf *buf,
                                   ulg stored_len, صحيح آخري);
فراغ ZLIB_اندروني tr_flush_بٽ(deflate_state *s);
فراغ ZLIB_اندروني _tr_align(deflate_state *s);
فراغ ZLIB_اندروني tr_stored_بلاڪ(deflate_state *s, charf *buf,
                                    ulg stored_len, صحيح آخري);

#وصف_ڪريو d_ڪوڊ(dist) \
   ((dist) < 256 ? dist_ڪوڊ[dist] : dist_ڪوڊ[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#جيڪڏهن_وصف_نه_ٿيل ZLIB_DEBUG
/* Inline versions of _tr_tally for speed: */

#إذا defined(GEN_TREES_H) || !defined(STDC)
  خارجي uch ZLIB_اندروني ڊيگهائي_ڪوڊ[];
  خارجي uch ZLIB_اندروني dist_ڪوڊ[];
#وإلا
  خارجي ثابت uch ZLIB_اندروني ڊيگهائي_ڪوڊ[];
  خارجي ثابت uch ZLIB_اندروني dist_ڪوڊ[];
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل LIT_MEM
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->sym_ايندڙ] = 0; \
    s->l_buf[s->sym_ايندڙ++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_ايندڙ == s->sym_پڇاڙي); \
   }
# define _tr_tally_dist(s, مفاصلو, ڊيگهائي, flush) \
  { uch len = (uch)(ڊيگهائي); \
    ush dist = (ush)(مفاصلو); \
    s->d_buf[s->sym_ايندڙ] = dist; \
    s->l_buf[s->sym_ايندڙ++] = len; \
    dist--; \
    s->dyn_ltree[ڊيگهائي_ڪوڊ[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_ڪوڊ(dist)].Freq++; \
    flush = (s->sym_ايندڙ == s->sym_پڇاڙي); \
  }
#وإلا
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->sym_buf[s->sym_ايندڙ++] = 0; \
    s->sym_buf[s->sym_ايندڙ++] = 0; \
    s->sym_buf[s->sym_ايندڙ++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_ايندڙ == s->sym_پڇاڙي); \
   }
# define _tr_tally_dist(s, مفاصلو, ڊيگهائي, flush) \
  { uch len = (uch)(ڊيگهائي); \
    ush dist = (ush)(مفاصلو); \
    s->sym_buf[s->sym_ايندڙ++] = (uch)dist; \
    s->sym_buf[s->sym_ايندڙ++] = (uch)(dist >> 8); \
    s->sym_buf[s->sym_ايندڙ++] = len; \
    dist--; \
    s->dyn_ltree[ڊيگهائي_ڪوڊ[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_ڪوڊ(dist)].Freq++; \
    flush = (s->sym_ايندڙ == s->sym_پڇاڙي); \
  }
#جيڪڏهن_پورو
#وإلا
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, مفاصلو, ڊيگهائي, flush) \
              flush = _tr_tally(s, مفاصلو, ڊيگهائي)
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* DEFLATE_H */
