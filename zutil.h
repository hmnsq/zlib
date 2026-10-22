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
#അല്ലെങ്കിൽ
#  define ZLIB_INTERNAL
#endif

#അന്തര്ഭാവ "zlib.h"

#എങ്കിൽ defined(STDC) && !defined(Z_SOLO)
#  എങ്കിൽ !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local സ്ഥിതം
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

ബാഹ്യ സ്ഥിരം അക്ഷരം deflate_copyright[];
ബാഹ്യ സ്ഥിരം അക്ഷരം inflate_copyright[];
ബാഹ്യ സ്ഥിരം അക്ഷരം inflate9_copyright[];

തരനിർവചനം unsigned അക്ഷരം  uch;
തരനിർവചനം uch FAR uchf;
തരനിർവചനം unsigned short ush;
തരനിർവചനം ush FAR ushf;
തരനിർവചനം unsigned long  ulg;

#എങ്കിൽ !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  എങ്കിൽ (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

ബാഹ്യ z_const അക്ഷരം * സ്ഥിരം z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_സന്ദേശം(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  തിരികെ (strm->സന്ദേശം = ERR_സന്ദേശം(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#എങ്കിൽ MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#എങ്കിൽ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#അല്ലെങ്കിൽ
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

#എങ്കിൽ defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    എങ്കിൽ defined(__TURBOC__) || defined(__BORLANDC__)
#      എങ്കിൽ (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         ശൂന്യം _Cdecl farfree( ശൂന്യം *block );
         ശൂന്യം *_Cdecl farmalloc( unsigned long nbytes );
#      അല്ലെങ്കിൽ
#        include <alloc.h>
#      endif
#    അല്ലെങ്കിൽ /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#എങ്കിൽ defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  എങ്കിൽ __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  അല്ലെങ്കിൽ
#    define OS_CODE 8
#  endif
#endif

#എങ്കിൽ defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  എങ്കിൽ defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#എങ്കിൽ defined(MACOS)
#  define OS_CODE  7
#endif

#എങ്കിൽ defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#എങ്കിൽ defined(WIN32) && !defined(__CYGWIN__)
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

#എങ്കിൽ defined(__BORLANDC__) && !defined(MSDOS)
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

#എങ്കിൽ defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#എങ്കിൽ defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#എങ്കിൽ defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  അല്ലെങ്കിൽ
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#അല്ലെങ്കിൽ
   ശൂന്യം ZLIB_INTERNAL zmemcpy(ശൂന്യം FAR *, സ്ഥിരം ശൂന്യം FAR *, z_size_t);
   പൂർണ്ണസംഖ്യ ZLIB_INTERNAL zmemcmp(സ്ഥിരം ശൂന്യം FAR *, സ്ഥിരം ശൂന്യം FAR *, z_size_t);
   ശൂന്യം ZLIB_INTERNAL zmemzero(ശൂന്യം FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   ബാഹ്യ പൂർണ്ണസംഖ്യ ZLIB_INTERNAL z_verbose;
   ബാഹ്യ ശൂന്യം ZLIB_INTERNAL z_error(അക്ഷരം *m);
#  define Assert(cond,സന്ദേശം) {എങ്കിൽ(!(cond)) z_error(സന്ദേശം);}
#  define Trace(x) {എങ്കിൽ (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {എങ്കിൽ (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {എങ്കിൽ (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {എങ്കിൽ (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {എങ്കിൽ (z_verbose>1 && (c)) fprintf x ;}
#അല്ലെങ്കിൽ
#  define Assert(cond,സന്ദേശം)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ഇനങ്ങൾ_2,
                                unsigned size);
   ശൂന്യം ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, ഇനങ്ങൾ_2, size) \
           (*((strm)->zalloc))((strm)->opaque, (ഇനങ്ങൾ_2), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {എങ്കിൽ (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#എങ്കിൽ defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#അന്തര്ഭാവ <stdatomic.h>
തരനിർവചനം ഘടന {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local ശൂന്യം z_once(z_once_t *state, ശൂന്യം (*init)(ശൂന്യം)) {
    എങ്കിൽ (!atomic_load(&state->done)) {
        എങ്കിൽ (atomic_flag_test_and_set(&state->begun))
            ആകുമ്പോൾ (!atomic_load(&state->done))
                ;
        അല്ലെങ്കിൽ {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#അല്ലെങ്കിൽ   /* no atomics */

#warning zlib not thread-safe

തരനിർവചനം ഘടന z_once_s {
    അസ്ഥിര പൂർണ്ണസംഖ്യ begun;
    അസ്ഥിര പൂർണ്ണസംഖ്യ done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local പൂർണ്ണസംഖ്യ test_and_set(പൂർണ്ണസംഖ്യ അസ്ഥിര *flag) {
    പൂർണ്ണസംഖ്യ was;

    was = *flag;
    *flag = 1;
    തിരികെ was;
}

/* Run the provided init() function once. This is not thread-safe. */
local ശൂന്യം z_once(z_once_t *state, ശൂന്യം (*init)(ശൂന്യം)) {
    എങ്കിൽ (!state->done) {
        എങ്കിൽ (test_and_set(&state->begun))
            ആകുമ്പോൾ (!state->done)
                ;
        അല്ലെങ്കിൽ {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
