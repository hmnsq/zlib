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

#अगर_अघोषित SKIPSET_H
#घोषणा SKIPSET_H

#समावेश <stdlib.h>     /* realloc(), free(), NULL, size_t */
#समावेश <stddef.h>     /* ptrdiff_t */
#समावेश <setjmp.h>     /* jmp_buf, longjmp() */
#समावेश <errno.h>      /* ENOMEM */
#समावेश <time.h>       /* time(), clock() */
#समावेश <assert.h>     /* assert.h */
#समावेश "ints.h"       /* i16_t, ui32_t, ui64_t */

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
प्रकार काष्ठा {
    ui64_t state;       /* 64-bit generator state */
    ui64_t inc;         /* 63-bit sequence id */
} set_rand_t;
/* --private-- Initialize the state *gen using seed and seq. seed seeds the
   advancing 64-bit state. seq is a sequence selection constant. */
अटल व्योम set_seed(set_rand_t *gen, ui64_t seed, ui64_t seq) {
    gen->inc = (seq << 1) | 1;
    gen->state = (seed + gen->inc) * 6364136223846793005ULL + gen->inc;
}
/* Start a unique random number sequence using bits from noise sources. */
अटल व्योम set_uniq(set_rand_t *gen, स्थिर व्योम *ptr) {
    set_seed(gen, ((ui64_t)(सूचक_भेद_प्रकार)ptr << 32) ^
                  ((ui64_t)समय(शून्य) << 12) ^ घड़ी(), 0);
}
/* Return 32 random bits, advancing the state *gen. */
अटल ui32_t set_rand(set_rand_t *gen) {
    ui32_t mix;
    पूर्णांक rot;
    ui64_t state = gen->state;
    gen->state = state * 6364136223846793005ULL + gen->inc;
    mix = (ui32_t)(((state >> 18) ^ state) >> 27);
    rot = state >> 59;
    वापस (mix >> rot) | (mix << ((-rot) & 31));
}
/* End of PCG32 code. */

/* --private-- Linked-list node. */
प्रकार काष्ठा set_गाँठ_s set_गाँठ_t;
काष्ठा set_गाँठ_s {
    set_कुंजी_t कुंजी;          /* the key (not used for head or path) */
    i16_t आकार;             /* number of allocated pointers in right[] */
    i16_t fill;             /* number of pointers in right[] filled in */
    set_गाँठ_t **दायाँ;     /* pointer for each level, each to the right */
};

/* A set. The application sets env, may use gen with set_rand(), and may read
   allocs and memory. The remaining variables are --private-- . */
प्रकार काष्ठा set_s {
    set_गाँठ_t *शीर्ष;       /* skiplist head -- no key, just links */
    set_गाँठ_t *path;       /* right[] is path to key from set_found() */
    set_गाँठ_t *गाँठ;       /* node under construction, in case of longjmp() */
    i16_t depth;            /* maximum depth of the skiplist */
    ui64_t ran;             /* a precious trove of random bits */
    set_rand_t gen;         /* random number generator state */
    लाँघ_बफ env;            /* setjmp() environment for allocation errors */
#अगर_घोषित SET_TRACK
    माप_प्रकार allocs;          /* number of allocations */
    माप_प्रकार memory;          /* total size of allocated memory (>= requests) */
#पूर्ण_अगर
} set_t;

/* Memory allocation and deallocation. set_alloc(set, ptr, size) returns a
   pointer to an allocation of size bytes if ptr is NULL, or the previous
   allocation ptr resized to size bytes. set_alloc() will never return NULL.
   set_free(set, ptr) frees an allocation created by set_alloc(). These may be
   used by the application. e.g. if allocation tracking is desired. */
