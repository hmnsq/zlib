/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ಯದಿ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ಯದಿ defined(VMS)
#  define TESTFILE "foo-gz"
#ನೋಚೇದ್ಯದಿ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#ಅನ್ಯಥಾ
#  define TESTFILE "foo.gz"
#endif

#define CHECK_ERR(err, ಸಂದೇಶ) { \
    ಯದಿ (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", ಸಂದೇಶ, err); \
        exit(1); \
    } \
}

ಸ್ಥಿರ z_const ವರ್ಣ hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ dictionary[] = "hello";
ಸ್ಥಿರ uLong dictId;    /* Adler32 value of the dictionary */

#ifdef Z_SOLO

ಸ್ಥಿರ ಶೂನ್ಯ *myalloc(ಶೂನ್ಯ *q, unsigned n, unsigned m) {
    (ಶೂನ್ಯ)q;
    ಪ್ರತ್ಯಾಗಮ calloc(n, m);
}

ಸ್ಥಿರ ಶೂನ್ಯ myfree(ಶೂನ್ಯ *q, ಶೂನ್ಯ *p) {
    (ಶೂನ್ಯ)q;
    ಮುಕ್ತ(p);
}

ಸ್ಥಿರ alloc_func zalloc = myalloc;
ಸ್ಥಿರ free_func zfree = myfree;

#ಅನ್ಯಥಾ /* !Z_SOLO */

ಸ್ಥಿರ alloc_func zalloc = (alloc_func)0;
ಸ್ಥಿರ free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    ಪೂರ್ಣಾಙ್ಕ err;
    uLong len = (uLong)strlen(hello)+1;

    err = compress(compr, &comprLen, (ಧ್ರುವ Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((ವರ್ಣ*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    ಯದಿ (strcmp((ವರ್ಣ*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        exit(1);
    } ಅನ್ಯಥಾ {
        printf("uncompress(): %s\n", (ವರ್ಣ *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_gzio(ಧ್ರುವ ವರ್ಣ *fname, Byte *uncompr, uLong uncomprLen) {
#ifdef NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#ಅನ್ಯಥಾ
    ಪೂರ್ಣಾಙ್ಕ err;
    ಪೂರ್ಣಾಙ್ಕ len = (ಪೂರ್ಣಾಙ್ಕ)strlen(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    ಯದಿ (file == ರಿಕ್ತ) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    gzputc(file, 'h');
    ಯದಿ (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        exit(1);
    }
    ಯದಿ (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        exit(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    ಯದಿ (file == ರಿಕ್ತ) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    strcpy((ವರ್ಣ*)uncompr, "garbage");

    ಯದಿ (gzread(file, uncompr, (unsigned)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        exit(1);
    }
    ಯದಿ (strcmp((ವರ್ಣ*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (ವರ್ಣ*)uncompr);
        exit(1);
    } ಅನ್ಯಥಾ {
        printf("gzread(): %s\n", (ವರ್ಣ*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    ಯದಿ (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (long)pos, (long)gztell(file));
        exit(1);
    }

    ಯದಿ (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        exit(1);
    }

    ಯದಿ (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        exit(1);
    }

    gzgets(file, (ವರ್ಣ*)uncompr, (ಪೂರ್ಣಾಙ್ಕ)uncomprLen);
    ಯದಿ (strlen((ವರ್ಣ*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        exit(1);
    }
    ಯದಿ (strcmp((ವರ್ಣ*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        exit(1);
    } ಅನ್ಯಥಾ {
        printf("gzgets() after gzseek: %s\n", (ವರ್ಣ*)uncompr);
    }

    gzclose(file);
#endif
}

#endif /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    ಪೂರ್ಣಾಙ್ಕ err;
    uLong len = (uLong)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned ವರ್ಣ *)hello;
    c_stream.next_out = compr;

    ಯಾವತ್ (c_stream.total_in != len && c_stream.total_out < comprLen) {
        c_stream.avail_in = c_stream.avail_out = 1; /* force small buffers */
        err = deflate(&c_stream, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    ಪ್ರತಿ (;;) {
        c_stream.avail_out = 1;
        err = deflate(&c_stream, Z_FINISH);
        ಯದಿ (err == Z_STREAM_END) ಭಙ್ಗ;
        CHECK_ERR(err, "deflate");
    }

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    ಪೂರ್ಣಾಙ್ಕ err;
    z_stream d_stream; /* decompression stream */

    strcpy((ವರ್ಣ*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    ಯಾವತ್ (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
        ಯದಿ (err == Z_STREAM_END) ಭಙ್ಗ;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    ಯದಿ (strcmp((ವರ್ಣ*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        exit(1);
    } ಅನ್ಯಥಾ {
        printf("inflate(): %s\n", (ವರ್ಣ *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_stream c_stream; /* compression stream */
    ಪೂರ್ಣಾಙ್ಕ err;

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
    ಯದಿ (c_stream.avail_in != 0) {
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
    ಯದಿ (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    ಪೂರ್ಣಾಙ್ಕ err;
    z_stream d_stream; /* decompression stream */

    strcpy((ವರ್ಣ*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    ಪ್ರತಿ (;;) {
        d_stream.next_out = uncompr;            /* discard the output */
        d_stream.avail_out = (uInt)uncomprLen;
        err = inflate(&d_stream, Z_NO_FLUSH);
        ಯದಿ (err == Z_STREAM_END) ಭಙ್ಗ;
        CHECK_ERR(err, "large inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    ಯದಿ (d_stream.total_out != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_stream.total_out);
        exit(1);
    } ಅನ್ಯಥಾ {
        printf("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_flush(Byte *compr, uLong *comprLen) {
    z_stream c_stream; /* compression stream */
    ಪೂರ್ಣಾಙ್ಕ err;
    uInt len = (uInt)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned ವರ್ಣ *)hello;
    c_stream.next_out = compr;
    c_stream.avail_in = 3;
    c_stream.avail_out = (uInt)*comprLen;
    err = deflate(&c_stream, Z_FULL_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_stream.avail_in = len - 3;

    err = deflate(&c_stream, Z_FINISH);
    ಯದಿ (err != Z_STREAM_END) {
        CHECK_ERR(err, "deflate");
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_stream.total_out;
}

/* ===========================================================================
 * Test inflateSync()
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    ಪೂರ್ಣಾಙ್ಕ err;
    z_stream d_stream; /* decompression stream */

    strcpy((ವರ್ಣ*)uncompr, "garbage");

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
    ಯದಿ (err != Z_STREAM_END) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    printf("after inflateSync(): hel%s\n", (ವರ್ಣ *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    ಪೂರ್ಣಾಙ್ಕ err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflateSetDictionary(&c_stream,
                (ಧ್ರುವ Bytef*)dictionary, (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_stream.adler;
    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    c_stream.next_in = (z_const unsigned ವರ್ಣ *)hello;
    c_stream.avail_in = (uInt)strlen(hello)+1;

    err = deflate(&c_stream, Z_FINISH);
    ಯದಿ (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    ಪೂರ್ಣಾಙ್ಕ err;
    z_stream d_stream; /* decompression stream */

    strcpy((ವರ್ಣ*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    ಪ್ರತಿ (;;) {
        err = inflate(&d_stream, Z_NO_FLUSH);
        ಯದಿ (err == Z_STREAM_END) ಭಙ್ಗ;
        ಯದಿ (err == Z_NEED_DICT) {
            ಯದಿ (d_stream.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                exit(1);
            }
            err = inflateSetDictionary(&d_stream, (ಧ್ರುವ Bytef*)dictionary,
                                       (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    ಯದಿ (strcmp((ವರ್ಣ*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        exit(1);
    } ಅನ್ಯಥಾ {
        printf("inflate with dictionary: %s\n", (ವರ್ಣ *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ* myVersion = ZLIB_VERSION;

    ಯದಿ (zlibVersion()[0] != myVersion[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        exit(1);

    } ಅನ್ಯಥಾ ಯದಿ (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
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
    ಯದಿ (compr == Z_NULL || uncompr == Z_NULL) {
        printf("out of memory\n");
        exit(1);
    }

#ifdef Z_SOLO
    (ಶೂನ್ಯ)argc;
    (ಶೂನ್ಯ)argv;
#ಅನ್ಯಥಾ
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

    ಮುಕ್ತ(compr);
    ಮುಕ್ತ(uncompr);

    ಪ್ರತ್ಯಾಗಮ 0;
}
