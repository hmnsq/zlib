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

#અન્તર્ભાવ <strstream.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ "zlib.h"

#જો defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_દ્વિઅંકી_MODE(file) setmode(fileno(file), O_દ્વિઅંકી)
#નહીંતર
#   define SET_દ્વિઅંકી_MODE(file)
#endif

class zstringlen {
જાહેર:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, અચલ અક્ષર*);
    size_t કિંમત() અચલ { પાછા val.word; }
ખાનગી:
    સંરચના Val { unsigned અક્ષર byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    જાહેર:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(અચલ અક્ષર* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        શૂન્ય open(અચલ અક્ષર* name) {
            જો (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        શૂન્ય open(FILE* fp) {
            SET_દ્વિઅંકી_MODE(fp);
            જો (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        પૂર્ણાંક close() {
            પૂર્ણાંક r = ::gzclose(m_fp);
            m_fp = 0; પાછા r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        પૂર્ણાંક પઠ(શૂન્ય* buf, size_t len) {
            પાછા ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        અચલ અક્ષર* error(પૂર્ણાંક* errnum) {
            પાછા ::gzerror(m_fp, errnum);
        }

        gzFile fp() { પાછા m_fp; }

    ખાનગી:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class વસ્તુઓ>
અંતઃરેખ પૂર્ણાંક પઠ(izstream& zs, T* x, વસ્તુઓ વસ્તુઓ_2) {
    પાછા ::gzread(zs.fp(), x, વસ્તુઓ_2*કદ(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
અંતઃરેખ izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, કદ(T));
    પાછા zs;
}


અંતઃરેખ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    જો (val.byte == 255) zs > val.word;
    નહીંતર val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
અંતઃરેખ izstream& operator>(izstream& zs, અક્ષર* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.કિંમત());
    x[len.કિંમત()] = '\0';
    પાછા zs;
}

અંતઃરેખ અક્ષર* read_string(izstream& zs) {
    zstringlen len(zs);
    અક્ષર* x = new અક્ષર[len.કિંમત()+1];
    ::gzread(zs.fp(), x, len.કિંમત());
    x[len.કિંમત()] = '\0';
    પાછા x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    જાહેર:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, પૂર્ણાંક level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(અચલ અક્ષર* name, પૂર્ણાંક level = Z_DEFAULT_COMPRESSION)
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
        શૂન્ય open(અચલ અક્ષર* name, પૂર્ણાંક level = Z_DEFAULT_COMPRESSION) {
            અક્ષર mode[4] = "wb\0";
            જો (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            જો (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        શૂન્ય open(FILE* fp, પૂર્ણાંક level = Z_DEFAULT_COMPRESSION) {
            SET_દ્વિઅંકી_MODE(fp);
            અક્ષર mode[4] = "wb\0";
            જો (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            જો (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        પૂર્ણાંક close() {
            જો (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            પૂર્ણાંક r = ::gzclose(m_fp); m_fp = 0; પાછા r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        પૂર્ણાંક લિખ(અચલ શૂન્ય* buf, size_t len) {
            પાછા ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        પૂર્ણાંક flush(પૂર્ણાંક _flush) {
            os_flush();
            પાછા ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        અચલ અક્ષર* error(પૂર્ણાંક* errnum) {
            પાછા ::gzerror(m_fp, errnum);
        }

        gzFile fp() { પાછા m_fp; }

        ostream& os() {
            જો (m_os == 0) m_os = new ostrstream;
            પાછા *m_os;
        }

        શૂન્ય os_flush() {
            જો (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    ખાનગી:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class વસ્તુઓ>
અંતઃરેખ પૂર્ણાંક લિખ(ozstream& zs, અચલ T* x, વસ્તુઓ વસ્તુઓ_2) {
    પાછા ::gzwrite(zs.fp(), (voidp) x, વસ્તુઓ_2*કદ(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
અંતઃરેખ ozstream& operator<(ozstream& zs, અચલ T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, કદ(T));
    પાછા zs;
}

અંતઃરેખ zstringlen::zstringlen(ozstream& zs, અચલ અક્ષર* x) {
    val.byte = 255;  val.word = ::strlen(x);
    જો (val.word < 255) zs < (val.byte = val.word);
    નહીંતર zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
અંતઃરેખ ozstream& operator<(ozstream& zs, અચલ અક્ષર* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.કિંમત());
    પાછા zs;
}

#ifdef _MSC_VER
અંતઃરેખ ozstream& operator<(ozstream& zs, અક્ષર* અચલ& x) {
    પાછા zs < (અચલ અક્ષર*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
અંતઃરેખ ostream& operator<<(ozstream& zs, અચલ T& x) {
    zs.os_flush();
    પાછા zs.os() << x;
}

#endif
