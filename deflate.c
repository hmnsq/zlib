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
    need_نور,      /* block not completed, need more input or more output */
    بلاک_بشپړ,     /* block flush performed */
    پای_started, /* finish started, need only more output at next deflate */
    پای_بشپړ     /* finish done, accept no more input or output */
} بلاک_state;

تعريفنوع بلاک_state (*compress_func)(deflate_state *s, صحيح flush);
/* Compression function. Returns the block state after the call. */

local بلاک_state deflate_stored(deflate_state *s, صحيح flush);
local بلاک_state deflate_fast(deflate_state *s, صحيح flush);
#که_نه_تعریف_شوی FASTEST
local بلاک_state deflate_slow(deflate_state *s, صحيح flush);
#د_که_پای
local بلاک_state deflate_rle(deflate_state *s, صحيح flush);
local بلاک_state deflate_huff(deflate_state *s, صحيح flush);

/* ===========================================================================
 * Local data
 */

#تعریف_کړه NIL 0
/* Tail of hash chains */

#که_نه_تعریف_شوی TOO_FAR
#  define TOO_FAR 4096
#د_که_پای
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
تعريفنوع بنية config_s {
   ush good_اوږدتیا; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_اوږدتیا; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#که_تعریف_شوی FASTEST
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
#د_که_پای

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#تعریف_کړه RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#تعریف_کړه UPDATE_HASH(s,h,c) (h = (((h) << s->hash_ښويدنه) ^ (c)) & s->hash_ماسک)


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
#که_تعریف_شوی FASTEST
#تعریف_کړه ورننباسه_STRING(s, str, سمون_سر) \
   (UPDATE_HASH(s, s->ins_h, s->کړکۍ[(str) + (MIN_سمون-1)]), \
    سمون_سر = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#وإلا
#تعریف_کړه ورننباسه_STRING(s, str, سمون_سر) \
   (UPDATE_HASH(s, s->ins_h, s->کړکۍ[(str) + (MIN_سمون-1)]), \
    سمون_سر = s->prev[(str) & s->w_ماسک] = s->سر[s->ins_h], \
    s->سر[s->ins_h] = (Pos)(str))
#د_که_پای

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#تعریف_کړه CLEAR_HASH(s) \
    افعل { \
        s->سر[s->hash_اندازه - 1] = NIL; \
        zmemzero(s->سر, (بینښه)(s->hash_اندازه - 1)*حجم(*s->سر)); \
        s->slid = 0; \
    } طالما (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#إذا defined(__has_feature)
#  إذا __has_feature(حافظه_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#د_که_پای
local فراغ slide_hash(deflate_state *s) {
    بینښه n, m;
    Posf *p;
    uInt wsize = s->w_اندازه;

    n = s->hash_اندازه;
    p = &s->سر[n];
    افعل {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } طالما (--n);
#که_نه_تعریف_شوی FASTEST
    n = wsize;
    p = &s->prev[n];
    افعل {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } طالما (--n);
#د_که_پای
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local بینښه read_buf(z_streamp strm, Bytef *buf, بینښه اندازه) {
    بینښه len = strm->avail_دننه;

    إذا (len > اندازه) len = اندازه;
    إذا (len == 0) أرجع 0;

    strm->avail_دننه  -= len;

    zmemcpy(buf, strm->بل_دننه, len);
    إذا (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#که_تعریف_شوی GZIP
    وإلا إذا (strm->state->wrap == 2) {
        strm->adler = دوراني_ازموینه_32_2(strm->adler, buf, len);
    }
#د_که_پای
    strm->بل_دننه  += len;
    strm->ټول_دننه += len;

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
local فراغ fill_کړکۍ(deflate_state *s) {
    بینښه n;
    بینښه نور;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_اندازه;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    افعل {
        نور = (بینښه)(s->کړکۍ_اندازه -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#که_تعریف_شوی _MSC_VER
#لارښوونه warning(ټېل)
#لارښوونه warning(disable: 4127)
#د_که_پای
        إذا (حجم(صحيح) <= 2) {
#که_تعریف_شوی _MSC_VER
#لارښوونه warning(راوباسه)
#د_که_پای
            إذا (نور == 0 && s->strstart == 0 && s->lookahead == 0) {
                نور = wsize;

            } وإلا إذا (نور == (بینښه)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                نور--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        إذا (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->کړکۍ, s->کړکۍ + wsize, (بینښه)wsize - نور);
            s->سمون_پیل -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->بلاک_پیل -= (اوږد) wsize;
            إذا (s->ورننباسه > s->strstart)
                s->ورننباسه = s->strstart;
            slide_hash(s);
            نور += wsize;
        }
        إذا (s->strm->avail_دننه == 0) اكسر;

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
        Assert(نور >= 2, "more < 2");

        n = read_buf(s->strm, s->کړکۍ + s->strstart + s->lookahead, نور);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        إذا (s->lookahead + s->ورننباسه >= MIN_سمون) {
            uInt str = s->strstart - s->ورننباسه;
            s->ins_h = s->کړکۍ[str];
            UPDATE_HASH(s, s->ins_h, s->کړکۍ[str + 1]);
#إذا MIN_سمون != 3
            Call UPDATE_HASH() MIN_سمون-3 نور times
#د_که_پای
            طالما (s->ورننباسه) {
                UPDATE_HASH(s, s->ins_h, s->کړکۍ[str + MIN_سمون-1]);
#که_نه_تعریف_شوی FASTEST
                s->prev[str & s->w_ماسک] = s->سر[s->ins_h];
#د_که_پای
                s->سر[s->ins_h] = (Pos)str;
                str++;
                s->ورننباسه--;
                إذا (s->lookahead + s->ورننباسه < MIN_سمون)
                    اكسر;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } طالما (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_دننه != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    إذا (s->لوړ_water < s->کړکۍ_اندازه) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        إذا (s->لوړ_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->کړکۍ_اندازه - curr;
            إذا (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->کړکۍ + curr, (بینښه)init);
            s->لوړ_water = curr + init;
        }
        وإلا إذا (s->لوړ_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->لوړ_water;
            إذا (init > s->کړکۍ_اندازه - s->لوړ_water)
                init = s->کړکۍ_اندازه - s->لوړ_water;
            zmemzero(s->کړکۍ + s->لوړ_water, (بینښه)init);
            s->لوړ_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->کړکۍ_اندازه - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit_(z_streamp strm, صحيح level, ثابت حرف *نسخه,
                         صحيح بهیر_اندازه) {
    أرجع deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, نسخه, بهیر_اندازه);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
صحيح ZEXPORT deflateInit2_(z_streamp strm, صحيح level, صحيح لاره,
                          صحيح کړکۍ_بټونه, صحيح memLevel, صحيح strategy,
                          ثابت حرف *نسخه, صحيح بهیر_اندازه) {
    deflate_state *s;
    صحيح wrap = 1;
    ساكن ثابت حرف my_نسخه[] = ZLIB_نسخه;

    إذا (نسخه == Z_NULL || نسخه[0] != my_نسخه[0] ||
        بهیر_اندازه != حجم(z_بهیر)) {
        أرجع Z_نسخه_ERROR;
    }
    إذا (strm == Z_NULL) أرجع Z_بهیر_ERROR;

    strm->پیغام = Z_NULL;
    إذا (strm->zalloc == (alloc_func)0) {
#که_تعریف_شوی Z_SOLO
        أرجع Z_بهیر_ERROR;
#وإلا
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#د_که_پای
    }
    إذا (strm->zfree == (free_func)0)
#که_تعریف_شوی Z_SOLO
        أرجع Z_بهیر_ERROR;
#وإلا
        strm->zfree = zcfree;
#د_که_پای

#که_تعریف_شوی FASTEST
    إذا (level != 0) level = 1;
#وإلا
    إذا (level == Z_DEFAULT_COMPRESSION) level = 6;
#د_که_پای

    إذا (کړکۍ_بټونه < 0) { /* suppress zlib wrapper */
        wrap = 0;
        إذا (کړکۍ_بټونه < -15)
            أرجع Z_بهیر_ERROR;
        کړکۍ_بټونه = -کړکۍ_بټونه;
    }
#که_تعریف_شوی GZIP
    وإلا إذا (کړکۍ_بټونه > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        کړکۍ_بټونه -= 16;
    }
#د_که_پای
    إذا (memLevel < 1 || memLevel > MAX_MEM_LEVEL || لاره != Z_DEFLATED ||
        کړکۍ_بټونه < 8 || کړکۍ_بټونه > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (کړکۍ_بټونه == 8 && wrap != 1)) {
        أرجع Z_بهیر_ERROR;
    }
    إذا (کړکۍ_بټونه == 8) کړکۍ_بټونه = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, حجم(deflate_state));
    إذا (s == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(s, حجم(deflate_state));
    strm->state = (بنية دننی_state FAR *)s;
    s->strm = strm;
    s->حالت = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_بټونه = (uInt)کړکۍ_بټونه;
    s->w_اندازه = 1 << s->w_بټونه;
    s->w_ماسک = s->w_اندازه - 1;

    s->hash_بټونه = (uInt)memLevel + 7;
    s->hash_اندازه = 1 << s->hash_بټونه;
    s->hash_ماسک = s->hash_اندازه - 1;
    s->hash_ښويدنه =  ((s->hash_بټونه + MIN_سمون-1) / MIN_سمون);

    s->کړکۍ = (Bytef *) ZALLOC(strm, s->w_اندازه, 2*حجم(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_اندازه, حجم(Pos));
    s->سر   = (Posf *)  ZALLOC(strm, s->hash_اندازه, حجم(Pos));

    s->لوړ_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_اندازه = (ulg)s->lit_bufsize * 4;

    إذا (s->کړکۍ == Z_NULL || s->prev == Z_NULL || s->سر == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->حالت = پای_STATE;
        strm->پیغام = ERR_پیغام(Z_MEM_ERROR);
        deflate_پای_ټکی (strm);
        أرجع Z_MEM_ERROR;
    }
#که_تعریف_شوی LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_پای_ټکی = s->lit_bufsize - 1;
#وإلا
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_پای_ټکی = (s->lit_bufsize - 1) * 3;
#د_که_پای
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->لاره = (Byte)لاره;

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
#که_تعریف_شوی GZIP
                                           s->حالت != GZIP_STATE &&
#د_که_پای
                                           s->حالت != EXTRA_STATE &&
                                           s->حالت != NAME_STATE &&
                                           s->حالت != COMMENT_STATE &&
                                           s->حالت != HCRC_STATE &&
                                           s->حالت != BUSY_STATE &&
                                           s->حالت != پای_STATE))
        أرجع 1;
    أرجع 0;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_سېټ_Dictionary(z_streamp strm, ثابت Bytef *dictionary,
                                 uInt  dict_اوږدتیا) {
    deflate_state *s;
    uInt str, n;
    صحيح wrap;
    بینښه avail;
    z_const بینښه حرف *بل;

    إذا (deflateStateCheck(strm) || dictionary == Z_NULL)
        أرجع Z_بهیر_ERROR;
    s = strm->state;
    wrap = s->wrap;
    إذا (wrap == 2 || (wrap == 1 && s->حالت != INIT_STATE) || s->lookahead)
        أرجع Z_بهیر_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    إذا (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_اوږدتیا);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    إذا (dict_اوږدتیا >= s->w_اندازه) {
        إذا (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->بلاک_پیل = 0L;
            s->ورننباسه = 0;
        }
        dictionary += dict_اوږدتیا - s->w_اندازه;  /* use the tail */
        dict_اوږدتیا = s->w_اندازه;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_دننه;
    بل = strm->بل_دننه;
    strm->avail_دننه = dict_اوږدتیا;
    strm->بل_دننه = (z_const Bytef *)dictionary;
    fill_کړکۍ(s);
    طالما (s->lookahead >= MIN_سمون) {
        str = s->strstart;
        n = s->lookahead - (MIN_سمون-1);
        افعل {
            UPDATE_HASH(s, s->ins_h, s->کړکۍ[str + MIN_سمون-1]);
#که_نه_تعریف_شوی FASTEST
            s->prev[str & s->w_ماسک] = s->سر[s->ins_h];
#د_که_پای
            s->سر[s->ins_h] = (Pos)str;
            str++;
        } طالما (--n);
        s->strstart = str;
        s->lookahead = MIN_سمون-1;
        fill_کړکۍ(s);
    }
    s->strstart += s->lookahead;
    s->بلاک_پیل = (اوږد)s->strstart;
    s->ورننباسه = s->lookahead;
    s->lookahead = 0;
    s->سمون_اوږدتیا = s->prev_اوږدتیا = MIN_سمون-1;
    s->سمون_شته = 0;
    strm->بل_دننه = بل;
    strm->avail_دننه = avail;
    s->wrap = wrap;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_اوږدتیا) {
    deflate_state *s;
    uInt len;

    إذا (deflateStateCheck(strm))
        أرجع Z_بهیر_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    إذا (len > s->w_اندازه)
        len = s->w_اندازه;
    إذا (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->کړکۍ + s->strstart + s->lookahead - len, len);
    إذا (dict_اوږدتیا != Z_NULL)
        *dict_اوږدتیا = len;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    إذا (deflateStateCheck(strm)) {
        أرجع Z_بهیر_ERROR;
    }

    strm->ټول_دننه = strm->ټول_بهر = 0;
    strm->پیغام = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->معلومات_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_بهر = s->pending_buf;

    إذا (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->حالت =
#که_تعریف_شوی GZIP
        s->wrap == 2 ? GZIP_STATE :
#د_که_پای
        INIT_STATE;
    strm->adler =
#که_تعریف_شوی GZIP
        s->wrap == 2 ? دوراني_ازموینه_32_2(0L, Z_NULL, 0) :
#د_که_پای
        adler32(0L, Z_NULL, 0);
    s->وروستی_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    أرجع Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local فراغ lm_init(deflate_state *s) {
    s->کړکۍ_اندازه = (ulg)2L*s->w_اندازه;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_سمون   = configuration_جدول[s->level].max_lazy;
    s->good_سمون       = configuration_جدول[s->level].good_اوږدتیا;
    s->nice_سمون       = configuration_جدول[s->level].nice_اوږدتیا;
    s->max_chain_اوږدتیا = configuration_جدول[s->level].max_chain;

    s->strstart = 0;
    s->بلاک_پیل = 0L;
    s->lookahead = 0;
    s->ورننباسه = 0;
    s->سمون_اوږدتیا = s->prev_اوږدتیا = MIN_سمون-1;
    s->سمون_شته = 0;
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
صحيح ZEXPORT deflate_سېټ_سرلیک(z_streamp strm, gz_headerp سر) {
    إذا (deflateStateCheck(strm) || strm->state->wrap != 2)
        أرجع Z_بهیر_ERROR;
    strm->state->gzhead = سر;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflatePending(z_streamp strm, بینښه *pending, صحيح *بټونه) {
    إذا (deflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    إذا (بټونه != Z_NULL)
        *بټونه = strm->state->bi_valid;
    إذا (pending != Z_NULL) {
        *pending = (بینښه)strm->state->pending;
        إذا (*pending != strm->state->pending) {
            *pending = (بینښه)-1;
            أرجع Z_BUF_ERROR;
        }
    }
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_کارول_شوی(z_streamp strm, صحيح *بټونه) {
    إذا (deflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    إذا (بټونه != Z_NULL)
        *بټونه = strm->state->bi_کارول_شوی;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_اولي(z_streamp strm, صحيح بټونه, صحيح ارزښت) {
    deflate_state *s;
    صحيح put;

    إذا (deflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    s = strm->state;
#که_تعریف_شوی LIT_MEM
    إذا (بټونه < 0 || بټونه > 16 ||
        (uchf *)s->d_buf < s->pending_بهر + ((Buf_اندازه + 7) >> 3))
        أرجع Z_BUF_ERROR;
#وإلا
    إذا (بټونه < 0 || بټونه > 16 ||
        s->sym_buf < s->pending_بهر + ((Buf_اندازه + 7) >> 3))
        أرجع Z_BUF_ERROR;
#د_که_پای
    افعل {
        put = Buf_اندازه - s->bi_valid;
        إذا (put > بټونه)
            put = بټونه;
        s->bi_buf |= (ush)((ارزښت & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_بټونه(s);
        ارزښت >>= put;
        بټونه -= put;
    } طالما (بټونه);
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateParams(z_streamp strm, صحيح level, صحيح strategy) {
    deflate_state *s;
    compress_func func;

    إذا (deflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    s = strm->state;

#که_تعریف_شوی FASTEST
    إذا (level != 0) level = 1;
#وإلا
    إذا (level == Z_DEFAULT_COMPRESSION) level = 6;
#د_که_پای
    إذا (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        أرجع Z_بهیر_ERROR;
    }
    func = configuration_جدول[s->level].func;

    إذا ((strategy != s->strategy || func != configuration_جدول[level].func) &&
        s->وروستی_flush != -2) {
        /* Flush the last buffer: */
        صحيح err = deflate(strm, Z_بلاک);
        إذا (err == Z_بهیر_ERROR)
            أرجع err;
        إذا (strm->avail_دننه || (s->strstart - s->بلاک_پیل) + s->lookahead)
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
        s->max_lazy_سمون   = configuration_جدول[level].max_lazy;
        s->good_سمون       = configuration_جدول[level].good_اوږدتیا;
        s->nice_سمون       = configuration_جدول[level].nice_اوږدتیا;
        s->max_chain_اوږدتیا = configuration_جدول[level].max_chain;
    }
    s->strategy = strategy;
    أرجع Z_OK;
}

/* ========================================================================= */
صحيح ZEXPORT deflateTune(z_streamp strm, صحيح good_اوږدتیا, صحيح max_lazy,
                        صحيح nice_اوږدتیا, صحيح max_chain) {
    deflate_state *s;

    إذا (deflateStateCheck(strm)) أرجع Z_بهیر_ERROR;
    s = strm->state;
    s->good_سمون = (uInt)good_اوږدتیا;
    s->max_lazy_سمون = (uInt)max_lazy;
    s->nice_سمون = nice_اوږدتیا;
    s->max_chain_اوږدتیا = (uInt)max_chain;
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
z_اندازه_t ZEXPORT deflateBound_z(z_streamp strm, z_اندازه_t سرچینه_Len) {
    deflate_state *s;
    z_اندازه_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = سرچینه_Len + (سرچینه_Len >> 3) + (سرچینه_Len >> 8) +
               (سرچینه_Len >> 9) + 4;
    إذا (fixedlen < سرچینه_Len)
        fixedlen = (z_اندازه_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = سرچینه_Len + (سرچینه_Len >> 5) + (سرچینه_Len >> 7) +
               (سرچینه_Len >> 11) + 7;
    إذا (storelen < سرچینه_Len)
        storelen = (z_اندازه_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    إذا (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        أرجع bound + 18 < bound ? (z_اندازه_t)-1 : bound + 18;
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
#که_تعریف_شوی GZIP
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
#د_که_پای
    افتراضي:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    إذا (s->w_بټونه != 15 || s->hash_بټونه != 8 + 7) {
        bound = s->w_بټونه <= s->hash_بټونه && s->level ? fixedlen :
                                                        storelen;
        أرجع bound + wraplen < bound ? (z_اندازه_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = سرچینه_Len + (سرچینه_Len >> 12) + (سرچینه_Len >> 14) +
            (سرچینه_Len >> 25) + 13 - 6 + wraplen;
    أرجع bound < سرچینه_Len ? (z_اندازه_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong سرچینه_Len) {
    z_اندازه_t bound = deflateBound_z(strm, سرچینه_Len);
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
    بینښه len;
    deflate_state *s = strm->state;

    tr_flush_بټونه(s);
    len = s->pending > strm->avail_بهر ? strm->avail_بهر :
                                         (بینښه)s->pending;
    إذا (len == 0) أرجع;

    zmemcpy(strm->بل_بهر, s->pending_بهر, len);
    strm->بل_بهر  += len;
    s->pending_بهر  += len;
    strm->ټول_بهر += len;
    strm->avail_بهر -= len;
    s->pending      -= len;
    إذا (s->pending == 0) {
        s->pending_بهر = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#تعریف_کړه HCRC_UPDATE(beg) \
    افعل { \
        إذا (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = دوراني_ازموینه_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } طالما (0)

/* ========================================================================= */
صحيح ZEXPORT deflate(z_streamp strm, صحيح flush) {
    صحيح old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    إذا (deflateStateCheck(strm) || flush > Z_بلاک || flush < 0) {
        أرجع Z_بهیر_ERROR;
    }
    s = strm->state;

    إذا (strm->بل_بهر == Z_NULL ||
        (strm->avail_دننه != 0 && strm->بل_دننه == Z_NULL) ||
        (s->حالت == پای_STATE && flush != Z_پای)) {
        ERR_RETURN(strm, Z_بهیر_ERROR);
    }
    إذا (strm->avail_بهر == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->وروستی_flush;
    s->وروستی_flush = flush;

    /* Flush as much pending output as possible */
    إذا (s->pending != 0) {
        flush_pending(strm);
        إذا (strm->avail_بهر == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->وروستی_flush = -1;
            أرجع Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } وإلا إذا (strm->avail_دننه == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_پای) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    إذا (s->حالت == پای_STATE && strm->avail_دننه != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    إذا (s->حالت == INIT_STATE && s->wrap == 0)
        s->حالت = BUSY_STATE;
    إذا (s->حالت == INIT_STATE) {
        /* zlib header */
        uInt سرلیک = (Z_DEFLATED + ((s->w_بټونه - 8) << 4)) << 8;
        uInt level_flags;

        إذا (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        وإلا إذا (s->level < 6)
            level_flags = 1;
        وإلا إذا (s->level == 6)
            level_flags = 2;
        وإلا
            level_flags = 3;
        سرلیک |= (level_flags << 6);
        إذا (s->strstart != 0) سرلیک |= PRESET_DICT;
        سرلیک += 31 - (سرلیک % 31);

        putShortMSB(s, سرلیک);

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
            s->وروستی_flush = -1;
            أرجع Z_OK;
        }
    }
#که_تعریف_شوی GZIP
    إذا (s->حالت == GZIP_STATE) {
        /* gzip header */
        strm->adler = دوراني_ازموینه_32_2(0L, Z_NULL, 0);
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
            put_byte(s, OS_کوډ);
            s->حالت = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            إذا (s->pending != 0) {
                s->وروستی_flush = -1;
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
                strm->adler = دوراني_ازموینه_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->حالت = EXTRA_STATE;
        }
    }
    إذا (s->حالت == EXTRA_STATE) {
        إذا (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg کیڼ = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            طالما (s->pending + کیڼ > s->pending_buf_اندازه) {
                ulg copy = s->pending_buf_اندازه - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_اندازه;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                إذا (s->pending != 0) {
                    s->وروستی_flush = -1;
                    أرجع Z_OK;
                }
                beg = 0;
                کیڼ -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, کیڼ);
            s->pending += کیڼ;
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
                إذا (s->pending == s->pending_buf_اندازه) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    إذا (s->pending != 0) {
                        s->وروستی_flush = -1;
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
                إذا (s->pending == s->pending_buf_اندازه) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    إذا (s->pending != 0) {
                        s->وروستی_flush = -1;
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
            إذا (s->pending + 2 > s->pending_buf_اندازه) {
                flush_pending(strm);
                إذا (s->pending != 0) {
                    s->وروستی_flush = -1;
                    أرجع Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = دوراني_ازموینه_32_2(0L, Z_NULL, 0);
        }
        s->حالت = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        إذا (s->pending != 0) {
            s->وروستی_flush = -1;
            أرجع Z_OK;
        }
    }
#د_که_پای

    /* Start a new block or continue the current one.
     */
    إذا (strm->avail_دننه != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->حالت != پای_STATE)) {
        بلاک_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_جدول[s->level].func))(s, flush);

        إذا (bstate == پای_started || bstate == پای_بشپړ) {
            s->حالت = پای_STATE;
        }
        إذا (bstate == need_نور || bstate == پای_started) {
            إذا (strm->avail_بهر == 0) {
                s->وروستی_flush = -1; /* avoid BUF_ERROR next call, see above */
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
        إذا (bstate == بلاک_بشپړ) {
            إذا (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } وإلا إذا (flush != Z_بلاک) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_بلاک(s, (حرف*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                إذا (flush == Z_ډک_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    إذا (s->lookahead == 0) {
                        s->strstart = 0;
                        s->بلاک_پیل = 0L;
                        s->ورننباسه = 0;
                    }
                }
            }
            flush_pending(strm);
            إذا (strm->avail_بهر == 0) {
              s->وروستی_flush = -1; /* avoid BUF_ERROR at next call, see above */
              أرجع Z_OK;
            }
        }
    }

    إذا (flush != Z_پای) أرجع Z_OK;
    إذا (s->wrap <= 0) أرجع Z_بهیر_پای_ټکی;

    /* Write the trailer */
#که_تعریف_شوی GZIP
    إذا (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->ټول_دننه & 0xff));
        put_byte(s, (Byte)((strm->ټول_دننه >> 8) & 0xff));
        put_byte(s, (Byte)((strm->ټول_دننه >> 16) & 0xff));
        put_byte(s, (Byte)((strm->ټول_دننه >> 24) & 0xff));
    }
    وإلا
#د_که_پای
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    إذا (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    أرجع s->pending != 0 ? Z_OK : Z_بهیر_پای_ټکی;
}

/* ========================================================================= */
صحيح ZEXPORT deflate_پای_ټکی(z_streamp strm) {
    صحيح حالت;

    إذا (deflateStateCheck(strm)) أرجع Z_بهیر_ERROR;

    حالت = strm->state->حالت;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->سر);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->کړکۍ);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    أرجع حالت == BUSY_STATE ? Z_معلومات_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
صحيح ZEXPORT deflateCopy(z_streamp dest, z_streamp سرچینه) {
#که_تعریف_شوی MAXSEG_64K
    (فراغ)dest;
    (فراغ)سرچینه;
    أرجع Z_بهیر_ERROR;
#وإلا
    deflate_state *ds;
    deflate_state *ss;


    إذا (deflateStateCheck(سرچینه) || dest == Z_NULL) {
        أرجع Z_بهیر_ERROR;
    }

    ss = سرچینه->state;

    zmemcpy(dest, سرچینه, حجم(z_بهیر));

    ds = (deflate_state *) ZALLOC(dest, 1, حجم(deflate_state));
    إذا (ds == Z_NULL) أرجع Z_MEM_ERROR;
    zmemzero(ds, حجم(deflate_state));
    dest->state = (بنية دننی_state FAR *) ds;
    zmemcpy(ds, ss, حجم(deflate_state));
    ds->strm = dest;

    ds->کړکۍ = (Bytef *) ZALLOC(dest, ds->w_اندازه, 2*حجم(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_اندازه, حجم(Pos));
    ds->سر   = (Posf *)  ZALLOC(dest, ds->hash_اندازه, حجم(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    إذا (ds->کړکۍ == Z_NULL || ds->prev == Z_NULL || ds->سر == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_پای_ټکی (dest);
        أرجع Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->کړکۍ, ss->کړکۍ, ss->لوړ_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->ورننباسه > ds->w_اندازه ? ds->w_اندازه :
                ss->strstart - ss->ورننباسه) * حجم(Pos));
    zmemcpy(ds->سر, ss->سر, ds->hash_اندازه * حجم(Pos));

    ds->pending_بهر = ds->pending_buf + (ss->pending_بهر - ss->pending_buf);
    zmemcpy(ds->pending_بهر, ss->pending_بهر, ss->pending);
#که_تعریف_شوی LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_بل * حجم(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_بل);
#وإلا
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_بل);
#د_که_پای

    ds->l_desc.dyn_ونه = ds->dyn_ltree;
    ds->d_desc.dyn_ونه = ds->dyn_dtree;
    ds->bl_desc.dyn_ونه = ds->bl_ونه;

    أرجع Z_OK;
#د_که_پای /* MAXSEG_64K */
}

#که_نه_تعریف_شوی FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_سمون(deflate_state *s, IPos cur_سمون) {
    بینښه chain_اوږدتیا = s->max_chain_اوږدتیا;/* max hash chain length */
    Bytef *لټون = s->کړکۍ + s->strstart;      /* current string */
    Bytef *سمون;                               /* matched string */
    صحيح len;                                    /* length of current match */
    صحيح غوره_len = (صحيح)s->prev_اوږدتیا;         /* best match length so far */
    صحيح nice_سمون = s->nice_سمون;             /* stop if match long enough */
    IPos حد = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_ماسک;

#که_تعریف_شوی UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->کړکۍ + s->strstart + MAX_سمون - 1;
    ush لټون_پیل = *(ushf*)لټون;
    ush لټون_پای_ټکی   = *(ushf*)(لټون + غوره_len - 1);
#وإلا
    Bytef *strend = s->کړکۍ + s->strstart + MAX_سمون;
    Byte لټون_end1  = لټون[غوره_len - 1];
    Byte لټون_پای_ټکی   = لټون[غوره_len];
#د_که_پای

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بټونه >= 8 && MAX_سمون == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    إذا (s->prev_اوږدتیا >= s->good_سمون) {
        chain_اوږدتیا >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    إذا ((uInt)nice_سمون > s->lookahead) nice_سمون = (صحيح)s->lookahead;

    Assert((ulg)s->strstart <= s->کړکۍ_اندازه - MIN_LOOKAHEAD,
           "need lookahead");

    افعل {
        Assert(cur_سمون < s->strstart, "no future");
        سمون = s->کړکۍ + cur_سمون;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#إذا (defined(UNALIGNED_OK) && MAX_سمون == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        إذا (*(ushf*)(سمون + غوره_len - 1) != لټون_پای_ټکی ||
            *(ushf*)سمون != لټون_پیل) استمر;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(لټون[2] == سمون[2], "scan[2]?");
        لټون++, سمون++;
        افعل {
        } طالما (*(ushf*)(لټون += 2) == *(ushf*)(سمون += 2) &&
                 *(ushf*)(لټون += 2) == *(ushf*)(سمون += 2) &&
                 *(ushf*)(لټون += 2) == *(ushf*)(سمون += 2) &&
                 *(ushf*)(لټون += 2) == *(ushf*)(سمون += 2) &&
                 لټون < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(لټون <= s->کړکۍ + (بینښه)(s->کړکۍ_اندازه - 1),
               "wild scan");
        إذا (*لټون == *سمون) لټون++;

        len = (MAX_سمون - 1) - (صحيح)(strend - لټون);
        لټون = strend - (MAX_سمون-1);

#وإلا /* UNALIGNED_OK */

        إذا (سمون[غوره_len]     != لټون_پای_ټکی  ||
            سمون[غوره_len - 1] != لټون_end1 ||
            *سمون              != *لټون     ||
            *++سمون            != لټون[1])      استمر;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        لټون += 2, سمون++;
        Assert(*لټون == *سمون, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        افعل {
        } طالما (*++لټون == *++سمون && *++لټون == *++سمون &&
                 *++لټون == *++سمون && *++لټون == *++سمون &&
                 *++لټون == *++سمون && *++لټون == *++سمون &&
                 *++لټون == *++سمون && *++لټون == *++سمون &&
                 لټون < strend);

        Assert(لټون <= s->کړکۍ + (بینښه)(s->کړکۍ_اندازه - 1),
               "wild scan");

        len = MAX_سمون - (صحيح)(strend - لټون);
        لټون = strend - MAX_سمون;

#د_که_پای /* UNALIGNED_OK */

        إذا (len > غوره_len) {
            s->سمون_پیل = cur_سمون;
            غوره_len = len;
            إذا (len >= nice_سمون) اكسر;
#که_تعریف_شوی UNALIGNED_OK
            لټون_پای_ټکی = *(ushf*)(لټون + غوره_len - 1);
#وإلا
            لټون_end1  = لټون[غوره_len - 1];
            لټون_پای_ټکی   = لټون[غوره_len];
#د_که_پای
        }
    } طالما ((cur_سمون = prev[cur_سمون & wmask]) > حد
             && --chain_اوږدتیا != 0);

    إذا ((uInt)غوره_len <= s->lookahead) أرجع (uInt)غوره_len;
    أرجع s->lookahead;
}

#وإلا /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_سمون(deflate_state *s, IPos cur_سمون) {
    Bytef *لټون = s->کړکۍ + s->strstart;      /* current string */
    Bytef *سمون;                               /* matched string */
    صحيح len;                                    /* length of current match */
    Bytef *strend = s->کړکۍ + s->strstart + MAX_سمون;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_بټونه >= 8 && MAX_سمون == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->کړکۍ_اندازه - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_سمون < s->strstart, "no future");

    سمون = s->کړکۍ + cur_سمون;

    /* Return failure if the match length is less than 2:
     */
    إذا (سمون[0] != لټون[0] || سمون[1] != لټون[1]) أرجع MIN_سمون-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    لټون += 2, سمون += 2;
    Assert(*لټون == *سمون, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    افعل {
    } طالما (*++لټون == *++سمون && *++لټون == *++سمون &&
             *++لټون == *++سمون && *++لټون == *++سمون &&
             *++لټون == *++سمون && *++لټون == *++سمون &&
             *++لټون == *++سمون && *++لټون == *++سمون &&
             لټون < strend);

    Assert(لټون <= s->کړکۍ + (بینښه)(s->کړکۍ_اندازه - 1), "wild scan");

    len = MAX_سمون - (صحيح)(strend - لټون);

    إذا (len < MIN_سمون) أرجع MIN_سمون - 1;

    s->سمون_پیل = cur_سمون;
    أرجع (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#د_که_پای /* FASTEST */

#که_تعریف_شوی ZLIB_DEBUG

#تعریف_کړه EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local فراغ check_سمون(deflate_state *s, IPos پیل, IPos سمون, صحيح اوږدتیا) {
    /* check that the match is indeed a match */
    Bytef *شا = s->کړکۍ + (صحيح)سمون, *دلته = s->کړکۍ + پیل;
    IPos len = (IPos)اوږدتیا;
    إذا (سمون == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        شا++;
        دلته++;
        len--;
    }
    إذا (zmemcmp(شا, دلته, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                پیل, (صحيح)سمون, اوږدتیا);
        افعل {
            fprintf(stderr, "(%02x %02x)", *شا++, *دلته++);
        } طالما (--len != 0);
        z_error("invalid match");
    }
    إذا (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", پیل - سمون, اوږدتیا);
        افعل { putc(s->کړکۍ[پیل++], stderr); } طالما (--اوږدتیا != 0);
    }
}
#وإلا
#  define check_سمون(s, پیل, سمون, اوږدتیا)
#د_که_پای /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#تعریف_کړه FLUSH_بلاک_ONLY(s, وروستی) { \
   tr_flush_بلاک(s, (s->بلاک_پیل >= 0L ? \
                   (charf *)&s->کړکۍ[(بینښه)s->بلاک_پیل] : \
                   (charf *)Z_NULL), \
                (ulg)((اوږد)s->strstart - s->بلاک_پیل), \
                (وروستی)); \
   s->بلاک_پیل = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#تعریف_کړه FLUSH_بلاک(s, وروستی) { \
   FLUSH_بلاک_ONLY(s, وروستی); \
   إذا (s->strm->avail_بهر == 0) أرجع (وروستی) ? پای_started : need_نور; \
}

/* Maximum stored block length in deflate format (not including header). */
#تعریف_کړه MAX_STORED 65535

/* Minimum of a and b. */
#تعریف_کړه MIN(a, b) ((a) > (b) ? (b) : (a))

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
    بینښه min_بلاک = (بینښه)(MIN(s->pending_buf_اندازه - 5, s->w_اندازه));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    صحيح وروستی = 0;
    بینښه len, کیڼ, have;
    بینښه کارول_شوی = s->strm->avail_دننه;
    افعل {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((بینښه)s->bi_valid + 42) >> 3;   /* bytes in header */
        إذا (s->strm->avail_بهر < have)          /* need room for header */
            اكسر;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_بهر - have;
        کیڼ = (بینښه)(s->strstart - s->بلاک_پیل);    /* window bytes */
        إذا (len > (ulg)کیڼ + s->strm->avail_دننه)
            len = کیڼ + s->strm->avail_دننه;     /* limit len to the input */
        إذا (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        إذا (len < min_بلاک && ((len == 0 && flush != Z_پای) ||
                                flush == Z_NO_FLUSH ||
                                len != کیڼ + s->strm->avail_دننه))
            اكسر;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        وروستی = flush == Z_پای && len == کیڼ + s->strm->avail_دننه ? 1 : 0;
        tr_stored_بلاک(s, (حرف *)0, 0L, وروستی);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#که_تعریف_شوی ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->بټونه_لېږل_شوی += len << 3;
#د_که_پای

        /* Copy uncompressed bytes from the window to next_out. */
        إذا (کیڼ) {
            إذا (کیڼ > len)
                کیڼ = len;
            zmemcpy(s->strm->بل_بهر, s->کړکۍ + s->بلاک_پیل, کیڼ);
            s->strm->بل_بهر += کیڼ;
            s->strm->avail_بهر -= کیڼ;
            s->strm->ټول_بهر += کیڼ;
            s->بلاک_پیل += کیڼ;
            len -= کیڼ;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        إذا (len) {
            read_buf(s->strm, s->strm->بل_بهر, len);
            s->strm->بل_بهر += len;
            s->strm->avail_بهر -= len;
            s->strm->ټول_بهر += len;
        }
    } طالما (وروستی == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    کارول_شوی -= s->strm->avail_دننه;      /* number of input bytes directly copied */
    إذا (کارول_شوی) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        إذا (کارول_شوی >= s->w_اندازه) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->کړکۍ, s->strm->بل_دننه - s->w_اندازه, s->w_اندازه);
            s->strstart = s->w_اندازه;
            s->ورننباسه = s->strstart;
        }
        وإلا {
            إذا (s->کړکۍ_اندازه - s->strstart <= کارول_شوی) {
                /* Slide the window down. */
                s->strstart -= s->w_اندازه;
                zmemcpy(s->کړکۍ, s->کړکۍ + s->w_اندازه, s->strstart);
                إذا (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                إذا (s->ورننباسه > s->strstart)
                    s->ورننباسه = s->strstart;
            }
            zmemcpy(s->کړکۍ + s->strstart, s->strm->بل_دننه - کارول_شوی, کارول_شوی);
            s->strstart += کارول_شوی;
            s->ورننباسه += MIN(کارول_شوی, s->w_اندازه - s->ورننباسه);
        }
        s->بلاک_پیل = s->strstart;
    }
    إذا (s->لوړ_water < s->strstart)
        s->لوړ_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    إذا (وروستی) {
        s->bi_کارول_شوی = 8;
        أرجع پای_بشپړ;
    }

    /* If flushing and all input has been consumed, then done. */
    إذا (flush != Z_NO_FLUSH && flush != Z_پای &&
        s->strm->avail_دننه == 0 && (اوږد)s->strstart == s->بلاک_پیل)
        أرجع بلاک_بشپړ;

    /* Fill the window with any remaining input. */
    have = (بینښه)(s->کړکۍ_اندازه - s->strstart);
    إذا (s->strm->avail_دننه > have && s->بلاک_پیل >= (اوږد)s->w_اندازه) {
        /* Slide the window down. */
        s->بلاک_پیل -= s->w_اندازه;
        s->strstart -= s->w_اندازه;
        zmemcpy(s->کړکۍ, s->کړکۍ + s->w_اندازه, s->strstart);
        إذا (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_اندازه;          /* more space now */
        إذا (s->ورننباسه > s->strstart)
            s->ورننباسه = s->strstart;
    }
    إذا (have > s->strm->avail_دننه)
        have = s->strm->avail_دننه;
    إذا (have) {
        read_buf(s->strm, s->کړکۍ + s->strstart, have);
        s->strstart += have;
        s->ورننباسه += MIN(have, s->w_اندازه - s->ورننباسه);
    }
    إذا (s->لوړ_water < s->strstart)
        s->لوړ_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((بینښه)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (بینښه)MIN(s->pending_buf_اندازه - have, MAX_STORED);
    min_بلاک = MIN(have, s->w_اندازه);
    کیڼ = (بینښه)(s->strstart - s->بلاک_پیل);
    إذا (کیڼ >= min_بلاک ||
        ((کیڼ || flush == Z_پای) && flush != Z_NO_FLUSH &&
         s->strm->avail_دننه == 0 && کیڼ <= have)) {
        len = MIN(کیڼ, have);
        وروستی = flush == Z_پای && s->strm->avail_دننه == 0 &&
               len == کیڼ ? 1 : 0;
        tr_stored_بلاک(s, (charf *)s->کړکۍ + s->بلاک_پیل, len, وروستی);
        s->بلاک_پیل += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    إذا (وروستی)
        s->bi_کارول_شوی = 8;
    أرجع وروستی ? پای_started : need_نور;
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
            fill_کړکۍ(s);
            إذا (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_نور;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        إذا (s->lookahead >= MIN_سمون) {
            ورننباسه_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        إذا (hash_سر != NIL && s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->سمون_اوږدتیا = longest_سمون (s, hash_سر);
            /* longest_match() sets match_start */
        }
        إذا (s->سمون_اوږدتیا >= MIN_سمون) {
            check_سمون(s, s->strstart, s->سمون_پیل, (صحيح)s->سمون_اوږدتیا);

            _tr_tally_dist(s, s->strstart - s->سمون_پیل,
                           s->سمون_اوږدتیا - MIN_سمون, bflush);

            s->lookahead -= s->سمون_اوږدتیا;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#که_نه_تعریف_شوی FASTEST
            إذا (s->سمون_اوږدتیا <= s->max_ورننباسه_اوږدتیا &&
                s->lookahead >= MIN_سمون) {
                s->سمون_اوږدتیا--; /* string at strstart already in table */
                افعل {
                    s->strstart++;
                    ورننباسه_STRING(s, s->strstart, hash_سر);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } طالما (--s->سمون_اوږدتیا != 0);
                s->strstart++;
            } وإلا
#د_که_پای
            {
                s->strstart += s->سمون_اوږدتیا;
                s->سمون_اوږدتیا = 0;
                s->ins_h = s->کړکۍ[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->کړکۍ[s->strstart + 1]);
#إذا MIN_سمون != 3
                Call UPDATE_HASH() MIN_سمون-3 نور times
#د_که_پای
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->کړکۍ[s->strstart]));
            _tr_tally_lit(s, s->کړکۍ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        إذا (bflush) FLUSH_بلاک(s, 0);
    }
    s->ورننباسه = s->strstart < MIN_سمون-1 ? s->strstart : MIN_سمون-1;
    إذا (flush == Z_پای) {
        FLUSH_بلاک(s, 1);
        أرجع پای_بشپړ;
    }
    إذا (s->sym_بل)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_بشپړ;
}

#که_نه_تعریف_شوی FASTEST
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
            fill_کړکۍ(s);
            إذا (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                أرجع need_نور;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_سر = NIL;
        إذا (s->lookahead >= MIN_سمون) {
            ورننباسه_STRING(s, s->strstart, hash_سر);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_اوږدتیا = s->سمون_اوږدتیا, s->prev_سمون = s->سمون_پیل;
        s->سمون_اوږدتیا = MIN_سمون-1;

        إذا (hash_سر != NIL && s->prev_اوږدتیا < s->max_lazy_سمون &&
            s->strstart - hash_سر <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->سمون_اوږدتیا = longest_سمون (s, hash_سر);
            /* longest_match() sets match_start */

            إذا (s->سمون_اوږدتیا <= 5 && (s->strategy == Z_FILTERED
#إذا TOO_FAR <= 32767
                || (s->سمون_اوږدتیا == MIN_سمون &&
                    s->strstart - s->سمون_پیل > TOO_FAR)
#د_که_پای
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->سمون_اوږدتیا = MIN_سمون-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        إذا (s->prev_اوږدتیا >= MIN_سمون && s->سمون_اوږدتیا <= s->prev_اوږدتیا) {
            uInt max_ورننباسه = s->strstart + s->lookahead - MIN_سمون;
            /* Do not insert strings in hash table beyond this. */

            check_سمون(s, s->strstart - 1, s->prev_سمون, (صحيح)s->prev_اوږدتیا);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_سمون,
                           s->prev_اوږدتیا - MIN_سمون, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_اوږدتیا - 1;
            s->prev_اوږدتیا -= 2;
            افعل {
                إذا (++s->strstart <= max_ورننباسه) {
                    ورننباسه_STRING(s, s->strstart, hash_سر);
                }
            } طالما (--s->prev_اوږدتیا != 0);
            s->سمون_شته = 0;
            s->سمون_اوږدتیا = MIN_سمون-1;
            s->strstart++;

            إذا (bflush) FLUSH_بلاک(s, 0);

        } وإلا إذا (s->سمون_شته) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->کړکۍ[s->strstart - 1]));
            _tr_tally_lit(s, s->کړکۍ[s->strstart - 1], bflush);
            إذا (bflush) {
                FLUSH_بلاک_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            إذا (s->strm->avail_بهر == 0) أرجع need_نور;
        } وإلا {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->سمون_شته = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    إذا (s->سمون_شته) {
        Tracevv((stderr,"%c", s->کړکۍ[s->strstart - 1]));
        _tr_tally_lit(s, s->کړکۍ[s->strstart - 1], bflush);
        s->سمون_شته = 0;
    }
    s->ورننباسه = s->strstart < MIN_سمون-1 ? s->strstart : MIN_سمون-1;
    إذا (flush == Z_پای) {
        FLUSH_بلاک(s, 1);
        أرجع پای_بشپړ;
    }
    إذا (s->sym_بل)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_بشپړ;
}
#د_که_پای /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local بلاک_state deflate_rle(deflate_state *s, صحيح flush) {
    صحيح bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *لټون, *strend;   /* scan goes up to strend for length of run */

    لكل (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        إذا (s->lookahead <= MAX_سمون) {
            fill_کړکۍ(s);
            إذا (s->lookahead <= MAX_سمون && flush == Z_NO_FLUSH) {
                أرجع need_نور;
            }
            إذا (s->lookahead == 0) اكسر; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->سمون_اوږدتیا = 0;
        إذا (s->lookahead >= MIN_سمون && s->strstart > 0) {
            لټون = s->کړکۍ + s->strstart - 1;
            prev = *لټون;
            إذا (prev == *++لټون && prev == *++لټون && prev == *++لټون) {
                strend = s->کړکۍ + s->strstart + MAX_سمون;
                افعل {
                } طالما (prev == *++لټون && prev == *++لټون &&
                         prev == *++لټون && prev == *++لټون &&
                         prev == *++لټون && prev == *++لټون &&
                         prev == *++لټون && prev == *++لټون &&
                         لټون < strend);
                s->سمون_اوږدتیا = MAX_سمون - (uInt)(strend - لټون);
                إذا (s->سمون_اوږدتیا > s->lookahead)
                    s->سمون_اوږدتیا = s->lookahead;
            }
            Assert(لټون <= s->کړکۍ + (uInt)(s->کړکۍ_اندازه - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        إذا (s->سمون_اوږدتیا >= MIN_سمون) {
            check_سمون(s, s->strstart, s->strstart - 1, (صحيح)s->سمون_اوږدتیا);

            _tr_tally_dist(s, 1, s->سمون_اوږدتیا - MIN_سمون, bflush);

            s->lookahead -= s->سمون_اوږدتیا;
            s->strstart += s->سمون_اوږدتیا;
            s->سمون_اوږدتیا = 0;
        } وإلا {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->کړکۍ[s->strstart]));
            _tr_tally_lit(s, s->کړکۍ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        إذا (bflush) FLUSH_بلاک(s, 0);
    }
    s->ورننباسه = 0;
    إذا (flush == Z_پای) {
        FLUSH_بلاک(s, 1);
        أرجع پای_بشپړ;
    }
    إذا (s->sym_بل)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_بشپړ;
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
            fill_کړکۍ(s);
            إذا (s->lookahead == 0) {
                إذا (flush == Z_NO_FLUSH)
                    أرجع need_نور;
                اكسر;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->سمون_اوږدتیا = 0;
        Tracevv((stderr,"%c", s->کړکۍ[s->strstart]));
        _tr_tally_lit(s, s->کړکۍ[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        إذا (bflush) FLUSH_بلاک(s, 0);
    }
    s->ورننباسه = 0;
    إذا (flush == Z_پای) {
        FLUSH_بلاک(s, 1);
        أرجع پای_بشپړ;
    }
    إذا (s->sym_بل)
        FLUSH_بلاک(s, 0);
    أرجع بلاک_بشپړ;
}
