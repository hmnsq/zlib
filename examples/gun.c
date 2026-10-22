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
#അന്തര്ഭാവ <stdio.h>          /* fprintf() */
#അന്തര്ഭാവ <stdlib.h>         /* malloc(), free() */
#അന്തര്ഭാവ <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#അന്തര്ഭാവ <errno.h>          /* errno */
#അന്തര്ഭാവ <fcntl.h>          /* open() */
#അന്തര്ഭാവ <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>       /* stat(), chmod() */
#അന്തര്ഭാവ <utime.h>          /* utime() */
#അന്തര്ഭാവ "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#define local സ്ഥിതം

/* buffer constants */
#define SIZE 32768U         /* input and output buffer sizes */
#define PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
ഘടന ind {
    പൂർണ്ണസംഖ്യ infile;
    unsigned അക്ഷരം *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local unsigned in(ശൂന്യം *in_desc, z_const unsigned അക്ഷരം **buf)
{
    പൂർണ്ണസംഖ്യ ret;
    unsigned len;
    unsigned അക്ഷരം *next;
    ഘടന ind *me = (ഘടന ind *)in_desc;

    next = me->inbuf;
    *buf = next;
    len = 0;
    ചെയ്യുക {
        ret = PIECE;
        എങ്കിൽ ((unsigned)ret > SIZE - len)
            ret = (പൂർണ്ണസംഖ്യ)(SIZE - len);
        ret = (പൂർണ്ണസംഖ്യ)പഠ(me->infile, next, ret);
        എങ്കിൽ (ret == -1) {
            len = 0;
            നിർത്തുക;
        }
        next += ret;
        len += ret;
    } ആകുമ്പോൾ (ret != 0 && len < SIZE);
    തിരികെ len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
ഘടന outd {
    പൂർണ്ണസംഖ്യ outfile;
    പൂർണ്ണസംഖ്യ check;                  /* true if checking crc and total */
    unsigned long crc;
    unsigned long total;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local പൂർണ്ണസംഖ്യ out(ശൂന്യം *out_desc, unsigned അക്ഷരം *buf, unsigned len)
{
    പൂർണ്ണസംഖ്യ ret;
    ഘടന outd *me = (ഘടന outd *)out_desc;

    എങ്കിൽ (me->check) {
        me->crc = crc32(me->crc, buf, len);
        me->total += len;
    }
    എങ്കിൽ (me->outfile != -1)
        ചെയ്യുക {
            ret = PIECE;
            എങ്കിൽ ((unsigned)ret > len)
                ret = (പൂർണ്ണസംഖ്യ)len;
            ret = (പൂർണ്ണസംഖ്യ)ലിഖ(me->outfile, buf, ret);
            എങ്കിൽ (ret == -1)
                തിരികെ 1;
            buf += ret;
            len -= ret;
        } ആകുമ്പോൾ (len != 0);
    തിരികെ 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#define NEXT() (have ? 0 : (have = in(indp, &next)), \
                last = have ? (have--, (പൂർണ്ണസംഖ്യ)(*next++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
unsigned അക്ഷരം inbuf[SIZE];              /* input buffer */
unsigned അക്ഷരം outbuf[SIZE];             /* output buffer */
unsigned short prefix[65536];           /* index to LZW prefix string */
unsigned അക്ഷരം suffix[65536];            /* one-character LZW suffix */
unsigned അക്ഷരം match[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#define FLUSHCODE() \
    ചെയ്യുക { \
        ഇടത് = 0; \
        rem = 0; \
        എങ്കിൽ (chunk > have) { \
            chunk -= have; \
            have = 0; \
            എങ്കിൽ (NEXT() == -1) \
                നിർത്തുക; \
            chunk--; \
            എങ്കിൽ (chunk > have) { \
                chunk = have = 0; \
                നിർത്തുക; \
            } \
        } \
        have -= chunk; \
        next += chunk; \
        chunk = 0; \
    } ആകുമ്പോൾ (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local പൂർണ്ണസംഖ്യ lunpipe(unsigned have, z_const unsigned അക്ഷരം *next, ഘടന ind *indp,
                  പൂർണ്ണസംഖ്യ outfile, z_stream *strm)
{
    പൂർണ്ണസംഖ്യ last;                   /* last byte read by NEXT(), or -1 if EOF */
    unsigned chunk;             /* bytes left in current chunk */
    പൂർണ്ണസംഖ്യ ഇടത്;                   /* bits left in rem */
    unsigned rem;               /* unused bits from input */
    പൂർണ്ണസംഖ്യ bits;                   /* current bits per code */
    unsigned code;              /* code, table traversal index */
    unsigned mask;              /* mask for current bits codes */
    പൂർണ്ണസംഖ്യ max;                    /* maximum bits per code for this stream */
    unsigned flags;             /* compress flags, then block compress flag */
    unsigned end;               /* last valid entry in prefix/suffix tables */
    unsigned temp;              /* current code */
    unsigned prev;              /* previous code */
    unsigned final;             /* last character written for previous code */
    unsigned അട്ടി;             /* next position for reversed string */
    unsigned outcnt;            /* bytes in output buffer */
    ഘടന outd outd;           /* output structure */
    unsigned അക്ഷരം *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = NEXT();
    എങ്കിൽ (last == -1)
        തിരികെ Z_BUF_ERROR;
    എങ്കിൽ (flags & 0x60) {
        strm->സന്ദേശം = (z_const അക്ഷരം *)"unknown lzw flags set";
        തിരികെ Z_DATA_ERROR;
    }
    max = flags & 0x1f;
    എങ്കിൽ (max < 9 || max > 16) {
        strm->സന്ദേശം = (z_const അക്ഷരം *)"lzw bits out of range";
        തിരികെ Z_DATA_ERROR;
    }
    എങ്കിൽ (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    bits = 9;
    mask = 0x1ff;
    end = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    എങ്കിൽ (NEXT() == -1)                       /* no compressed data is ok */
        തിരികെ Z_OK;
    final = prev = (unsigned)last;          /* low 8 bits of code */
    എങ്കിൽ (NEXT() == -1)                       /* missing a bit */
        തിരികെ Z_BUF_ERROR;
    എങ്കിൽ (last & 1) {                         /* code must be < 256 */
        strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid lzw code";
        തിരികെ Z_DATA_ERROR;
    }
    rem = (unsigned)last >> 1;              /* remaining 7 bits */
    ഇടത് = 7;
    chunk = bits - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (unsigned അക്ഷരം)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    അട്ടി = 0;
    വേണ്ടി (;;) {
        /* if the table will be full after this, increment the code size */
        എങ്കിൽ (end >= mask && bits < max) {
            FLUSHCODE();
            bits++;
            mask <<= 1;
            mask++;
        }

        /* get a code of length bits */
        എങ്കിൽ (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = bits;
        code = rem;                         /* low bits of code */
        എങ്കിൽ (NEXT() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            എങ്കിൽ (outcnt && out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf;     /* signal write error */
                തിരികെ Z_BUF_ERROR;
            }
            തിരികെ Z_OK;
        }
        code += (unsigned)last << ഇടത്;     /* middle (or high) bits of code */
        ഇടത് += 8;
        chunk--;
        എങ്കിൽ (bits > ഇടത്) {                  /* need more bits */
            എങ്കിൽ (NEXT() == -1)               /* can't end in middle of code */
                തിരികെ Z_BUF_ERROR;
            code += (unsigned)last << ഇടത്; /* high bits of code */
            ഇടത് += 8;
            chunk--;
        }
        code &= mask;                       /* mask to current code length */
        ഇടത് -= bits;                       /* number of unused bits */
        rem = (unsigned)last >> (8 - ഇടത്); /* unused bits from last byte */

        /* process clear code (256) */
        എങ്കിൽ (code == 256 && flags) {
            FLUSHCODE();
            bits = 9;                       /* initialize bits and mask */
            mask = 0x1ff;
            end = 255;                      /* empty table */
            തുടരുക;                       /* get next code */
        }

        /* special code to reuse last match */
        temp = code;                        /* save the current code */
        എങ്കിൽ (code > end) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            എങ്കിൽ (code != end + 1 || prev > end) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"invalid lzw code";
                തിരികെ Z_DATA_ERROR;
            }
            match[അട്ടി++] = (unsigned അക്ഷരം)final;
            code = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = match + അട്ടി;
        ആകുമ്പോൾ (code >= 256) {
            *p++ = suffix[code];
            code = prefix[code];
        }
        അട്ടി = p - match;
        match[അട്ടി++] = (unsigned അക്ഷരം)code;
        final = code;

        /* link new table entry */
        എങ്കിൽ (end < mask) {
            end++;
            prefix[end] = (unsigned short)prev;
            suffix[end] = (unsigned അക്ഷരം)final;
        }

        /* set previous code for next iteration */
        prev = temp;

        /* write output in forward order */
        ആകുമ്പോൾ (അട്ടി > SIZE - outcnt) {
            ആകുമ്പോൾ (outcnt < SIZE)
                outbuf[outcnt++] = match[--അട്ടി];
            എങ്കിൽ (out(&outd, outbuf, outcnt)) {
                strm->next_in = outbuf; /* signal write error */
                തിരികെ Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = match + അട്ടി;
        ചെയ്യുക {
            outbuf[outcnt++] = *--p;
        } ആകുമ്പോൾ (p > match);
        അട്ടി = 0;

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
local പൂർണ്ണസംഖ്യ gunpipe(z_stream *strm, പൂർണ്ണസംഖ്യ infile, പൂർണ്ണസംഖ്യ outfile)
{
    പൂർണ്ണസംഖ്യ ret, first, last;
    unsigned have, flags, len;
    z_const unsigned അക്ഷരം *next = ശൂന്യ;
    ഘടന ind ind, *indp;
    ഘടന outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    first = 1;                              /* looking for first gzip header */
    strm->next_in = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    വേണ്ടി (;;) {
        /* look for the two magic header bytes for a gzip stream */
        എങ്കിൽ (NEXT() == -1) {
            ret = Z_OK;
            നിർത്തുക;                          /* empty gzip stream is ok */
        }
        എങ്കിൽ (last != 31 || (NEXT() != 139 && last != 157)) {
            strm->സന്ദേശം = (z_const അക്ഷരം *)"incorrect header check";
            ret = first ? Z_DATA_ERROR : Z_ERRNO;
            നിർത്തുക;                          /* not a gzip or compress header */
        }
        first = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        എങ്കിൽ (last == 157) {
            ret = lunpipe(have, next, indp, outfile, strm);
            നിർത്തുക;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        എങ്കിൽ (NEXT() != 8) {                  /* only deflate method allowed */
            എങ്കിൽ (last == -1) നിർത്തുക;
            strm->സന്ദേശം = (z_const അക്ഷരം *)"unknown compression method";
            ret = Z_DATA_ERROR;
            നിർത്തുക;
        }
        flags = NEXT();                     /* header flags */
        NEXT();                             /* discard mod time, xflgs, os */
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        NEXT();
        എങ്കിൽ (last == -1) നിർത്തുക;
        എങ്കിൽ (flags & 0xe0) {
            strm->സന്ദേശം = (z_const അക്ഷരം *)"unknown header flags set";
            ret = Z_DATA_ERROR;
            നിർത്തുക;
        }
        എങ്കിൽ (flags & 4) {                    /* extra field */
            len = NEXT();
            len += (unsigned)(NEXT()) << 8;
            എങ്കിൽ (last == -1) നിർത്തുക;
            ആകുമ്പോൾ (len > have) {
                len -= have;
                have = 0;
                എങ്കിൽ (NEXT() == -1) നിർത്തുക;
                len--;
            }
            എങ്കിൽ (last == -1) നിർത്തുക;
            have -= len;
            next += len;
        }
        എങ്കിൽ (flags & 8)                      /* file name */
            ആകുമ്പോൾ (NEXT() != 0 && last != -1)
                ;
        എങ്കിൽ (flags & 16)                     /* comment */
            ആകുമ്പോൾ (NEXT() != 0 && last != -1)
                ;
        എങ്കിൽ (flags & 2) {                    /* header crc */
            NEXT();
            NEXT();
        }
        എങ്കിൽ (last == -1) നിർത്തുക;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.crc = crc32(0L, Z_NULL, 0);
        outd.total = 0;

        /* decompress data to output */
        strm->next_in = next;
        strm->avail_in = have;
        ret = inflateBack(strm, in, indp, out, &outd);
        എങ്കിൽ (ret != Z_STREAM_END) നിർത്തുക;
        next = strm->next_in;
        have = strm->avail_in;
        strm->next_in = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        എങ്കിൽ (NEXT() != (പൂർണ്ണസംഖ്യ)(outd.crc & 0xff) ||
            NEXT() != (പൂർണ്ണസംഖ്യ)((outd.crc >> 8) & 0xff) ||
            NEXT() != (പൂർണ്ണസംഖ്യ)((outd.crc >> 16) & 0xff) ||
            NEXT() != (പൂർണ്ണസംഖ്യ)((outd.crc >> 24) & 0xff)) {
            /* crc error */
            എങ്കിൽ (last != -1) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"incorrect data check";
                ret = Z_DATA_ERROR;
            }
            നിർത്തുക;
        }
        എങ്കിൽ (NEXT() != (പൂർണ്ണസംഖ്യ)(outd.total & 0xff) ||
            NEXT() != (പൂർണ്ണസംഖ്യ)((outd.total >> 8) & 0xff) ||
            NEXT() != (പൂർണ്ണസംഖ്യ)((outd.total >> 16) & 0xff) ||
            NEXT() != (പൂർണ്ണസംഖ്യ)((outd.total >> 24) & 0xff)) {
            /* length error */
            എങ്കിൽ (last != -1) {
                strm->സന്ദേശം = (z_const അക്ഷരം *)"incorrect length check";
                ret = Z_DATA_ERROR;
            }
            നിർത്തുക;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    തിരികെ ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local ശൂന്യം copymeta(അക്ഷരം *from, അക്ഷരം *to)
{
    ഘടന stat was;
    ഘടന utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    എങ്കിൽ (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        തിരികെ;

    /* set to's mode bits, ignore errors */
    (ശൂന്യം)chmod(to, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (ശൂന്യം)chown(to, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (ശൂന്യം)utime(to, &when);
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
local പൂർണ്ണസംഖ്യ gunzip(z_stream *strm, അക്ഷരം *inname, അക്ഷരം *outname, പൂർണ്ണസംഖ്യ test)
{
    പൂർണ്ണസംഖ്യ ret;
    പൂർണ്ണസംഖ്യ infile, outfile;

    /* open files */
    എങ്കിൽ (inname == ശൂന്യ || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    അല്ലെങ്കിൽ {
        infile = open(inname, O_RDONLY, 0);
        എങ്കിൽ (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            തിരികെ 0;
        }
    }
    എങ്കിൽ (test)
        outfile = -1;
    അല്ലെങ്കിൽ എങ്കിൽ (outname == ശൂന്യ || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    അല്ലെങ്കിൽ {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        എങ്കിൽ (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            തിരികെ 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    എങ്കിൽ (outfile > 2) close(outfile);
    എങ്കിൽ (infile > 2) close(infile);

    /* interpret result */
    തിരഞ്ഞെടുപ്പ് (ret) {
    അവസ്ഥ Z_OK:
    അവസ്ഥ Z_ERRNO:
        എങ്കിൽ (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        എങ്കിൽ (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        നിർത്തുക;
    അവസ്ഥ Z_DATA_ERROR:
        എങ്കിൽ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->സന്ദേശം);
        നിർത്തുക;
    അവസ്ഥ Z_MEM_ERROR:
        എങ്കിൽ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        തിരികെ 1;
    അവസ്ഥ Z_BUF_ERROR:
        എങ്കിൽ (outfile > 2) unlink(outname);
        എങ്കിൽ (strm->next_in != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        അല്ലെങ്കിൽ എങ്കിൽ (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        അല്ലെങ്കിൽ {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        എങ്കിൽ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        തിരികെ 1;
    }
    തിരികെ 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
    പൂർണ്ണസംഖ്യ ret, len, test;
    അക്ഷരം *outname;
    unsigned അക്ഷരം *window;
    z_stream strm;

    /* initialize inflateBack state for repeated use */
    window = match;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateBackInit(&strm, 15, window);
    എങ്കിൽ (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        തിരികെ 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    എങ്കിൽ (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        തിരികെ 0;
    }
    എങ്കിൽ (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    എങ്കിൽ (argc)
        ചെയ്യുക {
            എങ്കിൽ (test)
                outname = ശൂന്യ;
            അല്ലെങ്കിൽ {
                len = (പൂർണ്ണസംഖ്യ)strlen(*argv);
                എങ്കിൽ (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                അല്ലെങ്കിൽ എങ്കിൽ (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                അല്ലെങ്കിൽ {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    തുടരുക;
                }
                outname = malloc(len + 1);
                എങ്കിൽ (outname == ശൂന്യ) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    നിർത്തുക;
                }
                memcpy(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            എങ്കിൽ (outname != ശൂന്യ) സ്വതന്ത്രം(outname);
            എങ്കിൽ (ret) നിർത്തുക;
        } ആകുമ്പോൾ (argv++, --argc);
    അല്ലെങ്കിൽ
        ret = gunzip(&strm, ശൂന്യ, ശൂന്യ, test);

    /* clean up */
    inflateBackEnd(&strm);
    തിരികെ ret;
}
