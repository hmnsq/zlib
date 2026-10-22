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
#ضمن <stdio.h>          /* fprintf() */
#ضمن <stdlib.h>         /* malloc(), free() */
#ضمن <string.h>         /* strerror(), strcmp(), strlen(), memcpy() */
#ضمن <errno.h>          /* errno */
#ضمن <fcntl.h>          /* open() */
#ضمن <unistd.h>         /* read(), write(), close(), chown(), unlink() */
#ضمن <sys/types.h>
#ضمن <sys/stat.h>       /* stat(), chmod() */
#ضمن <utime.h>          /* utime() */
#ضمن "zlib.h"           /* inflateBackInit(), inflateBack(), */
                            /* inflateBackEnd(), crc32() */

/* function declaration */
#تعریف_کرو local ساكن

/* buffer constants */
#تعریف_کرو ناپ_2 32768U         /* input and output buffer sizes */
#تعریف_کرو PIECE 16384         /* limits i/o chunks for 16-bit int case */

/* structure for infback() to pass to input function in() -- it maintains the
   input file and a buffer of size SIZE */
بنية ind {
    صحيح infile;
    بنانشانی حرف *inbuf;
};

/* Load input buffer, assumed to be empty, and return bytes loaded and a
   pointer to them.  read() is called until the buffer is full, or until it
   returns end-of-file or error.  Return 0 on error. */
local بنانشانی وچ(فراغ *وچ_desc, z_const بنانشانی حرف **buf)
{
    صحيح ret;
    بنانشانی len;
    بنانشانی حرف *اگلا;
    بنية ind *me = (بنية ind *)وچ_desc;

    اگلا = me->inbuf;
    *buf = اگلا;
    len = 0;
    افعل {
        ret = PIECE;
        إذا ((بنانشانی)ret > ناپ_2 - len)
            ret = (صحيح)(ناپ_2 - len);
        ret = (صحيح)اقرأ(me->infile, اگلا, ret);
        إذا (ret == -1) {
            len = 0;
            اكسر;
        }
        اگلا += ret;
        len += ret;
    } طالما (ret != 0 && len < ناپ_2);
    أرجع len;
}

/* structure for infback() to pass to output function out() -- it maintains the
   output file, a running CRC-32 check on the output and the total number of
   bytes output, both for checking against the gzip trailer.  (The length in
   the gzip trailer is stored modulo 2^32, so it's ok if a long is 32 bits and
   the output is greater than 4 GB.) */
بنية outd {
    صحيح outfile;
    صحيح check;                  /* true if checking crc and total */
    بنانشانی لما چکری_جانچ;
    بنانشانی لما کل;
};

/* Write output buffer and update the CRC-32 and total bytes written.  write()
   is called until all of the output is written or an error is encountered.
   On success out() returns 0.  For a write failure, out() returns 1.  If the
   output file descriptor is -1, then nothing is written.
 */
local صحيح باہرلا(فراغ *باہرلا_desc, بنانشانی حرف *buf, بنانشانی len)
{
    صحيح ret;
    بنية outd *me = (بنية outd *)باہرلا_desc;

    إذا (me->check) {
        me->چکری_جانچ = چکری_جانچ_32_2(me->چکری_جانچ, buf, len);
        me->کل += len;
    }
    إذا (me->outfile != -1)
        افعل {
            ret = PIECE;
            إذا ((بنانشانی)ret > len)
                ret = (صحيح)len;
            ret = (صحيح)اكتب(me->outfile, buf, ret);
            إذا (ret == -1)
                أرجع 1;
            buf += ret;
            len -= ret;
        } طالما (len != 0);
    أرجع 0;
}

/* next input byte macro for use inside lunpipe() and gunpipe() */
#تعریف_کرو اگلا_2() (have ? 0 : (have = وچ(indp, &اگلا)), \
                آخری = have ? (have--, (صحيح)(*اگلا++)) : -1)

