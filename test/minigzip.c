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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#ਜੇ_ਅੰਤ

#ਜੇ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#ਜੇ_ਅੰਤ
#ਜੇ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ STDC
#  include <string.h>
#  include <stdlib.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file) setmode(fileno(file), O_ਦੁਆਧਾਰੀ)
#ਨਹੀਂਤਾਂ
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#ਜੇ_ਅੰਤ
#ਜੇ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#ਜੇ_ਅੰਤ

#ਜੇ !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_ਸਰੋਤ)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ WIN32 /* unlink already in stdio.h for WIN32 */
  ਬਾਹਰੀ ਪੂਰਨ unlink(ਅਚਲ ਅੱਖਰ *);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

ਸਥਿਰ ਅੱਖਰ *strwinerror (error)
     DWORD error;
{
    ਸਥਿਰ ਅੱਖਰ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_ਆਖਰੀ_Error();
    DWORD chars = Format_ਸੁਨੇਹਾ(FORMAT_ਸੁਨੇਹਾ_FROM_SYSTEM
        | FORMAT_ਸੁਨੇਹਾ_ALLOCATE_ਬਫਰ,
        ਨਿਰਾ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ਨਿਰਾ);
    ਜੇ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ਜੇ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ਜੇ (chars > ਆਕਾਰ (buf) - 1) {
            chars = ਆਕਾਰ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ਨਹੀਂਤਾਂ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    ਸੈੱਟ_ਆਖਰੀ_Error(lasterr);
    ਵਾਪਸ buf;
}

ਸਥਿਰ ਖਾਲੀ pwinerror (s)
    ਅਚਲ ਅੱਖਰ *s;
{
    ਜੇ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_ਆਖਰੀ_Error ()));
    ਨਹੀਂਤਾਂ
        fprintf(stderr, "%s\n", strwinerror(Get_ਆਖਰੀ_Error ()));
}

#ਜੇ_ਅੰਤ /* UNDER_CE */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SUFFIX_LEN (ਆਕਾਰ(GZ_SUFFIX)-1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFLEN      16384
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_NAME_LEN 1024

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAXSEG_64K
#  define local ਸਥਿਰ
   /* Needed for systems with limitation on stack size. */
