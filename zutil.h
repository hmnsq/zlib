/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#अगर_अघोषित ZUTIL_H
#घोषणा ZUTIL_H

#अगर_घोषित HAVE_HIDDEN
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#अन्यथा
#  define ZLIB_INTERNAL
#पूर्ण_अगर

#समावेश "zlib.h"

#अगर defined(STDC) && !defined(Z_SOLO)
#  अगर !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#पूर्ण_अगर

#अगर_अघोषित local
#  define local अटल
#पूर्ण_अगर
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

बाह्य स्थिर अक्षर deflate_copyright[];
बाह्य स्थिर अक्षर inflate_copyright[];
बाह्य स्थिर अक्षर inflate9_copyright[];

प्रकार अचिन्हित अक्षर  uch;
प्रकार uch FAR uchf;
प्रकार अचिन्हित लघु ush;
प्रकार ush FAR ushf;
प्रकार अचिन्हित दीर्घ  ulg;

#अगर !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  अगर (अच_दीर्घ_उच्च == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित दीर्घ
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित दीर्घ दीर्घ
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित दीर्घ दीर्घ
#  elif (अच_पूर्णांक_उच्च == 0xffffffffffffffff)
#    define Z_U8 अचिन्हित
#  endif
#पूर्ण_अगर