/* memory for gunpipe() and lunpipe() --
   the first 256 entries of prefix[] and suffix[] are never used, could
   have offset the index, but it's faster to waste the memory */
بنانشانی حرف inbuf[ناپ_2];              /* input buffer */
بنانشانی حرف outbuf[ناپ_2];             /* output buffer */
بنانشانی چھوٹا اگیتر[65536];           /* index to LZW prefix string */
بنانشانی حرف suffix[65536];            /* one-character LZW suffix */
بنانشانی حرف میل[65280 + 2];         /* buffer for reversed match or gzip
                                           32K sliding window */

/* throw out what's left in the current bits byte buffer (this is a vestigial
   aspect of the compressed data format derived from an implementation that
   made use of a special VAX machine instruction!) */
#تعریف_کرو FLUSHCODE() \
    افعل { \
        بایاں = 0; \
        rem = 0; \
        إذا (chunk > have) { \
            chunk -= have; \
            have = 0; \
            إذا (اگلا_2() == -1) \
                اكسر; \
            chunk--; \
            إذا (chunk > have) { \
                chunk = have = 0; \
                اكسر; \
            } \
        } \
        have -= chunk; \
        اگلا += chunk; \
        chunk = 0; \
    } طالما (0)

/* Decompress a compress (LZW) file from indp to outfile.  The compress magic
   header (two bytes) has already been read and verified.  There are have bytes
   of buffered input at next.  strm is used for passing error information back
   to gunpipe().

   lunpipe() will return Z_OK on success, Z_BUF_ERROR for an unexpected end of
   file, read error, or write error (a write error indicated by strm->next_in
   not equal to Z_NULL), or Z_DATA_ERROR for invalid input.
 */
