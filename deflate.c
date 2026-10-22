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
    need_ہور,      /* block not completed, need more input or more output */
    بلاک_پورا,     /* block flush performed */
    انت_started, /* finish started, need only more output at next deflate */
    انت_پورا     /* finish done, accept no more input or output */
} بلاک_state;

تعريفنوع بلاک_state (*compress_func)(deflate_state *s, صحيح flush);
/* Compression function. Returns the block state after the call. */

local بلاک_state deflate_stored(deflate_state *s, صحيح flush);
local بلاک_state deflate_fast(deflate_state *s, صحيح flush);
#جے_بنا_تعریف FASTEST
local بلاک_state deflate_slow(deflate_state *s, صحيح flush);
#جے_انت
local بلاک_state deflate_rle(deflate_state *s, صحيح flush);
local بلاک_state deflate_huff(deflate_state *s, صحيح flush);

/* ===========================================================================
 * Local data
 */

#تعریف_کرو NIL 0
/* Tail of hash chains */

#جے_بنا_تعریف TOO_FAR
#  define TOO_FAR 4096
#جے_انت
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
تعريفنوع بنية config_s {
   ush good_لمائی; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_لمائی; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#جے_تعریف_شدہ FASTEST
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
#جے_انت

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#تعریف_کرو RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#تعریف_کرو UPDATE_HASH(s,h,c) (h = (((h) << s->hash_کھسکاؤ) ^ (c)) & s->hash_نقاب)


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
#جے_تعریف_شدہ FASTEST
#تعریف_کرو داخل_کرو_STRING(s, str, میل_سر) \
   (UPDATE_HASH(s, s->ins_h, s->کھڑکی[(str) + (MIN_میل-1)]), \
    میل_سر = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#وإلا
#تعریف_کرو داخل_کرو_STRING(s, str, میل_سر) \
   (UPDATE_HASH(s, s->ins_h, s->کھڑکی[(str) + (MIN_میل-1)]), \
    میل_سر = s->prev[(str) & s->w_نقاب] = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#جے_انت

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#تعریف_کرو CLEAR_HASH(s) \
    افعل { \
        s->سر[s->hash_ناپ - 1] = NIL; \
        zmemzero(s->سر, (بنانشانی)(s->hash_ناپ - 1)*حجم(*s->سر)); \
        s->slid = 0; \
    } طالما (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#إذا defined(__has_feature)
#  إذا __has_feature(حافظہ_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#جے_انت
local فراغ slide_hash(deflate_state *s) {
    بنانشانی n, m;
    Posf *p;
    uInt wsize = s->w_ناپ;

    n = s->hash_ناپ;
    p = &s->سر[n];
    افعل {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } طالما (--n);
#جے_بنا_تعریف FASTEST
    n = wsize;
    p = &s->prev[n];
    افعل {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } طالما (--n);
#جے_انت
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local بنانشانی read_buf(z_streamp strm, Bytef *buf, بنانشانی ناپ) {
    بنانشانی len = strm->avail_وچ;

    إذا (len > ناپ) len = ناپ;
    إذا (len == 0) أرجع 0;

    strm->avail_وچ  -= len;

    zmemcpy(buf, strm->اگلا_وچ, len);
    إذا (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#جے_تعریف_شدہ GZIP
    وإلا إذا (strm->state->wrap == 2) {
        strm->adler = چکری_جانچ_32_2(strm->adler, buf, len);
    }
#جے_انت
    strm->اگلا_وچ  += len;
    strm->کل_وچ += len;

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
local فراغ fill_کھڑکی(deflate_state *s) {
    بنانشانی n;
    بنانشانی ہور;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_ناپ;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    افعل {
        ہور = (بنانشانی)(s->کھڑکی_ناپ -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#جے_تعریف_شدہ _MSC_VER
#ہدایت warning(ڈالو)
#ہدایت warning(disable: 4127)
#جے_انت
        إذا (حجم(صحيح) <= 2) {
#جے_تعریف_شدہ _MSC_VER
#ہدایت warning(نکالو)
#جے_انت
            إذا (ہور == 0 && s->strstart == 0 && s->lookahead == 0) {
                ہور = wsize;

            } وإلا إذا (ہور == (بنانشانی)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                ہور--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        إذا (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->کھڑکی, s->کھڑکی + wsize, (بنانشانی)wsize - ہور);
            s->میل_شروع -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->بلاک_شروع -= (لما) wsize;
            إذا (s->داخل_کرو > s->strstart)
                s->داخل_کرو = s->strstart;
            slide_hash(s);
            ہور += wsize;
        }
        إذا (s->strm->avail_وچ == 0) اكسر;

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
        Assert(ہور >= 2, "more < 2");

        n = read_buf(s->strm, s->کھڑکی + s->strstart + s->lookahead, ہور);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        إذا (s->lookahead + s->داخل_کرو >= MIN_میل) {
            uInt str = s->strstart - s->داخل_کرو;
            s->ins_h = s->کھڑکی[str];
            UPDATE_HASH(s, s->ins_h, s->کھڑکی[str + 1]);
#إذا MIN_میل != 3
            Call UPDATE_HASH() MIN_میل-3 ہور times
#جے_انت
            طالما (s->داخل_کرو) {
                UPDATE_HASH(s, s->ins_h, s->کھڑکی[str + MIN_میل-1]);
#جے_بنا_تعریف FASTEST
                s->prev[str & s->w_نقاب] = s->سر[s->ins_h];
#جے_انت
                s->سر[s->ins_h] = (Pos)str;
                str++;
                s->داخل_کرو--;
                إذا (s->lookahead + s->داخل_کرو < MIN_میل)
                    اكسر;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } طالما (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_وچ != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    إذا (s->بلند_water < s->کھڑکی_ناپ) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        إذا (s->بلند_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->کھڑکی_ناپ - curr;
            إذا (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->کھڑکی + curr, (بنانشانی)init);
            s->بلند_water = curr + init;
        }
        وإلا إذا (s->بلند_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->بلند_water;
            إذا (init > s->کھڑکی_ناپ - s->بلند_water)
                init = s->کھڑکی_ناپ - s->بلند_water;
            zmemzero(s->کھڑکی + s->بلند_water, (بنانشانی)init);
            s->بلند_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->کھڑکی_ناپ - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit_(z_streamp strm, صحيح level, ثابت حرف *نسخہ,
                         صحيح دھارا_ناپ) {
    أرجع deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, نسخہ, دھارا_ناپ);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit2_(z_streamp strm, صحيح level, صحيح طریقہ,
                          صحيح کھڑکی_بٹ, صحيح memLevel, صحيح strategy,
                          ثابت حرف *نسخہ, صحيح دھارا_ناپ) {
    deflate_state *s;
    صحيح wrap = 1;
    ساكن ثابت حرف my_نسخہ[] = ZLIB_نسخہ;

    إذا (نسخہ == Z_NULL || نسخہ[0] != my_نسخہ[0] ||
        دھارا_ناپ != حجم(z_دھارا)) {
        أرجع Z_نسخہ_ERROR;
    }
    إذا (strm == Z_NULL) أرجع Z_دھارا_ERROR;

    strm->پیغام = Z_NULL;
    إذا (strm->zalloc == (alloc_func)0) {
#جے_تعریف_شدہ Z_SOLO
        أرجع Z_دھارا_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#جے_انت
    }
    إذا (strm->zfree == (free_func)0)
#جے_تعریف_شدہ Z_SOLO
        أرجع Z_دھارا_ERROR;
#وإلا
        strm->zfree = zcfree;
#جے_انت

#جے_تعریف_شدہ FASTEST
    إذا (level != 0) level = 1;
#وإلا
    إذا (level == Z_DEFAULT_COMPRESSION) level = 6;
#جے_انت

    إذا (کھڑکی_بٹ < 0) { /* suppress zlib wrapper */
        wrap = 0;
        إذا (کھڑکی_بٹ < -15)
            أرجع Z_دھارا_ERROR;
        کھڑکی_بٹ = -کھڑکی_بٹ;
    }
#جے_تعریف_شدہ GZIP
    وإلا إذا (کھڑکی_بٹ > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        کھڑکی_بٹ -= 16;
    }
#جے_انت
    إذا (memLevel < 1 || memLevel > MAX_MEM_LEVEL || طریقہ != Z_DEFLATED ||
        کھڑکی_بٹ < 8 || کھڑکی_بٹ > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (کھڑکی_بٹ == 8 && wrap != 1)) {
        أرجع Z_دھارا_ERROR;
    }
    إذا (کھڑکی_بٹ == 8) کھڑکی_بٹ = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, حجم(deflate_state));
    إذا (s == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(s, حجم(deflate_state));
    strm->state = (بنية اندرونی_state FAR *)s;
    s->strm = strm;
    s->حالت = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_بٹ = (uInt)کھڑکی_بٹ;
    s->w_ناپ = 1 << s->w_بٹ;
    s->w_نقاب = s->w_ناپ - 1;

    s->hash_بٹ = (uInt)memLevel + 7;
    s->hash_ناپ = 1 << s->hash_بٹ;
    s->hash_نقاب = s->hash_ناپ - 1;
    s->hash_کھسکاؤ =  ((s->hash_بٹ + MIN_میل-1) / MIN_میل);

    s->کھڑکی = (Bytef *) ZALLOC(strm, s->w_ناپ, 2*حجم(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_ناپ, حجم(Pos));
    s->سر   = (Posf *)  ZALLOC(strm, s->hash_ناپ, حجم(Pos));

    s->بلند_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_ناپ = (ulg)s->lit_bufsize * 4;

    إذا (s->کھڑکی == Z_NULL || s->prev == Z_NULL || s->سر == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->حالت = انت_STATE;
        strm->پیغام = ERR_پیغام(Z_MEM_ERROR);
        deflate_سرا (strm);
        أرجع Z_MEM_ERROR;
    }
#جے_تعریف_شدہ LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_سرا = s->lit_bufsize - 1;
#وإلا
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_سرا = (s->lit_bufsize - 1) * 3;
#جے_انت
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->طریقہ = (Byte)طریقہ;

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
#جے_تعریف_شدہ GZIP
                                           s->حالت != GZIP_STATE &&
#جے_انت
                                           s->حالت != EXTRA_STATE &&
                                           s->حالت != NAME_STATE &&
                                           s->حالت != COMMENT_STATE &&
                                           s->حالت != HCRC_STATE &&
                                           s->حالت != BUSY_STATE &&
                                           s->حالت != انت_STATE))
        أرجع 1;
    أرجع 0;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_سیٹ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt  dict_لمائی) {
    deflate_state *s;
    uInt str, n;
    صحيح wrap;
    بنانشانی avail;
    z_const بنانشانی حرف *اگلا;

    إذا (deflateStateCheck(strm) || dictionary == Z_NULL)
        أرجع Z_دھارا_ERROR;
    s = strm->state;
    wrap = s->wrap;
    إذا (wrap == 2 || (wrap == 1 && s->حالت != INIT_STATE) || s->lookahead)
        أرجع Z_دھارا_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    إذا (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_لمائی);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    إذا (dict_لمائی >= s->w_ناپ) {
        إذا (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->بلاک_شروع = 0L;
            s->داخل_کرو = 0;
        }
        dictionary += dict_لمائی - s->w_ناپ;  /* use the tail */
        dict_لمائی = s->w_ناپ;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_وچ;
    اگلا = strm->اگلا_وچ;
    strm->avail_وچ = dict_لمائی;
    strm->اگلا_وچ = (z_const Bytef *)dictionary;
    fill_کھڑکی(s);
    طالما (s->lookahead >= MIN_میل) {
        str = s->strstart;
        n = s->lookahead - (MIN_میل-1);
        افعل {
            UPDATE_HASH(s, s->ins_h, s->کھڑکی[str + MIN_میل-1]);
#جے_بنا_تعریف FASTEST
            s->prev[str & s->w_نقاب] = s->سر[s->ins_h];
#جے_انت
            s->سر[s->ins_h] = (Pos)str;
            str++;
        } طالما (--n);
        s->strstart = str;
        s->lookahead = MIN_میل-1;
        fill_کھڑکی(s);
    }
    s->strstart += s->lookahead;
    s->بلاک_شروع = (لما)s->strstart;
    s->داخل_کرو = s->lookahead;
    s->lookahead = 0;
    s->میل_لمائی = s->prev_لمائی = MIN_میل-1;
    s->میل_دستیاب = 0;
    strm->اگلا_وچ = اگلا;
    strm->avail_وچ = avail;
    s->wrap = wrap;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_لمائی) {
    deflate_state *s;
    uInt len;

    إذا (deflateStateCheck(strm))
        أرجع Z_دھارا_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    إذا (len > s->w_ناپ)
        len = s->w_ناپ;
    إذا (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->کھڑکی + s->strstart + s->lookahead - len, len);
    إذا (dict_لمائی != Z_NULL)
        *dict_لمائی = len;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    إذا (deflateStateCheck(strm)) {
        أرجع Z_دھارا_ERROR;
    }

    strm->کل_وچ = strm->کل_باہرلا = 0;
    strm->پیغام = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->انکڑے_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_باہرلا = s->pending_buf;

    إذا (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->حالت =
#جے_تعریف_شدہ GZIP
        s->wrap == 2 ? GZIP_STATE :
#جے_انت
        INIT_STATE;
    strm->adler =
#جے_تعریف_شدہ GZIP
        s->wrap == 2 ? چکری_جانچ_32_2(0L, Z_NULL, 0) :
#جے_انت
        adler32(0L, Z_NULL, 0);
    s->آخری_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    أرجع Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local فراغ lm_init(deflate_state *s) {
    s->کھڑکی_ناپ = (ulg)2L*s->w_ناپ;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_میل   = configuration_جدول[s->level].max_lazy;
    s->good_میل       = configuration_جدول[s->level].good_لمائی;
    s->nice_میل       = configuration_جدول[s->level].nice_لمائی;
    s->max_chain_لمائی = configuration_جدول[s->level].max_chain;

    s->strstart = 0;
    s->بلاک_شروع = 0L;
    s->lookahead = 0;
    s->داخل_کرو = 0;
    s->میل_لمائی = s->prev_لمائی = MIN_میل-1;
    s->میل_دستیاب = 0;
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
صحيح ZEXPORT deflate_سیٹ_سرخی(z_streamp strm, gz_headerp سر) {
    إذا (deflateStateCheck(strm) || strm->state->wrap != 2)
        أرجع Z_دھارا_ERROR;
    strm->state->gzhead = سر;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflatePending(z_streamp strm, بنانشانی *pending, صحيح *بٹ) {
    إذا (deflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    إذا (بٹ != Z_NULL)
        *بٹ = strm->state->bi_valid;
    إذا (pending != Z_NULL) {
        *pending = (بنانشانی)strm->state->pending;
        إذا (*pending != strm->state->pending) {
            *pending = (بنانشانی)-1;
            أرجع Z_BUF_ERROR;
        }
    }
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_ورتیا(z_streamp strm, صحيح *بٹ) {
    إذا (deflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    إذا (بٹ != Z_NULL)
        *بٹ = strm->state->bi_ورتیا;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_مفرد(z_streamp strm, صحيح بٹ, صحيح قدر) {
    deflate_state *s;
    صحيح put;

    إذا (deflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    s = strm->state;
#جے_تعریف_شدہ LIT_MEM
    إذا (بٹ < 0 || بٹ > 16 ||
        (uchf *)s->d_buf < s->pending_باہرلا + ((Buf_ناپ + 7) >> 3))
        أرجع Z_BUF_ERROR;
#وإلا
    إذا (بٹ < 0 || بٹ > 16 ||
        s->sym_buf < s->pending_باہرلا + ((Buf_ناپ + 7) >> 3))
        أرجع Z_BUF_ERROR;
#جے_انت
    افعل {
        put = Buf_ناپ - s->bi_valid;
        إذا (put > بٹ)
            put = بٹ;
        s->bi_buf |= (ush)((قدر & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_بٹ(s);
        قدر >>= put;
        بٹ -= put;
    } طالما (بٹ);
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateParams(z_streamp strm, صحيح level, صحيح strategy) {
    deflate_state *s;
    compress_func func;

    إذا (deflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    s = strm->state;

#جے_تعریف_شدہ FASTEST
    إذا (level != 0) level = 1;
#وإلا
    إذا (level == Z_DEFAULT_COMPRESSION) level = 6;
#جے_انت
    إذا (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        أرجع Z_دھارا_ERROR;
    }
    func = configuration_جدول[s->level].func;

    إذا ((strategy != s->strategy || func != configuration_جدول[level].func) &&
        s->آخری_flush != -2) {
        /* Flush the last buffer: */
        صحيح err = deflate(strm, Z_بلاک);
        إذا (err == Z_دھارا_ERROR)
            أرجع err;
        إذا (strm->avail_وچ || (s->strstart - s->بلاک_شروع) + s->lookahead)
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
        s->max_lazy_میل   = configuration_جدول[level].max_lazy;
        s->good_میل       = configuration_جدول[level].good_لمائی;
        s->nice_میل       = configuration_جدول[level].nice_لمائی;
        s->max_chain_لمائی = configuration_جدول[level].max_chain;
    }
    s->strategy = strategy;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateTune(z_streamp strm, صحيح good_لمائی, صحيح max_lazy,
                        صحيح nice_لمائی, صحيح max_chain) {
    deflate_state *s;

    إذا (deflateStateCheck(strm)) أرجع Z_دھارا_ERROR;
    s = strm->state;
    s->good_میل = (uInt)good_لمائی;
    s->max_lazy_میل = (uInt)max_lazy;
    s->nice_میل = nice_لمائی;
    s->max_chain_لمائی = (uInt)max_chain;
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
z_ناپ_t ZEXPORT deflateBound_z(z_streamp strm, z_ناپ_t سروت_Len) {
    deflate_state *s;
    z_ناپ_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = سروت_Len + (سروت_Len >> 3) + (سروت_Len >> 8) +
               (سروت_Len >> 9) + 4;
    إذا (fixedlen < سروت_Len)
        fixedlen = (z_ناپ_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = سروت_Len + (سروت_Len >> 5) + (سروت_Len >> 7) +
               (سروت_Len >> 11) + 7;
    إذا (storelen < سروت_Len)
        storelen = (z_ناپ_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    إذا (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        أرجع bound + 18 < bound ? (z_ناپ_t)-1 : bound + 18;
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
#جے_تعریف_شدہ GZIP
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
#جے_انت
    افتراضي:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    إذا (s->w_بٹ != 15 || s->hash_بٹ != 8 + 7) {
        bound = s->w_بٹ <= s->hash_بٹ && s->level ? fixedlen :
                                                        storelen;
        أرجع bound + wraplen < bound ? (z_ناپ_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = سروت_Len + (سروت_Len >> 12) + (سروت_Len >> 14) +
            (سروت_Len >> 25) + 13 - 6 + wraplen;
    أرجع bound < سروت_Len ? (z_ناپ_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong سروت_Len) {
    z_ناپ_t bound = deflateBound_z(strm, سروت_Len);
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
    بنانشانی len;
    deflate_state *s = strm->state;

    tr_flush_بٹ(s);
    len = s->pending > strm->avail_باہرلا ? strm->avail_باہرلا :
                                         (بنانشانی)s->pending;
    إذا (len == 0) أرجع;

    zmemcpy(strm->اگلا_باہرلا, s->pending_باہرلا, len);
    strm->اگلا_باہرلا  += len;
    s->pending_باہرلا  += len;
    strm->کل_باہرلا += len;
    strm->avail_باہرلا -= len;
    s->pending      -= len;
    إذا (s->pending == 0) {
        s->pending_باہرلا = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#تعریف_کرو HCRC_UPDATE(beg) \
    افعل { \
        إذا (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = چکری_جانچ_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } طالما (0)

/* ========================================================================= */
صحيح ZEXPORT deflate(z_streamp strm, صحيح flush) {
    صحيح old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    إذا (deflateStateCheck(strm) || flush > Z_بلاک || flush < 0) {
        أرجع Z_دھارا_ERROR;
    }
    s = strm->state;

    إذا (strm->اگلا_باہرلا == Z_NULL ||
        (strm->avail_وچ != 0 && strm->اگلا_وچ == Z_NULL) ||
        (s->حالت == انت_STATE && flush != Z_انت)) {
        ERR_RETURN(strm, Z_دھارا_ERROR);
    }
    إذا (strm->avail_باہرلا == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->آخری_flush;
    s->آخری_flush = flush;

    /* Flush as much pending output as possible */
    إذا (s->pending != 0) {
        flush_pending(strm);
        إذا (strm->avail_باہرلا == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->آخری_flush = -1;
            أرجع Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } وإلا إذا (strm->avail_وچ == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_انت) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    إذا (s->حالت == انت_STATE && strm->avail_وچ != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    إذا (s->حالت == INIT_STATE && s->wrap == 0)
        s->حالت = BUSY_STATE;
    إذا (s->حالت == INIT_STATE) {
        /* zlib header */
        uInt سرخی = (Z_DEFLATED + ((s->w_بٹ - 8) << 4)) << 8;
        uInt level_flags;

        إذا (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        وإلا إذا (s->level < 6)
            level_flags = 1;
        وإلا إذا (s->level == 6)
            level_flags = 2;
        وإلا
            level_flags = 3;
        سرخی |= (level_flags << 6);
        إذا (s->strstart != 0) سرخی |= PRESET_DICT;
        سرخی += 31 - (سرخی % 31);

        putShortMSB(s, سرخی);

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
            s->آخری_flush = -1;
            أرجع Z_OK;
        }
    }
#جے_تعریف_شدہ GZIP
    إذا (s->حالت == GZIP_STATE) {
        /* gzip header */
        strm->adler = چکری_جانچ_32_2(0L, Z_NULL, 0);
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
            put_byte(s, OS_کوڈ);
            s->حالت = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            إذا (s->pending != 0) {
                s->آخری_flush = -1;
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
                strm->adler = چکری_جانچ_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->حالت = EXTRA_STATE;
        }
    }
    إذا (s->حالت == EXTRA_STATE) {
        إذا (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg بایاں = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            طالما (s->pending + بایاں > s->pending_buf_ناپ) {
                ulg copy = s->pending_buf_ناپ - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_ناپ;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                إذا (s->pending != 0) {
                    s->آخری_flush = -1;
                    أرجع Z_OK;
                }
                beg = 0;
                بایاں -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, بایاں);
            s->pending += بایاں;
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
                إذا (s->pending == s->pending_buf_ناپ) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    إذا (s->pending != 0) {
                        s->آخری_flush = -1;
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
                إذا (s->pending == s->pending_buf_ناپ) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    إذا (s->pending != 0) {
                        s->آخری_flush = -1;
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
            إذا (s->pending + 2 > s->pending_buf_ناپ) {
                flush_pending(strm);
                إذا (s->pending != 0) {
                    s->آخری_flush = -1;
                    أرجع Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = چکری_جانچ_32_2(0L, Z_NULL, 0);
        }
        s->حالت = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        إذا (s->pending != 0) {
            s->آخری_flush = -1;
            أرجع Z_OK;
        }
    }
#جے_انت

    /* Start a new block or continue the current one.
     */
    إذا (strm->avail_وچ != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->حالت != انت_STATE)) {
        بلاک_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_جدول[s->level].func))(s, flush);

        إذا (bstate == انت_started || bstate == انت_پورا) {
            s->حالت = انت_STATE;
        }
        إذا (bstate == need_ہور || bstate == انت_started) {
            إذا (strm->avail_باہرلا == 0) {
                s->آخری_flush = -1; /* avoid BUF_ERROR next call, see above */
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
        إذا (bstate == بلاک_پورا) {
            إذا (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } وإلا إذا (flush != Z_بلاک) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_بلاک(s, (حرف*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                إذا (flush == Z_بھریا_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    إذا (s->lookahead == 0) {
                        s->strstart = 0;
                        s->بلاک_شروع = 0L;
                        s->داخل_کرو = 0;
                    }
                }
            }
            flush_pending(strm);
            إذا (strm->avail_باہرلا == 0) {
              s->آخری_flush = -1; /* avoid BUF_ERROR at next call, see above */
              أرجع Z_OK;
            }
        }
    }

    إذا (flush != Z_انت) أرجع Z_OK;
    إذا (s->wrap <= 0) أرجع Z_دھارا_سرا;

    /* Write the trailer */
#جے_تعریف_شدہ GZIP
    إذا (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->کل_وچ & 0xff));
        put_byte(s, (Byte)((strm->کل_وچ >> 8) & 0xff));
        put_byte(s, (Byte)((strm->کل_وچ >> 16) & 0xff));
        put_byte(s, (Byte)((strm->کل_وچ >> 24) & 0xff));
    }
    وإلا
#جے_انت
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    إذا (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    أرجع s->pending != 0 ? Z_OK : Z_دھارا_سرا;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_سرا(z_streamp strm) {
    صحيح حالت;

    إذا (deflateStateCheck(strm)) أرجع Z_دھارا_ERROR;

    حالت = strm->state->حالت;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->سر);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->کھڑکی);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    أرجع حالت == BUSY_STATE ? Z_انکڑے_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
صحيح ZEXPORT deflateCopy(z_streamp dest, z_streamp سروت) {
#جے_تعریف_شدہ MAXSEG_64K
    (فراغ)dest;
    (فراغ)سروت;
    أرجع Z_دھارا_ERROR;
#وإلا
    deflate_state *ds;
    deflate_state *ss;


    إذا (deflateStateCheck(سروت) || dest == Z_NULL) {
        أرجع Z_دھارا_ERROR;
    }

    ss = سروت->state;

    zmemcpy(dest, سروت, حجم(z_دھارا));

    ds = (deflate_state *) ZALLOC(dest, 1, حجم(deflate_state));
    إذا (ds == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(ds, حجم(deflate_state));
    dest->state = (بنية اندرونی_state FAR *) ds;
    zmemcpy(ds, ss, حجم(deflate_state));
    ds->strm = dest;

    ds->کھڑکی = (Bytef *) ZALLOC(dest, ds->w_ناپ, 2*حجم(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_ناپ, حجم(Pos));
    ds->سر   = (Posf *)  ZALLOC(dest, ds->hash_ناپ, حجم(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    إذا (ds->کھڑکی == Z_NULL || ds->prev == Z_NULL || ds->سر == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_سرا (dest);
        أرجع Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->کھڑکی, ss->کھڑکی, ss->بلند_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->داخل_کرو > ds->w_ناپ ? ds->w_ناپ :
                ss->strstart - ss->داخل_کرو) * حجم(Pos));
    zmemcpy(ds->سر, ss->سر, ds->hash_ناپ * حجم(Pos));

    ds->pending_باہرلا = ds->pending_buf + (ss->pending_باہرلا - ss->pending_buf);
    zmemcpy(ds->pending_باہرلا, ss->pending_باہرلا, ss->pending);
#جے_تعریف_شدہ LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_اگلا * حجم(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_اگلا);
#وإلا
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_اگلا);
#جے_انت

    ds->l_desc.dyn_درخت = ds->dyn_ltree;
    ds->d_desc.dyn_درخت = ds->dyn_dtree;
    ds->bl_desc.dyn_درخت = ds->bl_درخت;

    أرجع Z_OK;
#جے_انت /* MAXSEG_64K */
}

#جے_بنا_تعریف FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_میل(deflate_state *s, IPos cur_میل) {
    بنانشانی chain_لمائی = s->max_chain_لمائی;/* max hash chain length */
    Bytef *جانچ = s->کھڑکی + s->strstart;      /* current string */
    Bytef *میل;                               /* matched string */
    صحيح len;                                    /* length of current match */
    صحيح بہترین_len = (صحيح)s->prev_لمائی;         /* best match length so far */
    صحيح nice_میل = s->nice_میل;             /* stop if match long enough */
    IPos حد_بندی = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_نقاب;

#جے_تعریف_شدہ UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->کھڑکی + s->strstart + MAX_میل - 1;
    ush جانچ_شروع = *(ushf*)جانچ;
    ush جانچ_سرا   = *(ushf*)(جانچ + بہترین_len - 1);
#وإلا
    Bytef *strend = s->کھڑکی + s->strstart + MAX_میل;
    Byte جانچ_end1  = جانچ[بہترین_len - 1];
    Byte جانچ_سرا   = جانچ[بہترین_len];
#جے_انت

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بٹ >= 8 && MAX_میل == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    إذا (s->prev_لمائی >= s->good_میل) {
        chain_لمائی >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    إذا ((uInt)nice_میل > s->lookahead) nice_میل = (صحيح)s->lookahead;

    Assert((ulg)s->strstart <= s->کھڑکی_ناپ - MIN_LOOKAHEAD,
           "need lookahead");

    افعل {
        Assert(cur_میل < s->strstart, "no future");
        میل = s->کھڑکی + cur_میل;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#إذا (defined(UNALIGNED_OK) && MAX_میل == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        إذا (*(ushf*)(میل + بہترین_len - 1) != جانچ_سرا ||
            *(ushf*)میل != جانچ_شروع) استمر;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(جانچ[2] == میل[2], "scan[2]?");
        جانچ++, میل++;
        افعل {
        } طالما (*(ushf*)(جانچ += 2) == *(ushf*)(میل += 2) &&
                 *(ushf*)(جانچ += 2) == *(ushf*)(میل += 2) &&
                 *(ushf*)(جانچ += 2) == *(ushf*)(میل += 2) &&
                 *(ushf*)(جانچ += 2) == *(ushf*)(میل += 2) &&
                 جانچ < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(جانچ <= s->کھڑکی + (بنانشانی)(s->کھڑکی_ناپ - 1),
               "wild scan");
        إذا (*جانچ == *میل) جانچ++;

        len = (MAX_میل - 1) - (صحيح)(strend - جانچ);
        جانچ = strend - (MAX_میل-1);

#وإلا /* UNALIGNED_OK */

        إذا (میل[بہترین_len]     != جانچ_سرا  ||
            میل[بہترین_len - 1] != جانچ_end1 ||
            *میل              != *جانچ     ||
            *++میل            != جانچ[1])      استمر;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        جانچ += 2, میل++;
        Assert(*جانچ == *میل, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        افعل {
        } طالما (*++جانچ == *++میل && *++جانچ == *++میل &&
                 *++جانچ == *++میل && *++جانچ == *++میل &&
                 *++جانچ == *++میل && *++جانچ == *++میل &&
                 *++جانچ == *++میل && *++جانچ == *++میل &&
                 جانچ < strend);

        Assert(جانچ <= s->کھڑکی + (بنانشانی)(s->کھڑکی_ناپ - 1),
               "wild scan");

        len = MAX_میل - (صحيح)(strend - جانچ);
        جانچ = strend - MAX_میل;

#جے_انت /* UNALIGNED_OK */

        إذا (len > بہترین_len) {
            s->میل_شروع = cur_میل;
            بہترین_len = len;
            إذا (len >= nice_میل) اكسر;
#جے_تعریف_شدہ UNALIGNED_OK
            جانچ_سرا = *(ushf*)(جانچ + بہترین_len - 1);
#وإلا
            جانچ_end1  = جانچ[بہترین_len - 1];
            جانچ_سرا   = جانچ[بہترین_len];
#جے_انت
        }
    } طالما ((cur_میل = prev[cur_میل & wmask]) > حد_بندی
             && --chain_لمائی != 0);

    إذا ((uInt)بہترین_len <= s->lookahead) أرجع (uInt)بہترین_len;
    أرجع s->lookahead;
}

#وإلا /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_میل(deflate_state *s, IPos cur_میل) {
    Bytef *جانچ = s->کھڑکی + s->strstart;      /* current string */
    Bytef *میل;                               /* matched string */
    صحيح len;                                    /* length of current match */
    Bytef *strend = s->کھڑکی + s->strstart + MAX_میل;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بٹ >= 8 && MAX_میل == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->کھڑکی_ناپ - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_میل < s->strstart, "no future");

    میل = s->کھڑکی + cur_میل;

    /* Return failure if the match length is less than 2:
     */
    إذا (میل[0] != جانچ[0] || میل[1] != جانچ[1]) أرجع MIN_میل-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    جانچ += 2, میل += 2;
    Assert(*جانچ == *میل, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    افعل {
    } طالما (*++جانچ == *++میل && *++جانچ == *++میل &&
             *++جانچ == *++میل && *++جانچ == *++میل &&
             *++جانچ == *++میل && *++جانچ == *++میل &&
             *++جانچ == *++میل && *++جانچ == *++میل &&
             جانچ < strend);

    Assert(جانچ <= s->کھڑکی + (بنانشانی)(s->کھڑکی_ناپ - 1), "wild scan");

    len = MAX_میل - (صحيح)(strend - جانچ);

    إذا (len < MIN_میل) أرجع MIN_میل - 1;

    s->میل_شروع = cur_میل;
    أرجع (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#جے_انت /* FASTEST */

#جے_تعریف_شدہ ZLIB_DEBUG

#تعریف_کرو EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local فراغ check_میل(deflate_state *s, IPos شروع, IPos میل, صحيح لمائی) {
    /* check that the match is indeed a match */
    Bytef *پٹھ = s->کھڑکی + (صحيح)میل, *ایتھے = s->کھڑکی + شروع;
    IPos len = (IPos)لمائی;
    إذا (میل == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        پٹھ++;
        ایتھے++;
        len--;
    }
    إذا (zmemcmp(پٹھ, ایتھے, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                شروع, (صحيح)میل, لمائی);
        افعل {
            fprintf(stderr, "(%02x %02x)", *پٹھ++, *ایتھے++);
        } طالما (--len != 0);
        z_error("invalid match");
    }
    إذا (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", شروع - میل, لمائی);
        افعل { putc(s->کھڑکی[شروع++], stderr); } طالما (--لمائی != 0);
    }
}
#وإلا
#  define check_میل(s, شروع, میل, لمائی)
#جے_انت /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#تعریف_کرو FLUSH_بلاک_ONLY(s, آخری) { \
   tr_flush_بلاک(s, (s->بلاک_شروع >= 0L ? \
                   (charf *)&s->کھڑکی[(بنانشانی)s->بلاک_شروع] : \
                   (charf *)Z_NULL), \
                (ulg)((لما)s->strstart - s->بلاک_شروع), \
                (آخری)); \
   s->بلاک_شروع = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#تعریف_کرو FLUSH_بلاک(s, آخری) { \
   FLUSH_بلاک_ONLY(s, آخری); \
   إذا (s->strm->avail_باہرلا == 0) أرجع (آخری) ? انت_started : need_ہور; \
}

/* Maximum stored block length in deflate format (not including header). */
#تعریف_کرو MAX_STORED 65535

/* Minimum of a and b. */
#تعریف_کرو MIN(a, b) ((a) > (b) ? (b) : (a))

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
local بلاک_state deflate_stored(deflate_state *s, صحيح flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    بنانشانی min_بلاک = (بنانشانی)(MIN(s->pending_buf_ناپ - 5, s->w_ناپ));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    صحيح آخری = 0;
    بنانشانی len, بایاں, have;
    بنانشانی ورتیا = s->strm->avail_وچ;
    افعل {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((بنانشانی)s->bi_valid + 42) >> 3;   /* bytes in header */
        إذا (s->strm->avail_باہرلا < have)          /* need room for header */
            اكسر;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_باہرلا - have;
        بایاں = (بنانشانی)(s->strstart - s->بلاک_شروع);    /* window bytes */
        إذا (len > (ulg)بایاں + s->strm->avail_وچ)
            len = بایاں + s->strm->avail_وچ;     /* limit len to the input */
        إذا (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        إذا (len < min_بلاک && ((len == 0 && flush != Z_انت) ||
                                flush == Z_NO_FLUSH ||
                                len != بایاں + s->strm->avail_وچ))
            اكسر;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        آخری = flush == Z_انت && len == بایاں + s->strm->avail_وچ ? 1 : 0;
        tr_stored_بلاک(s, (حرف *)0, 0L, آخری);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#جے_تعریف_شدہ ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->بٹ_گھلیا += len << 3;
#جے_انت

        /* Copy uncompressed bytes from the window to next_out. */
        إذا (بایاں) {
            إذا (بایاں > len)
                بایاں = len;
            zmemcpy(s->strm->اگلا_باہرلا, s->کھڑکی + s->بلاک_شروع, بایاں);
            s->strm->اگلا_باہرلا += بایاں;
            s->strm->avail_باہرلا -= بایاں;
            s->strm->کل_باہرلا += بایاں;
            s->بلاک_شروع += بایاں;
            len -= بایاں;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        إذا (len) {
            read_buf(s->strm, s->strm->اگلا_باہرلا, len);
            s->strm->اگلا_باہرلا += len;
            s->strm->avail_باہرلا -= len;
            s->strm->کل_باہرلا += len;
        }
    } طالما (آخری == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    ورتیا -= s->strm->avail_وچ;      /* number of input bytes directly copied */
    إذا (ورتیا) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        إذا (ورتیا >= s->w_ناپ) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->کھڑکی, s->strm->اگلا_وچ - s->w_ناپ, s->w_ناپ);
            s->strstart = s->w_ناپ;
            s->داخل_کرو = s->strstart;
        }
        وإلا {
            إذا (s->کھڑکی_ناپ - s->strstart <= ورتیا) {
                /* Slide the window down. */
                s->strstart -= s->w_ناپ;
                zmemcpy(s->کھڑکی, s->کھڑکی + s->w_ناپ, s->strstart);
                إذا (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                إذا (s->داخل_کرو > s->strstart)
                    s->داخل_کرو = s->strstart;
            }
            zmemcpy(s->کھڑکی + s->strstart, s->strm->اگلا_وچ - ورتیا, ورتیا);
            s->strstart += ورتیا;
            s->داخل_کرو += MIN(ورتیا, s->w_ناپ - s->داخل_کرو);
        }
        s->بلاک_شروع = s->strstart;
    }
    إذا (s->بلند_water < s->strstart)
        s->بلند_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    إذا (آخری) {
        s->bi_ورتیا = 8;
        أرجع انت_پورا;
    }

    /* If flushing and all input has been consumed, then done. */
    إذا (flush != Z_NO_FLUSH && flush != Z_انت &&
        s->strm->avail_وچ == 0 && (لما)s->strstart == s->بلاک_شروع)
        أرجع بلاک_پورا;

    /* Fill the window with any remaining input. */
    have = (بنانشانی)(s->کھڑکی_ناپ - s->strstart);
    إذا (s->strm->avail_وچ > have && s->بلاک_شروع >= (لما)s->w_ناپ) {
        /* Slide the window down. */
        s->بلاک_شروع -= s->w_ناپ;
        s->strstart -= s->w_ناپ;
        zmemcpy(s->کھڑکی, s->کھڑکی + s->w_ناپ, s->strstart);
        إذا (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_ناپ;          /* more space now */
        إذا (s->داخل_کرو > s->strstart)
            s->داخل_کرو = s->strstart;
    }
    إذا (have > s->strm->avail_وچ)
        have = s->strm->avail_وچ;
    إذا (have) {
        read_buf(s->strm, s->کھڑکی + s->strstart, have);
        s->strstart += have;
        s->داخل_کرو += MIN(have, s->w_ناپ - s->داخل_کرو);
    }
    إذا (s->بلند_water < s->strstart)
        s->بلند_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((بنانشانی)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (بنانشانی)MIN(s->pending_buf_ناپ - have, MAX_STORED);
    min_بلاک = MIN(have, s->w_ناپ);
    بایاں = (بنانشانی)(s->strstart - s->بلاک_شروع);
    إذا (بایاں >= min_بلاک ||
        ((بایاں || flush == Z_انت) && flush != Z_NO_FLUSH &&
         s->strm->avail_وچ == 0 && بایاں <= have)) {
        len = MIN(بایاں, have);
        آخری = flush == Z_انت && s->strm->avail_وچ == 0 &&
               len == بایاں ? 1 : 0;
        tr_stored_بلاک(s, (charf *)s->کھڑکی + s->بلاک_شروع, len, آخری);
        s->بلاک_شروع += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    إذا (آخری)
        s->bi_ورتیا = 8;
    أرجع آخری ? انت_started : need_ہور;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local بلاک_state deflate_fast(deflate_state *s, صحيح flush) {
    IPos hash_سر;       /* head of the hash chain */
    صحيح bflush;           /* set if current block must be flushed */

    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        إذا (s->lookahead < MIN_LOOKAHEAD) {
            fill_کھڑکی(s);
            إذا (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_ہور;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        إذا (s->lookahead >= MIN_میل) {
            داخل_کرو_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        إذا (hash_سر != NIL && s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->میل_لمائی = longest_میل (s, hash_سر);
            /* longest_match() sets match_start */
        }
        إذا (s->میل_لمائی >= MIN_میل) {
            check_میل(s, s->strstart, s->میل_شروع, (صحيح)s->میل_لمائی);

            _tr_tally_dist(s, s->strstart - s->میل_شروع,
                           s->میل_لمائی - MIN_میل, bflush);

            s->lookahead -= s->میل_لمائی;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#جے_بنا_تعریف FASTEST
            إذا (s->میل_لمائی <= s->max_داخل_کرو_لمائی &&
                s->lookahead >= MIN_میل) {
                s->میل_لمائی--; /* string at strstart already in table */
                افعل {
                    s->strstart++;
                    داخل_کرو_STRING(s, s->strstart, hash_سر);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } طالما (--s->میل_لمائی != 0);
                s->strstart++;
            } وإلا
#جے_انت
            {
                s->strstart += s->میل_لمائی;
                s->میل_لمائی = 0;
                s->ins_h = s->کھڑکی[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->کھڑکی[s->strstart + 1]);
#إذا MIN_میل != 3
                Call UPDATE_HASH() MIN_میل-3 ہور times
#جے_انت
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->کھڑکی[s->strstart]));
            _tr_tally_lit(s, s->کھڑکی[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        إذا (bflush) FLUSH_بلاک(s, 0);
    }
    s->داخل_کرو = s->strstart < MIN_میل-1 ? s->strstart : MIN_میل-1;
    إذا (flush == Z_انت) {
        FLUSH_بلاک(s, 1);
        أرجع انت_پورا;
    }
    إذا (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_پورا;
}

#جے_بنا_تعریف FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local بلاک_state deflate_slow(deflate_state *s, صحيح flush) {
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
            fill_کھڑکی(s);
            إذا (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_ہور;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        إذا (s->lookahead >= MIN_میل) {
            داخل_کرو_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_لمائی = s->میل_لمائی, s->prev_میل = s->میل_شروع;
        s->میل_لمائی = MIN_میل-1;

        إذا (hash_سر != NIL && s->prev_لمائی < s->max_lazy_میل &&
            s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->میل_لمائی = longest_میل (s, hash_سر);
            /* longest_match() sets match_start */

            إذا (s->میل_لمائی <= 5 && (s->strategy == Z_FILTERED
#إذا TOO_FAR <= 32767
                || (s->میل_لمائی == MIN_میل &&
                    s->strstart - s->میل_شروع > TOO_FAR)
#جے_انت
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->میل_لمائی = MIN_میل-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        إذا (s->prev_لمائی >= MIN_میل && s->میل_لمائی <= s->prev_لمائی) {
            uInt max_داخل_کرو = s->strstart + s->lookahead - MIN_میل;
            /* Do not insert strings in hash table beyond this. */

            check_میل(s, s->strstart - 1, s->prev_میل, (صحيح)s->prev_لمائی);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_میل,
                           s->prev_لمائی - MIN_میل, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_لمائی - 1;
            s->prev_لمائی -= 2;
            افعل {
                إذا (++s->strstart <= max_داخل_کرو) {
                    داخل_کرو_STRING(s, s->strstart, hash_سر);
                }
            } طالما (--s->prev_لمائی != 0);
            s->میل_دستیاب = 0;
            s->میل_لمائی = MIN_میل-1;
            s->strstart++;

            إذا (bflush) FLUSH_بلاک(s, 0);

        } وإلا إذا (s->میل_دستیاب) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->کھڑکی[s->strstart - 1]));
            _tr_tally_lit(s, s->کھڑکی[s->strstart - 1], bflush);
            إذا (bflush) {
                FLUSH_بلاک_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            إذا (s->strm->avail_باہرلا == 0) أرجع need_ہور;
        } وإلا {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->میل_دستیاب = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    إذا (s->میل_دستیاب) {
        Tracevv((stderr,"%c", s->کھڑکی[s->strstart - 1]));
        _tr_tally_lit(s, s->کھڑکی[s->strstart - 1], bflush);
        s->میل_دستیاب = 0;
    }
    s->داخل_کرو = s->strstart < MIN_میل-1 ? s->strstart : MIN_میل-1;
    إذا (flush == Z_انت) {
        FLUSH_بلاک(s, 1);
        أرجع انت_پورا;
    }
    إذا (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_پورا;
}
#جے_انت /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local بلاک_state deflate_rle(deflate_state *s, صحيح flush) {
    صحيح bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *جانچ, *strend;   /* scan goes up to strend for length of run */

    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        إذا (s->lookahead <= MAX_میل) {
            fill_کھڑکی(s);
            إذا (s->lookahead <= MAX_میل && flush == Z_NO_FLUSH) {
                أرجع need_ہور;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->میل_لمائی = 0;
        إذا (s->lookahead >= MIN_میل && s->strstart > 0) {
            جانچ = s->کھڑکی + s->strstart - 1;
            prev = *جانچ;
            إذا (prev == *++جانچ && prev == *++جانچ && prev == *++جانچ) {
                strend = s->کھڑکی + s->strstart + MAX_میل;
                افعل {
                } طالما (prev == *++جانچ && prev == *++جانچ &&
                         prev == *++جانچ && prev == *++جانچ &&
                         prev == *++جانچ && prev == *++جانچ &&
                         prev == *++جانچ && prev == *++جانچ &&
                         جانچ < strend);
                s->میل_لمائی = MAX_میل - (uInt)(strend - جانچ);
                إذا (s->میل_لمائی > s->lookahead)
                    s->میل_لمائی = s->lookahead;
            }
            Assert(جانچ <= s->کھڑکی + (uInt)(s->کھڑکی_ناپ - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        إذا (s->میل_لمائی >= MIN_میل) {
            check_میل(s, s->strstart, s->strstart - 1, (صحيح)s->میل_لمائی);

            _tr_tally_dist(s, 1, s->میل_لمائی - MIN_میل, bflush);

            s->lookahead -= s->میل_لمائی;
            s->strstart += s->میل_لمائی;
            s->میل_لمائی = 0;
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->کھڑکی[s->strstart]));
            _tr_tally_lit(s, s->کھڑکی[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        إذا (bflush) FLUSH_بلاک(s, 0);
    }
    s->داخل_کرو = 0;
    إذا (flush == Z_انت) {
        FLUSH_بلاک(s, 1);
        أرجع انت_پورا;
    }
    إذا (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_پورا;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local بلاک_state deflate_huff(deflate_state *s, صحيح flush) {
    صحيح bflush;             /* set if current block must be flushed */

    لكل (;;) {
        /* Make sure that we have a literal to write. */
        إذا (s->lookahead == 0) {
            fill_کھڑکی(s);
            إذا (s->lookahead == 0) {
                إذا (flush == Z_NO_FLUSH)
                    أرجع need_ہور;
                اكسر;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->میل_لمائی = 0;
        Tracevv((stderr,"%c", s->کھڑکی[s->strstart]));
        _tr_tally_lit(s, s->کھڑکی[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        إذا (bflush) FLUSH_بلاک(s, 0);
    }
    s->داخل_کرو = 0;
    إذا (flush == Z_انت) {
        FLUSH_بلاک(s, 1);
        أرجع انت_پورا;
    }
    إذا (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_پورا;
}
