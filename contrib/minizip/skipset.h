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

#اگر_تعریف_نشده SKIPSET_H
#تعریف_کن SKIPSET_H

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
} مجموعه_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
ساكن فراغ مجموعه_دانه(مجموعه_rand_t *gen, ui64_t دانه, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (دانه + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
ساكن فراغ مجموعه_uniq(مجموعه_rand_t *gen, ثابت فراغ *ptr) {
    مجموعه_دانه(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(عدم) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
ساكن ui32_t مجموعه_rand(مجموعه_rand_t *gen) {
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
تعريفنوع بنية مجموعه_گره_s مجموعه_گره_t;
بنية مجموعه_گره_s {
    مجموعه_کلید_t کلید;          /* the key (not used for head or path) */
    i16_t اندازه;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    مجموعه_گره_t **راست_2;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
تعريفنوع بنية مجموعه_s {
    مجموعه_گره_t *سر;       /* skiplist head -- no key, just links */
    مجموعه_گره_t *مسیر;       /* right[] is path to key from set_found() */
    مجموعه_گره_t *گره;       /* node under construction, in case of longjmp() */
    i16_t عمق;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    مجموعه_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#اگر_تعریف_شده مجموعه_TRACK
    size_t allocs;          /* number of allocations */
    size_t حافظه;          /* total size of allocated memory (>= requests) */
#پایان_اگر
} مجموعه_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#اگر_تعریف_شده مجموعه_TRACK
/* Track the number of allocations and the total backing memory size. */
#  إذا defined(_WIN32)
#    include <malloc.h>
#    define مجموعه_ALLOC_اندازه(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define مجموعه_ALLOC_اندازه(ptr) malloc_اندازه(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define مجموعه_ALLOC_اندازه(ptr) malloc_usable_اندازه(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define مجموعه_ALLOC_اندازه(ptr) malloc_usable_اندازه(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define مجموعه_ALLOC_اندازه(ptr) malloc_usable_اندازه(ptr)
#  وإلا     /* e.g. OpenBSD */
#    define مجموعه_ALLOC_اندازه(ptr) 0
#  endif
/* With tracking. */
ساكن فراغ *مجموعه_alloc(مجموعه_t *مجموعه, فراغ *ptr, size_t اندازه) {
    size_t had = ptr == عدم ? 0 : مجموعه_ALLOC_اندازه(ptr);
    فراغ *mem = realloc(ptr, اندازه);
    إذا (mem == عدم)
        longjmp(مجموعه->env, ENOMEM);
    مجموعه->allocs += ptr == عدم;
    مجموعه->حافظه += مجموعه_ALLOC_اندازه(mem) - had;
    أرجع mem;
}
ساكن فراغ مجموعه_free(مجموعه_t *مجموعه, فراغ *ptr) {
    إذا (ptr != عدم) {
        مجموعه->allocs--;
        مجموعه->حافظه -= مجموعه_ALLOC_اندازه(ptr);
        حرر(ptr);
    }
}
#وإلا
/* Without tracking. */
ساكن فراغ *مجموعه_alloc(مجموعه_t *مجموعه, فراغ *ptr, size_t اندازه) {
    فراغ *mem = realloc(ptr, اندازه);
    إذا (mem == عدم)
        longjmp(مجموعه->env, ENOMEM);
    أرجع mem;
}
ساكن فراغ مجموعه_free(مجموعه_t *مجموعه, فراغ *ptr) {
    (فراغ)مجموعه;
    حرر(ptr);
}
#پایان_اگر

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
ساكن فراغ مجموعه_grow(مجموعه_t *مجموعه, مجموعه_گره_t *گره, صحيح want, صحيح fill) {
    صحيح i;
    إذا (گره->اندازه < want) {
        صحيح بیشتر = گره->اندازه ? گره->اندازه : 1;
        طالما (بیشتر < want)
            بیشتر <<= 1;
        گره->راست_2 = مجموعه_alloc(مجموعه, گره->راست_2,
                                (size_t)بیشتر * حجم(مجموعه_گره_t *));
        گره->اندازه = (i16_t)بیشتر;
    }
    إذا (fill)
        لكل (i = گره->fill; i < want; i++)
            گره->راست_2[i] = مجموعه->سر;
    گره->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
ساكن مجموعه_گره_t *مجموعه_گره(مجموعه_t *مجموعه) {
    مجموعه_گره_t *گره = مجموعه_alloc(مجموعه, عدم, حجم(مجموعه_گره_t));
    گره->اندازه = 0;
    گره->fill = 0;
    گره->راست_2 = عدم;
    أرجع گره;
}

/* --private-- Free the list linked from head, along with the keys. */
ساكن فراغ مجموعه_sweep(مجموعه_t *مجموعه) {
    مجموعه_گره_t *گام = مجموعه->سر->راست_2[0];
    طالما (گام != مجموعه->سر) {
        مجموعه_گره_t *بعدی = گام->راست_2[0];      /* save link to next node */
        مجموعه_drop(مجموعه, گام->کلید);
        مجموعه_free(مجموعه, گام->راست_2);
        مجموعه_free(مجموعه, گام);
        گام = بعدی;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT فراغ مجموعه_شروع(مجموعه_t *مجموعه) {
#اگر_تعریف_شده مجموعه_TRACK
    مجموعه->allocs = 0;
    مجموعه->حافظه = 0;
#پایان_اگر
    مجموعه->سر = مجموعه->مسیر = مجموعه->گره = عدم;   /* in case set_node() fails */
    مجموعه->مسیر = مجموعه_گره(مجموعه);
    مجموعه->سر = مجموعه_گره(مجموعه);
    مجموعه_grow(مجموعه, مجموعه->سر, 1, 1); /* one link back to head for empty set */
    *(بیعلامت حرف *)&مجموعه->سر->کلید = 137;    /* set id */
    مجموعه->عمق = 0;
    مجموعه_uniq(&مجموعه->gen, مجموعه);
    مجموعه->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT صحيح مجموعه_ok(مجموعه_t *مجموعه) {
    أرجع مجموعه->سر != عدم &&
           مجموعه->سر->راست_2 != عدم &&
           *(بیعلامت حرف *)&مجموعه->سر->کلید == 137;
}

#إذا 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT فراغ مجموعه_clear(مجموعه_t *مجموعه) {
    أكد(مجموعه_ok(مجموعه) && "improper use");

    /* Free all the keys and their nodes. */
    مجموعه_sweep(مجموعه);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    مجموعه->سر->راست_2[0] = مجموعه->سر;
    مجموعه->سر->fill = 1;
    مجموعه->مسیر->fill = 0;
    مجموعه->عمق = 0;
}
#پایان_اگر

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT فراغ مجموعه_انتها(مجموعه_t *مجموعه) {
    إذا (مجموعه->سر != عدم) {
        /* Empty the set and free the head node. */
        إذا (مجموعه->سر->راست_2 != عدم) {
            مجموعه_sweep(مجموعه);
            مجموعه_free(مجموعه, مجموعه->سر->راست_2);
        }
        مجموعه_free(مجموعه, مجموعه->سر);
        مجموعه->سر = عدم;
    }
    إذا (مجموعه->مسیر != عدم) {
        /* Free the path work area. */
        مجموعه_free(مجموعه, مجموعه->مسیر->راست_2);
        مجموعه_free(مجموعه, مجموعه->مسیر);
        مجموعه->مسیر = عدم;
    }
    إذا (مجموعه->گره != عدم) {
        /* Free the node that was under construction when longjmp() hit. */
        مجموعه_drop(مجموعه, مجموعه->گره->کلید);
        مجموعه_free(مجموعه, مجموعه->گره->راست_2);
        مجموعه_free(مجموعه, مجموعه->گره);
        مجموعه->گره = عدم;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT صحيح مجموعه_یافت(مجموعه_t *مجموعه, مجموعه_کلید_t کلید) {
    مجموعه_گره_t *سر, *اینجا;
    صحيح i;
    أكد(مجموعه_ok(مجموعه) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    سر = مجموعه->سر;
    اینجا = سر;
    i = مجموعه->عمق;
    مجموعه_grow(مجموعه, مجموعه->مسیر, i + 1, 0);
    افعل {
        طالما (اینجا->راست_2[i] != سر &&
               مجموعه_cmp(اینجا->راست_2[i]->کلید, کلید) < 0)
            اینجا = اینجا->راست_2[i];
        مجموعه->مسیر->راست_2[i] = اینجا;
    } طالما (i--);

    /* See if the key matches. */
    اینجا = اینجا->راست_2[0];
    أرجع اینجا != سر && مجموعه_cmp(اینجا->کلید, کلید) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT صحيح مجموعه_درج(مجموعه_t *مجموعه, مجموعه_کلید_t کلید) {
    صحيح level = 0;
    صحيح بیت;
    صحيح i;
    أكد(مجموعه_ok(مجموعه) && "improper use");

    إذا (مجموعه_یافت(مجموعه, کلید))
        /* That key is already in the set. */
        أرجع 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    لكل (;;) {
        إذا (مجموعه->ran == 1)
            /* Ran out. Get another 32 random bits. */
            مجموعه->ran = مجموعه_rand(&مجموعه->gen) | (1ULL << 32);
        بیت = مجموعه->ran & 1;
        مجموعه->ran >>= 1;
        إذا (بیت)
            اكسر;
        أكد(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    إذا (level > مجموعه->عمق) {
        /* The maximum depth is now deeper. Update the structures. */
        مجموعه_grow(مجموعه, مجموعه->مسیر, level + 1, 1);
        مجموعه_grow(مجموعه, مجموعه->سر, level + 1, 1);
        مجموعه->عمق = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    مجموعه->گره = مجموعه_گره(مجموعه);
    مجموعه->گره->کلید = کلید;
    مجموعه_grow(مجموعه, مجموعه->گره, level + 1, 0);
    لكل (i = 0; i <= level; i++) {
        مجموعه->گره->راست_2[i] = مجموعه->مسیر->راست_2[i]->راست_2[i];
        مجموعه->مسیر->راست_2[i]->راست_2[i] = مجموعه->گره;
    }
    مجموعه->گره = عدم;
    أرجع 0;
}

#وإلا
#خطا ** another skiplist مجموعه already created اینجا
/* Would need to implement a prefix in order to support multiple sets. */
#پایان_اگر
