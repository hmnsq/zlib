/* enough.c -- determine the maximum size of inflate's Huffman code tables over
 * all possible valid and complete prefix codes, subject to a length limit.
 * Copyright (C) 2007, 2008, 2012, 2018, 2024 Mark Adler
 * Version 1.6  29 July 2024  Mark Adler
 */

/* Version history:
   1.0   3 Jan 2007  First version (derived from codecount.c version 1.4)
   1.1   4 Jan 2007  Use faster incremental table usage computation
                     Prune examine() search on previously visited states
   1.2   5 Jan 2007  Comments clean up
                     As inflate does, decrease root for short codes
                     Refuse cases where inflate would increase root
   1.3  17 Feb 2008  Add argument for initial root table size
                     Fix bug for initial root table size == max - 1
                     Use a macro to compute the history index
   1.4  18 Aug 2012  Avoid shifts more than bits in type (caused endless loop!)
                     Clean up comparisons of different types
                     Clean up code indentation
   1.5   5 Aug 2018  Clean up code style, formatting, and comments
                     Show all the codes for the maximum, and only the maximum
   1.6  29 Jul 2024  Avoid use of uintmax_t
 */

/*
   Examine all possible prefix codes for a given number of symbols and a
   maximum code length in bits to determine the maximum table size for zlib's
   inflate. Only complete prefix codes are counted.

   Two codes are considered distinct if the vectors of the number of codes per
   length are not identical. So permutations of the symbol assignments result
   in the same code for the counting, as do permutations of the assignments of
   the bit values to the codes (i.e. only canonical codes are counted).

   We build a code from shorter to longer lengths, determining how many symbols
   are coded at each length. At each step, we have how many symbols remain to
   be coded, what the last code length used was, and how many bit patterns of
   that length remain unused. Then we add one to the code length and double the
   number of unused patterns to graduate to the next code length. We then
   assign all portions of the remaining symbols to that code length that
   preserve the properties of a correct and eventually complete code. Those
   properties are: we cannot use more bit patterns than are available; and when
   all the symbols are used, there are exactly zero possible bit patterns left
   unused.

   The inflate Huffman decoding algorithm uses two-level lookup tables for
   speed. There is a single first-level table to decode codes up to root bits
   in length (root == 9 for literal/length codes and root == 6 for distance
   codes, in the current inflate implementation). The base table has 1 << root
   entries and is indexed by the next root bits of input. Codes shorter than
   root bits have replicated table entries, so that the correct entry is
   pointed to regardless of the bits that follow the short code. If the code is
   longer than root bits, then the table entry points to a second-level table.
   The size of that table is determined by the longest code with that root-bit
   prefix. If that longest code has length len, then the table has size 1 <<
   (len - root), to index the remaining bits in that set of codes. Each
   subsequent root-bit prefix then has its own sub-table. The total number of
   table entries required by the code is calculated incrementally as the number
   of codes at each bit length is populated. When all of the codes are shorter
   than root bits, then root is reduced to the longest code length, resulting
   in a single, smaller, one-level table.

   The inflate algorithm also provides for small values of root (relative to
   the log2 of the number of symbols), where the shortest code has more bits
   than root. In that case, root is increased to the length of the shortest
   code. This program, by design, does not handle that case, so it is verified
   that the number of symbols is less than 1 << (root + 1).

   In order to speed up the examination (by about ten orders of magnitude for
   the default arguments), the intermediate states in the build-up of a code
   are remembered and previously visited branches are pruned. The memory
   required for this will increase rapidly with the total number of symbols and
   the maximum code length in bits. However this is a very small price to pay
   for the vast speedup.

   First, all of the possible prefix codes are counted, and reachable
   intermediate states are noted by a non-zero count in a saved-results array.
   Second, the intermediate states that lead to (root + 1) bit or longer codes
   are used to look at all sub-codes from those junctures for their inflate
   memory usage. (The amount of memory used is not affected by the number of
   codes of root bits or less in length.)  Third, the visited states in the
   construction of those sub-codes and the associated calculation of the table
   size is recalled in order to avoid recalculating from the same juncture.
   Beginning the code examination at (root + 1) bit codes, which is enabled by
   identifying the reachable nodes, accounts for about six of the orders of
   magnitude of improvement for the default arguments. About another four
   orders of magnitude come from not revisiting previous states. Out of
   approximately 2x10^16 possible prefix codes, only about 2x10^6 sub-codes
   need to be examined to cover all of the possible table memory usage cases
   for the default arguments of 286 symbols limited to 15-bit codes.

   Note that unsigned long long is used for counting. It is quite easy to
   exceed the capacity of an eight-byte integer with a large number of symbols
   and a large maximum code length, so multiple-precision arithmetic would need
   to replace the integer arithmetic in that case. This program will abort if
   an overflow occurs. The big_t type identifies where the counting takes
   place.

   The unsigned long long type is also used for calculating the number of
   possible codes remaining at the maximum length. This limits the maximum code
   length to the number of bits in a long long minus the number of bits needed
   to represent the symbols in a flat code. The code_t type identifies where
   the bit-pattern counting takes place.
 */

