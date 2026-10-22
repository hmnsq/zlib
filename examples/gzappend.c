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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ "zlib.h"

#నిర్వచించు local స్థిర
#నిర్వచించు LGCHUNK 14
#నిర్వచించు CHUNK (1U << LGCHUNK)
#నిర్వచించు DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local శూన్య bye(అక్షరం *msg1, అక్షరం *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    నిష్క్రమించు(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local గుర్తులేని గరిష్ఠ_సామాన్య_భాజకం(గుర్తులేని a, గుర్తులేని b)
{
    గుర్తులేని c;

    అయితే (a && b)
        ఒకవేళ (a > b) {
            c = b;
            అయితే (a - c >= c)
                c <<= 1;
            a -= c;
        }
        లేకపోతే {
            c = a;
            అయితే (b - c >= c)
                c <<= 1;
            b -= c;
        }
    తిరిగి a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local శూన్య rotate(గుర్తులేని అక్షరం *list, గుర్తులేని len, గుర్తులేని rot)
{
    గుర్తులేని అక్షరం tmp;
    గుర్తులేని చక్రాలు;
    గుర్తులేని అక్షరం *ప్రారంభం, *చివరి, *గమ్యం, *from;

    /* normalize rot and handle degenerate cases */
    ఒకవేళ (len < 2) తిరిగి;
    ఒకవేళ (rot >= len) rot %= len;
    ఒకవేళ (rot == 0) తిరిగి;

    /* pointer to last entry in list */
    చివరి = list + (len - 1);

    /* do simple left shift by one */
    ఒకవేళ (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *చివరి = tmp;
        తిరిగి;
    }

    /* do simple right shift by one */
    ఒకవేళ (rot == len - 1) {
        tmp = *చివరి;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        తిరిగి;
    }

    /* otherwise do rotate as a set of cycles in place */
    చక్రాలు = గరిష్ఠ_సామాన్య_భాజకం(len, rot);             /* number of cycles */
    చేయి {
        ప్రారంభం = from = list + చక్రాలు;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        కొరకు (;;) {
            గమ్యం = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            ఒకవేళ (from > చివరి) from -= len;   /* (pointer better not wrap) */
            ఒకవేళ (from == ప్రారంభం) ఆపు;   /* all but one shifted */
            *గమ్యం = *from;                /* shift left */
        }
        *గమ్యం = tmp;                      /* complete the circle */
    } అయితే (--చక్రాలు);
}

/* structure for gzip file read operations */
రకనిర్వచనం నిర్మాణం {
    పూర్ణాంకం fd;                     /* file descriptor */
    పూర్ణాంకం కొలత;                   /* 1 << size is bytes in buf */
    గుర్తులేని ఎడమ;              /* bytes available at next */
    గుర్తులేని అక్షరం *buf;         /*బఫరు*/
    z_const గుర్తులేని అక్షరం *తదుపరి;    /* next byte in buffer */
    అక్షరం *name;                 /* file name for error messages */
} file;

/* reload buffer */
local పూర్ణాంకం readin(file *లోపల)
{
    పూర్ణాంకం len;

    len = పఠ(లోపల->fd, లోపల->buf, 1 << లోపల->కొలత);
    ఒకవేళ (len == -1) bye("error reading ", లోపల->name);
    లోపల->ఎడమ = (గుర్తులేని)len;
    లోపల->తదుపరి = లోపల->buf;
    తిరిగి len;
}

/* read from file in, exit if end-of-file */
local పూర్ణాంకం readmore(file *లోపల)
{
    ఒకవేళ (readin(లోపల) == 0) bye("unexpected end of ", లోపల->name);
    తిరిగి 0;
}

#నిర్వచించు read1(లోపల) (లోపల->ఎడమ == 0 ? readmore(లోపల) : 0, \
                   లోపల->ఎడమ--, *(లోపల->తదుపరి)++)

