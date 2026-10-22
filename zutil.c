/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#அந்தர்பாவ "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const எழுத்து * மாறிலி z_errmsg[10] = {
    (z_const எழுத்து *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const எழுத்து *)"stream end",          /* Z_STREAM_END      1  */
    (z_const எழுத்து *)"",                    /* Z_OK              0  */
    (z_const எழுத்து *)"file error",          /* Z_ERRNO         (-1) */
    (z_const எழுத்து *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const எழுத்து *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const எழுத்து *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const எழுத்து *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const எழுத்து *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const எழுத்து *)""
};


மாறிலி எழுத்து * ZEXPORT zlibVersion(வெறுமை) {
    திரும்பு ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(வெறுமை) {
    uLong flags;

    flags = 0;
    தேர்வு ((முழு)(அளவு(uInt))) {
    நிலை 2:     நிறுத்து;
    நிலை 4:     flags += 1;     நிறுத்து;
    நிலை 8:     flags += 2;     நிறுத்து;
    இயல்பு:    flags += 3;
    }
    தேர்வு ((முழு)(அளவு(uLong))) {
    நிலை 2:     நிறுத்து;
    நிலை 4:     flags += 1 << 2;        நிறுத்து;
    நிலை 8:     flags += 2 << 2;        நிறுத்து;
    இயல்பு:    flags += 3 << 2;
    }
    தேர்வு ((முழு)(அளவு(voidpf))) {
    நிலை 2:     நிறுத்து;
    நிலை 4:     flags += 1 << 4;        நிறுத்து;
    நிலை 8:     flags += 2 << 4;        நிறுத்து;
    இயல்பு:    flags += 3 << 4;
    }
    தேர்வு ((முழு)(அளவு(z_off_t))) {
    நிலை 2:     நிறுத்து;
    நிலை 4:     flags += 1 << 6;        நிறுத்து;
    நிலை 8:     flags += 2 << 6;        நிறுத்து;
    இயல்பு:    flags += 3 << 6;
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
#எனில் defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       இல்லையெனில்
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   இல்லையெனில்
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#இல்லையெனில்
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       இல்லையெனில்
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   இல்லையெனில்
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    திரும்பு flags;
}

#ifdef ZLIB_DEBUG
#அந்தர்பாவ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
முழு ZLIB_INTERNAL z_verbose = verbose;

வெறுமை ZLIB_INTERNAL z_error(எழுத்து *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
மாறிலி எழுத்து * ZEXPORT zError(முழு err) {
    திரும்பு ERR_செய்தி(err);
}

#எனில் defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    முழு errno = 0;
#endif

#ifndef HAVE_MEMCPY

வெறுமை ZLIB_INTERNAL zmemcpy(வெறுமை FAR *dst, மாறிலி வெறுமை FAR *src, z_size_t n) {
    uchf *p = dst;
    மாறிலி uchf *q = src;
    போது (n) {
        *p++ = *q++;
        n--;
    }
}

முழு ZLIB_INTERNAL zmemcmp(மாறிலி வெறுமை FAR *s1, மாறிலி வெறுமை FAR *s2, z_size_t n) {
    மாறிலி uchf *p = s1, *q = s2;
    போது (n) {
        எனில் (*p++ != *q++)
            திரும்பு (முழு)p[-1] - (முழு)q[-1];
        n--;
    }
    திரும்பு 0;
}

வெறுமை ZLIB_INTERNAL zmemzero(வெறுமை FAR *b, z_size_t len) {
    uchf *p = b;
    போது (len) {
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

local முழு next_ptr = 0;

வகைவரை கட்டமைப்பு ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned பொருட்கள்_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)பொருட்கள்_2*size;

    (வெறுமை)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    எனில் (bsize < 65520L) {
        buf = farmalloc(bsize);
        எனில் (*(ush*)&buf != 0) திரும்பு buf;
    } இல்லையெனில் {
        buf = farmalloc(bsize + 16L);
    }
    எனில் (buf == வெற்று || next_ptr >= MAX_PTR) திரும்பு வெற்று;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    திரும்பு buf;
}

வெறுமை ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    முழு n;

    (வெறுமை)opaque;

    எனில் (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        திரும்பு;
    }
    /* Find the original pointer */
    க்கு (n = 0; n < next_ptr; n++) {
        எனில் (ptr != table[n].new_ptr) தொடர்;

        farfree(table[n].org_ptr);
        போது (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        திரும்பு;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#எனில் (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt பொருட்கள்_2, uInt size) {
    (வெறுமை)opaque;
    திரும்பு _halloc((long)பொருட்கள்_2, size);
}

வெறுமை ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (வெறுமை)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
வெளி voidp malloc(uInt size);
வெளி voidp calloc(uInt பொருட்கள்_2, uInt size);
வெளி வெறுமை விடுவி(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned பொருட்கள்_2, unsigned size) {
    (வெறுமை)opaque;
    திரும்பு அளவு(uInt) > 2 ? (voidpf)malloc(பொருட்கள்_2 * size) :
                              (voidpf)calloc(பொருட்கள்_2, size);
}

வெறுமை ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (வெறுமை)opaque;
    விடுவி(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
