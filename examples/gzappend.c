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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

#ವ್ಯಾಖ್ಯಾನಿಸು local ಸ್ಥಿರ
#ವ್ಯಾಖ್ಯಾನಿಸು LGCHUNK 14
#ವ್ಯಾಖ್ಯಾನಿಸು CHUNK (1U << LGCHUNK)
#ವ್ಯಾಖ್ಯಾನಿಸು DSIZE 32768U

/* print an error message and terminate with extreme prejudice */
local ಶೂನ್ಯ bye(ಅಕ್ಷರ *msg1, ಅಕ್ಷರ *msg2)
{
    fprintf(stderr, "gzappend error: %s%s\n", msg1, msg2);
    ನಿರ್ಗಮಿಸು(1);
}

/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
local ಚಿಹ್ನೆರಹಿತ ಮಹತ್ತಮ_ಸಾಮಾನ್ಯ_ಅಪವರ್ತನ(ಚಿಹ್ನೆರಹಿತ a, ಚಿಹ್ನೆರಹಿತ b)
{
    ಚಿಹ್ನೆರಹಿತ c;

    ಆಗ (a && b)
        ಆದರೆ (a > b) {
            c = b;
            ಆಗ (a - c >= c)
                c <<= 1;
            a -= c;
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            c = a;
            ಆಗ (b - c >= c)
                c <<= 1;
            b -= c;
        }
    ಹಿಂತಿರುಗಿ a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
local ಶೂನ್ಯ rotate(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *list, ಚಿಹ್ನೆರಹಿತ len, ಚಿಹ್ನೆರಹಿತ rot)
{
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ tmp;
    ಚಿಹ್ನೆರಹಿತ ಚಕ್ರಗಳು;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಆರಂಭ, *ಕೊನೆಯ, *ಗಮ್ಯ, *from;

    /* normalize rot and handle degenerate cases */
    ಆದರೆ (len < 2) ಹಿಂತಿರುಗಿ;
    ಆದರೆ (rot >= len) rot %= len;
    ಆದರೆ (rot == 0) ಹಿಂತಿರುಗಿ;

    /* pointer to last entry in list */
    ಕೊನೆಯ = list + (len - 1);

    /* do simple left shift by one */
    ಆದರೆ (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *ಕೊನೆಯ = tmp;
        ಹಿಂತಿರುಗಿ;
    }

    /* do simple right shift by one */
    ಆದರೆ (rot == len - 1) {
        tmp = *ಕೊನೆಯ;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        ಹಿಂತಿರುಗಿ;
    }

    /* otherwise do rotate as a set of cycles in place */
    ಚಕ್ರಗಳು = ಮಹತ್ತಮ_ಸಾಮಾನ್ಯ_ಅಪವರ್ತನ(len, rot);             /* number of cycles */
    ಮಾಡು {
        ಆರಂಭ = from = list + ಚಕ್ರಗಳು;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        ಗಾಗಿ (;;) {
            ಗಮ್ಯ = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            ಆದರೆ (from > ಕೊನೆಯ) from -= len;   /* (pointer better not wrap) */
            ಆದರೆ (from == ಆರಂಭ) ನಿಲ್ಲಿಸು;   /* all but one shifted */
            *ಗಮ್ಯ = *from;                /* shift left */
        }
        *ಗಮ್ಯ = tmp;                      /* complete the circle */
    } ಆಗ (--ಚಕ್ರಗಳು);
}

/* structure for gzip file read operations */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    ಪೂರ್ಣಾಂಕ fd;                     /* file descriptor */
    ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;                   /* 1 << size is bytes in buf */
    ಚಿಹ್ನೆರಹಿತ ಎಡ;              /* bytes available at next */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf;         /*ಬಫರ್*/
    z_const ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮುಂದಿನ;    /* next byte in buffer */
    ಅಕ್ಷರ *name;                 /* file name for error messages */
} file;