/* skip over n bytes of in */
local శూన్య skip(file *లోపల, గుర్తులేని n)
{
    గుర్తులేని bypass;

    ఒకవేళ (n > లోపల->ఎడమ) {
        n -= లోపల->ఎడమ;
        bypass = n & ~((1U << లోపల->కొలత) - 1);
        ఒకవేళ (bypass) {
            ఒకవేళ (lseek(లోపల->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", లోపల->name);
            n -= bypass;
        }
        readmore(లోపల);
        ఒకవేళ (n > లోపల->ఎడమ)
            bye("unexpected end of ", లోపల->name);
    }
    లోపల->ఎడమ -= n;
    లోపల->తదుపరి += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
గుర్తులేని దీర్ఘ read4(file *లోపల)
{
    గుర్తులేని దీర్ఘ val;

    val = read1(లోపల);
    val += (గుర్తులేని)read1(లోపల) << 8;
    val += (గుర్తులేని దీర్ఘ)read1(లోపల) << 16;
    val += (గుర్తులేని దీర్ఘ)read1(లోపల) << 24;
    తిరిగి val;
}

/* skip over gzip header */
local శూన్య gzheader(file *లోపల)
{
    పూర్ణాంకం flags;
    గుర్తులేని n;

    ఒకవేళ (read1(లోపల) != 31 || read1(లోపల) != 139) bye(లోపల->name, " not a gzip file");
    ఒకవేళ (read1(లోపల) != 8) bye("unknown compression method in", లోపల->name);
    flags = read1(లోపల);
    ఒకవేళ (flags & 0xe0) bye("unknown header flags set in", లోపల->name);
    skip(లోపల, 6);
    ఒకవేళ (flags & 4) {
        n = read1(లోపల);
        n += (గుర్తులేని)(read1(లోపల)) << 8;
        skip(లోపల, n);
    }
    ఒకవేళ (flags & 8) అయితే (read1(లోపల) != 0) ;
    ఒకవేళ (flags & 16) అయితే (read1(లోపల) != 0) ;
    ఒకవేళ (flags & 2) skip(లోపల, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local పూర్ణాంకం gzscan(అక్షరం *name, z_ప్రవాహం *strm, పూర్ణాంకం level)
{
    పూర్ణాంకం ret, lastbit, ఎడమ, నిండిన;
    గుర్తులేని have;
    గుర్తులేని దీర్ఘ చక్రీయ_తనిఖీ, tot;
    గుర్తులేని అక్షరం *కిటికీ;
    off_t lastoff, చివర;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    ఒకవేళ (gz.fd == -1) bye("cannot open ", name);
    gz.buf = జ్ఞాపకం_ఇవ్వు(CHUNK);
    ఒకవేళ (gz.buf == ఖాళీ) bye("out of memory", "");
    gz.కొలత = LGCHUNK;
    gz.ఎడమ = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    కిటికీ = జ్ఞాపకం_ఇవ్వు(DSIZE);
    ఒకవేళ (కిటికీ == ఖాళీ) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    ఒకవేళ (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.ఎడమ;
    ఎడమ = 0;
    strm->avail_లోపల = gz.ఎడమ;
    strm->తదుపరి_లోపల = gz.తదుపరి;
    చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0);
    have = నిండిన = 0;
    చేయి {
        /* if needed, get more input */
        ఒకవేళ (strm->avail_లోపల == 0) {
            readmore(&gz);
            strm->avail_లోపల = gz.ఎడమ;
            strm->తదుపరి_లోపల = gz.తదుపరి;
        }

        /* set up output to next available section of sliding window */
        strm->avail_బహిర్గతం = DSIZE - have;
        strm->తదుపరి_బహిర్గతం = కిటికీ + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_ఖండం);
        ఒకవేళ (ret == Z_ప్రవాహం_ERROR) bye("internal stream error!", "");
        ఒకవేళ (ret == Z_MEM_ERROR) bye("out of memory", "");
        ఒకవేళ (ret == Z_దత్తాంశం_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_2(చక్రీయ_తనిఖీ, కిటికీ + have, DSIZE - have - strm->avail_బహిర్గతం);
        ఒకవేళ (strm->avail_బహిర్గతం)
            have = DSIZE - strm->avail_బహిర్గతం;
        లేకపోతే {
            have = 0;
            నిండిన = 1;
        }

        /* process end of block */
        ఒకవేళ (strm->దత్తాంశం_type & 128) {
            ఒకవేళ (strm->దత్తాంశం_type & 64)
                ఎడమ = strm->దత్తాంశం_type & 0x1f;
            లేకపోతే {
                lastbit = strm->దత్తాంశం_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_లోపల;
            }
        }
    } అయితే (ret != Z_ప్రవాహం_చివర);
    inflate_చివర(strm);
    gz.ఎడమ = strm->avail_లోపల;
    gz.తదుపరి = strm->తదుపరి_లోపల;

    /* save the location of the end of the compressed data */
    చివర = lseek(gz.fd, 0L, SEEK_CUR) - gz.ఎడమ;

    /* check gzip trailer and save total for deflate */
    ఒకవేళ (చక్రీయ_తనిఖీ != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->మొత్తం_బహిర్గతం;
    ఒకవేళ ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    ఒకవేళ (gz.ఎడమ || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    ఒకవేళ (పఠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (గుర్తులేని అక్షరం)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    ఒకవేళ (లిఖ(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    ఒకవేళ (నిండిన) {
        rotate(కిటికీ, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    ఒకవేళ (ret != Z_OK) bye("out of memory", "");
    deflate_సమితి_Dictionary(strm, కిటికీ, have);
    strm->adler = చక్రీయ_తనిఖీ;
    strm->మొత్తం_లోపల = tot;
    ఒకవేళ (ఎడమ) {
        lseek(gz.fd, --చివర, SEEK_SET);
        ఒకవేళ (పఠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_ప్రధానసంఖ్య(strm, 8 - ఎడమ, *gz.buf);
    }
    lseek(gz.fd, చివర, SEEK_SET);

    /* clean up and return */
    విడుదల(కిటికీ);
    విడుదల(gz.buf);
    తిరిగి gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local శూన్య gztack(అక్షరం *name, పూర్ణాంకం gd, z_ప్రవాహం *strm, పూర్ణాంకం చివరి)
{
    పూర్ణాంకం fd, len, ret;
    గుర్తులేని ఎడమ;
    గుర్తులేని అక్షరం *లోపల, *బహిర్గతం;

    /* open file to compress and append */
    fd = 0;
    ఒకవేళ (name != ఖాళీ) {
        fd = open(name, O_RDONLY, 0);
        ఒకవేళ (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    లోపల = జ్ఞాపకం_ఇవ్వు(CHUNK);
    బహిర్గతం = జ్ఞాపకం_ఇవ్వు(CHUNK);
    ఒకవేళ (లోపల == ఖాళీ || బహిర్గతం == ఖాళీ) bye("out of memory", "");

    /* compress input file and append to gzip file */
    చేయి {
        /* get more input */
        len = పఠ(fd, లోపల, CHUNK);
        ఒకవేళ (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_లోపల = (గుర్తులేని)len;
        strm->తదుపరి_లోపల = లోపల;
        ఒకవేళ (len) strm->adler = చక్రీయ_తనిఖీ_32_2(strm->adler, లోపల, (గుర్తులేని)len);

        /* compress and write all available output */
        చేయి {
            strm->avail_బహిర్గతం = CHUNK;
            strm->తదుపరి_బహిర్గతం = బహిర్గతం;
            ret = deflate(strm, చివరి && len == 0 ? Z_ముగింపు : Z_NO_FLUSH);
            ఎడమ = CHUNK - strm->avail_బహిర్గతం;
            అయితే (ఎడమ) {
                len = లిఖ(gd, బహిర్గతం + CHUNK - strm->avail_బహిర్గతం - ఎడమ, ఎడమ);
                ఒకవేళ (len == -1) bye("writing gzip file", "");
                ఎడమ -= (గుర్తులేని)len;
            }
        } అయితే (strm->avail_బహిర్గతం == 0 && ret != Z_ప్రవాహం_చివర);
    } అయితే (len != 0);

    /* write trailer after last entry */
    ఒకవేళ (చివరి) {
        deflate_చివర(strm);
        బహిర్గతం[0] = (గుర్తులేని అక్షరం)(strm->adler);
        బహిర్గతం[1] = (గుర్తులేని అక్షరం)(strm->adler >> 8);
        బహిర్గతం[2] = (గుర్తులేని అక్షరం)(strm->adler >> 16);
        బహిర్గతం[3] = (గుర్తులేని అక్షరం)(strm->adler >> 24);
        బహిర్గతం[4] = (గుర్తులేని అక్షరం)(strm->మొత్తం_లోపల);
        బహిర్గతం[5] = (గుర్తులేని అక్షరం)(strm->మొత్తం_లోపల >> 8);
        బహిర్గతం[6] = (గుర్తులేని అక్షరం)(strm->మొత్తం_లోపల >> 16);
        బహిర్గతం[7] = (గుర్తులేని అక్షరం)(strm->మొత్తం_లోపల >> 24);
        len = 8;
        చేయి {
            ret = లిఖ(gd, బహిర్గతం + 8 - len, len);
            ఒకవేళ (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } అయితే (len);
        close(gd);
    }

    /* clean up and return */
    విడుదల(బహిర్గతం);
    విడుదల(లోపల);
    ఒకవేళ (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
    పూర్ణాంకం gd, level;
    z_ప్రవాహం strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    ఒకవేళ (*argv == ఖాళీ) {
        రాయి(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        రాయి(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        తిరిగి 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    ఒకవేళ (argv[0][0] == '-') {
        ఒకవేళ (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        ఒకవేళ (*++argv == ఖాళీ) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    ఒకవేళ (*argv == ఖాళీ)
        gztack(ఖాళీ, gd, &strm, 1);
    లేకపోతే
        చేయి {
            gztack(*argv, gd, &strm, argv[1] == ఖాళీ);
        } అయితే (*++argv != ఖాళీ);
    తిరిగి 0;
}
