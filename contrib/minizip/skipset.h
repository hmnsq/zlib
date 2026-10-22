/* skipset.h -- set operations using a skiplist
   Copyright (C) 2024-2026 Mark Adler
   See MiniZip_info.txt for the license.
 */

/*
  This implements a skiplist set, i.e. just keys, no data, with ~O(log n) time
  insert and search operations. The application defines the type of a key, and
  provides a function to compare two keys.

  This header is not definitions of functions found in another source file --
  it creates the set functions, with the application's key type, right where
  the #include is. Before this header is #included, these must be defined:

  1. A macro or typedef for set_key_t, the type of a key.
  2. A macro or function set_cmp(a, b) to compare two keys. The return values
     are < 0 for a < b, 0 for a == b, and > 0 for a > b.
  3. A macro or function set_drop(s, k) to release the key k's resources, if
     any, when doing a set_end() or set_clear(). s is a pointer to the set
     that key is in, for use with set_free() if desired.

  Example usage:

       typedef int set_key_t;
       #define set_cmp(a, b) ((a) < (b) ? -1 : (a) == (b) ? 0 : 1)
       #define set_drop(s, k)
       #include "skipset.h"

       int test(void) {        // return 0: good, 1: bad, -1: out of memory
           set_t set;
           if (setjmp(set.env))
               return -1;
           set_start(&set);
           set_insert(&set, 2);
           set_insert(&set, 1);
           set_insert(&set, 7);
           int bad = !set_found(&set, 2);
           bad = bad || set_found(&set, 5);
           set_end(&set);
           return bad;
       }

  Interface summary (see more details below):
  - set_t is the type of the set being operated on (a set_t pointer is passed)
  - set_start() initializes a new, empty set (initialize set.env first)
  - set_insert() inserts a new key into the set, or not if it's already there
  - set_found() determines whether or not a key is in the set
  - set_end() ends the use of the set, freeing all memory
  - set_clear() empties the set, equivalent to set_end() and then set_start()
  - set_ok() checks if set appears to be usable, i.e. started and not ended

  Auxiliary functions available to the application:
  - set_alloc() allocates memory with optional tracking (#define SET_TRACK)
  - set_free() deallocates memory allocated by set_alloc()
  - set_rand() returns 32 random bits (seeded by set_start())
 */

#यदि_अपरिभाषितम् SKIPSET_H
#परिभाषय SKIPSET_H

#अन्तर्भाव <stdlib.h>     /* realloc(), free(), NULL, size_t */
#अन्तर्भाव <stddef.h>     /* ptrdiff_t */
#अन्तर्भाव <setjmp.h>     /* jmp_buf, longjmp() */
#अन्तर्भाव <errno.h>      /* ENOMEM */
#अन्तर्भाव <time.h>       /* time(), clock() */
#अन्तर्भाव <assert.h>     /* assert.h */
#अन्तर्भाव "ints.h"       /* i16_t, ui32_t, ui64_t */

/*
  Structures and functions below noted as "--private--" should not be used by
  the application. set_t is partially private and partially public -- see the
  comments there.

  There is no POSIX random() in MSVC, and rand() is awful. For portability, we
  cannot rely on a library function for random numbers. Instead we use the
  fast and effective algorithm below, invented by Melissa O'Neill.
 */

/*
  *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / www.pcg-random.org
  Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
  --private-- Random number generator state.
 */
