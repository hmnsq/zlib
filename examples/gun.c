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
#ଅନ୍ତର୍ଭାଵ <stdio.h>          /* fprintf() */
#ଅନ୍ତର୍ଭାଵ <stdlib.h>         /* malloc(), free() */
#ଅନ୍ତର୍ଭାଵ <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#ଅନ୍ତର୍ଭାଵ <errno.h>          /* errno */
#ଅନ୍ତର୍ଭାଵ <fcntl.h>          /* open() */
#ଅନ୍ତର୍ଭାଵ <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>       /* stat(), chmod() */
#ଅନ୍ତର୍ଭାଵ <utime.h>          /* utime() */
#ଅନ୍ତର୍ଭାଵ "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#define local ସ୍ଥିର

/* buffer constants */
#define SIZE 32768U         /* input and output buffer sizes */
#define PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
ସଂରଚନା ind {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା infile;
    unsigned ଅକ୍ଷର *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local unsigned in(ଶୂନ୍ୟ *in_desc, z_const unsigned ଅକ୍ଷର **buf)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    unsigned len;
    unsigned ଅକ୍ଷର *next;
    ସଂରଚନା ind *me = (ସଂରଚନା ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    କର {
        ret = PIECE;
        ଯଦି ((unsigned)ret > SIZE - len)
            ret = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(SIZE - len);
        ret = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ପଠ(me->infile, next, ret);
        ଯଦି (ret == -1) {
            len = 0;
            ଭାଙ୍ଗ;
        }
        next += ret;
        len += ret;
    } ଯେତେବେଳେ (ret != 0 && len < SIZE);
    ଫେରନ୍ତୁ len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
ସଂରଚନା outd {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା outfile;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା check;                  /* true if checking crc and total */
    unsigned long crc;
    unsigned long total;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା out(ଶୂନ୍ୟ *out_desc, unsigned ଅକ୍ଷର *buf, unsigned len)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    ସଂରଚନା outd *me = (ସଂରଚନା outd *)out_desc;

    ଯଦି (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    ଯଦି (me->outfile != -1)
        କର {
            ret = PIECE;
            ଯଦି ((unsigned)ret > len)
                ret = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len;
            ret = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଲିଖ(me->outfile, buf, ret);
            ଯଦି (ret == -1)
                ଫେରନ୍ତୁ 1;
            buf += ret;
            len -= ret;
        } ଯେତେବେଳେ (len != 0);
    ଫେରନ୍ତୁ 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define NEXT() (have ? 0 : (have = in(indp, &next)), \
                last = have ? (have--, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(*next++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
unsigned ଅକ୍ଷର inbuf[SIZE];              /* input buffer */
unsigned ଅକ୍ଷର outbuf[SIZE];             /* output buffer */
unsigned short prefix[65536];           /* index to LZW prefix string */
unsigned ଅକ୍ଷର suffix[65536];            /* one-character LZW suffix */
unsigned ଅକ୍ଷର match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE() \
    କର { \
        ବାମ = 0; \
        rem = 0; \
        ଯଦି (chunk > have) { \
            chunk -= have; \
            have = 0; \
            ଯଦି (NEXT() == -1) \
                ଭାଙ୍ଗ; \
            chunk--; \
            ଯଦି (chunk > have) { \
                chunk = have = 0; \
                ଭାଙ୍ଗ; \
            } \
        } \
        have -= chunk; \
        next += chunk; \
        chunk = 0; \
    } ଯେତେବେଳେ (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା lunpipe(unsigned have, z_const unsigned ଅକ୍ଷର *next, ସଂରଚନା ind *indp,
                  ପୂର୍ଣ୍ଣସଂଖ୍ୟା outfile, z_stream *strm)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା last;                   /* last byte read by NEXT(), or -1 if EOF */
    unsigned chunk;             /* bytes left in current chunk */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ବାମ;                   /* bits left in rem */
    unsigned rem;               /* unused bits from input */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bits;                   /* current bits per code */
    unsigned code;              /* code, table traversal index */
    unsigned mask;              /* mask for current bits codes */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା max;                    /* maximum bits per code for this stream */
    unsigned flags;             /* compress flags, then block compress flag */
    unsigned end;               /* last valid entry in prefix/suffix tables */
    unsigned temp;              /* current code */
    unsigned prev;              /* previous code */
    unsigned final;             /* last character written for previous code */
    unsigned ଗଦା;             /* next position for reversed string */
    unsigned outcnt;            /* bytes in output buffer */
    ସଂରଚନା outd outd;           /* output structure */
    unsigned ଅକ୍ଷର *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = NEXT();
    ଯଦି (last == -1)
        ଫେରନ୍ତୁ Z_BUF_ERROR;
    ଯଦି (flags & 0x60) {
        strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"unknown lzw flags set";
        ଫେରନ୍ତୁ Z_DATA_ERROR;
    }
    max = flags & 0x1f;
    ଯଦି (max < 9 || max > 16) {
        strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"lzw bits out of range";
        ଫେରନ୍ତୁ Z_DATA_ERROR;
    }
    ଯଦି (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    ଯଦି (NEXT() == -1)                       /* no compressed data is ok */
        ଫେରନ୍ତୁ Z_OK;
    final = prev = (unsigned)last;          /* low 8 bits of code */
    ଯଦି (NEXT() == -1)                       /* missing a bit */
        ଫେରନ୍ତୁ Z_BUF_ERROR;
    ଯଦି (last & 1) {                         /* code must be < 256 */
        strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid lzw code";
        ଫେରନ୍ତୁ Z_DATA_ERROR;
    }
    rem = (unsigned)last >> 1;              /* remaining 7 bits */
    ବାମ = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (unsigned ଅକ୍ଷର)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    ଗଦା = 0;
    ପାଇଁ (;;) {
        /* if the table will be full after this, increment the code size */
        ଯଦି (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        ଯଦି (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        ଯଦି (NEXT() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            ଯଦି (outcnt && out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf;     /* signal write error */
                ଫେରନ୍ତୁ Z_BUF_ERROR;
            }
            ଫେରନ୍ତୁ Z_OK;
        }
        code += (unsigned)last << ବାମ;     /* middle (or high) bits of code */
        ବାମ += 8;
        chunk--;
        ଯଦି (bits > ବାମ) {                  /* need more bits */
            ଯଦି (NEXT() == -1)               /* can't end in middle of code */
                ଫେରନ୍ତୁ Z_BUF_ERROR;
            code += (unsigned)last << ବାମ; /* high bits of code */
            ବାମ += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        ବାମ -= bits;                       /* number of unused bits */
        rem = (unsigned)last >> (8 - ବାମ); /* unused bits from last byte */

        /* process clear code (256) */
        ଯଦି (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            ଜାରି;                       /* get next code */
        }

        /* special code to reuse last match */
        temp = code;                        /* save the current code */
        ଯଦି (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            ଯଦି (code != end + 1 || prev > end) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"invalid lzw code";
                ଫେରନ୍ତୁ Z_DATA_ERROR;
            }
            match[ଗଦା++] = (unsigned ଅକ୍ଷର)final;
            code = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + ଗଦା;
        ଯେତେବେଳେ (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        ଗଦା = p - match;
        match[ଗଦା++] = (unsigned ଅକ୍ଷର)code;
        final = code;

        /* link new table entry */
        ଯଦି (end < mask) {
            end++;
            prefix[end] = (unsigned short)prev;
            suffix[end] = (unsigned ଅକ୍ଷର)final;
        }

        /* set previous code for next iteration */
        prev = temp;

        /* write output in forward order */
        ଯେତେବେଳେ (ଗଦା > SIZE - outcnt) {
            ଯେତେବେଳେ (outcnt < SIZE)
                outbuf[outcnt++] = match[--ଗଦା];
            ଯଦି (out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                ଫେରନ୍ତୁ Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + ଗଦା;
        କର {
            outbuf[outcnt++] = *--p;
        } ଯେତେବେଳେ (p > match);
        ଗଦା = 0;

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
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gunpipe(z_stream *strm, ପୂର୍ଣ୍ଣସଂଖ୍ୟା infile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା outfile)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, first, last;
    unsigned have, flags, len;
    z_const unsigned ଅକ୍ଷର *next = ଖାଲି;
    ସଂରଚନା ind ind, *indp;
    ସଂରଚନା outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->next_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    ପାଇଁ (;;) {
        /* look for the two magic header bytes for a gzip stream */
        ଯଦି (NEXT() == -1) {
            ret = Z_OK;
            ଭାଙ୍ଗ;                          /* empty gzip stream is ok */
        }
        ଯଦି (last != 31 || (NEXT() != 139 && last != 157)) {
            strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"incorrect header check";
            ret = first ? Z_DATA_ERROR : Z_ERRNO;
            ଭାଙ୍ଗ;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        ଯଦି (last == 157) {
            ret = lunpipe(have, next, indp, outfile, strm);
            ଭାଙ୍ଗ;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        ଯଦି (NEXT() != 8) {                  /* only deflate method allowed */
            ଯଦି (last == -1) ଭାଙ୍ଗ;
            strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"unknown compression method";
            ret = Z_DATA_ERROR;
            ଭାଙ୍ଗ;
        }
        flags = NEXT();                     /* header flags */
        NEXT();                             /* discard mod time, xflgs, os */
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        ଯଦି (last == -1) ଭାଙ୍ଗ;
        ଯଦି (flags & 0xe0) {
            strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"unknown header flags set";
            ret = Z_DATA_ERROR;
            ଭାଙ୍ଗ;
        }
        ଯଦି (flags & 4) {                    /* extra field */
            len = NEXT();
            len += (unsigned)(NEXT()) << 8;
            ଯଦି (last == -1) ଭାଙ୍ଗ;
            ଯେତେବେଳେ (len > have) {
                len -= have;
                have = 0;
                ଯଦି (NEXT() == -1) ଭାଙ୍ଗ;
                len--;
            }
            ଯଦି (last == -1) ଭାଙ୍ଗ;
            have -= len;
            next += len;
        }
        ଯଦି (flags & 8)                      /* file name */
            ଯେତେବେଳେ (NEXT() != 0 && last != -1)
                ;
        ଯଦି (flags & 16)                     /* comment */
            ଯେତେବେଳେ (NEXT() != 0 && last != -1)
                ;
        ଯଦି (flags & 2) {                    /* header crc */
            NEXT();
            NEXT();
        }
        ଯଦି (last == -1) ଭାଙ୍ଗ;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.total = 0;

        /* decompress data to output */
        strm->next_in = next;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        ଯଦି (ret != Z_STREAM_END) ଭାଙ୍ଗ;
        next = strm->next_in;
        have = strm->avail_in;
        strm->next_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        ଯଦି (NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(outd.crc & 0xff) ||
            NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((outd.crc >> 8) & 0xff) ||
            NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((outd.crc >> 16) & 0xff) ||
            NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            ଯଦି (last != -1) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"incorrect data check";
                ret = Z_DATA_ERROR;
            }
            ଭାଙ୍ଗ;
        }
        ଯଦି (NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(outd.total & 0xff) ||
            NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((outd.total >> 8) & 0xff) ||
            NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((outd.total >> 16) & 0xff) ||
            NEXT() != (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((outd.total >> 24) & 0xff)) {
            /* length error */
            ଯଦି (last != -1) {
                strm->ସନ୍ଦେଶ = (z_const ଅକ୍ଷର *)"incorrect length check";
                ret = Z_DATA_ERROR;
            }
            ଭାଙ୍ଗ;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    ଫେରନ୍ତୁ ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local ଶୂନ୍ୟ copymeta(ଅକ୍ଷର *from, ଅକ୍ଷର *to)
{
    ସଂରଚନା stat was;
    ସଂରଚନା utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    ଯଦି (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        ଫେରନ୍ତୁ;

    /* set to's mode bits, ignore errors */
    (ଶୂନ୍ୟ)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (ଶୂନ୍ୟ)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (ଶୂନ୍ୟ)utime(to, &when);
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
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gunzip(z_stream *strm, ଅକ୍ଷର *inname, ଅକ୍ଷର *outname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା test)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା infile, outfile;

    /* open files */
    ଯଦି (inname == ଖାଲି || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    ଅନ୍ୟଥା {
        infile = open(inname, O_RDONLY, 0);
        ଯଦି (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            ଫେରନ୍ତୁ 0;
        }
    }
    ଯଦି (test)
        outfile = -1;
    ଅନ୍ୟଥା ଯଦି (outname == ଖାଲି || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    ଅନ୍ୟଥା {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        ଯଦି (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            ଫେରନ୍ତୁ 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    ଯଦି (outfile > 2) close(outfile);
    ଯଦି (infile > 2) close(infile);

    /* interpret result */
    ବାଛ (ret) {
    ସ୍ଥିତି Z_OK:
    ସ୍ଥିତି Z_ERRNO:
        ଯଦି (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        ଯଦି (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି Z_DATA_ERROR:
        ଯଦି (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->ସନ୍ଦେଶ);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି Z_MEM_ERROR:
        ଯଦି (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        ଫେରନ୍ତୁ 1;
    ସ୍ଥିତି Z_BUF_ERROR:
        ଯଦି (outfile > 2) unlink(outname);
        ଯଦି (strm->next_in != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        ଅନ୍ୟଥା ଯଦି (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        ଅନ୍ୟଥା {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଯଦି (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        ଫେରନ୍ତୁ 1;
    }
    ଫେରନ୍ତୁ 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, len, test;
    ଅକ୍ଷର *outname;
    unsigned ଅକ୍ଷର *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    ଯଦି (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        ଫେରନ୍ତୁ 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    ଯଦି (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        ଫେରନ୍ତୁ 0;
    }
    ଯଦି (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    ଯଦି (argc)
        କର {
            ଯଦି (test)
                outname = ଖାଲି;
            ଅନ୍ୟଥା {
                len = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(*argv);
                ଯଦି (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                ଅନ୍ୟଥା ଯଦି (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                ଅନ୍ୟଥା {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    ଜାରି;
                }
                outname = malloc(len + 1);
                ଯଦି (outname == ଖାଲି) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    ଭାଙ୍ଗ;
                }
                memcpy(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            ଯଦି (outname != ଖାଲି) ମୁକ୍ତ(outname);
            ଯଦି (ret) ଭାଙ୍ଗ;
        } ଯେତେବେଳେ (argv++, --argc);
    ଅନ୍ୟଥା
        ret = gunzip(&strm, ଖାଲି, ଖାଲି, test);

    /* clean up */
    inflateBackEnd(&strm);
    ଫେରନ୍ତୁ ret;
}
