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

#அந்தர்பாவ <strstream.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ "zlib.h"

#எனில் defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_இருமம்_MODE(file) setmode(fileno(file), O_இருமம்)
#இல்லையெனில்
#   define SET_இருமம்_MODE(file)
#endif

class zstringlen {
பொது:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, மாறிலி எழுத்து*);
    size_t மதிப்பு() மாறிலி { திரும்பு val.word; }
தனிப்பட்ட:
    கட்டமைப்பு Val { unsigned எழுத்து byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    பொது:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(மாறிலி எழுத்து* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        வெறுமை open(மாறிலி எழுத்து* name) {
            எனில் (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        வெறுமை open(FILE* fp) {
            SET_இருமம்_MODE(fp);
            எனில் (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        முழு close() {
            முழு r = ::gzclose(m_fp);
            m_fp = 0; திரும்பு r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        முழு பட(வெறுமை* buf, size_t len) {
            திரும்பு ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        மாறிலி எழுத்து* error(முழு* errnum) {
            திரும்பு ::gzerror(m_fp, errnum);
        }

        gzFile fp() { திரும்பு m_fp; }

    தனிப்பட்ட:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class பொருட்கள்>
உள்வரி முழு பட(izstream& zs, T* x, பொருட்கள் பொருட்கள்_2) {
    திரும்பு ::gzread(zs.fp(), x, பொருட்கள்_2*அளவு(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
உள்வரி izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, அளவு(T));
    திரும்பு zs;
}


உள்வரி zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    எனில் (val.byte == 255) zs > val.word;
    இல்லையெனில் val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
உள்வரி izstream& operator>(izstream& zs, எழுத்து* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.மதிப்பு());
    x[len.மதிப்பு()] = '\0';
    திரும்பு zs;
}

உள்வரி எழுத்து* read_string(izstream& zs) {
    zstringlen len(zs);
    எழுத்து* x = new எழுத்து[len.மதிப்பு()+1];
    ::gzread(zs.fp(), x, len.மதிப்பு());
    x[len.மதிப்பு()] = '\0';
    திரும்பு x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    பொது:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, முழு level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(மாறிலி எழுத்து* name, முழு level = Z_DEFAULT_COMPRESSION)
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
        வெறுமை open(மாறிலி எழுத்து* name, முழு level = Z_DEFAULT_COMPRESSION) {
            எழுத்து mode[4] = "wb\0";
            எனில் (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            எனில் (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        வெறுமை open(FILE* fp, முழு level = Z_DEFAULT_COMPRESSION) {
            SET_இருமம்_MODE(fp);
            எழுத்து mode[4] = "wb\0";
            எனில் (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            எனில் (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        முழு close() {
            எனில் (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            முழு r = ::gzclose(m_fp); m_fp = 0; திரும்பு r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        முழு லிக(மாறிலி வெறுமை* buf, size_t len) {
            திரும்பு ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        முழு flush(முழு _flush) {
            os_flush();
            திரும்பு ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        மாறிலி எழுத்து* error(முழு* errnum) {
            திரும்பு ::gzerror(m_fp, errnum);
        }

        gzFile fp() { திரும்பு m_fp; }

        ostream& os() {
            எனில் (m_os == 0) m_os = new ostrstream;
            திரும்பு *m_os;
        }

        வெறுமை os_flush() {
            எனில் (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    தனிப்பட்ட:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class பொருட்கள்>
உள்வரி முழு லிக(ozstream& zs, மாறிலி T* x, பொருட்கள் பொருட்கள்_2) {
    திரும்பு ::gzwrite(zs.fp(), (voidp) x, பொருட்கள்_2*அளவு(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
உள்வரி ozstream& operator<(ozstream& zs, மாறிலி T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, அளவு(T));
    திரும்பு zs;
}

உள்வரி zstringlen::zstringlen(ozstream& zs, மாறிலி எழுத்து* x) {
    val.byte = 255;  val.word = ::strlen(x);
    எனில் (val.word < 255) zs < (val.byte = val.word);
    இல்லையெனில் zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
உள்வரி ozstream& operator<(ozstream& zs, மாறிலி எழுத்து* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.மதிப்பு());
    திரும்பு zs;
}

#ifdef _MSC_VER
உள்வரி ozstream& operator<(ozstream& zs, எழுத்து* மாறிலி& x) {
    திரும்பு zs < (மாறிலி எழுத்து*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
உள்வரி ostream& operator<<(ozstream& zs, மாறிலி T& x) {
    zs.os_flush();
    திரும்பு zs.os() << x;
}

#endif
