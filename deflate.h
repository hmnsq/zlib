/* deflate.h -- internal compression state
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DEFLATE_H
#ವ್ಯಾಖ್ಯಾನಿಸು DEFLATE_H

#ಅನ್ತರ್ಭಾವ "zutil.h"

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_GZIP
#  define GZIP
#ಆದರೆ_ಅಂತ್ಯ

/* define LIT_MEM to slightly increase the speed of deflate (order 1% to 2%) at
   the cost of a larger memory footprint */
/* #define LIT_MEM */

/* ===========================================================================
 * Internal compression state.
 */

#ವ್ಯಾಖ್ಯಾನಿಸು ಉದ್ದಳತೆ_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#ವ್ಯಾಖ್ಯಾನಿಸು LITERALS  256
/* number of literal bytes 0..255 */

#ವ್ಯಾಖ್ಯಾನಿಸು L_CODES (LITERALS+1+ಉದ್ದಳತೆ_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#ವ್ಯಾಖ್ಯಾನಿಸು D_CODES   30
/* number of distance codes */

#ವ್ಯಾಖ್ಯಾನಿಸು BL_CODES  19
/* number of codes used to transfer the bit lengths */

#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ಗಾತ್ರ (2*L_CODES+1)
/* maximum heap size */

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ಬಿಟ್ಟುಗಳು 15
/* All codes must not exceed MAX_BITS bits */

#ವ್ಯಾಖ್ಯಾನಿಸು Buf_ಗಾತ್ರ 16
/* size of bit buffer in bi_buf */

#ವ್ಯಾಖ್ಯಾನಿಸು INIT_STATE    42    /* zlib header -> BUSY_STATE */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
#  define GZIP_STATE  57    /* gzip header -> BUSY_STATE | EXTRA_STATE */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು EXTRA_STATE   69    /* gzip extra block -> NAME_STATE */
#ವ್ಯಾಖ್ಯಾನಿಸು NAME_STATE    73    /* gzip file name -> COMMENT_STATE */
#ವ್ಯಾಖ್ಯಾನಿಸು COMMENT_STATE 91    /* gzip comment -> HCRC_STATE */
#ವ್ಯಾಖ್ಯಾನಿಸು HCRC_STATE   103    /* gzip header CRC -> BUSY_STATE */
#ವ್ಯಾಖ್ಯಾನಿಸು BUSY_STATE   113    /* deflate -> FINISH_STATE */
#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಕ್ತಾಯ_STATE 666    /* stream complete */
/* Stream status */


