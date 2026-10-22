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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव "zlib.h"

#परिभाषय local स्थिर
#परिभाषय LGCHUNK 14
#परिभाषय CHUNK (1U << LGCHUNK)
#परिभाषय DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local शून्य bye(वर्ण *msg1, वर्ण *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    निर्गच्छ(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local अचिह्नित gcd(अचिह्नित a, अचिह्नित b)
{
    अचिह्नित c;

    यावत् (a && b)
        यदि (a > b) {
            c = b;
            यावत् (a - c >= c)
                c <<= 1;
            a -= c;
        }
        अन्यथा {
            c = a;
            यावत् (b - c >= c)
                c <<= 1;
            b -= c;
        }
    प्रत्यागम a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local शून्य rotate(अचिह्नित वर्ण *list, अचिह्नित len, अचिह्नित rot)
{
    अचिह्नित वर्ण tmp;
    अचिह्नित आवृत्तयः;
    अचिह्नित वर्ण *start, *चरमः, *गन्तव्यम्, *from;

    /* normalize rot and handle degenerate cases */
    यदि (len < 2) प्रत्यागम;
    यदि (rot >= len) rot %= len;
    यदि (rot == 0) प्रत्यागम;

    /* pointer to last entry in list */
    चरमः = list + (len - 1);

    /* do simple left shift by one */
    यदि (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *चरमः = tmp;
        प्रत्यागम;
    }

    /* do simple right shift by one */
    यदि (rot == len - 1) {
        tmp = *चरमः;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        प्रत्यागम;
    }

    /* otherwise do rotate as a set of cycles in place */
    आवृत्तयः = gcd(len, rot);             /* number of cycles */
    कुरु {
        start = from = list + आवृत्तयः;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        प्रति (;;) {
            गन्तव्यम् = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            यदि (from > चरमः) from -= len;   /* (pointer better not wrap) */
            यदि (from == start) भङ्ग;   /* all but one shifted */
            *गन्तव्यम् = *from;                /* shift left */
        }
        *गन्तव्यम् = tmp;                      /* complete the circle */
    } यावत् (--आवृत्तयः);
}

/* structure for gzip file read operations */
प्रकारनिर्देश संरचना {
    पूर्णाङ्क fd;                     /* file descriptor */
    पूर्णाङ्क आकार;                   /* 1 << size is bytes in buf */
    अचिह्नित वामः;              /* bytes available at next */
    अचिह्नित वर्ण *buf;         /*बफर*/
    z_const अचिह्नित वर्ण *अग्र;    /* next byte in buffer */
    वर्ण *name;                 /* file name for error messages */
} file;

/* reload buffer */
local पूर्णाङ्क readin(file *अन्तर्)
{
    पूर्णाङ्क len;

    len = पठ(अन्तर्->fd, अन्तर्->buf, 1 << अन्तर्->आकार);
    यदि (len == -1) bye("error reading ", अन्तर्->name);
    अन्तर्->वामः = (अचिह्नित)len;
    अन्तर्->अग्र = अन्तर्->buf;
    प्रत्यागम len;
}

/* read from file in, exit if end-of-file */
local पूर्णाङ्क readmore(file *अन्तर्)
{
    यदि (readin(अन्तर्) == 0) bye("unexpected end of ", अन्तर्->name);
    प्रत्यागम 0;
}

#परिभाषय read1(अन्तर्) (अन्तर्->वामः == 0 ? readmore(अन्तर्) : 0, \
                   अन्तर्->वामः--, *(अन्तर्->अग्र)++)

