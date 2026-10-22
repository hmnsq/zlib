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

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব "zlib.h"

#যদি defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_দ্বিমিক_MODE(file) _setmode(_fileno(file), O_দ্বিমিক_2)
#অন্যথা
#  define SET_দ্বিমিক_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
পূর্ণাঙ্ক def(FILE *source, FILE *dest, পূর্ণাঙ্ক level)
{
    পূর্ণাঙ্ক ret, flush;
    unsigned have;
    z_stream strm;
    unsigned বর্ণ in[CHUNK];
    unsigned বর্ণ out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    যদি (ret != Z_OK)
        প্রত্যাগম ret;

    /* compress until end of file */
    কুরু {
        strm.avail_in = fread(in, 1, CHUNK, source);
        যদি (ferror(source)) {
            (শূন্য)deflateEnd(&strm);
            প্রত্যাগম Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        কুরু {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            প্রতিজ্ঞা(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            যদি (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (শূন্য)deflateEnd(&strm);
                প্রত্যাগম Z_ERRNO;
            }
        } যাবত্ (strm.avail_out == 0);
        প্রতিজ্ঞা(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } যাবত্ (flush != Z_FINISH);
    প্রতিজ্ঞা(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (শূন্য)deflateEnd(&strm);
    প্রত্যাগম Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
পূর্ণাঙ্ক inf(FILE *source, FILE *dest)
{
    পূর্ণাঙ্ক ret;
    unsigned have;
    z_stream strm;
    unsigned বর্ণ in[CHUNK];
    unsigned বর্ণ out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    যদি (ret != Z_OK)
        প্রত্যাগম ret;

    /* decompress until deflate stream ends or end of file */
    কুরু {
        strm.avail_in = fread(in, 1, CHUNK, source);
        যদি (ferror(source)) {
            (শূন্য)inflateEnd(&strm);
            প্রত্যাগম Z_ERRNO;
        }
        যদি (strm.avail_in == 0)
            ভঙ্গ;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        কুরু {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            প্রতিজ্ঞা(ret != Z_STREAM_ERROR);  /* state not clobbered */
            বিকল্প (ret) {
            পক্ষ Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            পক্ষ Z_DATA_ERROR:
            পক্ষ Z_MEM_ERROR:
                (শূন্য)inflateEnd(&strm);
                প্রত্যাগম ret;
            }
            have = CHUNK - strm.avail_out;
            যদি (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (শূন্য)inflateEnd(&strm);
                প্রত্যাগম Z_ERRNO;
            }
        } যাবত্ (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } যাবত্ (ret != Z_STREAM_END);

    /* clean up and return */
    (শূন্য)inflateEnd(&strm);
    প্রত্যাগম ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
শূন্য zerr(পূর্ণাঙ্ক ret)
{
    fputs("zpipe: ", stderr);
    বিকল্প (ret) {
    পক্ষ Z_ERRNO:
        যদি (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        যদি (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        ভঙ্গ;
    পক্ষ Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        ভঙ্গ;
    পক্ষ Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        ভঙ্গ;
    পক্ষ Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        ভঙ্গ;
    পক্ষ Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
    পূর্ণাঙ্ক ret;

    /* avoid end-of-line conversions */
    SET_দ্বিমিক_MODE(stdin);
    SET_দ্বিমিক_MODE(stdout);

    /* do compression if no arguments */
    যদি (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        যদি (ret != Z_OK)
            zerr(ret);
        প্রত্যাগম ret;
    }

    /* do decompression if -d specified */
    অন্যথা যদি (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        যদি (ret != Z_OK)
            zerr(ret);
        প্রত্যাগম ret;
    }

    /* otherwise, report usage */
    অন্যথা {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        প্রত্যাগম 1;
    }
}
