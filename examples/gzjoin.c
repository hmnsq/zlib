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

#تعریف_کړه local ساكن

/* exit with an error (return a value to allow use in an expression) */
local صحيح bail(حرف *why1, حرف *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    وتل(1);
    أرجع 0;
}

/* -- simple buffered file input with access to the buffer -- */

#تعریف_کړه CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
تعريفنوع بنية {
    حرف *name;             /* name of file for error messages */
    صحيح fd;                 /* file descriptor */
    بینښه کیڼ;          /* bytes remaining at next */
    بینښه حرف *بل;    /* next byte to read */
    بینښه حرف *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local فراغ bclose(bin *دننه)
{
    إذا (دننه != عدم) {
        إذا (دننه->fd != -1)
            close(دننه->fd);
        إذا (دننه->buf != عدم)
            حرر(دننه->buf);
        حرر(دننه);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(حرف *name)
{
    bin *دننه;

    دننه = حافظه_ورکړه(حجم(bin));
    إذا (دننه == عدم)
        أرجع عدم;
    دننه->buf = حافظه_ورکړه(CHUNK);
    دننه->fd = open(name, O_RDONLY, 0);
    إذا (دننه->buf == عدم || دننه->fd == -1) {
        bclose(دننه);
        أرجع عدم;
    }
    دننه->کیڼ = 0;
    دننه->بل = دننه->buf;
    دننه->name = name;
    أرجع دننه;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local صحيح bload(bin *دننه)
{
    اوږد len;

    إذا (دننه == عدم)
        أرجع -1;
    إذا (دننه->کیڼ != 0)
        أرجع 0;
    دننه->بل = دننه->buf;
    افعل {
        len = (اوږد)اقرأ(دننه->fd, دننه->buf + دننه->کیڼ, CHUNK - دننه->کیڼ);
        إذا (len < 0)
            أرجع -1;
        دننه->کیڼ += (بینښه)len;
    } طالما (len != 0 && دننه->کیڼ < CHUNK);
    أرجع len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#تعریف_کړه bget(دننه) (دننه->کیڼ ? 0 : bload(دننه), \
                  دننه->کیڼ ? (دننه->کیڼ--, *(دننه->بل)++) : \
                    bail("unexpected end of file on ", دننه->name))

/* get a four-byte little-endian unsigned integer from file */
local بینښه اوږد bget4(bin *دننه)
{
    بینښه اوږد val;

    val = bget(دننه);
    val += (بینښه اوږد)(bget(دننه)) << 8;
    val += (بینښه اوږد)(bget(دننه)) << 16;
    val += (بینښه اوږد)(bget(دننه)) << 24;
    أرجع val;
}

/* skip bytes in file */
local فراغ bskip(bin *دننه, بینښه skip)
{
    /* check pointer */
    إذا (دننه == عدم)
        أرجع;

    /* easy case -- skip bytes in buffer */
    إذا (skip <= دننه->کیڼ) {
        دننه->کیڼ -= skip;
        دننه->بل += skip;
        أرجع;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= دننه->کیڼ;
    دننه->کیڼ = 0;

    /* seek past multiples of CHUNK bytes */
    إذا (skip > CHUNK) {
        بینښه کیڼ;

        کیڼ = skip & (CHUNK - 1);
        إذا (کیڼ == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(دننه->fd, skip - 1, SEEK_CUR);
            إذا (اقرأ(دننه->fd, دننه->buf, 1) != 1)
                bail("unexpected end of file on ", دننه->name);
            أرجع;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(دننه->fd, skip - کیڼ, SEEK_CUR);
        skip = کیڼ;
    }

    /* read more input and skip remainder */
    bload(دننه);
    إذا (skip > دننه->کیڼ)
        bail("unexpected end of file on ", دننه->name);
    دننه->کیڼ -= skip;
    دننه->بل += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local فراغ gzhead(bin *دننه)
{
    صحيح flags;

    /* verify gzip magic header and compression method */
    إذا (bget(دننه) != 0x1f || bget(دننه) != 0x8b || bget(دننه) != 8)
        bail(دننه->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(دننه);
    إذا ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", دننه->name);

    /* skip modification time, extra flags, and os */
    bskip(دننه, 6);

    /* skip extra field if present */
    إذا (flags & 4) {
        بینښه len;

        len = bget(دننه);
        len += (بینښه)(bget(دننه)) << 8;
        bskip(دننه, len);
    }

    /* skip file name if present */
    إذا (flags & 8)
        طالما (bget(دننه) != 0)
            ;

    /* skip comment if present */
    إذا (flags & 16)
        طالما (bget(دننه) != 0)
            ;

    /* skip header crc if present */
    إذا (flags & 2)
        bskip(دننه, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local فراغ put4(بینښه اوږد val, FILE *بهر)
{
    putc(val & 0xff, بهر);
    putc((val >> 8) & 0xff, بهر);
    putc((val >> 16) & 0xff, بهر);
    putc((val >> 24) & 0xff, بهر);
}

/* Load up zlib stream from buffered input, bail if end of file */
local فراغ zpull(z_streamp strm, bin *دننه)
{
    إذا (دننه->کیڼ == 0)
        bload(دننه);
    إذا (دننه->کیڼ == 0)
        bail("unexpected end of file on ", دننه->name);
    strm->avail_دننه = دننه->کیڼ;
    strm->بل_دننه = دننه->بل;
}

/* Write header for gzip file to out and initialize trailer. */
local فراغ gzinit(بینښه اوږد *دوراني_ازموینه, بینښه اوږد *tot, FILE *بهر)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, بهر);
    *دوراني_ازموینه = دوراني_ازموینه_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local فراغ gzcopy(حرف *name, صحيح clr, بینښه اوږد *دوراني_ازموینه, بینښه اوږد *tot,
                  FILE *بهر)
{
    صحيح ret;                /* return value from zlib functions */
    صحيح pos;                /* where the "last block" bit is in byte */
    صحيح وروستی;               /* true if processing the last block */
    bin *دننه;                /* buffered input file */
    بینښه حرف *پیل;   /* start of compressed data in buffer */
    بینښه حرف *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_بهیر strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    دننه = bopen(name);
    إذا (دننه == عدم)
        bail("could not open ", name);
    gzhead(دننه);

    /* allocate buffer for uncompressed data and initialize raw inflate
بهیر*/
    junk = حافظه_ورکړه(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    ret = inflateInit2(&strm, -15);
    إذا (junk == عدم || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, دننه);
    پیل = دننه->بل;
    وروستی = پیل[0] & 1;
    إذا (وروستی && clr)
        پیل[0] &= ~1;
    strm.avail_بهر = 0;
    لكل (;;) {
        /* if input used and output done, write used input and get more */
        إذا (strm.avail_دننه == 0 && strm.avail_بهر != 0) {
            fwrite(پیل, 1, strm.بل_دننه - پیل, بهر);
            پیل = دننه->buf;
            دننه->کیڼ = 0;
            zpull(&strm, دننه);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_بهر = CHUNK;
        strm.بل_بهر = junk;
        ret = inflate(&strm, Z_بلاک);
        بدل (ret) {
        حالة Z_MEM_ERROR:
            bail("out of memory", "");
        حالة Z_معلومات_ERROR:
            bail("invalid compressed data in ", دننه->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_بهر;

        /* check for block boundary (only get this when block copied out) */
        إذا (strm.معلومات_type & 128) {
            /* if that was the last block, then done */
            إذا (وروستی)
                اكسر;

            /* number of unused bits in last byte */
            pos = strm.معلومات_type & 7;

            /* find the next last-block bit */
            إذا (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                وروستی = strm.بل_دننه[-1] & pos;
                إذا (وروستی && clr)
                    دننه->buf[strm.بل_دننه - دننه->buf - 1] &= ~pos;
            }
            وإلا {
                /* next last-block bit is in next unused byte */
                إذا (strm.avail_دننه == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(پیل, 1, strm.بل_دننه - پیل, بهر);
                    پیل = دننه->buf;
                    دننه->کیڼ = 0;
                    zpull(&strm, دننه);
                }
                وروستی = strm.بل_دننه[0] & 1;
                إذا (وروستی && clr)
                    دننه->buf[strm.بل_دننه - دننه->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    دننه->کیڼ = strm.avail_دننه;
    دننه->بل = دننه->buf + (strm.بل_دننه - دننه->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.معلومات_type & 7;
    fwrite(پیل, 1, دننه->بل - پیل - 1, بهر);
    وروستی = دننه->بل[-1];
    إذا (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(وروستی, بهر);
    وإلا {
        /* append empty blocks to last byte */
        وروستی &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        إذا (pos & 1) {
            /* odd -- append an empty stored block */
            putc(وروستی, بهر);
            إذا (pos == 1)
                putc(0, بهر);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, بهر);
        }
        وإلا {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            بدل (pos) {
            حالة 6:
                putc(وروستی | 8, بهر);
                وروستی = 0;
            حالة 4:
                putc(وروستی | 0x20, بهر);
                وروستی = 0;
            حالة 2:
                putc(وروستی | 0x80, بهر);
                putc(0, بهر);
            }
        }
    }

    /* update crc and tot */
    *دوراني_ازموینه = دوراني_ازموینه_32_combine(*دوراني_ازموینه, bget4(دننه), len);
    *tot += (بینښه اوږد)len;

    /* clean up */
    inflate_پای_ټکی(&strm);
    حرر(junk);
    bclose(دننه);

    /* write trailer if this is the last gzip file */
    إذا (!clr) {
        put4(*دوراني_ازموینه, بهر);
        put4(*tot, بهر);
    }
}

/* join the gzip files on the command line, write result to stdout */
صحيح اصلي(صحيح argc, حرف **argv)
{
    بینښه اوږد دوراني_ازموینه, tot;     /* running crc and total uncompressed length */

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
    gzinit(&دوراني_ازموینه, &tot, stdout);
    طالما (argc--)
        gzcopy(*argv++, argc, &دوراني_ازموینه, &tot, stdout);

    /*بشپړ*/
    أرجع 0;
}
