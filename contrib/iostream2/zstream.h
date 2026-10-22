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

#ifndef ZSTREAM__H
#define ZSTREAM__H

/*
 * zstream.h - C++ interface to the 'zlib' general purpose compression library
 * $Id: zstream.h 1.1 1997-06-25 12:00:56+02 tyge Exp tyge $
 */

#अन्तर्भाव <strstream.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव "zlib.h"

#जर defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_द्विमान_MODE(file) setmode(fileno(file), O_द्विमान)
#अन्यथा
#   define SET_द्विमान_MODE(file)
#endif

class zstringlen {
सार्वजनिक:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, स्थिरांक अक्षर*);
    माप_प्रकार मूल्य() स्थिरांक { परत val.word; }
खाजगी:
    संरचना Val { अचिन्हित अक्षर byte; माप_प्रकार word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    सार्वजनिक:
        izstream() : m_fp(0) {}
        izstream(खाता* fp) : m_fp(0) { खोलो(fp); }
        izstream(स्थिरांक अक्षर* name) : m_fp(0) { खोलो(name); }
        ~izstream() { बंद(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        शून्य खोलो(स्थिरांक अक्षर* name) {
            जर (m_fp) बंद();
            m_fp = ::gzopen(name, "rb");
        }

        शून्य खोलो(खाता* fp) {
            SET_द्विमान_MODE(fp);
            जर (m_fp) बंद();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        पूर्णांक बंद() {
            पूर्णांक r = ::gzclose(m_fp);
            m_fp = 0; परत r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        पूर्णांक पठ(शून्य* buf, माप_प्रकार len) {
            परत ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        स्थिरांक अक्षर* error(पूर्णांक* errnum) {
            परत ::gzerror(m_fp, errnum);
        }

        gzFile fp() { परत m_fp; }

    खाजगी:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
ढाँचा <class T, class वस्तुसंच>
ओळीत पूर्णांक पठ(izstream& zs, T* x, वस्तुसंच वस्तुसंच_2) {
    परत ::gzread(zs.fp(), x, वस्तुसंच_2*आकार(T));
}

/*
 * Binary input with the '>' operator.
 */
ढाँचा <class T>
ओळीत izstream& चालक>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, आकार(T));
    परत zs;
}


ओळीत zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    जर (val.byte == 255) zs > val.word;
    अन्यथा val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
ओळीत izstream& चालक>(izstream& zs, अक्षर* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.मूल्य());
    x[len.मूल्य()] = '\0';
    परत zs;
}

ओळीत अक्षर* read_string(izstream& zs) {
    zstringlen len(zs);
    अक्षर* x = new अक्षर[len.मूल्य()+1];
    ::gzread(zs.fp(), x, len.मूल्य());
    x[len.मूल्य()] = '\0';
    परत x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    सार्वजनिक:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(खाता* fp, पूर्णांक level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            खोलो(fp, level);
        }
        ozstream(स्थिरांक अक्षर* name, पूर्णांक level = Z_DEFAULT_COMPRESSION)
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
        शून्य खोलो(स्थिरांक अक्षर* name, पूर्णांक level = Z_DEFAULT_COMPRESSION) {
            अक्षर mode[4] = "wb\0";
            जर (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            जर (m_fp) बंद();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        शून्य खोलो(खाता* fp, पूर्णांक level = Z_DEFAULT_COMPRESSION) {
            SET_द्विमान_MODE(fp);
            अक्षर mode[4] = "wb\0";
            जर (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            जर (m_fp) बंद();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        पूर्णांक बंद() {
            जर (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            पूर्णांक r = ::gzclose(m_fp); m_fp = 0; परत r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        पूर्णांक लिख(स्थिरांक शून्य* buf, माप_प्रकार len) {
            परत ::gzwrite(m_fp, (voidp) buf, len);
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
            परत ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        स्थिरांक अक्षर* error(पूर्णांक* errnum) {
            परत ::gzerror(m_fp, errnum);
        }

        gzFile fp() { परत m_fp; }

        ostream& os() {
            जर (m_os == 0) m_os = new ostrstream;
            परत *m_os;
        }

        शून्य os_flush() {
            जर (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    खाजगी:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
ढाँचा <class T, class वस्तुसंच>
ओळीत पूर्णांक लिख(ozstream& zs, स्थिरांक T* x, वस्तुसंच वस्तुसंच_2) {
    परत ::gzwrite(zs.fp(), (voidp) x, वस्तुसंच_2*आकार(T));
}

/*
 * Binary output with the '<' operator.
 */
ढाँचा <class T>
ओळीत ozstream& चालक<(ozstream& zs, स्थिरांक T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, आकार(T));
    परत zs;
}

ओळीत zstringlen::zstringlen(ozstream& zs, स्थिरांक अक्षर* x) {
    val.byte = 255;  val.word = ::म_माप(x);
    जर (val.word < 255) zs < (val.byte = val.word);
    अन्यथा zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
ओळीत ozstream& चालक<(ozstream& zs, स्थिरांक अक्षर* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.मूल्य());
    परत zs;
}

#ifdef _MSC_VER
ओळीत ozstream& चालक<(ozstream& zs, अक्षर* स्थिरांक& x) {
    परत zs < (स्थिरांक अक्षर*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
ढाँचा <class T>
ओळीत ostream& चालक<<(ozstream& zs, स्थिरांक T& x) {
    zs.os_flush();
    परत zs.os() << x;
}

#endif
