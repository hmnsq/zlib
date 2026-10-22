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

#تعریف_کرو local ساكن
#تعریف_کرو LGCHUNK 14
#تعریف_کرو CHUNK (1U << LGCHUNK)
#تعریف_کرو DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local فراغ bye(حرف *msg1, حرف *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    باہر_نکلو(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local بنانشانی عاد_اعظم(بنانشانی a, بنانشانی b)
{
    بنانشانی c;

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
local فراغ rotate(بنانشانی حرف *list, بنانشانی len, بنانشانی rot)
{
    بنانشانی حرف tmp;
    بنانشانی چکراں;
    بنانشانی حرف *شروع, *آخری, *منزل, *from;

    /* normalize rot and handle degenerate cases */
    إذا (len < 2) أرجع;
    إذا (rot >= len) rot %= len;
    إذا (rot == 0) أرجع;

    /* pointer to last entry in list */
    آخری = list + (len - 1);

    /* do simple left shift by one */
    إذا (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *آخری = tmp;
        أرجع;
    }

    /* do simple right shift by one */
    إذا (rot == len - 1) {
        tmp = *آخری;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        أرجع;
    }

    /* otherwise do rotate as a set of cycles in place */
    چکراں = عاد_اعظم(len, rot);             /* number of cycles */
    افعل {
        شروع = from = list + چکراں;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        لكل (;;) {
            منزل = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            إذا (from > آخری) from -= len;   /* (pointer better not wrap) */
            إذا (from == شروع) اكسر;   /* all but one shifted */
            *منزل = *from;                /* shift left */
        }
        *منزل = tmp;                      /* complete the circle */
    } طالما (--چکراں);
}

/* structure for gzip file read operations */
تعريفنوع بنية {
    صحيح fd;                     /* file descriptor */
    صحيح ناپ;                   /* 1 << size is bytes in buf */
    بنانشانی بایاں;              /* bytes available at next */
    بنانشانی حرف *buf;         /*بفر*/
    z_const بنانشانی حرف *اگلا;    /* next byte in buffer */
    حرف *name;                 /* file name for error messages */
} file;

/* reload buffer */
local صحيح readin(file *وچ)
{
    صحيح len;

    len = اقرأ(وچ->fd, وچ->buf, 1 << وچ->ناپ);
    إذا (len == -1) bye("error reading ", وچ->name);
    وچ->بایاں = (بنانشانی)len;
    وچ->اگلا = وچ->buf;
    أرجع len;
}

/* read from file in, exit if end-of-file */
local صحيح readmore(file *وچ)
{
    إذا (readin(وچ) == 0) bye("unexpected end of ", وچ->name);
    أرجع 0;
}

#تعریف_کرو read1(وچ) (وچ->بایاں == 0 ? readmore(وچ) : 0, \
                   وچ->بایاں--, *(وچ->اگلا)++)

