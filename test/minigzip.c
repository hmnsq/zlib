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

#ຖ້າ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ຖ້າ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
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

#ຖ້າ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#ບໍ່ດັ່ງນັ້ນ
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ຖ້າ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#ຖ້າ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ຖ້າ !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  extern ຈຳນວນເຕັມ unlink(const var_na *);
#endif
#endif

#ຖ້າ defined(UNDER_CE)
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
    ຖ້າ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ຖ້າ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ຖ້າ (chars > parimaa_na (buf) - 1) {
            chars = parimaa_na (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ບໍ່ດັ່ງນັ້ນ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ສົ່ງຄືນ buf;
}

static void pwinerror (s)
    const var_na *s;
{
    ຖ້າ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    ບໍ່ດັ່ງນັ້ນ
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
#ບໍ່ດັ່ງນັ້ນ
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
static var_na *string_copy(var_na *dst, var_na const *src, z_size_t len) {
    ຖ້າ (len == 0)
        ສົ່ງຄືນ NULL;
    ໃນຂະນະທີ່ (--len) {
        *dst = *src++;
        ຖ້າ (*dst == 0)
            ສົ່ງຄືນ dst;
        dst++;
    }
    *dst = 0;
    ສົ່ງຄືນ dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ຖ້າ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

static void *myalloc(void *q, unsigned n, unsigned m) {
    (void)q;
    ສົ່ງຄືນ calloc(n, m);
}

static void myfree(void *q, void *p) {
    (void)q;
    free(p);
}

typedef sa_nracanaa gzFile_s {
    FILE *file;
    ຈຳນວນເຕັມ write;
    ຈຳນວນເຕັມ err;
    var_na *msg;
    z_stream strm;
} *gzFile;

static gzFile gz_open(const var_na *path, ຈຳນວນເຕັມ fd, const var_na *mode) {
    gzFile gz;
    ຈຳນວນເຕັມ ret;

    gz = malloc(parimaa_na(sa_nracanaa gzFile_s));
    ຖ້າ (gz == NULL)
        ສົ່ງຄືນ NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ຖ້າ (gz->write)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ບໍ່ດັ່ງນັ້ນ {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ຖ້າ (ret != Z_OK) {
        free(gz);
        ສົ່ງຄືນ NULL;
    }
    gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
                              fopen(path, gz->write ? "wb" : "rb");
    ຖ້າ (gz->file == NULL) {
        gz->write ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        free(gz);
        ສົ່ງຄືນ NULL;
    }
    gz->err = 0;
    gz->msg = "";
    ສົ່ງຄືນ gz;
}

static gzFile gzopen(const var_na *path, const var_na *mode) {
    ສົ່ງຄືນ gz_open(path, -1, mode);
}

static gzFile gzdopen(ຈຳນວນເຕັມ fd, const var_na *mode) {
    ສົ່ງຄືນ gz_open(NULL, fd, mode);
}

static ຈຳນວນເຕັມ gzwrite(gzFile gz, const void *buf, unsigned len) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    ຖ້າ (gz == NULL || !gz->write)
        ສົ່ງຄືນ 0;
    strm = &(gz->strm);
    strm->next_in = (void *)buf;
    strm->avail_in = len;
    do {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (void)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ໃນຂະນະທີ່ (strm->avail_out == 0);
    ສົ່ງຄືນ (ຈຳນວນເຕັມ)len;
}

static ຈຳນວນເຕັມ gzread(gzFile gz, void *buf, unsigned len) {
    ຈຳນວນເຕັມ ret;
    unsigned got;
    unsigned var_na in[1];
    z_stream *strm;

    ຖ້າ (gz == NULL || gz->write)
        ສົ່ງຄືນ 0;
    ຖ້າ (gz->err)
        ສົ່ງຄືນ 0;
    strm = &(gz->strm);
    strm->next_out = (void *)buf;
    strm->avail_out = len;
    do {
        got = (unsigned)fread(in, 1, 1, gz->file);
        ຖ້າ (got == 0)
            bha_nga;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ຖ້າ (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->msg = strm->msg;
            ສົ່ງຄືນ 0;
        }
        ຖ້າ (ret == Z_STREAM_END)
            inflateReset(strm);
    } ໃນຂະນະທີ່ (strm->avail_out);
    ສົ່ງຄືນ (ຈຳນວນເຕັມ)(len - strm->avail_out);
}

static ຈຳນວນເຕັມ gzclose(gzFile gz) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    ຖ້າ (gz == NULL)
        ສົ່ງຄືນ Z_STREAM_ERROR;
    strm = &(gz->strm);
    ຖ້າ (gz->write) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        do {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (void)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ໃນຂະນະທີ່ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    ບໍ່ດັ່ງນັ້ນ
        inflateEnd(strm);
    fclose(gz->file);
    free(gz);
    ສົ່ງຄືນ Z_OK;
}

static const var_na *gzerror(gzFile gz, ຈຳນວນເຕັມ *err) {
    *err = gz->err;
    ສົ່ງຄືນ gz->msg;
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
static ຈຳນວນເຕັມ gz_compress_mmap(FILE *in, gzFile out) {
    ຈຳນວນເຕັມ len;
    ຈຳນວນເຕັມ err;
    ຈຳນວນເຕັມ ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    sa_nracanaa stat sb;

    /* Determine the size of the file, needed for mmap: */
    ຖ້າ (fstat(ifd, &sb) < 0) ສົ່ງຄືນ Z_ERRNO;
    buf_len = sb.st_size;
    ຖ້າ (buf_len <= 0) ສົ່ງຄືນ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ຖ້າ (buf == (caddr_t)(-1)) ສົ່ງຄືນ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (var_na *)buf, (unsigned)buf_len);

    ຖ້າ (len != (ຈຳນວນເຕັມ)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    ຖ້າ (gzclose(out) != Z_OK) error("failed gzclose");
    ສົ່ງຄືນ Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

static void gz_compress(FILE *in, gzFile out) {
    local var_na buf[BUFLEN];
    ຈຳນວນເຕັມ len;
    ຈຳນວນເຕັມ err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ຖ້າ (gz_compress_mmap(in, out) == Z_OK) ສົ່ງຄືນ;
#endif
    ສຳລັບ (;;) {
        len = (ຈຳນວນເຕັມ)fread(buf, 1, parimaa_na(buf), in);
        ຖ້າ (ferror(in)) {
            perror("fread");
            exit(1);
        }
        ຖ້າ (len == 0) bha_nga;

        ຖ້າ (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    ຖ້າ (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
static void gz_uncompress(gzFile in, FILE *out) {
    local var_na buf[BUFLEN];
    ຈຳນວນເຕັມ len;
    ຈຳນວນເຕັມ err;

    ສຳລັບ (;;) {
        len = gzread(in, buf, parimaa_na(buf));
        ຖ້າ (len < 0) error (gzerror(in, &err));
        ຖ້າ (len == 0) bha_nga;

        ຖ້າ ((ຈຳນວນເຕັມ)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    ຖ້າ (fclose(out)) error("failed fclose");

    ຖ້າ (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
static void file_compress(var_na *file, var_na *mode) {
    local var_na outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    ຖ້າ (strlen(file) + strlen(GZ_SUFFIX) >= parimaa_na(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, parimaa_na(outfile));
    string_copy(end, GZ_SUFFIX, parimaa_na(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    ຖ້າ (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    ຖ້າ (out == NULL) {
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

    ຖ້າ (len + strlen(GZ_SUFFIX) >= parimaa_na(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, parimaa_na(buf));

    ຖ້າ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ບໍ່ດັ່ງນັ້ນ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, parimaa_na(buf) - len);
    }
    in = gzopen(infile, "rb");
    ຖ້າ (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    ຖ້າ (out == NULL) {
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

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na *argv[]) {
    ຈຳນວນເຕັມ copyout = 0;
    ຈຳນວນເຕັມ uncompr = 0;
    gzFile file;
    var_na *bname, outmode[5];

    string_copy(outmode, "wb6 ", parimaa_na(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ຖ້າ (bname)
      bname++;
    ບໍ່ດັ່ງນັ້ນ
      bname = argv[0];
    argc--, argv++;

    ຖ້າ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ໃນຂະນະທີ່ (argc > 0) {
      ຖ້າ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ບໍ່ດັ່ງນັ້ນ ຖ້າ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ບໍ່ດັ່ງນັ້ນ
        bha_nga;
      argc--, argv++;
    }
    ຖ້າ (outmode[3] == ' ')
        outmode[3] = 0;
    ຖ້າ (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        ຖ້າ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ຖ້າ (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ບໍ່ດັ່ງນັ້ນ {
            file = gzdopen(fileno(stdout), outmode);
            ຖ້າ (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ບໍ່ດັ່ງນັ້ນ {
        ຖ້າ (copyout) {
            SET_BINARY_MODE(stdout);
        }
        do {
            ຖ້າ (uncompr) {
                ຖ້າ (copyout) {
                    file = gzopen(*argv, "rb");
                    ຖ້າ (file == NULL)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ບໍ່ດັ່ງນັ້ນ
                        gz_uncompress(file, stdout);
                } ບໍ່ດັ່ງນັ້ນ {
                    file_uncompress(*argv);
                }
            } ບໍ່ດັ່ງນັ້ນ {
                ຖ້າ (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    ຖ້າ (in == NULL) {
                        perror(*argv);
                    } ບໍ່ດັ່ງນັ້ນ {
                        file = gzdopen(fileno(stdout), outmode);
                        ຖ້າ (file == NULL) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } ບໍ່ດັ່ງນັ້ນ {
                    file_compress(*argv, outmode);
                }
            }
        } ໃນຂະນະທີ່ (argv++, --argc);
    }
    ສົ່ງຄືນ 0;
}