#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <stdarg.h>
#شامل <assert.h>

#تعریف_کرو local ساکن

// Special data types.
قسمتعریف غیرعلامتی طویل طویل big_t;   // type for code counting
#تعریف_کرو PRIbig "llu"        // printf format for big_t
قسمتعریف big_t رمز_t;       // type for bit pattern counting
ساخت tab {                // type for been-here check
    size_t len;             // allocated length of bit vector in octets
    حرف *vec;              // allocated bit vector
};

/* The array for saving results, num[], is indexed with this triplet:

      syms: number of symbols remaining to code
      left: number of available bit patterns at length len
      len: number of bits in the codes currently being assigned

   Those indices are constrained thusly when saving results:

      syms: 3..totsym (totsym == total symbols to code)
      left: 2..syms - 1, but only the evens (so syms == 8 -> 2, 4, 6)
      len: 1..max - 1 (max == maximum code length in bits)

   syms == 2 is not saved since that immediately leads to a single code. left
   must be even, since it represents the number of available bit patterns at
   the current length, which is double the number at the previous length. left
   ends at syms-1 since left == syms immediately results in a single code.
   (left > sym is not allowed since that would result in an incomplete code.)
   len is less than max, since the code completes immediately when len == max.

   The offset into the array is calculated for the three indices with the first
   one (syms) being outermost, and the last one (len) being innermost. We build
   the array with length max-1 lists for the len index, with syms-3 of those
   for each symbol. There are totsym-2 of those, with each one varying in
   length as a function of sym. See the calculation of index in map() for the
   index, and the calculation of size in main() for the size of the array.

   For the deflate example of 286 symbols limited to 15-bit codes, the array
   has 284,284 entries, taking up 2.17 MB for an 8-byte big_t. More than half
   of the space allocated for saved results is actually used -- not all
   possible triplets are reached in the generation of valid prefix codes.
 */

/* The array for tracking visited states, done[], is itself indexed identically
   to the num[] array as described above for the (syms, left, len) triplet.
   Each element in the array is further indexed by the (mem, rem) doublet,
   where mem is the amount of inflate table space used so far, and rem is the
   remaining unused entries in the current inflate sub-table. Each indexed
   element is simply one bit indicating whether the state has been visited or
   not. Since the ranges for mem and rem are not known a priori, each bit
   vector is of a variable size, and grows as needed to accommodate the visited
   states. mem and rem are used to calculate a single index in a triangular
   array. Since the range of mem is expected in the default case to be about
   ten times larger than the range of rem, the array is skewed to reduce the
   memory usage, with eight times the range for mem than for rem. See the
   calculations for offset and bit in been_here() for the details.

   For the deflate example of 286 symbols limited to 15-bit codes, the bit
   vectors grow to total 5.5 MB, in addition to the 4.3 MB done array itself.
 */

// Type for a variable-length, allocated string.
قسمتعریف ساخت {
    حرف *str;          // pointer to allocated string
    size_t سائز;        // size of allocation
    size_t len;         // length of string, not including terminating zero
} string_t;

