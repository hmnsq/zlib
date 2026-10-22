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
    need_وڌيڪ,      /* block not completed, need more input or more output */
    بلاڪ_پورو,     /* block flush performed */
    پڄاڻي_started, /* finish started, need only more output at next deflate */
    پڄاڻي_پورو     /* finish done, accept no more input or output */
} بلاڪ_state;

تعريفنوع بلاڪ_state (*compress_func)(deflate_state *s, صحيح flush);
/* Compression function. Returns the block state after the call. */

local بلاڪ_state deflate_stored(deflate_state *s, صحيح flush);
local بلاڪ_state deflate_fast(deflate_state *s, صحيح flush);
#جيڪڏهن_وصف_نه_ٿيل FASTEST
local بلاڪ_state deflate_slow(deflate_state *s, صحيح flush);
#جيڪڏهن_پورو
local بلاڪ_state deflate_rle(deflate_state *s, صحيح flush);
local بلاڪ_state deflate_huff(deflate_state *s, صحيح flush);

/* ===========================================================================
 * Local data
 */

#وصف_ڪريو NIL 0
/* Tail of hash chains */

#جيڪڏهن_وصف_نه_ٿيل TOO_FAR
#  define TOO_FAR 4096
#جيڪڏهن_پورو
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
تعريفنوع بنية config_s {
   ush good_ڊيگهائي; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_ڊيگهائي; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#جيڪڏهن_وصف_ٿيل FASTEST
local ثابت config configuration_جدول[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#وإلا
local ثابت config configuration_جدول[10] = {
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
#جيڪڏهن_پورو

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#وصف_ڪريو RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#وصف_ڪريو UPDATE_HASH(s,h,c) (h = (((h) << s->hash_سرڪايو) ^ (c)) & s->hash_نقاب)


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
#جيڪڏهن_وصف_ٿيل FASTEST
#وصف_ڪريو وجهو_STRING(s, str, ميلاپ_سر) \
   (UPDATE_HASH(s, s->ins_h, s->دري[(str) + (MIN_ميلاپ-1)]), \
    ميلاپ_سر = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#وإلا
#وصف_ڪريو وجهو_STRING(s, str, ميلاپ_سر) \
   (UPDATE_HASH(s, s->ins_h, s->دري[(str) + (MIN_ميلاپ-1)]), \
    ميلاپ_سر = s->prev[(str) & s->w_نقاب] = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#جيڪڏهن_پورو

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#وصف_ڪريو CLEAR_HASH(s) \
    افعل { \
        s->سر[s->hash_سائيز - 1] = NIL; \
        zmemzero(s->سر, (بنانشان)(s->hash_سائيز - 1)*حجم(*s->سر)); \
        s->slid = 0; \
    } طالما (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#إذا defined(__has_feature)
#  إذا __has_feature(يادگيري_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#جيڪڏهن_پورو
local فراغ slide_hash(deflate_state *s) {
    بنانشان n, m;
    Posf *p;
    uInt wsize = s->w_سائيز;

    n = s->hash_سائيز;
    p = &s->سر[n];
    افعل {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } طالما (--n);
#جيڪڏهن_وصف_نه_ٿيل FASTEST
    n = wsize;
    p = &s->prev[n];
    افعل {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } طالما (--n);
#جيڪڏهن_پورو
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local بنانشان read_buf(z_streamp strm, Bytef *buf, بنانشان سائيز) {
    بنانشان len = strm->avail_اندريون;

    إذا (len > سائيز) len = سائيز;
    إذا (len == 0) أرجع 0;

    strm->avail_اندريون  -= len;

    zmemcpy(buf, strm->ايندڙ_اندريون, len);
    إذا (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#جيڪڏهن_وصف_ٿيل GZIP
    وإلا إذا (strm->state->wrap == 2) {
        strm->adler = چڪري_جاچ_32_2(strm->adler, buf, len);
    }
#جيڪڏهن_پورو
    strm->ايندڙ_اندريون  += len;
    strm->ڪل_اندريون += len;

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
local فراغ fill_دري(deflate_state *s) {
    بنانشان n;
    بنانشان وڌيڪ;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_سائيز;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    افعل {
        وڌيڪ = (بنانشان)(s->دري_سائيز -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#جيڪڏهن_وصف_ٿيل _MSC_VER
#هدايت warning(ڌڪو)
#هدايت warning(disable: 4127)
#جيڪڏهن_پورو
        إذا (حجم(صحيح) <= 2) {
#جيڪڏهن_وصف_ٿيل _MSC_VER
#هدايت warning(ڪڍو)
#جيڪڏهن_پورو
            إذا (وڌيڪ == 0 && s->strstart == 0 && s->lookahead == 0) {
                وڌيڪ = wsize;

            } وإلا إذا (وڌيڪ == (بنانشان)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                وڌيڪ--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        إذا (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->دري, s->دري + wsize, (بنانشان)wsize - وڌيڪ);
            s->ميلاپ_شروعات -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->بلاڪ_شروعات -= (ڊگهو) wsize;
            إذا (s->وجهو > s->strstart)
                s->وجهو = s->strstart;
            slide_hash(s);
            وڌيڪ += wsize;
        }
        إذا (s->strm->avail_اندريون == 0) اكسر;

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
        Assert(وڌيڪ >= 2, "more < 2");

        n = read_buf(s->strm, s->دري + s->strstart + s->lookahead, وڌيڪ);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        إذا (s->lookahead + s->وجهو >= MIN_ميلاپ) {
            uInt str = s->strstart - s->وجهو;
            s->ins_h = s->دري[str];
            UPDATE_HASH(s, s->ins_h, s->دري[str + 1]);
#إذا MIN_ميلاپ != 3
            Call UPDATE_HASH() MIN_ميلاپ-3 وڌيڪ times
#جيڪڏهن_پورو
            طالما (s->وجهو) {
                UPDATE_HASH(s, s->ins_h, s->دري[str + MIN_ميلاپ-1]);
#جيڪڏهن_وصف_نه_ٿيل FASTEST
                s->prev[str & s->w_نقاب] = s->سر[s->ins_h];
#جيڪڏهن_پورو
                s->سر[s->ins_h] = (Pos)str;
                str++;
                s->وجهو--;
                إذا (s->lookahead + s->وجهو < MIN_ميلاپ)
                    اكسر;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } طالما (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_اندريون != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    إذا (s->مٿاهون_water < s->دري_سائيز) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        إذا (s->مٿاهون_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->دري_سائيز - curr;
            إذا (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->دري + curr, (بنانشان)init);
            s->مٿاهون_water = curr + init;
        }
        وإلا إذا (s->مٿاهون_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->مٿاهون_water;
            إذا (init > s->دري_سائيز - s->مٿاهون_water)
                init = s->دري_سائيز - s->مٿاهون_water;
            zmemzero(s->دري + s->مٿاهون_water, (بنانشان)init);
            s->مٿاهون_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->دري_سائيز - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit_(z_streamp strm, صحيح level, ثابت حرف *نسخو,
                         صحيح وهڪرو_سائيز) {
    أرجع deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, نسخو, وهڪرو_سائيز);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit2_(z_streamp strm, صحيح level, صحيح طريقو,
                          صحيح دري_بٽ, صحيح memLevel, صحيح strategy,
                          ثابت حرف *نسخو, صحيح وهڪرو_سائيز) {
    deflate_state *s;
    صحيح wrap = 1;
    ساكن ثابت حرف my_نسخو[] = ZLIB_نسخو;

    إذا (نسخو == Z_NULL || نسخو[0] != my_نسخو[0] ||
        وهڪرو_سائيز != حجم(z_وهڪرو)) {
        أرجع Z_نسخو_ERROR;
    }
    إذا (strm == Z_NULL) أرجع Z_وهڪرو_ERROR;

    strm->نياپو = Z_NULL;
    إذا (strm->zalloc == (alloc_func)0) {
#جيڪڏهن_وصف_ٿيل Z_SOLO
        أرجع Z_وهڪرو_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#جيڪڏهن_پورو
    }
    إذا (strm->zfree == (free_func)0)
#جيڪڏهن_وصف_ٿيل Z_SOLO
        أرجع Z_وهڪرو_ERROR;
#وإلا
        strm->zfree = zcfree;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل FASTEST
    إذا (level != 0) level = 1;
#وإلا
    إذا (level == Z_DEFAULT_COMPRESSION) level = 6;
#جيڪڏهن_پورو

    إذا (دري_بٽ < 0) { /* suppress zlib wrapper */
        wrap = 0;
        إذا (دري_بٽ < -15)
            أرجع Z_وهڪرو_ERROR;
        دري_بٽ = -دري_بٽ;
    }
#جيڪڏهن_وصف_ٿيل GZIP
    وإلا إذا (دري_بٽ > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        دري_بٽ -= 16;
    }
#جيڪڏهن_پورو
    إذا (memLevel < 1 || memLevel > MAX_MEM_LEVEL || طريقو != Z_DEFLATED ||
        دري_بٽ < 8 || دري_بٽ > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (دري_بٽ == 8 && wrap != 1)) {
        أرجع Z_وهڪرو_ERROR;
    }
    إذا (دري_بٽ == 8) دري_بٽ = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, حجم(deflate_state));
    إذا (s == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(s, حجم(deflate_state));
    strm->state = (بنية اندروني_state FAR *)s;
    s->strm = strm;
    s->حالت = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_بٽ = (uInt)دري_بٽ;
    s->w_سائيز = 1 << s->w_بٽ;
    s->w_نقاب = s->w_سائيز - 1;

    s->hash_بٽ = (uInt)memLevel + 7;
    s->hash_سائيز = 1 << s->hash_بٽ;
    s->hash_نقاب = s->hash_سائيز - 1;
    s->hash_سرڪايو =  ((s->hash_بٽ + MIN_ميلاپ-1) / MIN_ميلاپ);

    s->دري = (Bytef *) ZALLOC(strm, s->w_سائيز, 2*حجم(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_سائيز, حجم(Pos));
    s->سر   = (Posf *)  ZALLOC(strm, s->hash_سائيز, حجم(Pos));

    s->مٿاهون_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_سائيز = (ulg)s->lit_bufsize * 4;

    إذا (s->دري == Z_NULL || s->prev == Z_NULL || s->سر == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->حالت = پڄاڻي_STATE;
        strm->نياپو = ERR_نياپو(Z_MEM_ERROR);
        deflate_پڇاڙي (strm);
        أرجع Z_MEM_ERROR;
    }
#جيڪڏهن_وصف_ٿيل LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_پڇاڙي = s->lit_bufsize - 1;
#وإلا
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_پڇاڙي = (s->lit_bufsize - 1) * 3;
#جيڪڏهن_پورو
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->طريقو = (Byte)طريقو;

    أرجع deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local صحيح deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    إذا (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        أرجع 1;
    s = strm->state;
    إذا (s == Z_NULL || s->strm != strm || (s->حالت != INIT_STATE &&
#جيڪڏهن_وصف_ٿيل GZIP
                                           s->حالت != GZIP_STATE &&
#جيڪڏهن_پورو
                                           s->حالت != EXTRA_STATE &&
                                           s->حالت != NAME_STATE &&
                                           s->حالت != COMMENT_STATE &&
                                           s->حالت != HCRC_STATE &&
                                           s->حالت != BUSY_STATE &&
                                           s->حالت != پڄاڻي_STATE))
        أرجع 1;
    أرجع 0;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_سيٽ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt  dict_ڊيگهائي) {
    deflate_state *s;
    uInt str, n;
    صحيح wrap;
    بنانشان avail;
    z_const بنانشان حرف *ايندڙ;

    إذا (deflateStateCheck(strm) || dictionary == Z_NULL)
        أرجع Z_وهڪرو_ERROR;
    s = strm->state;
    wrap = s->wrap;
    إذا (wrap == 2 || (wrap == 1 && s->حالت != INIT_STATE) || s->lookahead)
        أرجع Z_وهڪرو_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    إذا (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_ڊيگهائي);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    إذا (dict_ڊيگهائي >= s->w_سائيز) {
        إذا (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->بلاڪ_شروعات = 0L;
            s->وجهو = 0;
        }
        dictionary += dict_ڊيگهائي - s->w_سائيز;  /* use the tail */
        dict_ڊيگهائي = s->w_سائيز;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_اندريون;
    ايندڙ = strm->ايندڙ_اندريون;
    strm->avail_اندريون = dict_ڊيگهائي;
    strm->ايندڙ_اندريون = (z_const Bytef *)dictionary;
    fill_دري(s);
    طالما (s->lookahead >= MIN_ميلاپ) {
        str = s->strstart;
        n = s->lookahead - (MIN_ميلاپ-1);
        افعل {
            UPDATE_HASH(s, s->ins_h, s->دري[str + MIN_ميلاپ-1]);
#جيڪڏهن_وصف_نه_ٿيل FASTEST
            s->prev[str & s->w_نقاب] = s->سر[s->ins_h];
#جيڪڏهن_پورو
            s->سر[s->ins_h] = (Pos)str;
            str++;
        } طالما (--n);
        s->strstart = str;
        s->lookahead = MIN_ميلاپ-1;
        fill_دري(s);
    }
    s->strstart += s->lookahead;
    s->بلاڪ_شروعات = (ڊگهو)s->strstart;
    s->وجهو = s->lookahead;
    s->lookahead = 0;
    s->ميلاپ_ڊيگهائي = s->prev_ڊيگهائي = MIN_ميلاپ-1;
    s->ميلاپ_موجود = 0;
    strm->ايندڙ_اندريون = ايندڙ;
    strm->avail_اندريون = avail;
    s->wrap = wrap;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_ڊيگهائي) {
    deflate_state *s;
    uInt len;

    إذا (deflateStateCheck(strm))
        أرجع Z_وهڪرو_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    إذا (len > s->w_سائيز)
        len = s->w_سائيز;
    إذا (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->دري + s->strstart + s->lookahead - len, len);
    إذا (dict_ڊيگهائي != Z_NULL)
        *dict_ڊيگهائي = len;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    إذا (deflateStateCheck(strm)) {
        أرجع Z_وهڪرو_ERROR;
    }

    strm->ڪل_اندريون = strm->ڪل_ٻاهريون = 0;
    strm->نياپو = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->معلومات_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_ٻاهريون = s->pending_buf;

    إذا (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->حالت =
#جيڪڏهن_وصف_ٿيل GZIP
        s->wrap == 2 ? GZIP_STATE :
#جيڪڏهن_پورو
        INIT_STATE;
    strm->adler =
#جيڪڏهن_وصف_ٿيل GZIP
        s->wrap == 2 ? چڪري_جاچ_32_2(0L, Z_NULL, 0) :
#جيڪڏهن_پورو
        adler32(0L, Z_NULL, 0);
    s->آخري_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    أرجع Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local فراغ lm_init(deflate_state *s) {
    s->دري_سائيز = (ulg)2L*s->w_سائيز;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_ميلاپ   = configuration_جدول[s->level].max_lazy;
    s->good_ميلاپ       = configuration_جدول[s->level].good_ڊيگهائي;
    s->nice_ميلاپ       = configuration_جدول[s->level].nice_ڊيگهائي;
    s->max_chain_ڊيگهائي = configuration_جدول[s->level].max_chain;

    s->strstart = 0;
    s->بلاڪ_شروعات = 0L;
    s->lookahead = 0;
    s->وجهو = 0;
    s->ميلاپ_ڊيگهائي = s->prev_ڊيگهائي = MIN_ميلاپ-1;
    s->ميلاپ_موجود = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
صحيح ZEXPORT deflateReset(z_streamp strm) {
    صحيح ret;

    ret = deflateResetKeep(strm);
    إذا (ret == Z_OK)
        lm_init(strm->state);
    أرجع ret;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_سيٽ_سرو(z_streamp strm, gz_headerp سر) {
    إذا (deflateStateCheck(strm) || strm->state->wrap != 2)
        أرجع Z_وهڪرو_ERROR;
    strm->state->gzhead = سر;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflatePending(z_streamp strm, بنانشان *pending, صحيح *بٽ) {
    إذا (deflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    إذا (بٽ != Z_NULL)
        *بٽ = strm->state->bi_valid;
    إذا (pending != Z_NULL) {
        *pending = (بنانشان)strm->state->pending;
        إذا (*pending != strm->state->pending) {
            *pending = (بنانشان)-1;
            أرجع Z_BUF_ERROR;
        }
    }
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_استعمال_ٿيل(z_streamp strm, صحيح *بٽ) {
    إذا (deflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    إذا (بٽ != Z_NULL)
        *بٽ = strm->state->bi_استعمال_ٿيل;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_مفرد(z_streamp strm, صحيح بٽ, صحيح قدر) {
    deflate_state *s;
    صحيح put;

    إذا (deflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    s = strm->state;
#جيڪڏهن_وصف_ٿيل LIT_MEM
    إذا (بٽ < 0 || بٽ > 16 ||
        (uchf *)s->d_buf < s->pending_ٻاهريون + ((Buf_سائيز + 7) >> 3))
        أرجع Z_BUF_ERROR;
#وإلا
    إذا (بٽ < 0 || بٽ > 16 ||
        s->sym_buf < s->pending_ٻاهريون + ((Buf_سائيز + 7) >> 3))
        أرجع Z_BUF_ERROR;
#جيڪڏهن_پورو
    افعل {
        put = Buf_سائيز - s->bi_valid;
        إذا (put > بٽ)
            put = بٽ;
        s->bi_buf |= (ush)((قدر & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_بٽ(s);
        قدر >>= put;
        بٽ -= put;
    } طالما (بٽ);
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateParams(z_streamp strm, صحيح level, صحيح strategy) {
    deflate_state *s;
    compress_func func;

    إذا (deflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    s = strm->state;

#جيڪڏهن_وصف_ٿيل FASTEST
    إذا (level != 0) level = 1;
#وإلا
    إذا (level == Z_DEFAULT_COMPRESSION) level = 6;
#جيڪڏهن_پورو
    إذا (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        أرجع Z_وهڪرو_ERROR;
    }
    func = configuration_جدول[s->level].func;

    إذا ((strategy != s->strategy || func != configuration_جدول[level].func) &&
        s->آخري_flush != -2) {
        /* Flush the last buffer: */
        صحيح err = deflate(strm, Z_بلاڪ);
        إذا (err == Z_وهڪرو_ERROR)
            أرجع err;
        إذا (strm->avail_اندريون || (s->strstart - s->بلاڪ_شروعات) + s->lookahead)
            أرجع Z_BUF_ERROR;
    }
    إذا (s->level != level) {
        إذا (s->level == 0 && s->matches != 0) {
            إذا (s->matches == 1)
                slide_hash(s);
            وإلا
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_ميلاپ   = configuration_جدول[level].max_lazy;
        s->good_ميلاپ       = configuration_جدول[level].good_ڊيگهائي;
        s->nice_ميلاپ       = configuration_جدول[level].nice_ڊيگهائي;
        s->max_chain_ڊيگهائي = configuration_جدول[level].max_chain;
    }
    s->strategy = strategy;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateTune(z_streamp strm, صحيح good_ڊيگهائي, صحيح max_lazy,
                        صحيح nice_ڊيگهائي, صحيح max_chain) {
    deflate_state *s;

    إذا (deflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;
    s = strm->state;
    s->good_ميلاپ = (uInt)good_ڊيگهائي;
    s->max_lazy_ميلاپ = (uInt)max_lazy;
    s->nice_ميلاپ = nice_ڊيگهائي;
    s->max_chain_ڊيگهائي = (uInt)max_chain;
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
z_سائيز_t ZEXPORT deflateBound_z(z_streamp strm, z_سائيز_t ذريعو_Len) {
    deflate_state *s;
    z_سائيز_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = ذريعو_Len + (ذريعو_Len >> 3) + (ذريعو_Len >> 8) +
               (ذريعو_Len >> 9) + 4;
    إذا (fixedlen < ذريعو_Len)
        fixedlen = (z_سائيز_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = ذريعو_Len + (ذريعو_Len >> 5) + (ذريعو_Len >> 7) +
               (ذريعو_Len >> 11) + 7;
    إذا (storelen < ذريعو_Len)
        storelen = (z_سائيز_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    إذا (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        أرجع bound + 18 < bound ? (z_سائيز_t)-1 : bound + 18;
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
#جيڪڏهن_وصف_ٿيل GZIP
    حالة 2:                                 /* gzip wrapper */
        wraplen = 18;
        إذا (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            إذا (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            إذا (str != Z_NULL)
                افعل {
                    wraplen++;
                } طالما (*str++);
            str = s->gzhead->comment;
            إذا (str != Z_NULL)
                افعل {
                    wraplen++;
                } طالما (*str++);
            إذا (s->gzhead->hcrc)
                wraplen += 2;
        }
        اكسر;
#جيڪڏهن_پورو
    افتراضي:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    إذا (s->w_بٽ != 15 || s->hash_بٽ != 8 + 7) {
        bound = s->w_بٽ <= s->hash_بٽ && s->level ? fixedlen :
                                                        storelen;
        أرجع bound + wraplen < bound ? (z_سائيز_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = ذريعو_Len + (ذريعو_Len >> 12) + (ذريعو_Len >> 14) +
            (ذريعو_Len >> 25) + 13 - 6 + wraplen;
    أرجع bound < ذريعو_Len ? (z_سائيز_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong ذريعو_Len) {
    z_سائيز_t bound = deflateBound_z(strm, ذريعو_Len);
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
    بنانشان len;
    deflate_state *s = strm->state;

    tr_flush_بٽ(s);
    len = s->pending > strm->avail_ٻاهريون ? strm->avail_ٻاهريون :
                                         (بنانشان)s->pending;
    إذا (len == 0) أرجع;

    zmemcpy(strm->ايندڙ_ٻاهريون, s->pending_ٻاهريون, len);
    strm->ايندڙ_ٻاهريون  += len;
    s->pending_ٻاهريون  += len;
    strm->ڪل_ٻاهريون += len;
    strm->avail_ٻاهريون -= len;
    s->pending      -= len;
    إذا (s->pending == 0) {
        s->pending_ٻاهريون = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#وصف_ڪريو HCRC_UPDATE(beg) \
    افعل { \
        إذا (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = چڪري_جاچ_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } طالما (0)

/* ========================================================================= */
صحيح ZEXPORT deflate(z_streamp strm, صحيح flush) {
    صحيح old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    إذا (deflateStateCheck(strm) || flush > Z_بلاڪ || flush < 0) {
        أرجع Z_وهڪرو_ERROR;
    }
    s = strm->state;

    إذا (strm->ايندڙ_ٻاهريون == Z_NULL ||
        (strm->avail_اندريون != 0 && strm->ايندڙ_اندريون == Z_NULL) ||
        (s->حالت == پڄاڻي_STATE && flush != Z_پڄاڻي)) {
        ERR_RETURN(strm, Z_وهڪرو_ERROR);
    }
    إذا (strm->avail_ٻاهريون == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->آخري_flush;
    s->آخري_flush = flush;

    /* Flush as much pending output as possible */
    إذا (s->pending != 0) {
        flush_pending(strm);
        إذا (strm->avail_ٻاهريون == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->آخري_flush = -1;
            أرجع Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } وإلا إذا (strm->avail_اندريون == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_پڄاڻي) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    إذا (s->حالت == پڄاڻي_STATE && strm->avail_اندريون != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    إذا (s->حالت == INIT_STATE && s->wrap == 0)
        s->حالت = BUSY_STATE;
    إذا (s->حالت == INIT_STATE) {
        /* zlib header */
        uInt سرو = (Z_DEFLATED + ((s->w_بٽ - 8) << 4)) << 8;
        uInt level_flags;

        إذا (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        وإلا إذا (s->level < 6)
            level_flags = 1;
        وإلا إذا (s->level == 6)
            level_flags = 2;
        وإلا
            level_flags = 3;
        سرو |= (level_flags << 6);
        إذا (s->strstart != 0) سرو |= PRESET_DICT;
        سرو += 31 - (سرو % 31);

        putShortMSB(s, سرو);

        /* Save the adler32 of the preset dictionary: */
        إذا (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->حالت = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        إذا (s->pending != 0) {
            s->آخري_flush = -1;
            أرجع Z_OK;
        }
    }
#جيڪڏهن_وصف_ٿيل GZIP
    إذا (s->حالت == GZIP_STATE) {
        /* gzip header */
        strm->adler = چڪري_جاچ_32_2(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        إذا (s->gzhead == Z_NULL) {
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, OS_ڪوڊ);
            s->حالت = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            إذا (s->pending != 0) {
                s->آخري_flush = -1;
                أرجع Z_OK;
            }
        }
        وإلا {
            put_byte(s, (s->gzhead->متن ? 1 : 0) +
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
            إذا (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            إذا (s->gzhead->hcrc)
                strm->adler = چڪري_جاچ_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->حالت = EXTRA_STATE;
        }
    }
    إذا (s->حالت == EXTRA_STATE) {
        إذا (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg کاٻو = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            طالما (s->pending + کاٻو > s->pending_buf_سائيز) {
                ulg copy = s->pending_buf_سائيز - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_سائيز;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                إذا (s->pending != 0) {
                    s->آخري_flush = -1;
                    أرجع Z_OK;
                }
                beg = 0;
                کاٻو -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, کاٻو);
            s->pending += کاٻو;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->حالت = NAME_STATE;
    }
    إذا (s->حالت == NAME_STATE) {
        إذا (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            صحيح val;
            افعل {
                إذا (s->pending == s->pending_buf_سائيز) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    إذا (s->pending != 0) {
                        s->آخري_flush = -1;
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
        s->حالت = COMMENT_STATE;
    }
    إذا (s->حالت == COMMENT_STATE) {
        إذا (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            صحيح val;
            افعل {
                إذا (s->pending == s->pending_buf_سائيز) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    إذا (s->pending != 0) {
                        s->آخري_flush = -1;
                        أرجع Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } طالما (val != 0);
            HCRC_UPDATE(beg);
        }
        s->حالت = HCRC_STATE;
    }
    إذا (s->حالت == HCRC_STATE) {
        إذا (s->gzhead->hcrc) {
            إذا (s->pending + 2 > s->pending_buf_سائيز) {
                flush_pending(strm);
                إذا (s->pending != 0) {
                    s->آخري_flush = -1;
                    أرجع Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = چڪري_جاچ_32_2(0L, Z_NULL, 0);
        }
        s->حالت = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        إذا (s->pending != 0) {
            s->آخري_flush = -1;
            أرجع Z_OK;
        }
    }
#جيڪڏهن_پورو

    /* Start a new block or continue the current one.
     */
    إذا (strm->avail_اندريون != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->حالت != پڄاڻي_STATE)) {
        بلاڪ_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_جدول[s->level].func))(s, flush);

        إذا (bstate == پڄاڻي_started || bstate == پڄاڻي_پورو) {
            s->حالت = پڄاڻي_STATE;
        }
        إذا (bstate == need_وڌيڪ || bstate == پڄاڻي_started) {
            إذا (strm->avail_ٻاهريون == 0) {
                s->آخري_flush = -1; /* avoid BUF_ERROR next call, see above */
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
        إذا (bstate == بلاڪ_پورو) {
            إذا (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } وإلا إذا (flush != Z_بلاڪ) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_بلاڪ(s, (حرف*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                إذا (flush == Z_ڀريل_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    إذا (s->lookahead == 0) {
                        s->strstart = 0;
                        s->بلاڪ_شروعات = 0L;
                        s->وجهو = 0;
                    }
                }
            }
            flush_pending(strm);
            إذا (strm->avail_ٻاهريون == 0) {
              s->آخري_flush = -1; /* avoid BUF_ERROR at next call, see above */
              أرجع Z_OK;
            }
        }
    }

    إذا (flush != Z_پڄاڻي) أرجع Z_OK;
    إذا (s->wrap <= 0) أرجع Z_وهڪرو_پڇاڙي;

    /* Write the trailer */
#جيڪڏهن_وصف_ٿيل GZIP
    إذا (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->ڪل_اندريون & 0xff));
        put_byte(s, (Byte)((strm->ڪل_اندريون >> 8) & 0xff));
        put_byte(s, (Byte)((strm->ڪل_اندريون >> 16) & 0xff));
        put_byte(s, (Byte)((strm->ڪل_اندريون >> 24) & 0xff));
    }
    وإلا
#جيڪڏهن_پورو
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    إذا (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    أرجع s->pending != 0 ? Z_OK : Z_وهڪرو_پڇاڙي;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_پڇاڙي(z_streamp strm) {
    صحيح حالت;

    إذا (deflateStateCheck(strm)) أرجع Z_وهڪرو_ERROR;

    حالت = strm->state->حالت;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->سر);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->دري);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    أرجع حالت == BUSY_STATE ? Z_معلومات_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
صحيح ZEXPORT deflateCopy(z_streamp dest, z_streamp ذريعو) {
#جيڪڏهن_وصف_ٿيل MAXSEG_64K
    (فراغ)dest;
    (فراغ)ذريعو;
    أرجع Z_وهڪرو_ERROR;
#وإلا
    deflate_state *ds;
    deflate_state *ss;


    إذا (deflateStateCheck(ذريعو) || dest == Z_NULL) {
        أرجع Z_وهڪرو_ERROR;
    }

    ss = ذريعو->state;

    zmemcpy(dest, ذريعو, حجم(z_وهڪرو));

    ds = (deflate_state *) ZALLOC(dest, 1, حجم(deflate_state));
    إذا (ds == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(ds, حجم(deflate_state));
    dest->state = (بنية اندروني_state FAR *) ds;
    zmemcpy(ds, ss, حجم(deflate_state));
    ds->strm = dest;

    ds->دري = (Bytef *) ZALLOC(dest, ds->w_سائيز, 2*حجم(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_سائيز, حجم(Pos));
    ds->سر   = (Posf *)  ZALLOC(dest, ds->hash_سائيز, حجم(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    إذا (ds->دري == Z_NULL || ds->prev == Z_NULL || ds->سر == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_پڇاڙي (dest);
        أرجع Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->دري, ss->دري, ss->مٿاهون_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->وجهو > ds->w_سائيز ? ds->w_سائيز :
                ss->strstart - ss->وجهو) * حجم(Pos));
    zmemcpy(ds->سر, ss->سر, ds->hash_سائيز * حجم(Pos));

    ds->pending_ٻاهريون = ds->pending_buf + (ss->pending_ٻاهريون - ss->pending_buf);
    zmemcpy(ds->pending_ٻاهريون, ss->pending_ٻاهريون, ss->pending);
#جيڪڏهن_وصف_ٿيل LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_ايندڙ * حجم(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_ايندڙ);
#وإلا
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_ايندڙ);
#جيڪڏهن_پورو

    ds->l_desc.dyn_وڻ = ds->dyn_ltree;
    ds->d_desc.dyn_وڻ = ds->dyn_dtree;
    ds->bl_desc.dyn_وڻ = ds->bl_وڻ;

    أرجع Z_OK;
#جيڪڏهن_پورو /* MAXSEG_64K */
}

#جيڪڏهن_وصف_نه_ٿيل FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_ميلاپ(deflate_state *s, IPos cur_ميلاپ) {
    بنانشان chain_ڊيگهائي = s->max_chain_ڊيگهائي;/* max hash chain length */
    Bytef *جاچ = s->دري + s->strstart;      /* current string */
    Bytef *ميلاپ;                               /* matched string */
    صحيح len;                                    /* length of current match */
    صحيح بهترين_len = (صحيح)s->prev_ڊيگهائي;         /* best match length so far */
    صحيح nice_ميلاپ = s->nice_ميلاپ;             /* stop if match long enough */
    IPos حد_بندي = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_نقاب;

#جيڪڏهن_وصف_ٿيل UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->دري + s->strstart + MAX_ميلاپ - 1;
    ush جاچ_شروعات = *(ushf*)جاچ;
    ush جاچ_پڇاڙي   = *(ushf*)(جاچ + بهترين_len - 1);
#وإلا
    Bytef *strend = s->دري + s->strstart + MAX_ميلاپ;
    Byte جاچ_end1  = جاچ[بهترين_len - 1];
    Byte جاچ_پڇاڙي   = جاچ[بهترين_len];
#جيڪڏهن_پورو

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بٽ >= 8 && MAX_ميلاپ == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    إذا (s->prev_ڊيگهائي >= s->good_ميلاپ) {
        chain_ڊيگهائي >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    إذا ((uInt)nice_ميلاپ > s->lookahead) nice_ميلاپ = (صحيح)s->lookahead;

    Assert((ulg)s->strstart <= s->دري_سائيز - MIN_LOOKAHEAD,
           "need lookahead");

    افعل {
        Assert(cur_ميلاپ < s->strstart, "no future");
        ميلاپ = s->دري + cur_ميلاپ;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#إذا (defined(UNALIGNED_OK) && MAX_ميلاپ == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        إذا (*(ushf*)(ميلاپ + بهترين_len - 1) != جاچ_پڇاڙي ||
            *(ushf*)ميلاپ != جاچ_شروعات) استمر;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(جاچ[2] == ميلاپ[2], "scan[2]?");
        جاچ++, ميلاپ++;
        افعل {
        } طالما (*(ushf*)(جاچ += 2) == *(ushf*)(ميلاپ += 2) &&
                 *(ushf*)(جاچ += 2) == *(ushf*)(ميلاپ += 2) &&
                 *(ushf*)(جاچ += 2) == *(ushf*)(ميلاپ += 2) &&
                 *(ushf*)(جاچ += 2) == *(ushf*)(ميلاپ += 2) &&
                 جاچ < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(جاچ <= s->دري + (بنانشان)(s->دري_سائيز - 1),
               "wild scan");
        إذا (*جاچ == *ميلاپ) جاچ++;

        len = (MAX_ميلاپ - 1) - (صحيح)(strend - جاچ);
        جاچ = strend - (MAX_ميلاپ-1);

#وإلا /* UNALIGNED_OK */

        إذا (ميلاپ[بهترين_len]     != جاچ_پڇاڙي  ||
            ميلاپ[بهترين_len - 1] != جاچ_end1 ||
            *ميلاپ              != *جاچ     ||
            *++ميلاپ            != جاچ[1])      استمر;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        جاچ += 2, ميلاپ++;
        Assert(*جاچ == *ميلاپ, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        افعل {
        } طالما (*++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
                 *++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
                 *++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
                 *++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
                 جاچ < strend);

        Assert(جاچ <= s->دري + (بنانشان)(s->دري_سائيز - 1),
               "wild scan");

        len = MAX_ميلاپ - (صحيح)(strend - جاچ);
        جاچ = strend - MAX_ميلاپ;

#جيڪڏهن_پورو /* UNALIGNED_OK */

        إذا (len > بهترين_len) {
            s->ميلاپ_شروعات = cur_ميلاپ;
            بهترين_len = len;
            إذا (len >= nice_ميلاپ) اكسر;
#جيڪڏهن_وصف_ٿيل UNALIGNED_OK
            جاچ_پڇاڙي = *(ushf*)(جاچ + بهترين_len - 1);
#وإلا
            جاچ_end1  = جاچ[بهترين_len - 1];
            جاچ_پڇاڙي   = جاچ[بهترين_len];
#جيڪڏهن_پورو
        }
    } طالما ((cur_ميلاپ = prev[cur_ميلاپ & wmask]) > حد_بندي
             && --chain_ڊيگهائي != 0);

    إذا ((uInt)بهترين_len <= s->lookahead) أرجع (uInt)بهترين_len;
    أرجع s->lookahead;
}

#وإلا /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_ميلاپ(deflate_state *s, IPos cur_ميلاپ) {
    Bytef *جاچ = s->دري + s->strstart;      /* current string */
    Bytef *ميلاپ;                               /* matched string */
    صحيح len;                                    /* length of current match */
    Bytef *strend = s->دري + s->strstart + MAX_ميلاپ;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بٽ >= 8 && MAX_ميلاپ == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->دري_سائيز - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_ميلاپ < s->strstart, "no future");

    ميلاپ = s->دري + cur_ميلاپ;

    /* Return failure if the match length is less than 2:
     */
    إذا (ميلاپ[0] != جاچ[0] || ميلاپ[1] != جاچ[1]) أرجع MIN_ميلاپ-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    جاچ += 2, ميلاپ += 2;
    Assert(*جاچ == *ميلاپ, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    افعل {
    } طالما (*++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
             *++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
             *++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
             *++جاچ == *++ميلاپ && *++جاچ == *++ميلاپ &&
             جاچ < strend);

    Assert(جاچ <= s->دري + (بنانشان)(s->دري_سائيز - 1), "wild scan");

    len = MAX_ميلاپ - (صحيح)(strend - جاچ);

    إذا (len < MIN_ميلاپ) أرجع MIN_ميلاپ - 1;

    s->ميلاپ_شروعات = cur_ميلاپ;
    أرجع (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#جيڪڏهن_پورو /* FASTEST */

#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG

#وصف_ڪريو EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local فراغ check_ميلاپ(deflate_state *s, IPos شروعات, IPos ميلاپ, صحيح ڊيگهائي) {
    /* check that the match is indeed a match */
    Bytef *پٺ = s->دري + (صحيح)ميلاپ, *هتي = s->دري + شروعات;
    IPos len = (IPos)ڊيگهائي;
    إذا (ميلاپ == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        پٺ++;
        هتي++;
        len--;
    }
    إذا (zmemcmp(پٺ, هتي, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                شروعات, (صحيح)ميلاپ, ڊيگهائي);
        افعل {
            fprintf(stderr, "(%02x %02x)", *پٺ++, *هتي++);
        } طالما (--len != 0);
        z_error("invalid match");
    }
    إذا (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", شروعات - ميلاپ, ڊيگهائي);
        افعل { putc(s->دري[شروعات++], stderr); } طالما (--ڊيگهائي != 0);
    }
}
#وإلا
#  define check_ميلاپ(s, شروعات, ميلاپ, ڊيگهائي)
#جيڪڏهن_پورو /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#وصف_ڪريو FLUSH_بلاڪ_ONLY(s, آخري) { \
   tr_flush_بلاڪ(s, (s->بلاڪ_شروعات >= 0L ? \
                   (charf *)&s->دري[(بنانشان)s->بلاڪ_شروعات] : \
                   (charf *)Z_NULL), \
                (ulg)((ڊگهو)s->strstart - s->بلاڪ_شروعات), \
                (آخري)); \
   s->بلاڪ_شروعات = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#وصف_ڪريو FLUSH_بلاڪ(s, آخري) { \
   FLUSH_بلاڪ_ONLY(s, آخري); \
   إذا (s->strm->avail_ٻاهريون == 0) أرجع (آخري) ? پڄاڻي_started : need_وڌيڪ; \
}

/* Maximum stored block length in deflate format (not including header). */
#وصف_ڪريو MAX_STORED 65535

/* Minimum of a and b. */
#وصف_ڪريو MIN(a, b) ((a) > (b) ? (b) : (a))

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
local بلاڪ_state deflate_stored(deflate_state *s, صحيح flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    بنانشان min_بلاڪ = (بنانشان)(MIN(s->pending_buf_سائيز - 5, s->w_سائيز));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    صحيح آخري = 0;
    بنانشان len, کاٻو, have;
    بنانشان استعمال_ٿيل = s->strm->avail_اندريون;
    افعل {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((بنانشان)s->bi_valid + 42) >> 3;   /* bytes in header */
        إذا (s->strm->avail_ٻاهريون < have)          /* need room for header */
            اكسر;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_ٻاهريون - have;
        کاٻو = (بنانشان)(s->strstart - s->بلاڪ_شروعات);    /* window bytes */
        إذا (len > (ulg)کاٻو + s->strm->avail_اندريون)
            len = کاٻو + s->strm->avail_اندريون;     /* limit len to the input */
        إذا (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        إذا (len < min_بلاڪ && ((len == 0 && flush != Z_پڄاڻي) ||
                                flush == Z_NO_FLUSH ||
                                len != کاٻو + s->strm->avail_اندريون))
            اكسر;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        آخري = flush == Z_پڄاڻي && len == کاٻو + s->strm->avail_اندريون ? 1 : 0;
        tr_stored_بلاڪ(s, (حرف *)0, 0L, آخري);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->بٽ_موڪليل += len << 3;
#جيڪڏهن_پورو

        /* Copy uncompressed bytes from the window to next_out. */
        إذا (کاٻو) {
            إذا (کاٻو > len)
                کاٻو = len;
            zmemcpy(s->strm->ايندڙ_ٻاهريون, s->دري + s->بلاڪ_شروعات, کاٻو);
            s->strm->ايندڙ_ٻاهريون += کاٻو;
            s->strm->avail_ٻاهريون -= کاٻو;
            s->strm->ڪل_ٻاهريون += کاٻو;
            s->بلاڪ_شروعات += کاٻو;
            len -= کاٻو;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        إذا (len) {
            read_buf(s->strm, s->strm->ايندڙ_ٻاهريون, len);
            s->strm->ايندڙ_ٻاهريون += len;
            s->strm->avail_ٻاهريون -= len;
            s->strm->ڪل_ٻاهريون += len;
        }
    } طالما (آخري == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    استعمال_ٿيل -= s->strm->avail_اندريون;      /* number of input bytes directly copied */
    إذا (استعمال_ٿيل) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        إذا (استعمال_ٿيل >= s->w_سائيز) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->دري, s->strm->ايندڙ_اندريون - s->w_سائيز, s->w_سائيز);
            s->strstart = s->w_سائيز;
            s->وجهو = s->strstart;
        }
        وإلا {
            إذا (s->دري_سائيز - s->strstart <= استعمال_ٿيل) {
                /* Slide the window down. */
                s->strstart -= s->w_سائيز;
                zmemcpy(s->دري, s->دري + s->w_سائيز, s->strstart);
                إذا (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                إذا (s->وجهو > s->strstart)
                    s->وجهو = s->strstart;
            }
            zmemcpy(s->دري + s->strstart, s->strm->ايندڙ_اندريون - استعمال_ٿيل, استعمال_ٿيل);
            s->strstart += استعمال_ٿيل;
            s->وجهو += MIN(استعمال_ٿيل, s->w_سائيز - s->وجهو);
        }
        s->بلاڪ_شروعات = s->strstart;
    }
    إذا (s->مٿاهون_water < s->strstart)
        s->مٿاهون_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    إذا (آخري) {
        s->bi_استعمال_ٿيل = 8;
        أرجع پڄاڻي_پورو;
    }

    /* If flushing and all input has been consumed, then done. */
    إذا (flush != Z_NO_FLUSH && flush != Z_پڄاڻي &&
        s->strm->avail_اندريون == 0 && (ڊگهو)s->strstart == s->بلاڪ_شروعات)
        أرجع بلاڪ_پورو;

    /* Fill the window with any remaining input. */
    have = (بنانشان)(s->دري_سائيز - s->strstart);
    إذا (s->strm->avail_اندريون > have && s->بلاڪ_شروعات >= (ڊگهو)s->w_سائيز) {
        /* Slide the window down. */
        s->بلاڪ_شروعات -= s->w_سائيز;
        s->strstart -= s->w_سائيز;
        zmemcpy(s->دري, s->دري + s->w_سائيز, s->strstart);
        إذا (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_سائيز;          /* more space now */
        إذا (s->وجهو > s->strstart)
            s->وجهو = s->strstart;
    }
    إذا (have > s->strm->avail_اندريون)
        have = s->strm->avail_اندريون;
    إذا (have) {
        read_buf(s->strm, s->دري + s->strstart, have);
        s->strstart += have;
        s->وجهو += MIN(have, s->w_سائيز - s->وجهو);
    }
    إذا (s->مٿاهون_water < s->strstart)
        s->مٿاهون_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((بنانشان)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (بنانشان)MIN(s->pending_buf_سائيز - have, MAX_STORED);
    min_بلاڪ = MIN(have, s->w_سائيز);
    کاٻو = (بنانشان)(s->strstart - s->بلاڪ_شروعات);
    إذا (کاٻو >= min_بلاڪ ||
        ((کاٻو || flush == Z_پڄاڻي) && flush != Z_NO_FLUSH &&
         s->strm->avail_اندريون == 0 && کاٻو <= have)) {
        len = MIN(کاٻو, have);
        آخري = flush == Z_پڄاڻي && s->strm->avail_اندريون == 0 &&
               len == کاٻو ? 1 : 0;
        tr_stored_بلاڪ(s, (charf *)s->دري + s->بلاڪ_شروعات, len, آخري);
        s->بلاڪ_شروعات += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    إذا (آخري)
        s->bi_استعمال_ٿيل = 8;
    أرجع آخري ? پڄاڻي_started : need_وڌيڪ;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local بلاڪ_state deflate_fast(deflate_state *s, صحيح flush) {
    IPos hash_سر;       /* head of the hash chain */
    صحيح bflush;           /* set if current block must be flushed */

    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        إذا (s->lookahead < MIN_LOOKAHEAD) {
            fill_دري(s);
            إذا (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_وڌيڪ;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        إذا (s->lookahead >= MIN_ميلاپ) {
            وجهو_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        إذا (hash_سر != NIL && s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->ميلاپ_ڊيگهائي = longest_ميلاپ (s, hash_سر);
            /* longest_match() sets match_start */
        }
        إذا (s->ميلاپ_ڊيگهائي >= MIN_ميلاپ) {
            check_ميلاپ(s, s->strstart, s->ميلاپ_شروعات, (صحيح)s->ميلاپ_ڊيگهائي);

            _tr_tally_dist(s, s->strstart - s->ميلاپ_شروعات,
                           s->ميلاپ_ڊيگهائي - MIN_ميلاپ, bflush);

            s->lookahead -= s->ميلاپ_ڊيگهائي;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#جيڪڏهن_وصف_نه_ٿيل FASTEST
            إذا (s->ميلاپ_ڊيگهائي <= s->max_وجهو_ڊيگهائي &&
                s->lookahead >= MIN_ميلاپ) {
                s->ميلاپ_ڊيگهائي--; /* string at strstart already in table */
                افعل {
                    s->strstart++;
                    وجهو_STRING(s, s->strstart, hash_سر);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } طالما (--s->ميلاپ_ڊيگهائي != 0);
                s->strstart++;
            } وإلا
#جيڪڏهن_پورو
            {
                s->strstart += s->ميلاپ_ڊيگهائي;
                s->ميلاپ_ڊيگهائي = 0;
                s->ins_h = s->دري[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->دري[s->strstart + 1]);
#إذا MIN_ميلاپ != 3
                Call UPDATE_HASH() MIN_ميلاپ-3 وڌيڪ times
#جيڪڏهن_پورو
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->دري[s->strstart]));
            _tr_tally_lit(s, s->دري[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        إذا (bflush) FLUSH_بلاڪ(s, 0);
    }
    s->وجهو = s->strstart < MIN_ميلاپ-1 ? s->strstart : MIN_ميلاپ-1;
    إذا (flush == Z_پڄاڻي) {
        FLUSH_بلاڪ(s, 1);
        أرجع پڄاڻي_پورو;
    }
    إذا (s->sym_ايندڙ)
        FLUSH_بلاڪ(s, 0);
    أرجع بلاڪ_پورو;
}

#جيڪڏهن_وصف_نه_ٿيل FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local بلاڪ_state deflate_slow(deflate_state *s, صحيح flush) {
    IPos hash_سر;          /* head of hash chain */
    صحيح bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        إذا (s->lookahead < MIN_LOOKAHEAD) {
            fill_دري(s);
            إذا (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_وڌيڪ;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        إذا (s->lookahead >= MIN_ميلاپ) {
            وجهو_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_ڊيگهائي = s->ميلاپ_ڊيگهائي, s->prev_ميلاپ = s->ميلاپ_شروعات;
        s->ميلاپ_ڊيگهائي = MIN_ميلاپ-1;

        إذا (hash_سر != NIL && s->prev_ڊيگهائي < s->max_lazy_ميلاپ &&
            s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->ميلاپ_ڊيگهائي = longest_ميلاپ (s, hash_سر);
            /* longest_match() sets match_start */

            إذا (s->ميلاپ_ڊيگهائي <= 5 && (s->strategy == Z_FILTERED
#إذا TOO_FAR <= 32767
                || (s->ميلاپ_ڊيگهائي == MIN_ميلاپ &&
                    s->strstart - s->ميلاپ_شروعات > TOO_FAR)
#جيڪڏهن_پورو
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->ميلاپ_ڊيگهائي = MIN_ميلاپ-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        إذا (s->prev_ڊيگهائي >= MIN_ميلاپ && s->ميلاپ_ڊيگهائي <= s->prev_ڊيگهائي) {
            uInt max_وجهو = s->strstart + s->lookahead - MIN_ميلاپ;
            /* Do not insert strings in hash table beyond this. */

            check_ميلاپ(s, s->strstart - 1, s->prev_ميلاپ, (صحيح)s->prev_ڊيگهائي);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_ميلاپ,
                           s->prev_ڊيگهائي - MIN_ميلاپ, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_ڊيگهائي - 1;
            s->prev_ڊيگهائي -= 2;
            افعل {
                إذا (++s->strstart <= max_وجهو) {
                    وجهو_STRING(s, s->strstart, hash_سر);
                }
            } طالما (--s->prev_ڊيگهائي != 0);
            s->ميلاپ_موجود = 0;
            s->ميلاپ_ڊيگهائي = MIN_ميلاپ-1;
            s->strstart++;

            إذا (bflush) FLUSH_بلاڪ(s, 0);

        } وإلا إذا (s->ميلاپ_موجود) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->دري[s->strstart - 1]));
            _tr_tally_lit(s, s->دري[s->strstart - 1], bflush);
            إذا (bflush) {
                FLUSH_بلاڪ_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            إذا (s->strm->avail_ٻاهريون == 0) أرجع need_وڌيڪ;
        } وإلا {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->ميلاپ_موجود = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    إذا (s->ميلاپ_موجود) {
        Tracevv((stderr,"%c", s->دري[s->strstart - 1]));
        _tr_tally_lit(s, s->دري[s->strstart - 1], bflush);
        s->ميلاپ_موجود = 0;
    }
    s->وجهو = s->strstart < MIN_ميلاپ-1 ? s->strstart : MIN_ميلاپ-1;
    إذا (flush == Z_پڄاڻي) {
        FLUSH_بلاڪ(s, 1);
        أرجع پڄاڻي_پورو;
    }
    إذا (s->sym_ايندڙ)
        FLUSH_بلاڪ(s, 0);
    أرجع بلاڪ_پورو;
}
#جيڪڏهن_پورو /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local بلاڪ_state deflate_rle(deflate_state *s, صحيح flush) {
    صحيح bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *جاچ, *strend;   /* scan goes up to strend for length of run */

    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        إذا (s->lookahead <= MAX_ميلاپ) {
            fill_دري(s);
            إذا (s->lookahead <= MAX_ميلاپ && flush == Z_NO_FLUSH) {
                أرجع need_وڌيڪ;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->ميلاپ_ڊيگهائي = 0;
        إذا (s->lookahead >= MIN_ميلاپ && s->strstart > 0) {
            جاچ = s->دري + s->strstart - 1;
            prev = *جاچ;
            إذا (prev == *++جاچ && prev == *++جاچ && prev == *++جاچ) {
                strend = s->دري + s->strstart + MAX_ميلاپ;
                افعل {
                } طالما (prev == *++جاچ && prev == *++جاچ &&
                         prev == *++جاچ && prev == *++جاچ &&
                         prev == *++جاچ && prev == *++جاچ &&
                         prev == *++جاچ && prev == *++جاچ &&
                         جاچ < strend);
                s->ميلاپ_ڊيگهائي = MAX_ميلاپ - (uInt)(strend - جاچ);
                إذا (s->ميلاپ_ڊيگهائي > s->lookahead)
                    s->ميلاپ_ڊيگهائي = s->lookahead;
            }
            Assert(جاچ <= s->دري + (uInt)(s->دري_سائيز - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        إذا (s->ميلاپ_ڊيگهائي >= MIN_ميلاپ) {
            check_ميلاپ(s, s->strstart, s->strstart - 1, (صحيح)s->ميلاپ_ڊيگهائي);

            _tr_tally_dist(s, 1, s->ميلاپ_ڊيگهائي - MIN_ميلاپ, bflush);

            s->lookahead -= s->ميلاپ_ڊيگهائي;
            s->strstart += s->ميلاپ_ڊيگهائي;
            s->ميلاپ_ڊيگهائي = 0;
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->دري[s->strstart]));
            _tr_tally_lit(s, s->دري[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        إذا (bflush) FLUSH_بلاڪ(s, 0);
    }
    s->وجهو = 0;
    إذا (flush == Z_پڄاڻي) {
        FLUSH_بلاڪ(s, 1);
        أرجع پڄاڻي_پورو;
    }
    إذا (s->sym_ايندڙ)
        FLUSH_بلاڪ(s, 0);
    أرجع بلاڪ_پورو;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local بلاڪ_state deflate_huff(deflate_state *s, صحيح flush) {
    صحيح bflush;             /* set if current block must be flushed */

    لكل (;;) {
        /* Make sure that we have a literal to write. */
        إذا (s->lookahead == 0) {
            fill_دري(s);
            إذا (s->lookahead == 0) {
                إذا (flush == Z_NO_FLUSH)
                    أرجع need_وڌيڪ;
                اكسر;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->ميلاپ_ڊيگهائي = 0;
        Tracevv((stderr,"%c", s->دري[s->strstart]));
        _tr_tally_lit(s, s->دري[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        إذا (bflush) FLUSH_بلاڪ(s, 0);
    }
    s->وجهو = 0;
    إذا (flush == Z_پڄاڻي) {
        FLUSH_بلاڪ(s, 1);
        أرجع پڄاڻي_پورو;
    }
    إذا (s->sym_ايندڙ)
        FLUSH_بلاڪ(s, 0);
    أرجع بلاڪ_پورو;
}
