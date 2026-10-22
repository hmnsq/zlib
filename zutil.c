/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#অন্তর্ভাৱ "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const ৱর্ণ * ধ্রুৱ z_errmsg[10] = {
    (z_const ৱর্ণ *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const ৱর্ণ *)"stream end",          /* Z_STREAM_END      1  */
    (z_const ৱর্ণ *)"",                    /* Z_OK              0  */
    (z_const ৱর্ণ *)"file error",          /* Z_ERRNO         (-1) */
    (z_const ৱর্ণ *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const ৱর্ণ *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const ৱর্ণ *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const ৱর্ণ *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const ৱর্ণ *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const ৱর্ণ *)""
};


ধ্রুৱ ৱর্ণ * ZEXPORT zlibVersion(শূন্য) {
    ঘূৰাই ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(শূন্য) {
    uLong flags;

    flags = 0;
    ৱিকল্প ((পূর্ণাঙ্ক)(পরিমাণ(uInt))) {
    পক্ষ 2:     ভঙ্গ;
    পক্ষ 4:     flags += 1;     ভঙ্গ;
    পক্ষ 8:     flags += 2;     ভঙ্গ;
    স্ৱভাৱ:    flags += 3;
    }
    ৱিকল্প ((পূর্ণাঙ্ক)(পরিমাণ(uLong))) {
    পক্ষ 2:     ভঙ্গ;
    পক্ষ 4:     flags += 1 << 2;        ভঙ্গ;
    পক্ষ 8:     flags += 2 << 2;        ভঙ্গ;
    স্ৱভাৱ:    flags += 3 << 2;
    }
    ৱিকল্প ((পূর্ণাঙ্ক)(পরিমাণ(voidpf))) {
    পক্ষ 2:     ভঙ্গ;
    পক্ষ 4:     flags += 1 << 4;        ভঙ্গ;
    পক্ষ 8:     flags += 2 << 4;        ভঙ্গ;
    স্ৱভাৱ:    flags += 3 << 4;
    }
    ৱিকল্প ((পূর্ণাঙ্ক)(পরিমাণ(z_off_t))) {
    পক্ষ 2:     ভঙ্গ;
    পক্ষ 4:     flags += 1 << 6;        ভঙ্গ;
    পক্ষ 8:     flags += 2 << 6;        ভঙ্গ;
    স্ৱভাৱ:    flags += 3 << 6;
    }
#ifdef ZLIB_DEBUG
    flags += 1 << 8;
#endif
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#ifdef ZLIB_WINAPI
    flags += 1 << 10;
#endif
#ifdef BUILDFIXED
    flags += 1 << 12;
#endif
#ifdef DYNAMIC_CRC_TABLE
    flags += 1 << 13;
#endif
#ifdef NO_GZCOMPRESS
    flags += 1L << 16;
#endif
#ifdef NO_GZIP
    flags += 1L << 17;
#endif
#ifdef PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#endif
#ifdef FASTEST
    flags += 1L << 21;
#endif
#যদি defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       অন্যথা
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   অন্যথা
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#অন্যথা
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       অন্যথা
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   অন্যথা
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    ঘূৰাই flags;
}

#ifdef ZLIB_DEBUG
#অন্তর্ভাৱ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
পূর্ণাঙ্ক ZLIB_INTERNAL z_verbose = verbose;

শূন্য ZLIB_INTERNAL z_error(ৱর্ণ *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ধ্রুৱ ৱর্ণ * ZEXPORT zError(পূর্ণাঙ্ক err) {
    ঘূৰাই ERR_বাৰ্তা(err);
}

#যদি defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    পূর্ণাঙ্ক errno = 0;
#endif

#ifndef HAVE_MEMCPY

শূন্য ZLIB_INTERNAL zmemcpy(শূন্য FAR *dst, ধ্রুৱ শূন্য FAR *src, z_size_t n) {
    uchf *p = dst;
    ধ্রুৱ uchf *q = src;
    যেতিয়ালৈ (n) {
        *p++ = *q++;
        n--;
    }
}

পূর্ণাঙ্ক ZLIB_INTERNAL zmemcmp(ধ্রুৱ শূন্য FAR *s1, ধ্রুৱ শূন্য FAR *s2, z_size_t n) {
    ধ্রুৱ uchf *p = s1, *q = s2;
    যেতিয়ালৈ (n) {
        যদি (*p++ != *q++)
            ঘূৰাই (পূর্ণাঙ্ক)p[-1] - (পূর্ণাঙ্ক)q[-1];
        n--;
    }
    ঘূৰাই 0;
}

শূন্য ZLIB_INTERNAL zmemzero(শূন্য FAR *b, z_size_t len) {
    uchf *p = b;
    যেতিয়ালৈ (len) {
        *p++ = 0;
        len--;
    }
}

#endif

#ifndef Z_SOLO

#ifdef SYS16BIT

#ifdef __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#define MAX_PTR 10
/* 10*64K = 640K */

local পূর্ণাঙ্ক next_ptr = 0;

প্রকারনির্দেশ সংরচনা ptr_table_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_table;

local ptr_table table[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned বস্তুসমূহ_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)বস্তুসমূহ_2*size;

    (শূন্য)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    যদি (bsize < 65520L) {
        buf = farmalloc(bsize);
        যদি (*(ush*)&buf != 0) ঘূৰাই buf;
    } অন্যথা {
        buf = farmalloc(bsize + 16L);
    }
    যদি (buf == রিক্ত || next_ptr >= MAX_PTR) ঘূৰাই রিক্ত;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    ঘূৰাই buf;
}

শূন্য ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    পূর্ণাঙ্ক n;

    (শূন্য)opaque;

    যদি (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        ঘূৰাই;
    }
    /* Find the original pointer */
    বাবে (n = 0; n < next_ptr; n++) {
        যদি (ptr != table[n].new_ptr) অনুৱর্ত;

        farfree(table[n].org_ptr);
        যেতিয়ালৈ (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        ঘূৰাই;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#যদি (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt বস্তুসমূহ_2, uInt size) {
    (শূন্য)opaque;
    ঘূৰাই _halloc((long)বস্তুসমূহ_2, size);
}

শূন্য ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (শূন্য)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
বাহ্য voidp malloc(uInt size);
বাহ্য voidp calloc(uInt বস্তুসমূহ_2, uInt size);
বাহ্য শূন্য মুক্ত(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned বস্তুসমূহ_2, unsigned size) {
    (শূন্য)opaque;
    ঘূৰাই পরিমাণ(uInt) > 2 ? (voidpf)malloc(বস্তুসমূহ_2 * size) :
                              (voidpf)calloc(বস্তুসমূহ_2, size);
}

শূন্য ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (শূন্য)opaque;
    মুক্ত(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
