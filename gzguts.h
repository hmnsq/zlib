/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#اگر_تعریف_شده LARGEFILE64_منبع
#  ifndef LARGEFILE_منبع
#    define LARGEFILE_منبع 1
#  endif
#  undef FILE_جابه_جایی_بیتها
#  undef زمان_بیتها
#پایان_اگر

#اگر_تعریف_شده HAVE_HIDDEN
#  define ZLIB_داخلی __attribute__((visibility ("hidden")))
#وإلا
#  define ZLIB_داخلی
#پایان_اگر

#إذا defined(_WIN32)
#  ifndef WIN32_LEAN_AND_میانگین
#    define WIN32_LEAN_AND_میانگین
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#پایان_اگر

#ضمن <stdio.h>
#ضمن "zlib.h"
#اگر_تعریف_شده STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#پایان_اگر

#اگر_تعریف_نشده _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#پایان_اگر
#ضمن <fcntl.h>

#اگر_تعریف_شده _WIN32
#  include <stddef.h>
#پایان_اگر

#إذا defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#پایان_اگر

#إذا defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#پایان_اگر

#اگر_تعریف_شده NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#پایان_اگر

#إذا defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#پایان_اگر

#إذا defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#پایان_اگر

#إذا defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#پایان_اگر

#اگر_تعریف_نشده HAVE_VSNPRINTF
#  إذا !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    إذا !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
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
#پایان_اگر

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#إذا defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#پایان_اگر

#اگر_تعریف_نشده local
#  define local ساكن
#پایان_اگر
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#اگر_تعریف_نشده STDC
  خارجي voidp  حافظه_بده(uInt اندازه);
  خارجي فراغ   حرر(voidpf ptr);
#پایان_اگر

/* get errno and strerror definition */
#إذا defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)Get_آخرین_Error())
#وإلا
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  وإلا
#    define zstrerror() "stdio error (consult errno)"
#  endif
#پایان_اگر

#اگر_تعریف_شده EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  وإلا
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#وإلا
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  وإلا
#    define BLOCKED(e) (0)
#  endif
#پایان_اگر

/* provide prototypes for these when building zlib without LFS */
#إذا !defined(LARGEFILE64_منبع) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ثابت حرف *, ثابت حرف *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, صحيح);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#پایان_اگر

/* default memLevel */
#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#پایان_اگر

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#تعریف_کن GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#تعریف_کن GZ_NONE 0
#تعریف_کن GZ_READ 7247
#تعریف_کن GZ_WRITE 31153
#تعریف_کن GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#تعریف_کن LOOK 0      /* look for a gzip header */
#تعریف_کن COPY 1      /* copy input directly */
#تعریف_کن GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
تعريفنوع بنية {
        /* exposed contents for gzgetc() macro */
    بنية gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    صحيح نما;               /* see gzip modes above */
    صحيح fd;                 /* file descriptor */
    حرف *مسیر;             /* path or fd for error messages */
    بیعلامت اندازه;          /* buffer size, zero if not allocated yet */
    بیعلامت want;          /* requested buffer size, default is GZBUFSIZE */
    بیعلامت حرف *درون;      /* input buffer (double-sized when writing) */
    بیعلامت حرف *بیرون;     /* output buffer (double-sized when reading) */
    صحيح مستقیم;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    صحيح junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    صحيح how;                /* 0: get header, 1: copy, 2: decompress */
    صحيح again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t شروع;        /* where the gzip data started, for rewinding */
    صحيح eof;                /* true if end of input file reached */
    صحيح past;               /* true if read requested past end */
        /* just for writing */
    صحيح level;              /* compression level */
    صحيح strategy;           /* compression strategy */
    صحيح reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    صحيح err;                /* error code */
    حرف *پیام;              /* error message */
        /* zlib inflate or deflate stream */
    z_جریان strm;          /* stream structure in-place (not a pointer) */
} gz_state;
تعريفنوع gz_state FAR *gz_statep;

/* shared functions */
فراغ ZLIB_داخلی gz_error(gz_statep, صحيح, ثابت حرف *);
#إذا defined UNDER_CE
حرف ZLIB_داخلی *gz_strwinerror(DWORD error);
#پایان_اگر

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
بیعلامت ZLIB_داخلی gz_intmax(فراغ);
#تعریف_کن GT_OFF(x) (حجم(صحيح) == حجم(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#اگر_تعریف_نشده CHAR_BIT
#  define CHAR_BIT 8
#پایان_اگر
#تعریف_کن GZ_OFF_MAX \
    ((((z_off64_t)1 << (حجم(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
