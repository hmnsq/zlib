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

#ಅನ್ತರ್ಭಾವ <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#ಅನ್ತರ್ಭಾವ <stdlib.h>     /* exit(), malloc(), free() */
#ಅನ್ತರ್ಭಾವ <fcntl.h>      /* open() */
#ಅನ್ತರ್ಭಾವ <unistd.h>     /* close(), read(), lseek() */
#ಅನ್ತರ್ಭಾವ "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#ವ್ಯಾಖ್ಯಾನಿಸು local ಸ್ಥಿರ

/* exit with an error (return a value to allow use in an expression) */
local ಪೂರ್ಣಾಂಕ bail(ಅಕ್ಷರ *why1, ಅಕ್ಷರ *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    ನಿರ್ಗಮಿಸು(1);
    ಹಿಂತಿರುಗಿ 0;
}

/* -- simple buffered file input with access to the buffer -- */

#ವ್ಯಾಖ್ಯಾನಿಸು CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    ಅಕ್ಷರ *name;             /* name of file for error messages */
    ಪೂರ್ಣಾಂಕ fd;                 /* file descriptor */
    ಚಿಹ್ನೆರಹಿತ ಎಡ;          /* bytes remaining at next */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮುಂದಿನ;    /* next byte to read */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local ಶೂನ್ಯ bclose(bin *ಒಳ)
{
    ಆದರೆ (ಒಳ != ಖಾಲಿ) {
        ಆದರೆ (ಒಳ->fd != -1)
            close(ಒಳ->fd);
        ಆದರೆ (ಒಳ->buf != ಖಾಲಿ)
            ಬಿಡುಗಡೆ(ಒಳ->buf);
        ಬಿಡುಗಡೆ(ಒಳ);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(ಅಕ್ಷರ *name)
{
    bin *ಒಳ;

    ಒಳ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(bin));
    ಆದರೆ (ಒಳ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಒಳ->buf = ಸ್ಮರಣೆ_ಕೊಡು(CHUNK);
    ಒಳ->fd = open(name, O_RDONLY, 0);
    ಆದರೆ (ಒಳ->buf == ಖಾಲಿ || ಒಳ->fd == -1) {
        bclose(ಒಳ);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಒಳ->ಎಡ = 0;
    ಒಳ->ಮುಂದಿನ = ಒಳ->buf;
    ಒಳ->name = name;
    ಹಿಂತಿರುಗಿ ಒಳ;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local ಪೂರ್ಣಾಂಕ bload(bin *ಒಳ)
{
    ದೀರ್ಘ len;

    ಆದರೆ (ಒಳ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (ಒಳ->ಎಡ != 0)
        ಹಿಂತಿರುಗಿ 0;
    ಒಳ->ಮುಂದಿನ = ಒಳ->buf;
    ಮಾಡು {
        len = (ದೀರ್ಘ)ಪಠ(ಒಳ->fd, ಒಳ->buf + ಒಳ->ಎಡ, CHUNK - ಒಳ->ಎಡ);
        ಆದರೆ (len < 0)
            ಹಿಂತಿರುಗಿ -1;
        ಒಳ->ಎಡ += (ಚಿಹ್ನೆರಹಿತ)len;
    } ಆಗ (len != 0 && ಒಳ->ಎಡ < CHUNK);
    ಹಿಂತಿರುಗಿ len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#ವ್ಯಾಖ್ಯಾನಿಸು bget(ಒಳ) (ಒಳ->ಎಡ ? 0 : bload(ಒಳ), \
                  ಒಳ->ಎಡ ? (ಒಳ->ಎಡ--, *(ಒಳ->ಮುಂದಿನ)++) : \
                    bail("unexpected end of file on ", ಒಳ->name))

/* get a four-byte little-endian unsigned integer from file */
local ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ bget4(bin *ಒಳ)
{
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ val;

    val = bget(ಒಳ);
    val += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(bget(ಒಳ)) << 8;
    val += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(bget(ಒಳ)) << 16;
    val += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(bget(ಒಳ)) << 24;
    ಹಿಂತಿರುಗಿ val;
}

/* skip bytes in file */
local ಶೂನ್ಯ bskip(bin *ಒಳ, ಚಿಹ್ನೆರಹಿತ skip)
{
    /* check pointer */
    ಆದರೆ (ಒಳ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ;

    /* easy case -- skip bytes in buffer */
    ಆದರೆ (skip <= ಒಳ->ಎಡ) {
        ಒಳ->ಎಡ -= skip;
        ಒಳ->ಮುಂದಿನ += skip;
        ಹಿಂತಿರುಗಿ;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= ಒಳ->ಎಡ;
    ಒಳ->ಎಡ = 0;

    /* seek past multiples of CHUNK bytes */
    ಆದರೆ (skip > CHUNK) {
        ಚಿಹ್ನೆರಹಿತ ಎಡ;

        ಎಡ = skip & (CHUNK - 1);
        ಆದರೆ (ಎಡ == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(ಒಳ->fd, skip - 1, SEEK_CUR);
            ಆದರೆ (ಪಠ(ಒಳ->fd, ಒಳ->buf, 1) != 1)
                bail("unexpected end of file on ", ಒಳ->name);
            ಹಿಂತಿರುಗಿ;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(ಒಳ->fd, skip - ಎಡ, SEEK_CUR);
        skip = ಎಡ;
    }

    /* read more input and skip remainder */
    bload(ಒಳ);
    ಆದರೆ (skip > ಒಳ->ಎಡ)
        bail("unexpected end of file on ", ಒಳ->name);
    ಒಳ->ಎಡ -= skip;
    ಒಳ->ಮುಂದಿನ += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local ಶೂನ್ಯ gzhead(bin *ಒಳ)
{
    ಪೂರ್ಣಾಂಕ flags;

    /* verify gzip magic header and compression method */
    ಆದರೆ (bget(ಒಳ) != 0x1f || bget(ಒಳ) != 0x8b || bget(ಒಳ) != 8)
        bail(ಒಳ->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(ಒಳ);
    ಆದರೆ ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", ಒಳ->name);

    /* skip modification time, extra flags, and os */
    bskip(ಒಳ, 6);

    /* skip extra field if present */
    ಆದರೆ (flags & 4) {
        ಚಿಹ್ನೆರಹಿತ len;

        len = bget(ಒಳ);
        len += (ಚಿಹ್ನೆರಹಿತ)(bget(ಒಳ)) << 8;
        bskip(ಒಳ, len);
    }

    /* skip file name if present */
    ಆದರೆ (flags & 8)
        ಆಗ (bget(ಒಳ) != 0)
            ;

    /* skip comment if present */
    ಆದರೆ (flags & 16)
        ಆಗ (bget(ಒಳ) != 0)
            ;

    /* skip header crc if present */
    ಆದರೆ (flags & 2)
        bskip(ಒಳ, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local ಶೂನ್ಯ put4(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ val, FILE *ಹೊರಹೋಗು)
{
    putc(val & 0xff, ಹೊರಹೋಗು);
    putc((val >> 8) & 0xff, ಹೊರಹೋಗು);
    putc((val >> 16) & 0xff, ಹೊರಹೋಗು);
    putc((val >> 24) & 0xff, ಹೊರಹೋಗು);
}

/* Load up zlib stream from buffered input, bail if end of file */
local ಶೂನ್ಯ zpull(z_streamp strm, bin *ಒಳ)
{
    ಆದರೆ (ಒಳ->ಎಡ == 0)
        bload(ಒಳ);
    ಆದರೆ (ಒಳ->ಎಡ == 0)
        bail("unexpected end of file on ", ಒಳ->name);
    strm->avail_ಒಳ = ಒಳ->ಎಡ;
    strm->ಮುಂದಿನ_ಒಳ = ಒಳ->ಮುಂದಿನ;
}

/* Write header for gzip file to out and initialize trailer. */
local ಶೂನ್ಯ gzinit(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *tot, FILE *ಹೊರಹೋಗು)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, ಹೊರಹೋಗು);
    *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local ಶೂನ್ಯ gzcopy(ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ clr, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *tot,
                  FILE *ಹೊರಹೋಗು)
{
    ಪೂರ್ಣಾಂಕ ret;                /* return value from zlib functions */
    ಪೂರ್ಣಾಂಕ pos;                /* where the "last block" bit is in byte */
    ಪೂರ್ಣಾಂಕ ಕೊನೆಯ;               /* true if processing the last block */
    bin *ಒಳ;                /* buffered input file */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಆರಂಭ;   /* start of compressed data in buffer */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_ಹರಿವು strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    ಒಳ = bopen(name);
    ಆದರೆ (ಒಳ == ಖಾಲಿ)
        bail("could not open ", name);
    gzhead(ಒಳ);

    /* allocate buffer for uncompressed data and initialize raw inflate
ಹರಿವು*/
    junk = ಸ್ಮರಣೆ_ಕೊಡು(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit2(&strm, -15);
    ಆದರೆ (junk == ಖಾಲಿ || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, ಒಳ);
    ಆರಂಭ = ಒಳ->ಮುಂದಿನ;
    ಕೊನೆಯ = ಆರಂಭ[0] & 1;
    ಆದರೆ (ಕೊನೆಯ && clr)
        ಆರಂಭ[0] &= ~1;
    strm.avail_ಹೊರಹೋಗು = 0;
    ಗಾಗಿ (;;) {
        /* if input used and output done, write used input and get more */
        ಆದರೆ (strm.avail_ಒಳ == 0 && strm.avail_ಹೊರಹೋಗು != 0) {
            fwrite(ಆರಂಭ, 1, strm.ಮುಂದಿನ_ಒಳ - ಆರಂಭ, ಹೊರಹೋಗು);
            ಆರಂಭ = ಒಳ->buf;
            ಒಳ->ಎಡ = 0;
            zpull(&strm, ಒಳ);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_ಹೊರಹೋಗು = CHUNK;
        strm.ಮುಂದಿನ_ಹೊರಹೋಗು = junk;
        ret = inflate(&strm, Z_ಖಂಡ);
        ಆಯ್ಕೆ (ret) {
        ಸ್ಥಿತಿ Z_MEM_ERROR:
            bail("out of memory", "");
        ಸ್ಥಿತಿ Z_ದತ್ತಾಂಶ_ERROR:
            bail("invalid compressed data in ", ಒಳ->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_ಹೊರಹೋಗು;

        /* check for block boundary (only get this when block copied out) */
        ಆದರೆ (strm.ದತ್ತಾಂಶ_type & 128) {
            /* if that was the last block, then done */
            ಆದರೆ (ಕೊನೆಯ)
                ನಿಲ್ಲಿಸು;

            /* number of unused bits in last byte */
            pos = strm.ದತ್ತಾಂಶ_type & 7;

            /* find the next last-block bit */
            ಆದರೆ (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                ಕೊನೆಯ = strm.ಮುಂದಿನ_ಒಳ[-1] & pos;
                ಆದರೆ (ಕೊನೆಯ && clr)
                    ಒಳ->buf[strm.ಮುಂದಿನ_ಒಳ - ಒಳ->buf - 1] &= ~pos;
            }
            ಇಲ್ಲದಿದ್ದರೆ {
                /* next last-block bit is in next unused byte */
                ಆದರೆ (strm.avail_ಒಳ == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(ಆರಂಭ, 1, strm.ಮುಂದಿನ_ಒಳ - ಆರಂಭ, ಹೊರಹೋಗು);
                    ಆರಂಭ = ಒಳ->buf;
                    ಒಳ->ಎಡ = 0;
                    zpull(&strm, ಒಳ);
                }
                ಕೊನೆಯ = strm.ಮುಂದಿನ_ಒಳ[0] & 1;
                ಆದರೆ (ಕೊನೆಯ && clr)
                    ಒಳ->buf[strm.ಮುಂದಿನ_ಒಳ - ಒಳ->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    ಒಳ->ಎಡ = strm.avail_ಒಳ;
    ಒಳ->ಮುಂದಿನ = ಒಳ->buf + (strm.ಮುಂದಿನ_ಒಳ - ಒಳ->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.ದತ್ತಾಂಶ_type & 7;
    fwrite(ಆರಂಭ, 1, ಒಳ->ಮುಂದಿನ - ಆರಂಭ - 1, ಹೊರಹೋಗು);
    ಕೊನೆಯ = ಒಳ->ಮುಂದಿನ[-1];
    ಆದರೆ (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(ಕೊನೆಯ, ಹೊರಹೋಗು);
    ಇಲ್ಲದಿದ್ದರೆ {
        /* append empty blocks to last byte */
        ಕೊನೆಯ &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        ಆದರೆ (pos & 1) {
            /* odd -- append an empty stored block */
            putc(ಕೊನೆಯ, ಹೊರಹೋಗು);
            ಆದರೆ (pos == 1)
                putc(0, ಹೊರಹೋಗು);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, ಹೊರಹೋಗು);
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            ಆಯ್ಕೆ (pos) {
            ಸ್ಥಿತಿ 6:
                putc(ಕೊನೆಯ | 8, ಹೊರಹೋಗು);
                ಕೊನೆಯ = 0;
            ಸ್ಥಿತಿ 4:
                putc(ಕೊನೆಯ | 0x20, ಹೊರಹೋಗು);
                ಕೊನೆಯ = 0;
            ಸ್ಥಿತಿ 2:
                putc(ಕೊನೆಯ | 0x80, ಹೊರಹೋಗು);
                putc(0, ಹೊರಹೋಗು);
            }
        }
    }

    /* update crc and tot */
    *ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine(*ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, bget4(ಒಳ), len);
    *tot += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)len;

    /* clean up */
    inflate_ಕೊನೆ(&strm);
    ಬಿಡುಗಡೆ(junk);
    bclose(ಒಳ);

    /* write trailer if this is the last gzip file */
    ಆದರೆ (!clr) {
        put4(*ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, ಹೊರಹೋಗು);
        put4(*tot, ಹೊರಹೋಗು);
    }
}

/* join the gzip files on the command line, write result to stdout */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    ಆದರೆ (argc == 0) {
        fputs("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              stderr);
        ಹಿಂತಿರುಗಿ 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, &tot, stdout);
    ಆಗ (argc--)
        gzcopy(*argv++, argc, &ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, &tot, stdout);

    /*ಮುಗಿದ*/
    ಹಿಂತಿರುಗಿ 0;
}
