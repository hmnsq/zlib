/* gzjoin -- command to join gzip files into one gzip file

  Copyright (C) 2004, 2005, 2012 Mark Adler, all rights reserved
  version 1.2, 14 Aug 2012

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
 * 1.0  11 Dec 2004     - First version
 * 1.1  12 Jun 2005     - Changed ssize_t to long for portability
 * 1.2  14 Aug 2012     - Clean up for z_const usage
 */

/*
   gzjoin takes one or more gzip files on the command line and writes out a
   single gzip file that will uncompress to the concatenation of the
   uncompressed data from the individual gzip files.  gzjoin does this without
   having to recompress any of the data and without having to calculate a new
   crc32 for the concatenated uncompressed data.  gzjoin does however have to
   decompress all of the input data in order to find the bits in the compressed
   data that need to be modified to concatenate the streams.

   gzjoin does not do an integrity check on the input gzip files other than
   checking the gzip header and decompressing the compressed data.  They are
   otherwise assumed to be complete and correct.

   Each joint between gzip files removes at least 18 bytes of previous trailer
   and subsequent header, and inserts an average of about three bytes to the
   compressed data in order to connect the streams.  The output gzip file
   has a minimal ten-byte gzip header with no file name or modification time.

   This program was written to illustrate the use of the Z_BLOCK option of
   inflate() and the crc32_combine() function.  gzjoin will not compile with
   versions of zlib earlier than 1.2.3.
 */

#ଅନ୍ତର୍ଭାଵ <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#ଅନ୍ତର୍ଭାଵ <stdlib.h>     /* exit(), malloc(), free() */
#ଅନ୍ତର୍ଭାଵ <fcntl.h>      /* open() */
#ଅନ୍ତର୍ଭାଵ <unistd.h>     /* close(), read(), lseek() */
#ଅନ୍ତର୍ଭାଵ "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#define local ସ୍ଥିର

/* exit with an error (return a value to allow use in an expression) */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା bail(ଅକ୍ଷର *why1, ଅକ୍ଷର *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    exit(1);
    ଫେରନ୍ତୁ 0;
}

/* -- simple buffered file input with access to the buffer -- */

