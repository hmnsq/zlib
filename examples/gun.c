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
#অন্তর্ভাব <stdio.h>          /* fprintf() */
#অন্তর্ভাব <stdlib.h>         /* malloc(), free() */
#অন্তর্ভাব <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#অন্তর্ভাব <errno.h>          /* errno */
#অন্তর্ভাব <fcntl.h>          /* open() */
#অন্তর্ভাব <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>       /* stat(), chmod() */
#অন্তর্ভাব <utime.h>          /* utime() */
#অন্তর্ভাব "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#define local স্থির

/* buffer constants */
#define আকার_3 32768U         /* input and output buffer sizes */
#define PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
কাঠামো ind {
    পূর্ণসংখ্যা infile;
    unsigned অক্ষর *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local unsigned in(শূন্য *in_desc, z_const unsigned অক্ষর **buf)
{
    পূর্ণসংখ্যা ret;
    unsigned len;
    unsigned অক্ষর *পরবর্তী;
    কাঠামো ind *me = (কাঠামো ind *)in_desc;

    পরবর্তী = me->inbuf;
    *buf = পরবর্তী;
    len = 0;
    করো {
        ret = PIECE;
        যদি ((unsigned)ret > আকার_3 - len)
            ret = (পূর্ণসংখ্যা)(আকার_3 - len);
        ret = (পূর্ণসংখ্যা)পঠ(me->infile, পরবর্তী, ret);
        যদি (ret == -1) {
            len = 0;
            বিরতি;
        }
        পরবর্তী += ret;
        len += ret;
    } যখন (ret != 0 && len < আকার_3);
    ফেরত len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
কাঠামো outd {
    পূর্ণসংখ্যা outfile;
    পূর্ণসংখ্যা check;                  /* true if checking crc and total */
    unsigned long crc;
    unsigned long যোগ_2;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local পূর্ণসংখ্যা out(শূন্য *out_desc, unsigned অক্ষর *buf, unsigned len)
{
    পূর্ণসংখ্যা ret;
    কাঠামো outd *me = (কাঠামো outd *)out_desc;

    যদি (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->যোগ_2 += len;
    }
    যদি (me->outfile != -1)
        করো {
            ret = PIECE;
            যদি ((unsigned)ret > len)
                ret = (পূর্ণসংখ্যা)len;
            ret = (পূর্ণসংখ্যা)লিখ(me->outfile, buf, ret);
            যদি (ret == -1)
                ফেরত 1;
            buf += ret;
            len -= ret;
        } যখন (len != 0);
    ফেরত 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define পরবর্তী_2() (have ? 0 : (have = in(indp, &পরবর্তী)), \
                last = have ? (have--, (পূর্ণসংখ্যা)(*পরবর্তী++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
unsigned অক্ষর inbuf[আকার_3];              /* input buffer */
unsigned অক্ষর outbuf[আকার_3];             /* output buffer */
unsigned short prefix[65536];           /* index to LZW prefix string */
unsigned অক্ষর suffix[65536];            /* one-character LZW suffix */
unsigned অক্ষর match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE() \
    করো { \
        বাম = 0; \
        rem = 0; \
        যদি (chunk > have) { \
            chunk -= have; \
            have = 0; \
            যদি (পরবর্তী_2() == -1) \
                বিরতি; \
            chunk--; \
            যদি (chunk > have) { \
                chunk = have = 0; \
                বিরতি; \
            } \
        } \
        have -= chunk; \
        পরবর্তী += chunk; \
        chunk = 0; \
    } যখন (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local পূর্ণসংখ্যা lunpipe(unsigned have, z_const unsigned অক্ষর *পরবর্তী, কাঠামো ind *indp,
                  পূর্ণসংখ্যা outfile, z_stream *strm)
{
    পূর্ণসংখ্যা last;                   /* last byte read by NEXT(), or -1 if EOF */
    unsigned chunk;             /* bytes left in current chunk */
    পূর্ণসংখ্যা বাম;                   /* bits left in rem */
    unsigned rem;               /* unused bits from input */
    পূর্ণসংখ্যা bits;                   /* current bits per code */
    unsigned code;              /* code, table traversal index */
    unsigned mask;              /* mask for current bits codes */
    পূর্ণসংখ্যা max;                    /* maximum bits per code for this stream */
    unsigned flags;             /* compress flags, then block compress flag */
    unsigned end;               /* last valid entry in prefix/suffix tables */
    unsigned অস্থায়ী;              /* current code */
    unsigned পূর্ব;              /* previous code */
    unsigned final;             /* last character written for previous code */
    unsigned স্তূপ;             /* next position for reversed string */
    unsigned outcnt;            /* bytes in output buffer */
    কাঠামো outd outd;           /* output structure */
    unsigned অক্ষর *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = পরবর্তী_2();
    যদি (last == -1)
        ফেরত Z_BUF_ERROR;
    যদি (flags & 0x60) {
        strm->বার্তা = (z_const অক্ষর *)"unknown lzw flags set";
        ফেরত Z_তথ্য_ERROR;
    }
    max = flags & 0x1f;
    যদি (max < 9 || max > 16) {
        strm->বার্তা = (z_const অক্ষর *)"lzw bits out of range";
        ফেরত Z_তথ্য_ERROR;
    }
    যদি (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    যদি (পরবর্তী_2() == -1)                       /* no compressed data is ok */
        ফেরত Z_OK;
    final = পূর্ব = (unsigned)last;          /* low 8 bits of code */
    যদি (পরবর্তী_2() == -1)                       /* missing a bit */
        ফেরত Z_BUF_ERROR;
    যদি (last & 1) {                         /* code must be < 256 */
        strm->বার্তা = (z_const অক্ষর *)"invalid lzw code";
        ফেরত Z_তথ্য_ERROR;
    }
    rem = (unsigned)last >> 1;              /* remaining 7 bits */
    বাম = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (unsigned অক্ষর)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    স্তূপ = 0;
    জন্য (;;) {
        /* if the table will be full after this, increment the code size */
        যদি (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        যদি (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        যদি (পরবর্তী_2() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            যদি (outcnt && out(&outd, outbuf, outcnt)) {
                strm->পরবর্তী_in = outbuf;     /* signal write error */
                ফেরত Z_BUF_ERROR;
            }
            ফেরত Z_OK;
        }
        code += (unsigned)last << বাম;     /* middle (or high) bits of code */
        বাম += 8;
        chunk--;
        যদি (bits > বাম) {                  /* need more bits */
            যদি (পরবর্তী_2() == -1)               /* can't end in middle of code */
                ফেরত Z_BUF_ERROR;
            code += (unsigned)last << বাম; /* high bits of code */
            বাম += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        বাম -= bits;                       /* number of unused bits */
        rem = (unsigned)last >> (8 - বাম); /* unused bits from last byte */

        /* process clear code (256) */
        যদি (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            চালু;                       /* get next code */
        }

        /* special code to reuse last match */
        অস্থায়ী = code;                        /* save the current code */
        যদি (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            যদি (code != end + 1 || পূর্ব > end) {
                strm->বার্তা = (z_const অক্ষর *)"invalid lzw code";
                ফেরত Z_তথ্য_ERROR;
            }
            match[স্তূপ++] = (unsigned অক্ষর)final;
            code = পূর্ব;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + স্তূপ;
        যখন (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        স্তূপ = p - match;
        match[স্তূপ++] = (unsigned অক্ষর)code;
        final = code;

        /* link new table entry */
        যদি (end < mask) {
            end++;
            prefix[end] = (unsigned short)পূর্ব;
            suffix[end] = (unsigned অক্ষর)final;
        }

        /* set previous code for next iteration */
        পূর্ব = অস্থায়ী;

        /* write output in forward order */
        যখন (স্তূপ > আকার_3 - outcnt) {
            যখন (outcnt < আকার_3)
                outbuf[outcnt++] = match[--স্তূপ];
            যদি (out(&outd, outbuf, outcnt)) {
                strm->পরবর্তী_in = outbuf; /* signal write error */
                ফেরত Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + স্তূপ;
        করো {
            outbuf[outcnt++] = *--p;
        } যখন (p > match);
        স্তূপ = 0;

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
local পূর্ণসংখ্যা gunpipe(z_stream *strm, পূর্ণসংখ্যা infile, পূর্ণসংখ্যা outfile)
{
    পূর্ণসংখ্যা ret, first, last;
    unsigned have, flags, len;
    z_const unsigned অক্ষর *পরবর্তী = ফাঁকা;
    কাঠামো ind ind, *indp;
    কাঠামো outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->পরবর্তী_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    জন্য (;;) {
        /* look for the two magic header bytes for a gzip stream */
        যদি (পরবর্তী_2() == -1) {
            ret = Z_OK;
            বিরতি;                          /* empty gzip stream is ok */
        }
        যদি (last != 31 || (পরবর্তী_2() != 139 && last != 157)) {
            strm->বার্তা = (z_const অক্ষর *)"incorrect header check";
            ret = first ? Z_তথ্য_ERROR : Z_ERRNO;
            বিরতি;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        যদি (last == 157) {
            ret = lunpipe(have, পরবর্তী, indp, outfile, strm);
            বিরতি;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        যদি (পরবর্তী_2() != 8) {                  /* only deflate method allowed */
            যদি (last == -1) বিরতি;
            strm->বার্তা = (z_const অক্ষর *)"unknown compression method";
            ret = Z_তথ্য_ERROR;
            বিরতি;
        }
        flags = পরবর্তী_2();                     /* header flags */
        পরবর্তী_2();                             /* discard mod time, xflgs, os */
        পরবর্তী_2();
        পরবর্তী_2();
        পরবর্তী_2();
        পরবর্তী_2();
        পরবর্তী_2();
        যদি (last == -1) বিরতি;
        যদি (flags & 0xe0) {
            strm->বার্তা = (z_const অক্ষর *)"unknown header flags set";
            ret = Z_তথ্য_ERROR;
            বিরতি;
        }
        যদি (flags & 4) {                    /* extra field */
            len = পরবর্তী_2();
            len += (unsigned)(পরবর্তী_2()) << 8;
            যদি (last == -1) বিরতি;
            যখন (len > have) {
                len -= have;
                have = 0;
                যদি (পরবর্তী_2() == -1) বিরতি;
                len--;
            }
            যদি (last == -1) বিরতি;
            have -= len;
            পরবর্তী += len;
        }
        যদি (flags & 8)                      /* file name */
            যখন (পরবর্তী_2() != 0 && last != -1)
                ;
        যদি (flags & 16)                     /* comment */
            যখন (পরবর্তী_2() != 0 && last != -1)
                ;
        যদি (flags & 2) {                    /* header crc */
            পরবর্তী_2();
            পরবর্তী_2();
        }
        যদি (last == -1) বিরতি;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.যোগ_2 = 0;

        /* decompress data to output */
        strm->পরবর্তী_in = পরবর্তী;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        যদি (ret != Z_STREAM_END) বিরতি;
        পরবর্তী = strm->পরবর্তী_in;
        have = strm->avail_in;
        strm->পরবর্তী_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        যদি (পরবর্তী_2() != (পূর্ণসংখ্যা)(outd.crc & 0xff) ||
            পরবর্তী_2() != (পূর্ণসংখ্যা)((outd.crc >> 8) & 0xff) ||
            পরবর্তী_2() != (পূর্ণসংখ্যা)((outd.crc >> 16) & 0xff) ||
            পরবর্তী_2() != (পূর্ণসংখ্যা)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            যদি (last != -1) {
                strm->বার্তা = (z_const অক্ষর *)"incorrect data check";
                ret = Z_তথ্য_ERROR;
            }
            বিরতি;
        }
        যদি (পরবর্তী_2() != (পূর্ণসংখ্যা)(outd.যোগ_2 & 0xff) ||
            পরবর্তী_2() != (পূর্ণসংখ্যা)((outd.যোগ_2 >> 8) & 0xff) ||
            পরবর্তী_2() != (পূর্ণসংখ্যা)((outd.যোগ_2 >> 16) & 0xff) ||
            পরবর্তী_2() != (পূর্ণসংখ্যা)((outd.যোগ_2 >> 24) & 0xff)) {
            /* length error */
            যদি (last != -1) {
                strm->বার্তা = (z_const অক্ষর *)"incorrect length check";
                ret = Z_তথ্য_ERROR;
            }
            বিরতি;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    ফেরত ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local শূন্য copymeta(অক্ষর *from, অক্ষর *to)
{
    কাঠামো stat was;
    কাঠামো utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    যদি (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        ফেরত;

    /* set to's mode bits, ignore errors */
    (শূন্য)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (শূন্য)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (শূন্য)utime(to, &when);
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
local পূর্ণসংখ্যা gunzip(z_stream *strm, অক্ষর *inname, অক্ষর *outname, পূর্ণসংখ্যা test)
{
    পূর্ণসংখ্যা ret;
    পূর্ণসংখ্যা infile, outfile;

    /* open files */
    যদি (inname == ফাঁকা || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    অন্যথা {
        infile = open(inname, O_RDONLY, 0);
        যদি (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            ফেরত 0;
        }
    }
    যদি (test)
        outfile = -1;
    অন্যথা যদি (outname == ফাঁকা || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    অন্যথা {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        যদি (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            ফেরত 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    যদি (outfile > 2) close(outfile);
    যদি (infile > 2) close(infile);

    /* interpret result */
    নির্বাচন (ret) {
    ক্ষেত্র Z_OK:
    ক্ষেত্র Z_ERRNO:
        যদি (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        যদি (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        বিরতি;
    ক্ষেত্র Z_তথ্য_ERROR:
        যদি (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->বার্তা);
        বিরতি;
    ক্ষেত্র Z_MEM_ERROR:
        যদি (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        ফেরত 1;
    ক্ষেত্র Z_BUF_ERROR:
        যদি (outfile > 2) unlink(outname);
        যদি (strm->পরবর্তী_in != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        অন্যথা যদি (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        অন্যথা {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        বিরতি;
    পূর্বনির্ধারিত:
        যদি (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        ফেরত 1;
    }
    ফেরত 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
    পূর্ণসংখ্যা ret, len, test;
    অক্ষর *outname;
    unsigned অক্ষর *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    যদি (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        ফেরত 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    যদি (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        ফেরত 0;
    }
    যদি (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    যদি (argc)
        করো {
            যদি (test)
                outname = ফাঁকা;
            অন্যথা {
                len = (পূর্ণসংখ্যা)strlen(*argv);
                যদি (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                অন্যথা যদি (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                অন্যথা {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    চালু;
                }
                outname = malloc(len + 1);
                যদি (outname == ফাঁকা) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    বিরতি;
                }
                memcpy(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            যদি (outname != ফাঁকা) মুক্ত(outname);
            যদি (ret) বিরতি;
        } যখন (argv++, --argc);
    অন্যথা
        ret = gunzip(&strm, ফাঁকা, ফাঁকা, test);

    /* clean up */
    inflateBackEnd(&strm);
    ফেরত ret;
}
