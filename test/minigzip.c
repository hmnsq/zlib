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

#എങ്കിൽ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#എങ്കിൽ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#അന്തര്ഭാവ "zlib.h"
#അന്തര്ഭാവ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#എങ്കിൽ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_ദ്വിമാന_MODE(file) setmode(fileno(file), O_ദ്വിമാന)
#അല്ലെങ്കിൽ
#  define SET_ദ്വിമാന_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#എങ്കിൽ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#എങ്കിൽ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#എങ്കിൽ !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  ബാഹ്യ പൂർണ്ണസംഖ്യ unlink(സ്ഥിരം അക്ഷരം *);
#endif
#endif

#എങ്കിൽ defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

സ്ഥിതം അക്ഷരം *strwinerror (error)
     DWORD error;
{
    സ്ഥിതം അക്ഷരം buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ശൂന്യ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ശൂന്യ);
    എങ്കിൽ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        എങ്കിൽ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        എങ്കിൽ (chars > വലുപ്പം (buf) - 1) {
            chars = വലുപ്പം (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    അല്ലെങ്കിൽ {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    തിരികെ buf;
}

സ്ഥിതം ശൂന്യം pwinerror (s)
    സ്ഥിരം അക്ഷരം *s;
{
    എങ്കിൽ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    അല്ലെങ്കിൽ
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (വലുപ്പം(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local സ്ഥിതം
   /* Needed for systems with limitation on stack size. */
#അല്ലെങ്കിൽ
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
സ്ഥിതം അക്ഷരം *string_copy(അക്ഷരം *dst, അക്ഷരം സ്ഥിരം *src, z_size_t len) {
    എങ്കിൽ (len == 0)
        തിരികെ ശൂന്യ;
    ആകുമ്പോൾ (--len) {
        *dst = *src++;
        എങ്കിൽ (*dst == 0)
            തിരികെ dst;
        dst++;
    }
    *dst = 0;
    തിരികെ dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#എങ്കിൽ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

സ്ഥിതം ശൂന്യം *myalloc(ശൂന്യം *q, unsigned n, unsigned m) {
    (ശൂന്യം)q;
    തിരികെ calloc(n, m);
}

സ്ഥിതം ശൂന്യം myfree(ശൂന്യം *q, ശൂന്യം *p) {
    (ശൂന്യം)q;
    സ്വതന്ത്രം(p);
}

തരനിർവചനം ഘടന gzFile_s {
    FILE *file;
    പൂർണ്ണസംഖ്യ ലിഖ;
    പൂർണ്ണസംഖ്യ err;
    അക്ഷരം *സന്ദേശം;
    z_stream strm;
} *gzFile;

സ്ഥിതം gzFile gz_open(സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ fd, സ്ഥിരം അക്ഷരം *mode) {
    gzFile gz;
    പൂർണ്ണസംഖ്യ ret;

    gz = malloc(വലുപ്പം(ഘടന gzFile_s));
    എങ്കിൽ (gz == ശൂന്യ)
        തിരികെ ശൂന്യ;
    gz->ലിഖ = strchr(mode, 'w') != ശൂന്യ;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    എങ്കിൽ (gz->ലിഖ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    അല്ലെങ്കിൽ {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    എങ്കിൽ (ret != Z_OK) {
        സ്വതന്ത്രം(gz);
        തിരികെ ശൂന്യ;
    }
    gz->file = path == ശൂന്യ ? fdopen(fd, gz->ലിഖ ? "wb" : "rb") :
                              fopen(path, gz->ലിഖ ? "wb" : "rb");
    എങ്കിൽ (gz->file == ശൂന്യ) {
        gz->ലിഖ ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        സ്വതന്ത്രം(gz);
        തിരികെ ശൂന്യ;
    }
    gz->err = 0;
    gz->സന്ദേശം = "";
    തിരികെ gz;
}

സ്ഥിതം gzFile gzopen(സ്ഥിരം അക്ഷരം *path, സ്ഥിരം അക്ഷരം *mode) {
    തിരികെ gz_open(path, -1, mode);
}

സ്ഥിതം gzFile gzdopen(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം അക്ഷരം *mode) {
    തിരികെ gz_open(ശൂന്യ, fd, mode);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ gzwrite(gzFile gz, സ്ഥിരം ശൂന്യം *buf, unsigned len) {
    z_stream *strm;
    unsigned അക്ഷരം out[BUFLEN];

    എങ്കിൽ (gz == ശൂന്യ || !gz->ലിഖ)
        തിരികെ 0;
    strm = &(gz->strm);
    strm->next_in = (ശൂന്യം *)buf;
    strm->avail_in = len;
    ചെയ്യുക {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (ശൂന്യം)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } ആകുമ്പോൾ (strm->avail_out == 0);
    തിരികെ (പൂർണ്ണസംഖ്യ)len;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ gzread(gzFile gz, ശൂന്യം *buf, unsigned len) {
    പൂർണ്ണസംഖ്യ ret;
    unsigned got;
    unsigned അക്ഷരം in[1];
    z_stream *strm;

    എങ്കിൽ (gz == ശൂന്യ || gz->ലിഖ)
        തിരികെ 0;
    എങ്കിൽ (gz->err)
        തിരികെ 0;
    strm = &(gz->strm);
    strm->next_out = (ശൂന്യം *)buf;
    strm->avail_out = len;
    ചെയ്യുക {
        got = (unsigned)fread(in, 1, 1, gz->file);
        എങ്കിൽ (got == 0)
            നിർത്തുക;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        എങ്കിൽ (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->സന്ദേശം = strm->സന്ദേശം;
            തിരികെ 0;
        }
        എങ്കിൽ (ret == Z_STREAM_END)
            inflateReset(strm);
    } ആകുമ്പോൾ (strm->avail_out);
    തിരികെ (പൂർണ്ണസംഖ്യ)(len - strm->avail_out);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ gzclose(gzFile gz) {
    z_stream *strm;
    unsigned അക്ഷരം out[BUFLEN];

    എങ്കിൽ (gz == ശൂന്യ)
        തിരികെ Z_STREAM_ERROR;
    strm = &(gz->strm);
    എങ്കിൽ (gz->ലിഖ) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        ചെയ്യുക {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (ശൂന്യം)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } ആകുമ്പോൾ (strm->avail_out == 0);
        deflateEnd(strm);
    }
    അല്ലെങ്കിൽ
        inflateEnd(strm);
    fclose(gz->file);
    സ്വതന്ത്രം(gz);
    തിരികെ Z_OK;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *gzerror(gzFile gz, പൂർണ്ണസംഖ്യ *err) {
    *err = gz->err;
    തിരികെ gz->സന്ദേശം;
}

#endif

സ്ഥിതം അക്ഷരം *prog;

/* ===========================================================================
 * Display error message and exit
 */
സ്ഥിതം ശൂന്യം error(സ്ഥിരം അക്ഷരം *സന്ദേശം) {
    fprintf(stderr, "%s: %s\n", prog, സന്ദേശം);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ gz_compress_mmap(FILE *in, gzFile out) {
    പൂർണ്ണസംഖ്യ len;
    പൂർണ്ണസംഖ്യ err;
    പൂർണ്ണസംഖ്യ ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    ഘടന stat sb;

    /* Determine the size of the file, needed for mmap: */
    എങ്കിൽ (fstat(ifd, &sb) < 0) തിരികെ Z_ERRNO;
    buf_len = sb.st_size;
    എങ്കിൽ (buf_len <= 0) തിരികെ Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    എങ്കിൽ (buf == (caddr_t)(-1)) തിരികെ Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (അക്ഷരം *)buf, (unsigned)buf_len);

    എങ്കിൽ (len != (പൂർണ്ണസംഖ്യ)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    എങ്കിൽ (gzclose(out) != Z_OK) error("failed gzclose");
    തിരികെ Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

സ്ഥിതം ശൂന്യം gz_compress(FILE *in, gzFile out) {
    local അക്ഷരം buf[BUFLEN];
    പൂർണ്ണസംഖ്യ len;
    പൂർണ്ണസംഖ്യ err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    എങ്കിൽ (gz_compress_mmap(in, out) == Z_OK) തിരികെ;
#endif
    വേണ്ടി (;;) {
        len = (പൂർണ്ണസംഖ്യ)fread(buf, 1, വലുപ്പം(buf), in);
        എങ്കിൽ (ferror(in)) {
            perror("fread");
            exit(1);
        }
        എങ്കിൽ (len == 0) നിർത്തുക;

        എങ്കിൽ (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    എങ്കിൽ (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
സ്ഥിതം ശൂന്യം gz_uncompress(gzFile in, FILE *out) {
    local അക്ഷരം buf[BUFLEN];
    പൂർണ്ണസംഖ്യ len;
    പൂർണ്ണസംഖ്യ err;

    വേണ്ടി (;;) {
        len = gzread(in, buf, വലുപ്പം(buf));
        എങ്കിൽ (len < 0) error (gzerror(in, &err));
        എങ്കിൽ (len == 0) നിർത്തുക;

        എങ്കിൽ ((പൂർണ്ണസംഖ്യ)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    എങ്കിൽ (fclose(out)) error("failed fclose");

    എങ്കിൽ (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
സ്ഥിതം ശൂന്യം file_compress(അക്ഷരം *file, അക്ഷരം *mode) {
    local അക്ഷരം outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    എങ്കിൽ (strlen(file) + strlen(GZ_SUFFIX) >= വലുപ്പം(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, വലുപ്പം(outfile));
    string_copy(end, GZ_SUFFIX, വലുപ്പം(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    എങ്കിൽ (in == ശൂന്യ) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    എങ്കിൽ (out == ശൂന്യ) {
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
സ്ഥിതം ശൂന്യം file_uncompress(അക്ഷരം *file) {
    local അക്ഷരം buf[MAX_NAME_LEN+1];
    അക്ഷരം *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    എങ്കിൽ (len + strlen(GZ_SUFFIX) >= വലുപ്പം(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, വലുപ്പം(buf));

    എങ്കിൽ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } അല്ലെങ്കിൽ {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, വലുപ്പം(buf) - len);
    }
    in = gzopen(infile, "rb");
    എങ്കിൽ (in == ശൂന്യ) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    എങ്കിൽ (out == ശൂന്യ) {
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

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[]) {
    പൂർണ്ണസംഖ്യ copyout = 0;
    പൂർണ്ണസംഖ്യ uncompr = 0;
    gzFile file;
    അക്ഷരം *bname, outmode[5];

    string_copy(outmode, "wb6 ", വലുപ്പം(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    എങ്കിൽ (bname)
      bname++;
    അല്ലെങ്കിൽ
      bname = argv[0];
    argc--, argv++;

    എങ്കിൽ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    അല്ലെങ്കിൽ എങ്കിൽ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    ആകുമ്പോൾ (argc > 0) {
      എങ്കിൽ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      അല്ലെങ്കിൽ എങ്കിൽ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      അല്ലെങ്കിൽ എങ്കിൽ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      അല്ലെങ്കിൽ എങ്കിൽ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      അല്ലെങ്കിൽ എങ്കിൽ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      അല്ലെങ്കിൽ എങ്കിൽ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      അല്ലെങ്കിൽ
        നിർത്തുക;
      argc--, argv++;
    }
    എങ്കിൽ (outmode[3] == ' ')
        outmode[3] = 0;
    എങ്കിൽ (argc == 0) {
        SET_ദ്വിമാന_MODE(stdin);
        SET_ദ്വിമാന_MODE(stdout);
        എങ്കിൽ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            എങ്കിൽ (file == ശൂന്യ) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } അല്ലെങ്കിൽ {
            file = gzdopen(fileno(stdout), outmode);
            എങ്കിൽ (file == ശൂന്യ) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } അല്ലെങ്കിൽ {
        എങ്കിൽ (copyout) {
            SET_ദ്വിമാന_MODE(stdout);
        }
        ചെയ്യുക {
            എങ്കിൽ (uncompr) {
                എങ്കിൽ (copyout) {
                    file = gzopen(*argv, "rb");
                    എങ്കിൽ (file == ശൂന്യ)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    അല്ലെങ്കിൽ
                        gz_uncompress(file, stdout);
                } അല്ലെങ്കിൽ {
                    file_uncompress(*argv);
                }
            } അല്ലെങ്കിൽ {
                എങ്കിൽ (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    എങ്കിൽ (in == ശൂന്യ) {
                        perror(*argv);
                    } അല്ലെങ്കിൽ {
                        file = gzdopen(fileno(stdout), outmode);
                        എങ്കിൽ (file == ശൂന്യ) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } അല്ലെങ്കിൽ {
                    file_compress(*argv, outmode);
                }
            }
        } ആകുമ്പോൾ (argv++, --argc);
    }
    തിരികെ 0;
}
