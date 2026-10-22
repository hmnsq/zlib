/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#यदि_अपरिभाषितम् ZUTIL_H
#परिभाषय ZUTIL_H

#यदि_परिभाषितम् HAVE_HIDDEN
#  define ZLIB_आन्तरिकम् __attribute__((visibility ("hidden")))
#अन्यथा
#  define ZLIB_आन्तरिकम्
#यदि_अन्तः

#अन्तर्भाव "zlib.h"

#यदि defined(STDC) && !defined(Z_SOLO)
#  यदि !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#यदि_अन्तः

#यदि_अपरिभाषितम् local
#  define local स्थिर
#यदि_अन्तः
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

बाह्य ध्रुव वर्ण deflate_copyright[];
बाह्य ध्रुव वर्ण inflate_copyright[];
बाह्य ध्रुव वर्ण inflate9_copyright[];

प्रकारनिर्देश अचिह्नित वर्ण  uch;
प्रकारनिर्देश uch FAR uchf;
प्रकारनिर्देश अचिह्नित ह्रस्व ush;
प्रकारनिर्देश ush FAR ushf;
प्रकारनिर्देश अचिह्नित दीर्घ  ulg;

#यदि !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  यदि (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिह्नित दीर्घ
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिह्नित दीर्घ दीर्घ
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिह्नित दीर्घ दीर्घ
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 अचिह्नित
#  endif
#यदि_अन्तः

