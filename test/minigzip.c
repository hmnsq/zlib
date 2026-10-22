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

#که_نه_تعریف_شوی _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#د_که_پای

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#د_که_پای
#إذا defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#د_که_پای

#ضمن "zlib.h"
#ضمن <stdio.h>

#که_تعریف_شوی STDC
#  include <string.h>
#  include <stdlib.h>
#د_که_پای

#که_تعریف_شوی USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#د_که_پای

#إذا defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define سېټ_دوګونی_ډېرپېښ(file) setmode(fileno(file), O_دوګونی)
#وإلا
#  define سېټ_دوګونی_ډېرپېښ(file)
#د_که_پای

#که_تعریف_شوی VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#د_که_پای
#إذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#د_که_پای
#إذا defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#د_که_پای

#إذا !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_سرچینه)
#که_نه_تعریف_شوی WIN32 /* unlink already in stdio.h for WIN32 */
  خارجي صحيح unlink(ثابت حرف *);
#د_که_پای
#د_که_پای

#إذا defined(UNDER_CE)
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
    DWORD lasterr = Get_وروستی_Error();
    DWORD chars = Format_پیغام(FORMAT_پیغام_FROM_SYSTEM
        | FORMAT_پیغام_ALLOCATE_بفر,
        عدم,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        عدم);
    إذا (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        إذا (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        إذا (chars > حجم (buf) - 1) {
            chars = حجم (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    وإلا {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    سېټ_وروستی_Error(lasterr);
    أرجع buf;
}

ساكن فراغ pwinerror (s)
    ثابت حرف *s;
{
    إذا (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_وروستی_Error ()));
    وإلا
        fprintf(stderr, "%s\n", strwinerror(Get_وروستی_Error ()));
}

#د_که_پای /* UNDER_CE */

#که_نه_تعریف_شوی GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#د_که_پای
#تعریف_کړه SUFFIX_LEN (حجم(GZ_SUFFIX)-1)

#تعریف_کړه BUFLEN      16384
#تعریف_کړه MAX_NAME_LEN 1024

#که_تعریف_شوی MAXSEG_64K
#  define local ساكن
   /* Needed for systems with limitation on stack size. */
#وإلا
#  define local
#د_که_پای

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ساكن حرف *string_copy(حرف *dst, حرف ثابت *src, z_اندازه_t len) {
    إذا (len == 0)
        أرجع عدم;
    طالما (--len) {
        *dst = *src++;
        إذا (*dst == 0)
            أرجع dst;
        dst++;
    }
    *dst = 0;
    أرجع dst;
}

#که_تعریف_شوی Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#إذا defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#د_که_پای

ساكن فراغ *myalloc(فراغ *q, بینښه n, بینښه m) {
    (فراغ)q;
    أرجع حافظه_صفر_ورکړه(n, m);
}

ساكن فراغ myfree(فراغ *q, فراغ *p) {
    (فراغ)q;
    حرر(p);
}

تعريفنوع بنية gzFile_s {
    FILE *file;
    صحيح اكتب;
    صحيح err;
    حرف *پیغام;
    z_بهیر strm;
} *gzFile;

ساكن gzFile gz_open(ثابت حرف *لار, صحيح fd, ثابت حرف *ډېرپېښ) {
    gzFile gz;
    صحيح ret;

    gz = حافظه_ورکړه(حجم(بنية gzFile_s));
    إذا (gz == عدم)
        أرجع عدم;
    gz->اكتب = strchr(ډېرپېښ, 'w') != عدم;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    إذا (gz->اكتب)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    وإلا {
        gz->strm.بل_دننه = 0;
        gz->strm.avail_دننه = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    إذا (ret != Z_OK) {
        حرر(gz);
        أرجع عدم;
    }
    gz->file = لار == عدم ? fdopen(fd, gz->اكتب ? "wb" : "rb") :
                              fopen(لار, gz->اكتب ? "wb" : "rb");
    إذا (gz->file == عدم) {
        gz->اكتب ? deflate_پای_ټکی(&(gz->strm)) : inflate_پای_ټکی(&(gz->strm));
        حرر(gz);
        أرجع عدم;
    }
    gz->err = 0;
    gz->پیغام = "";
    أرجع gz;
}

ساكن gzFile gzopen(ثابت حرف *لار, ثابت حرف *ډېرپېښ) {
    أرجع gz_open(لار, -1, ډېرپېښ);
}

ساكن gzFile gzdopen(صحيح fd, ثابت حرف *ډېرپېښ) {
    أرجع gz_open(عدم, fd, ډېرپېښ);
}

ساكن صحيح gzwrite(gzFile gz, ثابت فراغ *buf, بینښه len) {
    z_بهیر *strm;
    بینښه حرف بهر[BUFLEN];

    إذا (gz == عدم || !gz->اكتب)
        أرجع 0;
    strm = &(gz->strm);
    strm->بل_دننه = (فراغ *)buf;
    strm->avail_دننه = len;
    افعل {
        strm->بل_بهر = بهر;
        strm->avail_بهر = BUFLEN;
        (فراغ)deflate(strm, Z_NO_FLUSH);
        fwrite(بهر, 1, BUFLEN - strm->avail_بهر, gz->file);
    } طالما (strm->avail_بهر == 0);
    أرجع (صحيح)len;
}

ساكن صحيح gzread(gzFile gz, فراغ *buf, بینښه len) {
    صحيح ret;
    بینښه got;
    بینښه حرف دننه[1];
    z_بهیر *strm;

    إذا (gz == عدم || gz->اكتب)
        أرجع 0;
    إذا (gz->err)
        أرجع 0;
    strm = &(gz->strm);
    strm->بل_بهر = (فراغ *)buf;
    strm->avail_بهر = len;
    افعل {
        got = (بینښه)fread(دننه, 1, 1, gz->file);
        إذا (got == 0)
            اكسر;
        strm->بل_دننه = دننه;
        strm->avail_دننه = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        إذا (ret == Z_معلومات_ERROR) {
            gz->err = Z_معلومات_ERROR;
            gz->پیغام = strm->پیغام;
            أرجع 0;
        }
        إذا (ret == Z_بهیر_پای_ټکی)
            inflateReset(strm);
    } طالما (strm->avail_بهر);
    أرجع (صحيح)(len - strm->avail_بهر);
}

ساكن صحيح gzclose(gzFile gz) {
    z_بهیر *strm;
    بینښه حرف بهر[BUFLEN];

    إذا (gz == عدم)
        أرجع Z_بهیر_ERROR;
    strm = &(gz->strm);
    إذا (gz->اكتب) {
        strm->بل_دننه = Z_NULL;
        strm->avail_دننه = 0;
        افعل {
            strm->بل_بهر = بهر;
            strm->avail_بهر = BUFLEN;
            (فراغ)deflate(strm, Z_پای);
            fwrite(بهر, 1, BUFLEN - strm->avail_بهر, gz->file);
        } طالما (strm->avail_بهر == 0);
        deflate_پای_ټکی(strm);
    }
    وإلا
        inflate_پای_ټکی(strm);
    fclose(gz->file);
    حرر(gz);
    أرجع Z_OK;
}

ساكن ثابت حرف *gzerror(gzFile gz, صحيح *err) {
    *err = gz->err;
    أرجع gz->پیغام;
}

#د_که_پای

ساكن حرف *prog;

/* ===========================================================================
 * Display error message and exit
 */
ساكن فراغ error(ثابت حرف *پیغام) {
    fprintf(stderr, "%s: %s\n", prog, پیغام);
    وتل(1);
}

#که_تعریف_شوی USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ساكن صحيح gz_compress_mmap(FILE *دننه, gzFile بهر) {
    صحيح len;
    صحيح err;
    صحيح ifd = fileno(دننه);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    بنية stat sb;

    /* Determine the size of the file, needed for mmap: */
    إذا (fstat(ifd, &sb) < 0) أرجع Z_ERRNO;
    buf_len = sb.st_size;
    إذا (buf_len <= 0) أرجع Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    إذا (buf == (caddr_t)(-1)) أرجع Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(بهر, (حرف *)buf, (بینښه)buf_len);

    إذا (len != (صحيح)buf_len) error(gzerror(بهر, &err));

    munmap(buf, buf_len);
    fclose(دننه);
    إذا (gzclose(بهر) != Z_OK) error("failed gzclose");
    أرجع Z_OK;
}
#د_که_پای /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ساكن فراغ gz_compress(FILE *دننه, gzFile بهر) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

#که_تعریف_شوی USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    إذا (gz_compress_mmap(دننه, بهر) == Z_OK) أرجع;
#د_که_پای
    لكل (;;) {
        len = (صحيح)fread(buf, 1, حجم(buf), دننه);
        إذا (ferror(دننه)) {
            perror("fread");
            وتل(1);
        }
        إذا (len == 0) اكسر;

        إذا (gzwrite(بهر, buf, (بینښه)len) != len) error(gzerror(بهر, &err));
    }
    fclose(دننه);
    إذا (gzclose(بهر) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ساكن فراغ gz_uncompress(gzFile دننه, FILE *بهر) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

    لكل (;;) {
        len = gzread(دننه, buf, حجم(buf));
        إذا (len < 0) error (gzerror(دننه, &err));
        إذا (len == 0) اكسر;

        إذا ((صحيح)fwrite(buf, 1, (بینښه)len, بهر) != len) {
            error("failed fwrite");
        }
    }
    إذا (fclose(بهر)) error("failed fclose");

    إذا (gzclose(دننه) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ساكن فراغ file_compress(حرف *file, حرف *ډېرپېښ) {
    local حرف outfile[MAX_NAME_LEN+1], *پای_ټکی;
    FILE  *دننه;
    gzFile بهر;

    إذا (مزي_اوږدوالی(file) + مزي_اوږدوالی(GZ_SUFFIX) >= حجم(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        وتل(1);
    }

    پای_ټکی = string_copy(outfile, file, حجم(outfile));
    string_copy(پای_ټکی, GZ_SUFFIX, حجم(outfile) - (z_اندازه_t)(پای_ټکی - outfile));

    دننه = fopen(file, "rb");
    إذا (دننه == عدم) {
        perror(file);
        وتل(1);
    }
    بهر = gzopen(outfile, ډېرپېښ);
    إذا (بهر == عدم) {
        fclose(دننه);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        وتل(1);
    }
    gz_compress(دننه, بهر);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ساكن فراغ file_uncompress(حرف *file) {
    local حرف buf[MAX_NAME_LEN+1];
    حرف *infile, *outfile;
    FILE  *بهر;
    gzFile دننه;
    z_اندازه_t len = مزي_اوږدوالی(file);

    إذا (len + مزي_اوږدوالی(GZ_SUFFIX) >= حجم(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        وتل(1);
    }

    string_copy(buf, file, حجم(buf));

    إذا (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } وإلا {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, حجم(buf) - len);
    }
    دننه = gzopen(infile, "rb");
    إذا (دننه == عدم) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        وتل(1);
    }
    بهر = fopen(outfile, "wb");
    إذا (بهر == عدم) {
        gzclose(دننه);
        perror(file);
        وتل(1);
    }

    gz_uncompress(دننه, بهر);

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

صحيح اصلي(صحيح argc, حرف *argv[]) {
    صحيح copyout = 0;
    صحيح uncompr = 0;
    gzFile file;
    حرف *bname, outmode[5];

    string_copy(outmode, "wb6 ", حجم(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    إذا (bname)
      bname++;
    وإلا
      bname = argv[0];
    argc--, argv++;

    إذا (!strcmp(bname, "gunzip"))
      uncompr = 1;
    وإلا إذا (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    طالما (argc > 0) {
      إذا (strcmp(*argv, "-c") == 0)
        copyout = 1;
      وإلا إذا (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      وإلا إذا (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      وإلا إذا (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      وإلا إذا (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      وإلا إذا ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      وإلا
        اكسر;
      argc--, argv++;
    }
    إذا (outmode[3] == ' ')
        outmode[3] = 0;
    إذا (argc == 0) {
        سېټ_دوګونی_ډېرپېښ(stdin);
        سېټ_دوګونی_ډېرپېښ(stdout);
        إذا (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            إذا (file == عدم) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } وإلا {
            file = gzdopen(fileno(stdout), outmode);
            إذا (file == عدم) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } وإلا {
        إذا (copyout) {
            سېټ_دوګونی_ډېرپېښ(stdout);
        }
        افعل {
            إذا (uncompr) {
                إذا (copyout) {
                    file = gzopen(*argv, "rb");
                    إذا (file == عدم)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    وإلا
                        gz_uncompress(file, stdout);
                } وإلا {
                    file_uncompress(*argv);
                }
            } وإلا {
                إذا (copyout) {
                    FILE * دننه = fopen(*argv, "rb");

                    إذا (دننه == عدم) {
                        perror(*argv);
                    } وإلا {
                        file = gzdopen(fileno(stdout), outmode);
                        إذا (file == عدم) error("can't gzdopen stdout");

                        gz_compress(دننه, file);
                    }

                } وإلا {
                    file_compress(*argv, outmode);
                }
            }
        } طالما (argv++, --argc);
    }
    أرجع 0;
}
