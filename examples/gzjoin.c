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

#شامل <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#شامل <stdlib.h>     /* exit(), malloc(), free() */
#شامل <fcntl.h>      /* open() */
#شامل <unistd.h>     /* close(), read(), lseek() */
#شامل "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#تعریف_کرو local ساکن

/* exit with an error (return a value to allow use in an expression) */
local صحیح bail(حرف *why1, حرف *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    باہر_نکلو(1);
    واپس 0;
}

/* -- simple buffered file input with access to the buffer -- */

#تعریف_کرو CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
قسمتعریف ساخت {
    حرف *name;             /* name of file for error messages */
    صحیح fd;                 /* file descriptor */
    غیرعلامتی بایاں;          /* bytes remaining at next */
    غیرعلامتی حرف *اگلا;    /* next byte to read */
    غیرعلامتی حرف *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local خالی bclose(bin *اندر)
{
    اگر (اندر != تہی) {
        اگر (اندر->fd != -1)
            close(اندر->fd);
        اگر (اندر->buf != تہی)
            آزاد(اندر->buf);
        آزاد(اندر);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(حرف *name)
{
    bin *اندر;

    اندر = حافظہ_دو(حجم(bin));
    اگر (اندر == تہی)
        واپس تہی;
    اندر->buf = حافظہ_دو(CHUNK);
    اندر->fd = open(name, O_RDONLY, 0);
    اگر (اندر->buf == تہی || اندر->fd == -1) {
        bclose(اندر);
        واپس تہی;
    }
    اندر->بایاں = 0;
    اندر->اگلا = اندر->buf;
    اندر->name = name;
    واپس اندر;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local صحیح bload(bin *اندر)
{
    طویل len;

    اگر (اندر == تہی)
        واپس -1;
    اگر (اندر->بایاں != 0)
        واپس 0;
    اندر->اگلا = اندر->buf;
    کرو {
        len = (طویل)اقرأ(اندر->fd, اندر->buf + اندر->بایاں, CHUNK - اندر->بایاں);
        اگر (len < 0)
            واپس -1;
        اندر->بایاں += (غیرعلامتی)len;
    } جب (len != 0 && اندر->بایاں < CHUNK);
    واپس len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#تعریف_کرو bget(اندر) (اندر->بایاں ? 0 : bload(اندر), \
                  اندر->بایاں ? (اندر->بایاں--, *(اندر->اگلا)++) : \
                    bail("unexpected end of file on ", اندر->name))

/* get a four-byte little-endian unsigned integer from file */
local غیرعلامتی طویل bget4(bin *اندر)
{
    غیرعلامتی طویل val;

    val = bget(اندر);
    val += (غیرعلامتی طویل)(bget(اندر)) << 8;
    val += (غیرعلامتی طویل)(bget(اندر)) << 16;
    val += (غیرعلامتی طویل)(bget(اندر)) << 24;
    واپس val;
}

/* skip bytes in file */
local خالی bskip(bin *اندر, غیرعلامتی skip)
{
    /* check pointer */
    اگر (اندر == تہی)
        واپس;

    /* easy case -- skip bytes in buffer */
    اگر (skip <= اندر->بایاں) {
        اندر->بایاں -= skip;
        اندر->اگلا += skip;
        واپس;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= اندر->بایاں;
    اندر->بایاں = 0;

    /* seek past multiples of CHUNK bytes */
    اگر (skip > CHUNK) {
        غیرعلامتی بایاں;

        بایاں = skip & (CHUNK - 1);
        اگر (بایاں == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(اندر->fd, skip - 1, SEEK_CUR);
            اگر (اقرأ(اندر->fd, اندر->buf, 1) != 1)
                bail("unexpected end of file on ", اندر->name);
            واپس;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(اندر->fd, skip - بایاں, SEEK_CUR);
        skip = بایاں;
    }

    /* read more input and skip remainder */
    bload(اندر);
    اگر (skip > اندر->بایاں)
        bail("unexpected end of file on ", اندر->name);
    اندر->بایاں -= skip;
    اندر->اگلا += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local خالی gzhead(bin *اندر)
{
    صحیح flags;

    /* verify gzip magic header and compression method */
    اگر (bget(اندر) != 0x1f || bget(اندر) != 0x8b || bget(اندر) != 8)
        bail(اندر->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(اندر);
    اگر ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", اندر->name);

    /* skip modification time, extra flags, and os */
    bskip(اندر, 6);

    /* skip extra field if present */
    اگر (flags & 4) {
        غیرعلامتی len;

        len = bget(اندر);
        len += (غیرعلامتی)(bget(اندر)) << 8;
        bskip(اندر, len);
    }

    /* skip file name if present */
    اگر (flags & 8)
        جب (bget(اندر) != 0)
            ;

    /* skip comment if present */
    اگر (flags & 16)
        جب (bget(اندر) != 0)
            ;

    /* skip header crc if present */
    اگر (flags & 2)
        bskip(اندر, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local خالی put4(غیرعلامتی طویل val, FILE *برآمد)
{
    putc(val & 0xff, برآمد);
    putc((val >> 8) & 0xff, برآمد);
    putc((val >> 16) & 0xff, برآمد);
    putc((val >> 24) & 0xff, برآمد);
}

/* Load up zlib stream from buffered input, bail if end of file */
local خالی zpull(z_streamp strm, bin *اندر)
{
    اگر (اندر->بایاں == 0)
        bload(اندر);
    اگر (اندر->بایاں == 0)
        bail("unexpected end of file on ", اندر->name);
    strm->avail_اندر = اندر->بایاں;
    strm->اگلا_اندر = اندر->اگلا;
}

/* Write header for gzip file to out and initialize trailer. */
local خالی gzinit(غیرعلامتی طویل *چکری_جانچ, غیرعلامتی طویل *tot, FILE *برآمد)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, برآمد);
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
local خالی gzcopy(حرف *name, صحیح clr, غیرعلامتی طویل *چکری_جانچ, غیرعلامتی طویل *tot,
                  FILE *برآمد)
{
    صحیح ret;                /* return value from zlib functions */
    صحیح pos;                /* where the "last block" bit is in byte */
    صحیح آخری;               /* true if processing the last block */
    bin *اندر;                /* buffered input file */
    غیرعلامتی حرف *آغاز;   /* start of compressed data in buffer */
    غیرعلامتی حرف *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_دھارا strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    اندر = bopen(name);
    اگر (اندر == تہی)
        bail("could not open ", name);
    gzhead(اندر);

    /* allocate buffer for uncompressed data and initialize raw inflate
دھارا*/
    junk = حافظہ_دو(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_اندر = 0;
    strm.اگلا_اندر = Z_NULL;
    ret = inflateInit2(&strm, -15);
    اگر (junk == تہی || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, اندر);
    آغاز = اندر->اگلا;
    آخری = آغاز[0] & 1;
    اگر (آخری && clr)
        آغاز[0] &= ~1;
    strm.avail_برآمد = 0;
    برائے (;;) {
        /* if input used and output done, write used input and get more */
        اگر (strm.avail_اندر == 0 && strm.avail_برآمد != 0) {
            fwrite(آغاز, 1, strm.اگلا_اندر - آغاز, برآمد);
            آغاز = اندر->buf;
            اندر->بایاں = 0;
            zpull(&strm, اندر);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_برآمد = CHUNK;
        strm.اگلا_برآمد = junk;
        ret = inflate(&strm, Z_بلاک);
        انتخاب (ret) {
        حالت Z_MEM_ERROR:
            bail("out of memory", "");
        حالت Z_ڈیٹا_ERROR:
            bail("invalid compressed data in ", اندر->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_برآمد;

        /* check for block boundary (only get this when block copied out) */
        اگر (strm.ڈیٹا_type & 128) {
            /* if that was the last block, then done */
            اگر (آخری)
                توڑو;

            /* number of unused bits in last byte */
            pos = strm.ڈیٹا_type & 7;

            /* find the next last-block bit */
            اگر (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                آخری = strm.اگلا_اندر[-1] & pos;
                اگر (آخری && clr)
                    اندر->buf[strm.اگلا_اندر - اندر->buf - 1] &= ~pos;
            }
            ورنہ {
                /* next last-block bit is in next unused byte */
                اگر (strm.avail_اندر == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(آغاز, 1, strm.اگلا_اندر - آغاز, برآمد);
                    آغاز = اندر->buf;
                    اندر->بایاں = 0;
                    zpull(&strm, اندر);
                }
                آخری = strm.اگلا_اندر[0] & 1;
                اگر (آخری && clr)
                    اندر->buf[strm.اگلا_اندر - اندر->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    اندر->بایاں = strm.avail_اندر;
    اندر->اگلا = اندر->buf + (strm.اگلا_اندر - اندر->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.ڈیٹا_type & 7;
    fwrite(آغاز, 1, اندر->اگلا - آغاز - 1, برآمد);
    آخری = اندر->اگلا[-1];
    اگر (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(آخری, برآمد);
    ورنہ {
        /* append empty blocks to last byte */
        آخری &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        اگر (pos & 1) {
            /* odd -- append an empty stored block */
            putc(آخری, برآمد);
            اگر (pos == 1)
                putc(0, برآمد);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, برآمد);
        }
        ورنہ {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            انتخاب (pos) {
            حالت 6:
                putc(آخری | 8, برآمد);
                آخری = 0;
            حالت 4:
                putc(آخری | 0x20, برآمد);
                آخری = 0;
            حالت 2:
                putc(آخری | 0x80, برآمد);
                putc(0, برآمد);
            }
        }
    }

    /* update crc and tot */
    *چکری_جانچ = چکری_جانچ_32_combine(*چکری_جانچ, bget4(اندر), len);
    *tot += (غیرعلامتی طویل)len;

    /* clean up */
    inflate_انتہا(&strm);
    آزاد(junk);
    bclose(اندر);

    /* write trailer if this is the last gzip file */
    اگر (!clr) {
        put4(*چکری_جانچ, برآمد);
        put4(*tot, برآمد);
    }
}

/* join the gzip files on the command line, write result to stdout */
صحیح مین(صحیح argc, حرف **argv)
{
    غیرعلامتی طویل چکری_جانچ, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    اگر (argc == 0) {
        fputs("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              stderr);
        واپس 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&چکری_جانچ, &tot, stdout);
    جب (argc--)
        gzcopy(*argv++, argc, &چکری_جانچ, &tot, stdout);

    /*مکمل*/
    واپس 0;
}
