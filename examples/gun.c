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
#ಅನ್ತರ್ಭಾವ <stdio.h>          /* fprintf() */
#ಅನ್ತರ್ಭಾವ <stdlib.h>         /* malloc(), free() */
#ಅನ್ತರ್ಭಾವ <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#ಅನ್ತರ್ಭಾವ <errno.h>          /* errno */
#ಅನ್ತರ್ಭಾವ <fcntl.h>          /* open() */
#ಅನ್ತರ್ಭಾವ <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>       /* stat(), chmod() */
#ಅನ್ತರ್ಭಾವ <utime.h>          /* utime() */
#ಅನ್ತರ್ಭಾವ "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#ವ್ಯಾಖ್ಯಾನಿಸು local ಸ್ಥಿರ

/* buffer constants */
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_3 32768U         /* input and output buffer sizes */
#ವ್ಯಾಖ್ಯಾನಿಸು PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
ರಚನೆ ind {
    ಪೂರ್ಣಾಂಕ infile;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local ಚಿಹ್ನೆರಹಿತ ಒಳ(ಶೂನ್ಯ *ಒಳ_desc, z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **buf)
{
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ len;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮುಂದಿನ;
    ರಚನೆ ind *me = (ರಚನೆ ind *)ಒಳ_desc;

    ಮುಂದಿನ = me->inbuf;
    *buf = ಮುಂದಿನ;
    len = 0;
    ಮಾಡು {
        ret = PIECE;
        ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)ret > ಗಾತ್ರ_3 - len)
            ret = (ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ_3 - len);
        ret = (ಪೂರ್ಣಾಂಕ)ಪಠ(me->infile, ಮುಂದಿನ, ret);
        ಆದರೆ (ret == -1) {
            len = 0;
            ನಿಲ್ಲಿಸು;
        }
        ಮುಂದಿನ += ret;
        len += ret;
    } ಆಗ (ret != 0 && len < ಗಾತ್ರ_3);
    ಹಿಂತಿರುಗಿ len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
