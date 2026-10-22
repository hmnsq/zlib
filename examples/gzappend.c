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

#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <fcntl.h>
#شامل <unistd.h>
#شامل "zlib.h"

#تعریف_کرو local ساکن
#تعریف_کرو LGCHUNK 14
#تعریف_کرو CHUNK (1U << LGCHUNK)
#تعریف_کرو DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local خالی bye(حرف *msg1, حرف *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    باہر_نکلو(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local غیرعلامتی عاد_اعظم(غیرعلامتی a, غیرعلامتی b)
{
    غیرعلامتی c;

    جب (a && b)
        اگر (a > b) {
            c = b;
            جب (a - c >= c)
                c <<= 1;
            a -= c;
        }
        ورنہ {
            c = a;
            جب (b - c >= c)
                c <<= 1;
            b -= c;
        }
    واپس a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local خالی rotate(غیرعلامتی حرف *list, غیرعلامتی len, غیرعلامتی rot)
{
    غیرعلامتی حرف tmp;
    غیرعلامتی ادوار;
    غیرعلامتی حرف *آغاز, *آخری, *مقصد, *from;

    /* normalize rot and handle degenerate cases */
    اگر (len < 2) واپس;
    اگر (rot >= len) rot %= len;
    اگر (rot == 0) واپس;

    /* pointer to last entry in list */
    آخری = list + (len - 1);

    /* do simple left shift by one */
    اگر (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *آخری = tmp;
        واپس;
    }

    /* do simple right shift by one */
    اگر (rot == len - 1) {
        tmp = *آخری;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        واپس;
    }

    /* otherwise do rotate as a set of cycles in place */
    ادوار = عاد_اعظم(len, rot);             /* number of cycles */
    کرو {
        آغاز = from = list + ادوار;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        برائے (;;) {
            مقصد = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            اگر (from > آخری) from -= len;   /* (pointer better not wrap) */
            اگر (from == آغاز) توڑو;   /* all but one shifted */
            *مقصد = *from;                /* shift left */
        }
        *مقصد = tmp;                      /* complete the circle */
    } جب (--ادوار);
}

/* structure for gzip file read operations */
قسمتعریف ساخت {
    صحیح fd;                     /* file descriptor */
    صحیح سائز;                   /* 1 << size is bytes in buf */
    غیرعلامتی بایاں;              /* bytes available at next */
    غیرعلامتی حرف *buf;         /*بفر*/
    z_const غیرعلامتی حرف *اگلا;    /* next byte in buffer */
    حرف *name;                 /* file name for error messages */
} file;

/* reload buffer */
local صحیح readin(file *اندر)
{
    صحیح len;

    len = اقرأ(اندر->fd, اندر->buf, 1 << اندر->سائز);
    اگر (len == -1) bye("error reading ", اندر->name);
    اندر->بایاں = (غیرعلامتی)len;
    اندر->اگلا = اندر->buf;
    واپس len;
}

/* read from file in, exit if end-of-file */
local صحیح readmore(file *اندر)
{
    اگر (readin(اندر) == 0) bye("unexpected end of ", اندر->name);
    واپس 0;
}

#تعریف_کرو read1(اندر) (اندر->بایاں == 0 ? readmore(اندر) : 0, \
                   اندر->بایاں--, *(اندر->اگلا)++)

