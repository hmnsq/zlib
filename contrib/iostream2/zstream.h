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

#నిర్వచితం_కాకపోతే ZSTREAM__H
#నిర్వచించు ZSTREAM__H

/*
 * zstream.h - C++ interface to the 'zlib' general purpose compression library
 * $Id: zstream.h 1.1 1997-06-25 12:00:56+02 tyge Exp tyge $
 */

#అన్తర్భావ <strstream.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ "zlib.h"

#ఒకవేళ defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define సమితి_ద్విమాన_బహుళకం(file) setmode(fileno(file), O_ద్విమాన)
#లేకపోతే
#   define సమితి_ద్విమాన_బహుళకం(file)
#అయితే_ముగింపు

class zstringlen {
బహిరంగ:
    zstringlen(class izstream&);
    zstringlen(class ozstream&, స్థిరరాశి అక్షరం*);
    size_t విలువ() స్థిరరాశి { తిరిగి val.మాట; }
అంతరంగ:
    నిర్మాణం Val { గుర్తులేని అక్షరం byte; size_t మాట; } val;
};

//  ----------------------------- izstream -----------------------------

class izstream
{
    బహిరంగ:
        izstream() : m_fp(0) {}
        izstream(FILE* fp) : m_fp(0) { open(fp); }
        izstream(స్థిరరాశి అక్షరం* name) : m_fp(0) { open(name); }
        ~izstream() { close(); }

        /* Opens a gzip (.gz) file for reading.
         * open() can be used to read a file which is not in gzip format;
         * in this case read() will directly read from the file without
         * decompression. errno can be checked to distinguish two error
         * cases (if errno is zero, the zlib error is Z_MEM_ERROR).
         */
        శూన్య open(స్థిరరాశి అక్షరం* name) {
            ఒకవేళ (m_fp) close();
            m_fp = ::gzopen(name, "rb");
        }

        శూన్య open(FILE* fp) {
            సమితి_ద్విమాన_బహుళకం(fp);
            ఒకవేళ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), "rb");
        }

        /* Flushes all pending input if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        పూర్ణాంకం close() {
            పూర్ణాంకం r = ::gzclose(m_fp);
            m_fp = 0; తిరిగి r;
        }

        /* Binary read the given number of bytes from the compressed file.
         */
        పూర్ణాంకం పఠ(శూన్య* buf, size_t len) {
            తిరిగి ::gzread(m_fp, buf, len);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        స్థిరరాశి అక్షరం* error(పూర్ణాంకం* errnum) {
            తిరిగి ::gzerror(m_fp, errnum);
        }

        gzFile fp() { తిరిగి m_fp; }

    అంతరంగ:
        gzFile m_fp;
};

/*
 * Binary read the given (array of) object(s) from the compressed file.
 * If the input file was not in gzip format, read() copies the objects number
 * of bytes into the buffer.
 * returns the number of uncompressed bytes actually read
 * (0 for end of file, -1 for error).
 */
మూస <class T, class వస్తువులు>
అంతఃవరుస పూర్ణాంకం పఠ(izstream& zs, T* x, వస్తువులు వస్తువులు_2) {
    తిరిగి ::gzread(zs.fp(), x, వస్తువులు_2*పరిమాణం(T));
}

/*
 * Binary input with the '>' operator.
 */
మూస <class T>
అంతఃవరుస izstream& క్రియాకారకం>(izstream& zs, T& x) {
    ::gzread(zs.fp(), &x, పరిమాణం(T));
    తిరిగి zs;
}


అంతఃవరుస zstringlen::zstringlen(izstream& zs) {
    zs > val.byte;
    ఒకవేళ (val.byte == 255) zs > val.మాట;
    లేకపోతే val.మాట = val.byte;
}

/*
 * Read length of string + the string with the '>' operator.
 */
అంతఃవరుస izstream& క్రియాకారకం>(izstream& zs, అక్షరం* x) {
    zstringlen len(zs);
    ::gzread(zs.fp(), x, len.విలువ());
    x[len.విలువ()] = '\0';
    తిరిగి zs;
}

అంతఃవరుస అక్షరం* read_string(izstream& zs) {
    zstringlen len(zs);
    అక్షరం* x = new అక్షరం[len.విలువ()+1];
    ::gzread(zs.fp(), x, len.విలువ());
    x[len.విలువ()] = '\0';
    తిరిగి x;
}

// ----------------------------- ozstream -----------------------------

class ozstream
{
    బహిరంగ:
        ozstream() : m_fp(0), m_os(0) {
        }
        ozstream(FILE* fp, పూర్ణాంకం level = Z_DEFAULT_COMPRESSION)
            : m_fp(0), m_os(0) {
            open(fp, level);
        }
        ozstream(స్థిరరాశి అక్షరం* name, పూర్ణాంకం level = Z_DEFAULT_COMPRESSION)
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
        శూన్య open(స్థిరరాశి అక్షరం* name, పూర్ణాంకం level = Z_DEFAULT_COMPRESSION) {
            అక్షరం బహుళకం[4] = "wb\0";
            ఒకవేళ (level != Z_DEFAULT_COMPRESSION) బహుళకం[2] = '0'+level;
            ఒకవేళ (m_fp) close();
            m_fp = ::gzopen(name, బహుళకం);
        }

