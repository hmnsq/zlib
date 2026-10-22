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

#ضمن "deflate.h"

ثابت حرف deflate_copyright[] =
   " deflate 1.3.2.1 Copyright 1995-2026 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

تعريفنوع تعداد {
    need_more,      /* block not completed, need more input or more output */
    block_done,     /* block flush performed */
    finish_started, /* finish started, need only more output at next deflate */
    finish_done     /* finish done, accept no more input or output */
} block_state;

تعريفنوع block_state (*compress_func)(deflate_state *s, صحيح flush);
/* Compression function. Returns the block state after the call. */

local block_state deflate_stored(deflate_state *s, صحيح flush);
local block_state deflate_fast(deflate_state *s, صحيح flush);
#ifndef FASTEST
local block_state deflate_slow(deflate_state *s, صحيح flush);
#endif
local block_state deflate_rle(deflate_state *s, صحيح flush);
local block_state deflate_huff(deflate_state *s, صحيح flush);

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
تعريفنوع بنية config_s {
   ush good_length; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_length; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#ifdef FASTEST
local ثابت config configuration_table[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#وإلا
local ثابت config configuration_table[10] = {
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
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#وإلا
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->prev[(str) & s->w_mask] = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#endif

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#define CLEAR_HASH(s) \
    𐡏𐡁𐡃 { \
        s->head[s->hash_size - 1] = NIL; \
        zmemzero(s->head, (unsigned)(s->hash_size - 1)*حجم(*s->head)); \
        s->slid = 0; \
    } طالما (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#𐡄𐡍 defined(__has_feature)
#  𐡄𐡍 __has_feature(memory_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#endif
local فراغ slide_hash(deflate_state *s) {
    unsigned n, m;
    Posf *p;
    uInt wsize = s->w_size;

    n = s->hash_size;
    p = &s->head[n];
    𐡏𐡁𐡃 {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } طالما (--n);
#ifndef FASTEST
    n = wsize;
    p = &s->prev[n];
    𐡏𐡁𐡃 {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } طالما (--n);
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

    𐡄𐡍 (len > size) len = size;
    𐡄𐡍 (len == 0) أرجع 0;

    strm->avail_in  -= len;

    zmemcpy(buf, strm->next_in, len);
    𐡄𐡍 (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#ifdef GZIP
    وإلا 𐡄𐡍 (strm->state->wrap == 2) {
        strm->adler = crc32(strm->adler, buf, len);
    }
#endif
    strm->next_in  += len;
    strm->total_in += len;

    أرجع len;
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
local فراغ fill_window(deflate_state *s) {
    unsigned n;
    unsigned more;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_size;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    𐡏𐡁𐡃 {
        more = (unsigned)(s->window_size -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
        𐡄𐡍 (حجم(صحيح) <= 2) {
#ifdef _MSC_VER
#pragma warning(pop)
#endif
            𐡄𐡍 (more == 0 && s->strstart == 0 && s->lookahead == 0) {
                more = wsize;

            } وإلا 𐡄𐡍 (more == (unsigned)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                more--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        𐡄𐡍 (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->window, s->window + wsize, (unsigned)wsize - more);
            s->match_start -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->block_start -= (long) wsize;
            𐡄𐡍 (s->insert > s->strstart)
                s->insert = s->strstart;
            slide_hash(s);
            more += wsize;
        }
        𐡄𐡍 (s->strm->avail_in == 0) اكسر;

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
        𐡄𐡍 (s->lookahead + s->insert >= MIN_MATCH) {
            uInt str = s->strstart - s->insert;
            s->ins_h = s->window[str];
            UPDATE_HASH(s, s->ins_h, s->window[str + 1]);
#𐡄𐡍 MIN_MATCH != 3
            Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
            طالما (s->insert) {
                UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
                s->prev[str & s->w_mask] = s->head[s->ins_h];
#endif
                s->head[s->ins_h] = (Pos)str;
                str++;
                s->insert--;
                𐡄𐡍 (s->lookahead + s->insert < MIN_MATCH)
                    اكسر;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } طالما (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    𐡄𐡍 (s->high_water < s->window_size) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        𐡄𐡍 (s->high_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->window_size - curr;
            𐡄𐡍 (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->window + curr, (unsigned)init);
            s->high_water = curr + init;
        }
        وإلا 𐡄𐡍 (s->high_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->high_water;
            𐡄𐡍 (init > s->window_size - s->high_water)
                init = s->window_size - s->high_water;
            zmemzero(s->window + s->high_water, (unsigned)init);
            s->high_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit_(z_streamp strm, صحيح level, ثابت حرف *version,
                         صحيح stream_size) {
    أرجع deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, version, stream_size);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit2_(z_streamp strm, صحيح level, صحيح method,
                          صحيح windowBits, صحيح memLevel, صحيح strategy,
                          ثابت حرف *version, صحيح stream_size) {
    deflate_state *s;
    صحيح wrap = 1;
    ساكن ثابت حرف my_version[] = ZLIB_VERSION;

    𐡄𐡍 (version == Z_NULL || version[0] != my_version[0] ||
        stream_size != حجم(z_stream)) {
        أرجع Z_VERSION_ERROR;
    }
    𐡄𐡍 (strm == Z_NULL) أرجع Z_STREAM_ERROR;

    strm->msg = Z_NULL;
    𐡄𐡍 (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        أرجع Z_STREAM_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    𐡄𐡍 (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        أرجع Z_STREAM_ERROR;
#وإلا
        strm->zfree = zcfree;
#endif

#ifdef FASTEST
    𐡄𐡍 (level != 0) level = 1;
#وإلا
    𐡄𐡍 (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif

    𐡄𐡍 (windowBits < 0) { /* suppress zlib wrapper */
        wrap = 0;
        𐡄𐡍 (windowBits < -15)
            أرجع Z_STREAM_ERROR;
        windowBits = -windowBits;
    }
#ifdef GZIP
    وإلا 𐡄𐡍 (windowBits > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        windowBits -= 16;
    }
#endif
    𐡄𐡍 (memLevel < 1 || memLevel > MAX_MEM_LEVEL || method != Z_DEFLATED ||
        windowBits < 8 || windowBits > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (windowBits == 8 && wrap != 1)) {
        أرجع Z_STREAM_ERROR;
    }
    𐡄𐡍 (windowBits == 8) windowBits = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, حجم(deflate_state));
    𐡄𐡍 (s == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(s, حجم(deflate_state));
    strm->state = (بنية internal_state FAR *)s;
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

    s->window = (Bytef *) ZALLOC(strm, s->w_size, 2*حجم(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_size, حجم(Pos));
    s->head   = (Posf *)  ZALLOC(strm, s->hash_size, حجم(Pos));

    s->high_water = 0;      /* nothing written to s->window yet */

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

    𐡄𐡍 (s->window == Z_NULL || s->prev == Z_NULL || s->head == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->status = FINISH_STATE;
        strm->msg = ERR_MSG(Z_MEM_ERROR);
        deflateEnd (strm);
        أرجع Z_MEM_ERROR;
    }
#ifdef LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_end = s->lit_bufsize - 1;
#وإلا
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

    أرجع deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local صحيح deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    𐡄𐡍 (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        أرجع 1;
    s = strm->state;
    𐡄𐡍 (s == Z_NULL || s->strm != strm || (s->status != INIT_STATE &&
#ifdef GZIP
                                           s->status != GZIP_STATE &&
#endif
                                           s->status != EXTRA_STATE &&
                                           s->status != NAME_STATE &&
                                           s->status != COMMENT_STATE &&
                                           s->status != HCRC_STATE &&
                                           s->status != BUSY_STATE &&
                                           s->status != FINISH_STATE))
        أرجع 1;
    أرجع 0;
}

/* ========================================================================= */
صحيح ZEXPORT deflateSetDictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt  dictLength) {
    deflate_state *s;
    uInt str, n;
    صحيح wrap;
    unsigned avail;
    z_const unsigned حرف *next;

    𐡄𐡍 (deflateStateCheck(strm) || dictionary == Z_NULL)
        أرجع Z_STREAM_ERROR;
    s = strm->state;
    wrap = s->wrap;
    𐡄𐡍 (wrap == 2 || (wrap == 1 && s->status != INIT_STATE) || s->lookahead)
        أرجع Z_STREAM_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    𐡄𐡍 (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dictLength);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    𐡄𐡍 (dictLength >= s->w_size) {
        𐡄𐡍 (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->block_start = 0L;
            s->insert = 0;
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
    طالما (s->lookahead >= MIN_MATCH) {
        str = s->strstart;
        n = s->lookahead - (MIN_MATCH-1);
        𐡏𐡁𐡃 {
            UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
            s->prev[str & s->w_mask] = s->head[s->ins_h];
#endif
            s->head[s->ins_h] = (Pos)str;
            str++;
        } طالما (--n);
        s->strstart = str;
        s->lookahead = MIN_MATCH-1;
        fill_window(s);
    }
    s->strstart += s->lookahead;
    s->block_start = (long)s->strstart;
    s->insert = s->lookahead;
    s->lookahead = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    strm->next_in = next;
    strm->avail_in = avail;
    s->wrap = wrap;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dictLength) {
    deflate_state *s;
    uInt len;

    𐡄𐡍 (deflateStateCheck(strm))
        أرجع Z_STREAM_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    𐡄𐡍 (len > s->w_size)
        len = s->w_size;
    𐡄𐡍 (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->window + s->strstart + s->lookahead - len, len);
    𐡄𐡍 (dictLength != Z_NULL)
        *dictLength = len;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    𐡄𐡍 (deflateStateCheck(strm)) {
        أرجع Z_STREAM_ERROR;
    }

    strm->total_in = strm->total_out = 0;
    strm->msg = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->data_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_out = s->pending_buf;

    𐡄𐡍 (s->wrap < 0) {
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

    أرجع Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local فراغ lm_init(deflate_state *s) {
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
    s->insert = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
صحيح ZEXPORT deflateReset(z_streamp strm) {
    صحيح ret;

    ret = deflateResetKeep(strm);
    𐡄𐡍 (ret == Z_OK)
        lm_init(strm->state);
    أرجع ret;
}

/* ========================================================================= */
صحيح ZEXPORT deflateSetHeader(z_streamp strm, gz_headerp head) {
    𐡄𐡍 (deflateStateCheck(strm) || strm->state->wrap != 2)
        أرجع Z_STREAM_ERROR;
    strm->state->gzhead = head;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflatePending(z_streamp strm, unsigned *pending, صحيح *bits) {
    𐡄𐡍 (deflateStateCheck(strm)) أرجع Z_STREAM_ERROR;
    𐡄𐡍 (bits != Z_NULL)
        *bits = strm->state->bi_valid;
    𐡄𐡍 (pending != Z_NULL) {
        *pending = (unsigned)strm->state->pending;
        𐡄𐡍 (*pending != strm->state->pending) {
            *pending = (unsigned)-1;
            أرجع Z_BUF_ERROR;
        }
    }
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateUsed(z_streamp strm, صحيح *bits) {
    𐡄𐡍 (deflateStateCheck(strm)) أرجع Z_STREAM_ERROR;
    𐡄𐡍 (bits != Z_NULL)
        *bits = strm->state->bi_used;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflatePrime(z_streamp strm, صحيح bits, صحيح value) {
    deflate_state *s;
    صحيح put;

    𐡄𐡍 (deflateStateCheck(strm)) أرجع Z_STREAM_ERROR;
    s = strm->state;
#ifdef LIT_MEM
    𐡄𐡍 (bits < 0 || bits > 16 ||
        (uchf *)s->d_buf < s->pending_out + ((Buf_size + 7) >> 3))
        أرجع Z_BUF_ERROR;
#وإلا
    𐡄𐡍 (bits < 0 || bits > 16 ||
        s->sym_buf < s->pending_out + ((Buf_size + 7) >> 3))
        أرجع Z_BUF_ERROR;
#endif
    𐡏𐡁𐡃 {
        put = Buf_size - s->bi_valid;
        𐡄𐡍 (put > bits)
            put = bits;
        s->bi_buf |= (ush)((value & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        _tr_flush_bits(s);
        value >>= put;
        bits -= put;
    } طالما (bits);
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateParams(z_streamp strm, صحيح level, صحيح strategy) {
    deflate_state *s;
    compress_func func;

    𐡄𐡍 (deflateStateCheck(strm)) أرجع Z_STREAM_ERROR;
    s = strm->state;

#ifdef FASTEST
    𐡄𐡍 (level != 0) level = 1;
#وإلا
    𐡄𐡍 (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif
    𐡄𐡍 (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        أرجع Z_STREAM_ERROR;
    }
    func = configuration_table[s->level].func;

    𐡄𐡍 ((strategy != s->strategy || func != configuration_table[level].func) &&
        s->last_flush != -2) {
        /* Flush the last buffer: */
        صحيح err = deflate(strm, Z_BLOCK);
        𐡄𐡍 (err == Z_STREAM_ERROR)
            أرجع err;
        𐡄𐡍 (strm->avail_in || (s->strstart - s->block_start) + s->lookahead)
            أرجع Z_BUF_ERROR;
    }
    𐡄𐡍 (s->level != level) {
        𐡄𐡍 (s->level == 0 && s->matches != 0) {
            𐡄𐡍 (s->matches == 1)
                slide_hash(s);
            وإلا
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
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateTune(z_streamp strm, صحيح good_length, صحيح max_lazy,
                        صحيح nice_length, صحيح max_chain) {
    deflate_state *s;

    𐡄𐡍 (deflateStateCheck(strm)) أرجع Z_STREAM_ERROR;
    s = strm->state;
    s->good_match = (uInt)good_length;
    s->max_lazy_match = (uInt)max_lazy;
    s->nice_match = nice_length;
    s->max_chain_length = (uInt)max_chain;
    أرجع Z_OK;
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
    𐡄𐡍 (fixedlen < sourceLen)
        fixedlen = (z_size_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = sourceLen + (sourceLen >> 5) + (sourceLen >> 7) +
               (sourceLen >> 11) + 7;
    𐡄𐡍 (storelen < sourceLen)
        storelen = (z_size_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    𐡄𐡍 (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        أرجع bound + 18 < bound ? (z_size_t)-1 : bound + 18;
    }

    /* compute wrapper length */
    s = strm->state;
    بدل (s->wrap < 0 ? -s->wrap : s->wrap) {
    حالة 0:                                 /* raw deflate */
        wraplen = 0;
        اكسر;
    حالة 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        اكسر;
#ifdef GZIP
    حالة 2:                                 /* gzip wrapper */
        wraplen = 18;
        𐡄𐡍 (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            𐡄𐡍 (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            𐡄𐡍 (str != Z_NULL)
                𐡏𐡁𐡃 {
                    wraplen++;
                } طالما (*str++);
            str = s->gzhead->comment;
            𐡄𐡍 (str != Z_NULL)
                𐡏𐡁𐡃 {
                    wraplen++;
                } طالما (*str++);
            𐡄𐡍 (s->gzhead->hcrc)
                wraplen += 2;
        }
        اكسر;
#endif
    افتراضي:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    𐡄𐡍 (s->w_bits != 15 || s->hash_bits != 8 + 7) {
        bound = s->w_bits <= s->hash_bits && s->level ? fixedlen :
                                                        storelen;
        أرجع bound + wraplen < bound ? (z_size_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
            (sourceLen >> 25) + 13 - 6 + wraplen;
    أرجع bound < sourceLen ? (z_size_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong sourceLen) {
    z_size_t bound = deflateBound_z(strm, sourceLen);
    أرجع (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local فراغ putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local فراغ flush_pending(z_streamp strm) {
    unsigned len;
    deflate_state *s = strm->state;

    _tr_flush_bits(s);
    len = s->pending > strm->avail_out ? strm->avail_out :
                                         (unsigned)s->pending;
    𐡄𐡍 (len == 0) أرجع;

    zmemcpy(strm->next_out, s->pending_out, len);
    strm->next_out  += len;
    s->pending_out  += len;
    strm->total_out += len;
    strm->avail_out -= len;
    s->pending      -= len;
    𐡄𐡍 (s->pending == 0) {
        s->pending_out = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#define HCRC_UPDATE(beg) \
    𐡏𐡁𐡃 { \
        𐡄𐡍 (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = crc32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } طالما (0)

/* ========================================================================= */
صحيح ZEXPORT deflate(z_streamp strm, صحيح flush) {
    صحيح old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    𐡄𐡍 (deflateStateCheck(strm) || flush > Z_BLOCK || flush < 0) {
        أرجع Z_STREAM_ERROR;
    }
    s = strm->state;

    𐡄𐡍 (strm->next_out == Z_NULL ||
        (strm->avail_in != 0 && strm->next_in == Z_NULL) ||
        (s->status == FINISH_STATE && flush != Z_FINISH)) {
        ERR_RETURN(strm, Z_STREAM_ERROR);
    }
    𐡄𐡍 (strm->avail_out == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->last_flush;
    s->last_flush = flush;

    /* Flush as much pending output as possible */
    𐡄𐡍 (s->pending != 0) {
        flush_pending(strm);
        𐡄𐡍 (strm->avail_out == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->last_flush = -1;
            أرجع Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } وإلا 𐡄𐡍 (strm->avail_in == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_FINISH) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    𐡄𐡍 (s->status == FINISH_STATE && strm->avail_in != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    𐡄𐡍 (s->status == INIT_STATE && s->wrap == 0)
        s->status = BUSY_STATE;
    𐡄𐡍 (s->status == INIT_STATE) {
        /* zlib header */
        uInt header = (Z_DEFLATED + ((s->w_bits - 8) << 4)) << 8;
        uInt level_flags;

        𐡄𐡍 (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        وإلا 𐡄𐡍 (s->level < 6)
            level_flags = 1;
        وإلا 𐡄𐡍 (s->level == 6)
            level_flags = 2;
        وإلا
            level_flags = 3;
        header |= (level_flags << 6);
        𐡄𐡍 (s->strstart != 0) header |= PRESET_DICT;
        header += 31 - (header % 31);

        putShortMSB(s, header);

        /* Save the adler32 of the preset dictionary: */
        𐡄𐡍 (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        𐡄𐡍 (s->pending != 0) {
            s->last_flush = -1;
            أرجع Z_OK;
        }
    }
#ifdef GZIP
    𐡄𐡍 (s->status == GZIP_STATE) {
        /* gzip header */
        strm->adler = crc32(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        𐡄𐡍 (s->gzhead == Z_NULL) {
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
            𐡄𐡍 (s->pending != 0) {
                s->last_flush = -1;
                أرجع Z_OK;
            }
        }
        وإلا {
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
            𐡄𐡍 (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            𐡄𐡍 (s->gzhead->hcrc)
                strm->adler = crc32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->status = EXTRA_STATE;
        }
    }
    𐡄𐡍 (s->status == EXTRA_STATE) {
        𐡄𐡍 (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg left = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            طالما (s->pending + left > s->pending_buf_size) {
                ulg copy = s->pending_buf_size - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_size;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                𐡄𐡍 (s->pending != 0) {
                    s->last_flush = -1;
                    أرجع Z_OK;
                }
                beg = 0;
                left -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, left);
            s->pending += left;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = NAME_STATE;
    }
    𐡄𐡍 (s->status == NAME_STATE) {
        𐡄𐡍 (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            صحيح val;
            𐡏𐡁𐡃 {
                𐡄𐡍 (s->pending == s->pending_buf_size) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    𐡄𐡍 (s->pending != 0) {
                        s->last_flush = -1;
                        أرجع Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } طالما (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = COMMENT_STATE;
    }
    𐡄𐡍 (s->status == COMMENT_STATE) {
        𐡄𐡍 (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            صحيح val;
            𐡏𐡁𐡃 {
                𐡄𐡍 (s->pending == s->pending_buf_size) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    𐡄𐡍 (s->pending != 0) {
                        s->last_flush = -1;
                        أرجع Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } طالما (val != 0);
            HCRC_UPDATE(beg);
        }
        s->status = HCRC_STATE;
    }
    𐡄𐡍 (s->status == HCRC_STATE) {
        𐡄𐡍 (s->gzhead->hcrc) {
            𐡄𐡍 (s->pending + 2 > s->pending_buf_size) {
                flush_pending(strm);
                𐡄𐡍 (s->pending != 0) {
                    s->last_flush = -1;
                    أرجع Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = crc32(0L, Z_NULL, 0);
        }
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        𐡄𐡍 (s->pending != 0) {
            s->last_flush = -1;
            أرجع Z_OK;
        }
    }
#endif

    /* Start a new block or continue the current one.
     */
    𐡄𐡍 (strm->avail_in != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->status != FINISH_STATE)) {
        block_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_table[s->level].func))(s, flush);

        𐡄𐡍 (bstate == finish_started || bstate == finish_done) {
            s->status = FINISH_STATE;
        }
        𐡄𐡍 (bstate == need_more || bstate == finish_started) {
            𐡄𐡍 (strm->avail_out == 0) {
                s->last_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            أرجع Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        𐡄𐡍 (bstate == block_done) {
            𐡄𐡍 (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } وإلا 𐡄𐡍 (flush != Z_BLOCK) { /* FULL_FLUSH or SYNC_FLUSH */
                _tr_stored_block(s, (حرف*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                𐡄𐡍 (flush == Z_FULL_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    𐡄𐡍 (s->lookahead == 0) {
                        s->strstart = 0;
                        s->block_start = 0L;
                        s->insert = 0;
                    }
                }
            }
            flush_pending(strm);
            𐡄𐡍 (strm->avail_out == 0) {
              s->last_flush = -1; /* avoid BUF_ERROR at next call, see above */
              أرجع Z_OK;
            }
        }
    }

    𐡄𐡍 (flush != Z_FINISH) أرجع Z_OK;
    𐡄𐡍 (s->wrap <= 0) أرجع Z_STREAM_END;

    /* Write the trailer */
#ifdef GZIP
    𐡄𐡍 (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->total_in & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 8) & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 16) & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 24) & 0xff));
    }
    وإلا
#endif
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    𐡄𐡍 (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    أرجع s->pending != 0 ? Z_OK : Z_STREAM_END;
}

/* ========================================================================= */
صحيح ZEXPORT deflateEnd(z_streamp strm) {
    صحيح status;

    𐡄𐡍 (deflateStateCheck(strm)) أرجع Z_STREAM_ERROR;

    status = strm->state->status;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->head);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->window);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    أرجع status == BUSY_STATE ? Z_DATA_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
صحيح ZEXPORT deflateCopy(z_streamp dest, z_streamp source) {
#ifdef MAXSEG_64K
    (فراغ)dest;
    (فراغ)source;
    أرجع Z_STREAM_ERROR;
#وإلا
    deflate_state *ds;
    deflate_state *ss;


    𐡄𐡍 (deflateStateCheck(source) || dest == Z_NULL) {
        أرجع Z_STREAM_ERROR;
    }

    ss = source->state;

    zmemcpy(dest, source, حجم(z_stream));

    ds = (deflate_state *) ZALLOC(dest, 1, حجم(deflate_state));
    𐡄𐡍 (ds == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(ds, حجم(deflate_state));
    dest->state = (بنية internal_state FAR *) ds;
    zmemcpy(ds, ss, حجم(deflate_state));
    ds->strm = dest;

    ds->window = (Bytef *) ZALLOC(dest, ds->w_size, 2*حجم(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_size, حجم(Pos));
    ds->head   = (Posf *)  ZALLOC(dest, ds->hash_size, حجم(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    𐡄𐡍 (ds->window == Z_NULL || ds->prev == Z_NULL || ds->head == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflateEnd (dest);
        أرجع Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->window, ss->window, ss->high_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->insert > ds->w_size ? ds->w_size :
                ss->strstart - ss->insert) * حجم(Pos));
    zmemcpy(ds->head, ss->head, ds->hash_size * حجم(Pos));

    ds->pending_out = ds->pending_buf + (ss->pending_out - ss->pending_buf);
    zmemcpy(ds->pending_out, ss->pending_out, ss->pending);
#ifdef LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_next * حجم(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_next);
#وإلا
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_next);
#endif

    ds->l_desc.dyn_tree = ds->dyn_ltree;
    ds->d_desc.dyn_tree = ds->dyn_dtree;
    ds->bl_desc.dyn_tree = ds->bl_tree;

    أرجع Z_OK;
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
    صحيح len;                                    /* length of current match */
    صحيح best_len = (صحيح)s->prev_length;         /* best match length so far */
    صحيح nice_match = s->nice_match;             /* stop if match long enough */
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
#وإلا
    Bytef *strend = s->window + s->strstart + MAX_MATCH;
    Byte scan_end1  = scan[best_len - 1];
    Byte scan_end   = scan[best_len];
#endif

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    𐡄𐡍 (s->prev_length >= s->good_match) {
        chain_length >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    𐡄𐡍 ((uInt)nice_match > s->lookahead) nice_match = (صحيح)s->lookahead;

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "need lookahead");

    𐡏𐡁𐡃 {
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
#𐡄𐡍 (defined(UNALIGNED_OK) && MAX_MATCH == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        𐡄𐡍 (*(ushf*)(match + best_len - 1) != scan_end ||
            *(ushf*)match != scan_start) استمر;

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
        𐡏𐡁𐡃 {
        } طالما (*(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(scan <= s->window + (unsigned)(s->window_size - 1),
               "wild scan");
        𐡄𐡍 (*scan == *match) scan++;

        len = (MAX_MATCH - 1) - (صحيح)(strend - scan);
        scan = strend - (MAX_MATCH-1);

#وإلا /* UNALIGNED_OK */

        𐡄𐡍 (match[best_len]     != scan_end  ||
            match[best_len - 1] != scan_end1 ||
            *match              != *scan     ||
            *++match            != scan[1])      استمر;

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
        𐡏𐡁𐡃 {
        } طالما (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        Assert(scan <= s->window + (unsigned)(s->window_size - 1),
               "wild scan");

        len = MAX_MATCH - (صحيح)(strend - scan);
        scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

        𐡄𐡍 (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            𐡄𐡍 (len >= nice_match) اكسر;
#ifdef UNALIGNED_OK
            scan_end = *(ushf*)(scan + best_len - 1);
#وإلا
            scan_end1  = scan[best_len - 1];
            scan_end   = scan[best_len];
#endif
        }
    } طالما ((cur_match = prev[cur_match & wmask]) > limit
             && --chain_length != 0);

    𐡄𐡍 ((uInt)best_len <= s->lookahead) أرجع (uInt)best_len;
    أرجع s->lookahead;
}

#وإلا /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_match(deflate_state *s, IPos cur_match) {
    Bytef *scan = s->window + s->strstart;      /* current string */
    Bytef *match;                               /* matched string */
    صحيح len;                                    /* length of current match */
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
    𐡄𐡍 (match[0] != scan[0] || match[1] != scan[1]) أرجع MIN_MATCH-1;

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
    𐡏𐡁𐡃 {
    } طالما (*++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             scan < strend);

    Assert(scan <= s->window + (unsigned)(s->window_size - 1), "wild scan");

    len = MAX_MATCH - (صحيح)(strend - scan);

    𐡄𐡍 (len < MIN_MATCH) أرجع MIN_MATCH - 1;

    s->match_start = cur_match;
    أرجع (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#endif /* FASTEST */

#ifdef ZLIB_DEBUG

#define EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local فراغ check_match(deflate_state *s, IPos start, IPos match, صحيح length) {
    /* check that the match is indeed a match */
    Bytef *back = s->window + (صحيح)match, *here = s->window + start;
    IPos len = (IPos)length;
    𐡄𐡍 (match == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        back++;
        here++;
        len--;
    }
    𐡄𐡍 (zmemcmp(back, here, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                start, (صحيح)match, length);
        𐡏𐡁𐡃 {
            fprintf(stderr, "(%02x %02x)", *back++, *here++);
        } طالما (--len != 0);
        z_error("invalid match");
    }
    𐡄𐡍 (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", start - match, length);
        𐡏𐡁𐡃 { putc(s->window[start++], stderr); } طالما (--length != 0);
    }
}
#وإلا
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
   𐡄𐡍 (s->strm->avail_out == 0) أرجع (last) ? finish_started : need_more; \
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
local block_state deflate_stored(deflate_state *s, صحيح flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    unsigned min_block = (unsigned)(MIN(s->pending_buf_size - 5, s->w_size));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    صحيح last = 0;
    unsigned len, left, have;
    unsigned used = s->strm->avail_in;
    𐡏𐡁𐡃 {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((unsigned)s->bi_valid + 42) >> 3;   /* bytes in header */
        𐡄𐡍 (s->strm->avail_out < have)          /* need room for header */
            اكسر;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_out - have;
        left = (unsigned)(s->strstart - s->block_start);    /* window bytes */
        𐡄𐡍 (len > (ulg)left + s->strm->avail_in)
            len = left + s->strm->avail_in;     /* limit len to the input */
        𐡄𐡍 (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        𐡄𐡍 (len < min_block && ((len == 0 && flush != Z_FINISH) ||
                                flush == Z_NO_FLUSH ||
                                len != left + s->strm->avail_in))
            اكسر;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        last = flush == Z_FINISH && len == left + s->strm->avail_in ? 1 : 0;
        _tr_stored_block(s, (حرف *)0, 0L, last);

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
        𐡄𐡍 (left) {
            𐡄𐡍 (left > len)
                left = len;
            zmemcpy(s->strm->next_out, s->window + s->block_start, left);
            s->strm->next_out += left;
            s->strm->avail_out -= left;
            s->strm->total_out += left;
            s->block_start += left;
            len -= left;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        𐡄𐡍 (len) {
            read_buf(s->strm, s->strm->next_out, len);
            s->strm->next_out += len;
            s->strm->avail_out -= len;
            s->strm->total_out += len;
        }
    } طالما (last == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    used -= s->strm->avail_in;      /* number of input bytes directly copied */
    𐡄𐡍 (used) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        𐡄𐡍 (used >= s->w_size) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->window, s->strm->next_in - s->w_size, s->w_size);
            s->strstart = s->w_size;
            s->insert = s->strstart;
        }
        وإلا {
            𐡄𐡍 (s->window_size - s->strstart <= used) {
                /* Slide the window down. */
                s->strstart -= s->w_size;
                zmemcpy(s->window, s->window + s->w_size, s->strstart);
                𐡄𐡍 (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                𐡄𐡍 (s->insert > s->strstart)
                    s->insert = s->strstart;
            }
            zmemcpy(s->window + s->strstart, s->strm->next_in - used, used);
            s->strstart += used;
            s->insert += MIN(used, s->w_size - s->insert);
        }
        s->block_start = s->strstart;
    }
    𐡄𐡍 (s->high_water < s->strstart)
        s->high_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    𐡄𐡍 (last) {
        s->bi_used = 8;
        أرجع finish_done;
    }

    /* If flushing and all input has been consumed, then done. */
    𐡄𐡍 (flush != Z_NO_FLUSH && flush != Z_FINISH &&
        s->strm->avail_in == 0 && (long)s->strstart == s->block_start)
        أرجع block_done;

    /* Fill the window with any remaining input. */
    have = (unsigned)(s->window_size - s->strstart);
    𐡄𐡍 (s->strm->avail_in > have && s->block_start >= (long)s->w_size) {
        /* Slide the window down. */
        s->block_start -= s->w_size;
        s->strstart -= s->w_size;
        zmemcpy(s->window, s->window + s->w_size, s->strstart);
        𐡄𐡍 (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_size;          /* more space now */
        𐡄𐡍 (s->insert > s->strstart)
            s->insert = s->strstart;
    }
    𐡄𐡍 (have > s->strm->avail_in)
        have = s->strm->avail_in;
    𐡄𐡍 (have) {
        read_buf(s->strm, s->window + s->strstart, have);
        s->strstart += have;
        s->insert += MIN(have, s->w_size - s->insert);
    }
    𐡄𐡍 (s->high_water < s->strstart)
        s->high_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((unsigned)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (unsigned)MIN(s->pending_buf_size - have, MAX_STORED);
    min_block = MIN(have, s->w_size);
    left = (unsigned)(s->strstart - s->block_start);
    𐡄𐡍 (left >= min_block ||
        ((left || flush == Z_FINISH) && flush != Z_NO_FLUSH &&
         s->strm->avail_in == 0 && left <= have)) {
        len = MIN(left, have);
        last = flush == Z_FINISH && s->strm->avail_in == 0 &&
               len == left ? 1 : 0;
        _tr_stored_block(s, (charf *)s->window + s->block_start, len, last);
        s->block_start += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    𐡄𐡍 (last)
        s->bi_used = 8;
    أرجع last ? finish_started : need_more;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local block_state deflate_fast(deflate_state *s, صحيح flush) {
    IPos hash_head;       /* head of the hash chain */
    صحيح bflush;           /* set if current block must be flushed */

    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        𐡄𐡍 (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            𐡄𐡍 (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_more;
            }
            𐡄𐡍 (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_head = NIL;
        𐡄𐡍 (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        𐡄𐡍 (hash_head != NIL && s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_length = longest_match (s, hash_head);
            /* longest_match() sets match_start */
        }
        𐡄𐡍 (s->match_length >= MIN_MATCH) {
            check_match(s, s->strstart, s->match_start, (صحيح)s->match_length);

            _tr_tally_dist(s, s->strstart - s->match_start,
                           s->match_length - MIN_MATCH, bflush);

            s->lookahead -= s->match_length;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#ifndef FASTEST
            𐡄𐡍 (s->match_length <= s->max_insert_length &&
                s->lookahead >= MIN_MATCH) {
                s->match_length--; /* string at strstart already in table */
                𐡏𐡁𐡃 {
                    s->strstart++;
                    INSERT_STRING(s, s->strstart, hash_head);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } طالما (--s->match_length != 0);
                s->strstart++;
            } وإلا
#endif
            {
                s->strstart += s->match_length;
                s->match_length = 0;
                s->ins_h = s->window[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->window[s->strstart + 1]);
#𐡄𐡍 MIN_MATCH != 3
                Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        𐡄𐡍 (bflush) FLUSH_BLOCK(s, 0);
    }
    s->insert = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    𐡄𐡍 (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        أرجع finish_done;
    }
    𐡄𐡍 (s->sym_next)
        FLUSH_BLOCK(s, 0);
    أرجع block_done;
}

#ifndef FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local block_state deflate_slow(deflate_state *s, صحيح flush) {
    IPos hash_head;          /* head of hash chain */
    صحيح bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        𐡄𐡍 (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            𐡄𐡍 (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_more;
            }
            𐡄𐡍 (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_head = NIL;
        𐡄𐡍 (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = MIN_MATCH-1;

        𐡄𐡍 (hash_head != NIL && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_length = longest_match (s, hash_head);
            /* longest_match() sets match_start */

            𐡄𐡍 (s->match_length <= 5 && (s->strategy == Z_FILTERED
#𐡄𐡍 TOO_FAR <= 32767
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
        𐡄𐡍 (s->prev_length >= MIN_MATCH && s->match_length <= s->prev_length) {
            uInt max_insert = s->strstart + s->lookahead - MIN_MATCH;
            /* Do not insert strings in hash table beyond this. */

            check_match(s, s->strstart - 1, s->prev_match, (صحيح)s->prev_length);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_match,
                           s->prev_length - MIN_MATCH, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_length - 1;
            s->prev_length -= 2;
            𐡏𐡁𐡃 {
                𐡄𐡍 (++s->strstart <= max_insert) {
                    INSERT_STRING(s, s->strstart, hash_head);
                }
            } طالما (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = MIN_MATCH-1;
            s->strstart++;

            𐡄𐡍 (bflush) FLUSH_BLOCK(s, 0);

        } وإلا 𐡄𐡍 (s->match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->window[s->strstart - 1]));
            _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
            𐡄𐡍 (bflush) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            𐡄𐡍 (s->strm->avail_out == 0) أرجع need_more;
        } وإلا {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    𐡄𐡍 (s->match_available) {
        Tracevv((stderr,"%c", s->window[s->strstart - 1]));
        _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
        s->match_available = 0;
    }
    s->insert = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    𐡄𐡍 (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        أرجع finish_done;
    }
    𐡄𐡍 (s->sym_next)
        FLUSH_BLOCK(s, 0);
    أرجع block_done;
}
#endif /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local block_state deflate_rle(deflate_state *s, صحيح flush) {
    صحيح bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *scan, *strend;   /* scan goes up to strend for length of run */

    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        𐡄𐡍 (s->lookahead <= MAX_MATCH) {
            fill_window(s);
            𐡄𐡍 (s->lookahead <= MAX_MATCH && flush == Z_NO_FLUSH) {
                أرجع need_more;
            }
            𐡄𐡍 (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->match_length = 0;
        𐡄𐡍 (s->lookahead >= MIN_MATCH && s->strstart > 0) {
            scan = s->window + s->strstart - 1;
            prev = *scan;
            𐡄𐡍 (prev == *++scan && prev == *++scan && prev == *++scan) {
                strend = s->window + s->strstart + MAX_MATCH;
                𐡏𐡁𐡃 {
                } طالما (prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         scan < strend);
                s->match_length = MAX_MATCH - (uInt)(strend - scan);
                𐡄𐡍 (s->match_length > s->lookahead)
                    s->match_length = s->lookahead;
            }
            Assert(scan <= s->window + (uInt)(s->window_size - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        𐡄𐡍 (s->match_length >= MIN_MATCH) {
            check_match(s, s->strstart, s->strstart - 1, (صحيح)s->match_length);

            _tr_tally_dist(s, 1, s->match_length - MIN_MATCH, bflush);

            s->lookahead -= s->match_length;
            s->strstart += s->match_length;
            s->match_length = 0;
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        𐡄𐡍 (bflush) FLUSH_BLOCK(s, 0);
    }
    s->insert = 0;
    𐡄𐡍 (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        أرجع finish_done;
    }
    𐡄𐡍 (s->sym_next)
        FLUSH_BLOCK(s, 0);
    أرجع block_done;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local block_state deflate_huff(deflate_state *s, صحيح flush) {
    صحيح bflush;             /* set if current block must be flushed */

    لكل (;;) {
        /* Make sure that we have a literal to write. */
        𐡄𐡍 (s->lookahead == 0) {
            fill_window(s);
            𐡄𐡍 (s->lookahead == 0) {
                𐡄𐡍 (flush == Z_NO_FLUSH)
                    أرجع need_more;
                اكسر;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->match_length = 0;
        Tracevv((stderr,"%c", s->window[s->strstart]));
        _tr_tally_lit(s, s->window[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        𐡄𐡍 (bflush) FLUSH_BLOCK(s, 0);
    }
    s->insert = 0;
    𐡄𐡍 (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        أرجع finish_done;
    }
    𐡄𐡍 (s->sym_next)
        FLUSH_BLOCK(s, 0);
    أرجع block_done;
}
