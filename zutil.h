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
#अन्यथा
#  define ZLIB_INTERNAL
#endif

#अन्तर्भाव "zlib.h"

#जर defined(STDC) && !defined(Z_SOLO)
#  जर !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifndef local
#  define local स्थिर
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

बाह्य स्थिरांक अक्षर deflate_copyright[];
बाह्य स्थिरांक अक्षर inflate_copyright[];
बाह्य स्थिरांक अक्षर inflate9_copyright[];

प्रकारव्याख्या अचिन्हित अक्षर  uch;
प्रकारव्याख्या uch FAR uchf;
प्रकारव्याख्या अचिन्हित लघु ush;
प्रकारव्याख्या ush FAR ushf;
प्रकारव्याख्या अचिन्हित दीर्घ  ulg;

#जर !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  जर (अच_दीर्घ_उच्च == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित दीर्घ
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित दीर्घ दीर्घ
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित दीर्घ दीर्घ
#  elif (अच_पूर्णांक_उच्च == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित
#  endif
#endif

बाह्य z_const अक्षर * स्थिरांक z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_संदेश(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#define ERR_RETURN(strm,err) \
  परत (strm->संदेश = ERR_संदेश(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#जर MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#endif
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#जर MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
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

#जर defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    जर defined(__TURBOC__) || defined(__BORLANDC__)
#      जर (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         शून्य _Cdecl farfree( शून्य *block );
         शून्य *_Cdecl farmalloc( अचिन्हित दीर्घ nbytes );
#      अन्यथा
#        include <alloc.h>
#      endif
#    अन्यथा /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  1
#endif

#जर defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     ख_खोलो((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#ifdef __370__
#  जर __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  अन्यथा
#    define OS_CODE 8
#  endif
#endif

#जर defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#endif

#ifdef OS2
#  define OS_CODE  6
#  जर defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#जर defined(MACOS)
#  define OS_CODE  7
#endif

#जर defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#endif

#जर defined(WIN32) && !defined(__CYGWIN__)
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

#जर defined(__BORLANDC__) && !defined(MSDOS)
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
#  define F_OPEN(name, mode) ख_खोलो((name), (mode))
#endif

         /* functions */

#जर defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#जर defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#जर defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  अन्यथा
#    define zmemcpy स_नकल
#    define zmemcmp स_भेद
#    define zmemzero(dest, len) स_रखो(dest, 0, len)
#  endif
#अन्यथा
   शून्य ZLIB_INTERNAL zmemcpy(शून्य FAR *, स्थिरांक शून्य FAR *, z_size_t);
   पूर्णांक ZLIB_INTERNAL zmemcmp(स्थिरांक शून्य FAR *, स्थिरांक शून्य FAR *, z_size_t);
   शून्य ZLIB_INTERNAL zmemzero(शून्य FAR *, z_size_t);
#endif

/* Diagnostic functions */
#ifdef ZLIB_DEBUG
#  include <stdio.h>
   बाह्य पूर्णांक ZLIB_INTERNAL z_verbose;
   बाह्य शून्य ZLIB_INTERNAL z_error(अक्षर *m);
#  define Assert(cond,संदेश) {जर(!(cond)) z_error(संदेश);}
#  define Trace(x) {जर (z_verbose>=0) ख_लिखो x ;}
#  define Tracev(x) {जर (z_verbose>0) ख_लिखो x ;}
#  define Tracevv(x) {जर (z_verbose>1) ख_लिखो x ;}
#  define Tracec(c,x) {जर (z_verbose>0 && (c)) ख_लिखो x ;}
#  define Tracecv(c,x) {जर (z_verbose>1 && (c)) ख_लिखो x ;}
#अन्यथा
#  define Assert(cond,संदेश)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, अचिन्हित वस्तुसंच_2,
                                अचिन्हित size);
   शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#endif

#define ZALLOC(strm, वस्तुसंच_2, size) \
           (*((strm)->zalloc))((strm)->opaque, (वस्तुसंच_2), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {जर (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#जर defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#अन्तर्भाव <stdatomic.h>
प्रकारव्याख्या संरचना {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#define Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local शून्य z_once(z_once_t *state, शून्य (*init)(शून्य)) {
    जर (!atomic_load(&state->done)) {
        जर (atomic_flag_test_and_set(&state->begun))
            पर्यंत (!atomic_load(&state->done))
                ;
        अन्यथा {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#अन्यथा   /* no atomics */

#warning zlib not thread-safe

प्रकारव्याख्या संरचना z_once_s {
    अस्थिर पूर्णांक begun;
    अस्थिर पूर्णांक done;
} z_once_t;
#define Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local पूर्णांक test_and_set(पूर्णांक अस्थिर *flag) {
    पूर्णांक was;

    was = *flag;
    *flag = 1;
    परत was;
}

/* Run the provided init() function once. This is not thread-safe. */
local शून्य z_once(z_once_t *state, शून्य (*init)(शून्य)) {
    जर (!state->done) {
        जर (test_and_set(&state->begun))
            पर्यंत (!state->done)
                ;
        अन्यथा {
            init();
            state->done = 1;
        }
    }
}

#endif /* ?atomics */

#endif /* Z_ONCE */

#endif /* ZUTIL_H */
