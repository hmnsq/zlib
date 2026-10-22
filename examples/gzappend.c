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

#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ "zlib.h"

#define local স্থির
#define LGCHUNK 14
#define CHUNK (1U << LGCHUNK)
#define DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local শূন্য bye(ৱর্ণ *msg1, ৱর্ণ *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    exit(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local unsigned gcd(unsigned a, unsigned b)
{
    unsigned c;

    যেতিয়ালৈ (a && b)
        যদি (a > b) {
            c = b;
            যেতিয়ালৈ (a - c >= c)
                c <<= 1;
            a -= c;
        }
        অন্যথা {
            c = a;
            যেতিয়ালৈ (b - c >= c)
                c <<= 1;
            b -= c;
        }
    ঘূৰাই a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local শূন্য rotate(unsigned ৱর্ণ *list, unsigned len, unsigned rot)
{
    unsigned ৱর্ণ tmp;
    unsigned cycles;
    unsigned ৱর্ণ *start, *last, *to, *from;

    /* normalize rot and handle degenerate cases */
    যদি (len < 2) ঘূৰাই;
    যদি (rot >= len) rot %= len;
    যদি (rot == 0) ঘূৰাই;

    /* pointer to last entry in list */
    last = list + (len - 1);

    /* do simple left shift by one */
    যদি (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *last = tmp;
        ঘূৰাই;
    }

    /* do simple right shift by one */
    যদি (rot == len - 1) {
        tmp = *last;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        ঘূৰাই;
    }

    /* otherwise do rotate as a set of cycles in place */
    cycles = gcd(len, rot);             /* number of cycles */
    কৰা {
        start = from = list + cycles;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        বাবে (;;) {
            to = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            যদি (from > last) from -= len;   /* (pointer better not wrap) */
            যদি (from == start) ভঙ্গ;   /* all but one shifted */
            *to = *from;                /* shift left */
        }
        *to = tmp;                      /* complete the circle */
    } যেতিয়ালৈ (--cycles);
}

/* structure for gzip file read operations */
প্রকারনির্দেশ সংরচনা {
    পূর্ণাঙ্ক fd;                     /* file descriptor */
    পূর্ণাঙ্ক size;                   /* 1 << size is bytes in buf */
    unsigned বাম;              /* bytes available at next */
    unsigned ৱর্ণ *buf;         /* buffer */
    z_const unsigned ৱর্ণ *next;    /* next byte in buffer */
    ৱর্ণ *name;                 /* file name for error messages */
} file;

/* reload buffer */
local পূর্ণাঙ্ক readin(file *in)
{
    পূর্ণাঙ্ক len;

    len = পঠ(in->fd, in->buf, 1 << in->size);
    যদি (len == -1) bye("error reading ", in->name);
    in->বাম = (unsigned)len;
    in->next = in->buf;
    ঘূৰাই len;
}

/* read from file in, exit if end-of-file */
local পূর্ণাঙ্ক readmore(file *in)
{
    যদি (readin(in) == 0) bye("unexpected end of ", in->name);
    ঘূৰাই 0;
}

#define read1(in) (in->বাম == 0 ? readmore(in) : 0, \
                   in->বাম--, *(in->next)++)