// Clear a string_t.
local خالی string_clear(string_t *s) {
    s->str[0] = 0;
    s->len = 0;
}

// Initialize a string_t.
local خالی string_init(string_t *s) {
    s->سائز = 16;
    s->str = حافظہ_دو(s->سائز);
    أكد(s->str != تہی && "out of memory");
    string_clear(s);
}

// Release the allocation of a string_t.
local خالی string_free(string_t *s) {
    آزاد(s->str);
    s->str = تہی;
    s->سائز = 0;
    s->len = 0;
}

// Save the results of printf with fmt and the subsequent argument list to s.
// Each call appends to s. The allocated space for s is increased as needed.
local خالی string_printf(string_t *s, حرف *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t len = s->len;
    صحیح ret = vsnprintf(s->str + len, s->سائز - len, fmt, ap);
    أكد(ret >= 0 && "out of memory");
    s->len += ret;
    اگر (s->سائز < s->len + 1) {
        کرو {
            s->سائز <<= 1;
            أكد(s->سائز != 0 && "overflow");
        } جب (s->سائز < s->len + 1);
        s->str = realloc(s->str, s->سائز);
        أكد(s->str != تہی && "out of memory");
        vsnprintf(s->str + len, s->سائز - len, fmt, ap);
    }
    va_end(ap);
}

// Globals to avoid propagating constants or constant pointers recursively.
ساخت {
    صحیح max;            // maximum allowed bit length for the codes
    صحیح جڑ;           // size of base code table in bits
    صحیح large;          // largest code table so far
    size_t سائز;        // number of elements in num and done
    big_t tot;          // total number of codes with maximum tables size
    string_t برآمد;       // display of subcodes for maximum tables size
    صحیح *رمز;          // number of symbols assigned to each bit length
    big_t *عدد;         // saved results array for code counting
    ساخت tab *مکمل_2;   // states already evaluated array
} g;

// Index function for num[] and done[].
local اندرونی size_t map(صحیح syms, صحیح بایاں, صحیح len) {
    واپس ((size_t)((syms - 1) >> 1) * ((syms - 2) >> 1) +
            (بایاں >> 1) - 1) * (g.max - 1) +
           len - 1;
}

// Free allocated space in globals.
local خالی cleanup(خالی) {
    اگر (g.مکمل_2 != تہی) {
        برائے (size_t n = 0; n < g.سائز; n++)
            اگر (g.مکمل_2[n].len)
                آزاد(g.مکمل_2[n].vec);
        g.سائز = 0;
        آزاد(g.مکمل_2);   g.مکمل_2 = تہی;
    }
    آزاد(g.عدد);    g.عدد = تہی;
    آزاد(g.رمز);   g.رمز = تہی;
    string_free(&g.برآمد);
}

// Return the number of possible prefix codes using bit patterns of lengths len
// through max inclusive, coding syms symbols, with left bit patterns of length
// len unused -- return -1 if there is an overflow in the counting. Keep a
// record of previous results in num to prevent repeating the same calculation.
local big_t گنتی(صحیح syms, صحیح بایاں, صحیح len) {
    // see if only one possible code
    اگر (syms == بایاں)
        واپس 1;

    // note and verify the expected state
    أكد(syms > بایاں && بایاں > 0 && len < g.max);

    // see if we've done this one already
    size_t اشاریہ = map(syms, بایاں, len);
    big_t got = g.عدد[اشاریہ];
    اگر (got)
        واپس got;         // we have -- return the saved result

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    صحیح کم_سے_کم = (بایاں << 1) - syms;
    اگر (کم_سے_کم < 0)
        کم_سے_کم = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    صحیح most = (((رمز_t)بایاں << (g.max - len)) - syms) /
               (((رمز_t)1 << (g.max - len)) - 1);

    // count all possible codes from this juncture and add them up
    big_t مجموع = 0;
    برائے (صحیح use = کم_سے_کم; use <= most; use++) {
        got = گنتی(syms - use, (بایاں - use) << 1, len + 1);
        مجموع += got;
        اگر (got == (big_t)-1 || مجموع < got)      //حد_سے_تجاوز
            واپس (big_t)-1;
    }

    // verify that all recursive calls are productive
    أكد(مجموع != 0);

    // save the result and return it
    g.عدد[اشاریہ] = مجموع;
    واپس مجموع;
}

