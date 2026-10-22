/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ಅನ್ತರ್ಭಾವ "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const ವರ್ಣ * ಧ್ರುವ z_errmsg[10] = {
    (z_const ವರ್ಣ *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const ವರ್ಣ *)"stream end",          /* Z_STREAM_END      1  */
    (z_const ವರ್ಣ *)"",                    /* Z_OK              0  */
    (z_const ವರ್ಣ *)"file error",          /* Z_ERRNO         (-1) */
    (z_const ವರ್ಣ *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const ವರ್ಣ *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const ವರ್ಣ *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const ವರ್ಣ *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const ವರ್ಣ *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const ವರ್ಣ *)""
};


ಧ್ರುವ ವರ್ಣ * ZEXPORT zlibVersion(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(ಶೂನ್ಯ) {
    uLong flags;

    flags = 0;
    ವಿಕಲ್ಪ ((ಪೂರ್ಣಾಙ್ಕ)(ಪರಿಮಾಣ(uInt))) {
    ಪಕ್ಷ 2:     ಭಙ್ಗ;
    ಪಕ್ಷ 4:     flags += 1;     ಭಙ್ಗ;
    ಪಕ್ಷ 8:     flags += 2;     ಭಙ್ಗ;
    ಸ್ವಭಾವ:    flags += 3;
    }
    ವಿಕಲ್ಪ ((ಪೂರ್ಣಾಙ್ಕ)(ಪರಿಮಾಣ(uLong))) {
    ಪಕ್ಷ 2:     ಭಙ್ಗ;
    ಪಕ್ಷ 4:     flags += 1 << 2;        ಭಙ್ಗ;
    ಪಕ್ಷ 8:     flags += 2 << 2;        ಭಙ್ಗ;
    ಸ್ವಭಾವ:    flags += 3 << 2;
    }
    ವಿಕಲ್ಪ ((ಪೂರ್ಣಾಙ್ಕ)(ಪರಿಮಾಣ(voidpf))) {
    ಪಕ್ಷ 2:     ಭಙ್ಗ;
    ಪಕ್ಷ 4:     flags += 1 << 4;        ಭಙ್ಗ;
    ಪಕ್ಷ 8:     flags += 2 << 4;        ಭಙ್ಗ;
    ಸ್ವಭಾವ:    flags += 3 << 4;
    }
    ವಿಕಲ್ಪ ((ಪೂರ್ಣಾಙ್ಕ)(ಪರಿಮಾಣ(z_off_t))) {
    ಪಕ್ಷ 2:     ಭಙ್ಗ;
    ಪಕ್ಷ 4:     flags += 1 << 6;        ಭಙ್ಗ;
    ಪಕ್ಷ 8:     flags += 2 << 6;        ಭಙ್ಗ;
    ಸ್ವಭಾವ:    flags += 3 << 6;
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
#ಯದಿ defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ಅನ್ಯಥಾ
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   ಅನ್ಯಥಾ
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#ಅನ್ಯಥಾ
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ಅನ್ಯಥಾ
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   ಅನ್ಯಥಾ
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    ಪ್ರತ್ಯಾಗಮ flags;
}

#ifdef ZLIB_DEBUG
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
ಪೂರ್ಣಾಙ್ಕ ZLIB_INTERNAL z_verbose = verbose;

ಶೂನ್ಯ ZLIB_INTERNAL z_error(ವರ್ಣ *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ಧ್ರುವ ವರ್ಣ * ZEXPORT zError(ಪೂರ್ಣಾಙ್ಕ err) {
    ಪ್ರತ್ಯಾಗಮ ERR_ಸಂದೇಶ(err);
}

#ಯದಿ defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    ಪೂರ್ಣಾಙ್ಕ errno = 0;
#endif

#ifndef HAVE_MEMCPY

ಶೂನ್ಯ ZLIB_INTERNAL zmemcpy(ಶೂನ್ಯ FAR *dst, ಧ್ರುವ ಶೂನ್ಯ FAR *src, z_size_t n) {
    uchf *p = dst;
    ಧ್ರುವ uchf *q = src;
    ಯಾವತ್ (n) {
        *p++ = *q++;
        n--;
    }
}

ಪೂರ್ಣಾಙ್ಕ ZLIB_INTERNAL zmemcmp(ಧ್ರುವ ಶೂನ್ಯ FAR *s1, ಧ್ರುವ ಶೂನ್ಯ FAR *s2, z_size_t n) {
    ಧ್ರುವ uchf *p = s1, *q = s2;
    ಯಾವತ್ (n) {
        ಯದಿ (*p++ != *q++)
            ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)p[-1] - (ಪೂರ್ಣಾಙ್ಕ)q[-1];
        n--;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ ZLIB_INTERNAL zmemzero(ಶೂನ್ಯ FAR *b, z_size_t len) {
    uchf *p = b;
    ಯಾವತ್ (len) {
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

local ಪೂರ್ಣಾಙ್ಕ next_ptr = 0;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ವಸ್ತುಗಳು_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)ವಸ್ತುಗಳು_2*size;

    (ಶೂನ್ಯ)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    ಯದಿ (bsize < 65520L) {
        buf = farmalloc(bsize);
        ಯದಿ (*(ush*)&buf != 0) ಪ್ರತ್ಯಾಗಮ buf;
    } ಅನ್ಯಥಾ {
        buf = farmalloc(bsize + 16L);
    }
    ಯದಿ (buf == ರಿಕ್ತ || next_ptr >= MAX_PTR) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    ಪ್ರತ್ಯಾಗಮ buf;
}

ಶೂನ್ಯ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    ಪೂರ್ಣಾಙ್ಕ n;

    (ಶೂನ್ಯ)opaque;

    ಯದಿ (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        ಪ್ರತ್ಯಾಗಮ;
    }
    /* Find the original pointer */
    ಪ್ರತಿ (n = 0; n < next_ptr; n++) {
        ಯದಿ (ptr != table[n].new_ptr) ಅನುವರ್ತ;

        farfree(table[n].org_ptr);
        ಯಾವತ್ (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        ಪ್ರತ್ಯಾಗಮ;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#ಯದಿ (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt ವಸ್ತುಗಳು_2, uInt size) {
    (ಶೂನ್ಯ)opaque;
    ಪ್ರತ್ಯಾಗಮ _halloc((long)ವಸ್ತುಗಳು_2, size);
}

ಶೂನ್ಯ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (ಶೂನ್ಯ)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
ಬಾಹ್ಯ voidp malloc(uInt size);
ಬಾಹ್ಯ voidp calloc(uInt ವಸ್ತುಗಳು_2, uInt size);
ಬಾಹ್ಯ ಶೂನ್ಯ ಮುಕ್ತ(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned ವಸ್ತುಗಳು_2, unsigned size) {
    (ಶೂನ್ಯ)opaque;
    ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(uInt) > 2 ? (voidpf)malloc(ವಸ್ತುಗಳು_2 * size) :
                              (voidpf)calloc(ವಸ್ತುಗಳು_2, size);
}

ಶೂನ್ಯ ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (ಶೂನ್ಯ)opaque;
    ಮುಕ್ತ(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
