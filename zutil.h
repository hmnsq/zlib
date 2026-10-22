/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#جيڪڏهن_وصف_نه_ٿيل ZUTIL_H
#وصف_ڪريو ZUTIL_H

#جيڪڏهن_وصف_ٿيل HAVE_HIDDEN
#  define ZLIB_اندروني __attribute__((visibility ("hidden")))
#وإلا
#  define ZLIB_اندروني
#جيڪڏهن_پورو

#ضمن "zlib.h"

#إذا defined(STDC) && !defined(Z_SOLO)
#  إذا !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل local
#  define local ساكن
#جيڪڏهن_پورو
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

خارجي ثابت حرف deflate_copyright[];
خارجي ثابت حرف inflate_copyright[];
خارجي ثابت حرف inflate9_copyright[];

تعريفنوع بنانشان حرف  uch;
تعريفنوع uch FAR uchf;
تعريفنوع بنانشان ننڍو ush;
تعريفنوع ush FAR ushf;
تعريفنوع بنانشان ڊگهو  ulg;

#إذا !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  إذا (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشان ڊگهو
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشان ڊگهو ڊگهو
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشان ڊگهو ڊگهو
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشان
#  endif
#جيڪڏهن_پورو

خارجي z_const حرف * ثابت z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#وصف_ڪريو ERR_نياپو(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#وصف_ڪريو ERR_RETURN(strm,err) \
  أرجع (strm->نياپو = ERR_نياپو(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#إذا MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be اندريون 9..15
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#جيڪڏهن_پورو
/* default windowBits for decompression. MAX_WBITS is for compression only */

#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#جيڪڏهن_پورو
/* default memLevel */

#وصف_ڪريو STORED_بلاڪ 0
#وصف_ڪريو STATIC_TREES 1
#وصف_ڪريو DYN_TREES    2
/* The three kinds of block type */

#وصف_ڪريو MIN_ميلاپ  3
#وصف_ڪريو MAX_ميلاپ  258
/* The minimum and maximum match lengths */

#وصف_ڪريو PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#إذا defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_ڪوڊ  0x00
#  ifndef Z_SOLO
#    إذا defined(__TURBOC__) || defined(__BORLANDC__)
#      إذا (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         فراغ _Cdecl farfree( فراغ *بلاڪ );
         فراغ *_Cdecl farmalloc( بنانشان ڊگهو nbytes );
#      وإلا
#        include <alloc.h>
#      endif
#    وإلا /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل AMIGA
#  define OS_ڪوڊ  1
#جيڪڏهن_پورو

#إذا defined(VAXC) || defined(VMS)
#  define OS_ڪوڊ  2
#  define F_OPEN(name, گهڻائي) \
     fopen((name), (گهڻائي), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __370__
#  إذا __TARGET_LIB__ < 0x20000000
#    define OS_ڪوڊ 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_ڪوڊ 11
#  وإلا
#    define OS_ڪوڊ 8
#  endif
#جيڪڏهن_پورو

#إذا defined(ATARI) || defined(atarist)
#  define OS_ڪوڊ  5
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل OS2
#  define OS_ڪوڊ  6
#  إذا defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#جيڪڏهن_پورو

#إذا defined(MACOS)
#  define OS_ڪوڊ  7
#جيڪڏهن_پورو

#إذا defined(__acorn) || defined(__riscos)
#  define OS_ڪوڊ 13
#جيڪڏهن_پورو

#إذا defined(WIN32) && !defined(__CYGWIN__)
#  define OS_ڪوڊ  10
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل _BEOS_
#  define OS_ڪوڊ  16
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __TOS_OS400__
#  define OS_ڪوڊ 18
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __APPLE__
#  define OS_ڪوڊ 19
#جيڪڏهن_پورو

#إذا defined(__BORLANDC__) && !defined(MSDOS)
  #هدايت warn -8004
  #هدايت warn -8008
  #هدايت warn -8066
#جيڪڏهن_پورو

/* provide prototypes for these when building zlib without LFS */
#جيڪڏهن_وصف_نه_ٿيل Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT چڪري_جاچ_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT چڪري_جاچ_32_combine_gen64(z_off64_t);
#جيڪڏهن_پورو

        /* common defaults */

#جيڪڏهن_وصف_نه_ٿيل OS_ڪوڊ
#  define OS_ڪوڊ  3     /* assume Unix */
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل F_OPEN
#  define F_OPEN(name, گهڻائي) fopen((name), (گهڻائي))
#جيڪڏهن_پورو

         /* functions */

#إذا defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#جيڪڏهن_پورو
#إذا defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#جيڪڏهن_پورو
#إذا defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  وإلا
#    define zmemcpy يادگيري_نقل
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#وإلا
   فراغ ZLIB_اندروني zmemcpy(فراغ FAR *, ثابت فراغ FAR *, z_سائيز_t);
   صحيح ZLIB_اندروني zmemcmp(ثابت فراغ FAR *, ثابت فراغ FAR *, z_سائيز_t);
   فراغ ZLIB_اندروني zmemzero(فراغ FAR *, z_سائيز_t);
#جيڪڏهن_پورو

/* Diagnostic functions */
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
#  include <stdio.h>
   خارجي صحيح ZLIB_اندروني z_verbose;
   خارجي فراغ ZLIB_اندروني z_error(حرف *m);
#  define Assert(cond,نياپو) {إذا(!(cond)) z_error(نياپو);}
#  define Trace(x) {إذا (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {إذا (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {إذا (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {إذا (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {إذا (z_verbose>1 && (c)) fprintf x ;}
#وإلا
#  define Assert(cond,نياپو)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل Z_SOLO
   voidpf ZLIB_اندروني zcalloc(voidpf opaque, بنانشان شيون_2,
                                بنانشان سائيز);
   فراغ ZLIB_اندروني zcfree(voidpf opaque, voidpf ptr);
#جيڪڏهن_پورو

#وصف_ڪريو ZALLOC(strm, شيون_2, سائيز) \
           (*((strm)->zalloc))((strm)->opaque, (شيون_2), (سائيز))
#وصف_ڪريو ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#وصف_ڪريو TRY_FREE(s, p) {إذا (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#وصف_ڪريو ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#جيڪڏهن_وصف_ٿيل Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#إذا defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ضمن <stdatomic.h>
تعريفنوع بنية {
    atomic_flag begun;
    atomic_int پورو_2;
} z_once_t;
#وصف_ڪريو Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!atomic_ڀريو(&state->پورو_2)) {
        إذا (atomic_flag_test_and_set(&state->begun))
            طالما (!atomic_ڀريو(&state->پورو_2))
                ;
        وإلا {
            init();
            atomic_store(&state->پورو_2, 1);
        }
    }
}

#وإلا   /* no atomics */

#warning zlib not thread-محفوظ

تعريفنوع بنية z_once_s {
    متطاير صحيح begun;
    متطاير صحيح پورو_2;
} z_once_t;
#وصف_ڪريو Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local صحيح test_and_سيٽ(صحيح متطاير *flag) {
    صحيح was;

    was = *flag;
    *flag = 1;
    أرجع was;
}

/* Run the provided init() function once. This is not thread-safe. */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!state->پورو_2) {
        إذا (test_and_سيٽ(&state->begun))
            طالما (!state->پورو_2)
                ;
        وإلا {
            init();
            state->پورو_2 = 1;
        }
    }
}

#جيڪڏهن_پورو /* ?atomics */

#جيڪڏهن_پورو /* Z_ONCE */

#جيڪڏهن_پورو /* ZUTIL_H */
