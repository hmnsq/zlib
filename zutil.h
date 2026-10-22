/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZUTIL_H
#ವ್ಯಾಖ್ಯಾನಿಸು ZUTIL_H

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_HIDDEN
#  define ZLIB_ಆಂತರಿಕ __attribute__((visibility ("hidden")))
#ಇಲ್ಲದಿದ್ದರೆ
#  define ZLIB_ಆಂತರಿಕ
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "zlib.h"

#ಆದರೆ defined(STDC) && !defined(Z_SOLO)
#  ಆದರೆ !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ local
#  define local ಸ್ಥಿರ
#ಆದರೆ_ಅಂತ್ಯ
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ deflate_copyright[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ inflate_copyright[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ inflate9_copyright[];

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ  uch;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uch FAR uchf;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ush;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ush FAR ushf;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ  ulg;

#ಆದರೆ !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ಆದರೆ (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 ಚಿಹ್ನೆರಹಿತ
#  endif
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ z_const ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#ವ್ಯಾಖ್ಯಾನಿಸು ERR_ಸಂದೇಶ(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#ವ್ಯಾಖ್ಯಾನಿಸು ERR_RETURN(strm,err) \
  ಹಿಂತಿರುಗಿ (strm->ಸಂದೇಶ = ERR_ಸಂದೇಶ(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#ಆದರೆ MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be ಒಳ 9..15
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#ಆದರೆ_ಅಂತ್ಯ
/* default windowBits for decompression. MAX_WBITS is for compression only */

#ಆದರೆ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ಇಲ್ಲದಿದ್ದರೆ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#ಆದರೆ_ಅಂತ್ಯ
/* default memLevel */

#ವ್ಯಾಖ್ಯಾನಿಸು STORED_ಖಂಡ 0
#ವ್ಯಾಖ್ಯಾನಿಸು STATIC_TREES 1
#ವ್ಯಾಖ್ಯಾನಿಸು DYN_TREES    2
/* The three kinds of block type */

#ವ್ಯಾಖ್ಯಾನಿಸು MIN_ಹೊಂದಿಕೆ  3
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ಹೊಂದಿಕೆ  258
/* The minimum and maximum match lengths */

#ವ್ಯಾಖ್ಯಾನಿಸು PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#ಆದರೆ defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_ಸಂಕೇತ  0x00
#  ifndef Z_SOLO
#    ಆದರೆ defined(__TURBOC__) || defined(__BORLANDC__)
#      ಆದರೆ (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         ಶೂನ್ಯ _Cdecl farfree( ಶೂನ್ಯ *ಖಂಡ );
         ಶೂನ್ಯ *_Cdecl farmalloc( ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ nbytes );
#      ಇಲ್ಲದಿದ್ದರೆ
#        include <alloc.h>
#      endif
#    ಇಲ್ಲದಿದ್ದರೆ /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ AMIGA
#  define OS_ಸಂಕೇತ  1
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(VAXC) || defined(VMS)
#  define OS_ಸಂಕೇತ  2
#  define F_OPEN(name, ಬಹುಲಕ) \
     fopen((name), (ಬಹುಲಕ), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __370__
#  ಆದರೆ __TARGET_LIB__ < 0x20000000
#    define OS_ಸಂಕೇತ 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_ಸಂಕೇತ 11
#  ಇಲ್ಲದಿದ್ದರೆ
#    define OS_ಸಂಕೇತ 8
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(ATARI) || defined(atarist)
#  define OS_ಸಂಕೇತ  5
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ OS2
#  define OS_ಸಂಕೇತ  6
#  ಆದರೆ defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(MACOS)
#  define OS_ಸಂಕೇತ  7
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__acorn) || defined(__riscos)
#  define OS_ಸಂಕೇತ 13
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(WIN32) && !defined(__CYGWIN__)
#  define OS_ಸಂಕೇತ  10
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _BEOS_
#  define OS_ಸಂಕೇತ  16
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __TOS_OS400__
#  define OS_ಸಂಕೇತ 18
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
#  define OS_ಸಂಕೇತ 19
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__BORLANDC__) && !defined(MSDOS)
  #ಸೂಚನೆ warn -8004
  #ಸೂಚನೆ warn -8008
  #ಸೂಚನೆ warn -8066
#ಆದರೆ_ಅಂತ್ಯ

/* provide prototypes for these when building zlib without LFS */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen64(z_off64_t);
#ಆದರೆ_ಅಂತ್ಯ

        /* common defaults */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ OS_ಸಂಕೇತ
#  define OS_ಸಂಕೇತ  3     /* assume Unix */
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ F_OPEN
#  define F_OPEN(name, ಬಹುಲಕ) fopen((name), (ಬಹುಲಕ))
#ಆದರೆ_ಅಂತ್ಯ

         /* functions */

#ಆದರೆ defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  ಇಲ್ಲದಿದ್ದರೆ
#    define zmemcpy ಸ್ಮರಣೆ_ನಕಲು
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
   ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zmemcpy(ಶೂನ್ಯ FAR *, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ FAR *, z_ಗಾತ್ರ_t);
   ಪೂರ್ಣಾಂಕ ZLIB_ಆಂತರಿಕ zmemcmp(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ FAR *, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ FAR *, z_ಗಾತ್ರ_t);
   ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zmemzero(ಶೂನ್ಯ FAR *, z_ಗಾತ್ರ_t);
#ಆದರೆ_ಅಂತ್ಯ

/* Diagnostic functions */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
#  include <stdio.h>
   ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZLIB_ಆಂತರಿಕ z_verbose;
   ಬಾಹ್ಯ ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ z_error(ಅಕ್ಷರ *m);
#  define Assert(cond,ಸಂದೇಶ) {ಆದರೆ(!(cond)) z_error(ಸಂದೇಶ);}
#  define Trace(x) {ಆದರೆ (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {ಆದರೆ (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {ಆದರೆ (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {ಆದರೆ (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {ಆದರೆ (z_verbose>1 && (c)) fprintf x ;}
#ಇಲ್ಲದಿದ್ದರೆ
#  define Assert(cond,ಸಂದೇಶ)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_SOLO
   voidpf ZLIB_ಆಂತರಿಕ zcalloc(voidpf opaque, ಚಿಹ್ನೆರಹಿತ ವಸ್ತುಗಳು_2,
                                ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2);
   ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zcfree(voidpf opaque, voidpf ptr);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು ZALLOC(strm, ವಸ್ತುಗಳು_2, ಗಾತ್ರ_2) \
           (*((strm)->zalloc))((strm)->opaque, (ವಸ್ತುಗಳು_2), (ಗಾತ್ರ_2))
#ವ್ಯಾಖ್ಯಾನಿಸು ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#ವ್ಯಾಖ್ಯಾನಿಸು TRY_FREE(s, p) {ಆದರೆ (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#ವ್ಯಾಖ್ಯಾನಿಸು ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#ಆದರೆ defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ಅನ್ತರ್ಭಾವ <stdatomic.h>
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    atomic_flag begun;
    atomic_int ಮುಗಿದ_2;
} z_once_t;
#ವ್ಯಾಖ್ಯಾನಿಸು Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local ಶೂನ್ಯ z_once(z_once_t *state, ಶೂನ್ಯ (*init)(ಶೂನ್ಯ)) {
    ಆದರೆ (!atomic_ತುಂಬು(&state->ಮುಗಿದ_2)) {
        ಆದರೆ (atomic_flag_test_and_set(&state->begun))
            ಆಗ (!atomic_ತುಂಬು(&state->ಮುಗಿದ_2))
                ;
        ಇಲ್ಲದಿದ್ದರೆ {
            init();
            atomic_store(&state->ಮುಗಿದ_2, 1);
        }
    }
}

#ಇಲ್ಲದಿದ್ದರೆ   /* no atomics */

#warning zlib not thread-ಸುರಕ್ಷಿತ

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ z_once_s {
    ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ begun;
    ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_2;
} z_once_t;
#ವ್ಯಾಖ್ಯಾನಿಸು Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local ಪೂರ್ಣಾಂಕ test_and_ಗಣ(ಪೂರ್ಣಾಂಕ ಅಸ್ಥಿರ *flag) {
    ಪೂರ್ಣಾಂಕ was;

    was = *flag;
    *flag = 1;
    ಹಿಂತಿರುಗಿ was;
}

/* Run the provided init() function once. This is not thread-safe. */
local ಶೂನ್ಯ z_once(z_once_t *state, ಶೂನ್ಯ (*init)(ಶೂನ್ಯ)) {
    ಆದರೆ (!state->ಮುಗಿದ_2) {
        ಆದರೆ (test_and_ಗಣ(&state->begun))
            ಆಗ (!state->ಮುಗಿದ_2)
                ;
        ಇಲ್ಲದಿದ್ದರೆ {
            init();
            state->ಮುಗಿದ_2 = 1;
        }
    }
}

#ಆದರೆ_ಅಂತ್ಯ /* ?atomics */

#ಆದರೆ_ಅಂತ್ಯ /* Z_ONCE */

#ಆದರೆ_ಅಂತ್ಯ /* ZUTIL_H */
