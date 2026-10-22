/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LARGEFILE64_ಮೂಲ
#  ifndef LARGEFILE_ಮೂಲ
#    define LARGEFILE_ಮೂಲ 1
#  endif
#  undef FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ಟುಗಳು
#  undef ಸಮಯ_ಬಿಟ್ಟುಗಳು
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_HIDDEN
#  define ZLIB_ಆಂತರಿಕ __attribute__((visibility ("hidden")))
#ಇಲ್ಲದಿದ್ದರೆ
#  define ZLIB_ಆಂತರಿಕ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_WIN32)
#  ifndef WIN32_LEAN_AND_ಸರಾಸರಿ
#    define WIN32_LEAN_AND_ಸರಾಸರಿ
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <fcntl.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#  include <stddef.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_WIN32) && !defined(WIDECHAR)
#  define WIDECHAR
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_VSNPRINTF
#  ಆದರೆ !defined(NO_vsnprintf) && \
      (defined(MSDOS) || defined(__TURBOC__) || defined(__SASC) || \
       defined(VMS) || defined(__OS400) || defined(__MVS__))
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    ಆದರೆ !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
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
#ಆದರೆ_ಅಂತ್ಯ

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#ಆದರೆ defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ local
#  define local ಸ್ಥಿರ
#ಆದರೆ_ಅಂತ್ಯ
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDC
  ಬಾಹ್ಯ voidp  ಸ್ಮರಣೆ_ಕೊಡು(uInt ಗಾತ್ರ_2);
  ಬಾಹ್ಯ ಶೂನ್ಯ   ಬಿಡುಗಡೆ(voidpf ptr);
#ಆದರೆ_ಅಂತ್ಯ

/* get errno and strerror definition */
#ಆದರೆ defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)Get_ಕೊನೆಯ_Error())
#ಇಲ್ಲದಿದ್ದರೆ
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  ಇಲ್ಲದಿದ್ದರೆ
#    define zstrerror() "stdio error (consult errno)"
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ EAGAIN
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EAGAIN || (e) == EWOULDBLOCK)
#  ಇಲ್ಲದಿದ್ದರೆ
#    define BLOCKED(e) ((e) == EAGAIN)
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
#  ifdef EWOULDBLOCK
#    define BLOCKED(e) ((e) == EWOULDBLOCK)
#  ಇಲ್ಲದಿದ್ದರೆ
#    define BLOCKED(e) (0)
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/* provide prototypes for these when building zlib without LFS */
#ಆದರೆ !defined(LARGEFILE64_ಮೂಲ) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
    ZEXTERN z_off64_t ZEXPORT gzseek64(gzFile, z_off64_t, ಪೂರ್ಣಾಂಕ);
    ZEXTERN z_off64_t ZEXPORT gztell64(gzFile);
    ZEXTERN z_off64_t ZEXPORT gzoffset64(gzFile);
#ಆದರೆ_ಅಂತ್ಯ

/* default memLevel */
#ಆದರೆ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ಇಲ್ಲದಿದ್ದರೆ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#ಆದರೆ_ಅಂತ್ಯ

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#ವ್ಯಾಖ್ಯಾನಿಸು GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#ವ್ಯಾಖ್ಯಾನಿಸು GZ_NONE 0
#ವ್ಯಾಖ್ಯಾನಿಸು GZ_READ 7247
#ವ್ಯಾಖ್ಯಾನಿಸು GZ_WRITE 31153
#ವ್ಯಾಖ್ಯಾನಿಸು GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#ವ್ಯಾಖ್ಯಾನಿಸು LOOK 0      /* look for a gzip header */
#ವ್ಯಾಖ್ಯಾನಿಸು COPY 1      /* copy input directly */
#ವ್ಯಾಖ್ಯಾನಿಸು GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
        /* exposed contents for gzgetc() macro */
    ರಚನೆ gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    ಪೂರ್ಣಾಂಕ ಬಹುಲಕ;               /* see gzip modes above */
    ಪೂರ್ಣಾಂಕ fd;                 /* file descriptor */
    ಅಕ್ಷರ *ಮಾರ್ಗ;             /* path or fd for error messages */
    ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2;          /* buffer size, zero if not allocated yet */
    ಚಿಹ್ನೆರಹಿತ want;          /* requested buffer size, default is GZBUFSIZE */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ;      /* input buffer (double-sized when writing) */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಹೊರಹೋಗು;     /* output buffer (double-sized when reading) */
    ಪೂರ್ಣಾಂಕ ನೇರ;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    ಪೂರ್ಣಾಂಕ junk;               /* -1 = start, 1 = junk candidate, 0 = in gzip */
    ಪೂರ್ಣಾಂಕ how;                /* 0: get header, 1: copy, 2: decompress */
    ಪೂರ್ಣಾಂಕ again;              /* true if EAGAIN or EWOULDBLOCK on last i/o */
    z_off64_t ಆರಂಭ;        /* where the gzip data started, for rewinding */
    ಪೂರ್ಣಾಂಕ eof;                /* true if end of input file reached */
    ಪೂರ್ಣಾಂಕ past;               /* true if read requested past end */
        /* just for writing */
    ಪೂರ್ಣಾಂಕ level;              /* compression level */
    ಪೂರ್ಣಾಂಕ strategy;           /* compression strategy */
    ಪೂರ್ಣಾಂಕ reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
        /* error information */
    ಪೂರ್ಣಾಂಕ err;                /* error code */
    ಅಕ್ಷರ *ಸಂದೇಶ;              /* error message */
        /* zlib inflate or deflate stream */
    z_ಹರಿವು strm;          /* stream structure in-place (not a pointer) */
} gz_state;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ gz_state FAR *gz_statep;

/* shared functions */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ gz_error(gz_statep, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
#ಆದರೆ defined UNDER_CE
ಅಕ್ಷರ ZLIB_ಆಂತರಿಕ *gz_strwinerror(DWORD error);
#ಆದರೆ_ಅಂತ್ಯ

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
ಚಿಹ್ನೆರಹಿತ ZLIB_ಆಂತರಿಕ gz_intmax(ಶೂನ್ಯ);
#ವ್ಯಾಖ್ಯಾನಿಸು GT_OFF(x) (ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) == ಗಾತ್ರ(z_off64_t) && (x) > gz_intmax())

/* largest value representable in z_off64_t */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CHAR_BIT
#  define CHAR_BIT 8
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು GZ_OFF_MAX \
    ((((z_off64_t)1 << (ಗಾತ್ರ(z_off64_t) * CHAR_BIT - 2)) - 1) * 2 + 1)
