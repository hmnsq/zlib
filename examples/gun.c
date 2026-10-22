/* gun.c -- simple gunzip to give an example of the use of inflateBack()
 * Copyright (C) 2003, 2005, 2008, 2010, 2012 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
   Version 1.7  12 August 2012  Mark Adler */

/* Version history:
   1.0  16 Feb 2003  First version for testing of inflateBack()
   1.1  21 Feb 2005  Decompress concatenated gzip streams
                     Remove use of "this" variable (C++ keyword)
                     Fix return value for in()
                     Improve allocation failure checking
                     Add typecasting for void * structures
                     Add -h option for command version and usage
                     Add a bunch of comments
   1.2  20 Mar 2005  Add Unix compress (LZW) decompression
                     Copy file attributes from input file to output file
   1.3  12 Jun 2005  Add casts for error messages [Oberhumer]
   1.4   8 Dec 2006  LZW decompression speed improvements
   1.5   9 Feb 2008  Avoid warning in latest version of gcc
   1.6  17 Jan 2010  Avoid signed/unsigned comparison warnings
   1.7  12 Aug 2012  Update for z_const usage in zlib 1.2.8
 */

/*
   gun [ -t ] [ name ... ]

   decompresses the data in the named gzip files.  If no arguments are given,
   gun will decompress from stdin to stdout.  The names must end in .gz, -gz,
   .z, -z, _z, or .Z.  The uncompressed data will be written to a file name
   with the suffix stripped.  On success, the original file is deleted.  On
   failure, the output file is deleted.  For most failures, the command will
   continue to process the remaining names on the command line.  A memory
   allocation failure will abort the command.  If -t is specified, then the
   listed files or stdin will be tested as gzip files for integrity (without
   checking for a proper suffix), no output will be written, and no files
   will be deleted.

   Like gzip, gun allows concatenated gzip streams and will decompress them,
   writing all of the uncompressed data to the output.  Unlike gzip, gun allows
   an empty file on input, and will produce no error writing an empty output
   file.

   gun will also decompress files made by Unix compress, which uses LZW
   compression.  These files are automatically detected by virtue of their
   magic header bytes.  Since the end of Unix compress stream is marked by the
   end-of-file, they cannot be concatenated.  If a Unix compress stream is
   encountered in an input file, it is the last stream in that file.

   Like gunzip and uncompress, the file attributes of the original compressed
   file are maintained in the final uncompressed file, to the extent that the
   user permissions allow it.

   On my Mac OS X PowerPC G4, gun is almost twice as fast as gunzip (version
   1.2.4) is on the same file, when gun is linked with zlib 1.2.2.  Also the
   LZW decompression provided by gun is about twice as fast as the standard
   Unix uncompress command.
 */

/* external functions and related types and constants */
#अन्तर्भाव <stdio.h>          /* fprintf() */
#अन्तर्भाव <stdlib.h>         /* malloc(), free() */
#अन्तर्भाव <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#अन्तर्भाव <errno.h>          /* errno */
#अन्तर्भाव <fcntl.h>          /* open() */
#अन्तर्भाव <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>       /* stat(), chmod() */
#अन्तर्भाव <utime.h>          /* utime() */
#अन्तर्भाव "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#define local स्थिर

