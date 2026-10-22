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

#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <assert.h>
#ضمن "zlib.h"

#إذا defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define مجموعه_دودویی_نما(file) _setmode(_fileno(file), O_دودویی_2)
#وإلا
#  define مجموعه_دودویی_نما(file)
#پایان_اگر

#تعریف_کن CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
صحيح def(FILE *منبع, FILE *dest, صحيح level)
{
    صحيح ret, flush;
    بیعلامت have;
    z_جریان strm;
    بیعلامت حرف درون[CHUNK];
    بیعلامت حرف بیرون[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    إذا (ret != Z_OK)
        أرجع ret;

    /* compress until end of file */
    افعل {
        strm.avail_درون = fread(درون, 1, CHUNK, منبع);
        إذا (ferror(منبع)) {
            (فراغ)deflate_انتها(&strm);
            أرجع Z_ERRNO;
        }
        flush = feof(منبع) ? Z_پایان : Z_NO_FLUSH;
        strm.بعدی_درون = درون;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        افعل {
            strm.avail_بیرون = CHUNK;
            strm.بعدی_بیرون = بیرون;
            ret = deflate(&strm, flush);    /* no bad return value */
            أكد(ret != Z_جریان_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_بیرون;
            إذا (fwrite(بیرون, 1, have, dest) != have || ferror(dest)) {
                (فراغ)deflate_انتها(&strm);
                أرجع Z_ERRNO;
            }
        } طالما (strm.avail_بیرون == 0);
        أكد(strm.avail_درون == 0);     /* all input will be used */

        /* done when last data in file processed */
    } طالما (flush != Z_پایان);
    أكد(ret == Z_جریان_انتها);        /* stream will be complete */

    /* clean up and return */
    (فراغ)deflate_انتها(&strm);
    أرجع Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
صحيح inf(FILE *منبع, FILE *dest)
{
    صحيح ret;
    بیعلامت have;
    z_جریان strm;
    بیعلامت حرف درون[CHUNK];
    بیعلامت حرف بیرون[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_درون = 0;
    strm.بعدی_درون = Z_NULL;
    ret = inflateInit(&strm);
    إذا (ret != Z_OK)
        أرجع ret;

    /* decompress until deflate stream ends or end of file */
    افعل {
        strm.avail_درون = fread(درون, 1, CHUNK, منبع);
        إذا (ferror(منبع)) {
            (فراغ)inflate_انتها(&strm);
            أرجع Z_ERRNO;
        }
        إذا (strm.avail_درون == 0)
            اكسر;
        strm.بعدی_درون = درون;

        /* run inflate() on input until output buffer not full */
        افعل {
            strm.avail_بیرون = CHUNK;
            strm.بعدی_بیرون = بیرون;
            ret = inflate(&strm, Z_NO_FLUSH);
            أكد(ret != Z_جریان_ERROR);  /* state not clobbered */
            بدل (ret) {
            حالة Z_NEED_DICT:
                ret = Z_داده_ERROR;     /* and fall through */
            حالة Z_داده_ERROR:
            حالة Z_MEM_ERROR:
                (فراغ)inflate_انتها(&strm);
                أرجع ret;
            }
            have = CHUNK - strm.avail_بیرون;
            إذا (fwrite(بیرون, 1, have, dest) != have || ferror(dest)) {
                (فراغ)inflate_انتها(&strm);
                أرجع Z_ERRNO;
            }
        } طالما (strm.avail_بیرون == 0);

        /* done when inflate() says it's done */
    } طالما (ret != Z_جریان_انتها);

    /* clean up and return */
    (فراغ)inflate_انتها(&strm);
    أرجع ret == Z_جریان_انتها ? Z_OK : Z_داده_ERROR;
}

/* report a zlib or i/o error */
فراغ zerr(صحيح ret)
{
    fputs("zpipe: ", stderr);
    بدل (ret) {
    حالة Z_ERRNO:
        إذا (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        إذا (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        اكسر;
    حالة Z_جریان_ERROR:
        fputs("invalid compression level\n", stderr);
        اكسر;
    حالة Z_داده_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        اكسر;
    حالة Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        اكسر;
    حالة Z_نسخه_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
صحيح اصلی(صحيح argc, حرف **argv)
{
    صحيح ret;

    /* avoid end-of-line conversions */
    مجموعه_دودویی_نما(stdin);
    مجموعه_دودویی_نما(stdout);

    /* do compression if no arguments */
    إذا (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        إذا (ret != Z_OK)
            zerr(ret);
        أرجع ret;
    }

    /* do decompression if -d specified */
    وإلا إذا (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        إذا (ret != Z_OK)
            zerr(ret);
        أرجع ret;
    }

    /* otherwise, report usage */
    وإلا {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        أرجع 1;
    }
}
