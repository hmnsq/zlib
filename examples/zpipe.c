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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ "zlib.h"

#എങ്കിൽ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_ദ്വിമാന_MODE(file) _setmode(_fileno(file), O_ദ്വിമാന_2)
#അല്ലെങ്കിൽ
#  define SET_ദ്വിമാന_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
പൂർണ്ണസംഖ്യ def(FILE *source, FILE *dest, പൂർണ്ണസംഖ്യ level)
{
    പൂർണ്ണസംഖ്യ ret, flush;
    unsigned have;
    z_stream strm;
    unsigned അക്ഷരം in[CHUNK];
    unsigned അക്ഷരം out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    എങ്കിൽ (ret != Z_OK)
        തിരികെ ret;

    /* compress until end of file */
    ചെയ്യുക {
        strm.avail_in = fread(in, 1, CHUNK, source);
        എങ്കിൽ (ferror(source)) {
            (ശൂന്യം)deflateEnd(&strm);
            തിരികെ Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        ചെയ്യുക {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            പ്രതിജ്ഞാ(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            എങ്കിൽ (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (ശൂന്യം)deflateEnd(&strm);
                തിരികെ Z_ERRNO;
            }
        } ആകുമ്പോൾ (strm.avail_out == 0);
        പ്രതിജ്ഞാ(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } ആകുമ്പോൾ (flush != Z_FINISH);
    പ്രതിജ്ഞാ(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (ശൂന്യം)deflateEnd(&strm);
    തിരികെ Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
പൂർണ്ണസംഖ്യ inf(FILE *source, FILE *dest)
{
    പൂർണ്ണസംഖ്യ ret;
    unsigned have;
    z_stream strm;
    unsigned അക്ഷരം in[CHUNK];
    unsigned അക്ഷരം out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    എങ്കിൽ (ret != Z_OK)
        തിരികെ ret;

    /* decompress until deflate stream ends or end of file */
    ചെയ്യുക {
        strm.avail_in = fread(in, 1, CHUNK, source);
        എങ്കിൽ (ferror(source)) {
            (ശൂന്യം)inflateEnd(&strm);
            തിരികെ Z_ERRNO;
        }
        എങ്കിൽ (strm.avail_in == 0)
            നിർത്തുക;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        ചെയ്യുക {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            പ്രതിജ്ഞാ(ret != Z_STREAM_ERROR);  /* state not clobbered */
            തിരഞ്ഞെടുപ്പ് (ret) {
            അവസ്ഥ Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            അവസ്ഥ Z_DATA_ERROR:
            അവസ്ഥ Z_MEM_ERROR:
                (ശൂന്യം)inflateEnd(&strm);
                തിരികെ ret;
            }
            have = CHUNK - strm.avail_out;
            എങ്കിൽ (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (ശൂന്യം)inflateEnd(&strm);
                തിരികെ Z_ERRNO;
            }
        } ആകുമ്പോൾ (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } ആകുമ്പോൾ (ret != Z_STREAM_END);

    /* clean up and return */
    (ശൂന്യം)inflateEnd(&strm);
    തിരികെ ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
ശൂന്യം zerr(പൂർണ്ണസംഖ്യ ret)
{
    fputs("zpipe: ", stderr);
    തിരഞ്ഞെടുപ്പ് (ret) {
    അവസ്ഥ Z_ERRNO:
        എങ്കിൽ (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        എങ്കിൽ (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        നിർത്തുക;
    അവസ്ഥ Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        നിർത്തുക;
    അവസ്ഥ Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        നിർത്തുക;
    അവസ്ഥ Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        നിർത്തുക;
    അവസ്ഥ Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
    പൂർണ്ണസംഖ്യ ret;

    /* avoid end-of-line conversions */
    SET_ദ്വിമാന_MODE(stdin);
    SET_ദ്വിമാന_MODE(stdout);

    /* do compression if no arguments */
    എങ്കിൽ (argc == 1) {
        ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
        എങ്കിൽ (ret != Z_OK)
            zerr(ret);
        തിരികെ ret;
    }

    /* do decompression if -d specified */
    അല്ലെങ്കിൽ എങ്കിൽ (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        എങ്കിൽ (ret != Z_OK)
            zerr(ret);
        തിരികെ ret;
    }

    /* otherwise, report usage */
    അല്ലെങ്കിൽ {
        fputs("zpipe usage: zpipe [-d] < source > dest\n", stderr);
        തിരികെ 1;
    }
}
