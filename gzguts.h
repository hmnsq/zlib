/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#यदि_परिभाषितम् _LARGEFILE64_SOURCE
#  ifndef _LARGEFILE_SOURCE
#    define _LARGEFILE_SOURCE 1
#  endif
#  undef FILE_विस्थापनम्_द्वयङ्काः
#  undef TIME_द्वयङ्काः
#यदि_अन्तः

#यदि_परिभाषितम् HAVE_HIDDEN
#  define ZLIB_आन्तरिकम् __attribute__((visibility ("hidden")))
#अन्यथा
#  define ZLIB_आन्तरिकम्
#यदि_अन्तः

#यदि defined(_WIN32)
#  ifndef WIN32_LEAN_AND_माध्यम्
#    define WIN32_LEAN_AND_माध्यम्
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#यदि_अन्तः

#अन्तर्भाव <stdio.h>
#अन्तर्भाव "zlib.h"
#यदि_परिभाषितम् STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#यदि_अन्तः

#यदि_अपरिभाषितम् _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#यदि_अन्तः
#अन्तर्भाव <fcntl.h>

#यदि_परिभाषितम् _WIN32
#  include <stddef.h>
#यदि_अन्तः

#यदि defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#यदि_अन्तः

#यदि defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#यदि_अन्तः

#यदि_परिभाषितम् NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#यदि_अन्तः

#यदि defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#यदि_अन्तः

#यदि defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#यदि_अन्तः

#यदि defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#यदि_अन्तः

#यदि_अपरिभाषितम् HAVE_VSNPRINTF
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
#यदि_अन्तः

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#यदि defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#यदि_अन्तः

#यदि_अपरिभाषितम् local
#  define local स्थिर
#यदि_अन्तः
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#यदि_अपरिभाषितम् STDC
  बाह्य voidp  स्मृतिं_देहि(uInt आकार);
  बाह्य शून्य   मुक्त(voidpf ptr);
#यदि_अन्तः

/* get errno and strerror definition */
#यदि defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)Get_चरमः_Error())
#अन्यथा
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  अन्यथा
#    define zstrerror() "stdio error (consult errno)"
#  endif
#यदि_अन्तः

#यदि_परिभाषितम् EAGAIN
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
#यदि_अन्तः

/* provide prototypes for these when building zlib without LFS */
#यदि !defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ध्रुव वर्ण *, ध्रुव वर्ण *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, पूर्णाङ्क);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#यदि_अन्तः

/* default memLevel */
#यदि MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#यदि_अन्तः

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#परिभाषय GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#परिभाषय GZ_NONE 0
#परिभाषय GZ_READ 7247
#परिभाषय GZ_WRITE 31153
#परिभाषय GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#परिभाषय LOOK 0      /* look for a gzip header */
#परिभाषय COPY 1      /* copy input directly */
#परिभाषय GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
प्रकारनिर्देश संरचना {
        /* exposed contents for gzgetc() macro */
    संरचना gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    पूर्णाङ्क बहुलकम्;               /* see gzip modes above */
    पूर्णाङ्क fd;                 /* file descriptor */
    वर्ण *मार्गः;             /* path or fd for error messages */
    अचिह्नित आकार;          /* buffer size, zero if not allocated yet */
    अचिह्नित want;          /* requested buffer size, default is GZBUFSIZE */
    अचिह्नित वर्ण *अन्तर्;      /* input buffer (double-sized when writing) */
    अचिह्नित वर्ण *बहिर्गमनम्;     /* output buffer (double-sized when reading) */
    पूर्णाङ्क प्रत्यक्षम्;             /* 0 if processing gzip, 1 if transparent */
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
    वर्ण *सन्देशः;              /* error message */
        /* zlib inflate or deflate stream */
    z_प्रवाहः strm;          /* stream structure in-place (not a pointer) */
} gz_state;
प्रकारनिर्देश gz_state FAR *gz_statep;

/* shared functions */
शून्य ZLIB_आन्तरिकम् gz_error(gz_statep, पूर्णाङ्क, ध्रुव वर्ण *);
#यदि defined UNDER_CE
वर्ण ZLIB_आन्तरिकम् *gz_strwinerror(DWORD error);
#यदि_अन्तः

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
अचिह्नित ZLIB_आन्तरिकम् gz_intmax(शून्य);
#परिभाषय GT_OFF(x) (परिमाण(पूर्णाङ्क) == परिमाण(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#यदि_अपरिभाषितम् CHAR_BIT
#  define CHAR_BIT 8
#यदि_अन्तः
#परिभाषय GZ_OFF_MAX \
    ((((z_off64_t)1 << (परिमाण(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