// Return true if we've been here before, set to true if not. Set a bit in a
// bit vector to indicate visiting this state. Each (syms,len,left) state has a
// variable size bit vector indexed by (mem,rem). The bit vector is lengthened
// as needed to allow setting the (mem,rem) bit.
local صحیح been_یہاں(صحیح syms, صحیح بایاں, صحیح len, صحیح mem, صحیح rem) {
    // point to vector for (syms,left,len), bit in vector for (mem,rem)
    size_t اشاریہ = map(syms, بایاں, len);
    mem -= 1 << g.جڑ;             // mem always includes the root table
    mem >>= 1;                      // mem and rem are always even
    rem >>= 1;
    size_t انحراف = (mem >> 3) + rem;
    انحراف = ((انحراف * (انحراف + 1)) >> 1) + rem;
    صحیح ثنائی_ہندسہ = 1 << (mem & 7);

    // see if we've been here
    size_t لمبائی = g.مکمل_2[اشاریہ].len;
    اگر (انحراف < لمبائی && (g.مکمل_2[اشاریہ].vec[انحراف] & ثنائی_ہندسہ) != 0)
        واپس 1;       // done this!

    // we haven't been here before -- set the bit to show we have now

    // see if we need to lengthen the vector in order to set the bit
    اگر (لمبائی <= انحراف) {
        // if we have one already, enlarge it, zero out the appended space
        حرف *سمتیہ;
        اگر (لمبائی) {
            کرو {
                لمبائی <<= 1;
            } جب (لمبائی <= انحراف);
            سمتیہ = realloc(g.مکمل_2[اشاریہ].vec, لمبائی);
            أكد(سمتیہ != تہی && "out of memory");
            memset(سمتیہ + g.مکمل_2[اشاریہ].len, 0, لمبائی - g.مکمل_2[اشاریہ].len);
        }

        // otherwise we need to make a new vector and zero it out
        ورنہ {
            لمبائی = 16;
            جب (لمبائی <= انحراف)
                لمبائی <<= 1;
            سمتیہ = صفر_حافظہ_دو(لمبائی, 1);
            أكد(سمتیہ != تہی && "out of memory");
        }

        // install the new vector
        g.مکمل_2[اشاریہ].len = لمبائی;
        g.مکمل_2[اشاریہ].vec = سمتیہ;
    }

    // set the bit
    g.مکمل_2[اشاریہ].vec[انحراف] |= ثنائی_ہندسہ;
    واپس 0;
}

