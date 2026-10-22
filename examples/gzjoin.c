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

#అన్తర్భావ <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#అన్తర్భావ <stdlib.h>     /* exit(), malloc(), free() */
#అన్తర్భావ <fcntl.h>      /* open() */
#అన్తర్భావ <unistd.h>     /* close(), read(), lseek() */
#అన్తర్భావ "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#నిర్వచించు local స్థిర

/* exit with an error (return a value to allow use in an expression) */
local పూర్ణాంకం bail(అక్షరం *why1, అక్షరం *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    నిష్క్రమించు(1);
    తిరిగి 0;
}

/* -- simple buffered file input with access to the buffer -- */

#నిర్వచించు CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
రకనిర్వచనం నిర్మాణం {
    అక్షరం *name;             /* name of file for error messages */
    పూర్ణాంకం fd;                 /* file descriptor */
    గుర్తులేని ఎడమ;          /* bytes remaining at next */
    గుర్తులేని అక్షరం *తదుపరి;    /* next byte to read */
    గుర్తులేని అక్షరం *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local శూన్య bclose(bin *లోపల)
{
    ఒకవేళ (లోపల != ఖాళీ) {
        ఒకవేళ (లోపల->fd != -1)
            close(లోపల->fd);
        ఒకవేళ (లోపల->buf != ఖాళీ)
            విడుదల(లోపల->buf);
        విడుదల(లోపల);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(అక్షరం *name)
{
    bin *లోపల;

    లోపల = జ్ఞాపకం_ఇవ్వు(పరిమాణం(bin));
    ఒకవేళ (లోపల == ఖాళీ)
        తిరిగి ఖాళీ;
    లోపల->buf = జ్ఞాపకం_ఇవ్వు(CHUNK);
    లోపల->fd = open(name, O_RDONLY, 0);
    ఒకవేళ (లోపల->buf == ఖాళీ || లోపల->fd == -1) {
        bclose(లోపల);
        తిరిగి ఖాళీ;
    }
    లోపల->ఎడమ = 0;
    లోపల->తదుపరి = లోపల->buf;
    లోపల->name = name;
    తిరిగి లోపల;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local పూర్ణాంకం bload(bin *లోపల)
{
    దీర్ఘ len;

    ఒకవేళ (లోపల == ఖాళీ)
        తిరిగి -1;
    ఒకవేళ (లోపల->ఎడమ != 0)
        తిరిగి 0;
    లోపల->తదుపరి = లోపల->buf;
    చేయి {
        len = (దీర్ఘ)పఠ(లోపల->fd, లోపల->buf + లోపల->ఎడమ, CHUNK - లోపల->ఎడమ);
        ఒకవేళ (len < 0)
            తిరిగి -1;
        లోపల->ఎడమ += (గుర్తులేని)len;
    } అయితే (len != 0 && లోపల->ఎడమ < CHUNK);
    తిరిగి len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#నిర్వచించు bget(లోపల) (లోపల->ఎడమ ? 0 : bload(లోపల), \
                  లోపల->ఎడమ ? (లోపల->ఎడమ--, *(లోపల->తదుపరి)++) : \
                    bail("unexpected end of file on ", లోపల->name))

/* get a four-byte little-endian unsigned integer from file */
local గుర్తులేని దీర్ఘ bget4(bin *లోపల)
{
    గుర్తులేని దీర్ఘ val;

    val = bget(లోపల);
    val += (గుర్తులేని దీర్ఘ)(bget(లోపల)) << 8;
    val += (గుర్తులేని దీర్ఘ)(bget(లోపల)) << 16;
    val += (గుర్తులేని దీర్ఘ)(bget(లోపల)) << 24;
    తిరిగి val;
}

/* skip bytes in file */
local శూన్య bskip(bin *లోపల, గుర్తులేని skip)
{
    /* check pointer */
    ఒకవేళ (లోపల == ఖాళీ)
        తిరిగి;

    /* easy case -- skip bytes in buffer */
    ఒకవేళ (skip <= లోపల->ఎడమ) {
        లోపల->ఎడమ -= skip;
        లోపల->తదుపరి += skip;
        తిరిగి;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= లోపల->ఎడమ;
    లోపల->ఎడమ = 0;

    /* seek past multiples of CHUNK bytes */
    ఒకవేళ (skip > CHUNK) {
        గుర్తులేని ఎడమ;

        ఎడమ = skip & (CHUNK - 1);
        ఒకవేళ (ఎడమ == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(లోపల->fd, skip - 1, SEEK_CUR);
            ఒకవేళ (పఠ(లోపల->fd, లోపల->buf, 1) != 1)
                bail("unexpected end of file on ", లోపల->name);
            తిరిగి;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(లోపల->fd, skip - ఎడమ, SEEK_CUR);
        skip = ఎడమ;
    }

    /* read more input and skip remainder */
    bload(లోపల);
    ఒకవేళ (skip > లోపల->ఎడమ)
        bail("unexpected end of file on ", లోపల->name);
    లోపల->ఎడమ -= skip;
    లోపల->తదుపరి += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local శూన్య gzhead(bin *లోపల)
{
    పూర్ణాంకం flags;

    /* verify gzip magic header and compression method */
    ఒకవేళ (bget(లోపల) != 0x1f || bget(లోపల) != 0x8b || bget(లోపల) != 8)
        bail(లోపల->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(లోపల);
    ఒకవేళ ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", లోపల->name);

    /* skip modification time, extra flags, and os */
    bskip(లోపల, 6);

    /* skip extra field if present */
    ఒకవేళ (flags & 4) {
        గుర్తులేని len;

        len = bget(లోపల);
        len += (గుర్తులేని)(bget(లోపల)) << 8;
        bskip(లోపల, len);
    }

    /* skip file name if present */
    ఒకవేళ (flags & 8)
        అయితే (bget(లోపల) != 0)
            ;

    /* skip comment if present */
    ఒకవేళ (flags & 16)
        అయితే (bget(లోపల) != 0)
            ;

    /* skip header crc if present */
    ఒకవేళ (flags & 2)
        bskip(లోపల, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local శూన్య put4(గుర్తులేని దీర్ఘ val, FILE *బహిర్గతం)
{
    putc(val & 0xff, బహిర్గతం);
    putc((val >> 8) & 0xff, బహిర్గతం);
    putc((val >> 16) & 0xff, బహిర్గతం);
    putc((val >> 24) & 0xff, బహిర్గతం);
}

/* Load up zlib stream from buffered input, bail if end of file */
local శూన్య zpull(z_streamp strm, bin *లోపల)
{
    ఒకవేళ (లోపల->ఎడమ == 0)
        bload(లోపల);
    ఒకవేళ (లోపల->ఎడమ == 0)
        bail("unexpected end of file on ", లోపల->name);
    strm->avail_లోపల = లోపల->ఎడమ;
    strm->తదుపరి_లోపల = లోపల->తదుపరి;
}

/* Write header for gzip file to out and initialize trailer. */
local శూన్య gzinit(గుర్తులేని దీర్ఘ *చక్రీయ_తనిఖీ, గుర్తులేని దీర్ఘ *tot, FILE *బహిర్గతం)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, బహిర్గతం);
    *చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local శూన్య gzcopy(అక్షరం *name, పూర్ణాంకం clr, గుర్తులేని దీర్ఘ *చక్రీయ_తనిఖీ, గుర్తులేని దీర్ఘ *tot,
                  FILE *బహిర్గతం)
{
    పూర్ణాంకం ret;                /* return value from zlib functions */
    పూర్ణాంకం pos;                /* where the "last block" bit is in byte */
    పూర్ణాంకం చివరి;               /* true if processing the last block */
    bin *లోపల;                /* buffered input file */
    గుర్తులేని అక్షరం *ప్రారంభం;   /* start of compressed data in buffer */
    గుర్తులేని అక్షరం *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_ప్రవాహం strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    లోపల = bopen(name);
    ఒకవేళ (లోపల == ఖాళీ)
        bail("could not open ", name);
    gzhead(లోపల);

    /* allocate buffer for uncompressed data and initialize raw inflate
ప్రవాహం*/
    junk = జ్ఞాపకం_ఇవ్వు(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit2(&strm, -15);
    ఒకవేళ (junk == ఖాళీ || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, లోపల);
    ప్రారంభం = లోపల->తదుపరి;
    చివరి = ప్రారంభం[0] & 1;
    ఒకవేళ (చివరి && clr)
        ప్రారంభం[0] &= ~1;
    strm.avail_బహిర్గతం = 0;
    కొరకు (;;) {
        /* if input used and output done, write used input and get more */
        ఒకవేళ (strm.avail_లోపల == 0 && strm.avail_బహిర్గతం != 0) {
            fwrite(ప్రారంభం, 1, strm.తదుపరి_లోపల - ప్రారంభం, బహిర్గతం);
            ప్రారంభం = లోపల->buf;
            లోపల->ఎడమ = 0;
            zpull(&strm, లోపల);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_బహిర్గతం = CHUNK;
        strm.తదుపరి_బహిర్గతం = junk;
        ret = inflate(&strm, Z_ఖండం);
        ఎంపిక (ret) {
        స్థితి Z_MEM_ERROR:
            bail("out of memory", "");
        స్థితి Z_దత్తాంశం_ERROR:
            bail("invalid compressed data in ", లోపల->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_బహిర్గతం;

        /* check for block boundary (only get this when block copied out) */
        ఒకవేళ (strm.దత్తాంశం_type & 128) {
            /* if that was the last block, then done */
            ఒకవేళ (చివరి)
                ఆపు;

            /* number of unused bits in last byte */
            pos = strm.దత్తాంశం_type & 7;

            /* find the next last-block bit */
            ఒకవేళ (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                చివరి = strm.తదుపరి_లోపల[-1] & pos;
                ఒకవేళ (చివరి && clr)
                    లోపల->buf[strm.తదుపరి_లోపల - లోపల->buf - 1] &= ~pos;
            }
            లేకపోతే {
                /* next last-block bit is in next unused byte */
                ఒకవేళ (strm.avail_లోపల == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(ప్రారంభం, 1, strm.తదుపరి_లోపల - ప్రారంభం, బహిర్గతం);
                    ప్రారంభం = లోపల->buf;
                    లోపల->ఎడమ = 0;
                    zpull(&strm, లోపల);
                }
                చివరి = strm.తదుపరి_లోపల[0] & 1;
                ఒకవేళ (చివరి && clr)
                    లోపల->buf[strm.తదుపరి_లోపల - లోపల->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    లోపల->ఎడమ = strm.avail_లోపల;
    లోపల->తదుపరి = లోపల->buf + (strm.తదుపరి_లోపల - లోపల->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.దత్తాంశం_type & 7;
    fwrite(ప్రారంభం, 1, లోపల->తదుపరి - ప్రారంభం - 1, బహిర్గతం);
    చివరి = లోపల->తదుపరి[-1];
    ఒకవేళ (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(చివరి, బహిర్గతం);
    లేకపోతే {
        /* append empty blocks to last byte */
        చివరి &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        ఒకవేళ (pos & 1) {
            /* odd -- append an empty stored block */
            putc(చివరి, బహిర్గతం);
            ఒకవేళ (pos == 1)
                putc(0, బహిర్గతం);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, బహిర్గతం);
        }
        లేకపోతే {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            ఎంపిక (pos) {
            స్థితి 6:
                putc(చివరి | 8, బహిర్గతం);
                చివరి = 0;
            స్థితి 4:
                putc(చివరి | 0x20, బహిర్గతం);
                చివరి = 0;
            స్థితి 2:
                putc(చివరి | 0x80, బహిర్గతం);
                putc(0, బహిర్గతం);
            }
        }
    }

    /* update crc and tot */
    *చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_combine(*చక్రీయ_తనిఖీ, bget4(లోపల), len);
    *tot += (గుర్తులేని దీర్ఘ)len;

    /* clean up */
    inflate_చివర(&strm);
    విడుదల(junk);
    bclose(లోపల);

    /* write trailer if this is the last gzip file */
    ఒకవేళ (!clr) {
        put4(*చక్రీయ_తనిఖీ, బహిర్గతం);
        put4(*tot, బహిర్గతం);
    }
}

/* join the gzip files on the command line, write result to stdout */
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
    గుర్తులేని దీర్ఘ చక్రీయ_తనిఖీ, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    ఒకవేళ (argc == 0) {
        fputs("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              stderr);
        తిరిగి 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&చక్రీయ_తనిఖీ, &tot, stdout);
    అయితే (argc--)
        gzcopy(*argv++, argc, &చక్రీయ_తనిఖీ, &tot, stdout);

    /*పూర్తి*/
    తిరిగి 0;
}
