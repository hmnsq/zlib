/* deflate.c -- compress data using the deflation algorithm
 * Copyright (C) 1995-2026 Jean-loup Gailly and Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 *  ALGORITHM
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest.
 *      The key feature of this algorithm is that insertions into the string
 *      dictionary are very simple and thus fast, and deletions are avoided
 *      completely. Insertions are performed at each input character, whereas
 *      string matches are performed only when the previous match ends. So it
 *      is preferable to spend more time in matches to allow very fast string
 *      insertions and avoid deletions. The matching algorithm for small
 *      strings is inspired from that of Rabin & Karp. A brute force approach
 *      is used to find longer strings when a small match has been found.
 *      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
 *      (by Leonid Broukhis).
 *         A previous version of this file used a more sophisticated algorithm
 *      (by Fiala and Greene) which is guaranteed to run in linear amortized
 *      time, but has a larger average cost, uses more memory and is patented.
 *      However the F&G algorithm may be faster for some highly redundant
 *      files if the parameter max_chain_length (described below) is too large.
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many people for bug reports and testing.
 *
 *  REFERENCES
 *
 *      Deutsch, L.P.,"DEFLATE Compressed Data Format Specification".
 *      Available at https://datatracker.ietf.org/doc/html/rfc1951
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 */

/* @(#) $Id$ */

#अन्तर्भाव "deflate.h"

ध्रुव वर्ण deflate_copyright[] =
   " deflate 1.3.2.1 Copyright 1995-2026 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

प्रकारनिर्देश गणना {
    need_more,      /* block not completed, need more input or more output */
    block_done,     /* block flush performed */
    finish_started, /* finish started, need only more output at next deflate */
    finish_done     /* finish done, accept no more input or output */
} block_state;

प्रकारनिर्देश block_state (*compress_func)(deflate_state *s, पूर्णाङ्क flush);
/* Compression function. Returns the block state after the call. */

local block_state deflate_stored(deflate_state *s, पूर्णाङ्क flush);
local block_state deflate_fast(deflate_state *s, पूर्णाङ्क flush);
#ifndef FASTEST
local block_state deflate_slow(deflate_state *s, पूर्णाङ्क flush);
#endif
local block_state deflate_rle(deflate_state *s, पूर्णाङ्क flush);
local block_state deflate_huff(deflate_state *s, पूर्णाङ्क flush);

/* ===========================================================================
 * Local data
 */

#define NIL 0
/* Tail of hash chains */

#ifndef TOO_FAR
#  define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
प्रकारनिर्देश संरचना config_s {
   ush good_length; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_length; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#ifdef FASTEST
local ध्रुव config configuration_table[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#अन्यथा
local ध्रुव config configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}, /* max speed, no lazy matches */
/* 2 */ {4,    5, 16,    8, deflate_fast},
/* 3 */ {4,    6, 32,   32, deflate_fast},

/* 4 */ {4,    4, 16,   16, deflate_slow},  /* lazy matches */
/* 5 */ {8,   16, 32,   32, deflate_slow},
/* 6 */ {8,   16, 128, 128, deflate_slow},
/* 7 */ {8,   32, 128, 256, deflate_slow},
/* 8 */ {32, 128, 258, 1024, deflate_slow},
/* 9 */ {32, 258, 258, 4096, deflate_slow}}; /* max compression */
#endif

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#define RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#define UPDATE_HASH(s,h,c) (h = (((h) << s->hash_shift) ^ (c)) & s->hash_mask)


/* ===========================================================================
 * Insert string str in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * If this file is compiled with -DFASTEST, the compression level is forced
 * to 1, and no hash chains are maintained.
 * IN  assertion: all calls to INSERT_STRING are made with consecutive input
 *    characters and the first MIN_MATCH bytes of str are valid (except for
 *    the last MIN_MATCH-1 bytes of the input file).
 */