/* skip over n bytes of in */
local शून्य skip(file *अन्तर्, अचिह्नित n)
{
    अचिह्नित bypass;

    यदि (n > अन्तर्->वामः) {
        n -= अन्तर्->वामः;
        bypass = n & ~((1U << अन्तर्->आकार) - 1);
        यदि (bypass) {
            यदि (lseek(अन्तर्->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", अन्तर्->name);
            n -= bypass;
        }
        readmore(अन्तर्);
        यदि (n > अन्तर्->वामः)
            bye("unexpected end of ", अन्तर्->name);
    }
    अन्तर्->वामः -= n;
    अन्तर्->अग्र += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
अचिह्नित दीर्घ read4(file *अन्तर्)
{
    अचिह्नित दीर्घ val;

    val = read1(अन्तर्);
    val += (अचिह्नित)read1(अन्तर्) << 8;
    val += (अचिह्नित दीर्घ)read1(अन्तर्) << 16;
    val += (अचिह्नित दीर्घ)read1(अन्तर्) << 24;
    प्रत्यागम val;
}

/* skip over gzip header */
local शून्य gzheader(file *अन्तर्)
{
    पूर्णाङ्क flags;
    अचिह्नित n;

    यदि (read1(अन्तर्) != 31 || read1(अन्तर्) != 139) bye(अन्तर्->name, " not a gzip file");
    यदि (read1(अन्तर्) != 8) bye("unknown compression method in", अन्तर्->name);
    flags = read1(अन्तर्);
    यदि (flags & 0xe0) bye("unknown header flags set in", अन्तर्->name);
    skip(अन्तर्, 6);
    यदि (flags & 4) {
        n = read1(अन्तर्);
        n += (अचिह्नित)(read1(अन्तर्)) << 8;
        skip(अन्तर्, n);
    }
    यदि (flags & 8) यावत् (read1(अन्तर्) != 0) ;
    यदि (flags & 16) यावत् (read1(अन्तर्) != 0) ;
    यदि (flags & 2) skip(अन्तर्, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local पूर्णाङ्क gzscan(वर्ण *name, z_प्रवाहः *strm, पूर्णाङ्क level)
{
    पूर्णाङ्क ret, lastbit, वामः, पूर्णम्;
    अचिह्नित have;
    अचिह्नित दीर्घ crc, tot;
    अचिह्नित वर्ण *गवाक्षः;
    off_t lastoff, अन्तः_2;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = उद्घाटय(name, O_RDWR, 0);
    यदि (gz.fd == -1) bye("cannot open ", name);
    gz.buf = स्मृतिं_देहि(CHUNK);
    यदि (gz.buf == रिक्त) bye("out of memory", "");
    gz.आकार = LGCHUNK;
    gz.वामः = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    गवाक्षः = स्मृतिं_देहि(DSIZE);
    यदि (गवाक्षः == रिक्त) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    यदि (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.वामः;
    वामः = 0;
    strm->avail_अन्तर् = gz.वामः;
    strm->अग्र_अन्तर् = gz.अग्र;
    crc = चक्रीयपरीक्षा_32_2(0L, Z_NULL, 0);
    have = पूर्णम् = 0;
    कुरु {
        /* if needed, get more input */
        यदि (strm->avail_अन्तर् == 0) {
            readmore(&gz);
            strm->avail_अन्तर् = gz.वामः;
            strm->अग्र_अन्तर् = gz.अग्र;
        }

        /* set up output to next available section of sliding window */
        strm->avail_बहिर्गमनम् = DSIZE - have;
        strm->अग्र_बहिर्गमनम् = गवाक्षः + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_खण्डम्);
        यदि (ret == Z_प्रवाहः_ERROR) bye("internal stream error!", "");
        यदि (ret == Z_MEM_ERROR) bye("out of memory", "");
        यदि (ret == Z_दत्त_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        crc = चक्रीयपरीक्षा_32_2(crc, गवाक्षः + have, DSIZE - have - strm->avail_बहिर्गमनम्);
        यदि (strm->avail_बहिर्गमनम्)
            have = DSIZE - strm->avail_बहिर्गमनम्;
        अन्यथा {
            have = 0;
            पूर्णम् = 1;
        }

        /* process end of block */
        यदि (strm->दत्त_type & 128) {
            यदि (strm->दत्त_type & 64)
                वामः = strm->दत्त_type & 0x1f;
            अन्यथा {
                lastbit = strm->दत्त_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_अन्तर्;
            }
        }
    } यावत् (ret != Z_प्रवाहः_अन्तः);
    inflate_अन्तः(strm);
    gz.वामः = strm->avail_अन्तर्;
    gz.अग्र = strm->अग्र_अन्तर्;

    /* save the location of the end of the compressed data */
    अन्तः_2 = lseek(gz.fd, 0L, SEEK_CUR) - gz.वामः;

    /* check gzip trailer and save total for deflate */
    यदि (crc != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->योग_बहिर्गमनम्;
    यदि ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    यदि (gz.वामः || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    यदि (पठ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (अचिह्नित वर्ण)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    यदि (लिख(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    यदि (पूर्णम्) {
        rotate(गवाक्षः, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    यदि (ret != Z_OK) bye("out of memory", "");
    deflate_समुच्चयः_Dictionary(strm, गवाक्षः, have);
    strm->adler = crc;
    strm->योग_अन्तर् = tot;
    यदि (वामः) {
        lseek(gz.fd, --अन्तः_2, SEEK_SET);
        यदि (पठ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_अविभाज्यः(strm, 8 - वामः, *gz.buf);
    }
    lseek(gz.fd, अन्तः_2, SEEK_SET);

    /* clean up and return */
    मुक्त(गवाक्षः);
    मुक्त(gz.buf);
    प्रत्यागम gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local शून्य gztack(वर्ण *name, पूर्णाङ्क gd, z_प्रवाहः *strm, पूर्णाङ्क चरमः)
{
    पूर्णाङ्क fd, len, ret;
    अचिह्नित वामः;
    अचिह्नित वर्ण *अन्तर्, *बहिर्गमनम्;

    /* open file to compress and append */
    fd = 0;
    यदि (name != रिक्त) {
        fd = उद्घाटय(name, O_RDONLY, 0);
        यदि (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    अन्तर् = स्मृतिं_देहि(CHUNK);
    बहिर्गमनम् = स्मृतिं_देहि(CHUNK);
    यदि (अन्तर् == रिक्त || बहिर्गमनम् == रिक्त) bye("out of memory", "");

    /* compress input file and append to gzip file */
    कुरु {
        /* get more input */
        len = पठ(fd, अन्तर्, CHUNK);
        यदि (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_अन्तर् = (अचिह्नित)len;
        strm->अग्र_अन्तर् = अन्तर्;
        यदि (len) strm->adler = चक्रीयपरीक्षा_32_2(strm->adler, अन्तर्, (अचिह्नित)len);

        /* compress and write all available output */
        कुरु {
            strm->avail_बहिर्गमनम् = CHUNK;
            strm->अग्र_बहिर्गमनम् = बहिर्गमनम्;
            ret = deflate(strm, चरमः && len == 0 ? Z_FINISH : Z_NO_FLUSH);
            वामः = CHUNK - strm->avail_बहिर्गमनम्;
            यावत् (वामः) {
                len = लिख(gd, बहिर्गमनम् + CHUNK - strm->avail_बहिर्गमनम् - वामः, वामः);
                यदि (len == -1) bye("writing gzip file", "");
                वामः -= (अचिह्नित)len;
            }
        } यावत् (strm->avail_बहिर्गमनम् == 0 && ret != Z_प्रवाहः_अन्तः);
    } यावत् (len != 0);

    /* write trailer after last entry */
    यदि (चरमः) {
        deflate_अन्तः(strm);
        बहिर्गमनम्[0] = (अचिह्नित वर्ण)(strm->adler);
        बहिर्गमनम्[1] = (अचिह्नित वर्ण)(strm->adler >> 8);
        बहिर्गमनम्[2] = (अचिह्नित वर्ण)(strm->adler >> 16);
        बहिर्गमनम्[3] = (अचिह्नित वर्ण)(strm->adler >> 24);
        बहिर्गमनम्[4] = (अचिह्नित वर्ण)(strm->योग_अन्तर्);
        बहिर्गमनम्[5] = (अचिह्नित वर्ण)(strm->योग_अन्तर् >> 8);
        बहिर्गमनम्[6] = (अचिह्नित वर्ण)(strm->योग_अन्तर् >> 16);
        बहिर्गमनम्[7] = (अचिह्नित वर्ण)(strm->योग_अन्तर् >> 24);
        len = 8;
        कुरु {
            ret = लिख(gd, बहिर्गमनम् + 8 - len, len);
            यदि (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } यावत् (len);
        close(gd);
    }

    /* clean up and return */
    मुक्त(बहिर्गमनम्);
    मुक्त(अन्तर्);
    यदि (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
    पूर्णाङ्क gd, level;
    z_प्रवाहः strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    यदि (*argv == रिक्त) {
        प्रारूप_लिख(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        प्रारूप_लिख(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        प्रत्यागम 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    यदि (argv[0][0] == '-') {
        यदि (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        यदि (*++argv == रिक्त) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    यदि (*argv == रिक्त)
        gztack(रिक्त, gd, &strm, 1);
    अन्यथा
        कुरु {
            gztack(*argv, gd, &strm, argv[1] == रिक्त);
        } यावत् (*++argv != रिक्त);
    प्रत्यागम 0;
}
