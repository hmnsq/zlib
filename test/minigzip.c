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

#অন্তর্ভাৱ "zlib.h"
#অন্তর্ভাৱ <stdio.h>

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
  বাহ্য পূর্ণাঙ্ক unlink(ধ্রুৱ ৱর্ণ *);
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

স্থির ৱর্ণ *strwinerror (error)
     DWORD error;
{
    স্থির ৱর্ণ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        রিক্ত,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        রিক্ত);
    যদি (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        যদি (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        যদি (chars > পরিমাণ (buf) - 1) {
            chars = পরিমাণ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    অন্যথা {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    ঘূৰাই buf;
}

স্থির শূন্য pwinerror (s)
    ধ্রুৱ ৱর্ণ *s;
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
#define SUFFIX_LEN (পরিমাণ(GZ_SUFFIX)-1)

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
স্থির ৱর্ণ *string_copy(ৱর্ণ *dst, ৱর্ণ ধ্রুৱ *src, z_size_t len) {
    যদি (len == 0)
        ঘূৰাই রিক্ত;
    যেতিয়ালৈ (--len) {
        *dst = *src++;
        যদি (*dst == 0)
            ঘূৰাই dst;
        dst++;
    }
    *dst = 0;
    ঘূৰাই dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#যদি defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

স্থির শূন্য *myalloc(শূন্য *q, unsigned n, unsigned m) {
    (শূন্য)q;
    ঘূৰাই calloc(n, m);
}

স্থির শূন্য myfree(শূন্য *q, শূন্য *p) {
    (শূন্য)q;
    মুক্ত(p);
}

প্রকারনির্দেশ সংরচনা gzFile_s {
    FILE *file;
    পূর্ণাঙ্ক লিখ;
    পূর্ণাঙ্ক err;
    ৱর্ণ *বাৰ্তা;
    z_stream strm;
} *gzFile;

স্থির gzFile gz_open(ধ্রুৱ ৱর্ণ *path, পূর্ণাঙ্ক fd, ধ্রুৱ ৱর্ণ *mode) {
    gzFile gz;
    পূর্ণাঙ্ক ret;

    gz = malloc(পরিমাণ(সংরচনা gzFile_s));
    যদি (gz == রিক্ত)
        ঘূৰাই রিক্ত;
    gz->লিখ = strchr(mode, 'w') != রিক্ত;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    যদি (gz->লিখ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    অন্যথা {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    যদি (ret != Z_OK) {
        মুক্ত(gz);
        ঘূৰাই রিক্ত;
    }
    gz->file = path == রিক্ত ? fdopen(fd, gz->লিখ ? "wb" : "rb") :
                              fopen(path, gz->লিখ ? "wb" : "rb");
    যদি (gz->file == রিক্ত) {
        gz->লিখ ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        মুক্ত(gz);
        ঘূৰাই রিক্ত;
    }
    gz->err = 0;
    gz->বাৰ্তা = "";
    ঘূৰাই gz;
}

স্থির gzFile gzopen(ধ্রুৱ ৱর্ণ *path, ধ্রুৱ ৱর্ণ *mode) {
    ঘূৰাই gz_open(path, -1, mode);
}

স্থির gzFile gzdopen(পূর্ণাঙ্ক fd, ধ্রুৱ ৱর্ণ *mode) {
    ঘূৰাই gz_open(রিক্ত, fd, mode);
}

স্থির পূর্ণাঙ্ক gzwrite(gzFile gz, ধ্রুৱ শূন্য *buf, unsigned len) {
    z_stream *strm;
    unsigned ৱর্ণ out[BUFLEN];

    যদি (gz == রিক্ত || !gz->লিখ)
        ঘূৰাই 0;
    strm = &(gz->strm);
    strm->next_in = (শূন্য *)buf;
    strm->avail_in = len;
    কৰা {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (শূন্য)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } যেতিয়ালৈ (strm->avail_out == 0);
    ঘূৰাই (পূর্ণাঙ্ক)len;
}

স্থির পূর্ণাঙ্ক gzread(gzFile gz, শূন্য *buf, unsigned len) {
    পূর্ণাঙ্ক ret;
    unsigned got;
    unsigned ৱর্ণ in[1];
    z_stream *strm;

    যদি (gz == রিক্ত || gz->লিখ)
        ঘূৰাই 0;
    যদি (gz->err)
        ঘূৰাই 0;
    strm = &(gz->strm);
    strm->next_out = (শূন্য *)buf;
    strm->avail_out = len;
    কৰা {
        got = (unsigned)fread(in, 1, 1, gz->file);
        যদি (got == 0)
            ভঙ্গ;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        যদি (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->বাৰ্তা = strm->বাৰ্তা;
            ঘূৰাই 0;
        }
        যদি (ret == Z_STREAM_END)
            inflateReset(strm);
    } যেতিয়ালৈ (strm->avail_out);
    ঘূৰাই (পূর্ণাঙ্ক)(len - strm->avail_out);
}

স্থির পূর্ণাঙ্ক gzclose(gzFile gz) {
    z_stream *strm;
    unsigned ৱর্ণ out[BUFLEN];

    যদি (gz == রিক্ত)
        ঘূৰাই Z_STREAM_ERROR;
    strm = &(gz->strm);
    যদি (gz->লিখ) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        কৰা {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (শূন্য)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } যেতিয়ালৈ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    অন্যথা
        inflateEnd(strm);
    fclose(gz->file);
    মুক্ত(gz);
    ঘূৰাই Z_OK;
}

স্থির ধ্রুৱ ৱর্ণ *gzerror(gzFile gz, পূর্ণাঙ্ক *err) {
    *err = gz->err;
    ঘূৰাই gz->বাৰ্তা;
}

#endif

স্থির ৱর্ণ *prog;

/* ===========================================================================
 * Display error message and exit
 */
স্থির শূন্য error(ধ্রুৱ ৱর্ণ *বাৰ্তা) {
    fprintf(stderr, "%s: %s\n", prog, বাৰ্তা);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
স্থির পূর্ণাঙ্ক gz_compress_mmap(FILE *in, gzFile out) {
    পূর্ণাঙ্ক len;
    পূর্ণাঙ্ক err;
    পূর্ণাঙ্ক ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    সংরচনা stat sb;

    /* Determine the size of the file, needed for mmap: */
    যদি (fstat(ifd, &sb) < 0) ঘূৰাই Z_ERRNO;
    buf_len = sb.st_size;
    যদি (buf_len <= 0) ঘূৰাই Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    যদি (buf == (caddr_t)(-1)) ঘূৰাই Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (ৱর্ণ *)buf, (unsigned)buf_len);

    যদি (len != (পূর্ণাঙ্ক)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    যদি (gzclose(out) != Z_OK) error("failed gzclose");
    ঘূৰাই Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

স্থির শূন্য gz_compress(FILE *in, gzFile out) {
    local ৱর্ণ buf[BUFLEN];
    পূর্ণাঙ্ক len;
    পূর্ণাঙ্ক err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    যদি (gz_compress_mmap(in, out) == Z_OK) ঘূৰাই;
#endif
    বাবে (;;) {
        len = (পূর্ণাঙ্ক)fread(buf, 1, পরিমাণ(buf), in);
        যদি (ferror(in)) {
            perror("fread");
            exit(1);
        }
        যদি (len == 0) ভঙ্গ;

        যদি (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    যদি (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
স্থির শূন্য gz_uncompress(gzFile in, FILE *out) {
    local ৱর্ণ buf[BUFLEN];
    পূর্ণাঙ্ক len;
    পূর্ণাঙ্ক err;

    বাবে (;;) {
        len = gzread(in, buf, পরিমাণ(buf));
        যদি (len < 0) error (gzerror(in, &err));
        যদি (len == 0) ভঙ্গ;

        যদি ((পূর্ণাঙ্ক)fwrite(buf, 1, (unsigned)len, out) != len) {
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
স্থির শূন্য file_compress(ৱর্ণ *file, ৱর্ণ *mode) {
    local ৱর্ণ outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    যদি (strlen(file) + strlen(GZ_SUFFIX) >= পরিমাণ(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, পরিমাণ(outfile));
    string_copy(end, GZ_SUFFIX, পরিমাণ(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    যদি (in == রিক্ত) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    যদি (out == রিক্ত) {
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
স্থির শূন্য file_uncompress(ৱর্ণ *file) {
    local ৱর্ণ buf[MAX_NAME_LEN+1];
    ৱর্ণ *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    যদি (len + strlen(GZ_SUFFIX) >= পরিমাণ(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, পরিমাণ(buf));

    যদি (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } অন্যথা {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, পরিমাণ(buf) - len);
    }
    in = gzopen(infile, "rb");
    যদি (in == রিক্ত) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    যদি (out == রিক্ত) {
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

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[]) {
    পূর্ণাঙ্ক copyout = 0;
    পূর্ণাঙ্ক uncompr = 0;
    gzFile file;
    ৱর্ণ *bname, outmode[5];

    string_copy(outmode, "wb6 ", পরিমাণ(outmode));
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

    যেতিয়ালৈ (argc > 0) {
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
        ভঙ্গ;
      argc--, argv++;
    }
    যদি (outmode[3] == ' ')
        outmode[3] = 0;
    যদি (argc == 0) {
        SET_দ্বিমিক_MODE(stdin);
        SET_দ্বিমিক_MODE(stdout);
        যদি (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            যদি (file == রিক্ত) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } অন্যথা {
            file = gzdopen(fileno(stdout), outmode);
            যদি (file == রিক্ত) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } অন্যথা {
        যদি (copyout) {
            SET_দ্বিমিক_MODE(stdout);
        }
        কৰা {
            যদি (uncompr) {
                যদি (copyout) {
                    file = gzopen(*argv, "rb");
                    যদি (file == রিক্ত)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    অন্যথা
                        gz_uncompress(file, stdout);
                } অন্যথা {
                    file_uncompress(*argv);
                }
            } অন্যথা {
                যদি (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    যদি (in == রিক্ত) {
                        perror(*argv);
                    } অন্যথা {
                        file = gzdopen(fileno(stdout), outmode);
                        যদি (file == রিক্ত) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } অন্যথা {
                    file_compress(*argv, outmode);
                }
            }
        } যেতিয়ালৈ (argv++, --argc);
    }
    ঘূৰাই 0;
}