/* reload buffer */
local ಪೂರ್ಣಾಂಕ readin(file *ಒಳ)
{
    ಪೂರ್ಣಾಂಕ len;

    len = ಪಠ(ಒಳ->fd, ಒಳ->buf, 1 << ಒಳ->ಗಾತ್ರ_2);
    ಆದರೆ (len == -1) bye("error reading ", ಒಳ->name);
    ಒಳ->ಎಡ = (ಚಿಹ್ನೆರಹಿತ)len;
    ಒಳ->ಮುಂದಿನ = ಒಳ->buf;
    ಹಿಂತಿರುಗಿ len;
}

/* read from file in, exit if end-of-file */
local ಪೂರ್ಣಾಂಕ readmore(file *ಒಳ)
{
    ಆದರೆ (readin(ಒಳ) == 0) bye("unexpected end of ", ಒಳ->name);
    ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು read1(ಒಳ) (ಒಳ->ಎಡ == 0 ? readmore(ಒಳ) : 0, \
                   ಒಳ->ಎಡ--, *(ಒಳ->ಮುಂದಿನ)++)

/* skip over n bytes of in */
local ಶೂನ್ಯ skip(file *ಒಳ, ಚಿಹ್ನೆರಹಿತ n)
{
    ಚಿಹ್ನೆರಹಿತ bypass;

    ಆದರೆ (n > ಒಳ->ಎಡ) {
        n -= ಒಳ->ಎಡ;
        bypass = n & ~((1U << ಒಳ->ಗಾತ್ರ_2) - 1);
        ಆದರೆ (bypass) {
            ಆದರೆ (lseek(ಒಳ->fd, (off_t)bypass, SEEK_CUR) == -1)
                bye("seeking ", ಒಳ->name);
            n -= bypass;
        }
        readmore(ಒಳ);
        ಆದರೆ (n > ಒಳ->ಎಡ)
            bye("unexpected end of ", ಒಳ->name);
    }
    ಒಳ->ಎಡ -= n;
    ಒಳ->ಮುಂದಿನ += n;
}

/* read a four-byte unsigned integer, little-endian, from in */
ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ read4(file *ಒಳ)
{
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ val;

    val = read1(ಒಳ);
    val += (ಚಿಹ್ನೆರಹಿತ)read1(ಒಳ) << 8;
    val += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)read1(ಒಳ) << 16;
    val += (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)read1(ಒಳ) << 24;
    ಹಿಂತಿರುಗಿ val;
}

/* skip over gzip header */
local ಶೂನ್ಯ gzheader(file *ಒಳ)
{
    ಪೂರ್ಣಾಂಕ flags;
    ಚಿಹ್ನೆರಹಿತ n;

    ಆದರೆ (read1(ಒಳ) != 31 || read1(ಒಳ) != 139) bye(ಒಳ->name, " not a gzip file");
    ಆದರೆ (read1(ಒಳ) != 8) bye("unknown compression method in", ಒಳ->name);
    flags = read1(ಒಳ);
    ಆದರೆ (flags & 0xe0) bye("unknown header flags set in", ಒಳ->name);
    skip(ಒಳ, 6);
    ಆದರೆ (flags & 4) {
        n = read1(ಒಳ);
        n += (ಚಿಹ್ನೆರಹಿತ)(read1(ಒಳ)) << 8;
        skip(ಒಳ, n);
    }
    ಆದರೆ (flags & 8) ಆಗ (read1(ಒಳ) != 0) ;
    ಆದರೆ (flags & 16) ಆಗ (read1(ಒಳ) != 0) ;
    ಆದರೆ (flags & 2) skip(ಒಳ, 2);
}

/* decompress gzip file "name", return strm with a deflate stream ready to
   continue compression of the data in the gzip file, and return a file
   descriptor pointing to where to write the compressed data -- the deflate
   stream is initialized to compress using level "level" */
