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

#شامل "deflate.h"

ثابت حرف deflate_copyright[] =
   " deflate 1.3.2.1 Copyright 1995-2026 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

قسمتعریف شمار {
    need_مزید,      /* block not completed, need more input or more output */
    بلاک_مکمل,     /* block flush performed */
    اختتام_started, /* finish started, need only more output at next deflate */
    اختتام_مکمل     /* finish done, accept no more input or output */
} بلاک_state;

قسمتعریف بلاک_state (*compress_func)(deflate_state *s, صحیح flush);
/* Compression function. Returns the block state after the call. */

local بلاک_state deflate_stored(deflate_state *s, صحیح flush);
local بلاک_state deflate_fast(deflate_state *s, صحیح flush);
#اگر_غیر_تعریف_شدہ FASTEST
local بلاک_state deflate_slow(deflate_state *s, صحیح flush);
#اگر_ختم
local بلاک_state deflate_rle(deflate_state *s, صحیح flush);
local بلاک_state deflate_huff(deflate_state *s, صحیح flush);

/* ===========================================================================
 * Local data
 */

#تعریف_کرو NIL 0
/* Tail of hash chains */

#اگر_غیر_تعریف_شدہ TOO_FAR
#  define TOO_FAR 4096
#اگر_ختم
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
قسمتعریف ساخت config_s {
   ush good_لمبائی; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_لمبائی; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#اگر_تعریف_شدہ FASTEST
local ثابت config configuration_جدول[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#ورنہ
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
#اگر_ختم

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
#تعریف_کرو UPDATE_HASH(s,h,c) (h = (((h) << s->hash_سرکاؤ) ^ (c)) & s->hash_نقاب)


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
#اگر_تعریف_شدہ FASTEST
#تعریف_کرو داخل_کرو_STRING(s, str, مطابقت_سر) \
   (UPDATE_HASH(s, s->ins_h, s->کھڑکی[(str) + (MIN_مطابقت-1)]), \
    مطابقت_سر = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#ورنہ
#تعریف_کرو داخل_کرو_STRING(s, str, مطابقت_سر) \
   (UPDATE_HASH(s, s->ins_h, s->کھڑکی[(str) + (MIN_مطابقت-1)]), \
    مطابقت_سر = s->پچھلا[(str) & s->w_نقاب] = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#اگر_ختم

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#تعریف_کرو CLEAR_HASH(s) \
    کرو { \
        s->سر[s->hash_سائز - 1] = NIL; \
        zmemzero(s->سر, (غیرعلامتی)(s->hash_سائز - 1)*حجم(*s->سر)); \
        s->slid = 0; \
    } جب (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#اگر defined(__has_feature)
#  اگر __has_feature(حافظہ_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#اگر_ختم
local خالی slide_hash(deflate_state *s) {
    غیرعلامتی n, m;
    Posf *p;
    uInt wsize = s->w_سائز;

    n = s->hash_سائز;
    p = &s->سر[n];
    کرو {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } جب (--n);
#اگر_غیر_تعریف_شدہ FASTEST
    n = wsize;
    p = &s->پچھلا[n];
    کرو {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } جب (--n);
#اگر_ختم
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local غیرعلامتی read_buf(z_streamp strm, Bytef *buf, غیرعلامتی سائز) {
    غیرعلامتی len = strm->avail_اندر;

    اگر (len > سائز) len = سائز;
    اگر (len == 0) واپس 0;

    strm->avail_اندر  -= len;

    zmemcpy(buf, strm->اگلا_اندر, len);
    اگر (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#اگر_تعریف_شدہ GZIP
    ورنہ اگر (strm->state->wrap == 2) {
        strm->adler = چکری_جانچ_32_2(strm->adler, buf, len);
    }
#اگر_ختم
    strm->اگلا_اندر  += len;
    strm->کل_اندر += len;

    واپس len;
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
local خالی fill_کھڑکی(deflate_state *s) {
    غیرعلامتی n;
    غیرعلامتی مزید;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_سائز;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    کرو {
        مزید = (غیرعلامتی)(s->کھڑکی_سائز -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#اگر_تعریف_شدہ _MSC_VER
#ہدایت warning(ڈالو)
#ہدایت warning(disable: 4127)
#اگر_ختم
        اگر (حجم(صحیح) <= 2) {
#اگر_تعریف_شدہ _MSC_VER
#ہدایت warning(نکالو)
#اگر_ختم
            اگر (مزید == 0 && s->strstart == 0 && s->lookahead == 0) {
                مزید = wsize;

            } ورنہ اگر (مزید == (غیرعلامتی)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                مزید--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        اگر (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->کھڑکی, s->کھڑکی + wsize, (غیرعلامتی)wsize - مزید);
            s->مطابقت_آغاز -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->بلاک_آغاز -= (طویل) wsize;
            اگر (s->داخل_کرو > s->strstart)
                s->داخل_کرو = s->strstart;
            slide_hash(s);
            مزید += wsize;
        }
        اگر (s->strm->avail_اندر == 0) توڑو;

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
        Assert(مزید >= 2, "more < 2");

        n = read_buf(s->strm, s->کھڑکی + s->strstart + s->lookahead, مزید);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        اگر (s->lookahead + s->داخل_کرو >= MIN_مطابقت) {
            uInt str = s->strstart - s->داخل_کرو;
            s->ins_h = s->کھڑکی[str];
            UPDATE_HASH(s, s->ins_h, s->کھڑکی[str + 1]);
#اگر MIN_مطابقت != 3
            Call UPDATE_HASH() MIN_مطابقت-3 مزید times
#اگر_ختم
            جب (s->داخل_کرو) {
                UPDATE_HASH(s, s->ins_h, s->کھڑکی[str + MIN_مطابقت-1]);
#اگر_غیر_تعریف_شدہ FASTEST
                s->پچھلا[str & s->w_نقاب] = s->سر[s->ins_h];
#اگر_ختم
                s->سر[s->ins_h] = (Pos)str;
                str++;
                s->داخل_کرو--;
                اگر (s->lookahead + s->داخل_کرو < MIN_مطابقت)
                    توڑو;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } جب (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_اندر != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    اگر (s->بلند_water < s->کھڑکی_سائز) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        اگر (s->بلند_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->کھڑکی_سائز - curr;
            اگر (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->کھڑکی + curr, (غیرعلامتی)init);
            s->بلند_water = curr + init;
        }
        ورنہ اگر (s->بلند_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->بلند_water;
            اگر (init > s->کھڑکی_سائز - s->بلند_water)
                init = s->کھڑکی_سائز - s->بلند_water;
            zmemzero(s->کھڑکی + s->بلند_water, (غیرعلامتی)init);
            s->بلند_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->کھڑکی_سائز - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
صحیح ZEXPORT deflateInit_(z_streamp strm, صحیح level, ثابت حرف *نسخہ,
                         صحیح دھارا_سائز) {
    واپس deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, نسخہ, دھارا_سائز);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
صحیح ZEXPORT deflateInit2_(z_streamp strm, صحیح level, صحیح طریقہ_2,
                          صحیح کھڑکی_بٹ, صحیح memLevel, صحیح strategy,
                          ثابت حرف *نسخہ, صحیح دھارا_سائز) {
    deflate_state *s;
    صحیح wrap = 1;
    ساکن ثابت حرف my_نسخہ[] = ZLIB_نسخہ;

    اگر (نسخہ == Z_NULL || نسخہ[0] != my_نسخہ[0] ||
        دھارا_سائز != حجم(z_دھارا)) {
        واپس Z_نسخہ_ERROR;
    }
    اگر (strm == Z_NULL) واپس Z_دھارا_ERROR;

    strm->پیغام = Z_NULL;
    اگر (strm->zalloc == (alloc_func)0) {
#اگر_تعریف_شدہ Z_SOLO
        واپس Z_دھارا_ERROR;
#ورنہ
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#اگر_ختم
    }
    اگر (strm->zfree == (free_func)0)
#اگر_تعریف_شدہ Z_SOLO
        واپس Z_دھارا_ERROR;
#ورنہ
        strm->zfree = zcfree;
#اگر_ختم

#اگر_تعریف_شدہ FASTEST
    اگر (level != 0) level = 1;
#ورنہ
    اگر (level == Z_DEFAULT_COMPRESSION) level = 6;
#اگر_ختم

    اگر (کھڑکی_بٹ < 0) { /* suppress zlib wrapper */
        wrap = 0;
        اگر (کھڑکی_بٹ < -15)
            واپس Z_دھارا_ERROR;
        کھڑکی_بٹ = -کھڑکی_بٹ;
    }
#اگر_تعریف_شدہ GZIP
    ورنہ اگر (کھڑکی_بٹ > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        کھڑکی_بٹ -= 16;
    }
#اگر_ختم
    اگر (memLevel < 1 || memLevel > MAX_MEM_LEVEL || طریقہ_2 != Z_DEFLATED ||
        کھڑکی_بٹ < 8 || کھڑکی_بٹ > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (کھڑکی_بٹ == 8 && wrap != 1)) {
        واپس Z_دھارا_ERROR;
    }
    اگر (کھڑکی_بٹ == 8) کھڑکی_بٹ = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, حجم(deflate_state));
    اگر (s == Z_NULL) واپس Z_MEM_ERROR;
    zmemzero(s, حجم(deflate_state));
    strm->state = (ساخت اندرونی_state FAR *)s;
    s->strm = strm;
    s->حالت_2 = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_بٹ = (uInt)کھڑکی_بٹ;
    s->w_سائز = 1 << s->w_بٹ;
    s->w_نقاب = s->w_سائز - 1;

    s->hash_بٹ = (uInt)memLevel + 7;
    s->hash_سائز = 1 << s->hash_بٹ;
    s->hash_نقاب = s->hash_سائز - 1;
    s->hash_سرکاؤ =  ((s->hash_بٹ + MIN_مطابقت-1) / MIN_مطابقت);

    s->کھڑکی = (Bytef *) ZALLOC(strm, s->w_سائز, 2*حجم(Byte));
    s->پچھلا   = (Posf *)  ZALLOC(strm, s->w_سائز, حجم(Pos));
    s->سر   = (Posf *)  ZALLOC(strm, s->hash_سائز, حجم(Pos));

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
    s->pending_buf_سائز = (ulg)s->lit_bufsize * 4;

    اگر (s->کھڑکی == Z_NULL || s->پچھلا == Z_NULL || s->سر == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->حالت_2 = اختتام_STATE;
        strm->پیغام = ERR_پیغام(Z_MEM_ERROR);
        deflate_انتہا (strm);
        واپس Z_MEM_ERROR;
    }
#اگر_تعریف_شدہ LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_انتہا = s->lit_bufsize - 1;
#ورنہ
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_انتہا = (s->lit_bufsize - 1) * 3;
#اگر_ختم
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->طریقہ_2 = (Byte)طریقہ_2;

    واپس deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local صحیح deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    اگر (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        واپس 1;
    s = strm->state;
    اگر (s == Z_NULL || s->strm != strm || (s->حالت_2 != INIT_STATE &&
#اگر_تعریف_شدہ GZIP
                                           s->حالت_2 != GZIP_STATE &&
#اگر_ختم
                                           s->حالت_2 != EXTRA_STATE &&
                                           s->حالت_2 != NAME_STATE &&
                                           s->حالت_2 != COMMENT_STATE &&
                                           s->حالت_2 != HCRC_STATE &&
                                           s->حالت_2 != BUSY_STATE &&
                                           s->حالت_2 != اختتام_STATE))
        واپس 1;
    واپس 0;
}

/* ========================================================================= */
صحیح ZEXPORT deflate_سیٹ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt  dict_لمبائی) {
    deflate_state *s;
    uInt str, n;
    صحیح wrap;
    غیرعلامتی avail;
    z_const غیرعلامتی حرف *اگلا;

    اگر (deflateStateCheck(strm) || dictionary == Z_NULL)
        واپس Z_دھارا_ERROR;
    s = strm->state;
    wrap = s->wrap;
    اگر (wrap == 2 || (wrap == 1 && s->حالت_2 != INIT_STATE) || s->lookahead)
        واپس Z_دھارا_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    اگر (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_لمبائی);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    اگر (dict_لمبائی >= s->w_سائز) {
        اگر (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->بلاک_آغاز = 0L;
            s->داخل_کرو = 0;
        }
        dictionary += dict_لمبائی - s->w_سائز;  /* use the tail */
        dict_لمبائی = s->w_سائز;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_اندر;
    اگلا = strm->اگلا_اندر;
    strm->avail_اندر = dict_لمبائی;
    strm->اگلا_اندر = (z_const Bytef *)dictionary;
    fill_کھڑکی(s);
    جب (s->lookahead >= MIN_مطابقت) {
        str = s->strstart;
        n = s->lookahead - (MIN_مطابقت-1);
        کرو {
            UPDATE_HASH(s, s->ins_h, s->کھڑکی[str + MIN_مطابقت-1]);
#اگر_غیر_تعریف_شدہ FASTEST
            s->پچھلا[str & s->w_نقاب] = s->سر[s->ins_h];
#اگر_ختم
            s->سر[s->ins_h] = (Pos)str;
            str++;
        } جب (--n);
        s->strstart = str;
        s->lookahead = MIN_مطابقت-1;
        fill_کھڑکی(s);
    }
    s->strstart += s->lookahead;
    s->بلاک_آغاز = (طویل)s->strstart;
    s->داخل_کرو = s->lookahead;
    s->lookahead = 0;
    s->مطابقت_لمبائی = s->پچھلا_لمبائی = MIN_مطابقت-1;
    s->مطابقت_دستیاب = 0;
    strm->اگلا_اندر = اگلا;
    strm->avail_اندر = avail;
    s->wrap = wrap;
    واپس Z_OK;
}

/* ========================================================================= */
صحیح ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_لمبائی) {
    deflate_state *s;
    uInt len;

    اگر (deflateStateCheck(strm))
        واپس Z_دھارا_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    اگر (len > s->w_سائز)
        len = s->w_سائز;
    اگر (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->کھڑکی + s->strstart + s->lookahead - len, len);
    اگر (dict_لمبائی != Z_NULL)
        *dict_لمبائی = len;
    واپس Z_OK;
}

/* ========================================================================= */
صحیح ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    اگر (deflateStateCheck(strm)) {
        واپس Z_دھارا_ERROR;
    }

    strm->کل_اندر = strm->کل_برآمد = 0;
    strm->پیغام = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->ڈیٹا_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_برآمد = s->pending_buf;

    اگر (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->حالت_2 =
#اگر_تعریف_شدہ GZIP
        s->wrap == 2 ? GZIP_STATE :
#اگر_ختم
        INIT_STATE;
    strm->adler =
#اگر_تعریف_شدہ GZIP
        s->wrap == 2 ? چکری_جانچ_32_2(0L, Z_NULL, 0) :
#اگر_ختم
        adler32(0L, Z_NULL, 0);
    s->آخری_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    واپس Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local خالی lm_init(deflate_state *s) {
    s->کھڑکی_سائز = (ulg)2L*s->w_سائز;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_مطابقت   = configuration_جدول[s->level].max_lazy;
    s->good_مطابقت       = configuration_جدول[s->level].good_لمبائی;
    s->nice_مطابقت       = configuration_جدول[s->level].nice_لمبائی;
    s->max_chain_لمبائی = configuration_جدول[s->level].max_chain;

    s->strstart = 0;
    s->بلاک_آغاز = 0L;
    s->lookahead = 0;
    s->داخل_کرو = 0;
    s->مطابقت_لمبائی = s->پچھلا_لمبائی = MIN_مطابقت-1;
    s->مطابقت_دستیاب = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
صحیح ZEXPORT deflateReset(z_streamp strm) {
    صحیح ret;

    ret = deflateResetKeep(strm);
    اگر (ret == Z_OK)
        lm_init(strm->state);
    واپس ret;
}

/* ========================================================================= */
صحیح ZEXPORT deflate_سیٹ_سرخی(z_streamp strm, gz_headerp سر) {
    اگر (deflateStateCheck(strm) || strm->state->wrap != 2)
        واپس Z_دھارا_ERROR;
    strm->state->gzhead = سر;
    واپس Z_OK;
}

/* ========================================================================= */
صحیح ZEXPORT deflatePending(z_streamp strm, غیرعلامتی *pending, صحیح *بٹ) {
    اگر (deflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    اگر (بٹ != Z_NULL)
        *بٹ = strm->state->bi_valid;
    اگر (pending != Z_NULL) {
        *pending = (غیرعلامتی)strm->state->pending;
        اگر (*pending != strm->state->pending) {
            *pending = (غیرعلامتی)-1;
            واپس Z_BUF_ERROR;
        }
    }
    واپس Z_OK;
}

/* ========================================================================= */
صحیح ZEXPORT deflate_مستعمل(z_streamp strm, صحیح *بٹ) {
    اگر (deflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    اگر (بٹ != Z_NULL)
        *بٹ = strm->state->bi_مستعمل;
    واپس Z_OK;
}

/* ========================================================================= */
صحیح ZEXPORT deflate_مفرد(z_streamp strm, صحیح بٹ, صحیح قدر) {
    deflate_state *s;
    صحیح put;

    اگر (deflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    s = strm->state;
#اگر_تعریف_شدہ LIT_MEM
    اگر (بٹ < 0 || بٹ > 16 ||
        (uchf *)s->d_buf < s->pending_برآمد + ((Buf_سائز + 7) >> 3))
        واپس Z_BUF_ERROR;
#ورنہ
    اگر (بٹ < 0 || بٹ > 16 ||
        s->sym_buf < s->pending_برآمد + ((Buf_سائز + 7) >> 3))
        واپس Z_BUF_ERROR;
#اگر_ختم
    کرو {
        put = Buf_سائز - s->bi_valid;
        اگر (put > بٹ)
            put = بٹ;
        s->bi_buf |= (ush)((قدر & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_بٹ(s);
        قدر >>= put;
        بٹ -= put;
    } جب (بٹ);
    واپس Z_OK;
}

/* ========================================================================= */
صحیح ZEXPORT deflateParams(z_streamp strm, صحیح level, صحیح strategy) {
    deflate_state *s;
    compress_func func;

    اگر (deflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    s = strm->state;

#اگر_تعریف_شدہ FASTEST
    اگر (level != 0) level = 1;
#ورنہ
    اگر (level == Z_DEFAULT_COMPRESSION) level = 6;
#اگر_ختم
    اگر (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        واپس Z_دھارا_ERROR;
    }
    func = configuration_جدول[s->level].func;

    اگر ((strategy != s->strategy || func != configuration_جدول[level].func) &&
        s->آخری_flush != -2) {
        /* Flush the last buffer: */
        صحیح err = deflate(strm, Z_بلاک);
        اگر (err == Z_دھارا_ERROR)
            واپس err;
        اگر (strm->avail_اندر || (s->strstart - s->بلاک_آغاز) + s->lookahead)
            واپس Z_BUF_ERROR;
    }
    اگر (s->level != level) {
        اگر (s->level == 0 && s->matches != 0) {
            اگر (s->matches == 1)
                slide_hash(s);
            ورنہ
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_مطابقت   = configuration_جدول[level].max_lazy;
        s->good_مطابقت       = configuration_جدول[level].good_لمبائی;
        s->nice_مطابقت       = configuration_جدول[level].nice_لمبائی;
        s->max_chain_لمبائی = configuration_جدول[level].max_chain;
    }
    s->strategy = strategy;
    واپس Z_OK;
}

/* ========================================================================= */
صحیح ZEXPORT deflateTune(z_streamp strm, صحیح good_لمبائی, صحیح max_lazy,
                        صحیح nice_لمبائی, صحیح max_chain) {
    deflate_state *s;

    اگر (deflateStateCheck(strm)) واپس Z_دھارا_ERROR;
    s = strm->state;
    s->good_مطابقت = (uInt)good_لمبائی;
    s->max_lazy_مطابقت = (uInt)max_lazy;
    s->nice_مطابقت = nice_لمبائی;
    s->max_chain_لمبائی = (uInt)max_chain;
    واپس Z_OK;
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
z_سائز_t ZEXPORT deflateBound_z(z_streamp strm, z_سائز_t ماخذ_Len) {
    deflate_state *s;
    z_سائز_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = ماخذ_Len + (ماخذ_Len >> 3) + (ماخذ_Len >> 8) +
               (ماخذ_Len >> 9) + 4;
    اگر (fixedlen < ماخذ_Len)
        fixedlen = (z_سائز_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = ماخذ_Len + (ماخذ_Len >> 5) + (ماخذ_Len >> 7) +
               (ماخذ_Len >> 11) + 7;
    اگر (storelen < ماخذ_Len)
        storelen = (z_سائز_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    اگر (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        واپس bound + 18 < bound ? (z_سائز_t)-1 : bound + 18;
    }

    /* compute wrapper length */
    s = strm->state;
    انتخاب (s->wrap < 0 ? -s->wrap : s->wrap) {
    حالت 0:                                 /* raw deflate */
        wraplen = 0;
        توڑو;
    حالت 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        توڑو;
#اگر_تعریف_شدہ GZIP
    حالت 2:                                 /* gzip wrapper */
        wraplen = 18;
        اگر (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            اگر (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            اگر (str != Z_NULL)
                کرو {
                    wraplen++;
                } جب (*str++);
            str = s->gzhead->comment;
            اگر (str != Z_NULL)
                کرو {
                    wraplen++;
                } جب (*str++);
            اگر (s->gzhead->hcrc)
                wraplen += 2;
        }
        توڑو;
#اگر_ختم
    طےشدہ:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    اگر (s->w_بٹ != 15 || s->hash_بٹ != 8 + 7) {
        bound = s->w_بٹ <= s->hash_بٹ && s->level ? fixedlen :
                                                        storelen;
        واپس bound + wraplen < bound ? (z_سائز_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = ماخذ_Len + (ماخذ_Len >> 12) + (ماخذ_Len >> 14) +
            (ماخذ_Len >> 25) + 13 - 6 + wraplen;
    واپس bound < ماخذ_Len ? (z_سائز_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong ماخذ_Len) {
    z_سائز_t bound = deflateBound_z(strm, ماخذ_Len);
    واپس (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local خالی putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local خالی flush_pending(z_streamp strm) {
    غیرعلامتی len;
    deflate_state *s = strm->state;

    tr_flush_بٹ(s);
    len = s->pending > strm->avail_برآمد ? strm->avail_برآمد :
                                         (غیرعلامتی)s->pending;
    اگر (len == 0) واپس;

    zmemcpy(strm->اگلا_برآمد, s->pending_برآمد, len);
    strm->اگلا_برآمد  += len;
    s->pending_برآمد  += len;
    strm->کل_برآمد += len;
    strm->avail_برآمد -= len;
    s->pending      -= len;
    اگر (s->pending == 0) {
        s->pending_برآمد = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#تعریف_کرو HCRC_UPDATE(beg) \
    کرو { \
        اگر (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = چکری_جانچ_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } جب (0)

/* ========================================================================= */
صحیح ZEXPORT deflate(z_streamp strm, صحیح flush) {
    صحیح old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    اگر (deflateStateCheck(strm) || flush > Z_بلاک || flush < 0) {
        واپس Z_دھارا_ERROR;
    }
    s = strm->state;

    اگر (strm->اگلا_برآمد == Z_NULL ||
        (strm->avail_اندر != 0 && strm->اگلا_اندر == Z_NULL) ||
        (s->حالت_2 == اختتام_STATE && flush != Z_اختتام)) {
        ERR_RETURN(strm, Z_دھارا_ERROR);
    }
    اگر (strm->avail_برآمد == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->آخری_flush;
    s->آخری_flush = flush;

    /* Flush as much pending output as possible */
    اگر (s->pending != 0) {
        flush_pending(strm);
        اگر (strm->avail_برآمد == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->آخری_flush = -1;
            واپس Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } ورنہ اگر (strm->avail_اندر == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_اختتام) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    اگر (s->حالت_2 == اختتام_STATE && strm->avail_اندر != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    اگر (s->حالت_2 == INIT_STATE && s->wrap == 0)
        s->حالت_2 = BUSY_STATE;
    اگر (s->حالت_2 == INIT_STATE) {
        /* zlib header */
        uInt سرخی = (Z_DEFLATED + ((s->w_بٹ - 8) << 4)) << 8;
        uInt level_flags;

        اگر (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        ورنہ اگر (s->level < 6)
            level_flags = 1;
        ورنہ اگر (s->level == 6)
            level_flags = 2;
        ورنہ
            level_flags = 3;
        سرخی |= (level_flags << 6);
        اگر (s->strstart != 0) سرخی |= PRESET_DICT;
        سرخی += 31 - (سرخی % 31);

        putShortMSB(s, سرخی);

        /* Save the adler32 of the preset dictionary: */
        اگر (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->حالت_2 = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        اگر (s->pending != 0) {
            s->آخری_flush = -1;
            واپس Z_OK;
        }
    }
#اگر_تعریف_شدہ GZIP
    اگر (s->حالت_2 == GZIP_STATE) {
        /* gzip header */
        strm->adler = چکری_جانچ_32_2(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        اگر (s->gzhead == Z_NULL) {
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, OS_رمز);
            s->حالت_2 = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            اگر (s->pending != 0) {
                s->آخری_flush = -1;
                واپس Z_OK;
            }
        }
        ورنہ {
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
            اگر (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            اگر (s->gzhead->hcrc)
                strm->adler = چکری_جانچ_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->حالت_2 = EXTRA_STATE;
        }
    }
    اگر (s->حالت_2 == EXTRA_STATE) {
        اگر (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg بایاں = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            جب (s->pending + بایاں > s->pending_buf_سائز) {
                ulg copy = s->pending_buf_سائز - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_سائز;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                اگر (s->pending != 0) {
                    s->آخری_flush = -1;
                    واپس Z_OK;
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
        s->حالت_2 = NAME_STATE;
    }
    اگر (s->حالت_2 == NAME_STATE) {
        اگر (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            صحیح val;
            کرو {
                اگر (s->pending == s->pending_buf_سائز) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    اگر (s->pending != 0) {
                        s->آخری_flush = -1;
                        واپس Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } جب (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->حالت_2 = COMMENT_STATE;
    }
    اگر (s->حالت_2 == COMMENT_STATE) {
        اگر (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            صحیح val;
            کرو {
                اگر (s->pending == s->pending_buf_سائز) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    اگر (s->pending != 0) {
                        s->آخری_flush = -1;
                        واپس Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } جب (val != 0);
            HCRC_UPDATE(beg);
        }
        s->حالت_2 = HCRC_STATE;
    }
    اگر (s->حالت_2 == HCRC_STATE) {
        اگر (s->gzhead->hcrc) {
            اگر (s->pending + 2 > s->pending_buf_سائز) {
                flush_pending(strm);
                اگر (s->pending != 0) {
                    s->آخری_flush = -1;
                    واپس Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = چکری_جانچ_32_2(0L, Z_NULL, 0);
        }
        s->حالت_2 = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        اگر (s->pending != 0) {
            s->آخری_flush = -1;
            واپس Z_OK;
        }
    }
#اگر_ختم

    /* Start a new block or continue the current one.
     */
    اگر (strm->avail_اندر != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->حالت_2 != اختتام_STATE)) {
        بلاک_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_جدول[s->level].func))(s, flush);

        اگر (bstate == اختتام_started || bstate == اختتام_مکمل) {
            s->حالت_2 = اختتام_STATE;
        }
        اگر (bstate == need_مزید || bstate == اختتام_started) {
            اگر (strm->avail_برآمد == 0) {
                s->آخری_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            واپس Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        اگر (bstate == بلاک_مکمل) {
            اگر (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } ورنہ اگر (flush != Z_بلاک) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_بلاک(s, (حرف*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                اگر (flush == Z_بھرا_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    اگر (s->lookahead == 0) {
                        s->strstart = 0;
                        s->بلاک_آغاز = 0L;
                        s->داخل_کرو = 0;
                    }
                }
            }
            flush_pending(strm);
            اگر (strm->avail_برآمد == 0) {
              s->آخری_flush = -1; /* avoid BUF_ERROR at next call, see above */
              واپس Z_OK;
            }
        }
    }

    اگر (flush != Z_اختتام) واپس Z_OK;
    اگر (s->wrap <= 0) واپس Z_دھارا_انتہا;

    /* Write the trailer */
#اگر_تعریف_شدہ GZIP
    اگر (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->کل_اندر & 0xff));
        put_byte(s, (Byte)((strm->کل_اندر >> 8) & 0xff));
        put_byte(s, (Byte)((strm->کل_اندر >> 16) & 0xff));
        put_byte(s, (Byte)((strm->کل_اندر >> 24) & 0xff));
    }
    ورنہ
#اگر_ختم
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    اگر (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    واپس s->pending != 0 ? Z_OK : Z_دھارا_انتہا;
}

/* ========================================================================= */
صحیح ZEXPORT deflate_انتہا(z_streamp strm) {
    صحیح حالت_2;

    اگر (deflateStateCheck(strm)) واپس Z_دھارا_ERROR;

    حالت_2 = strm->state->حالت_2;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->سر);
    TRY_FREE(strm, strm->state->پچھلا);
    TRY_FREE(strm, strm->state->کھڑکی);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    واپس حالت_2 == BUSY_STATE ? Z_ڈیٹا_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
صحیح ZEXPORT deflateCopy(z_streamp dest, z_streamp ماخذ) {
#اگر_تعریف_شدہ MAXSEG_64K
    (خالی)dest;
    (خالی)ماخذ;
    واپس Z_دھارا_ERROR;
#ورنہ
    deflate_state *ds;
    deflate_state *ss;


    اگر (deflateStateCheck(ماخذ) || dest == Z_NULL) {
        واپس Z_دھارا_ERROR;
    }

    ss = ماخذ->state;

    zmemcpy(dest, ماخذ, حجم(z_دھارا));

    ds = (deflate_state *) ZALLOC(dest, 1, حجم(deflate_state));
    اگر (ds == Z_NULL) واپس Z_MEM_ERROR;
    zmemzero(ds, حجم(deflate_state));
    dest->state = (ساخت اندرونی_state FAR *) ds;
    zmemcpy(ds, ss, حجم(deflate_state));
    ds->strm = dest;

    ds->کھڑکی = (Bytef *) ZALLOC(dest, ds->w_سائز, 2*حجم(Byte));
    ds->پچھلا   = (Posf *)  ZALLOC(dest, ds->w_سائز, حجم(Pos));
    ds->سر   = (Posf *)  ZALLOC(dest, ds->hash_سائز, حجم(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    اگر (ds->کھڑکی == Z_NULL || ds->پچھلا == Z_NULL || ds->سر == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_انتہا (dest);
        واپس Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->کھڑکی, ss->کھڑکی, ss->بلند_water);
    zmemcpy(ds->پچھلا, ss->پچھلا,
            (ss->slid || ss->strstart - ss->داخل_کرو > ds->w_سائز ? ds->w_سائز :
                ss->strstart - ss->داخل_کرو) * حجم(Pos));
    zmemcpy(ds->سر, ss->سر, ds->hash_سائز * حجم(Pos));

    ds->pending_برآمد = ds->pending_buf + (ss->pending_برآمد - ss->pending_buf);
    zmemcpy(ds->pending_برآمد, ss->pending_برآمد, ss->pending);
#اگر_تعریف_شدہ LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_اگلا * حجم(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_اگلا);
#ورنہ
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_اگلا);
#اگر_ختم

    ds->l_desc.dyn_درخت = ds->dyn_ltree;
    ds->d_desc.dyn_درخت = ds->dyn_dtree;
    ds->bl_desc.dyn_درخت = ds->bl_درخت;

    واپس Z_OK;
#اگر_ختم /* MAXSEG_64K */
}

#اگر_غیر_تعریف_شدہ FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_مطابقت(deflate_state *s, IPos cur_مطابقت) {
    غیرعلامتی chain_لمبائی = s->max_chain_لمبائی;/* max hash chain length */
    Bytef *جائزہ = s->کھڑکی + s->strstart;      /* current string */
    Bytef *مطابقت;                               /* matched string */
    صحیح len;                                    /* length of current match */
    صحیح بہترین_len = (صحیح)s->پچھلا_لمبائی;         /* best match length so far */
    صحیح nice_مطابقت = s->nice_مطابقت;             /* stop if match long enough */
    IPos حد_بندی = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *پچھلا = s->پچھلا;
    uInt wmask = s->w_نقاب;

#اگر_تعریف_شدہ UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->کھڑکی + s->strstart + MAX_مطابقت - 1;
    ush جائزہ_آغاز = *(ushf*)جائزہ;
    ush جائزہ_انتہا   = *(ushf*)(جائزہ + بہترین_len - 1);
#ورنہ
    Bytef *strend = s->کھڑکی + s->strstart + MAX_مطابقت;
    Byte جائزہ_end1  = جائزہ[بہترین_len - 1];
    Byte جائزہ_انتہا   = جائزہ[بہترین_len];
#اگر_ختم

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بٹ >= 8 && MAX_مطابقت == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    اگر (s->پچھلا_لمبائی >= s->good_مطابقت) {
        chain_لمبائی >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    اگر ((uInt)nice_مطابقت > s->lookahead) nice_مطابقت = (صحیح)s->lookahead;

    Assert((ulg)s->strstart <= s->کھڑکی_سائز - MIN_LOOKAHEAD,
           "need lookahead");

    کرو {
        Assert(cur_مطابقت < s->strstart, "no future");
        مطابقت = s->کھڑکی + cur_مطابقت;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#اگر (defined(UNALIGNED_OK) && MAX_مطابقت == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        اگر (*(ushf*)(مطابقت + بہترین_len - 1) != جائزہ_انتہا ||
            *(ushf*)مطابقت != جائزہ_آغاز) جاری;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(جائزہ[2] == مطابقت[2], "scan[2]?");
        جائزہ++, مطابقت++;
        کرو {
        } جب (*(ushf*)(جائزہ += 2) == *(ushf*)(مطابقت += 2) &&
                 *(ushf*)(جائزہ += 2) == *(ushf*)(مطابقت += 2) &&
                 *(ushf*)(جائزہ += 2) == *(ushf*)(مطابقت += 2) &&
                 *(ushf*)(جائزہ += 2) == *(ushf*)(مطابقت += 2) &&
                 جائزہ < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(جائزہ <= s->کھڑکی + (غیرعلامتی)(s->کھڑکی_سائز - 1),
               "wild scan");
        اگر (*جائزہ == *مطابقت) جائزہ++;

        len = (MAX_مطابقت - 1) - (صحیح)(strend - جائزہ);
        جائزہ = strend - (MAX_مطابقت-1);

#ورنہ /* UNALIGNED_OK */

        اگر (مطابقت[بہترین_len]     != جائزہ_انتہا  ||
            مطابقت[بہترین_len - 1] != جائزہ_end1 ||
            *مطابقت              != *جائزہ     ||
            *++مطابقت            != جائزہ[1])      جاری;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        جائزہ += 2, مطابقت++;
        Assert(*جائزہ == *مطابقت, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        کرو {
        } جب (*++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
                 *++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
                 *++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
                 *++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
                 جائزہ < strend);

        Assert(جائزہ <= s->کھڑکی + (غیرعلامتی)(s->کھڑکی_سائز - 1),
               "wild scan");

        len = MAX_مطابقت - (صحیح)(strend - جائزہ);
        جائزہ = strend - MAX_مطابقت;

#اگر_ختم /* UNALIGNED_OK */

        اگر (len > بہترین_len) {
            s->مطابقت_آغاز = cur_مطابقت;
            بہترین_len = len;
            اگر (len >= nice_مطابقت) توڑو;
#اگر_تعریف_شدہ UNALIGNED_OK
            جائزہ_انتہا = *(ushf*)(جائزہ + بہترین_len - 1);
#ورنہ
            جائزہ_end1  = جائزہ[بہترین_len - 1];
            جائزہ_انتہا   = جائزہ[بہترین_len];
#اگر_ختم
        }
    } جب ((cur_مطابقت = پچھلا[cur_مطابقت & wmask]) > حد_بندی
             && --chain_لمبائی != 0);

    اگر ((uInt)بہترین_len <= s->lookahead) واپس (uInt)بہترین_len;
    واپس s->lookahead;
}

#ورنہ /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_مطابقت(deflate_state *s, IPos cur_مطابقت) {
    Bytef *جائزہ = s->کھڑکی + s->strstart;      /* current string */
    Bytef *مطابقت;                               /* matched string */
    صحیح len;                                    /* length of current match */
    Bytef *strend = s->کھڑکی + s->strstart + MAX_مطابقت;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بٹ >= 8 && MAX_مطابقت == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->کھڑکی_سائز - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_مطابقت < s->strstart, "no future");

    مطابقت = s->کھڑکی + cur_مطابقت;

    /* Return failure if the match length is less than 2:
     */
    اگر (مطابقت[0] != جائزہ[0] || مطابقت[1] != جائزہ[1]) واپس MIN_مطابقت-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    جائزہ += 2, مطابقت += 2;
    Assert(*جائزہ == *مطابقت, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    کرو {
    } جب (*++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
             *++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
             *++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
             *++جائزہ == *++مطابقت && *++جائزہ == *++مطابقت &&
             جائزہ < strend);

    Assert(جائزہ <= s->کھڑکی + (غیرعلامتی)(s->کھڑکی_سائز - 1), "wild scan");

    len = MAX_مطابقت - (صحیح)(strend - جائزہ);

    اگر (len < MIN_مطابقت) واپس MIN_مطابقت - 1;

    s->مطابقت_آغاز = cur_مطابقت;
    واپس (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#اگر_ختم /* FASTEST */

#اگر_تعریف_شدہ ZLIB_DEBUG

#تعریف_کرو EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local خالی check_مطابقت(deflate_state *s, IPos آغاز, IPos مطابقت, صحیح لمبائی) {
    /* check that the match is indeed a match */
    Bytef *عقب = s->کھڑکی + (صحیح)مطابقت, *یہاں = s->کھڑکی + آغاز;
    IPos len = (IPos)لمبائی;
    اگر (مطابقت == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        عقب++;
        یہاں++;
        len--;
    }
    اگر (zmemcmp(عقب, یہاں, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                آغاز, (صحیح)مطابقت, لمبائی);
        کرو {
            fprintf(stderr, "(%02x %02x)", *عقب++, *یہاں++);
        } جب (--len != 0);
        z_error("invalid match");
    }
    اگر (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", آغاز - مطابقت, لمبائی);
        کرو { putc(s->کھڑکی[آغاز++], stderr); } جب (--لمبائی != 0);
    }
}
#ورنہ
#  define check_مطابقت(s, آغاز, مطابقت, لمبائی)
#اگر_ختم /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#تعریف_کرو FLUSH_بلاک_ONLY(s, آخری) { \
   tr_flush_بلاک(s, (s->بلاک_آغاز >= 0L ? \
                   (charf *)&s->کھڑکی[(غیرعلامتی)s->بلاک_آغاز] : \
                   (charf *)Z_NULL), \
                (ulg)((طویل)s->strstart - s->بلاک_آغاز), \
                (آخری)); \
   s->بلاک_آغاز = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#تعریف_کرو FLUSH_بلاک(s, آخری) { \
   FLUSH_بلاک_ONLY(s, آخری); \
   اگر (s->strm->avail_برآمد == 0) واپس (آخری) ? اختتام_started : need_مزید; \
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
local بلاک_state deflate_stored(deflate_state *s, صحیح flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    غیرعلامتی min_بلاک = (غیرعلامتی)(MIN(s->pending_buf_سائز - 5, s->w_سائز));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    صحیح آخری = 0;
    غیرعلامتی len, بایاں, have;
    غیرعلامتی مستعمل = s->strm->avail_اندر;
    کرو {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((غیرعلامتی)s->bi_valid + 42) >> 3;   /* bytes in header */
        اگر (s->strm->avail_برآمد < have)          /* need room for header */
            توڑو;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_برآمد - have;
        بایاں = (غیرعلامتی)(s->strstart - s->بلاک_آغاز);    /* window bytes */
        اگر (len > (ulg)بایاں + s->strm->avail_اندر)
            len = بایاں + s->strm->avail_اندر;     /* limit len to the input */
        اگر (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        اگر (len < min_بلاک && ((len == 0 && flush != Z_اختتام) ||
                                flush == Z_NO_FLUSH ||
                                len != بایاں + s->strm->avail_اندر))
            توڑو;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        آخری = flush == Z_اختتام && len == بایاں + s->strm->avail_اندر ? 1 : 0;
        tr_stored_بلاک(s, (حرف *)0, 0L, آخری);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#اگر_تعریف_شدہ ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->بٹ_بھیجا += len << 3;
#اگر_ختم

        /* Copy uncompressed bytes from the window to next_out. */
        اگر (بایاں) {
            اگر (بایاں > len)
                بایاں = len;
            zmemcpy(s->strm->اگلا_برآمد, s->کھڑکی + s->بلاک_آغاز, بایاں);
            s->strm->اگلا_برآمد += بایاں;
            s->strm->avail_برآمد -= بایاں;
            s->strm->کل_برآمد += بایاں;
            s->بلاک_آغاز += بایاں;
            len -= بایاں;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        اگر (len) {
            read_buf(s->strm, s->strm->اگلا_برآمد, len);
            s->strm->اگلا_برآمد += len;
            s->strm->avail_برآمد -= len;
            s->strm->کل_برآمد += len;
        }
    } جب (آخری == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    مستعمل -= s->strm->avail_اندر;      /* number of input bytes directly copied */
    اگر (مستعمل) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        اگر (مستعمل >= s->w_سائز) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->کھڑکی, s->strm->اگلا_اندر - s->w_سائز, s->w_سائز);
            s->strstart = s->w_سائز;
            s->داخل_کرو = s->strstart;
        }
        ورنہ {
            اگر (s->کھڑکی_سائز - s->strstart <= مستعمل) {
                /* Slide the window down. */
                s->strstart -= s->w_سائز;
                zmemcpy(s->کھڑکی, s->کھڑکی + s->w_سائز, s->strstart);
                اگر (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                اگر (s->داخل_کرو > s->strstart)
                    s->داخل_کرو = s->strstart;
            }
            zmemcpy(s->کھڑکی + s->strstart, s->strm->اگلا_اندر - مستعمل, مستعمل);
            s->strstart += مستعمل;
            s->داخل_کرو += MIN(مستعمل, s->w_سائز - s->داخل_کرو);
        }
        s->بلاک_آغاز = s->strstart;
    }
    اگر (s->بلند_water < s->strstart)
        s->بلند_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    اگر (آخری) {
        s->bi_مستعمل = 8;
        واپس اختتام_مکمل;
    }

    /* If flushing and all input has been consumed, then done. */
    اگر (flush != Z_NO_FLUSH && flush != Z_اختتام &&
        s->strm->avail_اندر == 0 && (طویل)s->strstart == s->بلاک_آغاز)
        واپس بلاک_مکمل;

    /* Fill the window with any remaining input. */
    have = (غیرعلامتی)(s->کھڑکی_سائز - s->strstart);
    اگر (s->strm->avail_اندر > have && s->بلاک_آغاز >= (طویل)s->w_سائز) {
        /* Slide the window down. */
        s->بلاک_آغاز -= s->w_سائز;
        s->strstart -= s->w_سائز;
        zmemcpy(s->کھڑکی, s->کھڑکی + s->w_سائز, s->strstart);
        اگر (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_سائز;          /* more space now */
        اگر (s->داخل_کرو > s->strstart)
            s->داخل_کرو = s->strstart;
    }
    اگر (have > s->strm->avail_اندر)
        have = s->strm->avail_اندر;
    اگر (have) {
        read_buf(s->strm, s->کھڑکی + s->strstart, have);
        s->strstart += have;
        s->داخل_کرو += MIN(have, s->w_سائز - s->داخل_کرو);
    }
    اگر (s->بلند_water < s->strstart)
        s->بلند_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((غیرعلامتی)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (غیرعلامتی)MIN(s->pending_buf_سائز - have, MAX_STORED);
    min_بلاک = MIN(have, s->w_سائز);
    بایاں = (غیرعلامتی)(s->strstart - s->بلاک_آغاز);
    اگر (بایاں >= min_بلاک ||
        ((بایاں || flush == Z_اختتام) && flush != Z_NO_FLUSH &&
         s->strm->avail_اندر == 0 && بایاں <= have)) {
        len = MIN(بایاں, have);
        آخری = flush == Z_اختتام && s->strm->avail_اندر == 0 &&
               len == بایاں ? 1 : 0;
        tr_stored_بلاک(s, (charf *)s->کھڑکی + s->بلاک_آغاز, len, آخری);
        s->بلاک_آغاز += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    اگر (آخری)
        s->bi_مستعمل = 8;
    واپس آخری ? اختتام_started : need_مزید;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local بلاک_state deflate_fast(deflate_state *s, صحیح flush) {
    IPos hash_سر;       /* head of the hash chain */
    صحیح bflush;           /* set if current block must be flushed */

    برائے (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        اگر (s->lookahead < MIN_LOOKAHEAD) {
            fill_کھڑکی(s);
            اگر (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                واپس need_مزید;
            }
            اگر (s->lookahead == 0) توڑو; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        اگر (s->lookahead >= MIN_مطابقت) {
            داخل_کرو_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        اگر (hash_سر != NIL && s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->مطابقت_لمبائی = longest_مطابقت (s, hash_سر);
            /* longest_match() sets match_start */
        }
        اگر (s->مطابقت_لمبائی >= MIN_مطابقت) {
            check_مطابقت(s, s->strstart, s->مطابقت_آغاز, (صحیح)s->مطابقت_لمبائی);

            _tr_tally_dist(s, s->strstart - s->مطابقت_آغاز,
                           s->مطابقت_لمبائی - MIN_مطابقت, bflush);

            s->lookahead -= s->مطابقت_لمبائی;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#اگر_غیر_تعریف_شدہ FASTEST
            اگر (s->مطابقت_لمبائی <= s->max_داخل_کرو_لمبائی &&
                s->lookahead >= MIN_مطابقت) {
                s->مطابقت_لمبائی--; /* string at strstart already in table */
                کرو {
                    s->strstart++;
                    داخل_کرو_STRING(s, s->strstart, hash_سر);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } جب (--s->مطابقت_لمبائی != 0);
                s->strstart++;
            } ورنہ
#اگر_ختم
            {
                s->strstart += s->مطابقت_لمبائی;
                s->مطابقت_لمبائی = 0;
                s->ins_h = s->کھڑکی[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->کھڑکی[s->strstart + 1]);
#اگر MIN_مطابقت != 3
                Call UPDATE_HASH() MIN_مطابقت-3 مزید times
#اگر_ختم
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } ورنہ {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->کھڑکی[s->strstart]));
            _tr_tally_lit(s, s->کھڑکی[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        اگر (bflush) FLUSH_بلاک(s, 0);
    }
    s->داخل_کرو = s->strstart < MIN_مطابقت-1 ? s->strstart : MIN_مطابقت-1;
    اگر (flush == Z_اختتام) {
        FLUSH_بلاک(s, 1);
        واپس اختتام_مکمل;
    }
    اگر (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    واپس بلاک_مکمل;
}

#اگر_غیر_تعریف_شدہ FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local بلاک_state deflate_slow(deflate_state *s, صحیح flush) {
    IPos hash_سر;          /* head of hash chain */
    صحیح bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    برائے (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        اگر (s->lookahead < MIN_LOOKAHEAD) {
            fill_کھڑکی(s);
            اگر (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                واپس need_مزید;
            }
            اگر (s->lookahead == 0) توڑو; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        اگر (s->lookahead >= MIN_مطابقت) {
            داخل_کرو_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->پچھلا_لمبائی = s->مطابقت_لمبائی, s->پچھلا_مطابقت = s->مطابقت_آغاز;
        s->مطابقت_لمبائی = MIN_مطابقت-1;

        اگر (hash_سر != NIL && s->پچھلا_لمبائی < s->max_lazy_مطابقت &&
            s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->مطابقت_لمبائی = longest_مطابقت (s, hash_سر);
            /* longest_match() sets match_start */

            اگر (s->مطابقت_لمبائی <= 5 && (s->strategy == Z_FILTERED
#اگر TOO_FAR <= 32767
                || (s->مطابقت_لمبائی == MIN_مطابقت &&
                    s->strstart - s->مطابقت_آغاز > TOO_FAR)
#اگر_ختم
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->مطابقت_لمبائی = MIN_مطابقت-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        اگر (s->پچھلا_لمبائی >= MIN_مطابقت && s->مطابقت_لمبائی <= s->پچھلا_لمبائی) {
            uInt max_داخل_کرو = s->strstart + s->lookahead - MIN_مطابقت;
            /* Do not insert strings in hash table beyond this. */

            check_مطابقت(s, s->strstart - 1, s->پچھلا_مطابقت, (صحیح)s->پچھلا_لمبائی);

            _tr_tally_dist(s, s->strstart - 1 - s->پچھلا_مطابقت,
                           s->پچھلا_لمبائی - MIN_مطابقت, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->پچھلا_لمبائی - 1;
            s->پچھلا_لمبائی -= 2;
            کرو {
                اگر (++s->strstart <= max_داخل_کرو) {
                    داخل_کرو_STRING(s, s->strstart, hash_سر);
                }
            } جب (--s->پچھلا_لمبائی != 0);
            s->مطابقت_دستیاب = 0;
            s->مطابقت_لمبائی = MIN_مطابقت-1;
            s->strstart++;

            اگر (bflush) FLUSH_بلاک(s, 0);

        } ورنہ اگر (s->مطابقت_دستیاب) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->کھڑکی[s->strstart - 1]));
            _tr_tally_lit(s, s->کھڑکی[s->strstart - 1], bflush);
            اگر (bflush) {
                FLUSH_بلاک_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            اگر (s->strm->avail_برآمد == 0) واپس need_مزید;
        } ورنہ {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->مطابقت_دستیاب = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    اگر (s->مطابقت_دستیاب) {
        Tracevv((stderr,"%c", s->کھڑکی[s->strstart - 1]));
        _tr_tally_lit(s, s->کھڑکی[s->strstart - 1], bflush);
        s->مطابقت_دستیاب = 0;
    }
    s->داخل_کرو = s->strstart < MIN_مطابقت-1 ? s->strstart : MIN_مطابقت-1;
    اگر (flush == Z_اختتام) {
        FLUSH_بلاک(s, 1);
        واپس اختتام_مکمل;
    }
    اگر (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    واپس بلاک_مکمل;
}
#اگر_ختم /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local بلاک_state deflate_rle(deflate_state *s, صحیح flush) {
    صحیح bflush;             /* set if current block must be flushed */
    uInt پچھلا;              /* byte at distance one to match */
    Bytef *جائزہ, *strend;   /* scan goes up to strend for length of run */

    برائے (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        اگر (s->lookahead <= MAX_مطابقت) {
            fill_کھڑکی(s);
            اگر (s->lookahead <= MAX_مطابقت && flush == Z_NO_FLUSH) {
                واپس need_مزید;
            }
            اگر (s->lookahead == 0) توڑو; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->مطابقت_لمبائی = 0;
        اگر (s->lookahead >= MIN_مطابقت && s->strstart > 0) {
            جائزہ = s->کھڑکی + s->strstart - 1;
            پچھلا = *جائزہ;
            اگر (پچھلا == *++جائزہ && پچھلا == *++جائزہ && پچھلا == *++جائزہ) {
                strend = s->کھڑکی + s->strstart + MAX_مطابقت;
                کرو {
                } جب (پچھلا == *++جائزہ && پچھلا == *++جائزہ &&
                         پچھلا == *++جائزہ && پچھلا == *++جائزہ &&
                         پچھلا == *++جائزہ && پچھلا == *++جائزہ &&
                         پچھلا == *++جائزہ && پچھلا == *++جائزہ &&
                         جائزہ < strend);
                s->مطابقت_لمبائی = MAX_مطابقت - (uInt)(strend - جائزہ);
                اگر (s->مطابقت_لمبائی > s->lookahead)
                    s->مطابقت_لمبائی = s->lookahead;
            }
            Assert(جائزہ <= s->کھڑکی + (uInt)(s->کھڑکی_سائز - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        اگر (s->مطابقت_لمبائی >= MIN_مطابقت) {
            check_مطابقت(s, s->strstart, s->strstart - 1, (صحیح)s->مطابقت_لمبائی);

            _tr_tally_dist(s, 1, s->مطابقت_لمبائی - MIN_مطابقت, bflush);

            s->lookahead -= s->مطابقت_لمبائی;
            s->strstart += s->مطابقت_لمبائی;
            s->مطابقت_لمبائی = 0;
        } ورنہ {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->کھڑکی[s->strstart]));
            _tr_tally_lit(s, s->کھڑکی[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        اگر (bflush) FLUSH_بلاک(s, 0);
    }
    s->داخل_کرو = 0;
    اگر (flush == Z_اختتام) {
        FLUSH_بلاک(s, 1);
        واپس اختتام_مکمل;
    }
    اگر (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    واپس بلاک_مکمل;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local بلاک_state deflate_huff(deflate_state *s, صحیح flush) {
    صحیح bflush;             /* set if current block must be flushed */

    برائے (;;) {
        /* Make sure that we have a literal to write. */
        اگر (s->lookahead == 0) {
            fill_کھڑکی(s);
            اگر (s->lookahead == 0) {
                اگر (flush == Z_NO_FLUSH)
                    واپس need_مزید;
                توڑو;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->مطابقت_لمبائی = 0;
        Tracevv((stderr,"%c", s->کھڑکی[s->strstart]));
        _tr_tally_lit(s, s->کھڑکی[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        اگر (bflush) FLUSH_بلاک(s, 0);
    }
    s->داخل_کرو = 0;
    اگر (flush == Z_اختتام) {
        FLUSH_بلاک(s, 1);
        واپس اختتام_مکمل;
    }
    اگر (s->sym_اگلا)
        FLUSH_بلاک(s, 0);
    واپس بلاک_مکمل;
}
