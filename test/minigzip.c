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

#اگر_تعریف_نشده _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#پایان_اگر

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#پایان_اگر
#إذا defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#پایان_اگر

#ضمن "zlib.h"
#ضمن <stdio.h>

#اگر_تعریف_شده STDC
#  include <string.h>
#  include <stdlib.h>
#پایان_اگر

#اگر_تعریف_شده USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#پایان_اگر

#إذا defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define مجموعه_دودویی_نما(file) setmode(fileno(file), O_دودویی)
#وإلا
#  define مجموعه_دودویی_نما(file)
#پایان_اگر

#اگر_تعریف_شده VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#پایان_اگر
#إذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#پایان_اگر
#إذا defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#پایان_اگر

#إذا !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_منبع)
#اگر_تعریف_نشده WIN32 /* unlink already in stdio.h for WIN32 */
  خارجي صحيح unlink(ثابت حرف *);
#پایان_اگر
#پایان_اگر

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
    DWORD lasterr = Get_آخرین_Error();
    DWORD chars = Format_پیام(FORMAT_پیام_FROM_SYSTEM
        | FORMAT_پیام_ALLOCATE_میانگیر,
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

    مجموعه_آخرین_Error(lasterr);
    أرجع buf;
}

ساكن فراغ pwinerror (s)
    ثابت حرف *s;
{
    إذا (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_آخرین_Error ()));
    وإلا
        fprintf(stderr, "%s\n", strwinerror(Get_آخرین_Error ()));
}

#پایان_اگر /* UNDER_CE */

#اگر_تعریف_نشده GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#پایان_اگر
#تعریف_کن SUFFIX_LEN (حجم(GZ_SUFFIX)-1)

#تعریف_کن BUFLEN      16384
#تعریف_کن MAX_NAME_LEN 1024

#اگر_تعریف_شده MAXSEG_64K
#  define local ساكن
   /* Needed for systems with limitation on stack size. */
#وإلا
#  define local
#پایان_اگر

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

#اگر_تعریف_شده Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#إذا defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#پایان_اگر

ساكن فراغ *myalloc(فراغ *q, بیعلامت n, بیعلامت m) {
    (فراغ)q;
    أرجع حافظه_صفر_بده(n, m);
}

ساكن فراغ myfree(فراغ *q, فراغ *p) {
    (فراغ)q;
    حرر(p);
}

تعريفنوع بنية gzFile_s {
    FILE *file;
    صحيح اكتب;
    صحيح err;
    حرف *پیام;
    z_جریان strm;
} *gzFile;

ساكن gzFile gz_open(ثابت حرف *مسیر, صحيح fd, ثابت حرف *نما) {
    gzFile gz;
    صحيح ret;

    gz = حافظه_بده(حجم(بنية gzFile_s));
    إذا (gz == عدم)
        أرجع عدم;
    gz->اكتب = strchr(نما, 'w') != عدم;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    إذا (gz->اكتب)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    وإلا {
        gz->strm.بعدی_درون = 0;
        gz->strm.avail_درون = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    إذا (ret != Z_OK) {
        حرر(gz);
        أرجع عدم;
    }
    gz->file = مسیر == عدم ? fdopen(fd, gz->اكتب ? "wb" : "rb") :
                              fopen(مسیر, gz->اكتب ? "wb" : "rb");
    إذا (gz->file == عدم) {
        gz->اكتب ? deflate_انتها(&(gz->strm)) : inflate_انتها(&(gz->strm));
        حرر(gz);
        أرجع عدم;
    }
    gz->err = 0;
    gz->پیام = "";
    أرجع gz;
}

ساكن gzFile gzopen(ثابت حرف *مسیر, ثابت حرف *نما) {
    أرجع gz_open(مسیر, -1, نما);
}

ساكن gzFile gzdopen(صحيح fd, ثابت حرف *نما) {
    أرجع gz_open(عدم, fd, نما);
}

ساكن صحيح gzwrite(gzFile gz, ثابت فراغ *buf, بیعلامت len) {
    z_جریان *strm;
    بیعلامت حرف بیرون[BUFLEN];

    إذا (gz == عدم || !gz->اكتب)
        أرجع 0;
    strm = &(gz->strm);
    strm->بعدی_درون = (فراغ *)buf;
    strm->avail_درون = len;
    افعل {
        strm->بعدی_بیرون = بیرون;
        strm->avail_بیرون = BUFLEN;
        (فراغ)deflate(strm, Z_NO_FLUSH);
        fwrite(بیرون, 1, BUFLEN - strm->avail_بیرون, gz->file);
    } طالما (strm->avail_بیرون == 0);
    أرجع (صحيح)len;
}

ساكن صحيح gzread(gzFile gz, فراغ *buf, بیعلامت len) {
    صحيح ret;
    بیعلامت got;
    بیعلامت حرف درون[1];
    z_جریان *strm;

    إذا (gz == عدم || gz->اكتب)
        أرجع 0;
    إذا (gz->err)
        أرجع 0;
    strm = &(gz->strm);
    strm->بعدی_بیرون = (فراغ *)buf;
    strm->avail_بیرون = len;
    افعل {
        got = (بیعلامت)fread(درون, 1, 1, gz->file);
        إذا (got == 0)
            اكسر;
        strm->بعدی_درون = درون;
        strm->avail_درون = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        إذا (ret == Z_داده_ERROR) {
            gz->err = Z_داده_ERROR;
            gz->پیام = strm->پیام;
            أرجع 0;
        }
        إذا (ret == Z_جریان_انتها)
            inflateReset(strm);
    } طالما (strm->avail_بیرون);
    أرجع (صحيح)(len - strm->avail_بیرون);
}

ساكن صحيح gzclose(gzFile gz) {
    z_جریان *strm;
    بیعلامت حرف بیرون[BUFLEN];

    إذا (gz == عدم)
        أرجع Z_جریان_ERROR;
    strm = &(gz->strm);
    إذا (gz->اكتب) {
        strm->بعدی_درون = Z_NULL;
        strm->avail_درون = 0;
        افعل {
            strm->بعدی_بیرون = بیرون;
            strm->avail_بیرون = BUFLEN;
            (فراغ)deflate(strm, Z_پایان);
            fwrite(بیرون, 1, BUFLEN - strm->avail_بیرون, gz->file);
        } طالما (strm->avail_بیرون == 0);
        deflate_انتها(strm);
    }
    وإلا
        inflate_انتها(strm);
    fclose(gz->file);
    حرر(gz);
    أرجع Z_OK;
}

ساكن ثابت حرف *gzerror(gzFile gz, صحيح *err) {
    *err = gz->err;
    أرجع gz->پیام;
}

#پایان_اگر

ساكن حرف *prog;

/* ===========================================================================
 * Display error message and exit
 */
ساكن فراغ error(ثابت حرف *پیام) {
    fprintf(stderr, "%s: %s\n", prog, پیام);
    خروج(1);
}

#اگر_تعریف_شده USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ساكن صحيح gz_compress_mmap(FILE *درون, gzFile بیرون) {
    صحيح len;
    صحيح err;
    صحيح ifd = fileno(درون);
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
    len = gzwrite(بیرون, (حرف *)buf, (بیعلامت)buf_len);

    إذا (len != (صحيح)buf_len) error(gzerror(بیرون, &err));

    munmap(buf, buf_len);
    fclose(درون);
    إذا (gzclose(بیرون) != Z_OK) error("failed gzclose");
    أرجع Z_OK;
}
#پایان_اگر /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ساكن فراغ gz_compress(FILE *درون, gzFile بیرون) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

