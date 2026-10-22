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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"

#ถ้า defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), _O_BINARY)
#มิฉะนั้น
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
จำนวนเต็ม def(FILE *source, FILE *dest, จำนวนเต็ม level)
{
    จำนวนเต็ม ret, flush;
    unsigned have;
    z_stream strm;
    unsigned var_na in[CHUNK];
    unsigned var_na out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    ถ้า (ret != Z_OK)
        คืน ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        ถ้า (ferror(source)) {
            (void)deflateEnd(&strm);
            คืน Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            ถ้า (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                คืน Z_ERRNO;
            }
        } ขณะ (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } ขณะ (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    คืน Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
จำนวนเต็ม inf(FILE *source, FILE *dest)
{
    จำนวนเต็ม ret;
    unsigned have;
    z_stream strm;
    unsigned var_na in[CHUNK];
    unsigned var_na out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    ถ้า (ret != Z_OK)
        คืน ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        ถ้า (ferror(source)) {
            (void)inflateEnd(&strm);
            คืน Z_ERRNO;
        }
        ถ้า (strm.avail_in == 0)
            bha_nga;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            pak_sa Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            pak_sa Z_DATA_ERROR:
            pak_sa Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                คืน ret;
            }
            have = CHUNK - strm.avail_out;
            ถ้า (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                คืน Z_ERRNO;
            }
        } ขณะ (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } ขณะ (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    คืน ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(จำนวนเต็ม ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    pak_sa Z_ERRNO:
        ถ้า (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        ถ้า (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        bha_nga;
    pak_sa Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        bha_nga;
    pak_sa Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        bha_nga;
    pak_sa Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        bha_nga;
    pak_sa Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na **argv)
{
    จำนวนเต็ม ret;

    /* avoid end-of-line conversions */
    SET_BINARY_MODE(stdin);
    SET_BINARY_MODE(stdout);

    /* do compression if no arguments */
    ถ้า (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        ถ้า (ret != Z_OK)
            zerr(ret);
        คืน ret;
    }

    /* do decompression if -d specified */
    มิฉะนั้น ถ้า (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        ถ้า (ret != Z_OK)
            zerr(ret);
        คืน ret;
    }

    /* otherwise, report usage */
    มิฉะนั้น {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        คืน 1;
    }
}
