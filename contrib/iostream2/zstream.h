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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZSTREAM__H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZSTREAM__H

/*
 * zstream.h - C++ interface to the 'zlib' general purpose compression library
 * $Id: zstream.h 1.1 1997-06-25 12:00:56+02 tyge Exp tyge $
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <strstream.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

#ਜੇ defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file) setmode(fileno(file), O_ਦੁਆਧਾਰੀ)
#ਨਹੀਂਤਾਂ
#   define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file)
#ਜੇ_ਅੰਤ

class zstringlen {
ਜਨਤਕ:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ਅਚਲ ਅੱਖਰ*);
    size_t ਮੁੱਲ() ਅਚਲ { ਵਾਪਸ val.ਸ਼ਬਦ; }
ਨਿਜੀ:
    ਬਣਤਰ Val { ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ byte; size_t ਸ਼ਬਦ; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    ਜਨਤਕ:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ਅਚਲ ਅੱਖਰ* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        ਖਾਲੀ open(ਅਚਲ ਅੱਖਰ* name) {
            ਜੇ (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        ਖਾਲੀ open(FILE* fp) {
            ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(fp);
            ਜੇ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ਪੂਰਨ close() {
            ਪੂਰਨ r = ::gzclose(m_fp);
            m_fp = 0; ਵਾਪਸ r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        ਪੂਰਨ ਪਠ(ਖਾਲੀ* buf, size_t len) {
            ਵਾਪਸ ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ਅਚਲ ਅੱਖਰ* error(ਪੂਰਨ* errnum) {
            ਵਾਪਸ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ਵਾਪਸ m_fp; }

    ਨਿਜੀ:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
ਸਾਂਚਾ <class T, class ਚੀਜ਼ਾਂ>
ਸਤਰਅੰਦਰ ਪੂਰਨ ਪਠ(izstream& zs, T* x, ਚੀਜ਼ਾਂ ਚੀਜ਼ਾਂ_2) {
    ਵਾਪਸ ::gzread(zs.fp(), x, ਚੀਜ਼ਾਂ_2*ਆਕਾਰ(T));
}

/*
 * Binary input with the '>' operator.
 */
ਸਾਂਚਾ <class T>
ਸਤਰਅੰਦਰ izstream& ਸੰਚਾਲਕ>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, ਆਕਾਰ(T));
    ਵਾਪਸ zs;
}


ਸਤਰਅੰਦਰ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    ਜੇ (val.byte == 255) zs > val.ਸ਼ਬਦ;
    ਨਹੀਂਤਾਂ val.ਸ਼ਬਦ = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
ਸਤਰਅੰਦਰ izstream& ਸੰਚਾਲਕ>(izstream& zs, ਅੱਖਰ* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.ਮੁੱਲ());
    x[len.ਮੁੱਲ()] = '\0';
    ਵਾਪਸ zs;
}

ਸਤਰਅੰਦਰ ਅੱਖਰ* read_string(izstream& zs) {
    zstringlen len(zs);
    ਅੱਖਰ* x = new ਅੱਖਰ[len.ਮੁੱਲ()+1];
    ::gzread(zs.fp(), x, len.ਮੁੱਲ());
    x[len.ਮੁੱਲ()] = '\0';
    ਵਾਪਸ x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    ਜਨਤਕ:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, ਪੂਰਨ level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ਅਚਲ ਅੱਖਰ* name, ਪੂਰਨ level = Z_DEFAULT_COMPRESSION)
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
        ਖਾਲੀ open(ਅਚਲ ਅੱਖਰ* name, ਪੂਰਨ level = Z_DEFAULT_COMPRESSION) {
            ਅੱਖਰ ਬਹੁਲਕ[4] = "wb\0";
            ਜੇ (level != Z_DEFAULT_COMPRESSION) ਬਹੁਲਕ[2] = '0'+level;
            ਜੇ (m_fp) close();
            m_fp = ::gzopen(name, ਬਹੁਲਕ);
        }

        /* open from a FILE pointer.
         */
        ਖਾਲੀ open(FILE* fp, ਪੂਰਨ level = Z_DEFAULT_COMPRESSION) {
            ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(fp);
            ਅੱਖਰ ਬਹੁਲਕ[4] = "wb\0";
            ਜੇ (level != Z_DEFAULT_COMPRESSION) ਬਹੁਲਕ[2] = '0'+level;
            ਜੇ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), ਬਹੁਲਕ);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ਪੂਰਨ close() {
            ਜੇ (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            ਪੂਰਨ r = ::gzclose(m_fp); m_fp = 0; ਵਾਪਸ r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        ਪੂਰਨ ਲਿਖ(ਅਚਲ ਖਾਲੀ* buf, size_t len) {
            ਵਾਪਸ ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        ਪੂਰਨ flush(ਪੂਰਨ _flush) {
            os_flush();
            ਵਾਪਸ ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ਅਚਲ ਅੱਖਰ* error(ਪੂਰਨ* errnum) {
            ਵਾਪਸ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ਵਾਪਸ m_fp; }

        ostream& os() {
            ਜੇ (m_os == 0) m_os = new ostrstream;
            ਵਾਪਸ *m_os;
        }

        ਖਾਲੀ os_flush() {
            ਜੇ (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->ਚੌੜਾਈ(m_os->ਚੌੜਾਈ());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    ਨਿਜੀ:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
ਸਾਂਚਾ <class T, class ਚੀਜ਼ਾਂ>
ਸਤਰਅੰਦਰ ਪੂਰਨ ਲਿਖ(ozstream& zs, ਅਚਲ T* x, ਚੀਜ਼ਾਂ ਚੀਜ਼ਾਂ_2) {
    ਵਾਪਸ ::gzwrite(zs.fp(), (voidp) x, ਚੀਜ਼ਾਂ_2*ਆਕਾਰ(T));
}

/*
 * Binary output with the '<' operator.
 */
ਸਾਂਚਾ <class T>
ਸਤਰਅੰਦਰ ozstream& ਸੰਚਾਲਕ<(ozstream& zs, ਅਚਲ T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, ਆਕਾਰ(T));
    ਵਾਪਸ zs;
}

ਸਤਰਅੰਦਰ zstringlen::zstringlen(ozstream& zs, ਅਚਲ ਅੱਖਰ* x) {
    val.byte = 255;  val.ਸ਼ਬਦ = ::ਲੜੀ_ਲੰਬਾਈ(x);
    ਜੇ (val.ਸ਼ਬਦ < 255) zs < (val.byte = val.ਸ਼ਬਦ);
    ਨਹੀਂਤਾਂ zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
ਸਤਰਅੰਦਰ ozstream& ਸੰਚਾਲਕ<(ozstream& zs, ਅਚਲ ਅੱਖਰ* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.ਮੁੱਲ());
    ਵਾਪਸ zs;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
ਸਤਰਅੰਦਰ ozstream& ਸੰਚਾਲਕ<(ozstream& zs, ਅੱਖਰ* ਅਚਲ& x) {
    ਵਾਪਸ zs < (ਅਚਲ ਅੱਖਰ*) x;
}
#ਜੇ_ਅੰਤ

/*
 * Ascii write with the << operator;
 */
ਸਾਂਚਾ <class T>
ਸਤਰਅੰਦਰ ostream& ਸੰਚਾਲਕ<<(ozstream& zs, ਅਚਲ T& x) {
    zs.os_flush();
    ਵਾਪਸ zs.os() << x;
}

#ਜੇ_ਅੰਤ
