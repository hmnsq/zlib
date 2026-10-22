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
#નહીંતર
#  define ZLIB_INTERNAL
#endif

#અન્તર્ભાવ "zlib.h"

#જો defined(STDC) && !defined(Z_SOLO)
#  જો !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local સ્થિર
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

બાહ્ય અચલ અક્ષર deflate_copyright[];
બાહ્ય અચલ અક્ષર inflate_copyright[];
બાહ્ય અચલ અક્ષર inflate9_copyright[];

પ્રકારવ્યાખ્યા unsigned અક્ષર  uch;
પ્રકારવ્યાખ્યા uch FAR uchf;
પ્રકારવ્યાખ્યા unsigned short ush;
પ્રકારવ્યાખ્યા ush FAR ushf;
પ્રકારવ્યાખ્યા unsigned long  ulg;

#જો !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  જો (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned long long
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 unsigned
#  endif
#endif

બાહ્ય z_const અક્ષર * અચલ z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_સંદેશ(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  પાછા (strm->સંદેશ = ERR_સંદેશ(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#જો MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#જો MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#નહીંતર
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

#જો defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    જો defined(__TURBOC__) || defined(__BORLANDC__)
#      જો (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         શૂન્ય _Cdecl farfree( શૂન્ય *block );
         શૂન્ય *_Cdecl farmalloc( unsigned long nbytes );
#      નહીંતર
#        include <alloc.h>
#      endif
#    નહીંતર /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#જો defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  જો __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  નહીંતર
#    define OS_CODE 8
#  endif
#endif

#જો defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  જો defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#જો defined(MACOS)
#  define OS_CODE  7
#endif

#જો defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#જો defined(WIN32) && !defined(__CYGWIN__)
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

#જો defined(__BORLANDC__) && !defined(MSDOS)
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

#જો defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#જો defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#જો defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  નહીંતર
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#નહીંતર
   શૂન્ય ZLIB_INTERNAL zmemcpy(શૂન્ય FAR *, અચલ શૂન્ય FAR *, z_size_t);
   પૂર્ણાંક ZLIB_INTERNAL zmemcmp(અચલ શૂન્ય FAR *, અચલ શૂન્ય FAR *, z_size_t);
   શૂન્ય ZLIB_INTERNAL zmemzero(શૂન્ય FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   બાહ્ય પૂર્ણાંક ZLIB_INTERNAL z_verbose;
   બાહ્ય શૂન્ય ZLIB_INTERNAL z_error(અક્ષર *m);
#  define Assert(cond,સંદેશ) {જો(!(cond)) z_error(સંદેશ);}
#  define Trace(x) {જો (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {જો (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {જો (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {જો (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {જો (z_verbose>1 && (c)) fprintf x ;}
#નહીંતર
#  define Assert(cond,સંદેશ)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned વસ્તુઓ_2,
                                unsigned size);
   શૂન્ય ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, વસ્તુઓ_2, size) \
           (*((strm)->zalloc))((strm)->opaque, (વસ્તુઓ_2), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {જો (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#જો defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#અન્તર્ભાવ <stdatomic.h>
પ્રકારવ્યાખ્યા સંરચના {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local શૂન્ય z_once(z_once_t *state, શૂન્ય (*init)(શૂન્ય)) {
    જો (!atomic_load(&state->done)) {
        જો (atomic_flag_test_and_set(&state->begun))
            જ્યારે (!atomic_load(&state->done))
                ;
        નહીંતર {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#નહીંતર   /* no atomics */

#warning zlib not thread-safe

પ્રકારવ્યાખ્યા સંરચના z_once_s {
    અસ્થિર પૂર્ણાંક begun;
    અસ્થિર પૂર્ણાંક done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local પૂર્ણાંક test_and_set(પૂર્ણાંક અસ્થિર *flag) {
    પૂર્ણાંક was;

    was = *flag;
    *flag = 1;
    પાછા was;
}

/* Run the provided init() function once. This is not thread-safe. */
local શૂન્ય z_once(z_once_t *state, શૂન્ય (*init)(શૂન્ય)) {
    જો (!state->done) {
        જો (test_and_set(&state->begun))
            જ્યારે (!state->done)
                ;
        નહીંતર {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