/* skip over n bytes of in */
local শূন্য skip(file *in, unsigned n)
{
    unsigned bypass;

    যদি (n > in->বাম) {
        n -= in->বাম;
        bypass = n & ~((1U << in->size) - 1);
        যদি (bypass) {
            যদি (lseek(in->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", in->name);
            n -= bypass;
        }
        readmore(in);
        যদি (n > in->বাম)
            bye("unexpected end of ", in->name);
    }
    in->বাম -= n;
    in->next += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
unsigned long read4(file *in)
{
    unsigned long val;

    val = read1(in);
    val += (unsigned)read1(in) << 8;
    val += (unsigned long)read1(in) << 16;
    val += (unsigned long)read1(in) << 24;
    ঘূৰাই val;
}

/* skip over gzip header */
local শূন্য gzheader(file *in)
{
    পূর্ণাঙ্ক flags;
    unsigned n;

    যদি (read1(in) != 31 || read1(in) != 139) bye(in->name, " not a gzip file");
    যদি (read1(in) != 8) bye("unknown compression method in", in->name);
    flags = read1(in);
    যদি (flags & 0xe0) bye("unknown header flags set in", in->name);
    skip(in, 6);
    যদি (flags & 4) {
        n = read1(in);
        n += (unsigned)(read1(in)) << 8;
        skip(in, n);
    }
    যদি (flags & 8) যেতিয়ালৈ (read1(in) != 0) ;
    যদি (flags & 16) যেতিয়ালৈ (read1(in) != 0) ;
    যদি (flags & 2) skip(in, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local পূর্ণাঙ্ক gzscan(ৱর্ণ *name, z_stream *strm, পূর্ণাঙ্ক level)
{
    পূর্ণাঙ্ক ret, lastbit, বাম, full;
    unsigned have;
    unsigned long crc, tot;
    unsigned ৱর্ণ *window;
    off_t lastoff, end;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    যদি (gz.fd == -1) bye("cannot open ", name);
    gz.buf = malloc(CHUNK);
    যদি (gz.buf == রিক্ত) bye("out of memory", "");
    gz.size = LGCHUNK;
    gz.বাম = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    window = malloc(DSIZE);
    যদি (window == রিক্ত) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    যদি (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.বাম;
    বাম = 0;
    strm->avail_in = gz.বাম;
    strm->next_in = gz.next;
    crc = crc32(0L, Z_NULL, 0);
    have = full = 0;
    কৰা {
        /* if needed, get more input */
        যদি (strm->avail_in == 0) {
            readmore(&gz);
            strm->avail_in = gz.বাম;
            strm->next_in = gz.next;
        }

        /* set up output to next available section of sliding window */
        strm->avail_out = DSIZE - have;
        strm->next_out = window + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_BLOCK);
        যদি (ret == Z_STREAM_ERROR) bye("internal stream error!", "");
        যদি (ret == Z_MEM_ERROR) bye("out of memory", "");
        যদি (ret == Z_DATA_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        crc = crc32(crc, window + have, DSIZE - have - strm->avail_out);
        যদি (strm->avail_out)
            have = DSIZE - strm->avail_out;
        অন্যথা {
            have = 0;
            full = 1;
        }

        /* process end of block */
        যদি (strm->data_type & 128) {
            যদি (strm->data_type & 64)
                বাম = strm->data_type & 0x1f;
            অন্যথা {
                lastbit = strm->data_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_in;
            }
        }
    } যেতিয়ালৈ (ret != Z_STREAM_END);
    inflateEnd(strm);
    gz.বাম = strm->avail_in;
    gz.next = strm->next_in;

    /* save the location of the end of the compressed data */
    end = lseek(gz.fd, 0L, SEEK_CUR) - gz.বাম;

    /* check gzip trailer and save total for deflate */
    যদি (crc != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->total_out;
    যদি ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    যদি (gz.বাম || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    যদি (পঠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (unsigned ৱর্ণ)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    যদি (লিখ(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    যদি (full) {
        rotate(window, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    যদি (ret != Z_OK) bye("out of memory", "");
    deflateSetDictionary(strm, window, have);
    strm->adler = crc;
    strm->total_in = tot;
    যদি (বাম) {
        lseek(gz.fd, --end, SEEK_SET);
        যদি (পঠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_মৌলিক(strm, 8 - বাম, *gz.buf);
    }
    lseek(gz.fd, end, SEEK_SET);

    /* clean up and return */
    মুক্ত(window);
    মুক্ত(gz.buf);
    ঘূৰাই gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local শূন্য gztack(ৱর্ণ *name, পূর্ণাঙ্ক gd, z_stream *strm, পূর্ণাঙ্ক last)
{
    পূর্ণাঙ্ক fd, len, ret;
    unsigned বাম;
    unsigned ৱর্ণ *in, *out;

    /* open file to compress and append */
    fd = 0;
    যদি (name != রিক্ত) {
        fd = open(name, O_RDONLY, 0);
        যদি (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    in = malloc(CHUNK);
    out = malloc(CHUNK);
    যদি (in == রিক্ত || out == রিক্ত) bye("out of memory", "");

    /* compress input file and append to gzip file */
    কৰা {
        /* get more input */
        len = পঠ(fd, in, CHUNK);
        যদি (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_in = (unsigned)len;
        strm->next_in = in;
        যদি (len) strm->adler = crc32(strm->adler, in, (unsigned)len);

        /* compress and write all available output */
        কৰা {
            strm->avail_out = CHUNK;
            strm->next_out = out;
            ret = deflate(strm, last && len == 0 ? Z_FINISH : Z_NO_FLUSH);
            বাম = CHUNK - strm->avail_out;
            যেতিয়ালৈ (বাম) {
                len = লিখ(gd, out + CHUNK - strm->avail_out - বাম, বাম);
                যদি (len == -1) bye("writing gzip file", "");
                বাম -= (unsigned)len;
            }
        } যেতিয়ালৈ (strm->avail_out == 0 && ret != Z_STREAM_END);
    } যেতিয়ালৈ (len != 0);

    /* write trailer after last entry */
    যদি (last) {
        deflateEnd(strm);
        out[0] = (unsigned ৱর্ণ)(strm->adler);
        out[1] = (unsigned ৱর্ণ)(strm->adler >> 8);
        out[2] = (unsigned ৱর্ণ)(strm->adler >> 16);
        out[3] = (unsigned ৱর্ণ)(strm->adler >> 24);
        out[4] = (unsigned ৱর্ণ)(strm->total_in);
        out[5] = (unsigned ৱর্ণ)(strm->total_in >> 8);
        out[6] = (unsigned ৱর্ণ)(strm->total_in >> 16);
        out[7] = (unsigned ৱর্ণ)(strm->total_in >> 24);
        len = 8;
        কৰা {
            ret = লিখ(gd, out + 8 - len, len);
            যদি (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } যেতিয়ালৈ (len);
        close(gd);
    }

    /* clean up and return */
    মুক্ত(out);
    মুক্ত(in);
    যদি (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
    পূর্ণাঙ্ক gd, level;
    z_stream strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    যদি (*argv == রিক্ত) {
        printf(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        printf(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        ঘূৰাই 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    যদি (argv[0][0] == '-') {
        যদি (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        যদি (*++argv == রিক্ত) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    যদি (*argv == রিক্ত)
        gztack(রিক্ত, gd, &strm, 1);
    অন্যথা
        কৰা {
            gztack(*argv, gd, &strm, argv[1] == রিক্ত);
        } যেতিয়ালৈ (*++argv != রিক্ত);
    ঘূৰাই 0;
}
