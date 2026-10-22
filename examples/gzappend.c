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

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ "zlib.h"

#define local ସ୍ଥିର
#define LGCHUNK 14
#define CHUNK (1U << LGCHUNK)
#define DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local ଶୂନ୍ୟ bye(ଅକ୍ଷର *msg1, ଅକ୍ଷର *msg2)
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

    ଯେତେବେଳେ (a && b)
        ଯଦି (a > b) {
            c = b;
            ଯେତେବେଳେ (a - c >= c)
                c <<= 1;
            a -= c;
        }
        ଅନ୍ୟଥା {
            c = a;
            ଯେତେବେଳେ (b - c >= c)
                c <<= 1;
            b -= c;
        }
    ଫେରନ୍ତୁ a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local ଶୂନ୍ୟ rotate(unsigned ଅକ୍ଷର *list, unsigned len, unsigned rot)
{
    unsigned ଅକ୍ଷର tmp;
    unsigned cycles;
    unsigned ଅକ୍ଷର *start, *last, *to, *from;

    /* normalize rot and handle degenerate cases */
    ଯଦି (len < 2) ଫେରନ୍ତୁ;
    ଯଦି (rot >= len) rot %= len;
    ଯଦି (rot == 0) ଫେରନ୍ତୁ;

    /* pointer to last entry in list */
    last = list + (len - 1);

    /* do simple left shift by one */
    ଯଦି (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *last = tmp;
        ଫେରନ୍ତୁ;
    }

    /* do simple right shift by one */
    ଯଦି (rot == len - 1) {
        tmp = *last;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        ଫେରନ୍ତୁ;
    }

    /* otherwise do rotate as a set of cycles in place */
    cycles = gcd(len, rot);             /* number of cycles */
    କର {
        start = from = list + cycles;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        ପାଇଁ (;;) {
            to = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            ଯଦି (from > last) from -= len;   /* (pointer better not wrap) */
            ଯଦି (from == start) ଭାଙ୍ଗ;   /* all but one shifted */
            *to = *from;                /* shift left */
        }
        *to = tmp;                      /* complete the circle */
    } ଯେତେବେଳେ (--cycles);
}

/* structure for gzip file read operations */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;                     /* file descriptor */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;                   /* 1 << size is bytes in buf */
    unsigned ବାମ;              /* bytes available at next */
    unsigned ଅକ୍ଷର *buf;         /* buffer */
    z_const unsigned ଅକ୍ଷର *next;    /* next byte in buffer */
    ଅକ୍ଷର *name;                 /* file name for error messages */
} file;

/* reload buffer */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା readin(file *in)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

    len = ପଠ(in->fd, in->buf, 1 << in->size);
    ଯଦି (len == -1) bye("error reading ", in->name);
    in->ବାମ = (unsigned)len;
    in->next = in->buf;
    ଫେରନ୍ତୁ len;
}

/* read from file in, exit if end-of-file */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା readmore(file *in)
{
    ଯଦି (readin(in) == 0) bye("unexpected end of ", in->name);
    ଫେରନ୍ତୁ 0;
}

#define read1(in) (in->ବାମ == 0 ? readmore(in) : 0, \
                   in->ବାମ--, *(in->next)++)

