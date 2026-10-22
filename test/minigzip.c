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

#ꦪꦼꦤ꧀ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ꦪꦼꦤ꧀ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
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

#ꦪꦼꦤ꧀ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#ꦭꦶꦪ
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ꦪꦼꦤ꧀ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#ꦪꦼꦤ꧀ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ꦪꦼꦤ꧀ !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  extern puur_naa_nka unlink(const var_na *);
#endif
#endif

#ꦪꦼꦤ꧀ defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

static var_na *strwinerror (error)
     DWORD error;
{
    static var_na buf[1024];

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
    ꦪꦼꦤ꧀ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ꦪꦼꦤ꧀ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ꦪꦼꦤ꧀ (chars > parimaa_na (buf) - 1) {
            chars = parimaa_na (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ꦭꦶꦪ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ꦮꦭꦶ buf;
}

static void pwinerror (s)
    const var_na *s;
{
    ꦪꦼꦤ꧀ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    ꦭꦶꦪ
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (parimaa_na(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local static
   /* Needed for systems with limitation on stack size. */
#ꦭꦶꦪ
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
static var_na *string_copy(var_na *dst, var_na const *src, z_size_t len) {
    ꦪꦼꦤ꧀ (len == 0)
        ꦮꦭꦶ NULL;
    ꦱꦼꦢꦼꦭ (--len) {
        *dst = *src++;
        ꦪꦼꦤ꧀ (*dst == 0)
            ꦮꦭꦶ dst;
        dst++;
    }
    *dst = 0;
    ꦮꦭꦶ dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ꦪꦼꦤ꧀ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

static void *myalloc(void *q, unsigned n, unsigned m) {
    (void)q;
    ꦮꦭꦶ calloc(n, m);
}

static void myfree(void *q, void *p) {
    (void)q;
    free(p);
}

typedef sa_nracanaa gzFile_s {
    FILE *file;
    puur_naa_nka write;
    puur_naa_nka err;
    var_na *msg;
    z_stream strm;
} *gzFile;

static gzFile gz_open(const var_na *path, puur_naa_nka fd, const var_na *mode) {
    gzFile gz;
    puur_naa_nka ret;

    gz = malloc(parimaa_na(sa_nracanaa gzFile_s));
    ꦪꦼꦤ꧀ (gz == NULL)
        ꦮꦭꦶ NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ꦪꦼꦤ꧀ (gz->write)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ꦭꦶꦪ {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ꦪꦼꦤ꧀ (ret != Z_OK) {
        free(gz);
        ꦮꦭꦶ NULL;
    }
    gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
                              fopen(path, gz->write ? "wb" : "rb");
    ꦪꦼꦤ꧀ (gz->file == NULL) {
        gz->write ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        free(gz);
        ꦮꦭꦶ NULL;
    }
    gz->err = 0;
    gz->msg = "";
    ꦮꦭꦶ gz;
}

static gzFile gzopen(const var_na *path, const var_na *mode) {
    ꦮꦭꦶ gz_open(path, -1, mode);
}

static gzFile gzdopen(puur_naa_nka fd, const var_na *mode) {
    ꦮꦭꦶ gz_open(NULL, fd, mode);
}

static puur_naa_nka gzwrite(gzFile gz, const void *buf, unsigned len) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    ꦪꦼꦤ꧀ (gz == NULL || !gz->write)
        ꦮꦭꦶ 0;
    strm = &(gz->strm);
    strm->next_in = (void *)buf;
    strm->avail_in = len;
    do {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (void)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ꦱꦼꦢꦼꦭ (strm->avail_out == 0);
    ꦮꦭꦶ (puur_naa_nka)len;
}

static puur_naa_nka gzread(gzFile gz, void *buf, unsigned len) {
    puur_naa_nka ret;
    unsigned got;
    unsigned var_na in[1];
    z_stream *strm;

    ꦪꦼꦤ꧀ (gz == NULL || gz->write)
        ꦮꦭꦶ 0;
    ꦪꦼꦤ꧀ (gz->err)
        ꦮꦭꦶ 0;
    strm = &(gz->strm);
    strm->next_out = (void *)buf;
    strm->avail_out = len;
    do {
        got = (unsigned)fread(in, 1, 1, gz->file);
        ꦪꦼꦤ꧀ (got == 0)
            bha_nga;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ꦪꦼꦤ꧀ (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->msg = strm->msg;
            ꦮꦭꦶ 0;
        }
        ꦪꦼꦤ꧀ (ret == Z_STREAM_END)
            inflateReset(strm);
    } ꦱꦼꦢꦼꦭ (strm->avail_out);
    ꦮꦭꦶ (puur_naa_nka)(len - strm->avail_out);
}

static puur_naa_nka gzclose(gzFile gz) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    ꦪꦼꦤ꧀ (gz == NULL)
        ꦮꦭꦶ Z_STREAM_ERROR;
    strm = &(gz->strm);
    ꦪꦼꦤ꧀ (gz->write) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        do {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (void)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ꦱꦼꦢꦼꦭ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    ꦭꦶꦪ
        inflateEnd(strm);
    fclose(gz->file);
    free(gz);
    ꦮꦭꦶ Z_OK;
}

static const var_na *gzerror(gzFile gz, puur_naa_nka *err) {
    *err = gz->err;
    ꦮꦭꦶ gz->msg;
}

#endif

static var_na *prog;

/* ===========================================================================
 * Display error message and exit
 */
static void error(const var_na *msg) {
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
static puur_naa_nka gz_compress_mmap(FILE *in, gzFile out) {
    puur_naa_nka len;
    puur_naa_nka err;
    puur_naa_nka ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    sa_nracanaa stat sb;

    /* Determine the size of the file, needed for mmap: */
    ꦪꦼꦤ꧀ (fstat(ifd, &sb) < 0) ꦮꦭꦶ Z_ERRNO;
    buf_len = sb.st_size;
    ꦪꦼꦤ꧀ (buf_len <= 0) ꦮꦭꦶ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ꦪꦼꦤ꧀ (buf == (caddr_t)(-1)) ꦮꦭꦶ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (var_na *)buf, (unsigned)buf_len);

    ꦪꦼꦤ꧀ (len != (puur_naa_nka)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    ꦪꦼꦤ꧀ (gzclose(out) != Z_OK) error("failed gzclose");
    ꦮꦭꦶ Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

static void gz_compress(FILE *in, gzFile out) {
    local var_na buf[BUFLEN];
    puur_naa_nka len;
    puur_naa_nka err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ꦪꦼꦤ꧀ (gz_compress_mmap(in, out) == Z_OK) ꦮꦭꦶ;
#endif
    ꦏꦁꦒꦺꦴ (;;) {
        len = (puur_naa_nka)fread(buf, 1, parimaa_na(buf), in);
        ꦪꦼꦤ꧀ (ferror(in)) {
            perror("fread");
            exit(1);
        }
        ꦪꦼꦤ꧀ (len == 0) bha_nga;

        ꦪꦼꦤ꧀ (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    ꦪꦼꦤ꧀ (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
static void gz_uncompress(gzFile in, FILE *out) {
    local var_na buf[BUFLEN];
    puur_naa_nka len;
    puur_naa_nka err;

    ꦏꦁꦒꦺꦴ (;;) {
        len = gzread(in, buf, parimaa_na(buf));
        ꦪꦼꦤ꧀ (len < 0) error (gzerror(in, &err));
        ꦪꦼꦤ꧀ (len == 0) bha_nga;

        ꦪꦼꦤ꧀ ((puur_naa_nka)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    ꦪꦼꦤ꧀ (fclose(out)) error("failed fclose");

    ꦪꦼꦤ꧀ (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
static void file_compress(var_na *file, var_na *mode) {
    local var_na outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    ꦪꦼꦤ꧀ (strlen(file) + strlen(GZ_SUFFIX) >= parimaa_na(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, parimaa_na(outfile));
    string_copy(end, GZ_SUFFIX, parimaa_na(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    ꦪꦼꦤ꧀ (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    ꦪꦼꦤ꧀ (out == NULL) {
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
static void file_uncompress(var_na *file) {
    local var_na buf[MAX_NAME_LEN+1];
    var_na *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    ꦪꦼꦤ꧀ (len + strlen(GZ_SUFFIX) >= parimaa_na(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, parimaa_na(buf));

    ꦪꦼꦤ꧀ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ꦭꦶꦪ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, parimaa_na(buf) - len);
    }
    in = gzopen(infile, "rb");
    ꦪꦼꦤ꧀ (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    ꦪꦼꦤ꧀ (out == NULL) {
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

puur_naa_nka main(puur_naa_nka argc, var_na *argv[]) {
    puur_naa_nka copyout = 0;
    puur_naa_nka uncompr = 0;
    gzFile file;
    var_na *bname, outmode[5];

    string_copy(outmode, "wb6 ", parimaa_na(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ꦪꦼꦤ꧀ (bname)
      bname++;
    ꦭꦶꦪ
      bname = argv[0];
    argc--, argv++;

    ꦪꦼꦤ꧀ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ꦭꦶꦪ ꦪꦼꦤ꧀ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ꦱꦼꦢꦼꦭ (argc > 0) {
      ꦪꦼꦤ꧀ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ꦭꦶꦪ ꦪꦼꦤ꧀ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ꦭꦶꦪ
        bha_nga;
      argc--, argv++;
    }
    ꦪꦼꦤ꧀ (outmode[3] == ' ')
        outmode[3] = 0;
    ꦪꦼꦤ꧀ (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        ꦪꦼꦤ꧀ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ꦪꦼꦤ꧀ (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ꦭꦶꦪ {
            file = gzdopen(fileno(stdout), outmode);
            ꦪꦼꦤ꧀ (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ꦭꦶꦪ {
        ꦪꦼꦤ꧀ (copyout) {
            SET_BINARY_MODE(stdout);
        }
        do {
            ꦪꦼꦤ꧀ (uncompr) {
                ꦪꦼꦤ꧀ (copyout) {
                    file = gzopen(*argv, "rb");
                    ꦪꦼꦤ꧀ (file == NULL)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ꦭꦶꦪ
                        gz_uncompress(file, stdout);
                } ꦭꦶꦪ {
                    file_uncompress(*argv);
                }
            } ꦭꦶꦪ {
                ꦪꦼꦤ꧀ (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    ꦪꦼꦤ꧀ (in == NULL) {
                        perror(*argv);
                    } ꦭꦶꦪ {
                        file = gzdopen(fileno(stdout), outmode);
                        ꦪꦼꦤ꧀ (file == NULL) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } ꦭꦶꦪ {
                    file_compress(*argv, outmode);
                }
            }
        } ꦱꦼꦢꦼꦭ (argv++, --argc);
    }
    ꦮꦭꦶ 0;
}
