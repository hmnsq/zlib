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

#అన్తర్భావ "deflate.h"

స్థిరరాశి అక్షరం deflate_copyright[] =
   " deflate 1.3.2.1 Copyright 1995-2026 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

రకనిర్వచనం గణన {
    need_మరిన్ని,      /* block not completed, need more input or more output */
    ఖండం_పూర్తి,     /* block flush performed */
    ముగింపు_started, /* finish started, need only more output at next deflate */
    ముగింపు_పూర్తి     /* finish done, accept no more input or output */
} ఖండం_state;

రకనిర్వచనం ఖండం_state (*compress_func)(deflate_state *s, పూర్ణాంకం flush);
/* Compression function. Returns the block state after the call. */

local ఖండం_state deflate_stored(deflate_state *s, పూర్ణాంకం flush);
local ఖండం_state deflate_fast(deflate_state *s, పూర్ణాంకం flush);
#నిర్వచితం_కాకపోతే FASTEST
local ఖండం_state deflate_slow(deflate_state *s, పూర్ణాంకం flush);
#అయితే_ముగింపు
local ఖండం_state deflate_rle(deflate_state *s, పూర్ణాంకం flush);
local ఖండం_state deflate_huff(deflate_state *s, పూర్ణాంకం flush);

/* ===========================================================================
 * Local data
 */

#నిర్వచించు NIL 0
/* Tail of hash chains */

