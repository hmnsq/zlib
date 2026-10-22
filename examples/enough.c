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

#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <stdarg.h>
#समावेश <assert.h>

#घोषणा local अटल

// Special data types.
प्रकार अचिन्हित दीर्घ दीर्घ big_t;   // type for code counting
#घोषणा PRIbig "llu"        // printf format for big_t
प्रकार big_t code_t;       // type for bit pattern counting
काष्ठा tab {                // type for been-here check
    माप_प्रकार len;             // allocated length of bit vector in octets
    अक्षर *vec;              // allocated bit vector
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
प्रकार काष्ठा {
    अक्षर *str;          // pointer to allocated string
    माप_प्रकार आकार;        // size of allocation
    माप_प्रकार len;         // length of string, not including terminating zero
} string_t;

// Clear a string_t.
local व्योम string_clear(string_t *s) {
    s->str[0] = 0;
    s->len = 0;
}

// Initialize a string_t.
local व्योम string_init(string_t *s) {
    s->आकार = 16;
    s->str = दो_स्मृति(s->आकार);
    निश्चित(s->str != शून्य && "out of memory");
    string_clear(s);
}

// Release the allocation of a string_t.
local व्योम string_free(string_t *s) {
    मुक्त(s->str);
    s->str = शून्य;
    s->आकार = 0;
    s->len = 0;
}

// Save the results of printf with fmt and the subsequent argument list to s.
// Each call appends to s. The allocated space for s is increased as needed.
local व्योम string_printf(string_t *s, अक्षर *fmt, ...) {
    बहु_सूची ap;
    बहु_शुरू(ap, fmt);
    माप_प्रकार len = s->len;
    पूर्णांक ret = vsnprintf(s->str + len, s->आकार - len, fmt, ap);
    निश्चित(ret >= 0 && "out of memory");
    s->len += ret;
    अगर (s->आकार < s->len + 1) {
        करो {
            s->आकार <<= 1;
            निश्चित(s->आकार != 0 && "overflow");
        } जबतक (s->आकार < s->len + 1);
        s->str = पुनः_स्मृति(s->str, s->आकार);
        निश्चित(s->str != शून्य && "out of memory");
        vsnprintf(s->str + len, s->आकार - len, fmt, ap);
    }
    बहु_पूर्ण(ap);
}

// Globals to avoid propagating constants or constant pointers recursively.
काष्ठा {
    पूर्णांक max;            // maximum allowed bit length for the codes
    पूर्णांक मूल;           // size of base code table in bits
    पूर्णांक large;          // largest code table so far
    माप_प्रकार आकार;        // number of elements in num and done
    big_t tot;          // total number of codes with maximum tables size
    string_t out;       // display of subcodes for maximum tables size
    पूर्णांक *code;          // number of symbols assigned to each bit length
    big_t *अंक;         // saved results array for code counting
    काष्ठा tab *done;   // states already evaluated array
} g;

// Index function for num[] and done[].
local अंतरभूत माप_प्रकार map(पूर्णांक syms, पूर्णांक बायाँ, पूर्णांक len) {
    वापस ((माप_प्रकार)((syms - 1) >> 1) * ((syms - 2) >> 1) +
            (बायाँ >> 1) - 1) * (g.max - 1) +
           len - 1;
}

// Free allocated space in globals.
local व्योम cleanup(व्योम) {
    अगर (g.done != शून्य) {
        क्रम (माप_प्रकार n = 0; n < g.आकार; n++)
            अगर (g.done[n].len)
                मुक्त(g.done[n].vec);
        g.आकार = 0;
        मुक्त(g.done);   g.done = शून्य;
    }
    मुक्त(g.अंक);    g.अंक = शून्य;
    मुक्त(g.code);   g.code = शून्य;
    string_free(&g.out);
}

// Return the number of possible prefix codes using bit patterns of lengths len
// through max inclusive, coding syms symbols, with left bit patterns of length
// len unused -- return -1 if there is an overflow in the counting. Keep a
// record of previous results in num to prevent repeating the same calculation.
local big_t गिनती(पूर्णांक syms, पूर्णांक बायाँ, पूर्णांक len) {
    // see if only one possible code
    अगर (syms == बायाँ)
        वापस 1;

    // note and verify the expected state
    निश्चित(syms > बायाँ && बायाँ > 0 && len < g.max);

    // see if we've done this one already
    माप_प्रकार सूचकांक = map(syms, बायाँ, len);
    big_t got = g.अंक[सूचकांक];
    अगर (got)
        वापस got;         // we have -- return the saved result

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    पूर्णांक least = (बायाँ << 1) - syms;
    अगर (least < 0)
        least = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    पूर्णांक most = (((code_t)बायाँ << (g.max - len)) - syms) /
               (((code_t)1 << (g.max - len)) - 1);

    // count all possible codes from this juncture and add them up
    big_t योग = 0;
    क्रम (पूर्णांक use = least; use <= most; use++) {
        got = गिनती(syms - use, (बायाँ - use) << 1, len + 1);
        योग += got;
        अगर (got == (big_t)-1 || योग < got)      // overflow
            वापस (big_t)-1;
    }

    // verify that all recursive calls are productive
    निश्चित(योग != 0);

    // save the result and return it
    g.अंक[सूचकांक] = योग;
    वापस योग;
}

// Return true if we've been here before, set to true if not. Set a bit in a
// bit vector to indicate visiting this state. Each (syms,len,left) state has a
// variable size bit vector indexed by (mem,rem). The bit vector is lengthened
// as needed to allow setting the (mem,rem) bit.
local पूर्णांक been_here(पूर्णांक syms, पूर्णांक बायाँ, पूर्णांक len, पूर्णांक mem, पूर्णांक rem) {
    // point to vector for (syms,left,len), bit in vector for (mem,rem)
    माप_प्रकार सूचकांक = map(syms, बायाँ, len);
    mem -= 1 << g.मूल;             // mem always includes the root table
    mem >>= 1;                      // mem and rem are always even
    rem >>= 1;
    माप_प्रकार offset = (mem >> 3) + rem;
    offset = ((offset * (offset + 1)) >> 1) + rem;
    पूर्णांक bit = 1 << (mem & 7);

    // see if we've been here
    माप_प्रकार लंबाई = g.done[सूचकांक].len;
    अगर (offset < लंबाई && (g.done[सूचकांक].vec[offset] & bit) != 0)
        वापस 1;       // done this!

    // we haven't been here before -- set the bit to show we have now

    // see if we need to lengthen the vector in order to set the bit
    अगर (लंबाई <= offset) {
        // if we have one already, enlarge it, zero out the appended space
        अक्षर *vector;
        अगर (लंबाई) {
            करो {
                लंबाई <<= 1;
            } जबतक (लंबाई <= offset);
            vector = पुनः_स्मृति(g.done[सूचकांक].vec, लंबाई);
            निश्चित(vector != शून्य && "out of memory");
            स_रखो(vector + g.done[सूचकांक].len, 0, लंबाई - g.done[सूचकांक].len);
        }

        // otherwise we need to make a new vector and zero it out
        अन्यथा {
            लंबाई = 16;
            जबतक (लंबाई <= offset)
                लंबाई <<= 1;
            vector = सुस्मृति(लंबाई, 1);
            निश्चित(vector != शून्य && "out of memory");
        }

        // install the new vector
        g.done[सूचकांक].len = लंबाई;
        g.done[सूचकांक].vec = vector;
    }

    // set the bit
    g.done[सूचकांक].vec[offset] |= bit;
    वापस 0;
}

// Examine all possible codes from the given node (syms, len, left). Compute
// the amount of memory required to build inflate's decoding tables, where the
// number of code structures used so far is mem, and the number remaining in
// the current sub-table is rem.
local व्योम examine(पूर्णांक syms, पूर्णांक बायाँ, पूर्णांक len, पूर्णांक mem, पूर्णांक rem) {
    // see if we have a complete code
    अगर (syms == बायाँ) {
        // set the last code entry
        g.code[len] = बायाँ;

        // complete computation of memory used by this code
        जबतक (rem < बायाँ) {
            बायाँ -= rem;
            rem = 1 << (len - g.मूल);
            mem += rem;
        }
        निश्चित(rem == बायाँ);

        // if this is at the maximum, show the sub-code
        अगर (mem >= g.large) {
            // if this is a new maximum, update the maximum and clear out the
            // printed sub-codes from the previous maximum
            अगर (mem > g.large) {
                g.large = mem;
                string_clear(&g.out);
            }

            // compute the starting state for this sub-code
            syms = 0;
            बायाँ = 1 << g.max;
            क्रम (पूर्णांक bits = g.max; bits > g.मूल; bits--) {
                syms += g.code[bits];
                बायाँ -= g.code[bits];
                निश्चित((बायाँ & 1) == 0);
                बायाँ >>= 1;
            }

            // print the starting state and the resulting sub-code to g.out
            string_printf(&g.out, "<%u, %u, %u>:",
                          syms, g.मूल + 1, ((1 << g.मूल) - बायाँ) << 1);
            क्रम (पूर्णांक bits = g.मूल + 1; bits <= g.max; bits++)
                अगर (g.code[bits])
                    string_printf(&g.out, " %d[%d]", g.code[bits], bits);
            string_printf(&g.out, "\n");
        }

        // remove entries as we drop back down in the recursion
        g.code[len] = 0;
        वापस;
    }

    // prune the tree if we can
    अगर (been_here(syms, बायाँ, len, mem, rem))
        वापस;

    // we need to use at least this many bit patterns so that the code won't be
    // incomplete at the next length (more bit patterns than symbols)
    पूर्णांक least = (बायाँ << 1) - syms;
    अगर (least < 0)
        least = 0;

    // we can use at most this many bit patterns, lest there not be enough
    // available for the remaining symbols at the maximum length (if there were
    // no limit to the code length, this would become: most = left - 1)
    पूर्णांक most = (((code_t)बायाँ << (g.max - len)) - syms) /
               (((code_t)1 << (g.max - len)) - 1);

    // occupy least table spaces, creating new sub-tables as needed
    पूर्णांक use = least;
    जबतक (rem < use) {
        use -= rem;
        rem = 1 << (len - g.मूल);
        mem += rem;
    }
    rem -= use;

    // examine codes from here, updating table space as we go
    क्रम (use = least; use <= most; use++) {
        g.code[len] = use;
        examine(syms - use, (बायाँ - use) << 1, len + 1,
                mem + (rem ? 1 << (len - g.मूल) : 0), rem << 1);
        अगर (rem == 0) {
            rem = 1 << (len - g.मूल);
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
local व्योम enough(पूर्णांक syms) {
    // clear code
    क्रम (पूर्णांक n = 0; n <= g.max; n++)
        g.code[n] = 0;

    // look at all (root + 1) bit and longer codes
    string_clear(&g.out);           // empty saved results
    g.large = 1 << g.मूल;          // base table
    अगर (g.मूल < g.max)             // otherwise, there's only a base table
        क्रम (पूर्णांक n = 3; n <= syms; n++)
            क्रम (पूर्णांक बायाँ = 2; बायाँ < n; बायाँ += 2) {
                // look at all reachable (root + 1) bit nodes, and the
                // resulting codes (complete at root + 2 or more)
                माप_प्रकार सूचकांक = map(n, बायाँ, g.मूल + 1);
                अगर (g.मूल + 1 < g.max && g.अंक[सूचकांक]) // reachable node
                    examine(n, बायाँ, g.मूल + 1, 1 << g.मूल, 0);

                // also look at root bit codes with completions at root + 1
                // bits (not saved in num, since complete), just in case
                अगर (g.अंक[सूचकांक - 1] && n <= बायाँ << 1)
                    examine((n - बायाँ) << 1, (n - बायाँ) << 1, g.मूल + 1,
                            1 << g.मूल, 0);
            }

    // done
    म_लिखो("maximum of %d table entries for root = %d\n", g.large, g.मूल);
    ख_माला_दो(g.out.str, मानक_निकास);
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
पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv) {
    // set up globals for cleanup()
    g.code = शून्य;
    g.अंक = शून्य;
    g.done = शून्य;
    string_init(&g.out);

    // get arguments -- default to the deflate literal/length code
    पूर्णांक syms = 286;
    g.मूल = 9;
    g.max = 15;
    अगर (argc > 1) {
        syms = म_से_प(argv[1]);
        अगर (argc > 2) {
            g.मूल = म_से_प(argv[2]);
            अगर (argc > 3)
                g.max = म_से_प(argv[3]);
        }
    }
    अगर (argc > 4 || syms < 2 || g.मूल < 1 || g.max < 1) {
        ख_माला_दो("invalid arguments, need: [sym >= 2 [root >= 1 [max >= 1]]]\n",
              मानक_त्रुटि);
        वापस 1;
    }

    // if not restricting the code length, the longest is syms - 1
    अगर (g.max > syms - 1)
        g.max = syms - 1;

    // determine the number of bits in a code_t
    पूर्णांक bits = 0;
    क्रम (code_t word = 1; word; word <<= 1)
        bits++;

    // make sure that the calculation of most will not overflow
    अगर (g.max > bits || (code_t)(syms - 2) >= ((code_t)-1 >> (g.max - 1))) {
        ख_माला_दो("abort: code length too long for internal types\n", मानक_त्रुटि);
        वापस 1;
    }

    // reject impossible code requests
    अगर ((code_t)(syms - 1) > ((code_t)1 << g.max) - 1) {
        ख_लिखो(मानक_त्रुटि, "%d symbols cannot be coded in %d bits\n",
                syms, g.max);
        वापस 1;
    }

    // allocate code vector
    g.code = सुस्मृति(g.max + 1, माप(पूर्णांक));
    निश्चित(g.code != शून्य && "out of memory");

    // determine size of saved results array, checking for overflows,
    // allocate and clear the array (set all to zero with calloc())
    अगर (syms == 2)              // iff max == 1
        g.अंक = शून्य;           // won't be saving any results
    अन्यथा {
        g.आकार = syms >> 1;
        पूर्णांक n = (syms - 1) >> 1;
        निश्चित(g.आकार <= (माप_प्रकार)-1 / n && "overflow");
        g.आकार *= n;
        n = g.max - 1;
        निश्चित(g.आकार <= (माप_प्रकार)-1 / n && "overflow");
        g.आकार *= n;
        g.अंक = सुस्मृति(g.आकार, माप(big_t));
        निश्चित(g.अंक != शून्य && "out of memory");
    }

    // count possible codes for all numbers of symbols, add up counts
    big_t योग = 0;
    क्रम (पूर्णांक n = 2; n <= syms; n++) {
        big_t got = गिनती(n, 2, 1);
        योग += got;
        निश्चित(got != (big_t)-1 && योग >= got && "overflow");
    }
    म_लिखो("%"PRIbig" total codes for 2 to %d symbols", योग, syms);
    अगर (g.max < syms - 1)
        म_लिखो(" (%d-bit length limit)\n", g.max);
    अन्यथा
        माला_दो(" (no length limit)");

    // allocate and clear done array for been_here()
    अगर (syms == 2)
        g.done = शून्य;
    अन्यथा {
        g.done = सुस्मृति(g.आकार, माप(काष्ठा tab));
        निश्चित(g.done != शून्य && "out of memory");
    }

    // find and show maximum inflate table usage
    अगर (g.मूल > g.max)             // reduce root to max length
        g.मूल = g.max;
    अगर ((code_t)syms < ((code_t)1 << (g.मूल + 1)))
        enough(syms);
    अन्यथा
        ख_माला_दो("cannot handle minimum code lengths > root", मानक_त्रुटि);

    // done
    cleanup();
    वापस 0;
}