/* skip over n bytes of in */
local خالی skip(file *اندر, غیرعلامتی n)
{
    غیرعلامتی bypass;

    اگر (n > اندر->بایاں) {
        n -= اندر->بایاں;
        bypass = n & ~((1U << اندر->سائز) - 1);
        اگر (bypass) {
            اگر (lseek(اندر->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", اندر->name);
            n -= bypass;
        }
        readmore(اندر);
        اگر (n > اندر->بایاں)
            bye("unexpected end of ", اندر->name);
    }
    اندر->بایاں -= n;
    اندر->اگلا += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
غیرعلامتی طویل read4(file *اندر)
{
    غیرعلامتی طویل val;

    val = read1(اندر);
    val += (غیرعلامتی)read1(اندر) << 8;
    val += (غیرعلامتی طویل)read1(اندر) << 16;
    val += (غیرعلامتی طویل)read1(اندر) << 24;
    واپس val;
}

/* skip over gzip header */
local خالی gzheader(file *اندر)
{
    صحیح flags;
    غیرعلامتی n;

    اگر (read1(اندر) != 31 || read1(اندر) != 139) bye(اندر->name, " not a gzip file");
    اگر (read1(اندر) != 8) bye("unknown compression method in", اندر->name);
    flags = read1(اندر);
    اگر (flags & 0xe0) bye("unknown header flags set in", اندر->name);
    skip(اندر, 6);
    اگر (flags & 4) {
        n = read1(اندر);
        n += (غیرعلامتی)(read1(اندر)) << 8;
        skip(اندر, n);
    }
    اگر (flags & 8) جب (read1(اندر) != 0) ;
    اگر (flags & 16) جب (read1(اندر) != 0) ;
    اگر (flags & 2) skip(اندر, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local صحیح gzscan(حرف *name, z_دھارا *strm, صحیح level)
{
    صحیح ret, lastbit, بایاں, بھرا;
    غیرعلامتی have;
    غیرعلامتی طویل چکری_جانچ, tot;
    غیرعلامتی حرف *کھڑکی;
    off_t lastoff, انتہا;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    اگر (gz.fd == -1) bye("cannot open ", name);
    gz.buf = حافظہ_دو(CHUNK);
    اگر (gz.buf == تہی) bye("out of memory", "");
    gz.سائز = LGCHUNK;
    gz.بایاں = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    کھڑکی = حافظہ_دو(DSIZE);
    اگر (کھڑکی == تہی) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    اگر (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.بایاں;
    بایاں = 0;
    strm->avail_اندر = gz.بایاں;
    strm->اگلا_اندر = gz.اگلا;
    چکری_جانچ = چکری_جانچ_32_2(0L, Z_NULL, 0);
    have = بھرا = 0;
    کرو {
        /* if needed, get more input */
        اگر (strm->avail_اندر == 0) {
            readmore(&gz);
            strm->avail_اندر = gz.بایاں;
            strm->اگلا_اندر = gz.اگلا;
        }

        /* set up output to next available section of sliding window */
        strm->avail_برآمد = DSIZE - have;
        strm->اگلا_برآمد = کھڑکی + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_بلاک);
        اگر (ret == Z_دھارا_ERROR) bye("internal stream error!", "");
        اگر (ret == Z_MEM_ERROR) bye("out of memory", "");
        اگر (ret == Z_ڈیٹا_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        چکری_جانچ = چکری_جانچ_32_2(چکری_جانچ, کھڑکی + have, DSIZE - have - strm->avail_برآمد);
        اگر (strm->avail_برآمد)
            have = DSIZE - strm->avail_برآمد;
        ورنہ {
            have = 0;
            بھرا = 1;
        }

        /* process end of block */
        اگر (strm->ڈیٹا_type & 128) {
            اگر (strm->ڈیٹا_type & 64)
                بایاں = strm->ڈیٹا_type & 0x1f;
            ورنہ {
                lastbit = strm->ڈیٹا_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_اندر;
            }
        }
    } جب (ret != Z_دھارا_انتہا);
    inflate_انتہا(strm);
    gz.بایاں = strm->avail_اندر;
    gz.اگلا = strm->اگلا_اندر;

    /* save the location of the end of the compressed data */
    انتہا = lseek(gz.fd, 0L, SEEK_CUR) - gz.بایاں;

    /* check gzip trailer and save total for deflate */
    اگر (چکری_جانچ != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->کل_برآمد;
    اگر ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    اگر (gz.بایاں || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    اگر (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (غیرعلامتی حرف)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    اگر (اكتب(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    اگر (بھرا) {
        rotate(کھڑکی, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    اگر (ret != Z_OK) bye("out of memory", "");
    deflate_سیٹ_Dictionary(strm, کھڑکی, have);
    strm->adler = چکری_جانچ;
    strm->کل_اندر = tot;
    اگر (بایاں) {
        lseek(gz.fd, --انتہا, SEEK_SET);
        اگر (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_مفرد(strm, 8 - بایاں, *gz.buf);
    }
    lseek(gz.fd, انتہا, SEEK_SET);

    /* clean up and return */
    آزاد(کھڑکی);
    آزاد(gz.buf);
    واپس gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local خالی gztack(حرف *name, صحیح gd, z_دھارا *strm, صحیح آخری)
{
    صحیح fd, len, ret;
    غیرعلامتی بایاں;
    غیرعلامتی حرف *اندر, *برآمد;

    /* open file to compress and append */
    fd = 0;
    اگر (name != تہی) {
        fd = open(name, O_RDONLY, 0);
        اگر (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    اندر = حافظہ_دو(CHUNK);
    برآمد = حافظہ_دو(CHUNK);
    اگر (اندر == تہی || برآمد == تہی) bye("out of memory", "");

    /* compress input file and append to gzip file */
    کرو {
        /* get more input */
        len = اقرأ(fd, اندر, CHUNK);
        اگر (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_اندر = (غیرعلامتی)len;
        strm->اگلا_اندر = اندر;
        اگر (len) strm->adler = چکری_جانچ_32_2(strm->adler, اندر, (غیرعلامتی)len);

        /* compress and write all available output */
        کرو {
            strm->avail_برآمد = CHUNK;
            strm->اگلا_برآمد = برآمد;
            ret = deflate(strm, آخری && len == 0 ? Z_اختتام : Z_NO_FLUSH);
            بایاں = CHUNK - strm->avail_برآمد;
            جب (بایاں) {
                len = اكتب(gd, برآمد + CHUNK - strm->avail_برآمد - بایاں, بایاں);
                اگر (len == -1) bye("writing gzip file", "");
                بایاں -= (غیرعلامتی)len;
            }
        } جب (strm->avail_برآمد == 0 && ret != Z_دھارا_انتہا);
    } جب (len != 0);

    /* write trailer after last entry */
    اگر (آخری) {
        deflate_انتہا(strm);
        برآمد[0] = (غیرعلامتی حرف)(strm->adler);
        برآمد[1] = (غیرعلامتی حرف)(strm->adler >> 8);
        برآمد[2] = (غیرعلامتی حرف)(strm->adler >> 16);
        برآمد[3] = (غیرعلامتی حرف)(strm->adler >> 24);
        برآمد[4] = (غیرعلامتی حرف)(strm->کل_اندر);
        برآمد[5] = (غیرعلامتی حرف)(strm->کل_اندر >> 8);
        برآمد[6] = (غیرعلامتی حرف)(strm->کل_اندر >> 16);
        برآمد[7] = (غیرعلامتی حرف)(strm->کل_اندر >> 24);
        len = 8;
        کرو {
            ret = اكتب(gd, برآمد + 8 - len, len);
            اگر (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } جب (len);
        close(gd);
    }

    /* clean up and return */
    آزاد(برآمد);
    آزاد(اندر);
    اگر (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
صحیح مین(صحیح argc, حرف **argv)
{
    صحیح gd, level;
    z_دھارا strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    اگر (*argv == تہی) {
        لکھو(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        لکھو(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        واپس 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    اگر (argv[0][0] == '-') {
        اگر (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        اگر (*++argv == تہی) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    اگر (*argv == تہی)
        gztack(تہی, gd, &strm, 1);
    ورنہ
        کرو {
            gztack(*argv, gd, &strm, argv[1] == تہی);
        } جب (*++argv != تہی);
    واپس 0;
}
