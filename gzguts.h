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
#ಅನ್ಯಥಾ
#  define ZLIB_INTERNAL
#endif

#ಯದಿ defined(_WIN32)
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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#endif

#ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#endif
#ಅನ್ತರ್ಭಾವ <fcntl.h>

#ifdef _WIN32
#  include <stddef.h>
#endif

#ಯದಿ defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#endif

#ಯದಿ defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#endif

#ifdef NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#endif

#ಯದಿ defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ಯದಿ defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ಯದಿ defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ifndef HAVE_VSNPRINTF
#  ಯದಿ !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    ಯದಿ !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
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
#ಯದಿ defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#endif

#ifndef local
#  define local ಸ್ಥಿರ
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#ifndef STDC
  ಬಾಹ್ಯ voidp  malloc(uInt size);
  ಬಾಹ್ಯ ಶೂನ್ಯ   ಮುಕ್ತ(voidpf ptr);
#endif

/* get errno and strerror definition */
#ಯದಿ defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)GetLastError())
#ಅನ್ಯಥಾ
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  ಅನ್ಯಥಾ
#    define zstrerror() "stdio error (consult errno)"
#  endif
#endif

#ifdef EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  ಅನ್ಯಥಾ
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#ಅನ್ಯಥಾ
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  ಅನ್ಯಥಾ
#    define BLOCKED(e) (0)
#  endif
#endif

/* provide prototypes for these when building zlib without LFS */
#ಯದಿ !defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ಧ್ರುವ ವರ್ಣ *, ಧ್ರುವ ವರ್ಣ *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, ಪೂರ್ಣಾಙ್ಕ);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#endif

/* default memLevel */
#ಯದಿ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ಅನ್ಯಥಾ
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
        /* exposed contents for gzgetc() macro */
    ಸಂರಚನಾ gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    ಪೂರ್ಣಾಙ್ಕ mode;               /* see gzip modes above */
    ಪೂರ್ಣಾಙ್ಕ fd;                 /* file descriptor */
    ವರ್ಣ *path;             /* path or fd for error messages */
    unsigned size;          /* buffer size, zero if not allocated yet */
    unsigned want;          /* requested buffer size, default is GZBUFSIZE */
    unsigned ವರ್ಣ *in;      /* input buffer (double-sized when writing) */
    unsigned ವರ್ಣ *out;     /* output buffer (double-sized when reading) */
    ಪೂರ್ಣಾಙ್ಕ direct;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    ಪೂರ್ಣಾಙ್ಕ junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    ಪೂರ್ಣಾಙ್ಕ how;                /* 0: get header, 1: copy, 2: decompress */
    ಪೂರ್ಣಾಙ್ಕ again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t start;        /* where the gzip data started, for rewinding */
    ಪೂರ್ಣಾಙ್ಕ eof;                /* true if end of input file reached */
    ಪೂರ್ಣಾಙ್ಕ past;               /* true if read requested past end */
        /* just for writing */
    ಪೂರ್ಣಾಙ್ಕ level;              /* compression level */
    ಪೂರ್ಣಾಙ್ಕ strategy;           /* compression strategy */
    ಪೂರ್ಣಾಙ್ಕ reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    ಪೂರ್ಣಾಙ್ಕ err;                /* error code */
    ವರ್ಣ *ಸಂದೇಶ;              /* error message */
        /* zlib inflate or deflate stream */
    z_stream strm;          /* stream structure in-place (not a pointer) */
} gz_state;
ಪ್ರಕಾರನಿರ್ದೇಶ gz_state FAR *gz_statep;

/* shared functions */
ಶೂನ್ಯ ZLIB_INTERNAL gz_error(gz_statep, ಪೂರ್ಣಾಙ್ಕ, ಧ್ರುವ ವರ್ಣ *);
#ಯದಿ defined UNDER_CE
ವರ್ಣ ZLIB_INTERNAL *gz_strwinerror(DWORD error);
#endif

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
unsigned ZLIB_INTERNAL gz_intmax(ಶೂನ್ಯ);
#define GT_OFF(x) (ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) == ಪರಿಮಾಣ(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#ifndef CHAR_BIT
#  define CHAR_BIT 8
#endif
#define GZ_OFF_MAX \
    ((((z_off64_t)1 << (ಪರಿಮಾಣ(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
