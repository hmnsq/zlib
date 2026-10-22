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

#यदि defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#यदि defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#अन्तर्भाव "zlib.h"
#अन्तर्भाव <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#यदि defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_द्विआधारी_MODE(file) setmode(fileno(file), O_द्विआधारी)
#अन्यथा
#  define SET_द्विआधारी_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#यदि defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#यदि defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#यदि !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  बाह्य पूर्णाङ्क unlink(ध्रुव वर्ण *);
#endif
#endif

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
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
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

    SetLastError(lasterr);
    प्रत्यागम buf;
}

स्थिर शून्य pwinerror (s)
    ध्रुव वर्ण *s;
{
    यदि (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    अन्यथा
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (परिमाण(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local स्थिर
   /* Needed for systems with limitation on stack size. */
#अन्यथा
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
स्थिर वर्ण *string_copy(वर्ण *dst, वर्ण ध्रुव *src, z_size_t len) {
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

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#यदि defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

स्थिर शून्य *myalloc(शून्य *q, unsigned n, unsigned m) {
    (शून्य)q;
    प्रत्यागम calloc(n, m);
}

स्थिर शून्य myfree(शून्य *q, शून्य *p) {
    (शून्य)q;
    मुक्त(p);
}

प्रकारनिर्देश संरचना gzFile_s {
    FILE *file;
    पूर्णाङ्क लिख;
    पूर्णाङ्क err;
    वर्ण *संदेश;
    z_stream strm;
} *gzFile;

स्थिर gzFile gz_open(ध्रुव वर्ण *path, पूर्णाङ्क fd, ध्रुव वर्ण *mode) {
    gzFile gz;
    पूर्णाङ्क ret;

    gz = malloc(परिमाण(संरचना gzFile_s));
    यदि (gz == रिक्त)
        प्रत्यागम रिक्त;
    gz->लिख = strchr(mode, 'w') != रिक्त;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    यदि (gz->लिख)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    अन्यथा {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    यदि (ret != Z_OK) {
        मुक्त(gz);
        प्रत्यागम रिक्त;
    }
    gz->file = path == रिक्त ? fdopen(fd, gz->लिख ? "wb" : "rb") :
                              fopen(path, gz->लिख ? "wb" : "rb");
    यदि (gz->file == रिक्त) {
        gz->लिख ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        मुक्त(gz);
        प्रत्यागम रिक्त;
    }
    gz->err = 0;
    gz->संदेश = "";
    प्रत्यागम gz;
}

स्थिर gzFile gzopen(ध्रुव वर्ण *path, ध्रुव वर्ण *mode) {
    प्रत्यागम gz_open(path, -1, mode);
}

स्थिर gzFile gzdopen(पूर्णाङ्क fd, ध्रुव वर्ण *mode) {
    प्रत्यागम gz_open(रिक्त, fd, mode);
}

स्थिर पूर्णाङ्क gzwrite(gzFile gz, ध्रुव शून्य *buf, unsigned len) {
    z_stream *strm;
    unsigned वर्ण out[BUFLEN];

    यदि (gz == रिक्त || !gz->लिख)
        प्रत्यागम 0;
    strm = &(gz->strm);
    strm->next_in = (शून्य *)buf;
    strm->avail_in = len;
    कुरु {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (शून्य)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } यावत् (strm->avail_out == 0);
    प्रत्यागम (पूर्णाङ्क)len;
}

स्थिर पूर्णाङ्क gzread(gzFile gz, शून्य *buf, unsigned len) {
    पूर्णाङ्क ret;
    unsigned got;
    unsigned वर्ण in[1];
    z_stream *strm;

    यदि (gz == रिक्त || gz->लिख)
        प्रत्यागम 0;
    यदि (gz->err)
        प्रत्यागम 0;
    strm = &(gz->strm);
    strm->next_out = (शून्य *)buf;
    strm->avail_out = len;
    कुरु {
        got = (unsigned)fread(in, 1, 1, gz->file);
        यदि (got == 0)
            भङ्ग;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        यदि (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->संदेश = strm->संदेश;
            प्रत्यागम 0;
        }
        यदि (ret == Z_STREAM_END)
            inflateReset(strm);
    } यावत् (strm->avail_out);
    प्रत्यागम (पूर्णाङ्क)(len - strm->avail_out);
}

स्थिर पूर्णाङ्क gzclose(gzFile gz) {
    z_stream *strm;
    unsigned वर्ण out[BUFLEN];

    यदि (gz == रिक्त)
        प्रत्यागम Z_STREAM_ERROR;
    strm = &(gz->strm);
    यदि (gz->लिख) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        कुरु {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (शून्य)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } यावत् (strm->avail_out == 0);
        deflateEnd(strm);
    }
    अन्यथा
        inflateEnd(strm);
    fclose(gz->file);
    मुक्त(gz);
    प्रत्यागम Z_OK;
}

स्थिर ध्रुव वर्ण *gzerror(gzFile gz, पूर्णाङ्क *err) {
    *err = gz->err;
    प्रत्यागम gz->संदेश;
}

#endif

स्थिर वर्ण *prog;

/* ===========================================================================
 * Display error message and exit
 */
स्थिर शून्य error(ध्रुव वर्ण *संदेश) {
    fprintf(stderr, "%s: %s\n", prog, संदेश);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
स्थिर पूर्णाङ्क gz_compress_mmap(FILE *in, gzFile out) {
    पूर्णाङ्क len;
    पूर्णाङ्क err;
    पूर्णाङ्क ifd = fileno(in);
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
    len = gzwrite(out, (वर्ण *)buf, (unsigned)buf_len);

    यदि (len != (पूर्णाङ्क)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    यदि (gzclose(out) != Z_OK) error("failed gzclose");
    प्रत्यागम Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

स्थिर शून्य gz_compress(FILE *in, gzFile out) {
    local वर्ण buf[BUFLEN];
    पूर्णाङ्क len;
    पूर्णाङ्क err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    यदि (gz_compress_mmap(in, out) == Z_OK) प्रत्यागम;
#endif
    प्रति (;;) {
        len = (पूर्णाङ्क)fread(buf, 1, परिमाण(buf), in);
        यदि (ferror(in)) {
            perror("fread");
            exit(1);
        }
        यदि (len == 0) भङ्ग;

        यदि (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    यदि (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
स्थिर शून्य gz_uncompress(gzFile in, FILE *out) {
    local वर्ण buf[BUFLEN];
    पूर्णाङ्क len;
    पूर्णाङ्क err;

    प्रति (;;) {
        len = gzread(in, buf, परिमाण(buf));
        यदि (len < 0) error (gzerror(in, &err));
        यदि (len == 0) भङ्ग;

        यदि ((पूर्णाङ्क)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    यदि (fclose(out)) error("failed fclose");

    यदि (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
स्थिर शून्य file_compress(वर्ण *file, वर्ण *mode) {
    local वर्ण outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    यदि (strlen(file) + strlen(GZ_SUFFIX) >= परिमाण(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, परिमाण(outfile));
    string_copy(end, GZ_SUFFIX, परिमाण(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    यदि (in == रिक्त) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    यदि (out == रिक्त) {
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
स्थिर शून्य file_uncompress(वर्ण *file) {
    local वर्ण buf[MAX_NAME_LEN+1];
    वर्ण *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    यदि (len + strlen(GZ_SUFFIX) >= परिमाण(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, परिमाण(buf));

    यदि (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } अन्यथा {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, परिमाण(buf) - len);
    }
    in = gzopen(infile, "rb");
    यदि (in == रिक्त) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    यदि (out == रिक्त) {
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

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[]) {
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

    यदि (!strcmp(bname, "gunzip"))
      uncompr = 1;
    अन्यथा यदि (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    यावत् (argc > 0) {
      यदि (strcmp(*argv, "-c") == 0)
        copyout = 1;
      अन्यथा यदि (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      अन्यथा यदि (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      अन्यथा यदि (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      अन्यथा यदि (strcmp(*argv, "-r") == 0)
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
        SET_द्विआधारी_MODE(stdin);
        SET_द्विआधारी_MODE(stdout);
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
            SET_द्विआधारी_MODE(stdout);
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
                    FILE * in = fopen(*argv, "rb");

                    यदि (in == रिक्त) {
                        perror(*argv);
                    } अन्यथा {
                        file = gzdopen(fileno(stdout), outmode);
                        यदि (file == रिक्त) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } अन्यथा {
                    file_compress(*argv, outmode);
                }
            }
        } यावत् (argv++, --argc);
    }
    प्रत्यागम 0;
}
