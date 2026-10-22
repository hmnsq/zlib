/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ضمن "zutil.h"
#جيڪڏهن_وصف_نه_ٿيل Z_SOLO
#  include "gzguts.h"
#جيڪڏهن_پورو

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


ثابت حرف * ZEXPORT zlib_نسخو(فراغ) {
    أرجع ZLIB_نسخو;
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
#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
    flags += 1 << 8;
#جيڪڏهن_پورو
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#جيڪڏهن_وصف_ٿيل ZLIB_WINAPI
    flags += 1 << 10;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل BUILDFIXED
    flags += 1 << 12;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل DYNAMIC_چڪري_جاچ_جدول
    flags += 1 << 13;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل NO_GZCOMPRESS
    flags += 1L << 16;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل NO_GZIP
    flags += 1L << 17;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل FASTEST
    flags += 1L << 21;
#جيڪڏهن_پورو
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
#جيڪڏهن_پورو
    أرجع flags;
}

#جيڪڏهن_وصف_ٿيل ZLIB_DEBUG
#ضمن <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
صحيح ZLIB_اندروني z_verbose = verbose;

فراغ ZLIB_اندروني z_error(حرف *m) {
    fprintf(stderr, "%s\n", m);
    ٻاهر_نڪرو(1);
}
#جيڪڏهن_پورو

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ثابت حرف * ZEXPORT zError(صحيح err) {
    أرجع ERR_نياپو(err);
}

#إذا defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    صحيح errno = 0;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل HAVE_MEMCPY

فراغ ZLIB_اندروني zmemcpy(فراغ FAR *dst, ثابت فراغ FAR *src, z_سائيز_t n) {
    uchf *p = dst;
    ثابت uchf *q = src;
    طالما (n) {
        *p++ = *q++;
        n--;
    }
}

صحيح ZLIB_اندروني zmemcmp(ثابت فراغ FAR *s1, ثابت فراغ FAR *s2, z_سائيز_t n) {
    ثابت uchf *p = s1, *q = s2;
    طالما (n) {
        إذا (*p++ != *q++)
            أرجع (صحيح)p[-1] - (صحيح)q[-1];
        n--;
    }
    أرجع 0;
}

فراغ ZLIB_اندروني zmemzero(فراغ FAR *b, z_سائيز_t len) {
    uchf *p = b;
    طالما (len) {
        *p++ = 0;
        len--;
    }
}

#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل Z_SOLO

#جيڪڏهن_وصف_ٿيل SYS16_ٻٽو_انگ

#جيڪڏهن_وصف_ٿيل __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#وصف_ڪريو MAX_PTR 10
/* 10*64K = 640K */

local صحيح ايندڙ_ptr = 0;

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

voidpf ZLIB_اندروني zcalloc(voidpf opaque, بنانشان شيون_2, بنانشان سائيز) {
    voidpf buf;
    ulg bsize = (ulg)شيون_2*سائيز;

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
    إذا (buf == عدم || ايندڙ_ptr >= MAX_PTR) أرجع عدم;
    جدول_2[ايندڙ_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    جدول_2[ايندڙ_ptr++].new_ptr = buf;
    أرجع buf;
}

فراغ ZLIB_اندروني zcfree(voidpf opaque, voidpf ptr) {
    صحيح n;

    (فراغ)opaque;

    إذا (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        أرجع;
    }
    /* Find the original pointer */
    لكل (n = 0; n < ايندڙ_ptr; n++) {
        إذا (ptr != جدول_2[n].new_ptr) استمر;

        farfree(جدول_2[n].org_ptr);
        طالما (++n < ايندڙ_ptr) {
            جدول_2[n-1] = جدول_2[n];
        }
        ايندڙ_ptr--;
        أرجع;
    }
    Assert(0, "zcfree: ptr not found");
}

#جيڪڏهن_پورو /* __TURBOC__ */


#جيڪڏهن_وصف_ٿيل M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#إذا (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#جيڪڏهن_پورو

voidpf ZLIB_اندروني zcalloc(voidpf opaque, uInt شيون_2, uInt سائيز) {
    (فراغ)opaque;
    أرجع _halloc((ڊگهو)شيون_2, سائيز);
}

فراغ ZLIB_اندروني zcfree(voidpf opaque, voidpf ptr) {
    (فراغ)opaque;
    _hfree(ptr);
}

#جيڪڏهن_پورو /* M_I86 */

#جيڪڏهن_پورو /* SYS16BIT */


#جيڪڏهن_وصف_نه_ٿيل MY_ZCALLOC /* Any system without a special alloc function */

#جيڪڏهن_وصف_نه_ٿيل STDC
خارجي voidp يادگيري_ڏيو(uInt سائيز);
خارجي voidp صفر_يادگيري_ڏيو(uInt شيون_2, uInt سائيز);
خارجي فراغ حرر(voidpf ptr);
#جيڪڏهن_پورو

voidpf ZLIB_اندروني zcalloc(voidpf opaque, بنانشان شيون_2, بنانشان سائيز) {
    (فراغ)opaque;
    أرجع حجم(uInt) > 2 ? (voidpf)يادگيري_ڏيو(شيون_2 * سائيز) :
                              (voidpf)صفر_يادگيري_ڏيو(شيون_2, سائيز);
}

فراغ ZLIB_اندروني zcfree(voidpf opaque, voidpf ptr) {
    (فراغ)opaque;
    حرر(ptr);
}

#جيڪڏهن_پورو /* MY_ZCALLOC */

#جيڪڏهن_پورو /* !Z_SOLO */
