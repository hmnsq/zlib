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

#وصف_ڪريو local ساكن

/* exit with an error (return a value to allow use in an expression) */
local صحيح bail(حرف *why1, حرف *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    ٻاهر_نڪرو(1);
    أرجع 0;
}

/* -- simple buffered file input with access to the buffer -- */

#وصف_ڪريو CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
تعريفنوع بنية {
    حرف *name;             /* name of file for error messages */
    صحيح fd;                 /* file descriptor */
    بنانشان کاٻو;          /* bytes remaining at next */
    بنانشان حرف *ايندڙ;    /* next byte to read */
    بنانشان حرف *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local فراغ bclose(bin *اندريون)
{
    إذا (اندريون != عدم) {
        إذا (اندريون->fd != -1)
            close(اندريون->fd);
        إذا (اندريون->buf != عدم)
            حرر(اندريون->buf);
        حرر(اندريون);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(حرف *name)
{
    bin *اندريون;

    اندريون = يادگيري_ڏيو(حجم(bin));
    إذا (اندريون == عدم)
        أرجع عدم;
    اندريون->buf = يادگيري_ڏيو(CHUNK);
    اندريون->fd = open(name, O_RDONLY, 0);
    إذا (اندريون->buf == عدم || اندريون->fd == -1) {
        bclose(اندريون);
        أرجع عدم;
    }
    اندريون->کاٻو = 0;
    اندريون->ايندڙ = اندريون->buf;
    اندريون->name = name;
    أرجع اندريون;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local صحيح bload(bin *اندريون)
{
    ڊگهو len;

    إذا (اندريون == عدم)
        أرجع -1;
    إذا (اندريون->کاٻو != 0)
        أرجع 0;
    اندريون->ايندڙ = اندريون->buf;
    افعل {
        len = (ڊگهو)اقرأ(اندريون->fd, اندريون->buf + اندريون->کاٻو, CHUNK - اندريون->کاٻو);
        إذا (len < 0)
            أرجع -1;
        اندريون->کاٻو += (بنانشان)len;
    } طالما (len != 0 && اندريون->کاٻو < CHUNK);
    أرجع len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#وصف_ڪريو bget(اندريون) (اندريون->کاٻو ? 0 : bload(اندريون), \
                  اندريون->کاٻو ? (اندريون->کاٻو--, *(اندريون->ايندڙ)++) : \
                    bail("unexpected end of file on ", اندريون->name))

/* get a four-byte little-endian unsigned integer from file */
local بنانشان ڊگهو bget4(bin *اندريون)
{
    بنانشان ڊگهو val;

    val = bget(اندريون);
    val += (بنانشان ڊگهو)(bget(اندريون)) << 8;
    val += (بنانشان ڊگهو)(bget(اندريون)) << 16;
    val += (بنانشان ڊگهو)(bget(اندريون)) << 24;
    أرجع val;
}

/* skip bytes in file */
local فراغ bskip(bin *اندريون, بنانشان skip)
{
    /* check pointer */
    إذا (اندريون == عدم)
        أرجع;

    /* easy case -- skip bytes in buffer */
    إذا (skip <= اندريون->کاٻو) {
        اندريون->کاٻو -= skip;
        اندريون->ايندڙ += skip;
        أرجع;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= اندريون->کاٻو;
    اندريون->کاٻو = 0;

    /* seek past multiples of CHUNK bytes */
    إذا (skip > CHUNK) {
        بنانشان کاٻو;

        کاٻو = skip & (CHUNK - 1);
        إذا (کاٻو == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(اندريون->fd, skip - 1, SEEK_CUR);
            إذا (اقرأ(اندريون->fd, اندريون->buf, 1) != 1)
                bail("unexpected end of file on ", اندريون->name);
            أرجع;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(اندريون->fd, skip - کاٻو, SEEK_CUR);
        skip = کاٻو;
    }

    /* read more input and skip remainder */
    bload(اندريون);
    إذا (skip > اندريون->کاٻو)
        bail("unexpected end of file on ", اندريون->name);
    اندريون->کاٻو -= skip;
    اندريون->ايندڙ += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local فراغ gzhead(bin *اندريون)
{
    صحيح flags;

    /* verify gzip magic header and compression method */
    إذا (bget(اندريون) != 0x1f || bget(اندريون) != 0x8b || bget(اندريون) != 8)
        bail(اندريون->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(اندريون);
    إذا ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", اندريون->name);

    /* skip modification time, extra flags, and os */
    bskip(اندريون, 6);

    /* skip extra field if present */
    إذا (flags & 4) {
        بنانشان len;

        len = bget(اندريون);
        len += (بنانشان)(bget(اندريون)) << 8;
        bskip(اندريون, len);
    }

    /* skip file name if present */
    إذا (flags & 8)
        طالما (bget(اندريون) != 0)
            ;

    /* skip comment if present */
    إذا (flags & 16)
        طالما (bget(اندريون) != 0)
            ;

    /* skip header crc if present */
    إذا (flags & 2)
        bskip(اندريون, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local فراغ put4(بنانشان ڊگهو val, FILE *ٻاهريون)
{
    putc(val & 0xff, ٻاهريون);
    putc((val >> 8) & 0xff, ٻاهريون);
    putc((val >> 16) & 0xff, ٻاهريون);
    putc((val >> 24) & 0xff, ٻاهريون);
}

/* Load up zlib stream from buffered input, bail if end of file */
local فراغ zpull(z_streamp strm, bin *اندريون)
{
    إذا (اندريون->کاٻو == 0)
        bload(اندريون);
    إذا (اندريون->کاٻو == 0)
        bail("unexpected end of file on ", اندريون->name);
    strm->avail_اندريون = اندريون->کاٻو;
    strm->ايندڙ_اندريون = اندريون->ايندڙ;
}

/* Write header for gzip file to out and initialize trailer. */
local فراغ gzinit(بنانشان ڊگهو *چڪري_جاچ, بنانشان ڊگهو *tot, FILE *ٻاهريون)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, ٻاهريون);
    *چڪري_جاچ = چڪري_جاچ_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local فراغ gzcopy(حرف *name, صحيح clr, بنانشان ڊگهو *چڪري_جاچ, بنانشان ڊگهو *tot,
                  FILE *ٻاهريون)
{
    صحيح ret;                /* return value from zlib functions */
    صحيح pos;                /* where the "last block" bit is in byte */
    صحيح آخري;               /* true if processing the last block */
    bin *اندريون;                /* buffered input file */
    بنانشان حرف *شروعات;   /* start of compressed data in buffer */
    بنانشان حرف *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_وهڪرو strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    اندريون = bopen(name);
    إذا (اندريون == عدم)
        bail("could not open ", name);
    gzhead(اندريون);

    /* allocate buffer for uncompressed data and initialize raw inflate
وهڪرو*/
    junk = يادگيري_ڏيو(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_اندريون = 0;
    strm.ايندڙ_اندريون = Z_NULL;
    ret = inflateInit2(&strm, -15);
    إذا (junk == عدم || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, اندريون);
    شروعات = اندريون->ايندڙ;
    آخري = شروعات[0] & 1;
    إذا (آخري && clr)
        شروعات[0] &= ~1;
    strm.avail_ٻاهريون = 0;
    لكل (;;) {
        /* if input used and output done, write used input and get more */
        إذا (strm.avail_اندريون == 0 && strm.avail_ٻاهريون != 0) {
            fwrite(شروعات, 1, strm.ايندڙ_اندريون - شروعات, ٻاهريون);
            شروعات = اندريون->buf;
            اندريون->کاٻو = 0;
            zpull(&strm, اندريون);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_ٻاهريون = CHUNK;
        strm.ايندڙ_ٻاهريون = junk;
        ret = inflate(&strm, Z_بلاڪ);
        بدل (ret) {
        حالة Z_MEM_ERROR:
            bail("out of memory", "");
        حالة Z_معلومات_ERROR:
            bail("invalid compressed data in ", اندريون->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_ٻاهريون;

        /* check for block boundary (only get this when block copied out) */
        إذا (strm.معلومات_type & 128) {
            /* if that was the last block, then done */
            إذا (آخري)
                اكسر;

            /* number of unused bits in last byte */
            pos = strm.معلومات_type & 7;

            /* find the next last-block bit */
            إذا (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                آخري = strm.ايندڙ_اندريون[-1] & pos;
                إذا (آخري && clr)
                    اندريون->buf[strm.ايندڙ_اندريون - اندريون->buf - 1] &= ~pos;
            }
            وإلا {
                /* next last-block bit is in next unused byte */
                إذا (strm.avail_اندريون == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(شروعات, 1, strm.ايندڙ_اندريون - شروعات, ٻاهريون);
                    شروعات = اندريون->buf;
                    اندريون->کاٻو = 0;
                    zpull(&strm, اندريون);
                }
                آخري = strm.ايندڙ_اندريون[0] & 1;
                إذا (آخري && clr)
                    اندريون->buf[strm.ايندڙ_اندريون - اندريون->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    اندريون->کاٻو = strm.avail_اندريون;
    اندريون->ايندڙ = اندريون->buf + (strm.ايندڙ_اندريون - اندريون->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.معلومات_type & 7;
    fwrite(شروعات, 1, اندريون->ايندڙ - شروعات - 1, ٻاهريون);
    آخري = اندريون->ايندڙ[-1];
    إذا (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(آخري, ٻاهريون);
    وإلا {
        /* append empty blocks to last byte */
        آخري &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        إذا (pos & 1) {
            /* odd -- append an empty stored block */
            putc(آخري, ٻاهريون);
            إذا (pos == 1)
                putc(0, ٻاهريون);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, ٻاهريون);
        }
        وإلا {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            بدل (pos) {
            حالة 6:
                putc(آخري | 8, ٻاهريون);
                آخري = 0;
            حالة 4:
                putc(آخري | 0x20, ٻاهريون);
                آخري = 0;
            حالة 2:
                putc(آخري | 0x80, ٻاهريون);
                putc(0, ٻاهريون);
            }
        }
    }

    /* update crc and tot */
    *چڪري_جاچ = چڪري_جاچ_32_combine(*چڪري_جاچ, bget4(اندريون), len);
    *tot += (بنانشان ڊگهو)len;

    /* clean up */
    inflate_پڇاڙي(&strm);
    حرر(junk);
    bclose(اندريون);

    /* write trailer if this is the last gzip file */
    إذا (!clr) {
        put4(*چڪري_جاچ, ٻاهريون);
        put4(*tot, ٻاهريون);
    }
}

/* join the gzip files on the command line, write result to stdout */
صحيح مکيه(صحيح argc, حرف **argv)
{
    بنانشان ڊگهو چڪري_جاچ, tot;     /* running crc and total uncompressed length */

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
    gzinit(&چڪري_جاچ, &tot, stdout);
    طالما (argc--)
        gzcopy(*argv++, argc, &چڪري_جاچ, &tot, stdout);

    /*پورو*/
    أرجع 0;
}
