/* minigzip.c -- simulate gzip using the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * minigzip is a minimal implementation of the gzip utility. This is
 * only an example of using zlib and isn't meant to replace the
 * full-featured gzip. No attempt is made to deal with file systems
 * limiting names to 14 or 8+3 characters, etc... Error checking is
 * very limited. So use minigzip only for testing; use gzip for the
 * real thing. On MSDOS, use only on file names without extension
 * or in pipe mode.
 */

/* @(#) $Id$ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ STDC
#  include <string.h>
#  include <stdlib.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file) setmode(fileno(file), O_ದ್ವಿಮಾನ)
#ಇಲ್ಲದಿದ್ದರೆ
#  define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_ಮೂಲ)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ WIN32 /* unlink already in stdio.h for WIN32 */
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ unlink(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

ಸ್ಥಿರ ಅಕ್ಷರ *strwinerror (error)
     DWORD error;
{
    ಸ್ಥಿರ ಅಕ್ಷರ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_ಕೊನೆಯ_Error();
    DWORD chars = Format_ಸಂದೇಶ(FORMAT_ಸಂದೇಶ_FROM_SYSTEM
        | FORMAT_ಸಂದೇಶ_ALLOCATE_ಬಫರ್,
        ಖಾಲಿ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ಖಾಲಿ);
    ಆದರೆ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ಆದರೆ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ಆದರೆ (chars > ಗಾತ್ರ (buf) - 1) {
            chars = ಗಾತ್ರ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ಇಲ್ಲದಿದ್ದರೆ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    ಗಣ_ಕೊನೆಯ_Error(lasterr);
    ಹಿಂತಿರುಗಿ buf;
}

ಸ್ಥಿರ ಶೂನ್ಯ pwinerror (s)
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
{
    ಆದರೆ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_ಕೊನೆಯ_Error ()));
    ಇಲ್ಲದಿದ್ದರೆ
        fprintf(stderr, "%s\n", strwinerror(Get_ಕೊನೆಯ_Error ()));
}

#ಆದರೆ_ಅಂತ್ಯ /* UNDER_CE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು SUFFIX_LEN (ಗಾತ್ರ(GZ_SUFFIX)-1)

#ವ್ಯಾಖ್ಯಾನಿಸು BUFLEN      16384
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_NAME_LEN 1024

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAXSEG_64K
#  define local ಸ್ಥಿರ
   /* Needed for systems with limitation on stack size. */
