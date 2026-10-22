/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifdef _LARGEFILE64_SOURCE
#  ifndef _LARGEFILE_SOURCE
#    define _LARGEFILE_SOURCE 1
#  endif
#  undef _FILE_OFFSET_BITS
#  undef _TIME_BITS
#endif

#ifdef HAVE_HIDDEN
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#ଅନ୍ୟଥା
#  define ZLIB_INTERNAL
#endif

#ଯଦି defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#endif

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ "zlib.h"
#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#endif

#ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#endif
#ଅନ୍ତର୍ଭାଵ <fcntl.h>

#ifdef _WIN32
#  include <stddef.h>
#endif

#ଯଦି defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#endif

#ଯଦି defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#endif

#ifdef NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#endif

#ଯଦି defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ଯଦି defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ଯଦି defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ifndef HAVE_VSNPRINTF
#  ଯଦି !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    ଯଦି !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
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
#endif

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#ଯଦି defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#endif

#ifndef local
#  define local ସ୍ଥିର
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#ifndef STDC
  ବାହ୍ୟ voidp  malloc(uInt size);
  ବାହ୍ୟ ଶୂନ୍ୟ   ମୁକ୍ତ(voidpf ptr);
#endif

/* get errno and strerror definition */
#ଯଦି defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)GetLastError())
#ଅନ୍ୟଥା
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  ଅନ୍ୟଥା
#    define zstrerror() "stdio error (consult errno)"
#  endif
#endif

#ifdef EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  ଅନ୍ୟଥା
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#ଅନ୍ୟଥା
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  ଅନ୍ୟଥା
#    define BLOCKED(e) (0)
#  endif
#endif

/* provide prototypes for these when building zlib without LFS */
#ଯଦି !defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ଅଚଳ ଅକ୍ଷର *, ଅଚଳ ଅକ୍ଷର *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#endif

/* default memLevel */
#ଯଦି MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ଅନ୍ୟଥା
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#define GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#define GZ_NONE 0
#define GZ_READ 7247
#define GZ_WRITE 31153
#define GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#define LOOK 0      /* look for a gzip header */
#define COPY 1      /* copy input directly */
#define GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
        /* exposed contents for gzgetc() macro */
    ସଂରଚନା gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode;               /* see gzip modes above */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;                 /* file descriptor */
    ଅକ୍ଷର *path;             /* path or fd for error messages */
    unsigned size;          /* buffer size, zero if not allocated yet */
    unsigned want;          /* requested buffer size, default is GZBUFSIZE */
    unsigned ଅକ୍ଷର *in;      /* input buffer (double-sized when writing) */
    unsigned ଅକ୍ଷର *out;     /* output buffer (double-sized when reading) */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା direct;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା how;                /* 0: get header, 1: copy, 2: decompress */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t start;        /* where the gzip data started, for rewinding */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eof;                /* true if end of input file reached */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା past;               /* true if read requested past end */
        /* just for writing */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା level;              /* compression level */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା strategy;           /* compression strategy */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;                /* error code */
    ଅକ୍ଷର *ସନ୍ଦେଶ;              /* error message */
        /* zlib inflate or deflate stream */
    z_stream strm;          /* stream structure in-place (not a pointer) */
} gz_state;
ପ୍ରକାରବ୍ୟାଖ୍ୟା gz_state FAR *gz_statep;

/* shared functions */
ଶୂନ୍ୟ ZLIB_INTERNAL gz_error(gz_statep, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅଚଳ ଅକ୍ଷର *);
#ଯଦି defined UNDER_CE
ଅକ୍ଷର ZLIB_INTERNAL *gz_strwinerror(DWORD error);
#endif

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
unsigned ZLIB_INTERNAL gz_intmax(ଶୂନ୍ୟ);
#define GT_OFF(x) (ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) == ଆକାର(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#ifndef CHAR_BIT
#  define CHAR_BIT 8
#endif
#define GZ_OFF_MAX \
    ((((z_off64_t)1 << (ଆକାର(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
