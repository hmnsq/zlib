/* deflate.h -- internal compression state
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#నిర్వచితం_కాకపోతే DEFLATE_H
#నిర్వచించు DEFLATE_H

#అన్తర్భావ "zutil.h"

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#నిర్వచితం_కాకపోతే NO_GZIP
#  define GZIP
#అయితే_ముగింపు

/* define LIT_MEM to slightly increase the speed of deflate (order 1% to 2%) at
   the cost of a larger memory footprint */
/* #define LIT_MEM */

/* ===========================================================================
 * Internal compression state.
 */

#నిర్వచించు నిడివి_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#నిర్వచించు LITERALS  256
/* number of literal bytes 0..255 */

#నిర్వచించు L_CODES (LITERALS+1+నిడివి_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#నిర్వచించు D_CODES   30
/* number of distance codes */

#నిర్వచించు BL_CODES  19
/* number of codes used to transfer the bit lengths */

#నిర్వచించు కుప్ప_కొలత (2*L_CODES+1)
/* maximum heap size */

#నిర్వచించు MAX_బిట్లు 15
/* All codes must not exceed MAX_BITS bits */

#నిర్వచించు Buf_కొలత 16
/* size of bit buffer in bi_buf */

#నిర్వచించు INIT_STATE    42    /* zlib header -> BUSY_STATE */
#నిర్వచితమైతే GZIP
#  define GZIP_STATE  57    /* gzip header -> BUSY_STATE | EXTRA_STATE */
#అయితే_ముగింపు
#నిర్వచించు EXTRA_STATE   69    /* gzip extra block -> NAME_STATE */
#నిర్వచించు NAME_STATE    73    /* gzip file name -> COMMENT_STATE */
#నిర్వచించు COMMENT_STATE 91    /* gzip comment -> HCRC_STATE */
#నిర్వచించు HCRC_STATE   103    /* gzip header CRC -> BUSY_STATE */
#నిర్వచించు BUSY_STATE   113    /* deflate -> FINISH_STATE */
#నిర్వచించు ముగింపు_STATE 666    /* stream complete */
/* Stream status */


