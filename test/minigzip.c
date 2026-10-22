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

#यदि_अपरिभाषितम् _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#यदि_अन्तः

#यदि defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#यदि_अन्तः
#यदि defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#यदि_अन्तः

#अन्तर्भाव "zlib.h"
#अन्तर्भाव <stdio.h>

#यदि_परिभाषितम् STDC
#  include <string.h>
#  include <stdlib.h>
#यदि_अन्तः

#यदि_परिभाषितम् USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#यदि_अन्तः

#यदि defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define समुच्चयः_द्विमान_बहुलकम्(file) setmode(fileno(file), O_द्विमान)
#अन्यथा
#  define समुच्चयः_द्विमान_बहुलकम्(file)
#यदि_अन्तः

#यदि_परिभाषितम् VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#यदि_अन्तः
#यदि defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#यदि_अन्तः
#यदि defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#यदि_अन्तः

#यदि !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#यदि_अपरिभाषितम् WIN32 /* unlink already in stdio.h for WIN32 */
  बाह्य पूर्णाङ्क unlink(ध्रुव वर्ण *);
#यदि_अन्तः
#यदि_अन्तः

#यदि defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

स्थिर वर्ण *strwinerror (error)
     DWORD error;
{
    स्थिर वर्ण buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_चरमः_Error();
    DWORD chars = Format_सन्देशः(FORMAT_सन्देशः_FROM_SYSTEM
        | FORMAT_सन्देशः_ALLOCATE_बफर,
        रिक्त,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        रिक्त);
    यदि (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        यदि (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        यदि (chars > परिमाण (buf) - 1) {
            chars = परिमाण (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    अन्यथा {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    समुच्चयः_चरमः_Error(lasterr);
    प्रत्यागम buf;
}

स्थिर शून्य pwinerror (s)
    ध्रुव वर्ण *s;
{
    यदि (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_चरमः_Error ()));
    अन्यथा
        fprintf(stderr, "%s\n", strwinerror(Get_चरमः_Error ()));
}

#यदि_अन्तः /* UNDER_CE */

#यदि_अपरिभाषितम् GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#यदि_अन्तः
#परिभाषय SUFFIX_LEN (परिमाण(GZ_SUFFIX)-1)

#परिभाषय BUFLEN      16384
#परिभाषय MAX_NAME_LEN 1024

#यदि_परिभाषितम् MAXSEG_64K
#  define local स्थिर
   /* Needed for systems with limitation on stack size. */
#अन्यथा
#  define local
#यदि_अन्तः

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
स्थिर वर्ण *string_copy(वर्ण *dst, वर्ण ध्रुव *src, z_आकार_t len) {
    यदि (len == 0)
        प्रत्यागम रिक्त;
    यावत् (--len) {
        *dst = *src++;
        यदि (*dst == 0)
            प्रत्यागम dst;
        dst++;
    }
    *dst = 0;
    प्रत्यागम dst;
}

#यदि_परिभाषितम् Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#यदि defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#यदि_अन्तः

स्थिर शून्य *myalloc(शून्य *q, अचिह्नित n, अचिह्नित m) {
    (शून्य)q;
    प्रत्यागम शून्यस्मृतिं_देहि(n, m);
}

स्थिर शून्य myfree(शून्य *q, शून्य *p) {
    (शून्य)q;
    मुक्त(p);
}

प्रकारनिर्देश संरचना gzFile_s {
    FILE *file;
    पूर्णाङ्क लिख;
    पूर्णाङ्क err;
    वर्ण *सन्देशः;
    z_प्रवाहः strm;
} *gzFile;

स्थिर gzFile gz_open(ध्रुव वर्ण *मार्गः, पूर्णाङ्क fd, ध्रुव वर्ण *बहुलकम्) {
    gzFile gz;
    पूर्णाङ्क ret;

    gz = स्मृतिं_देहि(परिमाण(संरचना gzFile_s));
    यदि (gz == रिक्त)
        प्रत्यागम रिक्त;
    gz->लिख = strchr(बहुलकम्, 'w') != रिक्त;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    यदि (gz->लिख)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    अन्यथा {
        gz->strm.अग्र_अन्तर् = 0;
        gz->strm.avail_अन्तर् = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    यदि (ret != Z_OK) {
        मुक्त(gz);
        प्रत्यागम रिक्त;
    }
    gz->file = मार्गः == रिक्त ? fdopen(fd, gz->लिख ? "wb" : "rb") :
                              fopen(मार्गः, gz->लिख ? "wb" : "rb");
    यदि (gz->file == रिक्त) {
        gz->लिख ? deflate_अन्तः(&(gz->strm)) : inflate_अन्तः(&(gz->strm));
        मुक्त(gz);
        प्रत्यागम रिक्त;
    }
    gz->err = 0;
    gz->सन्देशः = "";
    प्रत्यागम gz;
}

स्थिर gzFile gzopen(ध्रुव वर्ण *मार्गः, ध्रुव वर्ण *बहुलकम्) {
    प्रत्यागम gz_open(मार्गः, -1, बहुलकम्);
}

स्थिर gzFile gzdopen(पूर्णाङ्क fd, ध्रुव वर्ण *बहुलकम्) {
    प्रत्यागम gz_open(रिक्त, fd, बहुलकम्);
}

स्थिर पूर्णाङ्क gzwrite(gzFile gz, ध्रुव शून्य *buf, अचिह्नित len) {
    z_प्रवाहः *strm;
    अचिह्नित वर्ण बहिर्गमनम्[BUFLEN];

    यदि (gz == रिक्त || !gz->लिख)
        प्रत्यागम 0;
    strm = &(gz->strm);
    strm->अग्र_अन्तर् = (शून्य *)buf;
    strm->avail_अन्तर् = len;
    कुरु {
        strm->अग्र_बहिर्गमनम् = बहिर्गमनम्;
        strm->avail_बहिर्गमनम् = BUFLEN;
        (शून्य)deflate(strm, Z_NO_FLUSH);
        fwrite(बहिर्गमनम्, 1, BUFLEN - strm->avail_बहिर्गमनम्, gz->file);
    } यावत् (strm->avail_बहिर्गमनम् == 0);
    प्रत्यागम (पूर्णाङ्क)len;
}

स्थिर पूर्णाङ्क gzread(gzFile gz, शून्य *buf, अचिह्नित len) {
    पूर्णाङ्क ret;
    अचिह्नित got;
    अचिह्नित वर्ण अन्तर्[1];
    z_प्रवाहः *strm;

    यदि (gz == रिक्त || gz->लिख)
        प्रत्यागम 0;
    यदि (gz->err)
        प्रत्यागम 0;
    strm = &(gz->strm);
    strm->अग्र_बहिर्गमनम् = (शून्य *)buf;
    strm->avail_बहिर्गमनम् = len;
    कुरु {
        got = (अचिह्नित)fread(अन्तर्, 1, 1, gz->file);
        यदि (got == 0)
            भङ्ग;
        strm->अग्र_अन्तर् = अन्तर्;
        strm->avail_अन्तर् = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        यदि (ret == Z_दत्त_ERROR) {
            gz->err = Z_दत्त_ERROR;
            gz->सन्देशः = strm->सन्देशः;
            प्रत्यागम 0;
        }
        यदि (ret == Z_प्रवाहः_अन्तः)
            inflateReset(strm);
    } यावत् (strm->avail_बहिर्गमनम्);
    प्रत्यागम (पूर्णाङ्क)(len - strm->avail_बहिर्गमनम्);
}

स्थिर पूर्णाङ्क gzclose(gzFile gz) {
    z_प्रवाहः *strm;
    अचिह्नित वर्ण बहिर्गमनम्[BUFLEN];

    यदि (gz == रिक्त)
        प्रत्यागम Z_प्रवाहः_ERROR;
    strm = &(gz->strm);
    यदि (gz->लिख) {
        strm->अग्र_अन्तर् = Z_NULL;
        strm->avail_अन्तर् = 0;
        कुरु {
            strm->अग्र_बहिर्गमनम् = बहिर्गमनम्;
            strm->avail_बहिर्गमनम् = BUFLEN;
            (शून्य)deflate(strm, Z_FINISH);
            fwrite(बहिर्गमनम्, 1, BUFLEN - strm->avail_बहिर्गमनम्, gz->file);
        } यावत् (strm->avail_बहिर्गमनम् == 0);
        deflate_अन्तः(strm);
    }
    अन्यथा
        inflate_अन्तः(strm);
    fclose(gz->file);
    मुक्त(gz);
    प्रत्यागम Z_OK;
}

स्थिर ध्रुव वर्ण *gzerror(gzFile gz, पूर्णाङ्क *err) {
    *err = gz->err;
    प्रत्यागम gz->सन्देशः;
}

#यदि_अन्तः

स्थिर वर्ण *prog;

/* ===========================================================================
 * Display error message and exit
 */
स्थिर शून्य error(ध्रुव वर्ण *सन्देशः) {
    fprintf(stderr, "%s: %s\n", prog, सन्देशः);
    निर्गच्छ(1);
}

#यदि_परिभाषितम् USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
स्थिर पूर्णाङ्क gz_compress_mmap(FILE *अन्तर्, gzFile बहिर्गमनम्) {
    पूर्णाङ्क len;
    पूर्णाङ्क err;
    पूर्णाङ्क ifd = fileno(अन्तर्);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    संरचना stat sb;

    /* Determine the size of the file, needed for mmap: */
    यदि (fstat(ifd, &sb) < 0) प्रत्यागम Z_ERRNO;
    buf_len = sb.st_size;
    यदि (buf_len <= 0) प्रत्यागम Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    यदि (buf == (caddr_t)(-1)) प्रत्यागम Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(बहिर्गमनम्, (वर्ण *)buf, (अचिह्नित)buf_len);

    यदि (len != (पूर्णाङ्क)buf_len) error(gzerror(बहिर्गमनम्, &err));

    munmap(buf, buf_len);
    fclose(अन्तर्);
    यदि (gzclose(बहिर्गमनम्) != Z_OK) error("failed gzclose");
    प्रत्यागम Z_OK;
}
#यदि_अन्तः /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

स्थिर शून्य gz_compress(FILE *अन्तर्, gzFile बहिर्गमनम्) {
    local वर्ण buf[BUFLEN];
    पूर्णाङ्क len;
    पूर्णाङ्क err;

#यदि_परिभाषितम् USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    यदि (gz_compress_mmap(अन्तर्, बहिर्गमनम्) == Z_OK) प्रत्यागम;
#यदि_अन्तः
    प्रति (;;) {
        len = (पूर्णाङ्क)fread(buf, 1, परिमाण(buf), अन्तर्);
        यदि (ferror(अन्तर्)) {
            perror("fread");
            निर्गच्छ(1);
        }
        यदि (len == 0) भङ्ग;

        यदि (gzwrite(बहिर्गमनम्, buf, (अचिह्नित)len) != len) error(gzerror(बहिर्गमनम्, &err));
    }
    fclose(अन्तर्);
    यदि (gzclose(बहिर्गमनम्) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
स्थिर शून्य gz_uncompress(gzFile अन्तर्, FILE *बहिर्गमनम्) {
    local वर्ण buf[BUFLEN];
    पूर्णाङ्क len;
    पूर्णाङ्क err;

    प्रति (;;) {
        len = gzread(अन्तर्, buf, परिमाण(buf));
        यदि (len < 0) error (gzerror(अन्तर्, &err));
        यदि (len == 0) भङ्ग;

        यदि ((पूर्णाङ्क)fwrite(buf, 1, (अचिह्नित)len, बहिर्गमनम्) != len) {
            error("failed fwrite");
        }
    }
    यदि (fclose(बहिर्गमनम्)) error("failed fclose");

    यदि (gzclose(अन्तर्) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
स्थिर शून्य file_compress(वर्ण *file, वर्ण *बहुलकम्) {
    local वर्ण outfile[MAX_NAME_LEN+1], *अन्तः_2;
    FILE  *अन्तर्;
    gzFile बहिर्गमनम्;

    यदि (वाक्यदैर्घ्यम्(file) + वाक्यदैर्घ्यम्(GZ_SUFFIX) >= परिमाण(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        निर्गच्छ(1);
    }

    अन्तः_2 = string_copy(outfile, file, परिमाण(outfile));
    string_copy(अन्तः_2, GZ_SUFFIX, परिमाण(outfile) - (z_आकार_t)(अन्तः_2 - outfile));

    अन्तर् = fopen(file, "rb");
    यदि (अन्तर् == रिक्त) {
        perror(file);
        निर्गच्छ(1);
    }
    बहिर्गमनम् = gzopen(outfile, बहुलकम्);
    यदि (बहिर्गमनम् == रिक्त) {
        fclose(अन्तर्);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        निर्गच्छ(1);
    }
    gz_compress(अन्तर्, बहिर्गमनम्);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
स्थिर शून्य file_uncompress(वर्ण *file) {
    local वर्ण buf[MAX_NAME_LEN+1];
    वर्ण *infile, *outfile;
    FILE  *बहिर्गमनम्;
    gzFile अन्तर्;
    z_आकार_t len = वाक्यदैर्घ्यम्(file);

    यदि (len + वाक्यदैर्घ्यम्(GZ_SUFFIX) >= परिमाण(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        निर्गच्छ(1);
    }

    string_copy(buf, file, परिमाण(buf));

    यदि (len > SUFFIX_LEN && वाक्यं_तुलय(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } अन्यथा {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, परिमाण(buf) - len);
    }
    अन्तर् = gzopen(infile, "rb");
    यदि (अन्तर् == रिक्त) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        निर्गच्छ(1);
    }
    बहिर्गमनम् = fopen(outfile, "wb");
    यदि (बहिर्गमनम् == रिक्त) {
        gzclose(अन्तर्);
        perror(file);
        निर्गच्छ(1);
    }

    gz_uncompress(अन्तर्, बहिर्गमनम्);

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

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[]) {
    पूर्णाङ्क copyout = 0;
    पूर्णाङ्क uncompr = 0;
    gzFile file;
    वर्ण *bname, outmode[5];

    string_copy(outmode, "wb6 ", परिमाण(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    यदि (bname)
      bname++;
    अन्यथा
      bname = argv[0];
    argc--, argv++;

    यदि (!वाक्यं_तुलय(bname, "gunzip"))
      uncompr = 1;
    अन्यथा यदि (!वाक्यं_तुलय(bname, "zcat"))
      copyout = uncompr = 1;

    यावत् (argc > 0) {
      यदि (वाक्यं_तुलय(*argv, "-c") == 0)
        copyout = 1;
      अन्यथा यदि (वाक्यं_तुलय(*argv, "-d") == 0)
        uncompr = 1;
      अन्यथा यदि (वाक्यं_तुलय(*argv, "-f") == 0)
        outmode[3] = 'f';
      अन्यथा यदि (वाक्यं_तुलय(*argv, "-h") == 0)
        outmode[3] = 'h';
      अन्यथा यदि (वाक्यं_तुलय(*argv, "-r") == 0)
        outmode[3] = 'R';
      अन्यथा यदि ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      अन्यथा
        भङ्ग;
      argc--, argv++;
    }
    यदि (outmode[3] == ' ')
        outmode[3] = 0;
    यदि (argc == 0) {
        समुच्चयः_द्विमान_बहुलकम्(stdin);
        समुच्चयः_द्विमान_बहुलकम्(stdout);
        यदि (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            यदि (file == रिक्त) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } अन्यथा {
            file = gzdopen(fileno(stdout), outmode);
            यदि (file == रिक्त) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } अन्यथा {
        यदि (copyout) {
            समुच्चयः_द्विमान_बहुलकम्(stdout);
        }
        कुरु {
            यदि (uncompr) {
                यदि (copyout) {
                    file = gzopen(*argv, "rb");
                    यदि (file == रिक्त)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    अन्यथा
                        gz_uncompress(file, stdout);
                } अन्यथा {
                    file_uncompress(*argv);
                }
            } अन्यथा {
                यदि (copyout) {
                    FILE * अन्तर् = fopen(*argv, "rb");

                    यदि (अन्तर् == रिक्त) {
                        perror(*argv);
                    } अन्यथा {
                        file = gzdopen(fileno(stdout), outmode);
                        यदि (file == रिक्त) error("can't gzdopen stdout");

                        gz_compress(अन्तर्, file);
                    }

                } अन्यथा {
                    file_compress(*argv, outmode);
                }
            }
        } यावत् (argv++, --argc);
    }
    प्रत्यागम 0;
}
