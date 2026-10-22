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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ "zlib.h"

#define local സ്ഥിതം
#define LGCHUNK 14
#define CHUNK (1U << LGCHUNK)
#define DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local ശൂന്യം bye(അക്ഷരം *msg1, അക്ഷരം *msg2)
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

    ആകുമ്പോൾ (a && b)
        എങ്കിൽ (a > b) {
            c = b;
            ആകുമ്പോൾ (a - c >= c)
                c <<= 1;
            a -= c;
        }
        അല്ലെങ്കിൽ {
            c = a;
            ആകുമ്പോൾ (b - c >= c)
                c <<= 1;
            b -= c;
        }
    തിരികെ a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local ശൂന്യം rotate(unsigned അക്ഷരം *list, unsigned len, unsigned rot)
{
    unsigned അക്ഷരം tmp;
    unsigned cycles;
    unsigned അക്ഷരം *start, *last, *to, *from;

    /* normalize rot and handle degenerate cases */
    എങ്കിൽ (len < 2) തിരികെ;
    എങ്കിൽ (rot >= len) rot %= len;
    എങ്കിൽ (rot == 0) തിരികെ;

    /* pointer to last entry in list */
    last = list + (len - 1);

    /* do simple left shift by one */
    എങ്കിൽ (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *last = tmp;
        തിരികെ;
    }

    /* do simple right shift by one */
    എങ്കിൽ (rot == len - 1) {
        tmp = *last;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        തിരികെ;
    }

    /* otherwise do rotate as a set of cycles in place */
    cycles = gcd(len, rot);             /* number of cycles */
    ചെയ്യുക {
        start = from = list + cycles;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        വേണ്ടി (;;) {
            to = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            എങ്കിൽ (from > last) from -= len;   /* (pointer better not wrap) */
            എങ്കിൽ (from == start) നിർത്തുക;   /* all but one shifted */
            *to = *from;                /* shift left */
        }
        *to = tmp;                      /* complete the circle */
    } ആകുമ്പോൾ (--cycles);
}

/* structure for gzip file read operations */
തരനിർവചനം ഘടന {
    പൂർണ്ണസംഖ്യ fd;                     /* file descriptor */
    പൂർണ്ണസംഖ്യ size;                   /* 1 << size is bytes in buf */
    unsigned ഇടത്;              /* bytes available at next */
    unsigned അക്ഷരം *buf;         /* buffer */
    z_const unsigned അക്ഷരം *next;    /* next byte in buffer */
    അക്ഷരം *name;                 /* file name for error messages */
} file;

/* reload buffer */
local പൂർണ്ണസംഖ്യ readin(file *in)
{
    പൂർണ്ണസംഖ്യ len;

    len = പഠ(in->fd, in->buf, 1 << in->size);
    എങ്കിൽ (len == -1) bye("error reading ", in->name);
    in->ഇടത് = (unsigned)len;
    in->next = in->buf;
    തിരികെ len;
}

/* read from file in, exit if end-of-file */
local പൂർണ്ണസംഖ്യ readmore(file *in)
{
    എങ്കിൽ (readin(in) == 0) bye("unexpected end of ", in->name);
    തിരികെ 0;
}

#define read1(in) (in->ഇടത് == 0 ? readmore(in) : 0, \
                   in->ഇടത്--, *(in->next)++)

