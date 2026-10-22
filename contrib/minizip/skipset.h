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

#ifndef SKIPSET_H
#define SKIPSET_H

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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} set_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
ಸ್ಥಿರ ಶೂನ್ಯ set_seed(set_rand_t *gen, ui64_t seed, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (seed + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
ಸ್ಥಿರ ಶೂನ್ಯ set_uniq(set_rand_t *gen, ಧ್ರುವ ಶೂನ್ಯ *ptr) {
    set_seed(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(ರಿಕ್ತ) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
ಸ್ಥಿರ ui32_t set_rand(set_rand_t *gen) {
    ui32_t mix;
    ಪೂರ್ಣಾಙ್ಕ rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    ಪ್ರತ್ಯಾಗಮ (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ set_ಗಂಟು_s set_ಗಂಟು_t;
ಸಂರಚನಾ set_ಗಂಟು_s {
    set_ಕೀಲಿ_t ಕೀಲಿ;          /* the key (not used for head or path) */
    i16_t size;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    set_ಗಂಟು_t **ಬಲ;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ set_s {
    set_ಗಂಟು_t *head;       /* skiplist head -- no key, just links */
    set_ಗಂಟು_t *path;       /* right[] is path to key from set_found() */
    set_ಗಂಟು_t *ಗಂಟು;       /* node under construction, in case of longjmp() */
    i16_t depth;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    set_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#ifdef SET_TRACK
    size_t allocs;          /* number of allocations */
    size_t memory;          /* total size of allocated memory (>= requests) */
#endif
} set_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#ifdef SET_TRACK
/* Track the number of allocations and the total backing memory size. */
#  ಯದಿ defined(_WIN32)
#    include <malloc.h>
#    define SET_ALLOC_SIZE(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define SET_ALLOC_SIZE(ptr) malloc_size(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define SET_ALLOC_SIZE(ptr) malloc_usable_size(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define SET_ALLOC_SIZE(ptr) malloc_usable_size(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define SET_ALLOC_SIZE(ptr) malloc_usable_size(ptr)
#  ಅನ್ಯಥಾ     /* e.g. OpenBSD */
#    define SET_ALLOC_SIZE(ptr) 0
#  endif
/* With tracking. */
ಸ್ಥಿರ ಶೂನ್ಯ *set_alloc(set_t *set, ಶೂನ್ಯ *ptr, size_t size) {
    size_t had = ptr == ರಿಕ್ತ ? 0 : SET_ALLOC_SIZE(ptr);
    ಶೂನ್ಯ *mem = realloc(ptr, size);
    ಯದಿ (mem == ರಿಕ್ತ)
        longjmp(set->env, ENOMEM);
    set->allocs += ptr == ರಿಕ್ತ;
    set->memory += SET_ALLOC_SIZE(mem) - had;
    ಪ್ರತ್ಯಾಗಮ mem;
}
ಸ್ಥಿರ ಶೂನ್ಯ set_free(set_t *set, ಶೂನ್ಯ *ptr) {
    ಯದಿ (ptr != ರಿಕ್ತ) {
        set->allocs--;
        set->memory -= SET_ALLOC_SIZE(ptr);
        ಮುಕ್ತ(ptr);
    }
}
#ಅನ್ಯಥಾ
/* Without tracking. */
ಸ್ಥಿರ ಶೂನ್ಯ *set_alloc(set_t *set, ಶೂನ್ಯ *ptr, size_t size) {
    ಶೂನ್ಯ *mem = realloc(ptr, size);
    ಯದಿ (mem == ರಿಕ್ತ)
        longjmp(set->env, ENOMEM);
    ಪ್ರತ್ಯಾಗಮ mem;
}
ಸ್ಥಿರ ಶೂನ್ಯ set_free(set_t *set, ಶೂನ್ಯ *ptr) {
    (ಶೂನ್ಯ)set;
    ಮುಕ್ತ(ptr);
}
#endif

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
ಸ್ಥಿರ ಶೂನ್ಯ set_grow(set_t *set, set_ಗಂಟು_t *ಗಂಟು, ಪೂರ್ಣಾಙ್ಕ want, ಪೂರ್ಣಾಙ್ಕ fill) {
    ಪೂರ್ಣಾಙ್ಕ i;
    ಯದಿ (ಗಂಟು->size < want) {
        ಪೂರ್ಣಾಙ್ಕ more = ಗಂಟು->size ? ಗಂಟು->size : 1;
        ಯಾವತ್ (more < want)
            more <<= 1;
        ಗಂಟು->ಬಲ = set_alloc(set, ಗಂಟು->ಬಲ,
                                (size_t)more * ಪರಿಮಾಣ(set_ಗಂಟು_t *));
        ಗಂಟು->size = (i16_t)more;
    }
    ಯದಿ (fill)
        ಪ್ರತಿ (i = ಗಂಟು->fill; i < want; i++)
            ಗಂಟು->ಬಲ[i] = set->head;
    ಗಂಟು->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
ಸ್ಥಿರ set_ಗಂಟು_t *set_ಗಂಟು(set_t *set) {
    set_ಗಂಟು_t *ಗಂಟು = set_alloc(set, ರಿಕ್ತ, ಪರಿಮಾಣ(set_ಗಂಟು_t));
    ಗಂಟು->size = 0;
    ಗಂಟು->fill = 0;
    ಗಂಟು->ಬಲ = ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು;
}

/* --private-- Free the list linked from head, along with the keys. */
ಸ್ಥಿರ ಶೂನ್ಯ set_sweep(set_t *set) {
    set_ಗಂಟು_t *step = set->head->ಬಲ[0];
    ಯಾವತ್ (step != set->head) {
        set_ಗಂಟು_t *next = step->ಬಲ[0];      /* save link to next node */
        set_drop(set, step->ಕೀಲಿ);
        set_free(set, step->ಬಲ);
        set_free(set, step);
        step = next;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT ಶೂನ್ಯ set_start(set_t *set) {
#ifdef SET_TRACK
    set->allocs = 0;
    set->memory = 0;
#endif
    set->head = set->path = set->ಗಂಟು = ರಿಕ್ತ;   /* in case set_node() fails */
    set->path = set_ಗಂಟು(set);
    set->head = set_ಗಂಟು(set);
    set_grow(set, set->head, 1, 1); /* one link back to head for empty set */
    *(unsigned ವರ್ಣ *)&set->head->ಕೀಲಿ = 137;    /* set id */
    set->depth = 0;
    set_uniq(&set->gen, set);
    set->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT ಪೂರ್ಣಾಙ್ಕ set_ok(set_t *set) {
    ಪ್ರತ್ಯಾಗಮ set->head != ರಿಕ್ತ &&
           set->head->ಬಲ != ರಿಕ್ತ &&
           *(unsigned ವರ್ಣ *)&set->head->ಕೀಲಿ == 137;
}

#ಯದಿ 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT ಶೂನ್ಯ set_clear(set_t *set) {
    ಪ್ರತಿಜ್ಞಾ(set_ok(set) && "improper use");

    /* Free all the keys and their nodes. */
    set_sweep(set);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    set->head->ಬಲ[0] = set->head;
    set->head->fill = 1;
    set->path->fill = 0;
    set->depth = 0;
}
#endif

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT ಶೂನ್ಯ set_end(set_t *set) {
    ಯದಿ (set->head != ರಿಕ್ತ) {
        /* Empty the set and free the head node. */
        ಯದಿ (set->head->ಬಲ != ರಿಕ್ತ) {
            set_sweep(set);
            set_free(set, set->head->ಬಲ);
        }
        set_free(set, set->head);
        set->head = ರಿಕ್ತ;
    }
    ಯದಿ (set->path != ರಿಕ್ತ) {
        /* Free the path work area. */
        set_free(set, set->path->ಬಲ);
        set_free(set, set->path);
        set->path = ರಿಕ್ತ;
    }
    ಯದಿ (set->ಗಂಟು != ರಿಕ್ತ) {
        /* Free the node that was under construction when longjmp() hit. */
        set_drop(set, set->ಗಂಟು->ಕೀಲಿ);
        set_free(set, set->ಗಂಟು->ಬಲ);
        set_free(set, set->ಗಂಟು);
        set->ಗಂಟು = ರಿಕ್ತ;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT ಪೂರ್ಣಾಙ್ಕ set_found(set_t *set, set_ಕೀಲಿ_t ಕೀಲಿ) {
    set_ಗಂಟು_t *head, *here;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿಜ್ಞಾ(set_ok(set) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    head = set->head;
    here = head;
    i = set->depth;
    set_grow(set, set->path, i + 1, 0);
    ಕುರು {
        ಯಾವತ್ (here->ಬಲ[i] != head &&
               set_cmp(here->ಬಲ[i]->ಕೀಲಿ, ಕೀಲಿ) < 0)
            here = here->ಬಲ[i];
        set->path->ಬಲ[i] = here;
    } ಯಾವತ್ (i--);

    /* See if the key matches. */
    here = here->ಬಲ[0];
    ಪ್ರತ್ಯಾಗಮ here != head && set_cmp(here->ಕೀಲಿ, ಕೀಲಿ) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT ಪೂರ್ಣಾಙ್ಕ set_ಸೇರಿಸು(set_t *set, set_ಕೀಲಿ_t ಕೀಲಿ) {
    ಪೂರ್ಣಾಙ್ಕ level = 0;
    ಪೂರ್ಣಾಙ್ಕ bit;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿಜ್ಞಾ(set_ok(set) && "improper use");

    ಯದಿ (set_found(set, ಕೀಲಿ))
        /* That key is already in the set. */
        ಪ್ರತ್ಯಾಗಮ 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    ಪ್ರತಿ (;;) {
        ಯದಿ (set->ran == 1)
            /* Ran out. Get another 32 random bits. */
            set->ran = set_rand(&set->gen) | (1ULL << 32);
        bit = set->ran & 1;
        set->ran >>= 1;
        ಯದಿ (bit)
            ಭಙ್ಗ;
        ಪ್ರತಿಜ್ಞಾ(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    ಯದಿ (level > set->depth) {
        /* The maximum depth is now deeper. Update the structures. */
        set_grow(set, set->path, level + 1, 1);
        set_grow(set, set->head, level + 1, 1);
        set->depth = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    set->ಗಂಟು = set_ಗಂಟು(set);
    set->ಗಂಟು->ಕೀಲಿ = ಕೀಲಿ;
    set_grow(set, set->ಗಂಟು, level + 1, 0);
    ಪ್ರತಿ (i = 0; i <= level; i++) {
        set->ಗಂಟು->ಬಲ[i] = set->path->ಬಲ[i]->ಬಲ[i];
        set->path->ಬಲ[i]->ಬಲ[i] = set->ಗಂಟು;
    }
    set->ಗಂಟು = ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ 0;
}

#ಅನ್ಯಥಾ
#error ** another skiplist set already created here
/* Would need to implement a prefix in order to support multiple sets. */
#endif
