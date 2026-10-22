/* deflate.h -- internal compression state
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEFLATE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFLATE_H

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_GZIP
#  define GZIP
#ਜੇ_ਅੰਤ

/* define LIT_MEM to slightly increase the speed of deflate (order 1% to 2%) at
   the cost of a larger memory footprint */
/* #define LIT_MEM */

/* ===========================================================================
 * Internal compression state.
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੰਮਾਈ_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LITERALS  256
/* number of literal bytes 0..255 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_CODES (LITERALS+1+ਲੰਮਾਈ_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ D_CODES   30
/* number of distance codes */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BL_CODES  19
/* number of codes used to transfer the bit lengths */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ਮਾਪ (2*L_CODES+1)
/* maximum heap size */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਬਿੱਟ 15
/* All codes must not exceed MAX_BITS bits */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Buf_ਮਾਪ 16
/* size of bit buffer in bi_buf */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_STATE    42    /* zlib header -> BUSY_STATE */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
#  define GZIP_STATE  57    /* gzip header -> BUSY_STATE | EXTRA_STATE */
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTRA_STATE   69    /* gzip extra block -> NAME_STATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NAME_STATE    73    /* gzip file name -> COMMENT_STATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COMMENT_STATE 91    /* gzip comment -> HCRC_STATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HCRC_STATE   103    /* gzip header CRC -> BUSY_STATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUSY_STATE   113    /* deflate -> FINISH_STATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਤ_STATE 666    /* stream complete */
/* Stream status */


