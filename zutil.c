/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#अन्तर्भाव "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const वर्ण * ध्रुव z_errmsg[10] = {
    (z_const वर्ण *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const वर्ण *)"stream end",          /* Z_STREAM_END      1  */
    (z_const वर्ण *)"",                    /* Z_OK              0  */
    (z_const वर्ण *)"file error",          /* Z_ERRNO         (-1) */
    (z_const वर्ण *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const वर्ण *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const वर्ण *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const वर्ण *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const वर्ण *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const वर्ण *)""
};


ध्रुव वर्ण * ZEXPORT zlibVersion(शून्य) {
    प्रत्यागम ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(शून्य) {
    uLong flags;

    flags = 0;
    विकल्प ((पूर्णाङ्क)(परिमाण(uInt))) {
    पक्ष 2:     भङ्ग;
    पक्ष 4:     flags += 1;     भङ्ग;
    पक्ष 8:     flags += 2;     भङ्ग;
    स्वभाव:    flags += 3;
    }
    विकल्प ((पूर्णाङ्क)(परिमाण(uLong))) {
    पक्ष 2:     भङ्ग;
    पक्ष 4:     flags += 1 << 2;        भङ्ग;
    पक्ष 8:     flags += 2 << 2;        भङ्ग;
    स्वभाव:    flags += 3 << 2;
    }
    विकल्प ((पूर्णाङ्क)(परिमाण(voidpf))) {
    पक्ष 2:     भङ्ग;
    पक्ष 4:     flags += 1 << 4;        भङ्ग;
    पक्ष 8:     flags += 2 << 4;        भङ्ग;
    स्वभाव:    flags += 3 << 4;
    }
    विकल्प ((पूर्णाङ्क)(परिमाण(z_off_t))) {
    पक्ष 2:     भङ्ग;
    पक्ष 4:     flags += 1 << 6;        भङ्ग;
    पक्ष 8:     flags += 2 << 6;        भङ्ग;
    स्वभाव:    flags += 3 << 6;
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
#यदि defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       अन्यथा
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   अन्यथा
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#अन्यथा
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       अन्यथा
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   अन्यथा
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    प्रत्यागम flags;
}

#ifdef ZLIB_DEBUG
#अन्तर्भाव <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
पूर्णाङ्क ZLIB_INTERNAL z_verbose = verbose;

शून्य ZLIB_INTERNAL z_error(वर्ण *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ध्रुव वर्ण * ZEXPORT zError(पूर्णाङ्क err) {
    प्रत्यागम ERR_संदेश(err);
}

#यदि defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    पूर्णाङ्क errno = 0;
#endif

#ifndef HAVE_MEMCPY

शून्य ZLIB_INTERNAL zmemcpy(शून्य FAR *dst, ध्रुव शून्य FAR *src, z_size_t n) {
    uchf *p = dst;
    ध्रुव uchf *q = src;
    यावत् (n) {
        *p++ = *q++;
        n--;
    }
}

पूर्णाङ्क ZLIB_INTERNAL zmemcmp(ध्रुव शून्य FAR *s1, ध्रुव शून्य FAR *s2, z_size_t n) {
    ध्रुव uchf *p = s1, *q = s2;
    यावत् (n) {
        यदि (*p++ != *q++)
            प्रत्यागम (पूर्णाङ्क)p[-1] - (पूर्णाङ्क)q[-1];
        n--;
    }
    प्रत्यागम 0;
}

शून्य ZLIB_INTERNAL zmemzero(शून्य FAR *b, z_size_t len) {
    uchf *p = b;
    यावत् (len) {
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

local पूर्णाङ्क next_ptr = 0;

प्रकारनिर्देश संरचना ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned वस्तुएँ_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)वस्तुएँ_2*size;

    (शून्य)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    यदि (bsize < 65520L) {
        buf = farmalloc(bsize);
        यदि (*(ush*)&buf != 0) प्रत्यागम buf;
    } अन्यथा {
        buf = farmalloc(bsize + 16L);
    }
    यदि (buf == रिक्त || next_ptr >= MAX_PTR) प्रत्यागम रिक्त;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    प्रत्यागम buf;
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    पूर्णाङ्क n;

    (शून्य)opaque;

    यदि (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        प्रत्यागम;
    }
    /* Find the original pointer */
    प्रति (n = 0; n < next_ptr; n++) {
        यदि (ptr != table[n].new_ptr) अनुवर्त;

        farfree(table[n].org_ptr);
        यावत् (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        प्रत्यागम;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#यदि (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt वस्तुएँ_2, uInt size) {
    (शून्य)opaque;
    प्रत्यागम _halloc((long)वस्तुएँ_2, size);
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (शून्य)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
बाह्य voidp malloc(uInt size);
बाह्य voidp calloc(uInt वस्तुएँ_2, uInt size);
बाह्य शून्य मुक्त(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned वस्तुएँ_2, unsigned size) {
    (शून्य)opaque;
    प्रत्यागम परिमाण(uInt) > 2 ? (voidpf)malloc(वस्तुएँ_2 * size) :
                              (voidpf)calloc(वस्तुएँ_2, size);
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (शून्य)opaque;
    मुक्त(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