/* skip over n bytes of in */
local ശൂന്യം skip(file *in, unsigned n)
{
    unsigned bypass;

    എങ്കിൽ (n > in->ഇടത്) {
        n -= in->ഇടത്;
        bypass = n & ~((1U << in->size) - 1);
        എങ്കിൽ (bypass) {
            എങ്കിൽ (lseek(in->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", in->name);
            n -= bypass;
        }
        readmore(in);
        എങ്കിൽ (n > in->ഇടത്)
            bye("unexpected end of ", in->name);
    }
    in->ഇടത് -= n;
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
    തിരികെ val;
}

/* skip over gzip header */
local ശൂന്യം gzheader(file *in)
{
    പൂർണ്ണസംഖ്യ flags;
    unsigned n;

    എങ്കിൽ (read1(in) != 31 || read1(in) != 139) bye(in->name, " not a gzip file");
    എങ്കിൽ (read1(in) != 8) bye("unknown compression method in", in->name);
    flags = read1(in);
    എങ്കിൽ (flags & 0xe0) bye("unknown header flags set in", in->name);
    skip(in, 6);
    എങ്കിൽ (flags & 4) {
        n = read1(in);
        n += (unsigned)(read1(in)) << 8;
        skip(in, n);
    }
    എങ്കിൽ (flags & 8) ആകുമ്പോൾ (read1(in) != 0) ;
    എങ്കിൽ (flags & 16) ആകുമ്പോൾ (read1(in) != 0) ;
    എങ്കിൽ (flags & 2) skip(in, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local പൂർണ്ണസംഖ്യ gzscan(അക്ഷരം *name, z_stream *strm, പൂർണ്ണസംഖ്യ level)
{
    പൂർണ്ണസംഖ്യ ret, lastbit, ഇടത്, full;
    unsigned have;
    unsigned long crc, tot;
    unsigned അക്ഷരം *window;
    off_t lastoff, end;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    എങ്കിൽ (gz.fd == -1) bye("cannot open ", name);
    gz.buf = malloc(CHUNK);
    എങ്കിൽ (gz.buf == ശൂന്യ) bye("out of memory", "");
    gz.size = LGCHUNK;
    gz.ഇടത് = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    window = malloc(DSIZE);
    എങ്കിൽ (window == ശൂന്യ) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    എങ്കിൽ (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.ഇടത്;
    ഇടത് = 0;
    strm->avail_in = gz.ഇടത്;
    strm->next_in = gz.next;
    crc = crc32(0L, Z_NULL, 0);
    have = full = 0;
    ചെയ്യുക {
        /* if needed, get more input */
        എങ്കിൽ (strm->avail_in == 0) {
            readmore(&gz);
            strm->avail_in = gz.ഇടത്;
            strm->next_in = gz.next;
        }

        /* set up output to next available section of sliding window */
        strm->avail_out = DSIZE - have;
        strm->next_out = window + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_BLOCK);
        എങ്കിൽ (ret == Z_STREAM_ERROR) bye("internal stream error!", "");
        എങ്കിൽ (ret == Z_MEM_ERROR) bye("out of memory", "");
        എങ്കിൽ (ret == Z_DATA_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        crc = crc32(crc, window + have, DSIZE - have - strm->avail_out);
        എങ്കിൽ (strm->avail_out)
            have = DSIZE - strm->avail_out;
        അല്ലെങ്കിൽ {
            have = 0;
            full = 1;
        }

        /* process end of block */
        എങ്കിൽ (strm->data_type & 128) {
            എങ്കിൽ (strm->data_type & 64)
                ഇടത് = strm->data_type & 0x1f;
            അല്ലെങ്കിൽ {
                lastbit = strm->data_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_in;
            }
        }
    } ആകുമ്പോൾ (ret != Z_STREAM_END);
    inflateEnd(strm);
    gz.ഇടത് = strm->avail_in;
    gz.next = strm->next_in;

    /* save the location of the end of the compressed data */
    end = lseek(gz.fd, 0L, SEEK_CUR) - gz.ഇടത്;

    /* check gzip trailer and save total for deflate */
    എങ്കിൽ (crc != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->total_out;
    എങ്കിൽ ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    എങ്കിൽ (gz.ഇടത് || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    എങ്കിൽ (പഠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (unsigned അക്ഷരം)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    എങ്കിൽ (ലിഖ(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    എങ്കിൽ (full) {
        rotate(window, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    എങ്കിൽ (ret != Z_OK) bye("out of memory", "");
    deflateSetDictionary(strm, window, have);
    strm->adler = crc;
    strm->total_in = tot;
    എങ്കിൽ (ഇടത്) {
        lseek(gz.fd, --end, SEEK_SET);
        എങ്കിൽ (പഠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_അഭാജ്യം(strm, 8 - ഇടത്, *gz.buf);
    }
    lseek(gz.fd, end, SEEK_SET);

    /* clean up and return */
    സ്വതന്ത്രം(window);
    സ്വതന്ത്രം(gz.buf);
    തിരികെ gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local ശൂന്യം gztack(അക്ഷരം *name, പൂർണ്ണസംഖ്യ gd, z_stream *strm, പൂർണ്ണസംഖ്യ last)
{
    പൂർണ്ണസംഖ്യ fd, len, ret;
    unsigned ഇടത്;
    unsigned അക്ഷരം *in, *out;

    /* open file to compress and append */
    fd = 0;
    എങ്കിൽ (name != ശൂന്യ) {
        fd = open(name, O_RDONLY, 0);
        എങ്കിൽ (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    in = malloc(CHUNK);
    out = malloc(CHUNK);
    എങ്കിൽ (in == ശൂന്യ || out == ശൂന്യ) bye("out of memory", "");

    /* compress input file and append to gzip file */
    ചെയ്യുക {
        /* get more input */
        len = പഠ(fd, in, CHUNK);
        എങ്കിൽ (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_in = (unsigned)len;
        strm->next_in = in;
        എങ്കിൽ (len) strm->adler = crc32(strm->adler, in, (unsigned)len);

        /* compress and write all available output */
        ചെയ്യുക {
            strm->avail_out = CHUNK;
            strm->next_out = out;
            ret = deflate(strm, last && len == 0 ? Z_FINISH : Z_NO_FLUSH);
            ഇടത് = CHUNK - strm->avail_out;
            ആകുമ്പോൾ (ഇടത്) {
                len = ലിഖ(gd, out + CHUNK - strm->avail_out - ഇടത്, ഇടത്);
                എങ്കിൽ (len == -1) bye("writing gzip file", "");
                ഇടത് -= (unsigned)len;
            }
        } ആകുമ്പോൾ (strm->avail_out == 0 && ret != Z_STREAM_END);
    } ആകുമ്പോൾ (len != 0);

    /* write trailer after last entry */
    എങ്കിൽ (last) {
        deflateEnd(strm);
        out[0] = (unsigned അക്ഷരം)(strm->adler);
        out[1] = (unsigned അക്ഷരം)(strm->adler >> 8);
        out[2] = (unsigned അക്ഷരം)(strm->adler >> 16);
        out[3] = (unsigned അക്ഷരം)(strm->adler >> 24);
        out[4] = (unsigned അക്ഷരം)(strm->total_in);
        out[5] = (unsigned അക്ഷരം)(strm->total_in >> 8);
        out[6] = (unsigned അക്ഷരം)(strm->total_in >> 16);
        out[7] = (unsigned അക്ഷരം)(strm->total_in >> 24);
        len = 8;
        ചെയ്യുക {
            ret = ലിഖ(gd, out + 8 - len, len);
            എങ്കിൽ (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } ആകുമ്പോൾ (len);
        close(gd);
    }

    /* clean up and return */
    സ്വതന്ത്രം(out);
    സ്വതന്ത്രം(in);
    എങ്കിൽ (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
    പൂർണ്ണസംഖ്യ gd, level;
    z_stream strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    എങ്കിൽ (*argv == ശൂന്യ) {
        printf(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        printf(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        തിരികെ 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    എങ്കിൽ (argv[0][0] == '-') {
        എങ്കിൽ (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        എങ്കിൽ (*++argv == ശൂന്യ) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    എങ്കിൽ (*argv == ശൂന്യ)
        gztack(ശൂന്യ, gd, &strm, 1);
    അല്ലെങ്കിൽ
        ചെയ്യുക {
            gztack(*argv, gd, &strm, argv[1] == ശൂന്യ);
        } ആകുമ്പോൾ (*++argv != ശൂന്യ);
    തിരികെ 0;
}
