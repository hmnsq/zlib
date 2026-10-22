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
#అన్తర్భావ <stdio.h>          /* fprintf() */
#అన్తర్భావ <stdlib.h>         /* malloc(), free() */
#అన్తర్భావ <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#అన్తర్భావ <errno.h>          /* errno */
#అన్తర్భావ <fcntl.h>          /* open() */
#అన్తర్భావ <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>       /* stat(), chmod() */
#అన్తర్భావ <utime.h>          /* utime() */
#అన్తర్భావ "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#నిర్వచించు local స్థిర

/* buffer constants */
#నిర్వచించు కొలత_2 32768U         /* input and output buffer sizes */
#నిర్వచించు PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
నిర్మాణం ind {
    పూర్ణాంకం infile;
    గుర్తులేని అక్షరం *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local గుర్తులేని లోపల(శూన్య *లోపల_desc, z_const గుర్తులేని అక్షరం **buf)
{
    పూర్ణాంకం ret;
    గుర్తులేని len;
    గుర్తులేని అక్షరం *తదుపరి;
    నిర్మాణం ind *me = (నిర్మాణం ind *)లోపల_desc;

    తదుపరి = me->inbuf;
    *buf = తదుపరి;
    len = 0;
    చేయి {
        ret = PIECE;
        ఒకవేళ ((గుర్తులేని)ret > కొలత_2 - len)
            ret = (పూర్ణాంకం)(కొలత_2 - len);
        ret = (పూర్ణాంకం)పఠ(me->infile, తదుపరి, ret);
        ఒకవేళ (ret == -1) {
            len = 0;
            ఆపు;
        }
        తదుపరి += ret;
        len += ret;
    } అయితే (ret != 0 && len < కొలత_2);
    తిరిగి len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
నిర్మాణం outd {
    పూర్ణాంకం outfile;
    పూర్ణాంకం check;                  /* true if checking crc and total */
    గుర్తులేని దీర్ఘ చక్రీయ_తనిఖీ;
    గుర్తులేని దీర్ఘ మొత్తం;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local పూర్ణాంకం బహిర్గతం(శూన్య *బహిర్గతం_desc, గుర్తులేని అక్షరం *buf, గుర్తులేని len)
{
    పూర్ణాంకం ret;
    నిర్మాణం outd *me = (నిర్మాణం outd *)బహిర్గతం_desc;

    ఒకవేళ (me->check) {
        me->చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_2(me->చక్రీయ_తనిఖీ, buf, len);
        me->మొత్తం += len;
    }
    ఒకవేళ (me->outfile != -1)
        చేయి {
            ret = PIECE;
            ఒకవేళ ((గుర్తులేని)ret > len)
                ret = (పూర్ణాంకం)len;
            ret = (పూర్ణాంకం)లిఖ(me->outfile, buf, ret);
            ఒకవేళ (ret == -1)
                తిరిగి 1;
            buf += ret;
            len -= ret;
        } అయితే (len != 0);
    తిరిగి 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#నిర్వచించు తదుపరి_2() (have ? 0 : (have = లోపల(indp, &తదుపరి)), \
                చివరి = have ? (have--, (పూర్ణాంకం)(*తదుపరి++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
గుర్తులేని అక్షరం inbuf[కొలత_2];              /* input buffer */
గుర్తులేని అక్షరం outbuf[కొలత_2];             /* output buffer */
గుర్తులేని పొట్టి ఉపసర్గ[65536];           /* index to LZW prefix string */
గుర్తులేని అక్షరం suffix[65536];            /* one-character LZW suffix */
గుర్తులేని అక్షరం సరిపోలిక[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#నిర్వచించు FLUSHCODE() \
    చేయి { \
        ఎడమ = 0; \
        rem = 0; \
        ఒకవేళ (chunk > have) { \
            chunk -= have; \
            have = 0; \
            ఒకవేళ (తదుపరి_2() == -1) \
                ఆపు; \
            chunk--; \
            ఒకవేళ (chunk > have) { \
                chunk = have = 0; \
                ఆపు; \
            } \
        } \
        have -= chunk; \
        తదుపరి += chunk; \
        chunk = 0; \
    } అయితే (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local పూర్ణాంకం lunpipe(గుర్తులేని have, z_const గుర్తులేని అక్షరం *తదుపరి, నిర్మాణం ind *indp,
                  పూర్ణాంకం outfile, z_ప్రవాహం *strm)
{
    పూర్ణాంకం చివరి;                   /* last byte read by NEXT(), or -1 if EOF */
    గుర్తులేని chunk;             /* bytes left in current chunk */
    పూర్ణాంకం ఎడమ;                   /* bits left in rem */
    గుర్తులేని rem;               /* unused bits from input */
    పూర్ణాంకం బిట్లు;                   /* current bits per code */
    గుర్తులేని సంకేతం_2;              /* code, table traversal index */
    గుర్తులేని ముసుగు;              /* mask for current bits codes */
    పూర్ణాంకం max;                    /* maximum bits per code for this stream */
    గుర్తులేని flags;             /* compress flags, then block compress flag */
    గుర్తులేని చివర;               /* last valid entry in prefix/suffix tables */
    గుర్తులేని తాత్కాలిక;              /* current code */
    గుర్తులేని prev;              /* previous code */
    గుర్తులేని final;             /* last character written for previous code */
    గుర్తులేని దొంతర;             /* next position for reversed string */
    గుర్తులేని outcnt;            /* bytes in output buffer */
    నిర్మాణం outd outd;           /* output structure */
    గుర్తులేని అక్షరం *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = తదుపరి_2();
    ఒకవేళ (చివరి == -1)
        తిరిగి Z_BUF_ERROR;
    ఒకవేళ (flags & 0x60) {
        strm->సందేశం = (z_const అక్షరం *)"unknown lzw flags set";
        తిరిగి Z_దత్తాంశం_ERROR;
    }
    max = flags & 0x1f;
    ఒకవేళ (max < 9 || max > 16) {
        strm->సందేశం = (z_const అక్షరం *)"lzw bits out of range";
        తిరిగి Z_దత్తాంశం_ERROR;
    }
    ఒకవేళ (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    బిట్లు = 9;
    ముసుగు = 0x1ff;
    చివర = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    ఒకవేళ (తదుపరి_2() == -1)                       /* no compressed data is ok */
        తిరిగి Z_OK;
    final = prev = (గుర్తులేని)చివరి;          /* low 8 bits of code */
    ఒకవేళ (తదుపరి_2() == -1)                       /* missing a bit */
        తిరిగి Z_BUF_ERROR;
    ఒకవేళ (చివరి & 1) {                         /* code must be < 256 */
        strm->సందేశం = (z_const అక్షరం *)"invalid lzw code";
        తిరిగి Z_దత్తాంశం_ERROR;
    }
    rem = (గుర్తులేని)చివరి >> 1;              /* remaining 7 bits */
    ఎడమ = 7;
    chunk = బిట్లు - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (గుర్తులేని అక్షరం)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    దొంతర = 0;
    కొరకు (;;) {
        /* if the table will be full after this, increment the code size */
        ఒకవేళ (చివర >= ముసుగు && బిట్లు < max) {
            FLUSHCODE();
            బిట్లు++;
            ముసుగు <<= 1;
            ముసుగు++;
        }

        /* get a code of length bits */
        ఒకవేళ (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = బిట్లు;
        సంకేతం_2 = rem;                         /* low bits of code */
        ఒకవేళ (తదుపరి_2() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            ఒకవేళ (outcnt && బహిర్గతం(&outd, outbuf, outcnt)) {
                strm->తదుపరి_లోపల = outbuf;     /* signal write error */
                తిరిగి Z_BUF_ERROR;
            }
            తిరిగి Z_OK;
        }
        సంకేతం_2 += (గుర్తులేని)చివరి << ఎడమ;     /* middle (or high) bits of code */
        ఎడమ += 8;
        chunk--;
        ఒకవేళ (బిట్లు > ఎడమ) {                  /* need more bits */
            ఒకవేళ (తదుపరి_2() == -1)               /* can't end in middle of code */
                తిరిగి Z_BUF_ERROR;
            సంకేతం_2 += (గుర్తులేని)చివరి << ఎడమ; /* high bits of code */
            ఎడమ += 8;
            chunk--;
        }
        సంకేతం_2 &= ముసుగు;                       /* mask to current code length */
        ఎడమ -= బిట్లు;                       /* number of unused bits */
        rem = (గుర్తులేని)చివరి >> (8 - ఎడమ); /* unused bits from last byte */

        /* process clear code (256) */
        ఒకవేళ (సంకేతం_2 == 256 && flags) {
            FLUSHCODE();
            బిట్లు = 9;                       /* initialize bits and mask */
            ముసుగు = 0x1ff;
            చివర = 255;                      /* empty table */
            కొనసాగు;                       /* get next code */
        }

        /* special code to reuse last match */
        తాత్కాలిక = సంకేతం_2;                        /* save the current code */
        ఒకవేళ (సంకేతం_2 > చివర) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            ఒకవేళ (సంకేతం_2 != చివర + 1 || prev > చివర) {
                strm->సందేశం = (z_const అక్షరం *)"invalid lzw code";
                తిరిగి Z_దత్తాంశం_ERROR;
            }
            సరిపోలిక[దొంతర++] = (గుర్తులేని అక్షరం)final;
            సంకేతం_2 = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = సరిపోలిక + దొంతర;
        అయితే (సంకేతం_2 >= 256) {
            *p++ = suffix[సంకేతం_2];
            సంకేతం_2 = ఉపసర్గ[సంకేతం_2];
        }
        దొంతర = p - సరిపోలిక;
        సరిపోలిక[దొంతర++] = (గుర్తులేని అక్షరం)సంకేతం_2;
        final = సంకేతం_2;

        /* link new table entry */
        ఒకవేళ (చివర < ముసుగు) {
            చివర++;
            ఉపసర్గ[చివర] = (గుర్తులేని పొట్టి)prev;
            suffix[చివర] = (గుర్తులేని అక్షరం)final;
        }

        /* set previous code for next iteration */
        prev = తాత్కాలిక;

        /* write output in forward order */
        అయితే (దొంతర > కొలత_2 - outcnt) {
            అయితే (outcnt < కొలత_2)
                outbuf[outcnt++] = సరిపోలిక[--దొంతర];
            ఒకవేళ (బహిర్గతం(&outd, outbuf, outcnt)) {
                strm->తదుపరి_లోపల = outbuf; /* signal write error */
                తిరిగి Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = సరిపోలిక + దొంతర;
        చేయి {
            outbuf[outcnt++] = *--p;
        } అయితే (p > సరిపోలిక);
        దొంతర = 0;

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
local పూర్ణాంకం gunpipe(z_ప్రవాహం *strm, పూర్ణాంకం infile, పూర్ణాంకం outfile)
{
    పూర్ణాంకం ret, మొదటి, చివరి;
    గుర్తులేని have, flags, len;
    z_const గుర్తులేని అక్షరం *తదుపరి = ఖాళీ;
    నిర్మాణం ind ind, *indp;
    నిర్మాణం outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    మొదటి = 1;                              /* looking for first gzip header */
    strm->తదుపరి_లోపల = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    కొరకు (;;) {
        /* look for the two magic header bytes for a gzip stream */
        ఒకవేళ (తదుపరి_2() == -1) {
            ret = Z_OK;
            ఆపు;                          /* empty gzip stream is ok */
        }
        ఒకవేళ (చివరి != 31 || (తదుపరి_2() != 139 && చివరి != 157)) {
            strm->సందేశం = (z_const అక్షరం *)"incorrect header check";
            ret = మొదటి ? Z_దత్తాంశం_ERROR : Z_ERRNO;
            ఆపు;                          /* not a gzip or compress header */
        }
        మొదటి = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        ఒకవేళ (చివరి == 157) {
            ret = lunpipe(have, తదుపరి, indp, outfile, strm);
            ఆపు;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        ఒకవేళ (తదుపరి_2() != 8) {                  /* only deflate method allowed */
            ఒకవేళ (చివరి == -1) ఆపు;
            strm->సందేశం = (z_const అక్షరం *)"unknown compression method";
            ret = Z_దత్తాంశం_ERROR;
            ఆపు;
        }
        flags = తదుపరి_2();                     /* header flags */
        తదుపరి_2();                             /* discard mod time, xflgs, os */
        తదుపరి_2();
        తదుపరి_2();
        తదుపరి_2();
        తదుపరి_2();
        తదుపరి_2();
        ఒకవేళ (చివరి == -1) ఆపు;
        ఒకవేళ (flags & 0xe0) {
            strm->సందేశం = (z_const అక్షరం *)"unknown header flags set";
            ret = Z_దత్తాంశం_ERROR;
            ఆపు;
        }
        ఒకవేళ (flags & 4) {                    /* extra field */
            len = తదుపరి_2();
            len += (గుర్తులేని)(తదుపరి_2()) << 8;
            ఒకవేళ (చివరి == -1) ఆపు;
            అయితే (len > have) {
                len -= have;
                have = 0;
                ఒకవేళ (తదుపరి_2() == -1) ఆపు;
                len--;
            }
            ఒకవేళ (చివరి == -1) ఆపు;
            have -= len;
            తదుపరి += len;
        }
        ఒకవేళ (flags & 8)                      /* file name */
            అయితే (తదుపరి_2() != 0 && చివరి != -1)
                ;
        ఒకవేళ (flags & 16)                     /* comment */
            అయితే (తదుపరి_2() != 0 && చివరి != -1)
                ;
        ఒకవేళ (flags & 2) {                    /* header crc */
            తదుపరి_2();
            తదుపరి_2();
        }
        ఒకవేళ (చివరి == -1) ఆపు;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0);
        outd.మొత్తం = 0;

        /* decompress data to output */
        strm->తదుపరి_లోపల = తదుపరి;
        strm->avail_లోపల = have;
        ret = inflate_వెనుక(strm, లోపల, indp, బహిర్గతం, &outd);
        ఒకవేళ (ret != Z_ప్రవాహం_చివర) ఆపు;
        తదుపరి = strm->తదుపరి_లోపల;
        have = strm->avail_లోపల;
        strm->తదుపరి_లోపల = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        ఒకవేళ (తదుపరి_2() != (పూర్ణాంకం)(outd.చక్రీయ_తనిఖీ & 0xff) ||
            తదుపరి_2() != (పూర్ణాంకం)((outd.చక్రీయ_తనిఖీ >> 8) & 0xff) ||
            తదుపరి_2() != (పూర్ణాంకం)((outd.చక్రీయ_తనిఖీ >> 16) & 0xff) ||
            తదుపరి_2() != (పూర్ణాంకం)((outd.చక్రీయ_తనిఖీ >> 24) & 0xff)) {
            /* crc error */
            ఒకవేళ (చివరి != -1) {
                strm->సందేశం = (z_const అక్షరం *)"incorrect data check";
                ret = Z_దత్తాంశం_ERROR;
            }
            ఆపు;
        }
        ఒకవేళ (తదుపరి_2() != (పూర్ణాంకం)(outd.మొత్తం & 0xff) ||
            తదుపరి_2() != (పూర్ణాంకం)((outd.మొత్తం >> 8) & 0xff) ||
            తదుపరి_2() != (పూర్ణాంకం)((outd.మొత్తం >> 16) & 0xff) ||
            తదుపరి_2() != (పూర్ణాంకం)((outd.మొత్తం >> 24) & 0xff)) {
            /* length error */
            ఒకవేళ (చివరి != -1) {
                strm->సందేశం = (z_const అక్షరం *)"incorrect length check";
                ret = Z_దత్తాంశం_ERROR;
            }
            ఆపు;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    తిరిగి ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local శూన్య copymeta(అక్షరం *from, అక్షరం *గమ్యం)
{
    నిర్మాణం stat was;
    నిర్మాణం utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    ఒకవేళ (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        తిరిగి;

    /* set to's mode bits, ignore errors */
    (శూన్య)chmod(గమ్యం, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (శూన్య)chown(గమ్యం, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (శూన్య)utime(గమ్యం, &when);
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
local పూర్ణాంకం gunzip(z_ప్రవాహం *strm, అక్షరం *inname, అక్షరం *outname, పూర్ణాంకం test)
{
    పూర్ణాంకం ret;
    పూర్ణాంకం infile, outfile;

    /* open files */
    ఒకవేళ (inname == ఖాళీ || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    లేకపోతే {
        infile = open(inname, O_RDONLY, 0);
        ఒకవేళ (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            తిరిగి 0;
        }
    }
    ఒకవేళ (test)
        outfile = -1;
    లేకపోతే ఒకవేళ (outname == ఖాళీ || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    లేకపోతే {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        ఒకవేళ (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            తిరిగి 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    ఒకవేళ (outfile > 2) close(outfile);
    ఒకవేళ (infile > 2) close(infile);

    /* interpret result */
    ఎంపిక (ret) {
    స్థితి Z_OK:
    స్థితి Z_ERRNO:
        ఒకవేళ (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        ఒకవేళ (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        ఆపు;
    స్థితి Z_దత్తాంశం_ERROR:
        ఒకవేళ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->సందేశం);
        ఆపు;
    స్థితి Z_MEM_ERROR:
        ఒకవేళ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        తిరిగి 1;
    స్థితి Z_BUF_ERROR:
        ఒకవేళ (outfile > 2) unlink(outname);
        ఒకవేళ (strm->తదుపరి_లోపల != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        లేకపోతే ఒకవేళ (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        లేకపోతే {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        ఆపు;
    అప్రమేయ:
        ఒకవేళ (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        తిరిగి 1;
    }
    తిరిగి 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
    పూర్ణాంకం ret, len, test;
    అక్షరం *outname;
    గుర్తులేని అక్షరం *కిటికీ;
    z_ప్రవాహం strm;

    /* initialize inflateBack state for repeated use */
    కిటికీ = సరిపోలిక;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflate_వెనుక_Init(&strm, 15, కిటికీ);
    ఒకవేళ (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        తిరిగి 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    ఒకవేళ (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        తిరిగి 0;
    }
    ఒకవేళ (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    ఒకవేళ (argc)
        చేయి {
            ఒకవేళ (test)
                outname = ఖాళీ;
            లేకపోతే {
                len = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(*argv);
                ఒకవేళ (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                లేకపోతే ఒకవేళ (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                లేకపోతే {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    కొనసాగు;
                }
                outname = జ్ఞాపకం_ఇవ్వు(len + 1);
                ఒకవేళ (outname == ఖాళీ) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    ఆపు;
                }
                జ్ఞాపకం_నకలు(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            ఒకవేళ (outname != ఖాళీ) విడుదల(outname);
            ఒకవేళ (ret) ఆపు;
        } అయితే (argv++, --argc);
    లేకపోతే
        ret = gunzip(&strm, ఖాళీ, ఖాళీ, test);

    /* clean up */
    inflate_వెనుక_చివర(&strm);
    తిరిగి ret;
}
