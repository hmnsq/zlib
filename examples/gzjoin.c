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

#تعریف_کن local ساكن

/* exit with an error (return a value to allow use in an expression) */
local صحيح bail(حرف *why1, حرف *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    خروج(1);
    أرجع 0;
}

/* -- simple buffered file input with access to the buffer -- */

#تعریف_کن CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
تعريفنوع بنية {
    حرف *name;             /* name of file for error messages */
    صحيح fd;                 /* file descriptor */
    بیعلامت چپ;          /* bytes remaining at next */
    بیعلامت حرف *بعدی;    /* next byte to read */
    بیعلامت حرف *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local فراغ bclose(bin *درون)
{
    إذا (درون != عدم) {
        إذا (درون->fd != -1)
            close(درون->fd);
        إذا (درون->buf != عدم)
            حرر(درون->buf);
        حرر(درون);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(حرف *name)
{
    bin *درون;

    درون = حافظه_بده(حجم(bin));
    إذا (درون == عدم)
        أرجع عدم;
    درون->buf = حافظه_بده(CHUNK);
    درون->fd = open(name, O_RDONLY, 0);
    إذا (درون->buf == عدم || درون->fd == -1) {
        bclose(درون);
        أرجع عدم;
    }
    درون->چپ = 0;
    درون->بعدی = درون->buf;
    درون->name = name;
    أرجع درون;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local صحيح bload(bin *درون)
{
    دراز len;

    إذا (درون == عدم)
        أرجع -1;
    إذا (درون->چپ != 0)
        أرجع 0;
    درون->بعدی = درون->buf;
    افعل {
        len = (دراز)اقرأ(درون->fd, درون->buf + درون->چپ, CHUNK - درون->چپ);
        إذا (len < 0)
            أرجع -1;
        درون->چپ += (بیعلامت)len;
    } طالما (len != 0 && درون->چپ < CHUNK);
    أرجع len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#تعریف_کن bget(درون) (درون->چپ ? 0 : bload(درون), \
                  درون->چپ ? (درون->چپ--, *(درون->بعدی)++) : \
                    bail("unexpected end of file on ", درون->name))

/* get a four-byte little-endian unsigned integer from file */
local بیعلامت دراز bget4(bin *درون)
{
    بیعلامت دراز val;

    val = bget(درون);
    val += (بیعلامت دراز)(bget(درون)) << 8;
    val += (بیعلامت دراز)(bget(درون)) << 16;
    val += (بیعلامت دراز)(bget(درون)) << 24;
    أرجع val;
}

/* skip bytes in file */
local فراغ bskip(bin *درون, بیعلامت skip)
{
    /* check pointer */
    إذا (درون == عدم)
        أرجع;

    /* easy case -- skip bytes in buffer */
    إذا (skip <= درون->چپ) {
        درون->چپ -= skip;
        درون->بعدی += skip;
        أرجع;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= درون->چپ;
    درون->چپ = 0;

    /* seek past multiples of CHUNK bytes */
    إذا (skip > CHUNK) {
        بیعلامت چپ;

        چپ = skip & (CHUNK - 1);
        إذا (چپ == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(درون->fd, skip - 1, SEEK_CUR);
            إذا (اقرأ(درون->fd, درون->buf, 1) != 1)
                bail("unexpected end of file on ", درون->name);
            أرجع;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(درون->fd, skip - چپ, SEEK_CUR);
        skip = چپ;
    }

    /* read more input and skip remainder */
    bload(درون);
    إذا (skip > درون->چپ)
        bail("unexpected end of file on ", درون->name);
    درون->چپ -= skip;
    درون->بعدی += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local فراغ gzhead(bin *درون)
{
    صحيح flags;

    /* verify gzip magic header and compression method */
    إذا (bget(درون) != 0x1f || bget(درون) != 0x8b || bget(درون) != 8)
        bail(درون->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(درون);
    إذا ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", درون->name);

    /* skip modification time, extra flags, and os */
    bskip(درون, 6);

    /* skip extra field if present */
    إذا (flags & 4) {
        بیعلامت len;

        len = bget(درون);
        len += (بیعلامت)(bget(درون)) << 8;
        bskip(درون, len);
    }

    /* skip file name if present */
    إذا (flags & 8)
        طالما (bget(درون) != 0)
            ;

    /* skip comment if present */
    إذا (flags & 16)
        طالما (bget(درون) != 0)
            ;

    /* skip header crc if present */
    إذا (flags & 2)
        bskip(درون, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local فراغ put4(بیعلامت دراز val, FILE *بیرون)
{
    putc(val & 0xff, بیرون);
    putc((val >> 8) & 0xff, بیرون);
    putc((val >> 16) & 0xff, بیرون);
    putc((val >> 24) & 0xff, بیرون);
}

/* Load up zlib stream from buffered input, bail if end of file */
local فراغ zpull(z_streamp strm, bin *درون)
{
    إذا (درون->چپ == 0)
        bload(درون);
    إذا (درون->چپ == 0)
        bail("unexpected end of file on ", درون->name);
    strm->avail_درون = درون->چپ;
    strm->بعدی_درون = درون->بعدی;
}

/* Write header for gzip file to out and initialize trailer. */
local فراغ gzinit(بیعلامت دراز *بررسی_چرخهای, بیعلامت دراز *tot, FILE *بیرون)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, بیرون);
    *بررسی_چرخهای = بررسی_چرخهای_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local فراغ gzcopy(حرف *name, صحيح clr, بیعلامت دراز *بررسی_چرخهای, بیعلامت دراز *tot,
                  FILE *بیرون)
{
    صحيح ret;                /* return value from zlib functions */
    صحيح pos;                /* where the "last block" bit is in byte */
    صحيح آخرین;               /* true if processing the last block */
    bin *درون;                /* buffered input file */
    بیعلامت حرف *شروع;   /* start of compressed data in buffer */
    بیعلامت حرف *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_جریان strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    درون = bopen(name);
    إذا (درون == عدم)
        bail("could not open ", name);
    gzhead(درون);

    /* allocate buffer for uncompressed data and initialize raw inflate
جریان*/
    junk = حافظه_بده(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_درون = 0;
    strm.بعدی_درون = Z_NULL;
    ret = inflateInit2(&strm, -15);
    إذا (junk == عدم || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, درون);
    شروع = درون->بعدی;
    آخرین = شروع[0] & 1;
    إذا (آخرین && clr)
        شروع[0] &= ~1;
    strm.avail_بیرون = 0;
    لكل (;;) {
        /* if input used and output done, write used input and get more */
        إذا (strm.avail_درون == 0 && strm.avail_بیرون != 0) {
            fwrite(شروع, 1, strm.بعدی_درون - شروع, بیرون);
            شروع = درون->buf;
            درون->چپ = 0;
            zpull(&strm, درون);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_بیرون = CHUNK;
        strm.بعدی_بیرون = junk;
        ret = inflate(&strm, Z_بلوک);
        بدل (ret) {
        حالة Z_MEM_ERROR:
            bail("out of memory", "");
        حالة Z_داده_ERROR:
            bail("invalid compressed data in ", درون->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_بیرون;

        /* check for block boundary (only get this when block copied out) */
        إذا (strm.داده_type & 128) {
            /* if that was the last block, then done */
            إذا (آخرین)
                اكسر;

            /* number of unused bits in last byte */
            pos = strm.داده_type & 7;

            /* find the next last-block bit */
            إذا (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                آخرین = strm.بعدی_درون[-1] & pos;
                إذا (آخرین && clr)
                    درون->buf[strm.بعدی_درون - درون->buf - 1] &= ~pos;
            }
            وإلا {
                /* next last-block bit is in next unused byte */
                إذا (strm.avail_درون == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(شروع, 1, strm.بعدی_درون - شروع, بیرون);
                    شروع = درون->buf;
                    درون->چپ = 0;
                    zpull(&strm, درون);
                }
                آخرین = strm.بعدی_درون[0] & 1;
                إذا (آخرین && clr)
                    درون->buf[strm.بعدی_درون - درون->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    درون->چپ = strm.avail_درون;
    درون->بعدی = درون->buf + (strm.بعدی_درون - درون->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.داده_type & 7;
    fwrite(شروع, 1, درون->بعدی - شروع - 1, بیرون);
    آخرین = درون->بعدی[-1];
    إذا (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(آخرین, بیرون);
    وإلا {
        /* append empty blocks to last byte */
        آخرین &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        إذا (pos & 1) {
            /* odd -- append an empty stored block */
            putc(آخرین, بیرون);
            إذا (pos == 1)
                putc(0, بیرون);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, بیرون);
        }
        وإلا {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            بدل (pos) {
            حالة 6:
                putc(آخرین | 8, بیرون);
                آخرین = 0;
            حالة 4:
                putc(آخرین | 0x20, بیرون);
                آخرین = 0;
            حالة 2:
                putc(آخرین | 0x80, بیرون);
                putc(0, بیرون);
            }
        }
    }

    /* update crc and tot */
    *بررسی_چرخهای = بررسی_چرخهای_32_combine(*بررسی_چرخهای, bget4(درون), len);
    *tot += (بیعلامت دراز)len;

    /* clean up */
    inflate_انتها(&strm);
    حرر(junk);
    bclose(درون);

    /* write trailer if this is the last gzip file */
    إذا (!clr) {
        put4(*بررسی_چرخهای, بیرون);
        put4(*tot, بیرون);
    }
}

/* join the gzip files on the command line, write result to stdout */
صحيح اصلی(صحيح argc, حرف **argv)
{
    بیعلامت دراز بررسی_چرخهای, tot;     /* running crc and total uncompressed length */

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
    gzinit(&بررسی_چرخهای, &tot, stdout);
    طالما (argc--)
        gzcopy(*argv++, argc, &بررسی_چرخهای, &tot, stdout);

    /*انجامشده*/
    أرجع 0;
}
