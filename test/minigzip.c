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

#જો defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#જો defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#અન્તર્ભાવ "zlib.h"
#અન્તર્ભાવ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#જો defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_દ્વિઅંકી_MODE(file) setmode(fileno(file), O_દ્વિઅંકી)
#નહીંતર
#  define SET_દ્વિઅંકી_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#જો defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#જો defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#જો !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  બાહ્ય પૂર્ણાંક unlink(અચલ અક્ષર *);
#endif
#endif

#જો defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

સ્થિર અક્ષર *strwinerror (error)
     DWORD error;
{
    સ્થિર અક્ષર buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ખાલી,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ખાલી);
    જો (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        જો (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        જો (chars > કદ (buf) - 1) {
            chars = કદ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    નહીંતર {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    પાછા buf;
}

સ્થિર શૂન્ય pwinerror (s)
    અચલ અક્ષર *s;
{
    જો (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    નહીંતર
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (કદ(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local સ્થિર
   /* Needed for systems with limitation on stack size. */
#નહીંતર
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
સ્થિર અક્ષર *string_copy(અક્ષર *dst, અક્ષર અચલ *src, z_size_t len) {
    જો (len == 0)
        પાછા ખાલી;
    જ્યારે (--len) {
        *dst = *src++;
        જો (*dst == 0)
            પાછા dst;
        dst++;
    }
    *dst = 0;
    પાછા dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#જો defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

સ્થિર શૂન્ય *myalloc(શૂન્ય *q, unsigned n, unsigned m) {
    (શૂન્ય)q;
    પાછા calloc(n, m);
}

સ્થિર શૂન્ય myfree(શૂન્ય *q, શૂન્ય *p) {
    (શૂન્ય)q;
    મુક્ત(p);
}

પ્રકારવ્યાખ્યા સંરચના gzFile_s {
    FILE *file;
    પૂર્ણાંક લિખ;
    પૂર્ણાંક err;
    અક્ષર *સંદેશ;
    z_stream strm;
} *gzFile;

સ્થિર gzFile gz_open(અચલ અક્ષર *path, પૂર્ણાંક fd, અચલ અક્ષર *mode) {
    gzFile gz;
    પૂર્ણાંક ret;

    gz = malloc(કદ(સંરચના gzFile_s));
    જો (gz == ખાલી)
        પાછા ખાલી;
    gz->લિખ = strchr(mode, 'w') != ખાલી;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    જો (gz->લિખ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    નહીંતર {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    જો (ret != Z_OK) {
        મુક્ત(gz);
        પાછા ખાલી;
    }
    gz->file = path == ખાલી ? fdopen(fd, gz->લિખ ? "wb" : "rb") :
                              fopen(path, gz->લિખ ? "wb" : "rb");
    જો (gz->file == ખાલી) {
        gz->લિખ ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        મુક્ત(gz);
        પાછા ખાલી;
    }
    gz->err = 0;
    gz->સંદેશ = "";
    પાછા gz;
}

સ્થિર gzFile gzopen(અચલ અક્ષર *path, અચલ અક્ષર *mode) {
    પાછા gz_open(path, -1, mode);
}

સ્થિર gzFile gzdopen(પૂર્ણાંક fd, અચલ અક્ષર *mode) {
    પાછા gz_open(ખાલી, fd, mode);
}

સ્થિર પૂર્ણાંક gzwrite(gzFile gz, અચલ શૂન્ય *buf, unsigned len) {
    z_stream *strm;
    unsigned અક્ષર out[BUFLEN];

    જો (gz == ખાલી || !gz->લિખ)
        પાછા 0;
    strm = &(gz->strm);
    strm->next_in = (શૂન્ય *)buf;
    strm->avail_in = len;
    કરો {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (શૂન્ય)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } જ્યારે (strm->avail_out == 0);
    પાછા (પૂર્ણાંક)len;
}

સ્થિર પૂર્ણાંક gzread(gzFile gz, શૂન્ય *buf, unsigned len) {
    પૂર્ણાંક ret;
    unsigned got;
    unsigned અક્ષર in[1];
    z_stream *strm;

    જો (gz == ખાલી || gz->લિખ)
        પાછા 0;
    જો (gz->err)
        પાછા 0;
    strm = &(gz->strm);
    strm->next_out = (શૂન્ય *)buf;
    strm->avail_out = len;
    કરો {
        got = (unsigned)fread(in, 1, 1, gz->file);
        જો (got == 0)
            તોડો;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        જો (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->સંદેશ = strm->સંદેશ;
            પાછા 0;
        }
        જો (ret == Z_STREAM_END)
            inflateReset(strm);
    } જ્યારે (strm->avail_out);
    પાછા (પૂર્ણાંક)(len - strm->avail_out);
}

સ્થિર પૂર્ણાંક gzclose(gzFile gz) {
    z_stream *strm;
    unsigned અક્ષર out[BUFLEN];

    જો (gz == ખાલી)
        પાછા Z_STREAM_ERROR;
    strm = &(gz->strm);
    જો (gz->લિખ) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        કરો {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (શૂન્ય)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } જ્યારે (strm->avail_out == 0);
        deflateEnd(strm);
    }
    નહીંતર
        inflateEnd(strm);
    fclose(gz->file);
    મુક્ત(gz);
    પાછા Z_OK;
}

સ્થિર અચલ અક્ષર *gzerror(gzFile gz, પૂર્ણાંક *err) {
    *err = gz->err;
    પાછા gz->સંદેશ;
}

#endif

સ્થિર અક્ષર *prog;

/* ===========================================================================
 * Display error message and exit
 */
સ્થિર શૂન્ય error(અચલ અક્ષર *સંદેશ) {
    fprintf(stderr, "%s: %s\n", prog, સંદેશ);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
સ્થિર પૂર્ણાંક gz_compress_mmap(FILE *in, gzFile out) {
    પૂર્ણાંક len;
    પૂર્ણાંક err;
    પૂર્ણાંક ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    સંરચના stat sb;

    /* Determine the size of the file, needed for mmap: */
    જો (fstat(ifd, &sb) < 0) પાછા Z_ERRNO;
    buf_len = sb.st_size;
    જો (buf_len <= 0) પાછા Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    જો (buf == (caddr_t)(-1)) પાછા Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (અક્ષર *)buf, (unsigned)buf_len);

    જો (len != (પૂર્ણાંક)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    જો (gzclose(out) != Z_OK) error("failed gzclose");
    પાછા Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

સ્થિર શૂન્ય gz_compress(FILE *in, gzFile out) {
    local અક્ષર buf[BUFLEN];
    પૂર્ણાંક len;
    પૂર્ણાંક err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    જો (gz_compress_mmap(in, out) == Z_OK) પાછા;
#endif
    માટે (;;) {
        len = (પૂર્ણાંક)fread(buf, 1, કદ(buf), in);
        જો (ferror(in)) {
            perror("fread");
            exit(1);
        }
        જો (len == 0) તોડો;

        જો (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    જો (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
સ્થિર શૂન્ય gz_uncompress(gzFile in, FILE *out) {
    local અક્ષર buf[BUFLEN];
    પૂર્ણાંક len;
    પૂર્ણાંક err;

    માટે (;;) {
        len = gzread(in, buf, કદ(buf));
        જો (len < 0) error (gzerror(in, &err));
        જો (len == 0) તોડો;

        જો ((પૂર્ણાંક)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    જો (fclose(out)) error("failed fclose");

    જો (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
સ્થિર શૂન્ય file_compress(અક્ષર *file, અક્ષર *mode) {
    local અક્ષર outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    જો (strlen(file) + strlen(GZ_SUFFIX) >= કદ(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, કદ(outfile));
    string_copy(end, GZ_SUFFIX, કદ(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    જો (in == ખાલી) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    જો (out == ખાલી) {
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
સ્થિર શૂન્ય file_uncompress(અક્ષર *file) {
    local અક્ષર buf[MAX_NAME_LEN+1];
    અક્ષર *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    જો (len + strlen(GZ_SUFFIX) >= કદ(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, કદ(buf));

    જો (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } નહીંતર {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, કદ(buf) - len);
    }
    in = gzopen(infile, "rb");
    જો (in == ખાલી) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    જો (out == ખાલી) {
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

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[]) {
    પૂર્ણાંક copyout = 0;
    પૂર્ણાંક uncompr = 0;
    gzFile file;
    અક્ષર *bname, outmode[5];

    string_copy(outmode, "wb6 ", કદ(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    જો (bname)
      bname++;
    નહીંતર
      bname = argv[0];
    argc--, argv++;

    જો (!strcmp(bname, "gunzip"))
      uncompr = 1;
    નહીંતર જો (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    જ્યારે (argc > 0) {
      જો (strcmp(*argv, "-c") == 0)
        copyout = 1;
      નહીંતર જો (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      નહીંતર જો (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      નહીંતર જો (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      નહીંતર જો (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      નહીંતર જો ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      નહીંતર
        તોડો;
      argc--, argv++;
    }
    જો (outmode[3] == ' ')
        outmode[3] = 0;
    જો (argc == 0) {
        SET_દ્વિઅંકી_MODE(stdin);
        SET_દ્વિઅંકી_MODE(stdout);
        જો (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            જો (file == ખાલી) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } નહીંતર {
            file = gzdopen(fileno(stdout), outmode);
            જો (file == ખાલી) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } નહીંતર {
        જો (copyout) {
            SET_દ્વિઅંકી_MODE(stdout);
        }
        કરો {
            જો (uncompr) {
                જો (copyout) {
                    file = gzopen(*argv, "rb");
                    જો (file == ખાલી)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    નહીંતર
                        gz_uncompress(file, stdout);
                } નહીંતર {
                    file_uncompress(*argv);
                }
            } નહીંતર {
                જો (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    જો (in == ખાલી) {
                        perror(*argv);
                    } નહીંતર {
                        file = gzdopen(fileno(stdout), outmode);
                        જો (file == ખાલી) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } નહીંતર {
                    file_compress(*argv, outmode);
                }
            }
        } જ્યારે (argv++, --argc);
    }
    પાછા 0;
}
