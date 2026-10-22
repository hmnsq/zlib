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

#ਅਨ੍ਤਰ੍ਭਾਵ "deflate.h"

ਅਚਲ ਅੱਖਰ deflate_copyright[] =
   " deflate 1.3.2.1 Copyright 1995-2026 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
    need_ਹੋਰ,      /* block not completed, need more input or more output */
    ਖੰਡ_ਪੂਰਾ,     /* block flush performed */
    ਅੰਤ_started, /* finish started, need only more output at next deflate */
    ਅੰਤ_ਪੂਰਾ     /* finish done, accept no more input or output */
} ਖੰਡ_state;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖੰਡ_state (*compress_func)(deflate_state *s, ਪੂਰਨ flush);
/* Compression function. Returns the block state after the call. */

local ਖੰਡ_state deflate_stored(deflate_state *s, ਪੂਰਨ flush);
local ਖੰਡ_state deflate_fast(deflate_state *s, ਪੂਰਨ flush);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FASTEST
local ਖੰਡ_state deflate_slow(deflate_state *s, ਪੂਰਨ flush);
#ਜੇ_ਅੰਤ
local ਖੰਡ_state deflate_rle(deflate_state *s, ਪੂਰਨ flush);
local ਖੰਡ_state deflate_huff(deflate_state *s, ਪੂਰਨ flush);

