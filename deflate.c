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

#অন্তর্ভাব "deflate.h"

ধ্রুবক অক্ষর deflate_copyright[] =
   " deflate 1.3.2.1 Copyright 1995-2026 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

প্রকারলেখ গণনা {
    need_more,      /* block not completed, need more input or more output */
    block_done,     /* block flush performed */
    finish_started, /* finish started, need only more output at next deflate */
    finish_done     /* finish done, accept no more input or output */
} block_state;

প্রকারলেখ block_state (*compress_func)(deflate_state *s, পূর্ণসংখ্যা flush);
/* Compression function. Returns the block state after the call. */

local block_state deflate_stored(deflate_state *s, পূর্ণসংখ্যা flush);
local block_state deflate_fast(deflate_state *s, পূর্ণসংখ্যা flush);
#ifndef FASTEST
local block_state deflate_slow(deflate_state *s, পূর্ণসংখ্যা flush);
#endif
local block_state deflate_rle(deflate_state *s, পূর্ণসংখ্যা flush);
local block_state deflate_huff(deflate_state *s, পূর্ণসংখ্যা flush);

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
প্রকারলেখ কাঠামো config_s {
   ush good_দৈর্ঘ্য; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_দৈর্ঘ্য; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#ifdef FASTEST
local ধ্রুবক config configuration_table[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#অন্যথা
local ধ্রুবক config configuration_table[10] = {
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
#define ঢোকাও_STRING(s, str, match_শীর্ষ) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_শীর্ষ = s->শীর্ষ[s->ins_h], \
    s->শীর্ষ[s->ins_h] = (Pos)(str))
#অন্যথা
#define ঢোকাও_STRING(s, str, match_শীর্ষ) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_শীর্ষ = s->পূর্ব[(str) & s->w_mask] = s->শীর্ষ[s->ins_h], \
    s->শীর্ষ[s->ins_h] = (Pos)(str))
#endif

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#define CLEAR_HASH(s) \
    করো { \
        s->শীর্ষ[s->hash_আকার - 1] = NIL; \
        zmemzero(s->শীর্ষ, (unsigned)(s->hash_আকার - 1)*আকার(*s->শীর্ষ)); \
        s->slid = 0; \
    } যখন (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#যদি defined(__has_feature)
#  যদি __has_feature(memory_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#endif
local শূন্য slide_hash(deflate_state *s) {
    unsigned n, m;
    Posf *p;
    uInt wsize = s->w_আকার;

    n = s->hash_আকার;
    p = &s->শীর্ষ[n];
    করো {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } যখন (--n);
#ifndef FASTEST
    n = wsize;
    p = &s->পূর্ব[n];
    করো {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } যখন (--n);
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
local unsigned read_buf(z_streamp strm, Bytef *buf, unsigned আকার_2) {
    unsigned len = strm->avail_in;

    যদি (len > আকার_2) len = আকার_2;
    যদি (len == 0) ফেরত 0;

    strm->avail_in  -= len;

    zmemcpy(buf, strm->পরবর্তী_in, len);
    যদি (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#ifdef GZIP
    অন্যথা যদি (strm->state->wrap == 2) {
        strm->adler = crc32(strm->adler, buf, len);
    }
#endif
    strm->পরবর্তী_in  += len;
    strm->যোগ_in += len;

    ফেরত len;
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
local শূন্য fill_window(deflate_state *s) {
    unsigned n;
    unsigned more;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_আকার;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    করো {
        more = (unsigned)(s->window_আকার -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#ifdef _MSC_VER
#pragma warning(ঠেলো)
#pragma warning(disable: 4127)
#endif
        যদি (আকার(পূর্ণসংখ্যা) <= 2) {
#ifdef _MSC_VER
#pragma warning(বের_করো)
#endif
            যদি (more == 0 && s->strstart == 0 && s->lookahead == 0) {
                more = wsize;

            } অন্যথা যদি (more == (unsigned)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                more--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        যদি (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->window, s->window + wsize, (unsigned)wsize - more);
            s->match_start -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->block_start -= (long) wsize;
            যদি (s->ঢোকাও > s->strstart)
                s->ঢোকাও = s->strstart;
            slide_hash(s);
            more += wsize;
        }
        যদি (s->strm->avail_in == 0) বিরতি;

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
        যদি (s->lookahead + s->ঢোকাও >= MIN_MATCH) {
            uInt str = s->strstart - s->ঢোকাও;
            s->ins_h = s->window[str];
            UPDATE_HASH(s, s->ins_h, s->window[str + 1]);
#যদি MIN_MATCH != 3
            Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
            যখন (s->ঢোকাও) {
                UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
                s->পূর্ব[str & s->w_mask] = s->শীর্ষ[s->ins_h];
#endif
                s->শীর্ষ[s->ins_h] = (Pos)str;
                str++;
                s->ঢোকাও--;
                যদি (s->lookahead + s->ঢোকাও < MIN_MATCH)
                    বিরতি;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } যখন (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    যদি (s->উপরের_water < s->window_আকার) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        যদি (s->উপরের_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->window_আকার - curr;
            যদি (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->window + curr, (unsigned)init);
            s->উপরের_water = curr + init;
        }
        অন্যথা যদি (s->উপরের_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->উপরের_water;
            যদি (init > s->window_আকার - s->উপরের_water)
                init = s->window_আকার - s->উপরের_water;
            zmemzero(s->window + s->উপরের_water, (unsigned)init);
            s->উপরের_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->window_আকার - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateInit_(z_streamp strm, পূর্ণসংখ্যা level, ধ্রুবক অক্ষর *version,
                         পূর্ণসংখ্যা stream_আকার) {
    ফেরত deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, version, stream_আকার);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateInit2_(z_streamp strm, পূর্ণসংখ্যা level, পূর্ণসংখ্যা method,
                          পূর্ণসংখ্যা windowBits, পূর্ণসংখ্যা memLevel, পূর্ণসংখ্যা strategy,
                          ধ্রুবক অক্ষর *version, পূর্ণসংখ্যা stream_আকার) {
    deflate_state *s;
    পূর্ণসংখ্যা wrap = 1;
    স্থির ধ্রুবক অক্ষর my_version[] = ZLIB_VERSION;

    যদি (version == Z_NULL || version[0] != my_version[0] ||
        stream_আকার != আকার(z_stream)) {
        ফেরত Z_VERSION_ERROR;
    }
    যদি (strm == Z_NULL) ফেরত Z_STREAM_ERROR;

    strm->বার্তা = Z_NULL;
    যদি (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        ফেরত Z_STREAM_ERROR;
#অন্যথা
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    যদি (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        ফেরত Z_STREAM_ERROR;
#অন্যথা
        strm->zfree = zcfree;
#endif

#ifdef FASTEST
    যদি (level != 0) level = 1;
#অন্যথা
    যদি (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif

    যদি (windowBits < 0) { /* suppress zlib wrapper */
        wrap = 0;
        যদি (windowBits < -15)
            ফেরত Z_STREAM_ERROR;
        windowBits = -windowBits;
    }
#ifdef GZIP
    অন্যথা যদি (windowBits > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        windowBits -= 16;
    }
#endif
    যদি (memLevel < 1 || memLevel > MAX_MEM_LEVEL || method != Z_DEFLATED ||
        windowBits < 8 || windowBits > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (windowBits == 8 && wrap != 1)) {
        ফেরত Z_STREAM_ERROR;
    }
    যদি (windowBits == 8) windowBits = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, আকার(deflate_state));
    যদি (s == Z_NULL) ফেরত Z_MEM_ERROR;
    zmemzero(s, আকার(deflate_state));
    strm->state = (কাঠামো internal_state FAR *)s;
    s->strm = strm;
    s->status = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_bits = (uInt)windowBits;
    s->w_আকার = 1 << s->w_bits;
    s->w_mask = s->w_আকার - 1;

    s->hash_bits = (uInt)memLevel + 7;
    s->hash_আকার = 1 << s->hash_bits;
    s->hash_mask = s->hash_আকার - 1;
    s->hash_shift =  ((s->hash_bits + MIN_MATCH-1) / MIN_MATCH);

    s->window = (Bytef *) ZALLOC(strm, s->w_আকার, 2*আকার(Byte));
    s->পূর্ব   = (Posf *)  ZALLOC(strm, s->w_আকার, আকার(Pos));
    s->শীর্ষ   = (Posf *)  ZALLOC(strm, s->hash_আকার, আকার(Pos));

    s->উপরের_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_আকার = (ulg)s->lit_bufsize * 4;

    যদি (s->window == Z_NULL || s->পূর্ব == Z_NULL || s->শীর্ষ == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->status = FINISH_STATE;
        strm->বার্তা = ERR_বার্তা(Z_MEM_ERROR);
        deflateEnd (strm);
        ফেরত Z_MEM_ERROR;
    }
#ifdef LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_end = s->lit_bufsize - 1;
#অন্যথা
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

    ফেরত deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local পূর্ণসংখ্যা deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    যদি (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        ফেরত 1;
    s = strm->state;
    যদি (s == Z_NULL || s->strm != strm || (s->status != INIT_STATE &&
#ifdef GZIP
                                           s->status != GZIP_STATE &&
#endif
                                           s->status != EXTRA_STATE &&
                                           s->status != NAME_STATE &&
                                           s->status != COMMENT_STATE &&
                                           s->status != HCRC_STATE &&
                                           s->status != BUSY_STATE &&
                                           s->status != FINISH_STATE))
        ফেরত 1;
    ফেরত 0;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateSetDictionary(z_streamp strm, ধ্রুবক Bytef *dictionary,
                                 uInt  dict_দৈর্ঘ্য) {
    deflate_state *s;
    uInt str, n;
    পূর্ণসংখ্যা wrap;
    unsigned avail;
    z_const unsigned অক্ষর *পরবর্তী;

    যদি (deflateStateCheck(strm) || dictionary == Z_NULL)
        ফেরত Z_STREAM_ERROR;
    s = strm->state;
    wrap = s->wrap;
    যদি (wrap == 2 || (wrap == 1 && s->status != INIT_STATE) || s->lookahead)
        ফেরত Z_STREAM_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    যদি (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_দৈর্ঘ্য);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    যদি (dict_দৈর্ঘ্য >= s->w_আকার) {
        যদি (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->block_start = 0L;
            s->ঢোকাও = 0;
        }
        dictionary += dict_দৈর্ঘ্য - s->w_আকার;  /* use the tail */
        dict_দৈর্ঘ্য = s->w_আকার;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_in;
    পরবর্তী = strm->পরবর্তী_in;
    strm->avail_in = dict_দৈর্ঘ্য;
    strm->পরবর্তী_in = (z_const Bytef *)dictionary;
    fill_window(s);
    যখন (s->lookahead >= MIN_MATCH) {
        str = s->strstart;
        n = s->lookahead - (MIN_MATCH-1);
        করো {
            UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
            s->পূর্ব[str & s->w_mask] = s->শীর্ষ[s->ins_h];
#endif
            s->শীর্ষ[s->ins_h] = (Pos)str;
            str++;
        } যখন (--n);
        s->strstart = str;
        s->lookahead = MIN_MATCH-1;
        fill_window(s);
    }
    s->strstart += s->lookahead;
    s->block_start = (long)s->strstart;
    s->ঢোকাও = s->lookahead;
    s->lookahead = 0;
    s->match_দৈর্ঘ্য = s->পূর্ব_দৈর্ঘ্য = MIN_MATCH-1;
    s->match_available = 0;
    strm->পরবর্তী_in = পরবর্তী;
    strm->avail_in = avail;
    s->wrap = wrap;
    ফেরত Z_OK;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_দৈর্ঘ্য) {
    deflate_state *s;
    uInt len;

    যদি (deflateStateCheck(strm))
        ফেরত Z_STREAM_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    যদি (len > s->w_আকার)
        len = s->w_আকার;
    যদি (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->window + s->strstart + s->lookahead - len, len);
    যদি (dict_দৈর্ঘ্য != Z_NULL)
        *dict_দৈর্ঘ্য = len;
    ফেরত Z_OK;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    যদি (deflateStateCheck(strm)) {
        ফেরত Z_STREAM_ERROR;
    }

    strm->যোগ_in = strm->যোগ_out = 0;
    strm->বার্তা = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->তথ্য_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_out = s->pending_buf;

    যদি (s->wrap < 0) {
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

    ফেরত Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local শূন্য lm_init(deflate_state *s) {
    s->window_আকার = (ulg)2L*s->w_আকার;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_match   = configuration_table[s->level].max_lazy;
    s->good_match       = configuration_table[s->level].good_দৈর্ঘ্য;
    s->nice_match       = configuration_table[s->level].nice_দৈর্ঘ্য;
    s->max_chain_দৈর্ঘ্য = configuration_table[s->level].max_chain;

    s->strstart = 0;
    s->block_start = 0L;
    s->lookahead = 0;
    s->ঢোকাও = 0;
    s->match_দৈর্ঘ্য = s->পূর্ব_দৈর্ঘ্য = MIN_MATCH-1;
    s->match_available = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateReset(z_streamp strm) {
    পূর্ণসংখ্যা ret;

    ret = deflateResetKeep(strm);
    যদি (ret == Z_OK)
        lm_init(strm->state);
    ফেরত ret;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateSetHeader(z_streamp strm, gz_headerp শীর্ষ) {
    যদি (deflateStateCheck(strm) || strm->state->wrap != 2)
        ফেরত Z_STREAM_ERROR;
    strm->state->gzhead = শীর্ষ;
    ফেরত Z_OK;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflatePending(z_streamp strm, unsigned *pending, পূর্ণসংখ্যা *bits) {
    যদি (deflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    যদি (bits != Z_NULL)
        *bits = strm->state->bi_valid;
    যদি (pending != Z_NULL) {
        *pending = (unsigned)strm->state->pending;
        যদি (*pending != strm->state->pending) {
            *pending = (unsigned)-1;
            ফেরত Z_BUF_ERROR;
        }
    }
    ফেরত Z_OK;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateUsed(z_streamp strm, পূর্ণসংখ্যা *bits) {
    যদি (deflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    যদি (bits != Z_NULL)
        *bits = strm->state->bi_used;
    ফেরত Z_OK;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflate_মৌলিক(z_streamp strm, পূর্ণসংখ্যা bits, পূর্ণসংখ্যা মান) {
    deflate_state *s;
    পূর্ণসংখ্যা put;

    যদি (deflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    s = strm->state;
#ifdef LIT_MEM
    যদি (bits < 0 || bits > 16 ||
        (uchf *)s->d_buf < s->pending_out + ((Buf_আকার + 7) >> 3))
        ফেরত Z_BUF_ERROR;
#অন্যথা
    যদি (bits < 0 || bits > 16 ||
        s->sym_buf < s->pending_out + ((Buf_আকার + 7) >> 3))
        ফেরত Z_BUF_ERROR;
#endif
    করো {
        put = Buf_আকার - s->bi_valid;
        যদি (put > bits)
            put = bits;
        s->bi_buf |= (ush)((মান & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        _tr_flush_bits(s);
        মান >>= put;
        bits -= put;
    } যখন (bits);
    ফেরত Z_OK;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateParams(z_streamp strm, পূর্ণসংখ্যা level, পূর্ণসংখ্যা strategy) {
    deflate_state *s;
    compress_func func;

    যদি (deflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    s = strm->state;

#ifdef FASTEST
    যদি (level != 0) level = 1;
#অন্যথা
    যদি (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif
    যদি (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        ফেরত Z_STREAM_ERROR;
    }
    func = configuration_table[s->level].func;

    যদি ((strategy != s->strategy || func != configuration_table[level].func) &&
        s->last_flush != -2) {
        /* Flush the last buffer: */
        পূর্ণসংখ্যা err = deflate(strm, Z_BLOCK);
        যদি (err == Z_STREAM_ERROR)
            ফেরত err;
        যদি (strm->avail_in || (s->strstart - s->block_start) + s->lookahead)
            ফেরত Z_BUF_ERROR;
    }
    যদি (s->level != level) {
        যদি (s->level == 0 && s->matches != 0) {
            যদি (s->matches == 1)
                slide_hash(s);
            অন্যথা
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_match   = configuration_table[level].max_lazy;
        s->good_match       = configuration_table[level].good_দৈর্ঘ্য;
        s->nice_match       = configuration_table[level].nice_দৈর্ঘ্য;
        s->max_chain_দৈর্ঘ্য = configuration_table[level].max_chain;
    }
    s->strategy = strategy;
    ফেরত Z_OK;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateTune(z_streamp strm, পূর্ণসংখ্যা good_দৈর্ঘ্য, পূর্ণসংখ্যা max_lazy,
                        পূর্ণসংখ্যা nice_দৈর্ঘ্য, পূর্ণসংখ্যা max_chain) {
    deflate_state *s;

    যদি (deflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;
    s = strm->state;
    s->good_match = (uInt)good_দৈর্ঘ্য;
    s->max_lazy_match = (uInt)max_lazy;
    s->nice_match = nice_দৈর্ঘ্য;
    s->max_chain_দৈর্ঘ্য = (uInt)max_chain;
    ফেরত Z_OK;
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
z_আকার_t ZEXPORT deflateBound_z(z_streamp strm, z_আকার_t sourceLen) {
    deflate_state *s;
    z_আকার_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = sourceLen + (sourceLen >> 3) + (sourceLen >> 8) +
               (sourceLen >> 9) + 4;
    যদি (fixedlen < sourceLen)
        fixedlen = (z_আকার_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = sourceLen + (sourceLen >> 5) + (sourceLen >> 7) +
               (sourceLen >> 11) + 7;
    যদি (storelen < sourceLen)
        storelen = (z_আকার_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    যদি (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        ফেরত bound + 18 < bound ? (z_আকার_t)-1 : bound + 18;
    }

    /* compute wrapper length */
    s = strm->state;
    নির্বাচন (s->wrap < 0 ? -s->wrap : s->wrap) {
    ক্ষেত্র 0:                                 /* raw deflate */
        wraplen = 0;
        বিরতি;
    ক্ষেত্র 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        বিরতি;
#ifdef GZIP
    ক্ষেত্র 2:                                 /* gzip wrapper */
        wraplen = 18;
        যদি (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            যদি (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            যদি (str != Z_NULL)
                করো {
                    wraplen++;
                } যখন (*str++);
            str = s->gzhead->comment;
            যদি (str != Z_NULL)
                করো {
                    wraplen++;
                } যখন (*str++);
            যদি (s->gzhead->hcrc)
                wraplen += 2;
        }
        বিরতি;
#endif
    পূর্বনির্ধারিত:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    যদি (s->w_bits != 15 || s->hash_bits != 8 + 7) {
        bound = s->w_bits <= s->hash_bits && s->level ? fixedlen :
                                                        storelen;
        ফেরত bound + wraplen < bound ? (z_আকার_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
            (sourceLen >> 25) + 13 - 6 + wraplen;
    ফেরত bound < sourceLen ? (z_আকার_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong sourceLen) {
    z_আকার_t bound = deflateBound_z(strm, sourceLen);
    ফেরত (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local শূন্য putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local শূন্য flush_pending(z_streamp strm) {
    unsigned len;
    deflate_state *s = strm->state;

    _tr_flush_bits(s);
    len = s->pending > strm->avail_out ? strm->avail_out :
                                         (unsigned)s->pending;
    যদি (len == 0) ফেরত;

    zmemcpy(strm->পরবর্তী_out, s->pending_out, len);
    strm->পরবর্তী_out  += len;
    s->pending_out  += len;
    strm->যোগ_out += len;
    strm->avail_out -= len;
    s->pending      -= len;
    যদি (s->pending == 0) {
        s->pending_out = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#define HCRC_UPDATE(beg) \
    করো { \
        যদি (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = crc32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } যখন (0)

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflate(z_streamp strm, পূর্ণসংখ্যা flush) {
    পূর্ণসংখ্যা old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    যদি (deflateStateCheck(strm) || flush > Z_BLOCK || flush < 0) {
        ফেরত Z_STREAM_ERROR;
    }
    s = strm->state;

    যদি (strm->পরবর্তী_out == Z_NULL ||
        (strm->avail_in != 0 && strm->পরবর্তী_in == Z_NULL) ||
        (s->status == FINISH_STATE && flush != Z_FINISH)) {
        ERR_RETURN(strm, Z_STREAM_ERROR);
    }
    যদি (strm->avail_out == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->last_flush;
    s->last_flush = flush;

    /* Flush as much pending output as possible */
    যদি (s->pending != 0) {
        flush_pending(strm);
        যদি (strm->avail_out == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->last_flush = -1;
            ফেরত Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } অন্যথা যদি (strm->avail_in == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_FINISH) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    যদি (s->status == FINISH_STATE && strm->avail_in != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    যদি (s->status == INIT_STATE && s->wrap == 0)
        s->status = BUSY_STATE;
    যদি (s->status == INIT_STATE) {
        /* zlib header */
        uInt header = (Z_DEFLATED + ((s->w_bits - 8) << 4)) << 8;
        uInt level_flags;

        যদি (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        অন্যথা যদি (s->level < 6)
            level_flags = 1;
        অন্যথা যদি (s->level == 6)
            level_flags = 2;
        অন্যথা
            level_flags = 3;
        header |= (level_flags << 6);
        যদি (s->strstart != 0) header |= PRESET_DICT;
        header += 31 - (header % 31);

        putShortMSB(s, header);

        /* Save the adler32 of the preset dictionary: */
        যদি (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        যদি (s->pending != 0) {
            s->last_flush = -1;
            ফেরত Z_OK;
        }
    }
#ifdef GZIP
    যদি (s->status == GZIP_STATE) {
        /* gzip header */
        strm->adler = crc32(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        যদি (s->gzhead == Z_NULL) {
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
            যদি (s->pending != 0) {
                s->last_flush = -1;
                ফেরত Z_OK;
            }
        }
        অন্যথা {
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
            যদি (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            যদি (s->gzhead->hcrc)
                strm->adler = crc32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->status = EXTRA_STATE;
        }
    }
    যদি (s->status == EXTRA_STATE) {
        যদি (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg বাম = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            যখন (s->pending + বাম > s->pending_buf_আকার) {
                ulg copy = s->pending_buf_আকার - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_আকার;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                যদি (s->pending != 0) {
                    s->last_flush = -1;
                    ফেরত Z_OK;
                }
                beg = 0;
                বাম -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, বাম);
            s->pending += বাম;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = NAME_STATE;
    }
    যদি (s->status == NAME_STATE) {
        যদি (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            পূর্ণসংখ্যা val;
            করো {
                যদি (s->pending == s->pending_buf_আকার) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    যদি (s->pending != 0) {
                        s->last_flush = -1;
                        ফেরত Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } যখন (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = COMMENT_STATE;
    }
    যদি (s->status == COMMENT_STATE) {
        যদি (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            পূর্ণসংখ্যা val;
            করো {
                যদি (s->pending == s->pending_buf_আকার) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    যদি (s->pending != 0) {
                        s->last_flush = -1;
                        ফেরত Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } যখন (val != 0);
            HCRC_UPDATE(beg);
        }
        s->status = HCRC_STATE;
    }
    যদি (s->status == HCRC_STATE) {
        যদি (s->gzhead->hcrc) {
            যদি (s->pending + 2 > s->pending_buf_আকার) {
                flush_pending(strm);
                যদি (s->pending != 0) {
                    s->last_flush = -1;
                    ফেরত Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = crc32(0L, Z_NULL, 0);
        }
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        যদি (s->pending != 0) {
            s->last_flush = -1;
            ফেরত Z_OK;
        }
    }
#endif

    /* Start a new block or continue the current one.
     */
    যদি (strm->avail_in != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->status != FINISH_STATE)) {
        block_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_table[s->level].func))(s, flush);

        যদি (bstate == finish_started || bstate == finish_done) {
            s->status = FINISH_STATE;
        }
        যদি (bstate == need_more || bstate == finish_started) {
            যদি (strm->avail_out == 0) {
                s->last_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            ফেরত Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        যদি (bstate == block_done) {
            যদি (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } অন্যথা যদি (flush != Z_BLOCK) { /* FULL_FLUSH or SYNC_FLUSH */
                _tr_stored_block(s, (অক্ষর*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                যদি (flush == Z_FULL_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    যদি (s->lookahead == 0) {
                        s->strstart = 0;
                        s->block_start = 0L;
                        s->ঢোকাও = 0;
                    }
                }
            }
            flush_pending(strm);
            যদি (strm->avail_out == 0) {
              s->last_flush = -1; /* avoid BUF_ERROR at next call, see above */
              ফেরত Z_OK;
            }
        }
    }

    যদি (flush != Z_FINISH) ফেরত Z_OK;
    যদি (s->wrap <= 0) ফেরত Z_STREAM_END;

    /* Write the trailer */
#ifdef GZIP
    যদি (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->যোগ_in & 0xff));
        put_byte(s, (Byte)((strm->যোগ_in >> 8) & 0xff));
        put_byte(s, (Byte)((strm->যোগ_in >> 16) & 0xff));
        put_byte(s, (Byte)((strm->যোগ_in >> 24) & 0xff));
    }
    অন্যথা
#endif
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    যদি (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    ফেরত s->pending != 0 ? Z_OK : Z_STREAM_END;
}

/* ========================================================================= */
পূর্ণসংখ্যা ZEXPORT deflateEnd(z_streamp strm) {
    পূর্ণসংখ্যা status;

    যদি (deflateStateCheck(strm)) ফেরত Z_STREAM_ERROR;

    status = strm->state->status;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->শীর্ষ);
    TRY_FREE(strm, strm->state->পূর্ব);
    TRY_FREE(strm, strm->state->window);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    ফেরত status == BUSY_STATE ? Z_তথ্য_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
পূর্ণসংখ্যা ZEXPORT deflateCopy(z_streamp dest, z_streamp source) {
#ifdef MAXSEG_64K
    (শূন্য)dest;
    (শূন্য)source;
    ফেরত Z_STREAM_ERROR;
#অন্যথা
    deflate_state *ds;
    deflate_state *ss;


    যদি (deflateStateCheck(source) || dest == Z_NULL) {
        ফেরত Z_STREAM_ERROR;
    }

    ss = source->state;

    zmemcpy(dest, source, আকার(z_stream));

    ds = (deflate_state *) ZALLOC(dest, 1, আকার(deflate_state));
    যদি (ds == Z_NULL) ফেরত Z_MEM_ERROR;
    zmemzero(ds, আকার(deflate_state));
    dest->state = (কাঠামো internal_state FAR *) ds;
    zmemcpy(ds, ss, আকার(deflate_state));
    ds->strm = dest;

    ds->window = (Bytef *) ZALLOC(dest, ds->w_আকার, 2*আকার(Byte));
    ds->পূর্ব   = (Posf *)  ZALLOC(dest, ds->w_আকার, আকার(Pos));
    ds->শীর্ষ   = (Posf *)  ZALLOC(dest, ds->hash_আকার, আকার(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    যদি (ds->window == Z_NULL || ds->পূর্ব == Z_NULL || ds->শীর্ষ == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflateEnd (dest);
        ফেরত Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->window, ss->window, ss->উপরের_water);
    zmemcpy(ds->পূর্ব, ss->পূর্ব,
            (ss->slid || ss->strstart - ss->ঢোকাও > ds->w_আকার ? ds->w_আকার :
                ss->strstart - ss->ঢোকাও) * আকার(Pos));
    zmemcpy(ds->শীর্ষ, ss->শীর্ষ, ds->hash_আকার * আকার(Pos));

    ds->pending_out = ds->pending_buf + (ss->pending_out - ss->pending_buf);
    zmemcpy(ds->pending_out, ss->pending_out, ss->pending);
#ifdef LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_পরবর্তী * আকার(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_পরবর্তী);
#অন্যথা
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_পরবর্তী);
#endif

    ds->l_desc.dyn_বৃক্ষ = ds->dyn_ltree;
    ds->d_desc.dyn_বৃক্ষ = ds->dyn_dtree;
    ds->bl_desc.dyn_বৃক্ষ = ds->bl_বৃক্ষ;

    ফেরত Z_OK;
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
    unsigned chain_দৈর্ঘ্য = s->max_chain_দৈর্ঘ্য;/* max hash chain length */
    Bytef *scan = s->window + s->strstart;      /* current string */
    Bytef *match;                               /* matched string */
    পূর্ণসংখ্যা len;                                    /* length of current match */
    পূর্ণসংখ্যা best_len = (পূর্ণসংখ্যা)s->পূর্ব_দৈর্ঘ্য;         /* best match length so far */
    পূর্ণসংখ্যা nice_match = s->nice_match;             /* stop if match long enough */
    IPos limit = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *পূর্ব = s->পূর্ব;
    uInt wmask = s->w_mask;

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->window + s->strstart + MAX_MATCH - 1;
    ush scan_start = *(ushf*)scan;
    ush scan_end   = *(ushf*)(scan + best_len - 1);
#অন্যথা
    Bytef *strend = s->window + s->strstart + MAX_MATCH;
    Byte scan_end1  = scan[best_len - 1];
    Byte scan_end   = scan[best_len];
#endif

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    যদি (s->পূর্ব_দৈর্ঘ্য >= s->good_match) {
        chain_দৈর্ঘ্য >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    যদি ((uInt)nice_match > s->lookahead) nice_match = (পূর্ণসংখ্যা)s->lookahead;

    Assert((ulg)s->strstart <= s->window_আকার - MIN_LOOKAHEAD,
           "need lookahead");

    করো {
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
#যদি (defined(UNALIGNED_OK) && MAX_MATCH == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        যদি (*(ushf*)(match + best_len - 1) != scan_end ||
            *(ushf*)match != scan_start) চালু;

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
        করো {
        } যখন (*(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(scan <= s->window + (unsigned)(s->window_আকার - 1),
               "wild scan");
        যদি (*scan == *match) scan++;

        len = (MAX_MATCH - 1) - (পূর্ণসংখ্যা)(strend - scan);
        scan = strend - (MAX_MATCH-1);

#অন্যথা /* UNALIGNED_OK */

        যদি (match[best_len]     != scan_end  ||
            match[best_len - 1] != scan_end1 ||
            *match              != *scan     ||
            *++match            != scan[1])      চালু;

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
        করো {
        } যখন (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        Assert(scan <= s->window + (unsigned)(s->window_আকার - 1),
               "wild scan");

        len = MAX_MATCH - (পূর্ণসংখ্যা)(strend - scan);
        scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

        যদি (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            যদি (len >= nice_match) বিরতি;
#ifdef UNALIGNED_OK
            scan_end = *(ushf*)(scan + best_len - 1);
#অন্যথা
            scan_end1  = scan[best_len - 1];
            scan_end   = scan[best_len];
#endif
        }
    } যখন ((cur_match = পূর্ব[cur_match & wmask]) > limit
             && --chain_দৈর্ঘ্য != 0);

    যদি ((uInt)best_len <= s->lookahead) ফেরত (uInt)best_len;
    ফেরত s->lookahead;
}

#অন্যথা /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_match(deflate_state *s, IPos cur_match) {
    Bytef *scan = s->window + s->strstart;      /* current string */
    Bytef *match;                               /* matched string */
    পূর্ণসংখ্যা len;                                    /* length of current match */
    Bytef *strend = s->window + s->strstart + MAX_MATCH;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->window_আকার - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_match < s->strstart, "no future");

    match = s->window + cur_match;

    /* Return failure if the match length is less than 2:
     */
    যদি (match[0] != scan[0] || match[1] != scan[1]) ফেরত MIN_MATCH-1;

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
    করো {
    } যখন (*++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             scan < strend);

    Assert(scan <= s->window + (unsigned)(s->window_আকার - 1), "wild scan");

    len = MAX_MATCH - (পূর্ণসংখ্যা)(strend - scan);

    যদি (len < MIN_MATCH) ফেরত MIN_MATCH - 1;

    s->match_start = cur_match;
    ফেরত (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#endif /* FASTEST */

#ifdef ZLIB_DEBUG

#define EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local শূন্য check_match(deflate_state *s, IPos start, IPos match, পূর্ণসংখ্যা দৈর্ঘ্য) {
    /* check that the match is indeed a match */
    Bytef *back = s->window + (পূর্ণসংখ্যা)match, *here = s->window + start;
    IPos len = (IPos)দৈর্ঘ্য;
    যদি (match == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        back++;
        here++;
        len--;
    }
    যদি (zmemcmp(back, here, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                start, (পূর্ণসংখ্যা)match, দৈর্ঘ্য);
        করো {
            fprintf(stderr, "(%02x %02x)", *back++, *here++);
        } যখন (--len != 0);
        z_error("invalid match");
    }
    যদি (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", start - match, দৈর্ঘ্য);
        করো { putc(s->window[start++], stderr); } যখন (--দৈর্ঘ্য != 0);
    }
}
#অন্যথা
#  define check_match(s, start, match, দৈর্ঘ্য)
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
   যদি (s->strm->avail_out == 0) ফেরত (last) ? finish_started : need_more; \
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
local block_state deflate_stored(deflate_state *s, পূর্ণসংখ্যা flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    unsigned min_block = (unsigned)(MIN(s->pending_buf_আকার - 5, s->w_আকার));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    পূর্ণসংখ্যা last = 0;
    unsigned len, বাম, have;
    unsigned used = s->strm->avail_in;
    করো {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((unsigned)s->bi_valid + 42) >> 3;   /* bytes in header */
        যদি (s->strm->avail_out < have)          /* need room for header */
            বিরতি;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_out - have;
        বাম = (unsigned)(s->strstart - s->block_start);    /* window bytes */
        যদি (len > (ulg)বাম + s->strm->avail_in)
            len = বাম + s->strm->avail_in;     /* limit len to the input */
        যদি (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        যদি (len < min_block && ((len == 0 && flush != Z_FINISH) ||
                                flush == Z_NO_FLUSH ||
                                len != বাম + s->strm->avail_in))
            বিরতি;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        last = flush == Z_FINISH && len == বাম + s->strm->avail_in ? 1 : 0;
        _tr_stored_block(s, (অক্ষর *)0, 0L, last);

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
        যদি (বাম) {
            যদি (বাম > len)
                বাম = len;
            zmemcpy(s->strm->পরবর্তী_out, s->window + s->block_start, বাম);
            s->strm->পরবর্তী_out += বাম;
            s->strm->avail_out -= বাম;
            s->strm->যোগ_out += বাম;
            s->block_start += বাম;
            len -= বাম;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        যদি (len) {
            read_buf(s->strm, s->strm->পরবর্তী_out, len);
            s->strm->পরবর্তী_out += len;
            s->strm->avail_out -= len;
            s->strm->যোগ_out += len;
        }
    } যখন (last == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    used -= s->strm->avail_in;      /* number of input bytes directly copied */
    যদি (used) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        যদি (used >= s->w_আকার) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->window, s->strm->পরবর্তী_in - s->w_আকার, s->w_আকার);
            s->strstart = s->w_আকার;
            s->ঢোকাও = s->strstart;
        }
        অন্যথা {
            যদি (s->window_আকার - s->strstart <= used) {
                /* Slide the window down. */
                s->strstart -= s->w_আকার;
                zmemcpy(s->window, s->window + s->w_আকার, s->strstart);
                যদি (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                যদি (s->ঢোকাও > s->strstart)
                    s->ঢোকাও = s->strstart;
            }
            zmemcpy(s->window + s->strstart, s->strm->পরবর্তী_in - used, used);
            s->strstart += used;
            s->ঢোকাও += MIN(used, s->w_আকার - s->ঢোকাও);
        }
        s->block_start = s->strstart;
    }
    যদি (s->উপরের_water < s->strstart)
        s->উপরের_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    যদি (last) {
        s->bi_used = 8;
        ফেরত finish_done;
    }

    /* If flushing and all input has been consumed, then done. */
    যদি (flush != Z_NO_FLUSH && flush != Z_FINISH &&
        s->strm->avail_in == 0 && (long)s->strstart == s->block_start)
        ফেরত block_done;

    /* Fill the window with any remaining input. */
    have = (unsigned)(s->window_আকার - s->strstart);
    যদি (s->strm->avail_in > have && s->block_start >= (long)s->w_আকার) {
        /* Slide the window down. */
        s->block_start -= s->w_আকার;
        s->strstart -= s->w_আকার;
        zmemcpy(s->window, s->window + s->w_আকার, s->strstart);
        যদি (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_আকার;          /* more space now */
        যদি (s->ঢোকাও > s->strstart)
            s->ঢোকাও = s->strstart;
    }
    যদি (have > s->strm->avail_in)
        have = s->strm->avail_in;
    যদি (have) {
        read_buf(s->strm, s->window + s->strstart, have);
        s->strstart += have;
        s->ঢোকাও += MIN(have, s->w_আকার - s->ঢোকাও);
    }
    যদি (s->উপরের_water < s->strstart)
        s->উপরের_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((unsigned)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (unsigned)MIN(s->pending_buf_আকার - have, MAX_STORED);
    min_block = MIN(have, s->w_আকার);
    বাম = (unsigned)(s->strstart - s->block_start);
    যদি (বাম >= min_block ||
        ((বাম || flush == Z_FINISH) && flush != Z_NO_FLUSH &&
         s->strm->avail_in == 0 && বাম <= have)) {
        len = MIN(বাম, have);
        last = flush == Z_FINISH && s->strm->avail_in == 0 &&
               len == বাম ? 1 : 0;
        _tr_stored_block(s, (charf *)s->window + s->block_start, len, last);
        s->block_start += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    যদি (last)
        s->bi_used = 8;
    ফেরত last ? finish_started : need_more;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local block_state deflate_fast(deflate_state *s, পূর্ণসংখ্যা flush) {
    IPos hash_শীর্ষ;       /* head of the hash chain */
    পূর্ণসংখ্যা bflush;           /* set if current block must be flushed */

    জন্য (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        যদি (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            যদি (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                ফেরত need_more;
            }
            যদি (s->lookahead == 0) বিরতি; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_শীর্ষ = NIL;
        যদি (s->lookahead >= MIN_MATCH) {
            ঢোকাও_STRING(s, s->strstart, hash_শীর্ষ);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        যদি (hash_শীর্ষ != NIL && s->strstart - hash_শীর্ষ <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_দৈর্ঘ্য = longest_match (s, hash_শীর্ষ);
            /* longest_match() sets match_start */
        }
        যদি (s->match_দৈর্ঘ্য >= MIN_MATCH) {
            check_match(s, s->strstart, s->match_start, (পূর্ণসংখ্যা)s->match_দৈর্ঘ্য);

            _tr_tally_dist(s, s->strstart - s->match_start,
                           s->match_দৈর্ঘ্য - MIN_MATCH, bflush);

            s->lookahead -= s->match_দৈর্ঘ্য;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#ifndef FASTEST
            যদি (s->match_দৈর্ঘ্য <= s->max_ঢোকাও_দৈর্ঘ্য &&
                s->lookahead >= MIN_MATCH) {
                s->match_দৈর্ঘ্য--; /* string at strstart already in table */
                করো {
                    s->strstart++;
                    ঢোকাও_STRING(s, s->strstart, hash_শীর্ষ);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } যখন (--s->match_দৈর্ঘ্য != 0);
                s->strstart++;
            } অন্যথা
#endif
            {
                s->strstart += s->match_দৈর্ঘ্য;
                s->match_দৈর্ঘ্য = 0;
                s->ins_h = s->window[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->window[s->strstart + 1]);
#যদি MIN_MATCH != 3
                Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } অন্যথা {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        যদি (bflush) FLUSH_BLOCK(s, 0);
    }
    s->ঢোকাও = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    যদি (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        ফেরত finish_done;
    }
    যদি (s->sym_পরবর্তী)
        FLUSH_BLOCK(s, 0);
    ফেরত block_done;
}

#ifndef FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local block_state deflate_slow(deflate_state *s, পূর্ণসংখ্যা flush) {
    IPos hash_শীর্ষ;          /* head of hash chain */
    পূর্ণসংখ্যা bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    জন্য (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        যদি (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            যদি (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                ফেরত need_more;
            }
            যদি (s->lookahead == 0) বিরতি; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_শীর্ষ = NIL;
        যদি (s->lookahead >= MIN_MATCH) {
            ঢোকাও_STRING(s, s->strstart, hash_শীর্ষ);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->পূর্ব_দৈর্ঘ্য = s->match_দৈর্ঘ্য, s->পূর্ব_match = s->match_start;
        s->match_দৈর্ঘ্য = MIN_MATCH-1;

        যদি (hash_শীর্ষ != NIL && s->পূর্ব_দৈর্ঘ্য < s->max_lazy_match &&
            s->strstart - hash_শীর্ষ <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_দৈর্ঘ্য = longest_match (s, hash_শীর্ষ);
            /* longest_match() sets match_start */

            যদি (s->match_দৈর্ঘ্য <= 5 && (s->strategy == Z_FILTERED
#যদি TOO_FAR <= 32767
                || (s->match_দৈর্ঘ্য == MIN_MATCH &&
                    s->strstart - s->match_start > TOO_FAR)
#endif
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->match_দৈর্ঘ্য = MIN_MATCH-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        যদি (s->পূর্ব_দৈর্ঘ্য >= MIN_MATCH && s->match_দৈর্ঘ্য <= s->পূর্ব_দৈর্ঘ্য) {
            uInt max_ঢোকাও = s->strstart + s->lookahead - MIN_MATCH;
            /* Do not insert strings in hash table beyond this. */

            check_match(s, s->strstart - 1, s->পূর্ব_match, (পূর্ণসংখ্যা)s->পূর্ব_দৈর্ঘ্য);

            _tr_tally_dist(s, s->strstart - 1 - s->পূর্ব_match,
                           s->পূর্ব_দৈর্ঘ্য - MIN_MATCH, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->পূর্ব_দৈর্ঘ্য - 1;
            s->পূর্ব_দৈর্ঘ্য -= 2;
            করো {
                যদি (++s->strstart <= max_ঢোকাও) {
                    ঢোকাও_STRING(s, s->strstart, hash_শীর্ষ);
                }
            } যখন (--s->পূর্ব_দৈর্ঘ্য != 0);
            s->match_available = 0;
            s->match_দৈর্ঘ্য = MIN_MATCH-1;
            s->strstart++;

            যদি (bflush) FLUSH_BLOCK(s, 0);

        } অন্যথা যদি (s->match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->window[s->strstart - 1]));
            _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
            যদি (bflush) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            যদি (s->strm->avail_out == 0) ফেরত need_more;
        } অন্যথা {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    যদি (s->match_available) {
        Tracevv((stderr,"%c", s->window[s->strstart - 1]));
        _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
        s->match_available = 0;
    }
    s->ঢোকাও = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    যদি (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        ফেরত finish_done;
    }
    যদি (s->sym_পরবর্তী)
        FLUSH_BLOCK(s, 0);
    ফেরত block_done;
}
#endif /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local block_state deflate_rle(deflate_state *s, পূর্ণসংখ্যা flush) {
    পূর্ণসংখ্যা bflush;             /* set if current block must be flushed */
    uInt পূর্ব;              /* byte at distance one to match */
    Bytef *scan, *strend;   /* scan goes up to strend for length of run */

    জন্য (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        যদি (s->lookahead <= MAX_MATCH) {
            fill_window(s);
            যদি (s->lookahead <= MAX_MATCH && flush == Z_NO_FLUSH) {
                ফেরত need_more;
            }
            যদি (s->lookahead == 0) বিরতি; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->match_দৈর্ঘ্য = 0;
        যদি (s->lookahead >= MIN_MATCH && s->strstart > 0) {
            scan = s->window + s->strstart - 1;
            পূর্ব = *scan;
            যদি (পূর্ব == *++scan && পূর্ব == *++scan && পূর্ব == *++scan) {
                strend = s->window + s->strstart + MAX_MATCH;
                করো {
                } যখন (পূর্ব == *++scan && পূর্ব == *++scan &&
                         পূর্ব == *++scan && পূর্ব == *++scan &&
                         পূর্ব == *++scan && পূর্ব == *++scan &&
                         পূর্ব == *++scan && পূর্ব == *++scan &&
                         scan < strend);
                s->match_দৈর্ঘ্য = MAX_MATCH - (uInt)(strend - scan);
                যদি (s->match_দৈর্ঘ্য > s->lookahead)
                    s->match_দৈর্ঘ্য = s->lookahead;
            }
            Assert(scan <= s->window + (uInt)(s->window_আকার - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        যদি (s->match_দৈর্ঘ্য >= MIN_MATCH) {
            check_match(s, s->strstart, s->strstart - 1, (পূর্ণসংখ্যা)s->match_দৈর্ঘ্য);

            _tr_tally_dist(s, 1, s->match_দৈর্ঘ্য - MIN_MATCH, bflush);

            s->lookahead -= s->match_দৈর্ঘ্য;
            s->strstart += s->match_দৈর্ঘ্য;
            s->match_দৈর্ঘ্য = 0;
        } অন্যথা {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        যদি (bflush) FLUSH_BLOCK(s, 0);
    }
    s->ঢোকাও = 0;
    যদি (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        ফেরত finish_done;
    }
    যদি (s->sym_পরবর্তী)
        FLUSH_BLOCK(s, 0);
    ফেরত block_done;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local block_state deflate_huff(deflate_state *s, পূর্ণসংখ্যা flush) {
    পূর্ণসংখ্যা bflush;             /* set if current block must be flushed */

    জন্য (;;) {
        /* Make sure that we have a literal to write. */
        যদি (s->lookahead == 0) {
            fill_window(s);
            যদি (s->lookahead == 0) {
                যদি (flush == Z_NO_FLUSH)
                    ফেরত need_more;
                বিরতি;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->match_দৈর্ঘ্য = 0;
        Tracevv((stderr,"%c", s->window[s->strstart]));
        _tr_tally_lit(s, s->window[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        যদি (bflush) FLUSH_BLOCK(s, 0);
    }
    s->ঢোকাও = 0;
    যদি (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        ফেরত finish_done;
    }
    যদি (s->sym_পরবর্তী)
        FLUSH_BLOCK(s, 0);
    ফেরত block_done;
}
