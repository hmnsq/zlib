/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#જો defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#અન્તર્ભાવ "zlib.h"
#અન્તર્ભાવ <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#જો defined(VMS)
#  define TESTFILE "foo-gz"
#અથવાજો defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#નહીંતર
#  define TESTFILE "foo.gz"
#endif

#define CHECK_ERR(err, સંદેશ) { \
    જો (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", સંદેશ, err); \
        exit(1); \
    } \
}

સ્થિર z_const અક્ષર hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

સ્થિર અચલ અક્ષર dictionary[] = "hello";
સ્થિર uLong dictId;    /* Adler32 value of the dictionary */

#ifdef Z_SOLO

સ્થિર શૂન્ય *myalloc(શૂન્ય *q, unsigned n, unsigned m) {
    (શૂન્ય)q;
    પાછા calloc(n, m);
}

સ્થિર શૂન્ય myfree(શૂન્ય *q, શૂન્ય *p) {
    (શૂન્ય)q;
    મુક્ત(p);
}

સ્થિર alloc_func zalloc = myalloc;
સ્થિર free_func zfree = myfree;

#નહીંતર /* !Z_SOLO */

સ્થિર alloc_func zalloc = (alloc_func)0;
સ્થિર free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
સ્થિર શૂન્ય test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    પૂર્ણાંક err;
    uLong len = (uLong)strlen(hello)+1;

    err = compress(compr, &comprLen, (અચલ Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((અક્ષર*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    જો (strcmp((અક્ષર*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        exit(1);
    } નહીંતર {
        printf("uncompress(): %s\n", (અક્ષર *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
સ્થિર શૂન્ય test_gzio(અચલ અક્ષર *fname, Byte *uncompr, uLong uncomprLen) {
#ifdef NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#નહીંતર
    પૂર્ણાંક err;
    પૂર્ણાંક len = (પૂર્ણાંક)strlen(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    જો (file == ખાલી) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    gzputc(file, 'h');
    જો (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        exit(1);
    }
    જો (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        exit(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    જો (file == ખાલી) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    strcpy((અક્ષર*)uncompr, "garbage");

    જો (gzread(file, uncompr, (unsigned)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        exit(1);
    }
    જો (strcmp((અક્ષર*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (અક્ષર*)uncompr);
        exit(1);
    } નહીંતર {
        printf("gzread(): %s\n", (અક્ષર*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    જો (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (long)pos, (long)gztell(file));
        exit(1);
    }

    જો (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        exit(1);
    }

    જો (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        exit(1);
    }

    gzgets(file, (અક્ષર*)uncompr, (પૂર્ણાંક)uncomprLen);
    જો (strlen((અક્ષર*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        exit(1);
    }
    જો (strcmp((અક્ષર*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        exit(1);
    } નહીંતર {
        printf("gzgets() after gzseek: %s\n", (અક્ષર*)uncompr);
    }

    gzclose(file);
#endif
}

#endif /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
સ્થિર શૂન્ય test_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    પૂર્ણાંક err;
    uLong len = (uLong)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned અક્ષર *)hello;
    c_stream.next_out = compr;

    જ્યારે (c_stream.total_in != len && c_stream.total_out < comprLen) {
        c_stream.avail_in = c_stream.avail_out = 1; /* force small buffers */
        err = deflate(&c_stream, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    માટે (;;) {
        c_stream.avail_out = 1;
        err = deflate(&c_stream, Z_FINISH);
        જો (err == Z_STREAM_END) તોડો;
        CHECK_ERR(err, "deflate");
    }

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
સ્થિર શૂન્ય test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    પૂર્ણાંક err;
    z_stream d_stream; /* decompression stream */

    strcpy((અક્ષર*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    જ્યારે (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
        જો (err == Z_STREAM_END) તોડો;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    જો (strcmp((અક્ષર*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        exit(1);
    } નહીંતર {
        printf("inflate(): %s\n", (અક્ષર *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
સ્થિર શૂન્ય test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_stream c_stream; /* compression stream */
    પૂર્ણાંક err;

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
    જો (c_stream.avail_in != 0) {
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
    જો (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
સ્થિર શૂન્ય test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    પૂર્ણાંક err;
    z_stream d_stream; /* decompression stream */

    strcpy((અક્ષર*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    માટે (;;) {
        d_stream.next_out = uncompr;            /* discard the output */
        d_stream.avail_out = (uInt)uncomprLen;
        err = inflate(&d_stream, Z_NO_FLUSH);
        જો (err == Z_STREAM_END) તોડો;
        CHECK_ERR(err, "large inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    જો (d_stream.total_out != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_stream.total_out);
        exit(1);
    } નહીંતર {
        printf("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
સ્થિર શૂન્ય test_flush(Byte *compr, uLong *comprLen) {
    z_stream c_stream; /* compression stream */
    પૂર્ણાંક err;
    uInt len = (uInt)strlen(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const unsigned અક્ષર *)hello;
    c_stream.next_out = compr;
    c_stream.avail_in = 3;
    c_stream.avail_out = (uInt)*comprLen;
    err = deflate(&c_stream, Z_FULL_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_stream.avail_in = len - 3;

    err = deflate(&c_stream, Z_FINISH);
    જો (err != Z_STREAM_END) {
        CHECK_ERR(err, "deflate");
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_stream.total_out;
}

/* ===========================================================================
 * Test inflateSync()
 */
સ્થિર શૂન્ય test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    પૂર્ણાંક err;
    z_stream d_stream; /* decompression stream */

    strcpy((અક્ષર*)uncompr, "garbage");

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
    જો (err != Z_STREAM_END) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    printf("after inflateSync(): hel%s\n", (અક્ષર *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
સ્થિર શૂન્ય test_dict_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    પૂર્ણાંક err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflateSetDictionary(&c_stream,
                (અચલ Bytef*)dictionary, (પૂર્ણાંક)કદ(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_stream.adler;
    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    c_stream.next_in = (z_const unsigned અક્ષર *)hello;
    c_stream.avail_in = (uInt)strlen(hello)+1;

    err = deflate(&c_stream, Z_FINISH);
    જો (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
સ્થિર શૂન્ય test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    પૂર્ણાંક err;
    z_stream d_stream; /* decompression stream */

    strcpy((અક્ષર*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    માટે (;;) {
        err = inflate(&d_stream, Z_NO_FLUSH);
        જો (err == Z_STREAM_END) તોડો;
        જો (err == Z_NEED_DICT) {
            જો (d_stream.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                exit(1);
            }
            err = inflateSetDictionary(&d_stream, (અચલ Bytef*)dictionary,
                                       (પૂર્ણાંક)કદ(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    જો (strcmp((અક્ષર*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        exit(1);
    } નહીંતર {
        printf("inflate with dictionary: %s\n", (અક્ષર *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    સ્થિર અચલ અક્ષર* myVersion = ZLIB_VERSION;

    જો (zlibVersion()[0] != myVersion[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        exit(1);

    } નહીંતર જો (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
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
    જો (compr == Z_NULL || uncompr == Z_NULL) {
        printf("out of memory\n");
        exit(1);
    }

#ifdef Z_SOLO
    (શૂન્ય)argc;
    (શૂન્ય)argv;
#નહીંતર
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

    મુક્ત(compr);
    મુક્ત(uncompr);

    પાછા 0;
}
