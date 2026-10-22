/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#جے_بنا_تعریف ZUTIL_H
#تعریف_کرو ZUTIL_H

#جے_تعریف_شدہ HAVE_HIDDEN
#  define ZLIB_اندرونی __attribute__((visibility ("hidden")))
#وإلا
#  define ZLIB_اندرونی
#جے_انت

#ضمن "zlib.h"

#إذا defined(STDC) && !defined(Z_SOLO)
#  إذا !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#جے_انت

#جے_بنا_تعریف local
#  define local ساكن
#جے_انت
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

خارجي ثابت حرف deflate_copyright[];
خارجي ثابت حرف inflate_copyright[];
خارجي ثابت حرف inflate9_copyright[];

تعريفنوع بنانشانی حرف  uch;
تعريفنوع uch FAR uchf;
تعريفنوع بنانشانی چھوٹا ush;
تعريفنوع ush FAR ushf;
تعريفنوع بنانشانی لما  ulg;

#إذا !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  إذا (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشانی لما
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشانی لما لما
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشانی لما لما
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 بنانشانی
#  endif
#جے_انت

خارجي z_const حرف * ثابت z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#تعریف_کرو ERR_پیغام(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#تعریف_کرو ERR_RETURN(strm,err) \
  أرجع (strm->پیغام = ERR_پیغام(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#إذا MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be وچ 9..15
#جے_انت
#جے_بنا_تعریف DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#جے_انت
/* default windowBits for decompression. MAX_WBITS is for compression only */

#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#جے_انت
/* default memLevel */

#تعریف_کرو STORED_بلاک 0
#تعریف_کرو STATIC_TREES 1
#تعریف_کرو DYN_TREES    2
/* The three kinds of block type */

#تعریف_کرو MIN_میل  3
#تعریف_کرو MAX_میل  258
/* The minimum and maximum match lengths */

#تعریف_کرو PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#إذا defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_کوڈ  0x00
#  ifndef Z_SOLO
#    إذا defined(__TURBOC__) || defined(__BORLANDC__)
#      إذا (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         فراغ _Cdecl farfree( فراغ *بلاک );
         فراغ *_Cdecl farmalloc( بنانشانی لما nbytes );
#      وإلا
#        include <alloc.h>
#      endif
#    وإلا /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#جے_انت

#جے_تعریف_شدہ AMIGA
#  define OS_کوڈ  1
#جے_انت

#إذا defined(VAXC) || defined(VMS)
#  define OS_کوڈ  2
#  define F_OPEN(name, منوال) \
     fopen((name), (منوال), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#جے_انت

#جے_تعریف_شدہ __370__
#  إذا __TARGET_LIB__ < 0x20000000
#    define OS_کوڈ 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_کوڈ 11
#  وإلا
#    define OS_کوڈ 8
#  endif
#جے_انت

#إذا defined(ATARI) || defined(atarist)
#  define OS_کوڈ  5
#جے_انت

#جے_تعریف_شدہ OS2
#  define OS_کوڈ  6
#  إذا defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#جے_انت

#إذا defined(MACOS)
#  define OS_کوڈ  7
#جے_انت

#إذا defined(__acorn) || defined(__riscos)
#  define OS_کوڈ 13
#جے_انت

#إذا defined(WIN32) && !defined(__CYGWIN__)
#  define OS_کوڈ  10
#جے_انت

#جے_تعریف_شدہ _BEOS_
#  define OS_کوڈ  16
#جے_انت

#جے_تعریف_شدہ __TOS_OS400__
#  define OS_کوڈ 18
#جے_انت

#جے_تعریف_شدہ __APPLE__
#  define OS_کوڈ 19
#جے_انت

#إذا defined(__BORLANDC__) && !defined(MSDOS)
  #ہدایت warn -8004
  #ہدایت warn -8008
  #ہدایت warn -8066
#جے_انت

/* provide prototypes for these when building zlib without LFS */
#جے_بنا_تعریف Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT چکری_جانچ_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT چکری_جانچ_32_combine_gen64(z_off64_t);
#جے_انت

        /* common defaults */

#جے_بنا_تعریف OS_کوڈ
#  define OS_کوڈ  3     /* assume Unix */
#جے_انت

#جے_بنا_تعریف F_OPEN
#  define F_OPEN(name, منوال) fopen((name), (منوال))
#جے_انت

         /* functions */

#إذا defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#جے_انت
#إذا defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#جے_انت
#إذا defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#جے_انت
#جے_تعریف_شدہ HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  وإلا
#    define zmemcpy حافظہ_نقل
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#وإلا
   فراغ ZLIB_اندرونی zmemcpy(فراغ FAR *, ثابت فراغ FAR *, z_ناپ_t);
   صحيح ZLIB_اندرونی zmemcmp(ثابت فراغ FAR *, ثابت فراغ FAR *, z_ناپ_t);
   فراغ ZLIB_اندرونی zmemzero(فراغ FAR *, z_ناپ_t);
#جے_انت

/* Diagnostic functions */
#جے_تعریف_شدہ ZLIB_DEBUG
#  include <stdio.h>
   خارجي صحيح ZLIB_اندرونی z_verbose;
   خارجي فراغ ZLIB_اندرونی z_error(حرف *m);
#  define Assert(cond,پیغام) {إذا(!(cond)) z_error(پیغام);}
#  define Trace(x) {إذا (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {إذا (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {إذا (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {إذا (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {إذا (z_verbose>1 && (c)) fprintf x ;}
#وإلا
#  define Assert(cond,پیغام)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#جے_انت

#جے_بنا_تعریف Z_SOLO
   voidpf ZLIB_اندرونی zcalloc(voidpf opaque, بنانشانی اشیاء_2,
                                بنانشانی ناپ);
   فراغ ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr);
#جے_انت

#تعریف_کرو ZALLOC(strm, اشیاء_2, ناپ) \
           (*((strm)->zalloc))((strm)->opaque, (اشیاء_2), (ناپ))
#تعریف_کرو ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#تعریف_کرو TRY_FREE(s, p) {إذا (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#تعریف_کرو ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#جے_تعریف_شدہ Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#إذا defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ضمن <stdatomic.h>
تعريفنوع بنية {
    atomic_flag begun;
    atomic_int پورا_2;
} z_once_t;
#تعریف_کرو Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!atomic_لدو(&state->پورا_2)) {
        إذا (atomic_flag_test_and_set(&state->begun))
            طالما (!atomic_لدو(&state->پورا_2))
                ;
        وإلا {
            init();
            atomic_store(&state->پورا_2, 1);
        }
    }
}

#وإلا   /* no atomics */

#warning zlib not thread-محفوظ

تعريفنوع بنية z_once_s {
    متطاير صحيح begun;
    متطاير صحيح پورا_2;
} z_once_t;
#تعریف_کرو Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local صحيح test_and_سیٹ(صحيح متطاير *flag) {
    صحيح was;

    was = *flag;
    *flag = 1;
    أرجع was;
}

/* Run the provided init() function once. This is not thread-safe. */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!state->پورا_2) {
        إذا (test_and_سیٹ(&state->begun))
            طالما (!state->پورا_2)
                ;
        وإلا {
            init();
            state->پورا_2 = 1;
        }
    }
}

#جے_انت /* ?atomics */

#جے_انت /* Z_ONCE */

#جے_انت /* ZUTIL_H */
