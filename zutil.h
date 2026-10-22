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
#இல்லையெனில்
#  define ZLIB_INTERNAL
#endif

#அந்தர்பாவ "zlib.h"

#எனில் defined(STDC) && !defined(Z_SOLO)
#  எனில் !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local நிலையான
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

வெளி மாறிலி எழுத்து deflate_copyright[];
வெளி மாறிலி எழுத்து inflate_copyright[];
வெளி மாறிலி எழுத்து inflate9_copyright[];

வகைவரை unsigned எழுத்து  uch;
வகைவரை uch FAR uchf;
வகைவரை unsigned short ush;
வகைவரை ush FAR ushf;
வகைவரை unsigned long  ulg;

#எனில் !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  எனில் (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

வெளி z_const எழுத்து * மாறிலி z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_செய்தி(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  திரும்பு (strm->செய்தி = ERR_செய்தி(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#எனில் MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#எனில் MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#இல்லையெனில்
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

#எனில் defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    எனில் defined(__TURBOC__) || defined(__BORLANDC__)
#      எனில் (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         வெறுமை _Cdecl farfree( வெறுமை *block );
         வெறுமை *_Cdecl farmalloc( unsigned long nbytes );
#      இல்லையெனில்
#        include <alloc.h>
#      endif
#    இல்லையெனில் /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#எனில் defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  எனில் __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  இல்லையெனில்
#    define OS_CODE 8
#  endif
#endif

#எனில் defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  எனில் defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#எனில் defined(MACOS)
#  define OS_CODE  7
#endif

#எனில் defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#எனில் defined(WIN32) && !defined(__CYGWIN__)
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

#எனில் defined(__BORLANDC__) && !defined(MSDOS)
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

#எனில் defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#எனில் defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#எனில் defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  இல்லையெனில்
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#இல்லையெனில்
   வெறுமை ZLIB_INTERNAL zmemcpy(வெறுமை FAR *, மாறிலி வெறுமை FAR *, z_size_t);
   முழு ZLIB_INTERNAL zmemcmp(மாறிலி வெறுமை FAR *, மாறிலி வெறுமை FAR *, z_size_t);
   வெறுமை ZLIB_INTERNAL zmemzero(வெறுமை FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   வெளி முழு ZLIB_INTERNAL z_verbose;
   வெளி வெறுமை ZLIB_INTERNAL z_error(எழுத்து *m);
#  define Assert(cond,செய்தி) {எனில்(!(cond)) z_error(செய்தி);}
#  define Trace(x) {எனில் (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {எனில் (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {எனில் (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {எனில் (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {எனில் (z_verbose>1 && (c)) fprintf x ;}
#இல்லையெனில்
#  define Assert(cond,செய்தி)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned பொருட்கள்_2,
                                unsigned size);
   வெறுமை ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, பொருட்கள்_2, size) \
           (*((strm)->zalloc))((strm)->opaque, (பொருட்கள்_2), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {எனில் (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#எனில் defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#அந்தர்பாவ <stdatomic.h>
வகைவரை கட்டமைப்பு {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local வெறுமை z_once(z_once_t *state, வெறுமை (*init)(வெறுமை)) {
    எனில் (!atomic_load(&state->done)) {
        எனில் (atomic_flag_test_and_set(&state->begun))
            போது (!atomic_load(&state->done))
                ;
        இல்லையெனில் {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#இல்லையெனில்   /* no atomics */

#warning zlib not thread-safe

வகைவரை கட்டமைப்பு z_once_s {
    நிலையற்ற முழு begun;
    நிலையற்ற முழு done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local முழு test_and_set(முழு நிலையற்ற *flag) {
    முழு was;

    was = *flag;
    *flag = 1;
    திரும்பு was;
}

/* Run the provided init() function once. This is not thread-safe. */
local வெறுமை z_once(z_once_t *state, வெறுமை (*init)(வெறுமை)) {
    எனில் (!state->done) {
        எனில் (test_and_set(&state->begun))
            போது (!state->done)
                ;
        இல்லையெனில் {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