local ಪೂರ್ಣಾಂಕ gzscan(ಅಕ್ಷರ *name, z_ಹರಿವು *strm, ಪೂರ್ಣಾಂಕ level)
{
    ಪೂರ್ಣಾಂಕ ret, lastbit, ಎಡ, ತುಂಬಿದ;
    ಚಿಹ್ನೆರಹಿತ have;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, tot;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಕಿಟಕಿ;
    off_t lastoff, ಕೊನೆ;
    file gz;

    /* open gzip file */
    gz.name = name;
    gz.fd = open(name, O_RDWR, 0);
    ಆದರೆ (gz.fd == -1) bye("cannot open ", name);
    gz.buf = ಸ್ಮರಣೆ_ಕೊಡು(CHUNK);
    ಆದರೆ (gz.buf == ಖಾಲಿ) bye("out of memory", "");
    gz.ಗಾತ್ರ_2 = LGCHUNK;
    gz.ಎಡ = 0;

    /* skip gzip header */
    gzheader(&gz);

    /* prepare to decompress */
    ಕಿಟಕಿ = ಸ್ಮರಣೆ_ಕೊಡು(DSIZE);
    ಆದರೆ (ಕಿಟಕಿ == ಖಾಲಿ) bye("out of memory", "");
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    ret = inflateInit2(strm, -15);
    ಆದರೆ (ret != Z_OK) bye("out of memory", " or library mismatch");

    /* decompress the deflate stream, saving append information */
    lastbit = 0;
    lastoff = lseek(gz.fd, 0L, SEEK_CUR) - gz.ಎಡ;
    ಎಡ = 0;
    strm->avail_ಒಳ = gz.ಎಡ;
    strm->ಮುಂದಿನ_ಒಳ = gz.ಮುಂದಿನ;
    ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(0L, Z_NULL, 0);
    have = ತುಂಬಿದ = 0;
    ಮಾಡು {
        /* if needed, get more input */
        ಆದರೆ (strm->avail_ಒಳ == 0) {
            readmore(&gz);
            strm->avail_ಒಳ = gz.ಎಡ;
            strm->ಮುಂದಿನ_ಒಳ = gz.ಮುಂದಿನ;
        }

        /* set up output to next available section of sliding window */
        strm->avail_ಹೊರಹೋಗು = DSIZE - have;
        strm->ಮುಂದಿನ_ಹೊರಹೋಗು = ಕಿಟಕಿ + have;

        /* inflate and check for errors */
        ret = inflate(strm, Z_ಖಂಡ);
        ಆದರೆ (ret == Z_ಹರಿವು_ERROR) bye("internal stream error!", "");
        ಆದರೆ (ret == Z_MEM_ERROR) bye("out of memory", "");
        ಆದರೆ (ret == Z_ದತ್ತಾಂಶ_ERROR)
            bye("invalid compressed data--format violated in", name);

        /* update crc and sliding window pointer */
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, ಕಿಟಕಿ + have, DSIZE - have - strm->avail_ಹೊರಹೋಗು);
        ಆದರೆ (strm->avail_ಹೊರಹೋಗು)
            have = DSIZE - strm->avail_ಹೊರಹೋಗು;
        ಇಲ್ಲದಿದ್ದರೆ {
            have = 0;
            ತುಂಬಿದ = 1;
        }

        /* process end of block */
        ಆದರೆ (strm->ದತ್ತಾಂಶ_type & 128) {
            ಆದರೆ (strm->ದತ್ತಾಂಶ_type & 64)
                ಎಡ = strm->ದತ್ತಾಂಶ_type & 0x1f;
            ಇಲ್ಲದಿದ್ದರೆ {
                lastbit = strm->ದತ್ತಾಂಶ_type & 0x1f;
                lastoff = lseek(gz.fd, 0L, SEEK_CUR) - strm->avail_ಒಳ;
            }
        }
    } ಆಗ (ret != Z_ಹರಿವು_ಕೊನೆ);
    inflate_ಕೊನೆ(strm);
    gz.ಎಡ = strm->avail_ಒಳ;
    gz.ಮುಂದಿನ = strm->ಮುಂದಿನ_ಒಳ;

    /* save the location of the end of the compressed data */
    ಕೊನೆ = lseek(gz.fd, 0L, SEEK_CUR) - gz.ಎಡ;

    /* check gzip trailer and save total for deflate */
    ಆದರೆ (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ != read4(&gz))
        bye("invalid compressed data--crc mismatch in ", name);
    tot = strm->ಒಟ್ಟು_ಹೊರಹೋಗು;
    ಆದರೆ ((tot & 0xffffffffUL) != read4(&gz))
        bye("invalid compressed data--length mismatch in", name);

    /* if not at end of file, warn */
    ಆದರೆ (gz.ಎಡ || readin(&gz))
        fprintf(stderr,
            "gzappend warning: junk at end of gzip file overwritten\n");

    /* clear last block bit */
    lseek(gz.fd, lastoff - (lastbit != 0), SEEK_SET);
    ಆದರೆ (ಪಠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
    *gz.buf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(*gz.buf ^ (1 << ((8 - lastbit) & 7)));
    lseek(gz.fd, -1L, SEEK_CUR);
    ಆದರೆ (ಲಿಖ(gz.fd, gz.buf, 1) != 1) bye("writing after seek to ", name);

    /* if window wrapped, build dictionary from window by rotating */
    ಆದರೆ (ತುಂಬಿದ) {
        rotate(ಕಿಟಕಿ, DSIZE, have);
        have = DSIZE;
    }

    /* set up deflate stream with window, crc, total_in, and leftover bits */
    ret = deflateInit2(strm, level, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    ಆದರೆ (ret != Z_OK) bye("out of memory", "");
    deflate_ಗಣ_Dictionary(strm, ಕಿಟಕಿ, have);
    strm->adler = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
    strm->ಒಟ್ಟು_ಒಳ = tot;
    ಆದರೆ (ಎಡ) {
        lseek(gz.fd, --ಕೊನೆ, SEEK_SET);
        ಆದರೆ (ಪಠ(gz.fd, gz.buf, 1) != 1) bye("reading after seek on ", name);
        deflate_ಅವಿಭಾಜ್ಯ(strm, 8 - ಎಡ, *gz.buf);
    }
    lseek(gz.fd, ಕೊನೆ, SEEK_SET);

    /* clean up and return */
    ಬಿಡುಗಡೆ(ಕಿಟಕಿ);
    ಬಿಡುಗಡೆ(gz.buf);
    ಹಿಂತಿರುಗಿ gz.fd;
}

/* append file "name" to gzip file gd using deflate stream strm -- if last
   is true, then finish off the deflate stream at the end */
local ಶೂನ್ಯ gztack(ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ gd, z_ಹರಿವು *strm, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ)
{
    ಪೂರ್ಣಾಂಕ fd, len, ret;
    ಚಿಹ್ನೆರಹಿತ ಎಡ;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ, *ಹೊರಹೋಗು;

    /* open file to compress and append */
    fd = 0;
    ಆದರೆ (name != ಖಾಲಿ) {
        fd = open(name, O_RDONLY, 0);
        ಆದರೆ (fd == -1)
            fprintf(stderr, "gzappend warning: %s not found, skipping ...\n",
                    name);
    }

    /* allocate buffers */
    ಒಳ = ಸ್ಮರಣೆ_ಕೊಡು(CHUNK);
    ಹೊರಹೋಗು = ಸ್ಮರಣೆ_ಕೊಡು(CHUNK);
    ಆದರೆ (ಒಳ == ಖಾಲಿ || ಹೊರಹೋಗು == ಖಾಲಿ) bye("out of memory", "");

    /* compress input file and append to gzip file */
    ಮಾಡು {
        /* get more input */
        len = ಪಠ(fd, ಒಳ, CHUNK);
        ಆದರೆ (len == -1) {
            fprintf(stderr,
                    "gzappend warning: error reading %s, skipping rest ...\n",
                    name);
            len = 0;
        }
        strm->avail_ಒಳ = (ಚಿಹ್ನೆರಹಿತ)len;
        strm->ಮುಂದಿನ_ಒಳ = ಒಳ;
        ಆದರೆ (len) strm->adler = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(strm->adler, ಒಳ, (ಚಿಹ್ನೆರಹಿತ)len);

        /* compress and write all available output */
        ಮಾಡು {
            strm->avail_ಹೊರಹೋಗು = CHUNK;
            strm->ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
            ret = deflate(strm, ಕೊನೆಯ && len == 0 ? Z_ಮುಕ್ತಾಯ : Z_NO_FLUSH);
            ಎಡ = CHUNK - strm->avail_ಹೊರಹೋಗು;
            ಆಗ (ಎಡ) {
                len = ಲಿಖ(gd, ಹೊರಹೋಗು + CHUNK - strm->avail_ಹೊರಹೋಗು - ಎಡ, ಎಡ);
                ಆದರೆ (len == -1) bye("writing gzip file", "");
                ಎಡ -= (ಚಿಹ್ನೆರಹಿತ)len;
            }
        } ಆಗ (strm->avail_ಹೊರಹೋಗು == 0 && ret != Z_ಹರಿವು_ಕೊನೆ);
    } ಆಗ (len != 0);

    /* write trailer after last entry */
    ಆದರೆ (ಕೊನೆಯ) {
        deflate_ಕೊನೆ(strm);
        ಹೊರಹೋಗು[0] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->adler);
        ಹೊರಹೋಗು[1] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->adler >> 8);
        ಹೊರಹೋಗು[2] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->adler >> 16);
        ಹೊರಹೋಗು[3] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->adler >> 24);
        ಹೊರಹೋಗು[4] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->ಒಟ್ಟು_ಒಳ);
        ಹೊರಹೋಗು[5] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->ಒಟ್ಟು_ಒಳ >> 8);
        ಹೊರಹೋಗು[6] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->ಒಟ್ಟು_ಒಳ >> 16);
        ಹೊರಹೋಗು[7] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(strm->ಒಟ್ಟು_ಒಳ >> 24);
        len = 8;
        ಮಾಡು {
            ret = ಲಿಖ(gd, ಹೊರಹೋಗು + 8 - len, len);
            ಆದರೆ (ret == -1) bye("writing gzip file", "");
            len -= ret;
        } ಆಗ (len);
        close(gd);
    }

    /* clean up and return */
    ಬಿಡುಗಡೆ(ಹೊರಹೋಗು);
    ಬಿಡುಗಡೆ(ಒಳ);
    ಆದರೆ (fd > 0) close(fd);
}

