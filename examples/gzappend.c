/* gzappend -- command to append to a gzip file

  Copyright (C) 2003, 2012 Mark Adler, all rights reserved
  version 1.2, 11 Oct 2012

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler    madler@alumni.caltech.edu
 */

/*
 * Change history:
 *
 * 1.0  19 Oct 2003     - First version
 * 1.1   4 Nov 2003     - Expand and clarify some comments and notes
 *                      - Add version and copyright to help
 *                      - Send help to stdout instead of stderr
 *                      - Add some preemptive typecasts
 *                      - Add L to constants in lseek() calls
 *                      - Remove some debugging information in error messages
 *                      - Use new data_type definition for zlib 1.2.1
 *                      - Simplify and unify file operations
 *                      - Finish off gzip file in gztack()
 *                      - Use deflatePrime() instead of adding empty blocks
 *                      - Keep gzip file clean on appended file read errors
 *                      - Use in-place rotate instead of auxiliary buffer
 *                        (Why you ask?  Because it was fun to write!)
 * 1.2  11 Oct 2012     - Fix for proper z_const usage
 *                      - Check for input buffer malloc failure
 */

/*
   gzappend takes a gzip file and appends to it, compressing files from the
   command line or data from stdin.  The gzip file is written to directly, to
   avoid copying that file, in case it's large.  Note that this results in the
   unfriendly behavior that if gzappend fails, the gzip file is corrupted.

   This program was written to illustrate the use of the new Z_BLOCK option of
   zlib 1.2.x's inflate() function.  This option returns from inflate() at each
   block boundary to facilitate locating and modifying the last block bit at
   the start of the final deflate block.  Also whether using Z_BLOCK or not,
   another required feature of zlib 1.2.x is that inflate() now provides the
   number of unused bits in the last input byte used.  gzappend will not work
   with versions of zlib earlier than 1.2.1.

   gzappend first decompresses the gzip file internally, discarding all but
   the last 32K of uncompressed data, and noting the location of the last block
   bit and the number of unused bits in the last byte of the compressed data.
   The gzip trailer containing the CRC-32 and length of the uncompressed data
   is verified.  This trailer will be later overwritten.

   Then the last block bit is cleared by seeking back in the file and rewriting
   the byte that contains it.  Seeking forward, the last byte of the compressed
   data is saved along with the number of unused bits to initialize deflate.

   A deflate process is initialized, using the last 32K of the uncompressed
   data from the gzip file to initialize the dictionary.  If the total
   uncompressed data was less than 32K, then all of it is used to initialize
   the dictionary.  The deflate output bit buffer is also initialized with the
   last bits from the original deflate stream.  From here on, the data to
   append is simply compressed using deflate, and written to the gzip file.
   When that is complete, the new CRC-32 and uncompressed length are written
   as the trailer of the gzip file.
 */

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <fcntl.h>
#ضمن <unistd.h>
#ضمن "zlib.h"

#تعریف_کړه local ساكن
#تعریف_کړه LGCHUNK 14
#تعریف_کړه CHUNK (1U << LGCHUNK)
#تعریف_کړه DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local فراغ bye(حرف *msg1, حرف *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    وتل(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local بینښه لوی_ګډ_وېشونکی(بینښه a, بینښه b)
{
    بینښه c;

    طالما (a && b)
        إذا (a > b) {
            c = b;
            طالما (a - c >= c)
                c <<= 1;
            a -= c;
        }
        وإلا {
            c = a;
            طالما (b - c >= c)
                c <<= 1;
            b -= c;
        }
    أرجع a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local فراغ rotate(بینښه حرف *list, بینښه len, بینښه rot)
{
    بینښه حرف tmp;
    بینښه دورې;
    بینښه حرف *پیل, *وروستی, *منزل, *from;

    /* normalize rot and handle degenerate cases */
    إذا (len < 2) أرجع;
    إذا (rot >= len) rot %= len;
    إذا (rot == 0) أرجع;

    /* pointer to last entry in list */
    وروستی = list + (len - 1);

    /* do simple left shift by one */
    إذا (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *وروستی = tmp;
        أرجع;
    }

    /* do simple right shift by one */
    إذا (rot == len - 1) {
        tmp = *وروستی;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        أرجع;
    }

    /* otherwise do rotate as a set of cycles in place */
    دورې = لوی_ګډ_وېشونکی(len, rot);             /* number of cycles */
    افعل {
        پیل = from = list + دورې;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        لكل (;;) {
            منزل = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            إذا (from > وروستی) from -= len;   /* (pointer better not wrap) */
            إذا (from == پیل) اكسر;   /* all but one shifted */
            *منزل = *from;                /* shift left */
        }
        *منزل = tmp;                      /* complete the circle */
    } طالما (--دورې);
}

/* structure for gzip file read operations */
تعريفنوع بنية {
    صحيح fd;                     /* file descriptor */
    صحيح اندازه;                   /* 1 << size is bytes in buf */
    بینښه کیڼ;              /* bytes available at next */
    بینښه حرف *buf;         /*بفر*/
    z_const بینښه حرف *بل;    /* next byte in buffer */
    حرف *name;                 /* file name for error messages */
} file;

/* reload buffer */
local صحيح readin(file *دننه)
{
    صحيح len;

    len = اقرأ(دننه->fd, دننه->buf, 1 << دننه->اندازه);
    إذا (len == -1) bye("error reading ", دننه->name);
    دننه->کیڼ = (بینښه)len;
    دننه->بل = دننه->buf;
    أرجع len;
}

/* read from file in, exit if end-of-file */
local صحيح readmore(file *دننه)
{
    إذا (readin(دننه) == 0) bye("unexpected end of ", دننه->name);
    أرجع 0;
}

#تعریف_کړه read1(دننه) (دننه->کیڼ == 0 ? readmore(دننه) : 0, \
                   دننه->کیڼ--, *(دننه->بل)++)

