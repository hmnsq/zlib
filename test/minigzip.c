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

#నిర్వచితం_కాకపోతే _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#అయితే_ముగింపు

#ఒకవేళ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#అయితే_ముగింపు
#ఒకవేళ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#అయితే_ముగింపు

#అన్తర్భావ "zlib.h"
#అన్తర్భావ <stdio.h>

#నిర్వచితమైతే STDC
#  include <string.h>
#  include <stdlib.h>
#అయితే_ముగింపు

#నిర్వచితమైతే USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#అయితే_ముగింపు

#ఒకవేళ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define సమితి_ద్విమాన_బహుళకం(file) setmode(fileno(file), O_ద్విమాన)
#లేకపోతే
#  define సమితి_ద్విమాన_బహుళకం(file)
#అయితే_ముగింపు

#నిర్వచితమైతే VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#అయితే_ముగింపు
#ఒకవేళ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink remove
#    define fileno(file) file->__file
#  endif
#అయితే_ముగింపు
#ఒకవేళ defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#అయితే_ముగింపు

#ఒకవేళ !defined(Z_HAVE_UNISTD_H) && !defined(LARGEFILE64_ఆధారం)
#నిర్వచితం_కాకపోతే WIN32 /* unlink already in stdio.h for WIN32 */
  బాహ్య పూర్ణాంకం unlink(స్థిరరాశి అక్షరం *);
#అయితే_ముగింపు
#అయితే_ముగింపు

#ఒకవేళ defined(UNDER_CE)
#  include <windows.h>
#  define perror(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

