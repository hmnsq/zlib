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

#ଅନ୍ତର୍ଭାଵ <strstream.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ "zlib.h"

#ଯଦି defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_ଦ୍ୱିମିକ_MODE(file) setmode(fileno(file), O_ଦ୍ୱିମିକ)
#ଅନ୍ୟଥା
#   define SET_ଦ୍ୱିମିକ_MODE(file)
#endif

class zstringlen {
ସର୍ବସାଧାରଣ:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ଅଚଳ ଅକ୍ଷର*);
    size_t ମୂଲ୍ୟ() ଅଚଳ { ଫେରନ୍ତୁ val.word; }
ବ୍ୟକ୍ତିଗତ:
    ସଂରଚନା Val { unsigned ଅକ୍ଷର byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    ସର୍ବସାଧାରଣ:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ଅଚଳ ଅକ୍ଷର* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        ଶୂନ୍ୟ open(ଅଚଳ ଅକ୍ଷର* name) {
            ଯଦି (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        ଶୂନ୍ୟ open(FILE* fp) {
            SET_ଦ୍ୱିମିକ_MODE(fp);
            ଯଦି (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା close() {
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = ::gzclose(m_fp);
            m_fp = 0; ଫେରନ୍ତୁ r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଠ(ଶୂନ୍ୟ* buf, size_t len) {
            ଫେରନ୍ତୁ ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ଅଚଳ ଅକ୍ଷର* error(ପୂର୍ଣ୍ଣସଂଖ୍ୟା* errnum) {
            ଫେରନ୍ତୁ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ଫେରନ୍ତୁ m_fp; }

    ବ୍ୟକ୍ତିଗତ:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class ବସ୍ତୁଗୁଡ଼ିକ>
ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଠ(izstream& zs, T* x, ବସ୍ତୁଗୁଡ଼ିକ ବସ୍ତୁଗୁଡ଼ିକ_2) {
    ଫେରନ୍ତୁ ::gzread(zs.fp(), x, ବସ୍ତୁଗୁଡ଼ିକ_2*ଆକାର(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
ଧାଡ଼ିଭିତରେ izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, ଆକାର(T));
    ଫେରନ୍ତୁ zs;
}


ଧାଡ଼ିଭିତରେ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    ଯଦି (val.byte == 255) zs > val.word;
    ଅନ୍ୟଥା val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
ଧାଡ଼ିଭିତରେ izstream& operator>(izstream& zs, ଅକ୍ଷର* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.ମୂଲ୍ୟ());
    x[len.ମୂଲ୍ୟ()] = '\0';
    ଫେରନ୍ତୁ zs;
}

ଧାଡ଼ିଭିତରେ ଅକ୍ଷର* read_string(izstream& zs) {
    zstringlen len(zs);
    ଅକ୍ଷର* x = new ଅକ୍ଷର[len.ମୂଲ୍ୟ()+1];
    ::gzread(zs.fp(), x, len.ମୂଲ୍ୟ());
    x[len.ମୂଲ୍ୟ()] = '\0';
    ଫେରନ୍ତୁ x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    ସର୍ବସାଧାରଣ:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ଅଚଳ ଅକ୍ଷର* name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = Z_DEFAULT_COMPRESSION)
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
        ଶୂନ୍ୟ open(ଅଚଳ ଅକ୍ଷର* name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = Z_DEFAULT_COMPRESSION) {
            ଅକ୍ଷର mode[4] = "wb\0";
            ଯଦି (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            ଯଦି (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        ଶୂନ୍ୟ open(FILE* fp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = Z_DEFAULT_COMPRESSION) {
            SET_ଦ୍ୱିମିକ_MODE(fp);
            ଅକ୍ଷର mode[4] = "wb\0";
            ଯଦି (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            ଯଦି (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା close() {
            ଯଦି (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = ::gzclose(m_fp); m_fp = 0; ଫେରନ୍ତୁ r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ(ଅଚଳ ଶୂନ୍ୟ* buf, size_t len) {
            ଫେରନ୍ତୁ ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା flush(ପୂର୍ଣ୍ଣସଂଖ୍ୟା _flush) {
            os_flush();
            ଫେରନ୍ତୁ ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ଅଚଳ ଅକ୍ଷର* error(ପୂର୍ଣ୍ଣସଂଖ୍ୟା* errnum) {
            ଫେରନ୍ତୁ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ଫେରନ୍ତୁ m_fp; }

        ostream& os() {
            ଯଦି (m_os == 0) m_os = new ostrstream;
            ଫେରନ୍ତୁ *m_os;
        }

        ଶୂନ୍ୟ os_flush() {
            ଯଦି (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    ବ୍ୟକ୍ତିଗତ:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class ବସ୍ତୁଗୁଡ଼ିକ>
ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ(ozstream& zs, ଅଚଳ T* x, ବସ୍ତୁଗୁଡ଼ିକ ବସ୍ତୁଗୁଡ଼ିକ_2) {
    ଫେରନ୍ତୁ ::gzwrite(zs.fp(), (voidp) x, ବସ୍ତୁଗୁଡ଼ିକ_2*ଆକାର(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
ଧାଡ଼ିଭିତରେ ozstream& operator<(ozstream& zs, ଅଚଳ T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, ଆକାର(T));
    ଫେରନ୍ତୁ zs;
}

ଧାଡ଼ିଭିତରେ zstringlen::zstringlen(ozstream& zs, ଅଚଳ ଅକ୍ଷର* x) {
    val.byte = 255;  val.word = ::strlen(x);
    ଯଦି (val.word < 255) zs < (val.byte = val.word);
    ଅନ୍ୟଥା zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
ଧାଡ଼ିଭିତରେ ozstream& operator<(ozstream& zs, ଅଚଳ ଅକ୍ଷର* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.ମୂଲ୍ୟ());
    ଫେରନ୍ତୁ zs;
}

#ifdef _MSC_VER
ଧାଡ଼ିଭିତରେ ozstream& operator<(ozstream& zs, ଅକ୍ଷର* ଅଚଳ& x) {
    ଫେରନ୍ତୁ zs < (ଅଚଳ ଅକ୍ଷର*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
ଧାଡ଼ିଭିତରେ ostream& operator<<(ozstream& zs, ଅଚଳ T& x) {
    zs.os_flush();
    ଫେରନ୍ତୁ zs.os() << x;
}

#endif
