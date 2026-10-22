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

#अगर_अघोषित _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200112L
#पूर्ण_अगर

#अगर defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#पूर्ण_अगर
#अगर defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#पूर्ण_अगर

#समावेश "zlib.h"
#समावेश <stdio.h>

#अगर_घोषित STDC
#  include <string.h>
#  include <stdlib.h>
#पूर्ण_अगर

#अगर_घोषित USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#पूर्ण_अगर

#अगर defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_द्विआधारी_MODE(file) setmode(fileno(file), O_द्विआधारी)
#अन्यथा
#  define SET_द्विआधारी_MODE(file)
#पूर्ण_अगर

#अगर_घोषित VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#पूर्ण_अगर
#अगर defined(__riscos) && !defined(__TARGET_UNIXLIB__)
#  define GZ_SUFFIX "/gz"
#  ifndef __GNUC__
#    define unlink हटाओ
#    define fileno(file) file->__file
#  endif
#पूर्ण_अगर
#अगर defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#पूर्ण_अगर

#अगर !defined(Z_HAVE_UNISTD_H) && !defined(_LARGEFILE64_SOURCE)
#अगर_अघोषित WIN32 /* unlink already in stdio.h for WIN32 */
  बाह्य पूर्णांक unlink(स्थिर अक्षर *);
#पूर्ण_अगर
#पूर्ण_अगर

#अगर defined(UNDER_CE)
#  include <windows.h>
#  define लिखो_त्रुटि(s) pwinerror(s)

/* Map the Windows error number in ERROR to a locale-dependent error
   message string and return a pointer to it.  Typically, the values
   for ERROR come from GetLastError.

   The string pointed to shall not be modified by the application,
   but may be overwritten by a subsequent call to strwinerror

   The strwinerror function does not change the current setting
   of GetLastError.  */

