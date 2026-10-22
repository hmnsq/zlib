/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#నిర్వచితమైతే LARGEFILE64_ఆధారం
#  ifndef LARGEFILE_ఆధారం
#    define LARGEFILE_ఆధారం 1
#  endif
#  undef FILE_స్థానభ్రంశం_బిట్లు
#  undef సమయం_బిట్లు
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_HIDDEN
#  define ZLIB_అంతర్గత __attribute__((visibility ("hidden")))
#లేకపోతే
#  define ZLIB_అంతర్గత
#అయితే_ముగింపు

#ఒకవేళ defined(_WIN32)
#  ifndef WIN32_LEAN_AND_సగటు
#    define WIN32_LEAN_AND_సగటు
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#అయితే_ముగింపు

#అన్తర్భావ <stdio.h>
#అన్తర్భావ "zlib.h"
#నిర్వచితమైతే STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#అయితే_ముగింపు
#అన్తర్భావ <fcntl.h>

#నిర్వచితమైతే _WIN32
#  include <stddef.h>
#అయితే_ముగింపు

#ఒకవేళ defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#అయితే_ముగింపు

#ఒకవేళ defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#అయితే_ముగింపు

#నిర్వచితమైతే NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#అయితే_ముగింపు

#ఒకవేళ defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే HAVE_VSNPRINTF
#  ఒకవేళ !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    ఒకవేళ !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
#      ifndef vsnprintf
#        define vsnprintf _vsnprintf
#      endif
#    endif
#  elif !defined(__STDC_VERSION__) || __STDC_VERSION__-0 < 199901L
/* Otherwise if C89/90, assume no C99 snprintf() or vsnprintf() */
#    ifndef NO_snprintf
#      define NO_snprintf
#    endif
#    ifndef NO_vsnprintf
#      define NO_vsnprintf
#    endif
#  endif
#అయితే_ముగింపు

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#ఒకవేళ defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే local
#  define local స్థిర
#అయితే_ముగింపు
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#నిర్వచితం_కాకపోతే STDC
  బాహ్య voidp  జ్ఞాపకం_ఇవ్వు(uInt కొలత);
  బాహ్య శూన్య   విడుదల(voidpf ptr);
#అయితే_ముగింపు

/* get errno and strerror definition */
#ఒకవేళ defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)Get_చివరి_Error())
#లేకపోతే
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  లేకపోతే
#    define zstrerror() "stdio error (consult errno)"
#  endif
#అయితే_ముగింపు

#నిర్వచితమైతే EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  లేకపోతే
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#లేకపోతే
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  లేకపోతే
#    define BLOCKED(e) (0)
#  endif
#అయితే_ముగింపు

/* provide prototypes for these when building zlib without LFS */
#ఒకవేళ !defined(LARGEFILE64_ఆధారం) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, పూర్ణాంకం);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#అయితే_ముగింపు

/* default memLevel */
#ఒకవేళ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#లేకపోతే
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#అయితే_ముగింపు

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#నిర్వచించు GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#నిర్వచించు GZ_NONE 0
#నిర్వచించు GZ_READ 7247
#నిర్వచించు GZ_WRITE 31153
#నిర్వచించు GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#నిర్వచించు LOOK 0      /* look for a gzip header */
#నిర్వచించు COPY 1      /* copy input directly */
#నిర్వచించు GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
రకనిర్వచనం నిర్మాణం {
        /* exposed contents for gzgetc() macro */
    నిర్మాణం gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    పూర్ణాంకం బహుళకం;               /* see gzip modes above */
    పూర్ణాంకం fd;                 /* file descriptor */
    అక్షరం *మార్గం;             /* path or fd for error messages */
    గుర్తులేని కొలత;          /* buffer size, zero if not allocated yet */
    గుర్తులేని want;          /* requested buffer size, default is GZBUFSIZE */
    గుర్తులేని అక్షరం *లోపల;      /* input buffer (double-sized when writing) */
    గుర్తులేని అక్షరం *బహిర్గతం;     /* output buffer (double-sized when reading) */
    పూర్ణాంకం ప్రత్యక్ష;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    పూర్ణాంకం junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    పూర్ణాంకం how;                /* 0: get header, 1: copy, 2: decompress */
    పూర్ణాంకం again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t ప్రారంభం;        /* where the gzip data started, for rewinding */
    పూర్ణాంకం eof;                /* true if end of input file reached */
    పూర్ణాంకం past;               /* true if read requested past end */
        /* just for writing */
    పూర్ణాంకం level;              /* compression level */
    పూర్ణాంకం strategy;           /* compression strategy */
    పూర్ణాంకం reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    పూర్ణాంకం err;                /* error code */
    అక్షరం *సందేశం;              /* error message */
        /* zlib inflate or deflate stream */
    z_ప్రవాహం strm;          /* stream structure in-place (not a pointer) */
} gz_state;
రకనిర్వచనం gz_state FAR *gz_statep;

/* shared functions */
శూన్య ZLIB_అంతర్గత gz_error(gz_statep, పూర్ణాంకం, స్థిరరాశి అక్షరం *);
#ఒకవేళ defined UNDER_CE
అక్షరం ZLIB_అంతర్గత *gz_strwinerror(DWORD error);
#అయితే_ముగింపు

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
గుర్తులేని ZLIB_అంతర్గత gz_intmax(శూన్య);
#నిర్వచించు GT_OFF(x) (పరిమాణం(పూర్ణాంకం) == పరిమాణం(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#నిర్వచితం_కాకపోతే CHAR_BIT
#  define CHAR_BIT 8
#అయితే_ముగింపు
#నిర్వచించు GZ_OFF_MAX \
    ((((z_off64_t)1 << (పరిమాణం(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