/* buffer constants */
#define SIZE 32768U         /* input and output buffer sizes */
#define PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
संरचना ind {
    पूर्णांक infile;
    अचिन्हित अक्षर *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local अचिन्हित in(शून्य *in_desc, z_const अचिन्हित अक्षर **buf)
{
    पूर्णांक ret;
    अचिन्हित len;
    अचिन्हित अक्षर *next;
    संरचना ind *me = (संरचना ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    करा {
        ret = PIECE;
        जर ((अचिन्हित)ret > SIZE - len)
            ret = (पूर्णांक)(SIZE - len);
        ret = (पूर्णांक)पठ(me->infile, next, ret);
        जर (ret == -1) {
            len = 0;
            थांबा;
        }
        next += ret;
        len += ret;
    } पर्यंत (ret != 0 && len < SIZE);
    परत len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
संरचना outd {
    पूर्णांक outfile;
    पूर्णांक check;                  /* true if checking crc and total */
    अचिन्हित दीर्घ crc;
    अचिन्हित दीर्घ total;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local पूर्णांक out(शून्य *out_desc, अचिन्हित अक्षर *buf, अचिन्हित len)
{
    पूर्णांक ret;
    संरचना outd *me = (संरचना outd *)out_desc;

    जर (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    जर (me->outfile != -1)
        करा {
            ret = PIECE;
            जर ((अचिन्हित)ret > len)
                ret = (पूर्णांक)len;
            ret = (पूर्णांक)लिख(me->outfile, buf, ret);
            जर (ret == -1)
                परत 1;
            buf += ret;
            len -= ret;
        } पर्यंत (len != 0);
    परत 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define NEXT() (have ? 0 : (have = in(indp, &next)), \
                last = have ? (have--, (पूर्णांक)(*next++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
अचिन्हित अक्षर inbuf[SIZE];              /* input buffer */
अचिन्हित अक्षर outbuf[SIZE];             /* output buffer */
अचिन्हित लघु prefix[65536];           /* index to LZW prefix string */
अचिन्हित अक्षर suffix[65536];            /* one-character LZW suffix */
अचिन्हित अक्षर match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE() \
    करा { \
        डावा = 0; \
        rem = 0; \
        जर (chunk > have) { \
            chunk -= have; \
            have = 0; \
            जर (NEXT() == -1) \
                थांबा; \
            chunk--; \
            जर (chunk > have) { \
                chunk = have = 0; \
                थांबा; \
            } \
        } \
        have -= chunk; \
        next += chunk; \
        chunk = 0; \
    } पर्यंत (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local पूर्णांक lunpipe(अचिन्हित have, z_const अचिन्हित अक्षर *next, संरचना ind *indp,
                  पूर्णांक outfile, z_stream *strm)
{
    पूर्णांक last;                   /* last byte read by NEXT(), or -1 if EOF */
    अचिन्हित chunk;             /* bytes left in current chunk */
    पूर्णांक डावा;                   /* bits left in rem */
    अचिन्हित rem;               /* unused bits from input */
    पूर्णांक bits;                   /* current bits per code */
    अचिन्हित code;              /* code, table traversal index */
    अचिन्हित mask;              /* mask for current bits codes */
    पूर्णांक max;                    /* maximum bits per code for this stream */
    अचिन्हित flags;             /* compress flags, then block compress flag */
    अचिन्हित end;               /* last valid entry in prefix/suffix tables */
    अचिन्हित temp;              /* current code */
    अचिन्हित prev;              /* previous code */
    अचिन्हित final;             /* last character written for previous code */
    अचिन्हित चळत;             /* next position for reversed string */
    अचिन्हित outcnt;            /* bytes in output buffer */
    संरचना outd outd;           /* output structure */
    अचिन्हित अक्षर *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = NEXT();
    जर (last == -1)
        परत Z_BUF_ERROR;
    जर (flags & 0x60) {
        strm->संदेश = (z_const अक्षर *)"unknown lzw flags set";
        परत Z_DATA_ERROR;
    }
    max = flags & 0x1f;
    जर (max < 9 || max > 16) {
        strm->संदेश = (z_const अक्षर *)"lzw bits out of range";
        परत Z_DATA_ERROR;
    }
    जर (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    जर (NEXT() == -1)                       /* no compressed data is ok */
        परत Z_OK;
    final = prev = (अचिन्हित)last;          /* low 8 bits of code */
    जर (NEXT() == -1)                       /* missing a bit */
        परत Z_BUF_ERROR;
    जर (last & 1) {                         /* code must be < 256 */
        strm->संदेश = (z_const अक्षर *)"invalid lzw code";
        परत Z_DATA_ERROR;
    }
    rem = (अचिन्हित)last >> 1;              /* remaining 7 bits */
    डावा = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (अचिन्हित अक्षर)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    चळत = 0;
    साठी (;;) {
        /* if the table will be full after this, increment the code size */
        जर (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        जर (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        जर (NEXT() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            जर (outcnt && out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf;     /* signal write error */
                परत Z_BUF_ERROR;
            }
            परत Z_OK;
        }
        code += (अचिन्हित)last << डावा;     /* middle (or high) bits of code */
        डावा += 8;
        chunk--;
        जर (bits > डावा) {                  /* need more bits */
            जर (NEXT() == -1)               /* can't end in middle of code */
                परत Z_BUF_ERROR;
            code += (अचिन्हित)last << डावा; /* high bits of code */
            डावा += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        डावा -= bits;                       /* number of unused bits */
        rem = (अचिन्हित)last >> (8 - डावा); /* unused bits from last byte */

        /* process clear code (256) */
        जर (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            सुरूठेवा;                       /* get next code */
        }

        /* special code to reuse last match */
        temp = code;                        /* save the current code */
        जर (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            जर (code != end + 1 || prev > end) {
                strm->संदेश = (z_const अक्षर *)"invalid lzw code";
                परत Z_DATA_ERROR;
            }
            match[चळत++] = (अचिन्हित अक्षर)final;
            code = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + चळत;
        पर्यंत (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        चळत = p - match;
        match[चळत++] = (अचिन्हित अक्षर)code;
        final = code;

        /* link new table entry */
        जर (end < mask) {
            end++;
            prefix[end] = (अचिन्हित लघु)prev;
            suffix[end] = (अचिन्हित अक्षर)final;
        }

        /* set previous code for next iteration */
        prev = temp;

        /* write output in forward order */
        पर्यंत (चळत > SIZE - outcnt) {
            पर्यंत (outcnt < SIZE)
                outbuf[outcnt++] = match[--चळत];
            जर (out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                परत Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + चळत;
        करा {
            outbuf[outcnt++] = *--p;
        } पर्यंत (p > match);
        चळत = 0;

        /* loop for next code with final and prev as the last match, rem and
           left provide the first 0..7 bits of the next code, end is the last
           valid table entry */
    }
}

/* Decompress a gzip file from infile to outfile.  strm is assumed to have been
   successfully initialized with inflateBackInit().  The input file may consist
   of a series of gzip streams, in which case all of them will be decompressed
   to the output file.  If outfile is -1, then the gzip stream(s) integrity is
   checked and nothing is written.

   The return value is a zlib error code: Z_MEM_ERROR if out of memory,
   Z_DATA_ERROR if the header or the compressed data is invalid, or if the
   trailer CRC-32 check or length doesn't match, Z_BUF_ERROR if the input ends
   prematurely or a write error occurs, or Z_ERRNO if junk (not a another gzip
   stream) follows a valid gzip stream.
 */
local पूर्णांक gunpipe(z_stream *strm, पूर्णांक infile, पूर्णांक outfile)
{
    पूर्णांक ret, first, last;
    अचिन्हित have, flags, len;
    z_const अचिन्हित अक्षर *next = रिक्त;
    संरचना ind ind, *indp;
    संरचना outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->next_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    साठी (;;) {
        /* look for the two magic header bytes for a gzip stream */
        जर (NEXT() == -1) {
            ret = Z_OK;
            थांबा;                          /* empty gzip stream is ok */
        }
        जर (last != 31 || (NEXT() != 139 && last != 157)) {
            strm->संदेश = (z_const अक्षर *)"incorrect header check";
            ret = first ? Z_DATA_ERROR : Z_ERRNO;
            थांबा;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        जर (last == 157) {
            ret = lunpipe(have, next, indp, outfile, strm);
            थांबा;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        जर (NEXT() != 8) {                  /* only deflate method allowed */
            जर (last == -1) थांबा;
            strm->संदेश = (z_const अक्षर *)"unknown compression method";
            ret = Z_DATA_ERROR;
            थांबा;
        }
        flags = NEXT();                     /* header flags */
        NEXT();                             /* discard mod time, xflgs, os */
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        जर (last == -1) थांबा;
        जर (flags & 0xe0) {
            strm->संदेश = (z_const अक्षर *)"unknown header flags set";
            ret = Z_DATA_ERROR;
            थांबा;
        }
        जर (flags & 4) {                    /* extra field */
            len = NEXT();
            len += (अचिन्हित)(NEXT()) << 8;
            जर (last == -1) थांबा;
            पर्यंत (len > have) {
                len -= have;
                have = 0;
                जर (NEXT() == -1) थांबा;
                len--;
            }
            जर (last == -1) थांबा;
            have -= len;
            next += len;
        }
        जर (flags & 8)                      /* file name */
            पर्यंत (NEXT() != 0 && last != -1)
                ;
        जर (flags & 16)                     /* comment */
            पर्यंत (NEXT() != 0 && last != -1)
                ;
        जर (flags & 2) {                    /* header crc */
            NEXT();
            NEXT();
        }
        जर (last == -1) थांबा;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.total = 0;

        /* decompress data to output */
        strm->next_in = next;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        जर (ret != Z_STREAM_END) थांबा;
        next = strm->next_in;
        have = strm->avail_in;
        strm->next_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        जर (NEXT() != (पूर्णांक)(outd.crc & 0xff) ||
            NEXT() != (पूर्णांक)((outd.crc >> 8) & 0xff) ||
            NEXT() != (पूर्णांक)((outd.crc >> 16) & 0xff) ||
            NEXT() != (पूर्णांक)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            जर (last != -1) {
                strm->संदेश = (z_const अक्षर *)"incorrect data check";
                ret = Z_DATA_ERROR;
            }
            थांबा;
        }
        जर (NEXT() != (पूर्णांक)(outd.total & 0xff) ||
            NEXT() != (पूर्णांक)((outd.total >> 8) & 0xff) ||
            NEXT() != (पूर्णांक)((outd.total >> 16) & 0xff) ||
            NEXT() != (पूर्णांक)((outd.total >> 24) & 0xff)) {
            /* length error */
            जर (last != -1) {
                strm->संदेश = (z_const अक्षर *)"incorrect length check";
                ret = Z_DATA_ERROR;
            }
            थांबा;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    परत ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local शून्य copymeta(अक्षर *from, अक्षर *to)
{
    संरचना stat was;
    संरचना utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    जर (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        परत;

    /* set to's mode bits, ignore errors */
    (शून्य)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (शून्य)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (शून्य)utime(to, &when);
}

/* Decompress the file inname to the file outnname, of if test is true, just
   decompress without writing and check the gzip trailer for integrity.  If
   inname is NULL or an empty string, read from stdin.  If outname is NULL or
   an empty string, write to stdout.  strm is a pre-initialized inflateBack
   structure.  When appropriate, copy the file attributes from inname to
   outname.

   gunzip() returns 1 if there is an out-of-memory error or an unexpected
   return code from gunpipe().  Otherwise it returns 0.
 */
local पूर्णांक gunzip(z_stream *strm, अक्षर *inname, अक्षर *outname, पूर्णांक test)
{
    पूर्णांक ret;
    पूर्णांक infile, outfile;

    /* open files */
    जर (inname == रिक्त || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    अन्यथा {
        infile = खोलो(inname, O_RDONLY, 0);
        जर (infile == -1) {
            ख_लिखो(मानक_त्रुटि, "gun cannot open %s\n", inname);
            परत 0;
        }
    }
    जर (test)
        outfile = -1;
    अन्यथा जर (outname == रिक्त || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    अन्यथा {
        outfile = खोलो(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        जर (outfile == -1) {
            बंद(infile);
            ख_लिखो(मानक_त्रुटि, "gun cannot create %s\n", outname);
            परत 0;
        }
    }
    त्रुटि_सं = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    जर (outfile > 2) बंद(outfile);
    जर (infile > 2) बंद(infile);

    /* interpret result */
    निवड (ret) {
    स्थिती Z_OK:
    स्थिती Z_ERRNO:
        जर (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        जर (ret == Z_ERRNO)
            ख_लिखो(मानक_त्रुटि, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        थांबा;
    स्थिती Z_DATA_ERROR:
        जर (outfile > 2) unlink(outname);
        ख_लिखो(मानक_त्रुटि, "gun data error on %s: %s\n", inname, strm->संदेश);
        थांबा;
    स्थिती Z_MEM_ERROR:
        जर (outfile > 2) unlink(outname);
        ख_लिखो(मानक_त्रुटि, "gun out of memory error--aborting\n");
        परत 1;
    स्थिती Z_BUF_ERROR:
        जर (outfile > 2) unlink(outname);
        जर (strm->next_in != Z_NULL) {
            ख_लिखो(मानक_त्रुटि, "gun write error on %s: %s\n",
                    outname, म_त्रुटि(त्रुटि_सं));
        }
        अन्यथा जर (त्रुटि_सं) {
            ख_लिखो(मानक_त्रुटि, "gun read error on %s: %s\n",
                    inname, म_त्रुटि(त्रुटि_सं));
        }
        अन्यथा {
            ख_लिखो(मानक_त्रुटि, "gun unexpected end of file on %s\n",
                    inname);
        }
        थांबा;
    पूर्वनिर्धारित:
        जर (outfile > 2) unlink(outname);
        ख_लिखो(मानक_त्रुटि, "gun internal error--aborting\n");
        परत 1;
    }
    परत 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
    पूर्णांक ret, len, test;
    अक्षर *outname;
    अचिन्हित अक्षर *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    जर (ret != Z_OK) {
        ख_लिखो(मानक_त्रुटि, "gun out of memory error--aborting\n");
        परत 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    जर (argc && म_भेद(*argv, "-h") == 0) {
        ख_लिखो(मानक_त्रुटि, "gun 1.6 (17 Jan 2010)\n");
        ख_लिखो(मानक_त्रुटि, "Copyright (C) 2003-2010 Mark Adler\n");
        ख_लिखो(मानक_त्रुटि, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        परत 0;
    }
    जर (argc && म_भेद(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    जर (argc)
        करा {
            जर (test)
                outname = रिक्त;
            अन्यथा {
                len = (पूर्णांक)म_माप(*argv);
                जर (म_भेद(*argv + len - 3, ".gz") == 0 ||
                    म_भेद(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                अन्यथा जर (म_भेद(*argv + len - 2, ".z") == 0 ||
                    म_भेद(*argv + len - 2, "-z") == 0 ||
                    म_भेद(*argv + len - 2, "_z") == 0 ||
                    म_भेद(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                अन्यथा {
                    ख_लिखो(मानक_त्रुटि, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    सुरूठेवा;
                }
                outname = दो_स्मृति(len + 1);
                जर (outname == रिक्त) {
                    ख_लिखो(मानक_त्रुटि, "gun out of memory error--aborting\n");
                    ret = 1;
                    थांबा;
                }
                स_नकल(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            जर (outname != रिक्त) मुक्त(outname);
            जर (ret) थांबा;
        } पर्यंत (argv++, --argc);
    अन्यथा
        ret = gunzip(&strm, रिक्त, रिक्त, test);

    /* clean up */
    inflateBackEnd(&strm);
    परत ret;
}