/* process the compression level option if present, scan the gzip file, and
   append the specified files, or append the data from stdin if no other file
   names are provided on the command line -- the gzip file must be writable
   and seekable */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
    ಪೂರ್ಣಾಂಕ gd, level;
    z_ಹರಿವು strm;

    /* ignore command name */
    argc--; argv++;

    /* provide usage if no arguments */
    ಆದರೆ (*argv == ಖಾಲಿ) {
        ಬರೆ(
            "gzappend 1.2 (11 Oct 2012) Copyright (C) 2003, 2012 Mark Adler\n"
               );
        ಬರೆ(
            "usage: gzappend [-level] file.gz [ addthis [ andthis ... ]]\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    /* set compression level */
    level = Z_DEFAULT_COMPRESSION;
    ಆದರೆ (argv[0][0] == '-') {
        ಆದರೆ (argv[0][1] < '0' || argv[0][1] > '9' || argv[0][2] != 0)
            bye("invalid compression level", "");
        level = argv[0][1] - '0';
        ಆದರೆ (*++argv == ಖಾಲಿ) bye("no gzip file name after options", "");
    }

    /* prepare to append to gzip file */
    gd = gzscan(*argv++, &strm, level);

    /* append files on command line, or from stdin if none */
    ಆದರೆ (*argv == ಖಾಲಿ)
        gztack(ಖಾಲಿ, gd, &strm, 1);
    ಇಲ್ಲದಿದ್ದರೆ
        ಮಾಡು {
            gztack(*argv, gd, &strm, argv[1] == ಖಾಲಿ);
        } ಆಗ (*++argv != ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ 0;
}
