/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ಅನ್ತರ್ಭಾವ "zutil.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_SOLO
#  include "gzguts.h"
#ಆದರೆ_ಅಂತ್ಯ

z_const ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ z_errmsg[10] = {
    (z_const ಅಕ್ಷರ *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const ಅಕ್ಷರ *)"stream end",          /* Z_STREAM_END      1  */
    (z_const ಅಕ್ಷರ *)"",                    /* Z_OK              0  */
    (z_const ಅಕ್ಷರ *)"file error",          /* Z_ERRNO         (-1) */
    (z_const ಅಕ್ಷರ *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const ಅಕ್ಷರ *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const ಅಕ್ಷರ *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const ಅಕ್ಷರ *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const ಅಕ್ಷರ *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const ಅಕ್ಷರ *)""
};


ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ZEXPORT zlib_ಆವೃತ್ತಿ(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ ZLIB_ಆವೃತ್ತಿ;
}

uLong ZEXPORT zlibCompileFlags(ಶೂನ್ಯ) {
    uLong flags;

    flags = 0;
    ಆಯ್ಕೆ ((ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(uInt))) {
    ಸ್ಥಿತಿ 2:     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 4:     flags += 1;     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8:     flags += 2;     ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:    flags += 3;
    }
    ಆಯ್ಕೆ ((ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(uLong))) {
    ಸ್ಥಿತಿ 2:     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 4:     flags += 1 << 2;        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8:     flags += 2 << 2;        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:    flags += 3 << 2;
    }
    ಆಯ್ಕೆ ((ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(voidpf))) {
    ಸ್ಥಿತಿ 2:     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 4:     flags += 1 << 4;        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8:     flags += 2 << 4;        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:    flags += 3 << 4;
    }
    ಆಯ್ಕೆ ((ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(z_off_t))) {
    ಸ್ಥಿತಿ 2:     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 4:     flags += 1 << 6;        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8:     flags += 2 << 6;        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:    flags += 3 << 6;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
    flags += 1 << 8;
#ಆದರೆ_ಅಂತ್ಯ
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_WINAPI
    flags += 1 << 10;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BUILDFIXED
    flags += 1 << 12;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
    flags += 1 << 13;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_GZCOMPRESS
    flags += 1L << 16;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_GZIP
    flags += 1L << 17;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FASTEST
    flags += 1L << 21;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ಇಲ್ಲದಿದ್ದರೆ
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   ಇಲ್ಲದಿದ್ದರೆ
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#ಇಲ್ಲದಿದ್ದರೆ
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       ಇಲ್ಲದಿದ್ದರೆ
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   ಇಲ್ಲದಿದ್ದರೆ
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#ಆದರೆ_ಅಂತ್ಯ
    ಹಿಂತಿರುಗಿ flags;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DEBUG
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
ಪೂರ್ಣಾಂಕ ZLIB_ಆಂತರಿಕ z_verbose = verbose;

ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ z_error(ಅಕ್ಷರ *m) {
    fprintf(stderr, "%s\n", m);
    ನಿರ್ಗಮಿಸು(1);
}
#ಆದರೆ_ಅಂತ್ಯ

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ZEXPORT zError(ಪೂರ್ಣಾಂಕ err) {
    ಹಿಂತಿರುಗಿ ERR_ಸಂದೇಶ(err);
}

#ಆದರೆ defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    ಪೂರ್ಣಾಂಕ errno = 0;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_MEMCPY

ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zmemcpy(ಶೂನ್ಯ FAR *dst, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ FAR *src, z_ಗಾತ್ರ_t n) {
    uchf *p = dst;
    ಸ್ಥಿರಾಂಕ uchf *q = src;
    ಆಗ (n) {
        *p++ = *q++;
        n--;
    }
}

ಪೂರ್ಣಾಂಕ ZLIB_ಆಂತರಿಕ zmemcmp(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ FAR *s1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ FAR *s2, z_ಗಾತ್ರ_t n) {
    ಸ್ಥಿರಾಂಕ uchf *p = s1, *q = s2;
    ಆಗ (n) {
        ಆದರೆ (*p++ != *q++)
            ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)p[-1] - (ಪೂರ್ಣಾಂಕ)q[-1];
        n--;
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zmemzero(ಶೂನ್ಯ FAR *b, z_ಗಾತ್ರ_t len) {
    uchf *p = b;
    ಆಗ (len) {
        *p++ = 0;
        len--;
    }
}

#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_SOLO

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SYS16_ಬಿಟ್

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_PTR 10
/* 10*64K = 640K */

local ಪೂರ್ಣಾಂಕ ಮುಂದಿನ_ptr = 0;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ptr_ಕೋಷ್ಟಕ_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_ಕೋಷ್ಟಕ;

local ptr_ಕೋಷ್ಟಕ ಕೋಷ್ಟಕ_2[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */

voidpf ZLIB_ಆಂತರಿಕ zcalloc(voidpf opaque, ಚಿಹ್ನೆರಹಿತ ವಸ್ತುಗಳು_2, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2) {
    voidpf buf;
    ulg bsize = (ulg)ವಸ್ತುಗಳು_2*ಗಾತ್ರ_2;

    (ಶೂನ್ಯ)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    ಆದರೆ (bsize < 65520L) {
        buf = farmalloc(bsize);
        ಆದರೆ (*(ush*)&buf != 0) ಹಿಂತಿರುಗಿ buf;
    } ಇಲ್ಲದಿದ್ದರೆ {
        buf = farmalloc(bsize + 16L);
    }
    ಆದರೆ (buf == ಖಾಲಿ || ಮುಂದಿನ_ptr >= MAX_PTR) ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಕೋಷ್ಟಕ_2[ಮುಂದಿನ_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    ಕೋಷ್ಟಕ_2[ಮುಂದಿನ_ptr++].new_ptr = buf;
    ಹಿಂತಿರುಗಿ buf;
}

ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zcfree(voidpf opaque, voidpf ptr) {
    ಪೂರ್ಣಾಂಕ n;

    (ಶೂನ್ಯ)opaque;

    ಆದರೆ (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        ಹಿಂತಿರುಗಿ;
    }
    /* Find the original pointer */
    ಗಾಗಿ (n = 0; n < ಮುಂದಿನ_ptr; n++) {
        ಆದರೆ (ptr != ಕೋಷ್ಟಕ_2[n].new_ptr) ಮುಂದುವರಿ;

        farfree(ಕೋಷ್ಟಕ_2[n].org_ptr);
        ಆಗ (++n < ಮುಂದಿನ_ptr) {
            ಕೋಷ್ಟಕ_2[n-1] = ಕೋಷ್ಟಕ_2[n];
        }
        ಮುಂದಿನ_ptr--;
        ಹಿಂತಿರುಗಿ;
    }
    Assert(0, "zcfree: ptr not found");
}

#ಆದರೆ_ಅಂತ್ಯ /* __TURBOC__ */


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#ಆದರೆ (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#ಆದರೆ_ಅಂತ್ಯ

voidpf ZLIB_ಆಂತರಿಕ zcalloc(voidpf opaque, uInt ವಸ್ತುಗಳು_2, uInt ಗಾತ್ರ_2) {
    (ಶೂನ್ಯ)opaque;
    ಹಿಂತಿರುಗಿ _halloc((ದೀರ್ಘ)ವಸ್ತುಗಳು_2, ಗಾತ್ರ_2);
}

ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zcfree(voidpf opaque, voidpf ptr) {
    (ಶೂನ್ಯ)opaque;
    _hfree(ptr);
}

#ಆದರೆ_ಅಂತ್ಯ /* M_I86 */

#ಆದರೆ_ಅಂತ್ಯ /* SYS16BIT */


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MY_ZCALLOC /* Any system without a special alloc function */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDC
ಬಾಹ್ಯ voidp ಸ್ಮರಣೆ_ಕೊಡು(uInt ಗಾತ್ರ_2);
ಬಾಹ್ಯ voidp ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(uInt ವಸ್ತುಗಳು_2, uInt ಗಾತ್ರ_2);
ಬಾಹ್ಯ ಶೂನ್ಯ ಬಿಡುಗಡೆ(voidpf ptr);
#ಆದರೆ_ಅಂತ್ಯ

voidpf ZLIB_ಆಂತರಿಕ zcalloc(voidpf opaque, ಚಿಹ್ನೆರಹಿತ ವಸ್ತುಗಳು_2, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2) {
    (ಶೂನ್ಯ)opaque;
    ಹಿಂತಿರುಗಿ ಗಾತ್ರ(uInt) > 2 ? (voidpf)ಸ್ಮರಣೆ_ಕೊಡು(ವಸ್ತುಗಳು_2 * ಗಾತ್ರ_2) :
                              (voidpf)ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(ವಸ್ತುಗಳು_2, ಗಾತ್ರ_2);
}

ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ zcfree(voidpf opaque, voidpf ptr) {
    (ಶೂನ್ಯ)opaque;
    ಬಿಡುಗಡೆ(ptr);
}

#ಆದರೆ_ಅಂತ್ಯ /* MY_ZCALLOC */

#ಆದರೆ_ಅಂತ್ಯ /* !Z_SOLO */