#अगर_घोषित SET_TRACK
/* Track the number of allocations and the total backing memory size. */
#  अगर defined(_WIN32)
#    include <malloc.h>
#    define SET_ALLOC_आकार(ptr) _msize(ptr)
#  elif defined(__MACH__)
#    include <malloc/malloc.h>
#    define SET_ALLOC_आकार(ptr) malloc_आकार(ptr)
#  elif defined(__linux__)
#    include <malloc.h>
#    define SET_ALLOC_आकार(ptr) malloc_usable_आकार(ptr)
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#    define SET_ALLOC_आकार(ptr) malloc_usable_आकार(ptr)
#  elif defined(__NetBSD__)
#    include <jemalloc/jemalloc.h>
#    define SET_ALLOC_आकार(ptr) malloc_usable_आकार(ptr)
#  अन्यथा     /* e.g. OpenBSD */
#    define SET_ALLOC_आकार(ptr) 0
#  endif
/* With tracking. */
अटल व्योम *set_alloc(set_t *set, व्योम *ptr, माप_प्रकार आकार) {
    माप_प्रकार had = ptr == शून्य ? 0 : SET_ALLOC_आकार(ptr);
    व्योम *mem = पुनः_स्मृति(ptr, आकार);
    अगर (mem == शून्य)
        दीर्घ_लाँघ(set->env, ENOMEM);
    set->allocs += ptr == शून्य;
    set->memory += SET_ALLOC_आकार(mem) - had;
    वापस mem;
}
अटल व्योम set_free(set_t *set, व्योम *ptr) {
    अगर (ptr != शून्य) {
        set->allocs--;
        set->memory -= SET_ALLOC_आकार(ptr);
        मुक्त(ptr);
    }
}
#अन्यथा
/* Without tracking. */
अटल व्योम *set_alloc(set_t *set, व्योम *ptr, माप_प्रकार आकार) {
    व्योम *mem = पुनः_स्मृति(ptr, आकार);
    अगर (mem == शून्य)
        दीर्घ_लाँघ(set->env, ENOMEM);
    वापस mem;
}
अटल व्योम set_free(set_t *set, व्योम *ptr) {
    (व्योम)set;
    मुक्त(ptr);
}
#पूर्ण_अगर

/* --private-- Grow node's array right[] as needed to be able to hold at least
   want links. If fill is true, assure that the first want links are filled in,
   setting them to set->head if not previously filled in. Otherwise it is
   assumed that the first want links are about to be filled in. */
अटल व्योम set_grow(set_t *set, set_गाँठ_t *गाँठ, पूर्णांक want, पूर्णांक fill) {
    पूर्णांक i;
    अगर (गाँठ->आकार < want) {
        पूर्णांक more = गाँठ->आकार ? गाँठ->आकार : 1;
        जबतक (more < want)
            more <<= 1;
        गाँठ->दायाँ = set_alloc(set, गाँठ->दायाँ,
                                (माप_प्रकार)more * माप(set_गाँठ_t *));
        गाँठ->आकार = (i16_t)more;
    }
    अगर (fill)
        क्रम (i = गाँठ->fill; i < want; i++)
            गाँठ->दायाँ[i] = set->शीर्ष;
    गाँठ->fill = (i16_t)want;
}

/* --private-- Return a new node. key is left uninitialized. */
अटल set_गाँठ_t *set_गाँठ(set_t *set) {
    set_गाँठ_t *गाँठ = set_alloc(set, शून्य, माप(set_गाँठ_t));
    गाँठ->आकार = 0;
    गाँठ->fill = 0;
    गाँठ->दायाँ = शून्य;
    वापस गाँठ;
}

/* --private-- Free the list linked from head, along with the keys. */
अटल व्योम set_sweep(set_t *set) {
    set_गाँठ_t *step = set->शीर्ष->दायाँ[0];
    जबतक (step != set->शीर्ष) {
        set_गाँठ_t *अग्र = step->दायाँ[0];      /* save link to next node */
        set_drop(set, step->कुंजी);
        set_free(set, step->दायाँ);
        set_free(set, step);
        step = अग्र;
    }
}

/* Initialize a new set. set->env must be initialized using setjmp() before
   set_start() is called. A longjmp(set->env, ENOMEM) will be used to handle a
   memory allocation failure during any of the operations. (See setjmp.h and
   errno.h.) The set can still be used if this happens, assuming that it didn't
   happen during set_start(). Whether set_start() completed or not, set_end()
   can be used to free the set's memory after a longjmp(). */
SKIPSET_EXPORT व्योम set_start(set_t *set) {
#अगर_घोषित SET_TRACK
    set->allocs = 0;
    set->memory = 0;
#पूर्ण_अगर
    set->शीर्ष = set->path = set->गाँठ = शून्य;   /* in case set_node() fails */
    set->path = set_गाँठ(set);
    set->शीर्ष = set_गाँठ(set);
    set_grow(set, set->शीर्ष, 1, 1); /* one link back to head for empty set */
    *(अचिन्हित अक्षर *)&set->शीर्ष->कुंजी = 137;    /* set id */
    set->depth = 0;
    set_uniq(&set->gen, set);
    set->ran = 1;
}

/* Return true if *set appears to be in a usable state. If *set has been zeroed
   out, then set_ok(set) will be false and set_end(set) will be safe. */
SKIPSET_EXPORT पूर्णांक set_ok(set_t *set) {
    वापस set->शीर्ष != शून्य &&
           set->शीर्ष->दायाँ != शून्य &&
           *(अचिन्हित अक्षर *)&set->शीर्ष->कुंजी == 137;
}

#अगर 0       /* not used in minizip */
/* Empty the set. This frees the memory used for the previous set contents.
   After set_clear(), *set is ready for use, as if after a set_start(). */