/* Data structure describing a single value and its code string. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ct_ਅੰਕੜੇ_s {
    ਸੰਘ {
        ush  freq;       /* frequency count */
        ush  ਕੋਡ;       /* bit string */
    } fc;
    ਸੰਘ {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_ਅੰਕੜੇ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Freq fc.freq
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਕੋਡ_2 fc.ਕੋਡ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Dad  dl.dad
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Len  dl.len

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ static_ਰੁੱਖ_desc_s  static_ਰੁੱਖ_desc;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਰੁੱਖ_desc_s {
    ct_ਅੰਕੜੇ *dyn_ਰੁੱਖ;           /* the dynamic tree */
    ਪੂਰਨ     max_ਕੋਡ;            /* largest code with non zero frequency */
    ਅਚਲ static_ਰੁੱਖ_desc *stat_desc;  /* the corresponding static tree */
} FAR ਰੁੱਖ_desc;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ush Pos;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ Pos FAR Posf;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਅੰਦਰੂਨੀ_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    ਪੂਰਨ   ਸਥਿਤੀ;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_ਮਾਪ; /* size of pending_buf */
    Bytef *pending_ਬਾਹਰਲਾ;  /* next pending byte to output to the stream */
    ulg   pending;       /* nb of bytes in the pending buffer */
    ਪੂਰਨ   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    ulg   gzindex;       /* where in extra, name, or comment */
    Byte  ਵਿਧੀ_2;        /* can only be DEFLATED */
    ਪੂਰਨ   ਆਖਰੀ_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_ਮਾਪ;        /* LZ77 window size (32K by default) */
    uInt  w_ਬਿੱਟ;        /* log2(w_size)  (8..16) */
    uInt  w_ਨਕਾਬ;        /* w_size - 1 */

    Bytef *ਖਿੜਕੀ;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg ਖਿੜਕੀ_ਮਾਪ;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *ਸਿਰ; /* Heads of the hash chains or NIL. */

    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_ਮਾਪ;      /* number of elements in hash table */
    uInt  hash_ਬਿੱਟ;      /* log2(hash_size) */
    uInt  hash_ਨਕਾਬ;      /* hash_size-1 */

    uInt  hash_ਖਿਸਕਾਓ;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    ਲੰਮਾ ਖੰਡ_ਸ਼ੁਰੂ;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt ਮੇਲ_ਲੰਮਾਈ;           /* length of best match */
    IPos prev_ਮੇਲ;             /* previous match */
    ਪੂਰਨ ਮੇਲ_ਉਪਲਬਧ;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt ਮੇਲ_ਸ਼ੁਰੂ;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt prev_ਲੰਮਾਈ;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_ਲੰਮਾਈ;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_ਮੇਲ;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_ਪਾਓ_ਲੰਮਾਈ  max_lazy_ਮੇਲ
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    ਪੂਰਨ level;    /* compression level (1..9) */
    ਪੂਰਨ strategy; /* favor or force Huffman coding*/

    uInt good_ਮੇਲ;
    /* Use a faster search when the previous match is longer than this */

    ਪੂਰਨ nice_ਮੇਲ; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    ਬਣਤਰ ct_ਅੰਕੜੇ_s dyn_ltree[ਢੇਰ_ਮਾਪ];   /* literal and length tree */
    ਬਣਤਰ ct_ਅੰਕੜੇ_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    ਬਣਤਰ ct_ਅੰਕੜੇ_s bl_ਰੁੱਖ[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    ਬਣਤਰ ਰੁੱਖ_desc_s l_desc;               /* desc. for literal tree */
    ਬਣਤਰ ਰੁੱਖ_desc_s d_desc;               /* desc. for distance tree */
    ਬਣਤਰ ਰੁੱਖ_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_ਗਿਣਤੀ[MAX_ਬਿੱਟ+1];
    /* number of codes at each bit length for an optimal tree */

    ਪੂਰਨ ਢੇਰ[2*L_CODES+1];      /* heap used to build the Huffman trees */
    ਪੂਰਨ ਢੇਰ_len;               /* number of elements in the heap */
    ਪੂਰਨ ਢੇਰ_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch ਡੂੰਘਾਈ[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
#   define LIT_BUFS 5
    ushf *d_buf;          /* buffer for distances */
    uchf *l_buf;          /* buffer for literals/lengths */
#ਨਹੀਂਤਾਂ
#   define LIT_BUFS 4
    uchf *sym_buf;        /* buffer for distances and literals/lengths */
#ਜੇ_ਅੰਤ

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

    uInt sym_ਅਗਲਾ;      /* running index in symbol buffer */
    uInt sym_ਸਿਰਾ;       /* symbol table full when sym_next reaches this */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt ਪਾਓ;        /* bytes at end of window left to insert */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg ਬਿੱਟ_ਭੇਜਿਆ;      /* bit length of compressed data sent mod 2^32 */
#ਜੇ_ਅੰਤ

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    ਪੂਰਨ bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
    ਪੂਰਨ bi_ਵਰਤਿਆ;
    /* Last number of used bits when going to a byte boundary.
     */

    ulg ਉੱਚਾ_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

    ਪੂਰਨ slid;
    /* True if the hash table has been slid since it was cleared. */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ put_byte(s, c) {s->pending_buf[s->pending++] = (Bytef)(c);}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_LOOKAHEAD (MAX_ਮੇਲ+MIN_ਮੇਲ+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_DIST(s)  ((s)->w_ਮਾਪ-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN_INIT MAX_ਮੇਲ
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ _tr_init(deflate_state *s);
ਪੂਰਨ ZLIB_ਅੰਦਰੂਨੀ _tr_tally(deflate_state *s, ਬਿਨਾਨਿਸ਼ਾਨੀ dist, ਬਿਨਾਨਿਸ਼ਾਨੀ lc);
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ tr_flush_ਖੰਡ(deflate_state *s, charf *buf,
                                   ulg stored_len, ਪੂਰਨ ਆਖਰੀ);
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ tr_flush_ਬਿੱਟ(deflate_state *s);
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ _tr_align(deflate_state *s);
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ tr_stored_ਖੰਡ(deflate_state *s, charf *buf,
                                    ulg stored_len, ਪੂਰਨ ਆਖਰੀ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ d_ਕੋਡ(dist) \
   ((dist) < 256 ? dist_ਕੋਡ[dist] : dist_ਕੋਡ[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
/* Inline versions of _tr_tally for speed: */

#ਜੇ defined(GEN_TREES_H) || !defined(STDC)
  ਬਾਹਰੀ uch ZLIB_ਅੰਦਰੂਨੀ ਲੰਮਾਈ_ਕੋਡ[];
  ਬਾਹਰੀ uch ZLIB_ਅੰਦਰੂਨੀ dist_ਕੋਡ[];
#ਨਹੀਂਤਾਂ
  ਬਾਹਰੀ ਅਚਲ uch ZLIB_ਅੰਦਰੂਨੀ ਲੰਮਾਈ_ਕੋਡ[];
  ਬਾਹਰੀ ਅਚਲ uch ZLIB_ਅੰਦਰੂਨੀ dist_ਕੋਡ[];
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->sym_ਅਗਲਾ] = 0; \
    s->l_buf[s->sym_ਅਗਲਾ++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_ਅਗਲਾ == s->sym_ਸਿਰਾ); \
   }
# define _tr_tally_dist(s, ਦੂਰੀ, ਲੰਮਾਈ, flush) \
  { uch len = (uch)(ਲੰਮਾਈ); \
    ush dist = (ush)(ਦੂਰੀ); \
    s->d_buf[s->sym_ਅਗਲਾ] = dist; \
    s->l_buf[s->sym_ਅਗਲਾ++] = len; \
    dist--; \
    s->dyn_ltree[ਲੰਮਾਈ_ਕੋਡ[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_ਕੋਡ(dist)].Freq++; \
    flush = (s->sym_ਅਗਲਾ == s->sym_ਸਿਰਾ); \
  }
#ਨਹੀਂਤਾਂ
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->sym_buf[s->sym_ਅਗਲਾ++] = 0; \
    s->sym_buf[s->sym_ਅਗਲਾ++] = 0; \
    s->sym_buf[s->sym_ਅਗਲਾ++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_ਅਗਲਾ == s->sym_ਸਿਰਾ); \
   }
# define _tr_tally_dist(s, ਦੂਰੀ, ਲੰਮਾਈ, flush) \
  { uch len = (uch)(ਲੰਮਾਈ); \
    ush dist = (ush)(ਦੂਰੀ); \
    s->sym_buf[s->sym_ਅਗਲਾ++] = (uch)dist; \
    s->sym_buf[s->sym_ਅਗਲਾ++] = (uch)(dist >> 8); \
    s->sym_buf[s->sym_ਅਗਲਾ++] = len; \
    dist--; \
    s->dyn_ltree[ਲੰਮਾਈ_ਕੋਡ[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_ਕੋਡ(dist)].Freq++; \
    flush = (s->sym_ਅਗਲਾ == s->sym_ਸਿਰਾ); \
  }
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, ਦੂਰੀ, ਲੰਮਾਈ, flush) \
              flush = _tr_tally(s, ਦੂਰੀ, ਲੰਮਾਈ)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* DEFLATE_H */
