/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ఒకవేళ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#అయితే_ముగింపు

#అన్తర్భావ "zlib.h"
#అన్తర్భావ <stdio.h>

#నిర్వచితమైతే STDC
#  include <string.h>
#  include <stdlib.h>
#అయితే_ముగింపు

#ఒకవేళ defined(VMS)
#  define TESTFILE "foo-gz"
#లేదావేళ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#లేకపోతే
#  define TESTFILE "foo.gz"
#అయితే_ముగింపు

#నిర్వచించు CHECK_ERR(err, సందేశం) { \
    ఒకవేళ (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", సందేశం, err); \
        నిష్క్రమించు(1); \
    } \
}

స్థిర z_const అక్షరం hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

స్థిర స్థిరరాశి అక్షరం dictionary[] = "hello";
స్థిర uLong dictId;    /* Adler32 value of the dictionary */

#నిర్వచితమైతే Z_SOLO

స్థిర శూన్య *myalloc(శూన్య *q, గుర్తులేని n, గుర్తులేని m) {
    (శూన్య)q;
    తిరిగి సున్నా_జ్ఞాపకం_ఇవ్వు(n, m);
}

స్థిర శూన్య myfree(శూన్య *q, శూన్య *p) {
    (శూన్య)q;
    విడుదల(p);
}

స్థిర alloc_func zalloc = myalloc;
స్థిర free_func zfree = myfree;

#లేకపోతే /* !Z_SOLO */

స్థిర alloc_func zalloc = (alloc_func)0;
స్థిర free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
స్థిర శూన్య test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    పూర్ణాంకం err;
    uLong len = (uLong)అక్షరశ్రేణి_పొడవు(hello)+1;

    err = compress(compr, &comprLen, (స్థిరరాశి Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((అక్షరం*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    ఒకవేళ (strcmp((అక్షరం*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        నిష్క్రమించు(1);
    } లేకపోతే {
        రాయి("uncompress(): %s\n", (అక్షరం *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
స్థిర శూన్య test_gzio(స్థిరరాశి అక్షరం *fname, Byte *uncompr, uLong uncomprLen) {
#నిర్వచితమైతే NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#లేకపోతే
    పూర్ణాంకం err;
    పూర్ణాంకం len = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    ఒకవేళ (file == ఖాళీ) {
        fprintf(stderr, "gzopen error\n");
        నిష్క్రమించు(1);
    }
    gzputc(file, 'h');
    ఒకవేళ (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        నిష్క్రమించు(1);
    }
    ఒకవేళ (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        నిష్క్రమించు(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    ఒకవేళ (file == ఖాళీ) {
        fprintf(stderr, "gzopen error\n");
        నిష్క్రమించు(1);
    }
    strcpy((అక్షరం*)uncompr, "garbage");

    ఒకవేళ (gzread(file, uncompr, (గుర్తులేని)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        నిష్క్రమించు(1);
    }
    ఒకవేళ (strcmp((అక్షరం*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (అక్షరం*)uncompr);
        నిష్క్రమించు(1);
    } లేకపోతే {
        రాయి("gzread(): %s\n", (అక్షరం*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    ఒకవేళ (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (దీర్ఘ)pos, (దీర్ఘ)gztell(file));
        నిష్క్రమించు(1);
    }

    ఒకవేళ (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        నిష్క్రమించు(1);
    }

    ఒకవేళ (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        నిష్క్రమించు(1);
    }

    gzgets(file, (అక్షరం*)uncompr, (పూర్ణాంకం)uncomprLen);
    ఒకవేళ (అక్షరశ్రేణి_పొడవు((అక్షరం*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        నిష్క్రమించు(1);
    }
    ఒకవేళ (strcmp((అక్షరం*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        నిష్క్రమించు(1);
    } లేకపోతే {
        రాయి("gzgets() after gzseek: %s\n", (అక్షరం*)uncompr);
    }

    gzclose(file);
#అయితే_ముగింపు
}

#అయితే_ముగింపు /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
స్థిర శూన్య test_deflate(Byte *compr, uLong comprLen) {
    z_ప్రవాహం c_ప్రవాహం; /* compression stream */
    పూర్ణాంకం err;
    uLong len = (uLong)అక్షరశ్రేణి_పొడవు(hello)+1;

    c_ప్రవాహం.zalloc = zalloc;
    c_ప్రవాహం.zfree = zfree;
    c_ప్రవాహం.opaque = (voidpf)0;

    err = deflateInit(&c_ప్రవాహం, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_ప్రవాహం.తదుపరి_లోపల  = (z_const గుర్తులేని అక్షరం *)hello;
    c_ప్రవాహం.తదుపరి_బహిర్గతం = compr;

    అయితే (c_ప్రవాహం.మొత్తం_లోపల != len && c_ప్రవాహం.మొత్తం_బహిర్గతం < comprLen) {
        c_ప్రవాహం.avail_లోపల = c_ప్రవాహం.avail_బహిర్గతం = 1; /* force small buffers */
        err = deflate(&c_ప్రవాహం, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    కొరకు (;;) {
        c_ప్రవాహం.avail_బహిర్గతం = 1;
        err = deflate(&c_ప్రవాహం, Z_ముగింపు);
        ఒకవేళ (err == Z_ప్రవాహం_చివర) ఆపు;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_చివర(&c_ప్రవాహం);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
స్థిర శూన్య test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    పూర్ణాంకం err;
    z_ప్రవాహం d_ప్రవాహం; /* decompression stream */

    strcpy((అక్షరం*)uncompr, "garbage");

    d_ప్రవాహం.zalloc = zalloc;
    d_ప్రవాహం.zfree = zfree;
    d_ప్రవాహం.opaque = (voidpf)0;

    d_ప్రవాహం.తదుపరి_లోపల  = compr;
    d_ప్రవాహం.avail_లోపల = 0;
    d_ప్రవాహం.తదుపరి_బహిర్గతం = uncompr;

    err = inflateInit(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateInit");

    అయితే (d_ప్రవాహం.మొత్తం_బహిర్గతం < uncomprLen && d_ప్రవాహం.మొత్తం_లోపల < comprLen) {
        d_ప్రవాహం.avail_లోపల = d_ప్రవాహం.avail_బహిర్గతం = 1; /* force small buffers */
        err = inflate(&d_ప్రవాహం, Z_NO_FLUSH);
        ఒకవేళ (err == Z_ప్రవాహం_చివర) ఆపు;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_చివర(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateEnd");

    ఒకవేళ (strcmp((అక్షరం*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        నిష్క్రమించు(1);
    } లేకపోతే {
        రాయి("inflate(): %s\n", (అక్షరం *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
స్థిర శూన్య test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_ప్రవాహం c_ప్రవాహం; /* compression stream */
    పూర్ణాంకం err;

    c_ప్రవాహం.zalloc = zalloc;
    c_ప్రవాహం.zfree = zfree;
    c_ప్రవాహం.opaque = (voidpf)0;

    err = deflateInit(&c_ప్రవాహం, Z_ఉత్తమ_వేగం);
    CHECK_ERR(err, "deflateInit");

    c_ప్రవాహం.తదుపరి_బహిర్గతం = compr;
    c_ప్రవాహం.avail_బహిర్గతం = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_ప్రవాహం.తదుపరి_లోపల = uncompr;
    c_ప్రవాహం.avail_లోపల = (uInt)uncomprLen;
    err = deflate(&c_ప్రవాహం, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    ఒకవేళ (c_ప్రవాహం.avail_లోపల != 0) {
        fprintf(stderr, "deflate not greedy\n");
        నిష్క్రమించు(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_ప్రవాహం, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_ప్రవాహం.తదుపరి_లోపల = compr;
    c_ప్రవాహం.avail_లోపల = (uInt)uncomprLen/2;
    err = deflate(&c_ప్రవాహం, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_ప్రవాహం, Z_ఉత్తమ_COMPRESSION, Z_FILTERED);
    c_ప్రవాహం.తదుపరి_లోపల = uncompr;
    c_ప్రవాహం.avail_లోపల = (uInt)uncomprLen;
    err = deflate(&c_ప్రవాహం, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_ప్రవాహం, Z_ముగింపు);
    ఒకవేళ (err != Z_ప్రవాహం_చివర) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        నిష్క్రమించు(1);
    }
    err = deflate_చివర(&c_ప్రవాహం);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
స్థిర శూన్య test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    పూర్ణాంకం err;
    z_ప్రవాహం d_ప్రవాహం; /* decompression stream */

    strcpy((అక్షరం*)uncompr, "garbage");

    d_ప్రవాహం.zalloc = zalloc;
    d_ప్రవాహం.zfree = zfree;
    d_ప్రవాహం.opaque = (voidpf)0;

    d_ప్రవాహం.తదుపరి_లోపల  = compr;
    d_ప్రవాహం.avail_లోపల = (uInt)comprLen;

    err = inflateInit(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateInit");

    కొరకు (;;) {
        d_ప్రవాహం.తదుపరి_బహిర్గతం = uncompr;            /* discard the output */
        d_ప్రవాహం.avail_బహిర్గతం = (uInt)uncomprLen;
        err = inflate(&d_ప్రవాహం, Z_NO_FLUSH);
        ఒకవేళ (err == Z_ప్రవాహం_చివర) ఆపు;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_చివర(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateEnd");

    ఒకవేళ (d_ప్రవాహం.మొత్తం_బహిర్గతం != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_ప్రవాహం.మొత్తం_బహిర్గతం);
        నిష్క్రమించు(1);
    } లేకపోతే {
        రాయి("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
స్థిర శూన్య test_flush(Byte *compr, uLong *comprLen) {
    z_ప్రవాహం c_ప్రవాహం; /* compression stream */
    పూర్ణాంకం err;
    uInt len = (uInt)అక్షరశ్రేణి_పొడవు(hello)+1;

    c_ప్రవాహం.zalloc = zalloc;
    c_ప్రవాహం.zfree = zfree;
    c_ప్రవాహం.opaque = (voidpf)0;

    err = deflateInit(&c_ప్రవాహం, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_ప్రవాహం.తదుపరి_లోపల  = (z_const గుర్తులేని అక్షరం *)hello;
    c_ప్రవాహం.తదుపరి_బహిర్గతం = compr;
    c_ప్రవాహం.avail_లోపల = 3;
    c_ప్రవాహం.avail_బహిర్గతం = (uInt)*comprLen;
    err = deflate(&c_ప్రవాహం, Z_నిండిన_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_ప్రవాహం.avail_లోపల = len - 3;

    err = deflate(&c_ప్రవాహం, Z_ముగింపు);
    ఒకవేళ (err != Z_ప్రవాహం_చివర) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_చివర(&c_ప్రవాహం);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_ప్రవాహం.మొత్తం_బహిర్గతం;
}

/* ===========================================================================
 * Test inflateSync()
 */
స్థిర శూన్య test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    పూర్ణాంకం err;
    z_ప్రవాహం d_ప్రవాహం; /* decompression stream */

    strcpy((అక్షరం*)uncompr, "garbage");

    d_ప్రవాహం.zalloc = zalloc;
    d_ప్రవాహం.zfree = zfree;
    d_ప్రవాహం.opaque = (voidpf)0;

    d_ప్రవాహం.తదుపరి_లోపల  = compr;
    d_ప్రవాహం.avail_లోపల = 2; /* just read the zlib header */

    err = inflateInit(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateInit");

    d_ప్రవాహం.తదుపరి_బహిర్గతం = uncompr;
    d_ప్రవాహం.avail_బహిర్గతం = (uInt)uncomprLen;

    err = inflate(&d_ప్రవాహం, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_ప్రవాహం.avail_లోపల = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_ప్రవాహం);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_ప్రవాహం, Z_ముగింపు);
    ఒకవేళ (err != Z_ప్రవాహం_చివర) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        నిష్క్రమించు(1);
    }
    err = inflate_చివర(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateEnd");

    రాయి("after inflateSync(): hel%s\n", (అక్షరం *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
స్థిర శూన్య test_dict_deflate(Byte *compr, uLong comprLen) {
    z_ప్రవాహం c_ప్రవాహం; /* compression stream */
    పూర్ణాంకం err;

    c_ప్రవాహం.zalloc = zalloc;
    c_ప్రవాహం.zfree = zfree;
    c_ప్రవాహం.opaque = (voidpf)0;

    err = deflateInit(&c_ప్రవాహం, Z_ఉత్తమ_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_సమితి_Dictionary(&c_ప్రవాహం,
                (స్థిరరాశి Bytef*)dictionary, (పూర్ణాంకం)పరిమాణం(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_ప్రవాహం.adler;
    c_ప్రవాహం.తదుపరి_బహిర్గతం = compr;
    c_ప్రవాహం.avail_బహిర్గతం = (uInt)comprLen;

    c_ప్రవాహం.తదుపరి_లోపల = (z_const గుర్తులేని అక్షరం *)hello;
    c_ప్రవాహం.avail_లోపల = (uInt)అక్షరశ్రేణి_పొడవు(hello)+1;

    err = deflate(&c_ప్రవాహం, Z_ముగింపు);
    ఒకవేళ (err != Z_ప్రవాహం_చివర) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        నిష్క్రమించు(1);
    }
    err = deflate_చివర(&c_ప్రవాహం);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
స్థిర శూన్య test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    పూర్ణాంకం err;
    z_ప్రవాహం d_ప్రవాహం; /* decompression stream */

    strcpy((అక్షరం*)uncompr, "garbage");

    d_ప్రవాహం.zalloc = zalloc;
    d_ప్రవాహం.zfree = zfree;
    d_ప్రవాహం.opaque = (voidpf)0;

    d_ప్రవాహం.తదుపరి_లోపల  = compr;
    d_ప్రవాహం.avail_లోపల = (uInt)comprLen;

    err = inflateInit(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateInit");

    d_ప్రవాహం.తదుపరి_బహిర్గతం = uncompr;
    d_ప్రవాహం.avail_బహిర్గతం = (uInt)uncomprLen;

    కొరకు (;;) {
        err = inflate(&d_ప్రవాహం, Z_NO_FLUSH);
        ఒకవేళ (err == Z_ప్రవాహం_చివర) ఆపు;
        ఒకవేళ (err == Z_NEED_DICT) {
            ఒకవేళ (d_ప్రవాహం.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                నిష్క్రమించు(1);
            }
            err = inflate_సమితి_Dictionary(&d_ప్రవాహం, (స్థిరరాశి Bytef*)dictionary,
                                       (పూర్ణాంకం)పరిమాణం(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_చివర(&d_ప్రవాహం);
    CHECK_ERR(err, "inflateEnd");

    ఒకవేళ (strcmp((అక్షరం*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        నిష్క్రమించు(1);
    } లేకపోతే {
        రాయి("inflate with dictionary: %s\n", (అక్షరం *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    స్థిర స్థిరరాశి అక్షరం* my_సంస్కరణ_2 = ZLIB_సంస్కరణ;

    ఒకవేళ (zlib_సంస్కరణ()[0] != my_సంస్కరణ_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        నిష్క్రమించు(1);

    } లేకపోతే ఒకవేళ (strcmp(zlib_సంస్కరణ(), ZLIB_సంస్కరణ) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_సంస్కరణ());
    }

    రాయి("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_సంస్కరణ, (గుర్తులేని)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)సున్నా_జ్ఞాపకం_ఇవ్వు((uInt)comprLen, 1);
    uncompr  = (Byte*)సున్నా_జ్ఞాపకం_ఇవ్వు((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    ఒకవేళ (compr == Z_NULL || uncompr == Z_NULL) {
        రాయి("out of memory\n");
        నిష్క్రమించు(1);
    }

#నిర్వచితమైతే Z_SOLO
    (శూన్య)argc;
    (శూన్య)argv;
#లేకపోతే
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#అయితే_ముగింపు

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = 3 * uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    విడుదల(compr);
    విడుదల(uncompr);

    తిరిగి 0;
}
