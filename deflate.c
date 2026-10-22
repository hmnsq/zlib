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

#ಅನ್ತರ್ಭಾವ "deflate.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ deflate_copyright[] =
   " deflate 1.3.2.1 Copyright 1995-2026 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
    need_ಇನ್ನಷ್ಟು,      /* block not completed, need more input or more output */
    ಖಂಡ_ಮುಗಿದ,     /* block flush performed */
    ಮುಕ್ತಾಯ_started, /* finish started, need only more output at next deflate */
    ಮುಕ್ತಾಯ_ಮುಗಿದ     /* finish done, accept no more input or output */
} ಖಂಡ_state;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಖಂಡ_state (*compress_func)(deflate_state *s, ಪೂರ್ಣಾಂಕ flush);
/* Compression function. Returns the block state after the call. */

local ಖಂಡ_state deflate_stored(deflate_state *s, ಪೂರ್ಣಾಂಕ flush);
local ಖಂಡ_state deflate_fast(deflate_state *s, ಪೂರ್ಣಾಂಕ flush);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FASTEST
local ಖಂಡ_state deflate_slow(deflate_state *s, ಪೂರ್ಣಾಂಕ flush);
#ಆದರೆ_ಅಂತ್ಯ
local ಖಂಡ_state deflate_rle(deflate_state *s, ಪೂರ್ಣಾಂಕ flush);
local ಖಂಡ_state deflate_huff(deflate_state *s, ಪೂರ್ಣಾಂಕ flush);

/* ===========================================================================
 * Local data
 */

