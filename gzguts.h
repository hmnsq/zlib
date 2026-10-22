/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#اگر_تعریف_شدہ LARGEFILE64_ماخذ
#  ifndef LARGEFILE_ماخذ
#    define LARGEFILE_ماخذ 1
#  endif
#  undef FILE_انحراف_بٹ
#  undef وقت_بٹ
#اگر_ختم

#اگر_تعریف_شدہ HAVE_HIDDEN
#  define ZLIB_اندرونی __attribute__((visibility ("hidden")))
#ورنہ
#  define ZLIB_اندرونی
#اگر_ختم

#اگر defined(_WIN32)
#  ifndef WIN32_LEAN_AND_اوسط
#    define WIN32_LEAN_AND_اوسط
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#اگر_ختم

#شامل <stdio.h>
#شامل "zlib.h"
#اگر_تعریف_شدہ STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#اگر_ختم

#اگر_غیر_تعریف_شدہ _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#اگر_ختم
#شامل <fcntl.h>

#اگر_تعریف_شدہ _WIN32
#  include <stddef.h>
#اگر_ختم

#اگر defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#اگر_ختم

#اگر defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#اگر_ختم

#اگر_تعریف_شدہ NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#اگر_ختم

#اگر defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#اگر_ختم

#اگر defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#اگر_ختم

#اگر defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#اگر_ختم

#اگر_غیر_تعریف_شدہ HAVE_VSNPRINTF
#  اگر !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    اگر !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
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
#اگر_ختم

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#اگر defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#اگر_ختم

#اگر_غیر_تعریف_شدہ local
#  define local ساکن
#اگر_ختم
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#اگر_غیر_تعریف_شدہ STDC
  خارجی voidp  حافظہ_دو(uInt سائز);
  خارجی خالی   آزاد(voidpf ptr);
#اگر_ختم

/* get errno and strerror definition */
#اگر defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)Get_آخری_Error())
#ورنہ
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  ورنہ
#    define zstrerror() "stdio error (consult errno)"
#  endif
#اگر_ختم

#اگر_تعریف_شدہ EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  ورنہ
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#ورنہ
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  ورنہ
#    define BLOCKED(e) (0)
#  endif
#اگر_ختم

/* provide prototypes for these when building zlib without LFS */
#اگر !defined(LARGEFILE64_ماخذ) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ثابت حرف *, ثابت حرف *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, صحیح);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#اگر_ختم

/* default memLevel */
#اگر MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ورنہ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#اگر_ختم

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#تعریف_کرو GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#تعریف_کرو GZ_NONE 0
#تعریف_کرو GZ_READ 7247
#تعریف_کرو GZ_WRITE 31153
#تعریف_کرو GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#تعریف_کرو LOOK 0      /* look for a gzip header */
#تعریف_کرو COPY 1      /* copy input directly */
#تعریف_کرو GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
قسمتعریف ساخت {
        /* exposed contents for gzgetc() macro */
    ساخت gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    صحیح منوال;               /* see gzip modes above */
    صحیح fd;                 /* file descriptor */
    حرف *راستہ;             /* path or fd for error messages */
    غیرعلامتی سائز;          /* buffer size, zero if not allocated yet */
    غیرعلامتی want;          /* requested buffer size, default is GZBUFSIZE */
    غیرعلامتی حرف *اندر;      /* input buffer (double-sized when writing) */
    غیرعلامتی حرف *برآمد;     /* output buffer (double-sized when reading) */
    صحیح براہ_راست;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    صحیح junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    صحیح how;                /* 0: get header, 1: copy, 2: decompress */
    صحیح again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t آغاز;        /* where the gzip data started, for rewinding */
    صحیح eof;                /* true if end of input file reached */
    صحیح past;               /* true if read requested past end */
        /* just for writing */
    صحیح level;              /* compression level */
    صحیح strategy;           /* compression strategy */
    صحیح reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    صحیح err;                /* error code */
    حرف *پیغام;              /* error message */
        /* zlib inflate or deflate stream */
    z_دھارا strm;          /* stream structure in-place (not a pointer) */
} gz_state;
قسمتعریف gz_state FAR *gz_statep;

/* shared functions */
خالی ZLIB_اندرونی gz_error(gz_statep, صحیح, ثابت حرف *);
#اگر defined UNDER_CE
حرف ZLIB_اندرونی *gz_strwinerror(DWORD error);
#اگر_ختم

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
غیرعلامتی ZLIB_اندرونی gz_intmax(خالی);
#تعریف_کرو GT_OFF(x) (حجم(صحیح) == حجم(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#اگر_غیر_تعریف_شدہ CHAR_BIT
#  define CHAR_BIT 8
#اگر_ختم
#تعریف_کرو GZ_OFF_MAX \
    ((((z_off64_t)1 << (حجم(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