// Examine all possible codes from the given node (syms, len, left). Compute
// the amount of memory required to build inflate's decoding tables, where the
// number of code structures used so far is mem, and the number remaining in
// the current sub-table is rem.
local خالی examine(صحیح syms, صحیح بایاں, صحیح len, صحیح mem, صحیح rem) {
    // see if we have a complete code
    اگر (syms == بایاں) {
        // set the last code entry
        g.رمز[len] = بایاں;

        // complete computation of memory used by this code
        جب (rem < بایاں) {
            بایاں -= rem;
            rem = 1 << (len - g.جڑ);
            mem += rem;
        }
        أكد(rem == بایاں);

        // if this is at the maximum, show the sub-code
        اگر (mem >= g.large) {
            // if this is a new maximum, update the maximum and clear out the
            // printed sub-codes from the previous maximum
            اگر (mem > g.large) {
                g.large = mem;
                string_clear(&g.برآمد);
            }

            // compute the starting state for this sub-code
            syms = 0;
            بایاں = 1 << g.max;
            برائے (صحیح بٹ = g.max; بٹ > g.جڑ; بٹ--) {
                syms += g.رمز[بٹ];
                بایاں -= g.رمز[بٹ];
                أكد((بایاں & 1) == 0);
                بایاں >>= 1;
            }

            // print the starting state and the resulting sub-code to g.out
            string_printf(&g.برآمد, "<%u, %u, %u>:",
                          syms, g.جڑ + 1, ((1 << g.جڑ) - بایاں) << 1);
            برائے (صحیح بٹ = g.جڑ + 1; بٹ <= g.max; بٹ++)
                اگر (g.رمز[بٹ])
                    string_printf(&g.برآمد, " %d[%d]", g.رمز[بٹ], بٹ);
            string_printf(&g.برآمد, "\n");
        }

        // remove entries as we drop back down in the recursion
        g.رمز[len] = 0;
        واپس;
    }

    // prune the tree if we can
    اگر (been_یہاں(syms, بایاں, len, mem, rem))
        واپس;

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    صحیح کم_سے_کم = (بایاں << 1) - syms;
    اگر (کم_سے_کم < 0)
        کم_سے_کم = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    صحیح most = (((رمز_t)بایاں << (g.max - len)) - syms) /
               (((رمز_t)1 << (g.max - len)) - 1);

    // occupy least table spaces, creating new sub-tables as needed
    صحیح use = کم_سے_کم;
    جب (rem < use) {
        use -= rem;
        rem = 1 << (len - g.جڑ);
        mem += rem;
    }
    rem -= use;

    // examine codes from here, updating table space as we go
    برائے (use = کم_سے_کم; use <= most; use++) {
        g.رمز[len] = use;
        examine(syms - use, (بایاں - use) << 1, len + 1,
                mem + (rem ? 1 << (len - g.جڑ) : 0), rem << 1);
        اگر (rem == 0) {
            rem = 1 << (len - g.جڑ);
            mem += rem;
        }
        rem--;
    }

    // remove entries as we drop back down in the recursion
    g.رمز[len] = 0;
}

// Look at all sub-codes starting with root + 1 bits. Look at only the valid
// intermediate code states (syms, left, len). For each completed code,
// calculate the amount of memory required by inflate to build the decoding
// tables. Find the maximum amount of memory required and show the codes that
// require that maximum.
local خالی enough(صحیح syms) {
    // clear code
    برائے (صحیح n = 0; n <= g.max; n++)
        g.رمز[n] = 0;

    // look at all (root + 1) bit and longer codes
    string_clear(&g.برآمد);           // empty saved results
    g.large = 1 << g.جڑ;          // base table
    اگر (g.جڑ < g.max)             // otherwise, there's only a base table
        برائے (صحیح n = 3; n <= syms; n++)
            برائے (صحیح بایاں = 2; بایاں < n; بایاں += 2) {
                // look at all reachable (root + 1) bit nodes, and the
                // resulting codes (complete at root + 2 or more)
                size_t اشاریہ = map(n, بایاں, g.جڑ + 1);
                اگر (g.جڑ + 1 < g.max && g.عدد[اشاریہ]) // reachable node
                    examine(n, بایاں, g.جڑ + 1, 1 << g.جڑ, 0);

                // also look at root bit codes with completions at root + 1
                // bits (not saved in num, since complete), just in case
                اگر (g.عدد[اشاریہ - 1] && n <= بایاں << 1)
                    examine((n - بایاں) << 1, (n - بایاں) << 1, g.جڑ + 1,
                            1 << g.جڑ, 0);
            }

    //مکمل
    لکھو("maximum of %d table entries for root = %d\n", g.large, g.جڑ);
    fputs(g.برآمد.str, stdout);
}

