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
    need_अधिकम्,      /* block not completed, need more input or more output */
    खण्डम्_समाप्तम्,     /* block flush performed */
    finish_started, /* finish started, need only more output at next deflate */
    finish_समाप्तम्     /* finish done, accept no more input or output */
} खण्डम्_state;

प्रकारनिर्देश खण्डम्_state (*compress_func)(deflate_state *s, पूर्णाङ्क flush);
/* Compression function. Returns the block state after the call. */

local खण्डम्_state deflate_stored(deflate_state *s, पूर्णाङ्क flush);
local खण्डम्_state deflate_fast(deflate_state *s, पूर्णाङ्क flush);
#यदि_अपरिभाषितम् FASTEST
local खण्डम्_state deflate_slow(deflate_state *s, पूर्णाङ्क flush);
#यदि_अन्तः
local खण्डम्_state deflate_rle(deflate_state *s, पूर्णाङ्क flush);
local खण्डम्_state deflate_huff(deflate_state *s, पूर्णाङ्क flush);

/* ===========================================================================
 * Local data
 */

#परिभाषय NIL 0
/* Tail of hash chains */

#यदि_अपरिभाषितम् TOO_FAR
#  define TOO_FAR 4096
#यदि_अन्तः
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
प्रकारनिर्देश संरचना config_s {
   ush good_दीर्घता; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_दीर्घता; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#यदि_परिभाषितम् FASTEST
local ध्रुव config configuration_सारणी[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#अन्यथा
local ध्रुव config configuration_सारणी[10] = {
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
#यदि_अन्तः

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#परिभाषय RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#परिभाषय UPDATE_HASH(s,h,c) (h = (((h) << s->hash_सरणम्) ^ (c)) & s->hash_आवरणम्)


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
#यदि_परिभाषितम् FASTEST
#परिभाषय निवेशय_STRING(s, str, मेलनम्_शीर्ष) \
   (UPDATE_HASH(s, s->ins_h, s->गवाक्षः[(str) + (MIN_मेलनम्-1)]), \
    मेलनम्_शीर्ष = s->शीर्ष[s->ins_h], \
    s->शीर्ष[s->ins_h] = (Pos)(str))
#अन्यथा
#परिभाषय निवेशय_STRING(s, str, मेलनम्_शीर्ष) \
   (UPDATE_HASH(s, s->ins_h, s->गवाक्षः[(str) + (MIN_मेलनम्-1)]), \
    मेलनम्_शीर्ष = s->पूर्व[(str) & s->w_आवरणम्] = s->शीर्ष[s->ins_h], \
    s->शीर्ष[s->ins_h] = (Pos)(str))
#यदि_अन्तः

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#परिभाषय CLEAR_HASH(s) \
    कुरु { \
        s->शीर्ष[s->hash_आकार - 1] = NIL; \
        zmemzero(s->शीर्ष, (अचिह्नित)(s->hash_आकार - 1)*परिमाण(*s->शीर्ष)); \
        s->slid = 0; \
    } यावत् (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#यदि defined(__has_feature)
#  यदि __has_feature(स्मृतिः_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#यदि_अन्तः
local शून्य slide_hash(deflate_state *s) {
    अचिह्नित n, m;
    Posf *p;
    uInt wsize = s->w_आकार;

    n = s->hash_आकार;
    p = &s->शीर्ष[n];
    कुरु {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } यावत् (--n);
#यदि_अपरिभाषितम् FASTEST
    n = wsize;
    p = &s->पूर्व[n];
    कुरु {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } यावत् (--n);
#यदि_अन्तः
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local अचिह्नित read_buf(z_streamp strm, Bytef *buf, अचिह्नित आकार) {
    अचिह्नित len = strm->avail_अन्तर्;

    यदि (len > आकार) len = आकार;
    यदि (len == 0) प्रत्यागम 0;

    strm->avail_अन्तर्  -= len;

    zmemcpy(buf, strm->अग्र_अन्तर्, len);
    यदि (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#यदि_परिभाषितम् GZIP
    अन्यथा यदि (strm->state->wrap == 2) {
        strm->adler = चक्रीयपरीक्षा_32_2(strm->adler, buf, len);
    }
#यदि_अन्तः
    strm->अग्र_अन्तर्  += len;
    strm->योग_अन्तर् += len;

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
local शून्य fill_गवाक्षः(deflate_state *s) {
    अचिह्नित n;
    अचिह्नित अधिकम्;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_आकार;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    कुरु {
        अधिकम् = (अचिह्नित)(s->गवाक्षः_आकार -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#यदि_परिभाषितम् _MSC_VER
#निर्देशः warning(प्रक्षिप)
#निर्देशः warning(disable: 4127)
#यदि_अन्तः
        यदि (परिमाण(पूर्णाङ्क) <= 2) {
#यदि_परिभाषितम् _MSC_VER
#निर्देशः warning(निष्कासय)
#यदि_अन्तः
            यदि (अधिकम् == 0 && s->strstart == 0 && s->lookahead == 0) {
                अधिकम् = wsize;

            } अन्यथा यदि (अधिकम् == (अचिह्नित)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                अधिकम्--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        यदि (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->गवाक्षः, s->गवाक्षः + wsize, (अचिह्नित)wsize - अधिकम्);
            s->मेलनम्_start -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->खण्डम्_start -= (दीर्घ) wsize;
            यदि (s->निवेशय > s->strstart)
                s->निवेशय = s->strstart;
            slide_hash(s);
            अधिकम् += wsize;
        }
        यदि (s->strm->avail_अन्तर् == 0) भङ्ग;

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
        Assert(अधिकम् >= 2, "more < 2");

        n = read_buf(s->strm, s->गवाक्षः + s->strstart + s->lookahead, अधिकम्);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        यदि (s->lookahead + s->निवेशय >= MIN_मेलनम्) {
            uInt str = s->strstart - s->निवेशय;
            s->ins_h = s->गवाक्षः[str];
            UPDATE_HASH(s, s->ins_h, s->गवाक्षः[str + 1]);
#यदि MIN_मेलनम् != 3
            Call UPDATE_HASH() MIN_मेलनम्-3 अधिकम् times
#यदि_अन्तः
            यावत् (s->निवेशय) {
                UPDATE_HASH(s, s->ins_h, s->गवाक्षः[str + MIN_मेलनम्-1]);
#यदि_अपरिभाषितम् FASTEST
                s->पूर्व[str & s->w_आवरणम्] = s->शीर्ष[s->ins_h];
#यदि_अन्तः
                s->शीर्ष[s->ins_h] = (Pos)str;
                str++;
                s->निवेशय--;
                यदि (s->lookahead + s->निवेशय < MIN_मेलनम्)
                    भङ्ग;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } यावत् (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_अन्तर् != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    यदि (s->उच्चः_water < s->गवाक्षः_आकार) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        यदि (s->उच्चः_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->गवाक्षः_आकार - curr;
            यदि (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->गवाक्षः + curr, (अचिह्नित)init);
            s->उच्चः_water = curr + init;
        }
        अन्यथा यदि (s->उच्चः_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->उच्चः_water;
            यदि (init > s->गवाक्षः_आकार - s->उच्चः_water)
                init = s->गवाक्षः_आकार - s->उच्चः_water;
            zmemzero(s->गवाक्षः + s->उच्चः_water, (अचिह्नित)init);
            s->उच्चः_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->गवाक्षः_आकार - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateInit_(z_streamp strm, पूर्णाङ्क level, ध्रुव वर्ण *संस्करणम्,
                         पूर्णाङ्क प्रवाहः_आकार) {
    प्रत्यागम deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, संस्करणम्, प्रवाहः_आकार);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateInit2_(z_streamp strm, पूर्णाङ्क level, पूर्णाङ्क विधानम्,
                          पूर्णाङ्क गवाक्षः_द्वयङ्काः, पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                          ध्रुव वर्ण *संस्करणम्, पूर्णाङ्क प्रवाहः_आकार) {
    deflate_state *s;
    पूर्णाङ्क wrap = 1;
    स्थिर ध्रुव वर्ण my_संस्करणम्[] = ZLIB_संस्करणम्;

    यदि (संस्करणम् == Z_NULL || संस्करणम्[0] != my_संस्करणम्[0] ||
        प्रवाहः_आकार != परिमाण(z_प्रवाहः)) {
        प्रत्यागम Z_संस्करणम्_ERROR;
    }
    यदि (strm == Z_NULL) प्रत्यागम Z_प्रवाहः_ERROR;

    strm->सन्देशः = Z_NULL;
    यदि (strm->zalloc == (alloc_func)0) {
#यदि_परिभाषितम् Z_SOLO
        प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#यदि_अन्तः
    }
    यदि (strm->zfree == (free_func)0)
#यदि_परिभाषितम् Z_SOLO
        प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
        strm->zfree = zcfree;
#यदि_अन्तः

#यदि_परिभाषितम् FASTEST
    यदि (level != 0) level = 1;
#अन्यथा
    यदि (level == Z_DEFAULT_COMPRESSION) level = 6;
#यदि_अन्तः

    यदि (गवाक्षः_द्वयङ्काः < 0) { /* suppress zlib wrapper */
        wrap = 0;
        यदि (गवाक्षः_द्वयङ्काः < -15)
            प्रत्यागम Z_प्रवाहः_ERROR;
        गवाक्षः_द्वयङ्काः = -गवाक्षः_द्वयङ्काः;
    }
#यदि_परिभाषितम् GZIP
    अन्यथा यदि (गवाक्षः_द्वयङ्काः > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        गवाक्षः_द्वयङ्काः -= 16;
    }
#यदि_अन्तः
    यदि (memLevel < 1 || memLevel > MAX_MEM_LEVEL || विधानम् != Z_DEFLATED ||
        गवाक्षः_द्वयङ्काः < 8 || गवाक्षः_द्वयङ्काः > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (गवाक्षः_द्वयङ्काः == 8 && wrap != 1)) {
        प्रत्यागम Z_प्रवाहः_ERROR;
    }
    यदि (गवाक्षः_द्वयङ्काः == 8) गवाक्षः_द्वयङ्काः = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, परिमाण(deflate_state));
    यदि (s == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(s, परिमाण(deflate_state));
    strm->state = (संरचना आन्तरिकम्_state FAR *)s;
    s->strm = strm;
    s->स्थितिः = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_द्वयङ्काः = (uInt)गवाक्षः_द्वयङ्काः;
    s->w_आकार = 1 << s->w_द्वयङ्काः;
    s->w_आवरणम् = s->w_आकार - 1;

    s->hash_द्वयङ्काः = (uInt)memLevel + 7;
    s->hash_आकार = 1 << s->hash_द्वयङ्काः;
    s->hash_आवरणम् = s->hash_आकार - 1;
    s->hash_सरणम् =  ((s->hash_द्वयङ्काः + MIN_मेलनम्-1) / MIN_मेलनम्);

    s->गवाक्षः = (Bytef *) ZALLOC(strm, s->w_आकार, 2*परिमाण(Byte));
    s->पूर्व   = (Posf *)  ZALLOC(strm, s->w_आकार, परिमाण(Pos));
    s->शीर्ष   = (Posf *)  ZALLOC(strm, s->hash_आकार, परिमाण(Pos));

    s->उच्चः_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_आकार = (ulg)s->lit_bufsize * 4;

    यदि (s->गवाक्षः == Z_NULL || s->पूर्व == Z_NULL || s->शीर्ष == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->स्थितिः = FINISH_STATE;
        strm->सन्देशः = ERR_सन्देशः(Z_MEM_ERROR);
        deflate_अन्तः (strm);
        प्रत्यागम Z_MEM_ERROR;
    }
#यदि_परिभाषितम् LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_अन्तः = s->lit_bufsize - 1;
#अन्यथा
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_अन्तः = (s->lit_bufsize - 1) * 3;
#यदि_अन्तः
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->विधानम् = (Byte)विधानम्;

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
    यदि (s == Z_NULL || s->strm != strm || (s->स्थितिः != INIT_STATE &&
#यदि_परिभाषितम् GZIP
                                           s->स्थितिः != GZIP_STATE &&
#यदि_अन्तः
                                           s->स्थितिः != EXTRA_STATE &&
                                           s->स्थितिः != NAME_STATE &&
                                           s->स्थितिः != COMMENT_STATE &&
                                           s->स्थितिः != HCRC_STATE &&
                                           s->स्थितिः != BUSY_STATE &&
                                           s->स्थितिः != FINISH_STATE))
        प्रत्यागम 1;
    प्रत्यागम 0;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflate_समुच्चयः_Dictionary(z_streamp strm, ध्रुव Bytef *dictionary,
                                 uInt  dict_दीर्घता) {
    deflate_state *s;
    uInt str, n;
    पूर्णाङ्क wrap;
    अचिह्नित avail;
    z_const अचिह्नित वर्ण *अग्र;

    यदि (deflateStateCheck(strm) || dictionary == Z_NULL)
        प्रत्यागम Z_प्रवाहः_ERROR;
    s = strm->state;
    wrap = s->wrap;
    यदि (wrap == 2 || (wrap == 1 && s->स्थितिः != INIT_STATE) || s->lookahead)
        प्रत्यागम Z_प्रवाहः_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    यदि (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_दीर्घता);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    यदि (dict_दीर्घता >= s->w_आकार) {
        यदि (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->खण्डम्_start = 0L;
            s->निवेशय = 0;
        }
        dictionary += dict_दीर्घता - s->w_आकार;  /* use the tail */
        dict_दीर्घता = s->w_आकार;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_अन्तर्;
    अग्र = strm->अग्र_अन्तर्;
    strm->avail_अन्तर् = dict_दीर्घता;
    strm->अग्र_अन्तर् = (z_const Bytef *)dictionary;
    fill_गवाक्षः(s);
    यावत् (s->lookahead >= MIN_मेलनम्) {
        str = s->strstart;
        n = s->lookahead - (MIN_मेलनम्-1);
        कुरु {
            UPDATE_HASH(s, s->ins_h, s->गवाक्षः[str + MIN_मेलनम्-1]);
#यदि_अपरिभाषितम् FASTEST
            s->पूर्व[str & s->w_आवरणम्] = s->शीर्ष[s->ins_h];
#यदि_अन्तः
            s->शीर्ष[s->ins_h] = (Pos)str;
            str++;
        } यावत् (--n);
        s->strstart = str;
        s->lookahead = MIN_मेलनम्-1;
        fill_गवाक्षः(s);
    }
    s->strstart += s->lookahead;
    s->खण्डम्_start = (दीर्घ)s->strstart;
    s->निवेशय = s->lookahead;
    s->lookahead = 0;
    s->मेलनम्_दीर्घता = s->पूर्व_दीर्घता = MIN_मेलनम्-1;
    s->मेलनम्_उपलब्धम् = 0;
    strm->अग्र_अन्तर् = अग्र;
    strm->avail_अन्तर् = avail;
    s->wrap = wrap;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_दीर्घता) {
    deflate_state *s;
    uInt len;

    यदि (deflateStateCheck(strm))
        प्रत्यागम Z_प्रवाहः_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    यदि (len > s->w_आकार)
        len = s->w_आकार;
    यदि (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->गवाक्षः + s->strstart + s->lookahead - len, len);
    यदि (dict_दीर्घता != Z_NULL)
        *dict_दीर्घता = len;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    यदि (deflateStateCheck(strm)) {
        प्रत्यागम Z_प्रवाहः_ERROR;
    }

    strm->योग_अन्तर् = strm->योग_बहिर्गमनम् = 0;
    strm->सन्देशः = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->दत्त_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_बहिर्गमनम् = s->pending_buf;

    यदि (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->स्थितिः =
#यदि_परिभाषितम् GZIP
        s->wrap == 2 ? GZIP_STATE :
#यदि_अन्तः
        INIT_STATE;
    strm->adler =
#यदि_परिभाषितम् GZIP
        s->wrap == 2 ? चक्रीयपरीक्षा_32_2(0L, Z_NULL, 0) :
#यदि_अन्तः
        adler32(0L, Z_NULL, 0);
    s->चरमः_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    प्रत्यागम Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local शून्य lm_init(deflate_state *s) {
    s->गवाक्षः_आकार = (ulg)2L*s->w_आकार;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_मेलनम्   = configuration_सारणी[s->level].max_lazy;
    s->good_मेलनम्       = configuration_सारणी[s->level].good_दीर्घता;
    s->nice_मेलनम्       = configuration_सारणी[s->level].nice_दीर्घता;
    s->max_chain_दीर्घता = configuration_सारणी[s->level].max_chain;

    s->strstart = 0;
    s->खण्डम्_start = 0L;
    s->lookahead = 0;
    s->निवेशय = 0;
    s->मेलनम्_दीर्घता = s->पूर्व_दीर्घता = MIN_मेलनम्-1;
    s->मेलनम्_उपलब्धम् = 0;
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
पूर्णाङ्क ZEXPORT deflate_समुच्चयः_शीर्षकम्(z_streamp strm, gz_headerp शीर्ष) {
    यदि (deflateStateCheck(strm) || strm->state->wrap != 2)
        प्रत्यागम Z_प्रवाहः_ERROR;
    strm->state->gzhead = शीर्ष;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflatePending(z_streamp strm, अचिह्नित *pending, पूर्णाङ्क *द्वयङ्काः) {
    यदि (deflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    यदि (द्वयङ्काः != Z_NULL)
        *द्वयङ्काः = strm->state->bi_valid;
    यदि (pending != Z_NULL) {
        *pending = (अचिह्नित)strm->state->pending;
        यदि (*pending != strm->state->pending) {
            *pending = (अचिह्नित)-1;
            प्रत्यागम Z_BUF_ERROR;
        }
    }
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflate_प्रयुक्तम्(z_streamp strm, पूर्णाङ्क *द्वयङ्काः) {
    यदि (deflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    यदि (द्वयङ्काः != Z_NULL)
        *द्वयङ्काः = strm->state->bi_प्रयुक्तम्;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflate_अविभाज्यः(z_streamp strm, पूर्णाङ्क द्वयङ्काः, पूर्णाङ्क मानम्) {
    deflate_state *s;
    पूर्णाङ्क put;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    s = strm->state;
#यदि_परिभाषितम् LIT_MEM
    यदि (द्वयङ्काः < 0 || द्वयङ्काः > 16 ||
        (uchf *)s->d_buf < s->pending_बहिर्गमनम् + ((Buf_आकार + 7) >> 3))
        प्रत्यागम Z_BUF_ERROR;
#अन्यथा
    यदि (द्वयङ्काः < 0 || द्वयङ्काः > 16 ||
        s->sym_buf < s->pending_बहिर्गमनम् + ((Buf_आकार + 7) >> 3))
        प्रत्यागम Z_BUF_ERROR;
#यदि_अन्तः
    कुरु {
        put = Buf_आकार - s->bi_valid;
        यदि (put > द्वयङ्काः)
            put = द्वयङ्काः;
        s->bi_buf |= (ush)((मानम् & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_द्वयङ्काः(s);
        मानम् >>= put;
        द्वयङ्काः -= put;
    } यावत् (द्वयङ्काः);
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateParams(z_streamp strm, पूर्णाङ्क level, पूर्णाङ्क strategy) {
    deflate_state *s;
    compress_func func;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    s = strm->state;

#यदि_परिभाषितम् FASTEST
    यदि (level != 0) level = 1;
#अन्यथा
    यदि (level == Z_DEFAULT_COMPRESSION) level = 6;
#यदि_अन्तः
    यदि (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        प्रत्यागम Z_प्रवाहः_ERROR;
    }
    func = configuration_सारणी[s->level].func;

    यदि ((strategy != s->strategy || func != configuration_सारणी[level].func) &&
        s->चरमः_flush != -2) {
        /* Flush the last buffer: */
        पूर्णाङ्क err = deflate(strm, Z_खण्डम्);
        यदि (err == Z_प्रवाहः_ERROR)
            प्रत्यागम err;
        यदि (strm->avail_अन्तर् || (s->strstart - s->खण्डम्_start) + s->lookahead)
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
        s->max_lazy_मेलनम्   = configuration_सारणी[level].max_lazy;
        s->good_मेलनम्       = configuration_सारणी[level].good_दीर्घता;
        s->nice_मेलनम्       = configuration_सारणी[level].nice_दीर्घता;
        s->max_chain_दीर्घता = configuration_सारणी[level].max_chain;
    }
    s->strategy = strategy;
    प्रत्यागम Z_OK;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflateTune(z_streamp strm, पूर्णाङ्क good_दीर्घता, पूर्णाङ्क max_lazy,
                        पूर्णाङ्क nice_दीर्घता, पूर्णाङ्क max_chain) {
    deflate_state *s;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;
    s = strm->state;
    s->good_मेलनम् = (uInt)good_दीर्घता;
    s->max_lazy_मेलनम् = (uInt)max_lazy;
    s->nice_मेलनम् = nice_दीर्घता;
    s->max_chain_दीर्घता = (uInt)max_chain;
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
z_आकार_t ZEXPORT deflateBound_z(z_streamp strm, z_आकार_t sourceLen) {
    deflate_state *s;
    z_आकार_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = sourceLen + (sourceLen >> 3) + (sourceLen >> 8) +
               (sourceLen >> 9) + 4;
    यदि (fixedlen < sourceLen)
        fixedlen = (z_आकार_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = sourceLen + (sourceLen >> 5) + (sourceLen >> 7) +
               (sourceLen >> 11) + 7;
    यदि (storelen < sourceLen)
        storelen = (z_आकार_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    यदि (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        प्रत्यागम bound + 18 < bound ? (z_आकार_t)-1 : bound + 18;
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
#यदि_परिभाषितम् GZIP
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
#यदि_अन्तः
    स्वभाव:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    यदि (s->w_द्वयङ्काः != 15 || s->hash_द्वयङ्काः != 8 + 7) {
        bound = s->w_द्वयङ्काः <= s->hash_द्वयङ्काः && s->level ? fixedlen :
                                                        storelen;
        प्रत्यागम bound + wraplen < bound ? (z_आकार_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
            (sourceLen >> 25) + 13 - 6 + wraplen;
    प्रत्यागम bound < sourceLen ? (z_आकार_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong sourceLen) {
    z_आकार_t bound = deflateBound_z(strm, sourceLen);
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
    अचिह्नित len;
    deflate_state *s = strm->state;

    tr_flush_द्वयङ्काः(s);
    len = s->pending > strm->avail_बहिर्गमनम् ? strm->avail_बहिर्गमनम् :
                                         (अचिह्नित)s->pending;
    यदि (len == 0) प्रत्यागम;

    zmemcpy(strm->अग्र_बहिर्गमनम्, s->pending_बहिर्गमनम्, len);
    strm->अग्र_बहिर्गमनम्  += len;
    s->pending_बहिर्गमनम्  += len;
    strm->योग_बहिर्गमनम् += len;
    strm->avail_बहिर्गमनम् -= len;
    s->pending      -= len;
    यदि (s->pending == 0) {
        s->pending_बहिर्गमनम् = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#परिभाषय HCRC_UPDATE(beg) \
    कुरु { \
        यदि (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = चक्रीयपरीक्षा_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } यावत् (0)

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflate(z_streamp strm, पूर्णाङ्क flush) {
    पूर्णाङ्क old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    यदि (deflateStateCheck(strm) || flush > Z_खण्डम् || flush < 0) {
        प्रत्यागम Z_प्रवाहः_ERROR;
    }
    s = strm->state;

    यदि (strm->अग्र_बहिर्गमनम् == Z_NULL ||
        (strm->avail_अन्तर् != 0 && strm->अग्र_अन्तर् == Z_NULL) ||
        (s->स्थितिः == FINISH_STATE && flush != Z_FINISH)) {
        ERR_RETURN(strm, Z_प्रवाहः_ERROR);
    }
    यदि (strm->avail_बहिर्गमनम् == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->चरमः_flush;
    s->चरमः_flush = flush;

    /* Flush as much pending output as possible */
    यदि (s->pending != 0) {
        flush_pending(strm);
        यदि (strm->avail_बहिर्गमनम् == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->चरमः_flush = -1;
            प्रत्यागम Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } अन्यथा यदि (strm->avail_अन्तर् == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_FINISH) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    यदि (s->स्थितिः == FINISH_STATE && strm->avail_अन्तर् != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    यदि (s->स्थितिः == INIT_STATE && s->wrap == 0)
        s->स्थितिः = BUSY_STATE;
    यदि (s->स्थितिः == INIT_STATE) {
        /* zlib header */
        uInt शीर्षकम् = (Z_DEFLATED + ((s->w_द्वयङ्काः - 8) << 4)) << 8;
        uInt level_flags;

        यदि (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        अन्यथा यदि (s->level < 6)
            level_flags = 1;
        अन्यथा यदि (s->level == 6)
            level_flags = 2;
        अन्यथा
            level_flags = 3;
        शीर्षकम् |= (level_flags << 6);
        यदि (s->strstart != 0) शीर्षकम् |= PRESET_DICT;
        शीर्षकम् += 31 - (शीर्षकम् % 31);

        putShortMSB(s, शीर्षकम्);

        /* Save the adler32 of the preset dictionary: */
        यदि (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->स्थितिः = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        यदि (s->pending != 0) {
            s->चरमः_flush = -1;
            प्रत्यागम Z_OK;
        }
    }
#यदि_परिभाषितम् GZIP
    यदि (s->स्थितिः == GZIP_STATE) {
        /* gzip header */
        strm->adler = चक्रीयपरीक्षा_32_2(0L, Z_NULL, 0);
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
            put_byte(s, OS_संहिता);
            s->स्थितिः = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            यदि (s->pending != 0) {
                s->चरमः_flush = -1;
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
                strm->adler = चक्रीयपरीक्षा_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->स्थितिः = EXTRA_STATE;
        }
    }
    यदि (s->स्थितिः == EXTRA_STATE) {
        यदि (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg वामः = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            यावत् (s->pending + वामः > s->pending_buf_आकार) {
                ulg copy = s->pending_buf_आकार - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_आकार;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                यदि (s->pending != 0) {
                    s->चरमः_flush = -1;
                    प्रत्यागम Z_OK;
                }
                beg = 0;
                वामः -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, वामः);
            s->pending += वामः;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->स्थितिः = NAME_STATE;
    }
    यदि (s->स्थितिः == NAME_STATE) {
        यदि (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            पूर्णाङ्क val;
            कुरु {
                यदि (s->pending == s->pending_buf_आकार) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    यदि (s->pending != 0) {
                        s->चरमः_flush = -1;
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
        s->स्थितिः = COMMENT_STATE;
    }
    यदि (s->स्थितिः == COMMENT_STATE) {
        यदि (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            पूर्णाङ्क val;
            कुरु {
                यदि (s->pending == s->pending_buf_आकार) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    यदि (s->pending != 0) {
                        s->चरमः_flush = -1;
                        प्रत्यागम Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } यावत् (val != 0);
            HCRC_UPDATE(beg);
        }
        s->स्थितिः = HCRC_STATE;
    }
    यदि (s->स्थितिः == HCRC_STATE) {
        यदि (s->gzhead->hcrc) {
            यदि (s->pending + 2 > s->pending_buf_आकार) {
                flush_pending(strm);
                यदि (s->pending != 0) {
                    s->चरमः_flush = -1;
                    प्रत्यागम Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = चक्रीयपरीक्षा_32_2(0L, Z_NULL, 0);
        }
        s->स्थितिः = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        यदि (s->pending != 0) {
            s->चरमः_flush = -1;
            प्रत्यागम Z_OK;
        }
    }
#यदि_अन्तः

    /* Start a new block or continue the current one.
     */
    यदि (strm->avail_अन्तर् != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->स्थितिः != FINISH_STATE)) {
        खण्डम्_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_सारणी[s->level].func))(s, flush);

        यदि (bstate == finish_started || bstate == finish_समाप्तम्) {
            s->स्थितिः = FINISH_STATE;
        }
        यदि (bstate == need_अधिकम् || bstate == finish_started) {
            यदि (strm->avail_बहिर्गमनम् == 0) {
                s->चरमः_flush = -1; /* avoid BUF_ERROR next call, see above */
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
        यदि (bstate == खण्डम्_समाप्तम्) {
            यदि (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } अन्यथा यदि (flush != Z_खण्डम्) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_खण्डम्(s, (वर्ण*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                यदि (flush == Z_पूर्णम्_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    यदि (s->lookahead == 0) {
                        s->strstart = 0;
                        s->खण्डम्_start = 0L;
                        s->निवेशय = 0;
                    }
                }
            }
            flush_pending(strm);
            यदि (strm->avail_बहिर्गमनम् == 0) {
              s->चरमः_flush = -1; /* avoid BUF_ERROR at next call, see above */
              प्रत्यागम Z_OK;
            }
        }
    }

    यदि (flush != Z_FINISH) प्रत्यागम Z_OK;
    यदि (s->wrap <= 0) प्रत्यागम Z_प्रवाहः_अन्तः;

    /* Write the trailer */
#यदि_परिभाषितम् GZIP
    यदि (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->योग_अन्तर् & 0xff));
        put_byte(s, (Byte)((strm->योग_अन्तर् >> 8) & 0xff));
        put_byte(s, (Byte)((strm->योग_अन्तर् >> 16) & 0xff));
        put_byte(s, (Byte)((strm->योग_अन्तर् >> 24) & 0xff));
    }
    अन्यथा
#यदि_अन्तः
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    यदि (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    प्रत्यागम s->pending != 0 ? Z_OK : Z_प्रवाहः_अन्तः;
}

/* ========================================================================= */
पूर्णाङ्क ZEXPORT deflate_अन्तः(z_streamp strm) {
    पूर्णाङ्क स्थितिः;

    यदि (deflateStateCheck(strm)) प्रत्यागम Z_प्रवाहः_ERROR;

    स्थितिः = strm->state->स्थितिः;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->शीर्ष);
    TRY_FREE(strm, strm->state->पूर्व);
    TRY_FREE(strm, strm->state->गवाक्षः);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    प्रत्यागम स्थितिः == BUSY_STATE ? Z_दत्त_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
पूर्णाङ्क ZEXPORT deflateCopy(z_streamp dest, z_streamp source) {
#यदि_परिभाषितम् MAXSEG_64K
    (शून्य)dest;
    (शून्य)source;
    प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
    deflate_state *ds;
    deflate_state *ss;


    यदि (deflateStateCheck(source) || dest == Z_NULL) {
        प्रत्यागम Z_प्रवाहः_ERROR;
    }

    ss = source->state;

    zmemcpy(dest, source, परिमाण(z_प्रवाहः));

    ds = (deflate_state *) ZALLOC(dest, 1, परिमाण(deflate_state));
    यदि (ds == Z_NULL) प्रत्यागम Z_MEM_ERROR;
    zmemzero(ds, परिमाण(deflate_state));
    dest->state = (संरचना आन्तरिकम्_state FAR *) ds;
    zmemcpy(ds, ss, परिमाण(deflate_state));
    ds->strm = dest;

    ds->गवाक्षः = (Bytef *) ZALLOC(dest, ds->w_आकार, 2*परिमाण(Byte));
    ds->पूर्व   = (Posf *)  ZALLOC(dest, ds->w_आकार, परिमाण(Pos));
    ds->शीर्ष   = (Posf *)  ZALLOC(dest, ds->hash_आकार, परिमाण(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    यदि (ds->गवाक्षः == Z_NULL || ds->पूर्व == Z_NULL || ds->शीर्ष == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_अन्तः (dest);
        प्रत्यागम Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->गवाक्षः, ss->गवाक्षः, ss->उच्चः_water);
    zmemcpy(ds->पूर्व, ss->पूर्व,
            (ss->slid || ss->strstart - ss->निवेशय > ds->w_आकार ? ds->w_आकार :
                ss->strstart - ss->निवेशय) * परिमाण(Pos));
    zmemcpy(ds->शीर्ष, ss->शीर्ष, ds->hash_आकार * परिमाण(Pos));

    ds->pending_बहिर्गमनम् = ds->pending_buf + (ss->pending_बहिर्गमनम् - ss->pending_buf);
    zmemcpy(ds->pending_बहिर्गमनम्, ss->pending_बहिर्गमनम्, ss->pending);
#यदि_परिभाषितम् LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_अग्र * परिमाण(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_अग्र);
#अन्यथा
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_अग्र);
#यदि_अन्तः

    ds->l_desc.dyn_वृक्षः = ds->dyn_ltree;
    ds->d_desc.dyn_वृक्षः = ds->dyn_dtree;
    ds->bl_desc.dyn_वृक्षः = ds->bl_वृक्षः;

    प्रत्यागम Z_OK;
#यदि_अन्तः /* MAXSEG_64K */
}

#यदि_अपरिभाषितम् FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_मेलनम्(deflate_state *s, IPos cur_मेलनम्) {
    अचिह्नित chain_दीर्घता = s->max_chain_दीर्घता;/* max hash chain length */
    Bytef *अवलोकनम् = s->गवाक्षः + s->strstart;      /* current string */
    Bytef *मेलनम्;                               /* matched string */
    पूर्णाङ्क len;                                    /* length of current match */
    पूर्णाङ्क best_len = (पूर्णाङ्क)s->पूर्व_दीर्घता;         /* best match length so far */
    पूर्णाङ्क nice_मेलनम् = s->nice_मेलनम्;             /* stop if match long enough */
    IPos सीमा = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *पूर्व = s->पूर्व;
    uInt wmask = s->w_आवरणम्;

#यदि_परिभाषितम् UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->गवाक्षः + s->strstart + MAX_मेलनम् - 1;
    ush अवलोकनम्_start = *(ushf*)अवलोकनम्;
    ush अवलोकनम्_अन्तः   = *(ushf*)(अवलोकनम् + best_len - 1);
#अन्यथा
    Bytef *strend = s->गवाक्षः + s->strstart + MAX_मेलनम्;
    Byte अवलोकनम्_end1  = अवलोकनम्[best_len - 1];
    Byte अवलोकनम्_अन्तः   = अवलोकनम्[best_len];
#यदि_अन्तः

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_द्वयङ्काः >= 8 && MAX_मेलनम् == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    यदि (s->पूर्व_दीर्घता >= s->good_मेलनम्) {
        chain_दीर्घता >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    यदि ((uInt)nice_मेलनम् > s->lookahead) nice_मेलनम् = (पूर्णाङ्क)s->lookahead;

    Assert((ulg)s->strstart <= s->गवाक्षः_आकार - MIN_LOOKAHEAD,
           "need lookahead");

    कुरु {
        Assert(cur_मेलनम् < s->strstart, "no future");
        मेलनम् = s->गवाक्षः + cur_मेलनम्;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#यदि (defined(UNALIGNED_OK) && MAX_मेलनम् == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        यदि (*(ushf*)(मेलनम् + best_len - 1) != अवलोकनम्_अन्तः ||
            *(ushf*)मेलनम् != अवलोकनम्_start) अनुवर्त;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(अवलोकनम्[2] == मेलनम्[2], "scan[2]?");
        अवलोकनम्++, मेलनम्++;
        कुरु {
        } यावत् (*(ushf*)(अवलोकनम् += 2) == *(ushf*)(मेलनम् += 2) &&
                 *(ushf*)(अवलोकनम् += 2) == *(ushf*)(मेलनम् += 2) &&
                 *(ushf*)(अवलोकनम् += 2) == *(ushf*)(मेलनम् += 2) &&
                 *(ushf*)(अवलोकनम् += 2) == *(ushf*)(मेलनम् += 2) &&
                 अवलोकनम् < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(अवलोकनम् <= s->गवाक्षः + (अचिह्नित)(s->गवाक्षः_आकार - 1),
               "wild scan");
        यदि (*अवलोकनम् == *मेलनम्) अवलोकनम्++;

        len = (MAX_मेलनम् - 1) - (पूर्णाङ्क)(strend - अवलोकनम्);
        अवलोकनम् = strend - (MAX_मेलनम्-1);

#अन्यथा /* UNALIGNED_OK */

        यदि (मेलनम्[best_len]     != अवलोकनम्_अन्तः  ||
            मेलनम्[best_len - 1] != अवलोकनम्_end1 ||
            *मेलनम्              != *अवलोकनम्     ||
            *++मेलनम्            != अवलोकनम्[1])      अनुवर्त;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        अवलोकनम् += 2, मेलनम्++;
        Assert(*अवलोकनम् == *मेलनम्, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        कुरु {
        } यावत् (*++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
                 *++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
                 *++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
                 *++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
                 अवलोकनम् < strend);

        Assert(अवलोकनम् <= s->गवाक्षः + (अचिह्नित)(s->गवाक्षः_आकार - 1),
               "wild scan");

        len = MAX_मेलनम् - (पूर्णाङ्क)(strend - अवलोकनम्);
        अवलोकनम् = strend - MAX_मेलनम्;

#यदि_अन्तः /* UNALIGNED_OK */

        यदि (len > best_len) {
            s->मेलनम्_start = cur_मेलनम्;
            best_len = len;
            यदि (len >= nice_मेलनम्) भङ्ग;
#यदि_परिभाषितम् UNALIGNED_OK
            अवलोकनम्_अन्तः = *(ushf*)(अवलोकनम् + best_len - 1);
#अन्यथा
            अवलोकनम्_end1  = अवलोकनम्[best_len - 1];
            अवलोकनम्_अन्तः   = अवलोकनम्[best_len];
#यदि_अन्तः
        }
    } यावत् ((cur_मेलनम् = पूर्व[cur_मेलनम् & wmask]) > सीमा
             && --chain_दीर्घता != 0);

    यदि ((uInt)best_len <= s->lookahead) प्रत्यागम (uInt)best_len;
    प्रत्यागम s->lookahead;
}

#अन्यथा /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_मेलनम्(deflate_state *s, IPos cur_मेलनम्) {
    Bytef *अवलोकनम् = s->गवाक्षः + s->strstart;      /* current string */
    Bytef *मेलनम्;                               /* matched string */
    पूर्णाङ्क len;                                    /* length of current match */
    Bytef *strend = s->गवाक्षः + s->strstart + MAX_मेलनम्;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_द्वयङ्काः >= 8 && MAX_मेलनम् == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->गवाक्षः_आकार - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_मेलनम् < s->strstart, "no future");

    मेलनम् = s->गवाक्षः + cur_मेलनम्;

    /* Return failure if the match length is less than 2:
     */
    यदि (मेलनम्[0] != अवलोकनम्[0] || मेलनम्[1] != अवलोकनम्[1]) प्रत्यागम MIN_मेलनम्-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    अवलोकनम् += 2, मेलनम् += 2;
    Assert(*अवलोकनम् == *मेलनम्, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    कुरु {
    } यावत् (*++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
             *++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
             *++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
             *++अवलोकनम् == *++मेलनम् && *++अवलोकनम् == *++मेलनम् &&
             अवलोकनम् < strend);

    Assert(अवलोकनम् <= s->गवाक्षः + (अचिह्नित)(s->गवाक्षः_आकार - 1), "wild scan");

    len = MAX_मेलनम् - (पूर्णाङ्क)(strend - अवलोकनम्);

    यदि (len < MIN_मेलनम्) प्रत्यागम MIN_मेलनम् - 1;

    s->मेलनम्_start = cur_मेलनम्;
    प्रत्यागम (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#यदि_अन्तः /* FASTEST */

#यदि_परिभाषितम् ZLIB_DEBUG

#परिभाषय EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local शून्य check_मेलनम्(deflate_state *s, IPos start, IPos मेलनम्, पूर्णाङ्क दीर्घता) {
    /* check that the match is indeed a match */
    Bytef *पृष्ठभागः = s->गवाक्षः + (पूर्णाङ्क)मेलनम्, *अत्र = s->गवाक्षः + start;
    IPos len = (IPos)दीर्घता;
    यदि (मेलनम् == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        पृष्ठभागः++;
        अत्र++;
        len--;
    }
    यदि (zmemcmp(पृष्ठभागः, अत्र, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                start, (पूर्णाङ्क)मेलनम्, दीर्घता);
        कुरु {
            fprintf(stderr, "(%02x %02x)", *पृष्ठभागः++, *अत्र++);
        } यावत् (--len != 0);
        z_error("invalid match");
    }
    यदि (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", start - मेलनम्, दीर्घता);
        कुरु { putc(s->गवाक्षः[start++], stderr); } यावत् (--दीर्घता != 0);
    }
}
#अन्यथा
#  define check_मेलनम्(s, start, मेलनम्, दीर्घता)
#यदि_अन्तः /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#परिभाषय FLUSH_खण्डम्_ONLY(s, चरमः) { \
   tr_flush_खण्डम्(s, (s->खण्डम्_start >= 0L ? \
                   (charf *)&s->गवाक्षः[(अचिह्नित)s->खण्डम्_start] : \
                   (charf *)Z_NULL), \
                (ulg)((दीर्घ)s->strstart - s->खण्डम्_start), \
                (चरमः)); \
   s->खण्डम्_start = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#परिभाषय FLUSH_खण्डम्(s, चरमः) { \
   FLUSH_खण्डम्_ONLY(s, चरमः); \
   यदि (s->strm->avail_बहिर्गमनम् == 0) प्रत्यागम (चरमः) ? finish_started : need_अधिकम्; \
}

/* Maximum stored block length in deflate format (not including header). */
#परिभाषय MAX_STORED 65535

/* Minimum of a and b. */
#परिभाषय MIN(a, b) ((a) > (b) ? (b) : (a))

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
local खण्डम्_state deflate_stored(deflate_state *s, पूर्णाङ्क flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    अचिह्नित min_खण्डम् = (अचिह्नित)(MIN(s->pending_buf_आकार - 5, s->w_आकार));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    पूर्णाङ्क चरमः = 0;
    अचिह्नित len, वामः, have;
    अचिह्नित प्रयुक्तम् = s->strm->avail_अन्तर्;
    कुरु {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((अचिह्नित)s->bi_valid + 42) >> 3;   /* bytes in header */
        यदि (s->strm->avail_बहिर्गमनम् < have)          /* need room for header */
            भङ्ग;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_बहिर्गमनम् - have;
        वामः = (अचिह्नित)(s->strstart - s->खण्डम्_start);    /* window bytes */
        यदि (len > (ulg)वामः + s->strm->avail_अन्तर्)
            len = वामः + s->strm->avail_अन्तर्;     /* limit len to the input */
        यदि (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        यदि (len < min_खण्डम् && ((len == 0 && flush != Z_FINISH) ||
                                flush == Z_NO_FLUSH ||
                                len != वामः + s->strm->avail_अन्तर्))
            भङ्ग;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        चरमः = flush == Z_FINISH && len == वामः + s->strm->avail_अन्तर् ? 1 : 0;
        tr_stored_खण्डम्(s, (वर्ण *)0, 0L, चरमः);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#यदि_परिभाषितम् ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->द्वयङ्काः_प्रेषितम् += len << 3;
#यदि_अन्तः

        /* Copy uncompressed bytes from the window to next_out. */
        यदि (वामः) {
            यदि (वामः > len)
                वामः = len;
            zmemcpy(s->strm->अग्र_बहिर्गमनम्, s->गवाक्षः + s->खण्डम्_start, वामः);
            s->strm->अग्र_बहिर्गमनम् += वामः;
            s->strm->avail_बहिर्गमनम् -= वामः;
            s->strm->योग_बहिर्गमनम् += वामः;
            s->खण्डम्_start += वामः;
            len -= वामः;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        यदि (len) {
            read_buf(s->strm, s->strm->अग्र_बहिर्गमनम्, len);
            s->strm->अग्र_बहिर्गमनम् += len;
            s->strm->avail_बहिर्गमनम् -= len;
            s->strm->योग_बहिर्गमनम् += len;
        }
    } यावत् (चरमः == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    प्रयुक्तम् -= s->strm->avail_अन्तर्;      /* number of input bytes directly copied */
    यदि (प्रयुक्तम्) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        यदि (प्रयुक्तम् >= s->w_आकार) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->गवाक्षः, s->strm->अग्र_अन्तर् - s->w_आकार, s->w_आकार);
            s->strstart = s->w_आकार;
            s->निवेशय = s->strstart;
        }
        अन्यथा {
            यदि (s->गवाक्षः_आकार - s->strstart <= प्रयुक्तम्) {
                /* Slide the window down. */
                s->strstart -= s->w_आकार;
                zmemcpy(s->गवाक्षः, s->गवाक्षः + s->w_आकार, s->strstart);
                यदि (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                यदि (s->निवेशय > s->strstart)
                    s->निवेशय = s->strstart;
            }
            zmemcpy(s->गवाक्षः + s->strstart, s->strm->अग्र_अन्तर् - प्रयुक्तम्, प्रयुक्तम्);
            s->strstart += प्रयुक्तम्;
            s->निवेशय += MIN(प्रयुक्तम्, s->w_आकार - s->निवेशय);
        }
        s->खण्डम्_start = s->strstart;
    }
    यदि (s->उच्चः_water < s->strstart)
        s->उच्चः_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    यदि (चरमः) {
        s->bi_प्रयुक्तम् = 8;
        प्रत्यागम finish_समाप्तम्;
    }

    /* If flushing and all input has been consumed, then done. */
    यदि (flush != Z_NO_FLUSH && flush != Z_FINISH &&
        s->strm->avail_अन्तर् == 0 && (दीर्घ)s->strstart == s->खण्डम्_start)
        प्रत्यागम खण्डम्_समाप्तम्;

    /* Fill the window with any remaining input. */
    have = (अचिह्नित)(s->गवाक्षः_आकार - s->strstart);
    यदि (s->strm->avail_अन्तर् > have && s->खण्डम्_start >= (दीर्घ)s->w_आकार) {
        /* Slide the window down. */
        s->खण्डम्_start -= s->w_आकार;
        s->strstart -= s->w_आकार;
        zmemcpy(s->गवाक्षः, s->गवाक्षः + s->w_आकार, s->strstart);
        यदि (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_आकार;          /* more space now */
        यदि (s->निवेशय > s->strstart)
            s->निवेशय = s->strstart;
    }
    यदि (have > s->strm->avail_अन्तर्)
        have = s->strm->avail_अन्तर्;
    यदि (have) {
        read_buf(s->strm, s->गवाक्षः + s->strstart, have);
        s->strstart += have;
        s->निवेशय += MIN(have, s->w_आकार - s->निवेशय);
    }
    यदि (s->उच्चः_water < s->strstart)
        s->उच्चः_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((अचिह्नित)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (अचिह्नित)MIN(s->pending_buf_आकार - have, MAX_STORED);
    min_खण्डम् = MIN(have, s->w_आकार);
    वामः = (अचिह्नित)(s->strstart - s->खण्डम्_start);
    यदि (वामः >= min_खण्डम् ||
        ((वामः || flush == Z_FINISH) && flush != Z_NO_FLUSH &&
         s->strm->avail_अन्तर् == 0 && वामः <= have)) {
        len = MIN(वामः, have);
        चरमः = flush == Z_FINISH && s->strm->avail_अन्तर् == 0 &&
               len == वामः ? 1 : 0;
        tr_stored_खण्डम्(s, (charf *)s->गवाक्षः + s->खण्डम्_start, len, चरमः);
        s->खण्डम्_start += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    यदि (चरमः)
        s->bi_प्रयुक्तम् = 8;
    प्रत्यागम चरमः ? finish_started : need_अधिकम्;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local खण्डम्_state deflate_fast(deflate_state *s, पूर्णाङ्क flush) {
    IPos hash_शीर्ष;       /* head of the hash chain */
    पूर्णाङ्क bflush;           /* set if current block must be flushed */

    प्रति (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        यदि (s->lookahead < MIN_LOOKAHEAD) {
            fill_गवाक्षः(s);
            यदि (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                प्रत्यागम need_अधिकम्;
            }
            यदि (s->lookahead == 0) भङ्ग; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_शीर्ष = NIL;
        यदि (s->lookahead >= MIN_मेलनम्) {
            निवेशय_STRING(s, s->strstart, hash_शीर्ष);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        यदि (hash_शीर्ष != NIL && s->strstart - hash_शीर्ष <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->मेलनम्_दीर्घता = longest_मेलनम् (s, hash_शीर्ष);
            /* longest_match() sets match_start */
        }
        यदि (s->मेलनम्_दीर्घता >= MIN_मेलनम्) {
            check_मेलनम्(s, s->strstart, s->मेलनम्_start, (पूर्णाङ्क)s->मेलनम्_दीर्घता);

            _tr_tally_dist(s, s->strstart - s->मेलनम्_start,
                           s->मेलनम्_दीर्घता - MIN_मेलनम्, bflush);

            s->lookahead -= s->मेलनम्_दीर्घता;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#यदि_अपरिभाषितम् FASTEST
            यदि (s->मेलनम्_दीर्घता <= s->max_निवेशय_दीर्घता &&
                s->lookahead >= MIN_मेलनम्) {
                s->मेलनम्_दीर्घता--; /* string at strstart already in table */
                कुरु {
                    s->strstart++;
                    निवेशय_STRING(s, s->strstart, hash_शीर्ष);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } यावत् (--s->मेलनम्_दीर्घता != 0);
                s->strstart++;
            } अन्यथा
#यदि_अन्तः
            {
                s->strstart += s->मेलनम्_दीर्घता;
                s->मेलनम्_दीर्घता = 0;
                s->ins_h = s->गवाक्षः[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->गवाक्षः[s->strstart + 1]);
#यदि MIN_मेलनम् != 3
                Call UPDATE_HASH() MIN_मेलनम्-3 अधिकम् times
#यदि_अन्तः
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } अन्यथा {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->गवाक्षः[s->strstart]));
            _tr_tally_lit(s, s->गवाक्षः[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        यदि (bflush) FLUSH_खण्डम्(s, 0);
    }
    s->निवेशय = s->strstart < MIN_मेलनम्-1 ? s->strstart : MIN_मेलनम्-1;
    यदि (flush == Z_FINISH) {
        FLUSH_खण्डम्(s, 1);
        प्रत्यागम finish_समाप्तम्;
    }
    यदि (s->sym_अग्र)
        FLUSH_खण्डम्(s, 0);
    प्रत्यागम खण्डम्_समाप्तम्;
}

#यदि_अपरिभाषितम् FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local खण्डम्_state deflate_slow(deflate_state *s, पूर्णाङ्क flush) {
    IPos hash_शीर्ष;          /* head of hash chain */
    पूर्णाङ्क bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    प्रति (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        यदि (s->lookahead < MIN_LOOKAHEAD) {
            fill_गवाक्षः(s);
            यदि (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                प्रत्यागम need_अधिकम्;
            }
            यदि (s->lookahead == 0) भङ्ग; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_शीर्ष = NIL;
        यदि (s->lookahead >= MIN_मेलनम्) {
            निवेशय_STRING(s, s->strstart, hash_शीर्ष);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->पूर्व_दीर्घता = s->मेलनम्_दीर्घता, s->पूर्व_मेलनम् = s->मेलनम्_start;
        s->मेलनम्_दीर्घता = MIN_मेलनम्-1;

        यदि (hash_शीर्ष != NIL && s->पूर्व_दीर्घता < s->max_lazy_मेलनम् &&
            s->strstart - hash_शीर्ष <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->मेलनम्_दीर्घता = longest_मेलनम् (s, hash_शीर्ष);
            /* longest_match() sets match_start */

            यदि (s->मेलनम्_दीर्घता <= 5 && (s->strategy == Z_FILTERED
#यदि TOO_FAR <= 32767
                || (s->मेलनम्_दीर्घता == MIN_मेलनम् &&
                    s->strstart - s->मेलनम्_start > TOO_FAR)
#यदि_अन्तः
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->मेलनम्_दीर्घता = MIN_मेलनम्-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        यदि (s->पूर्व_दीर्घता >= MIN_मेलनम् && s->मेलनम्_दीर्घता <= s->पूर्व_दीर्घता) {
            uInt max_निवेशय = s->strstart + s->lookahead - MIN_मेलनम्;
            /* Do not insert strings in hash table beyond this. */

            check_मेलनम्(s, s->strstart - 1, s->पूर्व_मेलनम्, (पूर्णाङ्क)s->पूर्व_दीर्घता);

            _tr_tally_dist(s, s->strstart - 1 - s->पूर्व_मेलनम्,
                           s->पूर्व_दीर्घता - MIN_मेलनम्, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->पूर्व_दीर्घता - 1;
            s->पूर्व_दीर्घता -= 2;
            कुरु {
                यदि (++s->strstart <= max_निवेशय) {
                    निवेशय_STRING(s, s->strstart, hash_शीर्ष);
                }
            } यावत् (--s->पूर्व_दीर्घता != 0);
            s->मेलनम्_उपलब्धम् = 0;
            s->मेलनम्_दीर्घता = MIN_मेलनम्-1;
            s->strstart++;

            यदि (bflush) FLUSH_खण्डम्(s, 0);

        } अन्यथा यदि (s->मेलनम्_उपलब्धम्) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->गवाक्षः[s->strstart - 1]));
            _tr_tally_lit(s, s->गवाक्षः[s->strstart - 1], bflush);
            यदि (bflush) {
                FLUSH_खण्डम्_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            यदि (s->strm->avail_बहिर्गमनम् == 0) प्रत्यागम need_अधिकम्;
        } अन्यथा {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->मेलनम्_उपलब्धम् = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    यदि (s->मेलनम्_उपलब्धम्) {
        Tracevv((stderr,"%c", s->गवाक्षः[s->strstart - 1]));
        _tr_tally_lit(s, s->गवाक्षः[s->strstart - 1], bflush);
        s->मेलनम्_उपलब्धम् = 0;
    }
    s->निवेशय = s->strstart < MIN_मेलनम्-1 ? s->strstart : MIN_मेलनम्-1;
    यदि (flush == Z_FINISH) {
        FLUSH_खण्डम्(s, 1);
        प्रत्यागम finish_समाप्तम्;
    }
    यदि (s->sym_अग्र)
        FLUSH_खण्डम्(s, 0);
    प्रत्यागम खण्डम्_समाप्तम्;
}
#यदि_अन्तः /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local खण्डम्_state deflate_rle(deflate_state *s, पूर्णाङ्क flush) {
    पूर्णाङ्क bflush;             /* set if current block must be flushed */
    uInt पूर्व;              /* byte at distance one to match */
    Bytef *अवलोकनम्, *strend;   /* scan goes up to strend for length of run */

    प्रति (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        यदि (s->lookahead <= MAX_मेलनम्) {
            fill_गवाक्षः(s);
            यदि (s->lookahead <= MAX_मेलनम् && flush == Z_NO_FLUSH) {
                प्रत्यागम need_अधिकम्;
            }
            यदि (s->lookahead == 0) भङ्ग; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->मेलनम्_दीर्घता = 0;
        यदि (s->lookahead >= MIN_मेलनम् && s->strstart > 0) {
            अवलोकनम् = s->गवाक्षः + s->strstart - 1;
            पूर्व = *अवलोकनम्;
            यदि (पूर्व == *++अवलोकनम् && पूर्व == *++अवलोकनम् && पूर्व == *++अवलोकनम्) {
                strend = s->गवाक्षः + s->strstart + MAX_मेलनम्;
                कुरु {
                } यावत् (पूर्व == *++अवलोकनम् && पूर्व == *++अवलोकनम् &&
                         पूर्व == *++अवलोकनम् && पूर्व == *++अवलोकनम् &&
                         पूर्व == *++अवलोकनम् && पूर्व == *++अवलोकनम् &&
                         पूर्व == *++अवलोकनम् && पूर्व == *++अवलोकनम् &&
                         अवलोकनम् < strend);
                s->मेलनम्_दीर्घता = MAX_मेलनम् - (uInt)(strend - अवलोकनम्);
                यदि (s->मेलनम्_दीर्घता > s->lookahead)
                    s->मेलनम्_दीर्घता = s->lookahead;
            }
            Assert(अवलोकनम् <= s->गवाक्षः + (uInt)(s->गवाक्षः_आकार - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        यदि (s->मेलनम्_दीर्घता >= MIN_मेलनम्) {
            check_मेलनम्(s, s->strstart, s->strstart - 1, (पूर्णाङ्क)s->मेलनम्_दीर्घता);

            _tr_tally_dist(s, 1, s->मेलनम्_दीर्घता - MIN_मेलनम्, bflush);

            s->lookahead -= s->मेलनम्_दीर्घता;
            s->strstart += s->मेलनम्_दीर्घता;
            s->मेलनम्_दीर्घता = 0;
        } अन्यथा {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->गवाक्षः[s->strstart]));
            _tr_tally_lit(s, s->गवाक्षः[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        यदि (bflush) FLUSH_खण्डम्(s, 0);
    }
    s->निवेशय = 0;
    यदि (flush == Z_FINISH) {
        FLUSH_खण्डम्(s, 1);
        प्रत्यागम finish_समाप्तम्;
    }
    यदि (s->sym_अग्र)
        FLUSH_खण्डम्(s, 0);
    प्रत्यागम खण्डम्_समाप्तम्;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local खण्डम्_state deflate_huff(deflate_state *s, पूर्णाङ्क flush) {
    पूर्णाङ्क bflush;             /* set if current block must be flushed */

    प्रति (;;) {
        /* Make sure that we have a literal to write. */
        यदि (s->lookahead == 0) {
            fill_गवाक्षः(s);
            यदि (s->lookahead == 0) {
                यदि (flush == Z_NO_FLUSH)
                    प्रत्यागम need_अधिकम्;
                भङ्ग;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->मेलनम्_दीर्घता = 0;
        Tracevv((stderr,"%c", s->गवाक्षः[s->strstart]));
        _tr_tally_lit(s, s->गवाक्षः[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        यदि (bflush) FLUSH_खण्डम्(s, 0);
    }
    s->निवेशय = 0;
    यदि (flush == Z_FINISH) {
        FLUSH_खण्डम्(s, 1);
        प्रत्यागम finish_समाप्तम्;
    }
    यदि (s->sym_अग्र)
        FLUSH_खण्डम्(s, 0);
    प्रत्यागम खण्डम्_समाप्तम्;
}