ರಚನೆ outd {
    ಪೂರ್ಣಾಂಕ outfile;
    ಪೂರ್ಣಾಂಕ check;                  /* true if checking crc and total */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಒಟ್ಟು;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local ಪೂರ್ಣಾಂಕ ಹೊರಹೋಗು(ಶೂನ್ಯ *ಹೊರಹೋಗು_desc, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ len)
{
    ಪೂರ್ಣಾಂಕ ret;
    ರಚನೆ outd *me = (ರಚನೆ outd *)ಹೊರಹೋಗು_desc;

    ಆದರೆ (me->check) {
        me->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(me->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, buf, len);
        me->ಒಟ್ಟು += len;
    }
    ಆದರೆ (me->outfile != -1)
        ಮಾಡು {
            ret = PIECE;
            ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)ret > len)
                ret = (ಪೂರ್ಣಾಂಕ)len;
            ret = (ಪೂರ್ಣಾಂಕ)ಲಿಖ(me->outfile, buf, ret);
            ಆದರೆ (ret == -1)
                ಹಿಂತಿರುಗಿ 1;
            buf += ret;
            len -= ret;
        } ಆಗ (len != 0);
    ಹಿಂತಿರುಗಿ 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಂದಿನ_2() (have ? 0 : (have = ಒಳ(indp, &ಮುಂದಿನ)), \
                ಕೊನೆಯ = have ? (have--, (ಪೂರ್ಣಾಂಕ)(*ಮುಂದಿನ++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ inbuf[ಗಾತ್ರ_3];              /* input buffer */
ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ outbuf[ಗಾತ್ರ_3];             /* output buffer */
ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಪೂರ್ವಪ್ರತ್ಯಯ[65536];           /* index to LZW prefix string */
ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ suffix[65536];            /* one-character LZW suffix */
ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಹೊಂದಿಕೆ[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#ವ್ಯಾಖ್ಯಾನಿಸು FLUSHCODE() \
    ಮಾಡು { \
        ಎಡ = 0; \
        rem = 0; \
        ಆದರೆ (chunk > have) { \
            chunk -= have; \
            have = 0; \
            ಆದರೆ (ಮುಂದಿನ_2() == -1) \
                ನಿಲ್ಲಿಸು; \
            chunk--; \
            ಆದರೆ (chunk > have) { \
                chunk = have = 0; \
                ನಿಲ್ಲಿಸು; \
            } \
        } \
        have -= chunk; \
        ಮುಂದಿನ += chunk; \
        chunk = 0; \
    } ಆಗ (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local ಪೂರ್ಣಾಂಕ lunpipe(ಚಿಹ್ನೆರಹಿತ have, z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮುಂದಿನ, ರಚನೆ ind *indp,
                  ಪೂರ್ಣಾಂಕ outfile, z_ಹರಿವು *strm)
{
    ಪೂರ್ಣಾಂಕ ಕೊನೆಯ;                   /* last byte read by NEXT(), or -1 if EOF */
    ಚಿಹ್ನೆರಹಿತ chunk;             /* bytes left in current chunk */
    ಪೂರ್ಣಾಂಕ ಎಡ;                   /* bits left in rem */
    ಚಿಹ್ನೆರಹಿತ rem;               /* unused bits from input */
    ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು;                   /* current bits per code */
    ಚಿಹ್ನೆರಹಿತ ಸಂಕೇತ_2;              /* code, table traversal index */
    ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ;              /* mask for current bits codes */
    ಪೂರ್ಣಾಂಕ max;                    /* maximum bits per code for this stream */
    ಚಿಹ್ನೆರಹಿತ flags;             /* compress flags, then block compress flag */
    ಚಿಹ್ನೆರಹಿತ ಕೊನೆ;               /* last valid entry in prefix/suffix tables */
    ಚಿಹ್ನೆರಹಿತ ತಾತ್ಕಾಲಿಕ;              /* current code */
    ಚಿಹ್ನೆರಹಿತ prev;              /* previous code */
    ಚಿಹ್ನೆರಹಿತ final;             /* last character written for previous code */
    ಚಿಹ್ನೆರಹಿತ ರಾಶಿ_2;             /* next position for reversed string */
    ಚಿಹ್ನೆರಹಿತ outcnt;            /* bytes in output buffer */
    ರಚನೆ outd outd;           /* output structure */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = ಮುಂದಿನ_2();
    ಆದರೆ (ಕೊನೆಯ == -1)
        ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
    ಆದರೆ (flags & 0x60) {
        strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"unknown lzw flags set";
        ಹಿಂತಿರುಗಿ Z_ದತ್ತಾಂಶ_ERROR;
    }
    max = flags & 0x1f;
    ಆದರೆ (max < 9 || max > 16) {
        strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"lzw bits out of range";
        ಹಿಂತಿರುಗಿ Z_ದತ್ತಾಂಶ_ERROR;
    }
    ಆದರೆ (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    ಬಿಟ್ಟುಗಳು = 9;
    ಮುಖವಾಡ = 0x1ff;
    ಕೊನೆ = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    ಆದರೆ (ಮುಂದಿನ_2() == -1)                       /* no compressed data is ok */
        ಹಿಂತಿರುಗಿ Z_OK;
    final = prev = (ಚಿಹ್ನೆರಹಿತ)ಕೊನೆಯ;          /* low 8 bits of code */
    ಆದರೆ (ಮುಂದಿನ_2() == -1)                       /* missing a bit */
        ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
    ಆದರೆ (ಕೊನೆಯ & 1) {                         /* code must be < 256 */
        strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid lzw code";
        ಹಿಂತಿರುಗಿ Z_ದತ್ತಾಂಶ_ERROR;
    }
    rem = (ಚಿಹ್ನೆರಹಿತ)ಕೊನೆಯ >> 1;              /* remaining 7 bits */
    ಎಡ = 7;
    chunk = ಬಿಟ್ಟುಗಳು - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    ರಾಶಿ_2 = 0;
    ಗಾಗಿ (;;) {
        /* if the table will be full after this, increment the code size */
        ಆದರೆ (ಕೊನೆ >= ಮುಖವಾಡ && ಬಿಟ್ಟುಗಳು < max) {
            FLUSHCODE();
            ಬಿಟ್ಟುಗಳು++;
            ಮುಖವಾಡ <<= 1;
            ಮುಖವಾಡ++;
        }

        /* get a code of length bits */
        ಆದರೆ (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = ಬಿಟ್ಟುಗಳು;
        ಸಂಕೇತ_2 = rem;                         /* low bits of code */
        ಆದರೆ (ಮುಂದಿನ_2() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            ಆದರೆ (outcnt && ಹೊರಹೋಗು(&outd, outbuf, outcnt)) {
                strm->ಮುಂದಿನ_ಒಳ = outbuf;     /* signal write error */
                ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
            }
            ಹಿಂತಿರುಗಿ Z_OK;
        }
        ಸಂಕೇತ_2 += (ಚಿಹ್ನೆರಹಿತ)ಕೊನೆಯ << ಎಡ;     /* middle (or high) bits of code */
        ಎಡ += 8;
        chunk--;
        ಆದರೆ (ಬಿಟ್ಟುಗಳು > ಎಡ) {                  /* need more bits */
            ಆದರೆ (ಮುಂದಿನ_2() == -1)               /* can't end in middle of code */
                ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
            ಸಂಕೇತ_2 += (ಚಿಹ್ನೆರಹಿತ)ಕೊನೆಯ << ಎಡ; /* high bits of code */
            ಎಡ += 8;
            chunk--;
        }
        ಸಂಕೇತ_2 &= ಮುಖವಾಡ;                       /* mask to current code length */
        ಎಡ -= ಬಿಟ್ಟುಗಳು;                       /* number of unused bits */
        rem = (ಚಿಹ್ನೆರಹಿತ)ಕೊನೆಯ >> (8 - ಎಡ); /* unused bits from last byte */

        /* process clear code (256) */
        ಆದರೆ (ಸಂಕೇತ_2 == 256 && flags) {
            FLUSHCODE();
            ಬಿಟ್ಟುಗಳು = 9;                       /* initialize bits and mask */
            ಮುಖವಾಡ = 0x1ff;
            ಕೊನೆ = 255;                      /* empty table */
            ಮುಂದುವರಿ;                       /* get next code */
        }

        /* special code to reuse last match */
        ತಾತ್ಕಾಲಿಕ = ಸಂಕೇತ_2;                        /* save the current code */
        ಆದರೆ (ಸಂಕೇತ_2 > ಕೊನೆ) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            ಆದರೆ (ಸಂಕೇತ_2 != ಕೊನೆ + 1 || prev > ಕೊನೆ) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"invalid lzw code";
                ಹಿಂತಿರುಗಿ Z_ದತ್ತಾಂಶ_ERROR;
            }
            ಹೊಂದಿಕೆ[ರಾಶಿ_2++] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)final;
            ಸಂಕೇತ_2 = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = ಹೊಂದಿಕೆ + ರಾಶಿ_2;
        ಆಗ (ಸಂಕೇತ_2 >= 256) {
            *p++ = suffix[ಸಂಕೇತ_2];
            ಸಂಕೇತ_2 = ಪೂರ್ವಪ್ರತ್ಯಯ[ಸಂಕೇತ_2];
        }
        ರಾಶಿ_2 = p - ಹೊಂದಿಕೆ;
        ಹೊಂದಿಕೆ[ರಾಶಿ_2++] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)ಸಂಕೇತ_2;
        final = ಸಂಕೇತ_2;

        /* link new table entry */
        ಆದರೆ (ಕೊನೆ < ಮುಖವಾಡ) {
            ಕೊನೆ++;
            ಪೂರ್ವಪ್ರತ್ಯಯ[ಕೊನೆ] = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ)prev;
            suffix[ಕೊನೆ] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)final;
        }

        /* set previous code for next iteration */
        prev = ತಾತ್ಕಾಲಿಕ;

        /* write output in forward order */
        ಆಗ (ರಾಶಿ_2 > ಗಾತ್ರ_3 - outcnt) {
            ಆಗ (outcnt < ಗಾತ್ರ_3)
                outbuf[outcnt++] = ಹೊಂದಿಕೆ[--ರಾಶಿ_2];
            ಆದರೆ (ಹೊರಹೋಗು(&outd, outbuf, outcnt)) {
                strm->ಮುಂದಿನ_ಒಳ = outbuf; /* signal write error */
                ಹಿಂತಿರುಗಿ Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = ಹೊಂದಿಕೆ + ರಾಶಿ_2;
        ಮಾಡು {
            outbuf[outcnt++] = *--p;
        } ಆಗ (p > ಹೊಂದಿಕೆ);
        ರಾಶಿ_2 = 0;

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
local ಪೂರ್ಣಾಂಕ gunpipe(z_ಹರಿವು *strm, ಪೂರ್ಣಾಂಕ infile, ಪೂರ್ಣಾಂಕ outfile)
{
    ಪೂರ್ಣಾಂಕ ret, ಮೊದಲ, ಕೊನೆಯ;
    ಚಿಹ್ನೆರಹಿತ have, flags, len;
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮುಂದಿನ = ಖಾಲಿ;
    ರಚನೆ ind ind, *indp;
    ರಚನೆ outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    ಮೊದಲ = 1;                              /* looking for first gzip header */
    strm->ಮುಂದಿನ_ಒಳ = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    ಗಾಗಿ (;;) {
        /* look for the two magic header bytes for a gzip stream */
        ಆದರೆ (ಮುಂದಿನ_2() == -1) {
            ret = Z_OK;
            ನಿಲ್ಲಿಸು;                          /* empty gzip stream is ok */
        }
        ಆದರೆ (ಕೊನೆಯ != 31 || (ಮುಂದಿನ_2() != 139 && ಕೊನೆಯ != 157)) {
            strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"incorrect header check";
            ret = ಮೊದಲ ? Z_ದತ್ತಾಂಶ_ERROR : Z_ERRNO;
            ನಿಲ್ಲಿಸು;                          /* not a gzip or compress header */
        }
        ಮೊದಲ = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        ಆದರೆ (ಕೊನೆಯ == 157) {
            ret = lunpipe(have, ಮುಂದಿನ, indp, outfile, strm);
            ನಿಲ್ಲಿಸು;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        ಆದರೆ (ಮುಂದಿನ_2() != 8) {                  /* only deflate method allowed */
            ಆದರೆ (ಕೊನೆಯ == -1) ನಿಲ್ಲಿಸು;
            strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"unknown compression method";
            ret = Z_ದತ್ತಾಂಶ_ERROR;
            ನಿಲ್ಲಿಸು;
        }
        flags = ಮುಂದಿನ_2();                     /* header flags */
        ಮುಂದಿನ_2();                             /* discard mod time, xflgs, os */
        ಮುಂದಿನ_2();
        ಮುಂದಿನ_2();
        ಮುಂದಿನ_2();
        ಮುಂದಿನ_2();
        ಮುಂದಿನ_2();
        ಆದರೆ (ಕೊನೆಯ == -1) ನಿಲ್ಲಿಸು;
        ಆದರೆ (flags & 0xe0) {
            strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"unknown header flags set";
            ret = Z_ದತ್ತಾಂಶ_ERROR;
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (flags & 4) {                    /* extra field */
            len = ಮುಂದಿನ_2();
            len += (ಚಿಹ್ನೆರಹಿತ)(ಮುಂದಿನ_2()) << 8;
            ಆದರೆ (ಕೊನೆಯ == -1) ನಿಲ್ಲಿಸು;
            ಆಗ (len > have) {
                len -= have;
                have = 0;
                ಆದರೆ (ಮುಂದಿನ_2() == -1) ನಿಲ್ಲಿಸು;
                len--;
            }
            ಆದರೆ (ಕೊನೆಯ == -1) ನಿಲ್ಲಿಸು;
            have -= len;
            ಮುಂದಿನ += len;
        }
        ಆದರೆ (flags & 8)                      /* file name */
            ಆಗ (ಮುಂದಿನ_2() != 0 && ಕೊನೆಯ != -1)
                ;
        ಆದರೆ (flags & 16)                     /* comment */
            ಆಗ (ಮುಂದಿನ_2() != 0 && ಕೊನೆಯ != -1)
                ;
        ಆದರೆ (flags & 2) {                    /* header crc */
            ಮುಂದಿನ_2();
            ಮುಂದಿನ_2();
        }
        ಆದರೆ (ಕೊನೆಯ == -1) ನಿಲ್ಲಿಸು;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0);
        outd.ಒಟ್ಟು = 0;

        /* decompress data to output */
        strm->ಮುಂದಿನ_ಒಳ = ಮುಂದಿನ;
        strm->avail_ಒಳ = have;
        ret = inflate_ಹಿಂಭಾಗ(strm, ಒಳ, indp, ಹೊರಹೋಗು, &outd);
        ಆದರೆ (ret != Z_ಹರಿವು_ಕೊನೆ) ನಿಲ್ಲಿಸು;
        ಮುಂದಿನ = strm->ಮುಂದಿನ_ಒಳ;
        have = strm->avail_ಒಳ;
        strm->ಮುಂದಿನ_ಒಳ = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        ಆದರೆ (ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)(outd.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ & 0xff) ||
            ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)((outd.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) & 0xff) ||
            ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)((outd.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 16) & 0xff) ||
            ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)((outd.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 24) & 0xff)) {
            /* crc error */
            ಆದರೆ (ಕೊನೆಯ != -1) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"incorrect data check";
                ret = Z_ದತ್ತಾಂಶ_ERROR;
            }
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)(outd.ಒಟ್ಟು & 0xff) ||
            ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)((outd.ಒಟ್ಟು >> 8) & 0xff) ||
            ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)((outd.ಒಟ್ಟು >> 16) & 0xff) ||
            ಮುಂದಿನ_2() != (ಪೂರ್ಣಾಂಕ)((outd.ಒಟ್ಟು >> 24) & 0xff)) {
            /* length error */
            ಆದರೆ (ಕೊನೆಯ != -1) {
                strm->ಸಂದೇಶ = (z_const ಅಕ್ಷರ *)"incorrect length check";
                ret = Z_ದತ್ತಾಂಶ_ERROR;
            }
            ನಿಲ್ಲಿಸು;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    ಹಿಂತಿರುಗಿ ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local ಶೂನ್ಯ copymeta(ಅಕ್ಷರ *from, ಅಕ್ಷರ *ಗಮ್ಯ)
{
    ರಚನೆ stat was;
    ರಚನೆ utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    ಆದರೆ (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        ಹಿಂತಿರುಗಿ;

    /* set to's mode bits, ignore errors */
    (ಶೂನ್ಯ)chmod(ಗಮ್ಯ, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (ಶೂನ್ಯ)chown(ಗಮ್ಯ, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (ಶೂನ್ಯ)utime(ಗಮ್ಯ, &when);
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
local ಪೂರ್ಣಾಂಕ gunzip(z_ಹರಿವು *strm, ಅಕ್ಷರ *inname, ಅಕ್ಷರ *outname, ಪೂರ್ಣಾಂಕ test)
{
    ಪೂರ್ಣಾಂಕ ret;
    ಪೂರ್ಣಾಂಕ infile, outfile;

    /* open files */
    ಆದರೆ (inname == ಖಾಲಿ || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    ಇಲ್ಲದಿದ್ದರೆ {
        infile = open(inname, O_RDONLY, 0);
        ಆದರೆ (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            ಹಿಂತಿರುಗಿ 0;
        }
    }
    ಆದರೆ (test)
        outfile = -1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (outname == ಖಾಲಿ || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    ಇಲ್ಲದಿದ್ದರೆ {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        ಆದರೆ (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            ಹಿಂತಿರುಗಿ 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    ಆದರೆ (outfile > 2) close(outfile);
    ಆದರೆ (infile > 2) close(infile);

    /* interpret result */
    ಆಯ್ಕೆ (ret) {
    ಸ್ಥಿತಿ Z_OK:
    ಸ್ಥಿತಿ Z_ERRNO:
        ಆದರೆ (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        ಆದರೆ (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Z_ದತ್ತಾಂಶ_ERROR:
        ಆದರೆ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->ಸಂದೇಶ);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Z_MEM_ERROR:
        ಆದರೆ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ Z_BUF_ERROR:
        ಆದರೆ (outfile > 2) unlink(outname);
        ಆದರೆ (strm->ಮುಂದಿನ_ಒಳ != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಆದರೆ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        ಹಿಂತಿರುಗಿ 1;
    }
    ಹಿಂತಿರುಗಿ 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
    ಪೂರ್ಣಾಂಕ ret, len, test;
    ಅಕ್ಷರ *outname;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಕಿಟಕಿ;
    z_ಹರಿವು strm;

    /* initialize inflateBack state for repeated use */
    ಕಿಟಕಿ = ಹೊಂದಿಕೆ;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflate_ಹಿಂಭಾಗ_Init(&strm, 15, ಕಿಟಕಿ);
    ಆದರೆ (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        ಹಿಂತಿರುಗಿ 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    ಆದರೆ (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        ಹಿಂತಿರುಗಿ 0;
    }
    ಆದರೆ (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    ಆದರೆ (argc)
        ಮಾಡು {
            ಆದರೆ (test)
                outname = ಖಾಲಿ;
            ಇಲ್ಲದಿದ್ದರೆ {
                len = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*argv);
                ಆದರೆ (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                ಇಲ್ಲದಿದ್ದರೆ {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    ಮುಂದುವರಿ;
                }
                outname = ಸ್ಮರಣೆ_ಕೊಡು(len + 1);
                ಆದರೆ (outname == ಖಾಲಿ) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    ನಿಲ್ಲಿಸು;
                }
                ಸ್ಮರಣೆ_ನಕಲು(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            ಆದರೆ (outname != ಖಾಲಿ) ಬಿಡುಗಡೆ(outname);
            ಆದರೆ (ret) ನಿಲ್ಲಿಸು;
        } ಆಗ (argv++, --argc);
    ಇಲ್ಲದಿದ್ದರೆ
        ret = gunzip(&strm, ಖಾಲಿ, ಖಾಲಿ, test);

    /* clean up */
    inflate_ಹಿಂಭಾಗ_ಕೊನೆ(&strm);
    ಹಿಂತಿರುಗಿ ret;
}
