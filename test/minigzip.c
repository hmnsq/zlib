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

#جے_بنا_تعریف _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#جے_انت

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#جے_انت
#إذا defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#جے_انت

#ضمن "zlib.h"
#ضمن <stdio.h>

#جے_تعریف_شدہ STDC
#  include <string.h>
#  include <stdlib.h>
#جے_انت

#جے_تعریف_شدہ USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#جے_انت

#إذا defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define سیٹ_ثنائی_منوال(file) setmode(fileno(file), O_ثنائی)
#وإلا
#  define سیٹ_ثنائی_منوال(file)
#جے_انت

#جے_تعریف_شدہ VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#جے_انت
#إذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#جے_انت
#إذا defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#جے_انت

#إذا !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_سروت)
#جے_بنا_تعریف WIN32 /* unlink already in stdio.h for WIN32 */
  خارجي صحيح unlink(ثابت حرف *);
#جے_انت
#جے_انت

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
    DWORD lasterr = Get_آخری_Error();
    DWORD chars = Format_سنیہا(FORMAT_سنیہا_FROM_SYSTEM
        | FORMAT_سنیہا_ALLOCATE_بفر,
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

    سیٹ_آخری_Error(lasterr);
    أرجع buf;
}

ساكن فراغ pwinerror (s)
    ثابت حرف *s;
{
    إذا (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_آخری_Error ()));
    وإلا
        fprintf(stderr, "%s\n", strwinerror(Get_آخری_Error ()));
}

#جے_انت /* UNDER_CE */

#جے_بنا_تعریف GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#جے_انت
#تعریف_کرو SUFFIX_LEN (حجم(GZ_SUFFIX)-1)

#تعریف_کرو BUFLEN      16384
#تعریف_کرو MAX_NAME_LEN 1024

#جے_تعریف_شدہ MAXSEG_64K
#  define local ساكن
   /* Needed for systems with limitation on stack size. */