/* skip over n bytes of in */
local فراغ skip(file *دننه, بینښه n)
{
    بینښه bypass;

    إذا (n > دننه->کیڼ) {
        n -= دننه->کیڼ;
        bypass = n & ~((1U << دننه->اندازه) - 1);
        إذا (bypass) {
            إذا (lseek(دننه->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", دننه->name);
            n -= bypass;
        }
        readmore(دننه);
        إذا (n > دننه->کیڼ)
            bye("unexpected end of ", دننه->name);
    }
    دننه->کیڼ -= n;
    دننه->بل += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
بینښه اوږد read4(file *دننه)
{
    بینښه اوږد val;

    val = read1(دننه);
    val += (بینښه)read1(دننه) << 8;
    val += (بینښه اوږد)read1(دننه) << 16;
    val += (بینښه اوږد)read1(دننه) << 24;
    أرجع val;
}

/* skip over gzip header */
local فراغ gzheader(file *دننه)
{
    صحيح flags;
    بینښه n;

    إذا (read1(دننه) != 31 || read1(دننه) != 139) bye(دننه->name, " not a gzip file");
    إذا (read1(دننه) != 8) bye("unknown compression method in", دننه->name);
    flags = read1(دننه);
    إذا (flags & 0xe0) bye("unknown header flags set in", دننه->name);
    skip(دننه, 6);
    إذا (flags & 4) {
        n = read1(دننه);
        n += (بینښه)(read1(دننه)) << 8;
        skip(دننه, n);
    }
    إذا (flags & 8) طالما (read1(دننه) != 0) ;
    إذا (flags & 16) طالما (read1(دننه) != 0) ;
    إذا (flags & 2) skip(دننه, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local صحيح gzscan(حرف *name, z_بهیر *strm, صحيح level)
{
    صحيح ret, lastbit, کیڼ, ډک;
    بینښه have;
    بینښه اوږد دوراني_ازموینه, tot;
    بینښه حرف *کړکۍ;
    off_t lastoff, پای_ټکی;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    إذا (gz.fd == -1) bye("cannot open ", name);
    gz.buf = حافظه_ورکړه(CHUNK);
    إذا (gz.buf == عدم) bye("out of memory", "");
    gz.اندازه = LGCHUNK;
    gz.کیڼ = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    کړکۍ = حافظه_ورکړه(DSIZE);
    إذا (کړکۍ == عدم) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    إذا (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.کیڼ;
    کیڼ = 0;
    strm->avail_دننه = gz.کیڼ;
    strm->بل_دننه = gz.بل;
    دوراني_ازموینه = دوراني_ازموینه_32_2(0L, Z_NULL, 0);
    have = ډک = 0;
    افعل {
        /* if needed, get more input */
        إذا (strm->avail_دننه == 0) {
            readmore(&gz);
            strm->avail_دننه = gz.کیڼ;
            strm->بل_دننه = gz.بل;
        }

        /* set up output to next available section of sliding window */
        strm->avail_بهر = DSIZE - have;
        strm->بل_بهر = کړکۍ + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_بلاک);
        إذا (ret == Z_بهیر_ERROR) bye("internal stream error!", "");
        إذا (ret == Z_MEM_ERROR) bye("out of memory", "");
        إذا (ret == Z_معلومات_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        دوراني_ازموینه = دوراني_ازموینه_32_2(دوراني_ازموینه, کړکۍ + have, DSIZE - have - strm->avail_بهر);
        إذا (strm->avail_بهر)
            have = DSIZE - strm->avail_بهر;
        وإلا {
            have = 0;
            ډک = 1;
        }

        /* process end of block */
        إذا (strm->معلومات_type & 128) {
            إذا (strm->معلومات_type & 64)
                کیڼ = strm->معلومات_type & 0x1f;
            وإلا {
                lastbit = strm->معلومات_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_دننه;
            }
        }
    } طالما (ret != Z_بهیر_پای_ټکی);
    inflate_پای_ټکی(strm);
    gz.کیڼ = strm->avail_دننه;
    gz.بل = strm->بل_دننه;

    /* save the location of the end of the compressed data */
    پای_ټکی = lseek(gz.fd, 0L, SEEK_CUR) - gz.کیڼ;

    /* check gzip trailer and save total for deflate */
    إذا (دوراني_ازموینه != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->ټول_بهر;
    إذا ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    إذا (gz.کیڼ || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    إذا (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (بینښه حرف)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    إذا (اكتب(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    إذا (ډک) {
        rotate(کړکۍ, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    إذا (ret != Z_OK) bye("out of memory", "");
    deflate_سېټ_Dictionary(strm, کړکۍ, have);
    strm->adler = دوراني_ازموینه;
    strm->ټول_دننه = tot;
    إذا (کیڼ) {
        lseek(gz.fd, --پای_ټکی, SEEK_SET);
        إذا (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_اولي(strm, 8 - کیڼ, *gz.buf);
    }
    lseek(gz.fd, پای_ټکی, SEEK_SET);

    /* clean up and return */
    حرر(کړکۍ);
    حرر(gz.buf);
    أرجع gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local فراغ gztack(حرف *name, صحيح gd, z_بهیر *strm, صحيح وروستی)
{
    صحيح fd, len, ret;
    بینښه کیڼ;
    بینښه حرف *دننه, *بهر;

    /* open file to compress and append */
    fd = 0;
    إذا (name != عدم) {
        fd = open(name, O_RDONLY, 0);
        إذا (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    دننه = حافظه_ورکړه(CHUNK);
    بهر = حافظه_ورکړه(CHUNK);
    إذا (دننه == عدم || بهر == عدم) bye("out of memory", "");

    /* compress input file and append to gzip file */
    افعل {
        /* get more input */
        len = اقرأ(fd, دننه, CHUNK);
        إذا (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_دننه = (بینښه)len;
        strm->بل_دننه = دننه;
        إذا (len) strm->adler = دوراني_ازموینه_32_2(strm->adler, دننه, (بینښه)len);

        /* compress and write all available output */
        افعل {
            strm->avail_بهر = CHUNK;
            strm->بل_بهر = بهر;
            ret = deflate(strm, وروستی && len == 0 ? Z_پای : Z_NO_FLUSH);
            کیڼ = CHUNK - strm->avail_بهر;
            طالما (کیڼ) {
                len = اكتب(gd, بهر + CHUNK - strm->avail_بهر - کیڼ, کیڼ);
                إذا (len == -1) bye("writing gzip file", "");
                کیڼ -= (بینښه)len;
            }
        } طالما (strm->avail_بهر == 0 && ret != Z_بهیر_پای_ټکی);
    } طالما (len != 0);

    /* write trailer after last entry */
    إذا (وروستی) {
        deflate_پای_ټکی(strm);
        بهر[0] = (بینښه حرف)(strm->adler);
        بهر[1] = (بینښه حرف)(strm->adler >> 8);
        بهر[2] = (بینښه حرف)(strm->adler >> 16);
        بهر[3] = (بینښه حرف)(strm->adler >> 24);
        بهر[4] = (بینښه حرف)(strm->ټول_دننه);
        بهر[5] = (بینښه حرف)(strm->ټول_دننه >> 8);
        بهر[6] = (بینښه حرف)(strm->ټول_دننه >> 16);
        بهر[7] = (بینښه حرف)(strm->ټول_دننه >> 24);
        len = 8;
        افعل {
            ret = اكتب(gd, بهر + 8 - len, len);
            إذا (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } طالما (len);
        close(gd);
    }

    /* clean up and return */
    حرر(بهر);
    حرر(دننه);
    إذا (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
صحيح اصلي(صحيح argc, حرف **argv)
{
    صحيح gd, level;
    z_بهیر strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    إذا (*argv == عدم) {
        چاپ_کړه(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        چاپ_کړه(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        أرجع 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    إذا (argv[0][0] == '-') {
        إذا (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        إذا (*++argv == عدم) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    إذا (*argv == عدم)
        gztack(عدم, gd, &strm, 1);
    وإلا
        افعل {
            gztack(*argv, gd, &strm, argv[1] == عدم);
        } طالما (*++argv != عدم);
    أرجع 0;
}