/* Data structure describing a single value and its code string. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ct_ದತ್ತಾಂಶ_s {
    ಒಕ್ಕೂಟ {
        ush  freq;       /* frequency count */
        ush  ಸಂಕೇತ_2;       /* bit string */
    } fc;
    ಒಕ್ಕೂಟ {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_ದತ್ತಾಂಶ;

#ವ್ಯಾಖ್ಯಾನಿಸು Freq fc.freq
#ವ್ಯಾಖ್ಯಾನಿಸು ಸಂಕೇತ_3 fc.ಸಂಕೇತ_2
#ವ್ಯಾಖ್ಯಾನಿಸು Dad  dl.dad
#ವ್ಯಾಖ್ಯಾನಿಸು Len  dl.len

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ static_ವೃಕ್ಷ_desc_s  static_ವೃಕ್ಷ_desc;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ವೃಕ್ಷ_desc_s {
    ct_ದತ್ತಾಂಶ *dyn_ವೃಕ್ಷ;           /* the dynamic tree */
    ಪೂರ್ಣಾಂಕ     max_ಸಂಕೇತ;            /* largest code with non zero frequency */
    ಸ್ಥಿರಾಂಕ static_ವೃಕ್ಷ_desc *stat_desc;  /* the corresponding static tree */
} FAR ವೃಕ್ಷ_desc;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ush Pos;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Pos FAR Posf;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಆಂತರಿಕ_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    ಪೂರ್ಣಾಂಕ   ಸ್ಥಿತಿ_2;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_ಗಾತ್ರ; /* size of pending_buf */
    Bytef *pending_ಹೊರಹೋಗು;  /* next pending byte to output to the stream */
    ulg   pending;       /* nb of bytes in the pending buffer */
    ಪೂರ್ಣಾಂಕ   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    ulg   gzindex;       /* where in extra, name, or comment */
    Byte  ವಿಧಾನ_2;        /* can only be DEFLATED */
    ಪೂರ್ಣಾಂಕ   ಕೊನೆಯ_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_ಗಾತ್ರ;        /* LZ77 window size (32K by default) */
    uInt  w_ಬಿಟ್ಟುಗಳು;        /* log2(w_size)  (8..16) */
    uInt  w_ಮುಖವಾಡ;        /* w_size - 1 */

    Bytef *ಕಿಟಕಿ;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg ಕಿಟಕಿ_ಗಾತ್ರ;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *ತಲೆ; /* Heads of the hash chains or NIL. */

    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_ಗಾತ್ರ;      /* number of elements in hash table */
    uInt  hash_ಬಿಟ್ಟುಗಳು;      /* log2(hash_size) */
    uInt  hash_ಮುಖವಾಡ;      /* hash_size-1 */

    uInt  hash_ಸರಿಸು;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    ದೀರ್ಘ ಖಂಡ_ಆರಂಭ;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;           /* length of best match */
    IPos prev_ಹೊಂದಿಕೆ;             /* previous match */
    ಪೂರ್ಣಾಂಕ ಹೊಂದಿಕೆ_ಲಭ್ಯ;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt ಹೊಂದಿಕೆ_ಆರಂಭ;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt prev_ಉದ್ದಳತೆ;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_ಉದ್ದಳತೆ;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_ಹೊಂದಿಕೆ;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_ಸೇರಿಸು_ಉದ್ದಳತೆ  max_lazy_ಹೊಂದಿಕೆ
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    ಪೂರ್ಣಾಂಕ level;    /* compression level (1..9) */
    ಪೂರ್ಣಾಂಕ strategy; /* favor or force Huffman coding*/

    uInt good_ಹೊಂದಿಕೆ;
    /* Use a faster search when the previous match is longer than this */

    ಪೂರ್ಣಾಂಕ nice_ಹೊಂದಿಕೆ; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    ರಚನೆ ct_ದತ್ತಾಂಶ_s dyn_ltree[ರಾಶಿ_ಗಾತ್ರ];   /* literal and length tree */
    ರಚನೆ ct_ದತ್ತಾಂಶ_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    ರಚನೆ ct_ದತ್ತಾಂಶ_s bl_ವೃಕ್ಷ[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    ರಚನೆ ವೃಕ್ಷ_desc_s l_desc;               /* desc. for literal tree */
    ರಚನೆ ವೃಕ್ಷ_desc_s d_desc;               /* desc. for distance tree */
    ರಚನೆ ವೃಕ್ಷ_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_ಎಣಿಕೆ[MAX_ಬಿಟ್ಟುಗಳು+1];
    /* number of codes at each bit length for an optimal tree */

    ಪೂರ್ಣಾಂಕ ರಾಶಿ[2*L_CODES+1];      /* heap used to build the Huffman trees */
    ಪೂರ್ಣಾಂಕ ರಾಶಿ_len;               /* number of elements in the heap */
    ಪೂರ್ಣಾಂಕ ರಾಶಿ_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch ಆಳ[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
#   define LIT_BUFS 5
    ushf *d_buf;          /* buffer for distances */
    uchf *l_buf;          /* buffer for literals/lengths */
#ಇಲ್ಲದಿದ್ದರೆ
#   define LIT_BUFS 4
    uchf *sym_buf;        /* buffer for distances and literals/lengths */
#ಆದರೆ_ಅಂತ್ಯ

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

    uInt sym_ಮುಂದಿನ;      /* running index in symbol buffer */
    uInt sym_ಕೊನೆ;       /* symbol table full when sym_next reaches this */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt ಸೇರಿಸು;        /* bytes at end of window left to insert */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ;      /* bit length of compressed data sent mod 2^32 */
#ಆದರೆ_ಅಂತ್ಯ

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    ಪೂರ್ಣಾಂಕ bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
    ಪೂರ್ಣಾಂಕ bi_ಬಳಸಿದ;
    /* Last number of used bits when going to a byte boundary.
     */

    ulg ಮೇಲಿನ_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

    ಪೂರ್ಣಾಂಕ slid;
    /* True if the hash table has been slid since it was cleared. */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು put_byte(s, c) {s->pending_buf[s->pending++] = (Bytef)(c);}


#ವ್ಯಾಖ್ಯಾನಿಸು MIN_LOOKAHEAD (MAX_ಹೊಂದಿಕೆ+MIN_ಹೊಂದಿಕೆ+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_DIST(s)  ((s)->w_ಗಾತ್ರ-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#ವ್ಯಾಖ್ಯಾನಿಸು WIN_INIT MAX_ಹೊಂದಿಕೆ
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ _tr_init(deflate_state *s);
ಪೂರ್ಣಾಂಕ ZLIB_ಆಂತರಿಕ _tr_tally(deflate_state *s, ಚಿಹ್ನೆರಹಿತ dist, ಚಿಹ್ನೆರಹಿತ lc);
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ tr_flush_ಖಂಡ(deflate_state *s, charf *buf,
                                   ulg stored_len, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ);
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ tr_flush_ಬಿಟ್ಟುಗಳು(deflate_state *s);
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ _tr_align(deflate_state *s);
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ tr_stored_ಖಂಡ(deflate_state *s, charf *buf,
                                    ulg stored_len, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ);

#ವ್ಯಾಖ್ಯಾನಿಸು d_ಸಂಕೇತ(dist) \
   ((dist) < 256 ? dist_ಸಂಕೇತ[dist] : dist_ಸಂಕೇತ[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZLIB_DEBUG
/* Inline versions of _tr_tally for speed: */

#ಆದರೆ defined(GEN_TREES_H) || !defined(STDC)
  ಬಾಹ್ಯ uch ZLIB_ಆಂತರಿಕ ಉದ್ದಳತೆ_ಸಂಕೇತ[];
  ಬಾಹ್ಯ uch ZLIB_ಆಂತರಿಕ dist_ಸಂಕೇತ[];
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ uch ZLIB_ಆಂತರಿಕ ಉದ್ದಳತೆ_ಸಂಕೇತ[];
  ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ uch ZLIB_ಆಂತರಿಕ dist_ಸಂಕೇತ[];
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->sym_ಮುಂದಿನ] = 0; \
    s->l_buf[s->sym_ಮುಂದಿನ++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_ಮುಂದಿನ == s->sym_ಕೊನೆ); \
   }
# define _tr_tally_dist(s, ದೂರ, ಉದ್ದಳತೆ, flush) \
  { uch len = (uch)(ಉದ್ದಳತೆ); \
    ush dist = (ush)(ದೂರ); \
    s->d_buf[s->sym_ಮುಂದಿನ] = dist; \
    s->l_buf[s->sym_ಮುಂದಿನ++] = len; \
    dist--; \
    s->dyn_ltree[ಉದ್ದಳತೆ_ಸಂಕೇತ[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_ಸಂಕೇತ(dist)].Freq++; \
    flush = (s->sym_ಮುಂದಿನ == s->sym_ಕೊನೆ); \
  }
#ಇಲ್ಲದಿದ್ದರೆ
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->sym_buf[s->sym_ಮುಂದಿನ++] = 0; \
    s->sym_buf[s->sym_ಮುಂದಿನ++] = 0; \
    s->sym_buf[s->sym_ಮುಂದಿನ++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_ಮುಂದಿನ == s->sym_ಕೊನೆ); \
   }
# define _tr_tally_dist(s, ದೂರ, ಉದ್ದಳತೆ, flush) \
  { uch len = (uch)(ಉದ್ದಳತೆ); \
    ush dist = (ush)(ದೂರ); \
    s->sym_buf[s->sym_ಮುಂದಿನ++] = (uch)dist; \
    s->sym_buf[s->sym_ಮುಂದಿನ++] = (uch)(dist >> 8); \
    s->sym_buf[s->sym_ಮುಂದಿನ++] = len; \
    dist--; \
    s->dyn_ltree[ಉದ್ದಳತೆ_ಸಂಕೇತ[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_ಸಂಕೇತ(dist)].Freq++; \
    flush = (s->sym_ಮುಂದಿನ == s->sym_ಕೊನೆ); \
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, ದೂರ, ಉದ್ದಳತೆ, flush) \
              flush = _tr_tally(s, ದೂರ, ಉದ್ದಳತೆ)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* DEFLATE_H */
