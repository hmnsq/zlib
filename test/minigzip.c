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

#အကယ်၍ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#အကယ်၍ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
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

#အကယ်၍ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#မဟုတ်ပါက
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#အကယ်၍ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#အကယ်၍ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#အကယ်၍ !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  extern ကိန်းပြည့် unlink(const var_na *);
#endif
#endif

#အကယ်၍ defined(UNDER_CE)
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
    အကယ်၍ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        အကယ်၍ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        အကယ်၍ (chars > parimaa_na (buf) - 1) {
            chars = parimaa_na (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    မဟုတ်ပါက {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ပြန်ပေး buf;
}

static void pwinerror (s)
    const var_na *s;
{
    အကယ်၍ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    မဟုတ်ပါက
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
#မဟုတ်ပါက
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
static var_na *string_copy(var_na *dst, var_na const *src, z_size_t len) {
    အကယ်၍ (len == 0)
        ပြန်ပေး NULL;
    နေစဉ် (--len) {
        *dst = *src++;
        အကယ်၍ (*dst == 0)
            ပြန်ပေး dst;
        dst++;
    }
    *dst = 0;
    ပြန်ပေး dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#အကယ်၍ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

static void *myalloc(void *q, unsigned n, unsigned m) {
    (void)q;
    ပြန်ပေး calloc(n, m);
}

static void myfree(void *q, void *p) {
    (void)q;
    free(p);
}

typedef sa_nracanaa gzFile_s {
    FILE *file;
    ကိန်းပြည့် write;
    ကိန်းပြည့် err;
    var_na *msg;
    z_stream strm;
} *gzFile;

static gzFile gz_open(const var_na *path, ကိန်းပြည့် fd, const var_na *mode) {
    gzFile gz;
    ကိန်းပြည့် ret;

    gz = malloc(parimaa_na(sa_nracanaa gzFile_s));
    အကယ်၍ (gz == NULL)
        ပြန်ပေး NULL;
    gz->write = strchr(mode, 'w') != NULL;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    အကယ်၍ (gz->write)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    မဟုတ်ပါက {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    အကယ်၍ (ret != Z_OK) {
        free(gz);
        ပြန်ပေး NULL;
    }
    gz->file = path == NULL ? fdopen(fd, gz->write ? "wb" : "rb") :
                              fopen(path, gz->write ? "wb" : "rb");
    အကယ်၍ (gz->file == NULL) {
        gz->write ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        free(gz);
        ပြန်ပေး NULL;
    }
    gz->err = 0;
    gz->msg = "";
    ပြန်ပေး gz;
}

static gzFile gzopen(const var_na *path, const var_na *mode) {
    ပြန်ပေး gz_open(path, -1, mode);
}

static gzFile gzdopen(ကိန်းပြည့် fd, const var_na *mode) {
    ပြန်ပေး gz_open(NULL, fd, mode);
}

static ကိန်းပြည့် gzwrite(gzFile gz, const void *buf, unsigned len) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    အကယ်၍ (gz == NULL || !gz->write)
        ပြန်ပေး 0;
    strm = &(gz->strm);
    strm->next_in = (void *)buf;
    strm->avail_in = len;
    do {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (void)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } နေစဉ် (strm->avail_out == 0);
    ပြန်ပေး (ကိန်းပြည့်)len;
}

static ကိန်းပြည့် gzread(gzFile gz, void *buf, unsigned len) {
    ကိန်းပြည့် ret;
    unsigned got;
    unsigned var_na in[1];
    z_stream *strm;

    အကယ်၍ (gz == NULL || gz->write)
        ပြန်ပေး 0;
    အကယ်၍ (gz->err)
        ပြန်ပေး 0;
    strm = &(gz->strm);
    strm->next_out = (void *)buf;
    strm->avail_out = len;
    do {
        got = (unsigned)fread(in, 1, 1, gz->file);
        အကယ်၍ (got == 0)
            bha_nga;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        အကယ်၍ (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->msg = strm->msg;
            ပြန်ပေး 0;
        }
        အကယ်၍ (ret == Z_STREAM_END)
            inflateReset(strm);
    } နေစဉ် (strm->avail_out);
    ပြန်ပေး (ကိန်းပြည့်)(len - strm->avail_out);
}

static ကိန်းပြည့် gzclose(gzFile gz) {
    z_stream *strm;
    unsigned var_na out[BUFLEN];

    အကယ်၍ (gz == NULL)
        ပြန်ပေး Z_STREAM_ERROR;
    strm = &(gz->strm);
    အကယ်၍ (gz->write) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        do {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (void)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } နေစဉ် (strm->avail_out == 0);
        deflateEnd(strm);
    }
    မဟုတ်ပါက
        inflateEnd(strm);
    fclose(gz->file);
    free(gz);
    ပြန်ပေး Z_OK;
}

static const var_na *gzerror(gzFile gz, ကိန်းပြည့် *err) {
    *err = gz->err;
    ပြန်ပေး gz->msg;
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
static ကိန်းပြည့် gz_compress_mmap(FILE *in, gzFile out) {
    ကိန်းပြည့် len;
    ကိန်းပြည့် err;
    ကိန်းပြည့် ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    sa_nracanaa stat sb;

    /* Determine the size of the file, needed for mmap: */
    အကယ်၍ (fstat(ifd, &sb) < 0) ပြန်ပေး Z_ERRNO;
    buf_len = sb.st_size;
    အကယ်၍ (buf_len <= 0) ပြန်ပေး Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    အကယ်၍ (buf == (caddr_t)(-1)) ပြန်ပေး Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (var_na *)buf, (unsigned)buf_len);

    အကယ်၍ (len != (ကိန်းပြည့်)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    အကယ်၍ (gzclose(out) != Z_OK) error("failed gzclose");
    ပြန်ပေး Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

static void gz_compress(FILE *in, gzFile out) {
    local var_na buf[BUFLEN];
    ကိန်းပြည့် len;
    ကိန်းပြည့် err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    အကယ်၍ (gz_compress_mmap(in, out) == Z_OK) ပြန်ပေး;
#endif
    အတွက် (;;) {
        len = (ကိန်းပြည့်)fread(buf, 1, parimaa_na(buf), in);
        အကယ်၍ (ferror(in)) {
            perror("fread");
            exit(1);
        }
        အကယ်၍ (len == 0) bha_nga;

        အကယ်၍ (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    အကယ်၍ (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
static void gz_uncompress(gzFile in, FILE *out) {
    local var_na buf[BUFLEN];
    ကိန်းပြည့် len;
    ကိန်းပြည့် err;

    အတွက် (;;) {
        len = gzread(in, buf, parimaa_na(buf));
        အကယ်၍ (len < 0) error (gzerror(in, &err));
        အကယ်၍ (len == 0) bha_nga;

        အကယ်၍ ((ကိန်းပြည့်)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    အကယ်၍ (fclose(out)) error("failed fclose");

    အကယ်၍ (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
static void file_compress(var_na *file, var_na *mode) {
    local var_na outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    အကယ်၍ (strlen(file) + strlen(GZ_SUFFIX) >= parimaa_na(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, parimaa_na(outfile));
    string_copy(end, GZ_SUFFIX, parimaa_na(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    အကယ်၍ (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    အကယ်၍ (out == NULL) {
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

    အကယ်၍ (len + strlen(GZ_SUFFIX) >= parimaa_na(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, parimaa_na(buf));

    အကယ်၍ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } မဟုတ်ပါက {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, parimaa_na(buf) - len);
    }
    in = gzopen(infile, "rb");
    အကယ်၍ (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    အကယ်၍ (out == NULL) {
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

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[]) {
    ကိန်းပြည့် copyout = 0;
    ကိန်းပြည့် uncompr = 0;
    gzFile file;
    var_na *bname, outmode[5];

    string_copy(outmode, "wb6 ", parimaa_na(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    အကယ်၍ (bname)
      bname++;
    မဟုတ်ပါက
      bname = argv[0];
    argc--, argv++;

    အကယ်၍ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    မဟုတ်ပါက အကယ်၍ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    နေစဉ် (argc > 0) {
      အကယ်၍ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      မဟုတ်ပါက အကယ်၍ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      မဟုတ်ပါက အကယ်၍ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      မဟုတ်ပါက အကယ်၍ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      မဟုတ်ပါက အကယ်၍ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      မဟုတ်ပါက အကယ်၍ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      မဟုတ်ပါက
        bha_nga;
      argc--, argv++;
    }
    အကယ်၍ (outmode[3] == ' ')
        outmode[3] = 0;
    အကယ်၍ (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        အကယ်၍ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            အကယ်၍ (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } မဟုတ်ပါက {
            file = gzdopen(fileno(stdout), outmode);
            အကယ်၍ (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } မဟုတ်ပါက {
        အကယ်၍ (copyout) {
            SET_BINARY_MODE(stdout);
        }
        do {
            အကယ်၍ (uncompr) {
                အကယ်၍ (copyout) {
                    file = gzopen(*argv, "rb");
                    အကယ်၍ (file == NULL)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    မဟုတ်ပါက
                        gz_uncompress(file, stdout);
                } မဟုတ်ပါက {
                    file_uncompress(*argv);
                }
            } မဟုတ်ပါက {
                အကယ်၍ (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    အကယ်၍ (in == NULL) {
                        perror(*argv);
                    } မဟုတ်ပါက {
                        file = gzdopen(fileno(stdout), outmode);
                        အကယ်၍ (file == NULL) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } မဟုတ်ပါက {
                    file_compress(*argv, outmode);
                }
            }
        } နေစဉ် (argv++, --argc);
    }
    ပြန်ပေး 0;
}