अटल अक्षर *strwinerror (error)
     DWORD error;
{
    अटल अक्षर buf[1024];

    ब_अक्षर_प्रकार *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_बफर,
        शून्य,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        शून्य);
    अगर (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        अगर (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        अगर (chars > माप (buf) - 1) {
            chars = माप (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);
        LocalFree(msgbuf);
    }
    अन्यथा {
        प्र_लिखो(buf, "unknown win32 error (%lu)", error);
    }

    SetLastError(lasterr);
    वापस buf;
}

अटल व्योम pwinerror (s)
    स्थिर अक्षर *s;
{
    अगर (s && *s)
        ख_लिखो(मानक_त्रुटि, "%s: %s\n", s, strwinerror(GetLastError ()));
    अन्यथा
        ख_लिखो(मानक_त्रुटि, "%s\n", strwinerror(GetLastError ()));
}

#पूर्ण_अगर /* UNDER_CE */

#अगर_अघोषित GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#पूर्ण_अगर
#घोषणा SUFFIX_LEN (माप(GZ_SUFFIX)-1)

#घोषणा BUFLEN      16384
#घोषणा MAX_NAME_LEN 1024

#अगर_घोषित MAXSEG_64K
#  define local अटल
   /* Needed for systems with limitation on stack size. */
#अन्यथा
#  define local
#पूर्ण_अगर

/* ===========================================================================
 * Safe string copy. Copy up to len bytes from src to dst, if src terminates
 * with a null by then. If not, copy len-1 bytes from src, terminating it with
 * a null in dst[len-1], cutting src short. Return a pointer to the terminating
 * null. If len is zero, nothing is written to *dst and NULL is returned.
 */
अटल अक्षर *string_copy(अक्षर *dst, अक्षर स्थिर *src, z_आकार_t len) {
    अगर (len == 0)
        वापस शून्य;
    जबतक (--len) {
        *dst = *src++;
        अगर (*dst == 0)
            वापस dst;
        dst++;
    }
    *dst = 0;
    वापस dst;
}

#अगर_घोषित Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#अगर defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#पूर्ण_अगर

अटल व्योम *myalloc(व्योम *q, अचिन्हित n, अचिन्हित m) {
    (व्योम)q;
    वापस सुस्मृति(n, m);
}

अटल व्योम myfree(व्योम *q, व्योम *p) {
    (व्योम)q;
    मुक्त(p);
}

प्रकार काष्ठा gzFile_s {
    खाता *file;
    पूर्णांक ग_लिखो;
    पूर्णांक err;
    अक्षर *संदेश;
    z_stream strm;
} *gzFile;

अटल gzFile gz_open(स्थिर अक्षर *path, पूर्णांक fd, स्थिर अक्षर *mode) {
    gzFile gz;
    पूर्णांक ret;

    gz = दो_स्मृति(माप(काष्ठा gzFile_s));
    अगर (gz == शून्य)
        वापस शून्य;
    gz->ग_लिखो = म_अक्षर(mode, 'w') != शून्य;
    gz->strm.zalloc = myalloc;
    gz->strm.zfree = myfree;
    gz->strm.opaque = Z_NULL;
    अगर (gz->ग_लिखो)
        ret = deflateInit2(&(gz->strm), -1, 8, 15 + 16, 8, 0);
    अन्यथा {
        gz->strm.अग्र_in = 0;
        gz->strm.avail_in = Z_NULL;
        ret = inflateInit2(&(gz->strm), 15 + 16);
    }
    अगर (ret != Z_OK) {
        मुक्त(gz);
        वापस शून्य;
    }
    gz->file = path == शून्य ? fdopen(fd, gz->ग_लिखो ? "wb" : "rb") :
                              ख_खोलो(path, gz->ग_लिखो ? "wb" : "rb");
    अगर (gz->file == शून्य) {
        gz->ग_लिखो ? deflateEnd(&(gz->strm)) : inflateEnd(&(gz->strm));
        मुक्त(gz);
        वापस शून्य;
    }
    gz->err = 0;
    gz->संदेश = "";
    वापस gz;
}

अटल gzFile gzopen(स्थिर अक्षर *path, स्थिर अक्षर *mode) {
    वापस gz_open(path, -1, mode);
}

अटल gzFile gzdopen(पूर्णांक fd, स्थिर अक्षर *mode) {
    वापस gz_open(शून्य, fd, mode);
}

अटल पूर्णांक gzwrite(gzFile gz, स्थिर व्योम *buf, अचिन्हित len) {
    z_stream *strm;
    अचिन्हित अक्षर out[BUFLEN];

    अगर (gz == शून्य || !gz->ग_लिखो)
        वापस 0;
    strm = &(gz->strm);
    strm->अग्र_in = (व्योम *)buf;
    strm->avail_in = len;
    करो {
        strm->अग्र_out = out;
        strm->avail_out = BUFLEN;
        (व्योम)deflate(strm, Z_NO_FLUSH);
        ख_डालो(out, 1, BUFLEN - strm->avail_out, gz->file);
    } जबतक (strm->avail_out == 0);
    वापस (पूर्णांक)len;
}

अटल पूर्णांक gzread(gzFile gz, व्योम *buf, अचिन्हित len) {
    पूर्णांक ret;
    अचिन्हित got;
    अचिन्हित अक्षर in[1];
    z_stream *strm;

    अगर (gz == शून्य || gz->ग_लिखो)
        वापस 0;
    अगर (gz->err)
        वापस 0;
    strm = &(gz->strm);
    strm->अग्र_out = (व्योम *)buf;
    strm->avail_out = len;
    करो {
        got = (अचिन्हित)ख_पढ़ो(in, 1, 1, gz->file);
        अगर (got == 0)
            अवरोध;
        strm->अग्र_in = in;
        strm->avail_in = 1;
        ret = inflate(strm, Z_NO_FLUSH);
        अगर (ret == Z_तथ्य_ERROR) {
            gz->err = Z_तथ्य_ERROR;
            gz->संदेश = strm->संदेश;
            वापस 0;
        }
        अगर (ret == Z_STREAM_END)
            inflateReset(strm);
    } जबतक (strm->avail_out);
    वापस (पूर्णांक)(len - strm->avail_out);
}

अटल पूर्णांक gzclose(gzFile gz) {
    z_stream *strm;
    अचिन्हित अक्षर out[BUFLEN];

    अगर (gz == शून्य)
        वापस Z_STREAM_ERROR;
    strm = &(gz->strm);
    अगर (gz->ग_लिखो) {
        strm->अग्र_in = Z_NULL;
        strm->avail_in = 0;
        करो {
            strm->अग्र_out = out;
            strm->avail_out = BUFLEN;
            (व्योम)deflate(strm, Z_FINISH);
            ख_डालो(out, 1, BUFLEN - strm->avail_out, gz->file);
        } जबतक (strm->avail_out == 0);
        deflateEnd(strm);
    }
    अन्यथा
        inflateEnd(strm);
    ख_बंद(gz->file);
    मुक्त(gz);
    वापस Z_OK;
}

अटल स्थिर अक्षर *gzerror(gzFile gz, पूर्णांक *err) {
    *err = gz->err;
    वापस gz->संदेश;
}

#पूर्ण_अगर

अटल अक्षर *prog;

/* ===========================================================================
 * Display error message and exit
 */
अटल व्योम error(स्थिर अक्षर *संदेश) {
    ख_लिखो(मानक_त्रुटि, "%s: %s\n", prog, संदेश);
    निकास(1);
}

#अगर_घोषित USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * success, Z_ERRNO otherwise.
 */
अटल पूर्णांक gz_compress_mmap(खाता *in, gzFile out) {
    पूर्णांक len;
    पूर्णांक err;
    पूर्णांक ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    काष्ठा stat sb;

    /* Determine the size of the file, needed for mmap: */
    अगर (ख_स्थिति(ifd, &sb) < 0) वापस Z_ERRNO;
    buf_len = sb.st_size;
    अगर (buf_len <= 0) वापस Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    अगर (buf == (caddr_t)(-1)) वापस Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (अक्षर *)buf, (अचिन्हित)buf_len);

    अगर (len != (पूर्णांक)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    ख_बंद(in);
    अगर (gzclose(out) != Z_OK) error("failed gzclose");
    वापस Z_OK;
}
#पूर्ण_अगर /* USE_MMAP */

/* ===========================================================================
 * Compress input to output then close both files.
 */

अटल व्योम gz_compress(खाता *in, gzFile out) {
    local अक्षर buf[BUFLEN];
    पूर्णांक len;
    पूर्णांक err;

#अगर_घोषित USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    अगर (gz_compress_mmap(in, out) == Z_OK) वापस;
#पूर्ण_अगर
    क्रम (;;) {
        len = (पूर्णांक)ख_पढ़ो(buf, 1, माप(buf), in);
        अगर (ख_त्रुटि(in)) {
            लिखो_त्रुटि("fread");
            निकास(1);
        }
        अगर (len == 0) अवरोध;

        अगर (gzwrite(out, buf, (अचिन्हित)len) != len) error(gzerror(out, &err));
    }
    ख_बंद(in);
    अगर (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
अटल व्योम gz_uncompress(gzFile in, खाता *out) {
    local अक्षर buf[BUFLEN];
    पूर्णांक len;
    पूर्णांक err;

    क्रम (;;) {
        len = gzread(in, buf, माप(buf));
        अगर (len < 0) error (gzerror(in, &err));
        अगर (len == 0) अवरोध;

        अगर ((पूर्णांक)ख_डालो(buf, 1, (अचिन्हित)len, out) != len) {
            error("failed fwrite");
        }
    }
    अगर (ख_बंद(out)) error("failed fclose");

    अगर (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
अटल व्योम file_compress(अक्षर *file, अक्षर *mode) {
    local अक्षर outfile[MAX_NAME_LEN+1], *end;
    खाता  *in;
    gzFile out;

    अगर (म_माप(file) + म_माप(GZ_SUFFIX) >= माप(outfile)) {
        ख_लिखो(मानक_त्रुटि, "%s: filename too long\n", prog);
        निकास(1);
    }

    end = string_copy(outfile, file, माप(outfile));
    string_copy(end, GZ_SUFFIX, माप(outfile) - (z_आकार_t)(end - outfile));

    in = ख_खोलो(file, "rb");
    अगर (in == शून्य) {
        लिखो_त्रुटि(file);
        निकास(1);
    }
    out = gzopen(outfile, mode);
    अगर (out == शून्य) {
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
अटल व्योम file_uncompress(अक्षर *file) {
    local अक्षर buf[MAX_NAME_LEN+1];
    अक्षर *infile, *outfile;
    खाता  *out;
    gzFile in;
    z_आकार_t len = म_माप(file);

    अगर (len + म_माप(GZ_SUFFIX) >= माप(buf)) {
        ख_लिखो(मानक_त्रुटि, "%s: filename too long\n", prog);
        निकास(1);
    }

    string_copy(buf, file, माप(buf));

    अगर (len > SUFFIX_LEN && म_भेद(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } अन्यथा {
        outfile = file;
        infile = buf;
        string_copy(buf + len, GZ_SUFFIX, माप(buf) - len);
    }
    in = gzopen(infile, "rb");
    अगर (in == शून्य) {
        ख_लिखो(मानक_त्रुटि, "%s: can't gzopen %s\n", prog, infile);
        निकास(1);
    }
    out = ख_खोलो(outfile, "wb");
    अगर (out == शून्य) {
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

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[]) {
    पूर्णांक copyout = 0;
    पूर्णांक uncompr = 0;
    gzFile file;
    अक्षर *bname, outmode[5];

    string_copy(outmode, "wb6 ", माप(outmode));
    prog = argv[0];
    bname = म_खोजप(argv[0], '/');
    अगर (bname)
      bname++;
    अन्यथा
      bname = argv[0];
    argc--, argv++;

    अगर (!म_भेद(bname, "gunzip"))
      uncompr = 1;
    अन्यथा अगर (!म_भेद(bname, "zcat"))
      copyout = uncompr = 1;

    जबतक (argc > 0) {
      अगर (म_भेद(*argv, "-c") == 0)
        copyout = 1;
      अन्यथा अगर (म_भेद(*argv, "-d") == 0)
        uncompr = 1;
      अन्यथा अगर (म_भेद(*argv, "-f") == 0)
        outmode[3] = 'f';
      अन्यथा अगर (म_भेद(*argv, "-h") == 0)
        outmode[3] = 'h';
      अन्यथा अगर (म_भेद(*argv, "-r") == 0)
        outmode[3] = 'R';
      अन्यथा अगर ((*argv)[0] == '-' && (*argv)[1] >= '1' && (*argv)[1] <= '9' &&
               (*argv)[2] == 0)
        outmode[2] = (*argv)[1];
      अन्यथा
        अवरोध;
      argc--, argv++;
    }
    अगर (outmode[3] == ' ')
        outmode[3] = 0;
    अगर (argc == 0) {
        SET_द्विआधारी_MODE(मानक_निवेश);
        SET_द्विआधारी_MODE(मानक_निकास);
        अगर (uncompr) {
            file = gzdopen(fileno(मानक_निवेश), "rb");
            अगर (file == शून्य) error("can't gzdopen stdin");
            gz_uncompress(file, मानक_निकास);
        } अन्यथा {
            file = gzdopen(fileno(मानक_निकास), outmode);
            अगर (file == शून्य) error("can't gzdopen stdout");
            gz_compress(मानक_निवेश, file);
        }
    } अन्यथा {
        अगर (copyout) {
            SET_द्विआधारी_MODE(मानक_निकास);
        }
        करो {
            अगर (uncompr) {
                अगर (copyout) {
                    file = gzopen(*argv, "rb");
                    अगर (file == शून्य)
                        ख_लिखो(मानक_त्रुटि, "%s: can't gzopen %s\n", prog, *argv);
                    अन्यथा
                        gz_uncompress(file, मानक_निकास);
                } अन्यथा {
                    file_uncompress(*argv);
                }
            } अन्यथा {
                अगर (copyout) {
                    खाता * in = ख_खोलो(*argv, "rb");

                    अगर (in == शून्य) {
                        लिखो_त्रुटि(*argv);
                    } अन्यथा {
                        file = gzdopen(fileno(मानक_निकास), outmode);
                        अगर (file == शून्य) error("can't gzdopen stdout");

                        gz_compress(in, file);
                    }

                } अन्यथा {
                    file_compress(*argv, outmode);
                }
            }
        } जबतक (argv++, --argc);
    }
    वापस 0;
}
