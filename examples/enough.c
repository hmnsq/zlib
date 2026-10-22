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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#define local ಸ್ಥಿರ

// Special data types.
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long long big_t;   // type for code counting
#define PRIbig "llu"        // printf format for big_t
ಪ್ರಕಾರನಿರ್ದೇಶ big_t code_t;       // type for bit pattern counting
ಸಂರಚನಾ tab {                // type for been-here check
    size_t len;             // allocated length of bit vector in octets
    ವರ್ಣ *vec;              // allocated bit vector
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    ವರ್ಣ *str;          // pointer to allocated string
    size_t size;        // size of allocation
    size_t len;         // length of string, not including terminating zero
} string_t;

// Clear a string_t.
local ಶೂನ್ಯ string_clear(string_t *s) {
    s->str[0] = 0;
    s->len = 0;
}

// Initialize a string_t.
local ಶೂನ್ಯ string_init(string_t *s) {
    s->size = 16;
    s->str = malloc(s->size);
    ಪ್ರತಿಜ್ಞಾ(s->str != ರಿಕ್ತ && "out of memory");
    string_clear(s);
}

// Release the allocation of a string_t.
local ಶೂನ್ಯ string_free(string_t *s) {
    ಮುಕ್ತ(s->str);
    s->str = ರಿಕ್ತ;
    s->size = 0;
    s->len = 0;
}

// Save the results of printf with fmt and the subsequent argument list to s.
// Each call appends to s. The allocated space for s is increased as needed.
local ಶೂನ್ಯ string_printf(string_t *s, ವರ್ಣ *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t len = s->len;
    ಪೂರ್ಣಾಙ್ಕ ret = vsnprintf(s->str + len, s->size - len, fmt, ap);
    ಪ್ರತಿಜ್ಞಾ(ret >= 0 && "out of memory");
    s->len += ret;
    ಯದಿ (s->size < s->len + 1) {
        ಕುರು {
            s->size <<= 1;
            ಪ್ರತಿಜ್ಞಾ(s->size != 0 && "overflow");
        } ಯಾವತ್ (s->size < s->len + 1);
        s->str = realloc(s->str, s->size);
        ಪ್ರತಿಜ್ಞಾ(s->str != ರಿಕ್ತ && "out of memory");
        vsnprintf(s->str + len, s->size - len, fmt, ap);
    }
    va_end(ap);
}

// Globals to avoid propagating constants or constant pointers recursively.
ಸಂರಚನಾ {
    ಪೂರ್ಣಾಙ್ಕ max;            // maximum allowed bit length for the codes
    ಪೂರ್ಣಾಙ್ಕ ಬೇರು;           // size of base code table in bits
    ಪೂರ್ಣಾಙ್ಕ large;          // largest code table so far
    size_t size;        // number of elements in num and done
    big_t tot;          // total number of codes with maximum tables size
    string_t out;       // display of subcodes for maximum tables size
    ಪೂರ್ಣಾಙ್ಕ *code;          // number of symbols assigned to each bit length
    big_t *ಅಂಕೆ;         // saved results array for code counting
    ಸಂರಚನಾ tab *done;   // states already evaluated array
} g;

// Index function for num[] and done[].
local ಅನ್ತಃಸ್ಥ size_t map(ಪೂರ್ಣಾಙ್ಕ syms, ಪೂರ್ಣಾಙ್ಕ ಎಡ, ಪೂರ್ಣಾಙ್ಕ len) {
    ಪ್ರತ್ಯಾಗಮ ((size_t)((syms - 1) >> 1) * ((syms - 2) >> 1) +
            (ಎಡ >> 1) - 1) * (g.max - 1) +
           len - 1;
}

// Free allocated space in globals.
local ಶೂನ್ಯ cleanup(ಶೂನ್ಯ) {
    ಯದಿ (g.done != ರಿಕ್ತ) {
        ಪ್ರತಿ (size_t n = 0; n < g.size; n++)
            ಯದಿ (g.done[n].len)
                ಮುಕ್ತ(g.done[n].vec);
        g.size = 0;
        ಮುಕ್ತ(g.done);   g.done = ರಿಕ್ತ;
    }
    ಮುಕ್ತ(g.ಅಂಕೆ);    g.ಅಂಕೆ = ರಿಕ್ತ;
    ಮುಕ್ತ(g.code);   g.code = ರಿಕ್ತ;
    string_free(&g.out);
}

