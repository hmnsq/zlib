/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#د_که_پای

#ضمن "zlib.h"
#ضمن <stdio.h>

#که_تعریف_شوی STDC
#  include <string.h>
#  include <stdlib.h>
#د_که_پای

#إذا defined(VMS)
#  define TESTFILE "foo-gz"
#وإلاإذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#وإلا
#  define TESTFILE "foo.gz"
#د_که_پای

#تعریف_کړه CHECK_ERR(err, پیغام) { \
    إذا (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", پیغام, err); \
        وتل(1); \
    } \
}

ساكن z_const حرف hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ساكن ثابت حرف dictionary[] = "hello";
ساكن uLong dictId;    /* Adler32 value of the dictionary */

#که_تعریف_شوی Z_SOLO

ساكن فراغ *myalloc(فراغ *q, بینښه n, بینښه m) {
    (فراغ)q;
    أرجع حافظه_صفر_ورکړه(n, m);
}

ساكن فراغ myfree(فراغ *q, فراغ *p) {
    (فراغ)q;
    حرر(p);
}

ساكن alloc_func zalloc = myalloc;
ساكن free_func zfree = myfree;

#وإلا /* !Z_SOLO */

ساكن alloc_func zalloc = (alloc_func)0;
ساكن free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
ساكن فراغ test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    صحيح err;
    uLong len = (uLong)مزي_اوږدوالی(hello)+1;

    err = compress(compr, &comprLen, (ثابت Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((حرف*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        وتل(1);
    } وإلا {
        چاپ_کړه("uncompress(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ساكن فراغ test_gzio(ثابت حرف *fname, Byte *uncompr, uLong uncomprLen) {
#که_تعریف_شوی NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#وإلا
    صحيح err;
    صحيح len = (صحيح)مزي_اوږدوالی(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        وتل(1);
    }
    gzputc(file, 'h');
    إذا (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        وتل(1);
    }
    إذا (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        وتل(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        وتل(1);
    }
    strcpy((حرف*)uncompr, "garbage");

    إذا (gzread(file, uncompr, (بینښه)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        وتل(1);
    }
    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (حرف*)uncompr);
        وتل(1);
    } وإلا {
        چاپ_کړه("gzread(): %s\n", (حرف*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    إذا (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (اوږد)pos, (اوږد)gztell(file));
        وتل(1);
    }

    إذا (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        وتل(1);
    }

    إذا (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        وتل(1);
    }

    gzgets(file, (حرف*)uncompr, (صحيح)uncomprLen);
    إذا (مزي_اوږدوالی((حرف*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        وتل(1);
    }
    إذا (strcmp((حرف*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        وتل(1);
    } وإلا {
        چاپ_کړه("gzgets() after gzseek: %s\n", (حرف*)uncompr);
    }

    gzclose(file);
#د_که_پای
}

#د_که_پای /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ساكن فراغ test_deflate(Byte *compr, uLong comprLen) {
    z_بهیر c_بهیر; /* compression stream */
    صحيح err;
    uLong len = (uLong)مزي_اوږدوالی(hello)+1;

    c_بهیر.zalloc = zalloc;
    c_بهیر.zfree = zfree;
    c_بهیر.opaque = (voidpf)0;

    err = deflateInit(&c_بهیر, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_بهیر.بل_دننه  = (z_const بینښه حرف *)hello;
    c_بهیر.بل_بهر = compr;

    طالما (c_بهیر.ټول_دننه != len && c_بهیر.ټول_بهر < comprLen) {
        c_بهیر.avail_دننه = c_بهیر.avail_بهر = 1; /* force small buffers */
        err = deflate(&c_بهیر, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    لكل (;;) {
        c_بهیر.avail_بهر = 1;
        err = deflate(&c_بهیر, Z_پای);
        إذا (err == Z_بهیر_پای_ټکی) اكسر;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_پای_ټکی(&c_بهیر);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ساكن فراغ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    صحيح err;
    z_بهیر d_بهیر; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_بهیر.zalloc = zalloc;
    d_بهیر.zfree = zfree;
    d_بهیر.opaque = (voidpf)0;

    d_بهیر.بل_دننه  = compr;
    d_بهیر.avail_دننه = 0;
    d_بهیر.بل_بهر = uncompr;

    err = inflateInit(&d_بهیر);
    CHECK_ERR(err, "inflateInit");

    طالما (d_بهیر.ټول_بهر < uncomprLen && d_بهیر.ټول_دننه < comprLen) {
        d_بهیر.avail_دننه = d_بهیر.avail_بهر = 1; /* force small buffers */
        err = inflate(&d_بهیر, Z_NO_FLUSH);
        إذا (err == Z_بهیر_پای_ټکی) اكسر;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_پای_ټکی(&d_بهیر);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        وتل(1);
    } وإلا {
        چاپ_کړه("inflate(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ساكن فراغ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_بهیر c_بهیر; /* compression stream */
    صحيح err;

    c_بهیر.zalloc = zalloc;
    c_بهیر.zfree = zfree;
    c_بهیر.opaque = (voidpf)0;

    err = deflateInit(&c_بهیر, Z_غوره_چټکتیا);
    CHECK_ERR(err, "deflateInit");

    c_بهیر.بل_بهر = compr;
    c_بهیر.avail_بهر = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_بهیر.بل_دننه = uncompr;
    c_بهیر.avail_دننه = (uInt)uncomprLen;
    err = deflate(&c_بهیر, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    إذا (c_بهیر.avail_دننه != 0) {
        fprintf(stderr, "deflate not greedy\n");
        وتل(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_بهیر, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_بهیر.بل_دننه = compr;
    c_بهیر.avail_دننه = (uInt)uncomprLen/2;
    err = deflate(&c_بهیر, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_بهیر, Z_غوره_COMPRESSION, Z_FILTERED);
    c_بهیر.بل_دننه = uncompr;
    c_بهیر.avail_دننه = (uInt)uncomprLen;
    err = deflate(&c_بهیر, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_بهیر, Z_پای);
    إذا (err != Z_بهیر_پای_ټکی) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        وتل(1);
    }
    err = deflate_پای_ټکی(&c_بهیر);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ساكن فراغ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    صحيح err;
    z_بهیر d_بهیر; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_بهیر.zalloc = zalloc;
    d_بهیر.zfree = zfree;
    d_بهیر.opaque = (voidpf)0;

    d_بهیر.بل_دننه  = compr;
    d_بهیر.avail_دننه = (uInt)comprLen;

    err = inflateInit(&d_بهیر);
    CHECK_ERR(err, "inflateInit");

    لكل (;;) {
        d_بهیر.بل_بهر = uncompr;            /* discard the output */
        d_بهیر.avail_بهر = (uInt)uncomprLen;
        err = inflate(&d_بهیر, Z_NO_FLUSH);
        إذا (err == Z_بهیر_پای_ټکی) اكسر;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_پای_ټکی(&d_بهیر);
    CHECK_ERR(err, "inflateEnd");

    إذا (d_بهیر.ټول_بهر != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_بهیر.ټول_بهر);
        وتل(1);
    } وإلا {
        چاپ_کړه("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ساكن فراغ test_flush(Byte *compr, uLong *comprLen) {
    z_بهیر c_بهیر; /* compression stream */
    صحيح err;
    uInt len = (uInt)مزي_اوږدوالی(hello)+1;

    c_بهیر.zalloc = zalloc;
    c_بهیر.zfree = zfree;
    c_بهیر.opaque = (voidpf)0;

    err = deflateInit(&c_بهیر, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_بهیر.بل_دننه  = (z_const بینښه حرف *)hello;
    c_بهیر.بل_بهر = compr;
    c_بهیر.avail_دننه = 3;
    c_بهیر.avail_بهر = (uInt)*comprLen;
    err = deflate(&c_بهیر, Z_ډک_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_بهیر.avail_دننه = len - 3;

    err = deflate(&c_بهیر, Z_پای);
    إذا (err != Z_بهیر_پای_ټکی) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_پای_ټکی(&c_بهیر);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_بهیر.ټول_بهر;
}

/* ===========================================================================
 * Test inflateSync()
 */
ساكن فراغ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    صحيح err;
    z_بهیر d_بهیر; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_بهیر.zalloc = zalloc;
    d_بهیر.zfree = zfree;
    d_بهیر.opaque = (voidpf)0;

    d_بهیر.بل_دننه  = compr;
    d_بهیر.avail_دننه = 2; /* just read the zlib header */

    err = inflateInit(&d_بهیر);
    CHECK_ERR(err, "inflateInit");

    d_بهیر.بل_بهر = uncompr;
    d_بهیر.avail_بهر = (uInt)uncomprLen;

    err = inflate(&d_بهیر, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_بهیر.avail_دننه = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_بهیر);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_بهیر, Z_پای);
    إذا (err != Z_بهیر_پای_ټکی) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        وتل(1);
    }
    err = inflate_پای_ټکی(&d_بهیر);
    CHECK_ERR(err, "inflateEnd");

    چاپ_کړه("after inflateSync(): hel%s\n", (حرف *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ساكن فراغ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_بهیر c_بهیر; /* compression stream */
    صحيح err;

    c_بهیر.zalloc = zalloc;
    c_بهیر.zfree = zfree;
    c_بهیر.opaque = (voidpf)0;

    err = deflateInit(&c_بهیر, Z_غوره_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_سېټ_Dictionary(&c_بهیر,
                (ثابت Bytef*)dictionary, (صحيح)حجم(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_بهیر.adler;
    c_بهیر.بل_بهر = compr;
    c_بهیر.avail_بهر = (uInt)comprLen;

    c_بهیر.بل_دننه = (z_const بینښه حرف *)hello;
    c_بهیر.avail_دننه = (uInt)مزي_اوږدوالی(hello)+1;

    err = deflate(&c_بهیر, Z_پای);
    إذا (err != Z_بهیر_پای_ټکی) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        وتل(1);
    }
    err = deflate_پای_ټکی(&c_بهیر);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ساكن فراغ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    صحيح err;
    z_بهیر d_بهیر; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_بهیر.zalloc = zalloc;
    d_بهیر.zfree = zfree;
    d_بهیر.opaque = (voidpf)0;

    d_بهیر.بل_دننه  = compr;
    d_بهیر.avail_دننه = (uInt)comprLen;

    err = inflateInit(&d_بهیر);
    CHECK_ERR(err, "inflateInit");

    d_بهیر.بل_بهر = uncompr;
    d_بهیر.avail_بهر = (uInt)uncomprLen;

    لكل (;;) {
        err = inflate(&d_بهیر, Z_NO_FLUSH);
        إذا (err == Z_بهیر_پای_ټکی) اكسر;
        إذا (err == Z_NEED_DICT) {
            إذا (d_بهیر.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                وتل(1);
            }
            err = inflate_سېټ_Dictionary(&d_بهیر, (ثابت Bytef*)dictionary,
                                       (صحيح)حجم(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_پای_ټکی(&d_بهیر);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        وتل(1);
    } وإلا {
        چاپ_کړه("inflate with dictionary: %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

صحيح اصلي(صحيح argc, حرف *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ساكن ثابت حرف* my_نسخه_2 = ZLIB_نسخه;

    إذا (zlib_نسخه()[0] != my_نسخه_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        وتل(1);

    } وإلا إذا (strcmp(zlib_نسخه(), ZLIB_نسخه) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_نسخه());
    }

    چاپ_کړه("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_نسخه, (بینښه)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)حافظه_صفر_ورکړه((uInt)comprLen, 1);
    uncompr  = (Byte*)حافظه_صفر_ورکړه((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    إذا (compr == Z_NULL || uncompr == Z_NULL) {
        چاپ_کړه("out of memory\n");
        وتل(1);
    }

#که_تعریف_شوی Z_SOLO
    (فراغ)argc;
    (فراغ)argv;
#وإلا
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#د_که_پای

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = 3 * uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    حرر(compr);
    حرر(uncompr);

    أرجع 0;
}
