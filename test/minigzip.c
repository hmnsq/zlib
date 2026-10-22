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

#جيڪڏهن_وصف_نه_ٿيل _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#جيڪڏهن_پورو

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#جيڪڏهن_پورو
#إذا defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#جيڪڏهن_پورو

#ضمن "zlib.h"
#ضمن <stdio.h>

#جيڪڏهن_وصف_ٿيل STDC
#  include <string.h>
#  include <stdlib.h>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#جيڪڏهن_پورو

#إذا defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define سيٽ_ٻٽو_گهڻائي(file) setmode(fileno(file), O_ٻٽو)
#وإلا
#  define سيٽ_ٻٽو_گهڻائي(file)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#جيڪڏهن_پورو
#إذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#جيڪڏهن_پورو
#إذا defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#جيڪڏهن_پورو

#إذا !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_ذريعو)
#جيڪڏهن_وصف_نه_ٿيل WIN32 /* unlink already in stdio.h for WIN32 */
  خارجي صحيح unlink(ثابت حرف *);
#جيڪڏهن_پورو
#جيڪڏهن_پورو

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
    DWORD lasterr = Get_آخري_Error();
    DWORD chars = Format_پيغام(FORMAT_پيغام_FROM_SYSTEM
        | FORMAT_پيغام_ALLOCATE_بفر,
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

    سيٽ_آخري_Error(lasterr);
    أرجع buf;
}

ساكن فراغ pwinerror (s)
    ثابت حرف *s;
{
    إذا (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_آخري_Error ()));
    وإلا
        fprintf(stderr, "%s\n", strwinerror(Get_آخري_Error ()));
}

#جيڪڏهن_پورو /* UNDER_CE */

#جيڪڏهن_وصف_نه_ٿيل GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#جيڪڏهن_پورو
#وصف_ڪريو SUFFIX_LEN (حجم(GZ_SUFFIX)-1)

#وصف_ڪريو BUFLEN      16384
#وصف_ڪريو MAX_NAME_LEN 1024

#جيڪڏهن_وصف_ٿيل MAXSEG_64K
#  define local ساكن
   /* Needed for systems with limitation on stack size. */
