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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव "zlib.h"

#यदि defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define समुच्चयः_द्विमान_बहुलकम्(file) _setmode(_fileno(file), O_द्विमान_2)
#अन्यथा
#  define समुच्चयः_द्विमान_बहुलकम्(file)
#यदि_अन्तः

#परिभाषय CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
पूर्णाङ्क def(FILE *source, FILE *dest, पूर्णाङ्क level)
{
    पूर्णाङ्क ret, flush;
    अचिह्नित have;
    z_प्रवाहः strm;
    अचिह्नित वर्ण अन्तर्[CHUNK];
    अचिह्नित वर्ण बहिर्गमनम्[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    यदि (ret != Z_OK)
        प्रत्यागम ret;

    /* compress until end of file */
    कुरु {
        strm.avail_अन्तर् = fread(अन्तर्, 1, CHUNK, source);
        यदि (ferror(source)) {
            (शून्य)deflate_अन्तः(&strm);
            प्रत्यागम Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.अग्र_अन्तर् = अन्तर्;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        कुरु {
            strm.avail_बहिर्गमनम् = CHUNK;
            strm.अग्र_बहिर्गमनम् = बहिर्गमनम्;
            ret = deflate(&strm, flush);    /* no bad return value */
            प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_बहिर्गमनम्;
            यदि (fwrite(बहिर्गमनम्, 1, have, dest) != have || ferror(dest)) {
                (शून्य)deflate_अन्तः(&strm);
                प्रत्यागम Z_ERRNO;
            }
        } यावत् (strm.avail_बहिर्गमनम् == 0);
        प्रतिज्ञा(strm.avail_अन्तर् == 0);     /* all input will be used */

        /* done when last data in file processed */
    } यावत् (flush != Z_FINISH);
    प्रतिज्ञा(ret == Z_प्रवाहः_अन्तः);        /* stream will be complete */

    /* clean up and return */
    (शून्य)deflate_अन्तः(&strm);
    प्रत्यागम Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
पूर्णाङ्क inf(FILE *source, FILE *dest)
{
    पूर्णाङ्क ret;
    अचिह्नित have;
    z_प्रवाहः strm;
    अचिह्नित वर्ण अन्तर्[CHUNK];
    अचिह्नित वर्ण बहिर्गमनम्[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit(&strm);
    यदि (ret != Z_OK)
        प्रत्यागम ret;

    /* decompress until deflate stream ends or end of file */
    कुरु {
        strm.avail_अन्तर् = fread(अन्तर्, 1, CHUNK, source);
        यदि (ferror(source)) {
            (शून्य)inflate_अन्तः(&strm);
            प्रत्यागम Z_ERRNO;
        }
        यदि (strm.avail_अन्तर् == 0)
            भङ्ग;
        strm.अग्र_अन्तर् = अन्तर्;

        /* run inflate() on input until output buffer not full */
        कुरु {
            strm.avail_बहिर्गमनम् = CHUNK;
            strm.अग्र_बहिर्गमनम् = बहिर्गमनम्;
            ret = inflate(&strm, Z_NO_FLUSH);
            प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);  /* state not clobbered */
            विकल्प (ret) {
            पक्ष Z_NEED_DICT:
                ret = Z_दत्त_ERROR;     /* and fall through */
            पक्ष Z_दत्त_ERROR:
            पक्ष Z_MEM_ERROR:
                (शून्य)inflate_अन्तः(&strm);
                प्रत्यागम ret;
            }
            have = CHUNK - strm.avail_बहिर्गमनम्;
            यदि (fwrite(बहिर्गमनम्, 1, have, dest) != have || ferror(dest)) {
                (शून्य)inflate_अन्तः(&strm);
                प्रत्यागम Z_ERRNO;
            }
        } यावत् (strm.avail_बहिर्गमनम् == 0);

        /* done when inflate() says it's done */
    } यावत् (ret != Z_प्रवाहः_अन्तः);

    /* clean up and return */
    (शून्य)inflate_अन्तः(&strm);
    प्रत्यागम ret == Z_प्रवाहः_अन्तः ? Z_OK : Z_दत्त_ERROR;
}

/* report a zlib or i/o error */
शून्य zerr(पूर्णाङ्क ret)
{
    fputs("zpipe: ", stderr);
    विकल्प (ret) {
    पक्ष Z_ERRNO:
        यदि (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        यदि (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        भङ्ग;
    पक्ष Z_प्रवाहः_ERROR:
        fputs("invalid compression level\n", stderr);
        भङ्ग;
    पक्ष Z_दत्त_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        भङ्ग;
    पक्ष Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        भङ्ग;
    पक्ष Z_संस्करणम्_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
    पूर्णाङ्क ret;

    /* avoid end-of-line conversions */
    समुच्चयः_द्विमान_बहुलकम्(stdin);
    समुच्चयः_द्विमान_बहुलकम्(stdout);

    /* do compression if no arguments */
    यदि (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        यदि (ret != Z_OK)
            zerr(ret);
        प्रत्यागम ret;
    }

    /* do decompression if -d specified */
    अन्यथा यदि (argc == 2 && वाक्यं_तुलय(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        यदि (ret != Z_OK)
            zerr(ret);
        प्रत्यागम ret;
    }

    /* otherwise, report usage */
    अन्यथा {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        प्रत्यागम 1;
    }
}
