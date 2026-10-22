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
#تعریف_کرو local ساکن

/* buffer constants */
#تعریف_کرو سائز_2 32768U         /* input and output buffer sizes */
#تعریف_کرو PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
ساخت ind {
    صحیح infile;
    غیرعلامتی حرف *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local غیرعلامتی اندر(خالی *اندر_desc, z_const غیرعلامتی حرف **buf)
{
    صحیح ret;
    غیرعلامتی len;
    غیرعلامتی حرف *اگلا;
    ساخت ind *me = (ساخت ind *)اندر_desc;

    اگلا = me->inbuf;
    *buf = اگلا;
    len = 0;
    کرو {
        ret = PIECE;
        اگر ((غیرعلامتی)ret > سائز_2 - len)
            ret = (صحیح)(سائز_2 - len);
        ret = (صحیح)اقرأ(me->infile, اگلا, ret);
        اگر (ret == -1) {
            len = 0;
            توڑو;
        }
        اگلا += ret;
        len += ret;
    } جب (ret != 0 && len < سائز_2);
    واپس len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
ساخت outd {
    صحیح outfile;
    صحیح check;                  /* true if checking crc and total */
    غیرعلامتی طویل چکری_جانچ;
    غیرعلامتی طویل کل;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local صحیح برآمد(خالی *برآمد_desc, غیرعلامتی حرف *buf, غیرعلامتی len)
{
    صحیح ret;
    ساخت outd *me = (ساخت outd *)برآمد_desc;

    اگر (me->check) {
        me->چکری_جانچ = چکری_جانچ_32_2(me->چکری_جانچ, buf, len);
        me->کل += len;
    }
    اگر (me->outfile != -1)
        کرو {
            ret = PIECE;
            اگر ((غیرعلامتی)ret > len)
                ret = (صحیح)len;
            ret = (صحیح)اكتب(me->outfile, buf, ret);
            اگر (ret == -1)
                واپس 1;
            buf += ret;
            len -= ret;
        } جب (len != 0);
    واپس 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#تعریف_کرو اگلا_2() (have ? 0 : (have = اندر(indp, &اگلا)), \
                آخری = have ? (have--, (صحیح)(*اگلا++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
غیرعلامتی حرف inbuf[سائز_2];              /* input buffer */
غیرعلامتی حرف outbuf[سائز_2];             /* output buffer */
غیرعلامتی مختصر سابقہ[65536];           /* index to LZW prefix string */
غیرعلامتی حرف suffix[65536];            /* one-character LZW suffix */
غیرعلامتی حرف مطابقت[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#تعریف_کرو FLUSHCODE() \
    کرو { \
        بایاں = 0; \
        rem = 0; \
        اگر (chunk > have) { \
            chunk -= have; \
            have = 0; \
            اگر (اگلا_2() == -1) \
                توڑو; \
            chunk--; \
            اگر (chunk > have) { \
                chunk = have = 0; \
                توڑو; \
            } \
        } \
        have -= chunk; \
        اگلا += chunk; \
        chunk = 0; \
    } جب (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local صحیح lunpipe(غیرعلامتی have, z_const غیرعلامتی حرف *اگلا, ساخت ind *indp,
                  صحیح outfile, z_دھارا *strm)
{
    صحیح آخری;                   /* last byte read by NEXT(), or -1 if EOF */
    غیرعلامتی chunk;             /* bytes left in current chunk */
    صحیح بایاں;                   /* bits left in rem */
    غیرعلامتی rem;               /* unused bits from input */
    صحیح بٹ;                   /* current bits per code */
    غیرعلامتی رمز;              /* code, table traversal index */
    غیرعلامتی نقاب;              /* mask for current bits codes */
    صحیح max;                    /* maximum bits per code for this stream */
    غیرعلامتی flags;             /* compress flags, then block compress flag */
    غیرعلامتی انتہا;               /* last valid entry in prefix/suffix tables */
    غیرعلامتی عارضی;              /* current code */
    غیرعلامتی پچھلا;              /* previous code */
    غیرعلامتی final;             /* last character written for previous code */
    غیرعلامتی ڈھیر_2;             /* next position for reversed string */
    غیرعلامتی outcnt;            /* bytes in output buffer */
    ساخت outd outd;           /* output structure */
    غیرعلامتی حرف *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = اگلا_2();
    اگر (آخری == -1)
        واپس Z_BUF_ERROR;
    اگر (flags & 0x60) {
        strm->پیغام = (z_const حرف *)"unknown lzw flags set";
        واپس Z_ڈیٹا_ERROR;
    }
    max = flags & 0x1f;
    اگر (max < 9 || max > 16) {
        strm->پیغام = (z_const حرف *)"lzw bits out of range";
        واپس Z_ڈیٹا_ERROR;
    }
    اگر (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    بٹ = 9;
    نقاب = 0x1ff;
    انتہا = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    اگر (اگلا_2() == -1)                       /* no compressed data is ok */
        واپس Z_OK;
    final = پچھلا = (غیرعلامتی)آخری;          /* low 8 bits of code */
    اگر (اگلا_2() == -1)                       /* missing a bit */
        واپس Z_BUF_ERROR;
    اگر (آخری & 1) {                         /* code must be < 256 */
        strm->پیغام = (z_const حرف *)"invalid lzw code";
        واپس Z_ڈیٹا_ERROR;
    }
    rem = (غیرعلامتی)آخری >> 1;              /* remaining 7 bits */
    بایاں = 7;
    chunk = بٹ - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (غیرعلامتی حرف)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    ڈھیر_2 = 0;
    برائے (;;) {
        /* if the table will be full after this, increment the code size */
        اگر (انتہا >= نقاب && بٹ < max) {
            FLUSHCODE();
            بٹ++;
            نقاب <<= 1;
            نقاب++;
        }

        /* get a code of length bits */
        اگر (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = بٹ;
        رمز = rem;                         /* low bits of code */
        اگر (اگلا_2() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            اگر (outcnt && برآمد(&outd, outbuf, outcnt)) {
                strm->اگلا_اندر = outbuf;     /* signal write error */
                واپس Z_BUF_ERROR;
            }
            واپس Z_OK;
        }
        رمز += (غیرعلامتی)آخری << بایاں;     /* middle (or high) bits of code */
        بایاں += 8;
        chunk--;
        اگر (بٹ > بایاں) {                  /* need more bits */
            اگر (اگلا_2() == -1)               /* can't end in middle of code */
                واپس Z_BUF_ERROR;
            رمز += (غیرعلامتی)آخری << بایاں; /* high bits of code */
            بایاں += 8;
            chunk--;
        }
        رمز &= نقاب;                       /* mask to current code length */
        بایاں -= بٹ;                       /* number of unused bits */
        rem = (غیرعلامتی)آخری >> (8 - بایاں); /* unused bits from last byte */

        /* process clear code (256) */
        اگر (رمز == 256 && flags) {
            FLUSHCODE();
            بٹ = 9;                       /* initialize bits and mask */
            نقاب = 0x1ff;
            انتہا = 255;                      /* empty table */
            جاری;                       /* get next code */
        }

        /* special code to reuse last match */
        عارضی = رمز;                        /* save the current code */
        اگر (رمز > انتہا) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            اگر (رمز != انتہا + 1 || پچھلا > انتہا) {
                strm->پیغام = (z_const حرف *)"invalid lzw code";
                واپس Z_ڈیٹا_ERROR;
            }
            مطابقت[ڈھیر_2++] = (غیرعلامتی حرف)final;
            رمز = پچھلا;
        }

        /* walk through linked list to generate output in reverse order */
        p = مطابقت + ڈھیر_2;
        جب (رمز >= 256) {
            *p++ = suffix[رمز];
            رمز = سابقہ[رمز];
        }
        ڈھیر_2 = p - مطابقت;
        مطابقت[ڈھیر_2++] = (غیرعلامتی حرف)رمز;
        final = رمز;

        /* link new table entry */
        اگر (انتہا < نقاب) {
            انتہا++;
            سابقہ[انتہا] = (غیرعلامتی مختصر)پچھلا;
            suffix[انتہا] = (غیرعلامتی حرف)final;
        }

        /* set previous code for next iteration */
        پچھلا = عارضی;

        /* write output in forward order */
        جب (ڈھیر_2 > سائز_2 - outcnt) {
            جب (outcnt < سائز_2)
                outbuf[outcnt++] = مطابقت[--ڈھیر_2];
            اگر (برآمد(&outd, outbuf, outcnt)) {
                strm->اگلا_اندر = outbuf; /* signal write error */
                واپس Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = مطابقت + ڈھیر_2;
        کرو {
            outbuf[outcnt++] = *--p;
        } جب (p > مطابقت);
        ڈھیر_2 = 0;

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
local صحیح gunpipe(z_دھارا *strm, صحیح infile, صحیح outfile)
{
    صحیح ret, پہلا, آخری;
    غیرعلامتی have, flags, len;
    z_const غیرعلامتی حرف *اگلا = تہی;
    ساخت ind ind, *indp;
    ساخت outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    پہلا = 1;                              /* looking for first gzip header */
    strm->اگلا_اندر = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    برائے (;;) {
        /* look for the two magic header bytes for a gzip stream */
        اگر (اگلا_2() == -1) {
            ret = Z_OK;
            توڑو;                          /* empty gzip stream is ok */
        }
        اگر (آخری != 31 || (اگلا_2() != 139 && آخری != 157)) {
            strm->پیغام = (z_const حرف *)"incorrect header check";
            ret = پہلا ? Z_ڈیٹا_ERROR : Z_ERRNO;
            توڑو;                          /* not a gzip or compress header */
        }
        پہلا = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        اگر (آخری == 157) {
            ret = lunpipe(have, اگلا, indp, outfile, strm);
            توڑو;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        اگر (اگلا_2() != 8) {                  /* only deflate method allowed */
            اگر (آخری == -1) توڑو;
            strm->پیغام = (z_const حرف *)"unknown compression method";
            ret = Z_ڈیٹا_ERROR;
            توڑو;
        }
        flags = اگلا_2();                     /* header flags */
        اگلا_2();                             /* discard mod time, xflgs, os */
        اگلا_2();
        اگلا_2();
        اگلا_2();
        اگلا_2();
        اگلا_2();
        اگر (آخری == -1) توڑو;
        اگر (flags & 0xe0) {
            strm->پیغام = (z_const حرف *)"unknown header flags set";
            ret = Z_ڈیٹا_ERROR;
            توڑو;
        }
        اگر (flags & 4) {                    /* extra field */
            len = اگلا_2();
            len += (غیرعلامتی)(اگلا_2()) << 8;
            اگر (آخری == -1) توڑو;
            جب (len > have) {
                len -= have;
                have = 0;
                اگر (اگلا_2() == -1) توڑو;
                len--;
            }
            اگر (آخری == -1) توڑو;
            have -= len;
            اگلا += len;
        }
        اگر (flags & 8)                      /* file name */
            جب (اگلا_2() != 0 && آخری != -1)
                ;
        اگر (flags & 16)                     /* comment */
            جب (اگلا_2() != 0 && آخری != -1)
                ;
        اگر (flags & 2) {                    /* header crc */
            اگلا_2();
            اگلا_2();
        }
        اگر (آخری == -1) توڑو;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.چکری_جانچ = چکری_جانچ_32_2(0L, Z_NULL, 0);
        outd.کل = 0;

        /* decompress data to output */
        strm->اگلا_اندر = اگلا;
        strm->avail_اندر = have;
        ret = inflate_عقب(strm, اندر, indp, برآمد, &outd);
        اگر (ret != Z_دھارا_انتہا) توڑو;
        اگلا = strm->اگلا_اندر;
        have = strm->avail_اندر;
        strm->اگلا_اندر = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        اگر (اگلا_2() != (صحیح)(outd.چکری_جانچ & 0xff) ||
            اگلا_2() != (صحیح)((outd.چکری_جانچ >> 8) & 0xff) ||
            اگلا_2() != (صحیح)((outd.چکری_جانچ >> 16) & 0xff) ||
            اگلا_2() != (صحیح)((outd.چکری_جانچ >> 24) & 0xff)) {
            /* crc error */
            اگر (آخری != -1) {
                strm->پیغام = (z_const حرف *)"incorrect data check";
                ret = Z_ڈیٹا_ERROR;
            }
            توڑو;
        }
        اگر (اگلا_2() != (صحیح)(outd.کل & 0xff) ||
            اگلا_2() != (صحیح)((outd.کل >> 8) & 0xff) ||
            اگلا_2() != (صحیح)((outd.کل >> 16) & 0xff) ||
            اگلا_2() != (صحیح)((outd.کل >> 24) & 0xff)) {
            /* length error */
            اگر (آخری != -1) {
                strm->پیغام = (z_const حرف *)"incorrect length check";
                ret = Z_ڈیٹا_ERROR;
            }
            توڑو;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    واپس ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local خالی copymeta(حرف *from, حرف *مقصد)
{
    ساخت stat was;
    ساخت utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    اگر (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        واپس;

    /* set to's mode bits, ignore errors */
    (خالی)chmod(مقصد, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (خالی)chown(مقصد, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (خالی)utime(مقصد, &when);
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
local صحیح gunzip(z_دھارا *strm, حرف *inname, حرف *outname, صحیح test)
{
    صحیح ret;
    صحیح infile, outfile;

    /* open files */
    اگر (inname == تہی || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    ورنہ {
        infile = open(inname, O_RDONLY, 0);
        اگر (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            واپس 0;
        }
    }
    اگر (test)
        outfile = -1;
    ورنہ اگر (outname == تہی || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    ورنہ {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        اگر (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            واپس 0;
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
        توڑو;
    حالت Z_ڈیٹا_ERROR:
        اگر (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->پیغام);
        توڑو;
    حالت Z_MEM_ERROR:
        اگر (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        واپس 1;
    حالت Z_BUF_ERROR:
        اگر (outfile > 2) unlink(outname);
        اگر (strm->اگلا_اندر != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        ورنہ اگر (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        ورنہ {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        توڑو;
    طےشدہ:
        اگر (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        واپس 1;
    }
    واپس 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
صحیح مین(صحیح argc, حرف **argv)
{
    صحیح ret, len, test;
    حرف *outname;
    غیرعلامتی حرف *کھڑکی;
    z_دھارا strm;

    /* initialize inflateBack state for repeated use */
    کھڑکی = مطابقت;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflate_عقب_Init(&strm, 15, کھڑکی);
    اگر (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        واپس 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    اگر (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        واپس 0;
    }
    اگر (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    اگر (argc)
        کرو {
            اگر (test)
                outname = تہی;
            ورنہ {
                len = (صحیح)لڑی_لمبائی(*argv);
                اگر (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                ورنہ اگر (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                ورنہ {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    جاری;
                }
                outname = حافظہ_دو(len + 1);
                اگر (outname == تہی) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    توڑو;
                }
                حافظہ_نقل(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            اگر (outname != تہی) آزاد(outname);
            اگر (ret) توڑو;
        } جب (argv++, --argc);
    ورنہ
        ret = gunzip(&strm, تہی, تہی, test);

    /* clean up */
    inflate_عقب_انتہا(&strm);
    واپس ret;
}
