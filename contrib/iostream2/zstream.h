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

#অন্তর্ভাব <strstream.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব "zlib.h"

#যদি defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_দ্বিমিক_MODE(file) setmode(fileno(file), O_দ্বিমিক)
#অন্যথা
#   define SET_দ্বিমিক_MODE(file)
#endif

class zstringlen {
সর্বজনীন:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ধ্রুবক অক্ষর*);
    size_t মান() ধ্রুবক { ফেরত val.word; }
ব্যক্তিগত:
    কাঠামো Val { unsigned অক্ষর byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    সর্বজনীন:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ধ্রুবক অক্ষর* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        শূন্য open(ধ্রুবক অক্ষর* name) {
            যদি (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        শূন্য open(FILE* fp) {
            SET_দ্বিমিক_MODE(fp);
            যদি (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        পূর্ণসংখ্যা close() {
            পূর্ণসংখ্যা r = ::gzclose(m_fp);
            m_fp = 0; ফেরত r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        পূর্ণসংখ্যা পঠ(শূন্য* buf, size_t len) {
            ফেরত ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ধ্রুবক অক্ষর* error(পূর্ণসংখ্যা* errnum) {
            ফেরত ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ফেরত m_fp; }

    ব্যক্তিগত:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class বস্তুসমূহ>
অন্তঃরেখ পূর্ণসংখ্যা পঠ(izstream& zs, T* x, বস্তুসমূহ বস্তুসমূহ_2) {
    ফেরত ::gzread(zs.fp(), x, বস্তুসমূহ_2*আকার(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
অন্তঃরেখ izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, আকার(T));
    ফেরত zs;
}


অন্তঃরেখ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    যদি (val.byte == 255) zs > val.word;
    অন্যথা val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
অন্তঃরেখ izstream& operator>(izstream& zs, অক্ষর* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.মান());
    x[len.মান()] = '\0';
    ফেরত zs;
}

অন্তঃরেখ অক্ষর* read_string(izstream& zs) {
    zstringlen len(zs);
    অক্ষর* x = new অক্ষর[len.মান()+1];
    ::gzread(zs.fp(), x, len.মান());
    x[len.মান()] = '\0';
    ফেরত x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    সর্বজনীন:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, পূর্ণসংখ্যা level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ধ্রুবক অক্ষর* name, পূর্ণসংখ্যা level = Z_DEFAULT_COMPRESSION)
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
        শূন্য open(ধ্রুবক অক্ষর* name, পূর্ণসংখ্যা level = Z_DEFAULT_COMPRESSION) {
            অক্ষর mode[4] = "wb\0";
            যদি (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            যদি (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        শূন্য open(FILE* fp, পূর্ণসংখ্যা level = Z_DEFAULT_COMPRESSION) {
            SET_দ্বিমিক_MODE(fp);
            অক্ষর mode[4] = "wb\0";
            যদি (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            যদি (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        পূর্ণসংখ্যা close() {
            যদি (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            পূর্ণসংখ্যা r = ::gzclose(m_fp); m_fp = 0; ফেরত r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        পূর্ণসংখ্যা লিখ(ধ্রুবক শূন্য* buf, size_t len) {
            ফেরত ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        পূর্ণসংখ্যা flush(পূর্ণসংখ্যা _flush) {
            os_flush();
            ফেরত ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ধ্রুবক অক্ষর* error(পূর্ণসংখ্যা* errnum) {
            ফেরত ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ফেরত m_fp; }

        ostream& os() {
            যদি (m_os == 0) m_os = new ostrstream;
            ফেরত *m_os;
        }

        শূন্য os_flush() {
            যদি (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    ব্যক্তিগত:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class বস্তুসমূহ>
অন্তঃরেখ পূর্ণসংখ্যা লিখ(ozstream& zs, ধ্রুবক T* x, বস্তুসমূহ বস্তুসমূহ_2) {
    ফেরত ::gzwrite(zs.fp(), (voidp) x, বস্তুসমূহ_2*আকার(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
অন্তঃরেখ ozstream& operator<(ozstream& zs, ধ্রুবক T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, আকার(T));
    ফেরত zs;
}

অন্তঃরেখ zstringlen::zstringlen(ozstream& zs, ধ্রুবক অক্ষর* x) {
    val.byte = 255;  val.word = ::strlen(x);
    যদি (val.word < 255) zs < (val.byte = val.word);
    অন্যথা zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
অন্তঃরেখ ozstream& operator<(ozstream& zs, ধ্রুবক অক্ষর* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.মান());
    ফেরত zs;
}

#ifdef _MSC_VER
অন্তঃরেখ ozstream& operator<(ozstream& zs, অক্ষর* ধ্রুবক& x) {
    ফেরত zs < (ধ্রুবক অক্ষর*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
অন্তঃরেখ ostream& operator<<(ozstream& zs, ধ্রুবক T& x) {
    zs.os_flush();
    ফেরত zs.os() << x;
}

#endif
