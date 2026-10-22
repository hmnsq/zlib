/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#که_نه_تعریف_شوی ZUTIL_H
#تعریف_کړه ZUTIL_H

#که_تعریف_شوی HAVE_HIDDEN
#  define ZLIB_دننی __attribute__((visibility ("hidden")))
#وإلا
#  define ZLIB_دننی
#د_که_پای

#ضمن "zlib.h"

#إذا defined(STDC) && !defined(Z_SOLO)
#  إذا !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#د_که_پای

#که_نه_تعریف_شوی local
#  define local ساكن
#د_که_پای
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

خارجي ثابت حرف deflate_copyright[];
خارجي ثابت حرف inflate_copyright[];
خارجي ثابت حرف inflate9_copyright[];

تعريفنوع بینښه حرف  uch;
تعريفنوع uch FAR uchf;
تعريفنوع بینښه لنډ ush;
تعريفنوع ush FAR ushf;
تعريفنوع بینښه اوږد  ulg;

#إذا !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  إذا (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بینښه اوږد
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بینښه اوږد اوږد
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 بینښه اوږد اوږد
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 بینښه
#  endif
#د_که_پای

خارجي z_const حرف * ثابت z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#تعریف_کړه ERR_پیغام(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#تعریف_کړه ERR_RETURN(strm,err) \
  أرجع (strm->پیغام = ERR_پیغام(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#إذا MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be دننه 9..15
#د_که_پای
#که_نه_تعریف_شوی DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#د_که_پای
/* default windowBits for decompression. MAX_WBITS is for compression only */

#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#د_که_پای
/* default memLevel */

#تعریف_کړه STORED_بلاک 0
#تعریف_کړه STATIC_TREES 1
#تعریف_کړه DYN_TREES    2
/* The three kinds of block type */

#تعریف_کړه MIN_سمون  3
#تعریف_کړه MAX_سمون  258
/* The minimum and maximum match lengths */

#تعریف_کړه PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#إذا defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_کوډ  0x00
#  ifndef Z_SOLO
#    إذا defined(__TURBOC__) || defined(__BORLANDC__)
#      إذا (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         فراغ _Cdecl farfree( فراغ *بلاک );
         فراغ *_Cdecl farmalloc( بینښه اوږد nbytes );
#      وإلا
#        include <alloc.h>
#      endif
#    وإلا /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#د_که_پای

#که_تعریف_شوی AMIGA
#  define OS_کوډ  1
#د_که_پای

#إذا defined(VAXC) || defined(VMS)
#  define OS_کوډ  2
#  define F_OPEN(name, ډېرپېښ) \
     fopen((name), (ډېرپېښ), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#د_که_پای

#که_تعریف_شوی __370__
#  إذا __TARGET_LIB__ < 0x20000000
#    define OS_کوډ 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_کوډ 11
#  وإلا
#    define OS_کوډ 8
#  endif
#د_که_پای

#إذا defined(ATARI) || defined(atarist)
#  define OS_کوډ  5
#د_که_پای

#که_تعریف_شوی OS2
#  define OS_کوډ  6
#  إذا defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#د_که_پای

#إذا defined(MACOS)
#  define OS_کوډ  7
#د_که_پای

#إذا defined(__acorn) || defined(__riscos)
#  define OS_کوډ 13
#د_که_پای

#إذا defined(WIN32) && !defined(__CYGWIN__)
#  define OS_کوډ  10
#د_که_پای

#که_تعریف_شوی _BEOS_
#  define OS_کوډ  16
#د_که_پای

#که_تعریف_شوی __TOS_OS400__
#  define OS_کوډ 18
#د_که_پای

#که_تعریف_شوی __APPLE__
#  define OS_کوډ 19
#د_که_پای

#إذا defined(__BORLANDC__) && !defined(MSDOS)
  #لارښوونه warn -8004
  #لارښوونه warn -8008
  #لارښوونه warn -8066
#د_که_پای

/* provide prototypes for these when building zlib without LFS */
#که_نه_تعریف_شوی Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT دوراني_ازموینه_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT دوراني_ازموینه_32_combine_gen64(z_off64_t);
#د_که_پای

        /* common defaults */

#که_نه_تعریف_شوی OS_کوډ
#  define OS_کوډ  3     /* assume Unix */
#د_که_پای

#که_نه_تعریف_شوی F_OPEN
#  define F_OPEN(name, ډېرپېښ) fopen((name), (ډېرپېښ))
#د_که_پای

         /* functions */

#إذا defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#د_که_پای
#إذا defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#د_که_پای
#إذا defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#د_که_پای
#که_تعریف_شوی HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  وإلا
#    define zmemcpy حافظه_نقل
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#وإلا
   فراغ ZLIB_دننی zmemcpy(فراغ FAR *, ثابت فراغ FAR *, z_اندازه_t);
   صحيح ZLIB_دننی zmemcmp(ثابت فراغ FAR *, ثابت فراغ FAR *, z_اندازه_t);
   فراغ ZLIB_دننی zmemzero(فراغ FAR *, z_اندازه_t);
#د_که_پای

/* Diagnostic functions */
#که_تعریف_شوی ZLIB_DEBUG
#  include <stdio.h>
   خارجي صحيح ZLIB_دننی z_verbose;
   خارجي فراغ ZLIB_دننی z_error(حرف *m);
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
#د_که_پای

#که_نه_تعریف_شوی Z_SOLO
   voidpf ZLIB_دننی zcalloc(voidpf opaque, بینښه توکي_2,
                                بینښه اندازه);
   فراغ ZLIB_دننی zcfree(voidpf opaque, voidpf ptr);
#د_که_پای

#تعریف_کړه ZALLOC(strm, توکي_2, اندازه) \
           (*((strm)->zalloc))((strm)->opaque, (توکي_2), (اندازه))
#تعریف_کړه ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#تعریف_کړه TRY_FREE(s, p) {إذا (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#تعریف_کړه ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#که_تعریف_شوی Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#إذا defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#ضمن <stdatomic.h>
تعريفنوع بنية {
    atomic_flag begun;
    atomic_int بشپړ_2;
} z_once_t;
#تعریف_کړه Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!atomic_بارول(&state->بشپړ_2)) {
        إذا (atomic_flag_test_and_set(&state->begun))
            طالما (!atomic_بارول(&state->بشپړ_2))
                ;
        وإلا {
            init();
            atomic_store(&state->بشپړ_2, 1);
        }
    }
}

#وإلا   /* no atomics */

#warning zlib not thread-خوندي

تعريفنوع بنية z_once_s {
    متطاير صحيح begun;
    متطاير صحيح بشپړ_2;
} z_once_t;
#تعریف_کړه Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local صحيح test_and_سېټ(صحيح متطاير *flag) {
    صحيح was;

    was = *flag;
    *flag = 1;
    أرجع was;
}

/* Run the provided init() function once. This is not thread-safe. */
local فراغ z_once(z_once_t *state, فراغ (*init)(فراغ)) {
    إذا (!state->بشپړ_2) {
        إذا (test_and_سېټ(&state->begun))
            طالما (!state->بشپړ_2)
                ;
        وإلا {
            init();
            state->بشپړ_2 = 1;
        }
    }
}

#د_که_پای /* ?atomics */

#د_که_پای /* Z_ONCE */

#د_که_پای /* ZUTIL_H */