/* ===========================================================================
 * Local data
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NIL 0
/* Tail of hash chains */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TOO_FAR
#  define TOO_FAR 4096
#ਜੇ_ਅੰਤ
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ config_s {
   ush good_ਲੰਮਾਈ; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_ਲੰਮਾਈ; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FASTEST
local ਅਚਲ config configuration_ਸਾਰਣੀ[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#ਨਹੀਂਤਾਂ
local ਅਚਲ config configuration_ਸਾਰਣੀ[10] = {
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
#ਜੇ_ਅੰਤ

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UPDATE_HASH(s,h,c) (h = (((h) << s->hash_ਖਿਸਕਾਓ) ^ (c)) & s->hash_ਨਕਾਬ)


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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FASTEST
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਾਓ_STRING(s, str, ਮੇਲ_ਸਿਰ) \
   (UPDATE_HASH(s, s->ins_h, s->ਖਿੜਕੀ[(str) + (MIN_ਮੇਲ-1)]), \
    ਮੇਲ_ਸਿਰ = s->ਸਿਰ[s->ins_h], \
    s->ਸਿਰ[s->ins_h] = (Pos)(str))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਾਓ_STRING(s, str, ਮੇਲ_ਸਿਰ) \
   (UPDATE_HASH(s, s->ins_h, s->ਖਿੜਕੀ[(str) + (MIN_ਮੇਲ-1)]), \
    ਮੇਲ_ਸਿਰ = s->prev[(str) & s->w_ਨਕਾਬ] = s->ਸਿਰ[s->ins_h], \
    s->ਸਿਰ[s->ins_h] = (Pos)(str))
#ਜੇ_ਅੰਤ

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CLEAR_HASH(s) \
    ਕਰੋ { \
        s->ਸਿਰ[s->hash_ਮਾਪ - 1] = NIL; \
        zmemzero(s->ਸਿਰ, (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->hash_ਮਾਪ - 1)*ਆਕਾਰ(*s->ਸਿਰ)); \
        s->slid = 0; \
    } ਜਦੋਂ (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#ਜੇ defined(__has_feature)
#  ਜੇ __has_feature(ਸਮ੍ਰਿਤੀ_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#ਜੇ_ਅੰਤ
local ਖਾਲੀ slide_hash(deflate_state *s) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ n, m;
    Posf *p;
    uInt wsize = s->w_ਮਾਪ;

    n = s->hash_ਮਾਪ;
    p = &s->ਸਿਰ[n];
    ਕਰੋ {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } ਜਦੋਂ (--n);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FASTEST
    n = wsize;
    p = &s->prev[n];
    ਕਰੋ {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } ਜਦੋਂ (--n);
#ਜੇ_ਅੰਤ
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local ਬਿਨਾਨਿਸ਼ਾਨੀ read_buf(z_streamp strm, Bytef *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ len = strm->avail_ਵਿੱਚ;

    ਜੇ (len > ਮਾਪ) len = ਮਾਪ;
    ਜੇ (len == 0) ਵਾਪਸ 0;

    strm->avail_ਵਿੱਚ  -= len;

    zmemcpy(buf, strm->ਅਗਲਾ_ਵਿੱਚ, len);
    ਜੇ (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
    ਨਹੀਂਤਾਂ ਜੇ (strm->state->wrap == 2) {
        strm->adler = ਚੱਕਰੀ_ਜਾਂਚ_32_2(strm->adler, buf, len);
    }
#ਜੇ_ਅੰਤ
    strm->ਅਗਲਾ_ਵਿੱਚ  += len;
    strm->ਕੁੱਲ_ਵਿੱਚ += len;

    ਵਾਪਸ len;
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
local ਖਾਲੀ fill_ਖਿੜਕੀ(deflate_state *s) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਹੋਰ;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_ਮਾਪ;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    ਕਰੋ {
        ਹੋਰ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->ਖਿੜਕੀ_ਮਾਪ -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਹਦਾਇਤ warning(ਧੱਕੋ)
#ਹਦਾਇਤ warning(disable: 4127)
#ਜੇ_ਅੰਤ
        ਜੇ (ਆਕਾਰ(ਪੂਰਨ) <= 2) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਹਦਾਇਤ warning(ਕੱਢੋ)
#ਜੇ_ਅੰਤ
            ਜੇ (ਹੋਰ == 0 && s->strstart == 0 && s->lookahead == 0) {
                ਹੋਰ = wsize;

            } ਨਹੀਂਤਾਂ ਜੇ (ਹੋਰ == (ਬਿਨਾਨਿਸ਼ਾਨੀ)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                ਹੋਰ--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        ਜੇ (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->ਖਿੜਕੀ, s->ਖਿੜਕੀ + wsize, (ਬਿਨਾਨਿਸ਼ਾਨੀ)wsize - ਹੋਰ);
            s->ਮੇਲ_ਸ਼ੁਰੂ -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->ਖੰਡ_ਸ਼ੁਰੂ -= (ਲੰਮਾ) wsize;
            ਜੇ (s->ਪਾਓ > s->strstart)
                s->ਪਾਓ = s->strstart;
            slide_hash(s);
            ਹੋਰ += wsize;
        }
        ਜੇ (s->strm->avail_ਵਿੱਚ == 0) ਰੋਕੋ;

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
        Assert(ਹੋਰ >= 2, "more < 2");

        n = read_buf(s->strm, s->ਖਿੜਕੀ + s->strstart + s->lookahead, ਹੋਰ);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        ਜੇ (s->lookahead + s->ਪਾਓ >= MIN_ਮੇਲ) {
            uInt str = s->strstart - s->ਪਾਓ;
            s->ins_h = s->ਖਿੜਕੀ[str];
            UPDATE_HASH(s, s->ins_h, s->ਖਿੜਕੀ[str + 1]);
#ਜੇ MIN_ਮੇਲ != 3
            Call UPDATE_HASH() MIN_ਮੇਲ-3 ਹੋਰ times
#ਜੇ_ਅੰਤ
            ਜਦੋਂ (s->ਪਾਓ) {
                UPDATE_HASH(s, s->ins_h, s->ਖਿੜਕੀ[str + MIN_ਮੇਲ-1]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FASTEST
                s->prev[str & s->w_ਨਕਾਬ] = s->ਸਿਰ[s->ins_h];
#ਜੇ_ਅੰਤ
                s->ਸਿਰ[s->ins_h] = (Pos)str;
                str++;
                s->ਪਾਓ--;
                ਜੇ (s->lookahead + s->ਪਾਓ < MIN_ਮੇਲ)
                    ਰੋਕੋ;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } ਜਦੋਂ (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_ਵਿੱਚ != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    ਜੇ (s->ਉੱਚਾ_water < s->ਖਿੜਕੀ_ਮਾਪ) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        ਜੇ (s->ਉੱਚਾ_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->ਖਿੜਕੀ_ਮਾਪ - curr;
            ਜੇ (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->ਖਿੜਕੀ + curr, (ਬਿਨਾਨਿਸ਼ਾਨੀ)init);
            s->ਉੱਚਾ_water = curr + init;
        }
        ਨਹੀਂਤਾਂ ਜੇ (s->ਉੱਚਾ_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->ਉੱਚਾ_water;
            ਜੇ (init > s->ਖਿੜਕੀ_ਮਾਪ - s->ਉੱਚਾ_water)
                init = s->ਖਿੜਕੀ_ਮਾਪ - s->ਉੱਚਾ_water;
            zmemzero(s->ਖਿੜਕੀ + s->ਉੱਚਾ_water, (ਬਿਨਾਨਿਸ਼ਾਨੀ)init);
            s->ਉੱਚਾ_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->ਖਿੜਕੀ_ਮਾਪ - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflateInit_(z_streamp strm, ਪੂਰਨ level, ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ,
                         ਪੂਰਨ ਧਾਰਾ_ਮਾਪ) {
    ਵਾਪਸ deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, ਸੰਸਕਰਣ, ਧਾਰਾ_ਮਾਪ);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflateInit2_(z_streamp strm, ਪੂਰਨ level, ਪੂਰਨ ਵਿਧੀ_2,
                          ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ, ਪੂਰਨ memLevel, ਪੂਰਨ strategy,
                          ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ, ਪੂਰਨ ਧਾਰਾ_ਮਾਪ) {
    deflate_state *s;
    ਪੂਰਨ wrap = 1;
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ my_ਸੰਸਕਰਣ[] = ZLIB_ਸੰਸਕਰਣ;

    ਜੇ (ਸੰਸਕਰਣ == Z_NULL || ਸੰਸਕਰਣ[0] != my_ਸੰਸਕਰਣ[0] ||
        ਧਾਰਾ_ਮਾਪ != ਆਕਾਰ(z_ਧਾਰਾ)) {
        ਵਾਪਸ Z_ਸੰਸਕਰਣ_ERROR;
    }
    ਜੇ (strm == Z_NULL) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    strm->ਸੁਨੇਹਾ = Z_NULL;
    ਜੇ (strm->zalloc == (alloc_func)0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
#ਨਹੀਂਤਾਂ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#ਜੇ_ਅੰਤ
    }
    ਜੇ (strm->zfree == (free_func)0)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
#ਨਹੀਂਤਾਂ
        strm->zfree = zcfree;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FASTEST
    ਜੇ (level != 0) level = 1;
#ਨਹੀਂਤਾਂ
    ਜੇ (level == Z_DEFAULT_COMPRESSION) level = 6;
#ਜੇ_ਅੰਤ

    ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ < 0) { /* suppress zlib wrapper */
        wrap = 0;
        ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ < -15)
            ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
        ਖਿੜਕੀ_ਬਿੱਟ = -ਖਿੜਕੀ_ਬਿੱਟ;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
    ਨਹੀਂਤਾਂ ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        ਖਿੜਕੀ_ਬਿੱਟ -= 16;
    }
#ਜੇ_ਅੰਤ
    ਜੇ (memLevel < 1 || memLevel > MAX_MEM_LEVEL || ਵਿਧੀ_2 != Z_DEFLATED ||
        ਖਿੜਕੀ_ਬਿੱਟ < 8 || ਖਿੜਕੀ_ਬਿੱਟ > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (ਖਿੜਕੀ_ਬਿੱਟ == 8 && wrap != 1)) {
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    }
    ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ == 8) ਖਿੜਕੀ_ਬਿੱਟ = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, ਆਕਾਰ(deflate_state));
    ਜੇ (s == Z_NULL) ਵਾਪਸ Z_MEM_ERROR;
    zmemzero(s, ਆਕਾਰ(deflate_state));
    strm->state = (ਬਣਤਰ ਅੰਦਰੂਨੀ_state FAR *)s;
    s->strm = strm;
    s->ਸਥਿਤੀ = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_ਬਿੱਟ = (uInt)ਖਿੜਕੀ_ਬਿੱਟ;
    s->w_ਮਾਪ = 1 << s->w_ਬਿੱਟ;
    s->w_ਨਕਾਬ = s->w_ਮਾਪ - 1;

    s->hash_ਬਿੱਟ = (uInt)memLevel + 7;
    s->hash_ਮਾਪ = 1 << s->hash_ਬਿੱਟ;
    s->hash_ਨਕਾਬ = s->hash_ਮਾਪ - 1;
    s->hash_ਖਿਸਕਾਓ =  ((s->hash_ਬਿੱਟ + MIN_ਮੇਲ-1) / MIN_ਮੇਲ);

    s->ਖਿੜਕੀ = (Bytef *) ZALLOC(strm, s->w_ਮਾਪ, 2*ਆਕਾਰ(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_ਮਾਪ, ਆਕਾਰ(Pos));
    s->ਸਿਰ   = (Posf *)  ZALLOC(strm, s->hash_ਮਾਪ, ਆਕਾਰ(Pos));

    s->ਉੱਚਾ_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_ਮਾਪ = (ulg)s->lit_bufsize * 4;

    ਜੇ (s->ਖਿੜਕੀ == Z_NULL || s->prev == Z_NULL || s->ਸਿਰ == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->ਸਥਿਤੀ = ਅੰਤ_STATE;
        strm->ਸੁਨੇਹਾ = ERR_ਸੁਨੇਹਾ(Z_MEM_ERROR);
        deflate_ਸਿਰਾ (strm);
        ਵਾਪਸ Z_MEM_ERROR;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_ਸਿਰਾ = s->lit_bufsize - 1;
#ਨਹੀਂਤਾਂ
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_ਸਿਰਾ = (s->lit_bufsize - 1) * 3;
#ਜੇ_ਅੰਤ
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->ਵਿਧੀ_2 = (Byte)ਵਿਧੀ_2;

    ਵਾਪਸ deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local ਪੂਰਨ deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    ਜੇ (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        ਵਾਪਸ 1;
    s = strm->state;
    ਜੇ (s == Z_NULL || s->strm != strm || (s->ਸਥਿਤੀ != INIT_STATE &&
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
                                           s->ਸਥਿਤੀ != GZIP_STATE &&
#ਜੇ_ਅੰਤ
                                           s->ਸਥਿਤੀ != EXTRA_STATE &&
                                           s->ਸਥਿਤੀ != NAME_STATE &&
                                           s->ਸਥਿਤੀ != COMMENT_STATE &&
                                           s->ਸਥਿਤੀ != HCRC_STATE &&
                                           s->ਸਥਿਤੀ != BUSY_STATE &&
                                           s->ਸਥਿਤੀ != ਅੰਤ_STATE))
        ਵਾਪਸ 1;
    ਵਾਪਸ 0;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflate_ਸੈੱਟ_Dictionary(z_streamp strm, ਅਚਲ Bytef *dictionary,
                                 uInt  dict_ਲੰਮਾਈ) {
    deflate_state *s;
    uInt str, n;
    ਪੂਰਨ wrap;
    ਬਿਨਾਨਿਸ਼ਾਨੀ avail;
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅਗਲਾ;

    ਜੇ (deflateStateCheck(strm) || dictionary == Z_NULL)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    s = strm->state;
    wrap = s->wrap;
    ਜੇ (wrap == 2 || (wrap == 1 && s->ਸਥਿਤੀ != INIT_STATE) || s->lookahead)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    ਜੇ (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_ਲੰਮਾਈ);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    ਜੇ (dict_ਲੰਮਾਈ >= s->w_ਮਾਪ) {
        ਜੇ (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->ਖੰਡ_ਸ਼ੁਰੂ = 0L;
            s->ਪਾਓ = 0;
        }
        dictionary += dict_ਲੰਮਾਈ - s->w_ਮਾਪ;  /* use the tail */
        dict_ਲੰਮਾਈ = s->w_ਮਾਪ;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_ਵਿੱਚ;
    ਅਗਲਾ = strm->ਅਗਲਾ_ਵਿੱਚ;
    strm->avail_ਵਿੱਚ = dict_ਲੰਮਾਈ;
    strm->ਅਗਲਾ_ਵਿੱਚ = (z_const Bytef *)dictionary;
    fill_ਖਿੜਕੀ(s);
    ਜਦੋਂ (s->lookahead >= MIN_ਮੇਲ) {
        str = s->strstart;
        n = s->lookahead - (MIN_ਮੇਲ-1);
        ਕਰੋ {
            UPDATE_HASH(s, s->ins_h, s->ਖਿੜਕੀ[str + MIN_ਮੇਲ-1]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FASTEST
            s->prev[str & s->w_ਨਕਾਬ] = s->ਸਿਰ[s->ins_h];
#ਜੇ_ਅੰਤ
            s->ਸਿਰ[s->ins_h] = (Pos)str;
            str++;
        } ਜਦੋਂ (--n);
        s->strstart = str;
        s->lookahead = MIN_ਮੇਲ-1;
        fill_ਖਿੜਕੀ(s);
    }
    s->strstart += s->lookahead;
    s->ਖੰਡ_ਸ਼ੁਰੂ = (ਲੰਮਾ)s->strstart;
    s->ਪਾਓ = s->lookahead;
    s->lookahead = 0;
    s->ਮੇਲ_ਲੰਮਾਈ = s->prev_ਲੰਮਾਈ = MIN_ਮੇਲ-1;
    s->ਮੇਲ_ਉਪਲਬਧ = 0;
    strm->ਅਗਲਾ_ਵਿੱਚ = ਅਗਲਾ;
    strm->avail_ਵਿੱਚ = avail;
    s->wrap = wrap;
    ਵਾਪਸ Z_OK;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_ਲੰਮਾਈ) {
    deflate_state *s;
    uInt len;

    ਜੇ (deflateStateCheck(strm))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    ਜੇ (len > s->w_ਮਾਪ)
        len = s->w_ਮਾਪ;
    ਜੇ (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->ਖਿੜਕੀ + s->strstart + s->lookahead - len, len);
    ਜੇ (dict_ਲੰਮਾਈ != Z_NULL)
        *dict_ਲੰਮਾਈ = len;
    ਵਾਪਸ Z_OK;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    ਜੇ (deflateStateCheck(strm)) {
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    }

    strm->ਕੁੱਲ_ਵਿੱਚ = strm->ਕੁੱਲ_ਬਾਹਰਲਾ = 0;
    strm->ਸੁਨੇਹਾ = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->ਅੰਕੜੇ_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_ਬਾਹਰਲਾ = s->pending_buf;

    ਜੇ (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->ਸਥਿਤੀ =
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
        s->wrap == 2 ? GZIP_STATE :
#ਜੇ_ਅੰਤ
        INIT_STATE;
    strm->adler =
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
        s->wrap == 2 ? ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0) :
#ਜੇ_ਅੰਤ
        adler32(0L, Z_NULL, 0);
    s->ਆਖਰੀ_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    ਵਾਪਸ Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local ਖਾਲੀ lm_init(deflate_state *s) {
    s->ਖਿੜਕੀ_ਮਾਪ = (ulg)2L*s->w_ਮਾਪ;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_ਮੇਲ   = configuration_ਸਾਰਣੀ[s->level].max_lazy;
    s->good_ਮੇਲ       = configuration_ਸਾਰਣੀ[s->level].good_ਲੰਮਾਈ;
    s->nice_ਮੇਲ       = configuration_ਸਾਰਣੀ[s->level].nice_ਲੰਮਾਈ;
    s->max_chain_ਲੰਮਾਈ = configuration_ਸਾਰਣੀ[s->level].max_chain;

    s->strstart = 0;
    s->ਖੰਡ_ਸ਼ੁਰੂ = 0L;
    s->lookahead = 0;
    s->ਪਾਓ = 0;
    s->ਮੇਲ_ਲੰਮਾਈ = s->prev_ਲੰਮਾਈ = MIN_ਮੇਲ-1;
    s->ਮੇਲ_ਉਪਲਬਧ = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflateReset(z_streamp strm) {
    ਪੂਰਨ ret;

    ret = deflateResetKeep(strm);
    ਜੇ (ret == Z_OK)
        lm_init(strm->state);
    ਵਾਪਸ ret;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflate_ਸੈੱਟ_ਸਿਰਲੇਖ(z_streamp strm, gz_headerp ਸਿਰ) {
    ਜੇ (deflateStateCheck(strm) || strm->state->wrap != 2)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    strm->state->gzhead = ਸਿਰ;
    ਵਾਪਸ Z_OK;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflatePending(z_streamp strm, ਬਿਨਾਨਿਸ਼ਾਨੀ *pending, ਪੂਰਨ *ਬਿੱਟ) {
    ਜੇ (deflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ਜੇ (ਬਿੱਟ != Z_NULL)
        *ਬਿੱਟ = strm->state->bi_valid;
    ਜੇ (pending != Z_NULL) {
        *pending = (ਬਿਨਾਨਿਸ਼ਾਨੀ)strm->state->pending;
        ਜੇ (*pending != strm->state->pending) {
            *pending = (ਬਿਨਾਨਿਸ਼ਾਨੀ)-1;
            ਵਾਪਸ Z_BUF_ERROR;
        }
    }
    ਵਾਪਸ Z_OK;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflate_ਵਰਤਿਆ(z_streamp strm, ਪੂਰਨ *ਬਿੱਟ) {
    ਜੇ (deflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ਜੇ (ਬਿੱਟ != Z_NULL)
        *ਬਿੱਟ = strm->state->bi_ਵਰਤਿਆ;
    ਵਾਪਸ Z_OK;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflate_ਅਭਾਜ(z_streamp strm, ਪੂਰਨ ਬਿੱਟ, ਪੂਰਨ ਮੁੱਲ) {
    deflate_state *s;
    ਪੂਰਨ put;

    ਜੇ (deflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    s = strm->state;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
    ਜੇ (ਬਿੱਟ < 0 || ਬਿੱਟ > 16 ||
        (uchf *)s->d_buf < s->pending_ਬਾਹਰਲਾ + ((Buf_ਮਾਪ + 7) >> 3))
        ਵਾਪਸ Z_BUF_ERROR;
#ਨਹੀਂਤਾਂ
    ਜੇ (ਬਿੱਟ < 0 || ਬਿੱਟ > 16 ||
        s->sym_buf < s->pending_ਬਾਹਰਲਾ + ((Buf_ਮਾਪ + 7) >> 3))
        ਵਾਪਸ Z_BUF_ERROR;
#ਜੇ_ਅੰਤ
    ਕਰੋ {
        put = Buf_ਮਾਪ - s->bi_valid;
        ਜੇ (put > ਬਿੱਟ)
            put = ਬਿੱਟ;
        s->bi_buf |= (ush)((ਮੁੱਲ & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_ਬਿੱਟ(s);
        ਮੁੱਲ >>= put;
        ਬਿੱਟ -= put;
    } ਜਦੋਂ (ਬਿੱਟ);
    ਵਾਪਸ Z_OK;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflateParams(z_streamp strm, ਪੂਰਨ level, ਪੂਰਨ strategy) {
    deflate_state *s;
    compress_func func;

    ਜੇ (deflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    s = strm->state;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FASTEST
    ਜੇ (level != 0) level = 1;
#ਨਹੀਂਤਾਂ
    ਜੇ (level == Z_DEFAULT_COMPRESSION) level = 6;
#ਜੇ_ਅੰਤ
    ਜੇ (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    }
    func = configuration_ਸਾਰਣੀ[s->level].func;

    ਜੇ ((strategy != s->strategy || func != configuration_ਸਾਰਣੀ[level].func) &&
        s->ਆਖਰੀ_flush != -2) {
        /* Flush the last buffer: */
        ਪੂਰਨ err = deflate(strm, Z_ਖੰਡ);
        ਜੇ (err == Z_ਧਾਰਾ_ERROR)
            ਵਾਪਸ err;
        ਜੇ (strm->avail_ਵਿੱਚ || (s->strstart - s->ਖੰਡ_ਸ਼ੁਰੂ) + s->lookahead)
            ਵਾਪਸ Z_BUF_ERROR;
    }
    ਜੇ (s->level != level) {
        ਜੇ (s->level == 0 && s->matches != 0) {
            ਜੇ (s->matches == 1)
                slide_hash(s);
            ਨਹੀਂਤਾਂ
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_ਮੇਲ   = configuration_ਸਾਰਣੀ[level].max_lazy;
        s->good_ਮੇਲ       = configuration_ਸਾਰਣੀ[level].good_ਲੰਮਾਈ;
        s->nice_ਮੇਲ       = configuration_ਸਾਰਣੀ[level].nice_ਲੰਮਾਈ;
        s->max_chain_ਲੰਮਾਈ = configuration_ਸਾਰਣੀ[level].max_chain;
    }
    s->strategy = strategy;
    ਵਾਪਸ Z_OK;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflateTune(z_streamp strm, ਪੂਰਨ good_ਲੰਮਾਈ, ਪੂਰਨ max_lazy,
                        ਪੂਰਨ nice_ਲੰਮਾਈ, ਪੂਰਨ max_chain) {
    deflate_state *s;

    ਜੇ (deflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    s = strm->state;
    s->good_ਮੇਲ = (uInt)good_ਲੰਮਾਈ;
    s->max_lazy_ਮੇਲ = (uInt)max_lazy;
    s->nice_ਮੇਲ = nice_ਲੰਮਾਈ;
    s->max_chain_ਲੰਮਾਈ = (uInt)max_chain;
    ਵਾਪਸ Z_OK;
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
z_ਮਾਪ_t ZEXPORT deflateBound_z(z_streamp strm, z_ਮਾਪ_t ਸਰੋਤ_Len) {
    deflate_state *s;
    z_ਮਾਪ_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = ਸਰੋਤ_Len + (ਸਰੋਤ_Len >> 3) + (ਸਰੋਤ_Len >> 8) +
               (ਸਰੋਤ_Len >> 9) + 4;
    ਜੇ (fixedlen < ਸਰੋਤ_Len)
        fixedlen = (z_ਮਾਪ_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = ਸਰੋਤ_Len + (ਸਰੋਤ_Len >> 5) + (ਸਰੋਤ_Len >> 7) +
               (ਸਰੋਤ_Len >> 11) + 7;
    ਜੇ (storelen < ਸਰੋਤ_Len)
        storelen = (z_ਮਾਪ_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    ਜੇ (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        ਵਾਪਸ bound + 18 < bound ? (z_ਮਾਪ_t)-1 : bound + 18;
    }

    /* compute wrapper length */
    s = strm->state;
    ਚੋਣ (s->wrap < 0 ? -s->wrap : s->wrap) {
    ਹਾਲਤ 0:                                 /* raw deflate */
        wraplen = 0;
        ਰੋਕੋ;
    ਹਾਲਤ 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
    ਹਾਲਤ 2:                                 /* gzip wrapper */
        wraplen = 18;
        ਜੇ (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            ਜੇ (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            ਜੇ (str != Z_NULL)
                ਕਰੋ {
                    wraplen++;
                } ਜਦੋਂ (*str++);
            str = s->gzhead->comment;
            ਜੇ (str != Z_NULL)
                ਕਰੋ {
                    wraplen++;
                } ਜਦੋਂ (*str++);
            ਜੇ (s->gzhead->hcrc)
                wraplen += 2;
        }
        ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    ਮੂਲ:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    ਜੇ (s->w_ਬਿੱਟ != 15 || s->hash_ਬਿੱਟ != 8 + 7) {
        bound = s->w_ਬਿੱਟ <= s->hash_ਬਿੱਟ && s->level ? fixedlen :
                                                        storelen;
        ਵਾਪਸ bound + wraplen < bound ? (z_ਮਾਪ_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = ਸਰੋਤ_Len + (ਸਰੋਤ_Len >> 12) + (ਸਰੋਤ_Len >> 14) +
            (ਸਰੋਤ_Len >> 25) + 13 - 6 + wraplen;
    ਵਾਪਸ bound < ਸਰੋਤ_Len ? (z_ਮਾਪ_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong ਸਰੋਤ_Len) {
    z_ਮਾਪ_t bound = deflateBound_z(strm, ਸਰੋਤ_Len);
    ਵਾਪਸ (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local ਖਾਲੀ putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local ਖਾਲੀ flush_pending(z_streamp strm) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;
    deflate_state *s = strm->state;

    tr_flush_ਬਿੱਟ(s);
    len = s->pending > strm->avail_ਬਾਹਰਲਾ ? strm->avail_ਬਾਹਰਲਾ :
                                         (ਬਿਨਾਨਿਸ਼ਾਨੀ)s->pending;
    ਜੇ (len == 0) ਵਾਪਸ;

    zmemcpy(strm->ਅਗਲਾ_ਬਾਹਰਲਾ, s->pending_ਬਾਹਰਲਾ, len);
    strm->ਅਗਲਾ_ਬਾਹਰਲਾ  += len;
    s->pending_ਬਾਹਰਲਾ  += len;
    strm->ਕੁੱਲ_ਬਾਹਰਲਾ += len;
    strm->avail_ਬਾਹਰਲਾ -= len;
    s->pending      -= len;
    ਜੇ (s->pending == 0) {
        s->pending_ਬਾਹਰਲਾ = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HCRC_UPDATE(beg) \
    ਕਰੋ { \
        ਜੇ (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = ਚੱਕਰੀ_ਜਾਂਚ_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } ਜਦੋਂ (0)

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflate(z_streamp strm, ਪੂਰਨ flush) {
    ਪੂਰਨ old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    ਜੇ (deflateStateCheck(strm) || flush > Z_ਖੰਡ || flush < 0) {
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    }
    s = strm->state;

    ਜੇ (strm->ਅਗਲਾ_ਬਾਹਰਲਾ == Z_NULL ||
        (strm->avail_ਵਿੱਚ != 0 && strm->ਅਗਲਾ_ਵਿੱਚ == Z_NULL) ||
        (s->ਸਥਿਤੀ == ਅੰਤ_STATE && flush != Z_ਅੰਤ)) {
        ERR_RETURN(strm, Z_ਧਾਰਾ_ERROR);
    }
    ਜੇ (strm->avail_ਬਾਹਰਲਾ == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->ਆਖਰੀ_flush;
    s->ਆਖਰੀ_flush = flush;

    /* Flush as much pending output as possible */
    ਜੇ (s->pending != 0) {
        flush_pending(strm);
        ਜੇ (strm->avail_ਬਾਹਰਲਾ == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->ਆਖਰੀ_flush = -1;
            ਵਾਪਸ Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } ਨਹੀਂਤਾਂ ਜੇ (strm->avail_ਵਿੱਚ == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_ਅੰਤ) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    ਜੇ (s->ਸਥਿਤੀ == ਅੰਤ_STATE && strm->avail_ਵਿੱਚ != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    ਜੇ (s->ਸਥਿਤੀ == INIT_STATE && s->wrap == 0)
        s->ਸਥਿਤੀ = BUSY_STATE;
    ਜੇ (s->ਸਥਿਤੀ == INIT_STATE) {
        /* zlib header */
        uInt ਸਿਰਲੇਖ = (Z_DEFLATED + ((s->w_ਬਿੱਟ - 8) << 4)) << 8;
        uInt level_flags;

        ਜੇ (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        ਨਹੀਂਤਾਂ ਜੇ (s->level < 6)
            level_flags = 1;
        ਨਹੀਂਤਾਂ ਜੇ (s->level == 6)
            level_flags = 2;
        ਨਹੀਂਤਾਂ
            level_flags = 3;
        ਸਿਰਲੇਖ |= (level_flags << 6);
        ਜੇ (s->strstart != 0) ਸਿਰਲੇਖ |= PRESET_DICT;
        ਸਿਰਲੇਖ += 31 - (ਸਿਰਲੇਖ % 31);

        putShortMSB(s, ਸਿਰਲੇਖ);

        /* Save the adler32 of the preset dictionary: */
        ਜੇ (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->ਸਥਿਤੀ = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        ਜੇ (s->pending != 0) {
            s->ਆਖਰੀ_flush = -1;
            ਵਾਪਸ Z_OK;
        }
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
    ਜੇ (s->ਸਥਿਤੀ == GZIP_STATE) {
        /* gzip header */
        strm->adler = ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        ਜੇ (s->gzhead == Z_NULL) {
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, OS_ਕੋਡ);
            s->ਸਥਿਤੀ = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            ਜੇ (s->pending != 0) {
                s->ਆਖਰੀ_flush = -1;
                ਵਾਪਸ Z_OK;
            }
        }
        ਨਹੀਂਤਾਂ {
            put_byte(s, (s->gzhead->ਪਾਠ ? 1 : 0) +
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
            ਜੇ (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            ਜੇ (s->gzhead->hcrc)
                strm->adler = ਚੱਕਰੀ_ਜਾਂਚ_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->ਸਥਿਤੀ = EXTRA_STATE;
        }
    }
    ਜੇ (s->ਸਥਿਤੀ == EXTRA_STATE) {
        ਜੇ (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg ਖੱਬਾ = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            ਜਦੋਂ (s->pending + ਖੱਬਾ > s->pending_buf_ਮਾਪ) {
                ulg copy = s->pending_buf_ਮਾਪ - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_ਮਾਪ;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                ਜੇ (s->pending != 0) {
                    s->ਆਖਰੀ_flush = -1;
                    ਵਾਪਸ Z_OK;
                }
                beg = 0;
                ਖੱਬਾ -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, ਖੱਬਾ);
            s->pending += ਖੱਬਾ;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->ਸਥਿਤੀ = NAME_STATE;
    }
    ਜੇ (s->ਸਥਿਤੀ == NAME_STATE) {
        ਜੇ (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ਪੂਰਨ val;
            ਕਰੋ {
                ਜੇ (s->pending == s->pending_buf_ਮਾਪ) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    ਜੇ (s->pending != 0) {
                        s->ਆਖਰੀ_flush = -1;
                        ਵਾਪਸ Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } ਜਦੋਂ (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->ਸਥਿਤੀ = COMMENT_STATE;
    }
    ਜੇ (s->ਸਥਿਤੀ == COMMENT_STATE) {
        ਜੇ (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ਪੂਰਨ val;
            ਕਰੋ {
                ਜੇ (s->pending == s->pending_buf_ਮਾਪ) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    ਜੇ (s->pending != 0) {
                        s->ਆਖਰੀ_flush = -1;
                        ਵਾਪਸ Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } ਜਦੋਂ (val != 0);
            HCRC_UPDATE(beg);
        }
        s->ਸਥਿਤੀ = HCRC_STATE;
    }
    ਜੇ (s->ਸਥਿਤੀ == HCRC_STATE) {
        ਜੇ (s->gzhead->hcrc) {
            ਜੇ (s->pending + 2 > s->pending_buf_ਮਾਪ) {
                flush_pending(strm);
                ਜੇ (s->pending != 0) {
                    s->ਆਖਰੀ_flush = -1;
                    ਵਾਪਸ Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0);
        }
        s->ਸਥਿਤੀ = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        ਜੇ (s->pending != 0) {
            s->ਆਖਰੀ_flush = -1;
            ਵਾਪਸ Z_OK;
        }
    }
#ਜੇ_ਅੰਤ

    /* Start a new block or continue the current one.
     */
    ਜੇ (strm->avail_ਵਿੱਚ != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->ਸਥਿਤੀ != ਅੰਤ_STATE)) {
        ਖੰਡ_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_ਸਾਰਣੀ[s->level].func))(s, flush);

        ਜੇ (bstate == ਅੰਤ_started || bstate == ਅੰਤ_ਪੂਰਾ) {
            s->ਸਥਿਤੀ = ਅੰਤ_STATE;
        }
        ਜੇ (bstate == need_ਹੋਰ || bstate == ਅੰਤ_started) {
            ਜੇ (strm->avail_ਬਾਹਰਲਾ == 0) {
                s->ਆਖਰੀ_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            ਵਾਪਸ Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        ਜੇ (bstate == ਖੰਡ_ਪੂਰਾ) {
            ਜੇ (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } ਨਹੀਂਤਾਂ ਜੇ (flush != Z_ਖੰਡ) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_ਖੰਡ(s, (ਅੱਖਰ*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                ਜੇ (flush == Z_ਭਰਿਆ_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    ਜੇ (s->lookahead == 0) {
                        s->strstart = 0;
                        s->ਖੰਡ_ਸ਼ੁਰੂ = 0L;
                        s->ਪਾਓ = 0;
                    }
                }
            }
            flush_pending(strm);
            ਜੇ (strm->avail_ਬਾਹਰਲਾ == 0) {
              s->ਆਖਰੀ_flush = -1; /* avoid BUF_ERROR at next call, see above */
              ਵਾਪਸ Z_OK;
            }
        }
    }

    ਜੇ (flush != Z_ਅੰਤ) ਵਾਪਸ Z_OK;
    ਜੇ (s->wrap <= 0) ਵਾਪਸ Z_ਧਾਰਾ_ਸਿਰਾ;

    /* Write the trailer */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ GZIP
    ਜੇ (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->ਕੁੱਲ_ਵਿੱਚ & 0xff));
        put_byte(s, (Byte)((strm->ਕੁੱਲ_ਵਿੱਚ >> 8) & 0xff));
        put_byte(s, (Byte)((strm->ਕੁੱਲ_ਵਿੱਚ >> 16) & 0xff));
        put_byte(s, (Byte)((strm->ਕੁੱਲ_ਵਿੱਚ >> 24) & 0xff));
    }
    ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    ਜੇ (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    ਵਾਪਸ s->pending != 0 ? Z_OK : Z_ਧਾਰਾ_ਸਿਰਾ;
}

/* ========================================================================= */
ਪੂਰਨ ZEXPORT deflate_ਸਿਰਾ(z_streamp strm) {
    ਪੂਰਨ ਸਥਿਤੀ;

    ਜੇ (deflateStateCheck(strm)) ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    ਸਥਿਤੀ = strm->state->ਸਥਿਤੀ;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->ਸਿਰ);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->ਖਿੜਕੀ);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    ਵਾਪਸ ਸਥਿਤੀ == BUSY_STATE ? Z_ਅੰਕੜੇ_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
ਪੂਰਨ ZEXPORT deflateCopy(z_streamp dest, z_streamp ਸਰੋਤ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAXSEG_64K
    (ਖਾਲੀ)dest;
    (ਖਾਲੀ)ਸਰੋਤ;
    ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
#ਨਹੀਂਤਾਂ
    deflate_state *ds;
    deflate_state *ss;


    ਜੇ (deflateStateCheck(ਸਰੋਤ) || dest == Z_NULL) {
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    }

    ss = ਸਰੋਤ->state;

    zmemcpy(dest, ਸਰੋਤ, ਆਕਾਰ(z_ਧਾਰਾ));

    ds = (deflate_state *) ZALLOC(dest, 1, ਆਕਾਰ(deflate_state));
    ਜੇ (ds == Z_NULL) ਵਾਪਸ Z_MEM_ERROR;
    zmemzero(ds, ਆਕਾਰ(deflate_state));
    dest->state = (ਬਣਤਰ ਅੰਦਰੂਨੀ_state FAR *) ds;
    zmemcpy(ds, ss, ਆਕਾਰ(deflate_state));
    ds->strm = dest;

    ds->ਖਿੜਕੀ = (Bytef *) ZALLOC(dest, ds->w_ਮਾਪ, 2*ਆਕਾਰ(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_ਮਾਪ, ਆਕਾਰ(Pos));
    ds->ਸਿਰ   = (Posf *)  ZALLOC(dest, ds->hash_ਮਾਪ, ਆਕਾਰ(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    ਜੇ (ds->ਖਿੜਕੀ == Z_NULL || ds->prev == Z_NULL || ds->ਸਿਰ == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_ਸਿਰਾ (dest);
        ਵਾਪਸ Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->ਖਿੜਕੀ, ss->ਖਿੜਕੀ, ss->ਉੱਚਾ_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->ਪਾਓ > ds->w_ਮਾਪ ? ds->w_ਮਾਪ :
                ss->strstart - ss->ਪਾਓ) * ਆਕਾਰ(Pos));
    zmemcpy(ds->ਸਿਰ, ss->ਸਿਰ, ds->hash_ਮਾਪ * ਆਕਾਰ(Pos));

    ds->pending_ਬਾਹਰਲਾ = ds->pending_buf + (ss->pending_ਬਾਹਰਲਾ - ss->pending_buf);
    zmemcpy(ds->pending_ਬਾਹਰਲਾ, ss->pending_ਬਾਹਰਲਾ, ss->pending);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_ਅਗਲਾ * ਆਕਾਰ(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_ਅਗਲਾ);
#ਨਹੀਂਤਾਂ
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_ਅਗਲਾ);
#ਜੇ_ਅੰਤ

    ds->l_desc.dyn_ਰੁੱਖ = ds->dyn_ltree;
    ds->d_desc.dyn_ਰੁੱਖ = ds->dyn_dtree;
    ds->bl_desc.dyn_ਰੁੱਖ = ds->bl_ਰੁੱਖ;

    ਵਾਪਸ Z_OK;
#ਜੇ_ਅੰਤ /* MAXSEG_64K */
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_ਮੇਲ(deflate_state *s, IPos cur_ਮੇਲ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ chain_ਲੰਮਾਈ = s->max_chain_ਲੰਮਾਈ;/* max hash chain length */
    Bytef *ਜਾਂਚ = s->ਖਿੜਕੀ + s->strstart;      /* current string */
    Bytef *ਮੇਲ;                               /* matched string */
    ਪੂਰਨ len;                                    /* length of current match */
    ਪੂਰਨ ਉੱਤਮ_len = (ਪੂਰਨ)s->prev_ਲੰਮਾਈ;         /* best match length so far */
    ਪੂਰਨ nice_ਮੇਲ = s->nice_ਮੇਲ;             /* stop if match long enough */
    IPos ਹੱਦਬੰਦੀ = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_ਨਕਾਬ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->ਖਿੜਕੀ + s->strstart + MAX_ਮੇਲ - 1;
    ush ਜਾਂਚ_ਸ਼ੁਰੂ = *(ushf*)ਜਾਂਚ;
    ush ਜਾਂਚ_ਸਿਰਾ   = *(ushf*)(ਜਾਂਚ + ਉੱਤਮ_len - 1);
#ਨਹੀਂਤਾਂ
    Bytef *strend = s->ਖਿੜਕੀ + s->strstart + MAX_ਮੇਲ;
    Byte ਜਾਂਚ_end1  = ਜਾਂਚ[ਉੱਤਮ_len - 1];
    Byte ਜਾਂਚ_ਸਿਰਾ   = ਜਾਂਚ[ਉੱਤਮ_len];
#ਜੇ_ਅੰਤ

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_ਬਿੱਟ >= 8 && MAX_ਮੇਲ == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    ਜੇ (s->prev_ਲੰਮਾਈ >= s->good_ਮੇਲ) {
        chain_ਲੰਮਾਈ >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    ਜੇ ((uInt)nice_ਮੇਲ > s->lookahead) nice_ਮੇਲ = (ਪੂਰਨ)s->lookahead;

    Assert((ulg)s->strstart <= s->ਖਿੜਕੀ_ਮਾਪ - MIN_LOOKAHEAD,
           "need lookahead");

    ਕਰੋ {
        Assert(cur_ਮੇਲ < s->strstart, "no future");
        ਮੇਲ = s->ਖਿੜਕੀ + cur_ਮੇਲ;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#ਜੇ (defined(UNALIGNED_OK) && MAX_ਮੇਲ == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        ਜੇ (*(ushf*)(ਮੇਲ + ਉੱਤਮ_len - 1) != ਜਾਂਚ_ਸਿਰਾ ||
            *(ushf*)ਮੇਲ != ਜਾਂਚ_ਸ਼ੁਰੂ) ਜਾਰੀ;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(ਜਾਂਚ[2] == ਮੇਲ[2], "scan[2]?");
        ਜਾਂਚ++, ਮੇਲ++;
        ਕਰੋ {
        } ਜਦੋਂ (*(ushf*)(ਜਾਂਚ += 2) == *(ushf*)(ਮੇਲ += 2) &&
                 *(ushf*)(ਜਾਂਚ += 2) == *(ushf*)(ਮੇਲ += 2) &&
                 *(ushf*)(ਜਾਂਚ += 2) == *(ushf*)(ਮੇਲ += 2) &&
                 *(ushf*)(ਜਾਂਚ += 2) == *(ushf*)(ਮੇਲ += 2) &&
                 ਜਾਂਚ < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(ਜਾਂਚ <= s->ਖਿੜਕੀ + (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->ਖਿੜਕੀ_ਮਾਪ - 1),
               "wild scan");
        ਜੇ (*ਜਾਂਚ == *ਮੇਲ) ਜਾਂਚ++;

        len = (MAX_ਮੇਲ - 1) - (ਪੂਰਨ)(strend - ਜਾਂਚ);
        ਜਾਂਚ = strend - (MAX_ਮੇਲ-1);

#ਨਹੀਂਤਾਂ /* UNALIGNED_OK */

        ਜੇ (ਮੇਲ[ਉੱਤਮ_len]     != ਜਾਂਚ_ਸਿਰਾ  ||
            ਮੇਲ[ਉੱਤਮ_len - 1] != ਜਾਂਚ_end1 ||
            *ਮੇਲ              != *ਜਾਂਚ     ||
            *++ਮੇਲ            != ਜਾਂਚ[1])      ਜਾਰੀ;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        ਜਾਂਚ += 2, ਮੇਲ++;
        Assert(*ਜਾਂਚ == *ਮੇਲ, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        ਕਰੋ {
        } ਜਦੋਂ (*++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
                 *++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
                 *++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
                 *++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
                 ਜਾਂਚ < strend);

        Assert(ਜਾਂਚ <= s->ਖਿੜਕੀ + (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->ਖਿੜਕੀ_ਮਾਪ - 1),
               "wild scan");

        len = MAX_ਮੇਲ - (ਪੂਰਨ)(strend - ਜਾਂਚ);
        ਜਾਂਚ = strend - MAX_ਮੇਲ;

#ਜੇ_ਅੰਤ /* UNALIGNED_OK */

        ਜੇ (len > ਉੱਤਮ_len) {
            s->ਮੇਲ_ਸ਼ੁਰੂ = cur_ਮੇਲ;
            ਉੱਤਮ_len = len;
            ਜੇ (len >= nice_ਮੇਲ) ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ UNALIGNED_OK
            ਜਾਂਚ_ਸਿਰਾ = *(ushf*)(ਜਾਂਚ + ਉੱਤਮ_len - 1);
#ਨਹੀਂਤਾਂ
            ਜਾਂਚ_end1  = ਜਾਂਚ[ਉੱਤਮ_len - 1];
            ਜਾਂਚ_ਸਿਰਾ   = ਜਾਂਚ[ਉੱਤਮ_len];
#ਜੇ_ਅੰਤ
        }
    } ਜਦੋਂ ((cur_ਮੇਲ = prev[cur_ਮੇਲ & wmask]) > ਹੱਦਬੰਦੀ
             && --chain_ਲੰਮਾਈ != 0);

    ਜੇ ((uInt)ਉੱਤਮ_len <= s->lookahead) ਵਾਪਸ (uInt)ਉੱਤਮ_len;
    ਵਾਪਸ s->lookahead;
}

#ਨਹੀਂਤਾਂ /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_ਮੇਲ(deflate_state *s, IPos cur_ਮੇਲ) {
    Bytef *ਜਾਂਚ = s->ਖਿੜਕੀ + s->strstart;      /* current string */
    Bytef *ਮੇਲ;                               /* matched string */
    ਪੂਰਨ len;                                    /* length of current match */
    Bytef *strend = s->ਖਿੜਕੀ + s->strstart + MAX_ਮੇਲ;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_ਬਿੱਟ >= 8 && MAX_ਮੇਲ == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->ਖਿੜਕੀ_ਮਾਪ - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_ਮੇਲ < s->strstart, "no future");

    ਮੇਲ = s->ਖਿੜਕੀ + cur_ਮੇਲ;

    /* Return failure if the match length is less than 2:
     */
    ਜੇ (ਮੇਲ[0] != ਜਾਂਚ[0] || ਮੇਲ[1] != ਜਾਂਚ[1]) ਵਾਪਸ MIN_ਮੇਲ-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    ਜਾਂਚ += 2, ਮੇਲ += 2;
    Assert(*ਜਾਂਚ == *ਮੇਲ, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    ਕਰੋ {
    } ਜਦੋਂ (*++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
             *++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
             *++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
             *++ਜਾਂਚ == *++ਮੇਲ && *++ਜਾਂਚ == *++ਮੇਲ &&
             ਜਾਂਚ < strend);

    Assert(ਜਾਂਚ <= s->ਖਿੜਕੀ + (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->ਖਿੜਕੀ_ਮਾਪ - 1), "wild scan");

    len = MAX_ਮੇਲ - (ਪੂਰਨ)(strend - ਜਾਂਚ);

    ਜੇ (len < MIN_ਮੇਲ) ਵਾਪਸ MIN_ਮੇਲ - 1;

    s->ਮੇਲ_ਸ਼ੁਰੂ = cur_ਮੇਲ;
    ਵਾਪਸ (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#ਜੇ_ਅੰਤ /* FASTEST */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local ਖਾਲੀ check_ਮੇਲ(deflate_state *s, IPos ਸ਼ੁਰੂ, IPos ਮੇਲ, ਪੂਰਨ ਲੰਮਾਈ) {
    /* check that the match is indeed a match */
    Bytef *ਪਿੱਠ = s->ਖਿੜਕੀ + (ਪੂਰਨ)ਮੇਲ, *ਇੱਥੇ = s->ਖਿੜਕੀ + ਸ਼ੁਰੂ;
    IPos len = (IPos)ਲੰਮਾਈ;
    ਜੇ (ਮੇਲ == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        ਪਿੱਠ++;
        ਇੱਥੇ++;
        len--;
    }
    ਜੇ (zmemcmp(ਪਿੱਠ, ਇੱਥੇ, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                ਸ਼ੁਰੂ, (ਪੂਰਨ)ਮੇਲ, ਲੰਮਾਈ);
        ਕਰੋ {
            fprintf(stderr, "(%02x %02x)", *ਪਿੱਠ++, *ਇੱਥੇ++);
        } ਜਦੋਂ (--len != 0);
        z_error("invalid match");
    }
    ਜੇ (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", ਸ਼ੁਰੂ - ਮੇਲ, ਲੰਮਾਈ);
        ਕਰੋ { putc(s->ਖਿੜਕੀ[ਸ਼ੁਰੂ++], stderr); } ਜਦੋਂ (--ਲੰਮਾਈ != 0);
    }
}
#ਨਹੀਂਤਾਂ
#  define check_ਮੇਲ(s, ਸ਼ੁਰੂ, ਮੇਲ, ਲੰਮਾਈ)
#ਜੇ_ਅੰਤ /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLUSH_ਖੰਡ_ONLY(s, ਆਖਰੀ) { \
   tr_flush_ਖੰਡ(s, (s->ਖੰਡ_ਸ਼ੁਰੂ >= 0L ? \
                   (charf *)&s->ਖਿੜਕੀ[(ਬਿਨਾਨਿਸ਼ਾਨੀ)s->ਖੰਡ_ਸ਼ੁਰੂ] : \
                   (charf *)Z_NULL), \
                (ulg)((ਲੰਮਾ)s->strstart - s->ਖੰਡ_ਸ਼ੁਰੂ), \
                (ਆਖਰੀ)); \
   s->ਖੰਡ_ਸ਼ੁਰੂ = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLUSH_ਖੰਡ(s, ਆਖਰੀ) { \
   FLUSH_ਖੰਡ_ONLY(s, ਆਖਰੀ); \
   ਜੇ (s->strm->avail_ਬਾਹਰਲਾ == 0) ਵਾਪਸ (ਆਖਰੀ) ? ਅੰਤ_started : need_ਹੋਰ; \
}

/* Maximum stored block length in deflate format (not including header). */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_STORED 65535

/* Minimum of a and b. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN(a, b) ((a) > (b) ? (b) : (a))

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
local ਖੰਡ_state deflate_stored(deflate_state *s, ਪੂਰਨ flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    ਬਿਨਾਨਿਸ਼ਾਨੀ min_ਖੰਡ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(MIN(s->pending_buf_ਮਾਪ - 5, s->w_ਮਾਪ));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    ਪੂਰਨ ਆਖਰੀ = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ len, ਖੱਬਾ, have;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਰਤਿਆ = s->strm->avail_ਵਿੱਚ;
    ਕਰੋ {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((ਬਿਨਾਨਿਸ਼ਾਨੀ)s->bi_valid + 42) >> 3;   /* bytes in header */
        ਜੇ (s->strm->avail_ਬਾਹਰਲਾ < have)          /* need room for header */
            ਰੋਕੋ;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_ਬਾਹਰਲਾ - have;
        ਖੱਬਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->strstart - s->ਖੰਡ_ਸ਼ੁਰੂ);    /* window bytes */
        ਜੇ (len > (ulg)ਖੱਬਾ + s->strm->avail_ਵਿੱਚ)
            len = ਖੱਬਾ + s->strm->avail_ਵਿੱਚ;     /* limit len to the input */
        ਜੇ (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        ਜੇ (len < min_ਖੰਡ && ((len == 0 && flush != Z_ਅੰਤ) ||
                                flush == Z_NO_FLUSH ||
                                len != ਖੱਬਾ + s->strm->avail_ਵਿੱਚ))
            ਰੋਕੋ;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        ਆਖਰੀ = flush == Z_ਅੰਤ && len == ਖੱਬਾ + s->strm->avail_ਵਿੱਚ ? 1 : 0;
        tr_stored_ਖੰਡ(s, (ਅੱਖਰ *)0, 0L, ਆਖਰੀ);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->ਬਿੱਟ_ਭੇਜਿਆ += len << 3;
#ਜੇ_ਅੰਤ

        /* Copy uncompressed bytes from the window to next_out. */
        ਜੇ (ਖੱਬਾ) {
            ਜੇ (ਖੱਬਾ > len)
                ਖੱਬਾ = len;
            zmemcpy(s->strm->ਅਗਲਾ_ਬਾਹਰਲਾ, s->ਖਿੜਕੀ + s->ਖੰਡ_ਸ਼ੁਰੂ, ਖੱਬਾ);
            s->strm->ਅਗਲਾ_ਬਾਹਰਲਾ += ਖੱਬਾ;
            s->strm->avail_ਬਾਹਰਲਾ -= ਖੱਬਾ;
            s->strm->ਕੁੱਲ_ਬਾਹਰਲਾ += ਖੱਬਾ;
            s->ਖੰਡ_ਸ਼ੁਰੂ += ਖੱਬਾ;
            len -= ਖੱਬਾ;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        ਜੇ (len) {
            read_buf(s->strm, s->strm->ਅਗਲਾ_ਬਾਹਰਲਾ, len);
            s->strm->ਅਗਲਾ_ਬਾਹਰਲਾ += len;
            s->strm->avail_ਬਾਹਰਲਾ -= len;
            s->strm->ਕੁੱਲ_ਬਾਹਰਲਾ += len;
        }
    } ਜਦੋਂ (ਆਖਰੀ == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    ਵਰਤਿਆ -= s->strm->avail_ਵਿੱਚ;      /* number of input bytes directly copied */
    ਜੇ (ਵਰਤਿਆ) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        ਜੇ (ਵਰਤਿਆ >= s->w_ਮਾਪ) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->ਖਿੜਕੀ, s->strm->ਅਗਲਾ_ਵਿੱਚ - s->w_ਮਾਪ, s->w_ਮਾਪ);
            s->strstart = s->w_ਮਾਪ;
            s->ਪਾਓ = s->strstart;
        }
        ਨਹੀਂਤਾਂ {
            ਜੇ (s->ਖਿੜਕੀ_ਮਾਪ - s->strstart <= ਵਰਤਿਆ) {
                /* Slide the window down. */
                s->strstart -= s->w_ਮਾਪ;
                zmemcpy(s->ਖਿੜਕੀ, s->ਖਿੜਕੀ + s->w_ਮਾਪ, s->strstart);
                ਜੇ (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                ਜੇ (s->ਪਾਓ > s->strstart)
                    s->ਪਾਓ = s->strstart;
            }
            zmemcpy(s->ਖਿੜਕੀ + s->strstart, s->strm->ਅਗਲਾ_ਵਿੱਚ - ਵਰਤਿਆ, ਵਰਤਿਆ);
            s->strstart += ਵਰਤਿਆ;
            s->ਪਾਓ += MIN(ਵਰਤਿਆ, s->w_ਮਾਪ - s->ਪਾਓ);
        }
        s->ਖੰਡ_ਸ਼ੁਰੂ = s->strstart;
    }
    ਜੇ (s->ਉੱਚਾ_water < s->strstart)
        s->ਉੱਚਾ_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    ਜੇ (ਆਖਰੀ) {
        s->bi_ਵਰਤਿਆ = 8;
        ਵਾਪਸ ਅੰਤ_ਪੂਰਾ;
    }

    /* If flushing and all input has been consumed, then done. */
    ਜੇ (flush != Z_NO_FLUSH && flush != Z_ਅੰਤ &&
        s->strm->avail_ਵਿੱਚ == 0 && (ਲੰਮਾ)s->strstart == s->ਖੰਡ_ਸ਼ੁਰੂ)
        ਵਾਪਸ ਖੰਡ_ਪੂਰਾ;

    /* Fill the window with any remaining input. */
    have = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->ਖਿੜਕੀ_ਮਾਪ - s->strstart);
    ਜੇ (s->strm->avail_ਵਿੱਚ > have && s->ਖੰਡ_ਸ਼ੁਰੂ >= (ਲੰਮਾ)s->w_ਮਾਪ) {
        /* Slide the window down. */
        s->ਖੰਡ_ਸ਼ੁਰੂ -= s->w_ਮਾਪ;
        s->strstart -= s->w_ਮਾਪ;
        zmemcpy(s->ਖਿੜਕੀ, s->ਖਿੜਕੀ + s->w_ਮਾਪ, s->strstart);
        ਜੇ (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_ਮਾਪ;          /* more space now */
        ਜੇ (s->ਪਾਓ > s->strstart)
            s->ਪਾਓ = s->strstart;
    }
    ਜੇ (have > s->strm->avail_ਵਿੱਚ)
        have = s->strm->avail_ਵਿੱਚ;
    ਜੇ (have) {
        read_buf(s->strm, s->ਖਿੜਕੀ + s->strstart, have);
        s->strstart += have;
        s->ਪਾਓ += MIN(have, s->w_ਮਾਪ - s->ਪਾਓ);
    }
    ਜੇ (s->ਉੱਚਾ_water < s->strstart)
        s->ਉੱਚਾ_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((ਬਿਨਾਨਿਸ਼ਾਨੀ)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (ਬਿਨਾਨਿਸ਼ਾਨੀ)MIN(s->pending_buf_ਮਾਪ - have, MAX_STORED);
    min_ਖੰਡ = MIN(have, s->w_ਮਾਪ);
    ਖੱਬਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(s->strstart - s->ਖੰਡ_ਸ਼ੁਰੂ);
    ਜੇ (ਖੱਬਾ >= min_ਖੰਡ ||
        ((ਖੱਬਾ || flush == Z_ਅੰਤ) && flush != Z_NO_FLUSH &&
         s->strm->avail_ਵਿੱਚ == 0 && ਖੱਬਾ <= have)) {
        len = MIN(ਖੱਬਾ, have);
        ਆਖਰੀ = flush == Z_ਅੰਤ && s->strm->avail_ਵਿੱਚ == 0 &&
               len == ਖੱਬਾ ? 1 : 0;
        tr_stored_ਖੰਡ(s, (charf *)s->ਖਿੜਕੀ + s->ਖੰਡ_ਸ਼ੁਰੂ, len, ਆਖਰੀ);
        s->ਖੰਡ_ਸ਼ੁਰੂ += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    ਜੇ (ਆਖਰੀ)
        s->bi_ਵਰਤਿਆ = 8;
    ਵਾਪਸ ਆਖਰੀ ? ਅੰਤ_started : need_ਹੋਰ;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local ਖੰਡ_state deflate_fast(deflate_state *s, ਪੂਰਨ flush) {
    IPos hash_ਸਿਰ;       /* head of the hash chain */
    ਪੂਰਨ bflush;           /* set if current block must be flushed */

    ਲਈ (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        ਜੇ (s->lookahead < MIN_LOOKAHEAD) {
            fill_ਖਿੜਕੀ(s);
            ਜੇ (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                ਵਾਪਸ need_ਹੋਰ;
            }
            ਜੇ (s->lookahead == 0) ਰੋਕੋ; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_ਸਿਰ = NIL;
        ਜੇ (s->lookahead >= MIN_ਮੇਲ) {
            ਪਾਓ_STRING(s, s->strstart, hash_ਸਿਰ);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        ਜੇ (hash_ਸਿਰ != NIL && s->strstart - hash_ਸਿਰ <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->ਮੇਲ_ਲੰਮਾਈ = longest_ਮੇਲ (s, hash_ਸਿਰ);
            /* longest_match() sets match_start */
        }
        ਜੇ (s->ਮੇਲ_ਲੰਮਾਈ >= MIN_ਮੇਲ) {
            check_ਮੇਲ(s, s->strstart, s->ਮੇਲ_ਸ਼ੁਰੂ, (ਪੂਰਨ)s->ਮੇਲ_ਲੰਮਾਈ);

            _tr_tally_dist(s, s->strstart - s->ਮੇਲ_ਸ਼ੁਰੂ,
                           s->ਮੇਲ_ਲੰਮਾਈ - MIN_ਮੇਲ, bflush);

            s->lookahead -= s->ਮੇਲ_ਲੰਮਾਈ;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FASTEST
            ਜੇ (s->ਮੇਲ_ਲੰਮਾਈ <= s->max_ਪਾਓ_ਲੰਮਾਈ &&
                s->lookahead >= MIN_ਮੇਲ) {
                s->ਮੇਲ_ਲੰਮਾਈ--; /* string at strstart already in table */
                ਕਰੋ {
                    s->strstart++;
                    ਪਾਓ_STRING(s, s->strstart, hash_ਸਿਰ);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } ਜਦੋਂ (--s->ਮੇਲ_ਲੰਮਾਈ != 0);
                s->strstart++;
            } ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
            {
                s->strstart += s->ਮੇਲ_ਲੰਮਾਈ;
                s->ਮੇਲ_ਲੰਮਾਈ = 0;
                s->ins_h = s->ਖਿੜਕੀ[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->ਖਿੜਕੀ[s->strstart + 1]);
#ਜੇ MIN_ਮੇਲ != 3
                Call UPDATE_HASH() MIN_ਮੇਲ-3 ਹੋਰ times
#ਜੇ_ਅੰਤ
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } ਨਹੀਂਤਾਂ {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->ਖਿੜਕੀ[s->strstart]));
            _tr_tally_lit(s, s->ਖਿੜਕੀ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        ਜੇ (bflush) FLUSH_ਖੰਡ(s, 0);
    }
    s->ਪਾਓ = s->strstart < MIN_ਮੇਲ-1 ? s->strstart : MIN_ਮੇਲ-1;
    ਜੇ (flush == Z_ਅੰਤ) {
        FLUSH_ਖੰਡ(s, 1);
        ਵਾਪਸ ਅੰਤ_ਪੂਰਾ;
    }
    ਜੇ (s->sym_ਅਗਲਾ)
        FLUSH_ਖੰਡ(s, 0);
    ਵਾਪਸ ਖੰਡ_ਪੂਰਾ;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local ਖੰਡ_state deflate_slow(deflate_state *s, ਪੂਰਨ flush) {
    IPos hash_ਸਿਰ;          /* head of hash chain */
    ਪੂਰਨ bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    ਲਈ (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        ਜੇ (s->lookahead < MIN_LOOKAHEAD) {
            fill_ਖਿੜਕੀ(s);
            ਜੇ (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                ਵਾਪਸ need_ਹੋਰ;
            }
            ਜੇ (s->lookahead == 0) ਰੋਕੋ; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_ਸਿਰ = NIL;
        ਜੇ (s->lookahead >= MIN_ਮੇਲ) {
            ਪਾਓ_STRING(s, s->strstart, hash_ਸਿਰ);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_ਲੰਮਾਈ = s->ਮੇਲ_ਲੰਮਾਈ, s->prev_ਮੇਲ = s->ਮੇਲ_ਸ਼ੁਰੂ;
        s->ਮੇਲ_ਲੰਮਾਈ = MIN_ਮੇਲ-1;

        ਜੇ (hash_ਸਿਰ != NIL && s->prev_ਲੰਮਾਈ < s->max_lazy_ਮੇਲ &&
            s->strstart - hash_ਸਿਰ <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->ਮੇਲ_ਲੰਮਾਈ = longest_ਮੇਲ (s, hash_ਸਿਰ);
            /* longest_match() sets match_start */

            ਜੇ (s->ਮੇਲ_ਲੰਮਾਈ <= 5 && (s->strategy == Z_FILTERED
#ਜੇ TOO_FAR <= 32767
                || (s->ਮੇਲ_ਲੰਮਾਈ == MIN_ਮੇਲ &&
                    s->strstart - s->ਮੇਲ_ਸ਼ੁਰੂ > TOO_FAR)
#ਜੇ_ਅੰਤ
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->ਮੇਲ_ਲੰਮਾਈ = MIN_ਮੇਲ-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        ਜੇ (s->prev_ਲੰਮਾਈ >= MIN_ਮੇਲ && s->ਮੇਲ_ਲੰਮਾਈ <= s->prev_ਲੰਮਾਈ) {
            uInt max_ਪਾਓ = s->strstart + s->lookahead - MIN_ਮੇਲ;
            /* Do not insert strings in hash table beyond this. */

            check_ਮੇਲ(s, s->strstart - 1, s->prev_ਮੇਲ, (ਪੂਰਨ)s->prev_ਲੰਮਾਈ);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_ਮੇਲ,
                           s->prev_ਲੰਮਾਈ - MIN_ਮੇਲ, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_ਲੰਮਾਈ - 1;
            s->prev_ਲੰਮਾਈ -= 2;
            ਕਰੋ {
                ਜੇ (++s->strstart <= max_ਪਾਓ) {
                    ਪਾਓ_STRING(s, s->strstart, hash_ਸਿਰ);
                }
            } ਜਦੋਂ (--s->prev_ਲੰਮਾਈ != 0);
            s->ਮੇਲ_ਉਪਲਬਧ = 0;
            s->ਮੇਲ_ਲੰਮਾਈ = MIN_ਮੇਲ-1;
            s->strstart++;

            ਜੇ (bflush) FLUSH_ਖੰਡ(s, 0);

        } ਨਹੀਂਤਾਂ ਜੇ (s->ਮੇਲ_ਉਪਲਬਧ) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->ਖਿੜਕੀ[s->strstart - 1]));
            _tr_tally_lit(s, s->ਖਿੜਕੀ[s->strstart - 1], bflush);
            ਜੇ (bflush) {
                FLUSH_ਖੰਡ_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            ਜੇ (s->strm->avail_ਬਾਹਰਲਾ == 0) ਵਾਪਸ need_ਹੋਰ;
        } ਨਹੀਂਤਾਂ {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->ਮੇਲ_ਉਪਲਬਧ = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    ਜੇ (s->ਮੇਲ_ਉਪਲਬਧ) {
        Tracevv((stderr,"%c", s->ਖਿੜਕੀ[s->strstart - 1]));
        _tr_tally_lit(s, s->ਖਿੜਕੀ[s->strstart - 1], bflush);
        s->ਮੇਲ_ਉਪਲਬਧ = 0;
    }
    s->ਪਾਓ = s->strstart < MIN_ਮੇਲ-1 ? s->strstart : MIN_ਮੇਲ-1;
    ਜੇ (flush == Z_ਅੰਤ) {
        FLUSH_ਖੰਡ(s, 1);
        ਵਾਪਸ ਅੰਤ_ਪੂਰਾ;
    }
    ਜੇ (s->sym_ਅਗਲਾ)
        FLUSH_ਖੰਡ(s, 0);
    ਵਾਪਸ ਖੰਡ_ਪੂਰਾ;
}
#ਜੇ_ਅੰਤ /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local ਖੰਡ_state deflate_rle(deflate_state *s, ਪੂਰਨ flush) {
    ਪੂਰਨ bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *ਜਾਂਚ, *strend;   /* scan goes up to strend for length of run */

    ਲਈ (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        ਜੇ (s->lookahead <= MAX_ਮੇਲ) {
            fill_ਖਿੜਕੀ(s);
            ਜੇ (s->lookahead <= MAX_ਮੇਲ && flush == Z_NO_FLUSH) {
                ਵਾਪਸ need_ਹੋਰ;
            }
            ਜੇ (s->lookahead == 0) ਰੋਕੋ; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->ਮੇਲ_ਲੰਮਾਈ = 0;
        ਜੇ (s->lookahead >= MIN_ਮੇਲ && s->strstart > 0) {
            ਜਾਂਚ = s->ਖਿੜਕੀ + s->strstart - 1;
            prev = *ਜਾਂਚ;
            ਜੇ (prev == *++ਜਾਂਚ && prev == *++ਜਾਂਚ && prev == *++ਜਾਂਚ) {
                strend = s->ਖਿੜਕੀ + s->strstart + MAX_ਮੇਲ;
                ਕਰੋ {
                } ਜਦੋਂ (prev == *++ਜਾਂਚ && prev == *++ਜਾਂਚ &&
                         prev == *++ਜਾਂਚ && prev == *++ਜਾਂਚ &&
                         prev == *++ਜਾਂਚ && prev == *++ਜਾਂਚ &&
                         prev == *++ਜਾਂਚ && prev == *++ਜਾਂਚ &&
                         ਜਾਂਚ < strend);
                s->ਮੇਲ_ਲੰਮਾਈ = MAX_ਮੇਲ - (uInt)(strend - ਜਾਂਚ);
                ਜੇ (s->ਮੇਲ_ਲੰਮਾਈ > s->lookahead)
                    s->ਮੇਲ_ਲੰਮਾਈ = s->lookahead;
            }
            Assert(ਜਾਂਚ <= s->ਖਿੜਕੀ + (uInt)(s->ਖਿੜਕੀ_ਮਾਪ - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        ਜੇ (s->ਮੇਲ_ਲੰਮਾਈ >= MIN_ਮੇਲ) {
            check_ਮੇਲ(s, s->strstart, s->strstart - 1, (ਪੂਰਨ)s->ਮੇਲ_ਲੰਮਾਈ);

            _tr_tally_dist(s, 1, s->ਮੇਲ_ਲੰਮਾਈ - MIN_ਮੇਲ, bflush);

            s->lookahead -= s->ਮੇਲ_ਲੰਮਾਈ;
            s->strstart += s->ਮੇਲ_ਲੰਮਾਈ;
            s->ਮੇਲ_ਲੰਮਾਈ = 0;
        } ਨਹੀਂਤਾਂ {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->ਖਿੜਕੀ[s->strstart]));
            _tr_tally_lit(s, s->ਖਿੜਕੀ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        ਜੇ (bflush) FLUSH_ਖੰਡ(s, 0);
    }
    s->ਪਾਓ = 0;
    ਜੇ (flush == Z_ਅੰਤ) {
        FLUSH_ਖੰਡ(s, 1);
        ਵਾਪਸ ਅੰਤ_ਪੂਰਾ;
    }
    ਜੇ (s->sym_ਅਗਲਾ)
        FLUSH_ਖੰਡ(s, 0);
    ਵਾਪਸ ਖੰਡ_ਪੂਰਾ;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local ਖੰਡ_state deflate_huff(deflate_state *s, ਪੂਰਨ flush) {
    ਪੂਰਨ bflush;             /* set if current block must be flushed */

    ਲਈ (;;) {
        /* Make sure that we have a literal to write. */
        ਜੇ (s->lookahead == 0) {
            fill_ਖਿੜਕੀ(s);
            ਜੇ (s->lookahead == 0) {
                ਜੇ (flush == Z_NO_FLUSH)
                    ਵਾਪਸ need_ਹੋਰ;
                ਰੋਕੋ;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->ਮੇਲ_ਲੰਮਾਈ = 0;
        Tracevv((stderr,"%c", s->ਖਿੜਕੀ[s->strstart]));
        _tr_tally_lit(s, s->ਖਿੜਕੀ[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        ਜੇ (bflush) FLUSH_ਖੰਡ(s, 0);
    }
    s->ਪਾਓ = 0;
    ਜੇ (flush == Z_ਅੰਤ) {
        FLUSH_ਖੰਡ(s, 1);
        ਵਾਪਸ ਅੰਤ_ਪੂਰਾ;
    }
    ਜੇ (s->sym_ਅਗਲਾ)
        FLUSH_ਖੰਡ(s, 0);
    ਵਾਪਸ ਖੰਡ_ਪੂਰਾ;
}
