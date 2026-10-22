/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#अन्तर्भाव "zutil.h"
#ifndef Z_SOLO
#  include "gzguts.h"
#endif

z_const अक्षर * स्थिरांक z_errmsg[10] = {
    (z_const अक्षर *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const अक्षर *)"stream end",          /* Z_STREAM_END      1  */
    (z_const अक्षर *)"",                    /* Z_OK              0  */
    (z_const अक्षर *)"file error",          /* Z_ERRNO         (-1) */
    (z_const अक्षर *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const अक्षर *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const अक्षर *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const अक्षर *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const अक्षर *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const अक्षर *)""
};


स्थिरांक अक्षर * ZEXPORT zlibVersion(शून्य) {
    परत ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(शून्य) {
    uLong flags;

    flags = 0;
    निवड ((पूर्णांक)(आकार(uInt))) {
    स्थिती 2:     थांबा;
    स्थिती 4:     flags += 1;     थांबा;
    स्थिती 8:     flags += 2;     थांबा;
    पूर्वनिर्धारित:    flags += 3;
    }
    निवड ((पूर्णांक)(आकार(uLong))) {
    स्थिती 2:     थांबा;
    स्थिती 4:     flags += 1 << 2;        थांबा;
    स्थिती 8:     flags += 2 << 2;        थांबा;
    पूर्वनिर्धारित:    flags += 3 << 2;
    }
    निवड ((पूर्णांक)(आकार(voidpf))) {
    स्थिती 2:     थांबा;
    स्थिती 4:     flags += 1 << 4;        थांबा;
    स्थिती 8:     flags += 2 << 4;        थांबा;
    पूर्वनिर्धारित:    flags += 3 << 4;
    }
    निवड ((पूर्णांक)(आकार(z_off_t))) {
    स्थिती 2:     थांबा;
    स्थिती 4:     flags += 1 << 6;        थांबा;
    स्थिती 8:     flags += 2 << 6;        थांबा;
    पूर्वनिर्धारित:    flags += 3 << 6;
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
#जर defined(STDC) || defined(Z_HAVE_STDARG_H)
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
    परत flags;
}

#ifdef ZLIB_DEBUG
#अन्तर्भाव <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
पूर्णांक ZLIB_INTERNAL z_verbose = verbose;

शून्य ZLIB_INTERNAL z_error(अक्षर *m) {
    ख_लिखो(मानक_त्रुटि, "%s\n", m);
    निकास(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
स्थिरांक अक्षर * ZEXPORT zError(पूर्णांक err) {
    परत ERR_संदेश(err);
}

#जर defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    पूर्णांक त्रुटि_सं = 0;
#endif

#ifndef HAVE_MEMCPY

शून्य ZLIB_INTERNAL zmemcpy(शून्य FAR *dst, स्थिरांक शून्य FAR *src, z_size_t n) {
    uchf *p = dst;
    स्थिरांक uchf *q = src;
    पर्यंत (n) {
        *p++ = *q++;
        n--;
    }
}

पूर्णांक ZLIB_INTERNAL zmemcmp(स्थिरांक शून्य FAR *s1, स्थिरांक शून्य FAR *s2, z_size_t n) {
    स्थिरांक uchf *p = s1, *q = s2;
    पर्यंत (n) {
        जर (*p++ != *q++)
            परत (पूर्णांक)p[-1] - (पूर्णांक)q[-1];
        n--;
    }
    परत 0;
}

शून्य ZLIB_INTERNAL zmemzero(शून्य FAR *b, z_size_t len) {
    uchf *p = b;
    पर्यंत (len) {
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

local पूर्णांक next_ptr = 0;

प्रकारव्याख्या संरचना ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, अचिन्हित वस्तुसंच_2, अचिन्हित size) {
    voidpf buf;
    ulg bsize = (ulg)वस्तुसंच_2*size;

    (शून्य)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    जर (bsize < 65520L) {
        buf = farmalloc(bsize);
        जर (*(ush*)&buf != 0) परत buf;
    } अन्यथा {
        buf = farmalloc(bsize + 16L);
    }
    जर (buf == रिक्त || next_ptr >= MAX_PTR) परत रिक्त;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    परत buf;
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    पूर्णांक n;

    (शून्य)opaque;

    जर (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        परत;
    }
    /* Find the original pointer */
    साठी (n = 0; n < next_ptr; n++) {
        जर (ptr != table[n].new_ptr) सुरूठेवा;

        farfree(table[n].org_ptr);
        पर्यंत (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        परत;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#जर (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt वस्तुसंच_2, uInt size) {
    (शून्य)opaque;
    परत _halloc((दीर्घ)वस्तुसंच_2, size);
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (शून्य)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
बाह्य voidp दो_स्मृति(uInt size);
बाह्य voidp सुस्मृति(uInt वस्तुसंच_2, uInt size);
बाह्य शून्य मुक्त(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, अचिन्हित वस्तुसंच_2, अचिन्हित size) {
    (शून्य)opaque;
    परत आकार(uInt) > 2 ? (voidpf)दो_स्मृति(वस्तुसंच_2 * size) :
                              (voidpf)सुस्मृति(वस्तुसंच_2, size);
}

शून्य ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (शून्य)opaque;
    मुक्त(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