#اگر_تعریف_شده USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    إذا (gz_compress_mmap(درون, بیرون) == Z_OK) أرجع;
#پایان_اگر
    لكل (;;) {
        len = (صحيح)fread(buf, 1, حجم(buf), درون);
        إذا (ferror(درون)) {
            perror("fread");
            خروج(1);
        }
        إذا (len == 0) اكسر;

        إذا (gzwrite(بیرون, buf, (بیعلامت)len) != len) error(gzerror(بیرون, &err));
    }
    fclose(درون);
    إذا (gzclose(بیرون) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ساكن فراغ gz_uncompress(gzFile درون, FILE *بیرون) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

    لكل (;;) {
        len = gzread(درون, buf, حجم(buf));
        إذا (len < 0) error (gzerror(درون, &err));
        إذا (len == 0) اكسر;

        إذا ((صحيح)fwrite(buf, 1, (بیعلامت)len, بیرون) != len) {
            error("failed fwrite");
        }
    }
    إذا (fclose(بیرون)) error("failed fclose");

    إذا (gzclose(درون) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ساكن فراغ file_compress(حرف *file, حرف *نما) {
    local حرف outfile[MAX_NAME_LEN+1], *انتها;
    FILE  *درون;
    gzFile بیرون;

    إذا (طول_رشته(file) + طول_رشته(GZ_SUFFIX) >= حجم(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        خروج(1);
    }

    انتها = string_copy(outfile, file, حجم(outfile));
    string_copy(انتها, GZ_SUFFIX, حجم(outfile) - (z_اندازه_t)(انتها - outfile));

    درون = fopen(file, "rb");
    إذا (درون == عدم) {
        perror(file);
        خروج(1);
    }
    بیرون = gzopen(outfile, نما);
    إذا (بیرون == عدم) {
        fclose(درون);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        خروج(1);
    }
    gz_compress(درون, بیرون);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ساكن فراغ file_uncompress(حرف *file) {
    local حرف buf[MAX_NAME_LEN+1];
    حرف *infile, *outfile;
    FILE  *بیرون;
    gzFile درون;
    z_اندازه_t len = طول_رشته(file);

    إذا (len + طول_رشته(GZ_SUFFIX) >= حجم(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        خروج(1);
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
    درون = gzopen(infile, "rb");
    إذا (درون == عدم) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        خروج(1);
    }
    بیرون = fopen(outfile, "wb");
    إذا (بیرون == عدم) {
        gzclose(درون);
        perror(file);
        خروج(1);
    }

    gz_uncompress(درون, بیرون);

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

صحيح اصلی(صحيح argc, حرف *argv[]) {
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
        مجموعه_دودویی_نما(stdin);
        مجموعه_دودویی_نما(stdout);
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
            مجموعه_دودویی_نما(stdout);
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
                    FILE * درون = fopen(*argv, "rb");

                    إذا (درون == عدم) {
                        perror(*argv);
                    } وإلا {
                        file = gzdopen(fileno(stdout), outmode);
                        إذا (file == عدم) error("can't gzdopen stdout");

                        gz_compress(درون, file);
                    }

                } وإلا {
                    file_compress(*argv, outmode);
                }
            }
        } طالما (argv++, --argc);
    }
    أرجع 0;
}
