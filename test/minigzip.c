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

#ᮚᮦᮔ᮪ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ᮚᮦᮔ᮪ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
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

#ᮚᮦᮔ᮪ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#ᮜᮤᮚ
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ᮚᮦᮔ᮪ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#ᮚᮦᮔ᮪ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ᮚᮦᮔ᮪ !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  extern puur_naa_nka unlink(const var_na *);
#endif
#endif

#ᮚᮦᮔ᮪ defined(UNDER_CE)
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
    ᮚᮦᮔ᮪ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ᮚᮦᮔ᮪ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ᮚᮦᮔ᮪ (chars > parimaa_na (buf) - 1) {
            chars = parimaa_na (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ᮜᮤᮚ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ᮝᮜᮤ buf;
}

static void pwinerror (s)
    const var_na *s;
{
    ᮚᮦᮔ᮪ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    ᮜᮤᮚ
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
#ᮜᮤᮚ
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
static var_na *string_copy(var_na *dst, var_na const *src, z_size_t len) {
    ᮚᮦᮔ᮪ (len == 0)
        ᮝᮜᮤ NULL;
    ᮞᮨᮓᮨᮜ (--len) {
        *dst = *src++;
        ᮚᮦᮔ᮪ (*dst == 0)
            ᮝᮜᮤ dst;
        dst++;
    }
    *dst = 0;
    ᮝᮜᮤ dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ᮚᮦᮔ᮪ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

static void *myalloc(void *q, unsigned n, unsigned m) {
    (void)q;
    ᮝᮜᮤ calloc(n, m);
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
    ᮚᮦᮔ᮪ (gz == NULL)
        ᮝᮜᮤ NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ᮚᮦᮔ᮪ (gz->write)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ᮜᮤᮚ {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ᮚᮦᮔ᮪ (ret != Z_OK) {
        free(gz);
        ᮝᮜᮤ NULL;
    }
    gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
                              fopen(path, gz->write ? "wb" : "rb");
    ᮚᮦᮔ᮪ (gz->file == NULL) {
        gz->write ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        free(gz);
        ᮝᮜᮤ NULL;
    }
    gz->err = 0;
    gz->msg = "";
    ᮝᮜᮤ gz;
}

static gzFile gzopen(const var_na *path, const var_na *mode) {
    ᮝᮜᮤ gz_open(path, -1, mode);
}

static gzFile gzdopen(puur_naa_nka fd, const var_na *mode) {
    ᮝᮜᮤ gz_open(NULL, fd, mode);
}

static puur_naa_nka gzwrite(gzFile gz, const void *buf, unsigned len) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    ᮚᮦᮔ᮪ (gz == NULL || !gz->write)
        ᮝᮜᮤ 0;
    strm = &(gz->strm);
    strm->next_in = (void *)buf;
    strm->avail_in = len;
    do {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (void)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ᮞᮨᮓᮨᮜ (strm->avail_out == 0);
    ᮝᮜᮤ (puur_naa_nka)len;
}

static puur_naa_nka gzread(gzFile gz, void *buf, unsigned len) {
    puur_naa_nka ret;
    unsigned got;
    unsigned var_na in[1];
    z_stream *strm;

    ᮚᮦᮔ᮪ (gz == NULL || gz->write)
        ᮝᮜᮤ 0;
    ᮚᮦᮔ᮪ (gz->err)
        ᮝᮜᮤ 0;
    strm = &(gz->strm);
    strm->next_out = (void *)buf;
    strm->avail_out = len;
    do {
        got = (unsigned)fread(in, 1, 1, gz->file);
        ᮚᮦᮔ᮪ (got == 0)
            bha_nga;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ᮚᮦᮔ᮪ (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->msg = strm->msg;
            ᮝᮜᮤ 0;
        }
        ᮚᮦᮔ᮪ (ret == Z_STREAM_END)
            inflateReset(strm);
    } ᮞᮨᮓᮨᮜ (strm->avail_out);
    ᮝᮜᮤ (puur_naa_nka)(len - strm->avail_out);
}

static puur_naa_nka gzclose(gzFile gz) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    ᮚᮦᮔ᮪ (gz == NULL)
        ᮝᮜᮤ Z_STREAM_ERROR;
    strm = &(gz->strm);
    ᮚᮦᮔ᮪ (gz->write) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        do {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (void)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ᮞᮨᮓᮨᮜ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    ᮜᮤᮚ
        inflateEnd(strm);
    fclose(gz->file);
    free(gz);
    ᮝᮜᮤ Z_OK;
}

static const var_na *gzerror(gzFile gz, puur_naa_nka *err) {
    *err = gz->err;
    ᮝᮜᮤ gz->msg;
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
    ᮚᮦᮔ᮪ (fstat(ifd, &sb) < 0) ᮝᮜᮤ Z_ERRNO;
    buf_len = sb.st_size;
    ᮚᮦᮔ᮪ (buf_len <= 0) ᮝᮜᮤ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ᮚᮦᮔ᮪ (buf == (caddr_t)(-1)) ᮝᮜᮤ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (var_na *)buf, (unsigned)buf_len);

    ᮚᮦᮔ᮪ (len != (puur_naa_nka)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    ᮚᮦᮔ᮪ (gzclose(out) != Z_OK) error("failed gzclose");
    ᮝᮜᮤ Z_OK;
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
    ᮚᮦᮔ᮪ (gz_compress_mmap(in, out) == Z_OK) ᮝᮜᮤ;
#endif
    for (;;) {
        len = (puur_naa_nka)fread(buf, 1, parimaa_na(buf), in);
        ᮚᮦᮔ᮪ (ferror(in)) {
            perror("fread");
            exit(1);
        }
        ᮚᮦᮔ᮪ (len == 0) bha_nga;

        ᮚᮦᮔ᮪ (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    ᮚᮦᮔ᮪ (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
static void gz_uncompress(gzFile in, FILE *out) {
    local var_na buf[BUFLEN];
    puur_naa_nka len;
    puur_naa_nka err;

    for (;;) {
        len = gzread(in, buf, parimaa_na(buf));
        ᮚᮦᮔ᮪ (len < 0) error (gzerror(in, &err));
        ᮚᮦᮔ᮪ (len == 0) bha_nga;

        ᮚᮦᮔ᮪ ((puur_naa_nka)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    ᮚᮦᮔ᮪ (fclose(out)) error("failed fclose");

    ᮚᮦᮔ᮪ (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
static void file_compress(var_na *file, var_na *mode) {
    local var_na outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    ᮚᮦᮔ᮪ (strlen(file) + strlen(GZ_SUFFIX) >= parimaa_na(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, parimaa_na(outfile));
    string_copy(end, GZ_SUFFIX, parimaa_na(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    ᮚᮦᮔ᮪ (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    ᮚᮦᮔ᮪ (out == NULL) {
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

    ᮚᮦᮔ᮪ (len + strlen(GZ_SUFFIX) >= parimaa_na(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, parimaa_na(buf));

    ᮚᮦᮔ᮪ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ᮜᮤᮚ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, parimaa_na(buf) - len);
    }
    in = gzopen(infile, "rb");
    ᮚᮦᮔ᮪ (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    ᮚᮦᮔ᮪ (out == NULL) {
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
    ᮚᮦᮔ᮪ (bname)
      bname++;
    ᮜᮤᮚ
      bname = argv[0];
    argc--, argv++;

    ᮚᮦᮔ᮪ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ᮜᮤᮚ ᮚᮦᮔ᮪ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ᮞᮨᮓᮨᮜ (argc > 0) {
      ᮚᮦᮔ᮪ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ᮜᮤᮚ ᮚᮦᮔ᮪ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ᮜᮤᮚ
        bha_nga;
      argc--, argv++;
    }
    ᮚᮦᮔ᮪ (outmode[3] == ' ')
        outmode[3] = 0;
    ᮚᮦᮔ᮪ (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        ᮚᮦᮔ᮪ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ᮚᮦᮔ᮪ (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ᮜᮤᮚ {
            file = gzdopen(fileno(stdout), outmode);
            ᮚᮦᮔ᮪ (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ᮜᮤᮚ {
        ᮚᮦᮔ᮪ (copyout) {
            SET_BINARY_MODE(stdout);
        }
        do {
            ᮚᮦᮔ᮪ (uncompr) {
                ᮚᮦᮔ᮪ (copyout) {
                    file = gzopen(*argv, "rb");
                    ᮚᮦᮔ᮪ (file == NULL)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ᮜᮤᮚ
                        gz_uncompress(file, stdout);
                } ᮜᮤᮚ {
                    file_uncompress(*argv);
                }
            } ᮜᮤᮚ {
                ᮚᮦᮔ᮪ (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    ᮚᮦᮔ᮪ (in == NULL) {
                        perror(*argv);
                    } ᮜᮤᮚ {
                        file = gzdopen(fileno(stdout), outmode);
                        ᮚᮦᮔ᮪ (file == NULL) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } ᮜᮤᮚ {
                    file_compress(*argv, outmode);
                }
            }
        } ᮞᮨᮓᮨᮜ (argv++, --argc);
    }
    ᮝᮜᮤ 0;
}
