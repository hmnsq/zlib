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

#もし defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#もし defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
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

#もし defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#否则
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#もし defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#もし defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#もし !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  extern 整数 unlink(const char *);
#endif
#endif

#もし defined(UNDER_CE)
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
    もし (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        もし (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        もし (chars > sizeof (buf) - 1) {
            chars = sizeof (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    否则 {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    戻る buf;
}

static void pwinerror (s)
    const char *s;
{
    もし (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    否则
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
#否则
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
static char *string_copy(char *dst, char const *src, z_size_t len) {
    もし (len == 0)
        戻る NULL;
    ながら (--len) {
        *dst = *src++;
        もし (*dst == 0)
            戻る dst;
        dst++;
    }
    *dst = 0;
    戻る dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#もし defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

static void *myalloc(void *q, unsigned n, unsigned m) {
    (void)q;
    戻る calloc(n, m);
}

static void myfree(void *q, void *p) {
    (void)q;
    free(p);
}

typedef struct gzFile_s {
    FILE *file;
    整数 write;
    整数 err;
    char *msg;
    z_stream strm;
} *gzFile;

static gzFile gz_open(const char *path, 整数 fd, const char *mode) {
    gzFile gz;
    整数 ret;

    gz = malloc(sizeof(struct gzFile_s));
    もし (gz == NULL)
        戻る NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    もし (gz->write)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    否则 {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    もし (ret != Z_OK) {
        free(gz);
        戻る NULL;
    }
    gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
                              fopen(path, gz->write ? "wb" : "rb");
    もし (gz->file == NULL) {
        gz->write ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        free(gz);
        戻る NULL;
    }
    gz->err = 0;
    gz->msg = "";
    戻る gz;
}

static gzFile gzopen(const char *path, const char *mode) {
    戻る gz_open(path, -1, mode);
}

static gzFile gzdopen(整数 fd, const char *mode) {
    戻る gz_open(NULL, fd, mode);
}

static 整数 gzwrite(gzFile gz, const void *buf, unsigned len) {
    z_stream *strm;
    unsigned char out[BUFLEN];

    もし (gz == NULL || !gz->write)
        戻る 0;
    strm = &(gz->strm);
    strm->next_in = (void *)buf;
    strm->avail_in = len;
    do {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (void)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ながら (strm->avail_out == 0);
    戻る (整数)len;
}

static 整数 gzread(gzFile gz, void *buf, unsigned len) {
    整数 ret;
    unsigned got;
    unsigned char in[1];
    z_stream *strm;

    もし (gz == NULL || gz->write)
        戻る 0;
    もし (gz->err)
        戻る 0;
    strm = &(gz->strm);
    strm->next_out = (void *)buf;
    strm->avail_out = len;
    do {
        got = (unsigned)fread(in, 1, 1, gz->file);
        もし (got == 0)
            break;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        もし (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->msg = strm->msg;
            戻る 0;
        }
        もし (ret == Z_STREAM_END)
            inflateReset(strm);
    } ながら (strm->avail_out);
    戻る (整数)(len - strm->avail_out);
}

static 整数 gzclose(gzFile gz) {
    z_stream *strm;
    unsigned char out[BUFLEN];

    もし (gz == NULL)
        戻る Z_STREAM_ERROR;
    strm = &(gz->strm);
    もし (gz->write) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        do {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (void)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ながら (strm->avail_out == 0);
        deflateEnd(strm);
    }
    否则
        inflateEnd(strm);
    fclose(gz->file);
    free(gz);
    戻る Z_OK;
}

static const char *gzerror(gzFile gz, 整数 *err) {
    *err = gz->err;
    戻る gz->msg;
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
static 整数 gz_compress_mmap(FILE *in, gzFile out) {
    整数 len;
    整数 err;
    整数 ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    struct stat sb;

    /* Determine the size of the file, needed for mmap: */
    もし (fstat(ifd, &sb) < 0) 戻る Z_ERRNO;
    buf_len = sb.st_size;
    もし (buf_len <= 0) 戻る Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    もし (buf == (caddr_t)(-1)) 戻る Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (char *)buf, (unsigned)buf_len);

    もし (len != (整数)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    もし (gzclose(out) != Z_OK) error("failed gzclose");
    戻る Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

static void gz_compress(FILE *in, gzFile out) {
    local char buf[BUFLEN];
    整数 len;
    整数 err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    もし (gz_compress_mmap(in, out) == Z_OK) 戻る;
#endif
    ために (;;) {
        len = (整数)fread(buf, 1, sizeof(buf), in);
        もし (ferror(in)) {
            perror("fread");
            exit(1);
        }
        もし (len == 0) break;

        もし (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    もし (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
static void gz_uncompress(gzFile in, FILE *out) {
    local char buf[BUFLEN];
    整数 len;
    整数 err;

    ために (;;) {
        len = gzread(in, buf, sizeof(buf));
        もし (len < 0) error (gzerror(in, &err));
        もし (len == 0) break;

        もし ((整数)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    もし (fclose(out)) error("failed fclose");

    もし (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
static void file_compress(char *file, char *mode) {
    local char outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    もし (strlen(file) + strlen(GZ_SUFFIX) >= sizeof(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, sizeof(outfile));
    string_copy(end, GZ_SUFFIX, sizeof(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    もし (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    もし (out == NULL) {
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

    もし (len + strlen(GZ_SUFFIX) >= sizeof(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, sizeof(buf));

    もし (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } 否则 {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, sizeof(buf) - len);
    }
    in = gzopen(infile, "rb");
    もし (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    もし (out == NULL) {
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

整数 メインページ(整数 argc, char *argv[]) {
    整数 copyout = 0;
    整数 uncompr = 0;
    gzFile file;
    char *bname, outmode[5];

    string_copy(outmode, "wb6 ", sizeof(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    もし (bname)
      bname++;
    否则
      bname = argv[0];
    argc--, argv++;

    もし (!strcmp(bname, "gunzip"))
      uncompr = 1;
    否则 もし (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ながら (argc > 0) {
      もし (strcmp(*argv, "-c") == 0)
        copyout = 1;
      否则 もし (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      否则 もし (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      否则 もし (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      否则 もし (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      否则 もし ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      否则
        break;
      argc--, argv++;
    }
    もし (outmode[3] == ' ')
        outmode[3] = 0;
    もし (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        もし (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            もし (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } 否则 {
            file = gzdopen(fileno(stdout), outmode);
            もし (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } 否则 {
        もし (copyout) {
            SET_BINARY_MODE(stdout);
        }
        do {
            もし (uncompr) {
                もし (copyout) {
                    file = gzopen(*argv, "rb");
                    もし (file == NULL)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    否则
                        gz_uncompress(file, stdout);
                } 否则 {
                    file_uncompress(*argv);
                }
            } 否则 {
                もし (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    もし (in == NULL) {
                        perror(*argv);
                    } 否则 {
                        file = gzdopen(fileno(stdout), outmode);
                        もし (file == NULL) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } 否则 {
                    file_compress(*argv, outmode);
                }
            }
        } ながら (argv++, --argc);
    }
    戻る 0;
}