#وإلا
#  define local
#جيڪڏهن_پورو

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ساكن حرف *string_copy(حرف *dst, حرف ثابت *src, z_سائيز_t len) {
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

#جيڪڏهن_وصف_ٿيل Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#إذا defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#جيڪڏهن_پورو

ساكن فراغ *myalloc(فراغ *q, بنانشان n, بنانشان m) {
    (فراغ)q;
    أرجع صفر_يادگيري_ڏيو(n, m);
}

ساكن فراغ myfree(فراغ *q, فراغ *p) {
    (فراغ)q;
    حرر(p);
}

تعريفنوع بنية gzFile_s {
    FILE *file;
    صحيح اكتب;
    صحيح err;
    حرف *نياپو;
    z_وهڪرو strm;
} *gzFile;

ساكن gzFile gz_open(ثابت حرف *رستو, صحيح fd, ثابت حرف *گهڻائي) {
    gzFile gz;
    صحيح ret;

    gz = يادگيري_ڏيو(حجم(بنية gzFile_s));
    إذا (gz == عدم)
        أرجع عدم;
    gz->اكتب = strchr(گهڻائي, 'w') != عدم;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    إذا (gz->اكتب)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    وإلا {
        gz->strm.ايندڙ_اندريون = 0;
        gz->strm.avail_اندريون = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    إذا (ret != Z_OK) {
        حرر(gz);
        أرجع عدم;
    }
    gz->file = رستو == عدم ? fdopen(fd, gz->اكتب ? "wb" : "rb") :
                              fopen(رستو, gz->اكتب ? "wb" : "rb");
    إذا (gz->file == عدم) {
        gz->اكتب ? deflate_پڇاڙي(&(gz->strm)) : inflate_پڇاڙي(&(gz->strm));
        حرر(gz);
        أرجع عدم;
    }
    gz->err = 0;
    gz->نياپو = "";
    أرجع gz;
}

ساكن gzFile gzopen(ثابت حرف *رستو, ثابت حرف *گهڻائي) {
    أرجع gz_open(رستو, -1, گهڻائي);
}

ساكن gzFile gzdopen(صحيح fd, ثابت حرف *گهڻائي) {
    أرجع gz_open(عدم, fd, گهڻائي);
}

ساكن صحيح gzwrite(gzFile gz, ثابت فراغ *buf, بنانشان len) {
    z_وهڪرو *strm;
    بنانشان حرف ٻاهريون[BUFLEN];

    إذا (gz == عدم || !gz->اكتب)
        أرجع 0;
    strm = &(gz->strm);
    strm->ايندڙ_اندريون = (فراغ *)buf;
    strm->avail_اندريون = len;
    افعل {
        strm->ايندڙ_ٻاهريون = ٻاهريون;
        strm->avail_ٻاهريون = BUFLEN;
        (فراغ)deflate(strm, Z_NO_FLUSH);
        fwrite(ٻاهريون, 1, BUFLEN - strm->avail_ٻاهريون, gz->file);
    } طالما (strm->avail_ٻاهريون == 0);
    أرجع (صحيح)len;
}

ساكن صحيح gzread(gzFile gz, فراغ *buf, بنانشان len) {
    صحيح ret;
    بنانشان got;
    بنانشان حرف اندريون[1];
    z_وهڪرو *strm;

    إذا (gz == عدم || gz->اكتب)
        أرجع 0;
    إذا (gz->err)
        أرجع 0;
    strm = &(gz->strm);
    strm->ايندڙ_ٻاهريون = (فراغ *)buf;
    strm->avail_ٻاهريون = len;
    افعل {
        got = (بنانشان)fread(اندريون, 1, 1, gz->file);
        إذا (got == 0)
            اكسر;
        strm->ايندڙ_اندريون = اندريون;
        strm->avail_اندريون = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        إذا (ret == Z_معلومات_ERROR) {
            gz->err = Z_معلومات_ERROR;
            gz->نياپو = strm->نياپو;
            أرجع 0;
        }
        إذا (ret == Z_وهڪرو_پڇاڙي)
            inflateReset(strm);
    } طالما (strm->avail_ٻاهريون);
    أرجع (صحيح)(len - strm->avail_ٻاهريون);
}

ساكن صحيح gzclose(gzFile gz) {
    z_وهڪرو *strm;
    بنانشان حرف ٻاهريون[BUFLEN];

    إذا (gz == عدم)
        أرجع Z_وهڪرو_ERROR;
    strm = &(gz->strm);
    إذا (gz->اكتب) {
        strm->ايندڙ_اندريون = Z_NULL;
        strm->avail_اندريون = 0;
        افعل {
            strm->ايندڙ_ٻاهريون = ٻاهريون;
            strm->avail_ٻاهريون = BUFLEN;
            (فراغ)deflate(strm, Z_پڄاڻي);
            fwrite(ٻاهريون, 1, BUFLEN - strm->avail_ٻاهريون, gz->file);
        } طالما (strm->avail_ٻاهريون == 0);
        deflate_پڇاڙي(strm);
    }
    وإلا
        inflate_پڇاڙي(strm);
    fclose(gz->file);
    حرر(gz);
    أرجع Z_OK;
}

ساكن ثابت حرف *gzerror(gzFile gz, صحيح *err) {
    *err = gz->err;
    أرجع gz->نياپو;
}

#جيڪڏهن_پورو

ساكن حرف *prog;

/* ===========================================================================
 * Display error message and exit
 */
ساكن فراغ error(ثابت حرف *نياپو) {
    fprintf(stderr, "%s: %s\n", prog, نياپو);
    ٻاهر_نڪرو(1);
}

#جيڪڏهن_وصف_ٿيل USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ساكن صحيح gz_compress_mmap(FILE *اندريون, gzFile ٻاهريون) {
    صحيح len;
    صحيح err;
    صحيح ifd = fileno(اندريون);
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
    len = gzwrite(ٻاهريون, (حرف *)buf, (بنانشان)buf_len);

    إذا (len != (صحيح)buf_len) error(gzerror(ٻاهريون, &err));

    munmap(buf, buf_len);
    fclose(اندريون);
    إذا (gzclose(ٻاهريون) != Z_OK) error("failed gzclose");
    أرجع Z_OK;
}
#جيڪڏهن_پورو /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ساكن فراغ gz_compress(FILE *اندريون, gzFile ٻاهريون) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

#جيڪڏهن_وصف_ٿيل USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    إذا (gz_compress_mmap(اندريون, ٻاهريون) == Z_OK) أرجع;
#جيڪڏهن_پورو
    لكل (;;) {
        len = (صحيح)fread(buf, 1, حجم(buf), اندريون);
        إذا (ferror(اندريون)) {
            perror("fread");
            ٻاهر_نڪرو(1);
        }
        إذا (len == 0) اكسر;

        إذا (gzwrite(ٻاهريون, buf, (بنانشان)len) != len) error(gzerror(ٻاهريون, &err));
    }
    fclose(اندريون);
    إذا (gzclose(ٻاهريون) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ساكن فراغ gz_uncompress(gzFile اندريون, FILE *ٻاهريون) {
    local حرف buf[BUFLEN];
    صحيح len;
    صحيح err;

    لكل (;;) {
        len = gzread(اندريون, buf, حجم(buf));
        إذا (len < 0) error (gzerror(اندريون, &err));
        إذا (len == 0) اكسر;

        إذا ((صحيح)fwrite(buf, 1, (بنانشان)len, ٻاهريون) != len) {
            error("failed fwrite");
        }
    }
    إذا (fclose(ٻاهريون)) error("failed fclose");

    إذا (gzclose(اندريون) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ساكن فراغ file_compress(حرف *file, حرف *گهڻائي) {
    local حرف outfile[MAX_NAME_LEN+1], *پڇاڙي;
    FILE  *اندريون;
    gzFile ٻاهريون;

    إذا (لڙي_ڊيگهه(file) + لڙي_ڊيگهه(GZ_SUFFIX) >= حجم(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        ٻاهر_نڪرو(1);
    }

    پڇاڙي = string_copy(outfile, file, حجم(outfile));
    string_copy(پڇاڙي, GZ_SUFFIX, حجم(outfile) - (z_سائيز_t)(پڇاڙي - outfile));

    اندريون = fopen(file, "rb");
    إذا (اندريون == عدم) {
        perror(file);
        ٻاهر_نڪرو(1);
    }
    ٻاهريون = gzopen(outfile, گهڻائي);
    إذا (ٻاهريون == عدم) {
        fclose(اندريون);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        ٻاهر_نڪرو(1);
    }
    gz_compress(اندريون, ٻاهريون);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ساكن فراغ file_uncompress(حرف *file) {
    local حرف buf[MAX_NAME_LEN+1];
    حرف *infile, *outfile;
    FILE  *ٻاهريون;
    gzFile اندريون;
    z_سائيز_t len = لڙي_ڊيگهه(file);

    إذا (len + لڙي_ڊيگهه(GZ_SUFFIX) >= حجم(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        ٻاهر_نڪرو(1);
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
    اندريون = gzopen(infile, "rb");
    إذا (اندريون == عدم) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        ٻاهر_نڪرو(1);
    }
    ٻاهريون = fopen(outfile, "wb");
    إذا (ٻاهريون == عدم) {
        gzclose(اندريون);
        perror(file);
        ٻاهر_نڪرو(1);
    }

    gz_uncompress(اندريون, ٻاهريون);

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

صحيح مکيه(صحيح argc, حرف *argv[]) {
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
        سيٽ_ٻٽو_گهڻائي(stdin);
        سيٽ_ٻٽو_گهڻائي(stdout);
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
            سيٽ_ٻٽو_گهڻائي(stdout);
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
                    FILE * اندريون = fopen(*argv, "rb");

                    إذا (اندريون == عدم) {
                        perror(*argv);
                    } وإلا {
                        file = gzdopen(fileno(stdout), outmode);
                        إذا (file == عدم) error("can't gzdopen stdout");

                        gz_compress(اندريون, file);
                    }

                } وإلا {
                    file_compress(*argv, outmode);
                }
            }
        } طالما (argv++, --argc);
    }
    أرجع 0;
}
