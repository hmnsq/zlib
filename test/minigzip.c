/* minigzip.c -- simulate gzip using the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * minigzip is a minimal implementation of the gzip utility. This is
 * only an example of using zlib and isn't meant to replace the
 * full-featured gzip. No attempt is made to deal with file systems
 * limiting names to 14 or 8+3 characters, etc... Error checking is
 * very limited. So use minigzip only for testing; use gzip for the
 * real thing. On MSDOS, use only on file names without extension
 * or in pipe mode.
 */

/* @(#) $Id$ */

#ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#endif

#जर defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#जर defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#अन्तर्भाव "zlib.h"
#अन्तर्भाव <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#जर defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_द्विमान_MODE(file) setmode(fileno(file), O_द्विमान)
#अन्यथा
#  define SET_द्विमान_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#जर defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink हटाओ
#    define fileno(file) file->__file
#  endif
#endif
#जर defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#जर !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  बाह्य पूर्णांक unlink(स्थिरांक अक्षर *);
#endif
#endif

#जर defined(UNDER_CE)
#  include <windows.h>
#  define लिखो_त्रुटि(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

स्थिर अक्षर *strwinerror (error)
     DWORD error;
{
    स्थिर अक्षर buf[1024];

    ब_अक्षर_प्रकार *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        रिक्त,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        रिक्त);
    जर (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        जर (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        जर (chars > आकार (buf) - 1) {
            chars = आकार (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    अन्यथा {
        प्र_लिखो(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    परत buf;
}

स्थिर शून्य pwinerror (s)
    स्थिरांक अक्षर *s;
{
    जर (s && *s)
        ख_लिखो(मानक_त्रुटि, "%s: %s\n", s, strwinerror(GetLastError ()));
    अन्यथा
        ख_लिखो(मानक_त्रुटि, "%s\n", strwinerror(GetLastError ()));
}

#endif /* UNDER_CE */

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (आकार(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local स्थिर
   /* Needed for systems with limitation on stack size. */
#अन्यथा
#  define local
#endif

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
स्थिर अक्षर *string_copy(अक्षर *dst, अक्षर स्थिरांक *src, z_size_t len) {
    जर (len == 0)
        परत रिक्त;
    पर्यंत (--len) {
        *dst = *src++;
        जर (*dst == 0)
            परत dst;
        dst++;
    }
    *dst = 0;
    परत dst;
}

#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#जर defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

स्थिर शून्य *myalloc(शून्य *q, अचिन्हित n, अचिन्हित m) {
    (शून्य)q;
    परत सुस्मृति(n, m);
}

स्थिर शून्य myfree(शून्य *q, शून्य *p) {
    (शून्य)q;
    मुक्त(p);
}

प्रकारव्याख्या संरचना gzFile_s {
    खाता *file;
    पूर्णांक लिख;
    पूर्णांक err;
    अक्षर *संदेश;
    z_stream strm;
} *gzFile;

स्थिर gzFile gz_open(स्थिरांक अक्षर *path, पूर्णांक fd, स्थिरांक अक्षर *mode) {
    gzFile gz;
    पूर्णांक ret;

    gz = दो_स्मृति(आकार(संरचना gzFile_s));
    जर (gz == रिक्त)
        परत रिक्त;
    gz->लिख = म_अक्षर(mode, 'w') != रिक्त;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    जर (gz->लिख)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    अन्यथा {
        gz->strm.next_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    जर (ret != Z_OK) {
        मुक्त(gz);
        परत रिक्त;
    }
    gz->file = path == रिक्त ? fdopen(fd, gz->लिख ? "wb" : "rb") :
                              ख_खोलो(path, gz->लिख ? "wb" : "rb");
    जर (gz->file == रिक्त) {
        gz->लिख ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        मुक्त(gz);
        परत रिक्त;
    }
    gz->err = 0;
    gz->संदेश = "";
    परत gz;
}

स्थिर gzFile gzopen(स्थिरांक अक्षर *path, स्थिरांक अक्षर *mode) {
    परत gz_open(path, -1, mode);
}

स्थिर gzFile gzdopen(पूर्णांक fd, स्थिरांक अक्षर *mode) {
    परत gz_open(रिक्त, fd, mode);
}

स्थिर पूर्णांक gzwrite(gzFile gz, स्थिरांक शून्य *buf, अचिन्हित len) {
    z_stream *strm;
    अचिन्हित अक्षर out[BUFLEN];

    जर (gz == रिक्त || !gz->लिख)
        परत 0;
    strm = &(gz->strm);
    strm->next_in = (शून्य *)buf;
    strm->avail_in = len;
    करा {
        strm->next_out = out;
        strm->avail_out = BUFLEN;
        (शून्य)deflate(strm, Z_NO_FLUSH);
        ख_डालो(out, 1, BUFLEN - strm->avail_out, gz->file);
    } पर्यंत (strm->avail_out == 0);
    परत (पूर्णांक)len;
}

स्थिर पूर्णांक gzread(gzFile gz, शून्य *buf, अचिन्हित len) {
    पूर्णांक ret;
    अचिन्हित got;
    अचिन्हित अक्षर in[1];
    z_stream *strm;

    जर (gz == रिक्त || gz->लिख)
        परत 0;
    जर (gz->err)
        परत 0;
    strm = &(gz->strm);
    strm->next_out = (शून्य *)buf;
    strm->avail_out = len;
    करा {
        got = (अचिन्हित)ख_पढ़ो(in, 1, 1, gz->file);
        जर (got == 0)
            थांबा;
        strm->next_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        जर (ret == Z_DATA_ERROR) {
            gz->err = Z_DATA_ERROR;
            gz->संदेश = strm->संदेश;
            परत 0;
        }
        जर (ret == Z_STREAM_END)
            inflateReset(strm);
    } पर्यंत (strm->avail_out);
    परत (पूर्णांक)(len - strm->avail_out);
}

स्थिर पूर्णांक gzclose(gzFile gz) {
    z_stream *strm;
    अचिन्हित अक्षर out[BUFLEN];

    जर (gz == रिक्त)
        परत Z_STREAM_ERROR;
    strm = &(gz->strm);
    जर (gz->लिख) {
        strm->next_in = Z_NULL;
        strm->avail_in = 0;
        करा {
            strm->next_out = out;
            strm->avail_out = BUFLEN;
            (शून्य)deflate(strm, Z_FINISH);
            ख_डालो(out, 1, BUFLEN - strm->avail_out, gz->file);
        } पर्यंत (strm->avail_out == 0);
        deflateEnd(strm);
    }
    अन्यथा
        inflateEnd(strm);
    ख_बंद(gz->file);
    मुक्त(gz);
    परत Z_OK;
}

स्थिर स्थिरांक अक्षर *gzerror(gzFile gz, पूर्णांक *err) {
    *err = gz->err;
    परत gz->संदेश;
}

#endif

स्थिर अक्षर *prog;

/* ===========================================================================
 * Display error message and exit
 */
स्थिर शून्य error(स्थिरांक अक्षर *संदेश) {
    ख_लिखो(मानक_त्रुटि, "%s: %s\n", prog, संदेश);
    निकास(1);
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
स्थिर पूर्णांक gz_compress_mmap(खाता *in, gzFile out) {
    पूर्णांक len;
    पूर्णांक err;
    पूर्णांक ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    संरचना stat sb;

    /* Determine the size of the file, needed for mmap: */
    जर (ख_स्थिति(ifd, &sb) < 0) परत Z_ERRNO;
    buf_len = sb.st_size;
    जर (buf_len <= 0) परत Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    जर (buf == (caddr_t)(-1)) परत Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (अक्षर *)buf, (अचिन्हित)buf_len);

    जर (len != (पूर्णांक)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    ख_बंद(in);
    जर (gzclose(out) != Z_OK) error("failed gzclose");
    परत Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

स्थिर शून्य gz_compress(खाता *in, gzFile out) {
    local अक्षर buf[BUFLEN];
    पूर्णांक len;
    पूर्णांक err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    जर (gz_compress_mmap(in, out) == Z_OK) परत;
#endif
    साठी (;;) {
        len = (पूर्णांक)ख_पढ़ो(buf, 1, आकार(buf), in);
        जर (ख_त्रुटि(in)) {
            लिखो_त्रुटि("fread");
            निकास(1);
        }
        जर (len == 0) थांबा;

        जर (gzwrite(out, buf, (अचिन्हित)len) != len) error(gzerror(out, &err));
    }
    ख_बंद(in);
    जर (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
स्थिर शून्य gz_uncompress(gzFile in, खाता *out) {
    local अक्षर buf[BUFLEN];
    पूर्णांक len;
    पूर्णांक err;

    साठी (;;) {
        len = gzread(in, buf, आकार(buf));
        जर (len < 0) error (gzerror(in, &err));
        जर (len == 0) थांबा;

        जर ((पूर्णांक)ख_डालो(buf, 1, (अचिन्हित)len, out) != len) {
            error("failed fwrite");
        }
    }
    जर (ख_बंद(out)) error("failed fclose");

    जर (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
स्थिर शून्य file_compress(अक्षर *file, अक्षर *mode) {
    local अक्षर outfile[MAX_NAME_LEN+1], *end;
    खाता  *in;
    gzFile out;

    जर (म_माप(file) + म_माप(GZ_SUFFIX) >= आकार(outfile)) {
        ख_लिखो(मानक_त्रुटि, "%s: filename too long\n", prog);
        निकास(1);
    }

    end = string_copy(outfile, file, आकार(outfile));
    string_copy(end, GZ_SUFFIX, आकार(outfile) - (z_size_t)(end - outfile));

    in = ख_खोलो(file, "rb");
    जर (in == रिक्त) {
        लिखो_त्रुटि(file);
        निकास(1);
    }
    out = gzopen(outfile, mode);
    जर (out == रिक्त) {
        ख_बंद(in);
        ख_लिखो(मानक_त्रुटि, "%s: can't gzopen %s\n", prog, outfile);
        निकास(1);
    }
    gz_compress(in, out);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
स्थिर शून्य file_uncompress(अक्षर *file) {
    local अक्षर buf[MAX_NAME_LEN+1];
    अक्षर *infile, *outfile;
    खाता  *out;
    gzFile in;
    z_size_t len = म_माप(file);

    जर (len + म_माप(GZ_SUFFIX) >= आकार(buf)) {
        ख_लिखो(मानक_त्रुटि, "%s: filename too long\n", prog);
        निकास(1);
    }

    string_copy(buf, file, आकार(buf));

    जर (len > SUFFIX_LEN && म_भेद(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } अन्यथा {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, आकार(buf) - len);
    }
    in = gzopen(infile, "rb");
    जर (in == रिक्त) {
        ख_लिखो(मानक_त्रुटि, "%s: can't gzopen %s\n", prog, infile);
        निकास(1);
    }
    out = ख_खोलो(outfile, "wb");
    जर (out == रिक्त) {
        gzclose(in);
        लिखो_त्रुटि(file);
        निकास(1);
    }

    gz_uncompress(in, out);

    unlink(infile);
}


/* ===========================================================================
 * Usage:  minigzip [-c] [-d] [-f] [-h] [-r] [-1 to -9] [files...]
 *   -c : write to standard output
 *   -d : decompress
 *   -f : compress with Z_FILTERED
 *   -h : compress with Z_HUFFMAN_ONLY
 *   -r : compress with Z_RLE
 *   -1 to -9 : compression level
 */

पूर्णांक main(पूर्णांक argc, अक्षर *argv[]) {
    पूर्णांक copyout = 0;
    पूर्णांक uncompr = 0;
    gzFile file;
    अक्षर *bname, outmode[5];

    string_copy(outmode, "wb6 ", आकार(outmode));
    prog = argv[0];
    bname = म_खोजप(argv[0], '/');
    जर (bname)
      bname++;
    अन्यथा
      bname = argv[0];
    argc--, argv++;

    जर (!म_भेद(bname, "gunzip"))
      uncompr = 1;
    अन्यथा जर (!म_भेद(bname, "zcat"))
      copyout = uncompr = 1;

    पर्यंत (argc > 0) {
      जर (म_भेद(*argv, "-c") == 0)
        copyout = 1;
      अन्यथा जर (म_भेद(*argv, "-d") == 0)
        uncompr = 1;
      अन्यथा जर (म_भेद(*argv, "-f") == 0)
        outmode[3] = 'f';
      अन्यथा जर (म_भेद(*argv, "-h") == 0)
        outmode[3] = 'h';
      अन्यथा जर (म_भेद(*argv, "-r") == 0)
        outmode[3] = 'R';
      अन्यथा जर ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      अन्यथा
        थांबा;
      argc--, argv++;
    }
    जर (outmode[3] == ' ')
        outmode[3] = 0;
    जर (argc == 0) {
        SET_द्विमान_MODE(मानक_निवेश);
        SET_द्विमान_MODE(मानक_निकास);
        जर (uncompr) {
            file = gzdopen(fileno(मानक_निवेश), "rb");
            जर (file == रिक्त) error("can't gzdopen stdin");
            gz_uncompress(file, मानक_निकास);
        } अन्यथा {
            file = gzdopen(fileno(मानक_निकास), outmode);
            जर (file == रिक्त) error("can't gzdopen stdout");
            gz_compress(मानक_निवेश, file);
        }
    } अन्यथा {
        जर (copyout) {
            SET_द्विमान_MODE(मानक_निकास);
        }
        करा {
            जर (uncompr) {
                जर (copyout) {
                    file = gzopen(*argv, "rb");
                    जर (file == रिक्त)
                        ख_लिखो(मानक_त्रुटि, "%s: can't gzopen %s\n", prog, *argv);
                    अन्यथा
                        gz_uncompress(file, मानक_निकास);
                } अन्यथा {
                    file_uncompress(*argv);
                }
            } अन्यथा {
                जर (copyout) {
                    खाता * in = ख_खोलो(*argv, "rb");

                    जर (in == रिक्त) {
                        लिखो_त्रुटि(*argv);
                    } अन्यथा {
                        file = gzdopen(fileno(मानक_निकास), outmode);
                        जर (file == रिक्त) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } अन्यथा {
                    file_compress(*argv, outmode);
                }
            }
        } पर्यंत (argv++, --argc);
    }
    परत 0;
}