#define CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    ଅକ୍ଷର *name;             /* name of file for error messages */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;                 /* file descriptor */
    unsigned ବାମ;          /* bytes remaining at next */
    unsigned ଅକ୍ଷର *next;    /* next byte to read */
    unsigned ଅକ୍ଷର *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local ଶୂନ୍ୟ bclose(bin *in)
{
    ଯଦି (in != ଖାଲି) {
        ଯଦି (in->fd != -1)
            close(in->fd);
        ଯଦି (in->buf != ଖାଲି)
            ମୁକ୍ତ(in->buf);
        ମୁକ୍ତ(in);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(ଅକ୍ଷର *name)
{
    bin *in;

    in = malloc(ଆକାର(bin));
    ଯଦି (in == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;
    in->buf = malloc(CHUNK);
    in->fd = open(name, O_RDONLY, 0);
    ଯଦି (in->buf == ଖାଲି || in->fd == -1) {
        bclose(in);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    in->ବାମ = 0;
    in->next = in->buf;
    in->name = name;
    ଫେରନ୍ତୁ in;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା bload(bin *in)
{
    long len;

    ଯଦି (in == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    ଯଦି (in->ବାମ != 0)
        ଫେରନ୍ତୁ 0;
    in->next = in->buf;
    କର {
        len = (long)ପଠ(in->fd, in->buf + in->ବାମ, CHUNK - in->ବାମ);
        ଯଦି (len < 0)
            ଫେରନ୍ତୁ -1;
        in->ବାମ += (unsigned)len;
    } ଯେତେବେଳେ (len != 0 && in->ବାମ < CHUNK);
    ଫେରନ୍ତୁ len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#define bget(in) (in->ବାମ ? 0 : bload(in), \
                  in->ବାମ ? (in->ବାମ--, *(in->next)++) : \
                    bail("unexpected end of file on ", in->name))

/* get a four-byte little-endian unsigned integer from file */
local unsigned long bget4(bin *in)
{
    unsigned long val;

    val = bget(in);
    val += (unsigned long)(bget(in)) << 8;
    val += (unsigned long)(bget(in)) << 16;
    val += (unsigned long)(bget(in)) << 24;
    ଫେରନ୍ତୁ val;
}

/* skip bytes in file */
local ଶୂନ୍ୟ bskip(bin *in, unsigned skip)
{
    /* check pointer */
    ଯଦି (in == ଖାଲି)
        ଫେରନ୍ତୁ;

    /* easy case -- skip bytes in buffer */
    ଯଦି (skip <= in->ବାମ) {
        in->ବାମ -= skip;
        in->next += skip;
        ଫେରନ୍ତୁ;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= in->ବାମ;
    in->ବାମ = 0;

    /* seek past multiples of CHUNK bytes */
    ଯଦି (skip > CHUNK) {
        unsigned ବାମ;

        ବାମ = skip & (CHUNK - 1);
        ଯଦି (ବାମ == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(in->fd, skip - 1, SEEK_CUR);
            ଯଦି (ପଠ(in->fd, in->buf, 1) != 1)
                bail("unexpected end of file on ", in->name);
            ଫେରନ୍ତୁ;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(in->fd, skip - ବାମ, SEEK_CUR);
        skip = ବାମ;
    }

    /* read more input and skip remainder */
    bload(in);
    ଯଦି (skip > in->ବାମ)
        bail("unexpected end of file on ", in->name);
    in->ବାମ -= skip;
    in->next += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local ଶୂନ୍ୟ gzhead(bin *in)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;

    /* verify gzip magic header and compression method */
    ଯଦି (bget(in) != 0x1f || bget(in) != 0x8b || bget(in) != 8)
        bail(in->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(in);
    ଯଦି ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", in->name);

    /* skip modification time, extra flags, and os */
    bskip(in, 6);

    /* skip extra field if present */
    ଯଦି (flags & 4) {
        unsigned len;

        len = bget(in);
        len += (unsigned)(bget(in)) << 8;
        bskip(in, len);
    }

    /* skip file name if present */
    ଯଦି (flags & 8)
        ଯେତେବେଳେ (bget(in) != 0)
            ;

    /* skip comment if present */
    ଯଦି (flags & 16)
        ଯେତେବେଳେ (bget(in) != 0)
            ;

    /* skip header crc if present */
    ଯଦି (flags & 2)
        bskip(in, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local ଶୂନ୍ୟ put4(unsigned long val, FILE *out)
{
    putc(val & 0xff, out);
    putc((val >> 8) & 0xff, out);
    putc((val >> 16) & 0xff, out);
    putc((val >> 24) & 0xff, out);
}

/* Load up zlib stream from buffered input, bail if end of file */
local ଶୂନ୍ୟ zpull(z_streamp strm, bin *in)
{
    ଯଦି (in->ବାମ == 0)
        bload(in);
    ଯଦି (in->ବାମ == 0)
        bail("unexpected end of file on ", in->name);
    strm->avail_in = in->ବାମ;
    strm->next_in = in->next;
}

/* Write header for gzip file to out and initialize trailer. */
local ଶୂନ୍ୟ gzinit(unsigned long *crc, unsigned long *tot, FILE *out)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, out);
    *crc = crc32(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local ଶୂନ୍ୟ gzcopy(ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା clr, unsigned long *crc, unsigned long *tot,
                  FILE *out)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;                /* return value from zlib functions */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos;                /* where the "last block" bit is in byte */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା last;               /* true if processing the last block */
    bin *in;                /* buffered input file */
    unsigned ଅକ୍ଷର *start;   /* start of compressed data in buffer */
    unsigned ଅକ୍ଷର *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_stream strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    in = bopen(name);
    ଯଦି (in == ଖାଲି)
        bail("could not open ", name);
    gzhead(in);

    /* allocate buffer for uncompressed data and initialize raw inflate
       stream */
    junk = malloc(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -15);
    ଯଦି (junk == ଖାଲି || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, in);
    start = in->next;
    last = start[0] & 1;
    ଯଦି (last && clr)
        start[0] &= ~1;
    strm.avail_out = 0;
    ପାଇଁ (;;) {
        /* if input used and output done, write used input and get more */
        ଯଦି (strm.avail_in == 0 && strm.avail_out != 0) {
            fwrite(start, 1, strm.next_in - start, out);
            start = in->buf;
            in->ବାମ = 0;
            zpull(&strm, in);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_out = CHUNK;
        strm.next_out = junk;
        ret = inflate(&strm, Z_BLOCK);
        ବାଛ (ret) {
        ସ୍ଥିତି Z_MEM_ERROR:
            bail("out of memory", "");
        ସ୍ଥିତି Z_DATA_ERROR:
            bail("invalid compressed data in ", in->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_out;

        /* check for block boundary (only get this when block copied out) */
        ଯଦି (strm.data_type & 128) {
            /* if that was the last block, then done */
            ଯଦି (last)
                ଭାଙ୍ଗ;

            /* number of unused bits in last byte */
            pos = strm.data_type & 7;

            /* find the next last-block bit */
            ଯଦି (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                last = strm.next_in[-1] & pos;
                ଯଦି (last && clr)
                    in->buf[strm.next_in - in->buf - 1] &= ~pos;
            }
            ଅନ୍ୟଥା {
                /* next last-block bit is in next unused byte */
                ଯଦି (strm.avail_in == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(start, 1, strm.next_in - start, out);
                    start = in->buf;
                    in->ବାମ = 0;
                    zpull(&strm, in);
                }
                last = strm.next_in[0] & 1;
                ଯଦି (last && clr)
                    in->buf[strm.next_in - in->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    in->ବାମ = strm.avail_in;
    in->next = in->buf + (strm.next_in - in->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.data_type & 7;
    fwrite(start, 1, in->next - start - 1, out);
    last = in->next[-1];
    ଯଦି (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(last, out);
    ଅନ୍ୟଥା {
        /* append empty blocks to last byte */
        last &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        ଯଦି (pos & 1) {
            /* odd -- append an empty stored block */
            putc(last, out);
            ଯଦି (pos == 1)
                putc(0, out);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, out);
        }
        ଅନ୍ୟଥା {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            ବାଛ (pos) {
            ସ୍ଥିତି 6:
                putc(last | 8, out);
                last = 0;
            ସ୍ଥିତି 4:
                putc(last | 0x20, out);
                last = 0;
            ସ୍ଥିତି 2:
                putc(last | 0x80, out);
                putc(0, out);
            }
        }
    }

    /* update crc and tot */
    *crc = crc32_combine(*crc, bget4(in), len);
    *tot += (unsigned long)len;

    /* clean up */
    inflateEnd(&strm);
    ମୁକ୍ତ(junk);
    bclose(in);

    /* write trailer if this is the last gzip file */
    ଯଦି (!clr) {
        put4(*crc, out);
        put4(*tot, out);
    }
}

/* join the gzip files on the command line, write result to stdout */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
    unsigned long crc, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    ଯଦି (argc == 0) {
        fputs("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              stderr);
        ଫେରନ୍ତୁ 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&crc, &tot, stdout);
    ଯେତେବେଳେ (argc--)
        gzcopy(*argv++, argc, &crc, &tot, stdout);

    /* done */
    ଫେରନ୍ତୁ 0;
}
