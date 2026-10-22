/* deflate.h -- internal compression state
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#اگر_غیر_تعریف_شدہ DEFLATE_H
#تعریف_کرو DEFLATE_H

#شامل "zutil.h"

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#اگر_غیر_تعریف_شدہ NO_GZIP
#  define GZIP
#اگر_ختم

/* define LIT_MEM to slightly increase the speed of deflate (order 1% to 2%) at
   the cost of a larger memory footprint */
/* #define LIT_MEM */

/* ===========================================================================
 * Internal compression state.
 */

#تعریف_کرو لمبائی_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#تعریف_کرو LITERALS  256
/* number of literal bytes 0..255 */

#تعریف_کرو L_CODES (LITERALS+1+لمبائی_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#تعریف_کرو D_CODES   30
/* number of distance codes */

#تعریف_کرو BL_CODES  19
/* number of codes used to transfer the bit lengths */

#تعریف_کرو ڈھیر_سائز (2*L_CODES+1)
/* maximum heap size */

#تعریف_کرو MAX_بٹ 15
/* All codes must not exceed MAX_BITS bits */

#تعریف_کرو Buf_سائز 16
/* size of bit buffer in bi_buf */

#تعریف_کرو INIT_STATE    42    /* zlib header -> BUSY_STATE */
#اگر_تعریف_شدہ GZIP
#  define GZIP_STATE  57    /* gzip header -> BUSY_STATE | EXTRA_STATE */
#اگر_ختم
#تعریف_کرو EXTRA_STATE   69    /* gzip extra block -> NAME_STATE */
#تعریف_کرو NAME_STATE    73    /* gzip file name -> COMMENT_STATE */
#تعریف_کرو COMMENT_STATE 91    /* gzip comment -> HCRC_STATE */
#تعریف_کرو HCRC_STATE   103    /* gzip header CRC -> BUSY_STATE */
#تعریف_کرو BUSY_STATE   113    /* deflate -> FINISH_STATE */
#تعریف_کرو اختتام_STATE 666    /* stream complete */
/* Stream status */


