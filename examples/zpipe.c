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

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ "zlib.h"

#જો defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_દ્વિઅંકી_MODE(file) _setmode(_fileno(file), O_દ્વિઅંકી_2)
#નહીંતર
#  define SET_દ્વિઅંકી_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
પૂર્ણાંક def(FILE *source, FILE *dest, પૂર્ણાંક level)
{
    પૂર્ણાંક ret, flush;
    unsigned have;
    z_stream strm;
    unsigned અક્ષર in[CHUNK];
    unsigned અક્ષર out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    જો (ret != Z_OK)
        પાછા ret;

    /* compress until end of file */
    કરો {
        strm.avail_in = fread(in, 1, CHUNK, source);
        જો (ferror(source)) {
            (શૂન્ય)deflateEnd(&strm);
            પાછા Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        કરો {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            પ્રતિજ્ઞા(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            જો (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (શૂન્ય)deflateEnd(&strm);
                પાછા Z_ERRNO;
            }
        } જ્યારે (strm.avail_out == 0);
        પ્રતિજ્ઞા(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } જ્યારે (flush != Z_FINISH);
    પ્રતિજ્ઞા(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (શૂન્ય)deflateEnd(&strm);
    પાછા Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
પૂર્ણાંક inf(FILE *source, FILE *dest)
{
    પૂર્ણાંક ret;
    unsigned have;
    z_stream strm;
    unsigned અક્ષર in[CHUNK];
    unsigned અક્ષર out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    જો (ret != Z_OK)
        પાછા ret;

    /* decompress until deflate stream ends or end of file */
    કરો {
        strm.avail_in = fread(in, 1, CHUNK, source);
        જો (ferror(source)) {
            (શૂન્ય)inflateEnd(&strm);
            પાછા Z_ERRNO;
        }
        જો (strm.avail_in == 0)
            તોડો;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        કરો {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            પ્રતિજ્ઞા(ret != Z_STREAM_ERROR);  /* state not clobbered */
            પસંદગી (ret) {
            સ્થિતિ Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            સ્થિતિ Z_DATA_ERROR:
            સ્થિતિ Z_MEM_ERROR:
                (શૂન્ય)inflateEnd(&strm);
                પાછા ret;
            }
            have = CHUNK - strm.avail_out;
            જો (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (શૂન્ય)inflateEnd(&strm);
                પાછા Z_ERRNO;
            }
        } જ્યારે (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } જ્યારે (ret != Z_STREAM_END);

    /* clean up and return */
    (શૂન્ય)inflateEnd(&strm);
    પાછા ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
શૂન્ય zerr(પૂર્ણાંક ret)
{
    fputs("zpipe: ", stderr);
    પસંદગી (ret) {
    સ્થિતિ Z_ERRNO:
        જો (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        જો (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        તોડો;
    સ્થિતિ Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        તોડો;
    સ્થિતિ Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        તોડો;
    સ્થિતિ Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        તોડો;
    સ્થિતિ Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
    પૂર્ણાંક ret;

    /* avoid end-of-line conversions */
    SET_દ્વિઅંકી_MODE(stdin);
    SET_દ્વિઅંકી_MODE(stdout);

    /* do compression if no arguments */
    જો (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        જો (ret != Z_OK)
            zerr(ret);
        પાછા ret;
    }

    /* do decompression if -d specified */
    નહીંતર જો (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        જો (ret != Z_OK)
            zerr(ret);
        પાછા ret;
    }

    /* otherwise, report usage */
    નહીંતર {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        પાછા 1;
    }
}
