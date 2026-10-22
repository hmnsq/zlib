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
#अन्यथा
#  define ZLIB_INTERNAL
#endif

#यदि defined(_WIN32)
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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव "zlib.h"
#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#endif

#ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#endif
#अन्तर्भाव <fcntl.h>

#ifdef _WIN32
#  include <stddef.h>
#endif

#यदि defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#endif

#यदि defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#endif

#ifdef NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#endif

#यदि defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#यदि defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#यदि defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ifndef HAVE_VSNPRINTF
#  यदि !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    यदि !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
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
#यदि defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#endif

#ifndef local
#  define local स्थिर
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#ifndef STDC
  बाह्य voidp  दो_स्मृति(uInt size);
  बाह्य शून्य   मुक्त(voidpf ptr);
#endif

/* get errno and strerror definition */
#यदि defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)GetLastError())
#अन्यथा
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() म_त्रुटि(त्रुटि_सं)
#  अन्यथा
#    define zstrerror() "stdio error (consult errno)"
#  endif
#endif

#ifdef EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  अन्यथा
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#अन्यथा
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  अन्यथा
#    define BLOCKED(e) (0)
#  endif
#endif

/* provide prototypes for these when building zlib without LFS */
#यदि !defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ध्रुव वर्ण *, ध्रुव वर्ण *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, पूर्णाङ्क);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#endif

/* default memLevel */
#यदि MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
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
प्रकारनिर्देश संरचना {
        /* exposed contents for gzgetc() macro */
    संरचना gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    पूर्णाङ्क mode;               /* see gzip modes above */
    पूर्णाङ्क fd;                 /* file descriptor */
    वर्ण *path;             /* path or fd for error messages */
    अचिन्हित size;          /* buffer size, zero if not allocated yet */
    अचिन्हित want;          /* requested buffer size, default is GZBUFSIZE */
    अचिन्हित वर्ण *in;      /* input buffer (double-sized when writing) */
    अचिन्हित वर्ण *out;     /* output buffer (double-sized when reading) */
    पूर्णाङ्क direct;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    पूर्णाङ्क junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    पूर्णाङ्क how;                /* 0: get header, 1: copy, 2: decompress */
    पूर्णाङ्क again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t start;        /* where the gzip data started, for rewinding */
    पूर्णाङ्क eof;                /* true if end of input file reached */
    पूर्णाङ्क past;               /* true if read requested past end */
        /* just for writing */
    पूर्णाङ्क level;              /* compression level */
    पूर्णाङ्क strategy;           /* compression strategy */
    पूर्णाङ्क reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    पूर्णाङ्क err;                /* error code */
    वर्ण *संदेश;              /* error message */
        /* zlib inflate or deflate stream */
    z_stream strm;          /* stream structure in-place (not a pointer) */
} gz_state;
प्रकारनिर्देश gz_state FAR *gz_statep;

/* shared functions */
शून्य ZLIB_INTERNAL gz_error(gz_statep, पूर्णाङ्क, ध्रुव वर्ण *);
#यदि defined UNDER_CE
वर्ण ZLIB_INTERNAL *gz_strwinerror(DWORD error);
#endif

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
अचिन्हित ZLIB_INTERNAL gz_intmax(शून्य);
#define GT_OFF(x) (परिमाण(पूर्णाङ्क) == परिमाण(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#ifndef अक्षर_बिट
#  define अक्षर_बिट 8
#endif
#define GZ_OFF_MAX \
    ((((z_off64_t)1 << (परिमाण(z_off64_t) * अक्षर_बिट - 2)) - 1) * 2 + 1)