/* Data structure describing a single value and its code string. */
قسمتعریف ساخت ct_ڈیٹا_s {
    اتحاد {
        ush  freq;       /* frequency count */
        ush  رمز;       /* bit string */
    } fc;
    اتحاد {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_ڈیٹا;

#تعریف_کرو Freq fc.freq
#تعریف_کرو رمز_2 fc.رمز
#تعریف_کرو Dad  dl.dad
#تعریف_کرو Len  dl.len

قسمتعریف ساخت static_درخت_desc_s  static_درخت_desc;

قسمتعریف ساخت درخت_desc_s {
    ct_ڈیٹا *dyn_درخت;           /* the dynamic tree */
    صحیح     max_رمز;            /* largest code with non zero frequency */
    ثابت static_درخت_desc *stat_desc;  /* the corresponding static tree */
} FAR درخت_desc;

قسمتعریف ush Pos;
قسمتعریف Pos FAR Posf;
قسمتعریف غیرعلامتی IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

قسمتعریف ساخت اندرونی_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    صحیح   حالت_2;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_سائز; /* size of pending_buf */
    Bytef *pending_برآمد;  /* next pending byte to output to the stream */
    ulg   pending;       /* nb of bytes in the pending buffer */
    صحیح   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    ulg   gzindex;       /* where in extra, name, or comment */
    Byte  طریقہ_2;        /* can only be DEFLATED */
    صحیح   آخری_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_سائز;        /* LZ77 window size (32K by default) */
    uInt  w_بٹ;        /* log2(w_size)  (8..16) */
    uInt  w_نقاب;        /* w_size - 1 */

    Bytef *کھڑکی;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg کھڑکی_سائز;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *پچھلا;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *سر; /* Heads of the hash chains or NIL. */

    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_سائز;      /* number of elements in hash table */
    uInt  hash_بٹ;      /* log2(hash_size) */
    uInt  hash_نقاب;      /* hash_size-1 */

    uInt  hash_سرکاؤ;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    طویل بلاک_آغاز;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt مطابقت_لمبائی;           /* length of best match */
    IPos پچھلا_مطابقت;             /* previous match */
    صحیح مطابقت_دستیاب;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt مطابقت_آغاز;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt پچھلا_لمبائی;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_لمبائی;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_مطابقت;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_داخل_کرو_لمبائی  max_lazy_مطابقت
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    صحیح level;    /* compression level (1..9) */
    صحیح strategy; /* favor or force Huffman coding*/

    uInt good_مطابقت;
    /* Use a faster search when the previous match is longer than this */

    صحیح nice_مطابقت; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    ساخت ct_ڈیٹا_s dyn_ltree[ڈھیر_سائز];   /* literal and length tree */
    ساخت ct_ڈیٹا_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    ساخت ct_ڈیٹا_s bl_درخت[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    ساخت درخت_desc_s l_desc;               /* desc. for literal tree */
    ساخت درخت_desc_s d_desc;               /* desc. for distance tree */
    ساخت درخت_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_گنتی[MAX_بٹ+1];
    /* number of codes at each bit length for an optimal tree */

    صحیح ڈھیر[2*L_CODES+1];      /* heap used to build the Huffman trees */
    صحیح ڈھیر_len;               /* number of elements in the heap */
    صحیح ڈھیر_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch گہرائی[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

#اگر_تعریف_شدہ LIT_MEM
#   define LIT_BUFS 5
    ushf *d_buf;          /* buffer for distances */
    uchf *l_buf;          /* buffer for literals/lengths */
#ورنہ
#   define LIT_BUFS 4
    uchf *sym_buf;        /* buffer for distances and literals/lengths */
#اگر_ختم

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

    uInt sym_اگلا;      /* running index in symbol buffer */
    uInt sym_انتہا;       /* symbol table full when sym_next reaches this */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt داخل_کرو;        /* bytes at end of window left to insert */

#اگر_تعریف_شدہ ZLIB_DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg بٹ_بھیجا;      /* bit length of compressed data sent mod 2^32 */
#اگر_ختم

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    صحیح bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
    صحیح bi_مستعمل;
    /* Last number of used bits when going to a byte boundary.
     */

    ulg بلند_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

    صحیح slid;
    /* True if the hash table has been slid since it was cleared. */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#تعریف_کرو put_byte(s, c) {s->pending_buf[s->pending++] = (Bytef)(c);}


#تعریف_کرو MIN_LOOKAHEAD (MAX_مطابقت+MIN_مطابقت+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#تعریف_کرو MAX_DIST(s)  ((s)->w_سائز-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#تعریف_کرو WIN_INIT MAX_مطابقت
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
خالی ZLIB_اندرونی _tr_init(deflate_state *s);
صحیح ZLIB_اندرونی _tr_tally(deflate_state *s, غیرعلامتی dist, غیرعلامتی lc);
خالی ZLIB_اندرونی tr_flush_بلاک(deflate_state *s, charf *buf,
                                   ulg stored_len, صحیح آخری);
خالی ZLIB_اندرونی tr_flush_بٹ(deflate_state *s);
خالی ZLIB_اندرونی _tr_align(deflate_state *s);
خالی ZLIB_اندرونی tr_stored_بلاک(deflate_state *s, charf *buf,
                                    ulg stored_len, صحیح آخری);

#تعریف_کرو d_رمز(dist) \
   ((dist) < 256 ? dist_رمز[dist] : dist_رمز[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#اگر_غیر_تعریف_شدہ ZLIB_DEBUG
/* Inline versions of _tr_tally for speed: */

#اگر defined(GEN_TREES_H) || !defined(STDC)
  خارجی uch ZLIB_اندرونی لمبائی_رمز[];
  خارجی uch ZLIB_اندرونی dist_رمز[];
#ورنہ
  خارجی ثابت uch ZLIB_اندرونی لمبائی_رمز[];
  خارجی ثابت uch ZLIB_اندرونی dist_رمز[];
#اگر_ختم

#اگر_تعریف_شدہ LIT_MEM
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->sym_اگلا] = 0; \
    s->l_buf[s->sym_اگلا++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_اگلا == s->sym_انتہا); \
   }
# define _tr_tally_dist(s, فاصلہ, لمبائی, flush) \
  { uch len = (uch)(لمبائی); \
    ush dist = (ush)(فاصلہ); \
    s->d_buf[s->sym_اگلا] = dist; \
    s->l_buf[s->sym_اگلا++] = len; \
    dist--; \
    s->dyn_ltree[لمبائی_رمز[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_رمز(dist)].Freq++; \
    flush = (s->sym_اگلا == s->sym_انتہا); \
  }
#ورنہ
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->sym_buf[s->sym_اگلا++] = 0; \
    s->sym_buf[s->sym_اگلا++] = 0; \
    s->sym_buf[s->sym_اگلا++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_اگلا == s->sym_انتہا); \
   }
# define _tr_tally_dist(s, فاصلہ, لمبائی, flush) \
  { uch len = (uch)(لمبائی); \
    ush dist = (ush)(فاصلہ); \
    s->sym_buf[s->sym_اگلا++] = (uch)dist; \
    s->sym_buf[s->sym_اگلا++] = (uch)(dist >> 8); \
    s->sym_buf[s->sym_اگلا++] = len; \
    dist--; \
    s->dyn_ltree[لمبائی_رمز[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_رمز(dist)].Freq++; \
    flush = (s->sym_اگلا == s->sym_انتہا); \
  }
#اگر_ختم
#ورنہ
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, فاصلہ, لمبائی, flush) \
              flush = _tr_tally(s, فاصلہ, لمبائی)
#اگر_ختم

#اگر_ختم /* DEFLATE_H */
