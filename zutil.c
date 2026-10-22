/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#અન્તર્ભાવ "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const અક્ષર * અચલ z_errmsg[10] = {
    (z_const અક્ષર *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const અક્ષર *)"stream end",          /* Z_STREAM_END      1  */
    (z_const અક્ષર *)"",                    /* Z_OK              0  */
    (z_const અક્ષર *)"file error",          /* Z_ERRNO         (-1) */
    (z_const અક્ષર *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const અક્ષર *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const અક્ષર *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const અક્ષર *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const અક્ષર *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const અક્ષર *)""
};


અચલ અક્ષર * ZEXPORT zlibVersion(શૂન્ય) {
    પાછા ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(શૂન્ય) {
    uLong flags;

    flags = 0;
    પસંદગી ((પૂર્ણાંક)(કદ(uInt))) {
    સ્થિતિ 2:     તોડો;
    સ્થિતિ 4:     flags += 1;     તોડો;
    સ્થિતિ 8:     flags += 2;     તોડો;
    મૂળભૂત:    flags += 3;
    }
    પસંદગી ((પૂર્ણાંક)(કદ(uLong))) {
    સ્થિતિ 2:     તોડો;
    સ્થિતિ 4:     flags += 1 << 2;        તોડો;
    સ્થિતિ 8:     flags += 2 << 2;        તોડો;
    મૂળભૂત:    flags += 3 << 2;
    }
    પસંદગી ((પૂર્ણાંક)(કદ(voidpf))) {
    સ્થિતિ 2:     તોડો;
    સ્થિતિ 4:     flags += 1 << 4;        તોડો;
    સ્થિતિ 8:     flags += 2 << 4;        તોડો;
    મૂળભૂત:    flags += 3 << 4;
    }
    પસંદગી ((પૂર્ણાંક)(કદ(z_off_t))) {
    સ્થિતિ 2:     તોડો;
    સ્થિતિ 4:     flags += 1 << 6;        તોડો;
    સ્થિતિ 8:     flags += 2 << 6;        તોડો;
    મૂળભૂત:    flags += 3 << 6;
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
#જો defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       નહીંતર
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   નહીંતર
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#નહીંતર
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       નહીંતર
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   નહીંતર
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    પાછા flags;
}

#ifdef ZLIB_DEBUG
#અન્તર્ભાવ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
પૂર્ણાંક ZLIB_INTERNAL z_verbose = verbose;

શૂન્ય ZLIB_INTERNAL z_error(અક્ષર *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
અચલ અક્ષર * ZEXPORT zError(પૂર્ણાંક err) {
    પાછા ERR_સંદેશ(err);
}

#જો defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    પૂર્ણાંક errno = 0;
#endif

#ifndef HAVE_MEMCPY

શૂન્ય ZLIB_INTERNAL zmemcpy(શૂન્ય FAR *dst, અચલ શૂન્ય FAR *src, z_size_t n) {
    uchf *p = dst;
    અચલ uchf *q = src;
    જ્યારે (n) {
        *p++ = *q++;
        n--;
    }
}

પૂર્ણાંક ZLIB_INTERNAL zmemcmp(અચલ શૂન્ય FAR *s1, અચલ શૂન્ય FAR *s2, z_size_t n) {
    અચલ uchf *p = s1, *q = s2;
    જ્યારે (n) {
        જો (*p++ != *q++)
            પાછા (પૂર્ણાંક)p[-1] - (પૂર્ણાંક)q[-1];
        n--;
    }
    પાછા 0;
}

શૂન્ય ZLIB_INTERNAL zmemzero(શૂન્ય FAR *b, z_size_t len) {
    uchf *p = b;
    જ્યારે (len) {
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

local પૂર્ણાંક next_ptr = 0;

પ્રકારવ્યાખ્યા સંરચના ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned વસ્તુઓ_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)વસ્તુઓ_2*size;

    (શૂન્ય)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    જો (bsize < 65520L) {
        buf = farmalloc(bsize);
        જો (*(ush*)&buf != 0) પાછા buf;
    } નહીંતર {
        buf = farmalloc(bsize + 16L);
    }
    જો (buf == ખાલી || next_ptr >= MAX_PTR) પાછા ખાલી;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    પાછા buf;
}

શૂન્ય ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    પૂર્ણાંક n;

    (શૂન્ય)opaque;

    જો (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        પાછા;
    }
    /* Find the original pointer */
    માટે (n = 0; n < next_ptr; n++) {
        જો (ptr != table[n].new_ptr) ચાલુ;

        farfree(table[n].org_ptr);
        જ્યારે (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        પાછા;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#જો (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt વસ્તુઓ_2, uInt size) {
    (શૂન્ય)opaque;
    પાછા _halloc((long)વસ્તુઓ_2, size);
}

શૂન્ય ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (શૂન્ય)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
બાહ્ય voidp malloc(uInt size);
બાહ્ય voidp calloc(uInt વસ્તુઓ_2, uInt size);
બાહ્ય શૂન્ય મુક્ત(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned વસ્તુઓ_2, unsigned size) {
    (શૂન્ય)opaque;
    પાછા કદ(uInt) > 2 ? (voidpf)malloc(વસ્તુઓ_2 * size) :
                              (voidpf)calloc(વસ્તુઓ_2, size);
}

શૂન્ય ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (શૂન્ય)opaque;
    મુક્ત(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
