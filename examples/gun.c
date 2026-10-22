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
#include <stdio.h>          /* fprintf() */
#include <stdlib.h>         /* malloc(), free() */
#include <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#include <errno.h>          /* errno */
#include <fcntl.h>          /* open() */
#include <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#include <sys/types.h>
#include <sys/stat.h>       /* stat(), chmod() */
#include <utime.h>          /* utime() */
#include "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#define local static

/* buffer constants */
#define SIZE 32768U         /* input and output buffer sizes */
#define PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
struct ind {
    цео infile;
    unsigned char *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local unsigned in(void *in_desc, z_const unsigned char **buf)
{
    цео ret;
    unsigned len;
    unsigned char *next;
    struct ind *me = (struct ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    do {
        ret = PIECE;
        ако ((unsigned)ret > SIZE - len)
            ret = (цео)(SIZE - len);
        ret = (цео)read(me->infile, next, ret);
        ако (ret == -1) {
            len = 0;
            break;
        }
        next += ret;
        len += ret;
    } док (ret != 0 && len < SIZE);
    врати len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
struct outd {
    цео outfile;
    цео check;                  /* true if checking crc and total */
    unsigned long crc;
    unsigned long total;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local цео out(void *out_desc, unsigned char *buf, unsigned len)
{
    цео ret;
    struct outd *me = (struct outd *)out_desc;

    ако (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    ако (me->outfile != -1)
        do {
            ret = PIECE;
            ако ((unsigned)ret > len)
                ret = (цео)len;
            ret = (цео)write(me->outfile, buf, ret);
            ако (ret == -1)
                врати 1;
            buf += ret;
            len -= ret;
        } док (len != 0);
    врати 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define NEXT() (have ? 0 : (have = in(indp, &next)), \
                last = have ? (have--, (цео)(*next++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
unsigned char inbuf[SIZE];              /* input buffer */
unsigned char outbuf[SIZE];             /* output buffer */
unsigned short prefix[65536];           /* index to LZW prefix string */
unsigned char suffix[65536];            /* one-character LZW suffix */
unsigned char match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE() \
    do { \
        left = 0; \
        rem = 0; \
        ако (chunk > have) { \
            chunk -= have; \
            have = 0; \
            ако (NEXT() == -1) \
                break; \
            chunk--; \
            ако (chunk > have) { \
                chunk = have = 0; \
                break; \
            } \
        } \
        have -= chunk; \
        next += chunk; \
        chunk = 0; \
    } док (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local цео lunpipe(unsigned have, z_const unsigned char *next, struct ind *indp,
                  цео outfile, z_stream *strm)
{
    цео last;                   /* last byte read by NEXT(), or -1 if EOF */
    unsigned chunk;             /* bytes left in current chunk */
    цео left;                   /* bits left in rem */
    unsigned rem;               /* unused bits from input */
    цео bits;                   /* current bits per code */
    unsigned code;              /* code, table traversal index */
    unsigned mask;              /* mask for current bits codes */
    цео max;                    /* maximum bits per code for this stream */
    unsigned flags;             /* compress flags, then block compress flag */
    unsigned end;               /* last valid entry in prefix/suffix tables */
    unsigned temp;              /* current code */
    unsigned prev;              /* previous code */
    unsigned final;             /* last character written for previous code */
    unsigned stack;             /* next position for reversed string */
    unsigned outcnt;            /* bytes in output buffer */
    struct outd outd;           /* output structure */
    unsigned char *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = NEXT();
    ако (last == -1)
        врати Z_BUF_ERROR;
    ако (flags & 0x60) {
        strm->msg = (z_const char *)"unknown lzw flags set";
        врати Z_DATA_ERROR;
    }
    max = flags & 0x1f;
    ако (max < 9 || max > 16) {
        strm->msg = (z_const char *)"lzw bits out of range";
        врати Z_DATA_ERROR;
    }
    ако (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    ако (NEXT() == -1)                       /* no compressed data is ok */
        врати Z_OK;
    final = prev = (unsigned)last;          /* low 8 bits of code */
    ако (NEXT() == -1)                       /* missing a bit */
        врати Z_BUF_ERROR;
    ако (last & 1) {                         /* code must be < 256 */
        strm->msg = (z_const char *)"invalid lzw code";
        врати Z_DATA_ERROR;
    }
    rem = (unsigned)last >> 1;              /* remaining 7 bits */
    left = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (unsigned char)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    stack = 0;
    за (;;) {
        /* if the table will be full after this, increment the code size */
        ако (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        ако (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        ако (NEXT() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            ако (outcnt && out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf;     /* signal write error */
                врати Z_BUF_ERROR;
            }
            врати Z_OK;
        }
        code += (unsigned)last << left;     /* middle (or high) bits of code */
        left += 8;
        chunk--;
        ако (bits > left) {                  /* need more bits */
            ако (NEXT() == -1)               /* can't end in middle of code */
                врати Z_BUF_ERROR;
            code += (unsigned)last << left; /* high bits of code */
            left += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        left -= bits;                       /* number of unused bits */
        rem = (unsigned)last >> (8 - left); /* unused bits from last byte */

        /* process clear code (256) */
        ако (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            continue;                       /* get next code */
        }

        /* special code to reuse last match */
        temp = code;                        /* save the current code */
        ако (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            ако (code != end + 1 || prev > end) {
                strm->msg = (z_const char *)"invalid lzw code";
                врати Z_DATA_ERROR;
            }
            match[stack++] = (unsigned char)final;
            code = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + stack;
        док (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        stack = p - match;
        match[stack++] = (unsigned char)code;
        final = code;

        /* link new table entry */
        ако (end < mask) {
            end++;
            prefix[end] = (unsigned short)prev;
            suffix[end] = (unsigned char)final;
        }

        /* set previous code for next iteration */
        prev = temp;

        /* write output in forward order */
        док (stack > SIZE - outcnt) {
            док (outcnt < SIZE)
                outbuf[outcnt++] = match[--stack];
            ако (out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                врати Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + stack;
        do {
            outbuf[outcnt++] = *--p;
        } док (p > match);
        stack = 0;

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
local цео gunpipe(z_stream *strm, цео infile, цео outfile)
{
    цео ret, first, last;
    unsigned have, flags, len;
    z_const unsigned char *next = NULL;
    struct ind ind, *indp;
    struct outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->next_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    за (;;) {
        /* look for the two magic header bytes for a gzip stream */
        ако (NEXT() == -1) {
            ret = Z_OK;
            break;                          /* empty gzip stream is ok */
        }
        ако (last != 31 || (NEXT() != 139 && last != 157)) {
            strm->msg = (z_const char *)"incorrect header check";
            ret = first ? Z_DATA_ERROR : Z_ERRNO;
            break;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        ако (last == 157) {
            ret = lunpipe(have, next, indp, outfile, strm);
            break;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        ако (NEXT() != 8) {                  /* only deflate method allowed */
            ако (last == -1) break;
            strm->msg = (z_const char *)"unknown compression method";
            ret = Z_DATA_ERROR;
            break;
        }
        flags = NEXT();                     /* header flags */
        NEXT();                             /* discard mod time, xflgs, os */
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        ако (last == -1) break;
        ако (flags & 0xe0) {
            strm->msg = (z_const char *)"unknown header flags set";
            ret = Z_DATA_ERROR;
            break;
        }
        ако (flags & 4) {                    /* extra field */
            len = NEXT();
            len += (unsigned)(NEXT()) << 8;
            ако (last == -1) break;
            док (len > have) {
                len -= have;
                have = 0;
                ако (NEXT() == -1) break;
                len--;
            }
            ако (last == -1) break;
            have -= len;
            next += len;
        }
        ако (flags & 8)                      /* file name */
            док (NEXT() != 0 && last != -1)
                ;
        ако (flags & 16)                     /* comment */
            док (NEXT() != 0 && last != -1)
                ;
        ако (flags & 2) {                    /* header crc */
            NEXT();
            NEXT();
        }
        ако (last == -1) break;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.total = 0;

        /* decompress data to output */
        strm->next_in = next;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        ако (ret != Z_STREAM_END) break;
        next = strm->next_in;
        have = strm->avail_in;
        strm->next_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        ако (NEXT() != (цео)(outd.crc & 0xff) ||
            NEXT() != (цео)((outd.crc >> 8) & 0xff) ||
            NEXT() != (цео)((outd.crc >> 16) & 0xff) ||
            NEXT() != (цео)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            ако (last != -1) {
                strm->msg = (z_const char *)"incorrect data check";
                ret = Z_DATA_ERROR;
            }
            break;
        }
        ако (NEXT() != (цео)(outd.total & 0xff) ||
            NEXT() != (цео)((outd.total >> 8) & 0xff) ||
            NEXT() != (цео)((outd.total >> 16) & 0xff) ||
            NEXT() != (цео)((outd.total >> 24) & 0xff)) {
            /* length error */
            ако (last != -1) {
                strm->msg = (z_const char *)"incorrect length check";
                ret = Z_DATA_ERROR;
            }
            break;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    врати ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local void copymeta(char *from, char *to)
{
    struct stat was;
    struct utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    ако (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        врати;

    /* set to's mode bits, ignore errors */
    (void)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (void)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (void)utime(to, &when);
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
local цео gunzip(z_stream *strm, char *inname, char *outname, цео test)
{
    цео ret;
    цео infile, outfile;

    /* open files */
    ако (inname == NULL || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    инако {
        infile = open(inname, O_RDONLY, 0);
        ако (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            врати 0;
        }
    }
    ако (test)
        outfile = -1;
    инако ако (outname == NULL || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    инако {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        ако (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            врати 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    ако (outfile > 2) close(outfile);
    ако (infile > 2) close(infile);

    /* interpret result */
    switch (ret) {
    case Z_OK:
    case Z_ERRNO:
        ако (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        ако (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        break;
    case Z_DATA_ERROR:
        ако (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->msg);
        break;
    case Z_MEM_ERROR:
        ако (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        врати 1;
    case Z_BUF_ERROR:
        ако (outfile > 2) unlink(outname);
        ако (strm->next_in != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        инако ако (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        инако {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        break;
    default:
        ако (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        врати 1;
    }
    врати 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
цео main(цео argc, char **argv)
{
    цео ret, len, test;
    char *outname;
    unsigned char *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    ако (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        врати 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    ако (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        врати 0;
    }
    ако (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    ако (argc)
        do {
            ако (test)
                outname = NULL;
            инако {
                len = (цео)strlen(*argv);
                ако (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                инако ако (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                инако {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    continue;
                }
                outname = malloc(len + 1);
                ако (outname == NULL) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    break;
                }
                memcpy(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            ако (outname != NULL) free(outname);
            ако (ret) break;
        } док (argv++, --argc);
    инако
        ret = gunzip(&strm, NULL, NULL, test);

    /* clean up */
    inflateBackEnd(&strm);
    врати ret;
}