        /* open from a FILE pointer.
         */
        శూన్య open(FILE* fp, పూర్ణాంకం level = Z_DEFAULT_COMPRESSION) {
            సమితి_ద్విమాన_బహుళకం(fp);
            అక్షరం బహుళకం[4] = "wb\0";
            ఒకవేళ (level != Z_DEFAULT_COMPRESSION) బహుళకం[2] = '0'+level;
            ఒకవేళ (m_fp) close();
            m_fp = ::gzdopen(fileno(fp), బహుళకం);
        }

        /* Flushes all pending output if necessary, closes the compressed file
         * and deallocates all the (de)compression state. The return value is
         * the zlib error number (see function error() below).
         */
        పూర్ణాంకం close() {
            ఒకవేళ (m_os) {
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = 0;
            }
            పూర్ణాంకం r = ::gzclose(m_fp); m_fp = 0; తిరిగి r;
        }

        /* Binary write the given number of bytes into the compressed file.
         */
        పూర్ణాంకం లిఖ(స్థిరరాశి శూన్య* buf, size_t len) {
            తిరిగి ::gzwrite(m_fp, (voidp) buf, len);
        }

        /* Flushes all pending output into the compressed file. The parameter
         * _flush is as in the deflate() function. The return value is the zlib
         * error number (see function gzerror below). flush() returns Z_OK if
         * the flush_ parameter is Z_FINISH and all output could be flushed.
         * flush() should be called only when strictly necessary because it can
         * degrade compression.
         */
        పూర్ణాంకం flush(పూర్ణాంకం _flush) {
            os_flush();
            తిరిగి ::gzflush(m_fp, _flush);
        }

        /* Returns the error message for the last error which occurred on the
         * given compressed file. errnum is set to zlib error number. If an
         * error occurred in the file system and not in the compression library,
         * errnum is set to Z_ERRNO and the application may consult errno
         * to get the exact error code.
         */
        స్థిరరాశి అక్షరం* error(పూర్ణాంకం* errnum) {
            తిరిగి ::gzerror(m_fp, errnum);
        }

        gzFile fp() { తిరిగి m_fp; }

        ostream& os() {
            ఒకవేళ (m_os == 0) m_os = new ostrstream;
            తిరిగి *m_os;
        }

        శూన్య os_flush() {
            ఒకవేళ (m_os && m_os->pcount()>0) {
                ostrstream* oss = new ostrstream;
                oss->fill(m_os->fill());
                oss->flags(m_os->flags());
                oss->precision(m_os->precision());
                oss->వెడల్పు(m_os->వెడల్పు());
                ::gzwrite(m_fp, m_os->str(), m_os->pcount());
                delete[] m_os->str(); delete m_os; m_os = oss;
            }
        }

    అంతరంగ:
        gzFile m_fp;
        ostrstream* m_os;
};

/*
 * Binary write the given (array of) object(s) into the compressed file.
 * returns the number of uncompressed bytes actually written
 * (0 in case of error).
 */
మూస <class T, class వస్తువులు>
అంతఃవరుస పూర్ణాంకం లిఖ(ozstream& zs, స్థిరరాశి T* x, వస్తువులు వస్తువులు_2) {
    తిరిగి ::gzwrite(zs.fp(), (voidp) x, వస్తువులు_2*పరిమాణం(T));
}

/*
 * Binary output with the '<' operator.
 */
మూస <class T>
అంతఃవరుస ozstream& క్రియాకారకం<(ozstream& zs, స్థిరరాశి T& x) {
    ::gzwrite(zs.fp(), (voidp) &x, పరిమాణం(T));
    తిరిగి zs;
}

అంతఃవరుస zstringlen::zstringlen(ozstream& zs, స్థిరరాశి అక్షరం* x) {
    val.byte = 255;  val.మాట = ::అక్షరశ్రేణి_పొడవు(x);
    ఒకవేళ (val.మాట < 255) zs < (val.byte = val.మాట);
    లేకపోతే zs < val;
}

/*
 * Write length of string + the string with the '<' operator.
 */
అంతఃవరుస ozstream& క్రియాకారకం<(ozstream& zs, స్థిరరాశి అక్షరం* x) {
    zstringlen len(zs, x);
    ::gzwrite(zs.fp(), (voidp) x, len.విలువ());
    తిరిగి zs;
}

#నిర్వచితమైతే _MSC_VER
అంతఃవరుస ozstream& క్రియాకారకం<(ozstream& zs, అక్షరం* స్థిరరాశి& x) {
    తిరిగి zs < (స్థిరరాశి అక్షరం*) x;
}
#అయితే_ముగింపు

/*
 * Ascii write with the << operator;
 */
మూస <class T>
అంతఃవరుస ostream& క్రియాకారకం<<(ozstream& zs, స్థిరరాశి T& x) {
    zs.os_flush();
    తిరిగి zs.os() << x;
}

#అయితే_ముగింపు
