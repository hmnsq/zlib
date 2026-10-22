/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#यदि defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#यदि_अन्तः

#अन्तर्भाव "zlib.h"
#अन्तर्भाव <stdio.h>

#यदि_परिभाषितम् STDC
#  include <string.h>
#  include <stdlib.h>
#यदि_अन्तः

#यदि defined(VMS)
#  define TESTFILE "foo-gz"
#नोचेद्यदि defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#अन्यथा
#  define TESTFILE "foo.gz"
#यदि_अन्तः

#परिभाषय CHECK_ERR(err, सन्देशः) { \
    यदि (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", सन्देशः, err); \
        निर्गच्छ(1); \
    } \
}

स्थिर z_const वर्ण hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

स्थिर ध्रुव वर्ण dictionary[] = "hello";
स्थिर uLong dictId;    /* Adler32 value of the dictionary */

#यदि_परिभाषितम् Z_SOLO

स्थिर शून्य *myalloc(शून्य *q, अचिह्नित n, अचिह्नित m) {
    (शून्य)q;
    प्रत्यागम शून्यस्मृतिं_देहि(n, m);
}

स्थिर शून्य myfree(शून्य *q, शून्य *p) {
    (शून्य)q;
    मुक्त(p);
}

स्थिर alloc_func zalloc = myalloc;
स्थिर free_func zfree = myfree;

#अन्यथा /* !Z_SOLO */

स्थिर alloc_func zalloc = (alloc_func)0;
स्थिर free_func zfree = (free_func)0;

/* ===========================================================================
 * Test compress() and uncompress()
 */
स्थिर शून्य test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    पूर्णाङ्क err;
    uLong len = (uLong)वाक्यदैर्घ्यम्(hello)+1;

    err = compress(compr, &comprLen, (ध्रुव Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    वाक्यं_प्रतिलिख((वर्ण*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    यदि (वाक्यं_तुलय((वर्ण*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        निर्गच्छ(1);
    } अन्यथा {
        प्रारूप_लिख("uncompress(): %s\n", (वर्ण *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
स्थिर शून्य test_gzio(ध्रुव वर्ण *fname, Byte *uncompr, uLong uncomprLen) {
#यदि_परिभाषितम् NO_GZCOMPRESS
    fprintf(stderr, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#अन्यथा
    पूर्णाङ्क err;
    पूर्णाङ्क len = (पूर्णाङ्क)वाक्यदैर्घ्यम्(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    यदि (file == रिक्त) {
        fprintf(stderr, "gzopen error\n");
        निर्गच्छ(1);
    }
    gzputc(file, 'h');
    यदि (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        निर्गच्छ(1);
    }
    यदि (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        निर्गच्छ(1);
    }
    gzseek(file, 1L, SEEK_CUR); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    यदि (file == रिक्त) {
        fprintf(stderr, "gzopen error\n");
        निर्गच्छ(1);
    }
    वाक्यं_प्रतिलिख((वर्ण*)uncompr, "garbage");

    यदि (gzread(file, uncompr, (अचिह्नित)uncomprLen) != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
        निर्गच्छ(1);
    }
    यदि (वाक्यं_तुलय((वर्ण*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (वर्ण*)uncompr);
        निर्गच्छ(1);
    } अन्यथा {
        प्रारूप_लिख("gzread(): %s\n", (वर्ण*)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    यदि (pos != 6 || gztell(file) != pos) {
        fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
                (दीर्घ)pos, (दीर्घ)gztell(file));
        निर्गच्छ(1);
    }

    यदि (gzgetc(file) != ' ') {
        fprintf(stderr, "gzgetc error\n");
        निर्गच्छ(1);
    }

    यदि (gzungetc(' ', file) != ' ') {
        fprintf(stderr, "gzungetc error\n");
        निर्गच्छ(1);
    }

    gzgets(file, (वर्ण*)uncompr, (पूर्णाङ्क)uncomprLen);
    यदि (वाक्यदैर्घ्यम्((वर्ण*)uncompr) != 7) { /* " hello!" */
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        निर्गच्छ(1);
    }
    यदि (वाक्यं_तुलय((वर्ण*)uncompr, hello + 6)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
        निर्गच्छ(1);
    } अन्यथा {
        प्रारूप_लिख("gzgets() after gzseek: %s\n", (वर्ण*)uncompr);
    }

    gzclose(file);
#यदि_अन्तः
}

#यदि_अन्तः /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
स्थिर शून्य test_deflate(Byte *compr, uLong comprLen) {
    z_प्रवाहः c_प्रवाहः; /* compression stream */
    पूर्णाङ्क err;
    uLong len = (uLong)वाक्यदैर्घ्यम्(hello)+1;

    c_प्रवाहः.zalloc = zalloc;
    c_प्रवाहः.zfree = zfree;
    c_प्रवाहः.opaque = (voidpf)0;

    err = deflateInit(&c_प्रवाहः, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_प्रवाहः.अग्र_अन्तर्  = (z_const अचिह्नित वर्ण *)hello;
    c_प्रवाहः.अग्र_बहिर्गमनम् = compr;

    यावत् (c_प्रवाहः.योग_अन्तर् != len && c_प्रवाहः.योग_बहिर्गमनम् < comprLen) {
        c_प्रवाहः.avail_अन्तर् = c_प्रवाहः.avail_बहिर्गमनम् = 1; /* force small buffers */
        err = deflate(&c_प्रवाहः, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    प्रति (;;) {
        c_प्रवाहः.avail_बहिर्गमनम् = 1;
        err = deflate(&c_प्रवाहः, Z_FINISH);
        यदि (err == Z_प्रवाहः_अन्तः) भङ्ग;
        CHECK_ERR(err, "deflate");
    }

    err = deflate_अन्तः(&c_प्रवाहः);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
स्थिर शून्य test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    पूर्णाङ्क err;
    z_प्रवाहः d_प्रवाहः; /* decompression stream */

    वाक्यं_प्रतिलिख((वर्ण*)uncompr, "garbage");

    d_प्रवाहः.zalloc = zalloc;
    d_प्रवाहः.zfree = zfree;
    d_प्रवाहः.opaque = (voidpf)0;

    d_प्रवाहः.अग्र_अन्तर्  = compr;
    d_प्रवाहः.avail_अन्तर् = 0;
    d_प्रवाहः.अग्र_बहिर्गमनम् = uncompr;

    err = inflateInit(&d_प्रवाहः);
    CHECK_ERR(err, "inflateInit");

    यावत् (d_प्रवाहः.योग_बहिर्गमनम् < uncomprLen && d_प्रवाहः.योग_अन्तर् < comprLen) {
        d_प्रवाहः.avail_अन्तर् = d_प्रवाहः.avail_बहिर्गमनम् = 1; /* force small buffers */
        err = inflate(&d_प्रवाहः, Z_NO_FLUSH);
        यदि (err == Z_प्रवाहः_अन्तः) भङ्ग;
        CHECK_ERR(err, "inflate");
    }

    err = inflate_अन्तः(&d_प्रवाहः);
    CHECK_ERR(err, "inflateEnd");

    यदि (वाक्यं_तुलय((वर्ण*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
        निर्गच्छ(1);
    } अन्यथा {
        प्रारूप_लिख("inflate(): %s\n", (वर्ण *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
स्थिर शून्य test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_प्रवाहः c_प्रवाहः; /* compression stream */
    पूर्णाङ्क err;

    c_प्रवाहः.zalloc = zalloc;
    c_प्रवाहः.zfree = zfree;
    c_प्रवाहः.opaque = (voidpf)0;

    err = deflateInit(&c_प्रवाहः, Z_BEST_जवः);
    CHECK_ERR(err, "deflateInit");

    c_प्रवाहः.अग्र_बहिर्गमनम् = compr;
    c_प्रवाहः.avail_बहिर्गमनम् = (uInt)comprLen;

    /* At this point, uncompr is still mostly zeroes, so it should compress
     * very well:
     */
    c_प्रवाहः.अग्र_अन्तर् = uncompr;
    c_प्रवाहः.avail_अन्तर् = (uInt)uncomprLen;
    err = deflate(&c_प्रवाहः, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    यदि (c_प्रवाहः.avail_अन्तर् != 0) {
        fprintf(stderr, "deflate not greedy\n");
        निर्गच्छ(1);
    }

    /* Feed in already compressed data and switch to no compression: */
    deflateParams(&c_प्रवाहः, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_प्रवाहः.अग्र_अन्तर् = compr;
    c_प्रवाहः.avail_अन्तर् = (uInt)uncomprLen/2;
    err = deflate(&c_प्रवाहः, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    /* Switch back to compressing mode: */
    deflateParams(&c_प्रवाहः, Z_BEST_COMPRESSION, Z_FILTERED);
    c_प्रवाहः.अग्र_अन्तर् = uncompr;
    c_प्रवाहः.avail_अन्तर् = (uInt)uncomprLen;
    err = deflate(&c_प्रवाहः, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_प्रवाहः, Z_FINISH);
    यदि (err != Z_प्रवाहः_अन्तः) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        निर्गच्छ(1);
    }
    err = deflate_अन्तः(&c_प्रवाहः);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
स्थिर शून्य test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    पूर्णाङ्क err;
    z_प्रवाहः d_प्रवाहः; /* decompression stream */

    वाक्यं_प्रतिलिख((वर्ण*)uncompr, "garbage");

    d_प्रवाहः.zalloc = zalloc;
    d_प्रवाहः.zfree = zfree;
    d_प्रवाहः.opaque = (voidpf)0;

    d_प्रवाहः.अग्र_अन्तर्  = compr;
    d_प्रवाहः.avail_अन्तर् = (uInt)comprLen;

    err = inflateInit(&d_प्रवाहः);
    CHECK_ERR(err, "inflateInit");

    प्रति (;;) {
        d_प्रवाहः.अग्र_बहिर्गमनम् = uncompr;            /* discard the output */
        d_प्रवाहः.avail_बहिर्गमनम् = (uInt)uncomprLen;
        err = inflate(&d_प्रवाहः, Z_NO_FLUSH);
        यदि (err == Z_प्रवाहः_अन्तः) भङ्ग;
        CHECK_ERR(err, "large inflate");
    }

    err = inflate_अन्तः(&d_प्रवाहः);
    CHECK_ERR(err, "inflateEnd");

    यदि (d_प्रवाहः.योग_बहिर्गमनम् != 2*uncomprLen + uncomprLen/2) {
        fprintf(stderr, "bad large inflate: %lu\n", d_प्रवाहः.योग_बहिर्गमनम्);
        निर्गच्छ(1);
    } अन्यथा {
        प्रारूप_लिख("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
स्थिर शून्य test_flush(Byte *compr, uLong *comprLen) {
    z_प्रवाहः c_प्रवाहः; /* compression stream */
    पूर्णाङ्क err;
    uInt len = (uInt)वाक्यदैर्घ्यम्(hello)+1;

    c_प्रवाहः.zalloc = zalloc;
    c_प्रवाहः.zfree = zfree;
    c_प्रवाहः.opaque = (voidpf)0;

    err = deflateInit(&c_प्रवाहः, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_प्रवाहः.अग्र_अन्तर्  = (z_const अचिह्नित वर्ण *)hello;
    c_प्रवाहः.अग्र_बहिर्गमनम् = compr;
    c_प्रवाहः.avail_अन्तर् = 3;
    c_प्रवाहः.avail_बहिर्गमनम् = (uInt)*comprLen;
    err = deflate(&c_प्रवाहः, Z_पूर्णम्_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_प्रवाहः.avail_अन्तर् = len - 3;

    err = deflate(&c_प्रवाहः, Z_FINISH);
    यदि (err != Z_प्रवाहः_अन्तः) {
        CHECK_ERR(err, "deflate");
    }
    err = deflate_अन्तः(&c_प्रवाहः);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_प्रवाहः.योग_बहिर्गमनम्;
}

/* ===========================================================================
 * Test inflateSync()
 */
स्थिर शून्य test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    पूर्णाङ्क err;
    z_प्रवाहः d_प्रवाहः; /* decompression stream */

    वाक्यं_प्रतिलिख((वर्ण*)uncompr, "garbage");

    d_प्रवाहः.zalloc = zalloc;
    d_प्रवाहः.zfree = zfree;
    d_प्रवाहः.opaque = (voidpf)0;

    d_प्रवाहः.अग्र_अन्तर्  = compr;
    d_प्रवाहः.avail_अन्तर् = 2; /* just read the zlib header */

    err = inflateInit(&d_प्रवाहः);
    CHECK_ERR(err, "inflateInit");

    d_प्रवाहः.अग्र_बहिर्गमनम् = uncompr;
    d_प्रवाहः.avail_बहिर्गमनम् = (uInt)uncomprLen;

    err = inflate(&d_प्रवाहः, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_प्रवाहः.avail_अन्तर् = (uInt)comprLen-2;   /* read all compressed data */
    err = inflateSync(&d_प्रवाहः);           /* but skip the damaged part */
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_प्रवाहः, Z_FINISH);
    यदि (err != Z_प्रवाहः_अन्तः) {
        fprintf(stderr, "inflate should report Z_STREAM_END\n");
        निर्गच्छ(1);
    }
    err = inflate_अन्तः(&d_प्रवाहः);
    CHECK_ERR(err, "inflateEnd");

    प्रारूप_लिख("after inflateSync(): hel%s\n", (वर्ण *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
स्थिर शून्य test_dict_deflate(Byte *compr, uLong comprLen) {
    z_प्रवाहः c_प्रवाहः; /* compression stream */
    पूर्णाङ्क err;

    c_प्रवाहः.zalloc = zalloc;
    c_प्रवाहः.zfree = zfree;
    c_प्रवाहः.opaque = (voidpf)0;

    err = deflateInit(&c_प्रवाहः, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflate_समुच्चयः_Dictionary(&c_प्रवाहः,
                (ध्रुव Bytef*)dictionary, (पूर्णाङ्क)परिमाण(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_प्रवाहः.adler;
    c_प्रवाहः.अग्र_बहिर्गमनम् = compr;
    c_प्रवाहः.avail_बहिर्गमनम् = (uInt)comprLen;

    c_प्रवाहः.अग्र_अन्तर् = (z_const अचिह्नित वर्ण *)hello;
    c_प्रवाहः.avail_अन्तर् = (uInt)वाक्यदैर्घ्यम्(hello)+1;

    err = deflate(&c_प्रवाहः, Z_FINISH);
    यदि (err != Z_प्रवाहः_अन्तः) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
        निर्गच्छ(1);
    }
    err = deflate_अन्तः(&c_प्रवाहः);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
स्थिर शून्य test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    पूर्णाङ्क err;
    z_प्रवाहः d_प्रवाहः; /* decompression stream */

    वाक्यं_प्रतिलिख((वर्ण*)uncompr, "garbage");

    d_प्रवाहः.zalloc = zalloc;
    d_प्रवाहः.zfree = zfree;
    d_प्रवाहः.opaque = (voidpf)0;

    d_प्रवाहः.अग्र_अन्तर्  = compr;
    d_प्रवाहः.avail_अन्तर् = (uInt)comprLen;

    err = inflateInit(&d_प्रवाहः);
    CHECK_ERR(err, "inflateInit");

    d_प्रवाहः.अग्र_बहिर्गमनम् = uncompr;
    d_प्रवाहः.avail_बहिर्गमनम् = (uInt)uncomprLen;

    प्रति (;;) {
        err = inflate(&d_प्रवाहः, Z_NO_FLUSH);
        यदि (err == Z_प्रवाहः_अन्तः) भङ्ग;
        यदि (err == Z_NEED_DICT) {
            यदि (d_प्रवाहः.adler != dictId) {
                fprintf(stderr, "unexpected dictionary");
                निर्गच्छ(1);
            }
            err = inflate_समुच्चयः_Dictionary(&d_प्रवाहः, (ध्रुव Bytef*)dictionary,
                                       (पूर्णाङ्क)परिमाण(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflate_अन्तः(&d_प्रवाहः);
    CHECK_ERR(err, "inflateEnd");

    यदि (वाक्यं_तुलय((वर्ण*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
        निर्गच्छ(1);
    } अन्यथा {
        प्रारूप_लिख("inflate with dictionary: %s\n", (वर्ण *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    स्थिर ध्रुव वर्ण* my_संस्करणम्_2 = ZLIB_संस्करणम्;

    यदि (zlib_संस्करणम्()[0] != my_संस्करणम्_2[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        निर्गच्छ(1);

    } अन्यथा यदि (वाक्यं_तुलय(zlib_संस्करणम्(), ZLIB_संस्करणम्) != 0) {
        fprintf(stderr, "warning: different zlib version linked: %s\n",
                zlib_संस्करणम्());
    }

    प्रारूप_लिख("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_संस्करणम्, (अचिह्नित)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)शून्यस्मृतिं_देहि((uInt)comprLen, 1);
    uncompr  = (Byte*)शून्यस्मृतिं_देहि((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    यदि (compr == Z_NULL || uncompr == Z_NULL) {
        प्रारूप_लिख("out of memory\n");
        निर्गच्छ(1);
    }

#यदि_परिभाषितम् Z_SOLO
    (शून्य)argc;
    (शून्य)argv;
#अन्यथा
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              uncompr, uncomprLen);
#यदि_अन्तः

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = 3 * uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    मुक्त(compr);
    मुक्त(uncompr);

    प्रत्यागम 0;
}
