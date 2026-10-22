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

#اگر_غیر_تعریف_شدہ _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#اگر_ختم

#اگر defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#اگر_ختم
#اگر defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#اگر_ختم

#شامل "zlib.h"
#شامل <stdio.h>

#اگر_تعریف_شدہ STDC
#  include <string.h>
#  include <stdlib.h>
#اگر_ختم

#اگر_تعریف_شدہ USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#اگر_ختم

#اگر defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define سیٹ_ثنائی_منوال(file) setmode(fileno(file), O_ثنائی)
#ورنہ
#  define سیٹ_ثنائی_منوال(file)
#اگر_ختم

#اگر_تعریف_شدہ VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#اگر_ختم
#اگر defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#اگر_ختم
#اگر defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#اگر_ختم

#اگر !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_ماخذ)
#اگر_غیر_تعریف_شدہ WIN32 /* unlink already in stdio.h for WIN32 */
  خارجی صحیح unlink(ثابت حرف *);
#اگر_ختم
#اگر_ختم

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

ساکن حرف *strwinerror (error)
     DWORD error;
{
    ساکن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_آخری_Error();
    DWORD chars = Format_پیغام(FORMAT_پیغام_FROM_SYSTEM
        | FORMAT_پیغام_ALLOCATE_بفر,
        تہی,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        تہی);
    اگر (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        اگر (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        اگر (chars > حجم (buf) - 1) {
            chars = حجم (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    ورنہ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    سیٹ_آخری_Error(lasterr);
    واپس buf;
}

ساکن خالی pwinerror (s)
    ثابت حرف *s;
{
    اگر (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_آخری_Error ()));
    ورنہ
        fprintf(stderr, "%s\n", strwinerror(Get_آخری_Error ()));
}

#اگر_ختم /* UNDER_CE */

#اگر_غیر_تعریف_شدہ GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#اگر_ختم
#تعریف_کرو SUFFIX_LEN (حجم(GZ_SUFFIX)-1)

#تعریف_کرو BUFLEN      16384
#تعریف_کرو MAX_NAME_LEN 1024

#اگر_تعریف_شدہ MAXSEG_64K
#  define local ساکن
   /* Needed for systems with limitation on stack size. */
#ورنہ
#  define local
#اگر_ختم

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
ساکن حرف *string_copy(حرف *dst, حرف ثابت *src, z_سائز_t len) {
    اگر (len == 0)
        واپس تہی;
    جب (--len) {
        *dst = *src++;
        اگر (*dst == 0)
            واپس dst;
        dst++;
    }
    *dst = 0;
    واپس dst;
}

#اگر_تعریف_شدہ Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#اگر defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#اگر_ختم

ساکن خالی *myalloc(خالی *q, غیرعلامتی n, غیرعلامتی m) {
    (خالی)q;
    واپس صفر_حافظہ_دو(n, m);
}

ساکن خالی myfree(خالی *q, خالی *p) {
    (خالی)q;
    آزاد(p);
}

قسمتعریف ساخت gzFile_s {
    FILE *file;
    صحیح اكتب;
    صحیح err;
    حرف *پیغام;
    z_دھارا strm;
} *gzFile;

ساکن gzFile gz_open(ثابت حرف *راستہ, صحیح fd, ثابت حرف *منوال) {
    gzFile gz;
    صحیح ret;

    gz = حافظہ_دو(حجم(ساخت gzFile_s));
    اگر (gz == تہی)
        واپس تہی;
    gz->اكتب = strchr(منوال, 'w') != تہی;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    اگر (gz->اكتب)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    ورنہ {
        gz->strm.اگلا_اندر = 0;
        gz->strm.avail_اندر = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    اگر (ret != Z_OK) {
        آزاد(gz);
        واپس تہی;
    }
    gz->file = راستہ == تہی ? fdopen(fd, gz->اكتب ? "wb" : "rb") :
                              fopen(راستہ, gz->اكتب ? "wb" : "rb");
    اگر (gz->file == تہی) {
        gz->اكتب ? deflate_انتہا(&(gz->strm)) : inflate_انتہا(&(gz->strm));
        آزاد(gz);
        واپس تہی;
    }
    gz->err = 0;
    gz->پیغام = "";
    واپس gz;
}

ساکن gzFile gzopen(ثابت حرف *راستہ, ثابت حرف *منوال) {
    واپس gz_open(راستہ, -1, منوال);
}

ساکن gzFile gzdopen(صحیح fd, ثابت حرف *منوال) {
    واپس gz_open(تہی, fd, منوال);
}

ساکن صحیح gzwrite(gzFile gz, ثابت خالی *buf, غیرعلامتی len) {
    z_دھارا *strm;
    غیرعلامتی حرف برآمد[BUFLEN];

    اگر (gz == تہی || !gz->اكتب)
        واپس 0;
    strm = &(gz->strm);
    strm->اگلا_اندر = (خالی *)buf;
    strm->avail_اندر = len;
    کرو {
        strm->اگلا_برآمد = برآمد;
        strm->avail_برآمد = BUFLEN;
        (خالی)deflate(strm, Z_NO_FLUSH);
        fwrite(برآمد, 1, BUFLEN - strm->avail_برآمد, gz->file);
    } جب (strm->avail_برآمد == 0);
    واپس (صحیح)len;
}

ساکن صحیح gzread(gzFile gz, خالی *buf, غیرعلامتی len) {
    صحیح ret;
    غیرعلامتی got;
    غیرعلامتی حرف اندر[1];
    z_دھارا *strm;

    اگر (gz == تہی || gz->اكتب)
        واپس 0;
    اگر (gz->err)
        واپس 0;
    strm = &(gz->strm);
    strm->اگلا_برآمد = (خالی *)buf;
    strm->avail_برآمد = len;
    کرو {
        got = (غیرعلامتی)fread(اندر, 1, 1, gz->file);
        اگر (got == 0)
            توڑو;
        strm->اگلا_اندر = اندر;
        strm->avail_اندر = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        اگر (ret == Z_ڈیٹا_ERROR) {
            gz->err = Z_ڈیٹا_ERROR;
            gz->پیغام = strm->پیغام;
            واپس 0;
        }
        اگر (ret == Z_دھارا_انتہا)
            inflateReset(strm);
    } جب (strm->avail_برآمد);
    واپس (صحیح)(len - strm->avail_برآمد);
}

ساکن صحیح gzclose(gzFile gz) {
    z_دھارا *strm;
    غیرعلامتی حرف برآمد[BUFLEN];

    اگر (gz == تہی)
        واپس Z_دھارا_ERROR;
    strm = &(gz->strm);
    اگر (gz->اكتب) {
        strm->اگلا_اندر = Z_NULL;
        strm->avail_اندر = 0;
        کرو {
            strm->اگلا_برآمد = برآمد;
            strm->avail_برآمد = BUFLEN;
            (خالی)deflate(strm, Z_اختتام);
            fwrite(برآمد, 1, BUFLEN - strm->avail_برآمد, gz->file);
        } جب (strm->avail_برآمد == 0);
        deflate_انتہا(strm);
    }
    ورنہ
        inflate_انتہا(strm);
    fclose(gz->file);
    آزاد(gz);
    واپس Z_OK;
}

ساکن ثابت حرف *gzerror(gzFile gz, صحیح *err) {
    *err = gz->err;
    واپس gz->پیغام;
}

#اگر_ختم

ساکن حرف *prog;

/* ===========================================================================
 * Display error message and exit
 */
ساکن خالی error(ثابت حرف *پیغام) {
    fprintf(stderr, "%s: %s\n", prog, پیغام);
    باہر_نکلو(1);
}

#اگر_تعریف_شدہ USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
ساکن صحیح gz_compress_mmap(FILE *اندر, gzFile برآمد) {
    صحیح len;
    صحیح err;
    صحیح ifd = fileno(اندر);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    ساخت stat sb;

    /* Determine the size of the file, needed for mmap: */
    اگر (fstat(ifd, &sb) < 0) واپس Z_ERRNO;
    buf_len = sb.st_size;
    اگر (buf_len <= 0) واپس Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    اگر (buf == (caddr_t)(-1)) واپس Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(برآمد, (حرف *)buf, (غیرعلامتی)buf_len);

    اگر (len != (صحیح)buf_len) error(gzerror(برآمد, &err));

    munmap(buf, buf_len);
    fclose(اندر);
    اگر (gzclose(برآمد) != Z_OK) error("failed gzclose");
    واپس Z_OK;
}
#اگر_ختم /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

ساکن خالی gz_compress(FILE *اندر, gzFile برآمد) {
    local حرف buf[BUFLEN];
    صحیح len;
    صحیح err;

#اگر_تعریف_شدہ USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    اگر (gz_compress_mmap(اندر, برآمد) == Z_OK) واپس;
#اگر_ختم
    برائے (;;) {
        len = (صحیح)fread(buf, 1, حجم(buf), اندر);
        اگر (ferror(اندر)) {
            perror("fread");
            باہر_نکلو(1);
        }
        اگر (len == 0) توڑو;

        اگر (gzwrite(برآمد, buf, (غیرعلامتی)len) != len) error(gzerror(برآمد, &err));
    }
    fclose(اندر);
    اگر (gzclose(برآمد) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
ساکن خالی gz_uncompress(gzFile اندر, FILE *برآمد) {
    local حرف buf[BUFLEN];
    صحیح len;
    صحیح err;

    برائے (;;) {
        len = gzread(اندر, buf, حجم(buf));
        اگر (len < 0) error (gzerror(اندر, &err));
        اگر (len == 0) توڑو;

        اگر ((صحیح)fwrite(buf, 1, (غیرعلامتی)len, برآمد) != len) {
            error("failed fwrite");
        }
    }
    اگر (fclose(برآمد)) error("failed fclose");

    اگر (gzclose(اندر) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
ساکن خالی file_compress(حرف *file, حرف *منوال) {
    local حرف outfile[MAX_NAME_LEN+1], *انتہا;
    FILE  *اندر;
    gzFile برآمد;

    اگر (لڑی_لمبائی(file) + لڑی_لمبائی(GZ_SUFFIX) >= حجم(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        باہر_نکلو(1);
    }

    انتہا = string_copy(outfile, file, حجم(outfile));
    string_copy(انتہا, GZ_SUFFIX, حجم(outfile) - (z_سائز_t)(انتہا - outfile));

    اندر = fopen(file, "rb");
    اگر (اندر == تہی) {
        perror(file);
        باہر_نکلو(1);
    }
    برآمد = gzopen(outfile, منوال);
    اگر (برآمد == تہی) {
        fclose(اندر);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        باہر_نکلو(1);
    }
    gz_compress(اندر, برآمد);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
ساکن خالی file_uncompress(حرف *file) {
    local حرف buf[MAX_NAME_LEN+1];
    حرف *infile, *outfile;
    FILE  *برآمد;
    gzFile اندر;
    z_سائز_t len = لڑی_لمبائی(file);

    اگر (len + لڑی_لمبائی(GZ_SUFFIX) >= حجم(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        باہر_نکلو(1);
    }

    string_copy(buf, file, حجم(buf));

    اگر (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } ورنہ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, حجم(buf) - len);
    }
    اندر = gzopen(infile, "rb");
    اگر (اندر == تہی) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        باہر_نکلو(1);
    }
    برآمد = fopen(outfile, "wb");
    اگر (برآمد == تہی) {
        gzclose(اندر);
        perror(file);
        باہر_نکلو(1);
    }

    gz_uncompress(اندر, برآمد);

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

صحیح مین(صحیح argc, حرف *argv[]) {
    صحیح copyout = 0;
    صحیح uncompr = 0;
    gzFile file;
    حرف *bname, outmode[5];

    string_copy(outmode, "wb6 ", حجم(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    اگر (bname)
      bname++;
    ورنہ
      bname = argv[0];
    argc--, argv++;

    اگر (!strcmp(bname, "gunzip"))
      uncompr = 1;
    ورنہ اگر (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    جب (argc > 0) {
      اگر (strcmp(*argv, "-c") == 0)
        copyout = 1;
      ورنہ اگر (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      ورنہ اگر (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      ورنہ اگر (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      ورنہ اگر (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      ورنہ اگر ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      ورنہ
        توڑو;
      argc--, argv++;
    }
    اگر (outmode[3] == ' ')
        outmode[3] = 0;
    اگر (argc == 0) {
        سیٹ_ثنائی_منوال(stdin);
        سیٹ_ثنائی_منوال(stdout);
        اگر (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            اگر (file == تہی) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } ورنہ {
            file = gzdopen(fileno(stdout), outmode);
            اگر (file == تہی) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } ورنہ {
        اگر (copyout) {
            سیٹ_ثنائی_منوال(stdout);
        }
        کرو {
            اگر (uncompr) {
                اگر (copyout) {
                    file = gzopen(*argv, "rb");
                    اگر (file == تہی)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    ورنہ
                        gz_uncompress(file, stdout);
                } ورنہ {
                    file_uncompress(*argv);
                }
            } ورنہ {
                اگر (copyout) {
                    FILE * اندر = fopen(*argv, "rb");

                    اگر (اندر == تہی) {
                        perror(*argv);
                    } ورنہ {
                        file = gzdopen(fileno(stdout), outmode);
                        اگر (file == تہی) error("can't gzdopen stdout");

                        gz_compress(اندر, file);
                    }

                } ورنہ {
                    file_compress(*argv, outmode);
                }
            }
        } جب (argv++, --argc);
    }
    واپس 0;
}
