/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#అన్తర్భావ "zutil.h"
#నిర్వచితం_కాకపోతే Z_SOLO
#  include "gzguts.h"
#అయితే_ముగింపు

z_const అక్షరం * స్థిరరాశి z_errmsg[10] = {
    (z_const అక్షరం *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const అక్షరం *)"stream end",          /* Z_STREAM_END      1  */
    (z_const అక్షరం *)"",                    /* Z_OK              0  */
    (z_const అక్షరం *)"file error",          /* Z_ERRNO         (-1) */
    (z_const అక్షరం *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const అక్షరం *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const అక్షరం *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const అక్షరం *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const అక్షరం *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const అక్షరం *)""
};


స్థిరరాశి అక్షరం * ZEXPORT zlib_సంస్కరణ(శూన్య) {
    తిరిగి ZLIB_సంస్కరణ;
}

uLong ZEXPORT zlibCompileFlags(శూన్య) {
    uLong flags;

    flags = 0;
    ఎంపిక ((పూర్ణాంకం)(పరిమాణం(uInt))) {
    స్థితి 2:     ఆపు;
    స్థితి 4:     flags += 1;     ఆపు;
    స్థితి 8:     flags += 2;     ఆపు;
    అప్రమేయ:    flags += 3;
    }
    ఎంపిక ((పూర్ణాంకం)(పరిమాణం(uLong))) {
    స్థితి 2:     ఆపు;
    స్థితి 4:     flags += 1 << 2;        ఆపు;
    స్థితి 8:     flags += 2 << 2;        ఆపు;
    అప్రమేయ:    flags += 3 << 2;
    }
    ఎంపిక ((పూర్ణాంకం)(పరిమాణం(voidpf))) {
    స్థితి 2:     ఆపు;
    స్థితి 4:     flags += 1 << 4;        ఆపు;
    స్థితి 8:     flags += 2 << 4;        ఆపు;
    అప్రమేయ:    flags += 3 << 4;
    }
    ఎంపిక ((పూర్ణాంకం)(పరిమాణం(z_off_t))) {
    స్థితి 2:     ఆపు;
    స్థితి 4:     flags += 1 << 6;        ఆపు;
    స్థితి 8:     flags += 2 << 6;        ఆపు;
    అప్రమేయ:    flags += 3 << 6;
    }
#నిర్వచితమైతే ZLIB_DEBUG
    flags += 1 << 8;
#అయితే_ముగింపు
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#నిర్వచితమైతే ZLIB_WINAPI
    flags += 1 << 10;
#అయితే_ముగింపు
#నిర్వచితమైతే BUILDFIXED
    flags += 1 << 12;
#అయితే_ముగింపు
#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
    flags += 1 << 13;
#అయితే_ముగింపు
#నిర్వచితమైతే NO_GZCOMPRESS
    flags += 1L << 16;
#అయితే_ముగింపు
#నిర్వచితమైతే NO_GZIP
    flags += 1L << 17;
#అయితే_ముగింపు
#నిర్వచితమైతే PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#అయితే_ముగింపు
#నిర్వచితమైతే FASTEST
    flags += 1L << 21;
#అయితే_ముగింపు
#ఒకవేళ defined(STDC) || defined(Z_HAVE_STDARG_H)
#   ifdef NO_vsnprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       లేకపోతే
            flags += 1L << 27;
#       endif
#       ifdef HAS_vsprintf_void
            flags += 1L << 26;
#       endif
#   లేకపోతే
#       ifdef HAS_vsnprintf_void
            flags += 1L << 26;
#       endif
#   endif
#లేకపోతే
    flags += 1L << 24;
#   ifdef NO_snprintf
#       ifdef ZLIB_INSECURE
            flags += 1L << 25;
#       లేకపోతే
            flags += 1L << 27;
#       endif
#       ifdef HAS_sprintf_void
            flags += 1L << 26;
#       endif
#   లేకపోతే
#       ifdef HAS_snprintf_void
            flags += 1L << 26;
#       endif
#   endif
#అయితే_ముగింపు
    తిరిగి flags;
}

#నిర్వచితమైతే ZLIB_DEBUG
#అన్తర్భావ <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
పూర్ణాంకం ZLIB_అంతర్గత z_verbose = verbose;

శూన్య ZLIB_అంతర్గత z_error(అక్షరం *m) {
    fprintf(stderr, "%s\n", m);
    నిష్క్రమించు(1);
}
#అయితే_ముగింపు

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
స్థిరరాశి అక్షరం * ZEXPORT zError(పూర్ణాంకం err) {
    తిరిగి ERR_సందేశం(err);
}

#ఒకవేళ defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    పూర్ణాంకం errno = 0;
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే HAVE_MEMCPY

శూన్య ZLIB_అంతర్గత zmemcpy(శూన్య FAR *dst, స్థిరరాశి శూన్య FAR *src, z_కొలత_t n) {
    uchf *p = dst;
    స్థిరరాశి uchf *q = src;
    అయితే (n) {
        *p++ = *q++;
        n--;
    }
}