/* skip over n bytes of in */
local فراغ skip(file *وچ, بنانشانی n)
{
    بنانشانی bypass;

    إذا (n > وچ->بایاں) {
        n -= وچ->بایاں;
        bypass = n & ~((1U << وچ->ناپ) - 1);
        إذا (bypass) {
            إذا (lseek(وچ->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", وچ->name);
            n -= bypass;
        }
        readmore(وچ);
        إذا (n > وچ->بایاں)
            bye("unexpected end of ", وچ->name);
    }
    وچ->بایاں -= n;
    وچ->اگلا += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
بنانشانی لما read4(file *وچ)
{
    بنانشانی لما val;

    val = read1(وچ);
    val += (بنانشانی)read1(وچ) << 8;
    val += (بنانشانی لما)read1(وچ) << 16;
    val += (بنانشانی لما)read1(وچ) << 24;
    أرجع val;
}

/* skip over gzip header */
local فراغ gzheader(file *وچ)
{
    صحيح flags;
    بنانشانی n;

    إذا (read1(وچ) != 31 || read1(وچ) != 139) bye(وچ->name, " not a gzip file");
    إذا (read1(وچ) != 8) bye("unknown compression method in", وچ->name);
    flags = read1(وچ);
    إذا (flags & 0xe0) bye("unknown header flags set in", وچ->name);
    skip(وچ, 6);
    إذا (flags & 4) {
        n = read1(وچ);
        n += (بنانشانی)(read1(وچ)) << 8;
        skip(وچ, n);
    }
    إذا (flags & 8) طالما (read1(وچ) != 0) ;
    إذا (flags & 16) طالما (read1(وچ) != 0) ;
    إذا (flags & 2) skip(وچ, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local صحيح gzscan(حرف *name, z_دھارا *strm, صحيح level)
{
    صحيح ret, lastbit, بایاں, بھریا;
    بنانشانی have;
    بنانشانی لما چکری_جانچ, tot;
    بنانشانی حرف *کھڑکی;
    off_t lastoff, سرا;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    إذا (gz.fd == -1) bye("cannot open ", name);
    gz.buf = حافظہ_دیو(CHUNK);
    إذا (gz.buf == عدم) bye("out of memory", "");
    gz.ناپ = LGCHUNK;
    gz.بایاں = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    کھڑکی = حافظہ_دیو(DSIZE);
    إذا (کھڑکی == عدم) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    إذا (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.بایاں;
    بایاں = 0;
    strm->avail_وچ = gz.بایاں;
    strm->اگلا_وچ = gz.اگلا;
    چکری_جانچ = چکری_جانچ_32_2(0L, Z_NULL, 0);
    have = بھریا = 0;
    افعل {
        /* if needed, get more input */
        إذا (strm->avail_وچ == 0) {
            readmore(&gz);
            strm->avail_وچ = gz.بایاں;
            strm->اگلا_وچ = gz.اگلا;
        }

        /* set up output to next available section of sliding window */
        strm->avail_باہرلا = DSIZE - have;
        strm->اگلا_باہرلا = کھڑکی + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_بلاک);
        إذا (ret == Z_دھارا_ERROR) bye("internal stream error!", "");
        إذا (ret == Z_MEM_ERROR) bye("out of memory", "");
        إذا (ret == Z_انکڑے_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        چکری_جانچ = چکری_جانچ_32_2(چکری_جانچ, کھڑکی + have, DSIZE - have - strm->avail_باہرلا);
        إذا (strm->avail_باہرلا)
            have = DSIZE - strm->avail_باہرلا;
        وإلا {
            have = 0;
            بھریا = 1;
        }

        /* process end of block */
        إذا (strm->انکڑے_type & 128) {
            إذا (strm->انکڑے_type & 64)
                بایاں = strm->انکڑے_type & 0x1f;
            وإلا {
                lastbit = strm->انکڑے_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_وچ;
            }
        }
    } طالما (ret != Z_دھارا_سرا);
    inflate_سرا(strm);
    gz.بایاں = strm->avail_وچ;
    gz.اگلا = strm->اگلا_وچ;

    /* save the location of the end of the compressed data */
    سرا = lseek(gz.fd, 0L, SEEK_CUR) - gz.بایاں;

    /* check gzip trailer and save total for deflate */
    إذا (چکری_جانچ != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->کل_باہرلا;
    إذا ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    إذا (gz.بایاں || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    إذا (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (بنانشانی حرف)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    إذا (اكتب(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    إذا (بھریا) {
        rotate(کھڑکی, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    إذا (ret != Z_OK) bye("out of memory", "");
    deflate_سیٹ_Dictionary(strm, کھڑکی, have);
    strm->adler = چکری_جانچ;
    strm->کل_وچ = tot;
    إذا (بایاں) {
        lseek(gz.fd, --سرا, SEEK_SET);
        إذا (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_مفرد(strm, 8 - بایاں, *gz.buf);
    }
    lseek(gz.fd, سرا, SEEK_SET);

    /* clean up and return */
    حرر(کھڑکی);
    حرر(gz.buf);
    أرجع gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local فراغ gztack(حرف *name, صحيح gd, z_دھارا *strm, صحيح آخری)
{
    صحيح fd, len, ret;
    بنانشانی بایاں;
    بنانشانی حرف *وچ, *باہرلا;

    /* open file to compress and append */
    fd = 0;
    إذا (name != عدم) {
        fd = open(name, O_RDONLY, 0);
        إذا (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    وچ = حافظہ_دیو(CHUNK);
    باہرلا = حافظہ_دیو(CHUNK);
    إذا (وچ == عدم || باہرلا == عدم) bye("out of memory", "");

    /* compress input file and append to gzip file */
    افعل {
        /* get more input */
        len = اقرأ(fd, وچ, CHUNK);
        إذا (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_وچ = (بنانشانی)len;
        strm->اگلا_وچ = وچ;
        إذا (len) strm->adler = چکری_جانچ_32_2(strm->adler, وچ, (بنانشانی)len);

        /* compress and write all available output */
        افعل {
            strm->avail_باہرلا = CHUNK;
            strm->اگلا_باہرلا = باہرلا;
            ret = deflate(strm, آخری && len == 0 ? Z_انت : Z_NO_FLUSH);
            بایاں = CHUNK - strm->avail_باہرلا;
            طالما (بایاں) {
                len = اكتب(gd, باہرلا + CHUNK - strm->avail_باہرلا - بایاں, بایاں);
                إذا (len == -1) bye("writing gzip file", "");
                بایاں -= (بنانشانی)len;
            }
        } طالما (strm->avail_باہرلا == 0 && ret != Z_دھارا_سرا);
    } طالما (len != 0);

    /* write trailer after last entry */
    إذا (آخری) {
        deflate_سرا(strm);
        باہرلا[0] = (بنانشانی حرف)(strm->adler);
        باہرلا[1] = (بنانشانی حرف)(strm->adler >> 8);
        باہرلا[2] = (بنانشانی حرف)(strm->adler >> 16);
        باہرلا[3] = (بنانشانی حرف)(strm->adler >> 24);
        باہرلا[4] = (بنانشانی حرف)(strm->کل_وچ);
        باہرلا[5] = (بنانشانی حرف)(strm->کل_وچ >> 8);
        باہرلا[6] = (بنانشانی حرف)(strm->کل_وچ >> 16);
        باہرلا[7] = (بنانشانی حرف)(strm->کل_وچ >> 24);
        len = 8;
        افعل {
            ret = اكتب(gd, باہرلا + 8 - len, len);
            إذا (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } طالما (len);
        close(gd);
    }

    /* clean up and return */
    حرر(باہرلا);
    حرر(وچ);
    إذا (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
صحيح مکھ(صحيح argc, حرف **argv)
{
    صحيح gd, level;
    z_دھارا strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    إذا (*argv == عدم) {
        لکھو(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        لکھو(
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