// Examine and show the total number of possible prefix codes for a given
// maximum number of symbols, initial root table size, and maximum code length
// in bits -- those are the command arguments in that order. The default values
// are 286, 9, and 15 respectively, for the deflate literal/length code. The
// possible codes are counted for each number of coded symbols from two to the
// maximum. The counts for each of those and the total number of codes are
// shown. The maximum number of inflate table entries is then calculated across
// all possible codes. Each new maximum number of table entries and the
// associated sub-code (starting at root + 1 == 10 bits) is shown.
//
// To count and examine prefix codes that are not length-limited, provide a
// maximum length equal to the number of symbols minus one.
//
// For the deflate literal/length code, use "enough". For the deflate distance
// code, use "enough 30 6".
صحیح مین(صحیح argc, حرف **argv) {
    // set up globals for cleanup()
    g.رمز = تہی;
    g.عدد = تہی;
    g.مکمل_2 = تہی;
    string_init(&g.برآمد);

    // get arguments -- default to the deflate literal/length code
    صحیح syms = 286;
    g.جڑ = 9;
    g.max = 15;
    اگر (argc > 1) {
        syms = atoi(argv[1]);
        اگر (argc > 2) {
            g.جڑ = atoi(argv[2]);
            اگر (argc > 3)
                g.max = atoi(argv[3]);
        }
    }
    اگر (argc > 4 || syms < 2 || g.جڑ < 1 || g.max < 1) {
        fputs("invalid arguments, need: [sym >= 2 [root >= 1 [max >= 1]]]\n",
              stderr);
        واپس 1;
    }

    // if not restricting the code length, the longest is syms - 1
    اگر (g.max > syms - 1)
        g.max = syms - 1;

    // determine the number of bits in a code_t
    صحیح بٹ = 0;
    برائے (رمز_t لفظ = 1; لفظ; لفظ <<= 1)
        بٹ++;

    // make sure that the calculation of most will not overflow
    اگر (g.max > بٹ || (رمز_t)(syms - 2) >= ((رمز_t)-1 >> (g.max - 1))) {
        fputs("abort: code length too long for internal types\n", stderr);
        واپس 1;
    }

    // reject impossible code requests
    اگر ((رمز_t)(syms - 1) > ((رمز_t)1 << g.max) - 1) {
        fprintf(stderr, "%d symbols cannot be coded in %d bits\n",
                syms, g.max);
        واپس 1;
    }

    // allocate code vector
    g.رمز = صفر_حافظہ_دو(g.max + 1, حجم(صحیح));
    أكد(g.رمز != تہی && "out of memory");

    // determine size of saved results array, checking for overflows,
    // allocate and clear the array (set all to zero with calloc())
    اگر (syms == 2)              // iff max == 1
        g.عدد = تہی;           // won't be saving any results
    ورنہ {
        g.سائز = syms >> 1;
        صحیح n = (syms - 1) >> 1;
        أكد(g.سائز <= (size_t)-1 / n && "overflow");
        g.سائز *= n;
        n = g.max - 1;
        أكد(g.سائز <= (size_t)-1 / n && "overflow");
        g.سائز *= n;
        g.عدد = صفر_حافظہ_دو(g.سائز, حجم(big_t));
        أكد(g.عدد != تہی && "out of memory");
    }

    // count possible codes for all numbers of symbols, add up counts
    big_t مجموع = 0;
    برائے (صحیح n = 2; n <= syms; n++) {
        big_t got = گنتی(n, 2, 1);
        مجموع += got;
        أكد(got != (big_t)-1 && مجموع >= got && "overflow");
    }
    لکھو("%"PRIbig" total codes for 2 to %d symbols", مجموع, syms);
    اگر (g.max < syms - 1)
        لکھو(" (%d-bit length limit)\n", g.max);
    ورنہ
        puts(" (no length limit)");

    // allocate and clear done array for been_here()
    اگر (syms == 2)
        g.مکمل_2 = تہی;
    ورنہ {
        g.مکمل_2 = صفر_حافظہ_دو(g.سائز, حجم(ساخت tab));
        أكد(g.مکمل_2 != تہی && "out of memory");
    }

    // find and show maximum inflate table usage
    اگر (g.جڑ > g.max)             // reduce root to max length
        g.جڑ = g.max;
    اگر ((رمز_t)syms < ((رمز_t)1 << (g.جڑ + 1)))
        enough(syms);
    ورنہ
        fputs("cannot handle minimum code lengths > root", stderr);

    //مکمل
    cleanup();
    واپس 0;
}
