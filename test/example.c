/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ଯଦି defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#ଅନ୍ତର୍ଭାଵ "zlib.h"
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ଯଦି defined(VMS)
#  define TESTFILE "foo-gz"
#ନଚେତଯଦି defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#ଅନ୍ୟଥା
#  define TESTFILE "foo.gz"
#endif

#define CHECK_ERR(err, ସନ୍ଦେଶ) { \
    ଯଦି (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", ସନ୍ଦେଶ, err); \
        exit(1); \
    } \
}

ସ୍ଥିର z_const ଅକ୍ଷର hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର dictionary[] = "hello";
ସ୍ଥିର uLong dictId;    /* Adler32 value of the dictionary */

#ifdef Z_SOLO

ସ୍ଥିର ଶୂନ୍ୟ *myalloc(ଶୂନ୍ୟ *q, unsigned n, unsigned m) {
    (ଶୂନ୍ୟ)q;
    ଫେରନ୍ତୁ calloc(n, m);
}

ସ୍ଥିର ଶୂନ୍ୟ myfree(ଶୂନ୍ୟ *q, ଶୂନ୍ୟ *p) {
    (ଶୂନ୍ୟ)q;
    ମୁକ୍ତ(p);
}

ସ୍ଥିର alloc_func zalloc = myalloc;
ସ୍ଥିର free_func zfree = myfree;

#ଅନ୍ୟଥା /* !Z_SOLO */

ସ୍ଥିର alloc_func zalloc = (alloc_func)0;
ସ୍ଥିର free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
ସ୍ଥିର ଶୂନ୍ୟ test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    uLong len = (uLong)strlen(hello)+1;

    err = compress(compr, &comprLen, (ଅଚଳ Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((ଅକ୍ଷର*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    ଯଦି (strcmp((ଅକ୍ଷର*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        exit(1);
    } ଅନ୍ୟଥା {
        printf("uncompress(): %s\n", (ଅକ୍ଷର *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ସ୍ଥିର ଶୂନ୍ୟ test_gzio(ଅଚଳ ଅକ୍ଷର *fname, Byte *uncompr, uLong uncomprLen) {
#ifdef NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#ଅନ୍ୟଥା
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    ଯଦି (file == ଖାଲି) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    gzputc(file, 'h');
    ଯଦି (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        exit(1);
    }
    ଯଦି (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        exit(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    ଯଦି (file == ଖାଲି) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    strcpy((ଅକ୍ଷର*)uncompr, "garbage");

    ଯଦି (gzread(file, uncompr, (unsigned)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        exit(1);
    }
    ଯଦି (strcmp((ଅକ୍ଷର*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (ଅକ୍ଷର*)uncompr);
        exit(1);
    } ଅନ୍ୟଥା {
        printf("gzread(): %s\n", (ଅକ୍ଷର*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    ଯଦି (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (long)pos, (long)gztell(file));
        exit(1);
    }

    ଯଦି (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        exit(1);
    }

    ଯଦି (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        exit(1);
    }

    gzgets(file, (ଅକ୍ଷର*)uncompr, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)uncomprLen);
    ଯଦି (strlen((ଅକ୍ଷର*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        exit(1);
    }
    ଯଦି (strcmp((ଅକ୍ଷର*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        exit(1);
    } ଅନ୍ୟଥା {
        printf("gzgets() after gzseek: %s\n", (ଅକ୍ଷର*)uncompr);
    }

    gzclose(file);
#endif
}

#endif /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ସ୍ଥିର ଶୂନ୍ୟ test_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    uLong len = (uLong)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned ଅକ୍ଷର *)hello;
    c_stream.next_out = compr;

    ଯେତେବେଳେ (c_stream.total_in != len && c_stream.total_out < comprLen) {
        c_stream.avail_in = c_stream.avail_out = 1; /* force small buffers */
        err = deflate(&c_stream, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    ପାଇଁ (;;) {
        c_stream.avail_out = 1;
        err = deflate(&c_stream, Z_FINISH);
        ଯଦି (err == Z_STREAM_END) ଭାଙ୍ଗ;
        CHECK_ERR(err, "deflate");
    }

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ସ୍ଥିର ଶୂନ୍ୟ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    z_stream d_stream; /* decompression stream */

    strcpy((ଅକ୍ଷର*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    ଯେତେବେଳେ (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
        ଯଦି (err == Z_STREAM_END) ଭାଙ୍ଗ;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    ଯଦି (strcmp((ଅକ୍ଷର*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        exit(1);
    } ଅନ୍ୟଥା {
        printf("inflate(): %s\n", (ଅକ୍ଷର *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ସ୍ଥିର ଶୂନ୍ୟ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_stream c_stream; /* compression stream */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

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
    ଯଦି (c_stream.avail_in != 0) {
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
    ଯଦି (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ସ୍ଥିର ଶୂନ୍ୟ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    z_stream d_stream; /* decompression stream */

    strcpy((ଅକ୍ଷର*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    ପାଇଁ (;;) {
        d_stream.next_out = uncompr;            /* discard the output */
        d_stream.avail_out = (uInt)uncomprLen;
        err = inflate(&d_stream, Z_NO_FLUSH);
        ଯଦି (err == Z_STREAM_END) ଭାଙ୍ଗ;
        CHECK_ERR(err, "large inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    ଯଦି (d_stream.total_out != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_stream.total_out);
        exit(1);
    } ଅନ୍ୟଥା {
        printf("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ସ୍ଥିର ଶୂନ୍ୟ test_flush(Byte *compr, uLong *comprLen) {
    z_stream c_stream; /* compression stream */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    uInt len = (uInt)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned ଅକ୍ଷର *)hello;
    c_stream.next_out = compr;
    c_stream.avail_in = 3;
    c_stream.avail_out = (uInt)*comprLen;
    err = deflate(&c_stream, Z_FULL_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_stream.avail_in = len - 3;

    err = deflate(&c_stream, Z_FINISH);
    ଯଦି (err != Z_STREAM_END) {
        CHECK_ERR(err, "deflate");
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_stream.total_out;
}

/* ===========================================================================
 * Test inflateSync()
 */
ସ୍ଥିର ଶୂନ୍ୟ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    z_stream d_stream; /* decompression stream */

    strcpy((ଅକ୍ଷର*)uncompr, "garbage");

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
    ଯଦି (err != Z_STREAM_END) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    printf("after inflateSync(): hel%s\n", (ଅକ୍ଷର *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ସ୍ଥିର ଶୂନ୍ୟ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflateSetDictionary(&c_stream,
                (ଅଚଳ Bytef*)dictionary, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_stream.adler;
    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    c_stream.next_in = (z_const unsigned ଅକ୍ଷର *)hello;
    c_stream.avail_in = (uInt)strlen(hello)+1;

    err = deflate(&c_stream, Z_FINISH);
    ଯଦି (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ସ୍ଥିର ଶୂନ୍ୟ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
    z_stream d_stream; /* decompression stream */

    strcpy((ଅକ୍ଷର*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    ପାଇଁ (;;) {
        err = inflate(&d_stream, Z_NO_FLUSH);
        ଯଦି (err == Z_STREAM_END) ଭାଙ୍ଗ;
        ଯଦି (err == Z_NEED_DICT) {
            ଯଦି (d_stream.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                exit(1);
            }
            err = inflateSetDictionary(&d_stream, (ଅଚଳ Bytef*)dictionary,
                                       (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    ଯଦି (strcmp((ଅକ୍ଷର*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        exit(1);
    } ଅନ୍ୟଥା {
        printf("inflate with dictionary: %s\n", (ଅକ୍ଷର *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର* myVersion = ZLIB_VERSION;

    ଯଦି (zlibVersion()[0] != myVersion[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        exit(1);

    } ଅନ୍ୟଥା ଯଦି (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
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
    ଯଦି (compr == Z_NULL || uncompr == Z_NULL) {
        printf("out of memory\n");
        exit(1);
    }

#ifdef Z_SOLO
    (ଶୂନ୍ୟ)argc;
    (ଶୂନ୍ୟ)argv;
#ଅନ୍ୟଥା
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

    ମୁକ୍ତ(compr);
    ମୁକ୍ତ(uncompr);

    ଫେରନ୍ତୁ 0;
}
