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

#ضمن <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#ضمن <stdlib.h>     /* exit(), malloc(), free() */
#ضمن <fcntl.h>      /* open() */
#ضمن <unistd.h>     /* close(), read(), lseek() */
#ضمن "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#تعریف_کرو local ساكن

/* exit with an error (return a value to allow use in an expression) */
local صحيح bail(حرف *why1, حرف *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    باہر_نکلو(1);
    أرجع 0;
}

/* -- simple buffered file input with access to the buffer -- */

#تعریف_کرو CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
تعريفنوع بنية {
    حرف *name;             /* name of file for error messages */
    صحيح fd;                 /* file descriptor */
    بنانشانی بایاں;          /* bytes remaining at next */
    بنانشانی حرف *اگلا;    /* next byte to read */
    بنانشانی حرف *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local فراغ bclose(bin *وچ)
{
    إذا (وچ != عدم) {
        إذا (وچ->fd != -1)
            close(وچ->fd);
        إذا (وچ->buf != عدم)
            حرر(وچ->buf);
        حرر(وچ);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(حرف *name)
{
    bin *وچ;

    وچ = حافظہ_دیو(حجم(bin));
    إذا (وچ == عدم)
        أرجع عدم;
    وچ->buf = حافظہ_دیو(CHUNK);
    وچ->fd = open(name, O_RDONLY, 0);
    إذا (وچ->buf == عدم || وچ->fd == -1) {
        bclose(وچ);
        أرجع عدم;
    }
    وچ->بایاں = 0;
    وچ->اگلا = وچ->buf;
    وچ->name = name;
    أرجع وچ;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local صحيح bload(bin *وچ)
{
    لما len;

    إذا (وچ == عدم)
        أرجع -1;
    إذا (وچ->بایاں != 0)
        أرجع 0;
    وچ->اگلا = وچ->buf;
    افعل {
        len = (لما)اقرأ(وچ->fd, وچ->buf + وچ->بایاں, CHUNK - وچ->بایاں);
        إذا (len < 0)
            أرجع -1;
        وچ->بایاں += (بنانشانی)len;
    } طالما (len != 0 && وچ->بایاں < CHUNK);
    أرجع len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#تعریف_کرو bget(وچ) (وچ->بایاں ? 0 : bload(وچ), \
                  وچ->بایاں ? (وچ->بایاں--, *(وچ->اگلا)++) : \
                    bail("unexpected end of file on ", وچ->name))

/* get a four-byte little-endian unsigned integer from file */
local بنانشانی لما bget4(bin *وچ)
{
    بنانشانی لما val;

    val = bget(وچ);
    val += (بنانشانی لما)(bget(وچ)) << 8;
    val += (بنانشانی لما)(bget(وچ)) << 16;
    val += (بنانشانی لما)(bget(وچ)) << 24;
    أرجع val;
}

/* skip bytes in file */
local فراغ bskip(bin *وچ, بنانشانی skip)
{
    /* check pointer */
    إذا (وچ == عدم)
        أرجع;

    /* easy case -- skip bytes in buffer */
    إذا (skip <= وچ->بایاں) {
        وچ->بایاں -= skip;
        وچ->اگلا += skip;
        أرجع;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= وچ->بایاں;
    وچ->بایاں = 0;

    /* seek past multiples of CHUNK bytes */
    إذا (skip > CHUNK) {
        بنانشانی بایاں;

        بایاں = skip & (CHUNK - 1);
        إذا (بایاں == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(وچ->fd, skip - 1, SEEK_CUR);
            إذا (اقرأ(وچ->fd, وچ->buf, 1) != 1)
                bail("unexpected end of file on ", وچ->name);
            أرجع;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(وچ->fd, skip - بایاں, SEEK_CUR);
        skip = بایاں;
    }

    /* read more input and skip remainder */
    bload(وچ);
    إذا (skip > وچ->بایاں)
        bail("unexpected end of file on ", وچ->name);
    وچ->بایاں -= skip;
    وچ->اگلا += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local فراغ gzhead(bin *وچ)
{
    صحيح flags;

    /* verify gzip magic header and compression method */
    إذا (bget(وچ) != 0x1f || bget(وچ) != 0x8b || bget(وچ) != 8)
        bail(وچ->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(وچ);
    إذا ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", وچ->name);

    /* skip modification time, extra flags, and os */
    bskip(وچ, 6);

    /* skip extra field if present */
    إذا (flags & 4) {
        بنانشانی len;

        len = bget(وچ);
        len += (بنانشانی)(bget(وچ)) << 8;
        bskip(وچ, len);
    }

    /* skip file name if present */
    إذا (flags & 8)
        طالما (bget(وچ) != 0)
            ;

    /* skip comment if present */
    إذا (flags & 16)
        طالما (bget(وچ) != 0)
            ;

    /* skip header crc if present */
    إذا (flags & 2)
        bskip(وچ, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local فراغ put4(بنانشانی لما val, FILE *باہرلا)
{
    putc(val & 0xff, باہرلا);
    putc((val >> 8) & 0xff, باہرلا);
    putc((val >> 16) & 0xff, باہرلا);
    putc((val >> 24) & 0xff, باہرلا);
}

/* Load up zlib stream from buffered input, bail if end of file */
local فراغ zpull(z_streamp strm, bin *وچ)
{
    إذا (وچ->بایاں == 0)
        bload(وچ);
    إذا (وچ->بایاں == 0)
        bail("unexpected end of file on ", وچ->name);
    strm->avail_وچ = وچ->بایاں;
    strm->اگلا_وچ = وچ->اگلا;
}

/* Write header for gzip file to out and initialize trailer. */
local فراغ gzinit(بنانشانی لما *چکری_جانچ, بنانشانی لما *tot, FILE *باہرلا)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, باہرلا);
    *چکری_جانچ = چکری_جانچ_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local فراغ gzcopy(حرف *name, صحيح clr, بنانشانی لما *چکری_جانچ, بنانشانی لما *tot,
                  FILE *باہرلا)
{
    صحيح ret;                /* return value from zlib functions */
    صحيح pos;                /* where the "last block" bit is in byte */
    صحيح آخری;               /* true if processing the last block */
    bin *وچ;                /* buffered input file */
    بنانشانی حرف *شروع;   /* start of compressed data in buffer */
    بنانشانی حرف *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_دھارا strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    وچ = bopen(name);
    إذا (وچ == عدم)
        bail("could not open ", name);
    gzhead(وچ);

    /* allocate buffer for uncompressed data and initialize raw inflate
دھارا*/
    junk = حافظہ_دیو(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_وچ = 0;
    strm.اگلا_وچ = Z_NULL;
    ret = inflateInit2(&strm, -15);
    إذا (junk == عدم || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, وچ);
    شروع = وچ->اگلا;
    آخری = شروع[0] & 1;
    إذا (آخری && clr)
        شروع[0] &= ~1;
    strm.avail_باہرلا = 0;
    لكل (;;) {
        /* if input used and output done, write used input and get more */
        إذا (strm.avail_وچ == 0 && strm.avail_باہرلا != 0) {
            fwrite(شروع, 1, strm.اگلا_وچ - شروع, باہرلا);
            شروع = وچ->buf;
            وچ->بایاں = 0;
            zpull(&strm, وچ);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_باہرلا = CHUNK;
        strm.اگلا_باہرلا = junk;
        ret = inflate(&strm, Z_بلاک);
        بدل (ret) {
        حالة Z_MEM_ERROR:
            bail("out of memory", "");
        حالة Z_انکڑے_ERROR:
            bail("invalid compressed data in ", وچ->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_باہرلا;

        /* check for block boundary (only get this when block copied out) */
        إذا (strm.انکڑے_type & 128) {
            /* if that was the last block, then done */
            إذا (آخری)
                اكسر;

            /* number of unused bits in last byte */
            pos = strm.انکڑے_type & 7;

            /* find the next last-block bit */
            إذا (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                آخری = strm.اگلا_وچ[-1] & pos;
                إذا (آخری && clr)
                    وچ->buf[strm.اگلا_وچ - وچ->buf - 1] &= ~pos;
            }
            وإلا {
                /* next last-block bit is in next unused byte */
                إذا (strm.avail_وچ == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(شروع, 1, strm.اگلا_وچ - شروع, باہرلا);
                    شروع = وچ->buf;
                    وچ->بایاں = 0;
                    zpull(&strm, وچ);
                }
                آخری = strm.اگلا_وچ[0] & 1;
                إذا (آخری && clr)
                    وچ->buf[strm.اگلا_وچ - وچ->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    وچ->بایاں = strm.avail_وچ;
    وچ->اگلا = وچ->buf + (strm.اگلا_وچ - وچ->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.انکڑے_type & 7;
    fwrite(شروع, 1, وچ->اگلا - شروع - 1, باہرلا);
    آخری = وچ->اگلا[-1];
    إذا (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(آخری, باہرلا);
    وإلا {
        /* append empty blocks to last byte */
        آخری &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        إذا (pos & 1) {
            /* odd -- append an empty stored block */
            putc(آخری, باہرلا);
            إذا (pos == 1)
                putc(0, باہرلا);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, باہرلا);
        }
        وإلا {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            بدل (pos) {
            حالة 6:
                putc(آخری | 8, باہرلا);
                آخری = 0;
            حالة 4:
                putc(آخری | 0x20, باہرلا);
                آخری = 0;
            حالة 2:
                putc(آخری | 0x80, باہرلا);
                putc(0, باہرلا);
            }
        }
    }

    /* update crc and tot */
    *چکری_جانچ = چکری_جانچ_32_combine(*چکری_جانچ, bget4(وچ), len);
    *tot += (بنانشانی لما)len;

    /* clean up */
    inflate_سرا(&strm);
    حرر(junk);
    bclose(وچ);

    /* write trailer if this is the last gzip file */
    إذا (!clr) {
        put4(*چکری_جانچ, باہرلا);
        put4(*tot, باہرلا);
    }
}

/* join the gzip files on the command line, write result to stdout */
صحيح مکھ(صحيح argc, حرف **argv)
{
    بنانشانی لما چکری_جانچ, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    إذا (argc == 0) {
        fputs("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              stderr);
        أرجع 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&چکری_جانچ, &tot, stdout);
    طالما (argc--)
        gzcopy(*argv++, argc, &چکری_جانچ, &tot, stdout);

    /*پورا*/
    أرجع 0;
}
