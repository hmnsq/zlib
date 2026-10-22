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

#ଯଦି defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ଯଦି defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ଅନ୍ତର୍ଭାଵ "zlib.h"
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#ଯଦି defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_ଦ୍ୱିମିକ_MODE(file) setmode(fileno(file), O_ଦ୍ୱିମିକ)
#ଅନ୍ୟଥା
#  define SET_ଦ୍ୱିମିକ_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ଯଦି defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#ଯଦି defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ଯଦି !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା unlink(ଅଚଳ ଅକ୍ଷର *);
#endif
#endif

#ଯଦି defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

ସ୍ଥିର ଅକ୍ଷର *strwinerror (error)
     DWORD error;
{
    ସ୍ଥିର ଅକ୍ଷର buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ଖାଲି,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ଖାଲି);
    ଯଦି (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ଯଦି (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ଯଦି (chars > ଆକାର (buf) - 1) {
            chars = ଆକାର (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ଅନ୍ୟଥା {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ଫେରନ୍ତୁ buf;
}

ସ୍ଥିର ଶୂନ୍ୟ pwinerror (s)
    ଅଚଳ ଅକ୍ଷର *s;
{
    ଯଦି (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    ଅନ୍ୟଥା
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (ଆକାର(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local ସ୍ଥିର
   /* Needed for systems with limitation on stack size. */
#ଅନ୍ୟଥା
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ସ୍ଥିର ଅକ୍ଷର *string_copy(ଅକ୍ଷର *dst, ଅକ୍ଷର ଅଚଳ *src, z_size_t len) {
    ଯଦି (len == 0)
        ଫେରନ୍ତୁ ଖାଲି;
    ଯେତେବେଳେ (--len) {
        *dst = *src++;
        ଯଦି (*dst == 0)
            ଫେରନ୍ତୁ dst;
        dst++;
    }
    *dst = 0;
    ଫେରନ୍ତୁ dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ଯଦି defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

ସ୍ଥିର ଶୂନ୍ୟ *myalloc(ଶୂନ୍ୟ *q, unsigned n, unsigned m) {
    (ଶୂନ୍ୟ)q;
    ଫେରନ୍ତୁ calloc(n, m);
}

ସ୍ଥିର ଶୂନ୍ୟ myfree(ଶୂନ୍ୟ *q, ଶୂନ୍ୟ *p) {
    (ଶୂନ୍ୟ)q;
    ମୁକ୍ତ(p);
}

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା gzFile_s {
    FILE *file;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    ଅକ୍ଷର *ସନ୍ଦେଶ;
    z_stream strm;
} *gzFile;

ସ୍ଥିର gzFile gz_open(ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଅକ୍ଷର *mode) {
    gzFile gz;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

    gz = malloc(ଆକାର(ସଂରଚନା gzFile_s));
    ଯଦି (gz == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;
    gz->ଲିଖ = strchr(mode, 'w') != ଖାଲି;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ଯଦି (gz->ଲିଖ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ଅନ୍ୟଥା {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ଯଦି (ret != Z_OK) {
        ମୁକ୍ତ(gz);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    gz->file = path == ଖାଲି ? fdopen(fd, gz->ଲିଖ ? "wb" : "rb") :
                              fopen(path, gz->ଲିଖ ? "wb" : "rb");
    ଯଦି (gz->file == ଖାଲି) {
        gz->ଲିଖ ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        ମୁକ୍ତ(gz);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    gz->err = 0;
    gz->ସନ୍ଦେଶ = "";
    ଫେରନ୍ତୁ gz;
}

ସ୍ଥିର gzFile gzopen(ଅଚଳ ଅକ୍ଷର *path, ଅଚଳ ଅକ୍ଷର *mode) {
    ଫେରନ୍ତୁ gz_open(path, -1, mode);
}

ସ୍ଥିର gzFile gzdopen(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଅକ୍ଷର *mode) {
    ଫେରନ୍ତୁ gz_open(ଖାଲି, fd, mode);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzwrite(gzFile gz, ଅଚଳ ଶୂନ୍ୟ *buf, unsigned len) {
    z_stream *strm;
    unsigned ଅକ୍ଷର out[BUFLEN];

    ଯଦି (gz == ଖାଲି || !gz->ଲିଖ)
        ଫେରନ୍ତୁ 0;
    strm = &(gz->strm);
    strm->next_in = (ଶୂନ୍ୟ *)buf;
    strm->avail_in = len;
    କର {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (ଶୂନ୍ୟ)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ଯେତେବେଳେ (strm->avail_out == 0);
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzread(gzFile gz, ଶୂନ୍ୟ *buf, unsigned len) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    unsigned got;
    unsigned ଅକ୍ଷର in[1];
    z_stream *strm;

    ଯଦି (gz == ଖାଲି || gz->ଲିଖ)
        ଫେରନ୍ତୁ 0;
    ଯଦି (gz->err)
        ଫେରନ୍ତୁ 0;
    strm = &(gz->strm);
    strm->next_out = (ଶୂନ୍ୟ *)buf;
    strm->avail_out = len;
    କର {
        got = (unsigned)fread(in, 1, 1, gz->file);
        ଯଦି (got == 0)
            ଭାଙ୍ଗ;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ଯଦି (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->ସନ୍ଦେଶ = strm->ସନ୍ଦେଶ;
            ଫେରନ୍ତୁ 0;
        }
        ଯଦି (ret == Z_STREAM_END)
            inflateReset(strm);
    } ଯେତେବେଳେ (strm->avail_out);
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(len - strm->avail_out);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzclose(gzFile gz) {
    z_stream *strm;
    unsigned ଅକ୍ଷର out[BUFLEN];

    ଯଦି (gz == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    strm = &(gz->strm);
    ଯଦି (gz->ଲିଖ) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        କର {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (ଶୂନ୍ୟ)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ଯେତେବେଳେ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    ଅନ୍ୟଥା
        inflateEnd(strm);
    fclose(gz->file);
    ମୁକ୍ତ(gz);
    ଫେରନ୍ତୁ Z_OK;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *gzerror(gzFile gz, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *err) {
    *err = gz->err;
    ଫେରନ୍ତୁ gz->ସନ୍ଦେଶ;
}

#endif

ସ୍ଥିର ଅକ୍ଷର *prog;

/* ===========================================================================
 * Display error message and exit
 */
ସ୍ଥିର ଶୂନ୍ୟ error(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
    fprintf(stderr, "%s: %s\n", prog, ସନ୍ଦେଶ);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_compress_mmap(FILE *in, gzFile out) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    ସଂରଚନା stat sb;

    /* Determine the size of the file, needed for mmap: */
    ଯଦି (fstat(ifd, &sb) < 0) ଫେରନ୍ତୁ Z_ERRNO;
    buf_len = sb.st_size;
    ଯଦି (buf_len <= 0) ଫେରନ୍ତୁ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ଯଦି (buf == (caddr_t)(-1)) ଫେରନ୍ତୁ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (ଅକ୍ଷର *)buf, (unsigned)buf_len);

    ଯଦି (len != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    ଯଦି (gzclose(out) != Z_OK) error("failed gzclose");
    ଫେରନ୍ତୁ Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ସ୍ଥିର ଶୂନ୍ୟ gz_compress(FILE *in, gzFile out) {
    local ଅକ୍ଷର buf[BUFLEN];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ଯଦି (gz_compress_mmap(in, out) == Z_OK) ଫେରନ୍ତୁ;
#endif
    ପାଇଁ (;;) {
        len = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fread(buf, 1, ଆକାର(buf), in);
        ଯଦି (ferror(in)) {
            perror("fread");
            exit(1);
        }
        ଯଦି (len == 0) ଭାଙ୍ଗ;

        ଯଦି (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    ଯଦି (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ସ୍ଥିର ଶୂନ୍ୟ gz_uncompress(gzFile in, FILE *out) {
    local ଅକ୍ଷର buf[BUFLEN];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

    ପାଇଁ (;;) {
        len = gzread(in, buf, ଆକାର(buf));
        ଯଦି (len < 0) error (gzerror(in, &err));
        ଯଦି (len == 0) ଭାଙ୍ଗ;

        ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    ଯଦି (fclose(out)) error("failed fclose");

    ଯଦି (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ସ୍ଥିର ଶୂନ୍ୟ file_compress(ଅକ୍ଷର *file, ଅକ୍ଷର *mode) {
    local ଅକ୍ଷର outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    ଯଦି (strlen(file) + strlen(GZ_SUFFIX) >= ଆକାର(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, ଆକାର(outfile));
    string_copy(end, GZ_SUFFIX, ଆକାର(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    ଯଦି (in == ଖାଲି) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    ଯଦି (out == ଖାଲି) {
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
ସ୍ଥିର ଶୂନ୍ୟ file_uncompress(ଅକ୍ଷର *file) {
    local ଅକ୍ଷର buf[MAX_NAME_LEN+1];
    ଅକ୍ଷର *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    ଯଦି (len + strlen(GZ_SUFFIX) >= ଆକାର(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, ଆକାର(buf));

    ଯଦି (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ଅନ୍ୟଥା {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, ଆକାର(buf) - len);
    }
    in = gzopen(infile, "rb");
    ଯଦି (in == ଖାଲି) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    ଯଦି (out == ଖାଲି) {
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

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[]) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା copyout = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା uncompr = 0;
    gzFile file;
    ଅକ୍ଷର *bname, outmode[5];

    string_copy(outmode, "wb6 ", ଆକାର(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ଯଦି (bname)
      bname++;
    ଅନ୍ୟଥା
      bname = argv[0];
    argc--, argv++;

    ଯଦି (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ଅନ୍ୟଥା ଯଦି (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ଯେତେବେଳେ (argc > 0) {
      ଯଦି (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ଅନ୍ୟଥା ଯଦି (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ଅନ୍ୟଥା ଯଦି (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ଅନ୍ୟଥା ଯଦି (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ଅନ୍ୟଥା ଯଦି (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ଅନ୍ୟଥା ଯଦି ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ଅନ୍ୟଥା
        ଭାଙ୍ଗ;
      argc--, argv++;
    }
    ଯଦି (outmode[3] == ' ')
        outmode[3] = 0;
    ଯଦି (argc == 0) {
        SET_ଦ୍ୱିମିକ_MODE(stdin);
        SET_ଦ୍ୱିମିକ_MODE(stdout);
        ଯଦି (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ଯଦି (file == ଖାଲି) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ଅନ୍ୟଥା {
            file = gzdopen(fileno(stdout), outmode);
            ଯଦି (file == ଖାଲି) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ଅନ୍ୟଥା {
        ଯଦି (copyout) {
            SET_ଦ୍ୱିମିକ_MODE(stdout);
        }
        କର {
            ଯଦି (uncompr) {
                ଯଦି (copyout) {
                    file = gzopen(*argv, "rb");
                    ଯଦି (file == ଖାଲି)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ଅନ୍ୟଥା
                        gz_uncompress(file, stdout);
                } ଅନ୍ୟଥା {
                    file_uncompress(*argv);
                }
            } ଅନ୍ୟଥା {
                ଯଦି (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    ଯଦି (in == ଖାଲି) {
                        perror(*argv);
                    } ଅନ୍ୟଥା {
                        file = gzdopen(fileno(stdout), outmode);
                        ଯଦି (file == ଖାଲି) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } ଅନ୍ୟଥା {
                    file_compress(*argv, outmode);
                }
            }
        } ଯେତେବେଳେ (argv++, --argc);
    }
    ଫେରନ୍ତୁ 0;
}
