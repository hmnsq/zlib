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

#यदि defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_द्विआधारी_MODE(file) setmode(fileno(file), O_द्विआधारी)
#अन्यथा
#   define SET_द्विआधारी_MODE(file)
#endif

class zstringlen {
सार्वजनिक:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ध्रुव वर्ण*);
    size_t मूल्य() ध्रुव { प्रत्यागम val.word; }
गुप्त:
    संरचना Val { unsigned वर्ण byte; size_t word; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    सार्वजनिक:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ध्रुव वर्ण* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        शून्य open(ध्रुव वर्ण* name) {
            यदि (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        शून्य open(FILE* fp) {
            SET_द्विआधारी_MODE(fp);
            यदि (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        पूर्णाङ्क close() {
            पूर्णाङ्क r = ::gzclose(m_fp);
            m_fp = 0; प्रत्यागम r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        पूर्णाङ्क पठ(शून्य* buf, size_t len) {
            प्रत्यागम ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ध्रुव वर्ण* error(पूर्णाङ्क* errnum) {
            प्रत्यागम ::gzerror(m_fp, errnum);
        }

        gzFile fp() { प्रत्यागम m_fp; }

    गुप्त:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
template <class T, class वस्तुएँ>
अन्तःस्थ पूर्णाङ्क पठ(izstream& zs, T* x, वस्तुएँ वस्तुएँ_2) {
    प्रत्यागम ::gzread(zs.fp(), x, वस्तुएँ_2*परिमाण(T));
}

/*
 * Binary input with the '>' operator.
 */
template <class T>
अन्तःस्थ izstream& operator>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, परिमाण(T));
    प्रत्यागम zs;
}


अन्तःस्थ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    यदि (val.byte == 255) zs > val.word;
    अन्यथा val.word = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
अन्तःस्थ izstream& operator>(izstream& zs, वर्ण* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.मूल्य());
    x[len.मूल्य()] = '\0';
    प्रत्यागम zs;
}

अन्तःस्थ वर्ण* read_string(izstream& zs) {
    zstringlen len(zs);
    वर्ण* x = new वर्ण[len.मूल्य()+1];
    ::gzread(zs.fp(), x, len.मूल्य());
    x[len.मूल्य()] = '\0';
    प्रत्यागम x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    सार्वजनिक:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, पूर्णाङ्क level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ध्रुव वर्ण* name, पूर्णाङ्क level = Z_DEFAULT_COMPRESSION)
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
        शून्य open(ध्रुव वर्ण* name, पूर्णाङ्क level = Z_DEFAULT_COMPRESSION) {
            वर्ण mode[4] = "wb\0";
            यदि (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            यदि (m_fp) close();
            m_fp = ::gzopen(name, mode);
        }

        /* open from a FILE pointer.
         */
        शून्य open(FILE* fp, पूर्णाङ्क level = Z_DEFAULT_COMPRESSION) {
            SET_द्विआधारी_MODE(fp);
            वर्ण mode[4] = "wb\0";
            यदि (level != Z_DEFAULT_COMPRESSION) mode[2] = '0'+level;
            यदि (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), mode);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        पूर्णाङ्क close() {
            यदि (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            पूर्णाङ्क r = ::gzclose(m_fp); m_fp = 0; प्रत्यागम r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        पूर्णाङ्क लिख(ध्रुव शून्य* buf, size_t len) {
            प्रत्यागम ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        पूर्णाङ्क flush(पूर्णाङ्क _flush) {
            os_flush();
            प्रत्यागम ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ध्रुव वर्ण* error(पूर्णाङ्क* errnum) {
            प्रत्यागम ::gzerror(m_fp, errnum);
        }

        gzFile fp() { प्रत्यागम m_fp; }

        ostream& os() {
            यदि (m_os == 0) m_os = new ostrstream;
            प्रत्यागम *m_os;
        }

        शून्य os_flush() {
            यदि (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->width(m_os->width());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    गुप्त:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
template <class T, class वस्तुएँ>
अन्तःस्थ पूर्णाङ्क लिख(ozstream& zs, ध्रुव T* x, वस्तुएँ वस्तुएँ_2) {
    प्रत्यागम ::gzwrite(zs.fp(), (voidp) x, वस्तुएँ_2*परिमाण(T));
}

/*
 * Binary output with the '<' operator.
 */
template <class T>
अन्तःस्थ ozstream& operator<(ozstream& zs, ध्रुव T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, परिमाण(T));
    प्रत्यागम zs;
}

अन्तःस्थ zstringlen::zstringlen(ozstream& zs, ध्रुव वर्ण* x) {
    val.byte = 255;  val.word = ::strlen(x);
    यदि (val.word < 255) zs < (val.byte = val.word);
    अन्यथा zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
अन्तःस्थ ozstream& operator<(ozstream& zs, ध्रुव वर्ण* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.मूल्य());
    प्रत्यागम zs;
}

#ifdef _MSC_VER
अन्तःस्थ ozstream& operator<(ozstream& zs, वर्ण* ध्रुव& x) {
    प्रत्यागम zs < (ध्रुव वर्ण*) x;
}
#endif

/*
 * Ascii write with the << operator;
 */
template <class T>
अन्तःस्थ ostream& operator<<(ozstream& zs, ध्रुव T& x) {
    zs.os_flush();
    प्रत्यागम zs.os() << x;
}

#endif
