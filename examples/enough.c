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

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <stdarg.h>
#ضمن <assert.h>

#define local ساكن

// Special data types.
تعريفنوع unsigned long long big_t;   // type for code counting
#define PRIbig "llu"        // printf format for big_t
تعريفنوع big_t code_t;       // type for bit pattern counting
بنية tab {                // type for been-here check
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
تعريفنوع بنية {
    حرف *str;          // pointer to allocated string
    size_t size;        // size of allocation
    size_t len;         // length of string, not including terminating zero
} string_t;

// Clear a string_t.
local فراغ string_clear(string_t *s) {
    s->str[0] = 0;
    s->len = 0;
}

// Initialize a string_t.
local فراغ string_init(string_t *s) {
    s->size = 16;
    s->str = malloc(s->size);
    أكد(s->str != عدم && "out of memory");
    string_clear(s);
}

// Release the allocation of a string_t.
local فراغ string_free(string_t *s) {
    حرر(s->str);
    s->str = عدم;
    s->size = 0;
    s->len = 0;
}

// Save the results of printf with fmt and the subsequent argument list to s.
// Each call appends to s. The allocated space for s is increased as needed.
local فراغ string_printf(string_t *s, حرف *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t len = s->len;
    صحيح ret = vsnprintf(s->str + len, s->size - len, fmt, ap);
    أكد(ret >= 0 && "out of memory");
    s->len += ret;
    إذا (s->size < s->len + 1) {
        افعل {
            s->size <<= 1;
            أكد(s->size != 0 && "overflow");
        } طالما (s->size < s->len + 1);
        s->str = realloc(s->str, s->size);
        أكد(s->str != عدم && "out of memory");
        vsnprintf(s->str + len, s->size - len, fmt, ap);
    }
    va_end(ap);
}

// Globals to avoid propagating constants or constant pointers recursively.
بنية {
    صحيح max;            // maximum allowed bit length for the codes
    صحيح سيو;           // size of base code table in bits
    صحيح large;          // largest code table so far
    size_t size;        // number of elements in num and done
    big_t tot;          // total number of codes with maximum tables size
    string_t out;       // display of subcodes for maximum tables size
    صحيح *code;          // number of symbols assigned to each bit length
    big_t *لمب_2;         // saved results array for code counting
    بنية tab *done;   // states already evaluated array
} g;

// Index function for num[] and done[].
local مضمن size_t map(صحيح syms, صحيح هغو, صحيح len) {
    أرجع ((size_t)((syms - 1) >> 1) * ((syms - 2) >> 1) +
            (هغو >> 1) - 1) * (g.max - 1) +
           len - 1;
}

// Free allocated space in globals.
local فراغ cleanup(فراغ) {
    إذا (g.done != عدم) {
        لكل (size_t n = 0; n < g.size; n++)
            إذا (g.done[n].len)
                حرر(g.done[n].vec);
        g.size = 0;
        حرر(g.done);   g.done = عدم;
    }
    حرر(g.لمب_2);    g.لمب_2 = عدم;
    حرر(g.code);   g.code = عدم;
    string_free(&g.out);
}

// Return the number of possible prefix codes using bit patterns of lengths len
// through max inclusive, coding syms symbols, with left bit patterns of length
// len unused -- return -1 if there is an overflow in the counting. Keep a
// record of previous results in num to prevent repeating the same calculation.
local big_t كدي(صحيح syms, صحيح هغو, صحيح len) {
    // see if only one possible code
    إذا (syms == هغو)
        أرجع 1;

    // note and verify the expected state
    أكد(syms > هغو && هغو > 0 && len < g.max);

    // see if we've done this one already
    size_t index = map(syms, هغو, len);
    big_t got = g.لمب_2[index];
    إذا (got)
        أرجع got;         // we have -- return the saved result

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    صحيح least = (هغو << 1) - syms;
    إذا (least < 0)
        least = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    صحيح most = (((code_t)هغو << (g.max - len)) - syms) /
               (((code_t)1 << (g.max - len)) - 1);

    // count all possible codes from this juncture and add them up
    big_t sum = 0;
    لكل (صحيح use = least; use <= most; use++) {
        got = كدي(syms - use, (هغو - use) << 1, len + 1);
        sum += got;
        إذا (got == (big_t)-1 || sum < got)      // overflow
            أرجع (big_t)-1;
    }

    // verify that all recursive calls are productive
    أكد(sum != 0);

    // save the result and return it
    g.لمب_2[index] = sum;
    أرجع sum;
}

// Return true if we've been here before, set to true if not. Set a bit in a
// bit vector to indicate visiting this state. Each (syms,len,left) state has a
// variable size bit vector indexed by (mem,rem). The bit vector is lengthened
// as needed to allow setting the (mem,rem) bit.
local صحيح been_here(صحيح syms, صحيح هغو, صحيح len, صحيح mem, صحيح rem) {
    // point to vector for (syms,left,len), bit in vector for (mem,rem)
    size_t index = map(syms, هغو, len);
    mem -= 1 << g.سيو;             // mem always includes the root table
    mem >>= 1;                      // mem and rem are always even
    rem >>= 1;
    size_t offset = (mem >> 3) + rem;
    offset = ((offset * (offset + 1)) >> 1) + rem;
    صحيح bit = 1 << (mem & 7);

    // see if we've been here
    size_t length = g.done[index].len;
    إذا (offset < length && (g.done[index].vec[offset] & bit) != 0)
        أرجع 1;       // done this!

    // we haven't been here before -- set the bit to show we have now

    // see if we need to lengthen the vector in order to set the bit
    إذا (length <= offset) {
        // if we have one already, enlarge it, zero out the appended space
        حرف *vector;
        إذا (length) {
            افعل {
                length <<= 1;
            } طالما (length <= offset);
            vector = realloc(g.done[index].vec, length);
            أكد(vector != عدم && "out of memory");
            memset(vector + g.done[index].len, 0, length - g.done[index].len);
        }

        // otherwise we need to make a new vector and zero it out
        وإلا {
            length = 16;
            طالما (length <= offset)
                length <<= 1;
            vector = calloc(length, 1);
            أكد(vector != عدم && "out of memory");
        }

        // install the new vector
        g.done[index].len = length;
        g.done[index].vec = vector;
    }

    // set the bit
    g.done[index].vec[offset] |= bit;
    أرجع 0;
}

// Examine all possible codes from the given node (syms, len, left). Compute
// the amount of memory required to build inflate's decoding tables, where the
// number of code structures used so far is mem, and the number remaining in
// the current sub-table is rem.
local فراغ examine(صحيح syms, صحيح هغو, صحيح len, صحيح mem, صحيح rem) {
    // see if we have a complete code
    إذا (syms == هغو) {
        // set the last code entry
        g.code[len] = هغو;

        // complete computation of memory used by this code
        طالما (rem < هغو) {
            هغو -= rem;
            rem = 1 << (len - g.سيو);
            mem += rem;
        }
        أكد(rem == هغو);

        // if this is at the maximum, show the sub-code
        إذا (mem >= g.large) {
            // if this is a new maximum, update the maximum and clear out the
            // printed sub-codes from the previous maximum
            إذا (mem > g.large) {
                g.large = mem;
                string_clear(&g.out);
            }

            // compute the starting state for this sub-code
            syms = 0;
            هغو = 1 << g.max;
            لكل (صحيح bits = g.max; bits > g.سيو; bits--) {
                syms += g.code[bits];
                هغو -= g.code[bits];
                أكد((هغو & 1) == 0);
                هغو >>= 1;
            }

            // print the starting state and the resulting sub-code to g.out
            string_printf(&g.out, "<%u, %u, %u>:",
                          syms, g.سيو + 1, ((1 << g.سيو) - هغو) << 1);
            لكل (صحيح bits = g.سيو + 1; bits <= g.max; bits++)
                إذا (g.code[bits])
                    string_printf(&g.out, " %d[%d]", g.code[bits], bits);
            string_printf(&g.out, "\n");
        }

        // remove entries as we drop back down in the recursion
        g.code[len] = 0;
        أرجع;
    }

    // prune the tree if we can
    إذا (been_here(syms, هغو, len, mem, rem))
        أرجع;

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    صحيح least = (هغو << 1) - syms;
    إذا (least < 0)
        least = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    صحيح most = (((code_t)هغو << (g.max - len)) - syms) /
               (((code_t)1 << (g.max - len)) - 1);

    // occupy least table spaces, creating new sub-tables as needed
    صحيح use = least;
    طالما (rem < use) {
        use -= rem;
        rem = 1 << (len - g.سيو);
        mem += rem;
    }
    rem -= use;

    // examine codes from here, updating table space as we go
    لكل (use = least; use <= most; use++) {
        g.code[len] = use;
        examine(syms - use, (هغو - use) << 1, len + 1,
                mem + (rem ? 1 << (len - g.سيو) : 0), rem << 1);
        إذا (rem == 0) {
            rem = 1 << (len - g.سيو);
            mem += rem;
        }
        rem--;
    }

    // remove entries as we drop back down in the recursion
    g.code[len] = 0;
}

// Look at all sub-codes starting with root + 1 bits. Look at only the valid
// intermediate code states (syms, left, len). For each completed code,
// calculate the amount of memory required by inflate to build the decoding
// tables. Find the maximum amount of memory required and show the codes that
// require that maximum.
local فراغ enough(صحيح syms) {
    // clear code
    لكل (صحيح n = 0; n <= g.max; n++)
        g.code[n] = 0;

    // look at all (root + 1) bit and longer codes
    string_clear(&g.out);           // empty saved results
    g.large = 1 << g.سيو;          // base table
    إذا (g.سيو < g.max)             // otherwise, there's only a base table
        لكل (صحيح n = 3; n <= syms; n++)
            لكل (صحيح هغو = 2; هغو < n; هغو += 2) {
                // look at all reachable (root + 1) bit nodes, and the
                // resulting codes (complete at root + 2 or more)
                size_t index = map(n, هغو, g.سيو + 1);
                إذا (g.سيو + 1 < g.max && g.لمب_2[index]) // reachable node
                    examine(n, هغو, g.سيو + 1, 1 << g.سيو, 0);

                // also look at root bit codes with completions at root + 1
                // bits (not saved in num, since complete), just in case
                إذا (g.لمب_2[index - 1] && n <= هغو << 1)
                    examine((n - هغو) << 1, (n - هغو) << 1, g.سيو + 1,
                            1 << g.سيو, 0);
            }

    // done
    printf("maximum of %d table entries for root = %d\n", g.large, g.سيو);
    fputs(g.out.str, stdout);
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
صحيح main(صحيح argc, حرف **argv) {
    // set up globals for cleanup()
    g.code = عدم;
    g.لمب_2 = عدم;
    g.done = عدم;
    string_init(&g.out);

    // get arguments -- default to the deflate literal/length code
    صحيح syms = 286;
    g.سيو = 9;
    g.max = 15;
    إذا (argc > 1) {
        syms = atoi(argv[1]);
        إذا (argc > 2) {
            g.سيو = atoi(argv[2]);
            إذا (argc > 3)
                g.max = atoi(argv[3]);
        }
    }
    إذا (argc > 4 || syms < 2 || g.سيو < 1 || g.max < 1) {
        fputs("invalid arguments, need: [sym >= 2 [root >= 1 [max >= 1]]]\n",
              stderr);
        أرجع 1;
    }

    // if not restricting the code length, the longest is syms - 1
    إذا (g.max > syms - 1)
        g.max = syms - 1;

    // determine the number of bits in a code_t
    صحيح bits = 0;
    لكل (code_t word = 1; word; word <<= 1)
        bits++;

    // make sure that the calculation of most will not overflow
    إذا (g.max > bits || (code_t)(syms - 2) >= ((code_t)-1 >> (g.max - 1))) {
        fputs("abort: code length too long for internal types\n", stderr);
        أرجع 1;
    }

    // reject impossible code requests
    إذا ((code_t)(syms - 1) > ((code_t)1 << g.max) - 1) {
        fprintf(stderr, "%d symbols cannot be coded in %d bits\n",
                syms, g.max);
        أرجع 1;
    }

    // allocate code vector
    g.code = calloc(g.max + 1, حجم(صحيح));
    أكد(g.code != عدم && "out of memory");

    // determine size of saved results array, checking for overflows,
    // allocate and clear the array (set all to zero with calloc())
    إذا (syms == 2)              // iff max == 1
        g.لمب_2 = عدم;           // won't be saving any results
    وإلا {
        g.size = syms >> 1;
        صحيح n = (syms - 1) >> 1;
        أكد(g.size <= (size_t)-1 / n && "overflow");
        g.size *= n;
        n = g.max - 1;
        أكد(g.size <= (size_t)-1 / n && "overflow");
        g.size *= n;
        g.لمب_2 = calloc(g.size, حجم(big_t));
        أكد(g.لمب_2 != عدم && "out of memory");
    }

    // count possible codes for all numbers of symbols, add up counts
    big_t sum = 0;
    لكل (صحيح n = 2; n <= syms; n++) {
        big_t got = كدي(n, 2, 1);
        sum += got;
        أكد(got != (big_t)-1 && sum >= got && "overflow");
    }
    printf("%"PRIbig" total codes for 2 to %d symbols", sum, syms);
    إذا (g.max < syms - 1)
        printf(" (%d-bit length limit)\n", g.max);
    وإلا
        puts(" (no length limit)");

    // allocate and clear done array for been_here()
    إذا (syms == 2)
        g.done = عدم;
    وإلا {
        g.done = calloc(g.size, حجم(بنية tab));
        أكد(g.done != عدم && "out of memory");
    }

    // find and show maximum inflate table usage
    إذا (g.سيو > g.max)             // reduce root to max length
        g.سيو = g.max;
    إذا ((code_t)syms < ((code_t)1 << (g.سيو + 1)))
        enough(syms);
    وإلا
        fputs("cannot handle minimum code lengths > root", stderr);

    // done
    cleanup();
    أرجع 0;
}
