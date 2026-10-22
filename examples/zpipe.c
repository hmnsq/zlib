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
#  define SET_دودویی_MODE(file) _setmode(_fileno(file), O_دودویی_2)
#وگرنه
#  define SET_دودویی_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
صحیح def(FILE *source, FILE *dest, صحیح level)
{
    صحیح ret, flush;
    unsigned have;
    z_stream strm;
    unsigned نویسه in[CHUNK];
    unsigned نویسه out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    اگر (ret != Z_OK)
        بازگشت ret;

    /* compress until end of file */
    انجامبده {
        strm.avail_in = fread(in, 1, CHUNK, source);
        اگر (ferror(source)) {
            (تهی)deflateEnd(&strm);
            بازگشت Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        انجامبده {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            أكد(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            اگر (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (تهی)deflateEnd(&strm);
                بازگشت Z_ERRNO;
            }
        } وقتی (strm.avail_out == 0);
        أكد(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } وقتی (flush != Z_FINISH);
    أكد(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (تهی)deflateEnd(&strm);
    بازگشت Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
صحیح inf(FILE *source, FILE *dest)
{
    صحیح ret;
    unsigned have;
    z_stream strm;
    unsigned نویسه in[CHUNK];
    unsigned نویسه out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    اگر (ret != Z_OK)
        بازگشت ret;

    /* decompress until deflate stream ends or end of file */
    انجامبده {
        strm.avail_in = fread(in, 1, CHUNK, source);
        اگر (ferror(source)) {
            (تهی)inflateEnd(&strm);
            بازگشت Z_ERRNO;
        }
        اگر (strm.avail_in == 0)
            بشکن;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        انجامبده {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            أكد(ret != Z_STREAM_ERROR);  /* state not clobbered */
            انتخاب (ret) {
            حالت Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            حالت Z_DATA_ERROR:
            حالت Z_MEM_ERROR:
                (تهی)inflateEnd(&strm);
                بازگشت ret;
            }
            have = CHUNK - strm.avail_out;
            اگر (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (تهی)inflateEnd(&strm);
                بازگشت Z_ERRNO;
            }
        } وقتی (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } وقتی (ret != Z_STREAM_END);

    /* clean up and return */
    (تهی)inflateEnd(&strm);
    بازگشت ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
تهی zerr(صحیح ret)
{
    fputs("zpipe: ", stderr);
    انتخاب (ret) {
    حالت Z_ERRNO:
        اگر (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        اگر (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        بشکن;
    حالت Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        بشکن;
    حالت Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        بشکن;
    حالت Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        بشکن;
    حالت Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
صحیح اصلی(صحیح argc, نویسه **argv)
{
    صحیح ret;

    /* avoid end-of-line conversions */
    SET_دودویی_MODE(stdin);
    SET_دودویی_MODE(stdout);

    /* do compression if no arguments */
    اگر (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        اگر (ret != Z_OK)
            zerr(ret);
        بازگشت ret;
    }

    /* do decompression if -d specified */
    وگرنه اگر (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        اگر (ret != Z_OK)
            zerr(ret);
        بازگشت ret;
    }

    /* otherwise, report usage */
    وگرنه {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        بازگشت 1;
    }
}
