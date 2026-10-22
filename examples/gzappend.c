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

#define local ساكن
#define LGCHUNK 14
#define CHUNK (1U << LGCHUNK)
#define DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local فراغ bye(حرف *msg1, حرف *msg2)
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

    כלעוד (a && b)
        אם (a > b) {
            c = b;
            כלעוד (a - c >= c)
                c <<= 1;
            a -= c;
        }
        אחרת {
            c = a;
            כלעוד (b - c >= c)
                c <<= 1;
            b -= c;
        }
    החזר a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local فراغ rotate(unsigned حرف *list, unsigned len, unsigned rot)
{
    unsigned حرف tmp;
    unsigned cycles;
    unsigned حرف *start, *last, *to, *from;

    /* normalize rot and handle degenerate cases */
    אם (len < 2) החזר;
    אם (rot >= len) rot %= len;
    אם (rot == 0) החזר;

    /* pointer to last entry in list */
    last = list + (len - 1);

    /* do simple left shift by one */
    אם (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *last = tmp;
        החזר;
    }

    /* do simple right shift by one */
    אם (rot == len - 1) {
        tmp = *last;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        החזר;
    }

    /* otherwise do rotate as a set of cycles in place */
    cycles = gcd(len, rot);             /* number of cycles */
    עשה {
        start = from = list + cycles;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        עבור (;;) {
            to = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            אם (from > last) from -= len;   /* (pointer better not wrap) */
            אם (from == start) اكسر;   /* all but one shifted */
            *to = *from;                /* shift left */
        }
        *to = tmp;                      /* complete the circle */
    } כלעוד (--cycles);
}

/* structure for gzip file read operations */
تعريفنوع بنية {
    صحيح fd;                     /* file descriptor */
    صحيح size;                   /* 1 << size is bytes in buf */
    unsigned left;              /* bytes available at next */
    unsigned حرف *buf;         /* buffer */
    z_const unsigned حرف *next;    /* next byte in buffer */
    حرف *name;                 /* file name for error messages */
} file;

/* reload buffer */
local صحيح readin(file *in)
{
    صحيح len;

    len = اقرأ(in->fd, in->buf, 1 << in->size);
    אם (len == -1) bye("error reading ", in->name);
    in->left = (unsigned)len;
    in->next = in->buf;
    החזר len;
}

/* read from file in, exit if end-of-file */
local صحيح readmore(file *in)
{
    אם (readin(in) == 0) bye("unexpected end of ", in->name);
    החזר 0;
}

#define read1(in) (in->left == 0 ? readmore(in) : 0, \
                   in->left--, *(in->next)++)

