/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ضمن "zutil.h"
#جے_بنا_تعریف Z_SOLO
#  include "gzguts.h"
#جے_انت

z_const حرف * ثابت z_errmsg[10] = {
    (z_const حرف *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const حرف *)"stream end",          /* Z_STREAM_END      1  */
    (z_const حرف *)"",                    /* Z_OK              0  */
    (z_const حرف *)"file error",          /* Z_ERRNO         (-1) */
    (z_const حرف *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const حرف *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const حرف *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const حرف *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const حرف *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const حرف *)""
};


ثابت حرف * ZEXPORT zlib_نسخہ(فراغ) {
    أرجع ZLIB_نسخہ;
}

uLong ZEXPORT zlibCompileFlags(فراغ) {
    uLong flags;

    flags = 0;
    بدل ((صحيح)(حجم(uInt))) {
    حالة 2:     اكسر;
    حالة 4:     flags += 1;     اكسر;
    حالة 8:     flags += 2;     اكسر;
    افتراضي:    flags += 3;
    }
    بدل ((صحيح)(حجم(uLong))) {
    حالة 2:     اكسر;
    حالة 4:     flags += 1 << 2;        اكسر;
    حالة 8:     flags += 2 << 2;        اكسر;
    افتراضي:    flags += 3 << 2;
    }
    بدل ((صحيح)(حجم(voidpf))) {
    حالة 2:     اكسر;
    حالة 4:     flags += 1 << 4;        اكسر;
    حالة 8:     flags += 2 << 4;        اكسر;
    افتراضي:    flags += 3 << 4;
    }
    بدل ((صحيح)(حجم(z_off_t))) {
    حالة 2:     اكسر;
    حالة 4:     flags += 1 << 6;        اكسر;
    حالة 8:     flags += 2 << 6;        اكسر;
    افتراضي:    flags += 3 << 6;
    }
#جے_تعریف_شدہ ZLIB_DEBUG
    flags += 1 << 8;
#جے_انت
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#جے_تعریف_شدہ ZLIB_WINAPI
    flags += 1 << 10;
#جے_انت
#جے_تعریف_شدہ BUILDFIXED
    flags += 1 << 12;
#جے_انت
#جے_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
    flags += 1 << 13;
#جے_انت
#جے_تعریف_شدہ NO_GZCOMPRESS
    flags += 1L << 16;
#جے_انت
#جے_تعریف_شدہ NO_GZIP
    flags += 1L << 17;
#جے_انت
#جے_تعریف_شدہ PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#جے_انت
#جے_تعریف_شدہ FASTEST
    flags += 1L << 21;
#جے_انت
#إذا defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       وإلا
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   وإلا
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#وإلا
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       وإلا
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   وإلا
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#جے_انت
    أرجع flags;
}

#جے_تعریف_شدہ ZLIB_DEBUG
#ضمن <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
صحيح ZLIB_اندرونی z_verbose = verbose;

فراغ ZLIB_اندرونی z_error(حرف *m) {
    fprintf(stderr, "%s\n", m);
    باہر_نکلو(1);
}
#جے_انت

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ثابت حرف * ZEXPORT zError(صحيح err) {
    أرجع ERR_پیغام(err);
}

#إذا defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    صحيح errno = 0;
#جے_انت

#جے_بنا_تعریف HAVE_MEMCPY

فراغ ZLIB_اندرونی zmemcpy(فراغ FAR *dst, ثابت فراغ FAR *src, z_ناپ_t n) {
    uchf *p = dst;
    ثابت uchf *q = src;
    طالما (n) {
        *p++ = *q++;
        n--;
    }
}

صحيح ZLIB_اندرونی zmemcmp(ثابت فراغ FAR *s1, ثابت فراغ FAR *s2, z_ناپ_t n) {
    ثابت uchf *p = s1, *q = s2;
    طالما (n) {
        إذا (*p++ != *q++)
            أرجع (صحيح)p[-1] - (صحيح)q[-1];
        n--;
    }
    أرجع 0;
}

فراغ ZLIB_اندرونی zmemzero(فراغ FAR *b, z_ناپ_t len) {
    uchf *p = b;
    طالما (len) {
        *p++ = 0;
        len--;
    }
}

#جے_انت

#جے_بنا_تعریف Z_SOLO

#جے_تعریف_شدہ SYS16_ثنائی_ہندسہ

#جے_تعریف_شدہ __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#تعریف_کرو MAX_PTR 10
/* 10*64K = 640K */

local صحيح اگلا_ptr = 0;

تعريفنوع بنية ptr_جدول_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_جدول;

local ptr_جدول جدول_2[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */

voidpf ZLIB_اندرونی zcalloc(voidpf opaque, بنانشانی اشیاء_2, بنانشانی ناپ) {
    voidpf buf;
    ulg bsize = (ulg)اشیاء_2*ناپ;

    (فراغ)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    إذا (bsize < 65520L) {
        buf = farmalloc(bsize);
        إذا (*(ush*)&buf != 0) أرجع buf;
    } وإلا {
        buf = farmalloc(bsize + 16L);
    }
    إذا (buf == عدم || اگلا_ptr >= MAX_PTR) أرجع عدم;
    جدول_2[اگلا_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    جدول_2[اگلا_ptr++].new_ptr = buf;
    أرجع buf;
}

فراغ ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr) {
    صحيح n;

    (فراغ)opaque;

    إذا (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        أرجع;
    }
    /* Find the original pointer */
    لكل (n = 0; n < اگلا_ptr; n++) {
        إذا (ptr != جدول_2[n].new_ptr) استمر;

        farfree(جدول_2[n].org_ptr);
        طالما (++n < اگلا_ptr) {
            جدول_2[n-1] = جدول_2[n];
        }
        اگلا_ptr--;
        أرجع;
    }
    Assert(0, "zcfree: ptr not found");
}

#جے_انت /* __TURBOC__ */


#جے_تعریف_شدہ M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#إذا (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#جے_انت

voidpf ZLIB_اندرونی zcalloc(voidpf opaque, uInt اشیاء_2, uInt ناپ) {
    (فراغ)opaque;
    أرجع _halloc((لما)اشیاء_2, ناپ);
}

فراغ ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr) {
    (فراغ)opaque;
    _hfree(ptr);
}

#جے_انت /* M_I86 */

#جے_انت /* SYS16BIT */


#جے_بنا_تعریف MY_ZCALLOC /* Any system without a special alloc function */

#جے_بنا_تعریف STDC
خارجي voidp حافظہ_دیو(uInt ناپ);
خارجي voidp صفر_حافظہ_دیو(uInt اشیاء_2, uInt ناپ);
خارجي فراغ حرر(voidpf ptr);
#جے_انت

voidpf ZLIB_اندرونی zcalloc(voidpf opaque, بنانشانی اشیاء_2, بنانشانی ناپ) {
    (فراغ)opaque;
    أرجع حجم(uInt) > 2 ? (voidpf)حافظہ_دیو(اشیاء_2 * ناپ) :
                              (voidpf)صفر_حافظہ_دیو(اشیاء_2, ناپ);
}

فراغ ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr) {
    (فراغ)opaque;
    حرر(ptr);
}

#جے_انت /* MY_ZCALLOC */

#جے_انت /* !Z_SOLO */
