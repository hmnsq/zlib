/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#اگر_غیر_تعریف_شدہ ZUTIL_H
#تعریف_کرو ZUTIL_H

#اگر_تعریف_شدہ HAVE_HIDDEN
#  define ZLIB_اندرونی __attribute__((visibility ("hidden")))
#ورنہ
#  define ZLIB_اندرونی
#اگر_ختم

#شامل "zlib.h"

#اگر defined(STDC) && !defined(Z_SOLO)
#  اگر !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#اگر_ختم

#اگر_غیر_تعریف_شدہ local
#  define local ساکن
#اگر_ختم
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

خارجی ثابت حرف deflate_copyright[];
خارجی ثابت حرف inflate_copyright[];
خارجی ثابت حرف inflate9_copyright[];

قسمتعریف غیرعلامتی حرف  uch;
قسمتعریف uch FAR uchf;
قسمتعریف غیرعلامتی مختصر ush;
قسمتعریف ush FAR ushf;
قسمتعریف غیرعلامتی طویل  ulg;

#اگر !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  اگر (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 غیرعلامتی طویل
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 غیرعلامتی طویل طویل
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 غیرعلامتی طویل طویل
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 غیرعلامتی
#  endif
#اگر_ختم

خارجی z_const حرف * ثابت z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#تعریف_کرو ERR_پیغام(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#تعریف_کرو ERR_RETURN(strm,err) \
  واپس (strm->پیغام = ERR_پیغام(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#اگر MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be اندر 9..15
#اگر_ختم
#اگر_غیر_تعریف_شدہ DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#اگر_ختم
/* default windowBits for decompression. MAX_WBITS is for compression only */

#اگر MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ورنہ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#اگر_ختم
/* default memLevel */

#تعریف_کرو STORED_بلاک 0
#تعریف_کرو STATIC_TREES 1
#تعریف_کرو DYN_TREES    2
/* The three kinds of block type */

#تعریف_کرو MIN_مطابقت  3
#تعریف_کرو MAX_مطابقت  258
/* The minimum and maximum match lengths */

#تعریف_کرو PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#اگر defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_رمز  0x00
#  ifndef Z_SOLO
#    اگر defined(__TURBOC__) || defined(__BORLANDC__)
#      اگر (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         خالی _Cdecl farfree( خالی *بلاک );
         خالی *_Cdecl farmalloc( غیرعلامتی طویل nbytes );
#      ورنہ
#        include <alloc.h>
#      endif
#    ورنہ /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#اگر_ختم

#اگر_تعریف_شدہ AMIGA
#  define OS_رمز  1
#اگر_ختم

#اگر defined(VAXC) || defined(VMS)
#  define OS_رمز  2
#  define F_OPEN(name, منوال) \
     fopen((name), (منوال), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#اگر_ختم

#اگر_تعریف_شدہ __370__
#  اگر __TARGET_LIB__ < 0x20000000
#    define OS_رمز 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_رمز 11
#  ورنہ
#    define OS_رمز 8
#  endif
#اگر_ختم

#اگر defined(ATARI) || defined(atarist)
#  define OS_رمز  5
#اگر_ختم

#اگر_تعریف_شدہ OS2
#  define OS_رمز  6
#  اگر defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#اگر_ختم

#اگر defined(MACOS)
#  define OS_رمز  7
#اگر_ختم

#اگر defined(__acorn) || defined(__riscos)
#  define OS_رمز 13
#اگر_ختم

#اگر defined(WIN32) && !defined(__CYGWIN__)
#  define OS_رمز  10
#اگر_ختم

#اگر_تعریف_شدہ _BEOS_
#  define OS_رمز  16
#اگر_ختم

#اگر_تعریف_شدہ __TOS_OS400__
#  define OS_رمز 18
#اگر_ختم

#اگر_تعریف_شدہ __APPLE__
#  define OS_رمز 19
#اگر_ختم

#اگر defined(__BORLANDC__) && !defined(MSDOS)
  #ہدایت warn -8004
  #ہدایت warn -8008
  #ہدایت warn -8066
#اگر_ختم

/* provide prototypes for these when building zlib without LFS */
#اگر_غیر_تعریف_شدہ Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT چکری_جانچ_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT چکری_جانچ_32_combine_gen64(z_off64_t);
#اگر_ختم

        /* common defaults */

#اگر_غیر_تعریف_شدہ OS_رمز
#  define OS_رمز  3     /* assume Unix */
#اگر_ختم

#اگر_غیر_تعریف_شدہ F_OPEN
#  define F_OPEN(name, منوال) fopen((name), (منوال))
#اگر_ختم

         /* functions */

#اگر defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#اگر_ختم
#اگر defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#اگر_ختم
#اگر defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#اگر_ختم
#اگر_تعریف_شدہ HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  ورنہ
#    define zmemcpy حافظہ_نقل
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#ورنہ
   خالی ZLIB_اندرونی zmemcpy(خالی FAR *, ثابت خالی FAR *, z_سائز_t);
   صحیح ZLIB_اندرونی zmemcmp(ثابت خالی FAR *, ثابت خالی FAR *, z_سائز_t);
   خالی ZLIB_اندرونی zmemzero(خالی FAR *, z_سائز_t);
#اگر_ختم

/* Diagnostic functions */
#اگر_تعریف_شدہ ZLIB_DEBUG
#  include <stdio.h>
   خارجی صحیح ZLIB_اندرونی z_verbose;
   خارجی خالی ZLIB_اندرونی z_error(حرف *m);
#  define Assert(cond,پیغام) {اگر(!(cond)) z_error(پیغام);}
#  define Trace(x) {اگر (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {اگر (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {اگر (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {اگر (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {اگر (z_verbose>1 && (c)) fprintf x ;}
#ورنہ
#  define Assert(cond,پیغام)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#اگر_ختم

#اگر_غیر_تعریف_شدہ Z_SOLO
   voidpf ZLIB_اندرونی zcalloc(voidpf opaque, غیرعلامتی اشیاء_2,
                                غیرعلامتی سائز);
   خالی ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr);
#اگر_ختم

#تعریف_کرو ZALLOC(strm, اشیاء_2, سائز) \
           (*((strm)->zalloc))((strm)->opaque, (اشیاء_2), (سائز))
#تعریف_کرو ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#تعریف_کرو TRY_FREE(s, p) {اگر (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#تعریف_کرو ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#اگر_تعریف_شدہ Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#اگر defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#شامل <stdatomic.h>
قسمتعریف ساخت {
    atomic_flag begun;
    atomic_int مکمل_2;
} z_once_t;
#تعریف_کرو Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local خالی z_once(z_once_t *state, خالی (*init)(خالی)) {
    اگر (!atomic_بھرو(&state->مکمل_2)) {
        اگر (atomic_flag_test_and_set(&state->begun))
            جب (!atomic_بھرو(&state->مکمل_2))
                ;
        ورنہ {
            init();
            atomic_store(&state->مکمل_2, 1);
        }
    }
}

#ورنہ   /* no atomics */

#warning zlib not thread-محفوظ_2

قسمتعریف ساخت z_once_s {
    متلون صحیح begun;
    متلون صحیح مکمل_2;
} z_once_t;
#تعریف_کرو Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local صحیح test_and_سیٹ(صحیح متلون *flag) {
    صحیح was;

    was = *flag;
    *flag = 1;
    واپس was;
}

/* Run the provided init() function once. This is not thread-safe. */
local خالی z_once(z_once_t *state, خالی (*init)(خالی)) {
    اگر (!state->مکمل_2) {
        اگر (test_and_سیٹ(&state->begun))
            جب (!state->مکمل_2)
                ;
        ورنہ {
            init();
            state->مکمل_2 = 1;
        }
    }
}

#اگر_ختم /* ?atomics */

#اگر_ختم /* Z_ONCE */

#اگر_ختم /* ZUTIL_H */
