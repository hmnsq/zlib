/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LARGEFILE64_ਸਰੋਤ
#  ifndef LARGEFILE_ਸਰੋਤ
#    define LARGEFILE_ਸਰੋਤ 1
#  endif
#  undef FILE_ਵਿੱਥ_ਬਿੱਟ
#  undef ਸਮਾਂ_ਬਿੱਟ
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_HIDDEN
#  define ZLIB_ਅੰਦਰੂਨੀ __attribute__((visibility ("hidden")))
#ਨਹੀਂਤਾਂ
#  define ZLIB_ਅੰਦਰੂਨੀ
#ਜੇ_ਅੰਤ

#ਜੇ defined(_WIN32)
#  ifndef WIN32_LEAN_AND_ਔਸਤ
#    define WIN32_LEAN_AND_ਔਸਤ
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#  include <stddef.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#ਜੇ_ਅੰਤ

#ਜੇ defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_VSNPRINTF
#  ਜੇ !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    ਜੇ !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
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
#ਜੇ_ਅੰਤ

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#ਜੇ defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ local
#  define local ਸਥਿਰ
#ਜੇ_ਅੰਤ
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDC
  ਬਾਹਰੀ voidp  ਸਮ੍ਰਿਤੀ_ਦਿਓ(uInt ਮਾਪ);
  ਬਾਹਰੀ ਖਾਲੀ   ਮੁਕਤ(voidpf ptr);
#ਜੇ_ਅੰਤ

/* get errno and strerror definition */
#ਜੇ defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)Get_ਆਖਰੀ_Error())
#ਨਹੀਂਤਾਂ
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  ਨਹੀਂਤਾਂ
#    define zstrerror() "stdio error (consult errno)"
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  ਨਹੀਂਤਾਂ
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#ਨਹੀਂਤਾਂ
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  ਨਹੀਂਤਾਂ
#    define BLOCKED(e) (0)
#  endif
#ਜੇ_ਅੰਤ

/* provide prototypes for these when building zlib without LFS */
#ਜੇ !defined(LARGEFILE64_ਸਰੋਤ) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ਅਚਲ ਅੱਖਰ *, ਅਚਲ ਅੱਖਰ *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, ਪੂਰਨ);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#ਜੇ_ਅੰਤ

/* default memLevel */
#ਜੇ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ਨਹੀਂਤਾਂ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#ਜੇ_ਅੰਤ

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZ_NONE 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZ_READ 7247
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZ_WRITE 31153
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOOK 0      /* look for a gzip header */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COPY 1      /* copy input directly */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
        /* exposed contents for gzgetc() macro */
    ਬਣਤਰ gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    ਪੂਰਨ ਬਹੁਲਕ;               /* see gzip modes above */
    ਪੂਰਨ fd;                 /* file descriptor */
    ਅੱਖਰ *ਰਸਤਾ;             /* path or fd for error messages */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ;          /* buffer size, zero if not allocated yet */
    ਬਿਨਾਨਿਸ਼ਾਨੀ want;          /* requested buffer size, default is GZBUFSIZE */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ;      /* input buffer (double-sized when writing) */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਾਹਰਲਾ;     /* output buffer (double-sized when reading) */
    ਪੂਰਨ ਸਿੱਧਾ;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    ਪੂਰਨ junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    ਪੂਰਨ how;                /* 0: get header, 1: copy, 2: decompress */
    ਪੂਰਨ again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t ਸ਼ੁਰੂ;        /* where the gzip data started, for rewinding */
    ਪੂਰਨ eof;                /* true if end of input file reached */
    ਪੂਰਨ past;               /* true if read requested past end */
        /* just for writing */
    ਪੂਰਨ level;              /* compression level */
    ਪੂਰਨ strategy;           /* compression strategy */
    ਪੂਰਨ reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    ਪੂਰਨ err;                /* error code */
    ਅੱਖਰ *ਸੁਨੇਹਾ;              /* error message */
        /* zlib inflate or deflate stream */
    z_ਧਾਰਾ strm;          /* stream structure in-place (not a pointer) */
} gz_state;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ gz_state FAR *gz_statep;

/* shared functions */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ gz_error(gz_statep, ਪੂਰਨ, ਅਚਲ ਅੱਖਰ *);
#ਜੇ defined UNDER_CE
ਅੱਖਰ ZLIB_ਅੰਦਰੂਨੀ *gz_strwinerror(DWORD error);
#ਜੇ_ਅੰਤ

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
ਬਿਨਾਨਿਸ਼ਾਨੀ ZLIB_ਅੰਦਰੂਨੀ gz_intmax(ਖਾਲੀ);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GT_OFF(x) (ਆਕਾਰ(ਪੂਰਨ) == ਆਕਾਰ(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CHAR_BIT
#  define CHAR_BIT 8
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZ_OFF_MAX \
    ((((z_off64_t)1 << (ਆਕਾਰ(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
