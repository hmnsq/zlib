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

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ "zlib.h"

#define local સ્થિર
#define LGCHUNK 14
#define CHUNK (1U << LGCHUNK)
#define DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local શૂન્ય bye(અક્ષર *msg1, અક્ષર *msg2)
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

    જ્યારે (a && b)
        જો (a > b) {
            c = b;
            જ્યારે (a - c >= c)
                c <<= 1;
            a -= c;
        }
        નહીંતર {
            c = a;
            જ્યારે (b - c >= c)
                c <<= 1;
            b -= c;
        }
    પાછા a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local શૂન્ય rotate(unsigned અક્ષર *list, unsigned len, unsigned rot)
{
    unsigned અક્ષર tmp;
    unsigned cycles;
    unsigned અક્ષર *start, *last, *to, *from;

    /* normalize rot and handle degenerate cases */
    જો (len < 2) પાછા;
    જો (rot >= len) rot %= len;
    જો (rot == 0) પાછા;

    /* pointer to last entry in list */
    last = list + (len - 1);

    /* do simple left shift by one */
    જો (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *last = tmp;
        પાછા;
    }

    /* do simple right shift by one */
    જો (rot == len - 1) {
        tmp = *last;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        પાછા;
    }

    /* otherwise do rotate as a set of cycles in place */
    cycles = gcd(len, rot);             /* number of cycles */
    કરો {
        start = from = list + cycles;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        માટે (;;) {
            to = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            જો (from > last) from -= len;   /* (pointer better not wrap) */
            જો (from == start) તોડો;   /* all but one shifted */
            *to = *from;                /* shift left */
        }
        *to = tmp;                      /* complete the circle */
    } જ્યારે (--cycles);
}

/* structure for gzip file read operations */
પ્રકારવ્યાખ્યા સંરચના {
    પૂર્ણાંક fd;                     /* file descriptor */
    પૂર્ણાંક size;                   /* 1 << size is bytes in buf */
    unsigned ડાબું;              /* bytes available at next */
    unsigned અક્ષર *buf;         /* buffer */
    z_const unsigned અક્ષર *next;    /* next byte in buffer */
    અક્ષર *name;                 /* file name for error messages */
} file;

/* reload buffer */
local પૂર્ણાંક readin(file *in)
{
    પૂર્ણાંક len;

    len = પઠ(in->fd, in->buf, 1 << in->size);
    જો (len == -1) bye("error reading ", in->name);
    in->ડાબું = (unsigned)len;
    in->next = in->buf;
    પાછા len;
}

/* read from file in, exit if end-of-file */
local પૂર્ણાંક readmore(file *in)
{
    જો (readin(in) == 0) bye("unexpected end of ", in->name);
    પાછા 0;
}

#define read1(in) (in->ડાબું == 0 ? readmore(in) : 0, \
                   in->ડાબું--, *(in->next)++)

