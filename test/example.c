/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#إذا defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#جيڪڏهن_پورو

#ضمن "zlib.h"
#ضمن <stdio.h>

#جيڪڏهن_وصف_ٿيل STDC
#  include <string.h>
#  include <stdlib.h>
#جيڪڏهن_پورو

#إذا defined(VMS)
#  define TESTFILE "foo-gz"
#وإلاإذا defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#وإلا
#  define TESTFILE "foo.gz"
#جيڪڏهن_پورو

#وصف_ڪريو CHECK_ERR(err, نياپو) { \
    إذا (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", نياپو, err); \
        ٻاهر_نڪرو(1); \
    } \
}

ساكن z_const حرف hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ساكن ثابت حرف dictionary[] = "hello";
ساكن uLong dictId;    /* Adler32 value of the dictionary */

#جيڪڏهن_وصف_ٿيل Z_SOLO

ساكن فراغ *myalloc(فراغ *q, بنانشان n, بنانشان m) {
    (فراغ)q;
    أرجع صفر_يادگيري_ڏيو(n, m);
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
    uLong len = (uLong)لڙي_ڊيگهه(hello)+1;

    err = compress(compr, &comprLen, (ثابت Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((حرف*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        ٻاهر_نڪرو(1);
    } وإلا {
        لکو("uncompress(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ساكن فراغ test_gzio(ثابت حرف *fname, Byte *uncompr, uLong uncomprLen) {
#جيڪڏهن_وصف_ٿيل NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#وإلا
    صحيح err;
    صحيح len = (صحيح)لڙي_ڊيگهه(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        ٻاهر_نڪرو(1);
    }
    gzputc(file, 'h');
    إذا (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        ٻاهر_نڪرو(1);
    }
    إذا (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        ٻاهر_نڪرو(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    إذا (file == عدم) {
        fprintf(stderr, "gzopen error\n");
        ٻاهر_نڪرو(1);
    }
    strcpy((حرف*)uncompr, "garbage");

    إذا (gzread(file, uncompr, (بنانشان)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        ٻاهر_نڪرو(1);
    }
    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (حرف*)uncompr);
        ٻاهر_نڪرو(1);
    } وإلا {
        لکو("gzread(): %s\n", (حرف*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    إذا (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (ڊگهو)pos, (ڊگهو)gztell(file));
        ٻاهر_نڪرو(1);
    }

    إذا (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        ٻاهر_نڪرو(1);
    }

    إذا (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        ٻاهر_نڪرو(1);
    }

    gzgets(file, (حرف*)uncompr, (صحيح)uncomprLen);
    إذا (لڙي_ڊيگهه((حرف*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        ٻاهر_نڪرو(1);
    }
    إذا (strcmp((حرف*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        ٻاهر_نڪرو(1);
    } وإلا {
        لکو("gzgets() after gzseek: %s\n", (حرف*)uncompr);
    }

    gzclose(file);
#جيڪڏهن_پورو
}

#جيڪڏهن_پورو /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ساكن فراغ test_deflate(Byte *compr, uLong comprLen) {
    z_وهڪرو c_وهڪرو; /* compression stream */
    صحيح err;
    uLong len = (uLong)لڙي_ڊيگهه(hello)+1;

    c_وهڪرو.zalloc = zalloc;
    c_وهڪرو.zfree = zfree;
    c_وهڪرو.opaque = (voidpf)0;

    err = deflateInit(&c_وهڪرو, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_وهڪرو.ايندڙ_اندريون  = (z_const بنانشان حرف *)hello;
    c_وهڪرو.ايندڙ_ٻاهريون = compr;

    طالما (c_وهڪرو.ڪل_اندريون != len && c_وهڪرو.ڪل_ٻاهريون < comprLen) {
        c_وهڪرو.avail_اندريون = c_وهڪرو.avail_ٻاهريون = 1; /* force small buffers */
        err = deflate(&c_وهڪرو, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    لكل (;;) {
        c_وهڪرو.avail_ٻاهريون = 1;
        err = deflate(&c_وهڪرو, Z_پڄاڻي);
        إذا (err == Z_وهڪرو_پڇاڙي) اكسر;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_پڇاڙي(&c_وهڪرو);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ساكن فراغ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    صحيح err;
    z_وهڪرو d_وهڪرو; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_وهڪرو.zalloc = zalloc;
    d_وهڪرو.zfree = zfree;
    d_وهڪرو.opaque = (voidpf)0;

    d_وهڪرو.ايندڙ_اندريون  = compr;
    d_وهڪرو.avail_اندريون = 0;
    d_وهڪرو.ايندڙ_ٻاهريون = uncompr;

    err = inflateInit(&d_وهڪرو);
    CHECK_ERR(err, "inflateInit");

    طالما (d_وهڪرو.ڪل_ٻاهريون < uncomprLen && d_وهڪرو.ڪل_اندريون < comprLen) {
        d_وهڪرو.avail_اندريون = d_وهڪرو.avail_ٻاهريون = 1; /* force small buffers */
        err = inflate(&d_وهڪرو, Z_NO_FLUSH);
        إذا (err == Z_وهڪرو_پڇاڙي) اكسر;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_پڇاڙي(&d_وهڪرو);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        ٻاهر_نڪرو(1);
    } وإلا {
        لکو("inflate(): %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ساكن فراغ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_وهڪرو c_وهڪرو; /* compression stream */
    صحيح err;

    c_وهڪرو.zalloc = zalloc;
    c_وهڪرو.zfree = zfree;
    c_وهڪرو.opaque = (voidpf)0;

    err = deflateInit(&c_وهڪرو, Z_بهترين_رفتار);
    CHECK_ERR(err, "deflateInit");

    c_وهڪرو.ايندڙ_ٻاهريون = compr;
    c_وهڪرو.avail_ٻاهريون = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_وهڪرو.ايندڙ_اندريون = uncompr;
    c_وهڪرو.avail_اندريون = (uInt)uncomprLen;
    err = deflate(&c_وهڪرو, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    إذا (c_وهڪرو.avail_اندريون != 0) {
        fprintf(stderr, "deflate not greedy\n");
        ٻاهر_نڪرو(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_وهڪرو, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_وهڪرو.ايندڙ_اندريون = compr;
    c_وهڪرو.avail_اندريون = (uInt)uncomprLen/2;
    err = deflate(&c_وهڪرو, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_وهڪرو, Z_بهترين_COMPRESSION, Z_FILTERED);
    c_وهڪرو.ايندڙ_اندريون = uncompr;
    c_وهڪرو.avail_اندريون = (uInt)uncomprLen;
    err = deflate(&c_وهڪرو, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_وهڪرو, Z_پڄاڻي);
    إذا (err != Z_وهڪرو_پڇاڙي) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        ٻاهر_نڪرو(1);
    }
    err = deflate_پڇاڙي(&c_وهڪرو);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ساكن فراغ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    صحيح err;
    z_وهڪرو d_وهڪرو; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_وهڪرو.zalloc = zalloc;
    d_وهڪرو.zfree = zfree;
    d_وهڪرو.opaque = (voidpf)0;

    d_وهڪرو.ايندڙ_اندريون  = compr;
    d_وهڪرو.avail_اندريون = (uInt)comprLen;

    err = inflateInit(&d_وهڪرو);
    CHECK_ERR(err, "inflateInit");

    لكل (;;) {
        d_وهڪرو.ايندڙ_ٻاهريون = uncompr;            /* discard the output */
        d_وهڪرو.avail_ٻاهريون = (uInt)uncomprLen;
        err = inflate(&d_وهڪرو, Z_NO_FLUSH);
        إذا (err == Z_وهڪرو_پڇاڙي) اكسر;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_پڇاڙي(&d_وهڪرو);
    CHECK_ERR(err, "inflateEnd");

    إذا (d_وهڪرو.ڪل_ٻاهريون != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_وهڪرو.ڪل_ٻاهريون);
        ٻاهر_نڪرو(1);
    } وإلا {
        لکو("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ساكن فراغ test_flush(Byte *compr, uLong *comprLen) {
    z_وهڪرو c_وهڪرو; /* compression stream */
    صحيح err;
    uInt len = (uInt)لڙي_ڊيگهه(hello)+1;

    c_وهڪرو.zalloc = zalloc;
    c_وهڪرو.zfree = zfree;
    c_وهڪرو.opaque = (voidpf)0;

    err = deflateInit(&c_وهڪرو, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_وهڪرو.ايندڙ_اندريون  = (z_const بنانشان حرف *)hello;
    c_وهڪرو.ايندڙ_ٻاهريون = compr;
    c_وهڪرو.avail_اندريون = 3;
    c_وهڪرو.avail_ٻاهريون = (uInt)*comprLen;
    err = deflate(&c_وهڪرو, Z_ڀريل_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_وهڪرو.avail_اندريون = len - 3;

    err = deflate(&c_وهڪرو, Z_پڄاڻي);
    إذا (err != Z_وهڪرو_پڇاڙي) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_پڇاڙي(&c_وهڪرو);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_وهڪرو.ڪل_ٻاهريون;
}

/* ===========================================================================
 * Test inflateSync()
 */
ساكن فراغ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    صحيح err;
    z_وهڪرو d_وهڪرو; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_وهڪرو.zalloc = zalloc;
    d_وهڪرو.zfree = zfree;
    d_وهڪرو.opaque = (voidpf)0;

    d_وهڪرو.ايندڙ_اندريون  = compr;
    d_وهڪرو.avail_اندريون = 2; /* just read the zlib header */

    err = inflateInit(&d_وهڪرو);
    CHECK_ERR(err, "inflateInit");

    d_وهڪرو.ايندڙ_ٻاهريون = uncompr;
    d_وهڪرو.avail_ٻاهريون = (uInt)uncomprLen;

    err = inflate(&d_وهڪرو, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_وهڪرو.avail_اندريون = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_وهڪرو);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_وهڪرو, Z_پڄاڻي);
    إذا (err != Z_وهڪرو_پڇاڙي) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        ٻاهر_نڪرو(1);
    }
    err = inflate_پڇاڙي(&d_وهڪرو);
    CHECK_ERR(err, "inflateEnd");

    لکو("after inflateSync(): hel%s\n", (حرف *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ساكن فراغ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_وهڪرو c_وهڪرو; /* compression stream */
    صحيح err;

    c_وهڪرو.zalloc = zalloc;
    c_وهڪرو.zfree = zfree;
    c_وهڪرو.opaque = (voidpf)0;

    err = deflateInit(&c_وهڪرو, Z_بهترين_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_سيٽ_Dictionary(&c_وهڪرو,
                (ثابت Bytef*)dictionary, (صحيح)حجم(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_وهڪرو.adler;
    c_وهڪرو.ايندڙ_ٻاهريون = compr;
    c_وهڪرو.avail_ٻاهريون = (uInt)comprLen;

    c_وهڪرو.ايندڙ_اندريون = (z_const بنانشان حرف *)hello;
    c_وهڪرو.avail_اندريون = (uInt)لڙي_ڊيگهه(hello)+1;

    err = deflate(&c_وهڪرو, Z_پڄاڻي);
    إذا (err != Z_وهڪرو_پڇاڙي) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        ٻاهر_نڪرو(1);
    }
    err = deflate_پڇاڙي(&c_وهڪرو);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ساكن فراغ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    صحيح err;
    z_وهڪرو d_وهڪرو; /* decompression stream */

    strcpy((حرف*)uncompr, "garbage");

    d_وهڪرو.zalloc = zalloc;
    d_وهڪرو.zfree = zfree;
    d_وهڪرو.opaque = (voidpf)0;

    d_وهڪرو.ايندڙ_اندريون  = compr;
    d_وهڪرو.avail_اندريون = (uInt)comprLen;

    err = inflateInit(&d_وهڪرو);
    CHECK_ERR(err, "inflateInit");

    d_وهڪرو.ايندڙ_ٻاهريون = uncompr;
    d_وهڪرو.avail_ٻاهريون = (uInt)uncomprLen;

    لكل (;;) {
        err = inflate(&d_وهڪرو, Z_NO_FLUSH);
        إذا (err == Z_وهڪرو_پڇاڙي) اكسر;
        إذا (err == Z_NEED_DICT) {
            إذا (d_وهڪرو.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                ٻاهر_نڪرو(1);
            }
            err = inflate_سيٽ_Dictionary(&d_وهڪرو, (ثابت Bytef*)dictionary,
                                       (صحيح)حجم(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_پڇاڙي(&d_وهڪرو);
    CHECK_ERR(err, "inflateEnd");

    إذا (strcmp((حرف*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        ٻاهر_نڪرو(1);
    } وإلا {
        لکو("inflate with dictionary: %s\n", (حرف *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

صحيح مکيه(صحيح argc, حرف *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ساكن ثابت حرف* my_نسخو_2 = ZLIB_نسخو;

    إذا (zlib_نسخو()[0] != my_نسخو_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        ٻاهر_نڪرو(1);

    } وإلا إذا (strcmp(zlib_نسخو(), ZLIB_نسخو) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_نسخو());
    }

    لکو("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_نسخو, (بنانشان)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)صفر_يادگيري_ڏيو((uInt)comprLen, 1);
    uncompr  = (Byte*)صفر_يادگيري_ڏيو((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    إذا (compr == Z_NULL || uncompr == Z_NULL) {
        لکو("out of memory\n");
        ٻاهر_نڪرو(1);
    }

#جيڪڏهن_وصف_ٿيل Z_SOLO
    (فراغ)argc;
    (فراغ)argv;
#وإلا
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#جيڪڏهن_پورو

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
