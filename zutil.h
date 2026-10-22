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
#অন্যথা
#  define ZLIB_INTERNAL
#endif

#অন্তর্ভাৱ "zlib.h"

#যদি defined(STDC) && !defined(Z_SOLO)
#  যদি !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local স্থির
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

বাহ্য ধ্রুৱ ৱর্ণ deflate_copyright[];
বাহ্য ধ্রুৱ ৱর্ণ inflate_copyright[];
বাহ্য ধ্রুৱ ৱর্ণ inflate9_copyright[];

প্রকারনির্দেশ unsigned ৱর্ণ  uch;
প্রকারনির্দেশ uch FAR uchf;
প্রকারনির্দেশ unsigned short ush;
প্রকারনির্দেশ ush FAR ushf;
প্রকারনির্দেশ unsigned long  ulg;

#যদি !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  যদি (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

বাহ্য z_const ৱর্ণ * ধ্রুৱ z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_বাৰ্তা(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  ঘূৰাই (strm->বাৰ্তা = ERR_বাৰ্তা(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#যদি MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#যদি MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#অন্যথা
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

#যদি defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    যদি defined(__TURBOC__) || defined(__BORLANDC__)
#      যদি (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         শূন্য _Cdecl farfree( শূন্য *block );
         শূন্য *_Cdecl farmalloc( unsigned long nbytes );
#      অন্যথা
#        include <alloc.h>
#      endif
#    অন্যথা /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#যদি defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  যদি __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  অন্যথা
#    define OS_CODE 8
#  endif
#endif

#যদি defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  যদি defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#যদি defined(MACOS)
#  define OS_CODE  7
#endif

#যদি defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#যদি defined(WIN32) && !defined(__CYGWIN__)
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

#যদি defined(__BORLANDC__) && !defined(MSDOS)
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

#যদি defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#যদি defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#যদি defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  অন্যথা
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#অন্যথা
   শূন্য ZLIB_INTERNAL zmemcpy(শূন্য FAR *, ধ্রুৱ শূন্য FAR *, z_size_t);
   পূর্ণাঙ্ক ZLIB_INTERNAL zmemcmp(ধ্রুৱ শূন্য FAR *, ধ্রুৱ শূন্য FAR *, z_size_t);
   শূন্য ZLIB_INTERNAL zmemzero(শূন্য FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   বাহ্য পূর্ণাঙ্ক ZLIB_INTERNAL z_verbose;
   বাহ্য শূন্য ZLIB_INTERNAL z_error(ৱর্ণ *m);
#  define Assert(cond,বাৰ্তা) {যদি(!(cond)) z_error(বাৰ্তা);}
#  define Trace(x) {যদি (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {যদি (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {যদি (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {যদি (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {যদি (z_verbose>1 && (c)) fprintf x ;}
#অন্যথা
#  define Assert(cond,বাৰ্তা)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned বস্তুসমূহ_2,
                                unsigned size);
   শূন্য ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, বস্তুসমূহ_2, size) \
           (*((strm)->zalloc))((strm)->opaque, (বস্তুসমূহ_2), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {যদি (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#যদি defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#অন্তর্ভাৱ <stdatomic.h>
প্রকারনির্দেশ সংরচনা {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local শূন্য z_once(z_once_t *state, শূন্য (*init)(শূন্য)) {
    যদি (!atomic_load(&state->done)) {
        যদি (atomic_flag_test_and_set(&state->begun))
            যেতিয়ালৈ (!atomic_load(&state->done))
                ;
        অন্যথা {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#অন্যথা   /* no atomics */

#warning zlib not thread-safe

প্রকারনির্দেশ সংরচনা z_once_s {
    চঞ্চল পূর্ণাঙ্ক begun;
    চঞ্চল পূর্ণাঙ্ক done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local পূর্ণাঙ্ক test_and_set(পূর্ণাঙ্ক চঞ্চল *flag) {
    পূর্ণাঙ্ক was;

    was = *flag;
    *flag = 1;
    ঘূৰাই was;
}

/* Run the provided init() function once. This is not thread-safe. */
local শূন্য z_once(z_once_t *state, শূন্য (*init)(শূন্য)) {
    যদি (!state->done) {
        যদি (test_and_set(&state->begun))
            যেতিয়ালৈ (!state->done)
                ;
        অন্যথা {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
