/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_SOLO
#  include "gzguts.h"
#ਜੇ_ਅੰਤ

z_const ਅੱਖਰ * ਅਚਲ z_errmsg[10] = {
    (z_const ਅੱਖਰ *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const ਅੱਖਰ *)"stream end",          /* Z_STREAM_END      1  */
    (z_const ਅੱਖਰ *)"",                    /* Z_OK              0  */
    (z_const ਅੱਖਰ *)"file error",          /* Z_ERRNO         (-1) */
    (z_const ਅੱਖਰ *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const ਅੱਖਰ *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const ਅੱਖਰ *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const ਅੱਖਰ *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const ਅੱਖਰ *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const ਅੱਖਰ *)""
};


ਅਚਲ ਅੱਖਰ * ZEXPORT zlib_ਸੰਸਕਰਣ(ਖਾਲੀ) {
    ਵਾਪਸ ZLIB_ਸੰਸਕਰਣ;
}

uLong ZEXPORT zlibCompileFlags(ਖਾਲੀ) {
    uLong flags;

    flags = 0;
    ਚੋਣ ((ਪੂਰਨ)(ਆਕਾਰ(uInt))) {
    ਹਾਲਤ 2:     ਰੋਕੋ;
    ਹਾਲਤ 4:     flags += 1;     ਰੋਕੋ;
    ਹਾਲਤ 8:     flags += 2;     ਰੋਕੋ;
    ਮੂਲ:    flags += 3;
    }
    ਚੋਣ ((ਪੂਰਨ)(ਆਕਾਰ(uLong))) {
    ਹਾਲਤ 2:     ਰੋਕੋ;
    ਹਾਲਤ 4:     flags += 1 << 2;        ਰੋਕੋ;
    ਹਾਲਤ 8:     flags += 2 << 2;        ਰੋਕੋ;
    ਮੂਲ:    flags += 3 << 2;
    }
    ਚੋਣ ((ਪੂਰਨ)(ਆਕਾਰ(voidpf))) {
    ਹਾਲਤ 2:     ਰੋਕੋ;
    ਹਾਲਤ 4:     flags += 1 << 4;        ਰੋਕੋ;
    ਹਾਲਤ 8:     flags += 2 << 4;        ਰੋਕੋ;
    ਮੂਲ:    flags += 3 << 4;
    }
    ਚੋਣ ((ਪੂਰਨ)(ਆਕਾਰ(z_off_t))) {
    ਹਾਲਤ 2:     ਰੋਕੋ;
    ਹਾਲਤ 4:     flags += 1 << 6;        ਰੋਕੋ;
    ਹਾਲਤ 8:     flags += 2 << 6;        ਰੋਕੋ;
    ਮੂਲ:    flags += 3 << 6;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
    flags += 1 << 8;
#ਜੇ_ਅੰਤ
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_WINAPI
    flags += 1 << 10;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BUILDFIXED
    flags += 1 << 12;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
    flags += 1 << 13;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_GZCOMPRESS
    flags += 1L << 16;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_GZIP
    flags += 1L << 17;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FASTEST
    flags += 1L << 21;
#ਜੇ_ਅੰਤ
#ਜੇ defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ਨਹੀਂਤਾਂ
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   ਨਹੀਂਤਾਂ
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#ਨਹੀਂਤਾਂ
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ਨਹੀਂਤਾਂ
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   ਨਹੀਂਤਾਂ
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#ਜੇ_ਅੰਤ
    ਵਾਪਸ flags;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DEBUG
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
ਪੂਰਨ ZLIB_ਅੰਦਰੂਨੀ z_verbose = verbose;

ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ z_error(ਅੱਖਰ *m) {
    fprintf(stderr, "%s\n", m);
    ਬਾਹਰ_ਨਿਕਲੋ(1);
}
#ਜੇ_ਅੰਤ

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ਅਚਲ ਅੱਖਰ * ZEXPORT zError(ਪੂਰਨ err) {
    ਵਾਪਸ ERR_ਸੁਨੇਹਾ(err);
}

#ਜੇ defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    ਪੂਰਨ errno = 0;
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_MEMCPY

ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zmemcpy(ਖਾਲੀ FAR *dst, ਅਚਲ ਖਾਲੀ FAR *src, z_ਮਾਪ_t n) {
    uchf *p = dst;
    ਅਚਲ uchf *q = src;
    ਜਦੋਂ (n) {
        *p++ = *q++;
        n--;
    }
}

ਪੂਰਨ ZLIB_ਅੰਦਰੂਨੀ zmemcmp(ਅਚਲ ਖਾਲੀ FAR *s1, ਅਚਲ ਖਾਲੀ FAR *s2, z_ਮਾਪ_t n) {
    ਅਚਲ uchf *p = s1, *q = s2;
    ਜਦੋਂ (n) {
        ਜੇ (*p++ != *q++)
            ਵਾਪਸ (ਪੂਰਨ)p[-1] - (ਪੂਰਨ)q[-1];
        n--;
    }
    ਵਾਪਸ 0;
}

ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zmemzero(ਖਾਲੀ FAR *b, z_ਮਾਪ_t len) {
    uchf *p = b;
    ਜਦੋਂ (len) {
        *p++ = 0;
        len--;
    }
}

#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_SOLO

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SYS16_ਦੋਆਧਾਰੀ_ਅੰਕ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_PTR 10
/* 10*64K = 640K */

local ਪੂਰਨ ਅਗਲਾ_ptr = 0;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ptr_ਸਾਰਣੀ_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_ਸਾਰਣੀ;

local ptr_ਸਾਰਣੀ ਸਾਰਣੀ_2[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */

voidpf ZLIB_ਅੰਦਰੂਨੀ zcalloc(voidpf opaque, ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੀਜ਼ਾਂ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ) {
    voidpf buf;
    ulg bsize = (ulg)ਚੀਜ਼ਾਂ_2*ਮਾਪ;

    (ਖਾਲੀ)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    ਜੇ (bsize < 65520L) {
        buf = farmalloc(bsize);
        ਜੇ (*(ush*)&buf != 0) ਵਾਪਸ buf;
    } ਨਹੀਂਤਾਂ {
        buf = farmalloc(bsize + 16L);
    }
    ਜੇ (buf == ਨਿਰਾ || ਅਗਲਾ_ptr >= MAX_PTR) ਵਾਪਸ ਨਿਰਾ;
    ਸਾਰਣੀ_2[ਅਗਲਾ_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    ਸਾਰਣੀ_2[ਅਗਲਾ_ptr++].new_ptr = buf;
    ਵਾਪਸ buf;
}

ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zcfree(voidpf opaque, voidpf ptr) {
    ਪੂਰਨ n;

    (ਖਾਲੀ)opaque;

    ਜੇ (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        ਵਾਪਸ;
    }
    /* Find the original pointer */
    ਲਈ (n = 0; n < ਅਗਲਾ_ptr; n++) {
        ਜੇ (ptr != ਸਾਰਣੀ_2[n].new_ptr) ਜਾਰੀ;

        farfree(ਸਾਰਣੀ_2[n].org_ptr);
        ਜਦੋਂ (++n < ਅਗਲਾ_ptr) {
            ਸਾਰਣੀ_2[n-1] = ਸਾਰਣੀ_2[n];
        }
        ਅਗਲਾ_ptr--;
        ਵਾਪਸ;
    }
    Assert(0, "zcfree: ptr not found");
}

#ਜੇ_ਅੰਤ /* __TURBOC__ */


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#ਜੇ (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#ਜੇ_ਅੰਤ

voidpf ZLIB_ਅੰਦਰੂਨੀ zcalloc(voidpf opaque, uInt ਚੀਜ਼ਾਂ_2, uInt ਮਾਪ) {
    (ਖਾਲੀ)opaque;
    ਵਾਪਸ _halloc((ਲੰਮਾ)ਚੀਜ਼ਾਂ_2, ਮਾਪ);
}

ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zcfree(voidpf opaque, voidpf ptr) {
    (ਖਾਲੀ)opaque;
    _hfree(ptr);
}

#ਜੇ_ਅੰਤ /* M_I86 */

#ਜੇ_ਅੰਤ /* SYS16BIT */


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MY_ZCALLOC /* Any system without a special alloc function */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDC
ਬਾਹਰੀ voidp ਸਮ੍ਰਿਤੀ_ਦਿਓ(uInt ਮਾਪ);
ਬਾਹਰੀ voidp ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(uInt ਚੀਜ਼ਾਂ_2, uInt ਮਾਪ);
ਬਾਹਰੀ ਖਾਲੀ ਮੁਕਤ(voidpf ptr);
#ਜੇ_ਅੰਤ

voidpf ZLIB_ਅੰਦਰੂਨੀ zcalloc(voidpf opaque, ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੀਜ਼ਾਂ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ) {
    (ਖਾਲੀ)opaque;
    ਵਾਪਸ ਆਕਾਰ(uInt) > 2 ? (voidpf)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਚੀਜ਼ਾਂ_2 * ਮਾਪ) :
                              (voidpf)ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਚੀਜ਼ਾਂ_2, ਮਾਪ);
}

ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ zcfree(voidpf opaque, voidpf ptr) {
    (ਖਾਲੀ)opaque;
    ਮੁਕਤ(ptr);
}

#ਜੇ_ਅੰਤ /* MY_ZCALLOC */

#ਜੇ_ਅੰਤ /* !Z_SOLO */
