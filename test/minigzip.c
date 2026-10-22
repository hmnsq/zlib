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

#যদি defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#যদি defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#অন্তর্ভাব "zlib.h"
#অন্তর্ভাব <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#যদি defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_দ্বিমিক_MODE(file) setmode(fileno(file), O_দ্বিমিক)
#অন্যথা
#  define SET_দ্বিমিক_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#যদি defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#যদি defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#যদি !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  বাহ্যিক পূর্ণসংখ্যা unlink(ধ্রুবক অক্ষর *);
#endif
#endif

#যদি defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

স্থির অক্ষর *strwinerror (error)
     DWORD error;
{
    স্থির অক্ষর buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_বাফার,
        ফাঁকা,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ফাঁকা);
    যদি (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        যদি (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        যদি (chars > আকার (buf) - 1) {
            chars = আকার (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    অন্যথা {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ফেরত buf;
}

স্থির শূন্য pwinerror (s)
    ধ্রুবক অক্ষর *s;
{
    যদি (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    অন্যথা
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (আকার(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local স্থির
   /* Needed for systems with limitation on stack size. */
#অন্যথা
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
স্থির অক্ষর *string_copy(অক্ষর *dst, অক্ষর ধ্রুবক *src, z_আকার_t len) {
    যদি (len == 0)
        ফেরত ফাঁকা;
    যখন (--len) {
        *dst = *src++;
        যদি (*dst == 0)
            ফেরত dst;
        dst++;
    }
    *dst = 0;
    ফেরত dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#যদি defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

স্থির শূন্য *myalloc(শূন্য *q, unsigned n, unsigned m) {
    (শূন্য)q;
    ফেরত calloc(n, m);
}

স্থির শূন্য myfree(শূন্য *q, শূন্য *p) {
    (শূন্য)q;
    মুক্ত(p);
}

প্রকারলেখ কাঠামো gzFile_s {
    FILE *file;
    পূর্ণসংখ্যা লিখ;
    পূর্ণসংখ্যা err;
    অক্ষর *বার্তা;
    z_stream strm;
} *gzFile;

স্থির gzFile gz_open(ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা fd, ধ্রুবক অক্ষর *mode) {
    gzFile gz;
    পূর্ণসংখ্যা ret;

    gz = malloc(আকার(কাঠামো gzFile_s));
    যদি (gz == ফাঁকা)
        ফেরত ফাঁকা;
    gz->লিখ = strchr(mode, 'w') != ফাঁকা;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    যদি (gz->লিখ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    অন্যথা {
        gz->strm.পরবর্তী_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    যদি (ret != Z_OK) {
        মুক্ত(gz);
        ফেরত ফাঁকা;
    }
    gz->file = path == ফাঁকা ? fdopen(fd, gz->লিখ ? "wb" : "rb") :
                              fopen(path, gz->লিখ ? "wb" : "rb");
    যদি (gz->file == ফাঁকা) {
        gz->লিখ ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        মুক্ত(gz);
        ফেরত ফাঁকা;
    }
    gz->err = 0;
    gz->বার্তা = "";
    ফেরত gz;
}

স্থির gzFile gzopen(ধ্রুবক অক্ষর *path, ধ্রুবক অক্ষর *mode) {
    ফেরত gz_open(path, -1, mode);
}

স্থির gzFile gzdopen(পূর্ণসংখ্যা fd, ধ্রুবক অক্ষর *mode) {
    ফেরত gz_open(ফাঁকা, fd, mode);
}

স্থির পূর্ণসংখ্যা gzwrite(gzFile gz, ধ্রুবক শূন্য *buf, unsigned len) {
    z_stream *strm;
    unsigned অক্ষর out[BUFLEN];

    যদি (gz == ফাঁকা || !gz->লিখ)
        ফেরত 0;
    strm = &(gz->strm);
    strm->পরবর্তী_in = (শূন্য *)buf;
    strm->avail_in = len;
    করো {
        strm->পরবর্তী_out = out;
        strm->avail_out = BUFLEN;
        (শূন্য)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } যখন (strm->avail_out == 0);
    ফেরত (পূর্ণসংখ্যা)len;
}

স্থির পূর্ণসংখ্যা gzread(gzFile gz, শূন্য *buf, unsigned len) {
    পূর্ণসংখ্যা ret;
    unsigned got;
    unsigned অক্ষর in[1];
    z_stream *strm;

    যদি (gz == ফাঁকা || gz->লিখ)
        ফেরত 0;
    যদি (gz->err)
        ফেরত 0;
    strm = &(gz->strm);
    strm->পরবর্তী_out = (শূন্য *)buf;
    strm->avail_out = len;
    করো {
        got = (unsigned)fread(in, 1, 1, gz->file);
        যদি (got == 0)
            বিরতি;
        strm->পরবর্তী_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        যদি (ret == Z_তথ্য_ERROR) {
            gz->err = Z_তথ্য_ERROR;
            gz->বার্তা = strm->বার্তা;
            ফেরত 0;
        }
        যদি (ret == Z_STREAM_END)
            inflateReset(strm);
    } যখন (strm->avail_out);
    ফেরত (পূর্ণসংখ্যা)(len - strm->avail_out);
}

স্থির পূর্ণসংখ্যা gzclose(gzFile gz) {
    z_stream *strm;
    unsigned অক্ষর out[BUFLEN];

    যদি (gz == ফাঁকা)
        ফেরত Z_STREAM_ERROR;
    strm = &(gz->strm);
    যদি (gz->লিখ) {
        strm->পরবর্তী_in = Z_NULL;
        strm->avail_in = 0;
        করো {
            strm->পরবর্তী_out = out;
            strm->avail_out = BUFLEN;
            (শূন্য)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } যখন (strm->avail_out == 0);
        deflateEnd(strm);
    }
    অন্যথা
        inflateEnd(strm);
    fclose(gz->file);
    মুক্ত(gz);
    ফেরত Z_OK;
}

স্থির ধ্রুবক অক্ষর *gzerror(gzFile gz, পূর্ণসংখ্যা *err) {
    *err = gz->err;
    ফেরত gz->বার্তা;
}

#endif

স্থির অক্ষর *prog;

/* ===========================================================================
 * Display error message and exit
 */
স্থির শূন্য error(ধ্রুবক অক্ষর *বার্তা) {
    fprintf(stderr, "%s: %s\n", prog, বার্তা);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
স্থির পূর্ণসংখ্যা gz_compress_mmap(FILE *in, gzFile out) {
    পূর্ণসংখ্যা len;
    পূর্ণসংখ্যা err;
    পূর্ণসংখ্যা ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    কাঠামো stat sb;

    /* Determine the size of the file, needed for mmap: */
    যদি (fstat(ifd, &sb) < 0) ফেরত Z_ERRNO;
    buf_len = sb.st_size;
    যদি (buf_len <= 0) ফেরত Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    যদি (buf == (caddr_t)(-1)) ফেরত Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (অক্ষর *)buf, (unsigned)buf_len);

    যদি (len != (পূর্ণসংখ্যা)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    যদি (gzclose(out) != Z_OK) error("failed gzclose");
    ফেরত Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

স্থির শূন্য gz_compress(FILE *in, gzFile out) {
    local অক্ষর buf[BUFLEN];
    পূর্ণসংখ্যা len;
    পূর্ণসংখ্যা err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    যদি (gz_compress_mmap(in, out) == Z_OK) ফেরত;
#endif
    জন্য (;;) {
        len = (পূর্ণসংখ্যা)fread(buf, 1, আকার(buf), in);
        যদি (ferror(in)) {
            perror("fread");
            exit(1);
        }
        যদি (len == 0) বিরতি;

        যদি (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    যদি (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
স্থির শূন্য gz_uncompress(gzFile in, FILE *out) {
    local অক্ষর buf[BUFLEN];
    পূর্ণসংখ্যা len;
    পূর্ণসংখ্যা err;

    জন্য (;;) {
        len = gzread(in, buf, আকার(buf));
        যদি (len < 0) error (gzerror(in, &err));
        যদি (len == 0) বিরতি;

        যদি ((পূর্ণসংখ্যা)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    যদি (fclose(out)) error("failed fclose");

    যদি (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
স্থির শূন্য file_compress(অক্ষর *file, অক্ষর *mode) {
    local অক্ষর outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    যদি (strlen(file) + strlen(GZ_SUFFIX) >= আকার(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, আকার(outfile));
    string_copy(end, GZ_SUFFIX, আকার(outfile) - (z_আকার_t)(end - outfile));

    in = fopen(file, "rb");
    যদি (in == ফাঁকা) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    যদি (out == ফাঁকা) {
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
স্থির শূন্য file_uncompress(অক্ষর *file) {
    local অক্ষর buf[MAX_NAME_LEN+1];
    অক্ষর *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_আকার_t len = strlen(file);

    যদি (len + strlen(GZ_SUFFIX) >= আকার(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, আকার(buf));

    যদি (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } অন্যথা {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, আকার(buf) - len);
    }
    in = gzopen(infile, "rb");
    যদি (in == ফাঁকা) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    যদি (out == ফাঁকা) {
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

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[]) {
    পূর্ণসংখ্যা copyout = 0;
    পূর্ণসংখ্যা uncompr = 0;
    gzFile file;
    অক্ষর *bname, outmode[5];

    string_copy(outmode, "wb6 ", আকার(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    যদি (bname)
      bname++;
    অন্যথা
      bname = argv[0];
    argc--, argv++;

    যদি (!strcmp(bname, "gunzip"))
      uncompr = 1;
    অন্যথা যদি (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    যখন (argc > 0) {
      যদি (strcmp(*argv, "-c") == 0)
        copyout = 1;
      অন্যথা যদি (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      অন্যথা যদি (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      অন্যথা যদি (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      অন্যথা যদি (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      অন্যথা যদি ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      অন্যথা
        বিরতি;
      argc--, argv++;
    }
    যদি (outmode[3] == ' ')
        outmode[3] = 0;
    যদি (argc == 0) {
        SET_দ্বিমিক_MODE(stdin);
        SET_দ্বিমিক_MODE(stdout);
        যদি (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            যদি (file == ফাঁকা) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } অন্যথা {
            file = gzdopen(fileno(stdout), outmode);
            যদি (file == ফাঁকা) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } অন্যথা {
        যদি (copyout) {
            SET_দ্বিমিক_MODE(stdout);
        }
        করো {
            যদি (uncompr) {
                যদি (copyout) {
                    file = gzopen(*argv, "rb");
                    যদি (file == ফাঁকা)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    অন্যথা
                        gz_uncompress(file, stdout);
                } অন্যথা {
                    file_uncompress(*argv);
                }
            } অন্যথা {
                যদি (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    যদি (in == ফাঁকা) {
                        perror(*argv);
                    } অন্যথা {
                        file = gzdopen(fileno(stdout), outmode);
                        যদি (file == ফাঁকা) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } অন্যথা {
                    file_compress(*argv, outmode);
                }
            }
        } যখন (argv++, --argc);
    }
    ফেরত 0;
}