प्रकारनिर्देश संरचना {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} समुच्चयः_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
स्थिर शून्य समुच्चयः_बीजम्(समुच्चयः_rand_t *gen, ui64_t बीजम्, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (बीजम् + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
स्थिर शून्य समुच्चयः_uniq(समुच्चयः_rand_t *gen, ध्रुव शून्य *ptr) {
    समुच्चयः_बीजम्(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(रिक्त) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
स्थिर ui32_t समुच्चयः_rand(समुच्चयः_rand_t *gen) {
    ui32_t mix;
    पूर्णाङ्क rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    प्रत्यागम (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
प्रकारनिर्देश संरचना समुच्चयः_ग्रन्थिः_s समुच्चयः_ग्रन्थिः_t;
संरचना समुच्चयः_ग्रन्थिः_s {
    समुच्चयः_कुञ्चिका_t कुञ्चिका;          /* the key (not used for head or path) */
    i16_t आकार;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    समुच्चयः_ग्रन्थिः_t **दक्षिणः;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
प्रकारनिर्देश संरचना समुच्चयः_s {
    समुच्चयः_ग्रन्थिः_t *शीर्ष;       /* skiplist head -- no key, just links */
    समुच्चयः_ग्रन्थिः_t *मार्गः;       /* right[] is path to key from set_found() */
    समुच्चयः_ग्रन्थिः_t *ग्रन्थिः;       /* node under construction, in case of longjmp() */
    i16_t गभीरता;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    समुच्चयः_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#यदि_परिभाषितम् समुच्चयः_TRACK
    size_t allocs;          /* number of allocations */
    size_t स्मृतिः;          /* total size of allocated memory (>= requests) */
#यदि_अन्तः
} समुच्चयः_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#यदि_परिभाषितम् समुच्चयः_TRACK
/* Track the number of allocations and the total backing memory size. */
#  यदि defined(_WIN32)
#    include <malloc.h>
#    define समुच्चयः_ALLOC_आकार(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define समुच्चयः_ALLOC_आकार(ptr) malloc_आकार(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define समुच्चयः_ALLOC_आकार(ptr) malloc_usable_आकार(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define समुच्चयः_ALLOC_आकार(ptr) malloc_usable_आकार(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define समुच्चयः_ALLOC_आकार(ptr) malloc_usable_आकार(ptr)
#  अन्यथा     /* e.g. OpenBSD */
#    define समुच्चयः_ALLOC_आकार(ptr) 0
#  endif
/* With tracking. */
स्थिर शून्य *समुच्चयः_alloc(समुच्चयः_t *समुच्चयः, शून्य *ptr, size_t आकार) {
    size_t had = ptr == रिक्त ? 0 : समुच्चयः_ALLOC_आकार(ptr);
    शून्य *mem = realloc(ptr, आकार);
    यदि (mem == रिक्त)
        longjmp(समुच्चयः->env, ENOMEM);
    समुच्चयः->allocs += ptr == रिक्त;
    समुच्चयः->स्मृतिः += समुच्चयः_ALLOC_आकार(mem) - had;
    प्रत्यागम mem;
}
स्थिर शून्य समुच्चयः_free(समुच्चयः_t *समुच्चयः, शून्य *ptr) {
    यदि (ptr != रिक्त) {
        समुच्चयः->allocs--;
        समुच्चयः->स्मृतिः -= समुच्चयः_ALLOC_आकार(ptr);
        मुक्त(ptr);
    }
}
#अन्यथा
/* Without tracking. */
स्थिर शून्य *समुच्चयः_alloc(समुच्चयः_t *समुच्चयः, शून्य *ptr, size_t आकार) {
    शून्य *mem = realloc(ptr, आकार);
    यदि (mem == रिक्त)
        longjmp(समुच्चयः->env, ENOMEM);
    प्रत्यागम mem;
}
स्थिर शून्य समुच्चयः_free(समुच्चयः_t *समुच्चयः, शून्य *ptr) {
    (शून्य)समुच्चयः;
    मुक्त(ptr);
}
#यदि_अन्तः

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
स्थिर शून्य समुच्चयः_grow(समुच्चयः_t *समुच्चयः, समुच्चयः_ग्रन्थिः_t *ग्रन्थिः, पूर्णाङ्क want, पूर्णाङ्क fill) {
    पूर्णाङ्क i;
    यदि (ग्रन्थिः->आकार < want) {
        पूर्णाङ्क अधिकम् = ग्रन्थिः->आकार ? ग्रन्थिः->आकार : 1;
        यावत् (अधिकम् < want)
            अधिकम् <<= 1;
        ग्रन्थिः->दक्षिणः = समुच्चयः_alloc(समुच्चयः, ग्रन्थिः->दक्षिणः,
                                (size_t)अधिकम् * परिमाण(समुच्चयः_ग्रन्थिः_t *));
        ग्रन्थिः->आकार = (i16_t)अधिकम्;
    }
    यदि (fill)
        प्रति (i = ग्रन्थिः->fill; i < want; i++)
            ग्रन्थिः->दक्षिणः[i] = समुच्चयः->शीर्ष;
    ग्रन्थिः->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
स्थिर समुच्चयः_ग्रन्थिः_t *समुच्चयः_ग्रन्थिः(समुच्चयः_t *समुच्चयः) {
    समुच्चयः_ग्रन्थिः_t *ग्रन्थिः = समुच्चयः_alloc(समुच्चयः, रिक्त, परिमाण(समुच्चयः_ग्रन्थिः_t));
    ग्रन्थिः->आकार = 0;
    ग्रन्थिः->fill = 0;
    ग्रन्थिः->दक्षिणः = रिक्त;
    प्रत्यागम ग्रन्थिः;
}

/* --private-- Free the list linked from head, along with the keys. */
स्थिर शून्य समुच्चयः_sweep(समुच्चयः_t *समुच्चयः) {
    समुच्चयः_ग्रन्थिः_t *चरणः = समुच्चयः->शीर्ष->दक्षिणः[0];
    यावत् (चरणः != समुच्चयः->शीर्ष) {
        समुच्चयः_ग्रन्थिः_t *अग्र = चरणः->दक्षिणः[0];      /* save link to next node */
        समुच्चयः_drop(समुच्चयः, चरणः->कुञ्चिका);
        समुच्चयः_free(समुच्चयः, चरणः->दक्षिणः);
        समुच्चयः_free(समुच्चयः, चरणः);
        चरणः = अग्र;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT शून्य समुच्चयः_start(समुच्चयः_t *समुच्चयः) {
#यदि_परिभाषितम् समुच्चयः_TRACK
    समुच्चयः->allocs = 0;
    समुच्चयः->स्मृतिः = 0;
#यदि_अन्तः
    समुच्चयः->शीर्ष = समुच्चयः->मार्गः = समुच्चयः->ग्रन्थिः = रिक्त;   /* in case set_node() fails */
    समुच्चयः->मार्गः = समुच्चयः_ग्रन्थिः(समुच्चयः);
    समुच्चयः->शीर्ष = समुच्चयः_ग्रन्थिः(समुच्चयः);
    समुच्चयः_grow(समुच्चयः, समुच्चयः->शीर्ष, 1, 1); /* one link back to head for empty set */
    *(अचिह्नित वर्ण *)&समुच्चयः->शीर्ष->कुञ्चिका = 137;    /* set id */
    समुच्चयः->गभीरता = 0;
    समुच्चयः_uniq(&समुच्चयः->gen, समुच्चयः);
    समुच्चयः->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT पूर्णाङ्क समुच्चयः_ok(समुच्चयः_t *समुच्चयः) {
    प्रत्यागम समुच्चयः->शीर्ष != रिक्त &&
           समुच्चयः->शीर्ष->दक्षिणः != रिक्त &&
           *(अचिह्नित वर्ण *)&समुच्चयः->शीर्ष->कुञ्चिका == 137;
}

#यदि 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT शून्य समुच्चयः_clear(समुच्चयः_t *समुच्चयः) {
    प्रतिज्ञा(समुच्चयः_ok(समुच्चयः) && "improper use");

    /* Free all the keys and their nodes. */
    समुच्चयः_sweep(समुच्चयः);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    समुच्चयः->शीर्ष->दक्षिणः[0] = समुच्चयः->शीर्ष;
    समुच्चयः->शीर्ष->fill = 1;
    समुच्चयः->मार्गः->fill = 0;
    समुच्चयः->गभीरता = 0;
}
#यदि_अन्तः

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT शून्य समुच्चयः_अन्तः(समुच्चयः_t *समुच्चयः) {
    यदि (समुच्चयः->शीर्ष != रिक्त) {
        /* Empty the set and free the head node. */
        यदि (समुच्चयः->शीर्ष->दक्षिणः != रिक्त) {
            समुच्चयः_sweep(समुच्चयः);
            समुच्चयः_free(समुच्चयः, समुच्चयः->शीर्ष->दक्षिणः);
        }
        समुच्चयः_free(समुच्चयः, समुच्चयः->शीर्ष);
        समुच्चयः->शीर्ष = रिक्त;
    }
    यदि (समुच्चयः->मार्गः != रिक्त) {
        /* Free the path work area. */
        समुच्चयः_free(समुच्चयः, समुच्चयः->मार्गः->दक्षिणः);
        समुच्चयः_free(समुच्चयः, समुच्चयः->मार्गः);
        समुच्चयः->मार्गः = रिक्त;
    }
    यदि (समुच्चयः->ग्रन्थिः != रिक्त) {
        /* Free the node that was under construction when longjmp() hit. */
        समुच्चयः_drop(समुच्चयः, समुच्चयः->ग्रन्थिः->कुञ्चिका);
        समुच्चयः_free(समुच्चयः, समुच्चयः->ग्रन्थिः->दक्षिणः);
        समुच्चयः_free(समुच्चयः, समुच्चयः->ग्रन्थिः);
        समुच्चयः->ग्रन्थिः = रिक्त;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT पूर्णाङ्क समुच्चयः_प्राप्तम्(समुच्चयः_t *समुच्चयः, समुच्चयः_कुञ्चिका_t कुञ्चिका) {
    समुच्चयः_ग्रन्थिः_t *शीर्ष, *अत्र;
    पूर्णाङ्क i;
    प्रतिज्ञा(समुच्चयः_ok(समुच्चयः) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    शीर्ष = समुच्चयः->शीर्ष;
    अत्र = शीर्ष;
    i = समुच्चयः->गभीरता;
    समुच्चयः_grow(समुच्चयः, समुच्चयः->मार्गः, i + 1, 0);
    कुरु {
        यावत् (अत्र->दक्षिणः[i] != शीर्ष &&
               समुच्चयः_cmp(अत्र->दक्षिणः[i]->कुञ्चिका, कुञ्चिका) < 0)
            अत्र = अत्र->दक्षिणः[i];
        समुच्चयः->मार्गः->दक्षिणः[i] = अत्र;
    } यावत् (i--);

    /* See if the key matches. */
    अत्र = अत्र->दक्षिणः[0];
    प्रत्यागम अत्र != शीर्ष && समुच्चयः_cmp(अत्र->कुञ्चिका, कुञ्चिका) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT पूर्णाङ्क समुच्चयः_निवेशय(समुच्चयः_t *समुच्चयः, समुच्चयः_कुञ्चिका_t कुञ्चिका) {
    पूर्णाङ्क level = 0;
    पूर्णाङ्क bit;
    पूर्णाङ्क i;
    प्रतिज्ञा(समुच्चयः_ok(समुच्चयः) && "improper use");

    यदि (समुच्चयः_प्राप्तम्(समुच्चयः, कुञ्चिका))
        /* That key is already in the set. */
        प्रत्यागम 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    प्रति (;;) {
        यदि (समुच्चयः->ran == 1)
            /* Ran out. Get another 32 random bits. */
            समुच्चयः->ran = समुच्चयः_rand(&समुच्चयः->gen) | (1ULL << 32);
        bit = समुच्चयः->ran & 1;
        समुच्चयः->ran >>= 1;
        यदि (bit)
            भङ्ग;
        प्रतिज्ञा(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    यदि (level > समुच्चयः->गभीरता) {
        /* The maximum depth is now deeper. Update the structures. */
        समुच्चयः_grow(समुच्चयः, समुच्चयः->मार्गः, level + 1, 1);
        समुच्चयः_grow(समुच्चयः, समुच्चयः->शीर्ष, level + 1, 1);
        समुच्चयः->गभीरता = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    समुच्चयः->ग्रन्थिः = समुच्चयः_ग्रन्थिः(समुच्चयः);
    समुच्चयः->ग्रन्थिः->कुञ्चिका = कुञ्चिका;
    समुच्चयः_grow(समुच्चयः, समुच्चयः->ग्रन्थिः, level + 1, 0);
    प्रति (i = 0; i <= level; i++) {
        समुच्चयः->ग्रन्थिः->दक्षिणः[i] = समुच्चयः->मार्गः->दक्षिणः[i]->दक्षिणः[i];
        समुच्चयः->मार्गः->दक्षिणः[i]->दक्षिणः[i] = समुच्चयः->ग्रन्थिः;
    }
    समुच्चयः->ग्रन्थिः = रिक्त;
    प्रत्यागम 0;
}

#अन्यथा
#दोषः ** another skiplist समुच्चयः already created अत्र
/* Would need to implement a prefix in order to support multiple sets. */
#यदि_अन्तः
