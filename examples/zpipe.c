/* zpipe.c: example of proper use of zlib's inflate() and deflate()
   Not copyrighted -- provided to the public domain
   Version 1.5  11 February 2026  Mark Adler */

/* Version history:
   1.0  30 Oct 2004  First version
   1.1   8 Nov 2004  Add void casting for unused return values
                     Use switch statement for inflate() return values
   1.2   9 Nov 2004  Add assertions to document zlib guarantees
   1.3   6 Apr 2005  Remove incorrect assertion in inf()
   1.4  11 Dec 2005  Add hack to avoid MSDOS end-of-line conversions
                     Avoid some compiler warnings for input and output buffers
   1.5  11 Feb 2026  Use underscores for Windows POSIX names
 */

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ "zlib.h"

#ఒకవేళ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define సమితి_ద్విమాన_బహుళకం(file) _setmode(_fileno(file), O_ద్విమాన_2)
#లేకపోతే
#  define సమితి_ద్విమాన_బహుళకం(file)
#అయితే_ముగింపు

#నిర్వచించు CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
పూర్ణాంకం def(FILE *ఆధారం, FILE *dest, పూర్ణాంకం level)
{
    పూర్ణాంకం ret, flush;
    గుర్తులేని have;
    z_ప్రవాహం strm;
    గుర్తులేని అక్షరం లోపల[CHUNK];
    గుర్తులేని అక్షరం బహిర్గతం[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    ఒకవేళ (ret != Z_OK)
        తిరిగి ret;

    /* compress until end of file */
    చేయి {
        strm.avail_లోపల = fread(లోపల, 1, CHUNK, ఆధారం);
        ఒకవేళ (ferror(ఆధారం)) {
            (శూన్య)deflate_చివర(&strm);
            తిరిగి Z_ERRNO;
        }
        flush = feof(ఆధారం) ? Z_ముగింపు : Z_NO_FLUSH;
        strm.తదుపరి_లోపల = లోపల;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        చేయి {
            strm.avail_బహిర్గతం = CHUNK;
            strm.తదుపరి_బహిర్గతం = బహిర్గతం;
            ret = deflate(&strm, flush);    /* no bad return value */
            ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_బహిర్గతం;
            ఒకవేళ (fwrite(బహిర్గతం, 1, have, dest) != have || ferror(dest)) {
                (శూన్య)deflate_చివర(&strm);
                తిరిగి Z_ERRNO;
            }
        } అయితే (strm.avail_బహిర్గతం == 0);
        ప్రతిజ్ఞా(strm.avail_లోపల == 0);     /* all input will be used */

        /* done when last data in file processed */
    } అయితే (flush != Z_ముగింపు);
    ప్రతిజ్ఞా(ret == Z_ప్రవాహం_చివర);        /* stream will be complete */

    /* clean up and return */
    (శూన్య)deflate_చివర(&strm);
    తిరిగి Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
పూర్ణాంకం inf(FILE *ఆధారం, FILE *dest)
{
    పూర్ణాంకం ret;
    గుర్తులేని have;
    z_ప్రవాహం strm;
    గుర్తులేని అక్షరం లోపల[CHUNK];
    గుర్తులేని అక్షరం బహిర్గతం[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit(&strm);
    ఒకవేళ (ret != Z_OK)
        తిరిగి ret;

    /* decompress until deflate stream ends or end of file */
    చేయి {
        strm.avail_లోపల = fread(లోపల, 1, CHUNK, ఆధారం);
        ఒకవేళ (ferror(ఆధారం)) {
            (శూన్య)inflate_చివర(&strm);
            తిరిగి Z_ERRNO;
        }
        ఒకవేళ (strm.avail_లోపల == 0)
            ఆపు;
        strm.తదుపరి_లోపల = లోపల;

        /* run inflate() on input until output buffer not full */
        చేయి {
            strm.avail_బహిర్గతం = CHUNK;
            strm.తదుపరి_బహిర్గతం = బహిర్గతం;
            ret = inflate(&strm, Z_NO_FLUSH);
            ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);  /* state not clobbered */
            ఎంపిక (ret) {
            స్థితి Z_NEED_DICT:
                ret = Z_దత్తాంశం_ERROR;     /* and fall through */
            స్థితి Z_దత్తాంశం_ERROR:
            స్థితి Z_MEM_ERROR:
                (శూన్య)inflate_చివర(&strm);
                తిరిగి ret;
            }
            have = CHUNK - strm.avail_బహిర్గతం;
            ఒకవేళ (fwrite(బహిర్గతం, 1, have, dest) != have || ferror(dest)) {
                (శూన్య)inflate_చివర(&strm);
                తిరిగి Z_ERRNO;
            }
        } అయితే (strm.avail_బహిర్గతం == 0);

        /* done when inflate() says it's done */
    } అయితే (ret != Z_ప్రవాహం_చివర);

    /* clean up and return */
    (శూన్య)inflate_చివర(&strm);
    తిరిగి ret == Z_ప్రవాహం_చివర ? Z_OK : Z_దత్తాంశం_ERROR;
}

/* report a zlib or i/o error */
శూన్య zerr(పూర్ణాంకం ret)
{
    fputs("zpipe: ", stderr);
    ఎంపిక (ret) {
    స్థితి Z_ERRNO:
        ఒకవేళ (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        ఒకవేళ (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        ఆపు;
    స్థితి Z_ప్రవాహం_ERROR:
        fputs("invalid compression level\n", stderr);
        ఆపు;
    స్థితి Z_దత్తాంశం_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        ఆపు;
    స్థితి Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        ఆపు;
    స్థితి Z_సంస్కరణ_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
    పూర్ణాంకం ret;

    /* avoid end-of-line conversions */
    సమితి_ద్విమాన_బహుళకం(stdin);
    సమితి_ద్విమాన_బహుళకం(stdout);

    /* do compression if no arguments */
    ఒకవేళ (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        ఒకవేళ (ret != Z_OK)
            zerr(ret);
        తిరిగి ret;
    }

    /* do decompression if -d specified */
    లేకపోతే ఒకవేళ (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        ఒకవేళ (ret != Z_OK)
            zerr(ret);
        తిరిగి ret;
    }

    /* otherwise, report usage */
    లేకపోతే {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        తిరిగి 1;
    }
}
