/*
 *
 * Copyright (c) 1997
 * Christian Michelsen Research AS
 * Advanced Computing
 * Fantoftvegen 38, 5036 BERGEN, Norway
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Christian Michelsen Research AS makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#अगर_अघोषित ZSTREAM__H
#घोषणा ZSTREAM__H

/*
 * zstream.h - C++ interface to the 'zlib' general purpose compression library
 * $Id: zstream.h 1.1 1997-06-25 12:00:56+02 tyge Exp tyge $
 */

#समावेश <strstream.h>
#समावेश <string.h>
#समावेश <stdio.h>
#समावेश "zlib.h"

#अगर defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_द्विआधारी_MODE(file) setmode(fileno(file), O_द्विआधारी)
#अन्यथा
#   define SET_द्विआधारी_MODE(file)
#पूर्ण_अगर

class zstringlen {
खुला:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, स्थिर अक्षर*);
    माप_प्रकार मूल्य() स्थिर { वापस val.word; }
निजी:
    काष्ठा Val { अचिन्हित अक्षर byte; माप_प्रकार word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    खुला:
        izstream() : m_fp(0) {}
        izstream(खाता* fp) : m_fp(0) { खोलो(fp); }
        izstream(स्थिर अक्षर* name) : m_fp(0) { खोलो(name); }
        ~izstream() { बंद(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        व्योम खोलो(स्थिर अक्षर* name) {
            अगर (m_fp) बंद();
            m_fp = ::gzopen(name, "rb");
        }

        व्योम खोलो(खाता* fp) {
            SET_द्विआधारी_MODE(fp);
            अगर (m_fp) बंद();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        पूर्णांक बंद() {
            पूर्णांक r = ::gzclose(m_fp);
            m_fp = 0; वापस r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        पूर्णांक पढ़ो(व्योम* buf, माप_प्रकार len) {
            वापस ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        स्थिर अक्षर* error(पूर्णांक* errnum) {
            वापस ::gzerror(m_fp, errnum);
        }

        gzFile fp() { वापस m_fp; }

    निजी:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
ढाँचा <class T, class वस्तुएँ>
अंतरभूत पूर्णांक पढ़ो(izstream& zs, T* x, वस्तुएँ वस्तुएँ_2) {
    वापस ::gzread(zs.fp(), x, वस्तुएँ_2*माप(T));
}

/*
 * Binary input with the '>' operator.
 */
ढाँचा <class T>
अंतरभूत izstream& चालक>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, माप(T));
    वापस zs;
}


अंतरभूत zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    अगर (val.byte == 255) zs > val.word;
    अन्यथा val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
अंतरभूत izstream& चालक>(izstream& zs, अक्षर* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.मूल्य());
    x[len.मूल्य()] = '\0';
    वापस zs;
}

अंतरभूत अक्षर* read_string(izstream& zs) {
    zstringlen len(zs);
    अक्षर* x = new अक्षर[len.मूल्य()+1];
    ::gzread(zs.fp(), x, len.मूल्य());
    x[len.मूल्य()] = '\0';
    वापस x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    खुला:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(खाता* fp, पूर्णांक level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            खोलो(fp, level);
        }
        ozstream(स्थिर अक्षर* name, पूर्णांक level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            खोलो(name, level);
        }
        ~ozstream() {
            बंद();
        }

        /* Opens a gzip (.gz) file for writing.
         * The compression level parameter should be in 0..9
         * errno can be checked to distinguish two error cases
         * (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        व्योम खोलो(स्थिर अक्षर* name, पूर्णांक level = Z_DEFAULT_COMPRESSION) {
            अक्षर mode[4] = "wb\0";
            अगर (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            अगर (m_fp) बंद();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        व्योम खोलो(खाता* fp, पूर्णांक level = Z_DEFAULT_COMPRESSION) {
            SET_द्विआधारी_MODE(fp);
            अक्षर mode[4] = "wb\0";
            अगर (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            अगर (m_fp) बंद();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        पूर्णांक बंद() {
            अगर (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            पूर्णांक r = ::gzclose(m_fp); m_fp = 0; वापस r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        पूर्णांक ग_लिखो(स्थिर व्योम* buf, माप_प्रकार len) {
            वापस ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        पूर्णांक flush(पूर्णांक _flush) {
            os_flush();
            वापस ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        स्थिर अक्षर* error(पूर्णांक* errnum) {
            वापस ::gzerror(m_fp, errnum);
        }

        gzFile fp() { वापस m_fp; }

        ostream& os() {
            अगर (m_os == 0) m_os = new ostrstream;
            वापस *m_os;
        }

        व्योम os_flush() {
            अगर (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    निजी:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
ढाँचा <class T, class वस्तुएँ>
अंतरभूत पूर्णांक ग_लिखो(ozstream& zs, स्थिर T* x, वस्तुएँ वस्तुएँ_2) {
    वापस ::gzwrite(zs.fp(), (voidp) x, वस्तुएँ_2*माप(T));
}

/*
 * Binary output with the '<' operator.
 */
ढाँचा <class T>
अंतरभूत ozstream& चालक<(ozstream& zs, स्थिर T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, माप(T));
    वापस zs;
}

अंतरभूत zstringlen::zstringlen(ozstream& zs, स्थिर अक्षर* x) {
    val.byte = 255;  val.word = ::म_माप(x);
    अगर (val.word < 255) zs < (val.byte = val.word);
    अन्यथा zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
अंतरभूत ozstream& चालक<(ozstream& zs, स्थिर अक्षर* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.मूल्य());
    वापस zs;
}

#अगर_घोषित _MSC_VER
अंतरभूत ozstream& चालक<(ozstream& zs, अक्षर* स्थिर& x) {
    वापस zs < (स्थिर अक्षर*) x;
}
#पूर्ण_अगर

/*
 * Ascii write with the << operator;
 */
ढाँचा <class T>
अंतरभूत ostream& चालक<<(ozstream& zs, स्थिर T& x) {
    zs.os_flush();
    वापस zs.os() << x;
}

#पूर्ण_अगर
