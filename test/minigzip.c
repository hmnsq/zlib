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

#اگر defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#اگر defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#شامل "zlib.h"
#شامل <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#اگر defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_دودویی_MODE(file) setmode(fileno(file), O_دودویی)
#وگرنه
#  define SET_دودویی_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#اگر defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#اگر defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#اگر !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  بیرونی صحیح unlink(ثابت نویسه *);
#endif
#endif

#اگر defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

ایستا نویسه *strwinerror (error)
     DWORD error;
{
    ایستا نویسه buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        پوچ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        پوچ);
    اگر (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        اگر (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        اگر (chars > اندازه (buf) - 1) {
            chars = اندازه (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    وگرنه {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    بازگشت buf;
}

ایستا تهی pwinerror (s)
    ثابت نویسه *s;
{
    اگر (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    وگرنه
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (اندازه(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local ایستا
   /* Needed for systems with limitation on stack size. */
#وگرنه
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ایستا نویسه *string_copy(نویسه *dst, نویسه ثابت *src, z_size_t len) {
    اگر (len == 0)
        بازگشت پوچ;
    وقتی (--len) {
        *dst = *src++;
        اگر (*dst == 0)
            بازگشت dst;
        dst++;
    }
    *dst = 0;
    بازگشت dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#اگر defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

ایستا تهی *myalloc(تهی *q, unsigned n, unsigned m) {
    (تهی)q;
    بازگشت calloc(n, m);
}

ایستا تهی myfree(تهی *q, تهی *p) {
    (تهی)q;
    آزاد(p);
}

تعریفنوع ساختار gzFile_s {
    FILE *file;
    صحیح اكتب;
    صحیح err;
    نویسه *پیام;
    z_stream strm;
} *gzFile;

ایستا gzFile gz_open(ثابت نویسه *path, صحیح fd, ثابت نویسه *mode) {
    gzFile gz;
    صحیح ret;

    gz = malloc(اندازه(ساختار gzFile_s));
    اگر (gz == پوچ)
        بازگشت پوچ;
    gz->اكتب = strchr(mode, 'w') != پوچ;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    اگر (gz->اكتب)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    وگرنه {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    اگر (ret != Z_OK) {
        آزاد(gz);
        بازگشت پوچ;
    }
    gz->file = path == پوچ ? fdopen(fd, gz->اكتب ? "wb" : "rb") :
                              fopen(path, gz->اكتب ? "wb" : "rb");
    اگر (gz->file == پوچ) {
        gz->اكتب ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        آزاد(gz);
        بازگشت پوچ;
    }
    gz->err = 0;
    gz->پیام = "";
    بازگشت gz;
}

ایستا gzFile gzopen(ثابت نویسه *path, ثابت نویسه *mode) {
    بازگشت gz_open(path, -1, mode);
}

ایستا gzFile gzdopen(صحیح fd, ثابت نویسه *mode) {
    بازگشت gz_open(پوچ, fd, mode);
}

ایستا صحیح gzwrite(gzFile gz, ثابت تهی *buf, unsigned len) {
    z_stream *strm;
    unsigned نویسه out[BUFLEN];

    اگر (gz == پوچ || !gz->اكتب)
        بازگشت 0;
    strm = &(gz->strm);
    strm->next_in = (تهی *)buf;
    strm->avail_in = len;
    انجامبده {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (تهی)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } وقتی (strm->avail_out == 0);
    بازگشت (صحیح)len;
}

ایستا صحیح gzread(gzFile gz, تهی *buf, unsigned len) {
    صحیح ret;
    unsigned got;
    unsigned نویسه in[1];
    z_stream *strm;

    اگر (gz == پوچ || gz->اكتب)
        بازگشت 0;
    اگر (gz->err)
        بازگشت 0;
    strm = &(gz->strm);
    strm->next_out = (تهی *)buf;
    strm->avail_out = len;
    انجامبده {
        got = (unsigned)fread(in, 1, 1, gz->file);
        اگر (got == 0)
            بشکن;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        اگر (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->پیام = strm->پیام;
            بازگشت 0;
        }
        اگر (ret == Z_STREAM_END)
            inflateReset(strm);
    } وقتی (strm->avail_out);
    بازگشت (صحیح)(len - strm->avail_out);
}

ایستا صحیح gzclose(gzFile gz) {
    z_stream *strm;
    unsigned نویسه out[BUFLEN];

    اگر (gz == پوچ)
        بازگشت Z_STREAM_ERROR;
    strm = &(gz->strm);
    اگر (gz->اكتب) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        انجامبده {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (تهی)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } وقتی (strm->avail_out == 0);
        deflateEnd(strm);
    }
    وگرنه
        inflateEnd(strm);
    fclose(gz->file);
    آزاد(gz);
    بازگشت Z_OK;
}

ایستا ثابت نویسه *gzerror(gzFile gz, صحیح *err) {
    *err = gz->err;
    بازگشت gz->پیام;
}

#endif

ایستا نویسه *prog;

/* ===========================================================================
 * Display error message and exit
 */
ایستا تهی error(ثابت نویسه *پیام) {
    fprintf(stderr, "%s: %s\n", prog, پیام);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ایستا صحیح gz_compress_mmap(FILE *in, gzFile out) {
    صحیح len;
    صحیح err;
    صحیح ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    ساختار stat sb;

    /* Determine the size of the file, needed for mmap: */
    اگر (fstat(ifd, &sb) < 0) بازگشت Z_ERRNO;
    buf_len = sb.st_size;
    اگر (buf_len <= 0) بازگشت Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    اگر (buf == (caddr_t)(-1)) بازگشت Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (نویسه *)buf, (unsigned)buf_len);

    اگر (len != (صحیح)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    اگر (gzclose(out) != Z_OK) error("failed gzclose");
    بازگشت Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ایستا تهی gz_compress(FILE *in, gzFile out) {
    local نویسه buf[BUFLEN];
    صحیح len;
    صحیح err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    اگر (gz_compress_mmap(in, out) == Z_OK) بازگشت;
#endif
    برای (;;) {
        len = (صحیح)fread(buf, 1, اندازه(buf), in);
        اگر (ferror(in)) {
            perror("fread");
            exit(1);
        }
        اگر (len == 0) بشکن;

        اگر (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    اگر (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ایستا تهی gz_uncompress(gzFile in, FILE *out) {
    local نویسه buf[BUFLEN];
    صحیح len;
    صحیح err;

    برای (;;) {
        len = gzread(in, buf, اندازه(buf));
        اگر (len < 0) error (gzerror(in, &err));
        اگر (len == 0) بشکن;

        اگر ((صحیح)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    اگر (fclose(out)) error("failed fclose");

    اگر (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ایستا تهی file_compress(نویسه *file, نویسه *mode) {
    local نویسه outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    اگر (strlen(file) + strlen(GZ_SUFFIX) >= اندازه(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, اندازه(outfile));
    string_copy(end, GZ_SUFFIX, اندازه(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    اگر (in == پوچ) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    اگر (out == پوچ) {
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
ایستا تهی file_uncompress(نویسه *file) {
    local نویسه buf[MAX_NAME_LEN+1];
    نویسه *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    اگر (len + strlen(GZ_SUFFIX) >= اندازه(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, اندازه(buf));

    اگر (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } وگرنه {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, اندازه(buf) - len);
    }
    in = gzopen(infile, "rb");
    اگر (in == پوچ) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    اگر (out == پوچ) {
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

صحیح اصلی(صحیح argc, نویسه *argv[]) {
    صحیح copyout = 0;
    صحیح uncompr = 0;
    gzFile file;
    نویسه *bname, outmode[5];

    string_copy(outmode, "wb6 ", اندازه(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    اگر (bname)
      bname++;
    وگرنه
      bname = argv[0];
    argc--, argv++;

    اگر (!strcmp(bname, "gunzip"))
      uncompr = 1;
    وگرنه اگر (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    وقتی (argc > 0) {
      اگر (strcmp(*argv, "-c") == 0)
        copyout = 1;
      وگرنه اگر (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      وگرنه اگر (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      وگرنه اگر (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      وگرنه اگر (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      وگرنه اگر ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      وگرنه
        بشکن;
      argc--, argv++;
    }
    اگر (outmode[3] == ' ')
        outmode[3] = 0;
    اگر (argc == 0) {
        SET_دودویی_MODE(stdin);
        SET_دودویی_MODE(stdout);
        اگر (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            اگر (file == پوچ) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } وگرنه {
            file = gzdopen(fileno(stdout), outmode);
            اگر (file == پوچ) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } وگرنه {
        اگر (copyout) {
            SET_دودویی_MODE(stdout);
        }
        انجامبده {
            اگر (uncompr) {
                اگر (copyout) {
                    file = gzopen(*argv, "rb");
                    اگر (file == پوچ)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    وگرنه
                        gz_uncompress(file, stdout);
                } وگرنه {
                    file_uncompress(*argv);
                }
            } وگرنه {
                اگر (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    اگر (in == پوچ) {
                        perror(*argv);
                    } وگرنه {
                        file = gzdopen(fileno(stdout), outmode);
                        اگر (file == پوچ) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } وگرنه {
                    file_compress(*argv, outmode);
                }
            }
        } وقتی (argv++, --argc);
    }
    بازگشت 0;
}