#ವ್ಯಾಖ್ಯಾನಿಸು NIL 0
/* Tail of hash chains */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TOO_FAR
#  define TOO_FAR 4096
#ಆದರೆ_ಅಂತ್ಯ
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ config_s {
   ush good_ಉದ್ದಳತೆ; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_ಉದ್ದಳತೆ; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FASTEST
local ಸ್ಥಿರಾಂಕ config configuration_ಕೋಷ್ಟಕ[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#ಇಲ್ಲದಿದ್ದರೆ
local ಸ್ಥಿರಾಂಕ config configuration_ಕೋಷ್ಟಕ[10] = {
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
#ಆದರೆ_ಅಂತ್ಯ

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#ವ್ಯಾಖ್ಯಾನಿಸು RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು UPDATE_HASH(s,h,c) (h = (((h) << s->hash_ಸರಿಸು) ^ (c)) & s->hash_ಮುಖವಾಡ)


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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FASTEST
#ವ್ಯಾಖ್ಯಾನಿಸು ಸೇರಿಸು_STRING(s, str, ಹೊಂದಿಕೆ_ತಲೆ) \
   (UPDATE_HASH(s, s->ins_h, s->ಕಿಟಕಿ[(str) + (MIN_ಹೊಂದಿಕೆ-1)]), \
    ಹೊಂದಿಕೆ_ತಲೆ = s->ತಲೆ[s->ins_h], \
    s->ತಲೆ[s->ins_h] = (Pos)(str))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ಸೇರಿಸು_STRING(s, str, ಹೊಂದಿಕೆ_ತಲೆ) \
   (UPDATE_HASH(s, s->ins_h, s->ಕಿಟಕಿ[(str) + (MIN_ಹೊಂದಿಕೆ-1)]), \
    ಹೊಂದಿಕೆ_ತಲೆ = s->prev[(str) & s->w_ಮುಖವಾಡ] = s->ತಲೆ[s->ins_h], \
    s->ತಲೆ[s->ins_h] = (Pos)(str))
#ಆದರೆ_ಅಂತ್ಯ

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು CLEAR_HASH(s) \
    ಮಾಡು { \
        s->ತಲೆ[s->hash_ಗಾತ್ರ - 1] = NIL; \
        zmemzero(s->ತಲೆ, (ಚಿಹ್ನೆರಹಿತ)(s->hash_ಗಾತ್ರ - 1)*ಗಾತ್ರ(*s->ತಲೆ)); \
        s->slid = 0; \
    } ಆಗ (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#ಆದರೆ defined(__has_feature)
#  ಆದರೆ __has_feature(ಸ್ಮರಣೆ_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#ಆದರೆ_ಅಂತ್ಯ
local ಶೂನ್ಯ slide_hash(deflate_state *s) {
    ಚಿಹ್ನೆರಹಿತ n, m;
    Posf *p;
    uInt wsize = s->w_ಗಾತ್ರ;

    n = s->hash_ಗಾತ್ರ;
    p = &s->ತಲೆ[n];
    ಮಾಡು {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } ಆಗ (--n);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FASTEST
    n = wsize;
    p = &s->prev[n];
    ಮಾಡು {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } ಆಗ (--n);
#ಆದರೆ_ಅಂತ್ಯ
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local ಚಿಹ್ನೆರಹಿತ read_buf(z_streamp strm, Bytef *buf, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2) {
    ಚಿಹ್ನೆರಹಿತ len = strm->avail_ಒಳ;

    ಆದರೆ (len > ಗಾತ್ರ_2) len = ಗಾತ್ರ_2;
    ಆದರೆ (len == 0) ಹಿಂತಿರುಗಿ 0;

    strm->avail_ಒಳ  -= len;

    zmemcpy(buf, strm->ಮುಂದಿನ_ಒಳ, len);
    ಆದರೆ (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strm->state->wrap == 2) {
        strm->adler = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(strm->adler, buf, len);
    }
#ಆದರೆ_ಅಂತ್ಯ
    strm->ಮುಂದಿನ_ಒಳ  += len;
    strm->ಒಟ್ಟು_ಒಳ += len;

    ಹಿಂತಿರುಗಿ len;
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
local ಶೂನ್ಯ fill_ಕಿಟಕಿ(deflate_state *s) {
    ಚಿಹ್ನೆರಹಿತ n;
    ಚಿಹ್ನೆರಹಿತ ಇನ್ನಷ್ಟು;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_ಗಾತ್ರ;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    ಮಾಡು {
        ಇನ್ನಷ್ಟು = (ಚಿಹ್ನೆರಹಿತ)(s->ಕಿಟಕಿ_ಗಾತ್ರ -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ಸೂಚನೆ warning(ತಳ್ಳು)
#ಸೂಚನೆ warning(disable: 4127)
#ಆದರೆ_ಅಂತ್ಯ
        ಆದರೆ (ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) <= 2) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ಸೂಚನೆ warning(ತೆಗೆ)
#ಆದರೆ_ಅಂತ್ಯ
            ಆದರೆ (ಇನ್ನಷ್ಟು == 0 && s->strstart == 0 && s->lookahead == 0) {
                ಇನ್ನಷ್ಟು = wsize;

            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಇನ್ನಷ್ಟು == (ಚಿಹ್ನೆರಹಿತ)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                ಇನ್ನಷ್ಟು--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        ಆದರೆ (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->ಕಿಟಕಿ, s->ಕಿಟಕಿ + wsize, (ಚಿಹ್ನೆರಹಿತ)wsize - ಇನ್ನಷ್ಟು);
            s->ಹೊಂದಿಕೆ_ಆರಂಭ -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->ಖಂಡ_ಆರಂಭ -= (ದೀರ್ಘ) wsize;
            ಆದರೆ (s->ಸೇರಿಸು > s->strstart)
                s->ಸೇರಿಸು = s->strstart;
            slide_hash(s);
            ಇನ್ನಷ್ಟು += wsize;
        }
        ಆದರೆ (s->strm->avail_ಒಳ == 0) ನಿಲ್ಲಿಸು;

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
        Assert(ಇನ್ನಷ್ಟು >= 2, "more < 2");

        n = read_buf(s->strm, s->ಕಿಟಕಿ + s->strstart + s->lookahead, ಇನ್ನಷ್ಟು);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        ಆದರೆ (s->lookahead + s->ಸೇರಿಸು >= MIN_ಹೊಂದಿಕೆ) {
            uInt str = s->strstart - s->ಸೇರಿಸು;
            s->ins_h = s->ಕಿಟಕಿ[str];
            UPDATE_HASH(s, s->ins_h, s->ಕಿಟಕಿ[str + 1]);
#ಆದರೆ MIN_ಹೊಂದಿಕೆ != 3
            Call UPDATE_HASH() MIN_ಹೊಂದಿಕೆ-3 ಇನ್ನಷ್ಟು times
#ಆದರೆ_ಅಂತ್ಯ
            ಆಗ (s->ಸೇರಿಸು) {
                UPDATE_HASH(s, s->ins_h, s->ಕಿಟಕಿ[str + MIN_ಹೊಂದಿಕೆ-1]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FASTEST
                s->prev[str & s->w_ಮುಖವಾಡ] = s->ತಲೆ[s->ins_h];
#ಆದರೆ_ಅಂತ್ಯ
                s->ತಲೆ[s->ins_h] = (Pos)str;
                str++;
                s->ಸೇರಿಸು--;
                ಆದರೆ (s->lookahead + s->ಸೇರಿಸು < MIN_ಹೊಂದಿಕೆ)
                    ನಿಲ್ಲಿಸು;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } ಆಗ (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_ಒಳ != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    ಆದರೆ (s->ಮೇಲಿನ_water < s->ಕಿಟಕಿ_ಗಾತ್ರ) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        ಆದರೆ (s->ಮೇಲಿನ_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->ಕಿಟಕಿ_ಗಾತ್ರ - curr;
            ಆದರೆ (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->ಕಿಟಕಿ + curr, (ಚಿಹ್ನೆರಹಿತ)init);
            s->ಮೇಲಿನ_water = curr + init;
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s->ಮೇಲಿನ_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->ಮೇಲಿನ_water;
            ಆದರೆ (init > s->ಕಿಟಕಿ_ಗಾತ್ರ - s->ಮೇಲಿನ_water)
                init = s->ಕಿಟಕಿ_ಗಾತ್ರ - s->ಮೇಲಿನ_water;
            zmemzero(s->ಕಿಟಕಿ + s->ಮೇಲಿನ_water, (ಚಿಹ್ನೆರಹಿತ)init);
            s->ಮೇಲಿನ_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->ಕಿಟಕಿ_ಗಾತ್ರ - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateInit_(z_streamp strm, ಪೂರ್ಣಾಂಕ level, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ,
                         ಪೂರ್ಣಾಂಕ ಹರಿವು_ಗಾತ್ರ) {
    ಹಿಂತಿರುಗಿ deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, ಆವೃತ್ತಿ, ಹರಿವು_ಗಾತ್ರ);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateInit2_(z_streamp strm, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                          ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು, ಪೂರ್ಣಾಂಕ memLevel, ಪೂರ್ಣಾಂಕ strategy,
                          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ, ಪೂರ್ಣಾಂಕ ಹರಿವು_ಗಾತ್ರ) {
    deflate_state *s;
    ಪೂರ್ಣಾಂಕ wrap = 1;
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ my_ಆವೃತ್ತಿ[] = ZLIB_ಆವೃತ್ತಿ;

    ಆದರೆ (ಆವೃತ್ತಿ == Z_NULL || ಆವೃತ್ತಿ[0] != my_ಆವೃತ್ತಿ[0] ||
        ಹರಿವು_ಗಾತ್ರ != ಗಾತ್ರ(z_ಹರಿವು)) {
        ಹಿಂತಿರುಗಿ Z_ಆವೃತ್ತಿ_ERROR;
    }
    ಆದರೆ (strm == Z_NULL) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    strm->ಸಂದೇಶ = Z_NULL;
    ಆದರೆ (strm->zalloc == (alloc_func)0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಆದರೆ (strm->zfree == (free_func)0)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
        strm->zfree = zcfree;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FASTEST
    ಆದರೆ (level != 0) level = 1;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (level == Z_DEFAULT_COMPRESSION) level = 6;
#ಆದರೆ_ಅಂತ್ಯ

    ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < 0) { /* suppress zlib wrapper */
        wrap = 0;
        ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < -15)
            ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
        ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು = -ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು -= 16;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ (memLevel < 1 || memLevel > MAX_MEM_LEVEL || ವಿಧಾನ_2 != Z_DEFLATED ||
        ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು < 8 || ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು == 8 && wrap != 1)) {
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    }
    ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು == 8) ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, ಗಾತ್ರ(deflate_state));
    ಆದರೆ (s == Z_NULL) ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    zmemzero(s, ಗಾತ್ರ(deflate_state));
    strm->state = (ರಚನೆ ಆಂತರಿಕ_state FAR *)s;
    s->strm = strm;
    s->ಸ್ಥಿತಿ_2 = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_ಬಿಟ್ಟುಗಳು = (uInt)ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು;
    s->w_ಗಾತ್ರ = 1 << s->w_ಬಿಟ್ಟುಗಳು;
    s->w_ಮುಖವಾಡ = s->w_ಗಾತ್ರ - 1;

    s->hash_ಬಿಟ್ಟುಗಳು = (uInt)memLevel + 7;
    s->hash_ಗಾತ್ರ = 1 << s->hash_ಬಿಟ್ಟುಗಳು;
    s->hash_ಮುಖವಾಡ = s->hash_ಗಾತ್ರ - 1;
    s->hash_ಸರಿಸು =  ((s->hash_ಬಿಟ್ಟುಗಳು + MIN_ಹೊಂದಿಕೆ-1) / MIN_ಹೊಂದಿಕೆ);

    s->ಕಿಟಕಿ = (Bytef *) ZALLOC(strm, s->w_ಗಾತ್ರ, 2*ಗಾತ್ರ(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_ಗಾತ್ರ, ಗಾತ್ರ(Pos));
    s->ತಲೆ   = (Posf *)  ZALLOC(strm, s->hash_ಗಾತ್ರ, ಗಾತ್ರ(Pos));

    s->ಮೇಲಿನ_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_ಗಾತ್ರ = (ulg)s->lit_bufsize * 4;

    ಆದರೆ (s->ಕಿಟಕಿ == Z_NULL || s->prev == Z_NULL || s->ತಲೆ == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->ಸ್ಥಿತಿ_2 = ಮುಕ್ತಾಯ_STATE;
        strm->ಸಂದೇಶ = ERR_ಸಂದೇಶ(Z_MEM_ERROR);
        deflate_ಕೊನೆ (strm);
        ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_ಕೊನೆ = s->lit_bufsize - 1;
#ಇಲ್ಲದಿದ್ದರೆ
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_ಕೊನೆ = (s->lit_bufsize - 1) * 3;
#ಆದರೆ_ಅಂತ್ಯ
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->ವಿಧಾನ_2 = (Byte)ವಿಧಾನ_2;

    ಹಿಂತಿರುಗಿ deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local ಪೂರ್ಣಾಂಕ deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    ಆದರೆ (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        ಹಿಂತಿರುಗಿ 1;
    s = strm->state;
    ಆದರೆ (s == Z_NULL || s->strm != strm || (s->ಸ್ಥಿತಿ_2 != INIT_STATE &&
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
                                           s->ಸ್ಥಿತಿ_2 != GZIP_STATE &&
#ಆದರೆ_ಅಂತ್ಯ
                                           s->ಸ್ಥಿತಿ_2 != EXTRA_STATE &&
                                           s->ಸ್ಥಿತಿ_2 != NAME_STATE &&
                                           s->ಸ್ಥಿತಿ_2 != COMMENT_STATE &&
                                           s->ಸ್ಥಿತಿ_2 != HCRC_STATE &&
                                           s->ಸ್ಥಿತಿ_2 != BUSY_STATE &&
                                           s->ಸ್ಥಿತಿ_2 != ಮುಕ್ತಾಯ_STATE))
        ಹಿಂತಿರುಗಿ 1;
    ಹಿಂತಿರುಗಿ 0;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflate_ಗಣ_Dictionary(z_streamp strm, ಸ್ಥಿರಾಂಕ Bytef *dictionary,
                                 uInt  dict_ಉದ್ದಳತೆ) {
    deflate_state *s;
    uInt str, n;
    ಪೂರ್ಣಾಂಕ wrap;
    ಚಿಹ್ನೆರಹಿತ avail;
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮುಂದಿನ;

    ಆದರೆ (deflateStateCheck(strm) || dictionary == Z_NULL)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    s = strm->state;
    wrap = s->wrap;
    ಆದರೆ (wrap == 2 || (wrap == 1 && s->ಸ್ಥಿತಿ_2 != INIT_STATE) || s->lookahead)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    ಆದರೆ (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_ಉದ್ದಳತೆ);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    ಆದರೆ (dict_ಉದ್ದಳತೆ >= s->w_ಗಾತ್ರ) {
        ಆದರೆ (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->ಖಂಡ_ಆರಂಭ = 0L;
            s->ಸೇರಿಸು = 0;
        }
        dictionary += dict_ಉದ್ದಳತೆ - s->w_ಗಾತ್ರ;  /* use the tail */
        dict_ಉದ್ದಳತೆ = s->w_ಗಾತ್ರ;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_ಒಳ;
    ಮುಂದಿನ = strm->ಮುಂದಿನ_ಒಳ;
    strm->avail_ಒಳ = dict_ಉದ್ದಳತೆ;
    strm->ಮುಂದಿನ_ಒಳ = (z_const Bytef *)dictionary;
    fill_ಕಿಟಕಿ(s);
    ಆಗ (s->lookahead >= MIN_ಹೊಂದಿಕೆ) {
        str = s->strstart;
        n = s->lookahead - (MIN_ಹೊಂದಿಕೆ-1);
        ಮಾಡು {
            UPDATE_HASH(s, s->ins_h, s->ಕಿಟಕಿ[str + MIN_ಹೊಂದಿಕೆ-1]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FASTEST
            s->prev[str & s->w_ಮುಖವಾಡ] = s->ತಲೆ[s->ins_h];
#ಆದರೆ_ಅಂತ್ಯ
            s->ತಲೆ[s->ins_h] = (Pos)str;
            str++;
        } ಆಗ (--n);
        s->strstart = str;
        s->lookahead = MIN_ಹೊಂದಿಕೆ-1;
        fill_ಕಿಟಕಿ(s);
    }
    s->strstart += s->lookahead;
    s->ಖಂಡ_ಆರಂಭ = (ದೀರ್ಘ)s->strstart;
    s->ಸೇರಿಸು = s->lookahead;
    s->lookahead = 0;
    s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = s->prev_ಉದ್ದಳತೆ = MIN_ಹೊಂದಿಕೆ-1;
    s->ಹೊಂದಿಕೆ_ಲಭ್ಯ = 0;
    strm->ಮುಂದಿನ_ಒಳ = ಮುಂದಿನ;
    strm->avail_ಒಳ = avail;
    s->wrap = wrap;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_ಉದ್ದಳತೆ) {
    deflate_state *s;
    uInt len;

    ಆದರೆ (deflateStateCheck(strm))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    ಆದರೆ (len > s->w_ಗಾತ್ರ)
        len = s->w_ಗಾತ್ರ;
    ಆದರೆ (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->ಕಿಟಕಿ + s->strstart + s->lookahead - len, len);
    ಆದರೆ (dict_ಉದ್ದಳತೆ != Z_NULL)
        *dict_ಉದ್ದಳತೆ = len;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    ಆದರೆ (deflateStateCheck(strm)) {
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    }

    strm->ಒಟ್ಟು_ಒಳ = strm->ಒಟ್ಟು_ಹೊರಹೋಗು = 0;
    strm->ಸಂದೇಶ = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->ದತ್ತಾಂಶ_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_ಹೊರಹೋಗು = s->pending_buf;

    ಆದರೆ (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->ಸ್ಥಿತಿ_2 =
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
        s->wrap == 2 ? GZIP_STATE :
#ಆದರೆ_ಅಂತ್ಯ
        INIT_STATE;
    strm->adler =
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
        s->wrap == 2 ? ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0) :
#ಆದರೆ_ಅಂತ್ಯ
        adler32(0L, Z_NULL, 0);
    s->ಕೊನೆಯ_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local ಶೂನ್ಯ lm_init(deflate_state *s) {
    s->ಕಿಟಕಿ_ಗಾತ್ರ = (ulg)2L*s->w_ಗಾತ್ರ;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_ಹೊಂದಿಕೆ   = configuration_ಕೋಷ್ಟಕ[s->level].max_lazy;
    s->good_ಹೊಂದಿಕೆ       = configuration_ಕೋಷ್ಟಕ[s->level].good_ಉದ್ದಳತೆ;
    s->nice_ಹೊಂದಿಕೆ       = configuration_ಕೋಷ್ಟಕ[s->level].nice_ಉದ್ದಳತೆ;
    s->max_chain_ಉದ್ದಳತೆ = configuration_ಕೋಷ್ಟಕ[s->level].max_chain;

    s->strstart = 0;
    s->ಖಂಡ_ಆರಂಭ = 0L;
    s->lookahead = 0;
    s->ಸೇರಿಸು = 0;
    s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = s->prev_ಉದ್ದಳತೆ = MIN_ಹೊಂದಿಕೆ-1;
    s->ಹೊಂದಿಕೆ_ಲಭ್ಯ = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateReset(z_streamp strm) {
    ಪೂರ್ಣಾಂಕ ret;

    ret = deflateResetKeep(strm);
    ಆದರೆ (ret == Z_OK)
        lm_init(strm->state);
    ಹಿಂತಿರುಗಿ ret;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflate_ಗಣ_ಶೀರ್ಷಿಕೆ(z_streamp strm, gz_headerp ತಲೆ) {
    ಆದರೆ (deflateStateCheck(strm) || strm->state->wrap != 2)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    strm->state->gzhead = ತಲೆ;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflatePending(z_streamp strm, ಚಿಹ್ನೆರಹಿತ *pending, ಪೂರ್ಣಾಂಕ *ಬಿಟ್ಟುಗಳು) {
    ಆದರೆ (deflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ಆದರೆ (ಬಿಟ್ಟುಗಳು != Z_NULL)
        *ಬಿಟ್ಟುಗಳು = strm->state->bi_valid;
    ಆದರೆ (pending != Z_NULL) {
        *pending = (ಚಿಹ್ನೆರಹಿತ)strm->state->pending;
        ಆದರೆ (*pending != strm->state->pending) {
            *pending = (ಚಿಹ್ನೆರಹಿತ)-1;
            ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
        }
    }
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflate_ಬಳಸಿದ(z_streamp strm, ಪೂರ್ಣಾಂಕ *ಬಿಟ್ಟುಗಳು) {
    ಆದರೆ (deflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ಆದರೆ (ಬಿಟ್ಟುಗಳು != Z_NULL)
        *ಬಿಟ್ಟುಗಳು = strm->state->bi_ಬಳಸಿದ;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflate_ಅವಿಭಾಜ್ಯ(z_streamp strm, ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ) {
    deflate_state *s;
    ಪೂರ್ಣಾಂಕ put;

    ಆದರೆ (deflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    s = strm->state;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
    ಆದರೆ (ಬಿಟ್ಟುಗಳು < 0 || ಬಿಟ್ಟುಗಳು > 16 ||
        (uchf *)s->d_buf < s->pending_ಹೊರಹೋಗು + ((Buf_ಗಾತ್ರ + 7) >> 3))
        ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (ಬಿಟ್ಟುಗಳು < 0 || ಬಿಟ್ಟುಗಳು > 16 ||
        s->sym_buf < s->pending_ಹೊರಹೋಗು + ((Buf_ಗಾತ್ರ + 7) >> 3))
        ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
#ಆದರೆ_ಅಂತ್ಯ
    ಮಾಡು {
        put = Buf_ಗಾತ್ರ - s->bi_valid;
        ಆದರೆ (put > ಬಿಟ್ಟುಗಳು)
            put = ಬಿಟ್ಟುಗಳು;
        s->bi_buf |= (ush)((ಮೌಲ್ಯ & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_ಬಿಟ್ಟುಗಳು(s);
        ಮೌಲ್ಯ >>= put;
        ಬಿಟ್ಟುಗಳು -= put;
    } ಆಗ (ಬಿಟ್ಟುಗಳು);
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateParams(z_streamp strm, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ strategy) {
    deflate_state *s;
    compress_func func;

    ಆದರೆ (deflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    s = strm->state;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FASTEST
    ಆದರೆ (level != 0) level = 1;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (level == Z_DEFAULT_COMPRESSION) level = 6;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    }
    func = configuration_ಕೋಷ್ಟಕ[s->level].func;

    ಆದರೆ ((strategy != s->strategy || func != configuration_ಕೋಷ್ಟಕ[level].func) &&
        s->ಕೊನೆಯ_flush != -2) {
        /* Flush the last buffer: */
        ಪೂರ್ಣಾಂಕ err = deflate(strm, Z_ಖಂಡ);
        ಆದರೆ (err == Z_ಹರಿವು_ERROR)
            ಹಿಂತಿರುಗಿ err;
        ಆದರೆ (strm->avail_ಒಳ || (s->strstart - s->ಖಂಡ_ಆರಂಭ) + s->lookahead)
            ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
    }
    ಆದರೆ (s->level != level) {
        ಆದರೆ (s->level == 0 && s->matches != 0) {
            ಆದರೆ (s->matches == 1)
                slide_hash(s);
            ಇಲ್ಲದಿದ್ದರೆ
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_ಹೊಂದಿಕೆ   = configuration_ಕೋಷ್ಟಕ[level].max_lazy;
        s->good_ಹೊಂದಿಕೆ       = configuration_ಕೋಷ್ಟಕ[level].good_ಉದ್ದಳತೆ;
        s->nice_ಹೊಂದಿಕೆ       = configuration_ಕೋಷ್ಟಕ[level].nice_ಉದ್ದಳತೆ;
        s->max_chain_ಉದ್ದಳತೆ = configuration_ಕೋಷ್ಟಕ[level].max_chain;
    }
    s->strategy = strategy;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateTune(z_streamp strm, ಪೂರ್ಣಾಂಕ good_ಉದ್ದಳತೆ, ಪೂರ್ಣಾಂಕ max_lazy,
                        ಪೂರ್ಣಾಂಕ nice_ಉದ್ದಳತೆ, ಪೂರ್ಣಾಂಕ max_chain) {
    deflate_state *s;

    ಆದರೆ (deflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    s = strm->state;
    s->good_ಹೊಂದಿಕೆ = (uInt)good_ಉದ್ದಳತೆ;
    s->max_lazy_ಹೊಂದಿಕೆ = (uInt)max_lazy;
    s->nice_ಹೊಂದಿಕೆ = nice_ಉದ್ದಳತೆ;
    s->max_chain_ಉದ್ದಳತೆ = (uInt)max_chain;
    ಹಿಂತಿರುಗಿ Z_OK;
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
z_ಗಾತ್ರ_t ZEXPORT deflateBound_z(z_streamp strm, z_ಗಾತ್ರ_t ಮೂಲ_Len) {
    deflate_state *s;
    z_ಗಾತ್ರ_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = ಮೂಲ_Len + (ಮೂಲ_Len >> 3) + (ಮೂಲ_Len >> 8) +
               (ಮೂಲ_Len >> 9) + 4;
    ಆದರೆ (fixedlen < ಮೂಲ_Len)
        fixedlen = (z_ಗಾತ್ರ_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = ಮೂಲ_Len + (ಮೂಲ_Len >> 5) + (ಮೂಲ_Len >> 7) +
               (ಮೂಲ_Len >> 11) + 7;
    ಆದರೆ (storelen < ಮೂಲ_Len)
        storelen = (z_ಗಾತ್ರ_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    ಆದರೆ (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        ಹಿಂತಿರುಗಿ bound + 18 < bound ? (z_ಗಾತ್ರ_t)-1 : bound + 18;
    }

    /* compute wrapper length */
    s = strm->state;
    ಆಯ್ಕೆ (s->wrap < 0 ? -s->wrap : s->wrap) {
    ಸ್ಥಿತಿ 0:                                 /* raw deflate */
        wraplen = 0;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
    ಸ್ಥಿತಿ 2:                                 /* gzip wrapper */
        wraplen = 18;
        ಆದರೆ (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            ಆದರೆ (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            ಆದರೆ (str != Z_NULL)
                ಮಾಡು {
                    wraplen++;
                } ಆಗ (*str++);
            str = s->gzhead->comment;
            ಆದರೆ (str != Z_NULL)
                ಮಾಡು {
                    wraplen++;
                } ಆಗ (*str++);
            ಆದರೆ (s->gzhead->hcrc)
                wraplen += 2;
        }
        ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    ಪೂರ್ವನಿಯೋಜಿತ:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    ಆದರೆ (s->w_ಬಿಟ್ಟುಗಳು != 15 || s->hash_ಬಿಟ್ಟುಗಳು != 8 + 7) {
        bound = s->w_ಬಿಟ್ಟುಗಳು <= s->hash_ಬಿಟ್ಟುಗಳು && s->level ? fixedlen :
                                                        storelen;
        ಹಿಂತಿರುಗಿ bound + wraplen < bound ? (z_ಗಾತ್ರ_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = ಮೂಲ_Len + (ಮೂಲ_Len >> 12) + (ಮೂಲ_Len >> 14) +
            (ಮೂಲ_Len >> 25) + 13 - 6 + wraplen;
    ಹಿಂತಿರುಗಿ bound < ಮೂಲ_Len ? (z_ಗಾತ್ರ_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong ಮೂಲ_Len) {
    z_ಗಾತ್ರ_t bound = deflateBound_z(strm, ಮೂಲ_Len);
    ಹಿಂತಿರುಗಿ (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local ಶೂನ್ಯ putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local ಶೂನ್ಯ flush_pending(z_streamp strm) {
    ಚಿಹ್ನೆರಹಿತ len;
    deflate_state *s = strm->state;

    tr_flush_ಬಿಟ್ಟುಗಳು(s);
    len = s->pending > strm->avail_ಹೊರಹೋಗು ? strm->avail_ಹೊರಹೋಗು :
                                         (ಚಿಹ್ನೆರಹಿತ)s->pending;
    ಆದರೆ (len == 0) ಹಿಂತಿರುಗಿ;

    zmemcpy(strm->ಮುಂದಿನ_ಹೊರಹೋಗು, s->pending_ಹೊರಹೋಗು, len);
    strm->ಮುಂದಿನ_ಹೊರಹೋಗು  += len;
    s->pending_ಹೊರಹೋಗು  += len;
    strm->ಒಟ್ಟು_ಹೊರಹೋಗು += len;
    strm->avail_ಹೊರಹೋಗು -= len;
    s->pending      -= len;
    ಆದರೆ (s->pending == 0) {
        s->pending_ಹೊರಹೋಗು = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#ವ್ಯಾಖ್ಯಾನಿಸು HCRC_UPDATE(beg) \
    ಮಾಡು { \
        ಆದರೆ (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } ಆಗ (0)

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflate(z_streamp strm, ಪೂರ್ಣಾಂಕ flush) {
    ಪೂರ್ಣಾಂಕ old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    ಆದರೆ (deflateStateCheck(strm) || flush > Z_ಖಂಡ || flush < 0) {
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    }
    s = strm->state;

    ಆದರೆ (strm->ಮುಂದಿನ_ಹೊರಹೋಗು == Z_NULL ||
        (strm->avail_ಒಳ != 0 && strm->ಮುಂದಿನ_ಒಳ == Z_NULL) ||
        (s->ಸ್ಥಿತಿ_2 == ಮುಕ್ತಾಯ_STATE && flush != Z_ಮುಕ್ತಾಯ)) {
        ERR_RETURN(strm, Z_ಹರಿವು_ERROR);
    }
    ಆದರೆ (strm->avail_ಹೊರಹೋಗು == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->ಕೊನೆಯ_flush;
    s->ಕೊನೆಯ_flush = flush;

    /* Flush as much pending output as possible */
    ಆದರೆ (s->pending != 0) {
        flush_pending(strm);
        ಆದರೆ (strm->avail_ಹೊರಹೋಗು == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->ಕೊನೆಯ_flush = -1;
            ಹಿಂತಿರುಗಿ Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strm->avail_ಒಳ == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_ಮುಕ್ತಾಯ) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == ಮುಕ್ತಾಯ_STATE && strm->avail_ಒಳ != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == INIT_STATE && s->wrap == 0)
        s->ಸ್ಥಿತಿ_2 = BUSY_STATE;
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == INIT_STATE) {
        /* zlib header */
        uInt ಶೀರ್ಷಿಕೆ = (Z_DEFLATED + ((s->w_ಬಿಟ್ಟುಗಳು - 8) << 4)) << 8;
        uInt level_flags;

        ಆದರೆ (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s->level < 6)
            level_flags = 1;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s->level == 6)
            level_flags = 2;
        ಇಲ್ಲದಿದ್ದರೆ
            level_flags = 3;
        ಶೀರ್ಷಿಕೆ |= (level_flags << 6);
        ಆದರೆ (s->strstart != 0) ಶೀರ್ಷಿಕೆ |= PRESET_DICT;
        ಶೀರ್ಷಿಕೆ += 31 - (ಶೀರ್ಷಿಕೆ % 31);

        putShortMSB(s, ಶೀರ್ಷಿಕೆ);

        /* Save the adler32 of the preset dictionary: */
        ಆದರೆ (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->ಸ್ಥಿತಿ_2 = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        ಆದರೆ (s->pending != 0) {
            s->ಕೊನೆಯ_flush = -1;
            ಹಿಂತಿರುಗಿ Z_OK;
        }
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == GZIP_STATE) {
        /* gzip header */
        strm->adler = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        ಆದರೆ (s->gzhead == Z_NULL) {
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, OS_ಸಂಕೇತ);
            s->ಸ್ಥಿತಿ_2 = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            ಆದರೆ (s->pending != 0) {
                s->ಕೊನೆಯ_flush = -1;
                ಹಿಂತಿರುಗಿ Z_OK;
            }
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            put_byte(s, (s->gzhead->ಪಠ್ಯ ? 1 : 0) +
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
            ಆದರೆ (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            ಆದರೆ (s->gzhead->hcrc)
                strm->adler = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->ಸ್ಥಿತಿ_2 = EXTRA_STATE;
        }
    }
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == EXTRA_STATE) {
        ಆದರೆ (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg ಎಡ = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            ಆಗ (s->pending + ಎಡ > s->pending_buf_ಗಾತ್ರ) {
                ulg copy = s->pending_buf_ಗಾತ್ರ - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_ಗಾತ್ರ;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                ಆದರೆ (s->pending != 0) {
                    s->ಕೊನೆಯ_flush = -1;
                    ಹಿಂತಿರುಗಿ Z_OK;
                }
                beg = 0;
                ಎಡ -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, ಎಡ);
            s->pending += ಎಡ;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->ಸ್ಥಿತಿ_2 = NAME_STATE;
    }
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == NAME_STATE) {
        ಆದರೆ (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ಪೂರ್ಣಾಂಕ val;
            ಮಾಡು {
                ಆದರೆ (s->pending == s->pending_buf_ಗಾತ್ರ) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    ಆದರೆ (s->pending != 0) {
                        s->ಕೊನೆಯ_flush = -1;
                        ಹಿಂತಿರುಗಿ Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } ಆಗ (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->ಸ್ಥಿತಿ_2 = COMMENT_STATE;
    }
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == COMMENT_STATE) {
        ಆದರೆ (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ಪೂರ್ಣಾಂಕ val;
            ಮಾಡು {
                ಆದರೆ (s->pending == s->pending_buf_ಗಾತ್ರ) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    ಆದರೆ (s->pending != 0) {
                        s->ಕೊನೆಯ_flush = -1;
                        ಹಿಂತಿರುಗಿ Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } ಆಗ (val != 0);
            HCRC_UPDATE(beg);
        }
        s->ಸ್ಥಿತಿ_2 = HCRC_STATE;
    }
    ಆದರೆ (s->ಸ್ಥಿತಿ_2 == HCRC_STATE) {
        ಆದರೆ (s->gzhead->hcrc) {
            ಆದರೆ (s->pending + 2 > s->pending_buf_ಗಾತ್ರ) {
                flush_pending(strm);
                ಆದರೆ (s->pending != 0) {
                    s->ಕೊನೆಯ_flush = -1;
                    ಹಿಂತಿರುಗಿ Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0);
        }
        s->ಸ್ಥಿತಿ_2 = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        ಆದರೆ (s->pending != 0) {
            s->ಕೊನೆಯ_flush = -1;
            ಹಿಂತಿರುಗಿ Z_OK;
        }
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* Start a new block or continue the current one.
     */
    ಆದರೆ (strm->avail_ಒಳ != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->ಸ್ಥಿತಿ_2 != ಮುಕ್ತಾಯ_STATE)) {
        ಖಂಡ_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_ಕೋಷ್ಟಕ[s->level].func))(s, flush);

        ಆದರೆ (bstate == ಮುಕ್ತಾಯ_started || bstate == ಮುಕ್ತಾಯ_ಮುಗಿದ) {
            s->ಸ್ಥಿತಿ_2 = ಮುಕ್ತಾಯ_STATE;
        }
        ಆದರೆ (bstate == need_ಇನ್ನಷ್ಟು || bstate == ಮುಕ್ತಾಯ_started) {
            ಆದರೆ (strm->avail_ಹೊರಹೋಗು == 0) {
                s->ಕೊನೆಯ_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            ಹಿಂತಿರುಗಿ Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        ಆದರೆ (bstate == ಖಂಡ_ಮುಗಿದ) {
            ಆದರೆ (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flush != Z_ಖಂಡ) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_ಖಂಡ(s, (ಅಕ್ಷರ*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                ಆದರೆ (flush == Z_ತುಂಬಿದ_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    ಆದರೆ (s->lookahead == 0) {
                        s->strstart = 0;
                        s->ಖಂಡ_ಆರಂಭ = 0L;
                        s->ಸೇರಿಸು = 0;
                    }
                }
            }
            flush_pending(strm);
            ಆದರೆ (strm->avail_ಹೊರಹೋಗು == 0) {
              s->ಕೊನೆಯ_flush = -1; /* avoid BUF_ERROR at next call, see above */
              ಹಿಂತಿರುಗಿ Z_OK;
            }
        }
    }

    ಆದರೆ (flush != Z_ಮುಕ್ತಾಯ) ಹಿಂತಿರುಗಿ Z_OK;
    ಆದರೆ (s->wrap <= 0) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ಕೊನೆ;

    /* Write the trailer */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ GZIP
    ಆದರೆ (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->ಒಟ್ಟು_ಒಳ & 0xff));
        put_byte(s, (Byte)((strm->ಒಟ್ಟು_ಒಳ >> 8) & 0xff));
        put_byte(s, (Byte)((strm->ಒಟ್ಟು_ಒಳ >> 16) & 0xff));
        put_byte(s, (Byte)((strm->ಒಟ್ಟು_ಒಳ >> 24) & 0xff));
    }
    ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    ಆದರೆ (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    ಹಿಂತಿರುಗಿ s->pending != 0 ? Z_OK : Z_ಹರಿವು_ಕೊನೆ;
}

/* ========================================================================= */
ಪೂರ್ಣಾಂಕ ZEXPORT deflate_ಕೊನೆ(z_streamp strm) {
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;

    ಆದರೆ (deflateStateCheck(strm)) ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    ಸ್ಥಿತಿ_2 = strm->state->ಸ್ಥಿತಿ_2;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->ತಲೆ);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->ಕಿಟಕಿ);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 == BUSY_STATE ? Z_ದತ್ತಾಂಶ_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
ಪೂರ್ಣಾಂಕ ZEXPORT deflateCopy(z_streamp dest, z_streamp ಮೂಲ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAXSEG_64K
    (ಶೂನ್ಯ)dest;
    (ಶೂನ್ಯ)ಮೂಲ;
    ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    deflate_state *ds;
    deflate_state *ss;


    ಆದರೆ (deflateStateCheck(ಮೂಲ) || dest == Z_NULL) {
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    }

    ss = ಮೂಲ->state;

    zmemcpy(dest, ಮೂಲ, ಗಾತ್ರ(z_ಹರಿವು));

    ds = (deflate_state *) ZALLOC(dest, 1, ಗಾತ್ರ(deflate_state));
    ಆದರೆ (ds == Z_NULL) ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    zmemzero(ds, ಗಾತ್ರ(deflate_state));
    dest->state = (ರಚನೆ ಆಂತರಿಕ_state FAR *) ds;
    zmemcpy(ds, ss, ಗಾತ್ರ(deflate_state));
    ds->strm = dest;

    ds->ಕಿಟಕಿ = (Bytef *) ZALLOC(dest, ds->w_ಗಾತ್ರ, 2*ಗಾತ್ರ(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_ಗಾತ್ರ, ಗಾತ್ರ(Pos));
    ds->ತಲೆ   = (Posf *)  ZALLOC(dest, ds->hash_ಗಾತ್ರ, ಗಾತ್ರ(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    ಆದರೆ (ds->ಕಿಟಕಿ == Z_NULL || ds->prev == Z_NULL || ds->ತಲೆ == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_ಕೊನೆ (dest);
        ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->ಕಿಟಕಿ, ss->ಕಿಟಕಿ, ss->ಮೇಲಿನ_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->ಸೇರಿಸು > ds->w_ಗಾತ್ರ ? ds->w_ಗಾತ್ರ :
                ss->strstart - ss->ಸೇರಿಸು) * ಗಾತ್ರ(Pos));
    zmemcpy(ds->ತಲೆ, ss->ತಲೆ, ds->hash_ಗಾತ್ರ * ಗಾತ್ರ(Pos));

    ds->pending_ಹೊರಹೋಗು = ds->pending_buf + (ss->pending_ಹೊರಹೋಗು - ss->pending_buf);
    zmemcpy(ds->pending_ಹೊರಹೋಗು, ss->pending_ಹೊರಹೋಗು, ss->pending);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_ಮುಂದಿನ * ಗಾತ್ರ(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_ಮುಂದಿನ);
#ಇಲ್ಲದಿದ್ದರೆ
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_ಮುಂದಿನ);
#ಆದರೆ_ಅಂತ್ಯ

    ds->l_desc.dyn_ವೃಕ್ಷ = ds->dyn_ltree;
    ds->d_desc.dyn_ವೃಕ್ಷ = ds->dyn_dtree;
    ds->bl_desc.dyn_ವೃಕ್ಷ = ds->bl_ವೃಕ್ಷ;

    ಹಿಂತಿರುಗಿ Z_OK;
#ಆದರೆ_ಅಂತ್ಯ /* MAXSEG_64K */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_ಹೊಂದಿಕೆ(deflate_state *s, IPos cur_ಹೊಂದಿಕೆ) {
    ಚಿಹ್ನೆರಹಿತ chain_ಉದ್ದಳತೆ = s->max_chain_ಉದ್ದಳತೆ;/* max hash chain length */
    Bytef *ಪರಿಶೀಲನೆ = s->ಕಿಟಕಿ + s->strstart;      /* current string */
    Bytef *ಹೊಂದಿಕೆ;                               /* matched string */
    ಪೂರ್ಣಾಂಕ len;                                    /* length of current match */
    ಪೂರ್ಣಾಂಕ ಉತ್ತಮ_len = (ಪೂರ್ಣಾಂಕ)s->prev_ಉದ್ದಳತೆ;         /* best match length so far */
    ಪೂರ್ಣಾಂಕ nice_ಹೊಂದಿಕೆ = s->nice_ಹೊಂದಿಕೆ;             /* stop if match long enough */
    IPos ಎಲ್ಲೆ = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_ಮುಖವಾಡ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->ಕಿಟಕಿ + s->strstart + MAX_ಹೊಂದಿಕೆ - 1;
    ush ಪರಿಶೀಲನೆ_ಆರಂಭ = *(ushf*)ಪರಿಶೀಲನೆ;
    ush ಪರಿಶೀಲನೆ_ಕೊನೆ   = *(ushf*)(ಪರಿಶೀಲನೆ + ಉತ್ತಮ_len - 1);
#ಇಲ್ಲದಿದ್ದರೆ
    Bytef *strend = s->ಕಿಟಕಿ + s->strstart + MAX_ಹೊಂದಿಕೆ;
    Byte ಪರಿಶೀಲನೆ_end1  = ಪರಿಶೀಲನೆ[ಉತ್ತಮ_len - 1];
    Byte ಪರಿಶೀಲನೆ_ಕೊನೆ   = ಪರಿಶೀಲನೆ[ಉತ್ತಮ_len];
#ಆದರೆ_ಅಂತ್ಯ

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_ಬಿಟ್ಟುಗಳು >= 8 && MAX_ಹೊಂದಿಕೆ == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    ಆದರೆ (s->prev_ಉದ್ದಳತೆ >= s->good_ಹೊಂದಿಕೆ) {
        chain_ಉದ್ದಳತೆ >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    ಆದರೆ ((uInt)nice_ಹೊಂದಿಕೆ > s->lookahead) nice_ಹೊಂದಿಕೆ = (ಪೂರ್ಣಾಂಕ)s->lookahead;

    Assert((ulg)s->strstart <= s->ಕಿಟಕಿ_ಗಾತ್ರ - MIN_LOOKAHEAD,
           "need lookahead");

    ಮಾಡು {
        Assert(cur_ಹೊಂದಿಕೆ < s->strstart, "no future");
        ಹೊಂದಿಕೆ = s->ಕಿಟಕಿ + cur_ಹೊಂದಿಕೆ;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#ಆದರೆ (defined(UNALIGNED_OK) && MAX_ಹೊಂದಿಕೆ == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        ಆದರೆ (*(ushf*)(ಹೊಂದಿಕೆ + ಉತ್ತಮ_len - 1) != ಪರಿಶೀಲನೆ_ಕೊನೆ ||
            *(ushf*)ಹೊಂದಿಕೆ != ಪರಿಶೀಲನೆ_ಆರಂಭ) ಮುಂದುವರಿ;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(ಪರಿಶೀಲನೆ[2] == ಹೊಂದಿಕೆ[2], "scan[2]?");
        ಪರಿಶೀಲನೆ++, ಹೊಂದಿಕೆ++;
        ಮಾಡು {
        } ಆಗ (*(ushf*)(ಪರಿಶೀಲನೆ += 2) == *(ushf*)(ಹೊಂದಿಕೆ += 2) &&
                 *(ushf*)(ಪರಿಶೀಲನೆ += 2) == *(ushf*)(ಹೊಂದಿಕೆ += 2) &&
                 *(ushf*)(ಪರಿಶೀಲನೆ += 2) == *(ushf*)(ಹೊಂದಿಕೆ += 2) &&
                 *(ushf*)(ಪರಿಶೀಲನೆ += 2) == *(ushf*)(ಹೊಂದಿಕೆ += 2) &&
                 ಪರಿಶೀಲನೆ < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(ಪರಿಶೀಲನೆ <= s->ಕಿಟಕಿ + (ಚಿಹ್ನೆರಹಿತ)(s->ಕಿಟಕಿ_ಗಾತ್ರ - 1),
               "wild scan");
        ಆದರೆ (*ಪರಿಶೀಲನೆ == *ಹೊಂದಿಕೆ) ಪರಿಶೀಲನೆ++;

        len = (MAX_ಹೊಂದಿಕೆ - 1) - (ಪೂರ್ಣಾಂಕ)(strend - ಪರಿಶೀಲನೆ);
        ಪರಿಶೀಲನೆ = strend - (MAX_ಹೊಂದಿಕೆ-1);

#ಇಲ್ಲದಿದ್ದರೆ /* UNALIGNED_OK */

        ಆದರೆ (ಹೊಂದಿಕೆ[ಉತ್ತಮ_len]     != ಪರಿಶೀಲನೆ_ಕೊನೆ  ||
            ಹೊಂದಿಕೆ[ಉತ್ತಮ_len - 1] != ಪರಿಶೀಲನೆ_end1 ||
            *ಹೊಂದಿಕೆ              != *ಪರಿಶೀಲನೆ     ||
            *++ಹೊಂದಿಕೆ            != ಪರಿಶೀಲನೆ[1])      ಮುಂದುವರಿ;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        ಪರಿಶೀಲನೆ += 2, ಹೊಂದಿಕೆ++;
        Assert(*ಪರಿಶೀಲನೆ == *ಹೊಂದಿಕೆ, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        ಮಾಡು {
        } ಆಗ (*++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
                 *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
                 *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
                 *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
                 ಪರಿಶೀಲನೆ < strend);

        Assert(ಪರಿಶೀಲನೆ <= s->ಕಿಟಕಿ + (ಚಿಹ್ನೆರಹಿತ)(s->ಕಿಟಕಿ_ಗಾತ್ರ - 1),
               "wild scan");

        len = MAX_ಹೊಂದಿಕೆ - (ಪೂರ್ಣಾಂಕ)(strend - ಪರಿಶೀಲನೆ);
        ಪರಿಶೀಲನೆ = strend - MAX_ಹೊಂದಿಕೆ;

#ಆದರೆ_ಅಂತ್ಯ /* UNALIGNED_OK */

        ಆದರೆ (len > ಉತ್ತಮ_len) {
            s->ಹೊಂದಿಕೆ_ಆರಂಭ = cur_ಹೊಂದಿಕೆ;
            ಉತ್ತಮ_len = len;
            ಆದರೆ (len >= nice_ಹೊಂದಿಕೆ) ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ UNALIGNED_OK
            ಪರಿಶೀಲನೆ_ಕೊನೆ = *(ushf*)(ಪರಿಶೀಲನೆ + ಉತ್ತಮ_len - 1);
#ಇಲ್ಲದಿದ್ದರೆ
            ಪರಿಶೀಲನೆ_end1  = ಪರಿಶೀಲನೆ[ಉತ್ತಮ_len - 1];
            ಪರಿಶೀಲನೆ_ಕೊನೆ   = ಪರಿಶೀಲನೆ[ಉತ್ತಮ_len];
#ಆದರೆ_ಅಂತ್ಯ
        }
    } ಆಗ ((cur_ಹೊಂದಿಕೆ = prev[cur_ಹೊಂದಿಕೆ & wmask]) > ಎಲ್ಲೆ
             && --chain_ಉದ್ದಳತೆ != 0);

    ಆದರೆ ((uInt)ಉತ್ತಮ_len <= s->lookahead) ಹಿಂತಿರುಗಿ (uInt)ಉತ್ತಮ_len;
    ಹಿಂತಿರುಗಿ s->lookahead;
}

#ಇಲ್ಲದಿದ್ದರೆ /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_ಹೊಂದಿಕೆ(deflate_state *s, IPos cur_ಹೊಂದಿಕೆ) {
    Bytef *ಪರಿಶೀಲನೆ = s->ಕಿಟಕಿ + s->strstart;      /* current string */
    Bytef *ಹೊಂದಿಕೆ;                               /* matched string */
    ಪೂರ್ಣಾಂಕ len;                                    /* length of current match */
    Bytef *strend = s->ಕಿಟಕಿ + s->strstart + MAX_ಹೊಂದಿಕೆ;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_ಬಿಟ್ಟುಗಳು >= 8 && MAX_ಹೊಂದಿಕೆ == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->ಕಿಟಕಿ_ಗಾತ್ರ - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_ಹೊಂದಿಕೆ < s->strstart, "no future");

    ಹೊಂದಿಕೆ = s->ಕಿಟಕಿ + cur_ಹೊಂದಿಕೆ;

    /* Return failure if the match length is less than 2:
     */
    ಆದರೆ (ಹೊಂದಿಕೆ[0] != ಪರಿಶೀಲನೆ[0] || ಹೊಂದಿಕೆ[1] != ಪರಿಶೀಲನೆ[1]) ಹಿಂತಿರುಗಿ MIN_ಹೊಂದಿಕೆ-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    ಪರಿಶೀಲನೆ += 2, ಹೊಂದಿಕೆ += 2;
    Assert(*ಪರಿಶೀಲನೆ == *ಹೊಂದಿಕೆ, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    ಮಾಡು {
    } ಆಗ (*++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
             *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
             *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
             *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ && *++ಪರಿಶೀಲನೆ == *++ಹೊಂದಿಕೆ &&
             ಪರಿಶೀಲನೆ < strend);

    Assert(ಪರಿಶೀಲನೆ <= s->ಕಿಟಕಿ + (ಚಿಹ್ನೆರಹಿತ)(s->ಕಿಟಕಿ_ಗಾತ್ರ - 1), "wild scan");

    len = MAX_ಹೊಂದಿಕೆ - (ಪೂರ್ಣಾಂಕ)(strend - ಪರಿಶೀಲನೆ);

    ಆದರೆ (len < MIN_ಹೊಂದಿಕೆ) ಹಿಂತಿರುಗಿ MIN_ಹೊಂದಿಕೆ - 1;

    s->ಹೊಂದಿಕೆ_ಆರಂಭ = cur_ಹೊಂದಿಕೆ;
    ಹಿಂತಿರುಗಿ (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#ಆದರೆ_ಅಂತ್ಯ /* FASTEST */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG

#ವ್ಯಾಖ್ಯಾನಿಸು EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local ಶೂನ್ಯ check_ಹೊಂದಿಕೆ(deflate_state *s, IPos ಆರಂಭ, IPos ಹೊಂದಿಕೆ, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ) {
    /* check that the match is indeed a match */
    Bytef *ಹಿಂಭಾಗ = s->ಕಿಟಕಿ + (ಪೂರ್ಣಾಂಕ)ಹೊಂದಿಕೆ, *ಇಲ್ಲಿ = s->ಕಿಟಕಿ + ಆರಂಭ;
    IPos len = (IPos)ಉದ್ದಳತೆ;
    ಆದರೆ (ಹೊಂದಿಕೆ == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        ಹಿಂಭಾಗ++;
        ಇಲ್ಲಿ++;
        len--;
    }
    ಆದರೆ (zmemcmp(ಹಿಂಭಾಗ, ಇಲ್ಲಿ, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                ಆರಂಭ, (ಪೂರ್ಣಾಂಕ)ಹೊಂದಿಕೆ, ಉದ್ದಳತೆ);
        ಮಾಡು {
            fprintf(stderr, "(%02x %02x)", *ಹಿಂಭಾಗ++, *ಇಲ್ಲಿ++);
        } ಆಗ (--len != 0);
        z_error("invalid match");
    }
    ಆದರೆ (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", ಆರಂಭ - ಹೊಂದಿಕೆ, ಉದ್ದಳತೆ);
        ಮಾಡು { putc(s->ಕಿಟಕಿ[ಆರಂಭ++], stderr); } ಆಗ (--ಉದ್ದಳತೆ != 0);
    }
}
#ಇಲ್ಲದಿದ್ದರೆ
#  define check_ಹೊಂದಿಕೆ(s, ಆರಂಭ, ಹೊಂದಿಕೆ, ಉದ್ದಳತೆ)
#ಆದರೆ_ಅಂತ್ಯ /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು FLUSH_ಖಂಡ_ONLY(s, ಕೊನೆಯ) { \
   tr_flush_ಖಂಡ(s, (s->ಖಂಡ_ಆರಂಭ >= 0L ? \
                   (charf *)&s->ಕಿಟಕಿ[(ಚಿಹ್ನೆರಹಿತ)s->ಖಂಡ_ಆರಂಭ] : \
                   (charf *)Z_NULL), \
                (ulg)((ದೀರ್ಘ)s->strstart - s->ಖಂಡ_ಆರಂಭ), \
                (ಕೊನೆಯ)); \
   s->ಖಂಡ_ಆರಂಭ = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#ವ್ಯಾಖ್ಯಾನಿಸು FLUSH_ಖಂಡ(s, ಕೊನೆಯ) { \
   FLUSH_ಖಂಡ_ONLY(s, ಕೊನೆಯ); \
   ಆದರೆ (s->strm->avail_ಹೊರಹೋಗು == 0) ಹಿಂತಿರುಗಿ (ಕೊನೆಯ) ? ಮುಕ್ತಾಯ_started : need_ಇನ್ನಷ್ಟು; \
}

/* Maximum stored block length in deflate format (not including header). */
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_STORED 65535

/* Minimum of a and b. */
#ವ್ಯಾಖ್ಯಾನಿಸು MIN(a, b) ((a) > (b) ? (b) : (a))

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
local ಖಂಡ_state deflate_stored(deflate_state *s, ಪೂರ್ಣಾಂಕ flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    ಚಿಹ್ನೆರಹಿತ min_ಖಂಡ = (ಚಿಹ್ನೆರಹಿತ)(MIN(s->pending_buf_ಗಾತ್ರ - 5, s->w_ಗಾತ್ರ));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    ಪೂರ್ಣಾಂಕ ಕೊನೆಯ = 0;
    ಚಿಹ್ನೆರಹಿತ len, ಎಡ, have;
    ಚಿಹ್ನೆರಹಿತ ಬಳಸಿದ = s->strm->avail_ಒಳ;
    ಮಾಡು {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((ಚಿಹ್ನೆರಹಿತ)s->bi_valid + 42) >> 3;   /* bytes in header */
        ಆದರೆ (s->strm->avail_ಹೊರಹೋಗು < have)          /* need room for header */
            ನಿಲ್ಲಿಸು;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_ಹೊರಹೋಗು - have;
        ಎಡ = (ಚಿಹ್ನೆರಹಿತ)(s->strstart - s->ಖಂಡ_ಆರಂಭ);    /* window bytes */
        ಆದರೆ (len > (ulg)ಎಡ + s->strm->avail_ಒಳ)
            len = ಎಡ + s->strm->avail_ಒಳ;     /* limit len to the input */
        ಆದರೆ (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        ಆದರೆ (len < min_ಖಂಡ && ((len == 0 && flush != Z_ಮುಕ್ತಾಯ) ||
                                flush == Z_NO_FLUSH ||
                                len != ಎಡ + s->strm->avail_ಒಳ))
            ನಿಲ್ಲಿಸು;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        ಕೊನೆಯ = flush == Z_ಮುಕ್ತಾಯ && len == ಎಡ + s->strm->avail_ಒಳ ? 1 : 0;
        tr_stored_ಖಂಡ(s, (ಅಕ್ಷರ *)0, 0L, ಕೊನೆಯ);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->ಬಿಟ್ಟುಗಳು_ಕಳುಹಿಸಿದ += len << 3;
#ಆದರೆ_ಅಂತ್ಯ

        /* Copy uncompressed bytes from the window to next_out. */
        ಆದರೆ (ಎಡ) {
            ಆದರೆ (ಎಡ > len)
                ಎಡ = len;
            zmemcpy(s->strm->ಮುಂದಿನ_ಹೊರಹೋಗು, s->ಕಿಟಕಿ + s->ಖಂಡ_ಆರಂಭ, ಎಡ);
            s->strm->ಮುಂದಿನ_ಹೊರಹೋಗು += ಎಡ;
            s->strm->avail_ಹೊರಹೋಗು -= ಎಡ;
            s->strm->ಒಟ್ಟು_ಹೊರಹೋಗು += ಎಡ;
            s->ಖಂಡ_ಆರಂಭ += ಎಡ;
            len -= ಎಡ;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        ಆದರೆ (len) {
            read_buf(s->strm, s->strm->ಮುಂದಿನ_ಹೊರಹೋಗು, len);
            s->strm->ಮುಂದಿನ_ಹೊರಹೋಗು += len;
            s->strm->avail_ಹೊರಹೋಗು -= len;
            s->strm->ಒಟ್ಟು_ಹೊರಹೋಗು += len;
        }
    } ಆಗ (ಕೊನೆಯ == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    ಬಳಸಿದ -= s->strm->avail_ಒಳ;      /* number of input bytes directly copied */
    ಆದರೆ (ಬಳಸಿದ) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        ಆದರೆ (ಬಳಸಿದ >= s->w_ಗಾತ್ರ) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->ಕಿಟಕಿ, s->strm->ಮುಂದಿನ_ಒಳ - s->w_ಗಾತ್ರ, s->w_ಗಾತ್ರ);
            s->strstart = s->w_ಗಾತ್ರ;
            s->ಸೇರಿಸು = s->strstart;
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            ಆದರೆ (s->ಕಿಟಕಿ_ಗಾತ್ರ - s->strstart <= ಬಳಸಿದ) {
                /* Slide the window down. */
                s->strstart -= s->w_ಗಾತ್ರ;
                zmemcpy(s->ಕಿಟಕಿ, s->ಕಿಟಕಿ + s->w_ಗಾತ್ರ, s->strstart);
                ಆದರೆ (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                ಆದರೆ (s->ಸೇರಿಸು > s->strstart)
                    s->ಸೇರಿಸು = s->strstart;
            }
            zmemcpy(s->ಕಿಟಕಿ + s->strstart, s->strm->ಮುಂದಿನ_ಒಳ - ಬಳಸಿದ, ಬಳಸಿದ);
            s->strstart += ಬಳಸಿದ;
            s->ಸೇರಿಸು += MIN(ಬಳಸಿದ, s->w_ಗಾತ್ರ - s->ಸೇರಿಸು);
        }
        s->ಖಂಡ_ಆರಂಭ = s->strstart;
    }
    ಆದರೆ (s->ಮೇಲಿನ_water < s->strstart)
        s->ಮೇಲಿನ_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    ಆದರೆ (ಕೊನೆಯ) {
        s->bi_ಬಳಸಿದ = 8;
        ಹಿಂತಿರುಗಿ ಮುಕ್ತಾಯ_ಮುಗಿದ;
    }

    /* If flushing and all input has been consumed, then done. */
    ಆದರೆ (flush != Z_NO_FLUSH && flush != Z_ಮುಕ್ತಾಯ &&
        s->strm->avail_ಒಳ == 0 && (ದೀರ್ಘ)s->strstart == s->ಖಂಡ_ಆರಂಭ)
        ಹಿಂತಿರುಗಿ ಖಂಡ_ಮುಗಿದ;

    /* Fill the window with any remaining input. */
    have = (ಚಿಹ್ನೆರಹಿತ)(s->ಕಿಟಕಿ_ಗಾತ್ರ - s->strstart);
    ಆದರೆ (s->strm->avail_ಒಳ > have && s->ಖಂಡ_ಆರಂಭ >= (ದೀರ್ಘ)s->w_ಗಾತ್ರ) {
        /* Slide the window down. */
        s->ಖಂಡ_ಆರಂಭ -= s->w_ಗಾತ್ರ;
        s->strstart -= s->w_ಗಾತ್ರ;
        zmemcpy(s->ಕಿಟಕಿ, s->ಕಿಟಕಿ + s->w_ಗಾತ್ರ, s->strstart);
        ಆದರೆ (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_ಗಾತ್ರ;          /* more space now */
        ಆದರೆ (s->ಸೇರಿಸು > s->strstart)
            s->ಸೇರಿಸು = s->strstart;
    }
    ಆದರೆ (have > s->strm->avail_ಒಳ)
        have = s->strm->avail_ಒಳ;
    ಆದರೆ (have) {
        read_buf(s->strm, s->ಕಿಟಕಿ + s->strstart, have);
        s->strstart += have;
        s->ಸೇರಿಸು += MIN(have, s->w_ಗಾತ್ರ - s->ಸೇರಿಸು);
    }
    ಆದರೆ (s->ಮೇಲಿನ_water < s->strstart)
        s->ಮೇಲಿನ_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((ಚಿಹ್ನೆರಹಿತ)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (ಚಿಹ್ನೆರಹಿತ)MIN(s->pending_buf_ಗಾತ್ರ - have, MAX_STORED);
    min_ಖಂಡ = MIN(have, s->w_ಗಾತ್ರ);
    ಎಡ = (ಚಿಹ್ನೆರಹಿತ)(s->strstart - s->ಖಂಡ_ಆರಂಭ);
    ಆದರೆ (ಎಡ >= min_ಖಂಡ ||
        ((ಎಡ || flush == Z_ಮುಕ್ತಾಯ) && flush != Z_NO_FLUSH &&
         s->strm->avail_ಒಳ == 0 && ಎಡ <= have)) {
        len = MIN(ಎಡ, have);
        ಕೊನೆಯ = flush == Z_ಮುಕ್ತಾಯ && s->strm->avail_ಒಳ == 0 &&
               len == ಎಡ ? 1 : 0;
        tr_stored_ಖಂಡ(s, (charf *)s->ಕಿಟಕಿ + s->ಖಂಡ_ಆರಂಭ, len, ಕೊನೆಯ);
        s->ಖಂಡ_ಆರಂಭ += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    ಆದರೆ (ಕೊನೆಯ)
        s->bi_ಬಳಸಿದ = 8;
    ಹಿಂತಿರುಗಿ ಕೊನೆಯ ? ಮುಕ್ತಾಯ_started : need_ಇನ್ನಷ್ಟು;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local ಖಂಡ_state deflate_fast(deflate_state *s, ಪೂರ್ಣಾಂಕ flush) {
    IPos hash_ತಲೆ;       /* head of the hash chain */
    ಪೂರ್ಣಾಂಕ bflush;           /* set if current block must be flushed */

    ಗಾಗಿ (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        ಆದರೆ (s->lookahead < MIN_LOOKAHEAD) {
            fill_ಕಿಟಕಿ(s);
            ಆದರೆ (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                ಹಿಂತಿರುಗಿ need_ಇನ್ನಷ್ಟು;
            }
            ಆದರೆ (s->lookahead == 0) ನಿಲ್ಲಿಸು; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_ತಲೆ = NIL;
        ಆದರೆ (s->lookahead >= MIN_ಹೊಂದಿಕೆ) {
            ಸೇರಿಸು_STRING(s, s->strstart, hash_ತಲೆ);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        ಆದರೆ (hash_ತಲೆ != NIL && s->strstart - hash_ತಲೆ <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = longest_ಹೊಂದಿಕೆ (s, hash_ತಲೆ);
            /* longest_match() sets match_start */
        }
        ಆದರೆ (s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ >= MIN_ಹೊಂದಿಕೆ) {
            check_ಹೊಂದಿಕೆ(s, s->strstart, s->ಹೊಂದಿಕೆ_ಆರಂಭ, (ಪೂರ್ಣಾಂಕ)s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ);

            _tr_tally_dist(s, s->strstart - s->ಹೊಂದಿಕೆ_ಆರಂಭ,
                           s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ - MIN_ಹೊಂದಿಕೆ, bflush);

            s->lookahead -= s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FASTEST
            ಆದರೆ (s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ <= s->max_ಸೇರಿಸು_ಉದ್ದಳತೆ &&
                s->lookahead >= MIN_ಹೊಂದಿಕೆ) {
                s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ--; /* string at strstart already in table */
                ಮಾಡು {
                    s->strstart++;
                    ಸೇರಿಸು_STRING(s, s->strstart, hash_ತಲೆ);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } ಆಗ (--s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ != 0);
                s->strstart++;
            } ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
            {
                s->strstart += s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;
                s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = 0;
                s->ins_h = s->ಕಿಟಕಿ[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->ಕಿಟಕಿ[s->strstart + 1]);
#ಆದರೆ MIN_ಹೊಂದಿಕೆ != 3
                Call UPDATE_HASH() MIN_ಹೊಂದಿಕೆ-3 ಇನ್ನಷ್ಟು times
#ಆದರೆ_ಅಂತ್ಯ
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } ಇಲ್ಲದಿದ್ದರೆ {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->ಕಿಟಕಿ[s->strstart]));
            _tr_tally_lit(s, s->ಕಿಟಕಿ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        ಆದರೆ (bflush) FLUSH_ಖಂಡ(s, 0);
    }
    s->ಸೇರಿಸು = s->strstart < MIN_ಹೊಂದಿಕೆ-1 ? s->strstart : MIN_ಹೊಂದಿಕೆ-1;
    ಆದರೆ (flush == Z_ಮುಕ್ತಾಯ) {
        FLUSH_ಖಂಡ(s, 1);
        ಹಿಂತಿರುಗಿ ಮುಕ್ತಾಯ_ಮುಗಿದ;
    }
    ಆದರೆ (s->sym_ಮುಂದಿನ)
        FLUSH_ಖಂಡ(s, 0);
    ಹಿಂತಿರುಗಿ ಖಂಡ_ಮುಗಿದ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local ಖಂಡ_state deflate_slow(deflate_state *s, ಪೂರ್ಣಾಂಕ flush) {
    IPos hash_ತಲೆ;          /* head of hash chain */
    ಪೂರ್ಣಾಂಕ bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    ಗಾಗಿ (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        ಆದರೆ (s->lookahead < MIN_LOOKAHEAD) {
            fill_ಕಿಟಕಿ(s);
            ಆದರೆ (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                ಹಿಂತಿರುಗಿ need_ಇನ್ನಷ್ಟು;
            }
            ಆದರೆ (s->lookahead == 0) ನಿಲ್ಲಿಸು; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_ತಲೆ = NIL;
        ಆದರೆ (s->lookahead >= MIN_ಹೊಂದಿಕೆ) {
            ಸೇರಿಸು_STRING(s, s->strstart, hash_ತಲೆ);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_ಉದ್ದಳತೆ = s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ, s->prev_ಹೊಂದಿಕೆ = s->ಹೊಂದಿಕೆ_ಆರಂಭ;
        s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = MIN_ಹೊಂದಿಕೆ-1;

        ಆದರೆ (hash_ತಲೆ != NIL && s->prev_ಉದ್ದಳತೆ < s->max_lazy_ಹೊಂದಿಕೆ &&
            s->strstart - hash_ತಲೆ <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = longest_ಹೊಂದಿಕೆ (s, hash_ತಲೆ);
            /* longest_match() sets match_start */

            ಆದರೆ (s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ <= 5 && (s->strategy == Z_FILTERED
#ಆದರೆ TOO_FAR <= 32767
                || (s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ == MIN_ಹೊಂದಿಕೆ &&
                    s->strstart - s->ಹೊಂದಿಕೆ_ಆರಂಭ > TOO_FAR)
#ಆದರೆ_ಅಂತ್ಯ
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = MIN_ಹೊಂದಿಕೆ-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        ಆದರೆ (s->prev_ಉದ್ದಳತೆ >= MIN_ಹೊಂದಿಕೆ && s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ <= s->prev_ಉದ್ದಳತೆ) {
            uInt max_ಸೇರಿಸು = s->strstart + s->lookahead - MIN_ಹೊಂದಿಕೆ;
            /* Do not insert strings in hash table beyond this. */

            check_ಹೊಂದಿಕೆ(s, s->strstart - 1, s->prev_ಹೊಂದಿಕೆ, (ಪೂರ್ಣಾಂಕ)s->prev_ಉದ್ದಳತೆ);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_ಹೊಂದಿಕೆ,
                           s->prev_ಉದ್ದಳತೆ - MIN_ಹೊಂದಿಕೆ, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_ಉದ್ದಳತೆ - 1;
            s->prev_ಉದ್ದಳತೆ -= 2;
            ಮಾಡು {
                ಆದರೆ (++s->strstart <= max_ಸೇರಿಸು) {
                    ಸೇರಿಸು_STRING(s, s->strstart, hash_ತಲೆ);
                }
            } ಆಗ (--s->prev_ಉದ್ದಳತೆ != 0);
            s->ಹೊಂದಿಕೆ_ಲಭ್ಯ = 0;
            s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = MIN_ಹೊಂದಿಕೆ-1;
            s->strstart++;

            ಆದರೆ (bflush) FLUSH_ಖಂಡ(s, 0);

        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s->ಹೊಂದಿಕೆ_ಲಭ್ಯ) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->ಕಿಟಕಿ[s->strstart - 1]));
            _tr_tally_lit(s, s->ಕಿಟಕಿ[s->strstart - 1], bflush);
            ಆದರೆ (bflush) {
                FLUSH_ಖಂಡ_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            ಆದರೆ (s->strm->avail_ಹೊರಹೋಗು == 0) ಹಿಂತಿರುಗಿ need_ಇನ್ನಷ್ಟು;
        } ಇಲ್ಲದಿದ್ದರೆ {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->ಹೊಂದಿಕೆ_ಲಭ್ಯ = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    ಆದರೆ (s->ಹೊಂದಿಕೆ_ಲಭ್ಯ) {
        Tracevv((stderr,"%c", s->ಕಿಟಕಿ[s->strstart - 1]));
        _tr_tally_lit(s, s->ಕಿಟಕಿ[s->strstart - 1], bflush);
        s->ಹೊಂದಿಕೆ_ಲಭ್ಯ = 0;
    }
    s->ಸೇರಿಸು = s->strstart < MIN_ಹೊಂದಿಕೆ-1 ? s->strstart : MIN_ಹೊಂದಿಕೆ-1;
    ಆದರೆ (flush == Z_ಮುಕ್ತಾಯ) {
        FLUSH_ಖಂಡ(s, 1);
        ಹಿಂತಿರುಗಿ ಮುಕ್ತಾಯ_ಮುಗಿದ;
    }
    ಆದರೆ (s->sym_ಮುಂದಿನ)
        FLUSH_ಖಂಡ(s, 0);
    ಹಿಂತಿರುಗಿ ಖಂಡ_ಮುಗಿದ;
}
#ಆದರೆ_ಅಂತ್ಯ /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local ಖಂಡ_state deflate_rle(deflate_state *s, ಪೂರ್ಣಾಂಕ flush) {
    ಪೂರ್ಣಾಂಕ bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *ಪರಿಶೀಲನೆ, *strend;   /* scan goes up to strend for length of run */

    ಗಾಗಿ (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        ಆದರೆ (s->lookahead <= MAX_ಹೊಂದಿಕೆ) {
            fill_ಕಿಟಕಿ(s);
            ಆದರೆ (s->lookahead <= MAX_ಹೊಂದಿಕೆ && flush == Z_NO_FLUSH) {
                ಹಿಂತಿರುಗಿ need_ಇನ್ನಷ್ಟು;
            }
            ಆದರೆ (s->lookahead == 0) ನಿಲ್ಲಿಸು; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = 0;
        ಆದರೆ (s->lookahead >= MIN_ಹೊಂದಿಕೆ && s->strstart > 0) {
            ಪರಿಶೀಲನೆ = s->ಕಿಟಕಿ + s->strstart - 1;
            prev = *ಪರಿಶೀಲನೆ;
            ಆದರೆ (prev == *++ಪರಿಶೀಲನೆ && prev == *++ಪರಿಶೀಲನೆ && prev == *++ಪರಿಶೀಲನೆ) {
                strend = s->ಕಿಟಕಿ + s->strstart + MAX_ಹೊಂದಿಕೆ;
                ಮಾಡು {
                } ಆಗ (prev == *++ಪರಿಶೀಲನೆ && prev == *++ಪರಿಶೀಲನೆ &&
                         prev == *++ಪರಿಶೀಲನೆ && prev == *++ಪರಿಶೀಲನೆ &&
                         prev == *++ಪರಿಶೀಲನೆ && prev == *++ಪರಿಶೀಲನೆ &&
                         prev == *++ಪರಿಶೀಲನೆ && prev == *++ಪರಿಶೀಲನೆ &&
                         ಪರಿಶೀಲನೆ < strend);
                s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = MAX_ಹೊಂದಿಕೆ - (uInt)(strend - ಪರಿಶೀಲನೆ);
                ಆದರೆ (s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ > s->lookahead)
                    s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = s->lookahead;
            }
            Assert(ಪರಿಶೀಲನೆ <= s->ಕಿಟಕಿ + (uInt)(s->ಕಿಟಕಿ_ಗಾತ್ರ - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        ಆದರೆ (s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ >= MIN_ಹೊಂದಿಕೆ) {
            check_ಹೊಂದಿಕೆ(s, s->strstart, s->strstart - 1, (ಪೂರ್ಣಾಂಕ)s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ);

            _tr_tally_dist(s, 1, s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ - MIN_ಹೊಂದಿಕೆ, bflush);

            s->lookahead -= s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;
            s->strstart += s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;
            s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = 0;
        } ಇಲ್ಲದಿದ್ದರೆ {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->ಕಿಟಕಿ[s->strstart]));
            _tr_tally_lit(s, s->ಕಿಟಕಿ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        ಆದರೆ (bflush) FLUSH_ಖಂಡ(s, 0);
    }
    s->ಸೇರಿಸು = 0;
    ಆದರೆ (flush == Z_ಮುಕ್ತಾಯ) {
        FLUSH_ಖಂಡ(s, 1);
        ಹಿಂತಿರುಗಿ ಮುಕ್ತಾಯ_ಮುಗಿದ;
    }
    ಆದರೆ (s->sym_ಮುಂದಿನ)
        FLUSH_ಖಂಡ(s, 0);
    ಹಿಂತಿರುಗಿ ಖಂಡ_ಮುಗಿದ;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local ಖಂಡ_state deflate_huff(deflate_state *s, ಪೂರ್ಣಾಂಕ flush) {
    ಪೂರ್ಣಾಂಕ bflush;             /* set if current block must be flushed */

    ಗಾಗಿ (;;) {
        /* Make sure that we have a literal to write. */
        ಆದರೆ (s->lookahead == 0) {
            fill_ಕಿಟಕಿ(s);
            ಆದರೆ (s->lookahead == 0) {
                ಆದರೆ (flush == Z_NO_FLUSH)
                    ಹಿಂತಿರುಗಿ need_ಇನ್ನಷ್ಟು;
                ನಿಲ್ಲಿಸು;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ = 0;
        Tracevv((stderr,"%c", s->ಕಿಟಕಿ[s->strstart]));
        _tr_tally_lit(s, s->ಕಿಟಕಿ[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        ಆದರೆ (bflush) FLUSH_ಖಂಡ(s, 0);
    }
    s->ಸೇರಿಸು = 0;
    ಆದರೆ (flush == Z_ಮುಕ್ತಾಯ) {
        FLUSH_ಖಂಡ(s, 1);
        ಹಿಂತಿರುಗಿ ಮುಕ್ತಾಯ_ಮುಗಿದ;
    }
    ಆದರೆ (s->sym_ಮುಂದಿನ)
        FLUSH_ಖಂಡ(s, 0);
    ಹಿಂತಿರುಗಿ ಖಂಡ_ಮುಗಿದ;
}
