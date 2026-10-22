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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

#ਜੇ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file) _setmode(_fileno(file), O_ਦੁਆਧਾਰੀ_2)
#ਨਹੀਂਤਾਂ
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
ਪੂਰਨ def(FILE *ਸਰੋਤ, FILE *dest, ਪੂਰਨ level)
{
    ਪੂਰਨ ret, flush;
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;
    z_ਧਾਰਾ strm;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਵਿੱਚ[CHUNK];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਬਾਹਰਲਾ[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    ਜੇ (ret != Z_OK)
        ਵਾਪਸ ret;

    /* compress until end of file */
    ਕਰੋ {
        strm.avail_ਵਿੱਚ = fread(ਵਿੱਚ, 1, CHUNK, ਸਰੋਤ);
        ਜੇ (ferror(ਸਰੋਤ)) {
            (ਖਾਲੀ)deflate_ਸਿਰਾ(&strm);
            ਵਾਪਸ Z_ERRNO;
        }
        flush = feof(ਸਰੋਤ) ? Z_ਅੰਤ : Z_NO_FLUSH;
        strm.ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        ਕਰੋ {
            strm.avail_ਬਾਹਰਲਾ = CHUNK;
            strm.ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
            ret = deflate(&strm, flush);    /* no bad return value */
            ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_ਬਾਹਰਲਾ;
            ਜੇ (fwrite(ਬਾਹਰਲਾ, 1, have, dest) != have || ferror(dest)) {
                (ਖਾਲੀ)deflate_ਸਿਰਾ(&strm);
                ਵਾਪਸ Z_ERRNO;
            }
        } ਜਦੋਂ (strm.avail_ਬਾਹਰਲਾ == 0);
        ਪ੍ਰਤਿਜ੍ਞਾ(strm.avail_ਵਿੱਚ == 0);     /* all input will be used */

        /* done when last data in file processed */
    } ਜਦੋਂ (flush != Z_ਅੰਤ);
    ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ਸਿਰਾ);        /* stream will be complete */

    /* clean up and return */
    (ਖਾਲੀ)deflate_ਸਿਰਾ(&strm);
    ਵਾਪਸ Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
ਪੂਰਨ inf(FILE *ਸਰੋਤ, FILE *dest)
{
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;
    z_ਧਾਰਾ strm;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਵਿੱਚ[CHUNK];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਬਾਹਰਲਾ[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit(&strm);
    ਜੇ (ret != Z_OK)
        ਵਾਪਸ ret;

    /* decompress until deflate stream ends or end of file */
    ਕਰੋ {
        strm.avail_ਵਿੱਚ = fread(ਵਿੱਚ, 1, CHUNK, ਸਰੋਤ);
        ਜੇ (ferror(ਸਰੋਤ)) {
            (ਖਾਲੀ)inflate_ਸਿਰਾ(&strm);
            ਵਾਪਸ Z_ERRNO;
        }
        ਜੇ (strm.avail_ਵਿੱਚ == 0)
            ਰੋਕੋ;
        strm.ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;

        /* run inflate() on input until output buffer not full */
        ਕਰੋ {
            strm.avail_ਬਾਹਰਲਾ = CHUNK;
            strm.ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
            ret = inflate(&strm, Z_NO_FLUSH);
            ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);  /* state not clobbered */
            ਚੋਣ (ret) {
            ਹਾਲਤ Z_NEED_DICT:
                ret = Z_ਅੰਕੜੇ_ERROR;     /* and fall through */
            ਹਾਲਤ Z_ਅੰਕੜੇ_ERROR:
            ਹਾਲਤ Z_MEM_ERROR:
                (ਖਾਲੀ)inflate_ਸਿਰਾ(&strm);
                ਵਾਪਸ ret;
            }
            have = CHUNK - strm.avail_ਬਾਹਰਲਾ;
            ਜੇ (fwrite(ਬਾਹਰਲਾ, 1, have, dest) != have || ferror(dest)) {
                (ਖਾਲੀ)inflate_ਸਿਰਾ(&strm);
                ਵਾਪਸ Z_ERRNO;
            }
        } ਜਦੋਂ (strm.avail_ਬਾਹਰਲਾ == 0);

        /* done when inflate() says it's done */
    } ਜਦੋਂ (ret != Z_ਧਾਰਾ_ਸਿਰਾ);

    /* clean up and return */
    (ਖਾਲੀ)inflate_ਸਿਰਾ(&strm);
    ਵਾਪਸ ret == Z_ਧਾਰਾ_ਸਿਰਾ ? Z_OK : Z_ਅੰਕੜੇ_ERROR;
}

/* report a zlib or i/o error */
ਖਾਲੀ zerr(ਪੂਰਨ ret)
{
    fputs("zpipe: ", stderr);
    ਚੋਣ (ret) {
    ਹਾਲਤ Z_ERRNO:
        ਜੇ (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        ਜੇ (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        ਰੋਕੋ;
    ਹਾਲਤ Z_ਧਾਰਾ_ERROR:
        fputs("invalid compression level\n", stderr);
        ਰੋਕੋ;
    ਹਾਲਤ Z_ਅੰਕੜੇ_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        ਰੋਕੋ;
    ਹਾਲਤ Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        ਰੋਕੋ;
    ਹਾਲਤ Z_ਸੰਸਕਰਣ_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
    ਪੂਰਨ ret;

    /* avoid end-of-line conversions */
    ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdin);
    ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdout);

    /* do compression if no arguments */
    ਜੇ (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        ਜੇ (ret != Z_OK)
            zerr(ret);
        ਵਾਪਸ ret;
    }

    /* do decompression if -d specified */
    ਨਹੀਂਤਾਂ ਜੇ (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        ਜੇ (ret != Z_OK)
            zerr(ret);
        ਵਾਪਸ ret;
    }

    /* otherwise, report usage */
    ਨਹੀਂਤਾਂ {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        ਵਾਪਸ 1;
    }
}
