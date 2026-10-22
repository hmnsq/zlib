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

#شامل <stdio.h>
#شامل <string.h>
#شامل <assert.h>
#شامل "zlib.h"

#اگر defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define سیٹ_ثنائی_منوال(file) _setmode(_fileno(file), O_ثنائی_2)
#ورنہ
#  define سیٹ_ثنائی_منوال(file)
#اگر_ختم

#تعریف_کرو CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
صحیح def(FILE *ماخذ, FILE *dest, صحیح level)
{
    صحیح ret, flush;
    غیرعلامتی have;
    z_دھارا strm;
    غیرعلامتی حرف اندر[CHUNK];
    غیرعلامتی حرف برآمد[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    اگر (ret != Z_OK)
        واپس ret;

    /* compress until end of file */
    کرو {
        strm.avail_اندر = fread(اندر, 1, CHUNK, ماخذ);
        اگر (ferror(ماخذ)) {
            (خالی)deflate_انتہا(&strm);
            واپس Z_ERRNO;
        }
        flush = feof(ماخذ) ? Z_اختتام : Z_NO_FLUSH;
        strm.اگلا_اندر = اندر;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        کرو {
            strm.avail_برآمد = CHUNK;
            strm.اگلا_برآمد = برآمد;
            ret = deflate(&strm, flush);    /* no bad return value */
            أكد(ret != Z_دھارا_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_برآمد;
            اگر (fwrite(برآمد, 1, have, dest) != have || ferror(dest)) {
                (خالی)deflate_انتہا(&strm);
                واپس Z_ERRNO;
            }
        } جب (strm.avail_برآمد == 0);
        أكد(strm.avail_اندر == 0);     /* all input will be used */

        /* done when last data in file processed */
    } جب (flush != Z_اختتام);
    أكد(ret == Z_دھارا_انتہا);        /* stream will be complete */

    /* clean up and return */
    (خالی)deflate_انتہا(&strm);
    واپس Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
صحیح inf(FILE *ماخذ, FILE *dest)
{
    صحیح ret;
    غیرعلامتی have;
    z_دھارا strm;
    غیرعلامتی حرف اندر[CHUNK];
    غیرعلامتی حرف برآمد[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_اندر = 0;
    strm.اگلا_اندر = Z_NULL;
    ret = inflateInit(&strm);
    اگر (ret != Z_OK)
        واپس ret;

    /* decompress until deflate stream ends or end of file */
    کرو {
        strm.avail_اندر = fread(اندر, 1, CHUNK, ماخذ);
        اگر (ferror(ماخذ)) {
            (خالی)inflate_انتہا(&strm);
            واپس Z_ERRNO;
        }
        اگر (strm.avail_اندر == 0)
            توڑو;
        strm.اگلا_اندر = اندر;

        /* run inflate() on input until output buffer not full */
        کرو {
            strm.avail_برآمد = CHUNK;
            strm.اگلا_برآمد = برآمد;
            ret = inflate(&strm, Z_NO_FLUSH);
            أكد(ret != Z_دھارا_ERROR);  /* state not clobbered */
            انتخاب (ret) {
            حالت Z_NEED_DICT:
                ret = Z_ڈیٹا_ERROR;     /* and fall through */
            حالت Z_ڈیٹا_ERROR:
            حالت Z_MEM_ERROR:
                (خالی)inflate_انتہا(&strm);
                واپس ret;
            }
            have = CHUNK - strm.avail_برآمد;
            اگر (fwrite(برآمد, 1, have, dest) != have || ferror(dest)) {
                (خالی)inflate_انتہا(&strm);
                واپس Z_ERRNO;
            }
        } جب (strm.avail_برآمد == 0);

        /* done when inflate() says it's done */
    } جب (ret != Z_دھارا_انتہا);

    /* clean up and return */
    (خالی)inflate_انتہا(&strm);
    واپس ret == Z_دھارا_انتہا ? Z_OK : Z_ڈیٹا_ERROR;
}

/* report a zlib or i/o error */
خالی zerr(صحیح ret)
{
    fputs("zpipe: ", stderr);
    انتخاب (ret) {
    حالت Z_ERRNO:
        اگر (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        اگر (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        توڑو;
    حالت Z_دھارا_ERROR:
        fputs("invalid compression level\n", stderr);
        توڑو;
    حالت Z_ڈیٹا_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        توڑو;
    حالت Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        توڑو;
    حالت Z_نسخہ_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
صحیح مین(صحیح argc, حرف **argv)
{
    صحیح ret;

    /* avoid end-of-line conversions */
    سیٹ_ثنائی_منوال(stdin);
    سیٹ_ثنائی_منوال(stdout);

    /* do compression if no arguments */
    اگر (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        اگر (ret != Z_OK)
            zerr(ret);
        واپس ret;
    }

    /* do decompression if -d specified */
    ورنہ اگر (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        اگر (ret != Z_OK)
            zerr(ret);
        واپس ret;
    }

    /* otherwise, report usage */
    ورنہ {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        واپس 1;
    }
}
