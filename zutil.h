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
#ಅನ್ಯಥಾ
#  define ZLIB_INTERNAL
#endif

#ಅನ್ತರ್ಭಾವ "zlib.h"

#ಯದಿ defined(STDC) && !defined(Z_SOLO)
#  ಯದಿ !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local ಸ್ಥಿರ
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ deflate_copyright[];
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ inflate_copyright[];
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ inflate9_copyright[];

ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ವರ್ಣ  uch;
ಪ್ರಕಾರನಿರ್ದೇಶ uch FAR uchf;
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned short ush;
ಪ್ರಕಾರನಿರ್ದೇಶ ush FAR ushf;
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long  ulg;

#ಯದಿ !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ಯದಿ (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

ಬಾಹ್ಯ z_const ವರ್ಣ * ಧ್ರುವ z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_ಸಂದೇಶ(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  ಪ್ರತ್ಯಾಗಮ (strm->ಸಂದೇಶ = ERR_ಸಂದೇಶ(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#ಯದಿ MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#ಯದಿ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ಅನ್ಯಥಾ
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

#ಯದಿ defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    ಯದಿ defined(__TURBOC__) || defined(__BORLANDC__)
#      ಯದಿ (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         ಶೂನ್ಯ _Cdecl farfree( ಶೂನ್ಯ *block );
         ಶೂನ್ಯ *_Cdecl farmalloc( unsigned long nbytes );
#      ಅನ್ಯಥಾ
#        include <alloc.h>
#      endif
#    ಅನ್ಯಥಾ /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#ಯದಿ defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  ಯದಿ __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  ಅನ್ಯಥಾ
#    define OS_CODE 8
#  endif
#endif

#ಯದಿ defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  ಯದಿ defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#ಯದಿ defined(MACOS)
#  define OS_CODE  7
#endif

#ಯದಿ defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#ಯದಿ defined(WIN32) && !defined(__CYGWIN__)
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

#ಯದಿ defined(__BORLANDC__) && !defined(MSDOS)
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

#ಯದಿ defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#ಯದಿ defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#ಯದಿ defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  ಅನ್ಯಥಾ
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#ಅನ್ಯಥಾ
   ಶೂನ್ಯ ZLIB_INTERNAL zmemcpy(ಶೂನ್ಯ FAR *, ಧ್ರುವ ಶೂನ್ಯ FAR *, z_size_t);
   ಪೂರ್ಣಾಙ್ಕ ZLIB_INTERNAL zmemcmp(ಧ್ರುವ ಶೂನ್ಯ FAR *, ಧ್ರುವ ಶೂನ್ಯ FAR *, z_size_t);
   ಶೂನ್ಯ ZLIB_INTERNAL zmemzero(ಶೂನ್ಯ FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ZLIB_INTERNAL z_verbose;
   ಬಾಹ್ಯ ಶೂನ್ಯ ZLIB_INTERNAL z_error(ವರ್ಣ *m);
#  define Assert(cond,ಸಂದೇಶ) {ಯದಿ(!(cond)) z_error(ಸಂದೇಶ);}
#  define Trace(x) {ಯದಿ (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {ಯದಿ (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {ಯದಿ (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {ಯದಿ (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {ಯದಿ (z_verbose>1 && (c)) fprintf x ;}
#ಅನ್ಯಥಾ
#  define Assert(cond,ಸಂದೇಶ)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ವಸ್ತುಗಳು_2,
                                unsigned size);
   ಶೂನ್ಯ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, ವಸ್ತುಗಳು_2, size) \
           (*((strm)->zalloc))((strm)->opaque, (ವಸ್ತುಗಳು_2), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {ಯದಿ (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#ಯದಿ defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ಅನ್ತರ್ಭಾವ <stdatomic.h>
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local ಶೂನ್ಯ z_once(z_once_t *state, ಶೂನ್ಯ (*init)(ಶೂನ್ಯ)) {
    ಯದಿ (!atomic_load(&state->done)) {
        ಯದಿ (atomic_flag_test_and_set(&state->begun))
            ಯಾವತ್ (!atomic_load(&state->done))
                ;
        ಅನ್ಯಥಾ {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#ಅನ್ಯಥಾ   /* no atomics */

#warning zlib not thread-safe

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ z_once_s {
    ಚಞ್ಚಲ ಪೂರ್ಣಾಙ್ಕ begun;
    ಚಞ್ಚಲ ಪೂರ್ಣಾಙ್ಕ done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local ಪೂರ್ಣಾಙ್ಕ test_and_set(ಪೂರ್ಣಾಙ್ಕ ಚಞ್ಚಲ *flag) {
    ಪೂರ್ಣಾಙ್ಕ was;

    was = *flag;
    *flag = 1;
    ಪ್ರತ್ಯಾಗಮ was;
}

/* Run the provided init() function once. This is not thread-safe. */
local ಶೂನ್ಯ z_once(z_once_t *state, ಶೂನ್ಯ (*init)(ಶೂನ್ಯ)) {
    ಯದಿ (!state->done) {
        ಯದಿ (test_and_set(&state->begun))
            ಯಾವತ್ (!state->done)
                ;
        ಅನ್ಯಥಾ {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
