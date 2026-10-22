/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZUTIL_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZUTIL_H

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_HIDDEN
#  define ZLIB_ਅੰਦਰੂਨੀ __attribute__((visibility ("hidden")))
#ਨਹੀਂਤਾਂ
#  define ZLIB_ਅੰਦਰੂਨੀ
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

#ਜੇ defined(STDC) && !defined(Z_SOLO)
#  ਜੇ !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ local
#  define local ਸਥਿਰ
#ਜੇ_ਅੰਤ
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ deflate_copyright[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ inflate_copyright[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ inflate9_copyright[];

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ  uch;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uch FAR uchf;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ush;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ush FAR ushf;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ  ulg;

#ਜੇ !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ਜੇ (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 ਬਿਨਾਨਿਸ਼ਾਨੀ
#  endif
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ z_const ਅੱਖਰ * ਅਚਲ z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_ਸੁਨੇਹਾ(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_RETURN(strm,err) \
  ਵਾਪਸ (strm->ਸੁਨੇਹਾ = ERR_ਸੁਨੇਹਾ(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#ਜੇ MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be ਵਿੱਚ 9..15
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#ਜੇ_ਅੰਤ
/* default windowBits for decompression. MAX_WBITS is for compression only */

#ਜੇ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ਨਹੀਂਤਾਂ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#ਜੇ_ਅੰਤ
/* default memLevel */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STORED_ਖੰਡ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STATIC_TREES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DYN_TREES    2
/* The three kinds of block type */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_ਮੇਲ  3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਮੇਲ  258
/* The minimum and maximum match lengths */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#ਜੇ defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_ਕੋਡ  0x00
#  ifndef Z_SOLO
#    ਜੇ defined(__TURBOC__) || defined(__BORLANDC__)
#      ਜੇ (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         ਖਾਲੀ _Cdecl farfree( ਖਾਲੀ *ਖੰਡ );
         ਖਾਲੀ *_Cdecl farmalloc( ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ nbytes );
#      ਨਹੀਂਤਾਂ
#        include <alloc.h>
#      endif
#    ਨਹੀਂਤਾਂ /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ AMIGA
#  define OS_ਕੋਡ  1
#ਜੇ_ਅੰਤ

#ਜੇ defined(VAXC) || defined(VMS)
#  define OS_ਕੋਡ  2
#  define F_OPEN(name, ਬਹੁਲਕ) \
     fopen((name), (ਬਹੁਲਕ), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __370__
#  ਜੇ __TARGET_LIB__ < 0x20000000
#    define OS_ਕੋਡ 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_ਕੋਡ 11
#  ਨਹੀਂਤਾਂ
#    define OS_ਕੋਡ 8
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined(ATARI) || defined(atarist)
#  define OS_ਕੋਡ  5
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ OS2
#  define OS_ਕੋਡ  6
#  ਜੇ defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined(MACOS)
#  define OS_ਕੋਡ  7
#ਜੇ_ਅੰਤ

#ਜੇ defined(__acorn) || defined(__riscos)
#  define OS_ਕੋਡ 13
#ਜੇ_ਅੰਤ

#ਜੇ defined(WIN32) && !defined(__CYGWIN__)
#  define OS_ਕੋਡ  10
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _BEOS_
#  define OS_ਕੋਡ  16
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __TOS_OS400__
#  define OS_ਕੋਡ 18
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
#  define OS_ਕੋਡ 19
#ਜੇ_ਅੰਤ

#ਜੇ defined(__BORLANDC__) && !defined(MSDOS)
  #ਹਦਾਇਤ warn -8004
  #ਹਦਾਇਤ warn -8008
  #ਹਦਾਇਤ warn -8066
#ਜੇ_ਅੰਤ

/* provide prototypes for these when building zlib without LFS */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen64(z_off64_t);
#ਜੇ_ਅੰਤ

        /* common defaults */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ OS_ਕੋਡ
#  define OS_ਕੋਡ  3     /* assume Unix */
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ F_OPEN
#  define F_OPEN(name, ਬਹੁਲਕ) fopen((name), (ਬਹੁਲਕ))
#ਜੇ_ਅੰਤ

         /* functions */

#ਜੇ defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#ਜੇ_ਅੰਤ
#ਜੇ defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#ਜੇ_ਅੰਤ
#ਜੇ defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  ਨਹੀਂਤਾਂ
#    define zmemcpy ਸਮ੍ਰਿਤੀ_ਨਕਲ
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#ਨਹੀਂਤਾਂ
   ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zmemcpy(ਖਾਲੀ FAR *, ਅਚਲ ਖਾਲੀ FAR *, z_ਮਾਪ_t);
   ਪੂਰਨ ZLIB_ਅੰਦਰੂਨੀ zmemcmp(ਅਚਲ ਖਾਲੀ FAR *, ਅਚਲ ਖਾਲੀ FAR *, z_ਮਾਪ_t);
   ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zmemzero(ਖਾਲੀ FAR *, z_ਮਾਪ_t);
#ਜੇ_ਅੰਤ

/* Diagnostic functions */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
#  include <stdio.h>
   ਬਾਹਰੀ ਪੂਰਨ ZLIB_ਅੰਦਰੂਨੀ z_verbose;
   ਬਾਹਰੀ ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ z_error(ਅੱਖਰ *m);
#  define Assert(cond,ਸੁਨੇਹਾ) {ਜੇ(!(cond)) z_error(ਸੁਨੇਹਾ);}
#  define Trace(x) {ਜੇ (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {ਜੇ (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {ਜੇ (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {ਜੇ (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {ਜੇ (z_verbose>1 && (c)) fprintf x ;}
#ਨਹੀਂਤਾਂ
#  define Assert(cond,ਸੁਨੇਹਾ)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_SOLO
   voidpf ZLIB_ਅੰਦਰੂਨੀ zcalloc(voidpf opaque, ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੀਜ਼ਾਂ_2,
                                ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ);
   ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zcfree(voidpf opaque, voidpf ptr);
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZALLOC(strm, ਚੀਜ਼ਾਂ_2, ਮਾਪ) \
           (*((strm)->zalloc))((strm)->opaque, (ਚੀਜ਼ਾਂ_2), (ਮਾਪ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRY_FREE(s, p) {ਜੇ (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#ਜੇ defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ਅਨ੍ਤਰ੍ਭਾਵ <stdatomic.h>
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    atomic_flag begun;
    atomic_int ਪੂਰਾ_2;
} z_once_t;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local ਖਾਲੀ z_once(z_once_t *state, ਖਾਲੀ (*init)(ਖਾਲੀ)) {
    ਜੇ (!atomic_ਲੱਦੋ(&state->ਪੂਰਾ_2)) {
        ਜੇ (atomic_flag_test_and_set(&state->begun))
            ਜਦੋਂ (!atomic_ਲੱਦੋ(&state->ਪੂਰਾ_2))
                ;
        ਨਹੀਂਤਾਂ {
            init();
            atomic_store(&state->ਪੂਰਾ_2, 1);
        }
    }
}

#ਨਹੀਂਤਾਂ   /* no atomics */

#warning zlib not thread-ਸੁਰੱਖਿਅਤ_2

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ z_once_s {
    ਅਸਥਿਰ ਪੂਰਨ begun;
    ਅਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_2;
} z_once_t;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local ਪੂਰਨ test_and_ਸੈੱਟ(ਪੂਰਨ ਅਸਥਿਰ *flag) {
    ਪੂਰਨ was;

    was = *flag;
    *flag = 1;
    ਵਾਪਸ was;
}

/* Run the provided init() function once. This is not thread-safe. */
local ਖਾਲੀ z_once(z_once_t *state, ਖਾਲੀ (*init)(ਖਾਲੀ)) {
    ਜੇ (!state->ਪੂਰਾ_2) {
        ਜੇ (test_and_ਸੈੱਟ(&state->begun))
            ਜਦੋਂ (!state->ਪੂਰਾ_2)
                ;
        ਨਹੀਂਤਾਂ {
            init();
            state->ਪੂਰਾ_2 = 1;
        }
    }
}

#ਜੇ_ਅੰਤ /* ?atomics */

#ਜੇ_ਅੰਤ /* Z_ONCE */

#ਜੇ_ਅੰਤ /* ZUTIL_H */
