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
#અન્તર્ભાવ <stdio.h>          /* fprintf() */
#અન્તર્ભાવ <stdlib.h>         /* malloc(), free() */
#અન્તર્ભાવ <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#અન્તર્ભાવ <errno.h>          /* errno */
#અન્તર્ભાવ <fcntl.h>          /* open() */
#અન્તર્ભાવ <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>       /* stat(), chmod() */
#અન્તર્ભાવ <utime.h>          /* utime() */
#અન્તર્ભાવ "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#define local સ્થિર

/* buffer constants */
#define SIZE 32768U         /* input and output buffer sizes */
#define PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
સંરચના ind {
    પૂર્ણાંક infile;
    unsigned અક્ષર *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local unsigned in(શૂન્ય *in_desc, z_const unsigned અક્ષર **buf)
{
    પૂર્ણાંક ret;
    unsigned len;
    unsigned અક્ષર *next;
    સંરચના ind *me = (સંરચના ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    કરો {
        ret = PIECE;
        જો ((unsigned)ret > SIZE - len)
            ret = (પૂર્ણાંક)(SIZE - len);
        ret = (પૂર્ણાંક)પઠ(me->infile, next, ret);
        જો (ret == -1) {
            len = 0;
            તોડો;
        }
        next += ret;
        len += ret;
    } જ્યારે (ret != 0 && len < SIZE);
    પાછા len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
સંરચના outd {
    પૂર્ણાંક outfile;
    પૂર્ણાંક check;                  /* true if checking crc and total */
    unsigned long crc;
    unsigned long total;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local પૂર્ણાંક out(શૂન્ય *out_desc, unsigned અક્ષર *buf, unsigned len)
{
    પૂર્ણાંક ret;
    સંરચના outd *me = (સંરચના outd *)out_desc;

    જો (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    જો (me->outfile != -1)
        કરો {
            ret = PIECE;
            જો ((unsigned)ret > len)
                ret = (પૂર્ણાંક)len;
            ret = (પૂર્ણાંક)લિખ(me->outfile, buf, ret);
            જો (ret == -1)
                પાછા 1;
            buf += ret;
            len -= ret;
        } જ્યારે (len != 0);
    પાછા 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define NEXT() (have ? 0 : (have = in(indp, &next)), \
                last = have ? (have--, (પૂર્ણાંક)(*next++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
unsigned અક્ષર inbuf[SIZE];              /* input buffer */
unsigned અક્ષર outbuf[SIZE];             /* output buffer */
unsigned short prefix[65536];           /* index to LZW prefix string */
unsigned અક્ષર suffix[65536];            /* one-character LZW suffix */
unsigned અક્ષર match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE() \
    કરો { \
        ડાબું = 0; \
        rem = 0; \
        જો (chunk > have) { \
            chunk -= have; \
            have = 0; \
            જો (NEXT() == -1) \
                તોડો; \
            chunk--; \
            જો (chunk > have) { \
                chunk = have = 0; \
                તોડો; \
            } \
        } \
        have -= chunk; \
        next += chunk; \
        chunk = 0; \
    } જ્યારે (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local પૂર્ણાંક lunpipe(unsigned have, z_const unsigned અક્ષર *next, સંરચના ind *indp,
                  પૂર્ણાંક outfile, z_stream *strm)
{
    પૂર્ણાંક last;                   /* last byte read by NEXT(), or -1 if EOF */
    unsigned chunk;             /* bytes left in current chunk */
    પૂર્ણાંક ડાબું;                   /* bits left in rem */
    unsigned rem;               /* unused bits from input */
    પૂર્ણાંક bits;                   /* current bits per code */
    unsigned code;              /* code, table traversal index */
    unsigned mask;              /* mask for current bits codes */
    પૂર્ણાંક max;                    /* maximum bits per code for this stream */
    unsigned flags;             /* compress flags, then block compress flag */
    unsigned end;               /* last valid entry in prefix/suffix tables */
    unsigned temp;              /* current code */
    unsigned prev;              /* previous code */
    unsigned final;             /* last character written for previous code */
    unsigned ઢગલો;             /* next position for reversed string */
    unsigned outcnt;            /* bytes in output buffer */
    સંરચના outd outd;           /* output structure */
    unsigned અક્ષર *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = NEXT();
    જો (last == -1)
        પાછા Z_BUF_ERROR;
    જો (flags & 0x60) {
        strm->સંદેશ = (z_const અક્ષર *)"unknown lzw flags set";
        પાછા Z_DATA_ERROR;
    }
    max = flags & 0x1f;
    જો (max < 9 || max > 16) {
        strm->સંદેશ = (z_const અક્ષર *)"lzw bits out of range";
        પાછા Z_DATA_ERROR;
    }
    જો (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    જો (NEXT() == -1)                       /* no compressed data is ok */
        પાછા Z_OK;
    final = prev = (unsigned)last;          /* low 8 bits of code */
    જો (NEXT() == -1)                       /* missing a bit */
        પાછા Z_BUF_ERROR;
    જો (last & 1) {                         /* code must be < 256 */
        strm->સંદેશ = (z_const અક્ષર *)"invalid lzw code";
        પાછા Z_DATA_ERROR;
    }
    rem = (unsigned)last >> 1;              /* remaining 7 bits */
    ડાબું = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (unsigned અક્ષર)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    ઢગલો = 0;
    માટે (;;) {
        /* if the table will be full after this, increment the code size */
        જો (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        જો (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        જો (NEXT() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            જો (outcnt && out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf;     /* signal write error */
                પાછા Z_BUF_ERROR;
            }
            પાછા Z_OK;
        }
        code += (unsigned)last << ડાબું;     /* middle (or high) bits of code */
        ડાબું += 8;
        chunk--;
        જો (bits > ડાબું) {                  /* need more bits */
            જો (NEXT() == -1)               /* can't end in middle of code */
                પાછા Z_BUF_ERROR;
            code += (unsigned)last << ડાબું; /* high bits of code */
            ડાબું += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        ડાબું -= bits;                       /* number of unused bits */
        rem = (unsigned)last >> (8 - ડાબું); /* unused bits from last byte */

        /* process clear code (256) */
        જો (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            ચાલુ;                       /* get next code */
        }

        /* special code to reuse last match */
        temp = code;                        /* save the current code */
        જો (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            જો (code != end + 1 || prev > end) {
                strm->સંદેશ = (z_const અક્ષર *)"invalid lzw code";
                પાછા Z_DATA_ERROR;
            }
            match[ઢગલો++] = (unsigned અક્ષર)final;
            code = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + ઢગલો;
        જ્યારે (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        ઢગલો = p - match;
        match[ઢગલો++] = (unsigned અક્ષર)code;
        final = code;

        /* link new table entry */
        જો (end < mask) {
            end++;
            prefix[end] = (unsigned short)prev;
            suffix[end] = (unsigned અક્ષર)final;
        }

        /* set previous code for next iteration */
        prev = temp;

        /* write output in forward order */
        જ્યારે (ઢગલો > SIZE - outcnt) {
            જ્યારે (outcnt < SIZE)
                outbuf[outcnt++] = match[--ઢગલો];
            જો (out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                પાછા Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + ઢગલો;
        કરો {
            outbuf[outcnt++] = *--p;
        } જ્યારે (p > match);
        ઢગલો = 0;

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
local પૂર્ણાંક gunpipe(z_stream *strm, પૂર્ણાંક infile, પૂર્ણાંક outfile)
{
    પૂર્ણાંક ret, first, last;
    unsigned have, flags, len;
    z_const unsigned અક્ષર *next = ખાલી;
    સંરચના ind ind, *indp;
    સંરચના outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->next_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    માટે (;;) {
        /* look for the two magic header bytes for a gzip stream */
        જો (NEXT() == -1) {
            ret = Z_OK;
            તોડો;                          /* empty gzip stream is ok */
        }
        જો (last != 31 || (NEXT() != 139 && last != 157)) {
            strm->સંદેશ = (z_const અક્ષર *)"incorrect header check";
            ret = first ? Z_DATA_ERROR : Z_ERRNO;
            તોડો;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        જો (last == 157) {
            ret = lunpipe(have, next, indp, outfile, strm);
            તોડો;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        જો (NEXT() != 8) {                  /* only deflate method allowed */
            જો (last == -1) તોડો;
            strm->સંદેશ = (z_const અક્ષર *)"unknown compression method";
            ret = Z_DATA_ERROR;
            તોડો;
        }
        flags = NEXT();                     /* header flags */
        NEXT();                             /* discard mod time, xflgs, os */
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        જો (last == -1) તોડો;
        જો (flags & 0xe0) {
            strm->સંદેશ = (z_const અક્ષર *)"unknown header flags set";
            ret = Z_DATA_ERROR;
            તોડો;
        }
        જો (flags & 4) {                    /* extra field */
            len = NEXT();
            len += (unsigned)(NEXT()) << 8;
            જો (last == -1) તોડો;
            જ્યારે (len > have) {
                len -= have;
                have = 0;
                જો (NEXT() == -1) તોડો;
                len--;
            }
            જો (last == -1) તોડો;
            have -= len;
            next += len;
        }
        જો (flags & 8)                      /* file name */
            જ્યારે (NEXT() != 0 && last != -1)
                ;
        જો (flags & 16)                     /* comment */
            જ્યારે (NEXT() != 0 && last != -1)
                ;
        જો (flags & 2) {                    /* header crc */
            NEXT();
            NEXT();
        }
        જો (last == -1) તોડો;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.total = 0;

        /* decompress data to output */
        strm->next_in = next;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        જો (ret != Z_STREAM_END) તોડો;
        next = strm->next_in;
        have = strm->avail_in;
        strm->next_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        જો (NEXT() != (પૂર્ણાંક)(outd.crc & 0xff) ||
            NEXT() != (પૂર્ણાંક)((outd.crc >> 8) & 0xff) ||
            NEXT() != (પૂર્ણાંક)((outd.crc >> 16) & 0xff) ||
            NEXT() != (પૂર્ણાંક)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            જો (last != -1) {
                strm->સંદેશ = (z_const અક્ષર *)"incorrect data check";
                ret = Z_DATA_ERROR;
            }
            તોડો;
        }
        જો (NEXT() != (પૂર્ણાંક)(outd.total & 0xff) ||
            NEXT() != (પૂર્ણાંક)((outd.total >> 8) & 0xff) ||
            NEXT() != (પૂર્ણાંક)((outd.total >> 16) & 0xff) ||
            NEXT() != (પૂર્ણાંક)((outd.total >> 24) & 0xff)) {
            /* length error */
            જો (last != -1) {
                strm->સંદેશ = (z_const અક્ષર *)"incorrect length check";
                ret = Z_DATA_ERROR;
            }
            તોડો;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    પાછા ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local શૂન્ય copymeta(અક્ષર *from, અક્ષર *to)
{
    સંરચના stat was;
    સંરચના utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    જો (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        પાછા;

    /* set to's mode bits, ignore errors */
    (શૂન્ય)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (શૂન્ય)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (શૂન્ય)utime(to, &when);
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
local પૂર્ણાંક gunzip(z_stream *strm, અક્ષર *inname, અક્ષર *outname, પૂર્ણાંક test)
{
    પૂર્ણાંક ret;
    પૂર્ણાંક infile, outfile;

    /* open files */
    જો (inname == ખાલી || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    નહીંતર {
        infile = open(inname, O_RDONLY, 0);
        જો (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            પાછા 0;
        }
    }
    જો (test)
        outfile = -1;
    નહીંતર જો (outname == ખાલી || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    નહીંતર {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        જો (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            પાછા 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    જો (outfile > 2) close(outfile);
    જો (infile > 2) close(infile);

    /* interpret result */
    પસંદગી (ret) {
    સ્થિતિ Z_OK:
    સ્થિતિ Z_ERRNO:
        જો (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        જો (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        તોડો;
    સ્થિતિ Z_DATA_ERROR:
        જો (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->સંદેશ);
        તોડો;
    સ્થિતિ Z_MEM_ERROR:
        જો (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        પાછા 1;
    સ્થિતિ Z_BUF_ERROR:
        જો (outfile > 2) unlink(outname);
        જો (strm->next_in != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        નહીંતર જો (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        નહીંતર {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        તોડો;
    મૂળભૂત:
        જો (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        પાછા 1;
    }
    પાછા 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
    પૂર્ણાંક ret, len, test;
    અક્ષર *outname;
    unsigned અક્ષર *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    જો (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        પાછા 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    જો (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        પાછા 0;
    }
    જો (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    જો (argc)
        કરો {
            જો (test)
                outname = ખાલી;
            નહીંતર {
                len = (પૂર્ણાંક)strlen(*argv);
                જો (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                નહીંતર જો (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                નહીંતર {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    ચાલુ;
                }
                outname = malloc(len + 1);
                જો (outname == ખાલી) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    તોડો;
                }
                memcpy(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            જો (outname != ખાલી) મુક્ત(outname);
            જો (ret) તોડો;
        } જ્યારે (argv++, --argc);
    નહીંતર
        ret = gunzip(&strm, ખાલી, ખાલી, test);

    /* clean up */
    inflateBackEnd(&strm);
    પાછા ret;
}
