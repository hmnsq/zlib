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

#నిర్వచితం_కాకపోతే SKIPSET_H
#నిర్వచించు SKIPSET_H

#అన్తర్భావ <stdlib.h>     /* realloc(), free(), NULL, size_t */
#అన్తర్భావ <stddef.h>     /* ptrdiff_t */
#అన్తర్భావ <setjmp.h>     /* jmp_buf, longjmp() */
#అన్తర్భావ <errno.h>      /* ENOMEM */
#అన్తర్భావ <time.h>       /* time(), clock() */
#అన్తర్భావ <assert.h>     /* assert.h */
#అన్తర్భావ "ints.h"       /* i16_t, ui32_t, ui64_t */

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
రకనిర్వచనం నిర్మాణం {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} సమితి_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
స్థిర శూన్య సమితి_విత్తనం(సమితి_rand_t *gen, ui64_t విత్తనం, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (విత్తనం + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
స్థిర శూన్య సమితి_uniq(సమితి_rand_t *gen, స్థిరరాశి శూన్య *ptr) {
    సమితి_విత్తనం(gen, ((ui64_t)(ptrdiff_t)ptr << 32) ^
                  ((ui64_t)time(ఖాళీ) << 12) ^ clock(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
స్థిర ui32_t సమితి_rand(సమితి_rand_t *gen) {
    ui32_t mix;
    పూర్ణాంకం rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    తిరిగి (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
రకనిర్వచనం నిర్మాణం సమితి_కణుపు_s సమితి_కణుపు_t;
నిర్మాణం సమితి_కణుపు_s {
    సమితి_కీలకం_t కీలకం;          /* the key (not used for head or path) */
    i16_t కొలత;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    సమితి_కణుపు_t **కుడి_2;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
రకనిర్వచనం నిర్మాణం సమితి_s {
    సమితి_కణుపు_t *తల;       /* skiplist head -- no key, just links */
    సమితి_కణుపు_t *మార్గం;       /* right[] is path to key from set_found() */
    సమితి_కణుపు_t *కణుపు;       /* node under construction, in case of longjmp() */
    i16_t లోతు;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    సమితి_rand_t gen;         /* random number generator state */
    jmp_buf env;            /* setjmp() environment for allocation errors */
#నిర్వచితమైతే సమితి_TRACK
    size_t allocs;          /* number of allocations */
    size_t జ్ఞాపకం;          /* total size of allocated memory (>= requests) */
#అయితే_ముగింపు
} సమితి_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#నిర్వచితమైతే సమితి_TRACK
/* Track the number of allocations and the total backing memory size. */
#  ఒకవేళ defined(_WIN32)
#    include <malloc.h>
#    define సమితి_ALLOC_కొలత(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define సమితి_ALLOC_కొలత(ptr) malloc_కొలత(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define సమితి_ALLOC_కొలత(ptr) malloc_usable_కొలత(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define సమితి_ALLOC_కొలత(ptr) malloc_usable_కొలత(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define సమితి_ALLOC_కొలత(ptr) malloc_usable_కొలత(ptr)
#  లేకపోతే     /* e.g. OpenBSD */
#    define సమితి_ALLOC_కొలత(ptr) 0
#  endif
/* With tracking. */
స్థిర శూన్య *సమితి_alloc(సమితి_t *సమితి, శూన్య *ptr, size_t కొలత) {
    size_t had = ptr == ఖాళీ ? 0 : సమితి_ALLOC_కొలత(ptr);
    శూన్య *mem = realloc(ptr, కొలత);
    ఒకవేళ (mem == ఖాళీ)
        longjmp(సమితి->env, ENOMEM);
    సమితి->allocs += ptr == ఖాళీ;
    సమితి->జ్ఞాపకం += సమితి_ALLOC_కొలత(mem) - had;
    తిరిగి mem;
}
స్థిర శూన్య సమితి_free(సమితి_t *సమితి, శూన్య *ptr) {
    ఒకవేళ (ptr != ఖాళీ) {
        సమితి->allocs--;
        సమితి->జ్ఞాపకం -= సమితి_ALLOC_కొలత(ptr);
        విడుదల(ptr);
    }
}
#లేకపోతే
/* Without tracking. */
స్థిర శూన్య *సమితి_alloc(సమితి_t *సమితి, శూన్య *ptr, size_t కొలత) {
    శూన్య *mem = realloc(ptr, కొలత);
    ఒకవేళ (mem == ఖాళీ)
        longjmp(సమితి->env, ENOMEM);
    తిరిగి mem;
}
స్థిర శూన్య సమితి_free(సమితి_t *సమితి, శూన్య *ptr) {
    (శూన్య)సమితి;
    విడుదల(ptr);
}
#అయితే_ముగింపు

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
స్థిర శూన్య సమితి_grow(సమితి_t *సమితి, సమితి_కణుపు_t *కణుపు, పూర్ణాంకం want, పూర్ణాంకం fill) {
    పూర్ణాంకం i;
    ఒకవేళ (కణుపు->కొలత < want) {
        పూర్ణాంకం మరిన్ని = కణుపు->కొలత ? కణుపు->కొలత : 1;
        అయితే (మరిన్ని < want)
            మరిన్ని <<= 1;
        కణుపు->కుడి_2 = సమితి_alloc(సమితి, కణుపు->కుడి_2,
                                (size_t)మరిన్ని * పరిమాణం(సమితి_కణుపు_t *));
        కణుపు->కొలత = (i16_t)మరిన్ని;
    }
    ఒకవేళ (fill)
        కొరకు (i = కణుపు->fill; i < want; i++)
            కణుపు->కుడి_2[i] = సమితి->తల;
    కణుపు->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
స్థిర సమితి_కణుపు_t *సమితి_కణుపు(సమితి_t *సమితి) {
    సమితి_కణుపు_t *కణుపు = సమితి_alloc(సమితి, ఖాళీ, పరిమాణం(సమితి_కణుపు_t));
    కణుపు->కొలత = 0;
    కణుపు->fill = 0;
    కణుపు->కుడి_2 = ఖాళీ;
    తిరిగి కణుపు;
}

/* --private-- Free the list linked from head, along with the keys. */
స్థిర శూన్య సమితి_sweep(సమితి_t *సమితి) {
    సమితి_కణుపు_t *అడుగు = సమితి->తల->కుడి_2[0];
    అయితే (అడుగు != సమితి->తల) {
        సమితి_కణుపు_t *తదుపరి = అడుగు->కుడి_2[0];      /* save link to next node */
        సమితి_drop(సమితి, అడుగు->కీలకం);
        సమితి_free(సమితి, అడుగు->కుడి_2);
        సమితి_free(సమితి, అడుగు);
        అడుగు = తదుపరి;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT శూన్య సమితి_ప్రారంభం(సమితి_t *సమితి) {
#నిర్వచితమైతే సమితి_TRACK
    సమితి->allocs = 0;
    సమితి->జ్ఞాపకం = 0;
#అయితే_ముగింపు
    సమితి->తల = సమితి->మార్గం = సమితి->కణుపు = ఖాళీ;   /* in case set_node() fails */
    సమితి->మార్గం = సమితి_కణుపు(సమితి);
    సమితి->తల = సమితి_కణుపు(సమితి);
    సమితి_grow(సమితి, సమితి->తల, 1, 1); /* one link back to head for empty set */
    *(గుర్తులేని అక్షరం *)&సమితి->తల->కీలకం = 137;    /* set id */
    సమితి->లోతు = 0;
    సమితి_uniq(&సమితి->gen, సమితి);
    సమితి->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT పూర్ణాంకం సమితి_ok(సమితి_t *సమితి) {
    తిరిగి సమితి->తల != ఖాళీ &&
           సమితి->తల->కుడి_2 != ఖాళీ &&
           *(గుర్తులేని అక్షరం *)&సమితి->తల->కీలకం == 137;
}

#ఒకవేళ 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT శూన్య సమితి_clear(సమితి_t *సమితి) {
    ప్రతిజ్ఞా(సమితి_ok(సమితి) && "improper use");

    /* Free all the keys and their nodes. */
    సమితి_sweep(సమితి);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    సమితి->తల->కుడి_2[0] = సమితి->తల;
    సమితి->తల->fill = 1;
    సమితి->మార్గం->fill = 0;
    సమితి->లోతు = 0;
}
#అయితే_ముగింపు

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT శూన్య సమితి_చివర(సమితి_t *సమితి) {
    ఒకవేళ (సమితి->తల != ఖాళీ) {
        /* Empty the set and free the head node. */
        ఒకవేళ (సమితి->తల->కుడి_2 != ఖాళీ) {
            సమితి_sweep(సమితి);
            సమితి_free(సమితి, సమితి->తల->కుడి_2);
        }
        సమితి_free(సమితి, సమితి->తల);
        సమితి->తల = ఖాళీ;
    }
    ఒకవేళ (సమితి->మార్గం != ఖాళీ) {
        /* Free the path work area. */
        సమితి_free(సమితి, సమితి->మార్గం->కుడి_2);
        సమితి_free(సమితి, సమితి->మార్గం);
        సమితి->మార్గం = ఖాళీ;
    }
    ఒకవేళ (సమితి->కణుపు != ఖాళీ) {
        /* Free the node that was under construction when longjmp() hit. */
        సమితి_drop(సమితి, సమితి->కణుపు->కీలకం);
        సమితి_free(సమితి, సమితి->కణుపు->కుడి_2);
        సమితి_free(సమితి, సమితి->కణుపు);
        సమితి->కణుపు = ఖాళీ;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT పూర్ణాంకం సమితి_దొరికింది(సమితి_t *సమితి, సమితి_కీలకం_t కీలకం) {
    సమితి_కణుపు_t *తల, *ఇక్కడ;
    పూర్ణాంకం i;
    ప్రతిజ్ఞా(సమితి_ok(సమితి) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    తల = సమితి->తల;
    ఇక్కడ = తల;
    i = సమితి->లోతు;
    సమితి_grow(సమితి, సమితి->మార్గం, i + 1, 0);
    చేయి {
        అయితే (ఇక్కడ->కుడి_2[i] != తల &&
               సమితి_cmp(ఇక్కడ->కుడి_2[i]->కీలకం, కీలకం) < 0)
            ఇక్కడ = ఇక్కడ->కుడి_2[i];
        సమితి->మార్గం->కుడి_2[i] = ఇక్కడ;
    } అయితే (i--);

    /* See if the key matches. */
    ఇక్కడ = ఇక్కడ->కుడి_2[0];
    తిరిగి ఇక్కడ != తల && సమితి_cmp(ఇక్కడ->కీలకం, కీలకం) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT పూర్ణాంకం సమితి_చొప్పించు(సమితి_t *సమితి, సమితి_కీలకం_t కీలకం) {
    పూర్ణాంకం level = 0;
    పూర్ణాంకం బిట్టు;
    పూర్ణాంకం i;
    ప్రతిజ్ఞా(సమితి_ok(సమితి) && "improper use");

    ఒకవేళ (సమితి_దొరికింది(సమితి, కీలకం))
        /* That key is already in the set. */
        తిరిగి 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    కొరకు (;;) {
        ఒకవేళ (సమితి->ran == 1)
            /* Ran out. Get another 32 random bits. */
            సమితి->ran = సమితి_rand(&సమితి->gen) | (1ULL << 32);
        బిట్టు = సమితి->ran & 1;
        సమితి->ran >>= 1;
        ఒకవేళ (బిట్టు)
            ఆపు;
        ప్రతిజ్ఞా(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    ఒకవేళ (level > సమితి->లోతు) {
        /* The maximum depth is now deeper. Update the structures. */
        సమితి_grow(సమితి, సమితి->మార్గం, level + 1, 1);
        సమితి_grow(సమితి, సమితి->తల, level + 1, 1);
        సమితి->లోతు = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    సమితి->కణుపు = సమితి_కణుపు(సమితి);
    సమితి->కణుపు->కీలకం = కీలకం;
    సమితి_grow(సమితి, సమితి->కణుపు, level + 1, 0);
    కొరకు (i = 0; i <= level; i++) {
        సమితి->కణుపు->కుడి_2[i] = సమితి->మార్గం->కుడి_2[i]->కుడి_2[i];
        సమితి->మార్గం->కుడి_2[i]->కుడి_2[i] = సమితి->కణుపు;
    }
    సమితి->కణుపు = ఖాళీ;
    తిరిగి 0;
}

#లేకపోతే
#దోషం ** another skiplist సమితి already created ఇక్కడ
/* Would need to implement a prefix in order to support multiple sets. */
#అయితే_ముగింపు
