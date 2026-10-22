/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#اگر defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#اگر_ختم

#شامل "zlib.h"
#شامل <stdio.h>

#اگر_تعریف_شدہ STDC
#  include <string.h>
#  include <stdlib.h>
#اگر_ختم

#اگر defined(VMS)
#  define TESTFILE "foo-gz"
#وگرنہ_اگر defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#ورنہ
#  define TESTFILE "foo.gz"
#اگر_ختم

#تعریف_کرو CHECK_ERR(err, پیغام) { \
    اگر (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", پیغام, err); \
        باہر_نکلو(1); \
    } \
}

ساکن z_const حرف hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ساکن ثابت حرف dictionary[] = "hello";
ساکن uLong dictId;    /* Adler32 value of the dictionary */

#اگر_تعریف_شدہ Z_SOLO

ساکن خالی *myalloc(خالی *q, غیرعلامتی n, غیرعلامتی m) {
    (خالی)q;
    واپس صفر_حافظہ_دو(n, m);
}

ساکن خالی myfree(خالی *q, خالی *p) {
    (خالی)q;
    آزاد(p);
}

ساکن alloc_func zalloc = myalloc;
ساکن free_func zfree = myfree;

#ورنہ /* !Z_SOLO */

ساکن alloc_func zalloc = (alloc_func)0;
ساکن free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
ساکن خالی test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    صحیح err;
    uLong len = (uLong)لڑی_لمبائی(hello)+1;

    err = compress(compr, &comprLen, (ثابت Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((حرف*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    اگر (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        باہر_نکلو(1);
    } ورنہ {
        لکھو("uncompress(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ساکن خالی test_gzio(ثابت حرف *fname, Byte *uncompr, uLong uncomprLen) {
#اگر_تعریف_شدہ NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#ورنہ
    صحیح err;
    صحیح len = (صحیح)لڑی_لمبائی(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    اگر (file == تہی) {
        fprintf(stderr, "gzopen error\n");
        باہر_نکلو(1);
    }
    gzputc(file, 'h');
    اگر (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    اگر (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    اگر (file == تہی) {
        fprintf(stderr, "gzopen error\n");
        باہر_نکلو(1);
    }
    strcpy((حرف*)uncompr, "garbage");

    اگر (gzread(file, uncompr, (غیرعلامتی)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    اگر (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (حرف*)uncompr);
        باہر_نکلو(1);
    } ورنہ {
        لکھو("gzread(): %s\n", (حرف*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    اگر (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (طویل)pos, (طویل)gztell(file));
        باہر_نکلو(1);
    }

    اگر (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        باہر_نکلو(1);
    }

    اگر (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        باہر_نکلو(1);
    }

    gzgets(file, (حرف*)uncompr, (صحیح)uncomprLen);
    اگر (لڑی_لمبائی((حرف*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        باہر_نکلو(1);
    }
    اگر (strcmp((حرف*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        باہر_نکلو(1);
    } ورنہ {
        لکھو("gzgets() after gzseek: %s\n", (حرف*)uncompr);
    }

    gzclose(file);
#اگر_ختم
}

#اگر_ختم /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ساکن خالی test_deflate(Byte *compr, uLong comprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحیح err;
    uLong len = (uLong)لڑی_لمبائی(hello)+1;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_دھارا.اگلا_اندر  = (z_const غیرعلامتی حرف *)hello;
    c_دھارا.اگلا_برآمد = compr;

    جب (c_دھارا.کل_اندر != len && c_دھارا.کل_برآمد < comprLen) {
        c_دھارا.avail_اندر = c_دھارا.avail_برآمد = 1; /* force small buffers */
        err = deflate(&c_دھارا, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    برائے (;;) {
        c_دھارا.avail_برآمد = 1;
        err = deflate(&c_دھارا, Z_اختتام);
        اگر (err == Z_دھارا_انتہا) توڑو;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_انتہا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ساکن خالی test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    صحیح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_اندر  = compr;
    d_دھارا.avail_اندر = 0;
    d_دھارا.اگلا_برآمد = uncompr;

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    جب (d_دھارا.کل_برآمد < uncomprLen && d_دھارا.کل_اندر < comprLen) {
        d_دھارا.avail_اندر = d_دھارا.avail_برآمد = 1; /* force small buffers */
        err = inflate(&d_دھارا, Z_NO_FLUSH);
        اگر (err == Z_دھارا_انتہا) توڑو;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_انتہا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    اگر (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        باہر_نکلو(1);
    } ورنہ {
        لکھو("inflate(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ساکن خالی test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحیح err;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_بہترین_رفتار);
    CHECK_ERR(err, "deflateInit");

    c_دھارا.اگلا_برآمد = compr;
    c_دھارا.avail_برآمد = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_دھارا.اگلا_اندر = uncompr;
    c_دھارا.avail_اندر = (uInt)uncomprLen;
    err = deflate(&c_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    اگر (c_دھارا.avail_اندر != 0) {
        fprintf(stderr, "deflate not greedy\n");
        باہر_نکلو(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_دھارا, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_دھارا.اگلا_اندر = compr;
    c_دھارا.avail_اندر = (uInt)uncomprLen/2;
    err = deflate(&c_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_دھارا, Z_بہترین_COMPRESSION, Z_FILTERED);
    c_دھارا.اگلا_اندر = uncompr;
    c_دھارا.avail_اندر = (uInt)uncomprLen;
    err = deflate(&c_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_دھارا, Z_اختتام);
    اگر (err != Z_دھارا_انتہا) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        باہر_نکلو(1);
    }
    err = deflate_انتہا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ساکن خالی test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    صحیح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_اندر  = compr;
    d_دھارا.avail_اندر = (uInt)comprLen;

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    برائے (;;) {
        d_دھارا.اگلا_برآمد = uncompr;            /* discard the output */
        d_دھارا.avail_برآمد = (uInt)uncomprLen;
        err = inflate(&d_دھارا, Z_NO_FLUSH);
        اگر (err == Z_دھارا_انتہا) توڑو;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_انتہا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    اگر (d_دھارا.کل_برآمد != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_دھارا.کل_برآمد);
        باہر_نکلو(1);
    } ورنہ {
        لکھو("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ساکن خالی test_flush(Byte *compr, uLong *comprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحیح err;
    uInt len = (uInt)لڑی_لمبائی(hello)+1;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_دھارا.اگلا_اندر  = (z_const غیرعلامتی حرف *)hello;
    c_دھارا.اگلا_برآمد = compr;
    c_دھارا.avail_اندر = 3;
    c_دھارا.avail_برآمد = (uInt)*comprLen;
    err = deflate(&c_دھارا, Z_بھرا_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_دھارا.avail_اندر = len - 3;

    err = deflate(&c_دھارا, Z_اختتام);
    اگر (err != Z_دھارا_انتہا) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_انتہا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_دھارا.کل_برآمد;
}

/* ===========================================================================
 * Test inflateSync()
 */
ساکن خالی test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    صحیح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_اندر  = compr;
    d_دھارا.avail_اندر = 2; /* just read the zlib header */

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    d_دھارا.اگلا_برآمد = uncompr;
    d_دھارا.avail_برآمد = (uInt)uncomprLen;

    err = inflate(&d_دھارا, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_دھارا.avail_اندر = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_دھارا);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_دھارا, Z_اختتام);
    اگر (err != Z_دھارا_انتہا) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        باہر_نکلو(1);
    }
    err = inflate_انتہا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    لکھو("after inflateSync(): hel%s\n", (حرف *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ساکن خالی test_dict_deflate(Byte *compr, uLong comprLen) {
    z_دھارا c_دھارا; /* compression stream */
    صحیح err;

    c_دھارا.zalloc = zalloc;
    c_دھارا.zfree = zfree;
    c_دھارا.opaque = (voidpf)0;

    err = deflateInit(&c_دھارا, Z_بہترین_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_سیٹ_Dictionary(&c_دھارا,
                (ثابت Bytef*)dictionary, (صحیح)حجم(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_دھارا.adler;
    c_دھارا.اگلا_برآمد = compr;
    c_دھارا.avail_برآمد = (uInt)comprLen;

    c_دھارا.اگلا_اندر = (z_const غیرعلامتی حرف *)hello;
    c_دھارا.avail_اندر = (uInt)لڑی_لمبائی(hello)+1;

    err = deflate(&c_دھارا, Z_اختتام);
    اگر (err != Z_دھارا_انتہا) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        باہر_نکلو(1);
    }
    err = deflate_انتہا(&c_دھارا);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ساکن خالی test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    صحیح err;
    z_دھارا d_دھارا; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_دھارا.zalloc = zalloc;
    d_دھارا.zfree = zfree;
    d_دھارا.opaque = (voidpf)0;

    d_دھارا.اگلا_اندر  = compr;
    d_دھارا.avail_اندر = (uInt)comprLen;

    err = inflateInit(&d_دھارا);
    CHECK_ERR(err, "inflateInit");

    d_دھارا.اگلا_برآمد = uncompr;
    d_دھارا.avail_برآمد = (uInt)uncomprLen;

    برائے (;;) {
        err = inflate(&d_دھارا, Z_NO_FLUSH);
        اگر (err == Z_دھارا_انتہا) توڑو;
        اگر (err == Z_NEED_DICT) {
            اگر (d_دھارا.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                باہر_نکلو(1);
            }
            err = inflate_سیٹ_Dictionary(&d_دھارا, (ثابت Bytef*)dictionary,
                                       (صحیح)حجم(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_انتہا(&d_دھارا);
    CHECK_ERR(err, "inflateEnd");

    اگر (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        باہر_نکلو(1);
    } ورنہ {
        لکھو("inflate with dictionary: %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

صحیح مین(صحیح argc, حرف *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ساکن ثابت حرف* my_نسخہ_2 = ZLIB_نسخہ;

    اگر (zlib_نسخہ()[0] != my_نسخہ_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        باہر_نکلو(1);

    } ورنہ اگر (strcmp(zlib_نسخہ(), ZLIB_نسخہ) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_نسخہ());
    }

    لکھو("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_نسخہ, (غیرعلامتی)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)صفر_حافظہ_دو((uInt)comprLen, 1);
    uncompr  = (Byte*)صفر_حافظہ_دو((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    اگر (compr == Z_NULL || uncompr == Z_NULL) {
        لکھو("out of memory\n");
        باہر_نکلو(1);
    }

#اگر_تعریف_شدہ Z_SOLO
    (خالی)argc;
    (خالی)argv;
#ورنہ
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#اگر_ختم

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = 3 * uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    آزاد(compr);
    آزاد(uncompr);

    واپس 0;
}
