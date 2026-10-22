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
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>          /* fprintf() */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>         /* malloc(), free() */
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>          /* errno */
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>          /* open() */
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>       /* stat(), chmod() */
#ਅਨ੍ਤਰ੍ਭਾਵ <utime.h>          /* utime() */
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ local ਸਥਿਰ

/* buffer constants */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_2 32768U         /* input and output buffer sizes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
ਬਣਤਰ ind {
    ਪੂਰਨ infile;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਚ(ਖਾਲੀ *ਵਿੱਚ_desc, z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **buf)
{
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅਗਲਾ;
    ਬਣਤਰ ind *me = (ਬਣਤਰ ind *)ਵਿੱਚ_desc;

    ਅਗਲਾ = me->inbuf;
    *buf = ਅਗਲਾ;
    len = 0;
    ਕਰੋ {
        ret = PIECE;
        ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)ret > ਮਾਪ_2 - len)
            ret = (ਪੂਰਨ)(ਮਾਪ_2 - len);
        ret = (ਪੂਰਨ)ਪਠ(me->infile, ਅਗਲਾ, ret);
        ਜੇ (ret == -1) {
            len = 0;
            ਰੋਕੋ;
        }
        ਅਗਲਾ += ret;
        len += ret;
    } ਜਦੋਂ (ret != 0 && len < ਮਾਪ_2);
    ਵਾਪਸ len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
ਬਣਤਰ outd {
    ਪੂਰਨ outfile;
    ਪੂਰਨ check;                  /* true if checking crc and total */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੱਲ;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local ਪੂਰਨ ਬਾਹਰਲਾ(ਖਾਲੀ *ਬਾਹਰਲਾ_desc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len)
{
    ਪੂਰਨ ret;
    ਬਣਤਰ outd *me = (ਬਣਤਰ outd *)ਬਾਹਰਲਾ_desc;

    ਜੇ (me->check) {
        me->ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_2(me->ਚੱਕਰੀ_ਜਾਂਚ, buf, len);
        me->ਕੁੱਲ += len;
    }
    ਜੇ (me->outfile != -1)
        ਕਰੋ {
            ret = PIECE;
            ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)ret > len)
                ret = (ਪੂਰਨ)len;
            ret = (ਪੂਰਨ)ਲਿਖ(me->outfile, buf, ret);
            ਜੇ (ret == -1)
                ਵਾਪਸ 1;
            buf += ret;
            len -= ret;
        } ਜਦੋਂ (len != 0);
    ਵਾਪਸ 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_2() (have ? 0 : (have = ਵਿੱਚ(indp, &ਅਗਲਾ)), \
                ਆਖਰੀ = have ? (have--, (ਪੂਰਨ)(*ਅਗਲਾ++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ inbuf[ਮਾਪ_2];              /* input buffer */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ outbuf[ਮਾਪ_2];             /* output buffer */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਅਗੇਤਰ[65536];           /* index to LZW prefix string */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ suffix[65536];            /* one-character LZW suffix */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਮੇਲ[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLUSHCODE() \
    ਕਰੋ { \
        ਖੱਬਾ = 0; \
        rem = 0; \
        ਜੇ (chunk > have) { \
            chunk -= have; \
            have = 0; \
            ਜੇ (ਅਗਲਾ_2() == -1) \
                ਰੋਕੋ; \
            chunk--; \
            ਜੇ (chunk > have) { \
                chunk = have = 0; \
                ਰੋਕੋ; \
            } \
        } \
        have -= chunk; \
        ਅਗਲਾ += chunk; \
        chunk = 0; \
    } ਜਦੋਂ (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local ਪੂਰਨ lunpipe(ਬਿਨਾਨਿਸ਼ਾਨੀ have, z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅਗਲਾ, ਬਣਤਰ ind *indp,
                  ਪੂਰਨ outfile, z_ਧਾਰਾ *strm)
{
    ਪੂਰਨ ਆਖਰੀ;                   /* last byte read by NEXT(), or -1 if EOF */
    ਬਿਨਾਨਿਸ਼ਾਨੀ chunk;             /* bytes left in current chunk */
    ਪੂਰਨ ਖੱਬਾ;                   /* bits left in rem */
    ਬਿਨਾਨਿਸ਼ਾਨੀ rem;               /* unused bits from input */
    ਪੂਰਨ ਬਿੱਟ;                   /* current bits per code */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੋਡ;              /* code, table traversal index */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ;              /* mask for current bits codes */
    ਪੂਰਨ max;                    /* maximum bits per code for this stream */
    ਬਿਨਾਨਿਸ਼ਾਨੀ flags;             /* compress flags, then block compress flag */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ;               /* last valid entry in prefix/suffix tables */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਰਜ਼ੀ;              /* current code */
    ਬਿਨਾਨਿਸ਼ਾਨੀ prev;              /* previous code */
    ਬਿਨਾਨਿਸ਼ਾਨੀ final;             /* last character written for previous code */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਢੇਰ_2;             /* next position for reversed string */
    ਬਿਨਾਨਿਸ਼ਾਨੀ outcnt;            /* bytes in output buffer */
    ਬਣਤਰ outd outd;           /* output structure */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = ਅਗਲਾ_2();
    ਜੇ (ਆਖਰੀ == -1)
        ਵਾਪਸ Z_BUF_ERROR;
    ਜੇ (flags & 0x60) {
        strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"unknown lzw flags set";
        ਵਾਪਸ Z_ਅੰਕੜੇ_ERROR;
    }
    max = flags & 0x1f;
    ਜੇ (max < 9 || max > 16) {
        strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"lzw bits out of range";
        ਵਾਪਸ Z_ਅੰਕੜੇ_ERROR;
    }
    ਜੇ (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    ਬਿੱਟ = 9;
    ਨਕਾਬ = 0x1ff;
    ਸਿਰਾ = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    ਜੇ (ਅਗਲਾ_2() == -1)                       /* no compressed data is ok */
        ਵਾਪਸ Z_OK;
    final = prev = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਆਖਰੀ;          /* low 8 bits of code */
    ਜੇ (ਅਗਲਾ_2() == -1)                       /* missing a bit */
        ਵਾਪਸ Z_BUF_ERROR;
    ਜੇ (ਆਖਰੀ & 1) {                         /* code must be < 256 */
        strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid lzw code";
        ਵਾਪਸ Z_ਅੰਕੜੇ_ERROR;
    }
    rem = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਆਖਰੀ >> 1;              /* remaining 7 bits */
    ਖੱਬਾ = 7;
    chunk = ਬਿੱਟ - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    ਢੇਰ_2 = 0;
    ਲਈ (;;) {
        /* if the table will be full after this, increment the code size */
        ਜੇ (ਸਿਰਾ >= ਨਕਾਬ && ਬਿੱਟ < max) {
            FLUSHCODE();
            ਬਿੱਟ++;
            ਨਕਾਬ <<= 1;
            ਨਕਾਬ++;
        }

        /* get a code of length bits */
        ਜੇ (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = ਬਿੱਟ;
        ਕੋਡ = rem;                         /* low bits of code */
        ਜੇ (ਅਗਲਾ_2() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            ਜੇ (outcnt && ਬਾਹਰਲਾ(&outd, outbuf, outcnt)) {
                strm->ਅਗਲਾ_ਵਿੱਚ = outbuf;     /* signal write error */
                ਵਾਪਸ Z_BUF_ERROR;
            }
            ਵਾਪਸ Z_OK;
        }
        ਕੋਡ += (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਆਖਰੀ << ਖੱਬਾ;     /* middle (or high) bits of code */
        ਖੱਬਾ += 8;
        chunk--;
        ਜੇ (ਬਿੱਟ > ਖੱਬਾ) {                  /* need more bits */
            ਜੇ (ਅਗਲਾ_2() == -1)               /* can't end in middle of code */
                ਵਾਪਸ Z_BUF_ERROR;
            ਕੋਡ += (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਆਖਰੀ << ਖੱਬਾ; /* high bits of code */
            ਖੱਬਾ += 8;
            chunk--;
        }
        ਕੋਡ &= ਨਕਾਬ;                       /* mask to current code length */
        ਖੱਬਾ -= ਬਿੱਟ;                       /* number of unused bits */
        rem = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਆਖਰੀ >> (8 - ਖੱਬਾ); /* unused bits from last byte */

        /* process clear code (256) */
        ਜੇ (ਕੋਡ == 256 && flags) {
            FLUSHCODE();
            ਬਿੱਟ = 9;                       /* initialize bits and mask */
            ਨਕਾਬ = 0x1ff;
            ਸਿਰਾ = 255;                      /* empty table */
            ਜਾਰੀ;                       /* get next code */
        }

        /* special code to reuse last match */
        ਆਰਜ਼ੀ = ਕੋਡ;                        /* save the current code */
        ਜੇ (ਕੋਡ > ਸਿਰਾ) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            ਜੇ (ਕੋਡ != ਸਿਰਾ + 1 || prev > ਸਿਰਾ) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"invalid lzw code";
                ਵਾਪਸ Z_ਅੰਕੜੇ_ERROR;
            }
            ਮੇਲ[ਢੇਰ_2++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)final;
            ਕੋਡ = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = ਮੇਲ + ਢੇਰ_2;
        ਜਦੋਂ (ਕੋਡ >= 256) {
            *p++ = suffix[ਕੋਡ];
            ਕੋਡ = ਅਗੇਤਰ[ਕੋਡ];
        }
        ਢੇਰ_2 = p - ਮੇਲ;
        ਮੇਲ[ਢੇਰ_2++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)ਕੋਡ;
        final = ਕੋਡ;

        /* link new table entry */
        ਜੇ (ਸਿਰਾ < ਨਕਾਬ) {
            ਸਿਰਾ++;
            ਅਗੇਤਰ[ਸਿਰਾ] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ)prev;
            suffix[ਸਿਰਾ] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)final;
        }

        /* set previous code for next iteration */
        prev = ਆਰਜ਼ੀ;

        /* write output in forward order */
        ਜਦੋਂ (ਢੇਰ_2 > ਮਾਪ_2 - outcnt) {
            ਜਦੋਂ (outcnt < ਮਾਪ_2)
                outbuf[outcnt++] = ਮੇਲ[--ਢੇਰ_2];
            ਜੇ (ਬਾਹਰਲਾ(&outd, outbuf, outcnt)) {
                strm->ਅਗਲਾ_ਵਿੱਚ = outbuf; /* signal write error */
                ਵਾਪਸ Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = ਮੇਲ + ਢੇਰ_2;
        ਕਰੋ {
            outbuf[outcnt++] = *--p;
        } ਜਦੋਂ (p > ਮੇਲ);
        ਢੇਰ_2 = 0;

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
local ਪੂਰਨ gunpipe(z_ਧਾਰਾ *strm, ਪੂਰਨ infile, ਪੂਰਨ outfile)
{
    ਪੂਰਨ ret, ਪਹਿਲਾ, ਆਖਰੀ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ have, flags, len;
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅਗਲਾ = ਨਿਰਾ;
    ਬਣਤਰ ind ind, *indp;
    ਬਣਤਰ outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    ਪਹਿਲਾ = 1;                              /* looking for first gzip header */
    strm->ਅਗਲਾ_ਵਿੱਚ = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    ਲਈ (;;) {
        /* look for the two magic header bytes for a gzip stream */
        ਜੇ (ਅਗਲਾ_2() == -1) {
            ret = Z_OK;
            ਰੋਕੋ;                          /* empty gzip stream is ok */
        }
        ਜੇ (ਆਖਰੀ != 31 || (ਅਗਲਾ_2() != 139 && ਆਖਰੀ != 157)) {
            strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"incorrect header check";
            ret = ਪਹਿਲਾ ? Z_ਅੰਕੜੇ_ERROR : Z_ERRNO;
            ਰੋਕੋ;                          /* not a gzip or compress header */
        }
        ਪਹਿਲਾ = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        ਜੇ (ਆਖਰੀ == 157) {
            ret = lunpipe(have, ਅਗਲਾ, indp, outfile, strm);
            ਰੋਕੋ;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        ਜੇ (ਅਗਲਾ_2() != 8) {                  /* only deflate method allowed */
            ਜੇ (ਆਖਰੀ == -1) ਰੋਕੋ;
            strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"unknown compression method";
            ret = Z_ਅੰਕੜੇ_ERROR;
            ਰੋਕੋ;
        }
        flags = ਅਗਲਾ_2();                     /* header flags */
        ਅਗਲਾ_2();                             /* discard mod time, xflgs, os */
        ਅਗਲਾ_2();
        ਅਗਲਾ_2();
        ਅਗਲਾ_2();
        ਅਗਲਾ_2();
        ਅਗਲਾ_2();
        ਜੇ (ਆਖਰੀ == -1) ਰੋਕੋ;
        ਜੇ (flags & 0xe0) {
            strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"unknown header flags set";
            ret = Z_ਅੰਕੜੇ_ERROR;
            ਰੋਕੋ;
        }
        ਜੇ (flags & 4) {                    /* extra field */
            len = ਅਗਲਾ_2();
            len += (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਅਗਲਾ_2()) << 8;
            ਜੇ (ਆਖਰੀ == -1) ਰੋਕੋ;
            ਜਦੋਂ (len > have) {
                len -= have;
                have = 0;
                ਜੇ (ਅਗਲਾ_2() == -1) ਰੋਕੋ;
                len--;
            }
            ਜੇ (ਆਖਰੀ == -1) ਰੋਕੋ;
            have -= len;
            ਅਗਲਾ += len;
        }
        ਜੇ (flags & 8)                      /* file name */
            ਜਦੋਂ (ਅਗਲਾ_2() != 0 && ਆਖਰੀ != -1)
                ;
        ਜੇ (flags & 16)                     /* comment */
            ਜਦੋਂ (ਅਗਲਾ_2() != 0 && ਆਖਰੀ != -1)
                ;
        ਜੇ (flags & 2) {                    /* header crc */
            ਅਗਲਾ_2();
            ਅਗਲਾ_2();
        }
        ਜੇ (ਆਖਰੀ == -1) ਰੋਕੋ;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0);
        outd.ਕੁੱਲ = 0;

        /* decompress data to output */
        strm->ਅਗਲਾ_ਵਿੱਚ = ਅਗਲਾ;
        strm->avail_ਵਿੱਚ = have;
        ret = inflate_ਪਿੱਠ(strm, ਵਿੱਚ, indp, ਬਾਹਰਲਾ, &outd);
        ਜੇ (ret != Z_ਧਾਰਾ_ਸਿਰਾ) ਰੋਕੋ;
        ਅਗਲਾ = strm->ਅਗਲਾ_ਵਿੱਚ;
        have = strm->avail_ਵਿੱਚ;
        strm->ਅਗਲਾ_ਵਿੱਚ = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        ਜੇ (ਅਗਲਾ_2() != (ਪੂਰਨ)(outd.ਚੱਕਰੀ_ਜਾਂਚ & 0xff) ||
            ਅਗਲਾ_2() != (ਪੂਰਨ)((outd.ਚੱਕਰੀ_ਜਾਂਚ >> 8) & 0xff) ||
            ਅਗਲਾ_2() != (ਪੂਰਨ)((outd.ਚੱਕਰੀ_ਜਾਂਚ >> 16) & 0xff) ||
            ਅਗਲਾ_2() != (ਪੂਰਨ)((outd.ਚੱਕਰੀ_ਜਾਂਚ >> 24) & 0xff)) {
            /* crc error */
            ਜੇ (ਆਖਰੀ != -1) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"incorrect data check";
                ret = Z_ਅੰਕੜੇ_ERROR;
            }
            ਰੋਕੋ;
        }
        ਜੇ (ਅਗਲਾ_2() != (ਪੂਰਨ)(outd.ਕੁੱਲ & 0xff) ||
            ਅਗਲਾ_2() != (ਪੂਰਨ)((outd.ਕੁੱਲ >> 8) & 0xff) ||
            ਅਗਲਾ_2() != (ਪੂਰਨ)((outd.ਕੁੱਲ >> 16) & 0xff) ||
            ਅਗਲਾ_2() != (ਪੂਰਨ)((outd.ਕੁੱਲ >> 24) & 0xff)) {
            /* length error */
            ਜੇ (ਆਖਰੀ != -1) {
                strm->ਸੁਨੇਹਾ = (z_const ਅੱਖਰ *)"incorrect length check";
                ret = Z_ਅੰਕੜੇ_ERROR;
            }
            ਰੋਕੋ;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    ਵਾਪਸ ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local ਖਾਲੀ copymeta(ਅੱਖਰ *from, ਅੱਖਰ *ਮੰਜ਼ਿਲ)
{
    ਬਣਤਰ stat was;
    ਬਣਤਰ utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    ਜੇ (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        ਵਾਪਸ;

    /* set to's mode bits, ignore errors */
    (ਖਾਲੀ)chmod(ਮੰਜ਼ਿਲ, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (ਖਾਲੀ)chown(ਮੰਜ਼ਿਲ, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (ਖਾਲੀ)utime(ਮੰਜ਼ਿਲ, &when);
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
local ਪੂਰਨ gunzip(z_ਧਾਰਾ *strm, ਅੱਖਰ *inname, ਅੱਖਰ *outname, ਪੂਰਨ test)
{
    ਪੂਰਨ ret;
    ਪੂਰਨ infile, outfile;

    /* open files */
    ਜੇ (inname == ਨਿਰਾ || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    ਨਹੀਂਤਾਂ {
        infile = open(inname, O_RDONLY, 0);
        ਜੇ (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            ਵਾਪਸ 0;
        }
    }
    ਜੇ (test)
        outfile = -1;
    ਨਹੀਂਤਾਂ ਜੇ (outname == ਨਿਰਾ || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    ਨਹੀਂਤਾਂ {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        ਜੇ (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            ਵਾਪਸ 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    ਜੇ (outfile > 2) close(outfile);
    ਜੇ (infile > 2) close(infile);

    /* interpret result */
    ਚੋਣ (ret) {
    ਹਾਲਤ Z_OK:
    ਹਾਲਤ Z_ERRNO:
        ਜੇ (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        ਜੇ (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        ਰੋਕੋ;
    ਹਾਲਤ Z_ਅੰਕੜੇ_ERROR:
        ਜੇ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->ਸੁਨੇਹਾ);
        ਰੋਕੋ;
    ਹਾਲਤ Z_MEM_ERROR:
        ਜੇ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        ਵਾਪਸ 1;
    ਹਾਲਤ Z_BUF_ERROR:
        ਜੇ (outfile > 2) unlink(outname);
        ਜੇ (strm->ਅਗਲਾ_ਵਿੱਚ != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        ਨਹੀਂਤਾਂ ਜੇ (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        ਨਹੀਂਤਾਂ {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        ਰੋਕੋ;
    ਮੂਲ:
        ਜੇ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        ਵਾਪਸ 1;
    }
    ਵਾਪਸ 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
    ਪੂਰਨ ret, len, test;
    ਅੱਖਰ *outname;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਖਿੜਕੀ;
    z_ਧਾਰਾ strm;

    /* initialize inflateBack state for repeated use */
    ਖਿੜਕੀ = ਮੇਲ;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflate_ਪਿੱਠ_Init(&strm, 15, ਖਿੜਕੀ);
    ਜੇ (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        ਵਾਪਸ 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    ਜੇ (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        ਵਾਪਸ 0;
    }
    ਜੇ (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    ਜੇ (argc)
        ਕਰੋ {
            ਜੇ (test)
                outname = ਨਿਰਾ;
            ਨਹੀਂਤਾਂ {
                len = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(*argv);
                ਜੇ (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                ਨਹੀਂਤਾਂ ਜੇ (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                ਨਹੀਂਤਾਂ {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    ਜਾਰੀ;
                }
                outname = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len + 1);
                ਜੇ (outname == ਨਿਰਾ) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    ਰੋਕੋ;
                }
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            ਜੇ (outname != ਨਿਰਾ) ਮੁਕਤ(outname);
            ਜੇ (ret) ਰੋਕੋ;
        } ਜਦੋਂ (argv++, --argc);
    ਨਹੀਂਤਾਂ
        ret = gunzip(&strm, ਨਿਰਾ, ਨਿਰਾ, test);

    /* clean up */
    inflate_ਪਿੱਠ_ਸਿਰਾ(&strm);
    ਵਾਪਸ ret;
}
