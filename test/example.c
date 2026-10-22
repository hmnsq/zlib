/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#پایان_اگر

#ضمن "zlib.h"
#ضمن <stdio.h>

#اگر_تعریف_شده STDC
#  include <string.h>
#  include <stdlib.h>
#پایان_اگر

#إذا defined(VMS)
#  define TESTFILE "foo-gz"
#وإلاإذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#وإلا
#  define TESTFILE "foo.gz"
#پایان_اگر

#تعریف_کن CHECK_ERR(err, پیام) { \
    إذا (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", پیام, err); \
        خروج(1); \
    } \
}

ساكن z_const حرف hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ساكن ثابت حرف dictionary[] = "hello";
ساكن uLong dictId;    /* Adler32 value of the dictionary */

#اگر_تعریف_شده Z_SOLO

ساكن فراغ *myalloc(فراغ *q, بیعلامت n, بیعلامت m) {
    (فراغ)q;
    أرجع حافظه_صفر_بده(n, m);
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
    uLong len = (uLong)طول_رشته(hello)+1;

    err = compress(compr, &comprLen, (ثابت Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((حرف*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        خروج(1);
    } وإلا {
        چاپ_کن("uncompress(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ساكن فراغ test_gzio(ثابت حرف *fname, Byte *uncompr, uLong uncomprLen) {
#اگر_تعریف_شده NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#وإلا
    صحيح err;
    صحيح len = (صحيح)طول_رشته(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        خروج(1);
    }
    gzputc(file, 'h');
    إذا (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        خروج(1);
    }
    إذا (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        خروج(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        خروج(1);
    }
    strcpy((حرف*)uncompr, "garbage");

    إذا (gzread(file, uncompr, (بیعلامت)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        خروج(1);
    }
    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (حرف*)uncompr);
        خروج(1);
    } وإلا {
        چاپ_کن("gzread(): %s\n", (حرف*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    إذا (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (دراز)pos, (دراز)gztell(file));
        خروج(1);
    }

    إذا (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        خروج(1);
    }

    إذا (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        خروج(1);
    }

    gzgets(file, (حرف*)uncompr, (صحيح)uncomprLen);
    إذا (طول_رشته((حرف*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        خروج(1);
    }
    إذا (strcmp((حرف*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        خروج(1);
    } وإلا {
        چاپ_کن("gzgets() after gzseek: %s\n", (حرف*)uncompr);
    }

    gzclose(file);
#پایان_اگر
}

#پایان_اگر /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ساكن فراغ test_deflate(Byte *compr, uLong comprLen) {
    z_جریان c_جریان; /* compression stream */
    صحيح err;
    uLong len = (uLong)طول_رشته(hello)+1;

    c_جریان.zalloc = zalloc;
    c_جریان.zfree = zfree;
    c_جریان.opaque = (voidpf)0;

    err = deflateInit(&c_جریان, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_جریان.بعدی_درون  = (z_const بیعلامت حرف *)hello;
    c_جریان.بعدی_بیرون = compr;

    طالما (c_جریان.مجموع_درون != len && c_جریان.مجموع_بیرون < comprLen) {
        c_جریان.avail_درون = c_جریان.avail_بیرون = 1; /* force small buffers */
        err = deflate(&c_جریان, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    لكل (;;) {
        c_جریان.avail_بیرون = 1;
        err = deflate(&c_جریان, Z_پایان);
        إذا (err == Z_جریان_انتها) اكسر;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_انتها(&c_جریان);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ساكن فراغ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    صحيح err;
    z_جریان d_جریان; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_جریان.zalloc = zalloc;
    d_جریان.zfree = zfree;
    d_جریان.opaque = (voidpf)0;

    d_جریان.بعدی_درون  = compr;
    d_جریان.avail_درون = 0;
    d_جریان.بعدی_بیرون = uncompr;

    err = inflateInit(&d_جریان);
    CHECK_ERR(err, "inflateInit");

    طالما (d_جریان.مجموع_بیرون < uncomprLen && d_جریان.مجموع_درون < comprLen) {
        d_جریان.avail_درون = d_جریان.avail_بیرون = 1; /* force small buffers */
        err = inflate(&d_جریان, Z_NO_FLUSH);
        إذا (err == Z_جریان_انتها) اكسر;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_انتها(&d_جریان);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        خروج(1);
    } وإلا {
        چاپ_کن("inflate(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ساكن فراغ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_جریان c_جریان; /* compression stream */
    صحيح err;

    c_جریان.zalloc = zalloc;
    c_جریان.zfree = zfree;
    c_جریان.opaque = (voidpf)0;

    err = deflateInit(&c_جریان, Z_بهترین_سرعت);
    CHECK_ERR(err, "deflateInit");

    c_جریان.بعدی_بیرون = compr;
    c_جریان.avail_بیرون = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_جریان.بعدی_درون = uncompr;
    c_جریان.avail_درون = (uInt)uncomprLen;
    err = deflate(&c_جریان, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    إذا (c_جریان.avail_درون != 0) {
        fprintf(stderr, "deflate not greedy\n");
        خروج(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_جریان, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_جریان.بعدی_درون = compr;
    c_جریان.avail_درون = (uInt)uncomprLen/2;
    err = deflate(&c_جریان, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_جریان, Z_بهترین_COMPRESSION, Z_FILTERED);
    c_جریان.بعدی_درون = uncompr;
    c_جریان.avail_درون = (uInt)uncomprLen;
    err = deflate(&c_جریان, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_جریان, Z_پایان);
    إذا (err != Z_جریان_انتها) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        خروج(1);
    }
    err = deflate_انتها(&c_جریان);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ساكن فراغ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    صحيح err;
    z_جریان d_جریان; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_جریان.zalloc = zalloc;
    d_جریان.zfree = zfree;
    d_جریان.opaque = (voidpf)0;

    d_جریان.بعدی_درون  = compr;
    d_جریان.avail_درون = (uInt)comprLen;

    err = inflateInit(&d_جریان);
    CHECK_ERR(err, "inflateInit");

    لكل (;;) {
        d_جریان.بعدی_بیرون = uncompr;            /* discard the output */
        d_جریان.avail_بیرون = (uInt)uncomprLen;
        err = inflate(&d_جریان, Z_NO_FLUSH);
        إذا (err == Z_جریان_انتها) اكسر;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_انتها(&d_جریان);
    CHECK_ERR(err, "inflateEnd");

    إذا (d_جریان.مجموع_بیرون != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_جریان.مجموع_بیرون);
        خروج(1);
    } وإلا {
        چاپ_کن("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ساكن فراغ test_flush(Byte *compr, uLong *comprLen) {
    z_جریان c_جریان; /* compression stream */
    صحيح err;
    uInt len = (uInt)طول_رشته(hello)+1;

    c_جریان.zalloc = zalloc;
    c_جریان.zfree = zfree;
    c_جریان.opaque = (voidpf)0;

    err = deflateInit(&c_جریان, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_جریان.بعدی_درون  = (z_const بیعلامت حرف *)hello;
    c_جریان.بعدی_بیرون = compr;
    c_جریان.avail_درون = 3;
    c_جریان.avail_بیرون = (uInt)*comprLen;
    err = deflate(&c_جریان, Z_پر_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_جریان.avail_درون = len - 3;

    err = deflate(&c_جریان, Z_پایان);
    إذا (err != Z_جریان_انتها) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_انتها(&c_جریان);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_جریان.مجموع_بیرون;
}

/* ===========================================================================
 * Test inflateSync()
 */
ساكن فراغ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    صحيح err;
    z_جریان d_جریان; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_جریان.zalloc = zalloc;
    d_جریان.zfree = zfree;
    d_جریان.opaque = (voidpf)0;

    d_جریان.بعدی_درون  = compr;
    d_جریان.avail_درون = 2; /* just read the zlib header */

    err = inflateInit(&d_جریان);
    CHECK_ERR(err, "inflateInit");

    d_جریان.بعدی_بیرون = uncompr;
    d_جریان.avail_بیرون = (uInt)uncomprLen;

    err = inflate(&d_جریان, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_جریان.avail_درون = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_جریان);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_جریان, Z_پایان);
    إذا (err != Z_جریان_انتها) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        خروج(1);
    }
    err = inflate_انتها(&d_جریان);
    CHECK_ERR(err, "inflateEnd");

    چاپ_کن("after inflateSync(): hel%s\n", (حرف *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ساكن فراغ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_جریان c_جریان; /* compression stream */
    صحيح err;

    c_جریان.zalloc = zalloc;
    c_جریان.zfree = zfree;
    c_جریان.opaque = (voidpf)0;

    err = deflateInit(&c_جریان, Z_بهترین_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_مجموعه_Dictionary(&c_جریان,
                (ثابت Bytef*)dictionary, (صحيح)حجم(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_جریان.adler;
    c_جریان.بعدی_بیرون = compr;
    c_جریان.avail_بیرون = (uInt)comprLen;

    c_جریان.بعدی_درون = (z_const بیعلامت حرف *)hello;
    c_جریان.avail_درون = (uInt)طول_رشته(hello)+1;

    err = deflate(&c_جریان, Z_پایان);
    إذا (err != Z_جریان_انتها) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        خروج(1);
    }
    err = deflate_انتها(&c_جریان);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ساكن فراغ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    صحيح err;
    z_جریان d_جریان; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_جریان.zalloc = zalloc;
    d_جریان.zfree = zfree;
    d_جریان.opaque = (voidpf)0;

    d_جریان.بعدی_درون  = compr;
    d_جریان.avail_درون = (uInt)comprLen;

    err = inflateInit(&d_جریان);
    CHECK_ERR(err, "inflateInit");

    d_جریان.بعدی_بیرون = uncompr;
    d_جریان.avail_بیرون = (uInt)uncomprLen;

    لكل (;;) {
        err = inflate(&d_جریان, Z_NO_FLUSH);
        إذا (err == Z_جریان_انتها) اكسر;
        إذا (err == Z_NEED_DICT) {
            إذا (d_جریان.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                خروج(1);
            }
            err = inflate_مجموعه_Dictionary(&d_جریان, (ثابت Bytef*)dictionary,
                                       (صحيح)حجم(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_انتها(&d_جریان);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        خروج(1);
    } وإلا {
        چاپ_کن("inflate with dictionary: %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

صحيح اصلی(صحيح argc, حرف *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ساكن ثابت حرف* my_نسخه_2 = ZLIB_نسخه;

    إذا (zlib_نسخه()[0] != my_نسخه_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        خروج(1);

    } وإلا إذا (strcmp(zlib_نسخه(), ZLIB_نسخه) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_نسخه());
    }

    چاپ_کن("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_نسخه, (بیعلامت)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)حافظه_صفر_بده((uInt)comprLen, 1);
    uncompr  = (Byte*)حافظه_صفر_بده((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    إذا (compr == Z_NULL || uncompr == Z_NULL) {
        چاپ_کن("out of memory\n");
        خروج(1);
    }

#اگر_تعریف_شده Z_SOLO
    (فراغ)argc;
    (فراغ)argv;
#وإلا
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#پایان_اگر

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
