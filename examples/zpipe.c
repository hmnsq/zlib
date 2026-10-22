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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

#ಯದಿ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_ದ್ವಿಮಾನ_MODE(file) _setmode(_fileno(file), O_ದ್ವಿಮಾನ_2)
#ಅನ್ಯಥಾ
#  define SET_ದ್ವಿಮಾನ_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
ಪೂರ್ಣಾಙ್ಕ def(FILE *source, FILE *dest, ಪೂರ್ಣಾಙ್ಕ level)
{
    ಪೂರ್ಣಾಙ್ಕ ret, flush;
    unsigned have;
    z_stream strm;
    unsigned ವರ್ಣ in[CHUNK];
    unsigned ವರ್ಣ out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    ಯದಿ (ret != Z_OK)
        ಪ್ರತ್ಯಾಗಮ ret;

    /* compress until end of file */
    ಕುರು {
        strm.avail_in = fread(in, 1, CHUNK, source);
        ಯದಿ (ferror(source)) {
            (ಶೂನ್ಯ)deflateEnd(&strm);
            ಪ್ರತ್ಯಾಗಮ Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        ಕುರು {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            ಪ್ರತಿಜ್ಞಾ(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            ಯದಿ (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (ಶೂನ್ಯ)deflateEnd(&strm);
                ಪ್ರತ್ಯಾಗಮ Z_ERRNO;
            }
        } ಯಾವತ್ (strm.avail_out == 0);
        ಪ್ರತಿಜ್ಞಾ(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } ಯಾವತ್ (flush != Z_FINISH);
    ಪ್ರತಿಜ್ಞಾ(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (ಶೂನ್ಯ)deflateEnd(&strm);
    ಪ್ರತ್ಯಾಗಮ Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
ಪೂರ್ಣಾಙ್ಕ inf(FILE *source, FILE *dest)
{
    ಪೂರ್ಣಾಙ್ಕ ret;
    unsigned have;
    z_stream strm;
    unsigned ವರ್ಣ in[CHUNK];
    unsigned ವರ್ಣ out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    ಯದಿ (ret != Z_OK)
        ಪ್ರತ್ಯಾಗಮ ret;

    /* decompress until deflate stream ends or end of file */
    ಕುರು {
        strm.avail_in = fread(in, 1, CHUNK, source);
        ಯದಿ (ferror(source)) {
            (ಶೂನ್ಯ)inflateEnd(&strm);
            ಪ್ರತ್ಯಾಗಮ Z_ERRNO;
        }
        ಯದಿ (strm.avail_in == 0)
            ಭಙ್ಗ;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        ಕುರು {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            ಪ್ರತಿಜ್ಞಾ(ret != Z_STREAM_ERROR);  /* state not clobbered */
            ವಿಕಲ್ಪ (ret) {
            ಪಕ್ಷ Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            ಪಕ್ಷ Z_DATA_ERROR:
            ಪಕ್ಷ Z_MEM_ERROR:
                (ಶೂನ್ಯ)inflateEnd(&strm);
                ಪ್ರತ್ಯಾಗಮ ret;
            }
            have = CHUNK - strm.avail_out;
            ಯದಿ (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (ಶೂನ್ಯ)inflateEnd(&strm);
                ಪ್ರತ್ಯಾಗಮ Z_ERRNO;
            }
        } ಯಾವತ್ (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } ಯಾವತ್ (ret != Z_STREAM_END);

    /* clean up and return */
    (ಶೂನ್ಯ)inflateEnd(&strm);
    ಪ್ರತ್ಯಾಗಮ ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
ಶೂನ್ಯ zerr(ಪೂರ್ಣಾಙ್ಕ ret)
{
    fputs("zpipe: ", stderr);
    ವಿಕಲ್ಪ (ret) {
    ಪಕ್ಷ Z_ERRNO:
        ಯದಿ (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        ಯದಿ (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        ಭಙ್ಗ;
    ಪಕ್ಷ Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        ಭಙ್ಗ;
    ಪಕ್ಷ Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        ಭಙ್ಗ;
    ಪಕ್ಷ Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        ಭಙ್ಗ;
    ಪಕ್ಷ Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
    ಪೂರ್ಣಾಙ್ಕ ret;

    /* avoid end-of-line conversions */
    SET_ದ್ವಿಮಾನ_MODE(stdin);
    SET_ದ್ವಿಮಾನ_MODE(stdout);

    /* do compression if no arguments */
    ಯದಿ (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        ಯದಿ (ret != Z_OK)
            zerr(ret);
        ಪ್ರತ್ಯಾಗಮ ret;
    }

    /* do decompression if -d specified */
    ಅನ್ಯಥಾ ಯದಿ (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        ಯದಿ (ret != Z_OK)
            zerr(ret);
        ಪ್ರತ್ಯಾಗಮ ret;
    }

    /* otherwise, report usage */
    ಅನ್ಯಥಾ {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        ಪ್ರತ್ಯಾಗಮ 1;
    }
}
