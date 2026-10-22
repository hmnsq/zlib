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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ local ਸਥਿਰ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LGCHUNK 14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK (1U << LGCHUNK)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local ਖਾਲੀ bye(ਅੱਖਰ *msg1, ਅੱਖਰ *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    ਬਾਹਰ_ਨਿਕਲੋ(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਹੱਤਮ_ਸਾਂਝਾ_ਭਾਜਕ(ਬਿਨਾਨਿਸ਼ਾਨੀ a, ਬਿਨਾਨਿਸ਼ਾਨੀ b)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ c;

    ਜਦੋਂ (a && b)
        ਜੇ (a > b) {
            c = b;
            ਜਦੋਂ (a - c >= c)
                c <<= 1;
            a -= c;
        }
        ਨਹੀਂਤਾਂ {
            c = a;
            ਜਦੋਂ (b - c >= c)
                c <<= 1;
            b -= c;
        }
    ਵਾਪਸ a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local ਖਾਲੀ rotate(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *list, ਬਿਨਾਨਿਸ਼ਾਨੀ len, ਬਿਨਾਨਿਸ਼ਾਨੀ rot)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ tmp;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੱਕਰਾਂ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਸ਼ੁਰੂ, *ਆਖਰੀ, *ਮੰਜ਼ਿਲ, *from;

    /* normalize rot and handle degenerate cases */
    ਜੇ (len < 2) ਵਾਪਸ;
    ਜੇ (rot >= len) rot %= len;
    ਜੇ (rot == 0) ਵਾਪਸ;

    /* pointer to last entry in list */
    ਆਖਰੀ = list + (len - 1);

    /* do simple left shift by one */
    ਜੇ (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *ਆਖਰੀ = tmp;
        ਵਾਪਸ;
    }

    /* do simple right shift by one */
    ਜੇ (rot == len - 1) {
        tmp = *ਆਖਰੀ;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        ਵਾਪਸ;
    }

    /* otherwise do rotate as a set of cycles in place */
    ਚੱਕਰਾਂ = ਮਹੱਤਮ_ਸਾਂਝਾ_ਭਾਜਕ(len, rot);             /* number of cycles */
    ਕਰੋ {
        ਸ਼ੁਰੂ = from = list + ਚੱਕਰਾਂ;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        ਲਈ (;;) {
            ਮੰਜ਼ਿਲ = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            ਜੇ (from > ਆਖਰੀ) from -= len;   /* (pointer better not wrap) */
            ਜੇ (from == ਸ਼ੁਰੂ) ਰੋਕੋ;   /* all but one shifted */
            *ਮੰਜ਼ਿਲ = *from;                /* shift left */
        }
        *ਮੰਜ਼ਿਲ = tmp;                      /* complete the circle */
    } ਜਦੋਂ (--ਚੱਕਰਾਂ);
}

/* structure for gzip file read operations */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    ਪੂਰਨ fd;                     /* file descriptor */
    ਪੂਰਨ ਮਾਪ;                   /* 1 << size is bytes in buf */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ;              /* bytes available at next */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf;         /*ਬਫਰ*/
    z_const ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅਗਲਾ;    /* next byte in buffer */
    ਅੱਖਰ *name;                 /* file name for error messages */
} file;

/* reload buffer */
local ਪੂਰਨ readin(file *ਵਿੱਚ)
{
    ਪੂਰਨ len;

    len = ਪਠ(ਵਿੱਚ->fd, ਵਿੱਚ->buf, 1 << ਵਿੱਚ->ਮਾਪ);
    ਜੇ (len == -1) bye("error reading ", ਵਿੱਚ->name);
    ਵਿੱਚ->ਖੱਬਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
    ਵਿੱਚ->ਅਗਲਾ = ਵਿੱਚ->buf;
    ਵਾਪਸ len;
}