#ifdef FASTEST
#define जोड़ो_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#अन्यथा
#define जोड़ो_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->prev[(str) & s->w_mask] = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#endif

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#define CLEAR_HASH(s) \
    कुरु { \
        s->head[s->hash_size - 1] = NIL; \
        zmemzero(s->head, (unsigned)(s->hash_size - 1)*परिमाण(*s->head)); \
        s->slid = 0; \
    } यावत् (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#यदि defined(__has_feature)
#  यदि __has_feature(memory_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#endif
local शून्य slide_hash(deflate_state *s) {
    unsigned n, m;
    Posf *p;
    uInt wsize = s->w_size;

    n = s->hash_size;
    p = &s->head[n];
    कुरु {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } यावत् (--n);
#ifndef FASTEST
    n = wsize;
    p = &s->prev[n];
    कुरु {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } यावत् (--n);
#endif
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local unsigned read_buf(z_streamp strm, Bytef *buf, unsigned size) {
    unsigned len = strm->avail_in;

    यदि (len > size) len = size;
    यदि (len == 0) प्रत्यागम 0;

    strm->avail_in  -= len;

    zmemcpy(buf, strm->next_in, len);
    यदि (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#ifdef GZIP
    अन्यथा यदि (strm->state->wrap == 2) {
        strm->adler = crc32(strm->adler, buf, len);
    }
#endif
    strm->next_in  += len;
    strm->total_in += len;

    प्रत्यागम len;
}

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead.
 *
 * IN assertion: lookahead < MIN_LOOKAHEAD
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or avail_in == 0; reads are
 *    performed for at least two bytes (required for the zip translate_eol
 *    option -- not supported here).
 */
local शून्य fill_window(deflate_state *s) {
    unsigned n;
    unsigned more;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_size;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    कुरु {
        more = (unsigned)(s->window_size -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#ifdef _MSC_VER
#pragma warning(धकेलो)
#pragma warning(disable: 4127)
#endif
        यदि (परिमाण(पूर्णाङ्क) <= 2) {
#ifdef _MSC_VER
#pragma warning(निकालो)
#endif
            यदि (more == 0 && s->strstart == 0 && s->lookahead == 0) {
                more = wsize;

            } अन्यथा यदि (more == (unsigned)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                more--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        यदि (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->window, s->window + wsize, (unsigned)wsize - more);
            s->match_start -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->block_start -= (long) wsize;
            यदि (s->जोड़ो > s->strstart)
                s->जोड़ो = s->strstart;
            slide_hash(s);
            more += wsize;
        }
        यदि (s->strm->avail_in == 0) भङ्ग;

        /* If there was no sliding:
         *    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
         *    more == window_size - lookahead - strstart
         * => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
         * => more >= window_size - 2*WSIZE + 2
         * In the BIG_MEM or MMAP case (not yet supported),
         *   window_size == input_size + MIN_LOOKAHEAD  &&
         *   strstart + s->lookahead <= input_size => more >= MIN_LOOKAHEAD.
         * Otherwise, window_size == 2*WSIZE so more >= 2.
         * If there was sliding, more >= WSIZE. So in all cases, more >= 2.
         */
        Assert(more >= 2, "more < 2");

        n = read_buf(s->strm, s->window + s->strstart + s->lookahead, more);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        यदि (s->lookahead + s->जोड़ो >= MIN_MATCH) {
            uInt str = s->strstart - s->जोड़ो;
            s->ins_h = s->window[str];
            UPDATE_HASH(s, s->ins_h, s->window[str + 1]);
#यदि MIN_MATCH != 3
            Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
            यावत् (s->जोड़ो) {
                UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
                s->prev[str & s->w_mask] = s->head[s->ins_h];
#endif
                s->head[s->ins_h] = (Pos)str;
                str++;
                s->जोड़ो--;
                यदि (s->lookahead + s->जोड़ो < MIN_MATCH)
                    भङ्ग;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } यावत् (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    यदि (s->ऊपरी_water < s->window_size) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        यदि (s->ऊपरी_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->window_size - curr;
            यदि (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->window + curr, (unsigned)init);
            s->ऊपरी_water = curr + init;
        }
        अन्यथा यदि (s->ऊपरी_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->ऊपरी_water;
            यदि (init > s->window_size - s->ऊपरी_water)
                init = s->window_size - s->ऊपरी_water;
            zmemzero(s->window + s->ऊपरी_water, (unsigned)init);
            s->ऊपरी_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateInit_(z_streamp strm, पूर्णाङ्क level, ध्रुव वर्ण *version,
                         पूर्णाङ्क stream_size) {
    प्रत्यागम deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, version, stream_size);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateInit2_(z_streamp strm, पूर्णाङ्क level, पूर्णाङ्क method,
                          पूर्णाङ्क windowBits, पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                          ध्रुव वर्ण *version, पूर्णाङ्क stream_size) {
    deflate_state *s;
    पूर्णाङ्क wrap = 1;
    स्थिर ध्रुव वर्ण my_version[] = ZLIB_VERSION;

    यदि (version == Z_NULL || version[0] != my_version[0] ||
        stream_size != परिमाण(z_stream)) {
        प्रत्यागम Z_VERSION_ERROR;
    }
    यदि (strm == Z_NULL) प्रत्यागम Z_STREAM_ERROR;

    strm->संदेश = Z_NULL;
    यदि (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        प्रत्यागम Z_STREAM_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    यदि (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        प्रत्यागम Z_STREAM_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#endif

#ifdef FASTEST
    यदि (level != 0) level = 1;
#अन्यथा
    यदि (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif

    यदि (windowBits < 0) { /* suppress zlib wrapper */
        wrap = 0;
        यदि (windowBits < -15)
            प्रत्यागम Z_STREAM_ERROR;
        windowBits = -windowBits;
    }
#ifdef GZIP
    अन्यथा यदि (windowBits > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        windowBits -= 16;
    }
#endif
    यदि (memLevel < 1 || memLevel > MAX_MEM_LEVEL || method != Z_DEFLATED ||
        windowBits < 8 || windowBits > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (windowBits == 8 && wrap != 1)) {
        प्रत्यागम Z_STREAM_ERROR;
    }
    यदि (windowBits == 8) windowBits = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, परिमाण(deflate_state));
    यदि (s == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(s, परिमाण(deflate_state));
    strm->state = (संरचना internal_state FAR *)s;
    s->strm = strm;
    s->status = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_bits = (uInt)windowBits;
    s->w_size = 1 << s->w_bits;
    s->w_mask = s->w_size - 1;

    s->hash_bits = (uInt)memLevel + 7;
    s->hash_size = 1 << s->hash_bits;
    s->hash_mask = s->hash_size - 1;
    s->hash_shift =  ((s->hash_bits + MIN_MATCH-1) / MIN_MATCH);

    s->window = (Bytef *) ZALLOC(strm, s->w_size, 2*परिमाण(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_size, परिमाण(Pos));
    s->head   = (Posf *)  ZALLOC(strm, s->hash_size, परिमाण(Pos));

    s->ऊपरी_water = 0;      /* nothing written to s->window yet */

    s->lit_bufsize = 1 << (memLevel + 6); /* 16K elements by default */

    /* We overlay pending_buf and sym_buf. This works since the average size
     * for length/distance pairs over any compressed block is assured to be 31
     * bits or less.
     *
     * Analysis: The longest fixed codes are a length code of 8 bits plus 5
     * extra bits, for lengths 131 to 257. The longest fixed distance codes are
     * 5 bits plus 13 extra bits, for distances 16385 to 32768. The longest
     * possible fixed-codes length/distance pair is then 31 bits total.
     *
     * sym_buf starts one-fourth of the way into pending_buf. So there are
     * three bytes in sym_buf for every four bytes in pending_buf. Each symbol
     * in sym_buf is three bytes -- two for the distance and one for the
     * literal/length. As each symbol is consumed, the pointer to the next
     * sym_buf value to read moves forward three bytes. From that symbol, up to
     * 31 bits are written to pending_buf. The closest the written pending_buf
     * bits gets to the next sym_buf symbol to read is just before the last
     * code is written. At that time, 31*(n - 2) bits have been written, just
     * after 24*(n - 2) bits have been consumed from sym_buf. sym_buf starts at
     * 8*n bits into pending_buf. (Note that the symbol buffer fills when n - 1
     * symbols are written.) The closest the writing gets to what is unread is
     * then n + 14 bits. Here n is lit_bufsize, which is 16384 by default, and
     * can range from 128 to 32768.
     *
     * Therefore, at a minimum, there are 142 bits of space between what is
     * written and what is read in the overlain buffers, so the symbols cannot
     * be overwritten by the compressed data. That space is actually 139 bits,
     * due to the three-bit fixed-code block header.
     *
     * That covers the case where either Z_FIXED is specified, forcing fixed
     * codes, or when the use of fixed codes is chosen, because that choice
     * results in a smaller compressed block than dynamic codes. That latter
     * condition then assures that the above analysis also covers all dynamic
     * blocks. A dynamic-code block will only be chosen to be emitted if it has
     * fewer bits than a fixed-code block would for the same set of symbols.
     * Therefore its average symbol length is assured to be less than 31. So
     * the compressed data for a dynamic block also cannot overwrite the
     * symbols from which it is being constructed.
     */

    s->pending_buf = (uchf *) ZALLOC(strm, s->lit_bufsize, LIT_BUFS);
    s->pending_buf_size = (ulg)s->lit_bufsize * 4;

    यदि (s->window == Z_NULL || s->prev == Z_NULL || s->head == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->status = FINISH_STATE;
        strm->संदेश = ERR_संदेश(Z_MEM_ERROR);
        deflateEnd (strm);
        प्रत्यागम Z_MEM_ERROR;
    }
#ifdef LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_end = s->lit_bufsize - 1;
#अन्यथा
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_end = (s->lit_bufsize - 1) * 3;
#endif
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->method = (Byte)method;

    प्रत्यागम deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local पूर्णाङ्क deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    यदि (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        प्रत्यागम 1;
    s = strm->state;
    यदि (s == Z_NULL || s->strm != strm || (s->status != INIT_STATE &&
#ifdef GZIP
                                           s->status != GZIP_STATE &&
#endif
                                           s->status != EXTRA_STATE &&
                                           s->status != NAME_STATE &&
                                           s->status != COMMENT_STATE &&
                                           s->status != HCRC_STATE &&
                                           s->status != BUSY_STATE &&
                                           s->status != FINISH_STATE))
        प्रत्यागम 1;
    प्रत्यागम 0;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateSetDictionary(z_streamp strm, ध्रुव Bytef *dictionary,
                                 uInt  dictLength) {
    deflate_state *s;
    uInt str, n;
    पूर्णाङ्क wrap;
    unsigned avail;
    z_const unsigned वर्ण *next;

    यदि (deflateStateCheck(strm) || dictionary == Z_NULL)
        प्रत्यागम Z_STREAM_ERROR;
    s = strm->state;
    wrap = s->wrap;
    यदि (wrap == 2 || (wrap == 1 && s->status != INIT_STATE) || s->lookahead)
        प्रत्यागम Z_STREAM_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    यदि (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dictLength);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    यदि (dictLength >= s->w_size) {
        यदि (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->block_start = 0L;
            s->जोड़ो = 0;
        }
        dictionary += dictLength - s->w_size;  /* use the tail */
        dictLength = s->w_size;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_in;
    next = strm->next_in;
    strm->avail_in = dictLength;
    strm->next_in = (z_const Bytef *)dictionary;
    fill_window(s);
    यावत् (s->lookahead >= MIN_MATCH) {
        str = s->strstart;
        n = s->lookahead - (MIN_MATCH-1);
        कुरु {
            UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
            s->prev[str & s->w_mask] = s->head[s->ins_h];
#endif
            s->head[s->ins_h] = (Pos)str;
            str++;
        } यावत् (--n);
        s->strstart = str;
        s->lookahead = MIN_MATCH-1;
        fill_window(s);
    }
    s->strstart += s->lookahead;
    s->block_start = (long)s->strstart;
    s->जोड़ो = s->lookahead;
    s->lookahead = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    strm->next_in = next;
    strm->avail_in = avail;
    s->wrap = wrap;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dictLength) {
    deflate_state *s;
    uInt len;

    यदि (deflateStateCheck(strm))
        प्रत्यागम Z_STREAM_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    यदि (len > s->w_size)
        len = s->w_size;
    यदि (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->window + s->strstart + s->lookahead - len, len);
    यदि (dictLength != Z_NULL)
        *dictLength = len;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    यदि (deflateStateCheck(strm)) {
        प्रत्यागम Z_STREAM_ERROR;
    }

    strm->total_in = strm->total_out = 0;
    strm->संदेश = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->data_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_out = s->pending_buf;

    यदि (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->status =
#ifdef GZIP
        s->wrap == 2 ? GZIP_STATE :
#endif
        INIT_STATE;
    strm->adler =
#ifdef GZIP
        s->wrap == 2 ? crc32(0L, Z_NULL, 0) :
#endif
        adler32(0L, Z_NULL, 0);
    s->last_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    प्रत्यागम Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local शून्य lm_init(deflate_state *s) {
    s->window_size = (ulg)2L*s->w_size;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_match   = configuration_table[s->level].max_lazy;
    s->good_match       = configuration_table[s->level].good_length;
    s->nice_match       = configuration_table[s->level].nice_length;
    s->max_chain_length = configuration_table[s->level].max_chain;

    s->strstart = 0;
    s->block_start = 0L;
    s->lookahead = 0;
    s->जोड़ो = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateReset(z_streamp strm) {
    पूर्णाङ्क ret;

    ret = deflateResetKeep(strm);
    यदि (ret == Z_OK)
        lm_init(strm->state);
    प्रत्यागम ret;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateSetHeader(z_streamp strm, gz_headerp head) {
    यदि (deflateStateCheck(strm) || strm->state->wrap != 2)
        प्रत्यागम Z_STREAM_ERROR;
    strm->state->gzhead = head;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflatePending(z_streamp strm, unsigned *pending, पूर्णाङ्क *bits) {
    यदि (deflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    यदि (bits != Z_NULL)
        *bits = strm->state->bi_valid;
    यदि (pending != Z_NULL) {
        *pending = (unsigned)strm->state->pending;
        यदि (*pending != strm->state->pending) {
            *pending = (unsigned)-1;
            प्रत्यागम Z_BUF_ERROR;
        }
    }
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateUsed(z_streamp strm, पूर्णाङ्क *bits) {
    यदि (deflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    यदि (bits != Z_NULL)
        *bits = strm->state->bi_used;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflate_अभाज्य(z_streamp strm, पूर्णाङ्क bits, पूर्णाङ्क मूल्य) {
    deflate_state *s;
    पूर्णाङ्क put;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    s = strm->state;
#ifdef LIT_MEM
    यदि (bits < 0 || bits > 16 ||
        (uchf *)s->d_buf < s->pending_out + ((Buf_size + 7) >> 3))
        प्रत्यागम Z_BUF_ERROR;
#अन्यथा
    यदि (bits < 0 || bits > 16 ||
        s->sym_buf < s->pending_out + ((Buf_size + 7) >> 3))
        प्रत्यागम Z_BUF_ERROR;
#endif
    कुरु {
        put = Buf_size - s->bi_valid;
        यदि (put > bits)
            put = bits;
        s->bi_buf |= (ush)((मूल्य & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        _tr_flush_bits(s);
        मूल्य >>= put;
        bits -= put;
    } यावत् (bits);
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateParams(z_streamp strm, पूर्णाङ्क level, पूर्णाङ्क strategy) {
    deflate_state *s;
    compress_func func;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    s = strm->state;

#ifdef FASTEST
    यदि (level != 0) level = 1;
#अन्यथा
    यदि (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif
    यदि (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        प्रत्यागम Z_STREAM_ERROR;
    }
    func = configuration_table[s->level].func;

    यदि ((strategy != s->strategy || func != configuration_table[level].func) &&
        s->last_flush != -2) {
        /* Flush the last buffer: */
        पूर्णाङ्क err = deflate(strm, Z_BLOCK);
        यदि (err == Z_STREAM_ERROR)
            प्रत्यागम err;
        यदि (strm->avail_in || (s->strstart - s->block_start) + s->lookahead)
            प्रत्यागम Z_BUF_ERROR;
    }
    यदि (s->level != level) {
        यदि (s->level == 0 && s->matches != 0) {
            यदि (s->matches == 1)
                slide_hash(s);
            अन्यथा
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_match   = configuration_table[level].max_lazy;
        s->good_match       = configuration_table[level].good_length;
        s->nice_match       = configuration_table[level].nice_length;
        s->max_chain_length = configuration_table[level].max_chain;
    }
    s->strategy = strategy;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateTune(z_streamp strm, पूर्णाङ्क good_length, पूर्णाङ्क max_lazy,
                        पूर्णाङ्क nice_length, पूर्णाङ्क max_chain) {
    deflate_state *s;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;
    s = strm->state;
    s->good_match = (uInt)good_length;
    s->max_lazy_match = (uInt)max_lazy;
    s->nice_match = nice_length;
    s->max_chain_length = (uInt)max_chain;
    प्रत्यागम Z_OK;
}

/* =========================================================================
 * For the default windowBits of 15 and memLevel of 8, this function returns a
 * close to exact, as well as small, upper bound on the compressed size. This
 * is an expansion of ~0.03%, plus a small constant.
 *
 * For any setting other than those defaults for windowBits and memLevel, one
 * of two worst case bounds is returned. This is at most an expansion of ~4% or
 * ~13%, plus a small constant.
 *
 * Both the 0.03% and 4% derive from the overhead of stored blocks. The first
 * one is for stored blocks of 16383 bytes (memLevel == 8), whereas the second
 * is for stored blocks of 127 bytes (the worst case memLevel == 1). The
 * expansion results from five bytes of header for each stored block.
 *
 * The larger expansion of 13% results from a window size less than or equal to
 * the symbols buffer size (windowBits <= memLevel + 7). In that case some of
 * the data being compressed may have slid out of the sliding window, impeding
 * a stored block from being emitted. Then the only choice is a fixed or
 * dynamic block, where a fixed block limits the maximum expansion to 9 bits
 * per 8-bit byte, plus 10 bits for every block. The smallest block size for
 * which this can occur is 255 (memLevel == 2).
 *
 * Shifts are used to approximate divisions, for speed.
 */
z_size_t ZEXPORT deflateBound_z(z_streamp strm, z_size_t sourceLen) {
    deflate_state *s;
    z_size_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = sourceLen + (sourceLen >> 3) + (sourceLen >> 8) +
               (sourceLen >> 9) + 4;
    यदि (fixedlen < sourceLen)
        fixedlen = (z_size_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = sourceLen + (sourceLen >> 5) + (sourceLen >> 7) +
               (sourceLen >> 11) + 7;
    यदि (storelen < sourceLen)
        storelen = (z_size_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    यदि (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        प्रत्यागम bound + 18 < bound ? (z_size_t)-1 : bound + 18;
    }

    /* compute wrapper length */
    s = strm->state;
    विकल्प (s->wrap < 0 ? -s->wrap : s->wrap) {
    पक्ष 0:                                 /* raw deflate */
        wraplen = 0;
        भङ्ग;
    पक्ष 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        भङ्ग;
#ifdef GZIP
    पक्ष 2:                                 /* gzip wrapper */
        wraplen = 18;
        यदि (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            यदि (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            यदि (str != Z_NULL)
                कुरु {
                    wraplen++;
                } यावत् (*str++);
            str = s->gzhead->comment;
            यदि (str != Z_NULL)
                कुरु {
                    wraplen++;
                } यावत् (*str++);
            यदि (s->gzhead->hcrc)
                wraplen += 2;
        }
        भङ्ग;
#endif
    स्वभाव:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    यदि (s->w_bits != 15 || s->hash_bits != 8 + 7) {
        bound = s->w_bits <= s->hash_bits && s->level ? fixedlen :
                                                        storelen;
        प्रत्यागम bound + wraplen < bound ? (z_size_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
            (sourceLen >> 25) + 13 - 6 + wraplen;
    प्रत्यागम bound < sourceLen ? (z_size_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong sourceLen) {
    z_size_t bound = deflateBound_z(strm, sourceLen);
    प्रत्यागम (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local शून्य putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local शून्य flush_pending(z_streamp strm) {
    unsigned len;
    deflate_state *s = strm->state;

    _tr_flush_bits(s);
    len = s->pending > strm->avail_out ? strm->avail_out :
                                         (unsigned)s->pending;
    यदि (len == 0) प्रत्यागम;

    zmemcpy(strm->next_out, s->pending_out, len);
    strm->next_out  += len;
    s->pending_out  += len;
    strm->total_out += len;
    strm->avail_out -= len;
    s->pending      -= len;
    यदि (s->pending == 0) {
        s->pending_out = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#define HCRC_UPDATE(beg) \
    कुरु { \
        यदि (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = crc32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } यावत् (0)

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflate(z_streamp strm, पूर्णाङ्क flush) {
    पूर्णाङ्क old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    यदि (deflateStateCheck(strm) || flush > Z_BLOCK || flush < 0) {
        प्रत्यागम Z_STREAM_ERROR;
    }
    s = strm->state;

    यदि (strm->next_out == Z_NULL ||
        (strm->avail_in != 0 && strm->next_in == Z_NULL) ||
        (s->status == FINISH_STATE && flush != Z_FINISH)) {
        ERR_RETURN(strm, Z_STREAM_ERROR);
    }
    यदि (strm->avail_out == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->last_flush;
    s->last_flush = flush;

    /* Flush as much pending output as possible */
    यदि (s->pending != 0) {
        flush_pending(strm);
        यदि (strm->avail_out == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->last_flush = -1;
            प्रत्यागम Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } अन्यथा यदि (strm->avail_in == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_FINISH) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    यदि (s->status == FINISH_STATE && strm->avail_in != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    यदि (s->status == INIT_STATE && s->wrap == 0)
        s->status = BUSY_STATE;
    यदि (s->status == INIT_STATE) {
        /* zlib header */
        uInt header = (Z_DEFLATED + ((s->w_bits - 8) << 4)) << 8;
        uInt level_flags;

        यदि (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        अन्यथा यदि (s->level < 6)
            level_flags = 1;
        अन्यथा यदि (s->level == 6)
            level_flags = 2;
        अन्यथा
            level_flags = 3;
        header |= (level_flags << 6);
        यदि (s->strstart != 0) header |= PRESET_DICT;
        header += 31 - (header % 31);

        putShortMSB(s, header);

        /* Save the adler32 of the preset dictionary: */
        यदि (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        यदि (s->pending != 0) {
            s->last_flush = -1;
            प्रत्यागम Z_OK;
        }
    }
#ifdef GZIP
    यदि (s->status == GZIP_STATE) {
        /* gzip header */
        strm->adler = crc32(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        यदि (s->gzhead == Z_NULL) {
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, OS_CODE);
            s->status = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            यदि (s->pending != 0) {
                s->last_flush = -1;
                प्रत्यागम Z_OK;
            }
        }
        अन्यथा {
            put_byte(s, (s->gzhead->text ? 1 : 0) +
                     (s->gzhead->hcrc ? 2 : 0) +
                     (s->gzhead->extra == Z_NULL ? 0 : 4) +
                     (s->gzhead->name == Z_NULL ? 0 : 8) +
                     (s->gzhead->comment == Z_NULL ? 0 : 16)
                     );
            put_byte(s, (Byte)(s->gzhead->time & 0xff));
            put_byte(s, (Byte)((s->gzhead->time >> 8) & 0xff));
            put_byte(s, (Byte)((s->gzhead->time >> 16) & 0xff));
            put_byte(s, (Byte)((s->gzhead->time >> 24) & 0xff));
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, s->gzhead->os & 0xff);
            यदि (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            यदि (s->gzhead->hcrc)
                strm->adler = crc32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->status = EXTRA_STATE;
        }
    }
    यदि (s->status == EXTRA_STATE) {
        यदि (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg बायाँ = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            यावत् (s->pending + बायाँ > s->pending_buf_size) {
                ulg copy = s->pending_buf_size - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_size;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                यदि (s->pending != 0) {
                    s->last_flush = -1;
                    प्रत्यागम Z_OK;
                }
                beg = 0;
                बायाँ -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, बायाँ);
            s->pending += बायाँ;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = NAME_STATE;
    }
    यदि (s->status == NAME_STATE) {
        यदि (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            पूर्णाङ्क val;
            कुरु {
                यदि (s->pending == s->pending_buf_size) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    यदि (s->pending != 0) {
                        s->last_flush = -1;
                        प्रत्यागम Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } यावत् (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = COMMENT_STATE;
    }
    यदि (s->status == COMMENT_STATE) {
        यदि (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            पूर्णाङ्क val;
            कुरु {
                यदि (s->pending == s->pending_buf_size) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    यदि (s->pending != 0) {
                        s->last_flush = -1;
                        प्रत्यागम Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } यावत् (val != 0);
            HCRC_UPDATE(beg);
        }
        s->status = HCRC_STATE;
    }
    यदि (s->status == HCRC_STATE) {
        यदि (s->gzhead->hcrc) {
            यदि (s->pending + 2 > s->pending_buf_size) {
                flush_pending(strm);
                यदि (s->pending != 0) {
                    s->last_flush = -1;
                    प्रत्यागम Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = crc32(0L, Z_NULL, 0);
        }
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        यदि (s->pending != 0) {
            s->last_flush = -1;
            प्रत्यागम Z_OK;
        }
    }
#endif

    /* Start a new block or continue the current one.
     */
    यदि (strm->avail_in != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->status != FINISH_STATE)) {
        block_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_table[s->level].func))(s, flush);

        यदि (bstate == finish_started || bstate == finish_done) {
            s->status = FINISH_STATE;
        }
        यदि (bstate == need_more || bstate == finish_started) {
            यदि (strm->avail_out == 0) {
                s->last_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            प्रत्यागम Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        यदि (bstate == block_done) {
            यदि (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } अन्यथा यदि (flush != Z_BLOCK) { /* FULL_FLUSH or SYNC_FLUSH */
                _tr_stored_block(s, (वर्ण*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                यदि (flush == Z_FULL_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    यदि (s->lookahead == 0) {
                        s->strstart = 0;
                        s->block_start = 0L;
                        s->जोड़ो = 0;
                    }
                }
            }
            flush_pending(strm);
            यदि (strm->avail_out == 0) {
              s->last_flush = -1; /* avoid BUF_ERROR at next call, see above */
              प्रत्यागम Z_OK;
            }
        }
    }

    यदि (flush != Z_FINISH) प्रत्यागम Z_OK;
    यदि (s->wrap <= 0) प्रत्यागम Z_STREAM_END;

    /* Write the trailer */
#ifdef GZIP
    यदि (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->total_in & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 8) & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 16) & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 24) & 0xff));
    }
    अन्यथा
#endif
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    यदि (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    प्रत्यागम s->pending != 0 ? Z_OK : Z_STREAM_END;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateEnd(z_streamp strm) {
    पूर्णाङ्क status;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_STREAM_ERROR;

    status = strm->state->status;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->head);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->window);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    प्रत्यागम status == BUSY_STATE ? Z_DATA_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
पूर्णाङ्क ZEXPORT deflateCopy(z_streamp dest, z_streamp source) {
#ifdef MAXSEG_64K
    (शून्य)dest;
    (शून्य)source;
    प्रत्यागम Z_STREAM_ERROR;
#अन्यथा
    deflate_state *ds;
    deflate_state *ss;


    यदि (deflateStateCheck(source) || dest == Z_NULL) {
        प्रत्यागम Z_STREAM_ERROR;
    }

    ss = source->state;

    zmemcpy(dest, source, परिमाण(z_stream));

    ds = (deflate_state *) ZALLOC(dest, 1, परिमाण(deflate_state));
    यदि (ds == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(ds, परिमाण(deflate_state));
    dest->state = (संरचना internal_state FAR *) ds;
    zmemcpy(ds, ss, परिमाण(deflate_state));
    ds->strm = dest;

    ds->window = (Bytef *) ZALLOC(dest, ds->w_size, 2*परिमाण(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_size, परिमाण(Pos));
    ds->head   = (Posf *)  ZALLOC(dest, ds->hash_size, परिमाण(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    यदि (ds->window == Z_NULL || ds->prev == Z_NULL || ds->head == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflateEnd (dest);
        प्रत्यागम Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->window, ss->window, ss->ऊपरी_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->जोड़ो > ds->w_size ? ds->w_size :
                ss->strstart - ss->जोड़ो) * परिमाण(Pos));
    zmemcpy(ds->head, ss->head, ds->hash_size * परिमाण(Pos));

    ds->pending_out = ds->pending_buf + (ss->pending_out - ss->pending_buf);
    zmemcpy(ds->pending_out, ss->pending_out, ss->pending);
#ifdef LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_next * परिमाण(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_next);
#अन्यथा
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_next);
#endif

    ds->l_desc.dyn_वृक्ष = ds->dyn_ltree;
    ds->d_desc.dyn_वृक्ष = ds->dyn_dtree;
    ds->bl_desc.dyn_वृक्ष = ds->bl_वृक्ष;

    प्रत्यागम Z_OK;
#endif /* MAXSEG_64K */
}

#ifndef FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_match(deflate_state *s, IPos cur_match) {
    unsigned chain_length = s->max_chain_length;/* max hash chain length */
    Bytef *scan = s->window + s->strstart;      /* current string */
    Bytef *match;                               /* matched string */
    पूर्णाङ्क len;                                    /* length of current match */
    पूर्णाङ्क best_len = (पूर्णाङ्क)s->prev_length;         /* best match length so far */
    पूर्णाङ्क nice_match = s->nice_match;             /* stop if match long enough */
    IPos limit = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_mask;

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->window + s->strstart + MAX_MATCH - 1;
    ush scan_start = *(ushf*)scan;
    ush scan_end   = *(ushf*)(scan + best_len - 1);
#अन्यथा
    Bytef *strend = s->window + s->strstart + MAX_MATCH;
    Byte scan_end1  = scan[best_len - 1];
    Byte scan_end   = scan[best_len];
#endif

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    यदि (s->prev_length >= s->good_match) {
        chain_length >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    यदि ((uInt)nice_match > s->lookahead) nice_match = (पूर्णाङ्क)s->lookahead;

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "need lookahead");

    कुरु {
        Assert(cur_match < s->strstart, "no future");
        match = s->window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#यदि (defined(UNALIGNED_OK) && MAX_MATCH == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        यदि (*(ushf*)(match + best_len - 1) != scan_end ||
            *(ushf*)match != scan_start) अनुवर्त;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(scan[2] == match[2], "scan[2]?");
        scan++, match++;
        कुरु {
        } यावत् (*(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(scan <= s->window + (unsigned)(s->window_size - 1),
               "wild scan");
        यदि (*scan == *match) scan++;

        len = (MAX_MATCH - 1) - (पूर्णाङ्क)(strend - scan);
        scan = strend - (MAX_MATCH-1);

#अन्यथा /* UNALIGNED_OK */

        यदि (match[best_len]     != scan_end  ||
            match[best_len - 1] != scan_end1 ||
            *match              != *scan     ||
            *++match            != scan[1])      अनुवर्त;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        scan += 2, match++;
        Assert(*scan == *match, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        कुरु {
        } यावत् (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        Assert(scan <= s->window + (unsigned)(s->window_size - 1),
               "wild scan");

        len = MAX_MATCH - (पूर्णाङ्क)(strend - scan);
        scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

        यदि (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            यदि (len >= nice_match) भङ्ग;
#ifdef UNALIGNED_OK
            scan_end = *(ushf*)(scan + best_len - 1);
#अन्यथा
            scan_end1  = scan[best_len - 1];
            scan_end   = scan[best_len];
#endif
        }
    } यावत् ((cur_match = prev[cur_match & wmask]) > limit
             && --chain_length != 0);

    यदि ((uInt)best_len <= s->lookahead) प्रत्यागम (uInt)best_len;
    प्रत्यागम s->lookahead;
}

#अन्यथा /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_match(deflate_state *s, IPos cur_match) {
    Bytef *scan = s->window + s->strstart;      /* current string */
    Bytef *match;                               /* matched string */
    पूर्णाङ्क len;                                    /* length of current match */
    Bytef *strend = s->window + s->strstart + MAX_MATCH;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_match < s->strstart, "no future");

    match = s->window + cur_match;

    /* Return failure if the match length is less than 2:
     */
    यदि (match[0] != scan[0] || match[1] != scan[1]) प्रत्यागम MIN_MATCH-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    scan += 2, match += 2;
    Assert(*scan == *match, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    कुरु {
    } यावत् (*++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             scan < strend);

    Assert(scan <= s->window + (unsigned)(s->window_size - 1), "wild scan");

    len = MAX_MATCH - (पूर्णाङ्क)(strend - scan);

    यदि (len < MIN_MATCH) प्रत्यागम MIN_MATCH - 1;

    s->match_start = cur_match;
    प्रत्यागम (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#endif /* FASTEST */

#ifdef ZLIB_DEBUG

#define EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local शून्य check_match(deflate_state *s, IPos start, IPos match, पूर्णाङ्क length) {
    /* check that the match is indeed a match */
    Bytef *back = s->window + (पूर्णाङ्क)match, *here = s->window + start;
    IPos len = (IPos)length;
    यदि (match == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        back++;
        here++;
        len--;
    }
    यदि (zmemcmp(back, here, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                start, (पूर्णाङ्क)match, length);
        कुरु {
            fprintf(stderr, "(%02x %02x)", *back++, *here++);
        } यावत् (--len != 0);
        z_error("invalid match");
    }
    यदि (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", start - match, length);
        कुरु { putc(s->window[start++], stderr); } यावत् (--length != 0);
    }
}
#अन्यथा
#  define check_match(s, start, match, length)
#endif /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define FLUSH_BLOCK_ONLY(s, last) { \
   _tr_flush_block(s, (s->block_start >= 0L ? \
                   (charf *)&s->window[(unsigned)s->block_start] : \
                   (charf *)Z_NULL), \
                (ulg)((long)s->strstart - s->block_start), \
                (last)); \
   s->block_start = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#define FLUSH_BLOCK(s, last) { \
   FLUSH_BLOCK_ONLY(s, last); \
   यदि (s->strm->avail_out == 0) प्रत्यागम (last) ? finish_started : need_more; \
}

/* Maximum stored block length in deflate format (not including header). */
#define MAX_STORED 65535

/* Minimum of a and b. */
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/* ===========================================================================
 * Copy without compression as much as possible from the input stream, return
 * the current block state.
 *
 * In case deflateParams() is used to later switch to a non-zero compression
 * level, s->matches (otherwise unused when storing) keeps track of the number
 * of hash table slides to perform. If s->matches is 1, then one hash table
 * slide will be done when switching. If s->matches is 2, the maximum value
 * allowed here, then the hash table will be cleared, since two or more slides
 * is the same as a clear.
 *
 * deflate_stored() is written to minimize the number of times an input byte is
 * copied. It is most efficient with large input and output buffers, which
 * maximizes the opportunities to have a single copy from next_in to next_out.
 */
local block_state deflate_stored(deflate_state *s, पूर्णाङ्क flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    unsigned min_block = (unsigned)(MIN(s->pending_buf_size - 5, s->w_size));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    पूर्णाङ्क last = 0;
    unsigned len, बायाँ, have;
    unsigned used = s->strm->avail_in;
    कुरु {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((unsigned)s->bi_valid + 42) >> 3;   /* bytes in header */
        यदि (s->strm->avail_out < have)          /* need room for header */
            भङ्ग;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_out - have;
        बायाँ = (unsigned)(s->strstart - s->block_start);    /* window bytes */
        यदि (len > (ulg)बायाँ + s->strm->avail_in)
            len = बायाँ + s->strm->avail_in;     /* limit len to the input */
        यदि (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        यदि (len < min_block && ((len == 0 && flush != Z_FINISH) ||
                                flush == Z_NO_FLUSH ||
                                len != बायाँ + s->strm->avail_in))
            भङ्ग;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        last = flush == Z_FINISH && len == बायाँ + s->strm->avail_in ? 1 : 0;
        _tr_stored_block(s, (वर्ण *)0, 0L, last);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#ifdef ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->bits_sent += len << 3;
#endif

        /* Copy uncompressed bytes from the window to next_out. */
        यदि (बायाँ) {
            यदि (बायाँ > len)
                बायाँ = len;
            zmemcpy(s->strm->next_out, s->window + s->block_start, बायाँ);
            s->strm->next_out += बायाँ;
            s->strm->avail_out -= बायाँ;
            s->strm->total_out += बायाँ;
            s->block_start += बायाँ;
            len -= बायाँ;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        यदि (len) {
            read_buf(s->strm, s->strm->next_out, len);
            s->strm->next_out += len;
            s->strm->avail_out -= len;
            s->strm->total_out += len;
        }
    } यावत् (last == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    used -= s->strm->avail_in;      /* number of input bytes directly copied */
    यदि (used) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        यदि (used >= s->w_size) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->window, s->strm->next_in - s->w_size, s->w_size);
            s->strstart = s->w_size;
            s->जोड़ो = s->strstart;
        }
        अन्यथा {
            यदि (s->window_size - s->strstart <= used) {
                /* Slide the window down. */
                s->strstart -= s->w_size;
                zmemcpy(s->window, s->window + s->w_size, s->strstart);
                यदि (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                यदि (s->जोड़ो > s->strstart)
                    s->जोड़ो = s->strstart;
            }
            zmemcpy(s->window + s->strstart, s->strm->next_in - used, used);
            s->strstart += used;
            s->जोड़ो += MIN(used, s->w_size - s->जोड़ो);
        }
        s->block_start = s->strstart;
    }
    यदि (s->ऊपरी_water < s->strstart)
        s->ऊपरी_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    यदि (last) {
        s->bi_used = 8;
        प्रत्यागम finish_done;
    }

    /* If flushing and all input has been consumed, then done. */
    यदि (flush != Z_NO_FLUSH && flush != Z_FINISH &&
        s->strm->avail_in == 0 && (long)s->strstart == s->block_start)
        प्रत्यागम block_done;

    /* Fill the window with any remaining input. */
    have = (unsigned)(s->window_size - s->strstart);
    यदि (s->strm->avail_in > have && s->block_start >= (long)s->w_size) {
        /* Slide the window down. */
        s->block_start -= s->w_size;
        s->strstart -= s->w_size;
        zmemcpy(s->window, s->window + s->w_size, s->strstart);
        यदि (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_size;          /* more space now */
        यदि (s->जोड़ो > s->strstart)
            s->जोड़ो = s->strstart;
    }
    यदि (have > s->strm->avail_in)
        have = s->strm->avail_in;
    यदि (have) {
        read_buf(s->strm, s->window + s->strstart, have);
        s->strstart += have;
        s->जोड़ो += MIN(have, s->w_size - s->जोड़ो);
    }
    यदि (s->ऊपरी_water < s->strstart)
        s->ऊपरी_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((unsigned)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (unsigned)MIN(s->pending_buf_size - have, MAX_STORED);
    min_block = MIN(have, s->w_size);
    बायाँ = (unsigned)(s->strstart - s->block_start);
    यदि (बायाँ >= min_block ||
        ((बायाँ || flush == Z_FINISH) && flush != Z_NO_FLUSH &&
         s->strm->avail_in == 0 && बायाँ <= have)) {
        len = MIN(बायाँ, have);
        last = flush == Z_FINISH && s->strm->avail_in == 0 &&
               len == बायाँ ? 1 : 0;
        _tr_stored_block(s, (charf *)s->window + s->block_start, len, last);
        s->block_start += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    यदि (last)
        s->bi_used = 8;
    प्रत्यागम last ? finish_started : need_more;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local block_state deflate_fast(deflate_state *s, पूर्णाङ्क flush) {
    IPos hash_head;       /* head of the hash chain */
    पूर्णाङ्क bflush;           /* set if current block must be flushed */

    प्रति (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        यदि (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            यदि (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                प्रत्यागम need_more;
            }
            यदि (s->lookahead == 0) भङ्ग; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_head = NIL;
        यदि (s->lookahead >= MIN_MATCH) {
            जोड़ो_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        यदि (hash_head != NIL && s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_length = longest_match (s, hash_head);
            /* longest_match() sets match_start */
        }
        यदि (s->match_length >= MIN_MATCH) {
            check_match(s, s->strstart, s->match_start, (पूर्णाङ्क)s->match_length);

            _tr_tally_dist(s, s->strstart - s->match_start,
                           s->match_length - MIN_MATCH, bflush);

            s->lookahead -= s->match_length;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#ifndef FASTEST
            यदि (s->match_length <= s->max_जोड़ो_length &&
                s->lookahead >= MIN_MATCH) {
                s->match_length--; /* string at strstart already in table */
                कुरु {
                    s->strstart++;
                    जोड़ो_STRING(s, s->strstart, hash_head);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } यावत् (--s->match_length != 0);
                s->strstart++;
            } अन्यथा
#endif
            {
                s->strstart += s->match_length;
                s->match_length = 0;
                s->ins_h = s->window[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->window[s->strstart + 1]);
#यदि MIN_MATCH != 3
                Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } अन्यथा {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        यदि (bflush) FLUSH_BLOCK(s, 0);
    }
    s->जोड़ो = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    यदि (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        प्रत्यागम finish_done;
    }
    यदि (s->sym_next)
        FLUSH_BLOCK(s, 0);
    प्रत्यागम block_done;
}

#ifndef FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local block_state deflate_slow(deflate_state *s, पूर्णाङ्क flush) {
    IPos hash_head;          /* head of hash chain */
    पूर्णाङ्क bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    प्रति (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        यदि (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            यदि (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                प्रत्यागम need_more;
            }
            यदि (s->lookahead == 0) भङ्ग; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_head = NIL;
        यदि (s->lookahead >= MIN_MATCH) {
            जोड़ो_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = MIN_MATCH-1;

        यदि (hash_head != NIL && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_length = longest_match (s, hash_head);
            /* longest_match() sets match_start */

            यदि (s->match_length <= 5 && (s->strategy == Z_FILTERED
#यदि TOO_FAR <= 32767
                || (s->match_length == MIN_MATCH &&
                    s->strstart - s->match_start > TOO_FAR)
#endif
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->match_length = MIN_MATCH-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        यदि (s->prev_length >= MIN_MATCH && s->match_length <= s->prev_length) {
            uInt max_जोड़ो = s->strstart + s->lookahead - MIN_MATCH;
            /* Do not insert strings in hash table beyond this. */

            check_match(s, s->strstart - 1, s->prev_match, (पूर्णाङ्क)s->prev_length);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_match,
                           s->prev_length - MIN_MATCH, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_length - 1;
            s->prev_length -= 2;
            कुरु {
                यदि (++s->strstart <= max_जोड़ो) {
                    जोड़ो_STRING(s, s->strstart, hash_head);
                }
            } यावत् (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = MIN_MATCH-1;
            s->strstart++;

            यदि (bflush) FLUSH_BLOCK(s, 0);

        } अन्यथा यदि (s->match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->window[s->strstart - 1]));
            _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
            यदि (bflush) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            यदि (s->strm->avail_out == 0) प्रत्यागम need_more;
        } अन्यथा {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    यदि (s->match_available) {
        Tracevv((stderr,"%c", s->window[s->strstart - 1]));
        _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
        s->match_available = 0;
    }
    s->जोड़ो = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    यदि (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        प्रत्यागम finish_done;
    }
    यदि (s->sym_next)
        FLUSH_BLOCK(s, 0);
    प्रत्यागम block_done;
}
#endif /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local block_state deflate_rle(deflate_state *s, पूर्णाङ्क flush) {
    पूर्णाङ्क bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *scan, *strend;   /* scan goes up to strend for length of run */

    प्रति (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        यदि (s->lookahead <= MAX_MATCH) {
            fill_window(s);
            यदि (s->lookahead <= MAX_MATCH && flush == Z_NO_FLUSH) {
                प्रत्यागम need_more;
            }
            यदि (s->lookahead == 0) भङ्ग; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->match_length = 0;
        यदि (s->lookahead >= MIN_MATCH && s->strstart > 0) {
            scan = s->window + s->strstart - 1;
            prev = *scan;
            यदि (prev == *++scan && prev == *++scan && prev == *++scan) {
                strend = s->window + s->strstart + MAX_MATCH;
                कुरु {
                } यावत् (prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         scan < strend);
                s->match_length = MAX_MATCH - (uInt)(strend - scan);
                यदि (s->match_length > s->lookahead)
                    s->match_length = s->lookahead;
            }
            Assert(scan <= s->window + (uInt)(s->window_size - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        यदि (s->match_length >= MIN_MATCH) {
            check_match(s, s->strstart, s->strstart - 1, (पूर्णाङ्क)s->match_length);

            _tr_tally_dist(s, 1, s->match_length - MIN_MATCH, bflush);

            s->lookahead -= s->match_length;
            s->strstart += s->match_length;
            s->match_length = 0;
        } अन्यथा {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        यदि (bflush) FLUSH_BLOCK(s, 0);
    }
    s->जोड़ो = 0;
    यदि (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        प्रत्यागम finish_done;
    }
    यदि (s->sym_next)
        FLUSH_BLOCK(s, 0);
    प्रत्यागम block_done;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local block_state deflate_huff(deflate_state *s, पूर्णाङ्क flush) {
    पूर्णाङ्क bflush;             /* set if current block must be flushed */

    प्रति (;;) {
        /* Make sure that we have a literal to write. */
        यदि (s->lookahead == 0) {
            fill_window(s);
            यदि (s->lookahead == 0) {
                यदि (flush == Z_NO_FLUSH)
                    प्रत्यागम need_more;
                भङ्ग;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->match_length = 0;
        Tracevv((stderr,"%c", s->window[s->strstart]));
        _tr_tally_lit(s, s->window[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        यदि (bflush) FLUSH_BLOCK(s, 0);
    }
    s->जोड़ो = 0;
    यदि (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        प्रत्यागम finish_done;
    }
    यदि (s->sym_next)
        FLUSH_BLOCK(s, 0);
    प्रत्यागम block_done;
}