#ಇಲ್ಲದಿದ್ದರೆ
#  define local
#ಆದರೆ_ಅಂತ್ಯ

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *string_copy(ಅಕ್ಷರ *dst, ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *src, z_ಗಾತ್ರ_t len) {
    ಆದರೆ (len == 0)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಆಗ (--len) {
        *dst = *src++;
        ಆದರೆ (*dst == 0)
            ಹಿಂತಿರುಗಿ dst;
        dst++;
    }
    *dst = 0;
    ಹಿಂತಿರುಗಿ dst;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ಆದರೆ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ *myalloc(ಶೂನ್ಯ *q, ಚಿಹ್ನೆರಹಿತ n, ಚಿಹ್ನೆರಹಿತ m) {
    (ಶೂನ್ಯ)q;
    ಹಿಂತಿರುಗಿ ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(n, m);
}

ಸ್ಥಿರ ಶೂನ್ಯ myfree(ಶೂನ್ಯ *q, ಶೂನ್ಯ *p) {
    (ಶೂನ್ಯ)q;
    ಬಿಡುಗಡೆ(p);
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ gzFile_s {
    FILE *file;
    ಪೂರ್ಣಾಂಕ ಲಿಖ;
    ಪೂರ್ಣಾಂಕ err;
    ಅಕ್ಷರ *ಸಂದೇಶ;
    z_ಹರಿವು strm;
} *gzFile;

ಸ್ಥಿರ gzFile gz_open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    gzFile gz;
    ಪೂರ್ಣಾಂಕ ret;

    gz = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ gzFile_s));
    ಆದರೆ (gz == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    gz->ಲಿಖ = strchr(ಬಹುಲಕ, 'w') != ಖಾಲಿ;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ಆದರೆ (gz->ಲಿಖ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ಇಲ್ಲದಿದ್ದರೆ {
        gz->strm.ಮುಂದಿನ_ಒಳ = 0;
        gz->strm.avail_ಒಳ = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ಆದರೆ (ret != Z_OK) {
        ಬಿಡುಗಡೆ(gz);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    gz->file = ಮಾರ್ಗ == ಖಾಲಿ ? fdopen(fd, gz->ಲಿಖ ? "wb" : "rb") :
                              fopen(ಮಾರ್ಗ, gz->ಲಿಖ ? "wb" : "rb");
    ಆದರೆ (gz->file == ಖಾಲಿ) {
        gz->ಲಿಖ ? deflate_ಕೊನೆ(&(gz->strm)) : inflate_ಕೊನೆ(&(gz->strm));
        ಬಿಡುಗಡೆ(gz);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    gz->err = 0;
    gz->ಸಂದೇಶ = "";
    ಹಿಂತಿರುಗಿ gz;
}

ಸ್ಥಿರ gzFile gzopen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    ಹಿಂತಿರುಗಿ gz_open(ಮಾರ್ಗ, -1, ಬಹುಲಕ);
}

ಸ್ಥಿರ gzFile gzdopen(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    ಹಿಂತಿರುಗಿ gz_open(ಖಾಲಿ, fd, ಬಹುಲಕ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gzwrite(gzFile gz, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, ಚಿಹ್ನೆರಹಿತ len) {
    z_ಹರಿವು *strm;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಹೊರಹೋಗು[BUFLEN];

    ಆದರೆ (gz == ಖಾಲಿ || !gz->ಲಿಖ)
        ಹಿಂತಿರುಗಿ 0;
    strm = &(gz->strm);
    strm->ಮುಂದಿನ_ಒಳ = (ಶೂನ್ಯ *)buf;
    strm->avail_ಒಳ = len;
    ಮಾಡು {
        strm->ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
        strm->avail_ಹೊರಹೋಗು = BUFLEN;
        (ಶೂನ್ಯ)deflate(strm, Z_NO_FLUSH);
        fwrite(ಹೊರಹೋಗು, 1, BUFLEN - strm->avail_ಹೊರಹೋಗು, gz->file);
    } ಆಗ (strm->avail_ಹೊರಹೋಗು == 0);
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)len;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gzread(gzFile gz, ಶೂನ್ಯ *buf, ಚಿಹ್ನೆರಹಿತ len) {
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ got;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಒಳ[1];
    z_ಹರಿವು *strm;

    ಆದರೆ (gz == ಖಾಲಿ || gz->ಲಿಖ)
        ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ (gz->err)
        ಹಿಂತಿರುಗಿ 0;
    strm = &(gz->strm);
    strm->ಮುಂದಿನ_ಹೊರಹೋಗು = (ಶೂನ್ಯ *)buf;
    strm->avail_ಹೊರಹೋಗು = len;
    ಮಾಡು {
        got = (ಚಿಹ್ನೆರಹಿತ)fread(ಒಳ, 1, 1, gz->file);
        ಆದರೆ (got == 0)
            ನಿಲ್ಲಿಸು;
        strm->ಮುಂದಿನ_ಒಳ = ಒಳ;
        strm->avail_ಒಳ = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ಆದರೆ (ret == Z_ದತ್ತಾಂಶ_ERROR) {
            gz->err = Z_ದತ್ತಾಂಶ_ERROR;
            gz->ಸಂದೇಶ = strm->ಸಂದೇಶ;
            ಹಿಂತಿರುಗಿ 0;
        }
        ಆದರೆ (ret == Z_ಹರಿವು_ಕೊನೆ)
            inflateReset(strm);
    } ಆಗ (strm->avail_ಹೊರಹೋಗು);
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)(len - strm->avail_ಹೊರಹೋಗು);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gzclose(gzFile gz) {
    z_ಹರಿವು *strm;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಹೊರಹೋಗು[BUFLEN];

    ಆದರೆ (gz == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    strm = &(gz->strm);
    ಆದರೆ (gz->ಲಿಖ) {
        strm->ಮುಂದಿನ_ಒಳ = Z_NULL;
        strm->avail_ಒಳ = 0;
        ಮಾಡು {
            strm->ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
            strm->avail_ಹೊರಹೋಗು = BUFLEN;
            (ಶೂನ್ಯ)deflate(strm, Z_ಮುಕ್ತಾಯ);
            fwrite(ಹೊರಹೋಗು, 1, BUFLEN - strm->avail_ಹೊರಹೋಗು, gz->file);
        } ಆಗ (strm->avail_ಹೊರಹೋಗು == 0);
        deflate_ಕೊನೆ(strm);
    }
    ಇಲ್ಲದಿದ್ದರೆ
        inflate_ಕೊನೆ(strm);
    fclose(gz->file);
    ಬಿಡುಗಡೆ(gz);
    ಹಿಂತಿರುಗಿ Z_OK;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *gzerror(gzFile gz, ಪೂರ್ಣಾಂಕ *err) {
    *err = gz->err;
    ಹಿಂತಿರುಗಿ gz->ಸಂದೇಶ;
}

#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಅಕ್ಷರ *prog;

/* ===========================================================================
 * Display error message and exit
 */
ಸ್ಥಿರ ಶೂನ್ಯ error(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
    fprintf(stderr, "%s: %s\n", prog, ಸಂದೇಶ);
    ನಿರ್ಗಮಿಸು(1);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gz_compress_mmap(FILE *ಒಳ, gzFile ಹೊರಹೋಗು) {
    ಪೂರ್ಣಾಂಕ len;
    ಪೂರ್ಣಾಂಕ err;
    ಪೂರ್ಣಾಂಕ ifd = fileno(ಒಳ);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    ರಚನೆ stat sb;

    /* Determine the size of the file, needed for mmap: */
    ಆದರೆ (fstat(ifd, &sb) < 0) ಹಿಂತಿರುಗಿ Z_ERRNO;
    buf_len = sb.st_size;
    ಆದರೆ (buf_len <= 0) ಹಿಂತಿರುಗಿ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ಆದರೆ (buf == (caddr_t)(-1)) ಹಿಂತಿರುಗಿ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(ಹೊರಹೋಗು, (ಅಕ್ಷರ *)buf, (ಚಿಹ್ನೆರಹಿತ)buf_len);

    ಆದರೆ (len != (ಪೂರ್ಣಾಂಕ)buf_len) error(gzerror(ಹೊರಹೋಗು, &err));

    munmap(buf, buf_len);
    fclose(ಒಳ);
    ಆದರೆ (gzclose(ಹೊರಹೋಗು) != Z_OK) error("failed gzclose");
    ಹಿಂತಿರುಗಿ Z_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ಸ್ಥಿರ ಶೂನ್ಯ gz_compress(FILE *ಒಳ, gzFile ಹೊರಹೋಗು) {
    local ಅಕ್ಷರ buf[BUFLEN];
    ಪೂರ್ಣಾಂಕ len;
    ಪೂರ್ಣಾಂಕ err;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ಆದರೆ (gz_compress_mmap(ಒಳ, ಹೊರಹೋಗು) == Z_OK) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
    ಗಾಗಿ (;;) {
        len = (ಪೂರ್ಣಾಂಕ)fread(buf, 1, ಗಾತ್ರ(buf), ಒಳ);
        ಆದರೆ (ferror(ಒಳ)) {
            perror("fread");
            ನಿರ್ಗಮಿಸು(1);
        }
        ಆದರೆ (len == 0) ನಿಲ್ಲಿಸು;

        ಆದರೆ (gzwrite(ಹೊರಹೋಗು, buf, (ಚಿಹ್ನೆರಹಿತ)len) != len) error(gzerror(ಹೊರಹೋಗು, &err));
    }
    fclose(ಒಳ);
    ಆದರೆ (gzclose(ಹೊರಹೋಗು) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ಸ್ಥಿರ ಶೂನ್ಯ gz_uncompress(gzFile ಒಳ, FILE *ಹೊರಹೋಗು) {
    local ಅಕ್ಷರ buf[BUFLEN];
    ಪೂರ್ಣಾಂಕ len;
    ಪೂರ್ಣಾಂಕ err;

    ಗಾಗಿ (;;) {
        len = gzread(ಒಳ, buf, ಗಾತ್ರ(buf));
        ಆದರೆ (len < 0) error (gzerror(ಒಳ, &err));
        ಆದರೆ (len == 0) ನಿಲ್ಲಿಸು;

        ಆದರೆ ((ಪೂರ್ಣಾಂಕ)fwrite(buf, 1, (ಚಿಹ್ನೆರಹಿತ)len, ಹೊರಹೋಗು) != len) {
            error("failed fwrite");
        }
    }
    ಆದರೆ (fclose(ಹೊರಹೋಗು)) error("failed fclose");

    ಆದರೆ (gzclose(ಒಳ) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ಸ್ಥಿರ ಶೂನ್ಯ file_compress(ಅಕ್ಷರ *file, ಅಕ್ಷರ *ಬಹುಲಕ) {
    local ಅಕ್ಷರ outfile[MAX_NAME_LEN+1], *ಕೊನೆ;
    FILE  *ಒಳ;
    gzFile ಹೊರಹೋಗು;

    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(file) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(GZ_SUFFIX) >= ಗಾತ್ರ(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        ನಿರ್ಗಮಿಸು(1);
    }

    ಕೊನೆ = string_copy(outfile, file, ಗಾತ್ರ(outfile));
    string_copy(ಕೊನೆ, GZ_SUFFIX, ಗಾತ್ರ(outfile) - (z_ಗಾತ್ರ_t)(ಕೊನೆ - outfile));

    ಒಳ = fopen(file, "rb");
    ಆದರೆ (ಒಳ == ಖಾಲಿ) {
        perror(file);
        ನಿರ್ಗಮಿಸು(1);
    }
    ಹೊರಹೋಗು = gzopen(outfile, ಬಹುಲಕ);
    ಆದರೆ (ಹೊರಹೋಗು == ಖಾಲಿ) {
        fclose(ಒಳ);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        ನಿರ್ಗಮಿಸು(1);
    }
    gz_compress(ಒಳ, ಹೊರಹೋಗು);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ಸ್ಥಿರ ಶೂನ್ಯ file_uncompress(ಅಕ್ಷರ *file) {
    local ಅಕ್ಷರ buf[MAX_NAME_LEN+1];
    ಅಕ್ಷರ *infile, *outfile;
    FILE  *ಹೊರಹೋಗು;
    gzFile ಒಳ;
    z_ಗಾತ್ರ_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(file);

    ಆದರೆ (len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(GZ_SUFFIX) >= ಗಾತ್ರ(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        ನಿರ್ಗಮಿಸು(1);
    }

    string_copy(buf, file, ಗಾತ್ರ(buf));

    ಆದರೆ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ಇಲ್ಲದಿದ್ದರೆ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, ಗಾತ್ರ(buf) - len);
    }
    ಒಳ = gzopen(infile, "rb");
    ಆದರೆ (ಒಳ == ಖಾಲಿ) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        ನಿರ್ಗಮಿಸು(1);
    }
    ಹೊರಹೋಗು = fopen(outfile, "wb");
    ಆದರೆ (ಹೊರಹೋಗು == ಖಾಲಿ) {
        gzclose(ಒಳ);
        perror(file);
        ನಿರ್ಗಮಿಸು(1);
    }

    gz_uncompress(ಒಳ, ಹೊರಹೋಗು);

    unlink(infile);
}


/* ===========================================================================
 * Usage:  minigzip [-c] [-d] [-f] [-h] [-r] [-1 to -9] [files...]
 *   -c : write to standard output
 *   -d : decompress
 *   -f : compress with Z_FILTERED
 *   -h : compress with Z_HUFFMAN_ONLY
 *   -r : compress with Z_RLE
 *   -1 to -9 : compression level
 */

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[]) {
    ಪೂರ್ಣಾಂಕ copyout = 0;
    ಪೂರ್ಣಾಂಕ uncompr = 0;
    gzFile file;
    ಅಕ್ಷರ *bname, outmode[5];

    string_copy(outmode, "wb6 ", ಗಾತ್ರ(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ಆದರೆ (bname)
      bname++;
    ಇಲ್ಲದಿದ್ದರೆ
      bname = argv[0];
    argc--, argv++;

    ಆದರೆ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ಆಗ (argc > 0) {
      ಆದರೆ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ಇಲ್ಲದಿದ್ದರೆ
        ನಿಲ್ಲಿಸು;
      argc--, argv++;
    }
    ಆದರೆ (outmode[3] == ' ')
        outmode[3] = 0;
    ಆದರೆ (argc == 0) {
        ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(stdin);
        ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(stdout);
        ಆದರೆ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ಆದರೆ (file == ಖಾಲಿ) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ಇಲ್ಲದಿದ್ದರೆ {
            file = gzdopen(fileno(stdout), outmode);
            ಆದರೆ (file == ಖಾಲಿ) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (copyout) {
            ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(stdout);
        }
        ಮಾಡು {
            ಆದರೆ (uncompr) {
                ಆದರೆ (copyout) {
                    file = gzopen(*argv, "rb");
                    ಆದರೆ (file == ಖಾಲಿ)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ಇಲ್ಲದಿದ್ದರೆ
                        gz_uncompress(file, stdout);
                } ಇಲ್ಲದಿದ್ದರೆ {
                    file_uncompress(*argv);
                }
            } ಇಲ್ಲದಿದ್ದರೆ {
                ಆದರೆ (copyout) {
                    FILE * ಒಳ = fopen(*argv, "rb");

                    ಆದರೆ (ಒಳ == ಖಾಲಿ) {
                        perror(*argv);
                    } ಇಲ್ಲದಿದ್ದರೆ {
                        file = gzdopen(fileno(stdout), outmode);
                        ಆದರೆ (file == ಖಾಲಿ) error("can't gzdopen stdout");

                        gz_compress(ಒಳ, file);
                    }

                } ಇಲ್ಲದಿದ್ದರೆ {
                    file_compress(*argv, outmode);
                }
            }
        } ಆಗ (argv++, --argc);
    }
    ಹಿಂತಿರುಗಿ 0;
}
