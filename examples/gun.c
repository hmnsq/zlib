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
#समावेश <stdio.h>          /* fprintf() */
#समावेश <stdlib.h>         /* malloc(), free() */
#समावेश <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#समावेश <errno.h>          /* errno */
#समावेश <fcntl.h>          /* open() */
#समावेश <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#समावेश <sys/types.h>
#समावेश <sys/stat.h>       /* stat(), chmod() */
#समावेश <utime.h>          /* utime() */
#समावेश "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#घोषणा local अटल

/* buffer constants */
#घोषणा आकार_2 32768U         /* input and output buffer sizes */
#घोषणा PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
काष्ठा ind {
    पूर्णांक infile;
    अचिन्हित अक्षर *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local अचिन्हित in(व्योम *in_desc, z_const अचिन्हित अक्षर **buf)
{
    पूर्णांक ret;
    अचिन्हित len;
    अचिन्हित अक्षर *अग्र;
    काष्ठा ind *me = (काष्ठा ind *)in_desc;

    अग्र = me->inbuf;
    *buf = अग्र;
    len = 0;
    करो {
        ret = PIECE;
        अगर ((अचिन्हित)ret > आकार_2 - len)
            ret = (पूर्णांक)(आकार_2 - len);
        ret = (पूर्णांक)पढ़ो(me->infile, अग्र, ret);
        अगर (ret == -1) {
            len = 0;
            अवरोध;
        }
        अग्र += ret;
        len += ret;
    } जबतक (ret != 0 && len < आकार_2);
    वापस len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
काष्ठा outd {
    पूर्णांक outfile;
    पूर्णांक check;                  /* true if checking crc and total */
    अचिन्हित दीर्घ crc;
    अचिन्हित दीर्घ योग_2;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local पूर्णांक out(व्योम *out_desc, अचिन्हित अक्षर *buf, अचिन्हित len)
{
    पूर्णांक ret;
    काष्ठा outd *me = (काष्ठा outd *)out_desc;

    अगर (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->योग_2 += len;
    }
    अगर (me->outfile != -1)
        करो {
            ret = PIECE;
            अगर ((अचिन्हित)ret > len)
                ret = (पूर्णांक)len;
            ret = (पूर्णांक)ग_लिखो(me->outfile, buf, ret);
            अगर (ret == -1)
                वापस 1;
            buf += ret;
            len -= ret;
        } जबतक (len != 0);
    वापस 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#घोषणा अग्र_2() (have ? 0 : (have = in(indp, &अग्र)), \
                last = have ? (have--, (पूर्णांक)(*अग्र++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
अचिन्हित अक्षर inbuf[आकार_2];              /* input buffer */
अचिन्हित अक्षर outbuf[आकार_2];             /* output buffer */
अचिन्हित लघु prefix[65536];           /* index to LZW prefix string */
अचिन्हित अक्षर suffix[65536];            /* one-character LZW suffix */
अचिन्हित अक्षर match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#घोषणा FLUSHCODE() \
    करो { \
        बायाँ = 0; \
        rem = 0; \
        अगर (chunk > have) { \
            chunk -= have; \
            have = 0; \
            अगर (अग्र_2() == -1) \
                अवरोध; \
            chunk--; \
            अगर (chunk > have) { \
                chunk = have = 0; \
                अवरोध; \
            } \
        } \
        have -= chunk; \
        अग्र += chunk; \
        chunk = 0; \
    } जबतक (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local पूर्णांक lunpipe(अचिन्हित have, z_const अचिन्हित अक्षर *अग्र, काष्ठा ind *indp,
                  पूर्णांक outfile, z_stream *strm)
{
    पूर्णांक last;                   /* last byte read by NEXT(), or -1 if EOF */
    अचिन्हित chunk;             /* bytes left in current chunk */
    पूर्णांक बायाँ;                   /* bits left in rem */
    अचिन्हित rem;               /* unused bits from input */
    पूर्णांक bits;                   /* current bits per code */
    अचिन्हित code;              /* code, table traversal index */
    अचिन्हित mask;              /* mask for current bits codes */
    पूर्णांक max;                    /* maximum bits per code for this stream */
    अचिन्हित flags;             /* compress flags, then block compress flag */
    अचिन्हित end;               /* last valid entry in prefix/suffix tables */
    अचिन्हित अस्थायी;              /* current code */
    अचिन्हित पूर्व;              /* previous code */
    अचिन्हित final;             /* last character written for previous code */
    अचिन्हित ढेर_2;             /* next position for reversed string */
    अचिन्हित outcnt;            /* bytes in output buffer */
    काष्ठा outd outd;           /* output structure */
    अचिन्हित अक्षर *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = अग्र_2();
    अगर (last == -1)
        वापस Z_BUF_ERROR;
    अगर (flags & 0x60) {
        strm->संदेश = (z_const अक्षर *)"unknown lzw flags set";
        वापस Z_तथ्य_ERROR;
    }
    max = flags & 0x1f;
    अगर (max < 9 || max > 16) {
        strm->संदेश = (z_const अक्षर *)"lzw bits out of range";
        वापस Z_तथ्य_ERROR;
    }
    अगर (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    अगर (अग्र_2() == -1)                       /* no compressed data is ok */
        वापस Z_OK;
    final = पूर्व = (अचिन्हित)last;          /* low 8 bits of code */
    अगर (अग्र_2() == -1)                       /* missing a bit */
        वापस Z_BUF_ERROR;
    अगर (last & 1) {                         /* code must be < 256 */
        strm->संदेश = (z_const अक्षर *)"invalid lzw code";
        वापस Z_तथ्य_ERROR;
    }
    rem = (अचिन्हित)last >> 1;              /* remaining 7 bits */
    बायाँ = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (अचिन्हित अक्षर)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    ढेर_2 = 0;
    क्रम (;;) {
        /* if the table will be full after this, increment the code size */
        अगर (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        अगर (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        अगर (अग्र_2() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            अगर (outcnt && out(&outd, outbuf, outcnt)) {
                strm->अग्र_in = outbuf;     /* signal write error */
                वापस Z_BUF_ERROR;
            }
            वापस Z_OK;
        }
        code += (अचिन्हित)last << बायाँ;     /* middle (or high) bits of code */
        बायाँ += 8;
        chunk--;
        अगर (bits > बायाँ) {                  /* need more bits */
            अगर (अग्र_2() == -1)               /* can't end in middle of code */
                वापस Z_BUF_ERROR;
            code += (अचिन्हित)last << बायाँ; /* high bits of code */
            बायाँ += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        बायाँ -= bits;                       /* number of unused bits */
        rem = (अचिन्हित)last >> (8 - बायाँ); /* unused bits from last byte */

        /* process clear code (256) */
        अगर (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            जारी;                       /* get next code */
        }

        /* special code to reuse last match */
        अस्थायी = code;                        /* save the current code */
        अगर (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            अगर (code != end + 1 || पूर्व > end) {
                strm->संदेश = (z_const अक्षर *)"invalid lzw code";
                वापस Z_तथ्य_ERROR;
            }
            match[ढेर_2++] = (अचिन्हित अक्षर)final;
            code = पूर्व;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + ढेर_2;
        जबतक (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        ढेर_2 = p - match;
        match[ढेर_2++] = (अचिन्हित अक्षर)code;
        final = code;

        /* link new table entry */
        अगर (end < mask) {
            end++;
            prefix[end] = (अचिन्हित लघु)पूर्व;
            suffix[end] = (अचिन्हित अक्षर)final;
        }

        /* set previous code for next iteration */
        पूर्व = अस्थायी;

        /* write output in forward order */
        जबतक (ढेर_2 > आकार_2 - outcnt) {
            जबतक (outcnt < आकार_2)
                outbuf[outcnt++] = match[--ढेर_2];
            अगर (out(&outd, outbuf, outcnt)) {
                strm->अग्र_in = outbuf; /* signal write error */
                वापस Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + ढेर_2;
        करो {
            outbuf[outcnt++] = *--p;
        } जबतक (p > match);
        ढेर_2 = 0;

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
    z_const अचिन्हित अक्षर *अग्र = शून्य;
    काष्ठा ind ind, *indp;
    काष्ठा outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->अग्र_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    क्रम (;;) {
        /* look for the two magic header bytes for a gzip stream */
        अगर (अग्र_2() == -1) {
            ret = Z_OK;
            अवरोध;                          /* empty gzip stream is ok */
        }
        अगर (last != 31 || (अग्र_2() != 139 && last != 157)) {
            strm->संदेश = (z_const अक्षर *)"incorrect header check";
            ret = first ? Z_तथ्य_ERROR : Z_ERRNO;
            अवरोध;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        अगर (last == 157) {
            ret = lunpipe(have, अग्र, indp, outfile, strm);
            अवरोध;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        अगर (अग्र_2() != 8) {                  /* only deflate method allowed */
            अगर (last == -1) अवरोध;
            strm->संदेश = (z_const अक्षर *)"unknown compression method";
            ret = Z_तथ्य_ERROR;
            अवरोध;
        }
        flags = अग्र_2();                     /* header flags */
        अग्र_2();                             /* discard mod time, xflgs, os */
        अग्र_2();
        अग्र_2();
        अग्र_2();
        अग्र_2();
        अग्र_2();
        अगर (last == -1) अवरोध;
        अगर (flags & 0xe0) {
            strm->संदेश = (z_const अक्षर *)"unknown header flags set";
            ret = Z_तथ्य_ERROR;
            अवरोध;
        }
        अगर (flags & 4) {                    /* extra field */
            len = अग्र_2();
            len += (अचिन्हित)(अग्र_2()) << 8;
            अगर (last == -1) अवरोध;
            जबतक (len > have) {
                len -= have;
                have = 0;
                अगर (अग्र_2() == -1) अवरोध;
                len--;
            }
            अगर (last == -1) अवरोध;
            have -= len;
            अग्र += len;
        }
        अगर (flags & 8)                      /* file name */
            जबतक (अग्र_2() != 0 && last != -1)
                ;
        अगर (flags & 16)                     /* comment */
            जबतक (अग्र_2() != 0 && last != -1)
                ;
        अगर (flags & 2) {                    /* header crc */
            अग्र_2();
            अग्र_2();
        }
        अगर (last == -1) अवरोध;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.योग_2 = 0;

        /* decompress data to output */
        strm->अग्र_in = अग्र;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        अगर (ret != Z_STREAM_END) अवरोध;
        अग्र = strm->अग्र_in;
        have = strm->avail_in;
        strm->अग्र_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        अगर (अग्र_2() != (पूर्णांक)(outd.crc & 0xff) ||
            अग्र_2() != (पूर्णांक)((outd.crc >> 8) & 0xff) ||
            अग्र_2() != (पूर्णांक)((outd.crc >> 16) & 0xff) ||
            अग्र_2() != (पूर्णांक)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            अगर (last != -1) {
                strm->संदेश = (z_const अक्षर *)"incorrect data check";
                ret = Z_तथ्य_ERROR;
            }
            अवरोध;
        }
        अगर (अग्र_2() != (पूर्णांक)(outd.योग_2 & 0xff) ||
            अग्र_2() != (पूर्णांक)((outd.योग_2 >> 8) & 0xff) ||
            अग्र_2() != (पूर्णांक)((outd.योग_2 >> 16) & 0xff) ||
            अग्र_2() != (पूर्णांक)((outd.योग_2 >> 24) & 0xff)) {
            /* length error */
            अगर (last != -1) {
                strm->संदेश = (z_const अक्षर *)"incorrect length check";
                ret = Z_तथ्य_ERROR;
            }
            अवरोध;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    वापस ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local व्योम copymeta(अक्षर *from, अक्षर *to)
{
    काष्ठा stat was;
    काष्ठा utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    अगर (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        वापस;

    /* set to's mode bits, ignore errors */
    (व्योम)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (व्योम)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (व्योम)utime(to, &when);
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
    अगर (inname == शून्य || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    अन्यथा {
        infile = खोलो(inname, O_RDONLY, 0);
        अगर (infile == -1) {
            ख_लिखो(मानक_त्रुटि, "gun cannot open %s\n", inname);
            वापस 0;
        }
    }
    अगर (test)
        outfile = -1;
    अन्यथा अगर (outname == शून्य || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    अन्यथा {
        outfile = खोलो(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        अगर (outfile == -1) {
            बंद(infile);
            ख_लिखो(मानक_त्रुटि, "gun cannot create %s\n", outname);
            वापस 0;
        }
    }
    त्रुटि_सं = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    अगर (outfile > 2) बंद(outfile);
    अगर (infile > 2) बंद(infile);

    /* interpret result */
    चयन (ret) {
    हाल Z_OK:
    हाल Z_ERRNO:
        अगर (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        अगर (ret == Z_ERRNO)
            ख_लिखो(मानक_त्रुटि, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        अवरोध;
    हाल Z_तथ्य_ERROR:
        अगर (outfile > 2) unlink(outname);
        ख_लिखो(मानक_त्रुटि, "gun data error on %s: %s\n", inname, strm->संदेश);
        अवरोध;
    हाल Z_MEM_ERROR:
        अगर (outfile > 2) unlink(outname);
        ख_लिखो(मानक_त्रुटि, "gun out of memory error--aborting\n");
        वापस 1;
    हाल Z_BUF_ERROR:
        अगर (outfile > 2) unlink(outname);
        अगर (strm->अग्र_in != Z_NULL) {
            ख_लिखो(मानक_त्रुटि, "gun write error on %s: %s\n",
                    outname, म_त्रुटि(त्रुटि_सं));
        }
        अन्यथा अगर (त्रुटि_सं) {
            ख_लिखो(मानक_त्रुटि, "gun read error on %s: %s\n",
                    inname, म_त्रुटि(त्रुटि_सं));
        }
        अन्यथा {
            ख_लिखो(मानक_त्रुटि, "gun unexpected end of file on %s\n",
                    inname);
        }
        अवरोध;
    शेष:
        अगर (outfile > 2) unlink(outname);
        ख_लिखो(मानक_त्रुटि, "gun internal error--aborting\n");
        वापस 1;
    }
    वापस 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
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
    अगर (ret != Z_OK) {
        ख_लिखो(मानक_त्रुटि, "gun out of memory error--aborting\n");
        वापस 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    अगर (argc && म_भेद(*argv, "-h") == 0) {
        ख_लिखो(मानक_त्रुटि, "gun 1.6 (17 Jan 2010)\n");
        ख_लिखो(मानक_त्रुटि, "Copyright (C) 2003-2010 Mark Adler\n");
        ख_लिखो(मानक_त्रुटि, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        वापस 0;
    }
    अगर (argc && म_भेद(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    अगर (argc)
        करो {
            अगर (test)
                outname = शून्य;
            अन्यथा {
                len = (पूर्णांक)म_माप(*argv);
                अगर (म_भेद(*argv + len - 3, ".gz") == 0 ||
                    म_भेद(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                अन्यथा अगर (म_भेद(*argv + len - 2, ".z") == 0 ||
                    म_भेद(*argv + len - 2, "-z") == 0 ||
                    म_भेद(*argv + len - 2, "_z") == 0 ||
                    म_भेद(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                अन्यथा {
                    ख_लिखो(मानक_त्रुटि, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    जारी;
                }
                outname = दो_स्मृति(len + 1);
                अगर (outname == शून्य) {
                    ख_लिखो(मानक_त्रुटि, "gun out of memory error--aborting\n");
                    ret = 1;
                    अवरोध;
                }
                स_नकल(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            अगर (outname != शून्य) मुक्त(outname);
            अगर (ret) अवरोध;
        } जबतक (argv++, --argc);
    अन्यथा
        ret = gunzip(&strm, शून्य, शून्य, test);

    /* clean up */
    inflateBackEnd(&strm);
    वापस ret;
}
