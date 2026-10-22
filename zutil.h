/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#اگر_تعریف_نشده ZUTIL_H
#تعریف_کن ZUTIL_H

#اگر_تعریف_شده HAVE_HIDDEN
#  define ZLIB_داخلی __attribute__((visibility ("hidden")))
#وإلا
#  define ZLIB_داخلی
#پایان_اگر

#ضمن "zlib.h"

#إذا defined(STDC) && !defined(Z_SOLO)
#  إذا !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#پایان_اگر

#اگر_تعریف_نشده local
#  define local ساكن
#پایان_اگر
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

خارجي ثابت حرف deflate_copyright[];
خارجي ثابت حرف inflate_copyright[];
خارجي ثابت حرف inflate9_copyright[];

تعريفنوع بیعلامت حرف  uch;
تعريفنوع uch FAR uchf;
تعريفنوع بیعلامت کوتاه ush;
تعريفنوع ush FAR ushf;
تعريفنوع بیعلامت دراز  ulg;

#إذا !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  إذا (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بیعلامت دراز
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بیعلامت دراز دراز
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بیعلامت دراز دراز
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 بیعلامت
#  endif
#پایان_اگر

خارجي z_const حرف * ثابت z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#تعریف_کن ERR_پیام(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#تعریف_کن ERR_RETURN(strm,err) \
  أرجع (strm->پیام = ERR_پیام(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#إذا MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be درون 9..15
#پایان_اگر
#اگر_تعریف_نشده DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#پایان_اگر
/* default windowBits for decompression. MAX_WBITS is for compression only */

#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#پایان_اگر
/* default memLevel */

#تعریف_کن STORED_بلوک 0
#تعریف_کن STATIC_TREES 1
#تعریف_کن DYN_TREES    2
/* The three kinds of block type */

#تعریف_کن MIN_تطابق  3
#تعریف_کن MAX_تطابق  258
/* The minimum and maximum match lengths */

#تعریف_کن PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#إذا defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_کد  0x00
#  ifndef Z_SOLO
#    إذا defined(__TURBOC__) || defined(__BORLANDC__)
#      إذا (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         فراغ _Cdecl farfree( فراغ *بلوک );
         فراغ *_Cdecl farmalloc( بیعلامت دراز nbytes );
#      وإلا
#        include <alloc.h>
#      endif
#    وإلا /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#پایان_اگر

#اگر_تعریف_شده AMIGA
#  define OS_کد  1
#پایان_اگر

#إذا defined(VAXC) || defined(VMS)
#  define OS_کد  2
#  define F_OPEN(name, نما) \
     fopen((name), (نما), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#پایان_اگر

#اگر_تعریف_شده __370__
#  إذا __TARGET_LIB__ < 0x20000000
#    define OS_کد 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_کد 11
#  وإلا
#    define OS_کد 8
#  endif
#پایان_اگر

#إذا defined(ATARI) || defined(atarist)
#  define OS_کد  5
#پایان_اگر

#اگر_تعریف_شده OS2
#  define OS_کد  6
#  إذا defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#پایان_اگر

#إذا defined(MACOS)
#  define OS_کد  7
#پایان_اگر

#إذا defined(__acorn) || defined(__riscos)
#  define OS_کد 13
#پایان_اگر

#إذا defined(WIN32) && !defined(__CYGWIN__)
#  define OS_کد  10
#پایان_اگر

#اگر_تعریف_شده _BEOS_
#  define OS_کد  16
#پایان_اگر

#اگر_تعریف_شده __TOS_OS400__
#  define OS_کد 18
#پایان_اگر

#اگر_تعریف_شده __APPLE__
#  define OS_کد 19
#پایان_اگر

#إذا defined(__BORLANDC__) && !defined(MSDOS)
  #رهنمود warn -8004
  #رهنمود warn -8008
  #رهنمود warn -8066
#پایان_اگر

/* provide prototypes for these when building zlib without LFS */
#اگر_تعریف_نشده Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT بررسی_چرخهای_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT بررسی_چرخهای_32_combine_gen64(z_off64_t);
#پایان_اگر

        /* common defaults */

#اگر_تعریف_نشده OS_کد
#  define OS_کد  3     /* assume Unix */
#پایان_اگر

#اگر_تعریف_نشده F_OPEN
#  define F_OPEN(name, نما) fopen((name), (نما))
#پایان_اگر

         /* functions */

#إذا defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#پایان_اگر
#إذا defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#پایان_اگر
#إذا defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#پایان_اگر
#اگر_تعریف_شده HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  وإلا
#    define zmemcpy کپی_حافظه
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#وإلا
   فراغ ZLIB_داخلی zmemcpy(فراغ FAR *, ثابت فراغ FAR *, z_اندازه_t);
   صحيح ZLIB_داخلی zmemcmp(ثابت فراغ FAR *, ثابت فراغ FAR *, z_اندازه_t);
   فراغ ZLIB_داخلی zmemzero(فراغ FAR *, z_اندازه_t);
#پایان_اگر

/* Diagnostic functions */
#اگر_تعریف_شده ZLIB_DEBUG
#  include <stdio.h>
   خارجي صحيح ZLIB_داخلی z_verbose;
   خارجي فراغ ZLIB_داخلی z_error(حرف *m);
#  define Assert(cond,پیام) {إذا(!(cond)) z_error(پیام);}
#  define Trace(x) {إذا (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {إذا (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {إذا (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {إذا (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {إذا (z_verbose>1 && (c)) fprintf x ;}
#وإلا
#  define Assert(cond,پیام)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#پایان_اگر

#اگر_تعریف_نشده Z_SOLO
   voidpf ZLIB_داخلی zcalloc(voidpf opaque, بیعلامت اقلام_2,
                                بیعلامت اندازه);
   فراغ ZLIB_داخلی zcfree(voidpf opaque, voidpf ptr);
#پایان_اگر

#تعریف_کن ZALLOC(strm, اقلام_2, اندازه) \
           (*((strm)->zalloc))((strm)->opaque, (اقلام_2), (اندازه))
#تعریف_کن ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#تعریف_کن TRY_FREE(s, p) {إذا (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#تعریف_کن ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#اگر_تعریف_شده Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#إذا defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ضمن <stdatomic.h>
تعريفنوع بنية {
    atomic_flag begun;
    atomic_int انجامشده_2;
} z_once_t;
#تعریف_کن Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!atomic_بارگیری(&state->انجامشده_2)) {
        إذا (atomic_flag_test_and_set(&state->begun))
            طالما (!atomic_بارگیری(&state->انجامشده_2))
                ;
        وإلا {
            init();
            atomic_store(&state->انجامشده_2, 1);
        }
    }
}

#وإلا   /* no atomics */

#warning zlib not thread-امن

تعريفنوع بنية z_once_s {
    متطاير صحيح begun;
    متطاير صحيح انجامشده_2;
} z_once_t;
#تعریف_کن Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local صحيح test_and_مجموعه(صحيح متطاير *flag) {
    صحيح was;

    was = *flag;
    *flag = 1;
    أرجع was;
}

/* Run the provided init() function once. This is not thread-safe. */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!state->انجامشده_2) {
        إذا (test_and_مجموعه(&state->begun))
            طالما (!state->انجامشده_2)
                ;
        وإلا {
            init();
            state->انجامشده_2 = 1;
        }
    }
}

#پایان_اگر /* ?atomics */

#پایان_اگر /* Z_ONCE */

#پایان_اگر /* ZUTIL_H */