స్థిర అక్షరం *strwinerror (error)
     DWORD error;
{
    స్థిర అక్షరం buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_చివరి_Error();
    DWORD chars = Format_సందేశం(FORMAT_సందేశం_FROM_SYSTEM
        | FORMAT_సందేశం_ALLOCATE_బఫరు,
        ఖాళీ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ఖాళీ);
    ఒకవేళ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ఒకవేళ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ఒకవేళ (chars > పరిమాణం (buf) - 1) {
            chars = పరిమాణం (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    లేకపోతే {
        sprintf(buf, "unknown win32 error (%lu)", error);
    }

    సమితి_చివరి_Error(lasterr);
    తిరిగి buf;
}

స్థిర శూన్య pwinerror (s)
    స్థిరరాశి అక్షరం *s;
{
    ఒకవేళ (s && *s)
        fprintf(stderr, "%s: %s\n", s, strwinerror(Get_చివరి_Error ()));
    లేకపోతే
        fprintf(stderr, "%s\n", strwinerror(Get_చివరి_Error ()));
}

#అయితే_ముగింపు /* UNDER_CE */

#నిర్వచితం_కాకపోతే GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#అయితే_ముగింపు
#నిర్వచించు SUFFIX_LEN (పరిమాణం(GZ_SUFFIX)-1)

#నిర్వచించు BUFLEN      16384
#నిర్వచించు MAX_NAME_LEN 1024

#నిర్వచితమైతే MAXSEG_64K
#  define local స్థిర
   /* Needed for systems with limitation on stack size. */
#లేకపోతే
#  define local
#అయితే_ముగింపు

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
స్థిర అక్షరం *string_copy(అక్షరం *dst, అక్షరం స్థిరరాశి *src, z_కొలత_t len) {
    ఒకవేళ (len == 0)
        తిరిగి ఖాళీ;
    అయితే (--len) {
        *dst = *src++;
        ఒకవేళ (*dst == 0)
            తిరిగి dst;
        dst++;
    }
    *dst = 0;
    తిరిగి dst;
}

#నిర్వచితమైతే Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#ఒకవేళ defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#అయితే_ముగింపు

స్థిర శూన్య *myalloc(శూన్య *q, గుర్తులేని n, గుర్తులేని m) {
    (శూన్య)q;
    తిరిగి సున్నా_జ్ఞాపకం_ఇవ్వు(n, m);
}

స్థిర శూన్య myfree(శూన్య *q, శూన్య *p) {
    (శూన్య)q;
    విడుదల(p);
}

రకనిర్వచనం నిర్మాణం gzFile_s {
    FILE *file;
    పూర్ణాంకం లిఖ;
    పూర్ణాంకం err;
    అక్షరం *సందేశం;
    z_ప్రవాహం strm;
} *gzFile;

స్థిర gzFile gz_open(స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం fd, స్థిరరాశి అక్షరం *బహుళకం) {
    gzFile gz;
    పూర్ణాంకం ret;

    gz = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం gzFile_s));
    ఒకవేళ (gz == ఖాళీ)
        తిరిగి ఖాళీ;
    gz->లిఖ = strchr(బహుళకం, 'w') != ఖాళీ;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    ఒకవేళ (gz->లిఖ)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    లేకపోతే {
        gz->strm.తదుపరి_లోపల = 0;
        gz->strm.avail_లోపల = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    ఒకవేళ (ret != Z_OK) {
        విడుదల(gz);
        తిరిగి ఖాళీ;
    }
    gz->file = మార్గం == ఖాళీ ? fdopen(fd, gz->లిఖ ? "wb" : "rb") :
                              fopen(మార్గం, gz->లిఖ ? "wb" : "rb");
    ఒకవేళ (gz->file == ఖాళీ) {
        gz->లిఖ ? deflate_చివర(&(gz->strm)) : inflate_చివర(&(gz->strm));
        విడుదల(gz);
        తిరిగి ఖాళీ;
    }
    gz->err = 0;
    gz->సందేశం = "";
    తిరిగి gz;
}

స్థిర gzFile gzopen(స్థిరరాశి అక్షరం *మార్గం, స్థిరరాశి అక్షరం *బహుళకం) {
    తిరిగి gz_open(మార్గం, -1, బహుళకం);
}

స్థిర gzFile gzdopen(పూర్ణాంకం fd, స్థిరరాశి అక్షరం *బహుళకం) {
    తిరిగి gz_open(ఖాళీ, fd, బహుళకం);
}

స్థిర పూర్ణాంకం gzwrite(gzFile gz, స్థిరరాశి శూన్య *buf, గుర్తులేని len) {
    z_ప్రవాహం *strm;
    గుర్తులేని అక్షరం బహిర్గతం[BUFLEN];

    ఒకవేళ (gz == ఖాళీ || !gz->లిఖ)
        తిరిగి 0;
    strm = &(gz->strm);
    strm->తదుపరి_లోపల = (శూన్య *)buf;
    strm->avail_లోపల = len;
    చేయి {
        strm->తదుపరి_బహిర్గతం = బహిర్గతం;
        strm->avail_బహిర్గతం = BUFLEN;
        (శూన్య)deflate(strm, Z_NO_FLUSH);
        fwrite(బహిర్గతం, 1, BUFLEN - strm->avail_బహిర్గతం, gz->file);
    } అయితే (strm->avail_బహిర్గతం == 0);
    తిరిగి (పూర్ణాంకం)len;
}

స్థిర పూర్ణాంకం gzread(gzFile gz, శూన్య *buf, గుర్తులేని len) {
    పూర్ణాంకం ret;
    గుర్తులేని got;
    గుర్తులేని అక్షరం లోపల[1];
    z_ప్రవాహం *strm;

    ఒకవేళ (gz == ఖాళీ || gz->లిఖ)
        తిరిగి 0;
    ఒకవేళ (gz->err)
        తిరిగి 0;
    strm = &(gz->strm);
    strm->తదుపరి_బహిర్గతం = (శూన్య *)buf;
    strm->avail_బహిర్గతం = len;
    చేయి {
        got = (గుర్తులేని)fread(లోపల, 1, 1, gz->file);
        ఒకవేళ (got == 0)
            ఆపు;
        strm->తదుపరి_లోపల = లోపల;
        strm->avail_లోపల = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        ఒకవేళ (ret == Z_దత్తాంశం_ERROR) {
            gz->err = Z_దత్తాంశం_ERROR;
            gz->సందేశం = strm->సందేశం;
            తిరిగి 0;
        }
        ఒకవేళ (ret == Z_ప్రవాహం_చివర)
            inflateReset(strm);
    } అయితే (strm->avail_బహిర్గతం);
    తిరిగి (పూర్ణాంకం)(len - strm->avail_బహిర్గతం);
}

స్థిర పూర్ణాంకం gzclose(gzFile gz) {
    z_ప్రవాహం *strm;
    గుర్తులేని అక్షరం బహిర్గతం[BUFLEN];

    ఒకవేళ (gz == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    strm = &(gz->strm);
    ఒకవేళ (gz->లిఖ) {
        strm->తదుపరి_లోపల = Z_NULL;
        strm->avail_లోపల = 0;
        చేయి {
            strm->తదుపరి_బహిర్గతం = బహిర్గతం;
            strm->avail_బహిర్గతం = BUFLEN;
            (శూన్య)deflate(strm, Z_ముగింపు);
            fwrite(బహిర్గతం, 1, BUFLEN - strm->avail_బహిర్గతం, gz->file);
        } అయితే (strm->avail_బహిర్గతం == 0);
        deflate_చివర(strm);
    }
    లేకపోతే
        inflate_చివర(strm);
    fclose(gz->file);
    విడుదల(gz);
    తిరిగి Z_OK;
}

స్థిర స్థిరరాశి అక్షరం *gzerror(gzFile gz, పూర్ణాంకం *err) {
    *err = gz->err;
    తిరిగి gz->సందేశం;
}

#అయితే_ముగింపు

స్థిర అక్షరం *prog;

/* ===========================================================================
 * Display error message and exit
 */
స్థిర శూన్య error(స్థిరరాశి అక్షరం *సందేశం) {
    fprintf(stderr, "%s: %s\n", prog, సందేశం);
    నిష్క్రమించు(1);
}

#నిర్వచితమైతే USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
స్థిర పూర్ణాంకం gz_compress_mmap(FILE *లోపల, gzFile బహిర్గతం) {
    పూర్ణాంకం len;
    పూర్ణాంకం err;
    పూర్ణాంకం ifd = fileno(లోపల);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    నిర్మాణం stat sb;

    /* Determine the size of the file, needed for mmap: */
    ఒకవేళ (fstat(ifd, &sb) < 0) తిరిగి Z_ERRNO;
    buf_len = sb.st_size;
    ఒకవేళ (buf_len <= 0) తిరిగి Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    ఒకవేళ (buf == (caddr_t)(-1)) తిరిగి Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(బహిర్గతం, (అక్షరం *)buf, (గుర్తులేని)buf_len);

    ఒకవేళ (len != (పూర్ణాంకం)buf_len) error(gzerror(బహిర్గతం, &err));

    munmap(buf, buf_len);
    fclose(లోపల);
    ఒకవేళ (gzclose(బహిర్గతం) != Z_OK) error("failed gzclose");
    తిరిగి Z_OK;
}
#అయితే_ముగింపు /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

స్థిర శూన్య gz_compress(FILE *లోపల, gzFile బహిర్గతం) {
    local అక్షరం buf[BUFLEN];
    పూర్ణాంకం len;
    పూర్ణాంకం err;

#నిర్వచితమైతే USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    ఒకవేళ (gz_compress_mmap(లోపల, బహిర్గతం) == Z_OK) తిరిగి;
#అయితే_ముగింపు
    కొరకు (;;) {
        len = (పూర్ణాంకం)fread(buf, 1, పరిమాణం(buf), లోపల);
        ఒకవేళ (ferror(లోపల)) {
            perror("fread");
            నిష్క్రమించు(1);
        }
        ఒకవేళ (len == 0) ఆపు;

        ఒకవేళ (gzwrite(బహిర్గతం, buf, (గుర్తులేని)len) != len) error(gzerror(బహిర్గతం, &err));
    }
    fclose(లోపల);
    ఒకవేళ (gzclose(బహిర్గతం) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
స్థిర శూన్య gz_uncompress(gzFile లోపల, FILE *బహిర్గతం) {
    local అక్షరం buf[BUFLEN];
    పూర్ణాంకం len;
    పూర్ణాంకం err;

    కొరకు (;;) {
        len = gzread(లోపల, buf, పరిమాణం(buf));
        ఒకవేళ (len < 0) error (gzerror(లోపల, &err));
        ఒకవేళ (len == 0) ఆపు;

        ఒకవేళ ((పూర్ణాంకం)fwrite(buf, 1, (గుర్తులేని)len, బహిర్గతం) != len) {
            error("failed fwrite");
        }
    }
    ఒకవేళ (fclose(బహిర్గతం)) error("failed fclose");

    ఒకవేళ (gzclose(లోపల) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
స్థిర శూన్య file_compress(అక్షరం *file, అక్షరం *బహుళకం) {
    local అక్షరం outfile[MAX_NAME_LEN+1], *చివర;
    FILE  *లోపల;
    gzFile బహిర్గతం;

    ఒకవేళ (అక్షరశ్రేణి_పొడవు(file) + అక్షరశ్రేణి_పొడవు(GZ_SUFFIX) >= పరిమాణం(outfile)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        నిష్క్రమించు(1);
    }

    చివర = string_copy(outfile, file, పరిమాణం(outfile));
    string_copy(చివర, GZ_SUFFIX, పరిమాణం(outfile) - (z_కొలత_t)(చివర - outfile));

    లోపల = fopen(file, "rb");
    ఒకవేళ (లోపల == ఖాళీ) {
        perror(file);
        నిష్క్రమించు(1);
    }
    బహిర్గతం = gzopen(outfile, బహుళకం);
    ఒకవేళ (బహిర్గతం == ఖాళీ) {
        fclose(లోపల);
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        నిష్క్రమించు(1);
    }
    gz_compress(లోపల, బహిర్గతం);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
స్థిర శూన్య file_uncompress(అక్షరం *file) {
    local అక్షరం buf[MAX_NAME_LEN+1];
    అక్షరం *infile, *outfile;
    FILE  *బహిర్గతం;
    gzFile లోపల;
    z_కొలత_t len = అక్షరశ్రేణి_పొడవు(file);

    ఒకవేళ (len + అక్షరశ్రేణి_పొడవు(GZ_SUFFIX) >= పరిమాణం(buf)) {
        fprintf(stderr, "%s: filename too long\n", prog);
        నిష్క్రమించు(1);
    }

    string_copy(buf, file, పరిమాణం(buf));

    ఒకవేళ (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } లేకపోతే {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, పరిమాణం(buf) - len);
    }
    లోపల = gzopen(infile, "rb");
    ఒకవేళ (లోపల == ఖాళీ) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        నిష్క్రమించు(1);
    }
    బహిర్గతం = fopen(outfile, "wb");
    ఒకవేళ (బహిర్గతం == ఖాళీ) {
        gzclose(లోపల);
        perror(file);
        నిష్క్రమించు(1);
    }

    gz_uncompress(లోపల, బహిర్గతం);

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

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[]) {
    పూర్ణాంకం copyout = 0;
    పూర్ణాంకం uncompr = 0;
    gzFile file;
    అక్షరం *bname, outmode[5];

    string_copy(outmode, "wb6 ", పరిమాణం(outmode));
    prog = argv[0];
    bname = strrchr(argv[0], '/');
    ఒకవేళ (bname)
      bname++;
    లేకపోతే
      bname = argv[0];
    argc--, argv++;

    ఒకవేళ (!strcmp(bname, "gunzip"))
      uncompr = 1;
    లేకపోతే ఒకవేళ (!strcmp(bname, "zcat"))
      copyout = uncompr = 1;

    అయితే (argc > 0) {
      ఒకవేళ (strcmp(*argv, "-c") == 0)
        copyout = 1;
      లేకపోతే ఒకవేళ (strcmp(*argv, "-d") == 0)
        uncompr = 1;
      లేకపోతే ఒకవేళ (strcmp(*argv, "-f") == 0)
        outmode[3] = 'f';
      లేకపోతే ఒకవేళ (strcmp(*argv, "-h") == 0)
        outmode[3] = 'h';
      లేకపోతే ఒకవేళ (strcmp(*argv, "-r") == 0)
        outmode[3] = 'R';
      లేకపోతే ఒకవేళ ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      లేకపోతే
        ఆపు;
      argc--, argv++;
    }
    ఒకవేళ (outmode[3] == ' ')
        outmode[3] = 0;
    ఒకవేళ (argc == 0) {
        సమితి_ద్విమాన_బహుళకం(stdin);
        సమితి_ద్విమాన_బహుళకం(stdout);
        ఒకవేళ (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            ఒకవేళ (file == ఖాళీ) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } లేకపోతే {
            file = gzdopen(fileno(stdout), outmode);
            ఒకవేళ (file == ఖాళీ) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } లేకపోతే {
        ఒకవేళ (copyout) {
            సమితి_ద్విమాన_బహుళకం(stdout);
        }
        చేయి {
            ఒకవేళ (uncompr) {
                ఒకవేళ (copyout) {
                    file = gzopen(*argv, "rb");
                    ఒకవేళ (file == ఖాళీ)
                        fprintf(stderr, "%s: can't gzopen %s\n", prog, *argv);
                    లేకపోతే
                        gz_uncompress(file, stdout);
                } లేకపోతే {
                    file_uncompress(*argv);
                }
            } లేకపోతే {
                ఒకవేళ (copyout) {
                    FILE * లోపల = fopen(*argv, "rb");

                    ఒకవేళ (లోపల == ఖాళీ) {
                        perror(*argv);
                    } లేకపోతే {
                        file = gzdopen(fileno(stdout), outmode);
                        ఒకవేళ (file == ఖాళీ) error("can't gzdopen stdout");

                        gz_compress(లోపల, file);
                    }

                } లేకపోతే {
                    file_compress(*argv, outmode);
                }
            }
        } అయితే (argv++, --argc);
    }
    తిరిగి 0;
}