పూర్ణాంకం ZLIB_అంతర్గత zmemcmp(స్థిరరాశి శూన్య FAR *s1, స్థిరరాశి శూన్య FAR *s2, z_కొలత_t n) {
    స్థిరరాశి uchf *p = s1, *q = s2;
    అయితే (n) {
        ఒకవేళ (*p++ != *q++)
            తిరిగి (పూర్ణాంకం)p[-1] - (పూర్ణాంకం)q[-1];
        n--;
    }
    తిరిగి 0;
}

శూన్య ZLIB_అంతర్గత zmemzero(శూన్య FAR *b, z_కొలత_t len) {
    uchf *p = b;
    అయితే (len) {
        *p++ = 0;
        len--;
    }
}

#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే Z_SOLO

#నిర్వచితమైతే SYS16_బిట్టు

#నిర్వచితమైతే __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#నిర్వచించు MAX_PTR 10
/* 10*64K = 640K */

local పూర్ణాంకం తదుపరి_ptr = 0;

రకనిర్వచనం నిర్మాణం ptr_పట్టిక_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_పట్టిక;

local ptr_పట్టిక పట్టిక_2[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */

voidpf ZLIB_అంతర్గత zcalloc(voidpf opaque, గుర్తులేని వస్తువులు_2, గుర్తులేని కొలత) {
    voidpf buf;
    ulg bsize = (ulg)వస్తువులు_2*కొలత;

    (శూన్య)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    ఒకవేళ (bsize < 65520L) {
        buf = farmalloc(bsize);
        ఒకవేళ (*(ush*)&buf != 0) తిరిగి buf;
    } లేకపోతే {
        buf = farmalloc(bsize + 16L);
    }
    ఒకవేళ (buf == ఖాళీ || తదుపరి_ptr >= MAX_PTR) తిరిగి ఖాళీ;
    పట్టిక_2[తదుపరి_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    పట్టిక_2[తదుపరి_ptr++].new_ptr = buf;
    తిరిగి buf;
}

శూన్య ZLIB_అంతర్గత zcfree(voidpf opaque, voidpf ptr) {
    పూర్ణాంకం n;

    (శూన్య)opaque;

    ఒకవేళ (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        తిరిగి;
    }
    /* Find the original pointer */
    కొరకు (n = 0; n < తదుపరి_ptr; n++) {
        ఒకవేళ (ptr != పట్టిక_2[n].new_ptr) కొనసాగు;

        farfree(పట్టిక_2[n].org_ptr);
        అయితే (++n < తదుపరి_ptr) {
            పట్టిక_2[n-1] = పట్టిక_2[n];
        }
        తదుపరి_ptr--;
        తిరిగి;
    }
    Assert(0, "zcfree: ptr not found");
}

#అయితే_ముగింపు /* __TURBOC__ */


#నిర్వచితమైతే M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#ఒకవేళ (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#అయితే_ముగింపు

voidpf ZLIB_అంతర్గత zcalloc(voidpf opaque, uInt వస్తువులు_2, uInt కొలత) {
    (శూన్య)opaque;
    తిరిగి _halloc((దీర్ఘ)వస్తువులు_2, కొలత);
}

శూన్య ZLIB_అంతర్గత zcfree(voidpf opaque, voidpf ptr) {
    (శూన్య)opaque;
    _hfree(ptr);
}

#అయితే_ముగింపు /* M_I86 */

#అయితే_ముగింపు /* SYS16BIT */


#నిర్వచితం_కాకపోతే MY_ZCALLOC /* Any system without a special alloc function */

#నిర్వచితం_కాకపోతే STDC
బాహ్య voidp జ్ఞాపకం_ఇవ్వు(uInt కొలత);
బాహ్య voidp సున్నా_జ్ఞాపకం_ఇవ్వు(uInt వస్తువులు_2, uInt కొలత);
బాహ్య శూన్య విడుదల(voidpf ptr);
#అయితే_ముగింపు

voidpf ZLIB_అంతర్గత zcalloc(voidpf opaque, గుర్తులేని వస్తువులు_2, గుర్తులేని కొలత) {
    (శూన్య)opaque;
    తిరిగి పరిమాణం(uInt) > 2 ? (voidpf)జ్ఞాపకం_ఇవ్వు(వస్తువులు_2 * కొలత) :
                              (voidpf)సున్నా_జ్ఞాపకం_ఇవ్వు(వస్తువులు_2, కొలత);
}

శూన్య ZLIB_అంతర్గత zcfree(voidpf opaque, voidpf ptr) {
    (శూన్య)opaque;
    విడుదల(ptr);
}

#అయితే_ముగింపు /* MY_ZCALLOC */

#అయితే_ముగింపు /* !Z_SOLO */