/* read from file in, exit if end-of-file */
local ਪੂਰਨ readmore(file *ਵਿੱਚ)
{
    ਜੇ (readin(ਵਿੱਚ) == 0) bye("unexpected end of ", ਵਿੱਚ->name);
    ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ read1(ਵਿੱਚ) (ਵਿੱਚ->ਖੱਬਾ == 0 ? readmore(ਵਿੱਚ) : 0, \
                   ਵਿੱਚ->ਖੱਬਾ--, *(ਵਿੱਚ->ਅਗਲਾ)++)

/* skip over n bytes of in */
local ਖਾਲੀ skip(file *ਵਿੱਚ, ਬਿਨਾਨਿਸ਼ਾਨੀ n)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ bypass;

    ਜੇ (n > ਵਿੱਚ->ਖੱਬਾ) {
        n -= ਵਿੱਚ->ਖੱਬਾ;
        bypass = n & ~((1U << ਵਿੱਚ->ਮਾਪ) - 1);
        ਜੇ (bypass) {
            ਜੇ (lseek(ਵਿੱਚ->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", ਵਿੱਚ->name);
            n -= bypass;
        }
        readmore(ਵਿੱਚ);
        ਜੇ (n > ਵਿੱਚ->ਖੱਬਾ)
            bye("unexpected end of ", ਵਿੱਚ->name);
    }
    ਵਿੱਚ->ਖੱਬਾ -= n;
    ਵਿੱਚ->ਅਗਲਾ += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ read4(file *ਵਿੱਚ)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ val;

    val = read1(ਵਿੱਚ);
    val += (ਬਿਨਾਨਿਸ਼ਾਨੀ)read1(ਵਿੱਚ) << 8;
    val += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)read1(ਵਿੱਚ) << 16;
    val += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)read1(ਵਿੱਚ) << 24;
    ਵਾਪਸ val;
}