#ਨਹੀਂਤਾਂ
#  define local
#ਜੇ_ਅੰਤ

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ਸਥਿਰ ਅੱਖਰ *string_copy(ਅੱਖਰ *dst, ਅੱਖਰ ਅਚਲ *src, z_ਮਾਪ_t len) {
    ਜੇ (len == 0)
        ਵਾਪਸ ਨਿਰਾ;
    ਜਦੋਂ (--len) {
        *dst = *src++;
        ਜੇ (*dst == 0)
            ਵਾਪਸ dst;
        dst++;
    }
    *dst = 0;
    ਵਾਪਸ dst;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ਜੇ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ *myalloc(ਖਾਲੀ *q, ਬਿਨਾਨਿਸ਼ਾਨੀ n, ਬਿਨਾਨਿਸ਼ਾਨੀ m) {
    (ਖਾਲੀ)q;
    ਵਾਪਸ ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(n, m);
}

ਸਥਿਰ ਖਾਲੀ myfree(ਖਾਲੀ *q, ਖਾਲੀ *p) {
    (ਖਾਲੀ)q;
    ਮੁਕਤ(p);
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ gzFile_s {
    FILE *file;
    ਪੂਰਨ ਲਿਖ;
    ਪੂਰਨ err;
    ਅੱਖਰ *ਸੁਨੇਹਾ;
    z_ਧਾਰਾ strm;
} *gzFile;

ਸਥਿਰ gzFile gz_open(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ fd, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    gzFile gz;
    ਪੂਰਨ ret;

    gz = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ gzFile_s));
    ਜੇ (gz == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    gz->ਲਿਖ = strchr(ਬਹੁਲਕ, 'w') != ਨਿਰਾ;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ਜੇ (gz->ਲਿਖ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ਨਹੀਂਤਾਂ {
        gz->strm.ਅਗਲਾ_ਵਿੱਚ = 0;
        gz->strm.avail_ਵਿੱਚ = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ਜੇ (ret != Z_OK) {
        ਮੁਕਤ(gz);
        ਵਾਪਸ ਨਿਰਾ;
    }
    gz->file = ਰਸਤਾ == ਨਿਰਾ ? fdopen(fd, gz->ਲਿਖ ? "wb" : "rb") :
                              fopen(ਰਸਤਾ, gz->ਲਿਖ ? "wb" : "rb");
    ਜੇ (gz->file == ਨਿਰਾ) {
        gz->ਲਿਖ ? deflate_ਸਿਰਾ(&(gz->strm)) : inflate_ਸਿਰਾ(&(gz->strm));
        ਮੁਕਤ(gz);
        ਵਾਪਸ ਨਿਰਾ;
    }
    gz->err = 0;
    gz->ਸੁਨੇਹਾ = "";
    ਵਾਪਸ gz;
}

ਸਥਿਰ gzFile gzopen(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    ਵਾਪਸ gz_open(ਰਸਤਾ, -1, ਬਹੁਲਕ);
}

ਸਥਿਰ gzFile gzdopen(ਪੂਰਨ fd, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    ਵਾਪਸ gz_open(ਨਿਰਾ, fd, ਬਹੁਲਕ);
}

ਸਥਿਰ ਪੂਰਨ gzwrite(gzFile gz, ਅਚਲ ਖਾਲੀ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len) {
    z_ਧਾਰਾ *strm;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਬਾਹਰਲਾ[BUFLEN];

    ਜੇ (gz == ਨਿਰਾ || !gz->ਲਿਖ)
        ਵਾਪਸ 0;
    strm = &(gz->strm);
    strm->ਅਗਲਾ_ਵਿੱਚ = (ਖਾਲੀ *)buf;
    strm->avail_ਵਿੱਚ = len;
    ਕਰੋ {
        strm->ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
        strm->avail_ਬਾਹਰਲਾ = BUFLEN;
        (ਖਾਲੀ)deflate(strm, Z_NO_FLUSH);
        fwrite(ਬਾਹਰਲਾ, 1, BUFLEN - strm->avail_ਬਾਹਰਲਾ, gz->file);
    } ਜਦੋਂ (strm->avail_ਬਾਹਰਲਾ == 0);
    ਵਾਪਸ (ਪੂਰਨ)len;
}

ਸਥਿਰ ਪੂਰਨ gzread(gzFile gz, ਖਾਲੀ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len) {
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ got;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਵਿੱਚ[1];
    z_ਧਾਰਾ *strm;

    ਜੇ (gz == ਨਿਰਾ || gz->ਲਿਖ)
        ਵਾਪਸ 0;
    ਜੇ (gz->err)
        ਵਾਪਸ 0;
    strm = &(gz->strm);
    strm->ਅਗਲਾ_ਬਾਹਰਲਾ = (ਖਾਲੀ *)buf;
    strm->avail_ਬਾਹਰਲਾ = len;
    ਕਰੋ {
        got = (ਬਿਨਾਨਿਸ਼ਾਨੀ)fread(ਵਿੱਚ, 1, 1, gz->file);
        ਜੇ (got == 0)
            ਰੋਕੋ;
        strm->ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;
        strm->avail_ਵਿੱਚ = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ਜੇ (ret == Z_ਅੰਕੜੇ_ERROR) {
            gz->err = Z_ਅੰਕੜੇ_ERROR;
            gz->ਸੁਨੇਹਾ = strm->ਸੁਨੇਹਾ;
            ਵਾਪਸ 0;
        }
        ਜੇ (ret == Z_ਧਾਰਾ_ਸਿਰਾ)
            inflateReset(strm);
    } ਜਦੋਂ (strm->avail_ਬਾਹਰਲਾ);
    ਵਾਪਸ (ਪੂਰਨ)(len - strm->avail_ਬਾਹਰਲਾ);
}

ਸਥਿਰ ਪੂਰਨ gzclose(gzFile gz) {
    z_ਧਾਰਾ *strm;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਬਾਹਰਲਾ[BUFLEN];

    ਜੇ (gz == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    strm = &(gz->strm);
    ਜੇ (gz->ਲਿਖ) {
        strm->ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
        strm->avail_ਵਿੱਚ = 0;
        ਕਰੋ {
            strm->ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
            strm->avail_ਬਾਹਰਲਾ = BUFLEN;
            (ਖਾਲੀ)deflate(strm, Z_ਅੰਤ);
            fwrite(ਬਾਹਰਲਾ, 1, BUFLEN - strm->avail_ਬਾਹਰਲਾ, gz->file);
        } ਜਦੋਂ (strm->avail_ਬਾਹਰਲਾ == 0);
        deflate_ਸਿਰਾ(strm);
    }
    ਨਹੀਂਤਾਂ
        inflate_ਸਿਰਾ(strm);
    fclose(gz->file);
    ਮੁਕਤ(gz);
    ਵਾਪਸ Z_OK;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *gzerror(gzFile gz, ਪੂਰਨ *err) {
    *err = gz->err;
    ਵਾਪਸ gz->ਸੁਨੇਹਾ;
}

#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅੱਖਰ *prog;

/* ===========================================================================
 * Display error message and exit
 */
ਸਥਿਰ ਖਾਲੀ error(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
    fprintf(stderr, "%s: %s\n", prog, ਸੁਨੇਹਾ);
    ਬਾਹਰ_ਨਿਕਲੋ(1);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ਸਥਿਰ ਪੂਰਨ gz_compress_mmap(FILE *ਵਿੱਚ, gzFile ਬਾਹਰਲਾ) {
    ਪੂਰਨ len;
    ਪੂਰਨ err;
    ਪੂਰਨ ifd = fileno(ਵਿੱਚ);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    ਬਣਤਰ stat sb;

    /* Determine the size of the file, needed for mmap: */
    ਜੇ (fstat(ifd, &sb) < 0) ਵਾਪਸ Z_ERRNO;
    buf_len = sb.st_size;
    ਜੇ (buf_len <= 0) ਵਾਪਸ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ਜੇ (buf == (caddr_t)(-1)) ਵਾਪਸ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(ਬਾਹਰਲਾ, (ਅੱਖਰ *)buf, (ਬਿਨਾਨਿਸ਼ਾਨੀ)buf_len);

    ਜੇ (len != (ਪੂਰਨ)buf_len) error(gzerror(ਬਾਹਰਲਾ, &err));

    munmap(buf, buf_len);
    fclose(ਵਿੱਚ);
    ਜੇ (gzclose(ਬਾਹਰਲਾ) != Z_OK) error("failed gzclose");
    ਵਾਪਸ Z_OK;
}
#ਜੇ_ਅੰਤ /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ਸਥਿਰ ਖਾਲੀ gz_compress(FILE *ਵਿੱਚ, gzFile ਬਾਹਰਲਾ) {
    local ਅੱਖਰ buf[BUFLEN];
    ਪੂਰਨ len;
    ਪੂਰਨ err;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ਜੇ (gz_compress_mmap(ਵਿੱਚ, ਬਾਹਰਲਾ) == Z_OK) ਵਾਪਸ;
#ਜੇ_ਅੰਤ
    ਲਈ (;;) {
        len = (ਪੂਰਨ)fread(buf, 1, ਆਕਾਰ(buf), ਵਿੱਚ);
        ਜੇ (ferror(ਵਿੱਚ)) {
            perror("fread");
            ਬਾਹਰ_ਨਿਕਲੋ(1);
        }
        ਜੇ (len == 0) ਰੋਕੋ;

        ਜੇ (gzwrite(ਬਾਹਰਲਾ, buf, (ਬਿਨਾਨਿਸ਼ਾਨੀ)len) != len) error(gzerror(ਬਾਹਰਲਾ, &err));
    }
    fclose(ਵਿੱਚ);
    ਜੇ (gzclose(ਬਾਹਰਲਾ) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ਸਥਿਰ ਖਾਲੀ gz_uncompress(gzFile ਵਿੱਚ, FILE *ਬਾਹਰਲਾ) {
    local ਅੱਖਰ buf[BUFLEN];
    ਪੂਰਨ len;
    ਪੂਰਨ err;

    ਲਈ (;;) {
        len = gzread(ਵਿੱਚ, buf, ਆਕਾਰ(buf));
        ਜੇ (len < 0) error (gzerror(ਵਿੱਚ, &err));
        ਜੇ (len == 0) ਰੋਕੋ;

        ਜੇ ((ਪੂਰਨ)fwrite(buf, 1, (ਬਿਨਾਨਿਸ਼ਾਨੀ)len, ਬਾਹਰਲਾ) != len) {
            error("failed fwrite");
        }
    }
    ਜੇ (fclose(ਬਾਹਰਲਾ)) error("failed fclose");

    ਜੇ (gzclose(ਵਿੱਚ) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ਸਥਿਰ ਖਾਲੀ file_compress(ਅੱਖਰ *file, ਅੱਖਰ *ਬਹੁਲਕ) {
    local ਅੱਖਰ outfile[MAX_NAME_LEN+1], *ਸਿਰਾ;
    FILE  *ਵਿੱਚ;
    gzFile ਬਾਹਰਲਾ;

    ਜੇ (ਲੜੀ_ਲੰਬਾਈ(file) + ਲੜੀ_ਲੰਬਾਈ(GZ_SUFFIX) >= ਆਕਾਰ(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

    ਸਿਰਾ = string_copy(outfile, file, ਆਕਾਰ(outfile));
    string_copy(ਸਿਰਾ, GZ_SUFFIX, ਆਕਾਰ(outfile) - (z_ਮਾਪ_t)(ਸਿਰਾ - outfile));

    ਵਿੱਚ = fopen(file, "rb");
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ) {
        perror(file);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    ਬਾਹਰਲਾ = gzopen(outfile, ਬਹੁਲਕ);
    ਜੇ (ਬਾਹਰਲਾ == ਨਿਰਾ) {
        fclose(ਵਿੱਚ);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    gz_compress(ਵਿੱਚ, ਬਾਹਰਲਾ);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ਸਥਿਰ ਖਾਲੀ file_uncompress(ਅੱਖਰ *file) {
    local ਅੱਖਰ buf[MAX_NAME_LEN+1];
    ਅੱਖਰ *infile, *outfile;
    FILE  *ਬਾਹਰਲਾ;
    gzFile ਵਿੱਚ;
    z_ਮਾਪ_t len = ਲੜੀ_ਲੰਬਾਈ(file);

    ਜੇ (len + ਲੜੀ_ਲੰਬਾਈ(GZ_SUFFIX) >= ਆਕਾਰ(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

    string_copy(buf, file, ਆਕਾਰ(buf));

    ਜੇ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ਨਹੀਂਤਾਂ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, ਆਕਾਰ(buf) - len);
    }
    ਵਿੱਚ = gzopen(infile, "rb");
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    ਬਾਹਰਲਾ = fopen(outfile, "wb");
    ਜੇ (ਬਾਹਰਲਾ == ਨਿਰਾ) {
        gzclose(ਵਿੱਚ);
        perror(file);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

    gz_uncompress(ਵਿੱਚ, ਬਾਹਰਲਾ);

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

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[]) {
    ਪੂਰਨ copyout = 0;
    ਪੂਰਨ uncompr = 0;
    gzFile file;
    ਅੱਖਰ *bname, outmode[5];

    string_copy(outmode, "wb6 ", ਆਕਾਰ(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ਜੇ (bname)
      bname++;
    ਨਹੀਂਤਾਂ
      bname = argv[0];
    argc--, argv++;

    ਜੇ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ਨਹੀਂਤਾਂ ਜੇ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ਜਦੋਂ (argc > 0) {
      ਜੇ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ਨਹੀਂਤਾਂ ਜੇ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ਨਹੀਂਤਾਂ ਜੇ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ਨਹੀਂਤਾਂ ਜੇ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ਨਹੀਂਤਾਂ ਜੇ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ਨਹੀਂਤਾਂ ਜੇ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ਨਹੀਂਤਾਂ
        ਰੋਕੋ;
      argc--, argv++;
    }
    ਜੇ (outmode[3] == ' ')
        outmode[3] = 0;
    ਜੇ (argc == 0) {
        ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdin);
        ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdout);
        ਜੇ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ਜੇ (file == ਨਿਰਾ) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ਨਹੀਂਤਾਂ {
            file = gzdopen(fileno(stdout), outmode);
            ਜੇ (file == ਨਿਰਾ) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ਨਹੀਂਤਾਂ {
        ਜੇ (copyout) {
            ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdout);
        }
        ਕਰੋ {
            ਜੇ (uncompr) {
                ਜੇ (copyout) {
                    file = gzopen(*argv, "rb");
                    ਜੇ (file == ਨਿਰਾ)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ਨਹੀਂਤਾਂ
                        gz_uncompress(file, stdout);
                } ਨਹੀਂਤਾਂ {
                    file_uncompress(*argv);
                }
            } ਨਹੀਂਤਾਂ {
                ਜੇ (copyout) {
                    FILE * ਵਿੱਚ = fopen(*argv, "rb");

                    ਜੇ (ਵਿੱਚ == ਨਿਰਾ) {
                        perror(*argv);
                    } ਨਹੀਂਤਾਂ {
                        file = gzdopen(fileno(stdout), outmode);
                        ਜੇ (file == ਨਿਰਾ) error("can't gzdopen stdout");

                        gz_compress(ਵਿੱਚ, file);
                    }

                } ਨਹੀਂਤਾਂ {
                    file_compress(*argv, outmode);
                }
            }
        } ਜਦੋਂ (argv++, --argc);
    }
    ਵਾਪਸ 0;
}
