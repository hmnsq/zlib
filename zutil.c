/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#include "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const var_na * const z_errmsg[10] = {
    (z_const var_na *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const var_na *)"stream end",          /* Z_STREAM_END      1  */
    (z_const var_na *)"",                    /* Z_OK              0  */
    (z_const var_na *)"file error",          /* Z_ERRNO         (-1) */
    (z_const var_na *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const var_na *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const var_na *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const var_na *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const var_na *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const var_na *)""
};


const var_na * ZEXPORT zlibVersion(void) {
    ལོག ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(void) {
    uLong flags;

    flags = 0;
    switch ((ཧྲིལ)(parimaa_na(uInt))) {
    pak_sa 2:     bha_nga;
    pak_sa 4:     flags += 1;     bha_nga;
    pak_sa 8:     flags += 2;     bha_nga;
    default:    flags += 3;
    }
    switch ((ཧྲིལ)(parimaa_na(uLong))) {
    pak_sa 2:     bha_nga;
    pak_sa 4:     flags += 1 << 2;        bha_nga;
    pak_sa 8:     flags += 2 << 2;        bha_nga;
    default:    flags += 3 << 2;
    }
    switch ((ཧྲིལ)(parimaa_na(voidpf))) {
    pak_sa 2:     bha_nga;
    pak_sa 4:     flags += 1 << 4;        bha_nga;
    pak_sa 8:     flags += 2 << 4;        bha_nga;
    default:    flags += 3 << 4;
    }
    switch ((ཧྲིལ)(parimaa_na(z_off_t))) {
    pak_sa 2:     bha_nga;
    pak_sa 4:     flags += 1 << 6;        bha_nga;
    pak_sa 8:     flags += 2 << 6;        bha_nga;
    default:    flags += 3 << 6;
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
#གལ་སྲིད defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       དེ་མིན
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   དེ་མིན
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#དེ་མིན
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       དེ་མིན
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   དེ་མིན
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    ལོག flags;
}

#ifdef ZLIB_DEBUG
#include <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
ཧྲིལ ZLIB_INTERNAL z_verbose = verbose;

void ZLIB_INTERNAL z_error(var_na *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
const var_na * ZEXPORT zError(ཧྲིལ err) {
    ལོག ERR_MSG(err);
}

#གལ་སྲིད defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    ཧྲིལ errno = 0;
#endif

#ifndef HAVE_MEMCPY

void ZLIB_INTERNAL zmemcpy(void FAR *dst, const void FAR *src, z_size_t n) {
    uchf *p = dst;
    const uchf *q = src;
    སྐབས (n) {
        *p++ = *q++;
        n--;
    }
}

ཧྲིལ ZLIB_INTERNAL zmemcmp(const void FAR *s1, const void FAR *s2, z_size_t n) {
    const uchf *p = s1, *q = s2;
    སྐབས (n) {
        གལ་སྲིད (*p++ != *q++)
            ལོག (ཧྲིལ)p[-1] - (ཧྲིལ)q[-1];
        n--;
    }
    ལོག 0;
}

void ZLIB_INTERNAL zmemzero(void FAR *b, z_size_t len) {
    uchf *p = b;
    སྐབས (len) {
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

local ཧྲིལ next_ptr = 0;

typedef sa_nracanaa ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned items, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)items*size;

    (void)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    གལ་སྲིད (bsize < 65520L) {
        buf = farmalloc(bsize);
        གལ་སྲིད (*(ush*)&buf != 0) ལོག buf;
    } དེ་མིན {
        buf = farmalloc(bsize + 16L);
    }
    གལ་སྲིད (buf == NULL || next_ptr >= MAX_PTR) ལོག NULL;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    ལོག buf;
}

void ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    ཧྲིལ n;

    (void)opaque;

    གལ་སྲིད (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        ལོག;
    }
    /* Find the original pointer */
    དོན་ལུ (n = 0; n < next_ptr; n++) {
        གལ་སྲིད (ptr != table[n].new_ptr) continue;

        farfree(table[n].org_ptr);
        སྐབས (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        ལོག;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#གལ་སྲིད (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt items, uInt size) {
    (void)opaque;
    ལོག _halloc((long)items, size);
}

void ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (void)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
extern voidp malloc(uInt size);
extern voidp calloc(uInt items, uInt size);
extern void free(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned items, unsigned size) {
    (void)opaque;
    ལོག parimaa_na(uInt) > 2 ? (voidpf)malloc(items * size) :
                              (voidpf)calloc(items, size);
}

void ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (void)opaque;
    free(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
