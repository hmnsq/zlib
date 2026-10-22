/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#నిర్వచితం_కాకపోతే ZUTIL_H
#నిర్వచించు ZUTIL_H

#నిర్వచితమైతే HAVE_HIDDEN
#  define ZLIB_అంతర్గత __attribute__((visibility ("hidden")))
#లేకపోతే
#  define ZLIB_అంతర్గత
#అయితే_ముగింపు

#అన్తర్భావ "zlib.h"

#ఒకవేళ defined(STDC) && !defined(Z_SOLO)
#  ఒకవేళ !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే local
#  define local స్థిర
#అయితే_ముగింపు
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

బాహ్య స్థిరరాశి అక్షరం deflate_copyright[];
బాహ్య స్థిరరాశి అక్షరం inflate_copyright[];
బాహ్య స్థిరరాశి అక్షరం inflate9_copyright[];

రకనిర్వచనం గుర్తులేని అక్షరం  uch;
రకనిర్వచనం uch FAR uchf;
రకనిర్వచనం గుర్తులేని పొట్టి ush;
రకనిర్వచనం ush FAR ushf;
రకనిర్వచనం గుర్తులేని దీర్ఘ  ulg;

#ఒకవేళ !defined(Z_U8) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ఒకవేళ (ULONG_MAX == 0xffffffffffffffff)
#    define Z_U8 గుర్తులేని దీర్ఘ
#  elif (ULLONG_MAX == 0xffffffffffffffff)
#    define Z_U8 గుర్తులేని దీర్ఘ దీర్ఘ
#  elif (ULONG_LONG_MAX == 0xffffffffffffffff)
#    define Z_U8 గుర్తులేని దీర్ఘ దీర్ఘ
#  elif (UINT_MAX == 0xffffffffffffffff)
#    define Z_U8 గుర్తులేని
#  endif
#అయితే_ముగింపు

బాహ్య z_const అక్షరం * స్థిరరాశి z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#నిర్వచించు ERR_సందేశం(err) z_errmsg[(err) < -6 || (err) > 2 ? 9 : 2 - (err)]

