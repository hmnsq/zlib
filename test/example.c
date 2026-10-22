/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ਜੇ defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ STDC
#  include <string.h>
#  include <stdlib.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(VMS)
#  define TESTFILE "foo-gz"
#ਜਾਂਜੇ defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#ਨਹੀਂਤਾਂ
#  define TESTFILE "foo.gz"
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHECK_ERR(err, ਸੁਨੇਹਾ) { \
    ਜੇ (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", ਸੁਨੇਹਾ, err); \
        ਬਾਹਰ_ਨਿਕਲੋ(1); \
    } \
}

ਸਥਿਰ z_const ਅੱਖਰ hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

ਸਥਿਰ ਅਚਲ ਅੱਖਰ dictionary[] = "hello";
ਸਥਿਰ uLong dictId;    /* Adler32 value of the dictionary */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO

ਸਥਿਰ ਖਾਲੀ *myalloc(ਖਾਲੀ *q, ਬਿਨਾਨਿਸ਼ਾਨੀ n, ਬਿਨਾਨਿਸ਼ਾਨੀ m) {
    (ਖਾਲੀ)q;
    ਵਾਪਸ ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(n, m);
}

ਸਥਿਰ ਖਾਲੀ myfree(ਖਾਲੀ *q, ਖਾਲੀ *p) {
    (ਖਾਲੀ)q;
    ਮੁਕਤ(p);
}

ਸਥਿਰ alloc_func zalloc = myalloc;
ਸਥਿਰ free_func zfree = myfree;

#ਨਹੀਂਤਾਂ /* !Z_SOLO */

ਸਥਿਰ alloc_func zalloc = (alloc_func)0;
ਸਥਿਰ free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
ਸਥਿਰ ਖਾਲੀ test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    ਪੂਰਨ err;
    uLong len = (uLong)ਲੜੀ_ਲੰਬਾਈ(hello)+1;

    err = compress(compr, &comprLen, (ਅਚਲ Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((ਅੱਖਰ*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    ਜੇ (strcmp((ਅੱਖਰ*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    } ਨਹੀਂਤਾਂ {
        ਲਿਖੋ("uncompress(): %s\n", (ਅੱਖਰ *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
ਸਥਿਰ ਖਾਲੀ test_gzio(ਅਚਲ ਅੱਖਰ *fname, Byte *uncompr, uLong uncomprLen) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#ਨਹੀਂਤਾਂ
    ਪੂਰਨ err;
    ਪੂਰਨ len = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    ਜੇ (file == ਨਿਰਾ) {
        fprintf(stderr, "gzopen error\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    gzputc(file, 'h');
    ਜੇ (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    ਜੇ (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    ਜੇ (file == ਨਿਰਾ) {
        fprintf(stderr, "gzopen error\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    strcpy((ਅੱਖਰ*)uncompr, "garbage");

    ਜੇ (gzread(file, uncompr, (ਬਿਨਾਨਿਸ਼ਾਨੀ)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    ਜੇ (strcmp((ਅੱਖਰ*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (ਅੱਖਰ*)uncompr);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    } ਨਹੀਂਤਾਂ {
        ਲਿਖੋ("gzread(): %s\n", (ਅੱਖਰ*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    ਜੇ (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (ਲੰਮਾ)pos, (ਲੰਮਾ)gztell(file));
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

    ਜੇ (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

    ਜੇ (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

    gzgets(file, (ਅੱਖਰ*)uncompr, (ਪੂਰਨ)uncomprLen);
    ਜੇ (ਲੜੀ_ਲੰਬਾਈ((ਅੱਖਰ*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    ਜੇ (strcmp((ਅੱਖਰ*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    } ਨਹੀਂਤਾਂ {
        ਲਿਖੋ("gzgets() after gzseek: %s\n", (ਅੱਖਰ*)uncompr);
    }

    gzclose(file);
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਅੰਤ /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
ਸਥਿਰ ਖਾਲੀ test_deflate(Byte *compr, uLong comprLen) {
    z_ਧਾਰਾ c_ਧਾਰਾ; /* compression stream */
    ਪੂਰਨ err;
    uLong len = (uLong)ਲੜੀ_ਲੰਬਾਈ(hello)+1;

    c_ਧਾਰਾ.zalloc = zalloc;
    c_ਧਾਰਾ.zfree = zfree;
    c_ਧਾਰਾ.opaque = (voidpf)0;

    err = deflateInit(&c_ਧਾਰਾ, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ  = (z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)hello;
    c_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = compr;

    ਜਦੋਂ (c_ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ != len && c_ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ < comprLen) {
        c_ਧਾਰਾ.avail_ਵਿੱਚ = c_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = 1; /* force small buffers */
        err = deflate(&c_ਧਾਰਾ, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    ਲਈ (;;) {
        c_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = 1;
        err = deflate(&c_ਧਾਰਾ, Z_ਅੰਤ);
        ਜੇ (err == Z_ਧਾਰਾ_ਸਿਰਾ) ਰੋਕੋ;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_ਸਿਰਾ(&c_ਧਾਰਾ);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
ਸਥਿਰ ਖਾਲੀ test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    ਪੂਰਨ err;
    z_ਧਾਰਾ d_ਧਾਰਾ; /* decompression stream */

    strcpy((ਅੱਖਰ*)uncompr, "garbage");

    d_ਧਾਰਾ.zalloc = zalloc;
    d_ਧਾਰਾ.zfree = zfree;
    d_ਧਾਰਾ.opaque = (voidpf)0;

    d_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ  = compr;
    d_ਧਾਰਾ.avail_ਵਿੱਚ = 0;
    d_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = uncompr;

    err = inflateInit(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateInit");

    ਜਦੋਂ (d_ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ < uncomprLen && d_ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ < comprLen) {
        d_ਧਾਰਾ.avail_ਵਿੱਚ = d_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = 1; /* force small buffers */
        err = inflate(&d_ਧਾਰਾ, Z_NO_FLUSH);
        ਜੇ (err == Z_ਧਾਰਾ_ਸਿਰਾ) ਰੋਕੋ;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_ਸਿਰਾ(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateEnd");

    ਜੇ (strcmp((ਅੱਖਰ*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    } ਨਹੀਂਤਾਂ {
        ਲਿਖੋ("inflate(): %s\n", (ਅੱਖਰ *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
ਸਥਿਰ ਖਾਲੀ test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_ਧਾਰਾ c_ਧਾਰਾ; /* compression stream */
    ਪੂਰਨ err;

    c_ਧਾਰਾ.zalloc = zalloc;
    c_ਧਾਰਾ.zfree = zfree;
    c_ਧਾਰਾ.opaque = (voidpf)0;

    err = deflateInit(&c_ਧਾਰਾ, Z_ਉੱਤਮ_ਰਫਤਾਰ);
    CHECK_ERR(err, "deflateInit");

    c_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = compr;
    c_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = uncompr;
    c_ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)uncomprLen;
    err = deflate(&c_ਧਾਰਾ, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    ਜੇ (c_ਧਾਰਾ.avail_ਵਿੱਚ != 0) {
        fprintf(stderr, "deflate not greedy\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_ਧਾਰਾ, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = compr;
    c_ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)uncomprLen/2;
    err = deflate(&c_ਧਾਰਾ, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_ਧਾਰਾ, Z_ਉੱਤਮ_COMPRESSION, Z_FILTERED);
    c_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = uncompr;
    c_ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)uncomprLen;
    err = deflate(&c_ਧਾਰਾ, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_ਧਾਰਾ, Z_ਅੰਤ);
    ਜੇ (err != Z_ਧਾਰਾ_ਸਿਰਾ) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    err = deflate_ਸਿਰਾ(&c_ਧਾਰਾ);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
ਸਥਿਰ ਖਾਲੀ test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    ਪੂਰਨ err;
    z_ਧਾਰਾ d_ਧਾਰਾ; /* decompression stream */

    strcpy((ਅੱਖਰ*)uncompr, "garbage");

    d_ਧਾਰਾ.zalloc = zalloc;
    d_ਧਾਰਾ.zfree = zfree;
    d_ਧਾਰਾ.opaque = (voidpf)0;

    d_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ  = compr;
    d_ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)comprLen;

    err = inflateInit(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateInit");

    ਲਈ (;;) {
        d_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = uncompr;            /* discard the output */
        d_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)uncomprLen;
        err = inflate(&d_ਧਾਰਾ, Z_NO_FLUSH);
        ਜੇ (err == Z_ਧਾਰਾ_ਸਿਰਾ) ਰੋਕੋ;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_ਸਿਰਾ(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateEnd");

    ਜੇ (d_ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ);
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    } ਨਹੀਂਤਾਂ {
        ਲਿਖੋ("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
ਸਥਿਰ ਖਾਲੀ test_flush(Byte *compr, uLong *comprLen) {
    z_ਧਾਰਾ c_ਧਾਰਾ; /* compression stream */
    ਪੂਰਨ err;
    uInt len = (uInt)ਲੜੀ_ਲੰਬਾਈ(hello)+1;

    c_ਧਾਰਾ.zalloc = zalloc;
    c_ਧਾਰਾ.zfree = zfree;
    c_ਧਾਰਾ.opaque = (voidpf)0;

    err = deflateInit(&c_ਧਾਰਾ, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ  = (z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)hello;
    c_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = compr;
    c_ਧਾਰਾ.avail_ਵਿੱਚ = 3;
    c_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)*comprLen;
    err = deflate(&c_ਧਾਰਾ, Z_ਭਰਿਆ_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_ਧਾਰਾ.avail_ਵਿੱਚ = len - 3;

    err = deflate(&c_ਧਾਰਾ, Z_ਅੰਤ);
    ਜੇ (err != Z_ਧਾਰਾ_ਸਿਰਾ) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_ਸਿਰਾ(&c_ਧਾਰਾ);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ;
}

/* ===========================================================================
 * Test inflateSync()
 */
ਸਥਿਰ ਖਾਲੀ test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    ਪੂਰਨ err;
    z_ਧਾਰਾ d_ਧਾਰਾ; /* decompression stream */

    strcpy((ਅੱਖਰ*)uncompr, "garbage");

    d_ਧਾਰਾ.zalloc = zalloc;
    d_ਧਾਰਾ.zfree = zfree;
    d_ਧਾਰਾ.opaque = (voidpf)0;

    d_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ  = compr;
    d_ਧਾਰਾ.avail_ਵਿੱਚ = 2; /* just read the zlib header */

    err = inflateInit(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateInit");

    d_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = uncompr;
    d_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)uncomprLen;

    err = inflate(&d_ਧਾਰਾ, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_ਧਾਰਾ);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_ਧਾਰਾ, Z_ਅੰਤ);
    ਜੇ (err != Z_ਧਾਰਾ_ਸਿਰਾ) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    err = inflate_ਸਿਰਾ(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateEnd");

    ਲਿਖੋ("after inflateSync(): hel%s\n", (ਅੱਖਰ *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
ਸਥਿਰ ਖਾਲੀ test_dict_deflate(Byte *compr, uLong comprLen) {
    z_ਧਾਰਾ c_ਧਾਰਾ; /* compression stream */
    ਪੂਰਨ err;

    c_ਧਾਰਾ.zalloc = zalloc;
    c_ਧਾਰਾ.zfree = zfree;
    c_ਧਾਰਾ.opaque = (voidpf)0;

    err = deflateInit(&c_ਧਾਰਾ, Z_ਉੱਤਮ_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_ਸੈੱਟ_Dictionary(&c_ਧਾਰਾ,
                (ਅਚਲ Bytef*)dictionary, (ਪੂਰਨ)ਆਕਾਰ(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_ਧਾਰਾ.adler;
    c_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = compr;
    c_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)comprLen;

    c_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = (z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)hello;
    c_ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)ਲੜੀ_ਲੰਬਾਈ(hello)+1;

    err = deflate(&c_ਧਾਰਾ, Z_ਅੰਤ);
    ਜੇ (err != Z_ਧਾਰਾ_ਸਿਰਾ) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }
    err = deflate_ਸਿਰਾ(&c_ਧਾਰਾ);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
ਸਥਿਰ ਖਾਲੀ test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    ਪੂਰਨ err;
    z_ਧਾਰਾ d_ਧਾਰਾ; /* decompression stream */

    strcpy((ਅੱਖਰ*)uncompr, "garbage");

    d_ਧਾਰਾ.zalloc = zalloc;
    d_ਧਾਰਾ.zfree = zfree;
    d_ਧਾਰਾ.opaque = (voidpf)0;

    d_ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ  = compr;
    d_ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)comprLen;

    err = inflateInit(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateInit");

    d_ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = uncompr;
    d_ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)uncomprLen;

    ਲਈ (;;) {
        err = inflate(&d_ਧਾਰਾ, Z_NO_FLUSH);
        ਜੇ (err == Z_ਧਾਰਾ_ਸਿਰਾ) ਰੋਕੋ;
        ਜੇ (err == Z_NEED_DICT) {
            ਜੇ (d_ਧਾਰਾ.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                ਬਾਹਰ_ਨਿਕਲੋ(1);
            }
            err = inflate_ਸੈੱਟ_Dictionary(&d_ਧਾਰਾ, (ਅਚਲ Bytef*)dictionary,
                                       (ਪੂਰਨ)ਆਕਾਰ(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_ਸਿਰਾ(&d_ਧਾਰਾ);
    CHECK_ERR(err, "inflateEnd");

    ਜੇ (strcmp((ਅੱਖਰ*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    } ਨਹੀਂਤਾਂ {
        ਲਿਖੋ("inflate with dictionary: %s\n", (ਅੱਖਰ *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ* my_ਸੰਸਕਰਣ_2 = ZLIB_ਸੰਸਕਰਣ;

    ਜੇ (zlib_ਸੰਸਕਰਣ()[0] != my_ਸੰਸਕਰਣ_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);

    } ਨਹੀਂਤਾਂ ਜੇ (strcmp(zlib_ਸੰਸਕਰਣ(), ZLIB_ਸੰਸਕਰਣ) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_ਸੰਸਕਰਣ());
    }

    ਲਿਖੋ("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_ਸੰਸਕਰਣ, (ਬਿਨਾਨਿਸ਼ਾਨੀ)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ((uInt)comprLen, 1);
    uncompr  = (Byte*)ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    ਜੇ (compr == Z_NULL || uncompr == Z_NULL) {
        ਲਿਖੋ("out of memory\n");
        ਬਾਹਰ_ਨਿਕਲੋ(1);
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO
    (ਖਾਲੀ)argc;
    (ਖਾਲੀ)argv;
#ਨਹੀਂਤਾਂ
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#ਜੇ_ਅੰਤ

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = 3 * uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    ਮੁਕਤ(compr);
    ਮੁਕਤ(uncompr);

    ਵਾਪਸ 0;
}