local صحيح lunpipe(بنانشانی have, z_const بنانشانی حرف *اگلا, بنية ind *indp,
                  صحيح outfile, z_دھارا *strm)
{
    صحيح آخری;                   /* last byte read by NEXT(), or -1 if EOF */
    بنانشانی chunk;             /* bytes left in current chunk */
    صحيح بایاں;                   /* bits left in rem */
    بنانشانی rem;               /* unused bits from input */
    صحيح بٹ;                   /* current bits per code */
    بنانشانی کوڈ;              /* code, table traversal index */
    بنانشانی نقاب;              /* mask for current bits codes */
    صحيح max;                    /* maximum bits per code for this stream */
    بنانشانی flags;             /* compress flags, then block compress flag */
    بنانشانی سرا;               /* last valid entry in prefix/suffix tables */
    بنانشانی عارضی;              /* current code */
    بنانشانی prev;              /* previous code */
    بنانشانی final;             /* last character written for previous code */
    بنانشانی ڈھیر_2;             /* next position for reversed string */
    بنانشانی outcnt;            /* bytes in output buffer */
    بنية outd outd;           /* output structure */
    بنانشانی حرف *p;

    /* set up output */
    outd.outfile = outfile;
    outd.check = 0;

    /* process remainder of compress header -- a flags byte */
    flags = اگلا_2();
    إذا (آخری == -1)
        أرجع Z_BUF_ERROR;
    إذا (flags & 0x60) {
        strm->پیغام = (z_const حرف *)"unknown lzw flags set";
        أرجع Z_انکڑے_ERROR;
    }
    max = flags & 0x1f;
    إذا (max < 9 || max > 16) {
        strm->پیغام = (z_const حرف *)"lzw bits out of range";
        أرجع Z_انکڑے_ERROR;
    }
    إذا (max == 9)                           /* 9 doesn't really mean 9 */
        max = 10;
    flags &= 0x80;                          /* true if block compress */

    /* clear table */
    بٹ = 9;
    نقاب = 0x1ff;
    سرا = flags ? 256 : 255;

    /* set up: get first 9-bit code, which is the first decompressed byte, but
       don't create a table entry until the next code */
    إذا (اگلا_2() == -1)                       /* no compressed data is ok */
        أرجع Z_OK;
    final = prev = (بنانشانی)آخری;          /* low 8 bits of code */
    إذا (اگلا_2() == -1)                       /* missing a bit */
        أرجع Z_BUF_ERROR;
    إذا (آخری & 1) {                         /* code must be < 256 */
        strm->پیغام = (z_const حرف *)"invalid lzw code";
        أرجع Z_انکڑے_ERROR;
    }
    rem = (بنانشانی)آخری >> 1;              /* remaining 7 bits */
    بایاں = 7;
    chunk = بٹ - 2;                       /* 7 bytes left in this chunk */
    outbuf[0] = (بنانشانی حرف)final;       /* write first decompressed byte */
    outcnt = 1;

    /* decode codes */
    ڈھیر_2 = 0;
    لكل (;;) {
        /* if the table will be full after this, increment the code size */
        إذا (سرا >= نقاب && بٹ < max) {
            FLUSHCODE();
            بٹ++;
            نقاب <<= 1;
            نقاب++;
        }

        /* get a code of length bits */
        إذا (chunk == 0)                     /* decrement chunk modulo bits */
            chunk = بٹ;
        کوڈ = rem;                         /* low bits of code */
        إذا (اگلا_2() == -1) {                 /* EOF is end of compressed data */
            /* write remaining buffered output */
            إذا (outcnt && باہرلا(&outd, outbuf, outcnt)) {
                strm->اگلا_وچ = outbuf;     /* signal write error */
                أرجع Z_BUF_ERROR;
            }
            أرجع Z_OK;
        }
        کوڈ += (بنانشانی)آخری << بایاں;     /* middle (or high) bits of code */
        بایاں += 8;
        chunk--;
        إذا (بٹ > بایاں) {                  /* need more bits */
            إذا (اگلا_2() == -1)               /* can't end in middle of code */
                أرجع Z_BUF_ERROR;
            کوڈ += (بنانشانی)آخری << بایاں; /* high bits of code */
            بایاں += 8;
            chunk--;
        }
        کوڈ &= نقاب;                       /* mask to current code length */
        بایاں -= بٹ;                       /* number of unused bits */
        rem = (بنانشانی)آخری >> (8 - بایاں); /* unused bits from last byte */

        /* process clear code (256) */
        إذا (کوڈ == 256 && flags) {
            FLUSHCODE();
            بٹ = 9;                       /* initialize bits and mask */
            نقاب = 0x1ff;
            سرا = 255;                      /* empty table */
            استمر;                       /* get next code */
        }

        /* special code to reuse last match */
        عارضی = کوڈ;                        /* save the current code */
        إذا (کوڈ > سرا) {
            /* Be picky on the allowed code here, and make sure that the code
               we drop through (prev) will be a valid index so that random
               input does not cause an exception.  The code != end + 1 check is
               empirically derived, and not checked in the original uncompress
               code.  If this ever causes a problem, that check could be safely
               removed.  Leaving this check in greatly improves gun's ability
               to detect random or corrupted input after a compress header.
               In any case, the prev > end check must be retained. */
            إذا (کوڈ != سرا + 1 || prev > سرا) {
                strm->پیغام = (z_const حرف *)"invalid lzw code";
                أرجع Z_انکڑے_ERROR;
            }
            میل[ڈھیر_2++] = (بنانشانی حرف)final;
            کوڈ = prev;
        }

        /* walk through linked list to generate output in reverse order */
        p = میل + ڈھیر_2;
        طالما (کوڈ >= 256) {
            *p++ = suffix[کوڈ];
            کوڈ = اگیتر[کوڈ];
        }
        ڈھیر_2 = p - میل;
        میل[ڈھیر_2++] = (بنانشانی حرف)کوڈ;
        final = کوڈ;

        /* link new table entry */
        إذا (سرا < نقاب) {
            سرا++;
            اگیتر[سرا] = (بنانشانی چھوٹا)prev;
            suffix[سرا] = (بنانشانی حرف)final;
        }

        /* set previous code for next iteration */
        prev = عارضی;

        /* write output in forward order */
        طالما (ڈھیر_2 > ناپ_2 - outcnt) {
            طالما (outcnt < ناپ_2)
                outbuf[outcnt++] = میل[--ڈھیر_2];
            إذا (باہرلا(&outd, outbuf, outcnt)) {
                strm->اگلا_وچ = outbuf; /* signal write error */
                أرجع Z_BUF_ERROR;
            }
            outcnt = 0;
        }
        p = میل + ڈھیر_2;
        افعل {
            outbuf[outcnt++] = *--p;
        } طالما (p > میل);
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
local صحيح gunpipe(z_دھارا *strm, صحيح infile, صحيح outfile)
{
    صحيح ret, پہلا, آخری;
    بنانشانی have, flags, len;
    z_const بنانشانی حرف *اگلا = عدم;
    بنية ind ind, *indp;
    بنية outd outd;

    /* setup input buffer */
    ind.infile = infile;
    ind.inbuf = inbuf;
    indp = &ind;

    /* decompress concatenated gzip streams */
    have = 0;                               /* no input data read in yet */
    پہلا = 1;                              /* looking for first gzip header */
    strm->اگلا_وچ = Z_NULL;                 /* so Z_BUF_ERROR means EOF */
    لكل (;;) {
        /* look for the two magic header bytes for a gzip stream */
        إذا (اگلا_2() == -1) {
            ret = Z_OK;
            اكسر;                          /* empty gzip stream is ok */
        }
        إذا (آخری != 31 || (اگلا_2() != 139 && آخری != 157)) {
            strm->پیغام = (z_const حرف *)"incorrect header check";
            ret = پہلا ? Z_انکڑے_ERROR : Z_ERRNO;
            اكسر;                          /* not a gzip or compress header */
        }
        پہلا = 0;                          /* next non-header is junk */

        /* process a compress (LZW) file -- can't be concatenated after this */
        إذا (آخری == 157) {
            ret = lunpipe(have, اگلا, indp, outfile, strm);
            اكسر;
        }

        /* process remainder of gzip header */
        ret = Z_BUF_ERROR;
        إذا (اگلا_2() != 8) {                  /* only deflate method allowed */
            إذا (آخری == -1) اكسر;
            strm->پیغام = (z_const حرف *)"unknown compression method";
            ret = Z_انکڑے_ERROR;
            اكسر;
        }
        flags = اگلا_2();                     /* header flags */
        اگلا_2();                             /* discard mod time, xflgs, os */
        اگلا_2();
        اگلا_2();
        اگلا_2();
        اگلا_2();
        اگلا_2();
        إذا (آخری == -1) اكسر;
        إذا (flags & 0xe0) {
            strm->پیغام = (z_const حرف *)"unknown header flags set";
            ret = Z_انکڑے_ERROR;
            اكسر;
        }
        إذا (flags & 4) {                    /* extra field */
            len = اگلا_2();
            len += (بنانشانی)(اگلا_2()) << 8;
            إذا (آخری == -1) اكسر;
            طالما (len > have) {
                len -= have;
                have = 0;
                إذا (اگلا_2() == -1) اكسر;
                len--;
            }
            إذا (آخری == -1) اكسر;
            have -= len;
            اگلا += len;
        }
        إذا (flags & 8)                      /* file name */
            طالما (اگلا_2() != 0 && آخری != -1)
                ;
        إذا (flags & 16)                     /* comment */
            طالما (اگلا_2() != 0 && آخری != -1)
                ;
        إذا (flags & 2) {                    /* header crc */
            اگلا_2();
            اگلا_2();
        }
        إذا (آخری == -1) اكسر;

        /* set up output */
        outd.outfile = outfile;
        outd.check = 1;
        outd.چکری_جانچ = چکری_جانچ_32_2(0L, Z_NULL, 0);
        outd.کل = 0;

        /* decompress data to output */
        strm->اگلا_وچ = اگلا;
        strm->avail_وچ = have;
        ret = inflate_پٹھ(strm, وچ, indp, باہرلا, &outd);
        إذا (ret != Z_دھارا_سرا) اكسر;
        اگلا = strm->اگلا_وچ;
        have = strm->avail_وچ;
        strm->اگلا_وچ = Z_NULL;             /* so Z_BUF_ERROR means EOF */

        /* check trailer */
        ret = Z_BUF_ERROR;
        إذا (اگلا_2() != (صحيح)(outd.چکری_جانچ & 0xff) ||
            اگلا_2() != (صحيح)((outd.چکری_جانچ >> 8) & 0xff) ||
            اگلا_2() != (صحيح)((outd.چکری_جانچ >> 16) & 0xff) ||
            اگلا_2() != (صحيح)((outd.چکری_جانچ >> 24) & 0xff)) {
            /* crc error */
            إذا (آخری != -1) {
                strm->پیغام = (z_const حرف *)"incorrect data check";
                ret = Z_انکڑے_ERROR;
            }
            اكسر;
        }
        إذا (اگلا_2() != (صحيح)(outd.کل & 0xff) ||
            اگلا_2() != (صحيح)((outd.کل >> 8) & 0xff) ||
            اگلا_2() != (صحيح)((outd.کل >> 16) & 0xff) ||
            اگلا_2() != (صحيح)((outd.کل >> 24) & 0xff)) {
            /* length error */
            إذا (آخری != -1) {
                strm->پیغام = (z_const حرف *)"incorrect length check";
                ret = Z_انکڑے_ERROR;
            }
            اكسر;
        }

        /* go back and look for another gzip stream */
    }

    /* clean up and return */
    أرجع ret;
}

/* Copy file attributes, from -> to, as best we can.  This is best effort, so
   no errors are reported.  The mode bits, including suid, sgid, and the sticky
   bit are copied (if allowed), the owner's user id and group id are copied
   (again if allowed), and the access and modify times are copied. */
local فراغ copymeta(حرف *from, حرف *منزل)
{
    بنية stat was;
    بنية utimbuf when;

    /* get all of from's Unix meta data, return if not a regular file */
    إذا (stat(from, &was) != 0 || (was.st_mode & S_IFMT) != S_IFREG)
        أرجع;

    /* set to's mode bits, ignore errors */
    (فراغ)chmod(منزل, was.st_mode & 07777);

    /* copy owner's user and group, ignore errors */
    (فراغ)chown(منزل, was.st_uid, was.st_gid);

    /* copy access and modify times, ignore errors */
    when.actime = was.st_atime;
    when.modtime = was.st_mtime;
    (فراغ)utime(منزل, &when);
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
local صحيح gunzip(z_دھارا *strm, حرف *inname, حرف *outname, صحيح test)
{
    صحيح ret;
    صحيح infile, outfile;

    /* open files */
    إذا (inname == عدم || *inname == 0) {
        inname = "-";
        infile = 0;     /* stdin */
    }
    وإلا {
        infile = open(inname, O_RDONLY, 0);
        إذا (infile == -1) {
            fprintf(stderr, "gun cannot open %s\n", inname);
            أرجع 0;
        }
    }
    إذا (test)
        outfile = -1;
    وإلا إذا (outname == عدم || *outname == 0) {
        outname = "-";
        outfile = 1;    /* stdout */
    }
    وإلا {
        outfile = open(outname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        إذا (outfile == -1) {
            close(infile);
            fprintf(stderr, "gun cannot create %s\n", outname);
            أرجع 0;
        }
    }
    errno = 0;

    /* decompress */
    ret = gunpipe(strm, infile, outfile);
    إذا (outfile > 2) close(outfile);
    إذا (infile > 2) close(infile);

    /* interpret result */
    بدل (ret) {
    حالة Z_OK:
    حالة Z_ERRNO:
        إذا (infile > 2 && outfile > 2) {
            copymeta(inname, outname);          /* copy attributes */
            unlink(inname);
        }
        إذا (ret == Z_ERRNO)
            fprintf(stderr, "gun warning: trailing garbage ignored in %s\n",
                    inname);
        اكسر;
    حالة Z_انکڑے_ERROR:
        إذا (outfile > 2) unlink(outname);
        fprintf(stderr, "gun data error on %s: %s\n", inname, strm->پیغام);
        اكسر;
    حالة Z_MEM_ERROR:
        إذا (outfile > 2) unlink(outname);
        fprintf(stderr, "gun out of memory error--aborting\n");
        أرجع 1;
    حالة Z_BUF_ERROR:
        إذا (outfile > 2) unlink(outname);
        إذا (strm->اگلا_وچ != Z_NULL) {
            fprintf(stderr, "gun write error on %s: %s\n",
                    outname, strerror(errno));
        }
        وإلا إذا (errno) {
            fprintf(stderr, "gun read error on %s: %s\n",
                    inname, strerror(errno));
        }
        وإلا {
            fprintf(stderr, "gun unexpected end of file on %s\n",
                    inname);
        }
        اكسر;
    افتراضي:
        إذا (outfile > 2) unlink(outname);
        fprintf(stderr, "gun internal error--aborting\n");
        أرجع 1;
    }
    أرجع 0;
}

/* Process the gun command line arguments.  See the command syntax near the
   beginning of this source file. */
صحيح مکھ(صحيح argc, حرف **argv)
{
    صحيح ret, len, test;
    حرف *outname;
    بنانشانی حرف *کھڑکی;
    z_دھارا strm;

    /* initialize inflateBack state for repeated use */
    کھڑکی = میل;                         /* reuse LZW match buffer */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflate_پٹھ_Init(&strm, 15, کھڑکی);
    إذا (ret != Z_OK) {
        fprintf(stderr, "gun out of memory error--aborting\n");
        أرجع 1;
    }

    /* decompress each file to the same name with the suffix removed */
    argc--;
    argv++;
    test = 0;
    إذا (argc && strcmp(*argv, "-h") == 0) {
        fprintf(stderr, "gun 1.6 (17 Jan 2010)\n");
        fprintf(stderr, "Copyright (C) 2003-2010 Mark Adler\n");
        fprintf(stderr, "usage: gun [-t] [file1.gz [file2.Z ...]]\n");
        أرجع 0;
    }
    إذا (argc && strcmp(*argv, "-t") == 0) {
        test = 1;
        argc--;
        argv++;
    }
    إذا (argc)
        افعل {
            إذا (test)
                outname = عدم;
            وإلا {
                len = (صحيح)لڑی_لمبائی(*argv);
                إذا (strcmp(*argv + len - 3, ".gz") == 0 ||
                    strcmp(*argv + len - 3, "-gz") == 0)
                    len -= 3;
                وإلا إذا (strcmp(*argv + len - 2, ".z") == 0 ||
                    strcmp(*argv + len - 2, "-z") == 0 ||
                    strcmp(*argv + len - 2, "_z") == 0 ||
                    strcmp(*argv + len - 2, ".Z") == 0)
                    len -= 2;
                وإلا {
                    fprintf(stderr, "gun error: no gz type on %s--skipping\n",
                            *argv);
                    استمر;
                }
                outname = حافظہ_دیو(len + 1);
                إذا (outname == عدم) {
                    fprintf(stderr, "gun out of memory error--aborting\n");
                    ret = 1;
                    اكسر;
                }
                حافظہ_نقل(outname, *argv, len);
                outname[len] = 0;
            }
            ret = gunzip(&strm, *argv, outname, test);
            إذا (outname != عدم) حرر(outname);
            إذا (ret) اكسر;
        } طالما (argv++, --argc);
    وإلا
        ret = gunzip(&strm, عدم, عدم, test);

    /* clean up */
    inflate_پٹھ_سرا(&strm);
    أرجع ret;
}
