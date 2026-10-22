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

#ܐܢ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ܐܢ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ضمن "zlib.h"
#ضمن <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#ܐܢ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#ܐܚܪܢܐ
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ܐܢ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#ܐܢ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ܐܢ !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  خارجي صحيح unlink(ثابت حرف *);
#endif
#endif

#ܐܢ defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

ساكن حرف *strwinerror (error)
     DWORD error;
{
    ساكن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        عدم,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        عدم);
    ܐܢ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ܐܢ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ܐܢ (chars > حجم (buf) - 1) {
            chars = حجم (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ܐܚܪܢܐ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ܦܢܐ buf;
}

ساكن فراغ pwinerror (s)
    ثابت حرف *s;
{
    ܐܢ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    ܐܚܪܢܐ
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (حجم(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local ساكن
   /* Needed for systems with limitation on stack size. */
#ܐܚܪܢܐ
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ساكن حرف *string_copy(حرف *dst, حرف ثابت *src, z_size_t len) {
    ܐܢ (len == 0)
        ܦܢܐ عدم;
    ܟܡܐ (--len) {
        *dst = *src++;
        ܐܢ (*dst == 0)
            ܦܢܐ dst;
        dst++;
    }
    *dst = 0;
    ܦܢܐ dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ܐܢ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

ساكن فراغ *myalloc(فراغ *q, unsigned n, unsigned m) {
    (فراغ)q;
    ܦܢܐ calloc(n, m);
}

ساكن فراغ myfree(فراغ *q, فراغ *p) {
    (فراغ)q;
    حرر(p);
}

تعريفنوع بنية gzFile_s {
    FILE *file;
    صحيح اكتب;
    صحيح err;
    حرف *msg;
    z_stream strm;
} *gzFile;

ساكن gzFile gz_open(ثابت حرف *path, صحيح fd, ثابت حرف *mode) {
    gzFile gz;
    صحيح ret;

    gz = malloc(حجم(بنية gzFile_s));
    ܐܢ (gz == عدم)
        ܦܢܐ عدم;
    gz->اكتب = strchr(mode, 'w') != عدم;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ܐܢ (gz->اكتب)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ܐܚܪܢܐ {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ܐܢ (ret != Z_OK) {
        حرر(gz);
        ܦܢܐ عدم;
    }
    gz->file = path == عدم ? fdopen(fd, gz->اكتب ? "wb" : "rb") :
                              fopen(path, gz->اكتب ? "wb" : "rb");
    ܐܢ (gz->file == عدم) {
        gz->اكتب ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        حرر(gz);
        ܦܢܐ عدم;
    }
    gz->err = 0;
    gz->msg = "";
    ܦܢܐ gz;
}

ساكن gzFile gzopen(ثابت حرف *path, ثابت حرف *mode) {
    ܦܢܐ gz_open(path, -1, mode);
}

ساكن gzFile gzdopen(صحيح fd, ثابت حرف *mode) {
    ܦܢܐ gz_open(عدم, fd, mode);
}

ساكن صحيح gzwrite(gzFile gz, ثابت فراغ *buf, unsigned len) {
    z_stream *strm;
    unsigned حرف out[BUFLEN];

    ܐܢ (gz == عدم || !gz->اكتب)
        ܦܢܐ 0;
    strm = &(gz->strm);
    strm->next_in = (فراغ *)buf;
    strm->avail_in = len;
    ܥܒܕ {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (فراغ)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ܟܡܐ (strm->avail_out == 0);
    ܦܢܐ (صحيح)len;
}

ساكن صحيح gzread(gzFile gz, فراغ *buf, unsigned len) {
    صحيح ret;
    unsigned got;
    unsigned حرف in[1];
    z_stream *strm;

    ܐܢ (gz == عدم || gz->اكتب)
        ܦܢܐ 0;
    ܐܢ (gz->err)
        ܦܢܐ 0;
    strm = &(gz->strm);
    strm->next_out = (فراغ *)buf;
    strm->avail_out = len;
    ܥܒܕ {
        got = (unsigned)fread(in, 1, 1, gz->file);
        ܐܢ (got == 0)
            اكسر;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ܐܢ (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->msg = strm->msg;
            ܦܢܐ 0;
        }
        ܐܢ (ret == Z_STREAM_END)
            inflateReset(strm);
    } ܟܡܐ (strm->avail_out);
    ܦܢܐ (صحيح)(len - strm->avail_out);
}

ساكن صحيح gzclose(gzFile gz) {
    z_stream *strm;
    unsigned حرف out[BUFLEN];

    ܐܢ (gz == عدم)
        ܦܢܐ Z_STREAM_ERROR;
    strm = &(gz->strm);
    ܐܢ (gz->اكتب) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        ܥܒܕ {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (فراغ)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ܟܡܐ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    ܐܚܪܢܐ
        inflateEnd(strm);
    fclose(gz->file);
    حرر(gz);
    ܦܢܐ Z_OK;
}

ساكن ثابت حرف *gzerror(gzFile gz, صحيح *err) {
    *err = gz->err;
    ܦܢܐ gz->msg;
}

#endif

ساكن حرف *prog;

/* ===========================================================================
 * Display error message and exit
 */
ساكن فراغ error(ثابت حرف *msg) {
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ساكن صحيح gz_compress_mmap(FILE *in, gzFile out) {
    صحيح len;
    صحيح err;
    صحيح ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    بنية stat sb;

    /* Determine the size of the file, needed for mmap: */
    ܐܢ (fstat(ifd, &sb) < 0) ܦܢܐ Z_ERRNO;
    buf_len = sb.st_size;
    ܐܢ (buf_len <= 0) ܦܢܐ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ܐܢ (buf == (caddr_t)(-1)) ܦܢܐ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (حرف *)buf, (unsigned)buf_len);

    ܐܢ (len != (صحيح)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    ܐܢ (gzclose(out) != Z_OK) error("failed gzclose");
    ܦܢܐ Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ساكن فراغ gz_compress(FILE *in, gzFile out) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ܐܢ (gz_compress_mmap(in, out) == Z_OK) ܦܢܐ;
#endif
    لكل (;;) {
        len = (صحيح)fread(buf, 1, حجم(buf), in);
        ܐܢ (ferror(in)) {
            perror("fread");
            exit(1);
        }
        ܐܢ (len == 0) اكسر;

        ܐܢ (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    ܐܢ (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ساكن فراغ gz_uncompress(gzFile in, FILE *out) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

    لكل (;;) {
        len = gzread(in, buf, حجم(buf));
        ܐܢ (len < 0) error (gzerror(in, &err));
        ܐܢ (len == 0) اكسر;

        ܐܢ ((صحيح)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    ܐܢ (fclose(out)) error("failed fclose");

    ܐܢ (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ساكن فراغ file_compress(حرف *file, حرف *mode) {
    local حرف outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    ܐܢ (strlen(file) + strlen(GZ_SUFFIX) >= حجم(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, حجم(outfile));
    string_copy(end, GZ_SUFFIX, حجم(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    ܐܢ (in == عدم) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    ܐܢ (out == عدم) {
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
ساكن فراغ file_uncompress(حرف *file) {
    local حرف buf[MAX_NAME_LEN+1];
    حرف *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    ܐܢ (len + strlen(GZ_SUFFIX) >= حجم(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, حجم(buf));

    ܐܢ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ܐܚܪܢܐ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, حجم(buf) - len);
    }
    in = gzopen(infile, "rb");
    ܐܢ (in == عدم) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    ܐܢ (out == عدم) {
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

صحيح main(صحيح argc, حرف *argv[]) {
    صحيح copyout = 0;
    صحيح uncompr = 0;
    gzFile file;
    حرف *bname, outmode[5];

    string_copy(outmode, "wb6 ", حجم(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ܐܢ (bname)
      bname++;
    ܐܚܪܢܐ
      bname = argv[0];
    argc--, argv++;

    ܐܢ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ܐܚܪܢܐ ܐܢ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ܟܡܐ (argc > 0) {
      ܐܢ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ܐܚܪܢܐ ܐܢ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ܐܚܪܢܐ ܐܢ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ܐܚܪܢܐ ܐܢ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ܐܚܪܢܐ ܐܢ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ܐܚܪܢܐ ܐܢ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ܐܚܪܢܐ
        اكسر;
      argc--, argv++;
    }
    ܐܢ (outmode[3] == ' ')
        outmode[3] = 0;
    ܐܢ (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        ܐܢ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ܐܢ (file == عدم) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ܐܚܪܢܐ {
            file = gzdopen(fileno(stdout), outmode);
            ܐܢ (file == عدم) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ܐܚܪܢܐ {
        ܐܢ (copyout) {
            SET_BINARY_MODE(stdout);
        }
        ܥܒܕ {
            ܐܢ (uncompr) {
                ܐܢ (copyout) {
                    file = gzopen(*argv, "rb");
                    ܐܢ (file == عدم)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ܐܚܪܢܐ
                        gz_uncompress(file, stdout);
                } ܐܚܪܢܐ {
                    file_uncompress(*argv);
                }
            } ܐܚܪܢܐ {
                ܐܢ (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    ܐܢ (in == عدم) {
                        perror(*argv);
                    } ܐܚܪܢܐ {
                        file = gzdopen(fileno(stdout), outmode);
                        ܐܢ (file == عدم) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } ܐܚܪܢܐ {
                    file_compress(*argv, outmode);
                }
            }
        } ܟܡܐ (argv++, --argc);
    }
    ܦܢܐ 0;
}
