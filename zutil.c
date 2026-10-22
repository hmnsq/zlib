/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ضمن "zutil.h"
#که_نه_تعریف_شوی Z_SOLO
#  include "gzguts.h"
#د_که_پای

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


ثابت حرف * ZEXPORT zlib_نسخه(فراغ) {
    أرجع ZLIB_نسخه;
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
#که_تعریف_شوی ZLIB_DEBUG
    flags += 1 << 8;
#د_که_پای
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#که_تعریف_شوی ZLIB_WINAPI
    flags += 1 << 10;
#د_که_پای
#که_تعریف_شوی BUILDFIXED
    flags += 1 << 12;
#د_که_پای
#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
    flags += 1 << 13;
#د_که_پای
#که_تعریف_شوی NO_GZCOMPRESS
    flags += 1L << 16;
#د_که_پای
#که_تعریف_شوی NO_GZIP
    flags += 1L << 17;
#د_که_پای
#که_تعریف_شوی PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#د_که_پای
#که_تعریف_شوی FASTEST
    flags += 1L << 21;
#د_که_پای
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
#د_که_پای
    أرجع flags;
}

#که_تعریف_شوی ZLIB_DEBUG
#ضمن <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
صحيح ZLIB_دننی z_verbose = verbose;

فراغ ZLIB_دننی z_error(حرف *m) {
    fprintf(stderr, "%s\n", m);
    وتل(1);
}
#د_که_پای

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
#د_که_پای

#که_نه_تعریف_شوی HAVE_MEMCPY

فراغ ZLIB_دننی zmemcpy(فراغ FAR *dst, ثابت فراغ FAR *src, z_اندازه_t n) {
    uchf *p = dst;
    ثابت uchf *q = src;
    طالما (n) {
        *p++ = *q++;
        n--;
    }
}

صحيح ZLIB_دننی zmemcmp(ثابت فراغ FAR *s1, ثابت فراغ FAR *s2, z_اندازه_t n) {
    ثابت uchf *p = s1, *q = s2;
    طالما (n) {
        إذا (*p++ != *q++)
            أرجع (صحيح)p[-1] - (صحيح)q[-1];
        n--;
    }
    أرجع 0;
}

فراغ ZLIB_دننی zmemzero(فراغ FAR *b, z_اندازه_t len) {
    uchf *p = b;
    طالما (len) {
        *p++ = 0;
        len--;
    }
}

#د_که_پای

#که_نه_تعریف_شوی Z_SOLO

#که_تعریف_شوی SYS16_بټ

#که_تعریف_شوی __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#تعریف_کړه MAX_PTR 10
/* 10*64K = 640K */

local صحيح بل_ptr = 0;

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

voidpf ZLIB_دننی zcalloc(voidpf opaque, بینښه توکي_2, بینښه اندازه) {
    voidpf buf;
    ulg bsize = (ulg)توکي_2*اندازه;

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
    إذا (buf == عدم || بل_ptr >= MAX_PTR) أرجع عدم;
    جدول_2[بل_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    جدول_2[بل_ptr++].new_ptr = buf;
    أرجع buf;
}

فراغ ZLIB_دننی zcfree(voidpf opaque, voidpf ptr) {
    صحيح n;

    (فراغ)opaque;

    إذا (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        أرجع;
    }
    /* Find the original pointer */
    لكل (n = 0; n < بل_ptr; n++) {
        إذا (ptr != جدول_2[n].new_ptr) استمر;

        farfree(جدول_2[n].org_ptr);
        طالما (++n < بل_ptr) {
            جدول_2[n-1] = جدول_2[n];
        }
        بل_ptr--;
        أرجع;
    }
    Assert(0, "zcfree: ptr not found");
}

#د_که_پای /* __TURBOC__ */


#که_تعریف_شوی M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#إذا (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#د_که_پای

voidpf ZLIB_دننی zcalloc(voidpf opaque, uInt توکي_2, uInt اندازه) {
    (فراغ)opaque;
    أرجع _halloc((اوږد)توکي_2, اندازه);
}

فراغ ZLIB_دننی zcfree(voidpf opaque, voidpf ptr) {
    (فراغ)opaque;
    _hfree(ptr);
}

#د_که_پای /* M_I86 */

#د_که_پای /* SYS16BIT */


#که_نه_تعریف_شوی MY_ZCALLOC /* Any system without a special alloc function */

#که_نه_تعریف_شوی STDC
خارجي voidp حافظه_ورکړه(uInt اندازه);
خارجي voidp حافظه_صفر_ورکړه(uInt توکي_2, uInt اندازه);
خارجي فراغ حرر(voidpf ptr);
#د_که_پای

voidpf ZLIB_دننی zcalloc(voidpf opaque, بینښه توکي_2, بینښه اندازه) {
    (فراغ)opaque;
    أرجع حجم(uInt) > 2 ? (voidpf)حافظه_ورکړه(توکي_2 * اندازه) :
                              (voidpf)حافظه_صفر_ورکړه(توکي_2, اندازه);
}

فراغ ZLIB_دننی zcfree(voidpf opaque, voidpf ptr) {
    (فراغ)opaque;
    حرر(ptr);
}

#د_که_پای /* MY_ZCALLOC */

#د_که_پای /* !Z_SOLO */
