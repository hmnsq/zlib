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
#ଅନ୍ୟଥା
#  define ZLIB_INTERNAL
#endif

#ଅନ୍ତର୍ଭାଵ "zlib.h"

#ଯଦି defined(STDC) && !defined(Z_SOLO)
#  ଯଦି !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local ସ୍ଥିର
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର deflate_copyright[];
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର inflate_copyright[];
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର inflate9_copyright[];

ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର  uch;
ପ୍ରକାରବ୍ୟାଖ୍ୟା uch FAR uchf;
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned short ush;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ush FAR ushf;
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned long  ulg;

#ଯଦି !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ଯଦି (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

ବାହ୍ୟ z_const ଅକ୍ଷର * ଅଚଳ z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_ସନ୍ଦେଶ(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  ଫେରନ୍ତୁ (strm->ସନ୍ଦେଶ = ERR_ସନ୍ଦେଶ(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#ଯଦି MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#ଯଦି MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ଅନ୍ୟଥା
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

#ଯଦି defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    ଯଦି defined(__TURBOC__) || defined(__BORLANDC__)
#      ଯଦି (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         ଶୂନ୍ୟ _Cdecl farfree( ଶୂନ୍ୟ *block );
         ଶୂନ୍ୟ *_Cdecl farmalloc( unsigned long nbytes );
#      ଅନ୍ୟଥା
#        include <alloc.h>
#      endif
#    ଅନ୍ୟଥା /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#ଯଦି defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  ଯଦି __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  ଅନ୍ୟଥା
#    define OS_CODE 8
#  endif
#endif

#ଯଦି defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  ଯଦି defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#ଯଦି defined(MACOS)
#  define OS_CODE  7
#endif

#ଯଦି defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#ଯଦି defined(WIN32) && !defined(__CYGWIN__)
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

#ଯଦି defined(__BORLANDC__) && !defined(MSDOS)
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

#ଯଦି defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#ଯଦି defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#ଯଦି defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  ଅନ୍ୟଥା
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#ଅନ୍ୟଥା
   ଶୂନ୍ୟ ZLIB_INTERNAL zmemcpy(ଶୂନ୍ୟ FAR *, ଅଚଳ ଶୂନ୍ୟ FAR *, z_size_t);
   ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZLIB_INTERNAL zmemcmp(ଅଚଳ ଶୂନ୍ୟ FAR *, ଅଚଳ ଶୂନ୍ୟ FAR *, z_size_t);
   ଶୂନ୍ୟ ZLIB_INTERNAL zmemzero(ଶୂନ୍ୟ FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZLIB_INTERNAL z_verbose;
   ବାହ୍ୟ ଶୂନ୍ୟ ZLIB_INTERNAL z_error(ଅକ୍ଷର *m);
#  define Assert(cond,ସନ୍ଦେଶ) {ଯଦି(!(cond)) z_error(ସନ୍ଦେଶ);}
#  define Trace(x) {ଯଦି (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {ଯଦି (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {ଯଦି (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {ଯଦି (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {ଯଦି (z_verbose>1 && (c)) fprintf x ;}
#ଅନ୍ୟଥା
#  define Assert(cond,ସନ୍ଦେଶ)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ବସ୍ତୁଗୁଡ଼ିକ_2,
                                unsigned size);
   ଶୂନ୍ୟ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, ବସ୍ତୁଗୁଡ଼ିକ_2, size) \
           (*((strm)->zalloc))((strm)->opaque, (ବସ୍ତୁଗୁଡ଼ିକ_2), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {ଯଦି (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#ଯଦି defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ଅନ୍ତର୍ଭାଵ <stdatomic.h>
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local ଶୂନ୍ୟ z_once(z_once_t *state, ଶୂନ୍ୟ (*init)(ଶୂନ୍ୟ)) {
    ଯଦି (!atomic_load(&state->done)) {
        ଯଦି (atomic_flag_test_and_set(&state->begun))
            ଯେତେବେଳେ (!atomic_load(&state->done))
                ;
        ଅନ୍ୟଥା {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#ଅନ୍ୟଥା   /* no atomics */

#warning zlib not thread-safe

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା z_once_s {
    ଅସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା begun;
    ଅସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_and_set(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅସ୍ଥିର *flag) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା was;

    was = *flag;
    *flag = 1;
    ଫେରନ୍ତୁ was;
}

/* Run the provided init() function once. This is not thread-safe. */
local ଶୂନ୍ୟ z_once(z_once_t *state, ଶୂନ୍ୟ (*init)(ଶୂନ୍ୟ)) {
    ଯଦି (!state->done) {
        ଯଦି (test_and_set(&state->begun))
            ଯେତେବେଳେ (!state->done)
                ;
        ଅନ୍ୟଥା {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
