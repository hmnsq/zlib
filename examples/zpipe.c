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

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ "zlib.h"

#ଯଦି defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_ଦ୍ୱିମିକ_MODE(file) _setmode(_fileno(file), O_ଦ୍ୱିମିକ_2)
#ଅନ୍ୟଥା
#  define SET_ଦ୍ୱିମିକ_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା def(FILE *source, FILE *dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, flush;
    unsigned have;
    z_stream strm;
    unsigned ଅକ୍ଷର in[CHUNK];
    unsigned ଅକ୍ଷର out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    ଯଦି (ret != Z_OK)
        ଫେରନ୍ତୁ ret;

    /* compress until end of file */
    କର {
        strm.avail_in = fread(in, 1, CHUNK, source);
        ଯଦି (ferror(source)) {
            (ଶୂନ୍ୟ)deflateEnd(&strm);
            ଫେରନ୍ତୁ Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        କର {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            ପ୍ରତିଜ୍ଞା(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            ଯଦି (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (ଶୂନ୍ୟ)deflateEnd(&strm);
                ଫେରନ୍ତୁ Z_ERRNO;
            }
        } ଯେତେବେଳେ (strm.avail_out == 0);
        ପ୍ରତିଜ୍ଞା(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } ଯେତେବେଳେ (flush != Z_FINISH);
    ପ୍ରତିଜ୍ଞା(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (ଶୂନ୍ୟ)deflateEnd(&strm);
    ଫେରନ୍ତୁ Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା inf(FILE *source, FILE *dest)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    unsigned have;
    z_stream strm;
    unsigned ଅକ୍ଷର in[CHUNK];
    unsigned ଅକ୍ଷର out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    ଯଦି (ret != Z_OK)
        ଫେରନ୍ତୁ ret;

    /* decompress until deflate stream ends or end of file */
    କର {
        strm.avail_in = fread(in, 1, CHUNK, source);
        ଯଦି (ferror(source)) {
            (ଶୂନ୍ୟ)inflateEnd(&strm);
            ଫେରନ୍ତୁ Z_ERRNO;
        }
        ଯଦି (strm.avail_in == 0)
            ଭାଙ୍ଗ;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        କର {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            ପ୍ରତିଜ୍ଞା(ret != Z_STREAM_ERROR);  /* state not clobbered */
            ବାଛ (ret) {
            ସ୍ଥିତି Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            ସ୍ଥିତି Z_DATA_ERROR:
            ସ୍ଥିତି Z_MEM_ERROR:
                (ଶୂନ୍ୟ)inflateEnd(&strm);
                ଫେରନ୍ତୁ ret;
            }
            have = CHUNK - strm.avail_out;
            ଯଦି (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (ଶୂନ୍ୟ)inflateEnd(&strm);
                ଫେରନ୍ତୁ Z_ERRNO;
            }
        } ଯେତେବେଳେ (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } ଯେତେବେଳେ (ret != Z_STREAM_END);

    /* clean up and return */
    (ଶୂନ୍ୟ)inflateEnd(&strm);
    ଫେରନ୍ତୁ ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
ଶୂନ୍ୟ zerr(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret)
{
    fputs("zpipe: ", stderr);
    ବାଛ (ret) {
    ସ୍ଥିତି Z_ERRNO:
        ଯଦି (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        ଯଦି (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

    /* avoid end-of-line conversions */
    SET_ଦ୍ୱିମିକ_MODE(stdin);
    SET_ଦ୍ୱିମିକ_MODE(stdout);

    /* do compression if no arguments */
    ଯଦି (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        ଯଦି (ret != Z_OK)
            zerr(ret);
        ଫେରନ୍ତୁ ret;
    }

    /* do decompression if -d specified */
    ଅନ୍ୟଥା ଯଦି (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        ଯଦି (ret != Z_OK)
            zerr(ret);
        ଫେରନ୍ତୁ ret;
    }

    /* otherwise, report usage */
    ଅନ୍ୟଥା {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        ଫେରନ୍ତୁ 1;
    }
}
