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

#شامل <strstream.h>
#شامل <string.h>
#شامل <stdio.h>
#شامل "zlib.h"

#اگر defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_دودویی_MODE(file) setmode(fileno(file), O_دودویی)
#وگرنه
#   define SET_دودویی_MODE(file)
#endif

class zstringlen {
عمومی:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ثابت نویسه*);
    size_t مقدار() ثابت { بازگشت val.word; }
خصوصی:
    ساختار Val { unsigned نویسه byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    عمومی:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ثابت نویسه* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        تهی open(ثابت نویسه* name) {
            اگر (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        تهی open(FILE* fp) {
            SET_دودویی_MODE(fp);
            اگر (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        صحیح close() {
            صحیح r = ::gzclose(m_fp);
            m_fp = 0; بازگشت r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        صحیح اقرأ(تهی* buf, size_t len) {
            بازگشت ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ثابت نویسه* error(صحیح* errnum) {
            بازگشت ::gzerror(m_fp, errnum);
        }

        gzFile fp() { بازگشت m_fp; }

    خصوصی:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class اقلام>
درونخطی صحیح اقرأ(izstream& zs, T* x, اقلام اقلام_2) {
    بازگشت ::gzread(zs.fp(), x, اقلام_2*اندازه(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
درونخطی izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, اندازه(T));
    بازگشت zs;
}


درونخطی zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    اگر (val.byte == 255) zs > val.word;
    وگرنه val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
درونخطی izstream& operator>(izstream& zs, نویسه* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.مقدار());
    x[len.مقدار()] = '\0';
    بازگشت zs;
}

درونخطی نویسه* read_string(izstream& zs) {
    zstringlen len(zs);
    نویسه* x = new نویسه[len.مقدار()+1];
    ::gzread(zs.fp(), x, len.مقدار());
    x[len.مقدار()] = '\0';
    بازگشت x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    عمومی:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, صحیح level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ثابت نویسه* name, صحیح level = Z_DEFAULT_COMPRESSION)
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
        تهی open(ثابت نویسه* name, صحیح level = Z_DEFAULT_COMPRESSION) {
            نویسه mode[4] = "wb\0";
            اگر (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            اگر (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        تهی open(FILE* fp, صحیح level = Z_DEFAULT_COMPRESSION) {
            SET_دودویی_MODE(fp);
            نویسه mode[4] = "wb\0";
            اگر (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            اگر (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
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
            صحیح r = ::gzclose(m_fp); m_fp = 0; بازگشت r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        صحیح اكتب(ثابت تهی* buf, size_t len) {
            بازگشت ::gzwrite(m_fp, (voidp) buf, len);
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
            بازگشت ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ثابت نویسه* error(صحیح* errnum) {
            بازگشت ::gzerror(m_fp, errnum);
        }

        gzFile fp() { بازگشت m_fp; }

        ostream& os() {
            اگر (m_os == 0) m_os = new ostrstream;
            بازگشت *m_os;
        }

        تهی os_flush() {
            اگر (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    خصوصی:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class اقلام>
درونخطی صحیح اكتب(ozstream& zs, ثابت T* x, اقلام اقلام_2) {
    بازگشت ::gzwrite(zs.fp(), (voidp) x, اقلام_2*اندازه(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
درونخطی ozstream& operator<(ozstream& zs, ثابت T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, اندازه(T));
    بازگشت zs;
}

درونخطی zstringlen::zstringlen(ozstream& zs, ثابت نویسه* x) {
    val.byte = 255;  val.word = ::strlen(x);
    اگر (val.word < 255) zs < (val.byte = val.word);
    وگرنه zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
درونخطی ozstream& operator<(ozstream& zs, ثابت نویسه* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.مقدار());
    بازگشت zs;
}

#ifdef _MSC_VER
درونخطی ozstream& operator<(ozstream& zs, نویسه* ثابت& x) {
    بازگشت zs < (ثابت نویسه*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
درونخطی ostream& operator<<(ozstream& zs, ثابت T& x) {
    zs.os_flush();
    بازگشت zs.os() << x;
}

#endif