// Return the number of possible prefix codes using bit patterns of lengths len
// through max inclusive, coding syms symbols, with left bit patterns of length
// len unused -- return -1 if there is an overflow in the counting. Keep a
// record of previous results in num to prevent repeating the same calculation.
local big_t ಎಣಿಕೆ(ಪೂರ್ಣಾಙ್ಕ syms, ಪೂರ್ಣಾಙ್ಕ ಎಡ, ಪೂರ್ಣಾಙ್ಕ len) {
    // see if only one possible code
    ಯದಿ (syms == ಎಡ)
        ಪ್ರತ್ಯಾಗಮ 1;

    // note and verify the expected state
    ಪ್ರತಿಜ್ಞಾ(syms > ಎಡ && ಎಡ > 0 && len < g.max);

    // see if we've done this one already
    size_t index = map(syms, ಎಡ, len);
    big_t got = g.ಅಂಕೆ[index];
    ಯದಿ (got)
        ಪ್ರತ್ಯಾಗಮ got;         // we have -- return the saved result

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    ಪೂರ್ಣಾಙ್ಕ least = (ಎಡ << 1) - syms;
    ಯದಿ (least < 0)
        least = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    ಪೂರ್ಣಾಙ್ಕ most = (((code_t)ಎಡ << (g.max - len)) - syms) /
               (((code_t)1 << (g.max - len)) - 1);

    // count all possible codes from this juncture and add them up
    big_t sum = 0;
    ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ use = least; use <= most; use++) {
        got = ಎಣಿಕೆ(syms - use, (ಎಡ - use) << 1, len + 1);
        sum += got;
        ಯದಿ (got == (big_t)-1 || sum < got)      // overflow
            ಪ್ರತ್ಯಾಗಮ (big_t)-1;
    }

    // verify that all recursive calls are productive
    ಪ್ರತಿಜ್ಞಾ(sum != 0);

    // save the result and return it
    g.ಅಂಕೆ[index] = sum;
    ಪ್ರತ್ಯಾಗಮ sum;
}

