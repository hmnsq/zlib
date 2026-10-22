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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>     /* exit(), malloc(), free() */
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>      /* open() */
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>     /* close(), read(), lseek() */
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ local ਸਥਿਰ

/* exit with an error (return a value to allow use in an expression) */
local ਪੂਰਨ bail(ਅੱਖਰ *why1, ਅੱਖਰ *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    ਬਾਹਰ_ਨਿਕਲੋ(1);
    ਵਾਪਸ 0;
}

/* -- simple buffered file input with access to the buffer -- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    ਅੱਖਰ *name;             /* name of file for error messages */
    ਪੂਰਨ fd;                 /* file descriptor */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ;          /* bytes remaining at next */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅਗਲਾ;    /* next byte to read */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local ਖਾਲੀ bclose(bin *ਵਿੱਚ)
{
    ਜੇ (ਵਿੱਚ != ਨਿਰਾ) {
        ਜੇ (ਵਿੱਚ->fd != -1)
            close(ਵਿੱਚ->fd);
        ਜੇ (ਵਿੱਚ->buf != ਨਿਰਾ)
            ਮੁਕਤ(ਵਿੱਚ->buf);
        ਮੁਕਤ(ਵਿੱਚ);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(ਅੱਖਰ *name)
{
    bin *ਵਿੱਚ;

    ਵਿੱਚ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(bin));
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਿੱਚ->buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(CHUNK);
    ਵਿੱਚ->fd = open(name, O_RDONLY, 0);
    ਜੇ (ਵਿੱਚ->buf == ਨਿਰਾ || ਵਿੱਚ->fd == -1) {
        bclose(ਵਿੱਚ);
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਵਿੱਚ->ਖੱਬਾ = 0;
    ਵਿੱਚ->ਅਗਲਾ = ਵਿੱਚ->buf;
    ਵਿੱਚ->name = name;
    ਵਾਪਸ ਵਿੱਚ;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local ਪੂਰਨ bload(bin *ਵਿੱਚ)
{
    ਲੰਮਾ len;

    ਜੇ (ਵਿੱਚ == ਨਿਰਾ)
        ਵਾਪਸ -1;
    ਜੇ (ਵਿੱਚ->ਖੱਬਾ != 0)
        ਵਾਪਸ 0;
    ਵਿੱਚ->ਅਗਲਾ = ਵਿੱਚ->buf;
    ਕਰੋ {
        len = (ਲੰਮਾ)ਪਠ(ਵਿੱਚ->fd, ਵਿੱਚ->buf + ਵਿੱਚ->ਖੱਬਾ, CHUNK - ਵਿੱਚ->ਖੱਬਾ);
        ਜੇ (len < 0)
            ਵਾਪਸ -1;
        ਵਿੱਚ->ਖੱਬਾ += (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
    } ਜਦੋਂ (len != 0 && ਵਿੱਚ->ਖੱਬਾ < CHUNK);
    ਵਾਪਸ len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bget(ਵਿੱਚ) (ਵਿੱਚ->ਖੱਬਾ ? 0 : bload(ਵਿੱਚ), \
                  ਵਿੱਚ->ਖੱਬਾ ? (ਵਿੱਚ->ਖੱਬਾ--, *(ਵਿੱਚ->ਅਗਲਾ)++) : \
                    bail("unexpected end of file on ", ਵਿੱਚ->name))

/* get a four-byte little-endian unsigned integer from file */
local ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ bget4(bin *ਵਿੱਚ)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ val;

    val = bget(ਵਿੱਚ);
    val += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(bget(ਵਿੱਚ)) << 8;
    val += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(bget(ਵਿੱਚ)) << 16;
    val += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(bget(ਵਿੱਚ)) << 24;
    ਵਾਪਸ val;
}

/* skip bytes in file */
local ਖਾਲੀ bskip(bin *ਵਿੱਚ, ਬਿਨਾਨਿਸ਼ਾਨੀ skip)
{
    /* check pointer */
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ)
        ਵਾਪਸ;

    /* easy case -- skip bytes in buffer */
    ਜੇ (skip <= ਵਿੱਚ->ਖੱਬਾ) {
        ਵਿੱਚ->ਖੱਬਾ -= skip;
        ਵਿੱਚ->ਅਗਲਾ += skip;
        ਵਾਪਸ;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= ਵਿੱਚ->ਖੱਬਾ;
    ਵਿੱਚ->ਖੱਬਾ = 0;

    /* seek past multiples of CHUNK bytes */
    ਜੇ (skip > CHUNK) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ;

        ਖੱਬਾ = skip & (CHUNK - 1);
        ਜੇ (ਖੱਬਾ == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(ਵਿੱਚ->fd, skip - 1, SEEK_CUR);
            ਜੇ (ਪਠ(ਵਿੱਚ->fd, ਵਿੱਚ->buf, 1) != 1)
                bail("unexpected end of file on ", ਵਿੱਚ->name);
            ਵਾਪਸ;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(ਵਿੱਚ->fd, skip - ਖੱਬਾ, SEEK_CUR);
        skip = ਖੱਬਾ;
    }

    /* read more input and skip remainder */
    bload(ਵਿੱਚ);
    ਜੇ (skip > ਵਿੱਚ->ਖੱਬਾ)
        bail("unexpected end of file on ", ਵਿੱਚ->name);
    ਵਿੱਚ->ਖੱਬਾ -= skip;
    ਵਿੱਚ->ਅਗਲਾ += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local ਖਾਲੀ gzhead(bin *ਵਿੱਚ)
{
    ਪੂਰਨ flags;

    /* verify gzip magic header and compression method */
    ਜੇ (bget(ਵਿੱਚ) != 0x1f || bget(ਵਿੱਚ) != 0x8b || bget(ਵਿੱਚ) != 8)
        bail(ਵਿੱਚ->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(ਵਿੱਚ);
    ਜੇ ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", ਵਿੱਚ->name);

    /* skip modification time, extra flags, and os */
    bskip(ਵਿੱਚ, 6);

    /* skip extra field if present */
    ਜੇ (flags & 4) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ len;

        len = bget(ਵਿੱਚ);
        len += (ਬਿਨਾਨਿਸ਼ਾਨੀ)(bget(ਵਿੱਚ)) << 8;
        bskip(ਵਿੱਚ, len);
    }

    /* skip file name if present */
    ਜੇ (flags & 8)
        ਜਦੋਂ (bget(ਵਿੱਚ) != 0)
            ;

    /* skip comment if present */
    ਜੇ (flags & 16)
        ਜਦੋਂ (bget(ਵਿੱਚ) != 0)
            ;

    /* skip header crc if present */
    ਜੇ (flags & 2)
        bskip(ਵਿੱਚ, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local ਖਾਲੀ put4(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ val, FILE *ਬਾਹਰਲਾ)
{
    putc(val & 0xff, ਬਾਹਰਲਾ);
    putc((val >> 8) & 0xff, ਬਾਹਰਲਾ);
    putc((val >> 16) & 0xff, ਬਾਹਰਲਾ);
    putc((val >> 24) & 0xff, ਬਾਹਰਲਾ);
}

/* Load up zlib stream from buffered input, bail if end of file */
local ਖਾਲੀ zpull(z_streamp strm, bin *ਵਿੱਚ)
{
    ਜੇ (ਵਿੱਚ->ਖੱਬਾ == 0)
        bload(ਵਿੱਚ);
    ਜੇ (ਵਿੱਚ->ਖੱਬਾ == 0)
        bail("unexpected end of file on ", ਵਿੱਚ->name);
    strm->avail_ਵਿੱਚ = ਵਿੱਚ->ਖੱਬਾ;
    strm->ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ->ਅਗਲਾ;
}

/* Write header for gzip file to out and initialize trailer. */
local ਖਾਲੀ gzinit(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਚੱਕਰੀ_ਜਾਂਚ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *tot, FILE *ਬਾਹਰਲਾ)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, ਬਾਹਰਲਾ);
    *ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local ਖਾਲੀ gzcopy(ਅੱਖਰ *name, ਪੂਰਨ clr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਚੱਕਰੀ_ਜਾਂਚ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *tot,
                  FILE *ਬਾਹਰਲਾ)
{
    ਪੂਰਨ ret;                /* return value from zlib functions */
    ਪੂਰਨ pos;                /* where the "last block" bit is in byte */
    ਪੂਰਨ ਆਖਰੀ;               /* true if processing the last block */
    bin *ਵਿੱਚ;                /* buffered input file */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਸ਼ੁਰੂ;   /* start of compressed data in buffer */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_ਧਾਰਾ strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    ਵਿੱਚ = bopen(name);
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ)
        bail("could not open ", name);
    gzhead(ਵਿੱਚ);

    /* allocate buffer for uncompressed data and initialize raw inflate
ਧਾਰਾ*/
    junk = ਸਮ੍ਰਿਤੀ_ਦਿਓ(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit2(&strm, -15);
    ਜੇ (junk == ਨਿਰਾ || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, ਵਿੱਚ);
    ਸ਼ੁਰੂ = ਵਿੱਚ->ਅਗਲਾ;
    ਆਖਰੀ = ਸ਼ੁਰੂ[0] & 1;
    ਜੇ (ਆਖਰੀ && clr)
        ਸ਼ੁਰੂ[0] &= ~1;
    strm.avail_ਬਾਹਰਲਾ = 0;
    ਲਈ (;;) {
        /* if input used and output done, write used input and get more */
        ਜੇ (strm.avail_ਵਿੱਚ == 0 && strm.avail_ਬਾਹਰਲਾ != 0) {
            fwrite(ਸ਼ੁਰੂ, 1, strm.ਅਗਲਾ_ਵਿੱਚ - ਸ਼ੁਰੂ, ਬਾਹਰਲਾ);
            ਸ਼ੁਰੂ = ਵਿੱਚ->buf;
            ਵਿੱਚ->ਖੱਬਾ = 0;
            zpull(&strm, ਵਿੱਚ);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_ਬਾਹਰਲਾ = CHUNK;
        strm.ਅਗਲਾ_ਬਾਹਰਲਾ = junk;
        ret = inflate(&strm, Z_ਖੰਡ);
        ਚੋਣ (ret) {
        ਹਾਲਤ Z_MEM_ERROR:
            bail("out of memory", "");
        ਹਾਲਤ Z_ਅੰਕੜੇ_ERROR:
            bail("invalid compressed data in ", ਵਿੱਚ->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_ਬਾਹਰਲਾ;

        /* check for block boundary (only get this when block copied out) */
        ਜੇ (strm.ਅੰਕੜੇ_type & 128) {
            /* if that was the last block, then done */
            ਜੇ (ਆਖਰੀ)
                ਰੋਕੋ;

            /* number of unused bits in last byte */
            pos = strm.ਅੰਕੜੇ_type & 7;

            /* find the next last-block bit */
            ਜੇ (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                ਆਖਰੀ = strm.ਅਗਲਾ_ਵਿੱਚ[-1] & pos;
                ਜੇ (ਆਖਰੀ && clr)
                    ਵਿੱਚ->buf[strm.ਅਗਲਾ_ਵਿੱਚ - ਵਿੱਚ->buf - 1] &= ~pos;
            }
            ਨਹੀਂਤਾਂ {
                /* next last-block bit is in next unused byte */
                ਜੇ (strm.avail_ਵਿੱਚ == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(ਸ਼ੁਰੂ, 1, strm.ਅਗਲਾ_ਵਿੱਚ - ਸ਼ੁਰੂ, ਬਾਹਰਲਾ);
                    ਸ਼ੁਰੂ = ਵਿੱਚ->buf;
                    ਵਿੱਚ->ਖੱਬਾ = 0;
                    zpull(&strm, ਵਿੱਚ);
                }
                ਆਖਰੀ = strm.ਅਗਲਾ_ਵਿੱਚ[0] & 1;
                ਜੇ (ਆਖਰੀ && clr)
                    ਵਿੱਚ->buf[strm.ਅਗਲਾ_ਵਿੱਚ - ਵਿੱਚ->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    ਵਿੱਚ->ਖੱਬਾ = strm.avail_ਵਿੱਚ;
    ਵਿੱਚ->ਅਗਲਾ = ਵਿੱਚ->buf + (strm.ਅਗਲਾ_ਵਿੱਚ - ਵਿੱਚ->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.ਅੰਕੜੇ_type & 7;
    fwrite(ਸ਼ੁਰੂ, 1, ਵਿੱਚ->ਅਗਲਾ - ਸ਼ੁਰੂ - 1, ਬਾਹਰਲਾ);
    ਆਖਰੀ = ਵਿੱਚ->ਅਗਲਾ[-1];
    ਜੇ (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(ਆਖਰੀ, ਬਾਹਰਲਾ);
    ਨਹੀਂਤਾਂ {
        /* append empty blocks to last byte */
        ਆਖਰੀ &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        ਜੇ (pos & 1) {
            /* odd -- append an empty stored block */
            putc(ਆਖਰੀ, ਬਾਹਰਲਾ);
            ਜੇ (pos == 1)
                putc(0, ਬਾਹਰਲਾ);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, ਬਾਹਰਲਾ);
        }
        ਨਹੀਂਤਾਂ {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            ਚੋਣ (pos) {
            ਹਾਲਤ 6:
                putc(ਆਖਰੀ | 8, ਬਾਹਰਲਾ);
                ਆਖਰੀ = 0;
            ਹਾਲਤ 4:
                putc(ਆਖਰੀ | 0x20, ਬਾਹਰਲਾ);
                ਆਖਰੀ = 0;
            ਹਾਲਤ 2:
                putc(ਆਖਰੀ | 0x80, ਬਾਹਰਲਾ);
                putc(0, ਬਾਹਰਲਾ);
            }
        }
    }

    /* update crc and tot */
    *ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_combine(*ਚੱਕਰੀ_ਜਾਂਚ, bget4(ਵਿੱਚ), len);
    *tot += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)len;

    /* clean up */
    inflate_ਸਿਰਾ(&strm);
    ਮੁਕਤ(junk);
    bclose(ਵਿੱਚ);

    /* write trailer if this is the last gzip file */
    ਜੇ (!clr) {
        put4(*ਚੱਕਰੀ_ਜਾਂਚ, ਬਾਹਰਲਾ);
        put4(*tot, ਬਾਹਰਲਾ);
    }
}

/* join the gzip files on the command line, write result to stdout */
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    ਜੇ (argc == 0) {
        fputs("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              stderr);
        ਵਾਪਸ 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&ਚੱਕਰੀ_ਜਾਂਚ, &tot, stdout);
    ਜਦੋਂ (argc--)
        gzcopy(*argv++, argc, &ਚੱਕਰੀ_ਜਾਂਚ, &tot, stdout);

    /*ਪੂਰਾ*/
    ਵਾਪਸ 0;
}
