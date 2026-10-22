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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SKIPSET_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SKIPSET_H

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>     /* realloc(), free(), NULL, size_t */
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>     /* ptrdiff_t */
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>     /* jmp_buf, longjmp() */
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>      /* ENOMEM */
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>       /* time(), clock() */
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>     /* assert.h */
#ਅਨ੍ਤਰ੍ਭਾਵ "ints.h"       /* i16_t, ui32_t, ui64_t */

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} ਸੈੱਟ_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_ਬੀਜ(ਸੈੱਟ_rand_t *gen, ui64_t ਬੀਜ, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (ਬੀਜ + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_uniq(ਸੈੱਟ_rand_t *gen, ਅਚਲ ਖਾਲੀ *ptr) {
    ਸੈੱਟ_ਬੀਜ(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(ਨਿਰਾ) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
ਸਥਿਰ ui32_t ਸੈੱਟ_rand(ਸੈੱਟ_rand_t *gen) {
    ui32_t mix;
    ਪੂਰਨ rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    ਵਾਪਸ (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸੈੱਟ_ਗੰਢ_s ਸੈੱਟ_ਗੰਢ_t;
ਬਣਤਰ ਸੈੱਟ_ਗੰਢ_s {
    ਸੈੱਟ_ਕੁੰਜੀ_t ਕੁੰਜੀ;          /* the key (not used for head or path) */
    i16_t ਮਾਪ;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    ਸੈੱਟ_ਗੰਢ_t **ਸੱਜਾ;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸੈੱਟ_s {
    ਸੈੱਟ_ਗੰਢ_t *ਸਿਰ;       /* skiplist head -- no key, just links */
    ਸੈੱਟ_ਗੰਢ_t *ਰਸਤਾ;       /* right[] is path to key from set_found() */
    ਸੈੱਟ_ਗੰਢ_t *ਗੰਢ;       /* node under construction, in case of longjmp() */
    i16_t ਡੂੰਘਾਈ;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    ਸੈੱਟ_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸੈੱਟ_TRACK
    size_t allocs;          /* number of allocations */
    size_t ਸਮ੍ਰਿਤੀ;          /* total size of allocated memory (>= requests) */
#ਜੇ_ਅੰਤ
} ਸੈੱਟ_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸੈੱਟ_TRACK
/* Track the number of allocations and the total backing memory size. */
#  ਜੇ defined(_WIN32)
#    include <malloc.h>
#    define ਸੈੱਟ_ALLOC_ਮਾਪ(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define ਸੈੱਟ_ALLOC_ਮਾਪ(ptr) malloc_ਮਾਪ(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define ਸੈੱਟ_ALLOC_ਮਾਪ(ptr) malloc_usable_ਮਾਪ(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define ਸੈੱਟ_ALLOC_ਮਾਪ(ptr) malloc_usable_ਮਾਪ(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define ਸੈੱਟ_ALLOC_ਮਾਪ(ptr) malloc_usable_ਮਾਪ(ptr)
#  ਨਹੀਂਤਾਂ     /* e.g. OpenBSD */
#    define ਸੈੱਟ_ALLOC_ਮਾਪ(ptr) 0
#  endif
/* With tracking. */
ਸਥਿਰ ਖਾਲੀ *ਸੈੱਟ_alloc(ਸੈੱਟ_t *ਸੈੱਟ, ਖਾਲੀ *ptr, size_t ਮਾਪ) {
    size_t had = ptr == ਨਿਰਾ ? 0 : ਸੈੱਟ_ALLOC_ਮਾਪ(ptr);
    ਖਾਲੀ *mem = realloc(ptr, ਮਾਪ);
    ਜੇ (mem == ਨਿਰਾ)
        longjmp(ਸੈੱਟ->env, ENOMEM);
    ਸੈੱਟ->allocs += ptr == ਨਿਰਾ;
    ਸੈੱਟ->ਸਮ੍ਰਿਤੀ += ਸੈੱਟ_ALLOC_ਮਾਪ(mem) - had;
    ਵਾਪਸ mem;
}
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_free(ਸੈੱਟ_t *ਸੈੱਟ, ਖਾਲੀ *ptr) {
    ਜੇ (ptr != ਨਿਰਾ) {
        ਸੈੱਟ->allocs--;
        ਸੈੱਟ->ਸਮ੍ਰਿਤੀ -= ਸੈੱਟ_ALLOC_ਮਾਪ(ptr);
        ਮੁਕਤ(ptr);
    }
}
#ਨਹੀਂਤਾਂ
/* Without tracking. */
ਸਥਿਰ ਖਾਲੀ *ਸੈੱਟ_alloc(ਸੈੱਟ_t *ਸੈੱਟ, ਖਾਲੀ *ptr, size_t ਮਾਪ) {
    ਖਾਲੀ *mem = realloc(ptr, ਮਾਪ);
    ਜੇ (mem == ਨਿਰਾ)
        longjmp(ਸੈੱਟ->env, ENOMEM);
    ਵਾਪਸ mem;
}
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_free(ਸੈੱਟ_t *ਸੈੱਟ, ਖਾਲੀ *ptr) {
    (ਖਾਲੀ)ਸੈੱਟ;
    ਮੁਕਤ(ptr);
}
#ਜੇ_ਅੰਤ

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_grow(ਸੈੱਟ_t *ਸੈੱਟ, ਸੈੱਟ_ਗੰਢ_t *ਗੰਢ, ਪੂਰਨ want, ਪੂਰਨ fill) {
    ਪੂਰਨ i;
    ਜੇ (ਗੰਢ->ਮਾਪ < want) {
        ਪੂਰਨ ਹੋਰ = ਗੰਢ->ਮਾਪ ? ਗੰਢ->ਮਾਪ : 1;
        ਜਦੋਂ (ਹੋਰ < want)
            ਹੋਰ <<= 1;
        ਗੰਢ->ਸੱਜਾ = ਸੈੱਟ_alloc(ਸੈੱਟ, ਗੰਢ->ਸੱਜਾ,
                                (size_t)ਹੋਰ * ਆਕਾਰ(ਸੈੱਟ_ਗੰਢ_t *));
        ਗੰਢ->ਮਾਪ = (i16_t)ਹੋਰ;
    }
    ਜੇ (fill)
        ਲਈ (i = ਗੰਢ->fill; i < want; i++)
            ਗੰਢ->ਸੱਜਾ[i] = ਸੈੱਟ->ਸਿਰ;
    ਗੰਢ->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
ਸਥਿਰ ਸੈੱਟ_ਗੰਢ_t *ਸੈੱਟ_ਗੰਢ(ਸੈੱਟ_t *ਸੈੱਟ) {
    ਸੈੱਟ_ਗੰਢ_t *ਗੰਢ = ਸੈੱਟ_alloc(ਸੈੱਟ, ਨਿਰਾ, ਆਕਾਰ(ਸੈੱਟ_ਗੰਢ_t));
    ਗੰਢ->ਮਾਪ = 0;
    ਗੰਢ->fill = 0;
    ਗੰਢ->ਸੱਜਾ = ਨਿਰਾ;
    ਵਾਪਸ ਗੰਢ;
}

/* --private-- Free the list linked from head, along with the keys. */
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_sweep(ਸੈੱਟ_t *ਸੈੱਟ) {
    ਸੈੱਟ_ਗੰਢ_t *ਕਦਮ = ਸੈੱਟ->ਸਿਰ->ਸੱਜਾ[0];
    ਜਦੋਂ (ਕਦਮ != ਸੈੱਟ->ਸਿਰ) {
        ਸੈੱਟ_ਗੰਢ_t *ਅਗਲਾ = ਕਦਮ->ਸੱਜਾ[0];      /* save link to next node */
        ਸੈੱਟ_drop(ਸੈੱਟ, ਕਦਮ->ਕੁੰਜੀ);
        ਸੈੱਟ_free(ਸੈੱਟ, ਕਦਮ->ਸੱਜਾ);
        ਸੈੱਟ_free(ਸੈੱਟ, ਕਦਮ);
        ਕਦਮ = ਅਗਲਾ;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT ਖਾਲੀ ਸੈੱਟ_ਸ਼ੁਰੂ(ਸੈੱਟ_t *ਸੈੱਟ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸੈੱਟ_TRACK
    ਸੈੱਟ->allocs = 0;
    ਸੈੱਟ->ਸਮ੍ਰਿਤੀ = 0;
#ਜੇ_ਅੰਤ
    ਸੈੱਟ->ਸਿਰ = ਸੈੱਟ->ਰਸਤਾ = ਸੈੱਟ->ਗੰਢ = ਨਿਰਾ;   /* in case set_node() fails */
    ਸੈੱਟ->ਰਸਤਾ = ਸੈੱਟ_ਗੰਢ(ਸੈੱਟ);
    ਸੈੱਟ->ਸਿਰ = ਸੈੱਟ_ਗੰਢ(ਸੈੱਟ);
    ਸੈੱਟ_grow(ਸੈੱਟ, ਸੈੱਟ->ਸਿਰ, 1, 1); /* one link back to head for empty set */
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&ਸੈੱਟ->ਸਿਰ->ਕੁੰਜੀ = 137;    /* set id */
    ਸੈੱਟ->ਡੂੰਘਾਈ = 0;
    ਸੈੱਟ_uniq(&ਸੈੱਟ->gen, ਸੈੱਟ);
    ਸੈੱਟ->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT ਪੂਰਨ ਸੈੱਟ_ok(ਸੈੱਟ_t *ਸੈੱਟ) {
    ਵਾਪਸ ਸੈੱਟ->ਸਿਰ != ਨਿਰਾ &&
           ਸੈੱਟ->ਸਿਰ->ਸੱਜਾ != ਨਿਰਾ &&
           *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&ਸੈੱਟ->ਸਿਰ->ਕੁੰਜੀ == 137;
}

#ਜੇ 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT ਖਾਲੀ ਸੈੱਟ_clear(ਸੈੱਟ_t *ਸੈੱਟ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਸੈੱਟ_ok(ਸੈੱਟ) && "improper use");

    /* Free all the keys and their nodes. */
    ਸੈੱਟ_sweep(ਸੈੱਟ);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    ਸੈੱਟ->ਸਿਰ->ਸੱਜਾ[0] = ਸੈੱਟ->ਸਿਰ;
    ਸੈੱਟ->ਸਿਰ->fill = 1;
    ਸੈੱਟ->ਰਸਤਾ->fill = 0;
    ਸੈੱਟ->ਡੂੰਘਾਈ = 0;
}
#ਜੇ_ਅੰਤ

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT ਖਾਲੀ ਸੈੱਟ_ਸਿਰਾ(ਸੈੱਟ_t *ਸੈੱਟ) {
    ਜੇ (ਸੈੱਟ->ਸਿਰ != ਨਿਰਾ) {
        /* Empty the set and free the head node. */
        ਜੇ (ਸੈੱਟ->ਸਿਰ->ਸੱਜਾ != ਨਿਰਾ) {
            ਸੈੱਟ_sweep(ਸੈੱਟ);
            ਸੈੱਟ_free(ਸੈੱਟ, ਸੈੱਟ->ਸਿਰ->ਸੱਜਾ);
        }
        ਸੈੱਟ_free(ਸੈੱਟ, ਸੈੱਟ->ਸਿਰ);
        ਸੈੱਟ->ਸਿਰ = ਨਿਰਾ;
    }
    ਜੇ (ਸੈੱਟ->ਰਸਤਾ != ਨਿਰਾ) {
        /* Free the path work area. */
        ਸੈੱਟ_free(ਸੈੱਟ, ਸੈੱਟ->ਰਸਤਾ->ਸੱਜਾ);
        ਸੈੱਟ_free(ਸੈੱਟ, ਸੈੱਟ->ਰਸਤਾ);
        ਸੈੱਟ->ਰਸਤਾ = ਨਿਰਾ;
    }
    ਜੇ (ਸੈੱਟ->ਗੰਢ != ਨਿਰਾ) {
        /* Free the node that was under construction when longjmp() hit. */
        ਸੈੱਟ_drop(ਸੈੱਟ, ਸੈੱਟ->ਗੰਢ->ਕੁੰਜੀ);
        ਸੈੱਟ_free(ਸੈੱਟ, ਸੈੱਟ->ਗੰਢ->ਸੱਜਾ);
        ਸੈੱਟ_free(ਸੈੱਟ, ਸੈੱਟ->ਗੰਢ);
        ਸੈੱਟ->ਗੰਢ = ਨਿਰਾ;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT ਪੂਰਨ ਸੈੱਟ_ਮਿਲਿਆ(ਸੈੱਟ_t *ਸੈੱਟ, ਸੈੱਟ_ਕੁੰਜੀ_t ਕੁੰਜੀ) {
    ਸੈੱਟ_ਗੰਢ_t *ਸਿਰ, *ਇੱਥੇ;
    ਪੂਰਨ i;
    ਪ੍ਰਤਿਜ੍ਞਾ(ਸੈੱਟ_ok(ਸੈੱਟ) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    ਸਿਰ = ਸੈੱਟ->ਸਿਰ;
    ਇੱਥੇ = ਸਿਰ;
    i = ਸੈੱਟ->ਡੂੰਘਾਈ;
    ਸੈੱਟ_grow(ਸੈੱਟ, ਸੈੱਟ->ਰਸਤਾ, i + 1, 0);
    ਕਰੋ {
        ਜਦੋਂ (ਇੱਥੇ->ਸੱਜਾ[i] != ਸਿਰ &&
               ਸੈੱਟ_cmp(ਇੱਥੇ->ਸੱਜਾ[i]->ਕੁੰਜੀ, ਕੁੰਜੀ) < 0)
            ਇੱਥੇ = ਇੱਥੇ->ਸੱਜਾ[i];
        ਸੈੱਟ->ਰਸਤਾ->ਸੱਜਾ[i] = ਇੱਥੇ;
    } ਜਦੋਂ (i--);

    /* See if the key matches. */
    ਇੱਥੇ = ਇੱਥੇ->ਸੱਜਾ[0];
    ਵਾਪਸ ਇੱਥੇ != ਸਿਰ && ਸੈੱਟ_cmp(ਇੱਥੇ->ਕੁੰਜੀ, ਕੁੰਜੀ) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT ਪੂਰਨ ਸੈੱਟ_ਪਾਓ(ਸੈੱਟ_t *ਸੈੱਟ, ਸੈੱਟ_ਕੁੰਜੀ_t ਕੁੰਜੀ) {
    ਪੂਰਨ level = 0;
    ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ;
    ਪੂਰਨ i;
    ਪ੍ਰਤਿਜ੍ਞਾ(ਸੈੱਟ_ok(ਸੈੱਟ) && "improper use");

    ਜੇ (ਸੈੱਟ_ਮਿਲਿਆ(ਸੈੱਟ, ਕੁੰਜੀ))
        /* That key is already in the set. */
        ਵਾਪਸ 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    ਲਈ (;;) {
        ਜੇ (ਸੈੱਟ->ran == 1)
            /* Ran out. Get another 32 random bits. */
            ਸੈੱਟ->ran = ਸੈੱਟ_rand(&ਸੈੱਟ->gen) | (1ULL << 32);
        ਦੋਆਧਾਰੀ_ਅੰਕ = ਸੈੱਟ->ran & 1;
        ਸੈੱਟ->ran >>= 1;
        ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ)
            ਰੋਕੋ;
        ਪ੍ਰਤਿਜ੍ਞਾ(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    ਜੇ (level > ਸੈੱਟ->ਡੂੰਘਾਈ) {
        /* The maximum depth is now deeper. Update the structures. */
        ਸੈੱਟ_grow(ਸੈੱਟ, ਸੈੱਟ->ਰਸਤਾ, level + 1, 1);
        ਸੈੱਟ_grow(ਸੈੱਟ, ਸੈੱਟ->ਸਿਰ, level + 1, 1);
        ਸੈੱਟ->ਡੂੰਘਾਈ = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    ਸੈੱਟ->ਗੰਢ = ਸੈੱਟ_ਗੰਢ(ਸੈੱਟ);
    ਸੈੱਟ->ਗੰਢ->ਕੁੰਜੀ = ਕੁੰਜੀ;
    ਸੈੱਟ_grow(ਸੈੱਟ, ਸੈੱਟ->ਗੰਢ, level + 1, 0);
    ਲਈ (i = 0; i <= level; i++) {
        ਸੈੱਟ->ਗੰਢ->ਸੱਜਾ[i] = ਸੈੱਟ->ਰਸਤਾ->ਸੱਜਾ[i]->ਸੱਜਾ[i];
        ਸੈੱਟ->ਰਸਤਾ->ਸੱਜਾ[i]->ਸੱਜਾ[i] = ਸੈੱਟ->ਗੰਢ;
    }
    ਸੈੱਟ->ਗੰਢ = ਨਿਰਾ;
    ਵਾਪਸ 0;
}

#ਨਹੀਂਤਾਂ
#ਗਲਤੀ ** another skiplist ਸੈੱਟ already created ਇੱਥੇ
/* Would need to implement a prefix in order to support multiple sets. */
#ਜੇ_ਅੰਤ
