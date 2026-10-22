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

#جيڪڏهن_وصف_نه_ٿيل SKIPSET_H
#وصف_ڪريو SKIPSET_H

#ضمن <stdlib.h>     /* realloc(), free(), NULL, size_t */
#ضمن <stddef.h>     /* ptrdiff_t */
#ضمن <setjmp.h>     /* jmp_buf, longjmp() */
#ضمن <errno.h>      /* ENOMEM */
#ضمن <time.h>       /* time(), clock() */
#ضمن <assert.h>     /* assert.h */
#ضمن "ints.h"       /* i16_t, ui32_t, ui64_t */

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
تعريفنوع بنية {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} سيٽ_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
ساكن فراغ سيٽ_ٻج(سيٽ_rand_t *gen, ui64_t ٻج, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (ٻج + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
ساكن فراغ سيٽ_uniq(سيٽ_rand_t *gen, ثابت فراغ *ptr) {
    سيٽ_ٻج(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(عدم) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
ساكن ui32_t سيٽ_rand(سيٽ_rand_t *gen) {
    ui32_t mix;
    صحيح rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    أرجع (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
تعريفنوع بنية سيٽ_ڳنڍ_s سيٽ_ڳنڍ_t;
بنية سيٽ_ڳنڍ_s {
    سيٽ_ڪنجي_t ڪنجي;          /* the key (not used for head or path) */
    i16_t سائيز;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    سيٽ_ڳنڍ_t **ساڄو;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
تعريفنوع بنية سيٽ_s {
    سيٽ_ڳنڍ_t *سر;       /* skiplist head -- no key, just links */
    سيٽ_ڳنڍ_t *رستو;       /* right[] is path to key from set_found() */
    سيٽ_ڳنڍ_t *ڳنڍ;       /* node under construction, in case of longjmp() */
    i16_t اونهائي;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    سيٽ_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#جيڪڏهن_وصف_ٿيل سيٽ_TRACK
    size_t allocs;          /* number of allocations */
    size_t يادگيري;          /* total size of allocated memory (>= requests) */
#جيڪڏهن_پورو
} سيٽ_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#جيڪڏهن_وصف_ٿيل سيٽ_TRACK
/* Track the number of allocations and the total backing memory size. */
#  إذا defined(_WIN32)
#    include <malloc.h>
#    define سيٽ_ALLOC_سائيز(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define سيٽ_ALLOC_سائيز(ptr) malloc_سائيز(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define سيٽ_ALLOC_سائيز(ptr) malloc_usable_سائيز(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define سيٽ_ALLOC_سائيز(ptr) malloc_usable_سائيز(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define سيٽ_ALLOC_سائيز(ptr) malloc_usable_سائيز(ptr)
#  وإلا     /* e.g. OpenBSD */
#    define سيٽ_ALLOC_سائيز(ptr) 0
#  endif
/* With tracking. */
ساكن فراغ *سيٽ_alloc(سيٽ_t *سيٽ, فراغ *ptr, size_t سائيز) {
    size_t had = ptr == عدم ? 0 : سيٽ_ALLOC_سائيز(ptr);
    فراغ *mem = realloc(ptr, سائيز);
    إذا (mem == عدم)
        longjmp(سيٽ->env, ENOMEM);
    سيٽ->allocs += ptr == عدم;
    سيٽ->يادگيري += سيٽ_ALLOC_سائيز(mem) - had;
    أرجع mem;
}
ساكن فراغ سيٽ_free(سيٽ_t *سيٽ, فراغ *ptr) {
    إذا (ptr != عدم) {
        سيٽ->allocs--;
        سيٽ->يادگيري -= سيٽ_ALLOC_سائيز(ptr);
        حرر(ptr);
    }
}
#وإلا
/* Without tracking. */
ساكن فراغ *سيٽ_alloc(سيٽ_t *سيٽ, فراغ *ptr, size_t سائيز) {
    فراغ *mem = realloc(ptr, سائيز);
    إذا (mem == عدم)
        longjmp(سيٽ->env, ENOMEM);
    أرجع mem;
}
ساكن فراغ سيٽ_free(سيٽ_t *سيٽ, فراغ *ptr) {
    (فراغ)سيٽ;
    حرر(ptr);
}
#جيڪڏهن_پورو

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
ساكن فراغ سيٽ_grow(سيٽ_t *سيٽ, سيٽ_ڳنڍ_t *ڳنڍ, صحيح want, صحيح fill) {
    صحيح i;
    إذا (ڳنڍ->سائيز < want) {
        صحيح وڌيڪ = ڳنڍ->سائيز ? ڳنڍ->سائيز : 1;
        طالما (وڌيڪ < want)
            وڌيڪ <<= 1;
        ڳنڍ->ساڄو = سيٽ_alloc(سيٽ, ڳنڍ->ساڄو,
                                (size_t)وڌيڪ * حجم(سيٽ_ڳنڍ_t *));
        ڳنڍ->سائيز = (i16_t)وڌيڪ;
    }
    إذا (fill)
        لكل (i = ڳنڍ->fill; i < want; i++)
            ڳنڍ->ساڄو[i] = سيٽ->سر;
    ڳنڍ->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
ساكن سيٽ_ڳنڍ_t *سيٽ_ڳنڍ(سيٽ_t *سيٽ) {
    سيٽ_ڳنڍ_t *ڳنڍ = سيٽ_alloc(سيٽ, عدم, حجم(سيٽ_ڳنڍ_t));
    ڳنڍ->سائيز = 0;
    ڳنڍ->fill = 0;
    ڳنڍ->ساڄو = عدم;
    أرجع ڳنڍ;
}

/* --private-- Free the list linked from head, along with the keys. */
ساكن فراغ سيٽ_sweep(سيٽ_t *سيٽ) {
    سيٽ_ڳنڍ_t *قدم = سيٽ->سر->ساڄو[0];
    طالما (قدم != سيٽ->سر) {
        سيٽ_ڳنڍ_t *ايندڙ = قدم->ساڄو[0];      /* save link to next node */
        سيٽ_drop(سيٽ, قدم->ڪنجي);
        سيٽ_free(سيٽ, قدم->ساڄو);
        سيٽ_free(سيٽ, قدم);
        قدم = ايندڙ;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT فراغ سيٽ_شروعات(سيٽ_t *سيٽ) {
#جيڪڏهن_وصف_ٿيل سيٽ_TRACK
    سيٽ->allocs = 0;
    سيٽ->يادگيري = 0;
#جيڪڏهن_پورو
    سيٽ->سر = سيٽ->رستو = سيٽ->ڳنڍ = عدم;   /* in case set_node() fails */
    سيٽ->رستو = سيٽ_ڳنڍ(سيٽ);
    سيٽ->سر = سيٽ_ڳنڍ(سيٽ);
    سيٽ_grow(سيٽ, سيٽ->سر, 1, 1); /* one link back to head for empty set */
    *(بنانشان حرف *)&سيٽ->سر->ڪنجي = 137;    /* set id */
    سيٽ->اونهائي = 0;
    سيٽ_uniq(&سيٽ->gen, سيٽ);
    سيٽ->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT صحيح سيٽ_ok(سيٽ_t *سيٽ) {
    أرجع سيٽ->سر != عدم &&
           سيٽ->سر->ساڄو != عدم &&
           *(بنانشان حرف *)&سيٽ->سر->ڪنجي == 137;
}

#إذا 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT فراغ سيٽ_clear(سيٽ_t *سيٽ) {
    أكد(سيٽ_ok(سيٽ) && "improper use");

    /* Free all the keys and their nodes. */
    سيٽ_sweep(سيٽ);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    سيٽ->سر->ساڄو[0] = سيٽ->سر;
    سيٽ->سر->fill = 1;
    سيٽ->رستو->fill = 0;
    سيٽ->اونهائي = 0;
}
#جيڪڏهن_پورو

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT فراغ سيٽ_پڇاڙي(سيٽ_t *سيٽ) {
    إذا (سيٽ->سر != عدم) {
        /* Empty the set and free the head node. */
        إذا (سيٽ->سر->ساڄو != عدم) {
            سيٽ_sweep(سيٽ);
            سيٽ_free(سيٽ, سيٽ->سر->ساڄو);
        }
        سيٽ_free(سيٽ, سيٽ->سر);
        سيٽ->سر = عدم;
    }
    إذا (سيٽ->رستو != عدم) {
        /* Free the path work area. */
        سيٽ_free(سيٽ, سيٽ->رستو->ساڄو);
        سيٽ_free(سيٽ, سيٽ->رستو);
        سيٽ->رستو = عدم;
    }
    إذا (سيٽ->ڳنڍ != عدم) {
        /* Free the node that was under construction when longjmp() hit. */
        سيٽ_drop(سيٽ, سيٽ->ڳنڍ->ڪنجي);
        سيٽ_free(سيٽ, سيٽ->ڳنڍ->ساڄو);
        سيٽ_free(سيٽ, سيٽ->ڳنڍ);
        سيٽ->ڳنڍ = عدم;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT صحيح سيٽ_مليو(سيٽ_t *سيٽ, سيٽ_ڪنجي_t ڪنجي) {
    سيٽ_ڳنڍ_t *سر, *هتي;
    صحيح i;
    أكد(سيٽ_ok(سيٽ) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    سر = سيٽ->سر;
    هتي = سر;
    i = سيٽ->اونهائي;
    سيٽ_grow(سيٽ, سيٽ->رستو, i + 1, 0);
    افعل {
        طالما (هتي->ساڄو[i] != سر &&
               سيٽ_cmp(هتي->ساڄو[i]->ڪنجي, ڪنجي) < 0)
            هتي = هتي->ساڄو[i];
        سيٽ->رستو->ساڄو[i] = هتي;
    } طالما (i--);

    /* See if the key matches. */
    هتي = هتي->ساڄو[0];
    أرجع هتي != سر && سيٽ_cmp(هتي->ڪنجي, ڪنجي) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT صحيح سيٽ_وجهو(سيٽ_t *سيٽ, سيٽ_ڪنجي_t ڪنجي) {
    صحيح level = 0;
    صحيح ٻٽو_انگ;
    صحيح i;
    أكد(سيٽ_ok(سيٽ) && "improper use");

    إذا (سيٽ_مليو(سيٽ, ڪنجي))
        /* That key is already in the set. */
        أرجع 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    لكل (;;) {
        إذا (سيٽ->ran == 1)
            /* Ran out. Get another 32 random bits. */
            سيٽ->ran = سيٽ_rand(&سيٽ->gen) | (1ULL << 32);
        ٻٽو_انگ = سيٽ->ran & 1;
        سيٽ->ran >>= 1;
        إذا (ٻٽو_انگ)
            اكسر;
        أكد(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    إذا (level > سيٽ->اونهائي) {
        /* The maximum depth is now deeper. Update the structures. */
        سيٽ_grow(سيٽ, سيٽ->رستو, level + 1, 1);
        سيٽ_grow(سيٽ, سيٽ->سر, level + 1, 1);
        سيٽ->اونهائي = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    سيٽ->ڳنڍ = سيٽ_ڳنڍ(سيٽ);
    سيٽ->ڳنڍ->ڪنجي = ڪنجي;
    سيٽ_grow(سيٽ, سيٽ->ڳنڍ, level + 1, 0);
    لكل (i = 0; i <= level; i++) {
        سيٽ->ڳنڍ->ساڄو[i] = سيٽ->رستو->ساڄو[i]->ساڄو[i];
        سيٽ->رستو->ساڄو[i]->ساڄو[i] = سيٽ->ڳنڍ;
    }
    سيٽ->ڳنڍ = عدم;
    أرجع 0;
}

#وإلا
#غلطي ** another skiplist سيٽ already created هتي
/* Would need to implement a prefix in order to support multiple sets. */
#جيڪڏهن_پورو
