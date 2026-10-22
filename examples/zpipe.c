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

#जर defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_द्विमान_MODE(file) _setmode(_fileno(file), O_द्विमान_2)
#अन्यथा
#  define SET_द्विमान_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
पूर्णांक def(खाता *source, खाता *dest, पूर्णांक level)
{
    पूर्णांक ret, flush;
    अचिन्हित have;
    z_stream strm;
    अचिन्हित अक्षर in[CHUNK];
    अचिन्हित अक्षर out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    जर (ret != Z_OK)
        परत ret;

    /* compress until end of file */
    करा {
        strm.avail_in = ख_पढ़ो(in, 1, CHUNK, source);
        जर (ख_त्रुटि(source)) {
            (शून्य)deflateEnd(&strm);
            परत Z_ERRNO;
        }
        flush = ख_पूर्ण(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        करा {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            प्रतिज्ञा(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            जर (ख_डालो(out, 1, have, dest) != have || ख_त्रुटि(dest)) {
                (शून्य)deflateEnd(&strm);
                परत Z_ERRNO;
            }
        } पर्यंत (strm.avail_out == 0);
        प्रतिज्ञा(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } पर्यंत (flush != Z_FINISH);
    प्रतिज्ञा(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (शून्य)deflateEnd(&strm);
    परत Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
पूर्णांक inf(खाता *source, खाता *dest)
{
    पूर्णांक ret;
    अचिन्हित have;
    z_stream strm;
    अचिन्हित अक्षर in[CHUNK];
    अचिन्हित अक्षर out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    जर (ret != Z_OK)
        परत ret;

    /* decompress until deflate stream ends or end of file */
    करा {
        strm.avail_in = ख_पढ़ो(in, 1, CHUNK, source);
        जर (ख_त्रुटि(source)) {
            (शून्य)inflateEnd(&strm);
            परत Z_ERRNO;
        }
        जर (strm.avail_in == 0)
            थांबा;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        करा {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            प्रतिज्ञा(ret != Z_STREAM_ERROR);  /* state not clobbered */
            निवड (ret) {
            स्थिती Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            स्थिती Z_DATA_ERROR:
            स्थिती Z_MEM_ERROR:
                (शून्य)inflateEnd(&strm);
                परत ret;
            }
            have = CHUNK - strm.avail_out;
            जर (ख_डालो(out, 1, have, dest) != have || ख_त्रुटि(dest)) {
                (शून्य)inflateEnd(&strm);
                परत Z_ERRNO;
            }
        } पर्यंत (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } पर्यंत (ret != Z_STREAM_END);

    /* clean up and return */
    (शून्य)inflateEnd(&strm);
    परत ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
शून्य zerr(पूर्णांक ret)
{
    ख_माला_दो("zpipe: ", मानक_त्रुटि);
    निवड (ret) {
    स्थिती Z_ERRNO:
        जर (ख_त्रुटि(मानक_निवेश))
            ख_माला_दो("error reading stdin\n", मानक_त्रुटि);
        जर (ख_त्रुटि(मानक_निकास))
            ख_माला_दो("error writing stdout\n", मानक_त्रुटि);
        थांबा;
    स्थिती Z_STREAM_ERROR:
        ख_माला_दो("invalid compression level\n", मानक_त्रुटि);
        थांबा;
    स्थिती Z_DATA_ERROR:
        ख_माला_दो("invalid or incomplete deflate data\n", मानक_त्रुटि);
        थांबा;
    स्थिती Z_MEM_ERROR:
        ख_माला_दो("out of memory\n", मानक_त्रुटि);
        थांबा;
    स्थिती Z_VERSION_ERROR:
        ख_माला_दो("zlib version mismatch!\n", मानक_त्रुटि);
    }
}

/* compress or decompress from stdin to stdout */
पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
    पूर्णांक ret;

    /* avoid end-of-line conversions */
    SET_द्विमान_MODE(मानक_निवेश);
    SET_द्विमान_MODE(मानक_निकास);

    /* do compression if no arguments */
    जर (argc == 1) {
        ret = def(मानक_निवेश, मानक_निकास, Z_DEFAULT_COMPRESSION);
        जर (ret != Z_OK)
            zerr(ret);
        परत ret;
    }

    /* do decompression if -d specified */
    अन्यथा जर (argc == 2 && म_भेद(argv[1], "-d") == 0) {
        ret = inf(मानक_निवेश, मानक_निकास);
        जर (ret != Z_OK)
            zerr(ret);
        परत ret;
    }

    /* otherwise, report usage */
    अन्यथा {
        ख_माला_दो("zpipe usage: zpipe [-d] < source > dest\n", मानक_त्रुटि);
        परत 1;
    }
}