बाह्य z_const वर्ण * ध्रुव z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#परिभाषय ERR_सन्देशः(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#परिभाषय ERR_RETURN(strm,err) \
  प्रत्यागम (strm->सन्देशः = ERR_सन्देशः(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#यदि MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be अन्तर् 9..15
#यदि_अन्तः
#यदि_अपरिभाषितम् DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#यदि_अन्तः
/* default windowBits for decompression. MAX_WBITS is for compression only */

#यदि MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#यदि_अन्तः
/* default memLevel */

#परिभाषय STORED_खण्डम् 0
#परिभाषय STATIC_TREES 1
#परिभाषय DYN_TREES    2
/* The three kinds of block type */

#परिभाषय MIN_मेलनम्  3
#परिभाषय MAX_मेलनम्  258
/* The minimum and maximum match lengths */

#परिभाषय PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#यदि defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_संहिता  0x00
#  ifndef Z_SOLO
#    यदि defined(__TURBOC__) || defined(__BORLANDC__)
#      यदि (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         शून्य _Cdecl farfree( शून्य *खण्डम् );
         शून्य *_Cdecl farmalloc( अचिह्नित दीर्घ nbytes );
#      अन्यथा
#        include <alloc.h>
#      endif
#    अन्यथा /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#यदि_अन्तः

#यदि_परिभाषितम् AMIGA
#  define OS_संहिता  1
#यदि_अन्तः

#यदि defined(VAXC) || defined(VMS)
#  define OS_संहिता  2
#  define F_OPEN(name, बहुलकम्) \
     fopen((name), (बहुलकम्), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#यदि_अन्तः

#यदि_परिभाषितम् __370__
#  यदि __TARGET_LIB__ < 0x20000000
#    define OS_संहिता 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_संहिता 11
#  अन्यथा
#    define OS_संहिता 8
#  endif
#यदि_अन्तः

#यदि defined(ATARI) || defined(atarist)
#  define OS_संहिता  5
#यदि_अन्तः

#यदि_परिभाषितम् OS2
#  define OS_संहिता  6
#  यदि defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#यदि_अन्तः

#यदि defined(MACOS)
#  define OS_संहिता  7
#यदि_अन्तः

#यदि defined(__acorn) || defined(__riscos)
#  define OS_संहिता 13
#यदि_अन्तः

#यदि defined(WIN32) && !defined(__CYGWIN__)
#  define OS_संहिता  10
#यदि_अन्तः

#यदि_परिभाषितम् _BEOS_
#  define OS_संहिता  16
#यदि_अन्तः

#यदि_परिभाषितम् __TOS_OS400__
#  define OS_संहिता 18
#यदि_अन्तः

#यदि_परिभाषितम् __APPLE__
#  define OS_संहिता 19
#यदि_अन्तः

#यदि defined(__BORLANDC__) && !defined(MSDOS)
  #निर्देशः warn -8004
  #निर्देशः warn -8008
  #निर्देशः warn -8066
#यदि_अन्तः

/* provide prototypes for these when building zlib without LFS */
#यदि_अपरिभाषितम् Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT चक्रीयपरीक्षा_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT चक्रीयपरीक्षा_32_combine_gen64(z_off64_t);
#यदि_अन्तः

        /* common defaults */

#यदि_अपरिभाषितम् OS_संहिता
#  define OS_संहिता  3     /* assume Unix */
#यदि_अन्तः

#यदि_अपरिभाषितम् F_OPEN
#  define F_OPEN(name, बहुलकम्) fopen((name), (बहुलकम्))
#यदि_अन्तः

         /* functions */

#यदि defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#यदि_अन्तः
#यदि defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#यदि_अन्तः
#यदि defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#यदि_अन्तः
#यदि_परिभाषितम् HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  अन्यथा
#    define zmemcpy स्मृतिं_प्रतिलिख
#    define zmemcmp memcmp
#    define zmemzero(dest, len) स्मृतिं_पूरय(dest, 0, len)
#  endif
#अन्यथा
   शून्य ZLIB_आन्तरिकम् zmemcpy(शून्य FAR *, ध्रुव शून्य FAR *, z_आकार_t);
   पूर्णाङ्क ZLIB_आन्तरिकम् zmemcmp(ध्रुव शून्य FAR *, ध्रुव शून्य FAR *, z_आकार_t);
   शून्य ZLIB_आन्तरिकम् zmemzero(शून्य FAR *, z_आकार_t);
#यदि_अन्तः

/* Diagnostic functions */
#यदि_परिभाषितम् ZLIB_DEBUG
#  include <stdio.h>
   बाह्य पूर्णाङ्क ZLIB_आन्तरिकम् z_verbose;
   बाह्य शून्य ZLIB_आन्तरिकम् z_error(वर्ण *m);
#  define Assert(cond,सन्देशः) {यदि(!(cond)) z_error(सन्देशः);}
#  define Trace(x) {यदि (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {यदि (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {यदि (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {यदि (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {यदि (z_verbose>1 && (c)) fprintf x ;}
#अन्यथा
#  define Assert(cond,सन्देशः)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#यदि_अन्तः

#यदि_अपरिभाषितम् Z_SOLO
   voidpf ZLIB_आन्तरिकम् zcalloc(voidpf opaque, अचिह्नित वस्तूनि_2,
                                अचिह्नित आकार);
   शून्य ZLIB_आन्तरिकम् zcfree(voidpf opaque, voidpf ptr);
#यदि_अन्तः

#परिभाषय ZALLOC(strm, वस्तूनि_2, आकार) \
           (*((strm)->zalloc))((strm)->opaque, (वस्तूनि_2), (आकार))
#परिभाषय ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#परिभाषय TRY_FREE(s, p) {यदि (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#परिभाषय ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#यदि_परिभाषितम् Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#यदि defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#अन्तर्भाव <stdatomic.h>
प्रकारनिर्देश संरचना {
    atomic_flag begun;
    atomic_int समाप्तम्_2;
} z_once_t;
#परिभाषय Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local शून्य z_once(z_once_t *state, शून्य (*init)(शून्य)) {
    यदि (!atomic_आरोपय(&state->समाप्तम्_2)) {
        यदि (atomic_flag_test_and_set(&state->begun))
            यावत् (!atomic_आरोपय(&state->समाप्तम्_2))
                ;
        अन्यथा {
            init();
            atomic_store(&state->समाप्तम्_2, 1);
        }
    }
}

#अन्यथा   /* no atomics */

#warning zlib not thread-safe

प्रकारनिर्देश संरचना z_once_s {
    चञ्चल पूर्णाङ्क begun;
    चञ्चल पूर्णाङ्क समाप्तम्_2;
} z_once_t;
#परिभाषय Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local पूर्णाङ्क test_and_समुच्चयः(पूर्णाङ्क चञ्चल *flag) {
    पूर्णाङ्क was;

    was = *flag;
    *flag = 1;
    प्रत्यागम was;
}

/* Run the provided init() function once. This is not thread-safe. */
local शून्य z_once(z_once_t *state, शून्य (*init)(शून्य)) {
    यदि (!state->समाप्तम्_2) {
        यदि (test_and_समुच्चयः(&state->begun))
            यावत् (!state->समाप्तम्_2)
                ;
        अन्यथा {
            init();
            state->समाप्तम्_2 = 1;
        }
    }
}

#यदि_अन्तः /* ?atomics */

#यदि_अन्तः /* Z_ONCE */

#यदि_अन्तः /* ZUTIL_H */
