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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SKIPSET_H
#ವ್ಯಾಖ್ಯಾನಿಸು SKIPSET_H

#ಅನ್ತರ್ಭಾವ <stdlib.h>     /* realloc(), free(), NULL, size_t */
#ಅನ್ತರ್ಭಾವ <stddef.h>     /* ptrdiff_t */
#ಅನ್ತರ್ಭಾವ <setjmp.h>     /* jmp_buf, longjmp() */
#ಅನ್ತರ್ಭಾವ <errno.h>      /* ENOMEM */
#ಅನ್ತರ್ಭಾವ <time.h>       /* time(), clock() */
#ಅನ್ತರ್ಭಾವ <assert.h>     /* assert.h */
#ಅನ್ತರ್ಭಾವ "ints.h"       /* i16_t, ui32_t, ui64_t */

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} ಗಣ_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_ಬೀಜ(ಗಣ_rand_t *gen, ui64_t ಬೀಜ, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (ಬೀಜ + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_uniq(ಗಣ_rand_t *gen, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr) {
    ಗಣ_ಬೀಜ(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(ಖಾಲಿ) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
ಸ್ಥಿರ ui32_t ಗಣ_rand(ಗಣ_rand_t *gen) {
    ui32_t mix;
    ಪೂರ್ಣಾಂಕ rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    ಹಿಂತಿರುಗಿ (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಗಣ_ಗಂಟು_s ಗಣ_ಗಂಟು_t;
ರಚನೆ ಗಣ_ಗಂಟು_s {
    ಗಣ_ಕೀಲಿ_t ಕೀಲಿ;          /* the key (not used for head or path) */
    i16_t ಗಾತ್ರ_2;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    ಗಣ_ಗಂಟು_t **ಬಲ_2;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಗಣ_s {
    ಗಣ_ಗಂಟು_t *ತಲೆ;       /* skiplist head -- no key, just links */
    ಗಣ_ಗಂಟು_t *ಮಾರ್ಗ;       /* right[] is path to key from set_found() */
    ಗಣ_ಗಂಟು_t *ಗಂಟು;       /* node under construction, in case of longjmp() */
    i16_t ಆಳ;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    ಗಣ_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಗಣ_TRACK
    size_t allocs;          /* number of allocations */
    size_t ಸ್ಮರಣೆ;          /* total size of allocated memory (>= requests) */
#ಆದರೆ_ಅಂತ್ಯ
} ಗಣ_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಗಣ_TRACK
/* Track the number of allocations and the total backing memory size. */
#  ಆದರೆ defined(_WIN32)
#    include <malloc.h>
#    define ಗಣ_ALLOC_ಗಾತ್ರ(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define ಗಣ_ALLOC_ಗಾತ್ರ(ptr) malloc_ಗಾತ್ರ(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define ಗಣ_ALLOC_ಗಾತ್ರ(ptr) malloc_usable_ಗಾತ್ರ(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define ಗಣ_ALLOC_ಗಾತ್ರ(ptr) malloc_usable_ಗಾತ್ರ(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define ಗಣ_ALLOC_ಗಾತ್ರ(ptr) malloc_usable_ಗಾತ್ರ(ptr)
#  ಇಲ್ಲದಿದ್ದರೆ     /* e.g. OpenBSD */
#    define ಗಣ_ALLOC_ಗಾತ್ರ(ptr) 0
#  endif
/* With tracking. */
ಸ್ಥಿರ ಶೂನ್ಯ *ಗಣ_alloc(ಗಣ_t *ಗಣ, ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2) {
    size_t had = ptr == ಖಾಲಿ ? 0 : ಗಣ_ALLOC_ಗಾತ್ರ(ptr);
    ಶೂನ್ಯ *mem = realloc(ptr, ಗಾತ್ರ_2);
    ಆದರೆ (mem == ಖಾಲಿ)
        longjmp(ಗಣ->env, ENOMEM);
    ಗಣ->allocs += ptr == ಖಾಲಿ;
    ಗಣ->ಸ್ಮರಣೆ += ಗಣ_ALLOC_ಗಾತ್ರ(mem) - had;
    ಹಿಂತಿರುಗಿ mem;
}
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_free(ಗಣ_t *ಗಣ, ಶೂನ್ಯ *ptr) {
    ಆದರೆ (ptr != ಖಾಲಿ) {
        ಗಣ->allocs--;
        ಗಣ->ಸ್ಮರಣೆ -= ಗಣ_ALLOC_ಗಾತ್ರ(ptr);
        ಬಿಡುಗಡೆ(ptr);
    }
}
#ಇಲ್ಲದಿದ್ದರೆ
/* Without tracking. */
ಸ್ಥಿರ ಶೂನ್ಯ *ಗಣ_alloc(ಗಣ_t *ಗಣ, ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2) {
    ಶೂನ್ಯ *mem = realloc(ptr, ಗಾತ್ರ_2);
    ಆದರೆ (mem == ಖಾಲಿ)
        longjmp(ಗಣ->env, ENOMEM);
    ಹಿಂತಿರುಗಿ mem;
}
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_free(ಗಣ_t *ಗಣ, ಶೂನ್ಯ *ptr) {
    (ಶೂನ್ಯ)ಗಣ;
    ಬಿಡುಗಡೆ(ptr);
}
#ಆದರೆ_ಅಂತ್ಯ

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_grow(ಗಣ_t *ಗಣ, ಗಣ_ಗಂಟು_t *ಗಂಟು, ಪೂರ್ಣಾಂಕ want, ಪೂರ್ಣಾಂಕ fill) {
    ಪೂರ್ಣಾಂಕ i;
    ಆದರೆ (ಗಂಟು->ಗಾತ್ರ_2 < want) {
        ಪೂರ್ಣಾಂಕ ಇನ್ನಷ್ಟು = ಗಂಟು->ಗಾತ್ರ_2 ? ಗಂಟು->ಗಾತ್ರ_2 : 1;
        ಆಗ (ಇನ್ನಷ್ಟು < want)
            ಇನ್ನಷ್ಟು <<= 1;
        ಗಂಟು->ಬಲ_2 = ಗಣ_alloc(ಗಣ, ಗಂಟು->ಬಲ_2,
                                (size_t)ಇನ್ನಷ್ಟು * ಗಾತ್ರ(ಗಣ_ಗಂಟು_t *));
        ಗಂಟು->ಗಾತ್ರ_2 = (i16_t)ಇನ್ನಷ್ಟು;
    }
    ಆದರೆ (fill)
        ಗಾಗಿ (i = ಗಂಟು->fill; i < want; i++)
            ಗಂಟು->ಬಲ_2[i] = ಗಣ->ತಲೆ;
    ಗಂಟು->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
ಸ್ಥಿರ ಗಣ_ಗಂಟು_t *ಗಣ_ಗಂಟು(ಗಣ_t *ಗಣ) {
    ಗಣ_ಗಂಟು_t *ಗಂಟು = ಗಣ_alloc(ಗಣ, ಖಾಲಿ, ಗಾತ್ರ(ಗಣ_ಗಂಟು_t));
    ಗಂಟು->ಗಾತ್ರ_2 = 0;
    ಗಂಟು->fill = 0;
    ಗಂಟು->ಬಲ_2 = ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ ಗಂಟು;
}

/* --private-- Free the list linked from head, along with the keys. */
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_sweep(ಗಣ_t *ಗಣ) {
    ಗಣ_ಗಂಟು_t *ಹೆಜ್ಜೆ = ಗಣ->ತಲೆ->ಬಲ_2[0];
    ಆಗ (ಹೆಜ್ಜೆ != ಗಣ->ತಲೆ) {
        ಗಣ_ಗಂಟು_t *ಮುಂದಿನ = ಹೆಜ್ಜೆ->ಬಲ_2[0];      /* save link to next node */
        ಗಣ_drop(ಗಣ, ಹೆಜ್ಜೆ->ಕೀಲಿ);
        ಗಣ_free(ಗಣ, ಹೆಜ್ಜೆ->ಬಲ_2);
        ಗಣ_free(ಗಣ, ಹೆಜ್ಜೆ);
        ಹೆಜ್ಜೆ = ಮುಂದಿನ;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT ಶೂನ್ಯ ಗಣ_ಆರಂಭ(ಗಣ_t *ಗಣ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಗಣ_TRACK
    ಗಣ->allocs = 0;
    ಗಣ->ಸ್ಮರಣೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ
    ಗಣ->ತಲೆ = ಗಣ->ಮಾರ್ಗ = ಗಣ->ಗಂಟು = ಖಾಲಿ;   /* in case set_node() fails */
    ಗಣ->ಮಾರ್ಗ = ಗಣ_ಗಂಟು(ಗಣ);
    ಗಣ->ತಲೆ = ಗಣ_ಗಂಟು(ಗಣ);
    ಗಣ_grow(ಗಣ, ಗಣ->ತಲೆ, 1, 1); /* one link back to head for empty set */
    *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&ಗಣ->ತಲೆ->ಕೀಲಿ = 137;    /* set id */
    ಗಣ->ಆಳ = 0;
    ಗಣ_uniq(&ಗಣ->gen, ಗಣ);
    ಗಣ->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT ಪೂರ್ಣಾಂಕ ಗಣ_ok(ಗಣ_t *ಗಣ) {
    ಹಿಂತಿರುಗಿ ಗಣ->ತಲೆ != ಖಾಲಿ &&
           ಗಣ->ತಲೆ->ಬಲ_2 != ಖಾಲಿ &&
           *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&ಗಣ->ತಲೆ->ಕೀಲಿ == 137;
}

#ಆದರೆ 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT ಶೂನ್ಯ ಗಣ_clear(ಗಣ_t *ಗಣ) {
    ಪ್ರತಿಜ್ಞಾ(ಗಣ_ok(ಗಣ) && "improper use");

    /* Free all the keys and their nodes. */
    ಗಣ_sweep(ಗಣ);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    ಗಣ->ತಲೆ->ಬಲ_2[0] = ಗಣ->ತಲೆ;
    ಗಣ->ತಲೆ->fill = 1;
    ಗಣ->ಮಾರ್ಗ->fill = 0;
    ಗಣ->ಆಳ = 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT ಶೂನ್ಯ ಗಣ_ಕೊನೆ(ಗಣ_t *ಗಣ) {
    ಆದರೆ (ಗಣ->ತಲೆ != ಖಾಲಿ) {
        /* Empty the set and free the head node. */
        ಆದರೆ (ಗಣ->ತಲೆ->ಬಲ_2 != ಖಾಲಿ) {
            ಗಣ_sweep(ಗಣ);
            ಗಣ_free(ಗಣ, ಗಣ->ತಲೆ->ಬಲ_2);
        }
        ಗಣ_free(ಗಣ, ಗಣ->ತಲೆ);
        ಗಣ->ತಲೆ = ಖಾಲಿ;
    }
    ಆದರೆ (ಗಣ->ಮಾರ್ಗ != ಖಾಲಿ) {
        /* Free the path work area. */
        ಗಣ_free(ಗಣ, ಗಣ->ಮಾರ್ಗ->ಬಲ_2);
        ಗಣ_free(ಗಣ, ಗಣ->ಮಾರ್ಗ);
        ಗಣ->ಮಾರ್ಗ = ಖಾಲಿ;
    }
    ಆದರೆ (ಗಣ->ಗಂಟು != ಖಾಲಿ) {
        /* Free the node that was under construction when longjmp() hit. */
        ಗಣ_drop(ಗಣ, ಗಣ->ಗಂಟು->ಕೀಲಿ);
        ಗಣ_free(ಗಣ, ಗಣ->ಗಂಟು->ಬಲ_2);
        ಗಣ_free(ಗಣ, ಗಣ->ಗಂಟು);
        ಗಣ->ಗಂಟು = ಖಾಲಿ;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT ಪೂರ್ಣಾಂಕ ಗಣ_ಸಿಕ್ಕಿತು(ಗಣ_t *ಗಣ, ಗಣ_ಕೀಲಿ_t ಕೀಲಿ) {
    ಗಣ_ಗಂಟು_t *ತಲೆ, *ಇಲ್ಲಿ;
    ಪೂರ್ಣಾಂಕ i;
    ಪ್ರತಿಜ್ಞಾ(ಗಣ_ok(ಗಣ) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    ತಲೆ = ಗಣ->ತಲೆ;
    ಇಲ್ಲಿ = ತಲೆ;
    i = ಗಣ->ಆಳ;
    ಗಣ_grow(ಗಣ, ಗಣ->ಮಾರ್ಗ, i + 1, 0);
    ಮಾಡು {
        ಆಗ (ಇಲ್ಲಿ->ಬಲ_2[i] != ತಲೆ &&
               ಗಣ_cmp(ಇಲ್ಲಿ->ಬಲ_2[i]->ಕೀಲಿ, ಕೀಲಿ) < 0)
            ಇಲ್ಲಿ = ಇಲ್ಲಿ->ಬಲ_2[i];
        ಗಣ->ಮಾರ್ಗ->ಬಲ_2[i] = ಇಲ್ಲಿ;
    } ಆಗ (i--);

    /* See if the key matches. */
    ಇಲ್ಲಿ = ಇಲ್ಲಿ->ಬಲ_2[0];
    ಹಿಂತಿರುಗಿ ಇಲ್ಲಿ != ತಲೆ && ಗಣ_cmp(ಇಲ್ಲಿ->ಕೀಲಿ, ಕೀಲಿ) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT ಪೂರ್ಣಾಂಕ ಗಣ_ಸೇರಿಸು(ಗಣ_t *ಗಣ, ಗಣ_ಕೀಲಿ_t ಕೀಲಿ) {
    ಪೂರ್ಣಾಂಕ level = 0;
    ಪೂರ್ಣಾಂಕ ಬಿಟ್;
    ಪೂರ್ಣಾಂಕ i;
    ಪ್ರತಿಜ್ಞಾ(ಗಣ_ok(ಗಣ) && "improper use");

    ಆದರೆ (ಗಣ_ಸಿಕ್ಕಿತು(ಗಣ, ಕೀಲಿ))
        /* That key is already in the set. */
        ಹಿಂತಿರುಗಿ 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    ಗಾಗಿ (;;) {
        ಆದರೆ (ಗಣ->ran == 1)
            /* Ran out. Get another 32 random bits. */
            ಗಣ->ran = ಗಣ_rand(&ಗಣ->gen) | (1ULL << 32);
        ಬಿಟ್ = ಗಣ->ran & 1;
        ಗಣ->ran >>= 1;
        ಆದರೆ (ಬಿಟ್)
            ನಿಲ್ಲಿಸು;
        ಪ್ರತಿಜ್ಞಾ(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    ಆದರೆ (level > ಗಣ->ಆಳ) {
        /* The maximum depth is now deeper. Update the structures. */
        ಗಣ_grow(ಗಣ, ಗಣ->ಮಾರ್ಗ, level + 1, 1);
        ಗಣ_grow(ಗಣ, ಗಣ->ತಲೆ, level + 1, 1);
        ಗಣ->ಆಳ = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    ಗಣ->ಗಂಟು = ಗಣ_ಗಂಟು(ಗಣ);
    ಗಣ->ಗಂಟು->ಕೀಲಿ = ಕೀಲಿ;
    ಗಣ_grow(ಗಣ, ಗಣ->ಗಂಟು, level + 1, 0);
    ಗಾಗಿ (i = 0; i <= level; i++) {
        ಗಣ->ಗಂಟು->ಬಲ_2[i] = ಗಣ->ಮಾರ್ಗ->ಬಲ_2[i]->ಬಲ_2[i];
        ಗಣ->ಮಾರ್ಗ->ಬಲ_2[i]->ಬಲ_2[i] = ಗಣ->ಗಂಟು;
    }
    ಗಣ->ಗಂಟು = ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ 0;
}

#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ ** another skiplist ಗಣ already created ಇಲ್ಲಿ
/* Would need to implement a prefix in order to support multiple sets. */
#ಆದರೆ_ಅಂತ್ಯ
