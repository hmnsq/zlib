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

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ "zlib.h"

#எனில் defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_இருமம்_MODE(file) _setmode(_fileno(file), O_இருமம்_2)
#இல்லையெனில்
#  define SET_இருமம்_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
முழு def(FILE *source, FILE *dest, முழு level)
{
    முழு ret, flush;
    unsigned have;
    z_stream strm;
    unsigned எழுத்து in[CHUNK];
    unsigned எழுத்து out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    எனில் (ret != Z_OK)
        திரும்பு ret;

    /* compress until end of file */
    செய் {
        strm.avail_in = fread(in, 1, CHUNK, source);
        எனில் (ferror(source)) {
            (வெறுமை)deflateEnd(&strm);
            திரும்பு Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        செய் {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            ப்ரதிஜ்ஞா(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            எனில் (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (வெறுமை)deflateEnd(&strm);
                திரும்பு Z_ERRNO;
            }
        } போது (strm.avail_out == 0);
        ப்ரதிஜ்ஞா(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } போது (flush != Z_FINISH);
    ப்ரதிஜ்ஞா(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (வெறுமை)deflateEnd(&strm);
    திரும்பு Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
முழு inf(FILE *source, FILE *dest)
{
    முழு ret;
    unsigned have;
    z_stream strm;
    unsigned எழுத்து in[CHUNK];
    unsigned எழுத்து out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    எனில் (ret != Z_OK)
        திரும்பு ret;

    /* decompress until deflate stream ends or end of file */
    செய் {
        strm.avail_in = fread(in, 1, CHUNK, source);
        எனில் (ferror(source)) {
            (வெறுமை)inflateEnd(&strm);
            திரும்பு Z_ERRNO;
        }
        எனில் (strm.avail_in == 0)
            நிறுத்து;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        செய் {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            ப்ரதிஜ்ஞா(ret != Z_STREAM_ERROR);  /* state not clobbered */
            தேர்வு (ret) {
            நிலை Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            நிலை Z_DATA_ERROR:
            நிலை Z_MEM_ERROR:
                (வெறுமை)inflateEnd(&strm);
                திரும்பு ret;
            }
            have = CHUNK - strm.avail_out;
            எனில் (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (வெறுமை)inflateEnd(&strm);
                திரும்பு Z_ERRNO;
            }
        } போது (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } போது (ret != Z_STREAM_END);

    /* clean up and return */
    (வெறுமை)inflateEnd(&strm);
    திரும்பு ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
வெறுமை zerr(முழு ret)
{
    fputs("zpipe: ", stderr);
    தேர்வு (ret) {
    நிலை Z_ERRNO:
        எனில் (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        எனில் (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        நிறுத்து;
    நிலை Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        நிறுத்து;
    நிலை Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        நிறுத்து;
    நிலை Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        நிறுத்து;
    நிலை Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
முழு முதன்மை(முழு argc, எழுத்து **argv)
{
    முழு ret;

    /* avoid end-of-line conversions */
    SET_இருமம்_MODE(stdin);
    SET_இருமம்_MODE(stdout);

    /* do compression if no arguments */
    எனில் (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        எனில் (ret != Z_OK)
            zerr(ret);
        திரும்பு ret;
    }

    /* do decompression if -d specified */
    இல்லையெனில் எனில் (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        எனில் (ret != Z_OK)
            zerr(ret);
        திரும்பு ret;
    }

    /* otherwise, report usage */
    இல்லையெனில் {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        திரும்பு 1;
    }
}