/* Data structure describing a single value and its code string. */
రకనిర్వచనం నిర్మాణం ct_దత్తాంశం_s {
    సంఘం {
        ush  freq;       /* frequency count */
        ush  సంకేతం_2;       /* bit string */
    } fc;
    సంఘం {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_దత్తాంశం;

#నిర్వచించు Freq fc.freq
#నిర్వచించు సంకేతం_3 fc.సంకేతం_2
#నిర్వచించు Dad  dl.dad
#నిర్వచించు Len  dl.len

రకనిర్వచనం నిర్మాణం static_వృక్షం_desc_s  static_వృక్షం_desc;

రకనిర్వచనం నిర్మాణం వృక్షం_desc_s {
    ct_దత్తాంశం *dyn_వృక్షం;           /* the dynamic tree */
    పూర్ణాంకం     max_సంకేతం;            /* largest code with non zero frequency */
    స్థిరరాశి static_వృక్షం_desc *stat_desc;  /* the corresponding static tree */
} FAR వృక్షం_desc;

రకనిర్వచనం ush Pos;
రకనిర్వచనం Pos FAR Posf;
రకనిర్వచనం గుర్తులేని IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

రకనిర్వచనం నిర్మాణం అంతర్గత_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    పూర్ణాంకం   స్థితి_2;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_కొలత; /* size of pending_buf */
    Bytef *pending_బహిర్గతం;  /* next pending byte to output to the stream */
    ulg   pending;       /* nb of bytes in the pending buffer */
    పూర్ణాంకం   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    ulg   gzindex;       /* where in extra, name, or comment */
    Byte  పద్ధతి;        /* can only be DEFLATED */
    పూర్ణాంకం   చివరి_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_కొలత;        /* LZ77 window size (32K by default) */
    uInt  w_బిట్లు;        /* log2(w_size)  (8..16) */
    uInt  w_ముసుగు;        /* w_size - 1 */

    Bytef *కిటికీ;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg కిటికీ_కొలత;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *తల; /* Heads of the hash chains or NIL. */

    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_కొలత;      /* number of elements in hash table */
    uInt  hash_బిట్లు;      /* log2(hash_size) */
    uInt  hash_ముసుగు;      /* hash_size-1 */

    uInt  hash_జరుపు;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    దీర్ఘ ఖండం_ప్రారంభం;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt సరిపోలిక_నిడివి;           /* length of best match */
    IPos prev_సరిపోలిక;             /* previous match */
    పూర్ణాంకం సరిపోలిక_అందుబాటు;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt సరిపోలిక_ప్రారంభం;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt prev_నిడివి;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_నిడివి;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_సరిపోలిక;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_చొప్పించు_నిడివి  max_lazy_సరిపోలిక
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    పూర్ణాంకం level;    /* compression level (1..9) */
    పూర్ణాంకం strategy; /* favor or force Huffman coding*/

    uInt good_సరిపోలిక;
    /* Use a faster search when the previous match is longer than this */

    పూర్ణాంకం nice_సరిపోలిక; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    నిర్మాణం ct_దత్తాంశం_s dyn_ltree[కుప్ప_కొలత];   /* literal and length tree */
    నిర్మాణం ct_దత్తాంశం_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    నిర్మాణం ct_దత్తాంశం_s bl_వృక్షం[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    నిర్మాణం వృక్షం_desc_s l_desc;               /* desc. for literal tree */
    నిర్మాణం వృక్షం_desc_s d_desc;               /* desc. for distance tree */
    నిర్మాణం వృక్షం_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_లెక్క[MAX_బిట్లు+1];
    /* number of codes at each bit length for an optimal tree */

    పూర్ణాంకం కుప్ప[2*L_CODES+1];      /* heap used to build the Huffman trees */
    పూర్ణాంకం కుప్ప_len;               /* number of elements in the heap */
    పూర్ణాంకం కుప్ప_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch లోతు[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

#నిర్వచితమైతే LIT_MEM
#   define LIT_BUFS 5
    ushf *d_buf;          /* buffer for distances */
    uchf *l_buf;          /* buffer for literals/lengths */
#లేకపోతే
#   define LIT_BUFS 4
    uchf *sym_buf;        /* buffer for distances and literals/lengths */
#అయితే_ముగింపు

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

    uInt sym_తదుపరి;      /* running index in symbol buffer */
    uInt sym_చివర;       /* symbol table full when sym_next reaches this */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt చొప్పించు;        /* bytes at end of window left to insert */

#నిర్వచితమైతే ZLIB_DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg బిట్లు_పంపిన;      /* bit length of compressed data sent mod 2^32 */
#అయితే_ముగింపు

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    పూర్ణాంకం bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
    పూర్ణాంకం bi_వాడిన;
    /* Last number of used bits when going to a byte boundary.
     */

    ulg ఎగువ_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

    పూర్ణాంకం slid;
    /* True if the hash table has been slid since it was cleared. */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#నిర్వచించు put_byte(s, c) {s->pending_buf[s->pending++] = (Bytef)(c);}


#నిర్వచించు MIN_LOOKAHEAD (MAX_సరిపోలిక+MIN_సరిపోలిక+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#నిర్వచించు MAX_DIST(s)  ((s)->w_కొలత-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#నిర్వచించు WIN_INIT MAX_సరిపోలిక
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
శూన్య ZLIB_అంతర్గత _tr_init(deflate_state *s);
పూర్ణాంకం ZLIB_అంతర్గత _tr_tally(deflate_state *s, గుర్తులేని dist, గుర్తులేని lc);
శూన్య ZLIB_అంతర్గత tr_flush_ఖండం(deflate_state *s, charf *buf,
                                   ulg stored_len, పూర్ణాంకం చివరి);
శూన్య ZLIB_అంతర్గత tr_flush_బిట్లు(deflate_state *s);
శూన్య ZLIB_అంతర్గత _tr_align(deflate_state *s);
శూన్య ZLIB_అంతర్గత tr_stored_ఖండం(deflate_state *s, charf *buf,
                                    ulg stored_len, పూర్ణాంకం చివరి);

#నిర్వచించు d_సంకేతం(dist) \
   ((dist) < 256 ? dist_సంకేతం[dist] : dist_సంకేతం[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#నిర్వచితం_కాకపోతే ZLIB_DEBUG
/* Inline versions of _tr_tally for speed: */

#ఒకవేళ defined(GEN_TREES_H) || !defined(STDC)
  బాహ్య uch ZLIB_అంతర్గత నిడివి_సంకేతం[];
  బాహ్య uch ZLIB_అంతర్గత dist_సంకేతం[];
#లేకపోతే
  బాహ్య స్థిరరాశి uch ZLIB_అంతర్గత నిడివి_సంకేతం[];
  బాహ్య స్థిరరాశి uch ZLIB_అంతర్గత dist_సంకేతం[];
#అయితే_ముగింపు

#నిర్వచితమైతే LIT_MEM
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->sym_తదుపరి] = 0; \
    s->l_buf[s->sym_తదుపరి++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_తదుపరి == s->sym_చివర); \
   }
# define _tr_tally_dist(s, దూరం, నిడివి, flush) \
  { uch len = (uch)(నిడివి); \
    ush dist = (ush)(దూరం); \
    s->d_buf[s->sym_తదుపరి] = dist; \
    s->l_buf[s->sym_తదుపరి++] = len; \
    dist--; \
    s->dyn_ltree[నిడివి_సంకేతం[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_సంకేతం(dist)].Freq++; \
    flush = (s->sym_తదుపరి == s->sym_చివర); \
  }
#లేకపోతే
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->sym_buf[s->sym_తదుపరి++] = 0; \
    s->sym_buf[s->sym_తదుపరి++] = 0; \
    s->sym_buf[s->sym_తదుపరి++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->sym_తదుపరి == s->sym_చివర); \
   }
# define _tr_tally_dist(s, దూరం, నిడివి, flush) \
  { uch len = (uch)(నిడివి); \
    ush dist = (ush)(దూరం); \
    s->sym_buf[s->sym_తదుపరి++] = (uch)dist; \
    s->sym_buf[s->sym_తదుపరి++] = (uch)(dist >> 8); \
    s->sym_buf[s->sym_తదుపరి++] = len; \
    dist--; \
    s->dyn_ltree[నిడివి_సంకేతం[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_సంకేతం(dist)].Freq++; \
    flush = (s->sym_తదుపరి == s->sym_చివర); \
  }
#అయితే_ముగింపు
#లేకపోతే
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, దూరం, నిడివి, flush) \
              flush = _tr_tally(s, దూరం, నిడివి)
#అయితే_ముగింపు

#అయితే_ముగింపు /* DEFLATE_H */
