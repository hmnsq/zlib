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

#எனில் defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#எனில் defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#அந்தர்பாவ "zlib.h"
#அந்தர்பாவ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#எனில் defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_இருமம்_MODE(file) setmode(fileno(file), O_இருமம்)
#இல்லையெனில்
#  define SET_இருமம்_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#எனில் defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#endif
#எனில் defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#எனில் !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  வெளி முழு unlink(மாறிலி எழுத்து *);
#endif
#endif

#எனில் defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

நிலையான எழுத்து *strwinerror (error)
     DWORD error;
{
    நிலையான எழுத்து buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        வெற்று,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        வெற்று);
    எனில் (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        எனில் (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        எனில் (chars > அளவு (buf) - 1) {
            chars = அளவு (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    இல்லையெனில் {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    திரும்பு buf;
}

நிலையான வெறுமை pwinerror (s)
    மாறிலி எழுத்து *s;
{
    எனில் (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(GetLastError ()));
    இல்லையெனில்
        fprintf(stderr, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (அளவு(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local நிலையான
   /* Needed for systems with limitation on stack size. */
#இல்லையெனில்
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
நிலையான எழுத்து *string_copy(எழுத்து *dst, எழுத்து மாறிலி *src, z_size_t len) {
    எனில் (len == 0)
        திரும்பு வெற்று;
    போது (--len) {
        *dst = *src++;
        எனில் (*dst == 0)
            திரும்பு dst;
        dst++;
    }
    *dst = 0;
    திரும்பு dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#எனில் defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

நிலையான வெறுமை *myalloc(வெறுமை *q, unsigned n, unsigned m) {
    (வெறுமை)q;
    திரும்பு calloc(n, m);
}

நிலையான வெறுமை myfree(வெறுமை *q, வெறுமை *p) {
    (வெறுமை)q;
    விடுவி(p);
}

வகைவரை கட்டமைப்பு gzFile_s {
    FILE *file;
    முழு லிக;
    முழு err;
    எழுத்து *செய்தி;
    z_stream strm;
} *gzFile;

நிலையான gzFile gz_open(மாறிலி எழுத்து *path, முழு fd, மாறிலி எழுத்து *mode) {
    gzFile gz;
    முழு ret;

    gz = malloc(அளவு(கட்டமைப்பு gzFile_s));
    எனில் (gz == வெற்று)
        திரும்பு வெற்று;
    gz->லிக = strchr(mode, 'w') != வெற்று;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    எனில் (gz->லிக)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    இல்லையெனில் {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    எனில் (ret != Z_OK) {
        விடுவி(gz);
        திரும்பு வெற்று;
    }
    gz->file = path == வெற்று ? fdopen(fd, gz->லிக ? "wb" : "rb") :
                              fopen(path, gz->லிக ? "wb" : "rb");
    எனில் (gz->file == வெற்று) {
        gz->லிக ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        விடுவி(gz);
        திரும்பு வெற்று;
    }
    gz->err = 0;
    gz->செய்தி = "";
    திரும்பு gz;
}

நிலையான gzFile gzopen(மாறிலி எழுத்து *path, மாறிலி எழுத்து *mode) {
    திரும்பு gz_open(path, -1, mode);
}

நிலையான gzFile gzdopen(முழு fd, மாறிலி எழுத்து *mode) {
    திரும்பு gz_open(வெற்று, fd, mode);
}

நிலையான முழு gzwrite(gzFile gz, மாறிலி வெறுமை *buf, unsigned len) {
    z_stream *strm;
    unsigned எழுத்து out[BUFLEN];

    எனில் (gz == வெற்று || !gz->லிக)
        திரும்பு 0;
    strm = &(gz->strm);
    strm->next_in = (வெறுமை *)buf;
    strm->avail_in = len;
    செய் {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (வெறுமை)deflate(strm, Z_NO_FLUSH);
        fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
    } போது (strm->avail_out == 0);
    திரும்பு (முழு)len;
}

நிலையான முழு gzread(gzFile gz, வெறுமை *buf, unsigned len) {
    முழு ret;
    unsigned got;
    unsigned எழுத்து in[1];
    z_stream *strm;

    எனில் (gz == வெற்று || gz->லிக)
        திரும்பு 0;
    எனில் (gz->err)
        திரும்பு 0;
    strm = &(gz->strm);
    strm->next_out = (வெறுமை *)buf;
    strm->avail_out = len;
    செய் {
        got = (unsigned)fread(in, 1, 1, gz->file);
        எனில் (got == 0)
            நிறுத்து;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        எனில் (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->செய்தி = strm->செய்தி;
            திரும்பு 0;
        }
        எனில் (ret == Z_STREAM_END)
            inflateReset(strm);
    } போது (strm->avail_out);
    திரும்பு (முழு)(len - strm->avail_out);
}

நிலையான முழு gzclose(gzFile gz) {
    z_stream *strm;
    unsigned எழுத்து out[BUFLEN];

    எனில் (gz == வெற்று)
        திரும்பு Z_STREAM_ERROR;
    strm = &(gz->strm);
    எனில் (gz->லிக) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        செய் {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (வெறுமை)deflate(strm, Z_FINISH);
            fwrite(out, 1, BUFLEN - strm->avail_out, gz->file);
        } போது (strm->avail_out == 0);
        deflateEnd(strm);
    }
    இல்லையெனில்
        inflateEnd(strm);
    fclose(gz->file);
    விடுவி(gz);
    திரும்பு Z_OK;
}

நிலையான மாறிலி எழுத்து *gzerror(gzFile gz, முழு *err) {
    *err = gz->err;
    திரும்பு gz->செய்தி;
}

#endif

நிலையான எழுத்து *prog;

/* ===========================================================================
 * Display error message and exit
 */
நிலையான வெறுமை error(மாறிலி எழுத்து *செய்தி) {
    fprintf(stderr, "%s: %s\n", prog, செய்தி);
    exit(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
நிலையான முழு gz_compress_mmap(FILE *in, gzFile out) {
    முழு len;
    முழு err;
    முழு ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    கட்டமைப்பு stat sb;

    /* Determine the size of the file, needed for mmap: */
    எனில் (fstat(ifd, &sb) < 0) திரும்பு Z_ERRNO;
    buf_len = sb.st_size;
    எனில் (buf_len <= 0) திரும்பு Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    எனில் (buf == (caddr_t)(-1)) திரும்பு Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (எழுத்து *)buf, (unsigned)buf_len);

    எனில் (len != (முழு)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    எனில் (gzclose(out) != Z_OK) error("failed gzclose");
    திரும்பு Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

நிலையான வெறுமை gz_compress(FILE *in, gzFile out) {
    local எழுத்து buf[BUFLEN];
    முழு len;
    முழு err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    எனில் (gz_compress_mmap(in, out) == Z_OK) திரும்பு;
#endif
    க்கு (;;) {
        len = (முழு)fread(buf, 1, அளவு(buf), in);
        எனில் (ferror(in)) {
            perror("fread");
            exit(1);
        }
        எனில் (len == 0) நிறுத்து;

        எனில் (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    எனில் (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
நிலையான வெறுமை gz_uncompress(gzFile in, FILE *out) {
    local எழுத்து buf[BUFLEN];
    முழு len;
    முழு err;

    க்கு (;;) {
        len = gzread(in, buf, அளவு(buf));
        எனில் (len < 0) error (gzerror(in, &err));
        எனில் (len == 0) நிறுத்து;

        எனில் ((முழு)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    எனில் (fclose(out)) error("failed fclose");

    எனில் (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
நிலையான வெறுமை file_compress(எழுத்து *file, எழுத்து *mode) {
    local எழுத்து outfile[MAX_NAME_LEN+1], *end;
    FILE  *in;
    gzFile out;

    எனில் (strlen(file) + strlen(GZ_SUFFIX) >= அளவு(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    end = string_copy(outfile, file, அளவு(outfile));
    string_copy(end, GZ_SUFFIX, அளவு(outfile) - (z_size_t)(end - outfile));

    in = fopen(file, "rb");
    எனில் (in == வெற்று) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    எனில் (out == வெற்று) {
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
நிலையான வெறுமை file_uncompress(எழுத்து *file) {
    local எழுத்து buf[MAX_NAME_LEN+1];
    எழுத்து *infile, *outfile;
    FILE  *out;
    gzFile in;
    z_size_t len = strlen(file);

    எனில் (len + strlen(GZ_SUFFIX) >= அளவு(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        exit(1);
    }

    string_copy(buf, file, அளவு(buf));

    எனில் (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } இல்லையெனில் {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, அளவு(buf) - len);
    }
    in = gzopen(infile, "rb");
    எனில் (in == வெற்று) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    எனில் (out == வெற்று) {
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

முழு முதன்மை(முழு argc, எழுத்து *argv[]) {
    முழு copyout = 0;
    முழு uncompr = 0;
    gzFile file;
    எழுத்து *bname, outmode[5];

    string_copy(outmode, "wb6 ", அளவு(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    எனில் (bname)
      bname++;
    இல்லையெனில்
      bname = argv[0];
    argc--, argv++;

    எனில் (!strcmp(bname, "gunzip"))
      uncompr = 1;
    இல்லையெனில் எனில் (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    போது (argc > 0) {
      எனில் (strcmp(*argv, "-c") == 0)
        copyout = 1;
      இல்லையெனில் எனில் (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      இல்லையெனில் எனில் (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      இல்லையெனில் எனில் (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      இல்லையெனில் எனில் (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      இல்லையெனில் எனில் ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      இல்லையெனில்
        நிறுத்து;
      argc--, argv++;
    }
    எனில் (outmode[3] == ' ')
        outmode[3] = 0;
    எனில் (argc == 0) {
        SET_இருமம்_MODE(stdin);
        SET_இருமம்_MODE(stdout);
        எனில் (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            எனில் (file == வெற்று) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } இல்லையெனில் {
            file = gzdopen(fileno(stdout), outmode);
            எனில் (file == வெற்று) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } இல்லையெனில் {
        எனில் (copyout) {
            SET_இருமம்_MODE(stdout);
        }
        செய் {
            எனில் (uncompr) {
                எனில் (copyout) {
                    file = gzopen(*argv, "rb");
                    எனில் (file == வெற்று)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    இல்லையெனில்
                        gz_uncompress(file, stdout);
                } இல்லையெனில் {
                    file_uncompress(*argv);
                }
            } இல்லையெனில் {
                எனில் (copyout) {
                    FILE * in = fopen(*argv, "rb");

                    எனில் (in == வெற்று) {
                        perror(*argv);
                    } இல்லையெனில் {
                        file = gzdopen(fileno(stdout), outmode);
                        எனில் (file == வெற்று) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } இல்லையெனில் {
                    file_compress(*argv, outmode);
                }
            }
        } போது (argv++, --argc);
    }
    திரும்பு 0;
}
