/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#जर defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#अन्तर्भाव "zlib.h"
#अन्तर्भाव <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#जर defined(VMS)
#  define TESTFILE "foo-gz"
#किंवाजर defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define TESTFILE "foo/gz"
#अन्यथा
#  define TESTFILE "foo.gz"
#endif

#define CHECK_ERR(err, संदेश) { \
    जर (err != Z_OK) { \
        ख_लिखो(मानक_त्रुटि, "%s error: %d\n", संदेश, err); \
        निकास(1); \
    } \
}

स्थिर z_const अक्षर hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

स्थिर स्थिरांक अक्षर dictionary[] = "hello";
स्थिर uLong dictId;    /* Adler32 value of the dictionary */

#ifdef Z_SOLO

स्थिर शून्य *myalloc(शून्य *q, अचिन्हित n, अचिन्हित m) {
    (शून्य)q;
    परत सुस्मृति(n, m);
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
    पूर्णांक err;
    uLong len = (uLong)म_माप(hello)+1;

    err = compress(compr, &comprLen, (स्थिरांक Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    म_नकल((अक्षर*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    जर (म_भेद((अक्षर*)uncompr, hello)) {
        ख_लिखो(मानक_त्रुटि, "bad uncompress\n");
        निकास(1);
    } अन्यथा {
        म_लिखो("uncompress(): %s\n", (अक्षर *)uncompr);
    }
}

/* ===========================================================================
 * Test read/write of .gz files
 */
स्थिर शून्य test_gzio(स्थिरांक अक्षर *fname, Byte *uncompr, uLong uncomprLen) {
#ifdef NO_GZCOMPRESS
    ख_लिखो(मानक_त्रुटि, "NO_GZCOMPRESS -- gz* functions cannot compress\n");
#अन्यथा
    पूर्णांक err;
    पूर्णांक len = (पूर्णांक)म_माप(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(fname, "wb");
    जर (file == रिक्त) {
        ख_लिखो(मानक_त्रुटि, "gzopen error\n");
        निकास(1);
    }
    gzputc(file, 'h');
    जर (gzputs(file, "ello") != 4) {
        ख_लिखो(मानक_त्रुटि, "gzputs err: %s\n", gzerror(file, &err));
        निकास(1);
    }
    जर (gzprintf(file, ", %s!", "hello") != 8) {
        ख_लिखो(मानक_त्रुटि, "gzprintf err: %s\n", gzerror(file, &err));
        निकास(1);
    }
    gzseek(file, 1L, प्रस्तुत_से); /* add one zero byte */
    gzclose(file);

    file = gzopen(fname, "rb");
    जर (file == रिक्त) {
        ख_लिखो(मानक_त्रुटि, "gzopen error\n");
        निकास(1);
    }
    म_नकल((अक्षर*)uncompr, "garbage");

    जर (gzread(file, uncompr, (अचिन्हित)uncomprLen) != len) {
        ख_लिखो(मानक_त्रुटि, "gzread err: %s\n", gzerror(file, &err));
        निकास(1);
    }
    जर (म_भेद((अक्षर*)uncompr, hello)) {
        ख_लिखो(मानक_त्रुटि, "bad gzread: %s\n", (अक्षर*)uncompr);
        निकास(1);
    } अन्यथा {
        म_लिखो("gzread(): %s\n", (अक्षर*)uncompr);
    }

    pos = gzseek(file, -8L, प्रस्तुत_से);
    जर (pos != 6 || gztell(file) != pos) {
        ख_लिखो(मानक_त्रुटि, "gzseek error, pos=%ld, gztell=%ld\n",
                (दीर्घ)pos, (दीर्घ)gztell(file));
        निकास(1);
    }

    जर (gzgetc(file) != ' ') {
        ख_लिखो(मानक_त्रुटि, "gzgetc error\n");
        निकास(1);
    }

    जर (gzungetc(' ', file) != ' ') {
        ख_लिखो(मानक_त्रुटि, "gzungetc error\n");
        निकास(1);
    }

    gzgets(file, (अक्षर*)uncompr, (पूर्णांक)uncomprLen);
    जर (म_माप((अक्षर*)uncompr) != 7) { /* " hello!" */
        ख_लिखो(मानक_त्रुटि, "gzgets err after gzseek: %s\n", gzerror(file, &err));
        निकास(1);
    }
    जर (म_भेद((अक्षर*)uncompr, hello + 6)) {
        ख_लिखो(मानक_त्रुटि, "bad gzgets after gzseek\n");
        निकास(1);
    } अन्यथा {
        म_लिखो("gzgets() after gzseek: %s\n", (अक्षर*)uncompr);
    }

    gzclose(file);
#endif
}

#endif /* Z_SOLO */

/* ===========================================================================
 * Test deflate() with small buffers
 */
स्थिर शून्य test_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    पूर्णांक err;
    uLong len = (uLong)म_माप(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const अचिन्हित अक्षर *)hello;
    c_stream.next_out = compr;

    पर्यंत (c_stream.total_in != len && c_stream.total_out < comprLen) {
        c_stream.avail_in = c_stream.avail_out = 1; /* force small buffers */
        err = deflate(&c_stream, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
    /* Finish the stream, still forcing small buffers: */
    साठी (;;) {
        c_stream.avail_out = 1;
        err = deflate(&c_stream, Z_FINISH);
        जर (err == Z_STREAM_END) थांबा;
        CHECK_ERR(err, "deflate");
    }

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with small buffers
 */
स्थिर शून्य test_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                  uLong uncomprLen) {
    पूर्णांक err;
    z_stream d_stream; /* decompression stream */

    म_नकल((अक्षर*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    पर्यंत (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
        जर (err == Z_STREAM_END) थांबा;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    जर (म_भेद((अक्षर*)uncompr, hello)) {
        ख_लिखो(मानक_त्रुटि, "bad inflate\n");
        निकास(1);
    } अन्यथा {
        म_लिखो("inflate(): %s\n", (अक्षर *)uncompr);
    }
}

/* ===========================================================================
 * Test deflate() with large buffers and dynamic change of compression level
 */
स्थिर शून्य test_large_deflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    z_stream c_stream; /* compression stream */
    पूर्णांक err;

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
    जर (c_stream.avail_in != 0) {
        ख_लिखो(मानक_त्रुटि, "deflate not greedy\n");
        निकास(1);
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
    जर (err != Z_STREAM_END) {
        ख_लिखो(मानक_त्रुटि, "deflate should report Z_STREAM_END\n");
        निकास(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with large buffers
 */
स्थिर शून्य test_large_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                        uLong uncomprLen) {
    पूर्णांक err;
    z_stream d_stream; /* decompression stream */

    म_नकल((अक्षर*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    साठी (;;) {
        d_stream.next_out = uncompr;            /* discard the output */
        d_stream.avail_out = (uInt)uncomprLen;
        err = inflate(&d_stream, Z_NO_FLUSH);
        जर (err == Z_STREAM_END) थांबा;
        CHECK_ERR(err, "large inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    जर (d_stream.total_out != 2*uncomprLen + uncomprLen/2) {
        ख_लिखो(मानक_त्रुटि, "bad large inflate: %lu\n", d_stream.total_out);
        निकास(1);
    } अन्यथा {
        म_लिखो("large_inflate(): OK\n");
    }
}

/* ===========================================================================
 * Test deflate() with full flush
 */
स्थिर शून्य test_flush(Byte *compr, uLong *comprLen) {
    z_stream c_stream; /* compression stream */
    पूर्णांक err;
    uInt len = (uInt)म_माप(hello)+1;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (z_const अचिन्हित अक्षर *)hello;
    c_stream.next_out = compr;
    c_stream.avail_in = 3;
    c_stream.avail_out = (uInt)*comprLen;
    err = deflate(&c_stream, Z_FULL_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++; /* force an error in first compressed block */
    c_stream.avail_in = len - 3;

    err = deflate(&c_stream, Z_FINISH);
    जर (err != Z_STREAM_END) {
        CHECK_ERR(err, "deflate");
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_stream.total_out;
}

/* ===========================================================================
 * Test inflateSync()
 */
स्थिर शून्य test_sync(Byte *compr, uLong comprLen, Byte *uncompr,
                      uLong uncomprLen) {
    पूर्णांक err;
    z_stream d_stream; /* decompression stream */

    म_नकल((अक्षर*)uncompr, "garbage");

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
    जर (err != Z_STREAM_END) {
        ख_लिखो(मानक_त्रुटि, "inflate should report Z_STREAM_END\n");
        निकास(1);
    }
    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    म_लिखो("after inflateSync(): hel%s\n", (अक्षर *)uncompr);
}

/* ===========================================================================
 * Test deflate() with preset dictionary
 */
स्थिर शून्य test_dict_deflate(Byte *compr, uLong comprLen) {
    z_stream c_stream; /* compression stream */
    पूर्णांक err;

    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflateSetDictionary(&c_stream,
                (स्थिरांक Bytef*)dictionary, (पूर्णांक)आकार(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_stream.adler;
    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    c_stream.next_in = (z_const अचिन्हित अक्षर *)hello;
    c_stream.avail_in = (uInt)म_माप(hello)+1;

    err = deflate(&c_stream, Z_FINISH);
    जर (err != Z_STREAM_END) {
        ख_लिखो(मानक_त्रुटि, "deflate should report Z_STREAM_END\n");
        निकास(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

/* ===========================================================================
 * Test inflate() with a preset dictionary
 */
स्थिर शून्य test_dict_inflate(Byte *compr, uLong comprLen, Byte *uncompr,
                       uLong uncomprLen) {
    पूर्णांक err;
    z_stream d_stream; /* decompression stream */

    म_नकल((अक्षर*)uncompr, "garbage");

    d_stream.zalloc = zalloc;
    d_stream.zfree = zfree;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    साठी (;;) {
        err = inflate(&d_stream, Z_NO_FLUSH);
        जर (err == Z_STREAM_END) थांबा;
        जर (err == Z_NEED_DICT) {
            जर (d_stream.adler != dictId) {
                ख_लिखो(मानक_त्रुटि, "unexpected dictionary");
                निकास(1);
            }
            err = inflateSetDictionary(&d_stream, (स्थिरांक Bytef*)dictionary,
                                       (पूर्णांक)आकार(dictionary));
        }
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    जर (म_भेद((अक्षर*)uncompr, hello)) {
        ख_लिखो(मानक_त्रुटि, "bad inflate with dict\n");
        निकास(1);
    } अन्यथा {
        म_लिखो("inflate with dictionary: %s\n", (अक्षर *)uncompr);
    }
}

/* ===========================================================================
 * Usage:  example [output.gz  [input.gz]]
 */

पूर्णांक main(पूर्णांक argc, अक्षर *argv[]) {
    Byte *compr, *uncompr;
    uLong uncomprLen = 20000;
    uLong comprLen = 3 * uncomprLen;
    स्थिर स्थिरांक अक्षर* myVersion = ZLIB_VERSION;

    जर (zlibVersion()[0] != myVersion[0]) {
        ख_लिखो(मानक_त्रुटि, "incompatible zlib version\n");
        निकास(1);

    } अन्यथा जर (म_भेद(zlibVersion(), ZLIB_VERSION) != 0) {
        ख_लिखो(मानक_त्रुटि, "warning: different zlib version linked: %s\n",
                zlibVersion());
    }

    म_लिखो("zlib version %s = 0x%04x, compile flags = 0x%lx\n",
            ZLIB_VERSION, (अचिन्हित)ZLIB_VERNUM, zlibCompileFlags());

    compr    = (Byte*)सुस्मृति((uInt)comprLen, 1);
    uncompr  = (Byte*)सुस्मृति((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    जर (compr == Z_NULL || uncompr == Z_NULL) {
        म_लिखो("out of memory\n");
        निकास(1);
    }

#ifdef Z_SOLO
    (शून्य)argc;
    (शून्य)argv;
#अन्यथा
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

    मुक्त(compr);
    मुक्त(uncompr);

    परत 0;
}
