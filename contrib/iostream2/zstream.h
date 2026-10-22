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

#اگر_غیر_تعریف_شدہ ZSTREAM__H
#تعریف_کرو ZSTREAM__H

/*
 * zstream.h - C++ interface to the 'zlib' general purpose compression library
 * $Id: zstream.h 1.1 1997-06-25 12:00:56+02 tyge Exp tyge $
 */

#شامل <strstream.h>
#شامل <string.h>
#شامل <stdio.h>
#شامل "zlib.h"

#اگر defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define سیٹ_ثنائی_منوال(file) setmode(fileno(file), O_ثنائی)
#ورنہ
#   define سیٹ_ثنائی_منوال(file)
#اگر_ختم

class zstringlen {
عوامی:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ثابت حرف*);
    size_t قدر() ثابت { واپس val.لفظ; }
نجی:
    ساخت Val { غیرعلامتی حرف byte; size_t لفظ; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    عوامی:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ثابت حرف* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        خالی open(ثابت حرف* name) {
            اگر (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        خالی open(FILE* fp) {
            سیٹ_ثنائی_منوال(fp);
            اگر (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        صحیح close() {
            صحیح r = ::gzclose(m_fp);
            m_fp = 0; واپس r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        صحیح اقرأ(خالی* buf, size_t len) {
            واپس ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ثابت حرف* error(صحیح* errnum) {
            واپس ::gzerror(m_fp, errnum);
        }

        gzFile fp() { واپس m_fp; }

    نجی:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
سانچہ <class T, class اشیاء>
اندرونی صحیح اقرأ(izstream& zs, T* x, اشیاء اشیاء_2) {
    واپس ::gzread(zs.fp(), x, اشیاء_2*حجم(T));
}

/*
 * Binary input with the '>' operator.
 */
سانچہ <class T>
اندرونی izstream& عامل>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, حجم(T));
    واپس zs;
}


اندرونی zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    اگر (val.byte == 255) zs > val.لفظ;
    ورنہ val.لفظ = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
اندرونی izstream& عامل>(izstream& zs, حرف* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.قدر());
    x[len.قدر()] = '\0';
    واپس zs;
}

اندرونی حرف* read_string(izstream& zs) {
    zstringlen len(zs);
    حرف* x = new حرف[len.قدر()+1];
    ::gzread(zs.fp(), x, len.قدر());
    x[len.قدر()] = '\0';
    واپس x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    عوامی:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, صحیح level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ثابت حرف* name, صحیح level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(name, level);
        }
        ~ozstream() {
            close();
        }

        /* Opens a gzip (.gz) file for writing.
         * The compression level parameter should be in 0..9
         * errno can be checked to distinguish two error cases
         * (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        خالی open(ثابت حرف* name, صحیح level = Z_DEFAULT_COMPRESSION) {
            حرف منوال[4] = "wb\0";
            اگر (level != Z_DEFAULT_COMPRESSION) منوال[2] = '0'+level;
            اگر (m_fp) close();
            m_fp = ::gzopen(name, منوال);
        }

        /* open from a FILE pointer.
         */
        خالی open(FILE* fp, صحیح level = Z_DEFAULT_COMPRESSION) {
            سیٹ_ثنائی_منوال(fp);
            حرف منوال[4] = "wb\0";
            اگر (level != Z_DEFAULT_COMPRESSION) منوال[2] = '0'+level;
            اگر (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), منوال);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        صحیح close() {
            اگر (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            صحیح r = ::gzclose(m_fp); m_fp = 0; واپس r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        صحیح اكتب(ثابت خالی* buf, size_t len) {
            واپس ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        صحیح flush(صحیح _flush) {
            os_flush();
            واپس ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ثابت حرف* error(صحیح* errnum) {
            واپس ::gzerror(m_fp, errnum);
        }

        gzFile fp() { واپس m_fp; }

        ostream& os() {
            اگر (m_os == 0) m_os = new ostrstream;
            واپس *m_os;
        }

        خالی os_flush() {
            اگر (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->چوڑائی(m_os->چوڑائی());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    نجی:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
سانچہ <class T, class اشیاء>
اندرونی صحیح اكتب(ozstream& zs, ثابت T* x, اشیاء اشیاء_2) {
    واپس ::gzwrite(zs.fp(), (voidp) x, اشیاء_2*حجم(T));
}

/*
 * Binary output with the '<' operator.
 */
سانچہ <class T>
اندرونی ozstream& عامل<(ozstream& zs, ثابت T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, حجم(T));
    واپس zs;
}

اندرونی zstringlen::zstringlen(ozstream& zs, ثابت حرف* x) {
    val.byte = 255;  val.لفظ = ::لڑی_لمبائی(x);
    اگر (val.لفظ < 255) zs < (val.byte = val.لفظ);
    ورنہ zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
اندرونی ozstream& عامل<(ozstream& zs, ثابت حرف* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.قدر());
    واپس zs;
}

#اگر_تعریف_شدہ _MSC_VER
اندرونی ozstream& عامل<(ozstream& zs, حرف* ثابت& x) {
    واپس zs < (ثابت حرف*) x;
}
#اگر_ختم

/*
 * Ascii write with the << operator;
 */
سانچہ <class T>
اندرونی ostream& عامل<<(ozstream& zs, ثابت T& x) {
    zs.os_flush();
    واپس zs.os() << x;
}

#اگر_ختم