// Return true if we've been here before, set to true if not. Set a bit in a
// bit vector to indicate visiting this state. Each (syms,len,left) state has a
// variable size bit vector indexed by (mem,rem). The bit vector is lengthened
// as needed to allow setting the (mem,rem) bit.
local ಪೂರ್ಣಾಙ್ಕ been_here(ಪೂರ್ಣಾಙ್ಕ syms, ಪೂರ್ಣಾಙ್ಕ ಎಡ, ಪೂರ್ಣಾಙ್ಕ len, ಪೂರ್ಣಾಙ್ಕ mem, ಪೂರ್ಣಾಙ್ಕ rem) {
    // point to vector for (syms,left,len), bit in vector for (mem,rem)
    size_t index = map(syms, ಎಡ, len);
    mem -= 1 << g.ಬೇರು;             // mem always includes the root table
    mem >>= 1;                      // mem and rem are always even
    rem >>= 1;
    size_t offset = (mem >> 3) + rem;
    offset = ((offset * (offset + 1)) >> 1) + rem;
    ಪೂರ್ಣಾಙ್ಕ bit = 1 << (mem & 7);

    // see if we've been here
    size_t length = g.done[index].len;
    ಯದಿ (offset < length && (g.done[index].vec[offset] & bit) != 0)
        ಪ್ರತ್ಯಾಗಮ 1;       // done this!

    // we haven't been here before -- set the bit to show we have now

    // see if we need to lengthen the vector in order to set the bit
    ಯದಿ (length <= offset) {
        // if we have one already, enlarge it, zero out the appended space
        ವರ್ಣ *vector;
        ಯದಿ (length) {
            ಕುರು {
                length <<= 1;
            } ಯಾವತ್ (length <= offset);
            vector = realloc(g.done[index].vec, length);
            ಪ್ರತಿಜ್ಞಾ(vector != ರಿಕ್ತ && "out of memory");
            memset(vector + g.done[index].len, 0, length - g.done[index].len);
        }

        // otherwise we need to make a new vector and zero it out
        ಅನ್ಯಥಾ {
            length = 16;
            ಯಾವತ್ (length <= offset)
                length <<= 1;
            vector = calloc(length, 1);
            ಪ್ರತಿಜ್ಞಾ(vector != ರಿಕ್ತ && "out of memory");
        }

        // install the new vector
        g.done[index].len = length;
        g.done[index].vec = vector;
    }

    // set the bit
    g.done[index].vec[offset] |= bit;
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Examine all possible codes from the given node (syms, len, left). Compute
// the amount of memory required to build inflate's decoding tables, where the
// number of code structures used so far is mem, and the number remaining in
// the current sub-table is rem.
local ಶೂನ್ಯ examine(ಪೂರ್ಣಾಙ್ಕ syms, ಪೂರ್ಣಾಙ್ಕ ಎಡ, ಪೂರ್ಣಾಙ್ಕ len, ಪೂರ್ಣಾಙ್ಕ mem, ಪೂರ್ಣಾಙ್ಕ rem) {
    // see if we have a complete code
    ಯದಿ (syms == ಎಡ) {
        // set the last code entry
        g.code[len] = ಎಡ;

        // complete computation of memory used by this code
        ಯಾವತ್ (rem < ಎಡ) {
            ಎಡ -= rem;
            rem = 1 << (len - g.ಬೇರು);
            mem += rem;
        }
        ಪ್ರತಿಜ್ಞಾ(rem == ಎಡ);

        // if this is at the maximum, show the sub-code
        ಯದಿ (mem >= g.large) {
            // if this is a new maximum, update the maximum and clear out the
            // printed sub-codes from the previous maximum
            ಯದಿ (mem > g.large) {
                g.large = mem;
                string_clear(&g.out);
            }

            // compute the starting state for this sub-code
            syms = 0;
            ಎಡ = 1 << g.max;
            ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ bits = g.max; bits > g.ಬೇರು; bits--) {
                syms += g.code[bits];
                ಎಡ -= g.code[bits];
                ಪ್ರತಿಜ್ಞಾ((ಎಡ & 1) == 0);
                ಎಡ >>= 1;
            }

            // print the starting state and the resulting sub-code to g.out
            string_printf(&g.out, "<%u, %u, %u>:",
                          syms, g.ಬೇರು + 1, ((1 << g.ಬೇರು) - ಎಡ) << 1);
            ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ bits = g.ಬೇರು + 1; bits <= g.max; bits++)
                ಯದಿ (g.code[bits])
                    string_printf(&g.out, " %d[%d]", g.code[bits], bits);
            string_printf(&g.out, "\n");
        }

        // remove entries as we drop back down in the recursion
        g.code[len] = 0;
        ಪ್ರತ್ಯಾಗಮ;
    }

    // prune the tree if we can
    ಯದಿ (been_here(syms, ಎಡ, len, mem, rem))
        ಪ್ರತ್ಯಾಗಮ;

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    ಪೂರ್ಣಾಙ್ಕ least = (ಎಡ << 1) - syms;
    ಯದಿ (least < 0)
        least = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    ಪೂರ್ಣಾಙ್ಕ most = (((code_t)ಎಡ << (g.max - len)) - syms) /
               (((code_t)1 << (g.max - len)) - 1);

    // occupy least table spaces, creating new sub-tables as needed
    ಪೂರ್ಣಾಙ್ಕ use = least;
    ಯಾವತ್ (rem < use) {
        use -= rem;
        rem = 1 << (len - g.ಬೇರು);
        mem += rem;
    }
    rem -= use;

    // examine codes from here, updating table space as we go
    ಪ್ರತಿ (use = least; use <= most; use++) {
        g.code[len] = use;
        examine(syms - use, (ಎಡ - use) << 1, len + 1,
                mem + (rem ? 1 << (len - g.ಬೇರು) : 0), rem << 1);
        ಯದಿ (rem == 0) {
            rem = 1 << (len - g.ಬೇರು);
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
local ಶೂನ್ಯ enough(ಪೂರ್ಣಾಙ್ಕ syms) {
    // clear code
    ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ n = 0; n <= g.max; n++)
        g.code[n] = 0;

    // look at all (root + 1) bit and longer codes
    string_clear(&g.out);           // empty saved results
    g.large = 1 << g.ಬೇರು;          // base table
    ಯದಿ (g.ಬೇರು < g.max)             // otherwise, there's only a base table
        ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ n = 3; n <= syms; n++)
            ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ ಎಡ = 2; ಎಡ < n; ಎಡ += 2) {
                // look at all reachable (root + 1) bit nodes, and the
                // resulting codes (complete at root + 2 or more)
                size_t index = map(n, ಎಡ, g.ಬೇರು + 1);
                ಯದಿ (g.ಬೇರು + 1 < g.max && g.ಅಂಕೆ[index]) // reachable node
                    examine(n, ಎಡ, g.ಬೇರು + 1, 1 << g.ಬೇರು, 0);

                // also look at root bit codes with completions at root + 1
                // bits (not saved in num, since complete), just in case
                ಯದಿ (g.ಅಂಕೆ[index - 1] && n <= ಎಡ << 1)
                    examine((n - ಎಡ) << 1, (n - ಎಡ) << 1, g.ಬೇರು + 1,
                            1 << g.ಬೇರು, 0);
            }

    // done
    printf("maximum of %d table entries for root = %d\n", g.large, g.ಬೇರು);
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
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) {
    // set up globals for cleanup()
    g.code = ರಿಕ್ತ;
    g.ಅಂಕೆ = ರಿಕ್ತ;
    g.done = ರಿಕ್ತ;
    string_init(&g.out);

    // get arguments -- default to the deflate literal/length code
    ಪೂರ್ಣಾಙ್ಕ syms = 286;
    g.ಬೇರು = 9;
    g.max = 15;
    ಯದಿ (argc > 1) {
        syms = atoi(argv[1]);
        ಯದಿ (argc > 2) {
            g.ಬೇರು = atoi(argv[2]);
            ಯದಿ (argc > 3)
                g.max = atoi(argv[3]);
        }
    }
    ಯದಿ (argc > 4 || syms < 2 || g.ಬೇರು < 1 || g.max < 1) {
        fputs("invalid arguments, need: [sym >= 2 [root >= 1 [max >= 1]]]\n",
              stderr);
        ಪ್ರತ್ಯಾಗಮ 1;
    }

    // if not restricting the code length, the longest is syms - 1
    ಯದಿ (g.max > syms - 1)
        g.max = syms - 1;

    // determine the number of bits in a code_t
    ಪೂರ್ಣಾಙ್ಕ bits = 0;
    ಪ್ರತಿ (code_t word = 1; word; word <<= 1)
        bits++;

    // make sure that the calculation of most will not overflow
    ಯದಿ (g.max > bits || (code_t)(syms - 2) >= ((code_t)-1 >> (g.max - 1))) {
        fputs("abort: code length too long for internal types\n", stderr);
        ಪ್ರತ್ಯಾಗಮ 1;
    }

    // reject impossible code requests
    ಯದಿ ((code_t)(syms - 1) > ((code_t)1 << g.max) - 1) {
        fprintf(stderr, "%d symbols cannot be coded in %d bits\n",
                syms, g.max);
        ಪ್ರತ್ಯಾಗಮ 1;
    }

    // allocate code vector
    g.code = calloc(g.max + 1, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
    ಪ್ರತಿಜ್ಞಾ(g.code != ರಿಕ್ತ && "out of memory");

    // determine size of saved results array, checking for overflows,
    // allocate and clear the array (set all to zero with calloc())
    ಯದಿ (syms == 2)              // iff max == 1
        g.ಅಂಕೆ = ರಿಕ್ತ;           // won't be saving any results
    ಅನ್ಯಥಾ {
        g.size = syms >> 1;
        ಪೂರ್ಣಾಙ್ಕ n = (syms - 1) >> 1;
        ಪ್ರತಿಜ್ಞಾ(g.size <= (size_t)-1 / n && "overflow");
        g.size *= n;
        n = g.max - 1;
        ಪ್ರತಿಜ್ಞಾ(g.size <= (size_t)-1 / n && "overflow");
        g.size *= n;
        g.ಅಂಕೆ = calloc(g.size, ಪರಿಮಾಣ(big_t));
        ಪ್ರತಿಜ್ಞಾ(g.ಅಂಕೆ != ರಿಕ್ತ && "out of memory");
    }

    // count possible codes for all numbers of symbols, add up counts
    big_t sum = 0;
    ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ n = 2; n <= syms; n++) {
        big_t got = ಎಣಿಕೆ(n, 2, 1);
        sum += got;
        ಪ್ರತಿಜ್ಞಾ(got != (big_t)-1 && sum >= got && "overflow");
    }
    printf("%"PRIbig" total codes for 2 to %d symbols", sum, syms);
    ಯದಿ (g.max < syms - 1)
        printf(" (%d-bit length limit)\n", g.max);
    ಅನ್ಯಥಾ
        puts(" (no length limit)");

    // allocate and clear done array for been_here()
    ಯದಿ (syms == 2)
        g.done = ರಿಕ್ತ;
    ಅನ್ಯಥಾ {
        g.done = calloc(g.size, ಪರಿಮಾಣ(ಸಂರಚನಾ tab));
        ಪ್ರತಿಜ್ಞಾ(g.done != ರಿಕ್ತ && "out of memory");
    }

    // find and show maximum inflate table usage
    ಯದಿ (g.ಬೇರು > g.max)             // reduce root to max length
        g.ಬೇರು = g.max;
    ಯದಿ ((code_t)syms < ((code_t)1 << (g.ಬೇರು + 1)))
        enough(syms);
    ಅನ್ಯಥಾ
        fputs("cannot handle minimum code lengths > root", stderr);

    // done
    cleanup();
    ಪ್ರತ್ಯಾಗಮ 0;
}
