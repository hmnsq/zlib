/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#എങ്കിൽ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#അന്തര്ഭാവ "zlib.h"
#അന്തര്ഭാവ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#എങ്കിൽ defined(VMS)
#  define TESTFILE "foo-gz"
#അല്ലെങ്കിലെങ്കിൽ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#അല്ലെങ്കിൽ
#  define TESTFILE "foo.gz"
#endif

#define CHECK_ERR(err, സന്ദേശം) { \
    എങ്കിൽ (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", സന്ദേശം, err); \
        exit(1); \
    } \
}

സ്ഥിതം z_const അക്ഷരം hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

സ്ഥിതം സ്ഥിരം അക്ഷരം dictionary[] = "hello";
സ്ഥിതം uLong dictId;    /* Adler32 value of the dictionary */

#ifdef Z_SOLO

സ്ഥിതം ശൂന്യം *myalloc(ശൂന്യം *q, unsigned n, unsigned m) {
    (ശൂന്യം)q;
    തിരികെ calloc(n, m);
}

സ്ഥിതം ശൂന്യം myfree(ശൂന്യം *q, ശൂന്യം *p) {
    (ശൂന്യം)q;
    സ്വതന്ത്രം(p);
}

സ്ഥിതം alloc_func zalloc = myalloc;
സ്ഥിതം free_func zfree = myfree;

#അല്ലെങ്കിൽ /* !Z_SOLO */

സ്ഥിതം alloc_func zalloc = (alloc_func)0;
സ്ഥിതം free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
സ്ഥിതം ശൂന്യം test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    പൂർണ്ണസംഖ്യ err;
    uLong len = (uLong)strlen(hello)+1;

    err = compress(compr, &comprLen, (സ്ഥിരം Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((അക്ഷരം*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    എങ്കിൽ (strcmp((അക്ഷരം*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        exit(1);
    } അല്ലെങ്കിൽ {
        printf("uncompress(): %s\n", (അക്ഷരം *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
സ്ഥിതം ശൂന്യം test_gzio(സ്ഥിരം അക്ഷരം *fname, Byte *uncompr, uLong uncomprLen) {
#ifdef NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#അല്ലെങ്കിൽ
    പൂർണ്ണസംഖ്യ err;
    പൂർണ്ണസംഖ്യ len = (പൂർണ്ണസംഖ്യ)strlen(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    എങ്കിൽ (file == ശൂന്യ) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    gzputc(file, 'h');
    എങ്കിൽ (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        exit(1);
    }
    എങ്കിൽ (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        exit(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    എങ്കിൽ (file == ശൂന്യ) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    strcpy((അക്ഷരം*)uncompr, "garbage");

    എങ്കിൽ (gzread(file, uncompr, (unsigned)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        exit(1);
    }
    എങ്കിൽ (strcmp((അക്ഷരം*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (അക്ഷരം*)uncompr);
        exit(1);
    } അല്ലെങ്കിൽ {
        printf("gzread(): %s\n", (അക്ഷരം*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    എങ്കിൽ (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (long)pos, (long)gztell(file));
        exit(1);
    }

    എങ്കിൽ (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        exit(1);
    }

    എങ്കിൽ (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        exit(1);
    }

    gzgets(file, (അക്ഷരം*)uncompr, (പൂർണ്ണസംഖ്യ)uncomprLen);
    എങ്കിൽ (strlen((അക്ഷരം*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        exit(1);
    }
    എങ്കിൽ (strcmp((അക്ഷരം*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        exit(1);
    } അല്ലെങ്കിൽ {
        printf("gzgets() after gzseek: %s\n", (അക്ഷരം*)uncompr);
    }

    gzclose(file);
#endif
}

#endif /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
സ്ഥിതം ശൂന്യം test_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    പൂർണ്ണസംഖ്യ err;
    uLong len = (uLong)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned അക്ഷരം *)hello;
    c_stream.next_out = compr;

    ആകുമ്പോൾ (c_stream.total_in != len && c_stream.total_out < comprLen) {
        c_stream.avail_in = c_stream.avail_out = 1; /* force small buffers */
        err = deflate(&c_stream, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    വേണ്ടി (;;) {
        c_stream.avail_out = 1;
        err = deflate(&c_stream, Z_FINISH);
        എങ്കിൽ (err == Z_STREAM_END) നിർത്തുക;
        CHECK_ERR(err, "deflate");
    }

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
സ്ഥിതം ശൂന്യം test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    പൂർണ്ണസംഖ്യ err;
    z_stream d_stream; /* decompression stream */

    strcpy((അക്ഷരം*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    ആകുമ്പോൾ (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
        എങ്കിൽ (err == Z_STREAM_END) നിർത്തുക;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    എങ്കിൽ (strcmp((അക്ഷരം*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        exit(1);
    } അല്ലെങ്കിൽ {
        printf("inflate(): %s\n", (അക്ഷരം *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
സ്ഥിതം ശൂന്യം test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_stream c_stream; /* compression stream */
    പൂർണ്ണസംഖ്യ err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_SPEED);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_stream.next_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    എങ്കിൽ (c_stream.avail_in != 0) {
        fprintf(stderr, "deflate not greedy\n");
        exit(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_stream, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_stream.next_in = compr;
    c_stream.avail_in = (uInt)uncomprLen/2;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_stream, Z_BEST_COMPRESSION, Z_FILTERED);
    c_stream.next_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_stream, Z_FINISH);
    എങ്കിൽ (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
സ്ഥിതം ശൂന്യം test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    പൂർണ്ണസംഖ്യ err;
    z_stream d_stream; /* decompression stream */

    strcpy((അക്ഷരം*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    വേണ്ടി (;;) {
        d_stream.next_out = uncompr;            /* discard the output */
        d_stream.avail_out = (uInt)uncomprLen;
        err = inflate(&d_stream, Z_NO_FLUSH);
        എങ്കിൽ (err == Z_STREAM_END) നിർത്തുക;
        CHECK_ERR(err, "large inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    എങ്കിൽ (d_stream.total_out != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_stream.total_out);
        exit(1);
    } അല്ലെങ്കിൽ {
        printf("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
സ്ഥിതം ശൂന്യം test_flush(Byte *compr, uLong *comprLen) {
    z_stream c_stream; /* compression stream */
    പൂർണ്ണസംഖ്യ err;
    uInt len = (uInt)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned അക്ഷരം *)hello;
    c_stream.next_out = compr;
    c_stream.avail_in = 3;
    c_stream.avail_out = (uInt)*comprLen;
    err = deflate(&c_stream, Z_FULL_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_stream.avail_in = len - 3;

    err = deflate(&c_stream, Z_FINISH);
    എങ്കിൽ (err != Z_STREAM_END) {
        CHECK_ERR(err, "deflate");
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_stream.total_out;
}

/* ===========================================================================
 * Test inflateSync()
 */
സ്ഥിതം ശൂന്യം test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    പൂർണ്ണസംഖ്യ err;
    z_stream d_stream; /* decompression stream */

    strcpy((അക്ഷരം*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 2; /* just read the zlib header */

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    err = inflate(&d_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_stream.avail_in = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_stream);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_stream, Z_FINISH);
    എങ്കിൽ (err != Z_STREAM_END) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    printf("after inflateSync(): hel%s\n", (അക്ഷരം *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
സ്ഥിതം ശൂന്യം test_dict_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    പൂർണ്ണസംഖ്യ err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflateSetDictionary(&c_stream,
                (സ്ഥിരം Bytef*)dictionary, (പൂർണ്ണസംഖ്യ)വലുപ്പം(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_stream.adler;
    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    c_stream.next_in = (z_const unsigned അക്ഷരം *)hello;
    c_stream.avail_in = (uInt)strlen(hello)+1;

    err = deflate(&c_stream, Z_FINISH);
    എങ്കിൽ (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
സ്ഥിതം ശൂന്യം test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    പൂർണ്ണസംഖ്യ err;
    z_stream d_stream; /* decompression stream */

    strcpy((അക്ഷരം*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    വേണ്ടി (;;) {
        err = inflate(&d_stream, Z_NO_FLUSH);
        എങ്കിൽ (err == Z_STREAM_END) നിർത്തുക;
        എങ്കിൽ (err == Z_NEED_DICT) {
            എങ്കിൽ (d_stream.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                exit(1);
            }
            err = inflateSetDictionary(&d_stream, (സ്ഥിരം Bytef*)dictionary,
                                       (പൂർണ്ണസംഖ്യ)വലുപ്പം(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    എങ്കിൽ (strcmp((അക്ഷരം*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        exit(1);
    } അല്ലെങ്കിൽ {
        printf("inflate with dictionary: %s\n", (അക്ഷരം *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    സ്ഥിതം സ്ഥിരം അക്ഷരം* myVersion = ZLIB_VERSION;

    എങ്കിൽ (zlibVersion()[0] != myVersion[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        exit(1);

    } അല്ലെങ്കിൽ എങ്കിൽ (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlibVersion());
    }

    printf("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_VERSION, (unsigned)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)calloc((uInt)comprLen, 1);
    uncompr  = (Byte*)calloc((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    എങ്കിൽ (compr == Z_NULL || uncompr == Z_NULL) {
        printf("out of memory\n");
        exit(1);
    }

#ifdef Z_SOLO
    (ശൂന്യം)argc;
    (ശൂന്യം)argv;
#അല്ലെങ്കിൽ
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#endif

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = 3 * uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    സ്വതന്ത്രം(compr);
    സ്വതന്ത്രം(uncompr);

    തിരികെ 0;
}