SKIPSET_EXPORT व्योम set_clear(set_t *set) {
    निश्चित(set_ok(set) && "improper use");

    /* Free all the keys and their nodes. */
    set_sweep(set);

    /* Leave the head and path allocations as is. Clear their contents, with
       head pointing to itself and setting depth to zero, for an empty set. */
    set->शीर्ष->दायाँ[0] = set->शीर्ष;
    set->शीर्ष->fill = 1;
    set->path->fill = 0;
    set->depth = 0;
}
#पूर्ण_अगर

/* Done using the set -- free all allocations. The only operation on *set
   permitted after this is set_start(). Though another set_end() would do no
   harm. This can be done at any time after a set_start(), or after a longjmp()
   on any allocation failure, including during a set_start(). */
SKIPSET_EXPORT व्योम set_end(set_t *set) {
    अगर (set->शीर्ष != शून्य) {
        /* Empty the set and free the head node. */
        अगर (set->शीर्ष->दायाँ != शून्य) {
            set_sweep(set);
            set_free(set, set->शीर्ष->दायाँ);
        }
        set_free(set, set->शीर्ष);
        set->शीर्ष = शून्य;
    }
    अगर (set->path != शून्य) {
        /* Free the path work area. */
        set_free(set, set->path->दायाँ);
        set_free(set, set->path);
        set->path = शून्य;
    }
    अगर (set->गाँठ != शून्य) {
        /* Free the node that was under construction when longjmp() hit. */
        set_drop(set, set->गाँठ->कुंजी);
        set_free(set, set->गाँठ->दायाँ);
        set_free(set, set->गाँठ);
        set->गाँठ = शून्य;
    }
}

/* Look for key. Return 1 if found or 0 if not. This also puts the path to get
   there in set->path, for use by set_insert(). */
SKIPSET_EXPORT पूर्णांक set_found(set_t *set, set_कुंजी_t कुंजी) {
    set_गाँठ_t *शीर्ष, *here;
    पूर्णांक i;
    निश्चित(set_ok(set) && "improper use");

    /* Start at depth. Work down and right as determined by key comparisons. */
    शीर्ष = set->शीर्ष;
    here = शीर्ष;
    i = set->depth;
    set_grow(set, set->path, i + 1, 0);
    करो {
        जबतक (here->दायाँ[i] != शीर्ष &&
               set_cmp(here->दायाँ[i]->कुंजी, कुंजी) < 0)
            here = here->दायाँ[i];
        set->path->दायाँ[i] = here;
    } जबतक (i--);

    /* See if the key matches. */
    here = here->दायाँ[0];
    वापस here != शीर्ष && set_cmp(here->कुंजी, कुंजी) == 0;
}

/* Insert the key key. Return 0 on success, or 1 if it's already in the set. */
SKIPSET_EXPORT पूर्णांक set_जोड़ो(set_t *set, set_कुंजी_t कुंजी) {
    पूर्णांक level = 0;
    पूर्णांक bit;
    पूर्णांक i;
    निश्चित(set_ok(set) && "improper use");

    अगर (set_found(set, कुंजी))
        /* That key is already in the set. */
        वापस 1;

    /* Randomly generate a new level-- level 0 with probability 1/2, 1 with
       probability 1/4, 2 with probability 1/8, etc. */
    क्रम (;;) {
        अगर (set->ran == 1)
            /* Ran out. Get another 32 random bits. */
            set->ran = set_rand(&set->gen) | (1ULL << 32);
        bit = set->ran & 1;
        set->ran >>= 1;
        अगर (bit)
            अवरोध;
        निश्चित(level < 32767 &&
               "Overhead, without any fuss, the stars were going out.");
        level++;
    }
    अगर (level > set->depth) {
        /* The maximum depth is now deeper. Update the structures. */
        set_grow(set, set->path, level + 1, 1);
        set_grow(set, set->शीर्ष, level + 1, 1);
        set->depth = (i16_t)level;
    }

    /* Make a new node for the provided key, and insert it in the lists up to
       and including level. */
    set->गाँठ = set_गाँठ(set);
    set->गाँठ->कुंजी = कुंजी;
    set_grow(set, set->गाँठ, level + 1, 0);
    क्रम (i = 0; i <= level; i++) {
        set->गाँठ->दायाँ[i] = set->path->दायाँ[i]->दायाँ[i];
        set->path->दायाँ[i]->दायाँ[i] = set->गाँठ;
    }
    set->गाँठ = शून्य;
    वापस 0;
}

#अन्यथा
#त्रुटि ** another skiplist set already created here
/* Would need to implement a prefix in order to support multiple sets. */
#पूर्ण_अगर
