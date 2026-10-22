/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#جے_انت

#ضمن "zlib.h"
#ضمن <stdio.h>

#جے_تعریف_شدہ STDC
#  include <string.h>
#  include <stdlib.h>
#جے_انت

#إذا defined(VMS)
#  define TESTFILE "foo-gz"
#وإلاإذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#وإلا
#  define TESTFILE "foo.gz"
#جے_انت

#تعریف_کرو CHECK_ERR(err, پیغام) { \
    إذا (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", پیغام, err); \
        باہر_نکلو(1); \
    } \
}

ساكن z_const حرف hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ساكن ثابت حرف dictionary[] = "hello";
ساكن uLong dictId;    /* Adler32 value of the dictionary */

#جے_تعریف_شدہ Z_SOLO

ساكن فراغ *myalloc(فراغ *q, بنانشانی n, بنانشانی m) {
    (فراغ)q;
    أرجع صفر_حافظہ_دیو(n, m);
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
    uLong len = (uLong)لڑی_لمبائی(hello)+1;

    err = compress(compr, &comprLen, (ثابت Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((حرف*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        باہر_نکلو(1);
    } وإلا {
        لکھو("uncompress(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ساكن فراغ test_gzio(ثابت حرف *fname, Byte *uncompr, uLong uncomprLen) {
#جے_تعریف_شدہ NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#وإلا
    صحيح err;
    صحيح len = (صحيح)لڑی_لمبائی(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        باہر_نکلو(1);
    }
    gzputc(file, 'h');
    إذا (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    إذا (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        باہر_نکلو(1);
    }
    strcpy((حرف*)uncompr, "garbage");

    إذا (gzread(file, uncompr, (بنانشانی)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (حرف*)uncompr);
        باہر_نکلو(1);
    } وإلا {
        لکھو("gzread(): %s\n", (حرف*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    إذا (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (لما)pos, (لما)gztell(file));
        باہر_نکلو(1);
    }

    إذا (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        باہر_نکلو(1);
    }

    إذا (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        باہر_نکلو(1);
    }

    gzgets(file, (حرف*)uncompr, (صحيح)uncomprLen);
    إذا (لڑی_لمبائی((حرف*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    إذا (strcmp((حرف*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        باہر_نکلو(1);
    } وإلا {
        لکھو("gzgets() after gzseek: %s\n", (حرف*)uncompr);
    }

    gzclose(file);
#جے_انت
}

#جے_انت /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ساكن فراغ test_deflate(Byte *compr, uLong comprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحيح err;
    uLong len = (uLong)لڑی_لمبائی(hello)+1;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_دھارا.اگلا_وچ  = (z_const بنانشانی حرف *)hello;
    c_دھارا.اگلا_باہرلا = compr;

    طالما (c_دھارا.کل_وچ != len && c_دھارا.کل_باہرلا < comprLen) {
        c_دھارا.avail_وچ = c_دھارا.avail_باہرلا = 1; /* force small buffers */
        err = deflate(&c_دھارا, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    لكل (;;) {
        c_دھارا.avail_باہرلا = 1;
        err = deflate(&c_دھارا, Z_انت);
        إذا (err == Z_دھارا_سرا) اكسر;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_سرا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ساكن فراغ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    صحيح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_وچ  = compr;
    d_دھارا.avail_وچ = 0;
    d_دھارا.اگلا_باہرلا = uncompr;

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    طالما (d_دھارا.کل_باہرلا < uncomprLen && d_دھارا.کل_وچ < comprLen) {
        d_دھارا.avail_وچ = d_دھارا.avail_باہرلا = 1; /* force small buffers */
        err = inflate(&d_دھارا, Z_NO_FLUSH);
        إذا (err == Z_دھارا_سرا) اكسر;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_سرا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        باہر_نکلو(1);
    } وإلا {
        لکھو("inflate(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ساكن فراغ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحيح err;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_بہترین_رفتار);
    CHECK_ERR(err, "deflateInit");

    c_دھارا.اگلا_باہرلا = compr;
    c_دھارا.avail_باہرلا = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_دھارا.اگلا_وچ = uncompr;
    c_دھارا.avail_وچ = (uInt)uncomprLen;
    err = deflate(&c_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    إذا (c_دھارا.avail_وچ != 0) {
        fprintf(stderr, "deflate not greedy\n");
        باہر_نکلو(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_دھارا, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_دھارا.اگلا_وچ = compr;
    c_دھارا.avail_وچ = (uInt)uncomprLen/2;
    err = deflate(&c_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_دھارا, Z_بہترین_COMPRESSION, Z_FILTERED);
    c_دھارا.اگلا_وچ = uncompr;
    c_دھارا.avail_وچ = (uInt)uncomprLen;
    err = deflate(&c_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_دھارا, Z_انت);
    إذا (err != Z_دھارا_سرا) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        باہر_نکلو(1);
    }
    err = deflate_سرا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ساكن فراغ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    صحيح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_وچ  = compr;
    d_دھارا.avail_وچ = (uInt)comprLen;

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    لكل (;;) {
        d_دھارا.اگلا_باہرلا = uncompr;            /* discard the output */
        d_دھارا.avail_باہرلا = (uInt)uncomprLen;
        err = inflate(&d_دھارا, Z_NO_FLUSH);
        إذا (err == Z_دھارا_سرا) اكسر;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_سرا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    إذا (d_دھارا.کل_باہرلا != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_دھارا.کل_باہرلا);
        باہر_نکلو(1);
    } وإلا {
        لکھو("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ساكن فراغ test_flush(Byte *compr, uLong *comprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحيح err;
    uInt len = (uInt)لڑی_لمبائی(hello)+1;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_دھارا.اگلا_وچ  = (z_const بنانشانی حرف *)hello;
    c_دھارا.اگلا_باہرلا = compr;
    c_دھارا.avail_وچ = 3;
    c_دھارا.avail_باہرلا = (uInt)*comprLen;
    err = deflate(&c_دھارا, Z_بھریا_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_دھارا.avail_وچ = len - 3;

    err = deflate(&c_دھارا, Z_انت);
    إذا (err != Z_دھارا_سرا) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_سرا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_دھارا.کل_باہرلا;
}

/* ===========================================================================
 * Test inflateSync()
 */
ساكن فراغ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    صحيح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_وچ  = compr;
    d_دھارا.avail_وچ = 2; /* just read the zlib header */

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    d_دھارا.اگلا_باہرلا = uncompr;
    d_دھارا.avail_باہرلا = (uInt)uncomprLen;

    err = inflate(&d_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_دھارا.avail_وچ = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_دھارا);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_دھارا, Z_انت);
    إذا (err != Z_دھارا_سرا) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        باہر_نکلو(1);
    }
    err = inflate_سرا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    لکھو("after inflateSync(): hel%s\n", (حرف *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ساكن فراغ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحيح err;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_بہترین_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_سیٹ_Dictionary(&c_دھارا,
                (ثابت Bytef*)dictionary, (صحيح)حجم(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_دھارا.adler;
    c_دھارا.اگلا_باہرلا = compr;
    c_دھارا.avail_باہرلا = (uInt)comprLen;

    c_دھارا.اگلا_وچ = (z_const بنانشانی حرف *)hello;
    c_دھارا.avail_وچ = (uInt)لڑی_لمبائی(hello)+1;

    err = deflate(&c_دھارا, Z_انت);
    إذا (err != Z_دھارا_سرا) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        باہر_نکلو(1);
    }
    err = deflate_سرا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ساكن فراغ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    صحيح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_وچ  = compr;
    d_دھارا.avail_وچ = (uInt)comprLen;

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    d_دھارا.اگلا_باہرلا = uncompr;
    d_دھارا.avail_باہرلا = (uInt)uncomprLen;

    لكل (;;) {
        err = inflate(&d_دھارا, Z_NO_FLUSH);
        إذا (err == Z_دھارا_سرا) اكسر;
        إذا (err == Z_NEED_DICT) {
            إذا (d_دھارا.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                باہر_نکلو(1);
            }
            err = inflate_سیٹ_Dictionary(&d_دھارا, (ثابت Bytef*)dictionary,
                                       (صحيح)حجم(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_سرا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        باہر_نکلو(1);
    } وإلا {
        لکھو("inflate with dictionary: %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

صحيح مکھ(صحيح argc, حرف *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ساكن ثابت حرف* my_نسخہ_2 = ZLIB_نسخہ;

    إذا (zlib_نسخہ()[0] != my_نسخہ_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        باہر_نکلو(1);

    } وإلا إذا (strcmp(zlib_نسخہ(), ZLIB_نسخہ) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_نسخہ());
    }

    لکھو("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_نسخہ, (بنانشانی)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)صفر_حافظہ_دیو((uInt)comprLen, 1);
    uncompr  = (Byte*)صفر_حافظہ_دیو((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    إذا (compr == Z_NULL || uncompr == Z_NULL) {
        لکھو("out of memory\n");
        باہر_نکلو(1);
    }

#جے_تعریف_شدہ Z_SOLO
    (فراغ)argc;
    (فراغ)argv;
#وإلا
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#جے_انت

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
