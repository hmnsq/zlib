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

#nếu defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#nếu defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include "zlib.h"
#include <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#nếu defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#không_thì
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#nếu defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#nếu defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#nếu !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  extern số_nguyên unlink(const char *);
#endif
#endif

#nếu defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

static char *strwinerror (error)
     DWORD error;
{
    static char buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        NULL);
    nếu (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        nếu (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        nếu (chars > sizeof (buf) - 1) {
            chars = sizeof (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    không_thì {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    trả_về buf;
}

static void pwinerror (s)
    const char *s;
{
    nếu (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    không_thì
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (sizeof(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local static
   /* Needed for systems with limitation on stack size. */
#không_thì
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
static char *string_copy(char *dst, char const *src, z_size_t len) {
    nếu (len == 0)
        trả_về NULL;
    trong_khi (--len) {
        *dst = *src++;
        nếu (*dst == 0)
            trả_về dst;
        dst++;
    }
    *dst = 0;
    trả_về dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#nếu defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

static void *myalloc(void *q, unsigned n, unsigned m) {
    (void)q;
    trả_về calloc(n, m);
}

static void myfree(void *q, void *p) {
    (void)q;
    free(p);
}

typedef struct gzFile_s {
    FILE *file;
    số_nguyên write;
    số_nguyên err;
    char *msg;
    z_stream strm;
} *gzFile;

static gzFile gz_open(const char *path, số_nguyên fd, const char *mode) {
    gzFile gz;
    số_nguyên ret;

    gz = malloc(sizeof(struct gzFile_s));
    nếu (gz == NULL)
        trả_về NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    nếu (gz->write)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    không_thì {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    nếu (ret != Z_OK) {
        free(gz);
        trả_về NULL;
    }
    gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
                              fopen(path, gz->write ? "wb" : "rb");
    nếu (gz->file == NULL) {
        gz->write ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        free(gz);
        trả_về NULL;
    }
    gz->err = 0;
    gz->msg = "";
    trả_về gz;
}

static gzFile gzopen(const char *path, const char *mode) {
    trả_về gz_open(path, -1, mode);
}

static gzFile gzdopen(số_nguyên fd, const char *mode) {
    trả_về gz_open(NULL, fd, mode);
}

static số_nguyên gzwrite(gzFile gz, const void *buf, unsigned len) {
    z_stream *strm;
    unsigned char out[BUFLEN];

    nếu (gz == NULL || !gz->write)
        trả_về 0;
    strm = &(gz->strm);
    strm->next_in = (void *)buf;
    strm->avail_in = len;
    do {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (void)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } trong_khi (strm->avail_out == 0);
    trả_về (số_nguyên)len;
}

static số_nguyên gzread(gzFile gz, void *buf, unsigned len) {
    số_nguyên ret;
    unsigned got;
    unsigned char in[1];
    z_stream *strm;

    nếu (gz == NULL || gz->write)
        trả_về 0;
    nếu (gz->err)
        trả_về 0;
    strm = &(gz->strm);
    strm->next_out = (void *)buf;
    strm->avail_out = len;
    do {
        got = (unsigned)fread(in, 1, 1, gz->file);
        nếu (got == 0)
            break;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        nếu (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->msg = strm->msg;
            trả_về 0;
        }
        nếu (ret == Z_STREAM_END)
            inflateReset(strm);
    } trong_khi (strm->avail_out);
    trả_về (số_nguyên)(len - strm->avail_out);
}

static số_nguyên gzclose(gzFile gz) {
    z_stream *strm;
    unsigned char out[BUFLEN];

    nếu (gz == NULL)
        trả_về Z_STREAM_ERROR;
    strm = &(gz->strm);
    nếu (gz->write) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        do {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (void)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } trong_khi (strm->avail_out == 0);
        deflateEnd(strm);
    }
    không_thì
        inflateEnd(strm);
    fclose(gz->file);
    free(gz);
    trả_về Z_OK;
}

static const char *gzerror(gzFile gz, số_nguyên *err) {
    *err = gz->err;
    trả_về gz->msg;
}

#endif

static char *prog;

/* ===========================================================================
 * Display error message and exit
 */
static void error(const char *msg) {
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
static số_nguyên gz_compress_mmap(FILE *in, gzFile out) {
    số_nguyên len;
    số_nguyên err;
    số_nguyên ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    struct stat sb;

    /* Determine the size of the file, needed for mmap: */
    nếu (fstat(ifd, &sb) < 0) trả_về Z_ERRNO;
    buf_len = sb.st_size;
    nếu (buf_len <= 0) trả_về Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    nếu (buf == (caddr_t)(-1)) trả_về Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (char *)buf, (unsigned)buf_len);

    nếu (len != (số_nguyên)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    nếu (gzclose(out) != Z_OK) error("failed gzclose");
    trả_về Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

static void gz_compress(FILE *in, gzFile out) {
    local char buf[BUFLEN];
    số_nguyên len;
    số_nguyên err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    nếu (gz_compress_mmap(in, out) == Z_OK) trả_về;
#endif
    cho (;;) {
        len = (số_nguyên)fread(buf, 1, sizeof(buf), in);
        nếu (ferror(in)) {
            perror("fread");
            exit(1);
        }
        nếu (len == 0) break;

        nếu (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    nếu (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
static void gz_uncompress(gzFile in, FILE *out) {
    local char buf[BUFLEN];
    số_nguyên len;
    số_nguyên err;

    cho (;;) {
        len = gzread(in, buf, sizeof(buf));
        nếu (len < 0) error (gzerror(in, &err));
        nếu (len == 0) break;

        nếu ((số_nguyên)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    nếu (fclose(out)) error("failed fclose");

    nếu (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
static void file_compress(char *file, char *mode) {
    local char outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    nếu (strlen(file) + strlen(GZ_SUFFIX) >= sizeof(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, sizeof(outfile));
    string_copy(end, GZ_SUFFIX, sizeof(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    nếu (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    nếu (out == NULL) {
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
static void file_uncompress(char *file) {
    local char buf[MAX_NAME_LEN+1];
    char *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    nếu (len + strlen(GZ_SUFFIX) >= sizeof(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, sizeof(buf));

    nếu (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } không_thì {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, sizeof(buf) - len);
    }
    in = gzopen(infile, "rb");
    nếu (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    nếu (out == NULL) {
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

số_nguyên main(số_nguyên argc, char *argv[]) {
    số_nguyên copyout = 0;
    số_nguyên uncompr = 0;
    gzFile file;
    char *bname, outmode[5];

    string_copy(outmode, "wb6 ", sizeof(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    nếu (bname)
      bname++;
    không_thì
      bname = argv[0];
    argc--, argv++;

    nếu (!strcmp(bname, "gunzip"))
      uncompr = 1;
    không_thì nếu (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    trong_khi (argc > 0) {
      nếu (strcmp(*argv, "-c") == 0)
        copyout = 1;
      không_thì nếu (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      không_thì nếu (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      không_thì nếu (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      không_thì nếu (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      không_thì nếu ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      không_thì
        break;
      argc--, argv++;
    }
    nếu (outmode[3] == ' ')
        outmode[3] = 0;
    nếu (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        nếu (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            nếu (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } không_thì {
            file = gzdopen(fileno(stdout), outmode);
            nếu (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } không_thì {
        nếu (copyout) {
            SET_BINARY_MODE(stdout);
        }
        do {
            nếu (uncompr) {
                nếu (copyout) {
                    file = gzopen(*argv, "rb");
                    nếu (file == NULL)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    không_thì
                        gz_uncompress(file, stdout);
                } không_thì {
                    file_uncompress(*argv);
                }
            } không_thì {
                nếu (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    nếu (in == NULL) {
                        perror(*argv);
                    } không_thì {
                        file = gzdopen(fileno(stdout), outmode);
                        nếu (file == NULL) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } không_thì {
                    file_compress(*argv, outmode);
                }
            }
        } trong_khi (argv++, --argc);
    }
    trả_về 0;
}
