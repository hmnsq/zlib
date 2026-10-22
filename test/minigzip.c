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

#ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#endif

#ಯದಿ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ಯದಿ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#ಯದಿ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_ದ್ವಿಮಾನ_MODE(file) setmode(fileno(file), O_ದ್ವಿಮಾನ)
#ಅನ್ಯಥಾ
#  define SET_ದ್ವಿಮಾನ_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ಯದಿ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#ಯದಿ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ಯದಿ !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ unlink(ಧ್ರುವ ವರ್ಣ *);
#endif
#endif

#ಯದಿ defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

ಸ್ಥಿರ ವರ್ಣ *strwinerror (error)
     DWORD error;
{
    ಸ್ಥಿರ ವರ್ಣ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ರಿಕ್ತ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ರಿಕ್ತ);
    ಯದಿ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ಯದಿ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ಯದಿ (chars > ಪರಿಮಾಣ (buf) - 1) {
            chars = ಪರಿಮಾಣ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ಅನ್ಯಥಾ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ಪ್ರತ್ಯಾಗಮ buf;
}

ಸ್ಥಿರ ಶೂನ್ಯ pwinerror (s)
    ಧ್ರುವ ವರ್ಣ *s;
{
    ಯದಿ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    ಅನ್ಯಥಾ
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (ಪರಿಮಾಣ(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local ಸ್ಥಿರ
   /* Needed for systems with limitation on stack size. */
#ಅನ್ಯಥಾ
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ಸ್ಥಿರ ವರ್ಣ *string_copy(ವರ್ಣ *dst, ವರ್ಣ ಧ್ರುವ *src, z_size_t len) {
    ಯದಿ (len == 0)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಯಾವತ್ (--len) {
        *dst = *src++;
        ಯದಿ (*dst == 0)
            ಪ್ರತ್ಯಾಗಮ dst;
        dst++;
    }
    *dst = 0;
    ಪ್ರತ್ಯಾಗಮ dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ಯದಿ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

ಸ್ಥಿರ ಶೂನ್ಯ *myalloc(ಶೂನ್ಯ *q, unsigned n, unsigned m) {
    (ಶೂನ್ಯ)q;
    ಪ್ರತ್ಯಾಗಮ calloc(n, m);
}

ಸ್ಥಿರ ಶೂನ್ಯ myfree(ಶೂನ್ಯ *q, ಶೂನ್ಯ *p) {
    (ಶೂನ್ಯ)q;
    ಮುಕ್ತ(p);
}

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ gzFile_s {
    FILE *file;
    ಪೂರ್ಣಾಙ್ಕ ಲಿಖ;
    ಪೂರ್ಣಾಙ್ಕ err;
    ವರ್ಣ *ಸಂದೇಶ;
    z_stream strm;
} *gzFile;

ಸ್ಥಿರ gzFile gz_open(ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ವರ್ಣ *mode) {
    gzFile gz;
    ಪೂರ್ಣಾಙ್ಕ ret;

    gz = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ gzFile_s));
    ಯದಿ (gz == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    gz->ಲಿಖ = strchr(mode, 'w') != ರಿಕ್ತ;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ಯದಿ (gz->ಲಿಖ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ಅನ್ಯಥಾ {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ಯದಿ (ret != Z_OK) {
        ಮುಕ್ತ(gz);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    gz->file = path == ರಿಕ್ತ ? fdopen(fd, gz->ಲಿಖ ? "wb" : "rb") :
                              fopen(path, gz->ಲಿಖ ? "wb" : "rb");
    ಯದಿ (gz->file == ರಿಕ್ತ) {
        gz->ಲಿಖ ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        ಮುಕ್ತ(gz);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    gz->err = 0;
    gz->ಸಂದೇಶ = "";
    ಪ್ರತ್ಯಾಗಮ gz;
}

ಸ್ಥಿರ gzFile gzopen(ಧ್ರುವ ವರ್ಣ *path, ಧ್ರುವ ವರ್ಣ *mode) {
    ಪ್ರತ್ಯಾಗಮ gz_open(path, -1, mode);
}

ಸ್ಥಿರ gzFile gzdopen(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ವರ್ಣ *mode) {
    ಪ್ರತ್ಯಾಗಮ gz_open(ರಿಕ್ತ, fd, mode);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gzwrite(gzFile gz, ಧ್ರುವ ಶೂನ್ಯ *buf, unsigned len) {
    z_stream *strm;
    unsigned ವರ್ಣ out[BUFLEN];

    ಯದಿ (gz == ರಿಕ್ತ || !gz->ಲಿಖ)
        ಪ್ರತ್ಯಾಗಮ 0;
    strm = &(gz->strm);
    strm->next_in = (ಶೂನ್ಯ *)buf;
    strm->avail_in = len;
    ಕುರು {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (ಶೂನ್ಯ)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ಯಾವತ್ (strm->avail_out == 0);
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)len;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gzread(gzFile gz, ಶೂನ್ಯ *buf, unsigned len) {
    ಪೂರ್ಣಾಙ್ಕ ret;
    unsigned got;
    unsigned ವರ್ಣ in[1];
    z_stream *strm;

    ಯದಿ (gz == ರಿಕ್ತ || gz->ಲಿಖ)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ (gz->err)
        ಪ್ರತ್ಯಾಗಮ 0;
    strm = &(gz->strm);
    strm->next_out = (ಶೂನ್ಯ *)buf;
    strm->avail_out = len;
    ಕುರು {
        got = (unsigned)fread(in, 1, 1, gz->file);
        ಯದಿ (got == 0)
            ಭಙ್ಗ;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ಯದಿ (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->ಸಂದೇಶ = strm->ಸಂದೇಶ;
            ಪ್ರತ್ಯಾಗಮ 0;
        }
        ಯದಿ (ret == Z_STREAM_END)
            inflateReset(strm);
    } ಯಾವತ್ (strm->avail_out);
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)(len - strm->avail_out);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gzclose(gzFile gz) {
    z_stream *strm;
    unsigned ವರ್ಣ out[BUFLEN];

    ಯದಿ (gz == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    strm = &(gz->strm);
    ಯದಿ (gz->ಲಿಖ) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        ಕುರು {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (ಶೂನ್ಯ)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ಯಾವತ್ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    ಅನ್ಯಥಾ
        inflateEnd(strm);
    fclose(gz->file);
    ಮುಕ್ತ(gz);
    ಪ್ರತ್ಯಾಗಮ Z_OK;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *gzerror(gzFile gz, ಪೂರ್ಣಾಙ್ಕ *err) {
    *err = gz->err;
    ಪ್ರತ್ಯಾಗಮ gz->ಸಂದೇಶ;
}

#endif

ಸ್ಥಿರ ವರ್ಣ *prog;

/* ===========================================================================
 * Display error message and exit
 */
ಸ್ಥಿರ ಶೂನ್ಯ error(ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) {
    fprintf(stderr, "%s: %s\n", prog, ಸಂದೇಶ);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gz_compress_mmap(FILE *in, gzFile out) {
    ಪೂರ್ಣಾಙ್ಕ len;
    ಪೂರ್ಣಾಙ್ಕ err;
    ಪೂರ್ಣಾಙ್ಕ ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    ಸಂರಚನಾ stat sb;

    /* Determine the size of the file, needed for mmap: */
    ಯದಿ (fstat(ifd, &sb) < 0) ಪ್ರತ್ಯಾಗಮ Z_ERRNO;
    buf_len = sb.st_size;
    ಯದಿ (buf_len <= 0) ಪ್ರತ್ಯಾಗಮ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ಯದಿ (buf == (caddr_t)(-1)) ಪ್ರತ್ಯಾಗಮ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (ವರ್ಣ *)buf, (unsigned)buf_len);

    ಯದಿ (len != (ಪೂರ್ಣಾಙ್ಕ)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    ಯದಿ (gzclose(out) != Z_OK) error("failed gzclose");
    ಪ್ರತ್ಯಾಗಮ Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ಸ್ಥಿರ ಶೂನ್ಯ gz_compress(FILE *in, gzFile out) {
    local ವರ್ಣ buf[BUFLEN];
    ಪೂರ್ಣಾಙ್ಕ len;
    ಪೂರ್ಣಾಙ್ಕ err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ಯದಿ (gz_compress_mmap(in, out) == Z_OK) ಪ್ರತ್ಯಾಗಮ;
#endif
    ಪ್ರತಿ (;;) {
        len = (ಪೂರ್ಣಾಙ್ಕ)fread(buf, 1, ಪರಿಮಾಣ(buf), in);
        ಯದಿ (ferror(in)) {
            perror("fread");
            exit(1);
        }
        ಯದಿ (len == 0) ಭಙ್ಗ;

        ಯದಿ (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    ಯದಿ (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ಸ್ಥಿರ ಶೂನ್ಯ gz_uncompress(gzFile in, FILE *out) {
    local ವರ್ಣ buf[BUFLEN];
    ಪೂರ್ಣಾಙ್ಕ len;
    ಪೂರ್ಣಾಙ್ಕ err;

    ಪ್ರತಿ (;;) {
        len = gzread(in, buf, ಪರಿಮಾಣ(buf));
        ಯದಿ (len < 0) error (gzerror(in, &err));
        ಯದಿ (len == 0) ಭಙ್ಗ;

        ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    ಯದಿ (fclose(out)) error("failed fclose");

    ಯದಿ (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ಸ್ಥಿರ ಶೂನ್ಯ file_compress(ವರ್ಣ *file, ವರ್ಣ *mode) {
    local ವರ್ಣ outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    ಯದಿ (strlen(file) + strlen(GZ_SUFFIX) >= ಪರಿಮಾಣ(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, ಪರಿಮಾಣ(outfile));
    string_copy(end, GZ_SUFFIX, ಪರಿಮಾಣ(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    ಯದಿ (in == ರಿಕ್ತ) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    ಯದಿ (out == ರಿಕ್ತ) {
        fclose(in);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        exit(1);
    }
    gz_compress(in, out);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ಸ್ಥಿರ ಶೂನ್ಯ file_uncompress(ವರ್ಣ *file) {
    local ವರ್ಣ buf[MAX_NAME_LEN+1];
    ವರ್ಣ *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    ಯದಿ (len + strlen(GZ_SUFFIX) >= ಪರಿಮಾಣ(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, ಪರಿಮಾಣ(buf));

    ಯದಿ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ಅನ್ಯಥಾ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, ಪರಿಮಾಣ(buf) - len);
    }
    in = gzopen(infile, "rb");
    ಯದಿ (in == ರಿಕ್ತ) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    ಯದಿ (out == ರಿಕ್ತ) {
        gzclose(in);
        perror(file);
        exit(1);
    }

    gz_uncompress(in, out);

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

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[]) {
    ಪೂರ್ಣಾಙ್ಕ copyout = 0;
    ಪೂರ್ಣಾಙ್ಕ uncompr = 0;
    gzFile file;
    ವರ್ಣ *bname, outmode[5];

    string_copy(outmode, "wb6 ", ಪರಿಮಾಣ(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ಯದಿ (bname)
      bname++;
    ಅನ್ಯಥಾ
      bname = argv[0];
    argc--, argv++;

    ಯದಿ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ಅನ್ಯಥಾ ಯದಿ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ಯಾವತ್ (argc > 0) {
      ಯದಿ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ಅನ್ಯಥಾ ಯದಿ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ಅನ್ಯಥಾ ಯದಿ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ಅನ್ಯಥಾ ಯದಿ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ಅನ್ಯಥಾ ಯದಿ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ಅನ್ಯಥಾ ಯದಿ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ಅನ್ಯಥಾ
        ಭಙ್ಗ;
      argc--, argv++;
    }
    ಯದಿ (outmode[3] == ' ')
        outmode[3] = 0;
    ಯದಿ (argc == 0) {
        SET_ದ್ವಿಮಾನ_MODE(stdin);
        SET_ದ್ವಿಮಾನ_MODE(stdout);
        ಯದಿ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ಯದಿ (file == ರಿಕ್ತ) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ಅನ್ಯಥಾ {
            file = gzdopen(fileno(stdout), outmode);
            ಯದಿ (file == ರಿಕ್ತ) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ಅನ್ಯಥಾ {
        ಯದಿ (copyout) {
            SET_ದ್ವಿಮಾನ_MODE(stdout);
        }
        ಕುರು {
            ಯದಿ (uncompr) {
                ಯದಿ (copyout) {
                    file = gzopen(*argv, "rb");
                    ಯದಿ (file == ರಿಕ್ತ)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ಅನ್ಯಥಾ
                        gz_uncompress(file, stdout);
                } ಅನ್ಯಥಾ {
                    file_uncompress(*argv);
                }
            } ಅನ್ಯಥಾ {
                ಯದಿ (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    ಯದಿ (in == ರಿಕ್ತ) {
                        perror(*argv);
                    } ಅನ್ಯಥಾ {
                        file = gzdopen(fileno(stdout), outmode);
                        ಯದಿ (file == ರಿಕ್ತ) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } ಅನ್ಯಥಾ {
                    file_compress(*argv, outmode);
                }
            }
        } ಯಾವತ್ (argv++, --argc);
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}
