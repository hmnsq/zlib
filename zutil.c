/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#شامل "zutil.h"
#اگر_غیر_تعریف_شدہ Z_SOLO
#  include "gzguts.h"
#اگر_ختم

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


ثابت حرف * ZEXPORT zlib_نسخہ(خالی) {
    واپس ZLIB_نسخہ;
}

uLong ZEXPORT zlibCompileFlags(خالی) {
    uLong flags;

    flags = 0;
    انتخاب ((صحیح)(حجم(uInt))) {
    حالت 2:     توڑو;
    حالت 4:     flags += 1;     توڑو;
    حالت 8:     flags += 2;     توڑو;
    طےشدہ:    flags += 3;
    }
    انتخاب ((صحیح)(حجم(uLong))) {
    حالت 2:     توڑو;
    حالت 4:     flags += 1 << 2;        توڑو;
    حالت 8:     flags += 2 << 2;        توڑو;
    طےشدہ:    flags += 3 << 2;
    }
    انتخاب ((صحیح)(حجم(voidpf))) {
    حالت 2:     توڑو;
    حالت 4:     flags += 1 << 4;        توڑو;
    حالت 8:     flags += 2 << 4;        توڑو;
    طےشدہ:    flags += 3 << 4;
    }
    انتخاب ((صحیح)(حجم(z_off_t))) {
    حالت 2:     توڑو;
    حالت 4:     flags += 1 << 6;        توڑو;
    حالت 8:     flags += 2 << 6;        توڑو;
    طےشدہ:    flags += 3 << 6;
    }
#اگر_تعریف_شدہ ZLIB_DEBUG
    flags += 1 << 8;
#اگر_ختم
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#اگر_تعریف_شدہ ZLIB_WINAPI
    flags += 1 << 10;
#اگر_ختم
#اگر_تعریف_شدہ BUILDFIXED
    flags += 1 << 12;
#اگر_ختم
#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
    flags += 1 << 13;
#اگر_ختم
#اگر_تعریف_شدہ NO_GZCOMPRESS
    flags += 1L << 16;
#اگر_ختم
#اگر_تعریف_شدہ NO_GZIP
    flags += 1L << 17;
#اگر_ختم
#اگر_تعریف_شدہ PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#اگر_ختم
#اگر_تعریف_شدہ FASTEST
    flags += 1L << 21;
#اگر_ختم
#اگر defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ورنہ
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   ورنہ
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#ورنہ
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ورنہ
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   ورنہ
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#اگر_ختم
    واپس flags;
}

#اگر_تعریف_شدہ ZLIB_DEBUG
#شامل <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
صحیح ZLIB_اندرونی z_verbose = verbose;

خالی ZLIB_اندرونی z_error(حرف *m) {
    fprintf(stderr, "%s\n", m);
    باہر_نکلو(1);
}
#اگر_ختم

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ثابت حرف * ZEXPORT zError(صحیح err) {
    واپس ERR_پیغام(err);
}

#اگر defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    صحیح errno = 0;
#اگر_ختم

#اگر_غیر_تعریف_شدہ HAVE_MEMCPY

خالی ZLIB_اندرونی zmemcpy(خالی FAR *dst, ثابت خالی FAR *src, z_سائز_t n) {
    uchf *p = dst;
    ثابت uchf *q = src;
    جب (n) {
        *p++ = *q++;
        n--;
    }
}

صحیح ZLIB_اندرونی zmemcmp(ثابت خالی FAR *s1, ثابت خالی FAR *s2, z_سائز_t n) {
    ثابت uchf *p = s1, *q = s2;
    جب (n) {
        اگر (*p++ != *q++)
            واپس (صحیح)p[-1] - (صحیح)q[-1];
        n--;
    }
    واپس 0;
}

خالی ZLIB_اندرونی zmemzero(خالی FAR *b, z_سائز_t len) {
    uchf *p = b;
    جب (len) {
        *p++ = 0;
        len--;
    }
}

#اگر_ختم

#اگر_غیر_تعریف_شدہ Z_SOLO

#اگر_تعریف_شدہ SYS16_ثنائی_ہندسہ

#اگر_تعریف_شدہ __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#تعریف_کرو MAX_PTR 10
/* 10*64K = 640K */

local صحیح اگلا_ptr = 0;

قسمتعریف ساخت ptr_جدول_s {
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

voidpf ZLIB_اندرونی zcalloc(voidpf opaque, غیرعلامتی اشیاء_2, غیرعلامتی سائز) {
    voidpf buf;
    ulg bsize = (ulg)اشیاء_2*سائز;

    (خالی)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    اگر (bsize < 65520L) {
        buf = farmalloc(bsize);
        اگر (*(ush*)&buf != 0) واپس buf;
    } ورنہ {
        buf = farmalloc(bsize + 16L);
    }
    اگر (buf == تہی || اگلا_ptr >= MAX_PTR) واپس تہی;
    جدول_2[اگلا_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    جدول_2[اگلا_ptr++].new_ptr = buf;
    واپس buf;
}

خالی ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr) {
    صحیح n;

    (خالی)opaque;

    اگر (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        واپس;
    }
    /* Find the original pointer */
    برائے (n = 0; n < اگلا_ptr; n++) {
        اگر (ptr != جدول_2[n].new_ptr) جاری;

        farfree(جدول_2[n].org_ptr);
        جب (++n < اگلا_ptr) {
            جدول_2[n-1] = جدول_2[n];
        }
        اگلا_ptr--;
        واپس;
    }
    Assert(0, "zcfree: ptr not found");
}

#اگر_ختم /* __TURBOC__ */


#اگر_تعریف_شدہ M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#اگر (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#اگر_ختم

voidpf ZLIB_اندرونی zcalloc(voidpf opaque, uInt اشیاء_2, uInt سائز) {
    (خالی)opaque;
    واپس _halloc((طویل)اشیاء_2, سائز);
}

خالی ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr) {
    (خالی)opaque;
    _hfree(ptr);
}

#اگر_ختم /* M_I86 */

#اگر_ختم /* SYS16BIT */


#اگر_غیر_تعریف_شدہ MY_ZCALLOC /* Any system without a special alloc function */

#اگر_غیر_تعریف_شدہ STDC
خارجی voidp حافظہ_دو(uInt سائز);
خارجی voidp صفر_حافظہ_دو(uInt اشیاء_2, uInt سائز);
خارجی خالی آزاد(voidpf ptr);
#اگر_ختم

voidpf ZLIB_اندرونی zcalloc(voidpf opaque, غیرعلامتی اشیاء_2, غیرعلامتی سائز) {
    (خالی)opaque;
    واپس حجم(uInt) > 2 ? (voidpf)حافظہ_دو(اشیاء_2 * سائز) :
                              (voidpf)صفر_حافظہ_دو(اشیاء_2, سائز);
}

خالی ZLIB_اندرونی zcfree(voidpf opaque, voidpf ptr) {
    (خالی)opaque;
    آزاد(ptr);
}

#اگر_ختم /* MY_ZCALLOC */

#اگر_ختم /* !Z_SOLO */