/* skip over n bytes of in */
local શૂન્ય skip(file *in, unsigned n)
{
    unsigned bypass;

    જો (n > in->ડાબું) {
        n -= in->ડાબું;
        bypass = n & ~((1U << in->size) - 1);
        જો (bypass) {
            જો (lseek(in->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", in->name);
            n -= bypass;
        }
        readmore(in);
        જો (n > in->ડાબું)
            bye("unexpected end of ", in->name);
    }
    in->ડાબું -= n;
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
    પાછા val;
}

/* skip over gzip header */
local શૂન્ય gzheader(file *in)
{
    પૂર્ણાંક flags;
    unsigned n;

    જો (read1(in) != 31 || read1(in) != 139) bye(in->name, " not a gzip file");
    જો (read1(in) != 8) bye("unknown compression method in", in->name);
    flags = read1(in);
    જો (flags & 0xe0) bye("unknown header flags set in", in->name);
    skip(in, 6);
    જો (flags & 4) {
        n = read1(in);
        n += (unsigned)(read1(in)) << 8;
        skip(in, n);
    }
    જો (flags & 8) જ્યારે (read1(in) != 0) ;
    જો (flags & 16) જ્યારે (read1(in) != 0) ;
    જો (flags & 2) skip(in, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local પૂર્ણાંક gzscan(અક્ષર *name, z_stream *strm, પૂર્ણાંક level)
{
    પૂર્ણાંક ret, lastbit, ડાબું, full;
    unsigned have;
    unsigned long crc, tot;
    unsigned અક્ષર *window;
    off_t lastoff, end;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    જો (gz.fd == -1) bye("cannot open ", name);
    gz.buf = malloc(CHUNK);
    જો (gz.buf == ખાલી) bye("out of memory", "");
    gz.size = LGCHUNK;
    gz.ડાબું = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    window = malloc(DSIZE);
    જો (window == ખાલી) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    જો (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.ડાબું;
    ડાબું = 0;
    strm->avail_in = gz.ડાબું;
    strm->next_in = gz.next;
    crc = crc32(0L, Z_NULL, 0);
    have = full = 0;
    કરો {
        /* if needed, get more input */
        જો (strm->avail_in == 0) {
            readmore(&gz);
            strm->avail_in = gz.ડાબું;
            strm->next_in = gz.next;
        }

        /* set up output to next available section of sliding window */
        strm->avail_out = DSIZE - have;
        strm->next_out = window + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_BLOCK);
        જો (ret == Z_STREAM_ERROR) bye("internal stream error!", "");
        જો (ret == Z_MEM_ERROR) bye("out of memory", "");
        જો (ret == Z_DATA_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        crc = crc32(crc, window + have, DSIZE - have - strm->avail_out);
        જો (strm->avail_out)
            have = DSIZE - strm->avail_out;
        નહીંતર {
            have = 0;
            full = 1;
        }

        /* process end of block */
        જો (strm->data_type & 128) {
            જો (strm->data_type & 64)
                ડાબું = strm->data_type & 0x1f;
            નહીંતર {
                lastbit = strm->data_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_in;
            }
        }
    } જ્યારે (ret != Z_STREAM_END);
    inflateEnd(strm);
    gz.ડાબું = strm->avail_in;
    gz.next = strm->next_in;

    /* save the location of the end of the compressed data */
    end = lseek(gz.fd, 0L, SEEK_CUR) - gz.ડાબું;

    /* check gzip trailer and save total for deflate */
    જો (crc != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->total_out;
    જો ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    જો (gz.ડાબું || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    જો (પઠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (unsigned અક્ષર)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    જો (લિખ(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    જો (full) {
        rotate(window, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    જો (ret != Z_OK) bye("out of memory", "");
    deflateSetDictionary(strm, window, have);
    strm->adler = crc;
    strm->total_in = tot;
    જો (ડાબું) {
        lseek(gz.fd, --end, SEEK_SET);
        જો (પઠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_અવિભાજ્ય(strm, 8 - ડાબું, *gz.buf);
    }
    lseek(gz.fd, end, SEEK_SET);

    /* clean up and return */
    મુક્ત(window);
    મુક્ત(gz.buf);
    પાછા gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local શૂન્ય gztack(અક્ષર *name, પૂર્ણાંક gd, z_stream *strm, પૂર્ણાંક last)
{
    પૂર્ણાંક fd, len, ret;
    unsigned ડાબું;
    unsigned અક્ષર *in, *out;

    /* open file to compress and append */
    fd = 0;
    જો (name != ખાલી) {
        fd = open(name, O_RDONLY, 0);
        જો (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    in = malloc(CHUNK);
    out = malloc(CHUNK);
    જો (in == ખાલી || out == ખાલી) bye("out of memory", "");

    /* compress input file and append to gzip file */
    કરો {
        /* get more input */
        len = પઠ(fd, in, CHUNK);
        જો (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_in = (unsigned)len;
        strm->next_in = in;
        જો (len) strm->adler = crc32(strm->adler, in, (unsigned)len);

        /* compress and write all available output */
        કરો {
            strm->avail_out = CHUNK;
            strm->next_out = out;
            ret = deflate(strm, last && len == 0 ? Z_FINISH : Z_NO_FLUSH);
            ડાબું = CHUNK - strm->avail_out;
            જ્યારે (ડાબું) {
                len = લિખ(gd, out + CHUNK - strm->avail_out - ડાબું, ડાબું);
                જો (len == -1) bye("writing gzip file", "");
                ડાબું -= (unsigned)len;
            }
        } જ્યારે (strm->avail_out == 0 && ret != Z_STREAM_END);
    } જ્યારે (len != 0);

    /* write trailer after last entry */
    જો (last) {
        deflateEnd(strm);
        out[0] = (unsigned અક્ષર)(strm->adler);
        out[1] = (unsigned અક્ષર)(strm->adler >> 8);
        out[2] = (unsigned અક્ષર)(strm->adler >> 16);
        out[3] = (unsigned અક્ષર)(strm->adler >> 24);
        out[4] = (unsigned અક્ષર)(strm->total_in);
        out[5] = (unsigned અક્ષર)(strm->total_in >> 8);
        out[6] = (unsigned અક્ષર)(strm->total_in >> 16);
        out[7] = (unsigned અક્ષર)(strm->total_in >> 24);
        len = 8;
        કરો {
            ret = લિખ(gd, out + 8 - len, len);
            જો (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } જ્યારે (len);
        close(gd);
    }

    /* clean up and return */
    મુક્ત(out);
    મુક્ત(in);
    જો (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
    પૂર્ણાંક gd, level;
    z_stream strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    જો (*argv == ખાલી) {
        printf(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        printf(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        પાછા 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    જો (argv[0][0] == '-') {
        જો (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        જો (*++argv == ખાલી) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    જો (*argv == ખાલી)
        gztack(ખાલી, gd, &strm, 1);
    નહીંતર
        કરો {
            gztack(*argv, gd, &strm, argv[1] == ખાલી);
        } જ્યારે (*++argv != ખાલી);
    પાછા 0;
}