बाह्य z_const अक्षर * स्थिर z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#घोषणा ERR_संदेश(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#घोषणा ERR_RETURN(strm,err) \
  वापस (strm->संदेश = ERR_संदेश(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#अगर MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be in 9..15
#पूर्ण_अगर
#अगर_अघोषित DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#पूर्ण_अगर
/* default windowBits for decompression. MAX_WBITS is for compression only */

#अगर MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#पूर्ण_अगर
/* default memLevel */

#घोषणा STORED_BLOCK 0
#घोषणा STATIC_TREES 1
#घोषणा DYN_TREES    2
/* The three kinds of block type */

#घोषणा MIN_MATCH  3
#घोषणा MAX_MATCH  258
/* The minimum and maximum match lengths */

#घोषणा PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#अगर defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    अगर defined(__TURBOC__) || defined(__BORLANDC__)
#      अगर (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         व्योम _Cdecl farfree( व्योम *block );
         व्योम *_Cdecl farmalloc( अचिन्हित दीर्घ nbytes );
#      अन्यथा
#        include <alloc.h>
#      endif
#    अन्यथा /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#पूर्ण_अगर

#अगर_घोषित AMIGA
#  define OS_CODE  1
#पूर्ण_अगर

#अगर defined(VAXC) || defined(VMS)
#  define OS_CODE  2
#  define F_OPEN(name, mode) \
     ख_खोलो((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#पूर्ण_अगर

#अगर_घोषित __370__
#  अगर __TARGET_LIB__ < 0x20000000
#    define OS_CODE 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_CODE 11
#  अन्यथा
#    define OS_CODE 8
#  endif
#पूर्ण_अगर

#अगर defined(ATARI) || defined(atarist)
#  define OS_CODE  5
#पूर्ण_अगर

#अगर_घोषित OS2
#  define OS_CODE  6
#  अगर defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#पूर्ण_अगर

#अगर defined(MACOS)
#  define OS_CODE  7
#पूर्ण_अगर

#अगर defined(__acorn) || defined(__riscos)
#  define OS_CODE 13
#पूर्ण_अगर

#अगर defined(WIN32) && !defined(__CYGWIN__)
#  define OS_CODE  10
#पूर्ण_अगर

#अगर_घोषित _BEOS_
#  define OS_CODE  16
#पूर्ण_अगर

#अगर_घोषित __TOS_OS400__
#  define OS_CODE 18
#पूर्ण_अगर

#अगर_घोषित __APPLE__
#  define OS_CODE 19
#पूर्ण_अगर

#अगर defined(__BORLANDC__) && !defined(MSDOS)
  #आशय warn -8004
  #आशय warn -8008
  #आशय warn -8066
#पूर्ण_अगर

/* provide prototypes for these when building zlib without LFS */
#अगर_अघोषित Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT crc32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT crc32_combine_gen64(z_off64_t);
#पूर्ण_अगर

        /* common defaults */

#अगर_अघोषित OS_CODE
#  define OS_CODE  3     /* assume Unix */
#पूर्ण_अगर

#अगर_अघोषित F_OPEN
#  define F_OPEN(name, mode) ख_खोलो((name), (mode))
#पूर्ण_अगर

         /* functions */

#अगर defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#पूर्ण_अगर
#अगर defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#पूर्ण_अगर
#अगर defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#पूर्ण_अगर
#अगर_घोषित HAVE_MEMCPY
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
   व्योम ZLIB_INTERNAL zmemcpy(व्योम FAR *, स्थिर व्योम FAR *, z_आकार_t);
   पूर्णांक ZLIB_INTERNAL zmemcmp(स्थिर व्योम FAR *, स्थिर व्योम FAR *, z_आकार_t);
   व्योम ZLIB_INTERNAL zmemzero(व्योम FAR *, z_आकार_t);
#पूर्ण_अगर

/* Diagnostic functions */
#अगर_घोषित ZLIB_DEBUG
#  include <stdio.h>
   बाह्य पूर्णांक ZLIB_INTERNAL z_verbose;
   बाह्य व्योम ZLIB_INTERNAL z_error(अक्षर *m);
#  define Assert(cond,संदेश) {अगर(!(cond)) z_error(संदेश);}
#  define Trace(x) {अगर (z_verbose>=0) ख_लिखो x ;}
#  define Tracev(x) {अगर (z_verbose>0) ख_लिखो x ;}
#  define Tracevv(x) {अगर (z_verbose>1) ख_लिखो x ;}
#  define Tracec(c,x) {अगर (z_verbose>0 && (c)) ख_लिखो x ;}
#  define Tracecv(c,x) {अगर (z_verbose>1 && (c)) ख_लिखो x ;}
#अन्यथा
#  define Assert(cond,संदेश)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#पूर्ण_अगर

#अगर_अघोषित Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, अचिन्हित वस्तुएँ_2,
                                अचिन्हित आकार);
   व्योम ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr);
#पूर्ण_अगर

#घोषणा ZALLOC(strm, वस्तुएँ_2, आकार) \
           (*((strm)->zalloc))((strm)->opaque, (वस्तुएँ_2), (आकार))
#घोषणा ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#घोषणा TRY_FREE(s, p) {अगर (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#घोषणा ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#अगर_घोषित Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#अगर defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#समावेश <stdatomic.h>
प्रकार काष्ठा {
    atomic_flag begun;
    atomic_int done;
} z_once_t;
#घोषणा Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local व्योम z_once(z_once_t *state, व्योम (*init)(व्योम)) {
    अगर (!atomic_load(&state->done)) {
        अगर (atomic_flag_test_and_set(&state->begun))
            जबतक (!atomic_load(&state->done))
                ;
        अन्यथा {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#अन्यथा   /* no atomics */

#warning zlib not thread-safe

प्रकार काष्ठा z_once_s {
    अस्थिर पूर्णांक begun;
    अस्थिर पूर्णांक done;
} z_once_t;
#घोषणा Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local पूर्णांक test_and_set(पूर्णांक अस्थिर *flag) {
    पूर्णांक was;

    was = *flag;
    *flag = 1;
    वापस was;
}

/* Run the provided init() function once. This is not thread-safe. */
local व्योम z_once(z_once_t *state, व्योम (*init)(व्योम)) {
    अगर (!state->done) {
        अगर (test_and_set(&state->begun))
            जबतक (!state->done)
                ;
        अन्यथा {
            init();
            state->done = 1;
        }
    }
}

#पूर्ण_अगर /* ?atomics */

#पूर्ण_अगर /* Z_ONCE */

#पूर्ण_अगर /* ZUTIL_H */
