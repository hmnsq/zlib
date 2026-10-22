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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZSTREAM__H
#ವ್ಯಾಖ್ಯಾನಿಸು ZSTREAM__H

/*
 * zstream.h - C++ interface to the 'zlib' general purpose compression library
 * $Id: zstream.h 1.1 1997-06-25 12:00:56+02 tyge Exp tyge $
 */

#ಅನ್ತರ್ಭಾವ <strstream.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

#ಆದರೆ defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file) setmode(fileno(file), O_ದ್ವಿಮಾನ)
#ಇಲ್ಲದಿದ್ದರೆ
#   define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file)
#ಆದರೆ_ಅಂತ್ಯ

class zstringlen {
ಸಾರ್ವಜನಿಕ:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
    size_t ಮೌಲ್ಯ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ val.ಶಬ್ದ; }
ಖಾಸಗಿ:
    ರಚನೆ Val { ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ byte; size_t ಶಬ್ದ; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    ಸಾರ್ವಜನಿಕ:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        ಶೂನ್ಯ open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name) {
            ಆದರೆ (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        ಶೂನ್ಯ open(FILE* fp) {
            ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(fp);
            ಆದರೆ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ಪೂರ್ಣಾಂಕ close() {
            ಪೂರ್ಣಾಂಕ r = ::gzclose(m_fp);
            m_fp = 0; ಹಿಂತಿರುಗಿ r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        ಪೂರ್ಣಾಂಕ ಪಠ(ಶೂನ್ಯ* buf, size_t len) {
            ಹಿಂತಿರುಗಿ ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* error(ಪೂರ್ಣಾಂಕ* errnum) {
            ಹಿಂತಿರುಗಿ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ಹಿಂತಿರುಗಿ m_fp; }

    ಖಾಸಗಿ:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
ಮಾದರಿ <class T, class ವಸ್ತುಗಳು>
ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ಪಠ(izstream& zs, T* x, ವಸ್ತುಗಳು ವಸ್ತುಗಳು_2) {
    ಹಿಂತಿರುಗಿ ::gzread(zs.fp(), x, ವಸ್ತುಗಳು_2*ಗಾತ್ರ(T));
}

/*
 * Binary input with the '>' operator.
 */
ಮಾದರಿ <class T>
ಸಾಲೊಳಗೆ izstream& ಕ್ರಿಯಾಕಾರಕ>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, ಗಾತ್ರ(T));
    ಹಿಂತಿರುಗಿ zs;
}


ಸಾಲೊಳಗೆ zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    ಆದರೆ (val.byte == 255) zs > val.ಶಬ್ದ;
    ಇಲ್ಲದಿದ್ದರೆ val.ಶಬ್ದ = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
ಸಾಲೊಳಗೆ izstream& ಕ್ರಿಯಾಕಾರಕ>(izstream& zs, ಅಕ್ಷರ* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.ಮೌಲ್ಯ());
    x[len.ಮೌಲ್ಯ()] = '\0';
    ಹಿಂತಿರುಗಿ zs;
}

ಸಾಲೊಳಗೆ ಅಕ್ಷರ* read_string(izstream& zs) {
    zstringlen len(zs);
    ಅಕ್ಷರ* x = new ಅಕ್ಷರ[len.ಮೌಲ್ಯ()+1];
    ::gzread(zs.fp(), x, len.ಮೌಲ್ಯ());
    x[len.ಮೌಲ್ಯ()] = '\0';
    ಹಿಂತಿರುಗಿ x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    ಸಾರ್ವಜನಿಕ:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, ಪೂರ್ಣಾಂಕ level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name, ಪೂರ್ಣಾಂಕ level = Z_DEFAULT_COMPRESSION)
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
        ಶೂನ್ಯ open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name, ಪೂರ್ಣಾಂಕ level = Z_DEFAULT_COMPRESSION) {
            ಅಕ್ಷರ ಬಹುಲಕ[4] = "wb\0";
            ಆದರೆ (level != Z_DEFAULT_COMPRESSION) ಬಹುಲಕ[2] = '0'+level;
            ಆದರೆ (m_fp) close();
            m_fp = ::gzopen(name, ಬಹುಲಕ);
        }

        /* open from a FILE pointer.
         */
        ಶೂನ್ಯ open(FILE* fp, ಪೂರ್ಣಾಂಕ level = Z_DEFAULT_COMPRESSION) {
            ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(fp);
            ಅಕ್ಷರ ಬಹುಲಕ[4] = "wb\0";
            ಆದರೆ (level != Z_DEFAULT_COMPRESSION) ಬಹುಲಕ[2] = '0'+level;
            ಆದರೆ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), ಬಹುಲಕ);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        ಪೂರ್ಣಾಂಕ close() {
            ಆದರೆ (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            ಪೂರ್ಣಾಂಕ r = ::gzclose(m_fp); m_fp = 0; ಹಿಂತಿರುಗಿ r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        ಪೂರ್ಣಾಂಕ ಲಿಖ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* buf, size_t len) {
            ಹಿಂತಿರುಗಿ ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        ಪೂರ್ಣಾಂಕ flush(ಪೂರ್ಣಾಂಕ _flush) {
            os_flush();
            ಹಿಂತಿರುಗಿ ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* error(ಪೂರ್ಣಾಂಕ* errnum) {
            ಹಿಂತಿರುಗಿ ::gzerror(m_fp, errnum);
        }

        gzFile fp() { ಹಿಂತಿರುಗಿ m_fp; }

        ostream& os() {
            ಆದರೆ (m_os == 0) m_os = new ostrstream;
            ಹಿಂತಿರುಗಿ *m_os;
        }

        ಶೂನ್ಯ os_flush() {
            ಆದರೆ (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->ಅಗಲ(m_os->ಅಗಲ());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    ಖಾಸಗಿ:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
ಮಾದರಿ <class T, class ವಸ್ತುಗಳು>
ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ಲಿಖ(ozstream& zs, ಸ್ಥಿರಾಂಕ T* x, ವಸ್ತುಗಳು ವಸ್ತುಗಳು_2) {
    ಹಿಂತಿರುಗಿ ::gzwrite(zs.fp(), (voidp) x, ವಸ್ತುಗಳು_2*ಗಾತ್ರ(T));
}

/*
 * Binary output with the '<' operator.
 */
ಮಾದರಿ <class T>
ಸಾಲೊಳಗೆ ozstream& ಕ್ರಿಯಾಕಾರಕ<(ozstream& zs, ಸ್ಥಿರಾಂಕ T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, ಗಾತ್ರ(T));
    ಹಿಂತಿರುಗಿ zs;
}

ಸಾಲೊಳಗೆ zstringlen::zstringlen(ozstream& zs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* x) {
    val.byte = 255;  val.ಶಬ್ದ = ::ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(x);
    ಆದರೆ (val.ಶಬ್ದ < 255) zs < (val.byte = val.ಶಬ್ದ);
    ಇಲ್ಲದಿದ್ದರೆ zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
ಸಾಲೊಳಗೆ ozstream& ಕ್ರಿಯಾಕಾರಕ<(ozstream& zs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.ಮೌಲ್ಯ());
    ಹಿಂತಿರುಗಿ zs;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
ಸಾಲೊಳಗೆ ozstream& ಕ್ರಿಯಾಕಾರಕ<(ozstream& zs, ಅಕ್ಷರ* ಸ್ಥಿರಾಂಕ& x) {
    ಹಿಂತಿರುಗಿ zs < (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*) x;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Ascii write with the << operator;
 */
ಮಾದರಿ <class T>
ಸಾಲೊಳಗೆ ostream& ಕ್ರಿಯಾಕಾರಕ<<(ozstream& zs, ಸ್ಥಿರಾಂಕ T& x) {
    zs.os_flush();
    ಹಿಂತಿರುಗಿ zs.os() << x;
}

#ಆದರೆ_ಅಂತ್ಯ
