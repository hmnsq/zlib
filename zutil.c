/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#അന്തര്ഭാവ "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const അക്ഷരം * സ്ഥിരം z_errmsg[10] = {
    (z_const അക്ഷരം *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const അക്ഷരം *)"stream end",          /* Z_STREAM_END      1  */
    (z_const അക്ഷരം *)"",                    /* Z_OK              0  */
    (z_const അക്ഷരം *)"file error",          /* Z_ERRNO         (-1) */
    (z_const അക്ഷരം *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const അക്ഷരം *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const അക്ഷരം *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const അക്ഷരം *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const അക്ഷരം *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const അക്ഷരം *)""
};


സ്ഥിരം അക്ഷരം * ZEXPORT zlibVersion(ശൂന്യം) {
    തിരികെ ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(ശൂന്യം) {
    uLong flags;

    flags = 0;
    തിരഞ്ഞെടുപ്പ് ((പൂർണ്ണസംഖ്യ)(വലുപ്പം(uInt))) {
    അവസ്ഥ 2:     നിർത്തുക;
    അവസ്ഥ 4:     flags += 1;     നിർത്തുക;
    അവസ്ഥ 8:     flags += 2;     നിർത്തുക;
    സ്ഥിരസ്ഥിതി:    flags += 3;
    }
    തിരഞ്ഞെടുപ്പ് ((പൂർണ്ണസംഖ്യ)(വലുപ്പം(uLong))) {
    അവസ്ഥ 2:     നിർത്തുക;
    അവസ്ഥ 4:     flags += 1 << 2;        നിർത്തുക;
    അവസ്ഥ 8:     flags += 2 << 2;        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:    flags += 3 << 2;
    }
    തിരഞ്ഞെടുപ്പ് ((പൂർണ്ണസംഖ്യ)(വലുപ്പം(voidpf))) {
    അവസ്ഥ 2:     നിർത്തുക;
    അവസ്ഥ 4:     flags += 1 << 4;        നിർത്തുക;
    അവസ്ഥ 8:     flags += 2 << 4;        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:    flags += 3 << 4;
    }
    തിരഞ്ഞെടുപ്പ് ((പൂർണ്ണസംഖ്യ)(വലുപ്പം(z_off_t))) {
    അവസ്ഥ 2:     നിർത്തുക;
    അവസ്ഥ 4:     flags += 1 << 6;        നിർത്തുക;
    അവസ്ഥ 8:     flags += 2 << 6;        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:    flags += 3 << 6;
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
#എങ്കിൽ defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       അല്ലെങ്കിൽ
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   അല്ലെങ്കിൽ
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#അല്ലെങ്കിൽ
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       അല്ലെങ്കിൽ
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   അല്ലെങ്കിൽ
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    തിരികെ flags;
}

#ifdef ZLIB_DEBUG
#അന്തര്ഭാവ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
പൂർണ്ണസംഖ്യ ZLIB_INTERNAL z_verbose = verbose;

ശൂന്യം ZLIB_INTERNAL z_error(അക്ഷരം *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
സ്ഥിരം അക്ഷരം * ZEXPORT zError(പൂർണ്ണസംഖ്യ err) {
    തിരികെ ERR_സന്ദേശം(err);
}

#എങ്കിൽ defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    പൂർണ്ണസംഖ്യ errno = 0;
#endif

#ifndef HAVE_MEMCPY

ശൂന്യം ZLIB_INTERNAL zmemcpy(ശൂന്യം FAR *dst, സ്ഥിരം ശൂന്യം FAR *src, z_size_t n) {
    uchf *p = dst;
    സ്ഥിരം uchf *q = src;
    ആകുമ്പോൾ (n) {
        *p++ = *q++;
        n--;
    }
}

പൂർണ്ണസംഖ്യ ZLIB_INTERNAL zmemcmp(സ്ഥിരം ശൂന്യം FAR *s1, സ്ഥിരം ശൂന്യം FAR *s2, z_size_t n) {
    സ്ഥിരം uchf *p = s1, *q = s2;
    ആകുമ്പോൾ (n) {
        എങ്കിൽ (*p++ != *q++)
            തിരികെ (പൂർണ്ണസംഖ്യ)p[-1] - (പൂർണ്ണസംഖ്യ)q[-1];
        n--;
    }
    തിരികെ 0;
}

ശൂന്യം ZLIB_INTERNAL zmemzero(ശൂന്യം FAR *b, z_size_t len) {
    uchf *p = b;
    ആകുമ്പോൾ (len) {
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

local പൂർണ്ണസംഖ്യ next_ptr = 0;

തരനിർവചനം ഘടന ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ഇനങ്ങൾ_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)ഇനങ്ങൾ_2*size;

    (ശൂന്യം)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    എങ്കിൽ (bsize < 65520L) {
        buf = farmalloc(bsize);
        എങ്കിൽ (*(ush*)&buf != 0) തിരികെ buf;
    } അല്ലെങ്കിൽ {
        buf = farmalloc(bsize + 16L);
    }
    എങ്കിൽ (buf == ശൂന്യ || next_ptr >= MAX_PTR) തിരികെ ശൂന്യ;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    തിരികെ buf;
}

ശൂന്യം ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    പൂർണ്ണസംഖ്യ n;

    (ശൂന്യം)opaque;

    എങ്കിൽ (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        തിരികെ;
    }
    /* Find the original pointer */
    വേണ്ടി (n = 0; n < next_ptr; n++) {
        എങ്കിൽ (ptr != table[n].new_ptr) തുടരുക;

        farfree(table[n].org_ptr);
        ആകുമ്പോൾ (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        തിരികെ;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#എങ്കിൽ (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt ഇനങ്ങൾ_2, uInt size) {
    (ശൂന്യം)opaque;
    തിരികെ _halloc((long)ഇനങ്ങൾ_2, size);
}

ശൂന്യം ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (ശൂന്യം)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
ബാഹ്യ voidp malloc(uInt size);
ബാഹ്യ voidp calloc(uInt ഇനങ്ങൾ_2, uInt size);
ബാഹ്യ ശൂന്യം സ്വതന്ത്രം(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ഇനങ്ങൾ_2, unsigned size) {
    (ശൂന്യം)opaque;
    തിരികെ വലുപ്പം(uInt) > 2 ? (voidpf)malloc(ഇനങ്ങൾ_2 * size) :
                              (voidpf)calloc(ഇനങ്ങൾ_2, size);
}

ശൂന്യം ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (ശൂന്യം)opaque;
    സ്വതന്ത്രം(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