/* skip over gzip header */
local ਖਾਲੀ gzheader(file *ਵਿੱਚ)
{
    ਪੂਰਨ flags;
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;

    ਜੇ (read1(ਵਿੱਚ) != 31 || read1(ਵਿੱਚ) != 139) bye(ਵਿੱਚ->name, " not a gzip file");
    ਜੇ (read1(ਵਿੱਚ) != 8) bye("unknown compression method in", ਵਿੱਚ->name);
    flags = read1(ਵਿੱਚ);
    ਜੇ (flags & 0xe0) bye("unknown header flags set in", ਵਿੱਚ->name);
    skip(ਵਿੱਚ, 6);
    ਜੇ (flags & 4) {
        n = read1(ਵਿੱਚ);
        n += (ਬਿਨਾਨਿਸ਼ਾਨੀ)(read1(ਵਿੱਚ)) << 8;
        skip(ਵਿੱਚ, n);
    }
    ਜੇ (flags & 8) ਜਦੋਂ (read1(ਵਿੱਚ) != 0) ;
    ਜੇ (flags & 16) ਜਦੋਂ (read1(ਵਿੱਚ) != 0) ;
    ਜੇ (flags & 2) skip(ਵਿੱਚ, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local ਪੂਰਨ gzscan(ਅੱਖਰ *name, z_ਧਾਰਾ *strm, ਪੂਰਨ level)
{
    ਪੂਰਨ ret, lastbit, ਖੱਬਾ, ਭਰਿਆ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ, tot;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਖਿੜਕੀ;
    off_t lastoff, ਸਿਰਾ;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    ਜੇ (gz.fd == -1) bye("cannot open ", name);
    gz.buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(CHUNK);
    ਜੇ (gz.buf == ਨਿਰਾ) bye("out of memory", "");
    gz.ਮਾਪ = LGCHUNK;
    gz.ਖੱਬਾ = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    ਖਿੜਕੀ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(DSIZE);
    ਜੇ (ਖਿੜਕੀ == ਨਿਰਾ) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    ਜੇ (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.ਖੱਬਾ;
    ਖੱਬਾ = 0;
    strm->avail_ਵਿੱਚ = gz.ਖੱਬਾ;
    strm->ਅਗਲਾ_ਵਿੱਚ = gz.ਅਗਲਾ;
    ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_2(0L, Z_NULL, 0);
    have = ਭਰਿਆ = 0;
    ਕਰੋ {
        /* if needed, get more input */
        ਜੇ (strm->avail_ਵਿੱਚ == 0) {
            readmore(&gz);
            strm->avail_ਵਿੱਚ = gz.ਖੱਬਾ;
            strm->ਅਗਲਾ_ਵਿੱਚ = gz.ਅਗਲਾ;
        }

        /* set up output to next available section of sliding window */
        strm->avail_ਬਾਹਰਲਾ = DSIZE - have;
        strm->ਅਗਲਾ_ਬਾਹਰਲਾ = ਖਿੜਕੀ + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_ਖੰਡ);
        ਜੇ (ret == Z_ਧਾਰਾ_ERROR) bye("internal stream error!", "");
        ਜੇ (ret == Z_MEM_ERROR) bye("out of memory", "");
        ਜੇ (ret == Z_ਅੰਕੜੇ_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_2(ਚੱਕਰੀ_ਜਾਂਚ, ਖਿੜਕੀ + have, DSIZE - have - strm->avail_ਬਾਹਰਲਾ);
        ਜੇ (strm->avail_ਬਾਹਰਲਾ)
            have = DSIZE - strm->avail_ਬਾਹਰਲਾ;
        ਨਹੀਂਤਾਂ {
            have = 0;
            ਭਰਿਆ = 1;
        }

        /* process end of block */
        ਜੇ (strm->ਅੰਕੜੇ_type & 128) {
            ਜੇ (strm->ਅੰਕੜੇ_type & 64)
                ਖੱਬਾ = strm->ਅੰਕੜੇ_type & 0x1f;
            ਨਹੀਂਤਾਂ {
                lastbit = strm->ਅੰਕੜੇ_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_ਵਿੱਚ;
            }
        }
    } ਜਦੋਂ (ret != Z_ਧਾਰਾ_ਸਿਰਾ);
    inflate_ਸਿਰਾ(strm);
    gz.ਖੱਬਾ = strm->avail_ਵਿੱਚ;
    gz.ਅਗਲਾ = strm->ਅਗਲਾ_ਵਿੱਚ;

    /* save the location of the end of the compressed data */
    ਸਿਰਾ = lseek(gz.fd, 0L, SEEK_CUR) - gz.ਖੱਬਾ;

    /* check gzip trailer and save total for deflate */
    ਜੇ (ਚੱਕਰੀ_ਜਾਂਚ != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->ਕੁੱਲ_ਬਾਹਰਲਾ;
    ਜੇ ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    ਜੇ (gz.ਖੱਬਾ || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    ਜੇ (ਪਠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    ਜੇ (ਲਿਖ(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    ਜੇ (ਭਰਿਆ) {
        rotate(ਖਿੜਕੀ, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    ਜੇ (ret != Z_OK) bye("out of memory", "");
    deflate_ਸੈੱਟ_Dictionary(strm, ਖਿੜਕੀ, have);
    strm->adler = ਚੱਕਰੀ_ਜਾਂਚ;
    strm->ਕੁੱਲ_ਵਿੱਚ = tot;
    ਜੇ (ਖੱਬਾ) {
        lseek(gz.fd, --ਸਿਰਾ, SEEK_SET);
        ਜੇ (ਪਠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_ਅਭਾਜ(strm, 8 - ਖੱਬਾ, *gz.buf);
    }
    lseek(gz.fd, ਸਿਰਾ, SEEK_SET);

    /* clean up and return */
    ਮੁਕਤ(ਖਿੜਕੀ);
    ਮੁਕਤ(gz.buf);
    ਵਾਪਸ gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local ਖਾਲੀ gztack(ਅੱਖਰ *name, ਪੂਰਨ gd, z_ਧਾਰਾ *strm, ਪੂਰਨ ਆਖਰੀ)
{
    ਪੂਰਨ fd, len, ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ, *ਬਾਹਰਲਾ;

    /* open file to compress and append */
    fd = 0;
    ਜੇ (name != ਨਿਰਾ) {
        fd = open(name, O_RDONLY, 0);
        ਜੇ (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    ਵਿੱਚ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(CHUNK);
    ਬਾਹਰਲਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(CHUNK);
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ || ਬਾਹਰਲਾ == ਨਿਰਾ) bye("out of memory", "");

    /* compress input file and append to gzip file */
    ਕਰੋ {
        /* get more input */
        len = ਪਠ(fd, ਵਿੱਚ, CHUNK);
        ਜੇ (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_ਵਿੱਚ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
        strm->ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;
        ਜੇ (len) strm->adler = ਚੱਕਰੀ_ਜਾਂਚ_32_2(strm->adler, ਵਿੱਚ, (ਬਿਨਾਨਿਸ਼ਾਨੀ)len);

        /* compress and write all available output */
        ਕਰੋ {
            strm->avail_ਬਾਹਰਲਾ = CHUNK;
            strm->ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
            ret = deflate(strm, ਆਖਰੀ && len == 0 ? Z_ਅੰਤ : Z_NO_FLUSH);
            ਖੱਬਾ = CHUNK - strm->avail_ਬਾਹਰਲਾ;
            ਜਦੋਂ (ਖੱਬਾ) {
                len = ਲਿਖ(gd, ਬਾਹਰਲਾ + CHUNK - strm->avail_ਬਾਹਰਲਾ - ਖੱਬਾ, ਖੱਬਾ);
                ਜੇ (len == -1) bye("writing gzip file", "");
                ਖੱਬਾ -= (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
            }
        } ਜਦੋਂ (strm->avail_ਬਾਹਰਲਾ == 0 && ret != Z_ਧਾਰਾ_ਸਿਰਾ);
    } ਜਦੋਂ (len != 0);

    /* write trailer after last entry */
    ਜੇ (ਆਖਰੀ) {
        deflate_ਸਿਰਾ(strm);
        ਬਾਹਰਲਾ[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->adler);
        ਬਾਹਰਲਾ[1] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->adler >> 8);
        ਬਾਹਰਲਾ[2] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->adler >> 16);
        ਬਾਹਰਲਾ[3] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->adler >> 24);
        ਬਾਹਰਲਾ[4] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->ਕੁੱਲ_ਵਿੱਚ);
        ਬਾਹਰਲਾ[5] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->ਕੁੱਲ_ਵਿੱਚ >> 8);
        ਬਾਹਰਲਾ[6] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->ਕੁੱਲ_ਵਿੱਚ >> 16);
        ਬਾਹਰਲਾ[7] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(strm->ਕੁੱਲ_ਵਿੱਚ >> 24);
        len = 8;
        ਕਰੋ {
            ret = ਲਿਖ(gd, ਬਾਹਰਲਾ + 8 - len, len);
            ਜੇ (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } ਜਦੋਂ (len);
        close(gd);
    }

    /* clean up and return */
    ਮੁਕਤ(ਬਾਹਰਲਾ);
    ਮੁਕਤ(ਵਿੱਚ);
    ਜੇ (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
    ਪੂਰਨ gd, level;
    z_ਧਾਰਾ strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    ਜੇ (*argv == ਨਿਰਾ) {
        ਲਿਖੋ(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        ਲਿਖੋ(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        ਵਾਪਸ 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    ਜੇ (argv[0][0] == '-') {
        ਜੇ (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        ਜੇ (*++argv == ਨਿਰਾ) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    ਜੇ (*argv == ਨਿਰਾ)
        gztack(ਨਿਰਾ, gd, &strm, 1);
    ਨਹੀਂਤਾਂ
        ਕਰੋ {
            gztack(*argv, gd, &strm, argv[1] == ਨਿਰਾ);
        } ਜਦੋਂ (*++argv != ਨਿਰਾ);
    ਵਾਪਸ 0;
}