#నిర్వచించు ERR_RETURN(strm,err) \
  తిరిగి (strm->సందేశం = ERR_సందేశం(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */
#ఒకవేళ MAX_WBITS < 9 || MAX_WBITS > 15
#  error MAX_WBITS must be లోపల 9..15
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#అయితే_ముగింపు
/* default windowBits for decompression. MAX_WBITS is for compression only */

#ఒకవేళ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#లేకపోతే
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#అయితే_ముగింపు
/* default memLevel */

#నిర్వచించు STORED_ఖండం 0
#నిర్వచించు STATIC_TREES 1
#నిర్వచించు DYN_TREES    2
/* The three kinds of block type */

#నిర్వచించు MIN_సరిపోలిక  3
#నిర్వచించు MAX_సరిపోలిక  258
/* The minimum and maximum match lengths */

#నిర్వచించు PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#ఒకవేళ defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_సంకేతం  0x00
#  ifndef Z_SOLO
#    ఒకవేళ defined(__TURBOC__) || defined(__BORLANDC__)
#      ఒకవేళ (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         శూన్య _Cdecl farfree( శూన్య *ఖండం );
         శూన్య *_Cdecl farmalloc( గుర్తులేని దీర్ఘ nbytes );
#      లేకపోతే
#        include <alloc.h>
#      endif
#    లేకపోతే /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#అయితే_ముగింపు

#నిర్వచితమైతే AMIGA
#  define OS_సంకేతం  1
#అయితే_ముగింపు

#ఒకవేళ defined(VAXC) || defined(VMS)
#  define OS_సంకేతం  2
#  define F_OPEN(name, బహుళకం) \
     fopen((name), (బహుళకం), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#అయితే_ముగింపు

#నిర్వచితమైతే __370__
#  ఒకవేళ __TARGET_LIB__ < 0x20000000
#    define OS_సంకేతం 4
#  elif __TARGET_LIB__ < 0x40000000
#    define OS_సంకేతం 11
#  లేకపోతే
#    define OS_సంకేతం 8
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined(ATARI) || defined(atarist)
#  define OS_సంకేతం  5
#అయితే_ముగింపు

#నిర్వచితమైతే OS2
#  define OS_సంకేతం  6
#  ఒకవేళ defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined(MACOS)
#  define OS_సంకేతం  7
#అయితే_ముగింపు

#ఒకవేళ defined(__acorn) || defined(__riscos)
#  define OS_సంకేతం 13
#అయితే_ముగింపు

#ఒకవేళ defined(WIN32) && !defined(__CYGWIN__)
#  define OS_సంకేతం  10
#అయితే_ముగింపు

#నిర్వచితమైతే _BEOS_
#  define OS_సంకేతం  16
#అయితే_ముగింపు

#నిర్వచితమైతే __TOS_OS400__
#  define OS_సంకేతం 18
#అయితే_ముగింపు

#నిర్వచితమైతే __APPLE__
#  define OS_సంకేతం 19
#అయితే_ముగింపు

#ఒకవేళ defined(__BORLANDC__) && !defined(MSDOS)
  #సూచన warn -8004
  #సూచన warn -8008
  #సూచన warn -8066
#అయితే_ముగింపు

/* provide prototypes for these when building zlib without LFS */
#నిర్వచితం_కాకపోతే Z_LARGE64
   ZEXTERN uLong ZEXPORT adler32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT చక్రీయ_తనిఖీ_32_combine64(uLong, uLong, z_off64_t);
   ZEXTERN uLong ZEXPORT చక్రీయ_తనిఖీ_32_combine_gen64(z_off64_t);
#అయితే_ముగింపు

        /* common defaults */

#నిర్వచితం_కాకపోతే OS_సంకేతం
#  define OS_సంకేతం  3     /* assume Unix */
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే F_OPEN
#  define F_OPEN(name, బహుళకం) fopen((name), (బహుళకం))
#అయితే_ముగింపు

         /* functions */

#ఒకవేళ defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#అయితే_ముగింపు
#ఒకవేళ defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#అయితే_ముగింపు
#ఒకవేళ defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#అయితే_ముగింపు
#నిర్వచితమైతే HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  లేకపోతే
#    define zmemcpy జ్ఞాపకం_నకలు
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#లేకపోతే
   శూన్య ZLIB_అంతర్గత zmemcpy(శూన్య FAR *, స్థిరరాశి శూన్య FAR *, z_కొలత_t);
   పూర్ణాంకం ZLIB_అంతర్గత zmemcmp(స్థిరరాశి శూన్య FAR *, స్థిరరాశి శూన్య FAR *, z_కొలత_t);
   శూన్య ZLIB_అంతర్గత zmemzero(శూన్య FAR *, z_కొలత_t);
#అయితే_ముగింపు

/* Diagnostic functions */
#నిర్వచితమైతే ZLIB_DEBUG
#  include <stdio.h>
   బాహ్య పూర్ణాంకం ZLIB_అంతర్గత z_verbose;
   బాహ్య శూన్య ZLIB_అంతర్గత z_error(అక్షరం *m);
#  define Assert(cond,సందేశం) {ఒకవేళ(!(cond)) z_error(సందేశం);}
#  define Trace(x) {ఒకవేళ (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {ఒకవేళ (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {ఒకవేళ (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {ఒకవేళ (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {ఒకవేళ (z_verbose>1 && (c)) fprintf x ;}
#లేకపోతే
#  define Assert(cond,సందేశం)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే Z_SOLO
   voidpf ZLIB_అంతర్గత zcalloc(voidpf opaque, గుర్తులేని వస్తువులు_2,
                                గుర్తులేని కొలత);
   శూన్య ZLIB_అంతర్గత zcfree(voidpf opaque, voidpf ptr);
#అయితే_ముగింపు

#నిర్వచించు ZALLOC(strm, వస్తువులు_2, కొలత) \
           (*((strm)->zalloc))((strm)->opaque, (వస్తువులు_2), (కొలత))
#నిర్వచించు ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#నిర్వచించు TRY_FREE(s, p) {ఒకవేళ (p) ZFREE(s, p);}

/* Reverse the bytes in a 32-bit value */
#నిర్వచించు ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#నిర్వచితమైతే Z_ONCE
/*
  Create a local z_once() function depending on the availability of atomics.
 */

/* Check for the availability of atomics. */
#ఒకవేళ defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#అన్తర్భావ <stdatomic.h>
రకనిర్వచనం నిర్మాణం {
    atomic_flag begun;
    atomic_int పూర్తి_2;
} z_once_t;
#నిర్వచించు Z_ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  Z_ONCE_INIT.
 */
local శూన్య z_once(z_once_t *state, శూన్య (*init)(శూన్య)) {
    ఒకవేళ (!atomic_ఎక్కించు(&state->పూర్తి_2)) {
        ఒకవేళ (atomic_flag_test_and_set(&state->begun))
            అయితే (!atomic_ఎక్కించు(&state->పూర్తి_2))
                ;
        లేకపోతే {
            init();
            atomic_store(&state->పూర్తి_2, 1);
        }
    }
}

#లేకపోతే   /* no atomics */

#warning zlib not thread-సురక్షిత

రకనిర్వచనం నిర్మాణం z_once_s {
    అస్థిర పూర్ణాంకం begun;
    అస్థిర పూర్ణాంకం పూర్తి_2;
} z_once_t;
#నిర్వచించు Z_ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to limit the period of
   vulnerability. */
local పూర్ణాంకం test_and_సమితి(పూర్ణాంకం అస్థిర *flag) {
    పూర్ణాంకం was;

    was = *flag;
    *flag = 1;
    తిరిగి was;
}

/* Run the provided init() function once. This is not thread-safe. */
local శూన్య z_once(z_once_t *state, శూన్య (*init)(శూన్య)) {
    ఒకవేళ (!state->పూర్తి_2) {
        ఒకవేళ (test_and_సమితి(&state->begun))
            అయితే (!state->పూర్తి_2)
                ;
        లేకపోతే {
            init();
            state->పూర్తి_2 = 1;
        }
    }
}

#అయితే_ముగింపు /* ?atomics */

#అయితే_ముగింపు /* Z_ONCE */

#అయితే_ముగింపు /* ZUTIL_H */
