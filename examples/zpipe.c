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

#ಆದರೆ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file) _setmode(_fileno(file), O_ದ್ವಿಮಾನ_2)
#ಇಲ್ಲದಿದ್ದರೆ
#  define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
ಪೂರ್ಣಾಂಕ def(FILE *ಮೂಲ, FILE *dest, ಪೂರ್ಣಾಂಕ level)
{
    ಪೂರ್ಣಾಂಕ ret, flush;
    ಚಿಹ್ನೆರಹಿತ have;
    z_ಹರಿವು strm;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಒಳ[CHUNK];
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಹೊರಹೋಗು[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    ಆದರೆ (ret != Z_OK)
        ಹಿಂತಿರುಗಿ ret;

    /* compress until end of file */
    ಮಾಡು {
        strm.avail_ಒಳ = fread(ಒಳ, 1, CHUNK, ಮೂಲ);
        ಆದರೆ (ferror(ಮೂಲ)) {
            (ಶೂನ್ಯ)deflate_ಕೊನೆ(&strm);
            ಹಿಂತಿರುಗಿ Z_ERRNO;
        }
        flush = feof(ಮೂಲ) ? Z_ಮುಕ್ತಾಯ : Z_NO_FLUSH;
        strm.ಮುಂದಿನ_ಒಳ = ಒಳ;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        ಮಾಡು {
            strm.avail_ಹೊರಹೋಗು = CHUNK;
            strm.ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
            ret = deflate(&strm, flush);    /* no bad return value */
            ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_ಹೊರಹೋಗು;
            ಆದರೆ (fwrite(ಹೊರಹೋಗು, 1, have, dest) != have || ferror(dest)) {
                (ಶೂನ್ಯ)deflate_ಕೊನೆ(&strm);
                ಹಿಂತಿರುಗಿ Z_ERRNO;
            }
        } ಆಗ (strm.avail_ಹೊರಹೋಗು == 0);
        ಪ್ರತಿಜ್ಞಾ(strm.avail_ಒಳ == 0);     /* all input will be used */

        /* done when last data in file processed */
    } ಆಗ (flush != Z_ಮುಕ್ತಾಯ);
    ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ಕೊನೆ);        /* stream will be complete */

    /* clean up and return */
    (ಶೂನ್ಯ)deflate_ಕೊನೆ(&strm);
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
ಪೂರ್ಣಾಂಕ inf(FILE *ಮೂಲ, FILE *dest)
{
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ have;
    z_ಹರಿವು strm;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಒಳ[CHUNK];
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಹೊರಹೋಗು[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit(&strm);
    ಆದರೆ (ret != Z_OK)
        ಹಿಂತಿರುಗಿ ret;

    /* decompress until deflate stream ends or end of file */
    ಮಾಡು {
        strm.avail_ಒಳ = fread(ಒಳ, 1, CHUNK, ಮೂಲ);
        ಆದರೆ (ferror(ಮೂಲ)) {
            (ಶೂನ್ಯ)inflate_ಕೊನೆ(&strm);
            ಹಿಂತಿರುಗಿ Z_ERRNO;
        }
        ಆದರೆ (strm.avail_ಒಳ == 0)
            ನಿಲ್ಲಿಸು;
        strm.ಮುಂದಿನ_ಒಳ = ಒಳ;

        /* run inflate() on input until output buffer not full */
        ಮಾಡು {
            strm.avail_ಹೊರಹೋಗು = CHUNK;
            strm.ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
            ret = inflate(&strm, Z_NO_FLUSH);
            ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);  /* state not clobbered */
            ಆಯ್ಕೆ (ret) {
            ಸ್ಥಿತಿ Z_NEED_DICT:
                ret = Z_ದತ್ತಾಂಶ_ERROR;     /* and fall through */
            ಸ್ಥಿತಿ Z_ದತ್ತಾಂಶ_ERROR:
            ಸ್ಥಿತಿ Z_MEM_ERROR:
                (ಶೂನ್ಯ)inflate_ಕೊನೆ(&strm);
                ಹಿಂತಿರುಗಿ ret;
            }
            have = CHUNK - strm.avail_ಹೊರಹೋಗು;
            ಆದರೆ (fwrite(ಹೊರಹೋಗು, 1, have, dest) != have || ferror(dest)) {
                (ಶೂನ್ಯ)inflate_ಕೊನೆ(&strm);
                ಹಿಂತಿರುಗಿ Z_ERRNO;
            }
        } ಆಗ (strm.avail_ಹೊರಹೋಗು == 0);

        /* done when inflate() says it's done */
    } ಆಗ (ret != Z_ಹರಿವು_ಕೊನೆ);

    /* clean up and return */
    (ಶೂನ್ಯ)inflate_ಕೊನೆ(&strm);
    ಹಿಂತಿರುಗಿ ret == Z_ಹರಿವು_ಕೊನೆ ? Z_OK : Z_ದತ್ತಾಂಶ_ERROR;
}

/* report a zlib or i/o error */
ಶೂನ್ಯ zerr(ಪೂರ್ಣಾಂಕ ret)
{
    fputs("zpipe: ", stderr);
    ಆಯ್ಕೆ (ret) {
    ಸ್ಥಿತಿ Z_ERRNO:
        ಆದರೆ (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        ಆದರೆ (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Z_ಹರಿವು_ERROR:
        fputs("invalid compression level\n", stderr);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Z_ದತ್ತಾಂಶ_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Z_ಆವೃತ್ತಿ_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
    ಪೂರ್ಣಾಂಕ ret;

    /* avoid end-of-line conversions */
    ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(stdin);
    ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(stdout);

    /* do compression if no arguments */
    ಆದರೆ (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        ಆದರೆ (ret != Z_OK)
            zerr(ret);
        ಹಿಂತಿರುಗಿ ret;
    }

    /* do decompression if -d specified */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        ಆದರೆ (ret != Z_OK)
            zerr(ret);
        ಹಿಂತಿರುಗಿ ret;
    }

    /* otherwise, report usage */
    ಇಲ್ಲದಿದ್ದರೆ {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        ಹಿಂತಿರುಗಿ 1;
    }
}