#నిర్వచితం_కాకపోతే TOO_FAR
#  define TOO_FAR 4096
#అయితే_ముగింపు
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
రకనిర్వచనం నిర్మాణం config_s {
   ush good_నిడివి; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_నిడివి; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#నిర్వచితమైతే FASTEST
local స్థిరరాశి config configuration_పట్టిక[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#లేకపోతే
local స్థిరరాశి config configuration_పట్టిక[10] = {
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
#అయితే_ముగింపు

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#నిర్వచించు RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#నిర్వచించు UPDATE_HASH(s,h,c) (h = (((h) << s->hash_జరుపు) ^ (c)) & s->hash_ముసుగు)


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
#నిర్వచితమైతే FASTEST
#నిర్వచించు చొప్పించు_STRING(s, str, సరిపోలిక_తల) \
   (UPDATE_HASH(s, s->ins_h, s->కిటికీ[(str) + (MIN_సరిపోలిక-1)]), \
    సరిపోలిక_తల = s->తల[s->ins_h], \
    s->తల[s->ins_h] = (Pos)(str))
#లేకపోతే
#నిర్వచించు చొప్పించు_STRING(s, str, సరిపోలిక_తల) \
   (UPDATE_HASH(s, s->ins_h, s->కిటికీ[(str) + (MIN_సరిపోలిక-1)]), \
    సరిపోలిక_తల = s->prev[(str) & s->w_ముసుగు] = s->తల[s->ins_h], \
    s->తల[s->ins_h] = (Pos)(str))
#అయితే_ముగింపు

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#నిర్వచించు CLEAR_HASH(s) \
    చేయి { \
        s->తల[s->hash_కొలత - 1] = NIL; \
        zmemzero(s->తల, (గుర్తులేని)(s->hash_కొలత - 1)*పరిమాణం(*s->తల)); \
        s->slid = 0; \
    } అయితే (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#ఒకవేళ defined(__has_feature)
#  ఒకవేళ __has_feature(జ్ఞాపకం_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#అయితే_ముగింపు
local శూన్య slide_hash(deflate_state *s) {
    గుర్తులేని n, m;
    Posf *p;
    uInt wsize = s->w_కొలత;

    n = s->hash_కొలత;
    p = &s->తల[n];
    చేయి {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } అయితే (--n);
#నిర్వచితం_కాకపోతే FASTEST
    n = wsize;
    p = &s->prev[n];
    చేయి {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } అయితే (--n);
#అయితే_ముగింపు
    s->slid = 1;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local గుర్తులేని read_buf(z_streamp strm, Bytef *buf, గుర్తులేని కొలత) {
    గుర్తులేని len = strm->avail_లోపల;

    ఒకవేళ (len > కొలత) len = కొలత;
    ఒకవేళ (len == 0) తిరిగి 0;

    strm->avail_లోపల  -= len;

    zmemcpy(buf, strm->తదుపరి_లోపల, len);
    ఒకవేళ (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#నిర్వచితమైతే GZIP
    లేకపోతే ఒకవేళ (strm->state->wrap == 2) {
        strm->adler = చక్రీయ_తనిఖీ_32_2(strm->adler, buf, len);
    }
#అయితే_ముగింపు
    strm->తదుపరి_లోపల  += len;
    strm->మొత్తం_లోపల += len;

    తిరిగి len;
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
local శూన్య fill_కిటికీ(deflate_state *s) {
    గుర్తులేని n;
    గుర్తులేని మరిన్ని;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_కొలత;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    చేయి {
        మరిన్ని = (గుర్తులేని)(s->కిటికీ_కొలత -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
#నిర్వచితమైతే _MSC_VER
#సూచన warning(నెట్టు)
#సూచన warning(disable: 4127)
#అయితే_ముగింపు
        ఒకవేళ (పరిమాణం(పూర్ణాంకం) <= 2) {
#నిర్వచితమైతే _MSC_VER
#సూచన warning(తీయి)
#అయితే_ముగింపు
            ఒకవేళ (మరిన్ని == 0 && s->strstart == 0 && s->lookahead == 0) {
                మరిన్ని = wsize;

            } లేకపోతే ఒకవేళ (మరిన్ని == (గుర్తులేని)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                మరిన్ని--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        ఒకవేళ (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->కిటికీ, s->కిటికీ + wsize, (గుర్తులేని)wsize - మరిన్ని);
            s->సరిపోలిక_ప్రారంభం -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->ఖండం_ప్రారంభం -= (దీర్ఘ) wsize;
            ఒకవేళ (s->చొప్పించు > s->strstart)
                s->చొప్పించు = s->strstart;
            slide_hash(s);
            మరిన్ని += wsize;
        }
        ఒకవేళ (s->strm->avail_లోపల == 0) ఆపు;

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
        Assert(మరిన్ని >= 2, "more < 2");

        n = read_buf(s->strm, s->కిటికీ + s->strstart + s->lookahead, మరిన్ని);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        ఒకవేళ (s->lookahead + s->చొప్పించు >= MIN_సరిపోలిక) {
            uInt str = s->strstart - s->చొప్పించు;
            s->ins_h = s->కిటికీ[str];
            UPDATE_HASH(s, s->ins_h, s->కిటికీ[str + 1]);
#ఒకవేళ MIN_సరిపోలిక != 3
            Call UPDATE_HASH() MIN_సరిపోలిక-3 మరిన్ని times
#అయితే_ముగింపు
            అయితే (s->చొప్పించు) {
                UPDATE_HASH(s, s->ins_h, s->కిటికీ[str + MIN_సరిపోలిక-1]);
#నిర్వచితం_కాకపోతే FASTEST
                s->prev[str & s->w_ముసుగు] = s->తల[s->ins_h];
#అయితే_ముగింపు
                s->తల[s->ins_h] = (Pos)str;
                str++;
                s->చొప్పించు--;
                ఒకవేళ (s->lookahead + s->చొప్పించు < MIN_సరిపోలిక)
                    ఆపు;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } అయితే (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_లోపల != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    ఒకవేళ (s->ఎగువ_water < s->కిటికీ_కొలత) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        ఒకవేళ (s->ఎగువ_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->కిటికీ_కొలత - curr;
            ఒకవేళ (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->కిటికీ + curr, (గుర్తులేని)init);
            s->ఎగువ_water = curr + init;
        }
        లేకపోతే ఒకవేళ (s->ఎగువ_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->ఎగువ_water;
            ఒకవేళ (init > s->కిటికీ_కొలత - s->ఎగువ_water)
                init = s->కిటికీ_కొలత - s->ఎగువ_water;
            zmemzero(s->కిటికీ + s->ఎగువ_water, (గుర్తులేని)init);
            s->ఎగువ_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->కిటికీ_కొలత - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflateInit_(z_streamp strm, పూర్ణాంకం level, స్థిరరాశి అక్షరం *సంస్కరణ,
                         పూర్ణాంకం ప్రవాహం_కొలత) {
    తిరిగి deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, సంస్కరణ, ప్రవాహం_కొలత);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflateInit2_(z_streamp strm, పూర్ణాంకం level, పూర్ణాంకం పద్ధతి,
                          పూర్ణాంకం కిటికీ_బిట్లు, పూర్ణాంకం memLevel, పూర్ణాంకం strategy,
                          స్థిరరాశి అక్షరం *సంస్కరణ, పూర్ణాంకం ప్రవాహం_కొలత) {
    deflate_state *s;
    పూర్ణాంకం wrap = 1;
    స్థిర స్థిరరాశి అక్షరం my_సంస్కరణ[] = ZLIB_సంస్కరణ;

    ఒకవేళ (సంస్కరణ == Z_NULL || సంస్కరణ[0] != my_సంస్కరణ[0] ||
        ప్రవాహం_కొలత != పరిమాణం(z_ప్రవాహం)) {
        తిరిగి Z_సంస్కరణ_ERROR;
    }
    ఒకవేళ (strm == Z_NULL) తిరిగి Z_ప్రవాహం_ERROR;

    strm->సందేశం = Z_NULL;
    ఒకవేళ (strm->zalloc == (alloc_func)0) {
#నిర్వచితమైతే Z_SOLO
        తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#అయితే_ముగింపు
    }
    ఒకవేళ (strm->zfree == (free_func)0)
#నిర్వచితమైతే Z_SOLO
        తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
        strm->zfree = zcfree;
#అయితే_ముగింపు

#నిర్వచితమైతే FASTEST
    ఒకవేళ (level != 0) level = 1;
#లేకపోతే
    ఒకవేళ (level == Z_DEFAULT_COMPRESSION) level = 6;
#అయితే_ముగింపు

    ఒకవేళ (కిటికీ_బిట్లు < 0) { /* suppress zlib wrapper */
        wrap = 0;
        ఒకవేళ (కిటికీ_బిట్లు < -15)
            తిరిగి Z_ప్రవాహం_ERROR;
        కిటికీ_బిట్లు = -కిటికీ_బిట్లు;
    }
#నిర్వచితమైతే GZIP
    లేకపోతే ఒకవేళ (కిటికీ_బిట్లు > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        కిటికీ_బిట్లు -= 16;
    }
#అయితే_ముగింపు
    ఒకవేళ (memLevel < 1 || memLevel > MAX_MEM_LEVEL || పద్ధతి != Z_DEFLATED ||
        కిటికీ_బిట్లు < 8 || కిటికీ_బిట్లు > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (కిటికీ_బిట్లు == 8 && wrap != 1)) {
        తిరిగి Z_ప్రవాహం_ERROR;
    }
    ఒకవేళ (కిటికీ_బిట్లు == 8) కిటికీ_బిట్లు = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, పరిమాణం(deflate_state));
    ఒకవేళ (s == Z_NULL) తిరిగి Z_MEM_ERROR;
    zmemzero(s, పరిమాణం(deflate_state));
    strm->state = (నిర్మాణం అంతర్గత_state FAR *)s;
    s->strm = strm;
    s->స్థితి_2 = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->w_బిట్లు = (uInt)కిటికీ_బిట్లు;
    s->w_కొలత = 1 << s->w_బిట్లు;
    s->w_ముసుగు = s->w_కొలత - 1;

    s->hash_బిట్లు = (uInt)memLevel + 7;
    s->hash_కొలత = 1 << s->hash_బిట్లు;
    s->hash_ముసుగు = s->hash_కొలత - 1;
    s->hash_జరుపు =  ((s->hash_బిట్లు + MIN_సరిపోలిక-1) / MIN_సరిపోలిక);

    s->కిటికీ = (Bytef *) ZALLOC(strm, s->w_కొలత, 2*పరిమాణం(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_కొలత, పరిమాణం(Pos));
    s->తల   = (Posf *)  ZALLOC(strm, s->hash_కొలత, పరిమాణం(Pos));

    s->ఎగువ_water = 0;      /* nothing written to s->window yet */

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
    s->pending_buf_కొలత = (ulg)s->lit_bufsize * 4;

    ఒకవేళ (s->కిటికీ == Z_NULL || s->prev == Z_NULL || s->తల == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->స్థితి_2 = ముగింపు_STATE;
        strm->సందేశం = ERR_సందేశం(Z_MEM_ERROR);
        deflate_చివర (strm);
        తిరిగి Z_MEM_ERROR;
    }
#నిర్వచితమైతే LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_చివర = s->lit_bufsize - 1;
#లేకపోతే
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_చివర = (s->lit_bufsize - 1) * 3;
#అయితే_ముగింపు
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->పద్ధతి = (Byte)పద్ధతి;

    తిరిగి deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local పూర్ణాంకం deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    ఒకవేళ (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        తిరిగి 1;
    s = strm->state;
    ఒకవేళ (s == Z_NULL || s->strm != strm || (s->స్థితి_2 != INIT_STATE &&
#నిర్వచితమైతే GZIP
                                           s->స్థితి_2 != GZIP_STATE &&
#అయితే_ముగింపు
                                           s->స్థితి_2 != EXTRA_STATE &&
                                           s->స్థితి_2 != NAME_STATE &&
                                           s->స్థితి_2 != COMMENT_STATE &&
                                           s->స్థితి_2 != HCRC_STATE &&
                                           s->స్థితి_2 != BUSY_STATE &&
                                           s->స్థితి_2 != ముగింపు_STATE))
        తిరిగి 1;
    తిరిగి 0;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflate_సమితి_Dictionary(z_streamp strm, స్థిరరాశి Bytef *dictionary,
                                 uInt  dict_నిడివి) {
    deflate_state *s;
    uInt str, n;
    పూర్ణాంకం wrap;
    గుర్తులేని avail;
    z_const గుర్తులేని అక్షరం *తదుపరి;

    ఒకవేళ (deflateStateCheck(strm) || dictionary == Z_NULL)
        తిరిగి Z_ప్రవాహం_ERROR;
    s = strm->state;
    wrap = s->wrap;
    ఒకవేళ (wrap == 2 || (wrap == 1 && s->స్థితి_2 != INIT_STATE) || s->lookahead)
        తిరిగి Z_ప్రవాహం_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    ఒకవేళ (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dict_నిడివి);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    ఒకవేళ (dict_నిడివి >= s->w_కొలత) {
        ఒకవేళ (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->ఖండం_ప్రారంభం = 0L;
            s->చొప్పించు = 0;
        }
        dictionary += dict_నిడివి - s->w_కొలత;  /* use the tail */
        dict_నిడివి = s->w_కొలత;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_లోపల;
    తదుపరి = strm->తదుపరి_లోపల;
    strm->avail_లోపల = dict_నిడివి;
    strm->తదుపరి_లోపల = (z_const Bytef *)dictionary;
    fill_కిటికీ(s);
    అయితే (s->lookahead >= MIN_సరిపోలిక) {
        str = s->strstart;
        n = s->lookahead - (MIN_సరిపోలిక-1);
        చేయి {
            UPDATE_HASH(s, s->ins_h, s->కిటికీ[str + MIN_సరిపోలిక-1]);
#నిర్వచితం_కాకపోతే FASTEST
            s->prev[str & s->w_ముసుగు] = s->తల[s->ins_h];
#అయితే_ముగింపు
            s->తల[s->ins_h] = (Pos)str;
            str++;
        } అయితే (--n);
        s->strstart = str;
        s->lookahead = MIN_సరిపోలిక-1;
        fill_కిటికీ(s);
    }
    s->strstart += s->lookahead;
    s->ఖండం_ప్రారంభం = (దీర్ఘ)s->strstart;
    s->చొప్పించు = s->lookahead;
    s->lookahead = 0;
    s->సరిపోలిక_నిడివి = s->prev_నిడివి = MIN_సరిపోలిక-1;
    s->సరిపోలిక_అందుబాటు = 0;
    strm->తదుపరి_లోపల = తదుపరి;
    strm->avail_లోపల = avail;
    s->wrap = wrap;
    తిరిగి Z_OK;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dict_నిడివి) {
    deflate_state *s;
    uInt len;

    ఒకవేళ (deflateStateCheck(strm))
        తిరిగి Z_ప్రవాహం_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    ఒకవేళ (len > s->w_కొలత)
        len = s->w_కొలత;
    ఒకవేళ (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->కిటికీ + s->strstart + s->lookahead - len, len);
    ఒకవేళ (dict_నిడివి != Z_NULL)
        *dict_నిడివి = len;
    తిరిగి Z_OK;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    ఒకవేళ (deflateStateCheck(strm)) {
        తిరిగి Z_ప్రవాహం_ERROR;
    }

    strm->మొత్తం_లోపల = strm->మొత్తం_బహిర్గతం = 0;
    strm->సందేశం = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->దత్తాంశం_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_బహిర్గతం = s->pending_buf;

    ఒకవేళ (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->స్థితి_2 =
#నిర్వచితమైతే GZIP
        s->wrap == 2 ? GZIP_STATE :
#అయితే_ముగింపు
        INIT_STATE;
    strm->adler =
#నిర్వచితమైతే GZIP
        s->wrap == 2 ? చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0) :
#అయితే_ముగింపు
        adler32(0L, Z_NULL, 0);
    s->చివరి_flush = -2;

    s->gzhead = Z_NULL;

    _tr_init(s);

    తిరిగి Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local శూన్య lm_init(deflate_state *s) {
    s->కిటికీ_కొలత = (ulg)2L*s->w_కొలత;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_సరిపోలిక   = configuration_పట్టిక[s->level].max_lazy;
    s->good_సరిపోలిక       = configuration_పట్టిక[s->level].good_నిడివి;
    s->nice_సరిపోలిక       = configuration_పట్టిక[s->level].nice_నిడివి;
    s->max_chain_నిడివి = configuration_పట్టిక[s->level].max_chain;

    s->strstart = 0;
    s->ఖండం_ప్రారంభం = 0L;
    s->lookahead = 0;
    s->చొప్పించు = 0;
    s->సరిపోలిక_నిడివి = s->prev_నిడివి = MIN_సరిపోలిక-1;
    s->సరిపోలిక_అందుబాటు = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflateReset(z_streamp strm) {
    పూర్ణాంకం ret;

    ret = deflateResetKeep(strm);
    ఒకవేళ (ret == Z_OK)
        lm_init(strm->state);
    తిరిగి ret;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflate_సమితి_శీర్షిక(z_streamp strm, gz_headerp తల) {
    ఒకవేళ (deflateStateCheck(strm) || strm->state->wrap != 2)
        తిరిగి Z_ప్రవాహం_ERROR;
    strm->state->gzhead = తల;
    తిరిగి Z_OK;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflatePending(z_streamp strm, గుర్తులేని *pending, పూర్ణాంకం *బిట్లు) {
    ఒకవేళ (deflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    ఒకవేళ (బిట్లు != Z_NULL)
        *బిట్లు = strm->state->bi_valid;
    ఒకవేళ (pending != Z_NULL) {
        *pending = (గుర్తులేని)strm->state->pending;
        ఒకవేళ (*pending != strm->state->pending) {
            *pending = (గుర్తులేని)-1;
            తిరిగి Z_BUF_ERROR;
        }
    }
    తిరిగి Z_OK;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflate_వాడిన(z_streamp strm, పూర్ణాంకం *బిట్లు) {
    ఒకవేళ (deflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    ఒకవేళ (బిట్లు != Z_NULL)
        *బిట్లు = strm->state->bi_వాడిన;
    తిరిగి Z_OK;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflate_ప్రధానసంఖ్య(z_streamp strm, పూర్ణాంకం బిట్లు, పూర్ణాంకం విలువ) {
    deflate_state *s;
    పూర్ణాంకం put;

    ఒకవేళ (deflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    s = strm->state;
#నిర్వచితమైతే LIT_MEM
    ఒకవేళ (బిట్లు < 0 || బిట్లు > 16 ||
        (uchf *)s->d_buf < s->pending_బహిర్గతం + ((Buf_కొలత + 7) >> 3))
        తిరిగి Z_BUF_ERROR;
#లేకపోతే
    ఒకవేళ (బిట్లు < 0 || బిట్లు > 16 ||
        s->sym_buf < s->pending_బహిర్గతం + ((Buf_కొలత + 7) >> 3))
        తిరిగి Z_BUF_ERROR;
#అయితే_ముగింపు
    చేయి {
        put = Buf_కొలత - s->bi_valid;
        ఒకవేళ (put > బిట్లు)
            put = బిట్లు;
        s->bi_buf |= (ush)((విలువ & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        tr_flush_బిట్లు(s);
        విలువ >>= put;
        బిట్లు -= put;
    } అయితే (బిట్లు);
    తిరిగి Z_OK;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflateParams(z_streamp strm, పూర్ణాంకం level, పూర్ణాంకం strategy) {
    deflate_state *s;
    compress_func func;

    ఒకవేళ (deflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    s = strm->state;

#నిర్వచితమైతే FASTEST
    ఒకవేళ (level != 0) level = 1;
#లేకపోతే
    ఒకవేళ (level == Z_DEFAULT_COMPRESSION) level = 6;
#అయితే_ముగింపు
    ఒకవేళ (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        తిరిగి Z_ప్రవాహం_ERROR;
    }
    func = configuration_పట్టిక[s->level].func;

    ఒకవేళ ((strategy != s->strategy || func != configuration_పట్టిక[level].func) &&
        s->చివరి_flush != -2) {
        /* Flush the last buffer: */
        పూర్ణాంకం err = deflate(strm, Z_ఖండం);
        ఒకవేళ (err == Z_ప్రవాహం_ERROR)
            తిరిగి err;
        ఒకవేళ (strm->avail_లోపల || (s->strstart - s->ఖండం_ప్రారంభం) + s->lookahead)
            తిరిగి Z_BUF_ERROR;
    }
    ఒకవేళ (s->level != level) {
        ఒకవేళ (s->level == 0 && s->matches != 0) {
            ఒకవేళ (s->matches == 1)
                slide_hash(s);
            లేకపోతే
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_సరిపోలిక   = configuration_పట్టిక[level].max_lazy;
        s->good_సరిపోలిక       = configuration_పట్టిక[level].good_నిడివి;
        s->nice_సరిపోలిక       = configuration_పట్టిక[level].nice_నిడివి;
        s->max_chain_నిడివి = configuration_పట్టిక[level].max_chain;
    }
    s->strategy = strategy;
    తిరిగి Z_OK;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflateTune(z_streamp strm, పూర్ణాంకం good_నిడివి, పూర్ణాంకం max_lazy,
                        పూర్ణాంకం nice_నిడివి, పూర్ణాంకం max_chain) {
    deflate_state *s;

    ఒకవేళ (deflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;
    s = strm->state;
    s->good_సరిపోలిక = (uInt)good_నిడివి;
    s->max_lazy_సరిపోలిక = (uInt)max_lazy;
    s->nice_సరిపోలిక = nice_నిడివి;
    s->max_chain_నిడివి = (uInt)max_chain;
    తిరిగి Z_OK;
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
z_కొలత_t ZEXPORT deflateBound_z(z_streamp strm, z_కొలత_t ఆధారం_Len) {
    deflate_state *s;
    z_కొలత_t fixedlen, storelen, wraplen, bound;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = ఆధారం_Len + (ఆధారం_Len >> 3) + (ఆధారం_Len >> 8) +
               (ఆధారం_Len >> 9) + 4;
    ఒకవేళ (fixedlen < ఆధారం_Len)
        fixedlen = (z_కొలత_t)-1;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = ఆధారం_Len + (ఆధారం_Len >> 5) + (ఆధారం_Len >> 7) +
               (ఆధారం_Len >> 11) + 7;
    ఒకవేళ (storelen < ఆధారం_Len)
        storelen = (z_కొలత_t)-1;

    /* if can't get parameters, return larger bound plus a wrapper */
    ఒకవేళ (deflateStateCheck(strm)) {
        bound = fixedlen > storelen ? fixedlen : storelen;
        తిరిగి bound + 18 < bound ? (z_కొలత_t)-1 : bound + 18;
    }

    /* compute wrapper length */
    s = strm->state;
    ఎంపిక (s->wrap < 0 ? -s->wrap : s->wrap) {
    స్థితి 0:                                 /* raw deflate */
        wraplen = 0;
        ఆపు;
    స్థితి 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        ఆపు;
#నిర్వచితమైతే GZIP
    స్థితి 2:                                 /* gzip wrapper */
        wraplen = 18;
        ఒకవేళ (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            ఒకవేళ (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            ఒకవేళ (str != Z_NULL)
                చేయి {
                    wraplen++;
                } అయితే (*str++);
            str = s->gzhead->comment;
            ఒకవేళ (str != Z_NULL)
                చేయి {
                    wraplen++;
                } అయితే (*str++);
            ఒకవేళ (s->gzhead->hcrc)
                wraplen += 2;
        }
        ఆపు;
#అయితే_ముగింపు
    అప్రమేయ:                                /* for compiler happiness */
        wraplen = 18;
    }

    /* if not default parameters, return one of the conservative bounds */
    ఒకవేళ (s->w_బిట్లు != 15 || s->hash_బిట్లు != 8 + 7) {
        bound = s->w_బిట్లు <= s->hash_బిట్లు && s->level ? fixedlen :
                                                        storelen;
        తిరిగి bound + wraplen < bound ? (z_కొలత_t)-1 : bound + wraplen;
    }

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    bound = ఆధారం_Len + (ఆధారం_Len >> 12) + (ఆధారం_Len >> 14) +
            (ఆధారం_Len >> 25) + 13 - 6 + wraplen;
    తిరిగి bound < ఆధారం_Len ? (z_కొలత_t)-1 : bound;
}
uLong ZEXPORT deflateBound(z_streamp strm, uLong ఆధారం_Len) {
    z_కొలత_t bound = deflateBound_z(strm, ఆధారం_Len);
    తిరిగి (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local శూన్య putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local శూన్య flush_pending(z_streamp strm) {
    గుర్తులేని len;
    deflate_state *s = strm->state;

    tr_flush_బిట్లు(s);
    len = s->pending > strm->avail_బహిర్గతం ? strm->avail_బహిర్గతం :
                                         (గుర్తులేని)s->pending;
    ఒకవేళ (len == 0) తిరిగి;

    zmemcpy(strm->తదుపరి_బహిర్గతం, s->pending_బహిర్గతం, len);
    strm->తదుపరి_బహిర్గతం  += len;
    s->pending_బహిర్గతం  += len;
    strm->మొత్తం_బహిర్గతం += len;
    strm->avail_బహిర్గతం -= len;
    s->pending      -= len;
    ఒకవేళ (s->pending == 0) {
        s->pending_బహిర్గతం = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#నిర్వచించు HCRC_UPDATE(beg) \
    చేయి { \
        ఒకవేళ (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = చక్రీయ_తనిఖీ_32_z(strm->adler, s->pending_buf + (beg), \
                                  s->pending - (beg)); \
    } అయితే (0)

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflate(z_streamp strm, పూర్ణాంకం flush) {
    పూర్ణాంకం old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    ఒకవేళ (deflateStateCheck(strm) || flush > Z_ఖండం || flush < 0) {
        తిరిగి Z_ప్రవాహం_ERROR;
    }
    s = strm->state;

    ఒకవేళ (strm->తదుపరి_బహిర్గతం == Z_NULL ||
        (strm->avail_లోపల != 0 && strm->తదుపరి_లోపల == Z_NULL) ||
        (s->స్థితి_2 == ముగింపు_STATE && flush != Z_ముగింపు)) {
        ERR_RETURN(strm, Z_ప్రవాహం_ERROR);
    }
    ఒకవేళ (strm->avail_బహిర్గతం == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->చివరి_flush;
    s->చివరి_flush = flush;

    /* Flush as much pending output as possible */
    ఒకవేళ (s->pending != 0) {
        flush_pending(strm);
        ఒకవేళ (strm->avail_బహిర్గతం == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->చివరి_flush = -1;
            తిరిగి Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } లేకపోతే ఒకవేళ (strm->avail_లోపల == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_ముగింపు) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    ఒకవేళ (s->స్థితి_2 == ముగింపు_STATE && strm->avail_లోపల != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    ఒకవేళ (s->స్థితి_2 == INIT_STATE && s->wrap == 0)
        s->స్థితి_2 = BUSY_STATE;
    ఒకవేళ (s->స్థితి_2 == INIT_STATE) {
        /* zlib header */
        uInt శీర్షిక = (Z_DEFLATED + ((s->w_బిట్లు - 8) << 4)) << 8;
        uInt level_flags;

        ఒకవేళ (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        లేకపోతే ఒకవేళ (s->level < 6)
            level_flags = 1;
        లేకపోతే ఒకవేళ (s->level == 6)
            level_flags = 2;
        లేకపోతే
            level_flags = 3;
        శీర్షిక |= (level_flags << 6);
        ఒకవేళ (s->strstart != 0) శీర్షిక |= PRESET_DICT;
        శీర్షిక += 31 - (శీర్షిక % 31);

        putShortMSB(s, శీర్షిక);

        /* Save the adler32 of the preset dictionary: */
        ఒకవేళ (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->స్థితి_2 = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        ఒకవేళ (s->pending != 0) {
            s->చివరి_flush = -1;
            తిరిగి Z_OK;
        }
    }
#నిర్వచితమైతే GZIP
    ఒకవేళ (s->స్థితి_2 == GZIP_STATE) {
        /* gzip header */
        strm->adler = చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        ఒకవేళ (s->gzhead == Z_NULL) {
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, OS_సంకేతం);
            s->స్థితి_2 = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            ఒకవేళ (s->pending != 0) {
                s->చివరి_flush = -1;
                తిరిగి Z_OK;
            }
        }
        లేకపోతే {
            put_byte(s, (s->gzhead->పాఠ్యం ? 1 : 0) +
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
            ఒకవేళ (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            ఒకవేళ (s->gzhead->hcrc)
                strm->adler = చక్రీయ_తనిఖీ_32_z(strm->adler, s->pending_buf,
                                      s->pending);
            s->gzindex = 0;
            s->స్థితి_2 = EXTRA_STATE;
        }
    }
    ఒకవేళ (s->స్థితి_2 == EXTRA_STATE) {
        ఒకవేళ (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            ulg ఎడమ = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            అయితే (s->pending + ఎడమ > s->pending_buf_కొలత) {
                ulg copy = s->pending_buf_కొలత - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_కొలత;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                ఒకవేళ (s->pending != 0) {
                    s->చివరి_flush = -1;
                    తిరిగి Z_OK;
                }
                beg = 0;
                ఎడమ -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, ఎడమ);
            s->pending += ఎడమ;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->స్థితి_2 = NAME_STATE;
    }
    ఒకవేళ (s->స్థితి_2 == NAME_STATE) {
        ఒకవేళ (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            పూర్ణాంకం val;
            చేయి {
                ఒకవేళ (s->pending == s->pending_buf_కొలత) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    ఒకవేళ (s->pending != 0) {
                        s->చివరి_flush = -1;
                        తిరిగి Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } అయితే (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->స్థితి_2 = COMMENT_STATE;
    }
    ఒకవేళ (s->స్థితి_2 == COMMENT_STATE) {
        ఒకవేళ (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            పూర్ణాంకం val;
            చేయి {
                ఒకవేళ (s->pending == s->pending_buf_కొలత) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    ఒకవేళ (s->pending != 0) {
                        s->చివరి_flush = -1;
                        తిరిగి Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } అయితే (val != 0);
            HCRC_UPDATE(beg);
        }
        s->స్థితి_2 = HCRC_STATE;
    }
    ఒకవేళ (s->స్థితి_2 == HCRC_STATE) {
        ఒకవేళ (s->gzhead->hcrc) {
            ఒకవేళ (s->pending + 2 > s->pending_buf_కొలత) {
                flush_pending(strm);
                ఒకవేళ (s->pending != 0) {
                    s->చివరి_flush = -1;
                    తిరిగి Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0);
        }
        s->స్థితి_2 = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        ఒకవేళ (s->pending != 0) {
            s->చివరి_flush = -1;
            తిరిగి Z_OK;
        }
    }
#అయితే_ముగింపు

    /* Start a new block or continue the current one.
     */
    ఒకవేళ (strm->avail_లోపల != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->స్థితి_2 != ముగింపు_STATE)) {
        ఖండం_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_పట్టిక[s->level].func))(s, flush);

        ఒకవేళ (bstate == ముగింపు_started || bstate == ముగింపు_పూర్తి) {
            s->స్థితి_2 = ముగింపు_STATE;
        }
        ఒకవేళ (bstate == need_మరిన్ని || bstate == ముగింపు_started) {
            ఒకవేళ (strm->avail_బహిర్గతం == 0) {
                s->చివరి_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            తిరిగి Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        ఒకవేళ (bstate == ఖండం_పూర్తి) {
            ఒకవేళ (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } లేకపోతే ఒకవేళ (flush != Z_ఖండం) { /* FULL_FLUSH or SYNC_FLUSH */
                tr_stored_ఖండం(s, (అక్షరం*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                ఒకవేళ (flush == Z_నిండిన_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    ఒకవేళ (s->lookahead == 0) {
                        s->strstart = 0;
                        s->ఖండం_ప్రారంభం = 0L;
                        s->చొప్పించు = 0;
                    }
                }
            }
            flush_pending(strm);
            ఒకవేళ (strm->avail_బహిర్గతం == 0) {
              s->చివరి_flush = -1; /* avoid BUF_ERROR at next call, see above */
              తిరిగి Z_OK;
            }
        }
    }

    ఒకవేళ (flush != Z_ముగింపు) తిరిగి Z_OK;
    ఒకవేళ (s->wrap <= 0) తిరిగి Z_ప్రవాహం_చివర;

    /* Write the trailer */
#నిర్వచితమైతే GZIP
    ఒకవేళ (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->మొత్తం_లోపల & 0xff));
        put_byte(s, (Byte)((strm->మొత్తం_లోపల >> 8) & 0xff));
        put_byte(s, (Byte)((strm->మొత్తం_లోపల >> 16) & 0xff));
        put_byte(s, (Byte)((strm->మొత్తం_లోపల >> 24) & 0xff));
    }
    లేకపోతే
#అయితే_ముగింపు
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    ఒకవేళ (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    తిరిగి s->pending != 0 ? Z_OK : Z_ప్రవాహం_చివర;
}

/* ========================================================================= */
పూర్ణాంకం ZEXPORT deflate_చివర(z_streamp strm) {
    పూర్ణాంకం స్థితి_2;

    ఒకవేళ (deflateStateCheck(strm)) తిరిగి Z_ప్రవాహం_ERROR;

    స్థితి_2 = strm->state->స్థితి_2;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->తల);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->కిటికీ);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    తిరిగి స్థితి_2 == BUSY_STATE ? Z_దత్తాంశం_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
పూర్ణాంకం ZEXPORT deflateCopy(z_streamp dest, z_streamp ఆధారం) {
#నిర్వచితమైతే MAXSEG_64K
    (శూన్య)dest;
    (శూన్య)ఆధారం;
    తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
    deflate_state *ds;
    deflate_state *ss;


    ఒకవేళ (deflateStateCheck(ఆధారం) || dest == Z_NULL) {
        తిరిగి Z_ప్రవాహం_ERROR;
    }

    ss = ఆధారం->state;

    zmemcpy(dest, ఆధారం, పరిమాణం(z_ప్రవాహం));

    ds = (deflate_state *) ZALLOC(dest, 1, పరిమాణం(deflate_state));
    ఒకవేళ (ds == Z_NULL) తిరిగి Z_MEM_ERROR;
    zmemzero(ds, పరిమాణం(deflate_state));
    dest->state = (నిర్మాణం అంతర్గత_state FAR *) ds;
    zmemcpy(ds, ss, పరిమాణం(deflate_state));
    ds->strm = dest;

    ds->కిటికీ = (Bytef *) ZALLOC(dest, ds->w_కొలత, 2*పరిమాణం(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_కొలత, పరిమాణం(Pos));
    ds->తల   = (Posf *)  ZALLOC(dest, ds->hash_కొలత, పరిమాణం(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, LIT_BUFS);

    ఒకవేళ (ds->కిటికీ == Z_NULL || ds->prev == Z_NULL || ds->తల == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflate_చివర (dest);
        తిరిగి Z_MEM_ERROR;
    }
    /* following zmemcpy's do not work for 16-bit MSDOS */
    zmemcpy(ds->కిటికీ, ss->కిటికీ, ss->ఎగువ_water);
    zmemcpy(ds->prev, ss->prev,
            (ss->slid || ss->strstart - ss->చొప్పించు > ds->w_కొలత ? ds->w_కొలత :
                ss->strstart - ss->చొప్పించు) * పరిమాణం(Pos));
    zmemcpy(ds->తల, ss->తల, ds->hash_కొలత * పరిమాణం(Pos));

    ds->pending_బహిర్గతం = ds->pending_buf + (ss->pending_బహిర్గతం - ss->pending_buf);
    zmemcpy(ds->pending_బహిర్గతం, ss->pending_బహిర్గతం, ss->pending);
#నిర్వచితమైతే LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
    zmemcpy(ds->d_buf, ss->d_buf, ss->sym_తదుపరి * పరిమాణం(ush));
    zmemcpy(ds->l_buf, ss->l_buf, ss->sym_తదుపరి);
#లేకపోతే
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
    zmemcpy(ds->sym_buf, ss->sym_buf, ss->sym_తదుపరి);
#అయితే_ముగింపు

    ds->l_desc.dyn_వృక్షం = ds->dyn_ltree;
    ds->d_desc.dyn_వృక్షం = ds->dyn_dtree;
    ds->bl_desc.dyn_వృక్షం = ds->bl_వృక్షం;

    తిరిగి Z_OK;
#అయితే_ముగింపు /* MAXSEG_64K */
}

#నిర్వచితం_కాకపోతే FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_సరిపోలిక(deflate_state *s, IPos cur_సరిపోలిక) {
    గుర్తులేని chain_నిడివి = s->max_chain_నిడివి;/* max hash chain length */
    Bytef *పరిశీలన = s->కిటికీ + s->strstart;      /* current string */
    Bytef *సరిపోలిక;                               /* matched string */
    పూర్ణాంకం len;                                    /* length of current match */
    పూర్ణాంకం ఉత్తమ_len = (పూర్ణాంకం)s->prev_నిడివి;         /* best match length so far */
    పూర్ణాంకం nice_సరిపోలిక = s->nice_సరిపోలిక;             /* stop if match long enough */
    IPos హద్దు = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_ముసుగు;

#నిర్వచితమైతే UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    Bytef *strend = s->కిటికీ + s->strstart + MAX_సరిపోలిక - 1;
    ush పరిశీలన_ప్రారంభం = *(ushf*)పరిశీలన;
    ush పరిశీలన_చివర   = *(ushf*)(పరిశీలన + ఉత్తమ_len - 1);
#లేకపోతే
    Bytef *strend = s->కిటికీ + s->strstart + MAX_సరిపోలిక;
    Byte పరిశీలన_end1  = పరిశీలన[ఉత్తమ_len - 1];
    Byte పరిశీలన_చివర   = పరిశీలన[ఉత్తమ_len];
#అయితే_ముగింపు

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_బిట్లు >= 8 && MAX_సరిపోలిక == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    ఒకవేళ (s->prev_నిడివి >= s->good_సరిపోలిక) {
        chain_నిడివి >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    ఒకవేళ ((uInt)nice_సరిపోలిక > s->lookahead) nice_సరిపోలిక = (పూర్ణాంకం)s->lookahead;

    Assert((ulg)s->strstart <= s->కిటికీ_కొలత - MIN_LOOKAHEAD,
           "need lookahead");

    చేయి {
        Assert(cur_సరిపోలిక < s->strstart, "no future");
        సరిపోలిక = s->కిటికీ + cur_సరిపోలిక;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#ఒకవేళ (defined(UNALIGNED_OK) && MAX_సరిపోలిక == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        ఒకవేళ (*(ushf*)(సరిపోలిక + ఉత్తమ_len - 1) != పరిశీలన_చివర ||
            *(ushf*)సరిపోలిక != పరిశీలన_ప్రారంభం) కొనసాగు;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(పరిశీలన[2] == సరిపోలిక[2], "scan[2]?");
        పరిశీలన++, సరిపోలిక++;
        చేయి {
        } అయితే (*(ushf*)(పరిశీలన += 2) == *(ushf*)(సరిపోలిక += 2) &&
                 *(ushf*)(పరిశీలన += 2) == *(ushf*)(సరిపోలిక += 2) &&
                 *(ushf*)(పరిశీలన += 2) == *(ushf*)(సరిపోలిక += 2) &&
                 *(ushf*)(పరిశీలన += 2) == *(ushf*)(సరిపోలిక += 2) &&
                 పరిశీలన < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(పరిశీలన <= s->కిటికీ + (గుర్తులేని)(s->కిటికీ_కొలత - 1),
               "wild scan");
        ఒకవేళ (*పరిశీలన == *సరిపోలిక) పరిశీలన++;

        len = (MAX_సరిపోలిక - 1) - (పూర్ణాంకం)(strend - పరిశీలన);
        పరిశీలన = strend - (MAX_సరిపోలిక-1);

#లేకపోతే /* UNALIGNED_OK */

        ఒకవేళ (సరిపోలిక[ఉత్తమ_len]     != పరిశీలన_చివర  ||
            సరిపోలిక[ఉత్తమ_len - 1] != పరిశీలన_end1 ||
            *సరిపోలిక              != *పరిశీలన     ||
            *++సరిపోలిక            != పరిశీలన[1])      కొనసాగు;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        పరిశీలన += 2, సరిపోలిక++;
        Assert(*పరిశీలన == *సరిపోలిక, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        చేయి {
        } అయితే (*++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
                 *++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
                 *++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
                 *++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
                 పరిశీలన < strend);

        Assert(పరిశీలన <= s->కిటికీ + (గుర్తులేని)(s->కిటికీ_కొలత - 1),
               "wild scan");

        len = MAX_సరిపోలిక - (పూర్ణాంకం)(strend - పరిశీలన);
        పరిశీలన = strend - MAX_సరిపోలిక;

#అయితే_ముగింపు /* UNALIGNED_OK */

        ఒకవేళ (len > ఉత్తమ_len) {
            s->సరిపోలిక_ప్రారంభం = cur_సరిపోలిక;
            ఉత్తమ_len = len;
            ఒకవేళ (len >= nice_సరిపోలిక) ఆపు;
#నిర్వచితమైతే UNALIGNED_OK
            పరిశీలన_చివర = *(ushf*)(పరిశీలన + ఉత్తమ_len - 1);
#లేకపోతే
            పరిశీలన_end1  = పరిశీలన[ఉత్తమ_len - 1];
            పరిశీలన_చివర   = పరిశీలన[ఉత్తమ_len];
#అయితే_ముగింపు
        }
    } అయితే ((cur_సరిపోలిక = prev[cur_సరిపోలిక & wmask]) > హద్దు
             && --chain_నిడివి != 0);

    ఒకవేళ ((uInt)ఉత్తమ_len <= s->lookahead) తిరిగి (uInt)ఉత్తమ_len;
    తిరిగి s->lookahead;
}

#లేకపోతే /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_సరిపోలిక(deflate_state *s, IPos cur_సరిపోలిక) {
    Bytef *పరిశీలన = s->కిటికీ + s->strstart;      /* current string */
    Bytef *సరిపోలిక;                               /* matched string */
    పూర్ణాంకం len;                                    /* length of current match */
    Bytef *strend = s->కిటికీ + s->strstart + MAX_సరిపోలిక;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_బిట్లు >= 8 && MAX_సరిపోలిక == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->కిటికీ_కొలత - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_సరిపోలిక < s->strstart, "no future");

    సరిపోలిక = s->కిటికీ + cur_సరిపోలిక;

    /* Return failure if the match length is less than 2:
     */
    ఒకవేళ (సరిపోలిక[0] != పరిశీలన[0] || సరిపోలిక[1] != పరిశీలన[1]) తిరిగి MIN_సరిపోలిక-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    పరిశీలన += 2, సరిపోలిక += 2;
    Assert(*పరిశీలన == *సరిపోలిక, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    చేయి {
    } అయితే (*++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
             *++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
             *++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
             *++పరిశీలన == *++సరిపోలిక && *++పరిశీలన == *++సరిపోలిక &&
             పరిశీలన < strend);

    Assert(పరిశీలన <= s->కిటికీ + (గుర్తులేని)(s->కిటికీ_కొలత - 1), "wild scan");

    len = MAX_సరిపోలిక - (పూర్ణాంకం)(strend - పరిశీలన);

    ఒకవేళ (len < MIN_సరిపోలిక) తిరిగి MIN_సరిపోలిక - 1;

    s->సరిపోలిక_ప్రారంభం = cur_సరిపోలిక;
    తిరిగి (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#అయితే_ముగింపు /* FASTEST */

#నిర్వచితమైతే ZLIB_DEBUG

#నిర్వచించు EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local శూన్య check_సరిపోలిక(deflate_state *s, IPos ప్రారంభం, IPos సరిపోలిక, పూర్ణాంకం నిడివి) {
    /* check that the match is indeed a match */
    Bytef *వెనుక = s->కిటికీ + (పూర్ణాంకం)సరిపోలిక, *ఇక్కడ = s->కిటికీ + ప్రారంభం;
    IPos len = (IPos)నిడివి;
    ఒకవేళ (సరిపోలిక == (IPos)-1) {
        /* match starts one byte before the current window -- just compare the
           subsequent length-1 bytes */
        వెనుక++;
        ఇక్కడ++;
        len--;
    }
    ఒకవేళ (zmemcmp(వెనుక, ఇక్కడ, len) != EQUAL) {
        fprintf(stderr, " start %u, match %d, length %d\n",
                ప్రారంభం, (పూర్ణాంకం)సరిపోలిక, నిడివి);
        చేయి {
            fprintf(stderr, "(%02x %02x)", *వెనుక++, *ఇక్కడ++);
        } అయితే (--len != 0);
        z_error("invalid match");
    }
    ఒకవేళ (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", ప్రారంభం - సరిపోలిక, నిడివి);
        చేయి { putc(s->కిటికీ[ప్రారంభం++], stderr); } అయితే (--నిడివి != 0);
    }
}
#లేకపోతే
#  define check_సరిపోలిక(s, ప్రారంభం, సరిపోలిక, నిడివి)
#అయితే_ముగింపు /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#నిర్వచించు FLUSH_ఖండం_ONLY(s, చివరి) { \
   tr_flush_ఖండం(s, (s->ఖండం_ప్రారంభం >= 0L ? \
                   (charf *)&s->కిటికీ[(గుర్తులేని)s->ఖండం_ప్రారంభం] : \
                   (charf *)Z_NULL), \
                (ulg)((దీర్ఘ)s->strstart - s->ఖండం_ప్రారంభం), \
                (చివరి)); \
   s->ఖండం_ప్రారంభం = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#నిర్వచించు FLUSH_ఖండం(s, చివరి) { \
   FLUSH_ఖండం_ONLY(s, చివరి); \
   ఒకవేళ (s->strm->avail_బహిర్గతం == 0) తిరిగి (చివరి) ? ముగింపు_started : need_మరిన్ని; \
}

/* Maximum stored block length in deflate format (not including header). */
#నిర్వచించు MAX_STORED 65535

/* Minimum of a and b. */
#నిర్వచించు MIN(a, b) ((a) > (b) ? (b) : (a))

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
local ఖండం_state deflate_stored(deflate_state *s, పూర్ణాంకం flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    గుర్తులేని min_ఖండం = (గుర్తులేని)(MIN(s->pending_buf_కొలత - 5, s->w_కొలత));

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    పూర్ణాంకం చివరి = 0;
    గుర్తులేని len, ఎడమ, have;
    గుర్తులేని వాడిన = s->strm->avail_లోపల;
    చేయి {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = ((గుర్తులేని)s->bi_valid + 42) >> 3;   /* bytes in header */
        ఒకవేళ (s->strm->avail_బహిర్గతం < have)          /* need room for header */
            ఆపు;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_బహిర్గతం - have;
        ఎడమ = (గుర్తులేని)(s->strstart - s->ఖండం_ప్రారంభం);    /* window bytes */
        ఒకవేళ (len > (ulg)ఎడమ + s->strm->avail_లోపల)
            len = ఎడమ + s->strm->avail_లోపల;     /* limit len to the input */
        ఒకవేళ (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        ఒకవేళ (len < min_ఖండం && ((len == 0 && flush != Z_ముగింపు) ||
                                flush == Z_NO_FLUSH ||
                                len != ఎడమ + s->strm->avail_లోపల))
            ఆపు;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        చివరి = flush == Z_ముగింపు && len == ఎడమ + s->strm->avail_లోపల ? 1 : 0;
        tr_stored_ఖండం(s, (అక్షరం *)0, 0L, చివరి);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = (Bytef)len;
        s->pending_buf[s->pending - 3] = (Bytef)(len >> 8);
        s->pending_buf[s->pending - 2] = (Bytef)~len;
        s->pending_buf[s->pending - 1] = (Bytef)(~len >> 8);

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#నిర్వచితమైతే ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->బిట్లు_పంపిన += len << 3;
#అయితే_ముగింపు

        /* Copy uncompressed bytes from the window to next_out. */
        ఒకవేళ (ఎడమ) {
            ఒకవేళ (ఎడమ > len)
                ఎడమ = len;
            zmemcpy(s->strm->తదుపరి_బహిర్గతం, s->కిటికీ + s->ఖండం_ప్రారంభం, ఎడమ);
            s->strm->తదుపరి_బహిర్గతం += ఎడమ;
            s->strm->avail_బహిర్గతం -= ఎడమ;
            s->strm->మొత్తం_బహిర్గతం += ఎడమ;
            s->ఖండం_ప్రారంభం += ఎడమ;
            len -= ఎడమ;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        ఒకవేళ (len) {
            read_buf(s->strm, s->strm->తదుపరి_బహిర్గతం, len);
            s->strm->తదుపరి_బహిర్గతం += len;
            s->strm->avail_బహిర్గతం -= len;
            s->strm->మొత్తం_బహిర్గతం += len;
        }
    } అయితే (చివరి == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    వాడిన -= s->strm->avail_లోపల;      /* number of input bytes directly copied */
    ఒకవేళ (వాడిన) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        ఒకవేళ (వాడిన >= s->w_కొలత) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->కిటికీ, s->strm->తదుపరి_లోపల - s->w_కొలత, s->w_కొలత);
            s->strstart = s->w_కొలత;
            s->చొప్పించు = s->strstart;
        }
        లేకపోతే {
            ఒకవేళ (s->కిటికీ_కొలత - s->strstart <= వాడిన) {
                /* Slide the window down. */
                s->strstart -= s->w_కొలత;
                zmemcpy(s->కిటికీ, s->కిటికీ + s->w_కొలత, s->strstart);
                ఒకవేళ (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                ఒకవేళ (s->చొప్పించు > s->strstart)
                    s->చొప్పించు = s->strstart;
            }
            zmemcpy(s->కిటికీ + s->strstart, s->strm->తదుపరి_లోపల - వాడిన, వాడిన);
            s->strstart += వాడిన;
            s->చొప్పించు += MIN(వాడిన, s->w_కొలత - s->చొప్పించు);
        }
        s->ఖండం_ప్రారంభం = s->strstart;
    }
    ఒకవేళ (s->ఎగువ_water < s->strstart)
        s->ఎగువ_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    ఒకవేళ (చివరి) {
        s->bi_వాడిన = 8;
        తిరిగి ముగింపు_పూర్తి;
    }

    /* If flushing and all input has been consumed, then done. */
    ఒకవేళ (flush != Z_NO_FLUSH && flush != Z_ముగింపు &&
        s->strm->avail_లోపల == 0 && (దీర్ఘ)s->strstart == s->ఖండం_ప్రారంభం)
        తిరిగి ఖండం_పూర్తి;

    /* Fill the window with any remaining input. */
    have = (గుర్తులేని)(s->కిటికీ_కొలత - s->strstart);
    ఒకవేళ (s->strm->avail_లోపల > have && s->ఖండం_ప్రారంభం >= (దీర్ఘ)s->w_కొలత) {
        /* Slide the window down. */
        s->ఖండం_ప్రారంభం -= s->w_కొలత;
        s->strstart -= s->w_కొలత;
        zmemcpy(s->కిటికీ, s->కిటికీ + s->w_కొలత, s->strstart);
        ఒకవేళ (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_కొలత;          /* more space now */
        ఒకవేళ (s->చొప్పించు > s->strstart)
            s->చొప్పించు = s->strstart;
    }
    ఒకవేళ (have > s->strm->avail_లోపల)
        have = s->strm->avail_లోపల;
    ఒకవేళ (have) {
        read_buf(s->strm, s->కిటికీ + s->strstart, have);
        s->strstart += have;
        s->చొప్పించు += MIN(have, s->w_కొలత - s->చొప్పించు);
    }
    ఒకవేళ (s->ఎగువ_water < s->strstart)
        s->ఎగువ_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = ((గుర్తులేని)s->bi_valid + 42) >> 3;   /* bytes in header */
        /* maximum stored block length that will fit in pending: */
    have = (గుర్తులేని)MIN(s->pending_buf_కొలత - have, MAX_STORED);
    min_ఖండం = MIN(have, s->w_కొలత);
    ఎడమ = (గుర్తులేని)(s->strstart - s->ఖండం_ప్రారంభం);
    ఒకవేళ (ఎడమ >= min_ఖండం ||
        ((ఎడమ || flush == Z_ముగింపు) && flush != Z_NO_FLUSH &&
         s->strm->avail_లోపల == 0 && ఎడమ <= have)) {
        len = MIN(ఎడమ, have);
        చివరి = flush == Z_ముగింపు && s->strm->avail_లోపల == 0 &&
               len == ఎడమ ? 1 : 0;
        tr_stored_ఖండం(s, (charf *)s->కిటికీ + s->ఖండం_ప్రారంభం, len, చివరి);
        s->ఖండం_ప్రారంభం += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    ఒకవేళ (చివరి)
        s->bi_వాడిన = 8;
    తిరిగి చివరి ? ముగింపు_started : need_మరిన్ని;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local ఖండం_state deflate_fast(deflate_state *s, పూర్ణాంకం flush) {
    IPos hash_తల;       /* head of the hash chain */
    పూర్ణాంకం bflush;           /* set if current block must be flushed */

    కొరకు (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        ఒకవేళ (s->lookahead < MIN_LOOKAHEAD) {
            fill_కిటికీ(s);
            ఒకవేళ (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                తిరిగి need_మరిన్ని;
            }
            ఒకవేళ (s->lookahead == 0) ఆపు; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_తల = NIL;
        ఒకవేళ (s->lookahead >= MIN_సరిపోలిక) {
            చొప్పించు_STRING(s, s->strstart, hash_తల);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        ఒకవేళ (hash_తల != NIL && s->strstart - hash_తల <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->సరిపోలిక_నిడివి = longest_సరిపోలిక (s, hash_తల);
            /* longest_match() sets match_start */
        }
        ఒకవేళ (s->సరిపోలిక_నిడివి >= MIN_సరిపోలిక) {
            check_సరిపోలిక(s, s->strstart, s->సరిపోలిక_ప్రారంభం, (పూర్ణాంకం)s->సరిపోలిక_నిడివి);

            _tr_tally_dist(s, s->strstart - s->సరిపోలిక_ప్రారంభం,
                           s->సరిపోలిక_నిడివి - MIN_సరిపోలిక, bflush);

            s->lookahead -= s->సరిపోలిక_నిడివి;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#నిర్వచితం_కాకపోతే FASTEST
            ఒకవేళ (s->సరిపోలిక_నిడివి <= s->max_చొప్పించు_నిడివి &&
                s->lookahead >= MIN_సరిపోలిక) {
                s->సరిపోలిక_నిడివి--; /* string at strstart already in table */
                చేయి {
                    s->strstart++;
                    చొప్పించు_STRING(s, s->strstart, hash_తల);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } అయితే (--s->సరిపోలిక_నిడివి != 0);
                s->strstart++;
            } లేకపోతే
#అయితే_ముగింపు
            {
                s->strstart += s->సరిపోలిక_నిడివి;
                s->సరిపోలిక_నిడివి = 0;
                s->ins_h = s->కిటికీ[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->కిటికీ[s->strstart + 1]);
#ఒకవేళ MIN_సరిపోలిక != 3
                Call UPDATE_HASH() MIN_సరిపోలిక-3 మరిన్ని times
#అయితే_ముగింపు
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } లేకపోతే {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->కిటికీ[s->strstart]));
            _tr_tally_lit(s, s->కిటికీ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        ఒకవేళ (bflush) FLUSH_ఖండం(s, 0);
    }
    s->చొప్పించు = s->strstart < MIN_సరిపోలిక-1 ? s->strstart : MIN_సరిపోలిక-1;
    ఒకవేళ (flush == Z_ముగింపు) {
        FLUSH_ఖండం(s, 1);
        తిరిగి ముగింపు_పూర్తి;
    }
    ఒకవేళ (s->sym_తదుపరి)
        FLUSH_ఖండం(s, 0);
    తిరిగి ఖండం_పూర్తి;
}

#నిర్వచితం_కాకపోతే FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local ఖండం_state deflate_slow(deflate_state *s, పూర్ణాంకం flush) {
    IPos hash_తల;          /* head of hash chain */
    పూర్ణాంకం bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    కొరకు (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        ఒకవేళ (s->lookahead < MIN_LOOKAHEAD) {
            fill_కిటికీ(s);
            ఒకవేళ (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                తిరిగి need_మరిన్ని;
            }
            ఒకవేళ (s->lookahead == 0) ఆపు; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_తల = NIL;
        ఒకవేళ (s->lookahead >= MIN_సరిపోలిక) {
            చొప్పించు_STRING(s, s->strstart, hash_తల);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_నిడివి = s->సరిపోలిక_నిడివి, s->prev_సరిపోలిక = s->సరిపోలిక_ప్రారంభం;
        s->సరిపోలిక_నిడివి = MIN_సరిపోలిక-1;

        ఒకవేళ (hash_తల != NIL && s->prev_నిడివి < s->max_lazy_సరిపోలిక &&
            s->strstart - hash_తల <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->సరిపోలిక_నిడివి = longest_సరిపోలిక (s, hash_తల);
            /* longest_match() sets match_start */

            ఒకవేళ (s->సరిపోలిక_నిడివి <= 5 && (s->strategy == Z_FILTERED
#ఒకవేళ TOO_FAR <= 32767
                || (s->సరిపోలిక_నిడివి == MIN_సరిపోలిక &&
                    s->strstart - s->సరిపోలిక_ప్రారంభం > TOO_FAR)
#అయితే_ముగింపు
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->సరిపోలిక_నిడివి = MIN_సరిపోలిక-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        ఒకవేళ (s->prev_నిడివి >= MIN_సరిపోలిక && s->సరిపోలిక_నిడివి <= s->prev_నిడివి) {
            uInt max_చొప్పించు = s->strstart + s->lookahead - MIN_సరిపోలిక;
            /* Do not insert strings in hash table beyond this. */

            check_సరిపోలిక(s, s->strstart - 1, s->prev_సరిపోలిక, (పూర్ణాంకం)s->prev_నిడివి);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_సరిపోలిక,
                           s->prev_నిడివి - MIN_సరిపోలిక, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_నిడివి - 1;
            s->prev_నిడివి -= 2;
            చేయి {
                ఒకవేళ (++s->strstart <= max_చొప్పించు) {
                    చొప్పించు_STRING(s, s->strstart, hash_తల);
                }
            } అయితే (--s->prev_నిడివి != 0);
            s->సరిపోలిక_అందుబాటు = 0;
            s->సరిపోలిక_నిడివి = MIN_సరిపోలిక-1;
            s->strstart++;

            ఒకవేళ (bflush) FLUSH_ఖండం(s, 0);

        } లేకపోతే ఒకవేళ (s->సరిపోలిక_అందుబాటు) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->కిటికీ[s->strstart - 1]));
            _tr_tally_lit(s, s->కిటికీ[s->strstart - 1], bflush);
            ఒకవేళ (bflush) {
                FLUSH_ఖండం_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            ఒకవేళ (s->strm->avail_బహిర్గతం == 0) తిరిగి need_మరిన్ని;
        } లేకపోతే {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->సరిపోలిక_అందుబాటు = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    ఒకవేళ (s->సరిపోలిక_అందుబాటు) {
        Tracevv((stderr,"%c", s->కిటికీ[s->strstart - 1]));
        _tr_tally_lit(s, s->కిటికీ[s->strstart - 1], bflush);
        s->సరిపోలిక_అందుబాటు = 0;
    }
    s->చొప్పించు = s->strstart < MIN_సరిపోలిక-1 ? s->strstart : MIN_సరిపోలిక-1;
    ఒకవేళ (flush == Z_ముగింపు) {
        FLUSH_ఖండం(s, 1);
        తిరిగి ముగింపు_పూర్తి;
    }
    ఒకవేళ (s->sym_తదుపరి)
        FLUSH_ఖండం(s, 0);
    తిరిగి ఖండం_పూర్తి;
}
#అయితే_ముగింపు /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local ఖండం_state deflate_rle(deflate_state *s, పూర్ణాంకం flush) {
    పూర్ణాంకం bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *పరిశీలన, *strend;   /* scan goes up to strend for length of run */

    కొరకు (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        ఒకవేళ (s->lookahead <= MAX_సరిపోలిక) {
            fill_కిటికీ(s);
            ఒకవేళ (s->lookahead <= MAX_సరిపోలిక && flush == Z_NO_FLUSH) {
                తిరిగి need_మరిన్ని;
            }
            ఒకవేళ (s->lookahead == 0) ఆపు; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->సరిపోలిక_నిడివి = 0;
        ఒకవేళ (s->lookahead >= MIN_సరిపోలిక && s->strstart > 0) {
            పరిశీలన = s->కిటికీ + s->strstart - 1;
            prev = *పరిశీలన;
            ఒకవేళ (prev == *++పరిశీలన && prev == *++పరిశీలన && prev == *++పరిశీలన) {
                strend = s->కిటికీ + s->strstart + MAX_సరిపోలిక;
                చేయి {
                } అయితే (prev == *++పరిశీలన && prev == *++పరిశీలన &&
                         prev == *++పరిశీలన && prev == *++పరిశీలన &&
                         prev == *++పరిశీలన && prev == *++పరిశీలన &&
                         prev == *++పరిశీలన && prev == *++పరిశీలన &&
                         పరిశీలన < strend);
                s->సరిపోలిక_నిడివి = MAX_సరిపోలిక - (uInt)(strend - పరిశీలన);
                ఒకవేళ (s->సరిపోలిక_నిడివి > s->lookahead)
                    s->సరిపోలిక_నిడివి = s->lookahead;
            }
            Assert(పరిశీలన <= s->కిటికీ + (uInt)(s->కిటికీ_కొలత - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        ఒకవేళ (s->సరిపోలిక_నిడివి >= MIN_సరిపోలిక) {
            check_సరిపోలిక(s, s->strstart, s->strstart - 1, (పూర్ణాంకం)s->సరిపోలిక_నిడివి);

            _tr_tally_dist(s, 1, s->సరిపోలిక_నిడివి - MIN_సరిపోలిక, bflush);

            s->lookahead -= s->సరిపోలిక_నిడివి;
            s->strstart += s->సరిపోలిక_నిడివి;
            s->సరిపోలిక_నిడివి = 0;
        } లేకపోతే {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->కిటికీ[s->strstart]));
            _tr_tally_lit(s, s->కిటికీ[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        ఒకవేళ (bflush) FLUSH_ఖండం(s, 0);
    }
    s->చొప్పించు = 0;
    ఒకవేళ (flush == Z_ముగింపు) {
        FLUSH_ఖండం(s, 1);
        తిరిగి ముగింపు_పూర్తి;
    }
    ఒకవేళ (s->sym_తదుపరి)
        FLUSH_ఖండం(s, 0);
    తిరిగి ఖండం_పూర్తి;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local ఖండం_state deflate_huff(deflate_state *s, పూర్ణాంకం flush) {
    పూర్ణాంకం bflush;             /* set if current block must be flushed */

    కొరకు (;;) {
        /* Make sure that we have a literal to write. */
        ఒకవేళ (s->lookahead == 0) {
            fill_కిటికీ(s);
            ఒకవేళ (s->lookahead == 0) {
                ఒకవేళ (flush == Z_NO_FLUSH)
                    తిరిగి need_మరిన్ని;
                ఆపు;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->సరిపోలిక_నిడివి = 0;
        Tracevv((stderr,"%c", s->కిటికీ[s->strstart]));
        _tr_tally_lit(s, s->కిటికీ[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        ఒకవేళ (bflush) FLUSH_ఖండం(s, 0);
    }
    s->చొప్పించు = 0;
    ఒకవేళ (flush == Z_ముగింపు) {
        FLUSH_ఖండం(s, 1);
        తిరిగి ముగింపు_పూర్తి;
    }
    ఒకవేళ (s->sym_తదుపరి)
        FLUSH_ఖండం(s, 0);
    తిరిగి ఖండం_పూర్తి;
}
