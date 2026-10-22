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
    पटिनिवत्त ZLIB_VERSION;
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
#सचे defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       अञ्ञथा
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   अञ्ञथा
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#अञ्ञथा
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       अञ्ञथा
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   अञ्ञथा
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#endif
    पटिनिवत्त flags;
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
    पटिनिवत्त ERR_सन्देशः(err);
}

#सचे defined(_WIN32_WCE) && _WIN32_WCE < 0x800
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
    याव (n) {
        *p++ = *q++;
        n--;
    }
}

पूर्णाङ्क ZLIB_INTERNAL zmemcmp(ध्रुव शून्य FAR *s1, ध्रुव शून्य FAR *s2, z_size_t n) {
    ध्रुव uchf *p = s1, *q = s2;
    याव (n) {
        सचे (*p++ != *q++)
            पटिनिवत्त (पूर्णाङ्क)p[-1] - (पूर्णाङ्क)q[-1];
        n--;
    }
    पटिनिवत्त 0;
}

शून्य ZLIB_INTERNAL zmemzero(शून्य FAR *b, z_size_t len) {
    uchf *p = b;
    याव (len) {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned वस्तूनि_2, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)वस्तूनि_2*size;

    (शून्य)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    सचे (bsize < 65520L) {
        buf = farmalloc(bsize);
        सचे (*(ush*)&buf != 0) पटिनिवत्त buf;
    } अञ्ञथा {
        buf = farmalloc(bsize + 16L);
    }
    सचे (buf == रिक्त || next_ptr >= MAX_PTR) पटिनिवत्त रिक्त;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    पटिनिवत्त buf;
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    पूर्णाङ्क n;

    (शून्य)opaque;

    सचे (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        पटिनिवत्त;
    }
    /* Find the original pointer */
    अत्थाय (n = 0; n < next_ptr; n++) {
        सचे (ptr != table[n].new_ptr) अनुवर्त;

        farfree(table[n].org_ptr);
        याव (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        पटिनिवत्त;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#सचे (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt वस्तूनि_2, uInt size) {
    (शून्य)opaque;
    पटिनिवत्त _halloc((long)वस्तूनि_2, size);
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
बाह्य voidp calloc(uInt वस्तूनि_2, uInt size);
बाह्य शून्य मुक्त(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned वस्तूनि_2, unsigned size) {
    (शून्य)opaque;
    पटिनिवत्त परिमाण(uInt) > 2 ? (voidpf)malloc(वस्तूनि_2 * size) :
                              (voidpf)calloc(वस्तूनि_2, size);
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (शून्य)opaque;
    मुक्त(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