#وإلا
#  define local
#جے_انت

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ساكن حرف *string_copy(حرف *dst, حرف ثابت *src, z_ناپ_t len) {
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

#جے_تعریف_شدہ Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#إذا defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#جے_انت

ساكن فراغ *myalloc(فراغ *q, بنانشانی n, بنانشانی m) {
    (فراغ)q;
    أرجع صفر_حافظہ_دیو(n, m);
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
    z_دھارا strm;
} *gzFile;

ساكن gzFile gz_open(ثابت حرف *رستہ, صحيح fd, ثابت حرف *منوال) {
    gzFile gz;
    صحيح ret;

    gz = حافظہ_دیو(حجم(بنية gzFile_s));
    إذا (gz == عدم)
        أرجع عدم;
    gz->اكتب = strchr(منوال, 'w') != عدم;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    إذا (gz->اكتب)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    وإلا {
        gz->strm.اگلا_وچ = 0;
        gz->strm.avail_وچ = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    إذا (ret != Z_OK) {
        حرر(gz);
        أرجع عدم;
    }
    gz->file = رستہ == عدم ? fdopen(fd, gz->اكتب ? "wb" : "rb") :
                              fopen(رستہ, gz->اكتب ? "wb" : "rb");
    إذا (gz->file == عدم) {
        gz->اكتب ? deflate_سرا(&(gz->strm)) : inflate_سرا(&(gz->strm));
        حرر(gz);
        أرجع عدم;
    }
    gz->err = 0;
    gz->پیغام = "";
    أرجع gz;
}

ساكن gzFile gzopen(ثابت حرف *رستہ, ثابت حرف *منوال) {
    أرجع gz_open(رستہ, -1, منوال);
}

ساكن gzFile gzdopen(صحيح fd, ثابت حرف *منوال) {
    أرجع gz_open(عدم, fd, منوال);
}

ساكن صحيح gzwrite(gzFile gz, ثابت فراغ *buf, بنانشانی len) {
    z_دھارا *strm;
    بنانشانی حرف باہرلا[BUFLEN];

    إذا (gz == عدم || !gz->اكتب)
        أرجع 0;
    strm = &(gz->strm);
    strm->اگلا_وچ = (فراغ *)buf;
    strm->avail_وچ = len;
    افعل {
        strm->اگلا_باہرلا = باہرلا;
        strm->avail_باہرلا = BUFLEN;
        (فراغ)deflate(strm, Z_NO_FLUSH);
        fwrite(باہرلا, 1, BUFLEN - strm->avail_باہرلا, gz->file);
    } طالما (strm->avail_باہرلا == 0);
    أرجع (صحيح)len;
}

ساكن صحيح gzread(gzFile gz, فراغ *buf, بنانشانی len) {
    صحيح ret;
    بنانشانی got;
    بنانشانی حرف وچ[1];
    z_دھارا *strm;

    إذا (gz == عدم || gz->اكتب)
        أرجع 0;
    إذا (gz->err)
        أرجع 0;
    strm = &(gz->strm);
    strm->اگلا_باہرلا = (فراغ *)buf;
    strm->avail_باہرلا = len;
    افعل {
        got = (بنانشانی)fread(وچ, 1, 1, gz->file);
        إذا (got == 0)
            اكسر;
        strm->اگلا_وچ = وچ;
        strm->avail_وچ = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        إذا (ret == Z_انکڑے_ERROR) {
            gz->err = Z_انکڑے_ERROR;
            gz->پیغام = strm->پیغام;
            أرجع 0;
        }
        إذا (ret == Z_دھارا_سرا)
            inflateReset(strm);
    } طالما (strm->avail_باہرلا);
    أرجع (صحيح)(len - strm->avail_باہرلا);
}

ساكن صحيح gzclose(gzFile gz) {
    z_دھارا *strm;
    بنانشانی حرف باہرلا[BUFLEN];

    إذا (gz == عدم)
        أرجع Z_دھارا_ERROR;
    strm = &(gz->strm);
    إذا (gz->اكتب) {
        strm->اگلا_وچ = Z_NULL;
        strm->avail_وچ = 0;
        افعل {
            strm->اگلا_باہرلا = باہرلا;
            strm->avail_باہرلا = BUFLEN;
            (فراغ)deflate(strm, Z_انت);
            fwrite(باہرلا, 1, BUFLEN - strm->avail_باہرلا, gz->file);
        } طالما (strm->avail_باہرلا == 0);
        deflate_سرا(strm);
    }
    وإلا
        inflate_سرا(strm);
    fclose(gz->file);
    حرر(gz);
    أرجع Z_OK;
}

ساكن ثابت حرف *gzerror(gzFile gz, صحيح *err) {
    *err = gz->err;
    أرجع gz->پیغام;
}

#جے_انت

ساكن حرف *prog;

/* ===========================================================================
 * Display error message and exit
 */
ساكن فراغ error(ثابت حرف *پیغام) {
    fprintf(stderr, "%s: %s\n", prog, پیغام);
    باہر_نکلو(1);
}

#جے_تعریف_شدہ USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ساكن صحيح gz_compress_mmap(FILE *وچ, gzFile باہرلا) {
    صحيح len;
    صحيح err;
    صحيح ifd = fileno(وچ);
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
    len = gzwrite(باہرلا, (حرف *)buf, (بنانشانی)buf_len);

    إذا (len != (صحيح)buf_len) error(gzerror(باہرلا, &err));

    munmap(buf, buf_len);
    fclose(وچ);
    إذا (gzclose(باہرلا) != Z_OK) error("failed gzclose");
    أرجع Z_OK;
}
#جے_انت /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ساكن فراغ gz_compress(FILE *وچ, gzFile باہرلا) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

#جے_تعریف_شدہ USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    إذا (gz_compress_mmap(وچ, باہرلا) == Z_OK) أرجع;
#جے_انت
    لكل (;;) {
        len = (صحيح)fread(buf, 1, حجم(buf), وچ);
        إذا (ferror(وچ)) {
            perror("fread");
            باہر_نکلو(1);
        }
        إذا (len == 0) اكسر;

        إذا (gzwrite(باہرلا, buf, (بنانشانی)len) != len) error(gzerror(باہرلا, &err));
    }
    fclose(وچ);
    إذا (gzclose(باہرلا) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ساكن فراغ gz_uncompress(gzFile وچ, FILE *باہرلا) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

    لكل (;;) {
        len = gzread(وچ, buf, حجم(buf));
        إذا (len < 0) error (gzerror(وچ, &err));
        إذا (len == 0) اكسر;

        إذا ((صحيح)fwrite(buf, 1, (بنانشانی)len, باہرلا) != len) {
            error("failed fwrite");
        }
    }
    إذا (fclose(باہرلا)) error("failed fclose");

    إذا (gzclose(وچ) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ساكن فراغ file_compress(حرف *file, حرف *منوال) {
    local حرف outfile[MAX_NAME_LEN+1], *سرا;
    FILE  *وچ;
    gzFile باہرلا;

    إذا (لڑی_لمبائی(file) + لڑی_لمبائی(GZ_SUFFIX) >= حجم(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        باہر_نکلو(1);
    }

    سرا = string_copy(outfile, file, حجم(outfile));
    string_copy(سرا, GZ_SUFFIX, حجم(outfile) - (z_ناپ_t)(سرا - outfile));

    وچ = fopen(file, "rb");
    إذا (وچ == عدم) {
        perror(file);
        باہر_نکلو(1);
    }
    باہرلا = gzopen(outfile, منوال);
    إذا (باہرلا == عدم) {
        fclose(وچ);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        باہر_نکلو(1);
    }
    gz_compress(وچ, باہرلا);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ساكن فراغ file_uncompress(حرف *file) {
    local حرف buf[MAX_NAME_LEN+1];
    حرف *infile, *outfile;
    FILE  *باہرلا;
    gzFile وچ;
    z_ناپ_t len = لڑی_لمبائی(file);

    إذا (len + لڑی_لمبائی(GZ_SUFFIX) >= حجم(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        باہر_نکلو(1);
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
    وچ = gzopen(infile, "rb");
    إذا (وچ == عدم) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        باہر_نکلو(1);
    }
    باہرلا = fopen(outfile, "wb");
    إذا (باہرلا == عدم) {
        gzclose(وچ);
        perror(file);
        باہر_نکلو(1);
    }

    gz_uncompress(وچ, باہرلا);

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

صحيح مکھ(صحيح argc, حرف *argv[]) {
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
        سیٹ_ثنائی_منوال(stdin);
        سیٹ_ثنائی_منوال(stdout);
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
            سیٹ_ثنائی_منوال(stdout);
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
                    FILE * وچ = fopen(*argv, "rb");

                    إذا (وچ == عدم) {
                        perror(*argv);
                    } وإلا {
                        file = gzdopen(fileno(stdout), outmode);
                        إذا (file == عدم) error("can't gzdopen stdout");

                        gz_compress(وچ, file);
                    }

                } وإلا {
                    file_compress(*argv, outmode);
                }
            }
        } طالما (argv++, --argc);
    }
    أرجع 0;
}
