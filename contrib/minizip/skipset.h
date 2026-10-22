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

#অন্তর্ভাৱ <stdlib.h>     /* realloc(), free(), NULL, size_t */
#অন্তর্ভাৱ <stddef.h>     /* ptrdiff_t */
#অন্তর্ভাৱ <setjmp.h>     /* jmp_buf, longjmp() */
#অন্তর্ভাৱ <errno.h>      /* ENOMEM */
#অন্তর্ভাৱ <time.h>       /* time(), clock() */
#অন্তর্ভাৱ <assert.h>     /* assert.h */
#অন্তর্ভাৱ "ints.h"       /* i16_t, ui32_t, ui64_t */

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
প্রকারনির্দেশ সংরচনা {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} set_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
স্থির শূন্য set_seed(set_rand_t *gen, ui64_t seed, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (seed + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
স্থির শূন্য set_uniq(set_rand_t *gen, ধ্রুৱ শূন্য *ptr) {
    set_seed(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(রিক্ত) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
স্থির ui32_t set_rand(set_rand_t *gen) {
    ui32_t mix;
    পূর্ণাঙ্ক rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    ঘূৰাই (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
প্রকারনির্দেশ সংরচনা set_গ্ৰন্থি_s set_গ্ৰন্থি_t;
সংরচনা set_গ্ৰন্থি_s {
    set_চাবি_t চাবি;          /* the key (not used for head or path) */
    i16_t size;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    set_গ্ৰন্থি_t **ডান;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
প্রকারনির্দেশ সংরচনা set_s {
    set_গ্ৰন্থি_t *head;       /* skiplist head -- no key, just links */
    set_গ্ৰন্থি_t *path;       /* right[] is path to key from set_found() */
    set_গ্ৰন্থি_t *গ্ৰন্থি;       /* node under construction, in case of longjmp() */
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
#  যদি defined(_WIN32)
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
#  অন্যথা     /* e.g. OpenBSD */
#    define SET_ALLOC_SIZE(ptr) 0
#  endif
/* With tracking. */
স্থির শূন্য *set_alloc(set_t *set, শূন্য *ptr, size_t size) {
    size_t had = ptr == রিক্ত ? 0 : SET_ALLOC_SIZE(ptr);
    শূন্য *mem = realloc(ptr, size);
    যদি (mem == রিক্ত)
        longjmp(set->env, ENOMEM);
    set->allocs += ptr == রিক্ত;
    set->memory += SET_ALLOC_SIZE(mem) - had;
    ঘূৰাই mem;
}
স্থির শূন্য set_free(set_t *set, শূন্য *ptr) {
    যদি (ptr != রিক্ত) {
        set->allocs--;
        set->memory -= SET_ALLOC_SIZE(ptr);
        মুক্ত(ptr);
    }
}
#অন্যথা
/* Without tracking. */
স্থির শূন্য *set_alloc(set_t *set, শূন্য *ptr, size_t size) {
    শূন্য *mem = realloc(ptr, size);
    যদি (mem == রিক্ত)
        longjmp(set->env, ENOMEM);
    ঘূৰাই mem;
}
স্থির শূন্য set_free(set_t *set, শূন্য *ptr) {
    (শূন্য)set;
    মুক্ত(ptr);
}
#endif

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
স্থির শূন্য set_grow(set_t *set, set_গ্ৰন্থি_t *গ্ৰন্থি, পূর্ণাঙ্ক want, পূর্ণাঙ্ক fill) {
    পূর্ণাঙ্ক i;
    যদি (গ্ৰন্থি->size < want) {
        পূর্ণাঙ্ক more = গ্ৰন্থি->size ? গ্ৰন্থি->size : 1;
        যেতিয়ালৈ (more < want)
            more <<= 1;
        গ্ৰন্থি->ডান = set_alloc(set, গ্ৰন্থি->ডান,
                                (size_t)more * পরিমাণ(set_গ্ৰন্থি_t *));
        গ্ৰন্থি->size = (i16_t)more;
    }
    যদি (fill)
        বাবে (i = গ্ৰন্থি->fill; i < want; i++)
            গ্ৰন্থি->ডান[i] = set->head;
    গ্ৰন্থি->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
স্থির set_গ্ৰন্থি_t *set_গ্ৰন্থি(set_t *set) {
    set_গ্ৰন্থি_t *গ্ৰন্থি = set_alloc(set, রিক্ত, পরিমাণ(set_গ্ৰন্থি_t));
    গ্ৰন্থি->size = 0;
    গ্ৰন্থি->fill = 0;
    গ্ৰন্থি->ডান = রিক্ত;
    ঘূৰাই গ্ৰন্থি;
}

/* --private-- Free the list linked from head, along with the keys. */
স্থির শূন্য set_sweep(set_t *set) {
    set_গ্ৰন্থি_t *step = set->head->ডান[0];
    যেতিয়ালৈ (step != set->head) {
        set_গ্ৰন্থি_t *next = step->ডান[0];      /* save link to next node */
        set_drop(set, step->চাবি);
        set_free(set, step->ডান);
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
SKIPSET_EXPORT শূন্য set_start(set_t *set) {
#ifdef SET_TRACK
    set->allocs = 0;
    set->memory = 0;
#endif
    set->head = set->path = set->গ্ৰন্থি = রিক্ত;   /* in case set_node() fails */
    set->path = set_গ্ৰন্থি(set);
    set->head = set_গ্ৰন্থি(set);
    set_grow(set, set->head, 1, 1); /* one link back to head for empty set */
    *(unsigned ৱর্ণ *)&set->head->চাবি = 137;    /* set id */
    set->depth = 0;
    set_uniq(&set->gen, set);
    set->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT পূর্ণাঙ্ক set_ok(set_t *set) {
    ঘূৰাই set->head != রিক্ত &&
           set->head->ডান != রিক্ত &&
           *(unsigned ৱর্ণ *)&set->head->চাবি == 137;
}

#যদি 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT শূন্য set_clear(set_t *set) {
    প্রতিজ্ঞা(set_ok(set) && "improper use");

    /* Free all the keys and their nodes. */
    set_sweep(set);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    set->head->ডান[0] = set->head;
    set->head->fill = 1;
    set->path->fill = 0;
    set->depth = 0;
}
#endif

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT শূন্য set_end(set_t *set) {
    যদি (set->head != রিক্ত) {
        /* Empty the set and free the head node. */
        যদি (set->head->ডান != রিক্ত) {
            set_sweep(set);
            set_free(set, set->head->ডান);
        }
        set_free(set, set->head);
        set->head = রিক্ত;
    }
    যদি (set->path != রিক্ত) {
        /* Free the path work area. */
        set_free(set, set->path->ডান);
        set_free(set, set->path);
        set->path = রিক্ত;
    }
    যদি (set->গ্ৰন্থি != রিক্ত) {
        /* Free the node that was under construction when longjmp() hit. */
        set_drop(set, set->গ্ৰন্থি->চাবি);
        set_free(set, set->গ্ৰন্থি->ডান);
        set_free(set, set->গ্ৰন্থি);
        set->গ্ৰন্থি = রিক্ত;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT পূর্ণাঙ্ক set_found(set_t *set, set_চাবি_t চাবি) {
    set_গ্ৰন্থি_t *head, *here;
    পূর্ণাঙ্ক i;
    প্রতিজ্ঞা(set_ok(set) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    head = set->head;
    here = head;
    i = set->depth;
    set_grow(set, set->path, i + 1, 0);
    কৰা {
        যেতিয়ালৈ (here->ডান[i] != head &&
               set_cmp(here->ডান[i]->চাবি, চাবি) < 0)
            here = here->ডান[i];
        set->path->ডান[i] = here;
    } যেতিয়ালৈ (i--);

    /* See if the key matches. */
    here = here->ডান[0];
    ঘূৰাই here != head && set_cmp(here->চাবি, চাবি) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT পূর্ণাঙ্ক set_ঢোকাও(set_t *set, set_চাবি_t চাবি) {
    পূর্ণাঙ্ক level = 0;
    পূর্ণাঙ্ক bit;
    পূর্ণাঙ্ক i;
    প্রতিজ্ঞা(set_ok(set) && "improper use");

    যদি (set_found(set, চাবি))
        /* That key is already in the set. */
        ঘূৰাই 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    বাবে (;;) {
        যদি (set->ran == 1)
            /* Ran out. Get another 32 random bits. */
            set->ran = set_rand(&set->gen) | (1ULL << 32);
        bit = set->ran & 1;
        set->ran >>= 1;
        যদি (bit)
            ভঙ্গ;
        প্রতিজ্ঞা(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    যদি (level > set->depth) {
        /* The maximum depth is now deeper. Update the structures. */
        set_grow(set, set->path, level + 1, 1);
        set_grow(set, set->head, level + 1, 1);
        set->depth = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    set->গ্ৰন্থি = set_গ্ৰন্থি(set);
    set->গ্ৰন্থি->চাবি = চাবি;
    set_grow(set, set->গ্ৰন্থি, level + 1, 0);
    বাবে (i = 0; i <= level; i++) {
        set->গ্ৰন্থি->ডান[i] = set->path->ডান[i]->ডান[i];
        set->path->ডান[i]->ডান[i] = set->গ্ৰন্থি;
    }
    set->গ্ৰন্থি = রিক্ত;
    ঘূৰাই 0;
}

#অন্যথা
#error ** another skiplist set already created here
/* Would need to implement a prefix in order to support multiple sets. */
#endif