/* skip over n bytes of in */
local ଶୂନ୍ୟ skip(file *in, unsigned n)
{
    unsigned bypass;

    ଯଦି (n > in->ବାମ) {
        n -= in->ବାମ;
        bypass = n & ~((1U << in->size) - 1);
        ଯଦି (bypass) {
            ଯଦି (lseek(in->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", in->name);
            n -= bypass;
        }
        readmore(in);
        ଯଦି (n > in->ବାମ)
            bye("unexpected end of ", in->name);
    }
    in->ବାମ -= n;
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
    ଫେରନ୍ତୁ val;
}

/* skip over gzip header */
local ଶୂନ୍ୟ gzheader(file *in)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
    unsigned n;

    ଯଦି (read1(in) != 31 || read1(in) != 139) bye(in->name, " not a gzip file");
    ଯଦି (read1(in) != 8) bye("unknown compression method in", in->name);
    flags = read1(in);
    ଯଦି (flags & 0xe0) bye("unknown header flags set in", in->name);
    skip(in, 6);
    ଯଦି (flags & 4) {
        n = read1(in);
        n += (unsigned)(read1(in)) << 8;
        skip(in, n);
    }
    ଯଦି (flags & 8) ଯେତେବେଳେ (read1(in) != 0) ;
    ଯଦି (flags & 16) ଯେତେବେଳେ (read1(in) != 0) ;
    ଯଦି (flags & 2) skip(in, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzscan(ଅକ୍ଷର *name, z_stream *strm, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, lastbit, ବାମ, full;
    unsigned have;
    unsigned long crc, tot;
    unsigned ଅକ୍ଷର *window;
    off_t lastoff, end;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    ଯଦି (gz.fd == -1) bye("cannot open ", name);
    gz.buf = malloc(CHUNK);
    ଯଦି (gz.buf == ଖାଲି) bye("out of memory", "");
    gz.size = LGCHUNK;
    gz.ବାମ = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    window = malloc(DSIZE);
    ଯଦି (window == ଖାଲି) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    ଯଦି (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.ବାମ;
    ବାମ = 0;
    strm->avail_in = gz.ବାମ;
    strm->next_in = gz.next;
    crc = crc32(0L, Z_NULL, 0);
    have = full = 0;
    କର {
        /* if needed, get more input */
        ଯଦି (strm->avail_in == 0) {
            readmore(&gz);
            strm->avail_in = gz.ବାମ;
            strm->next_in = gz.next;
        }

        /* set up output to next available section of sliding window */
        strm->avail_out = DSIZE - have;
        strm->next_out = window + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_BLOCK);
        ଯଦି (ret == Z_STREAM_ERROR) bye("internal stream error!", "");
        ଯଦି (ret == Z_MEM_ERROR) bye("out of memory", "");
        ଯଦି (ret == Z_DATA_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        crc = crc32(crc, window + have, DSIZE - have - strm->avail_out);
        ଯଦି (strm->avail_out)
            have = DSIZE - strm->avail_out;
        ଅନ୍ୟଥା {
            have = 0;
            full = 1;
        }

        /* process end of block */
        ଯଦି (strm->data_type & 128) {
            ଯଦି (strm->data_type & 64)
                ବାମ = strm->data_type & 0x1f;
            ଅନ୍ୟଥା {
                lastbit = strm->data_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_in;
            }
        }
    } ଯେତେବେଳେ (ret != Z_STREAM_END);
    inflateEnd(strm);
    gz.ବାମ = strm->avail_in;
    gz.next = strm->next_in;

    /* save the location of the end of the compressed data */
    end = lseek(gz.fd, 0L, SEEK_CUR) - gz.ବାମ;

    /* check gzip trailer and save total for deflate */
    ଯଦି (crc != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->total_out;
    ଯଦି ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    ଯଦି (gz.ବାମ || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    ଯଦି (ପଠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (unsigned ଅକ୍ଷର)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    ଯଦି (ଲିଖ(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    ଯଦି (full) {
        rotate(window, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    ଯଦି (ret != Z_OK) bye("out of memory", "");
    deflateSetDictionary(strm, window, have);
    strm->adler = crc;
    strm->total_in = tot;
    ଯଦି (ବାମ) {
        lseek(gz.fd, --end, SEEK_SET);
        ଯଦି (ପଠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_ମୌଳିକ(strm, 8 - ବାମ, *gz.buf);
    }
    lseek(gz.fd, end, SEEK_SET);

    /* clean up and return */
    ମୁକ୍ତ(window);
    ମୁକ୍ତ(gz.buf);
    ଫେରନ୍ତୁ gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local ଶୂନ୍ୟ gztack(ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା gd, z_stream *strm, ପୂର୍ଣ୍ଣସଂଖ୍ୟା last)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, len, ret;
    unsigned ବାମ;
    unsigned ଅକ୍ଷର *in, *out;

    /* open file to compress and append */
    fd = 0;
    ଯଦି (name != ଖାଲି) {
        fd = open(name, O_RDONLY, 0);
        ଯଦି (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    in = malloc(CHUNK);
    out = malloc(CHUNK);
    ଯଦି (in == ଖାଲି || out == ଖାଲି) bye("out of memory", "");

    /* compress input file and append to gzip file */
    କର {
        /* get more input */
        len = ପଠ(fd, in, CHUNK);
        ଯଦି (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_in = (unsigned)len;
        strm->next_in = in;
        ଯଦି (len) strm->adler = crc32(strm->adler, in, (unsigned)len);

        /* compress and write all available output */
        କର {
            strm->avail_out = CHUNK;
            strm->next_out = out;
            ret = deflate(strm, last && len == 0 ? Z_FINISH : Z_NO_FLUSH);
            ବାମ = CHUNK - strm->avail_out;
            ଯେତେବେଳେ (ବାମ) {
                len = ଲିଖ(gd, out + CHUNK - strm->avail_out - ବାମ, ବାମ);
                ଯଦି (len == -1) bye("writing gzip file", "");
                ବାମ -= (unsigned)len;
            }
        } ଯେତେବେଳେ (strm->avail_out == 0 && ret != Z_STREAM_END);
    } ଯେତେବେଳେ (len != 0);

    /* write trailer after last entry */
    ଯଦି (last) {
        deflateEnd(strm);
        out[0] = (unsigned ଅକ୍ଷର)(strm->adler);
        out[1] = (unsigned ଅକ୍ଷର)(strm->adler >> 8);
        out[2] = (unsigned ଅକ୍ଷର)(strm->adler >> 16);
        out[3] = (unsigned ଅକ୍ଷର)(strm->adler >> 24);
        out[4] = (unsigned ଅକ୍ଷର)(strm->total_in);
        out[5] = (unsigned ଅକ୍ଷର)(strm->total_in >> 8);
        out[6] = (unsigned ଅକ୍ଷର)(strm->total_in >> 16);
        out[7] = (unsigned ଅକ୍ଷର)(strm->total_in >> 24);
        len = 8;
        କର {
            ret = ଲିଖ(gd, out + 8 - len, len);
            ଯଦି (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } ଯେତେବେଳେ (len);
        close(gd);
    }

    /* clean up and return */
    ମୁକ୍ତ(out);
    ମୁକ୍ତ(in);
    ଯଦି (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା gd, level;
    z_stream strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    ଯଦି (*argv == ଖାଲି) {
        printf(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        printf(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        ଫେରନ୍ତୁ 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    ଯଦି (argv[0][0] == '-') {
        ଯଦି (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        ଯଦି (*++argv == ଖାଲି) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    ଯଦି (*argv == ଖାଲି)
        gztack(ଖାଲି, gd, &strm, 1);
    ଅନ୍ୟଥା
        କର {
            gztack(*argv, gd, &strm, argv[1] == ଖାଲି);
        } ଯେତେବେଳେ (*++argv != ଖାଲି);
    ଫେରନ୍ତୁ 0;
}
