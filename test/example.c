/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#যদি defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#অন্তর্ভাব "zlib.h"
#অন্তর্ভাব <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#যদি defined(VMS)
#  define TESTFILE "foo-gz"
#নতুবা_যদি defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#অন্যথা
#  define TESTFILE "foo.gz"
#endif

#define CHECK_ERR(err, বার্তা) { \
    যদি (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", বার্তা, err); \
        exit(1); \
    } \
}

স্থির z_const অক্ষর hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

স্থির ধ্রুবক অক্ষর dictionary[] = "hello";
স্থির uLong dictId;    /* Adler32 value of the dictionary */

#ifdef Z_SOLO

স্থির শূন্য *myalloc(শূন্য *q, unsigned n, unsigned m) {
    (শূন্য)q;
    ফেরত calloc(n, m);
}

স্থির শূন্য myfree(শূন্য *q, শূন্য *p) {
    (শূন্য)q;
    মুক্ত(p);
}

স্থির alloc_func zalloc = myalloc;
স্থির free_func zfree = myfree;

#অন্যথা /* !Z_SOLO */

স্থির alloc_func zalloc = (alloc_func)0;
স্থির free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
স্থির শূন্য test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    পূর্ণসংখ্যা err;
    uLong len = (uLong)strlen(hello)+1;

    err = compress(compr, &comprLen, (ধ্রুবক Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((অক্ষর*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    যদি (strcmp((অক্ষর*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        exit(1);
    } অন্যথা {
        printf("uncompress(): %s\n", (অক্ষর *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
স্থির শূন্য test_gzio(ধ্রুবক অক্ষর *fname, Byte *uncompr, uLong uncomprLen) {
#ifdef NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#অন্যথা
    পূর্ণসংখ্যা err;
    পূর্ণসংখ্যা len = (পূর্ণসংখ্যা)strlen(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    যদি (file == ফাঁকা) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    gzputc(file, 'h');
    যদি (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        exit(1);
    }
    যদি (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        exit(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    যদি (file == ফাঁকা) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    strcpy((অক্ষর*)uncompr, "garbage");

    যদি (gzread(file, uncompr, (unsigned)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        exit(1);
    }
    যদি (strcmp((অক্ষর*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (অক্ষর*)uncompr);
        exit(1);
    } অন্যথা {
        printf("gzread(): %s\n", (অক্ষর*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    যদি (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (long)pos, (long)gztell(file));
        exit(1);
    }

    যদি (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        exit(1);
    }

    যদি (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        exit(1);
    }

    gzgets(file, (অক্ষর*)uncompr, (পূর্ণসংখ্যা)uncomprLen);
    যদি (strlen((অক্ষর*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        exit(1);
    }
    যদি (strcmp((অক্ষর*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        exit(1);
    } অন্যথা {
        printf("gzgets() after gzseek: %s\n", (অক্ষর*)uncompr);
    }

    gzclose(file);
#endif
}

#endif /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
স্থির শূন্য test_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    পূর্ণসংখ্যা err;
    uLong len = (uLong)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.পরবর্তী_in  = (z_const unsigned অক্ষর *)hello;
    c_stream.পরবর্তী_out = compr;

    যখন (c_stream.যোগ_in != len && c_stream.যোগ_out < comprLen) {
        c_stream.avail_in = c_stream.avail_out = 1; /* force small buffers */
        err = deflate(&c_stream, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    জন্য (;;) {
        c_stream.avail_out = 1;
        err = deflate(&c_stream, Z_FINISH);
        যদি (err == Z_STREAM_END) বিরতি;
        CHECK_ERR(err, "deflate");
    }

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
স্থির শূন্য test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    পূর্ণসংখ্যা err;
    z_stream d_stream; /* decompression stream */

    strcpy((অক্ষর*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.পরবর্তী_in  = compr;
    d_stream.avail_in = 0;
    d_stream.পরবর্তী_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    যখন (d_stream.যোগ_out < uncomprLen && d_stream.যোগ_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
        যদি (err == Z_STREAM_END) বিরতি;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    যদি (strcmp((অক্ষর*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        exit(1);
    } অন্যথা {
        printf("inflate(): %s\n", (অক্ষর *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
স্থির শূন্য test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_stream c_stream; /* compression stream */
    পূর্ণসংখ্যা err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_SPEED);
    CHECK_ERR(err, "deflateInit");

    c_stream.পরবর্তী_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_stream.পরবর্তী_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    যদি (c_stream.avail_in != 0) {
        fprintf(stderr, "deflate not greedy\n");
        exit(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_stream, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_stream.পরবর্তী_in = compr;
    c_stream.avail_in = (uInt)uncomprLen/2;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_stream, Z_BEST_COMPRESSION, Z_FILTERED);
    c_stream.পরবর্তী_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_stream, Z_FINISH);
    যদি (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
স্থির শূন্য test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    পূর্ণসংখ্যা err;
    z_stream d_stream; /* decompression stream */

    strcpy((অক্ষর*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.পরবর্তী_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    জন্য (;;) {
        d_stream.পরবর্তী_out = uncompr;            /* discard the output */
        d_stream.avail_out = (uInt)uncomprLen;
        err = inflate(&d_stream, Z_NO_FLUSH);
        যদি (err == Z_STREAM_END) বিরতি;
        CHECK_ERR(err, "large inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    যদি (d_stream.যোগ_out != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_stream.যোগ_out);
        exit(1);
    } অন্যথা {
        printf("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
স্থির শূন্য test_flush(Byte *compr, uLong *comprLen) {
    z_stream c_stream; /* compression stream */
    পূর্ণসংখ্যা err;
    uInt len = (uInt)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.পরবর্তী_in  = (z_const unsigned অক্ষর *)hello;
    c_stream.পরবর্তী_out = compr;
    c_stream.avail_in = 3;
    c_stream.avail_out = (uInt)*comprLen;
    err = deflate(&c_stream, Z_FULL_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_stream.avail_in = len - 3;

    err = deflate(&c_stream, Z_FINISH);
    যদি (err != Z_STREAM_END) {
        CHECK_ERR(err, "deflate");
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_stream.যোগ_out;
}

/* ===========================================================================
 * Test inflateSync()
 */
স্থির শূন্য test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    পূর্ণসংখ্যা err;
    z_stream d_stream; /* decompression stream */

    strcpy((অক্ষর*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.পরবর্তী_in  = compr;
    d_stream.avail_in = 2; /* just read the zlib header */

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.পরবর্তী_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    err = inflate(&d_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_stream.avail_in = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_stream);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_stream, Z_FINISH);
    যদি (err != Z_STREAM_END) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    printf("after inflateSync(): hel%s\n", (অক্ষর *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
স্থির শূন্য test_dict_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    পূর্ণসংখ্যা err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflateSetDictionary(&c_stream,
                (ধ্রুবক Bytef*)dictionary, (পূর্ণসংখ্যা)আকার(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_stream.adler;
    c_stream.পরবর্তী_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    c_stream.পরবর্তী_in = (z_const unsigned অক্ষর *)hello;
    c_stream.avail_in = (uInt)strlen(hello)+1;

    err = deflate(&c_stream, Z_FINISH);
    যদি (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
স্থির শূন্য test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    পূর্ণসংখ্যা err;
    z_stream d_stream; /* decompression stream */

    strcpy((অক্ষর*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.পরবর্তী_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.পরবর্তী_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    জন্য (;;) {
        err = inflate(&d_stream, Z_NO_FLUSH);
        যদি (err == Z_STREAM_END) বিরতি;
        যদি (err == Z_NEED_DICT) {
            যদি (d_stream.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                exit(1);
            }
            err = inflateSetDictionary(&d_stream, (ধ্রুবক Bytef*)dictionary,
                                       (পূর্ণসংখ্যা)আকার(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    যদি (strcmp((অক্ষর*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        exit(1);
    } অন্যথা {
        printf("inflate with dictionary: %s\n", (অক্ষর *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    স্থির ধ্রুবক অক্ষর* myVersion = ZLIB_VERSION;

    যদি (zlibVersion()[0] != myVersion[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        exit(1);

    } অন্যথা যদি (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
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
    যদি (compr == Z_NULL || uncompr == Z_NULL) {
        printf("out of memory\n");
        exit(1);
    }

#ifdef Z_SOLO
    (শূন্য)argc;
    (শূন্য)argv;
#অন্যথা
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

    মুক্ত(compr);
    মুক্ত(uncompr);

    ফেরত 0;
}
