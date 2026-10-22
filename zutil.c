/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ଅନ୍ତର୍ଭାଵ "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const ଅକ୍ଷର * ଅଚଳ z_errmsg[10] = {
    (z_const ଅକ୍ଷର *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const ଅକ୍ଷର *)"stream end",          /* Z_STREAM_END      1  */
    (z_const ଅକ୍ଷର *)"",                    /* Z_OK              0  */
    (z_const ଅକ୍ଷର *)"file error",          /* Z_ERRNO         (-1) */
    (z_const ଅକ୍ଷର *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const ଅକ୍ଷର *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const ଅକ୍ଷର *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const ଅକ୍ଷର *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const ଅକ୍ଷର *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const ଅକ୍ଷର *)""
};


ଅଚଳ ଅକ୍ଷର * ZEXPORT zlibVersion(ଶୂନ୍ୟ) {
    ଫେରନ୍ତୁ ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(ଶୂନ୍ୟ) {
    uLong flags;

    flags = 0;
    ବାଛ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ଆକାର(uInt))) {
    ସ୍ଥିତି 2:     ଭାଙ୍ଗ;
    ସ୍ଥିତି 4:     flags += 1;     ଭାଙ୍ଗ;
    ସ୍ଥିତି 8:     flags += 2;     ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:    flags += 3;
    }
    ବାଛ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ଆକାର(uLong))) {
    ସ୍ଥିତି 2:     ଭାଙ୍ଗ;
    ସ୍ଥିତି 4:     flags += 1 << 2;        ଭାଙ୍ଗ;
    ସ୍ଥିତି 8:     flags += 2 << 2;        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:    flags += 3 << 2;
    }
    ବାଛ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ଆକାର(voidpf))) {
    ସ୍ଥିତି 2:     ଭାଙ୍ଗ;
    ସ୍ଥିତି 4:     flags += 1 << 4;        ଭାଙ୍ଗ;
    ସ୍ଥିତି 8:     flags += 2 << 4;        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:    flags += 3 << 4;
    }
    ବାଛ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ଆକାର(z_off_t))) {
    ସ୍ଥିତି 2:     ଭାଙ୍ଗ;
    ସ୍ଥିତି 4:     flags += 1 << 6;        ଭାଙ୍ଗ;
    ସ୍ଥିତି 8:     flags += 2 << 6;        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:    flags += 3 << 6;
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
#ଯଦି defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ଅନ୍ୟଥା
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   ଅନ୍ୟଥା
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#ଅନ୍ୟଥା
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ଅନ୍ୟଥା
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   ଅନ୍ୟଥା
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    ଫେରନ୍ତୁ flags;
}

#ifdef ZLIB_DEBUG
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZLIB_INTERNAL z_verbose = verbose;

ଶୂନ୍ୟ ZLIB_INTERNAL z_error(ଅକ୍ଷର *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ଅଚଳ ଅକ୍ଷର * ZEXPORT zError(ପୂର୍ଣ୍ଣସଂଖ୍ୟା err) {
    ଫେରନ୍ତୁ ERR_ସନ୍ଦେଶ(err);
}

#ଯଦି defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା errno = 0;
#endif

#ifndef HAVE_MEMCPY

ଶୂନ୍ୟ ZLIB_INTERNAL zmemcpy(ଶୂନ୍ୟ FAR *dst, ଅଚଳ ଶୂନ୍ୟ FAR *src, z_size_t n) {
    uchf *p = dst;
    ଅଚଳ uchf *q = src;
    ଯେତେବେଳେ (n) {
        *p++ = *q++;
        n--;
    }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZLIB_INTERNAL zmemcmp(ଅଚଳ ଶୂନ୍ୟ FAR *s1, ଅଚଳ ଶୂନ୍ୟ FAR *s2, z_size_t n) {
    ଅଚଳ uchf *p = s1, *q = s2;
    ଯେତେବେଳେ (n) {
        ଯଦି (*p++ != *q++)
            ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p[-1] - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)q[-1];
        n--;
    }
    ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ ZLIB_INTERNAL zmemzero(ଶୂନ୍ୟ FAR *b, z_size_t len) {
    uchf *p = b;
    ଯେତେବେଳେ (len) {
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

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା next_ptr = 0;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ବସ୍ତୁଗୁଡ଼ିକ_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)ବସ୍ତୁଗୁଡ଼ିକ_2*size;

    (ଶୂନ୍ୟ)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    ଯଦି (bsize < 65520L) {
        buf = farmalloc(bsize);
        ଯଦି (*(ush*)&buf != 0) ଫେରନ୍ତୁ buf;
    } ଅନ୍ୟଥା {
        buf = farmalloc(bsize + 16L);
    }
    ଯଦି (buf == ଖାଲି || next_ptr >= MAX_PTR) ଫେରନ୍ତୁ ଖାଲି;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    ଫେରନ୍ତୁ buf;
}

ଶୂନ୍ୟ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

    (ଶୂନ୍ୟ)opaque;

    ଯଦି (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        ଫେରନ୍ତୁ;
    }
    /* Find the original pointer */
    ପାଇଁ (n = 0; n < next_ptr; n++) {
        ଯଦି (ptr != table[n].new_ptr) ଜାରି;

        farfree(table[n].org_ptr);
        ଯେତେବେଳେ (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        ଫେରନ୍ତୁ;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#ଯଦି (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt ବସ୍ତୁଗୁଡ଼ିକ_2, uInt size) {
    (ଶୂନ୍ୟ)opaque;
    ଫେରନ୍ତୁ _halloc((long)ବସ୍ତୁଗୁଡ଼ିକ_2, size);
}

ଶୂନ୍ୟ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (ଶୂନ୍ୟ)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
ବାହ୍ୟ voidp malloc(uInt size);
ବାହ୍ୟ voidp calloc(uInt ବସ୍ତୁଗୁଡ଼ିକ_2, uInt size);
ବାହ୍ୟ ଶୂନ୍ୟ ମୁକ୍ତ(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ବସ୍ତୁଗୁଡ଼ିକ_2, unsigned size) {
    (ଶୂନ୍ୟ)opaque;
    ଫେରନ୍ତୁ ଆକାର(uInt) > 2 ? (voidpf)malloc(ବସ୍ତୁଗୁଡ଼ିକ_2 * size) :
                              (voidpf)calloc(ବସ୍ତୁଗୁଡ଼ିକ_2, size);
}

ଶୂନ୍ୟ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (ଶୂନ୍ୟ)opaque;
    ମୁକ୍ତ(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
