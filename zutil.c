/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#समावेश "zutil.h"
#अगर_अघोषित Z_SOLO
#  include "gzguts.h"
#पूर्ण_अगर

z_const अक्षर * स्थिर z_errmsg[10] = {
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


स्थिर अक्षर * ZEXPORT zlibVersion(व्योम) {
    वापस ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(व्योम) {
    uLong flags;

    flags = 0;
    चयन ((पूर्णांक)(माप(uInt))) {
    हाल 2:     अवरोध;
    हाल 4:     flags += 1;     अवरोध;
    हाल 8:     flags += 2;     अवरोध;
    शेष:    flags += 3;
    }
    चयन ((पूर्णांक)(माप(uLong))) {
    हाल 2:     अवरोध;
    हाल 4:     flags += 1 << 2;        अवरोध;
    हाल 8:     flags += 2 << 2;        अवरोध;
    शेष:    flags += 3 << 2;
    }
    चयन ((पूर्णांक)(माप(voidpf))) {
    हाल 2:     अवरोध;
    हाल 4:     flags += 1 << 4;        अवरोध;
    हाल 8:     flags += 2 << 4;        अवरोध;
    शेष:    flags += 3 << 4;
    }
    चयन ((पूर्णांक)(माप(z_off_t))) {
    हाल 2:     अवरोध;
    हाल 4:     flags += 1 << 6;        अवरोध;
    हाल 8:     flags += 2 << 6;        अवरोध;
    शेष:    flags += 3 << 6;
    }
#अगर_घोषित ZLIB_DEBUG
    flags += 1 << 8;
#पूर्ण_अगर
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#अगर_घोषित ZLIB_WINAPI
    flags += 1 << 10;
#पूर्ण_अगर
#अगर_घोषित BUILDFIXED
    flags += 1 << 12;
#पूर्ण_अगर
#अगर_घोषित DYNAMIC_CRC_TABLE
    flags += 1 << 13;
#पूर्ण_अगर
#अगर_घोषित NO_GZCOMPRESS
    flags += 1L << 16;
#पूर्ण_अगर
#अगर_घोषित NO_GZIP
    flags += 1L << 17;
#पूर्ण_अगर
#अगर_घोषित PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#पूर्ण_अगर
#अगर_घोषित FASTEST
    flags += 1L << 21;
#पूर्ण_अगर
#अगर defined(STDC) || defined(Z_HAVE_STDARG_H)
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
#पूर्ण_अगर
    वापस flags;
}

#अगर_घोषित ZLIB_DEBUG
#समावेश <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
पूर्णांक ZLIB_INTERNAL z_verbose = verbose;

व्योम ZLIB_INTERNAL z_error(अक्षर *m) {
    ख_लिखो(मानक_त्रुटि, "%s\n", m);
    निकास(1);
}
#पूर्ण_अगर

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
स्थिर अक्षर * ZEXPORT zError(पूर्णांक err) {
    वापस ERR_संदेश(err);
}

#अगर defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    पूर्णांक त्रुटि_सं = 0;
#पूर्ण_अगर

#अगर_अघोषित HAVE_MEMCPY

व्योम ZLIB_INTERNAL zmemcpy(व्योम FAR *dst, स्थिर व्योम FAR *src, z_आकार_t n) {
    uchf *p = dst;
    स्थिर uchf *q = src;
    जबतक (n) {
        *p++ = *q++;
        n--;
    }
}

पूर्णांक ZLIB_INTERNAL zmemcmp(स्थिर व्योम FAR *s1, स्थिर व्योम FAR *s2, z_आकार_t n) {
    स्थिर uchf *p = s1, *q = s2;
    जबतक (n) {
        अगर (*p++ != *q++)
            वापस (पूर्णांक)p[-1] - (पूर्णांक)q[-1];
        n--;
    }
    वापस 0;
}

व्योम ZLIB_INTERNAL zmemzero(व्योम FAR *b, z_आकार_t len) {
    uchf *p = b;
    जबतक (len) {
        *p++ = 0;
        len--;
    }
}

#पूर्ण_अगर

#अगर_अघोषित Z_SOLO

#अगर_घोषित SYS16BIT

#अगर_घोषित __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#घोषणा MAX_PTR 10
/* 10*64K = 640K */

local पूर्णांक अग्र_ptr = 0;

प्रकार काष्ठा ptr_table_s {
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

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, अचिन्हित वस्तुएँ_2, अचिन्हित आकार) {
    voidpf buf;
    ulg bsize = (ulg)वस्तुएँ_2*आकार;

    (व्योम)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    अगर (bsize < 65520L) {
        buf = farmalloc(bsize);
        अगर (*(ush*)&buf != 0) वापस buf;
    } अन्यथा {
        buf = farmalloc(bsize + 16L);
    }
    अगर (buf == शून्य || अग्र_ptr >= MAX_PTR) वापस शून्य;
    table[अग्र_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[अग्र_ptr++].new_ptr = buf;
    वापस buf;
}

व्योम ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    पूर्णांक n;

    (व्योम)opaque;

    अगर (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        वापस;
    }
    /* Find the original pointer */
    क्रम (n = 0; n < अग्र_ptr; n++) {
        अगर (ptr != table[n].new_ptr) जारी;

        farfree(table[n].org_ptr);
        जबतक (++n < अग्र_ptr) {
            table[n-1] = table[n];
        }
        अग्र_ptr--;
        वापस;
    }
    Assert(0, "zcfree: ptr not found");
}

#पूर्ण_अगर /* __TURBOC__ */


#अगर_घोषित M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#अगर (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#पूर्ण_अगर

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt वस्तुएँ_2, uInt आकार) {
    (व्योम)opaque;
    वापस _halloc((दीर्घ)वस्तुएँ_2, आकार);
}

व्योम ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (व्योम)opaque;
    _hfree(ptr);
}

#पूर्ण_अगर /* M_I86 */

#पूर्ण_अगर /* SYS16BIT */


#अगर_अघोषित MY_ZCALLOC /* Any system without a special alloc function */

#अगर_अघोषित STDC
बाह्य voidp दो_स्मृति(uInt आकार);
बाह्य voidp सुस्मृति(uInt वस्तुएँ_2, uInt आकार);
बाह्य व्योम मुक्त(voidpf ptr);
#पूर्ण_अगर

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, अचिन्हित वस्तुएँ_2, अचिन्हित आकार) {
    (व्योम)opaque;
    वापस माप(uInt) > 2 ? (voidpf)दो_स्मृति(वस्तुएँ_2 * आकार) :
                              (voidpf)सुस्मृति(वस्तुएँ_2, आकार);
}

व्योम ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (व्योम)opaque;
    मुक्त(ptr);
}

#पूर्ण_अगर /* MY_ZCALLOC */

#पूर्ण_अगर /* !Z_SOLO */
