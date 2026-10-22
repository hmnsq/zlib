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
#شامل <stdio.h>          /* fprintf() */
#شامل <stdlib.h>         /* malloc(), free() */
#شامل <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#شامل <errno.h>          /* errno */
#شامل <fcntl.h>          /* open() */
#شامل <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#شامل <sys/types.h>
#شامل <sys/stat.h>       /* stat(), chmod() */
#شامل <utime.h>          /* utime() */
#شامل "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#define local ایستا

/* buffer constants */
#define SIZE 32768U         /* input and output buffer sizes */
#define PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
ساختار ind {
    صحیح infile;
    unsigned نویسه *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local unsigned in(تهی *in_desc, z_const unsigned نویسه **buf)
{
    صحیح ret;
    unsigned len;
    unsigned نویسه *next;
    ساختار ind *me = (ساختار ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    انجامبده {
        ret = PIECE;
        اگر ((unsigned)ret > SIZE - len)
            ret = (صحیح)(SIZE - len);
        ret = (صحیح)اقرأ(me->infile, next, ret);
        اگر (ret == -1) {
            len = 0;
            بشکن;
        }
        next += ret;
        len += ret;
    } وقتی (ret != 0 && len < SIZE);
    بازگشت len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
ساختار outd {
    صحیح outfile;
    صحیح check;                  /* true if checking crc and total */
    unsigned long crc;
    unsigned long total;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local صحیح out(تهی *out_desc, unsigned نویسه *buf, unsigned len)
{
    صحیح ret;
    ساختار outd *me = (ساختار outd *)out_desc;

    اگر (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    اگر (me->outfile != -1)
        انجامبده {
            ret = PIECE;
            اگر ((unsigned)ret > len)
                ret = (صحیح)len;
            ret = (صحیح)اكتب(me->outfile, buf, ret);
            اگر (ret == -1)
                بازگشت 1;
            buf += ret;
            len -= ret;
        } وقتی (len != 0);
    بازگشت 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define NEXT() (have ? 0 : (have = in(indp, &next)), \
                last = have ? (have--, (صحیح)(*next++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
unsigned نویسه inbuf[SIZE];              /* input buffer */
unsigned نویسه outbuf[SIZE];             /* output buffer */
unsigned short prefix[65536];           /* index to LZW prefix string */
unsigned نویسه suffix[65536];            /* one-character LZW suffix */
unsigned نویسه match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE() \
    انجامبده { \
        چپ = 0; \
        rem = 0; \
        اگر (chunk > have) { \
            chunk -= have; \
            have = 0; \
            اگر (NEXT() == -1) \
                بشکن; \
            chunk--; \
            اگر (chunk > have) { \
                chunk = have = 0; \
                بشکن; \
            } \
        } \
        have -= chunk; \
        next += chunk; \
        chunk = 0; \
    } وقتی (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local صحیح lunpipe(unsigned have, z_const unsigned نویسه *next, ساختار ind *indp,
                  صحیح outfile, z_stream *strm)
{
    صحیح last;                   /* last byte read by NEXT(), or -1 if EOF */
    unsigned chunk;             /* bytes left in current chunk */
    صحیح چپ;                   /* bits left in rem */
    unsigned rem;               /* unused bits from input */
    صحیح bits;                   /* current bits per code */
    unsigned code;              /* code, table traversal index */
    unsigned mask;              /* mask for current bits codes */
    صحیح max;                    /* maximum bits per code for this stream */
    unsigned flags;             /* compress flags, then block compress flag */
    unsigned end;               /* last valid entry in prefix/suffix tables */
    unsigned temp;              /* current code */
    unsigned prev;              /* previous code */
    unsigned final;             /* last character written for previous code */
    unsigned پشته;             /* next position for reversed string */
    unsigned outcnt;            /* bytes in output buffer */
    ساختار outd outd;           /* output structure */
    unsigned نویسه *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = NEXT();
    اگر (last == -1)
        بازگشت Z_BUF_ERROR;
    اگر (flags & 0x60) {
        strm->پیام = (z_const نویسه *)"unknown lzw flags set";
        بازگشت Z_DATA_ERROR;
    }
    max = flags & 0x1f;
    اگر (max < 9 || max > 16) {
        strm->پیام = (z_const نویسه *)"lzw bits out of range";
        بازگشت Z_DATA_ERROR;
    }
    اگر (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    اگر (NEXT() == -1)                       /* no compressed data is ok */
        بازگشت Z_OK;
    final = prev = (unsigned)last;          /* low 8 bits of code */
    اگر (NEXT() == -1)                       /* missing a bit */
        بازگشت Z_BUF_ERROR;
    اگر (last & 1) {                         /* code must be < 256 */
        strm->پیام = (z_const نویسه *)"invalid lzw code";
        بازگشت Z_DATA_ERROR;
    }
    rem = (unsigned)last >> 1;              /* remaining 7 bits */
    چپ = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (unsigned نویسه)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    پشته = 0;
    برای (;;) {
        /* if the table will be full after this, increment the code size */
        اگر (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        اگر (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        اگر (NEXT() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            اگر (outcnt && out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf;     /* signal write error */
                بازگشت Z_BUF_ERROR;
            }
            بازگشت Z_OK;
        }
        code += (unsigned)last << چپ;     /* middle (or high) bits of code */
        چپ += 8;
        chunk--;
        اگر (bits > چپ) {                  /* need more bits */
            اگر (NEXT() == -1)               /* can't end in middle of code */
                بازگشت Z_BUF_ERROR;
            code += (unsigned)last << چپ; /* high bits of code */
            چپ += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        چپ -= bits;                       /* number of unused bits */
        rem = (unsigned)last >> (8 - چپ); /* unused bits from last byte */

        /* process clear code (256) */
        اگر (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            ادامه;                       /* get next code */
        }

        /* special code to reuse last match */
        temp = code;                        /* save the current code */
        اگر (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            اگر (code != end + 1 || prev > end) {
                strm->پیام = (z_const نویسه *)"invalid lzw code";
                بازگشت Z_DATA_ERROR;
            }
            match[پشته++] = (unsigned نویسه)final;
            code = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + پشته;
        وقتی (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        پشته = p - match;
        match[پشته++] = (unsigned نویسه)code;
        final = code;

        /* link new table entry */
        اگر (end < mask) {
            end++;
            prefix[end] = (unsigned short)prev;
            suffix[end] = (unsigned نویسه)final;
        }

        /* set previous code for next iteration */
        prev = temp;

        /* write output in forward order */
        وقتی (پشته > SIZE - outcnt) {
            وقتی (outcnt < SIZE)
                outbuf[outcnt++] = match[--پشته];
            اگر (out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                بازگشت Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + پشته;
        انجامبده {
            outbuf[outcnt++] = *--p;
        } وقتی (p > match);
        پشته = 0;

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
local صحیح gunpipe(z_stream *strm, صحیح infile, صحیح outfile)
{
    صحیح ret, first, last;
    unsigned have, flags, len;
    z_const unsigned نویسه *next = پوچ;
    ساختار ind ind, *indp;
    ساختار outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->next_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    برای (;;) {
        /* look for the two magic header bytes for a gzip stream */
        اگر (NEXT() == -1) {
            ret = Z_OK;
            بشکن;                          /* empty gzip stream is ok */
        }
        اگر (last != 31 || (NEXT() != 139 && last != 157)) {
            strm->پیام = (z_const نویسه *)"incorrect header check";
            ret = first ? Z_DATA_ERROR : Z_ERRNO;
            بشکن;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        اگر (last == 157) {
            ret = lunpipe(have, next, indp, outfile, strm);
            بشکن;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        اگر (NEXT() != 8) {                  /* only deflate method allowed */
            اگر (last == -1) بشکن;
            strm->پیام = (z_const نویسه *)"unknown compression method";
            ret = Z_DATA_ERROR;
            بشکن;
        }
        flags = NEXT();                     /* header flags */
        NEXT();                             /* discard mod time, xflgs, os */
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        اگر (last == -1) بشکن;
        اگر (flags & 0xe0) {
            strm->پیام = (z_const نویسه *)"unknown header flags set";
            ret = Z_DATA_ERROR;
            بشکن;
        }
        اگر (flags & 4) {                    /* extra field */
            len = NEXT();
            len += (unsigned)(NEXT()) << 8;
            اگر (last == -1) بشکن;
            وقتی (len > have) {
                len -= have;
                have = 0;
                اگر (NEXT() == -1) بشکن;
                len--;
            }
            اگر (last == -1) بشکن;
            have -= len;
            next += len;
        }
        اگر (flags & 8)                      /* file name */
            وقتی (NEXT() != 0 && last != -1)
                ;
        اگر (flags & 16)                     /* comment */
            وقتی (NEXT() != 0 && last != -1)
                ;
        اگر (flags & 2) {                    /* header crc */
            NEXT();
            NEXT();
        }
        اگر (last == -1) بشکن;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.total = 0;

        /* decompress data to output */
        strm->next_in = next;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        اگر (ret != Z_STREAM_END) بشکن;
        next = strm->next_in;
        have = strm->avail_in;
        strm->next_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        اگر (NEXT() != (صحیح)(outd.crc & 0xff) ||
            NEXT() != (صحیح)((outd.crc >> 8) & 0xff) ||
            NEXT() != (صحیح)((outd.crc >> 16) & 0xff) ||
            NEXT() != (صحیح)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            اگر (last != -1) {
                strm->پیام = (z_const نویسه *)"incorrect data check";
                ret = Z_DATA_ERROR;
            }
            بشکن;
        }
        اگر (NEXT() != (صحیح)(outd.total & 0xff) ||
            NEXT() != (صحیح)((outd.total >> 8) & 0xff) ||
            NEXT() != (صحیح)((outd.total >> 16) & 0xff) ||
            NEXT() != (صحیح)((outd.total >> 24) & 0xff)) {
            /* length error */
            اگر (last != -1) {
                strm->پیام = (z_const نویسه *)"incorrect length check";
                ret = Z_DATA_ERROR;
            }
            بشکن;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    بازگشت ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local تهی copymeta(نویسه *from, نویسه *to)
{
    ساختار stat was;
    ساختار utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    اگر (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        بازگشت;

    /* set to's mode bits, ignore errors */
    (تهی)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (تهی)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (تهی)utime(to, &when);
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
local صحیح gunzip(z_stream *strm, نویسه *inname, نویسه *outname, صحیح test)
{
    صحیح ret;
    صحیح infile, outfile;

    /* open files */
    اگر (inname == پوچ || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    وگرنه {
        infile = open(inname, O_RDONLY, 0);
        اگر (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            بازگشت 0;
        }
    }
    اگر (test)
        outfile = -1;
    وگرنه اگر (outname == پوچ || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    وگرنه {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        اگر (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            بازگشت 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    اگر (outfile > 2) close(outfile);
    اگر (infile > 2) close(infile);

    /* interpret result */
    انتخاب (ret) {
    حالت Z_OK:
    حالت Z_ERRNO:
        اگر (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        اگر (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        بشکن;
    حالت Z_DATA_ERROR:
        اگر (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->پیام);
        بشکن;
    حالت Z_MEM_ERROR:
        اگر (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        بازگشت 1;
    حالت Z_BUF_ERROR:
        اگر (outfile > 2) unlink(outname);
        اگر (strm->next_in != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        وگرنه اگر (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        وگرنه {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        بشکن;
    پیشفرض:
        اگر (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        بازگشت 1;
    }
    بازگشت 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
صحیح اصلی(صحیح argc, نویسه **argv)
{
    صحیح ret, len, test;
    نویسه *outname;
    unsigned نویسه *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    اگر (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        بازگشت 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    اگر (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        بازگشت 0;
    }
    اگر (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    اگر (argc)
        انجامبده {
            اگر (test)
                outname = پوچ;
            وگرنه {
                len = (صحیح)strlen(*argv);
                اگر (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                وگرنه اگر (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                وگرنه {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    ادامه;
                }
                outname = malloc(len + 1);
                اگر (outname == پوچ) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    بشکن;
                }
                memcpy(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            اگر (outname != پوچ) آزاد(outname);
            اگر (ret) بشکن;
        } وقتی (argv++, --argc);
    وگرنه
        ret = gunzip(&strm, پوچ, پوچ, test);

    /* clean up */
    inflateBackEnd(&strm);
    بازگشت ret;
}
