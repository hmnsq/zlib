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

#ಅನ್ತರ್ಭಾವ <strstream.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

#ಯದಿ defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_ದ್ವಿಮಾನ_MODE(file) setmode(fileno(file), O_ದ್ವಿಮಾನ)
#ಅನ್ಯಥಾ
#   define SET_ದ್ವಿಮಾನ_MODE(file)
#endif

class zstringlen {
ಸಾರ್ವಜನಿಕ:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ಧ್ರುವ ವರ್ಣ*);
    size_t ಮೌಲ್ಯ() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ val.word; }
ಗುಪ್ತ:
    ಸಂರಚನಾ Val { unsigned ವರ್ಣ byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    ಸಾರ್ವಜನಿಕ:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ಧ್ರುವ ವರ್ಣ* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        ಶೂನ್ಯ open(ಧ್ರುವ ವರ್ಣ* name) {
            ಯದಿ (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        ಶೂನ್ಯ open(FILE* fp) {
            SET_ದ್ವಿಮಾನ_MODE(fp);
            ಯದಿ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ಪೂರ್ಣಾಙ್ಕ close() {
            ಪೂರ್ಣಾಙ್ಕ r = ::gzclose(m_fp);
            m_fp = 0; ಪ್ರತ್ಯಾಗಮ r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        ಪೂರ್ಣಾಙ್ಕ ಪಠ(ಶೂನ್ಯ* buf, size_t len) {
            ಪ್ರತ್ಯಾಗಮ ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ಧ್ರುವ ವರ್ಣ* error(ಪೂರ್ಣಾಙ್ಕ* errnum) {
            ಪ್ರತ್ಯಾಗಮ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ಪ್ರತ್ಯಾಗಮ m_fp; }

    ಗುಪ್ತ:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class ವಸ್ತುಗಳು>
ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ ಪಠ(izstream& zs, T* x, ವಸ್ತುಗಳು ವಸ್ತುಗಳು_2) {
    ಪ್ರತ್ಯಾಗಮ ::gzread(zs.fp(), x, ವಸ್ತುಗಳು_2*ಪರಿಮಾಣ(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
ಅನ್ತಃಸ್ಥ izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, ಪರಿಮಾಣ(T));
    ಪ್ರತ್ಯಾಗಮ zs;
}


ಅನ್ತಃಸ್ಥ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    ಯದಿ (val.byte == 255) zs > val.word;
    ಅನ್ಯಥಾ val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
ಅನ್ತಃಸ್ಥ izstream& operator>(izstream& zs, ವರ್ಣ* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.ಮೌಲ್ಯ());
    x[len.ಮೌಲ್ಯ()] = '\0';
    ಪ್ರತ್ಯಾಗಮ zs;
}

ಅನ್ತಃಸ್ಥ ವರ್ಣ* read_string(izstream& zs) {
    zstringlen len(zs);
    ವರ್ಣ* x = new ವರ್ಣ[len.ಮೌಲ್ಯ()+1];
    ::gzread(zs.fp(), x, len.ಮೌಲ್ಯ());
    x[len.ಮೌಲ್ಯ()] = '\0';
    ಪ್ರತ್ಯಾಗಮ x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    ಸಾರ್ವಜನಿಕ:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, ಪೂರ್ಣಾಙ್ಕ level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ಧ್ರುವ ವರ್ಣ* name, ಪೂರ್ಣಾಙ್ಕ level = Z_DEFAULT_COMPRESSION)
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
        ಶೂನ್ಯ open(ಧ್ರುವ ವರ್ಣ* name, ಪೂರ್ಣಾಙ್ಕ level = Z_DEFAULT_COMPRESSION) {
            ವರ್ಣ mode[4] = "wb\0";
            ಯದಿ (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            ಯದಿ (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        ಶೂನ್ಯ open(FILE* fp, ಪೂರ್ಣಾಙ್ಕ level = Z_DEFAULT_COMPRESSION) {
            SET_ದ್ವಿಮಾನ_MODE(fp);
            ವರ್ಣ mode[4] = "wb\0";
            ಯದಿ (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            ಯದಿ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ಪೂರ್ಣಾಙ್ಕ close() {
            ಯದಿ (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            ಪೂರ್ಣಾಙ್ಕ r = ::gzclose(m_fp); m_fp = 0; ಪ್ರತ್ಯಾಗಮ r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        ಪೂರ್ಣಾಙ್ಕ ಲಿಖ(ಧ್ರುವ ಶೂನ್ಯ* buf, size_t len) {
            ಪ್ರತ್ಯಾಗಮ ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        ಪೂರ್ಣಾಙ್ಕ flush(ಪೂರ್ಣಾಙ್ಕ _flush) {
            os_flush();
            ಪ್ರತ್ಯಾಗಮ ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ಧ್ರುವ ವರ್ಣ* error(ಪೂರ್ಣಾಙ್ಕ* errnum) {
            ಪ್ರತ್ಯಾಗಮ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ಪ್ರತ್ಯಾಗಮ m_fp; }

        ostream& os() {
            ಯದಿ (m_os == 0) m_os = new ostrstream;
            ಪ್ರತ್ಯಾಗಮ *m_os;
        }

        ಶೂನ್ಯ os_flush() {
            ಯದಿ (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    ಗುಪ್ತ:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class ವಸ್ತುಗಳು>
ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ ಲಿಖ(ozstream& zs, ಧ್ರುವ T* x, ವಸ್ತುಗಳು ವಸ್ತುಗಳು_2) {
    ಪ್ರತ್ಯಾಗಮ ::gzwrite(zs.fp(), (voidp) x, ವಸ್ತುಗಳು_2*ಪರಿಮಾಣ(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
ಅನ್ತಃಸ್ಥ ozstream& operator<(ozstream& zs, ಧ್ರುವ T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, ಪರಿಮಾಣ(T));
    ಪ್ರತ್ಯಾಗಮ zs;
}

ಅನ್ತಃಸ್ಥ zstringlen::zstringlen(ozstream& zs, ಧ್ರುವ ವರ್ಣ* x) {
    val.byte = 255;  val.word = ::strlen(x);
    ಯದಿ (val.word < 255) zs < (val.byte = val.word);
    ಅನ್ಯಥಾ zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
ಅನ್ತಃಸ್ಥ ozstream& operator<(ozstream& zs, ಧ್ರುವ ವರ್ಣ* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.ಮೌಲ್ಯ());
    ಪ್ರತ್ಯಾಗಮ zs;
}

#ifdef _MSC_VER
ಅನ್ತಃಸ್ಥ ozstream& operator<(ozstream& zs, ವರ್ಣ* ಧ್ರುವ& x) {
    ಪ್ರತ್ಯಾಗಮ zs < (ಧ್ರುವ ವರ್ಣ*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
ಅನ್ತಃಸ್ಥ ostream& operator<<(ozstream& zs, ಧ್ರುವ T& x) {
    zs.os_flush();
    ಪ್ರತ್ಯಾಗಮ zs.os() << x;
}

#endif