/* skip over n bytes of in */
local فراغ skip(file *in, unsigned n)
{
    unsigned bypass;

    אם (n > in->left) {
        n -= in->left;
        bypass = n & ~((1U << in->size) - 1);
        אם (bypass) {
            אם (lseek(in->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", in->name);
            n -= bypass;
        }
        readmore(in);
        אם (n > in->left)
            bye("unexpected end of ", in->name);
    }
    in->left -= n;
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
    החזר val;
}

/* skip over gzip header */
local فراغ gzheader(file *in)
{
    صحيح flags;
    unsigned n;

    אם (read1(in) != 31 || read1(in) != 139) bye(in->name, " not a gzip file");
    אם (read1(in) != 8) bye("unknown compression method in", in->name);
    flags = read1(in);
    אם (flags & 0xe0) bye("unknown header flags set in", in->name);
    skip(in, 6);
    אם (flags & 4) {
        n = read1(in);
        n += (unsigned)(read1(in)) << 8;
        skip(in, n);
    }
    אם (flags & 8) כלעוד (read1(in) != 0) ;
    אם (flags & 16) כלעוד (read1(in) != 0) ;
    אם (flags & 2) skip(in, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local صحيح gzscan(حرف *name, z_stream *strm, صحيح level)
{
    صحيح ret, lastbit, left, full;
    unsigned have;
    unsigned long crc, tot;
    unsigned حرف *window;
    off_t lastoff, end;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    אם (gz.fd == -1) bye("cannot open ", name);
    gz.buf = malloc(CHUNK);
    אם (gz.buf == عدم) bye("out of memory", "");
    gz.size = LGCHUNK;
    gz.left = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    window = malloc(DSIZE);
    אם (window == عدم) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    אם (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.left;
    left = 0;
    strm->avail_in = gz.left;
    strm->next_in = gz.next;
    crc = crc32(0L, Z_NULL, 0);
    have = full = 0;
    עשה {
        /* if needed, get more input */
        אם (strm->avail_in == 0) {
            readmore(&gz);
            strm->avail_in = gz.left;
            strm->next_in = gz.next;
        }

        /* set up output to next available section of sliding window */
        strm->avail_out = DSIZE - have;
        strm->next_out = window + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_BLOCK);
        אם (ret == Z_STREAM_ERROR) bye("internal stream error!", "");
        אם (ret == Z_MEM_ERROR) bye("out of memory", "");
        אם (ret == Z_DATA_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        crc = crc32(crc, window + have, DSIZE - have - strm->avail_out);
        אם (strm->avail_out)
            have = DSIZE - strm->avail_out;
        אחרת {
            have = 0;
            full = 1;
        }

        /* process end of block */
        אם (strm->data_type & 128) {
            אם (strm->data_type & 64)
                left = strm->data_type & 0x1f;
            אחרת {
                lastbit = strm->data_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_in;
            }
        }
    } כלעוד (ret != Z_STREAM_END);
    inflateEnd(strm);
    gz.left = strm->avail_in;
    gz.next = strm->next_in;

    /* save the location of the end of the compressed data */
    end = lseek(gz.fd, 0L, SEEK_CUR) - gz.left;

    /* check gzip trailer and save total for deflate */
    אם (crc != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->total_out;
    אם ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    אם (gz.left || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    אם (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (unsigned حرف)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    אם (اكتب(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    אם (full) {
        rotate(window, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    אם (ret != Z_OK) bye("out of memory", "");
    deflateSetDictionary(strm, window, have);
    strm->adler = crc;
    strm->total_in = tot;
    אם (left) {
        lseek(gz.fd, --end, SEEK_SET);
        אם (اقرأ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflatePrime(strm, 8 - left, *gz.buf);
    }
    lseek(gz.fd, end, SEEK_SET);

    /* clean up and return */
    حرر(window);
    حرر(gz.buf);
    החזר gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local فراغ gztack(حرف *name, صحيح gd, z_stream *strm, صحيح last)
{
    صحيح fd, len, ret;
    unsigned left;
    unsigned حرف *in, *out;

    /* open file to compress and append */
    fd = 0;
    אם (name != عدم) {
        fd = open(name, O_RDONLY, 0);
        אם (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    in = malloc(CHUNK);
    out = malloc(CHUNK);
    אם (in == عدم || out == عدم) bye("out of memory", "");

    /* compress input file and append to gzip file */
    עשה {
        /* get more input */
        len = اقرأ(fd, in, CHUNK);
        אם (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_in = (unsigned)len;
        strm->next_in = in;
        אם (len) strm->adler = crc32(strm->adler, in, (unsigned)len);

        /* compress and write all available output */
        עשה {
            strm->avail_out = CHUNK;
            strm->next_out = out;
            ret = deflate(strm, last && len == 0 ? Z_FINISH : Z_NO_FLUSH);
            left = CHUNK - strm->avail_out;
            כלעוד (left) {
                len = اكتب(gd, out + CHUNK - strm->avail_out - left, left);
                אם (len == -1) bye("writing gzip file", "");
                left -= (unsigned)len;
            }
        } כלעוד (strm->avail_out == 0 && ret != Z_STREAM_END);
    } כלעוד (len != 0);

    /* write trailer after last entry */
    אם (last) {
        deflateEnd(strm);
        out[0] = (unsigned حرف)(strm->adler);
        out[1] = (unsigned حرف)(strm->adler >> 8);
        out[2] = (unsigned حرف)(strm->adler >> 16);
        out[3] = (unsigned حرف)(strm->adler >> 24);
        out[4] = (unsigned حرف)(strm->total_in);
        out[5] = (unsigned حرف)(strm->total_in >> 8);
        out[6] = (unsigned حرف)(strm->total_in >> 16);
        out[7] = (unsigned حرف)(strm->total_in >> 24);
        len = 8;
        עשה {
            ret = اكتب(gd, out + 8 - len, len);
            אם (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } כלעוד (len);
        close(gd);
    }

    /* clean up and return */
    حرر(out);
    حرر(in);
    אם (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
صحيح main(صحيح argc, حرف **argv)
{
    صحيح gd, level;
    z_stream strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    אם (*argv == عدم) {
        printf(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        printf(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        החזר 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    אם (argv[0][0] == '-') {
        אם (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        אם (*++argv == عدم) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    אם (*argv == عدم)
        gztack(عدم, gd, &strm, 1);
    אחרת
        עשה {
            gztack(*argv, gd, &strm, argv[1] == عدم);
        } כלעוד (*++argv != عدم);
    החזר 0;
}
