/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#অন্তর্ভাব "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const অক্ষর * ধ্রুবক z_errmsg[10] = {
    (z_const অক্ষর *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const অক্ষর *)"stream end",          /* Z_STREAM_END      1  */
    (z_const অক্ষর *)"",                    /* Z_OK              0  */
    (z_const অক্ষর *)"file error",          /* Z_ERRNO         (-1) */
    (z_const অক্ষর *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const অক্ষর *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const অক্ষর *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const অক্ষর *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const অক্ষর *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const অক্ষর *)""
};


ধ্রুবক অক্ষর * ZEXPORT zlibVersion(শূন্য) {
    ফেরত ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(শূন্য) {
    uLong flags;

    flags = 0;
    নির্বাচন ((পূর্ণসংখ্যা)(আকার(uInt))) {
    ক্ষেত্র 2:     বিরতি;
    ক্ষেত্র 4:     flags += 1;     বিরতি;
    ক্ষেত্র 8:     flags += 2;     বিরতি;
    পূর্বনির্ধারিত:    flags += 3;
    }
    নির্বাচন ((পূর্ণসংখ্যা)(আকার(uLong))) {
    ক্ষেত্র 2:     বিরতি;
    ক্ষেত্র 4:     flags += 1 << 2;        বিরতি;
    ক্ষেত্র 8:     flags += 2 << 2;        বিরতি;
    পূর্বনির্ধারিত:    flags += 3 << 2;
    }
    নির্বাচন ((পূর্ণসংখ্যা)(আকার(voidpf))) {
    ক্ষেত্র 2:     বিরতি;
    ক্ষেত্র 4:     flags += 1 << 4;        বিরতি;
    ক্ষেত্র 8:     flags += 2 << 4;        বিরতি;
    পূর্বনির্ধারিত:    flags += 3 << 4;
    }
    নির্বাচন ((পূর্ণসংখ্যা)(আকার(z_off_t))) {
    ক্ষেত্র 2:     বিরতি;
    ক্ষেত্র 4:     flags += 1 << 6;        বিরতি;
    ক্ষেত্র 8:     flags += 2 << 6;        বিরতি;
    পূর্বনির্ধারিত:    flags += 3 << 6;
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
    ফেরত flags;
}

#ifdef ZLIB_DEBUG
#অন্তর্ভাব <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
পূর্ণসংখ্যা ZLIB_INTERNAL z_verbose = verbose;

শূন্য ZLIB_INTERNAL z_error(অক্ষর *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ধ্রুবক অক্ষর * ZEXPORT zError(পূর্ণসংখ্যা err) {
    ফেরত ERR_বার্তা(err);
}

#যদি defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    পূর্ণসংখ্যা errno = 0;
#endif

#ifndef HAVE_MEMCPY

শূন্য ZLIB_INTERNAL zmemcpy(শূন্য FAR *dst, ধ্রুবক শূন্য FAR *src, z_আকার_t n) {
    uchf *p = dst;
    ধ্রুবক uchf *q = src;
    যখন (n) {
        *p++ = *q++;
        n--;
    }
}

পূর্ণসংখ্যা ZLIB_INTERNAL zmemcmp(ধ্রুবক শূন্য FAR *s1, ধ্রুবক শূন্য FAR *s2, z_আকার_t n) {
    ধ্রুবক uchf *p = s1, *q = s2;
    যখন (n) {
        যদি (*p++ != *q++)
            ফেরত (পূর্ণসংখ্যা)p[-1] - (পূর্ণসংখ্যা)q[-1];
        n--;
    }
    ফেরত 0;
}

শূন্য ZLIB_INTERNAL zmemzero(শূন্য FAR *b, z_আকার_t len) {
    uchf *p = b;
    যখন (len) {
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

local পূর্ণসংখ্যা পরবর্তী_ptr = 0;

প্রকারলেখ কাঠামো ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned বস্তুসমূহ_2, unsigned আকার_2) {
    voidpf buf;
    ulg bsize = (ulg)বস্তুসমূহ_2*আকার_2;

    (শূন্য)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    যদি (bsize < 65520L) {
        buf = farmalloc(bsize);
        যদি (*(ush*)&buf != 0) ফেরত buf;
    } অন্যথা {
        buf = farmalloc(bsize + 16L);
    }
    যদি (buf == ফাঁকা || পরবর্তী_ptr >= MAX_PTR) ফেরত ফাঁকা;
    table[পরবর্তী_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[পরবর্তী_ptr++].new_ptr = buf;
    ফেরত buf;
}

শূন্য ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    পূর্ণসংখ্যা n;

    (শূন্য)opaque;

    যদি (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        ফেরত;
    }
    /* Find the original pointer */
    জন্য (n = 0; n < পরবর্তী_ptr; n++) {
        যদি (ptr != table[n].new_ptr) চালু;

        farfree(table[n].org_ptr);
        যখন (++n < পরবর্তী_ptr) {
            table[n-1] = table[n];
        }
        পরবর্তী_ptr--;
        ফেরত;
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt বস্তুসমূহ_2, uInt আকার_2) {
    (শূন্য)opaque;
    ফেরত _halloc((long)বস্তুসমূহ_2, আকার_2);
}

শূন্য ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (শূন্য)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
বাহ্যিক voidp malloc(uInt আকার_2);
বাহ্যিক voidp calloc(uInt বস্তুসমূহ_2, uInt আকার_2);
বাহ্যিক শূন্য মুক্ত(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned বস্তুসমূহ_2, unsigned আকার_2) {
    (শূন্য)opaque;
    ফেরত আকার(uInt) > 2 ? (voidpf)malloc(বস্তুসমূহ_2 * আকার_2) :
                              (voidpf)calloc(বস্তুসমূহ_2, আকার_2);
}

শূন্য ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (শূন্য)opaque;
    মুক্ত(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
