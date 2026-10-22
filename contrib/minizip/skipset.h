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

#جے_بنا_تعریف SKIPSET_H
#تعریف_کرو SKIPSET_H

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
} سیٹ_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
ساكن فراغ سیٹ_بیج(سیٹ_rand_t *gen, ui64_t بیج, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (بیج + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
ساكن فراغ سیٹ_uniq(سیٹ_rand_t *gen, ثابت فراغ *ptr) {
    سیٹ_بیج(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(عدم) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
ساكن ui32_t سیٹ_rand(سیٹ_rand_t *gen) {
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
تعريفنوع بنية سیٹ_گرہ_s سیٹ_گرہ_t;
بنية سیٹ_گرہ_s {
    سیٹ_کلید_t کلید;          /* the key (not used for head or path) */
    i16_t ناپ;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    سیٹ_گرہ_t **دایاں;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
تعريفنوع بنية سیٹ_s {
    سیٹ_گرہ_t *سر;       /* skiplist head -- no key, just links */
    سیٹ_گرہ_t *رستہ;       /* right[] is path to key from set_found() */
    سیٹ_گرہ_t *گرہ;       /* node under construction, in case of longjmp() */
    i16_t ڈونگھائی;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    سیٹ_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#جے_تعریف_شدہ سیٹ_TRACK
    size_t allocs;          /* number of allocations */
    size_t حافظہ;          /* total size of allocated memory (>= requests) */
#جے_انت
} سیٹ_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#جے_تعریف_شدہ سیٹ_TRACK
/* Track the number of allocations and the total backing memory size. */
#  إذا defined(_WIN32)
#    include <malloc.h>
#    define سیٹ_ALLOC_ناپ(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define سیٹ_ALLOC_ناپ(ptr) malloc_ناپ(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define سیٹ_ALLOC_ناپ(ptr) malloc_usable_ناپ(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define سیٹ_ALLOC_ناپ(ptr) malloc_usable_ناپ(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define سیٹ_ALLOC_ناپ(ptr) malloc_usable_ناپ(ptr)
#  وإلا     /* e.g. OpenBSD */
#    define سیٹ_ALLOC_ناپ(ptr) 0
#  endif
/* With tracking. */
ساكن فراغ *سیٹ_alloc(سیٹ_t *سیٹ, فراغ *ptr, size_t ناپ) {
    size_t had = ptr == عدم ? 0 : سیٹ_ALLOC_ناپ(ptr);
    فراغ *mem = realloc(ptr, ناپ);
    إذا (mem == عدم)
        longjmp(سیٹ->env, ENOMEM);
    سیٹ->allocs += ptr == عدم;
    سیٹ->حافظہ += سیٹ_ALLOC_ناپ(mem) - had;
    أرجع mem;
}
ساكن فراغ سیٹ_free(سیٹ_t *سیٹ, فراغ *ptr) {
    إذا (ptr != عدم) {
        سیٹ->allocs--;
        سیٹ->حافظہ -= سیٹ_ALLOC_ناپ(ptr);
        حرر(ptr);
    }
}
#وإلا
/* Without tracking. */
ساكن فراغ *سیٹ_alloc(سیٹ_t *سیٹ, فراغ *ptr, size_t ناپ) {
    فراغ *mem = realloc(ptr, ناپ);
    إذا (mem == عدم)
        longjmp(سیٹ->env, ENOMEM);
    أرجع mem;
}
ساكن فراغ سیٹ_free(سیٹ_t *سیٹ, فراغ *ptr) {
    (فراغ)سیٹ;
    حرر(ptr);
}
#جے_انت

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
ساكن فراغ سیٹ_grow(سیٹ_t *سیٹ, سیٹ_گرہ_t *گرہ, صحيح want, صحيح fill) {
    صحيح i;
    إذا (گرہ->ناپ < want) {
        صحيح ہور = گرہ->ناپ ? گرہ->ناپ : 1;
        طالما (ہور < want)
            ہور <<= 1;
        گرہ->دایاں = سیٹ_alloc(سیٹ, گرہ->دایاں,
                                (size_t)ہور * حجم(سیٹ_گرہ_t *));
        گرہ->ناپ = (i16_t)ہور;
    }
    إذا (fill)
        لكل (i = گرہ->fill; i < want; i++)
            گرہ->دایاں[i] = سیٹ->سر;
    گرہ->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
ساكن سیٹ_گرہ_t *سیٹ_گرہ(سیٹ_t *سیٹ) {
    سیٹ_گرہ_t *گرہ = سیٹ_alloc(سیٹ, عدم, حجم(سیٹ_گرہ_t));
    گرہ->ناپ = 0;
    گرہ->fill = 0;
    گرہ->دایاں = عدم;
    أرجع گرہ;
}

/* --private-- Free the list linked from head, along with the keys. */
ساكن فراغ سیٹ_sweep(سیٹ_t *سیٹ) {
    سیٹ_گرہ_t *قدم = سیٹ->سر->دایاں[0];
    طالما (قدم != سیٹ->سر) {
        سیٹ_گرہ_t *اگلا = قدم->دایاں[0];      /* save link to next node */
        سیٹ_drop(سیٹ, قدم->کلید);
        سیٹ_free(سیٹ, قدم->دایاں);
        سیٹ_free(سیٹ, قدم);
        قدم = اگلا;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT فراغ سیٹ_شروع(سیٹ_t *سیٹ) {
#جے_تعریف_شدہ سیٹ_TRACK
    سیٹ->allocs = 0;
    سیٹ->حافظہ = 0;
#جے_انت
    سیٹ->سر = سیٹ->رستہ = سیٹ->گرہ = عدم;   /* in case set_node() fails */
    سیٹ->رستہ = سیٹ_گرہ(سیٹ);
    سیٹ->سر = سیٹ_گرہ(سیٹ);
    سیٹ_grow(سیٹ, سیٹ->سر, 1, 1); /* one link back to head for empty set */
    *(بنانشانی حرف *)&سیٹ->سر->کلید = 137;    /* set id */
    سیٹ->ڈونگھائی = 0;
    سیٹ_uniq(&سیٹ->gen, سیٹ);
    سیٹ->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT صحيح سیٹ_ok(سیٹ_t *سیٹ) {
    أرجع سیٹ->سر != عدم &&
           سیٹ->سر->دایاں != عدم &&
           *(بنانشانی حرف *)&سیٹ->سر->کلید == 137;
}

#إذا 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT فراغ سیٹ_clear(سیٹ_t *سیٹ) {
    أكد(سیٹ_ok(سیٹ) && "improper use");

    /* Free all the keys and their nodes. */
    سیٹ_sweep(سیٹ);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    سیٹ->سر->دایاں[0] = سیٹ->سر;
    سیٹ->سر->fill = 1;
    سیٹ->رستہ->fill = 0;
    سیٹ->ڈونگھائی = 0;
}
#جے_انت

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT فراغ سیٹ_سرا(سیٹ_t *سیٹ) {
    إذا (سیٹ->سر != عدم) {
        /* Empty the set and free the head node. */
        إذا (سیٹ->سر->دایاں != عدم) {
            سیٹ_sweep(سیٹ);
            سیٹ_free(سیٹ, سیٹ->سر->دایاں);
        }
        سیٹ_free(سیٹ, سیٹ->سر);
        سیٹ->سر = عدم;
    }
    إذا (سیٹ->رستہ != عدم) {
        /* Free the path work area. */
        سیٹ_free(سیٹ, سیٹ->رستہ->دایاں);
        سیٹ_free(سیٹ, سیٹ->رستہ);
        سیٹ->رستہ = عدم;
    }
    إذا (سیٹ->گرہ != عدم) {
        /* Free the node that was under construction when longjmp() hit. */
        سیٹ_drop(سیٹ, سیٹ->گرہ->کلید);
        سیٹ_free(سیٹ, سیٹ->گرہ->دایاں);
        سیٹ_free(سیٹ, سیٹ->گرہ);
        سیٹ->گرہ = عدم;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT صحيح سیٹ_لبھیا(سیٹ_t *سیٹ, سیٹ_کلید_t کلید) {
    سیٹ_گرہ_t *سر, *ایتھے;
    صحيح i;
    أكد(سیٹ_ok(سیٹ) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    سر = سیٹ->سر;
    ایتھے = سر;
    i = سیٹ->ڈونگھائی;
    سیٹ_grow(سیٹ, سیٹ->رستہ, i + 1, 0);
    افعل {
        طالما (ایتھے->دایاں[i] != سر &&
               سیٹ_cmp(ایتھے->دایاں[i]->کلید, کلید) < 0)
            ایتھے = ایتھے->دایاں[i];
        سیٹ->رستہ->دایاں[i] = ایتھے;
    } طالما (i--);

    /* See if the key matches. */
    ایتھے = ایتھے->دایاں[0];
    أرجع ایتھے != سر && سیٹ_cmp(ایتھے->کلید, کلید) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT صحيح سیٹ_داخل_کرو(سیٹ_t *سیٹ, سیٹ_کلید_t کلید) {
    صحيح level = 0;
    صحيح ثنائی_ہندسہ;
    صحيح i;
    أكد(سیٹ_ok(سیٹ) && "improper use");

    إذا (سیٹ_لبھیا(سیٹ, کلید))
        /* That key is already in the set. */
        أرجع 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    لكل (;;) {
        إذا (سیٹ->ran == 1)
            /* Ran out. Get another 32 random bits. */
            سیٹ->ran = سیٹ_rand(&سیٹ->gen) | (1ULL << 32);
        ثنائی_ہندسہ = سیٹ->ran & 1;
        سیٹ->ran >>= 1;
        إذا (ثنائی_ہندسہ)
            اكسر;
        أكد(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    إذا (level > سیٹ->ڈونگھائی) {
        /* The maximum depth is now deeper. Update the structures. */
        سیٹ_grow(سیٹ, سیٹ->رستہ, level + 1, 1);
        سیٹ_grow(سیٹ, سیٹ->سر, level + 1, 1);
        سیٹ->ڈونگھائی = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    سیٹ->گرہ = سیٹ_گرہ(سیٹ);
    سیٹ->گرہ->کلید = کلید;
    سیٹ_grow(سیٹ, سیٹ->گرہ, level + 1, 0);
    لكل (i = 0; i <= level; i++) {
        سیٹ->گرہ->دایاں[i] = سیٹ->رستہ->دایاں[i]->دایاں[i];
        سیٹ->رستہ->دایاں[i]->دایاں[i] = سیٹ->گرہ;
    }
    سیٹ->گرہ = عدم;
    أرجع 0;
}

#وإلا
#غلطی ** another skiplist سیٹ already created ایتھے
/* Would need to implement a prefix in order to support multiple sets. */
#جے_انت
