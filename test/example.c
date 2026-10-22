/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ಆದರೆ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ STDC
#  include <string.h>
#  include <stdlib.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(VMS)
#  define TESTFILE "foo-gz"
#ಅಥವಾಆದರೆ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#ಇಲ್ಲದಿದ್ದರೆ
#  define TESTFILE "foo.gz"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು CHECK_ERR(err, ಸಂದೇಶ) { \
    ಆದರೆ (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", ಸಂದೇಶ, err); \
        ನಿರ್ಗಮಿಸು(1); \
    } \
}

ಸ್ಥಿರ z_const ಅಕ್ಷರ hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ dictionary[] = "hello";
ಸ್ಥಿರ uLong dictId;    /* Adler32 value of the dictionary */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO

ಸ್ಥಿರ ಶೂನ್ಯ *myalloc(ಶೂನ್ಯ *q, ಚಿಹ್ನೆರಹಿತ n, ಚಿಹ್ನೆರಹಿತ m) {
    (ಶೂನ್ಯ)q;
    ಹಿಂತಿರುಗಿ ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(n, m);
}

ಸ್ಥಿರ ಶೂನ್ಯ myfree(ಶೂನ್ಯ *q, ಶೂನ್ಯ *p) {
    (ಶೂನ್ಯ)q;
    ಬಿಡುಗಡೆ(p);
}

ಸ್ಥಿರ alloc_func zalloc = myalloc;
ಸ್ಥಿರ free_func zfree = myfree;

#ಇಲ್ಲದಿದ್ದರೆ /* !Z_SOLO */

ಸ್ಥಿರ alloc_func zalloc = (alloc_func)0;
ಸ್ಥಿರ free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    ಪೂರ್ಣಾಂಕ err;
    uLong len = (uLong)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(hello)+1;

    err = compress(compr, &comprLen, (ಸ್ಥಿರಾಂಕ Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((ಅಕ್ಷರ*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    ಆದರೆ (strcmp((ಅಕ್ಷರ*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        ನಿರ್ಗಮಿಸು(1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ("uncompress(): %s\n", (ಅಕ್ಷರ *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_gzio(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname, Byte *uncompr, uLong uncomprLen) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#ಇಲ್ಲದಿದ್ದರೆ
    ಪೂರ್ಣಾಂಕ err;
    ಪೂರ್ಣಾಂಕ len = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    ಆದರೆ (file == ಖಾಲಿ) {
        fprintf(stderr, "gzopen error\n");
        ನಿರ್ಗಮಿಸು(1);
    }
    gzputc(file, 'h');
    ಆದರೆ (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        ನಿರ್ಗಮಿಸು(1);
    }
    ಆದರೆ (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        ನಿರ್ಗಮಿಸು(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    ಆದರೆ (file == ಖಾಲಿ) {
        fprintf(stderr, "gzopen error\n");
        ನಿರ್ಗಮಿಸು(1);
    }
    strcpy((ಅಕ್ಷರ*)uncompr, "garbage");

    ಆದರೆ (gzread(file, uncompr, (ಚಿಹ್ನೆರಹಿತ)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        ನಿರ್ಗಮಿಸು(1);
    }
    ಆದರೆ (strcmp((ಅಕ್ಷರ*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (ಅಕ್ಷರ*)uncompr);
        ನಿರ್ಗಮಿಸು(1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ("gzread(): %s\n", (ಅಕ್ಷರ*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    ಆದರೆ (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (ದೀರ್ಘ)pos, (ದೀರ್ಘ)gztell(file));
        ನಿರ್ಗಮಿಸು(1);
    }

    ಆದರೆ (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        ನಿರ್ಗಮಿಸು(1);
    }

    ಆದರೆ (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        ನಿರ್ಗಮಿಸು(1);
    }

    gzgets(file, (ಅಕ್ಷರ*)uncompr, (ಪೂರ್ಣಾಂಕ)uncomprLen);
    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ((ಅಕ್ಷರ*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        ನಿರ್ಗಮಿಸು(1);
    }
    ಆದರೆ (strcmp((ಅಕ್ಷರ*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        ನಿರ್ಗಮಿಸು(1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ("gzgets() after gzseek: %s\n", (ಅಕ್ಷರ*)uncompr);
    }

    gzclose(file);
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ_ಅಂತ್ಯ /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_deflate(Byte *compr, uLong comprLen) {
    z_ಹರಿವು c_ಹರಿವು; /* compression stream */
    ಪೂರ್ಣಾಂಕ err;
    uLong len = (uLong)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(hello)+1;

    c_ಹರಿವು.zalloc = zalloc;
    c_ಹರಿವು.zfree = zfree;
    c_ಹರಿವು.opaque = (voidpf)0;

    err = deflateInit(&c_ಹರಿವು, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_ಹರಿವು.ಮುಂದಿನ_ಒಳ  = (z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)hello;
    c_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = compr;

    ಆಗ (c_ಹರಿವು.ಒಟ್ಟು_ಒಳ != len && c_ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು < comprLen) {
        c_ಹರಿವು.avail_ಒಳ = c_ಹರಿವು.avail_ಹೊರಹೋಗು = 1; /* force small buffers */
        err = deflate(&c_ಹರಿವು, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    ಗಾಗಿ (;;) {
        c_ಹರಿವು.avail_ಹೊರಹೋಗು = 1;
        err = deflate(&c_ಹರಿವು, Z_ಮುಕ್ತಾಯ);
        ಆದರೆ (err == Z_ಹರಿವು_ಕೊನೆ) ನಿಲ್ಲಿಸು;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_ಕೊನೆ(&c_ಹರಿವು);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    ಪೂರ್ಣಾಂಕ err;
    z_ಹರಿವು d_ಹರಿವು; /* decompression stream */

    strcpy((ಅಕ್ಷರ*)uncompr, "garbage");

    d_ಹರಿವು.zalloc = zalloc;
    d_ಹರಿವು.zfree = zfree;
    d_ಹರಿವು.opaque = (voidpf)0;

    d_ಹರಿವು.ಮುಂದಿನ_ಒಳ  = compr;
    d_ಹರಿವು.avail_ಒಳ = 0;
    d_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = uncompr;

    err = inflateInit(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateInit");

    ಆಗ (d_ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು < uncomprLen && d_ಹರಿವು.ಒಟ್ಟು_ಒಳ < comprLen) {
        d_ಹರಿವು.avail_ಒಳ = d_ಹರಿವು.avail_ಹೊರಹೋಗು = 1; /* force small buffers */
        err = inflate(&d_ಹರಿವು, Z_NO_FLUSH);
        ಆದರೆ (err == Z_ಹರಿವು_ಕೊನೆ) ನಿಲ್ಲಿಸು;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_ಕೊನೆ(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateEnd");

    ಆದರೆ (strcmp((ಅಕ್ಷರ*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        ನಿರ್ಗಮಿಸು(1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ("inflate(): %s\n", (ಅಕ್ಷರ *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_ಹರಿವು c_ಹರಿವು; /* compression stream */
    ಪೂರ್ಣಾಂಕ err;

    c_ಹರಿವು.zalloc = zalloc;
    c_ಹರಿವು.zfree = zfree;
    c_ಹರಿವು.opaque = (voidpf)0;

    err = deflateInit(&c_ಹರಿವು, Z_ಉತ್ತಮ_ವೇಗ);
    CHECK_ERR(err, "deflateInit");

    c_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = compr;
    c_ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_ಹರಿವು.ಮುಂದಿನ_ಒಳ = uncompr;
    c_ಹರಿವು.avail_ಒಳ = (uInt)uncomprLen;
    err = deflate(&c_ಹರಿವು, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    ಆದರೆ (c_ಹರಿವು.avail_ಒಳ != 0) {
        fprintf(stderr, "deflate not greedy\n");
        ನಿರ್ಗಮಿಸು(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_ಹರಿವು, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_ಹರಿವು.ಮುಂದಿನ_ಒಳ = compr;
    c_ಹರಿವು.avail_ಒಳ = (uInt)uncomprLen/2;
    err = deflate(&c_ಹರಿವು, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_ಹರಿವು, Z_ಉತ್ತಮ_COMPRESSION, Z_FILTERED);
    c_ಹರಿವು.ಮುಂದಿನ_ಒಳ = uncompr;
    c_ಹರಿವು.avail_ಒಳ = (uInt)uncomprLen;
    err = deflate(&c_ಹರಿವು, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_ಹರಿವು, Z_ಮುಕ್ತಾಯ);
    ಆದರೆ (err != Z_ಹರಿವು_ಕೊನೆ) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        ನಿರ್ಗಮಿಸು(1);
    }
    err = deflate_ಕೊನೆ(&c_ಹರಿವು);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    ಪೂರ್ಣಾಂಕ err;
    z_ಹರಿವು d_ಹರಿವು; /* decompression stream */

    strcpy((ಅಕ್ಷರ*)uncompr, "garbage");

    d_ಹರಿವು.zalloc = zalloc;
    d_ಹರಿವು.zfree = zfree;
    d_ಹರಿವು.opaque = (voidpf)0;

    d_ಹರಿವು.ಮುಂದಿನ_ಒಳ  = compr;
    d_ಹರಿವು.avail_ಒಳ = (uInt)comprLen;

    err = inflateInit(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateInit");

    ಗಾಗಿ (;;) {
        d_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = uncompr;            /* discard the output */
        d_ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)uncomprLen;
        err = inflate(&d_ಹರಿವು, Z_NO_FLUSH);
        ಆದರೆ (err == Z_ಹರಿವು_ಕೊನೆ) ನಿಲ್ಲಿಸು;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_ಕೊನೆ(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateEnd");

    ಆದರೆ (d_ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು);
        ನಿರ್ಗಮಿಸು(1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_flush(Byte *compr, uLong *comprLen) {
    z_ಹರಿವು c_ಹರಿವು; /* compression stream */
    ಪೂರ್ಣಾಂಕ err;
    uInt len = (uInt)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(hello)+1;

    c_ಹರಿವು.zalloc = zalloc;
    c_ಹರಿವು.zfree = zfree;
    c_ಹರಿವು.opaque = (voidpf)0;

    err = deflateInit(&c_ಹರಿವು, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_ಹರಿವು.ಮುಂದಿನ_ಒಳ  = (z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)hello;
    c_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = compr;
    c_ಹರಿವು.avail_ಒಳ = 3;
    c_ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)*comprLen;
    err = deflate(&c_ಹರಿವು, Z_ತುಂಬಿದ_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_ಹರಿವು.avail_ಒಳ = len - 3;

    err = deflate(&c_ಹರಿವು, Z_ಮುಕ್ತಾಯ);
    ಆದರೆ (err != Z_ಹರಿವು_ಕೊನೆ) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_ಕೊನೆ(&c_ಹರಿವು);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು;
}

/* ===========================================================================
 * Test inflateSync()
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    ಪೂರ್ಣಾಂಕ err;
    z_ಹರಿವು d_ಹರಿವು; /* decompression stream */

    strcpy((ಅಕ್ಷರ*)uncompr, "garbage");

    d_ಹರಿವು.zalloc = zalloc;
    d_ಹರಿವು.zfree = zfree;
    d_ಹರಿವು.opaque = (voidpf)0;

    d_ಹರಿವು.ಮುಂದಿನ_ಒಳ  = compr;
    d_ಹರಿವು.avail_ಒಳ = 2; /* just read the zlib header */

    err = inflateInit(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateInit");

    d_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = uncompr;
    d_ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)uncomprLen;

    err = inflate(&d_ಹರಿವು, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_ಹರಿವು.avail_ಒಳ = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_ಹರಿವು);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_ಹರಿವು, Z_ಮುಕ್ತಾಯ);
    ಆದರೆ (err != Z_ಹರಿವು_ಕೊನೆ) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        ನಿರ್ಗಮಿಸು(1);
    }
    err = inflate_ಕೊನೆ(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateEnd");

    ಬರೆ("after inflateSync(): hel%s\n", (ಅಕ್ಷರ *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_ಹರಿವು c_ಹರಿವು; /* compression stream */
    ಪೂರ್ಣಾಂಕ err;

    c_ಹರಿವು.zalloc = zalloc;
    c_ಹರಿವು.zfree = zfree;
    c_ಹರಿವು.opaque = (voidpf)0;

    err = deflateInit(&c_ಹರಿವು, Z_ಉತ್ತಮ_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_ಗಣ_Dictionary(&c_ಹರಿವು,
                (ಸ್ಥಿರಾಂಕ Bytef*)dictionary, (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_ಹರಿವು.adler;
    c_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = compr;
    c_ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)comprLen;

    c_ಹರಿವು.ಮುಂದಿನ_ಒಳ = (z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)hello;
    c_ಹರಿವು.avail_ಒಳ = (uInt)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(hello)+1;

    err = deflate(&c_ಹರಿವು, Z_ಮುಕ್ತಾಯ);
    ಆದರೆ (err != Z_ಹರಿವು_ಕೊನೆ) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        ನಿರ್ಗಮಿಸು(1);
    }
    err = deflate_ಕೊನೆ(&c_ಹರಿವು);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ಸ್ಥಿರ ಶೂನ್ಯ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    ಪೂರ್ಣಾಂಕ err;
    z_ಹರಿವು d_ಹರಿವು; /* decompression stream */

    strcpy((ಅಕ್ಷರ*)uncompr, "garbage");

    d_ಹರಿವು.zalloc = zalloc;
    d_ಹರಿವು.zfree = zfree;
    d_ಹರಿವು.opaque = (voidpf)0;

    d_ಹರಿವು.ಮುಂದಿನ_ಒಳ  = compr;
    d_ಹರಿವು.avail_ಒಳ = (uInt)comprLen;

    err = inflateInit(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateInit");

    d_ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = uncompr;
    d_ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)uncomprLen;

    ಗಾಗಿ (;;) {
        err = inflate(&d_ಹರಿವು, Z_NO_FLUSH);
        ಆದರೆ (err == Z_ಹರಿವು_ಕೊನೆ) ನಿಲ್ಲಿಸು;
        ಆದರೆ (err == Z_NEED_DICT) {
            ಆದರೆ (d_ಹರಿವು.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                ನಿರ್ಗಮಿಸು(1);
            }
            err = inflate_ಗಣ_Dictionary(&d_ಹರಿವು, (ಸ್ಥಿರಾಂಕ Bytef*)dictionary,
                                       (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_ಕೊನೆ(&d_ಹರಿವು);
    CHECK_ERR(err, "inflateEnd");

    ಆದರೆ (strcmp((ಅಕ್ಷರ*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        ನಿರ್ಗಮಿಸು(1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಬರೆ("inflate with dictionary: %s\n", (ಅಕ್ಷರ *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* my_ಆವೃತ್ತಿ_2 = ZLIB_ಆವೃತ್ತಿ;

    ಆದರೆ (zlib_ಆವೃತ್ತಿ()[0] != my_ಆವೃತ್ತಿ_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        ನಿರ್ಗಮಿಸು(1);

    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(zlib_ಆವೃತ್ತಿ(), ZLIB_ಆವೃತ್ತಿ) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_ಆವೃತ್ತಿ());
    }

    ಬರೆ("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_ಆವೃತ್ತಿ, (ಚಿಹ್ನೆರಹಿತ)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು((uInt)comprLen, 1);
    uncompr  = (Byte*)ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    ಆದರೆ (compr == Z_NULL || uncompr == Z_NULL) {
        ಬರೆ("out of memory\n");
        ನಿರ್ಗಮಿಸು(1);
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO
    (ಶೂನ್ಯ)argc;
    (ಶೂನ್ಯ)argv;
#ಇಲ್ಲದಿದ್ದರೆ
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#ಆದರೆ_ಅಂತ್ಯ

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = 3 * uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    ಬಿಡುಗಡೆ(compr);
    ಬಿಡುಗಡೆ(uncompr);

    ಹಿಂತಿರುಗಿ 0;
}
