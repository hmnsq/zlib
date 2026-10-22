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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <assert.h>

#परिभाषय local स्थिर

// Special data types.
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ big_t;   // type for code counting
#परिभाषय PRIbig "llu"        // printf format for big_t
प्रकारनिर्देश big_t संहिता_t;       // type for bit pattern counting
संरचना tab {                // type for been-here check
    size_t len;             // allocated length of bit vector in octets
    वर्ण *vec;              // allocated bit vector
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
प्रकारनिर्देश संरचना {
    वर्ण *str;          // pointer to allocated string
    size_t आकार;        // size of allocation
    size_t len;         // length of string, not including terminating zero
} string_t;

// Clear a string_t.
local शून्य string_clear(string_t *s) {
    s->str[0] = 0;
    s->len = 0;
}

// Initialize a string_t.
local शून्य string_init(string_t *s) {
    s->आकार = 16;
    s->str = स्मृतिं_देहि(s->आकार);
    प्रतिज्ञा(s->str != रिक्त && "out of memory");
    string_clear(s);
}

// Release the allocation of a string_t.
local शून्य string_free(string_t *s) {
    मुक्त(s->str);
    s->str = रिक्त;
    s->आकार = 0;
    s->len = 0;
}

// Save the results of printf with fmt and the subsequent argument list to s.
// Each call appends to s. The allocated space for s is increased as needed.
local शून्य string_printf(string_t *s, वर्ण *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t len = s->len;
    पूर्णाङ्क ret = vsnprintf(s->str + len, s->आकार - len, fmt, ap);
    प्रतिज्ञा(ret >= 0 && "out of memory");
    s->len += ret;
    यदि (s->आकार < s->len + 1) {
        कुरु {
            s->आकार <<= 1;
            प्रतिज्ञा(s->आकार != 0 && "overflow");
        } यावत् (s->आकार < s->len + 1);
        s->str = realloc(s->str, s->आकार);
        प्रतिज्ञा(s->str != रिक्त && "out of memory");
        vsnprintf(s->str + len, s->आकार - len, fmt, ap);
    }
    va_end(ap);
}

// Globals to avoid propagating constants or constant pointers recursively.
संरचना {
    पूर्णाङ्क max;            // maximum allowed bit length for the codes
    पूर्णाङ्क मूलम्;           // size of base code table in bits
    पूर्णाङ्क large;          // largest code table so far
    size_t आकार;        // number of elements in num and done
    big_t tot;          // total number of codes with maximum tables size
    string_t बहिर्गमनम्;       // display of subcodes for maximum tables size
    पूर्णाङ्क *संहिता;          // number of symbols assigned to each bit length
    big_t *अङ्कः;         // saved results array for code counting
    संरचना tab *समाप्तम्_2;   // states already evaluated array
} g;

// Index function for num[] and done[].
local अन्तःस्थ size_t map(पूर्णाङ्क syms, पूर्णाङ्क वामः, पूर्णाङ्क len) {
    प्रत्यागम ((size_t)((syms - 1) >> 1) * ((syms - 2) >> 1) +
            (वामः >> 1) - 1) * (g.max - 1) +
           len - 1;
}

// Free allocated space in globals.
local शून्य cleanup(शून्य) {
    यदि (g.समाप्तम्_2 != रिक्त) {
        प्रति (size_t n = 0; n < g.आकार; n++)
            यदि (g.समाप्तम्_2[n].len)
                मुक्त(g.समाप्तम्_2[n].vec);
        g.आकार = 0;
        मुक्त(g.समाप्तम्_2);   g.समाप्तम्_2 = रिक्त;
    }
    मुक्त(g.अङ्कः);    g.अङ्कः = रिक्त;
    मुक्त(g.संहिता);   g.संहिता = रिक्त;
    string_free(&g.बहिर्गमनम्);
}

// Return the number of possible prefix codes using bit patterns of lengths len
// through max inclusive, coding syms symbols, with left bit patterns of length
// len unused -- return -1 if there is an overflow in the counting. Keep a
// record of previous results in num to prevent repeating the same calculation.
local big_t गणनम्(पूर्णाङ्क syms, पूर्णाङ्क वामः, पूर्णाङ्क len) {
    // see if only one possible code
    यदि (syms == वामः)
        प्रत्यागम 1;

    // note and verify the expected state
    प्रतिज्ञा(syms > वामः && वामः > 0 && len < g.max);

    // see if we've done this one already
    size_t सूचकांक = map(syms, वामः, len);
    big_t got = g.अङ्कः[सूचकांक];
    यदि (got)
        प्रत्यागम got;         // we have -- return the saved result

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    पूर्णाङ्क न्यूनतमम् = (वामः << 1) - syms;
    यदि (न्यूनतमम् < 0)
        न्यूनतमम् = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    पूर्णाङ्क most = (((संहिता_t)वामः << (g.max - len)) - syms) /
               (((संहिता_t)1 << (g.max - len)) - 1);

    // count all possible codes from this juncture and add them up
    big_t योग = 0;
    प्रति (पूर्णाङ्क use = न्यूनतमम्; use <= most; use++) {
        got = गणनम्(syms - use, (वामः - use) << 1, len + 1);
        योग += got;
        यदि (got == (big_t)-1 || योग < got)      //अतिप्रवाहः
            प्रत्यागम (big_t)-1;
    }

    // verify that all recursive calls are productive
    प्रतिज्ञा(योग != 0);

    // save the result and return it
    g.अङ्कः[सूचकांक] = योग;
    प्रत्यागम योग;
}

// Return true if we've been here before, set to true if not. Set a bit in a
// bit vector to indicate visiting this state. Each (syms,len,left) state has a
// variable size bit vector indexed by (mem,rem). The bit vector is lengthened
// as needed to allow setting the (mem,rem) bit.
local पूर्णाङ्क been_अत्र(पूर्णाङ्क syms, पूर्णाङ्क वामः, पूर्णाङ्क len, पूर्णाङ्क mem, पूर्णाङ्क rem) {
    // point to vector for (syms,left,len), bit in vector for (mem,rem)
    size_t सूचकांक = map(syms, वामः, len);
    mem -= 1 << g.मूलम्;             // mem always includes the root table
    mem >>= 1;                      // mem and rem are always even
    rem >>= 1;
    size_t विस्थापनम् = (mem >> 3) + rem;
    विस्थापनम् = ((विस्थापनम् * (विस्थापनम् + 1)) >> 1) + rem;
    पूर्णाङ्क bit = 1 << (mem & 7);

    // see if we've been here
    size_t दीर्घता = g.समाप्तम्_2[सूचकांक].len;
    यदि (विस्थापनम् < दीर्घता && (g.समाप्तम्_2[सूचकांक].vec[विस्थापनम्] & bit) != 0)
        प्रत्यागम 1;       // done this!

    // we haven't been here before -- set the bit to show we have now

    // see if we need to lengthen the vector in order to set the bit
    यदि (दीर्घता <= विस्थापनम्) {
        // if we have one already, enlarge it, zero out the appended space
        वर्ण *सदिशम्;
        यदि (दीर्घता) {
            कुरु {
                दीर्घता <<= 1;
            } यावत् (दीर्घता <= विस्थापनम्);
            सदिशम् = realloc(g.समाप्तम्_2[सूचकांक].vec, दीर्घता);
            प्रतिज्ञा(सदिशम् != रिक्त && "out of memory");
            स्मृतिं_पूरय(सदिशम् + g.समाप्तम्_2[सूचकांक].len, 0, दीर्घता - g.समाप्तम्_2[सूचकांक].len);
        }

        // otherwise we need to make a new vector and zero it out
        अन्यथा {
            दीर्घता = 16;
            यावत् (दीर्घता <= विस्थापनम्)
                दीर्घता <<= 1;
            सदिशम् = शून्यस्मृतिं_देहि(दीर्घता, 1);
            प्रतिज्ञा(सदिशम् != रिक्त && "out of memory");
        }

        // install the new vector
        g.समाप्तम्_2[सूचकांक].len = दीर्घता;
        g.समाप्तम्_2[सूचकांक].vec = सदिशम्;
    }

    // set the bit
    g.समाप्तम्_2[सूचकांक].vec[विस्थापनम्] |= bit;
    प्रत्यागम 0;
}

// Examine all possible codes from the given node (syms, len, left). Compute
// the amount of memory required to build inflate's decoding tables, where the
// number of code structures used so far is mem, and the number remaining in
// the current sub-table is rem.
local शून्य examine(पूर्णाङ्क syms, पूर्णाङ्क वामः, पूर्णाङ्क len, पूर्णाङ्क mem, पूर्णाङ्क rem) {
    // see if we have a complete code
    यदि (syms == वामः) {
        // set the last code entry
        g.संहिता[len] = वामः;

        // complete computation of memory used by this code
        यावत् (rem < वामः) {
            वामः -= rem;
            rem = 1 << (len - g.मूलम्);
            mem += rem;
        }
        प्रतिज्ञा(rem == वामः);

        // if this is at the maximum, show the sub-code
        यदि (mem >= g.large) {
            // if this is a new maximum, update the maximum and clear out the
            // printed sub-codes from the previous maximum
            यदि (mem > g.large) {
                g.large = mem;
                string_clear(&g.बहिर्गमनम्);
            }

            // compute the starting state for this sub-code
            syms = 0;
            वामः = 1 << g.max;
            प्रति (पूर्णाङ्क द्वयङ्काः = g.max; द्वयङ्काः > g.मूलम्; द्वयङ्काः--) {
                syms += g.संहिता[द्वयङ्काः];
                वामः -= g.संहिता[द्वयङ्काः];
                प्रतिज्ञा((वामः & 1) == 0);
                वामः >>= 1;
            }

            // print the starting state and the resulting sub-code to g.out
            string_printf(&g.बहिर्गमनम्, "<%u, %u, %u>:",
                          syms, g.मूलम् + 1, ((1 << g.मूलम्) - वामः) << 1);
            प्रति (पूर्णाङ्क द्वयङ्काः = g.मूलम् + 1; द्वयङ्काः <= g.max; द्वयङ्काः++)
                यदि (g.संहिता[द्वयङ्काः])
                    string_printf(&g.बहिर्गमनम्, " %d[%d]", g.संहिता[द्वयङ्काः], द्वयङ्काः);
            string_printf(&g.बहिर्गमनम्, "\n");
        }

        // remove entries as we drop back down in the recursion
        g.संहिता[len] = 0;
        प्रत्यागम;
    }

    // prune the tree if we can
    यदि (been_अत्र(syms, वामः, len, mem, rem))
        प्रत्यागम;

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    पूर्णाङ्क न्यूनतमम् = (वामः << 1) - syms;
    यदि (न्यूनतमम् < 0)
        न्यूनतमम् = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    पूर्णाङ्क most = (((संहिता_t)वामः << (g.max - len)) - syms) /
               (((संहिता_t)1 << (g.max - len)) - 1);

    // occupy least table spaces, creating new sub-tables as needed
    पूर्णाङ्क use = न्यूनतमम्;
    यावत् (rem < use) {
        use -= rem;
        rem = 1 << (len - g.मूलम्);
        mem += rem;
    }
    rem -= use;

    // examine codes from here, updating table space as we go
    प्रति (use = न्यूनतमम्; use <= most; use++) {
        g.संहिता[len] = use;
        examine(syms - use, (वामः - use) << 1, len + 1,
                mem + (rem ? 1 << (len - g.मूलम्) : 0), rem << 1);
        यदि (rem == 0) {
            rem = 1 << (len - g.मूलम्);
            mem += rem;
        }
        rem--;
    }

    // remove entries as we drop back down in the recursion
    g.संहिता[len] = 0;
}

// Look at all sub-codes starting with root + 1 bits. Look at only the valid
// intermediate code states (syms, left, len). For each completed code,
// calculate the amount of memory required by inflate to build the decoding
// tables. Find the maximum amount of memory required and show the codes that
// require that maximum.
local शून्य enough(पूर्णाङ्क syms) {
    // clear code
    प्रति (पूर्णाङ्क n = 0; n <= g.max; n++)
        g.संहिता[n] = 0;

    // look at all (root + 1) bit and longer codes
    string_clear(&g.बहिर्गमनम्);           // empty saved results
    g.large = 1 << g.मूलम्;          // base table
    यदि (g.मूलम् < g.max)             // otherwise, there's only a base table
        प्रति (पूर्णाङ्क n = 3; n <= syms; n++)
            प्रति (पूर्णाङ्क वामः = 2; वामः < n; वामः += 2) {
                // look at all reachable (root + 1) bit nodes, and the
                // resulting codes (complete at root + 2 or more)
                size_t सूचकांक = map(n, वामः, g.मूलम् + 1);
                यदि (g.मूलम् + 1 < g.max && g.अङ्कः[सूचकांक]) // reachable node
                    examine(n, वामः, g.मूलम् + 1, 1 << g.मूलम्, 0);

                // also look at root bit codes with completions at root + 1
                // bits (not saved in num, since complete), just in case
                यदि (g.अङ्कः[सूचकांक - 1] && n <= वामः << 1)
                    examine((n - वामः) << 1, (n - वामः) << 1, g.मूलम् + 1,
                            1 << g.मूलम्, 0);
            }

    //समाप्तम्
    प्रारूप_लिख("maximum of %d table entries for root = %d\n", g.large, g.मूलम्);
    fputs(g.बहिर्गमनम्.str, stdout);
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
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv) {
    // set up globals for cleanup()
    g.संहिता = रिक्त;
    g.अङ्कः = रिक्त;
    g.समाप्तम्_2 = रिक्त;
    string_init(&g.बहिर्गमनम्);

    // get arguments -- default to the deflate literal/length code
    पूर्णाङ्क syms = 286;
    g.मूलम् = 9;
    g.max = 15;
    यदि (argc > 1) {
        syms = atoi(argv[1]);
        यदि (argc > 2) {
            g.मूलम् = atoi(argv[2]);
            यदि (argc > 3)
                g.max = atoi(argv[3]);
        }
    }
    यदि (argc > 4 || syms < 2 || g.मूलम् < 1 || g.max < 1) {
        fputs("invalid arguments, need: [sym >= 2 [root >= 1 [max >= 1]]]\n",
              stderr);
        प्रत्यागम 1;
    }

    // if not restricting the code length, the longest is syms - 1
    यदि (g.max > syms - 1)
        g.max = syms - 1;

    // determine the number of bits in a code_t
    पूर्णाङ्क द्वयङ्काः = 0;
    प्रति (संहिता_t word = 1; word; word <<= 1)
        द्वयङ्काः++;

    // make sure that the calculation of most will not overflow
    यदि (g.max > द्वयङ्काः || (संहिता_t)(syms - 2) >= ((संहिता_t)-1 >> (g.max - 1))) {
        fputs("abort: code length too long for internal types\n", stderr);
        प्रत्यागम 1;
    }

    // reject impossible code requests
    यदि ((संहिता_t)(syms - 1) > ((संहिता_t)1 << g.max) - 1) {
        fprintf(stderr, "%d symbols cannot be coded in %d bits\n",
                syms, g.max);
        प्रत्यागम 1;
    }

    // allocate code vector
    g.संहिता = शून्यस्मृतिं_देहि(g.max + 1, परिमाण(पूर्णाङ्क));
    प्रतिज्ञा(g.संहिता != रिक्त && "out of memory");

    // determine size of saved results array, checking for overflows,
    // allocate and clear the array (set all to zero with calloc())
    यदि (syms == 2)              // iff max == 1
        g.अङ्कः = रिक्त;           // won't be saving any results
    अन्यथा {
        g.आकार = syms >> 1;
        पूर्णाङ्क n = (syms - 1) >> 1;
        प्रतिज्ञा(g.आकार <= (size_t)-1 / n && "overflow");
        g.आकार *= n;
        n = g.max - 1;
        प्रतिज्ञा(g.आकार <= (size_t)-1 / n && "overflow");
        g.आकार *= n;
        g.अङ्कः = शून्यस्मृतिं_देहि(g.आकार, परिमाण(big_t));
        प्रतिज्ञा(g.अङ्कः != रिक्त && "out of memory");
    }

    // count possible codes for all numbers of symbols, add up counts
    big_t योग = 0;
    प्रति (पूर्णाङ्क n = 2; n <= syms; n++) {
        big_t got = गणनम्(n, 2, 1);
        योग += got;
        प्रतिज्ञा(got != (big_t)-1 && योग >= got && "overflow");
    }
    प्रारूप_लिख("%"PRIbig" total codes for 2 to %d symbols", योग, syms);
    यदि (g.max < syms - 1)
        प्रारूप_लिख(" (%d-bit length limit)\n", g.max);
    अन्यथा
        पङ्क्तिं_लिख(" (no length limit)");

    // allocate and clear done array for been_here()
    यदि (syms == 2)
        g.समाप्तम्_2 = रिक्त;
    अन्यथा {
        g.समाप्तम्_2 = शून्यस्मृतिं_देहि(g.आकार, परिमाण(संरचना tab));
        प्रतिज्ञा(g.समाप्तम्_2 != रिक्त && "out of memory");
    }

    // find and show maximum inflate table usage
    यदि (g.मूलम् > g.max)             // reduce root to max length
        g.मूलम् = g.max;
    यदि ((संहिता_t)syms < ((संहिता_t)1 << (g.मूलम् + 1)))
        enough(syms);
    अन्यथा
        fputs("cannot handle minimum code lengths > root", stderr);

    //समाप्तम्
    cleanup();
    प्रत्यागम 0;
}
