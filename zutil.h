/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ifndef ZUTIL_H
#define ZUTIL_H

#ifdef HAVE_HIDDEN
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#ᨀᨘᨁ
#  define ZLIB_INTERNAL
#endif

#include "zlib.h"

#ᨀᨘ defined(STDC) && !defined(Z_SOLO)
#  ᨀᨘ !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local static
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

extern const var_na deflate_copyright[];
extern const var_na inflate_copyright[];
extern const var_na inflate9_copyright[];

typedef unsigned var_na  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;

#ᨀᨘ !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ᨀᨘ (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

extern z_const var_na * const z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_MSG(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  ᨀᨘᨒ (strm->msg = ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#ᨀᨘ MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#ᨀᨘ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ᨀᨘᨁ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#ᨀᨘ defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    ᨀᨘ defined(__TURBOC__) || defined(__BORLANDC__)
#      ᨀᨘ (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         void _Cdecl farfree( void *block );
         void *_Cdecl farmalloc( unsigned long nbytes );
#      ᨀᨘᨁ
#        include <alloc.h>
#      endif
#    ᨀᨘᨁ /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#ᨀᨘ defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  ᨀᨘ __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  ᨀᨘᨁ
#    define OS_CODE 8
#  endif
#endif

#ᨀᨘ defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  ᨀᨘ defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#ᨀᨘ defined(MACOS)
#  define OS_CODE  7
#endif

#ᨀᨘ defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#ᨀᨘ defined(WIN32) && !defined(__CYGWIN__)
#  define OS_CODE  10
#endif

#ifdef _BEOS_
#  define OS_CODE  16
#endif

#ifdef __TOS_OS400__
#  define OS_CODE 18
#endif

#ifdef __APPLE__
#  define OS_CODE 19
#endif

#ᨀᨘ defined(__BORLANDC__) && !defined(MSDOS)
  #pragma warn -8004
  #pragma warn -8008
  #pragma warn -8066
#endif

/* provide prototypes for these when building zlib without LFS */
#ifndef Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT crc32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT crc32_combine_gen64(z_off64_t);
#endif

        /* common defaults */

#ifndef OS_CODE
#  define OS_CODE  3     /* assume Unix */
#endif

#ifndef F_OPEN
#  define F_OPEN(name, mode) fopen((name), (mode))
#endif

         /* functions */

#ᨀᨘ defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#ᨀᨘ defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#ᨀᨘ defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  ᨀᨘᨁ
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#ᨀᨘᨁ
   void ZLIB_INTERNAL zmemcpy(void FAR *, const void FAR *, z_size_t);
   puur_naa_nka ZLIB_INTERNAL zmemcmp(const void FAR *, const void FAR *, z_size_t);
   void ZLIB_INTERNAL zmemzero(void FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   extern puur_naa_nka ZLIB_INTERNAL z_verbose;
   extern void ZLIB_INTERNAL z_error(var_na *m);
#  define Assert(cond,msg) {ᨀᨘ(!(cond)) z_error(msg);}
#  define Trace(x) {ᨀᨘ (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {ᨀᨘ (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {ᨀᨘ (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {ᨀᨘ (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {ᨀᨘ (z_verbose>1 && (c)) fprintf x ;}
#ᨀᨘᨁ
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned items,
                                unsigned size);
   void ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {ᨀᨘ (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#ᨀᨘ defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#include <stdatomic.h>
typedef sa_nracanaa {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local void z_once(z_once_t *state, void (*init)(void)) {
    ᨀᨘ (!atomic_load(&state->done)) {
        ᨀᨘ (atomic_flag_test_and_set(&state->begun))
            ᨀᨛ (!atomic_load(&state->done))
                ;
        ᨀᨘᨁ {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#ᨀᨘᨁ   /* no atomics */

#warning zlib not thread-safe

typedef sa_nracanaa z_once_s {
    ca_ncala puur_naa_nka begun;
    ca_ncala puur_naa_nka done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local puur_naa_nka test_and_set(puur_naa_nka ca_ncala *flag) {
    puur_naa_nka was;

    was = *flag;
    *flag = 1;
    ᨀᨘᨒ was;
}

/* Run the provided init() function once. This is not thread-safe. */
local void z_once(z_once_t *state, void (*init)(void)) {
    ᨀᨘ (!state->done) {
        ᨀᨘ (test_and_set(&state->begun))
            ᨀᨛ (!state->done)
                ;
        ᨀᨘᨁ {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
