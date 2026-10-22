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

#അന്തര്ഭാവ <strstream.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ "zlib.h"

#എങ്കിൽ defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_ദ്വിമാന_MODE(file) setmode(fileno(file), O_ദ്വിമാന)
#അല്ലെങ്കിൽ
#   define SET_ദ്വിമാന_MODE(file)
#endif

class zstringlen {
പൊതു:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, സ്ഥിരം അക്ഷരം*);
    size_t മൂല്യം() സ്ഥിരം { തിരികെ val.word; }
സ്വകാര്യ:
    ഘടന Val { unsigned അക്ഷരം byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    പൊതു:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(സ്ഥിരം അക്ഷരം* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        ശൂന്യം open(സ്ഥിരം അക്ഷരം* name) {
            എങ്കിൽ (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        ശൂന്യം open(FILE* fp) {
            SET_ദ്വിമാന_MODE(fp);
            എങ്കിൽ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        പൂർണ്ണസംഖ്യ close() {
            പൂർണ്ണസംഖ്യ r = ::gzclose(m_fp);
            m_fp = 0; തിരികെ r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        പൂർണ്ണസംഖ്യ പഠ(ശൂന്യം* buf, size_t len) {
            തിരികെ ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        സ്ഥിരം അക്ഷരം* error(പൂർണ്ണസംഖ്യ* errnum) {
            തിരികെ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { തിരികെ m_fp; }

    സ്വകാര്യ:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class ഇനങ്ങൾ>
വരിയിൽ പൂർണ്ണസംഖ്യ പഠ(izstream& zs, T* x, ഇനങ്ങൾ ഇനങ്ങൾ_2) {
    തിരികെ ::gzread(zs.fp(), x, ഇനങ്ങൾ_2*വലുപ്പം(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
വരിയിൽ izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, വലുപ്പം(T));
    തിരികെ zs;
}


വരിയിൽ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    എങ്കിൽ (val.byte == 255) zs > val.word;
    അല്ലെങ്കിൽ val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
വരിയിൽ izstream& operator>(izstream& zs, അക്ഷരം* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.മൂല്യം());
    x[len.മൂല്യം()] = '\0';
    തിരികെ zs;
}

വരിയിൽ അക്ഷരം* read_string(izstream& zs) {
    zstringlen len(zs);
    അക്ഷരം* x = new അക്ഷരം[len.മൂല്യം()+1];
    ::gzread(zs.fp(), x, len.മൂല്യം());
    x[len.മൂല്യം()] = '\0';
    തിരികെ x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    പൊതു:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, പൂർണ്ണസംഖ്യ level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(സ്ഥിരം അക്ഷരം* name, പൂർണ്ണസംഖ്യ level = Z_DEFAULT_COMPRESSION)
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
        ശൂന്യം open(സ്ഥിരം അക്ഷരം* name, പൂർണ്ണസംഖ്യ level = Z_DEFAULT_COMPRESSION) {
            അക്ഷരം mode[4] = "wb\0";
            എങ്കിൽ (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            എങ്കിൽ (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        ശൂന്യം open(FILE* fp, പൂർണ്ണസംഖ്യ level = Z_DEFAULT_COMPRESSION) {
            SET_ദ്വിമാന_MODE(fp);
            അക്ഷരം mode[4] = "wb\0";
            എങ്കിൽ (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            എങ്കിൽ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        പൂർണ്ണസംഖ്യ close() {
            എങ്കിൽ (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            പൂർണ്ണസംഖ്യ r = ::gzclose(m_fp); m_fp = 0; തിരികെ r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        പൂർണ്ണസംഖ്യ ലിഖ(സ്ഥിരം ശൂന്യം* buf, size_t len) {
            തിരികെ ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        പൂർണ്ണസംഖ്യ flush(പൂർണ്ണസംഖ്യ _flush) {
            os_flush();
            തിരികെ ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        സ്ഥിരം അക്ഷരം* error(പൂർണ്ണസംഖ്യ* errnum) {
            തിരികെ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { തിരികെ m_fp; }

        ostream& os() {
            എങ്കിൽ (m_os == 0) m_os = new ostrstream;
            തിരികെ *m_os;
        }

        ശൂന്യം os_flush() {
            എങ്കിൽ (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    സ്വകാര്യ:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class ഇനങ്ങൾ>
വരിയിൽ പൂർണ്ണസംഖ്യ ലിഖ(ozstream& zs, സ്ഥിരം T* x, ഇനങ്ങൾ ഇനങ്ങൾ_2) {
    തിരികെ ::gzwrite(zs.fp(), (voidp) x, ഇനങ്ങൾ_2*വലുപ്പം(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
വരിയിൽ ozstream& operator<(ozstream& zs, സ്ഥിരം T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, വലുപ്പം(T));
    തിരികെ zs;
}

വരിയിൽ zstringlen::zstringlen(ozstream& zs, സ്ഥിരം അക്ഷരം* x) {
    val.byte = 255;  val.word = ::strlen(x);
    എങ്കിൽ (val.word < 255) zs < (val.byte = val.word);
    അല്ലെങ്കിൽ zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
വരിയിൽ ozstream& operator<(ozstream& zs, സ്ഥിരം അക്ഷരം* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.മൂല്യം());
    തിരികെ zs;
}

#ifdef _MSC_VER
വരിയിൽ ozstream& operator<(ozstream& zs, അക്ഷരം* സ്ഥിരം& x) {
    തിരികെ zs < (സ്ഥിരം അക്ഷരം*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
വരിയിൽ ostream& operator<<(ozstream& zs, സ്ഥിരം T& x) {
    zs.os_flush();
    തിരികെ zs.os() << x;
}

#endif
