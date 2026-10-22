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

#अन्तर्भाव <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#अन्तर्भाव <stdlib.h>     /* exit(), malloc(), free() */
#अन्तर्भाव <fcntl.h>      /* open() */
#अन्तर्भाव <unistd.h>     /* close(), read(), lseek() */
#अन्तर्भाव "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#परिभाषय local स्थिर

/* exit with an error (return a value to allow use in an expression) */
local पूर्णाङ्क bail(वर्ण *why1, वर्ण *why2)
{
    fprintf(stderr, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    निर्गच्छ(1);
    प्रत्यागम 0;
}

/* -- simple buffered file input with access to the buffer -- */

#परिभाषय CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
प्रकारनिर्देश संरचना {
    वर्ण *name;             /* name of file for error messages */
    पूर्णाङ्क fd;                 /* file descriptor */
    अचिह्नित वामः;          /* bytes remaining at next */
    अचिह्नित वर्ण *अग्र;    /* next byte to read */
    अचिह्नित वर्ण *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local शून्य bclose(bin *अन्तर्)
{
    यदि (अन्तर् != रिक्त) {
        यदि (अन्तर्->fd != -1)
            close(अन्तर्->fd);
        यदि (अन्तर्->buf != रिक्त)
            मुक्त(अन्तर्->buf);
        मुक्त(अन्तर्);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(वर्ण *name)
{
    bin *अन्तर्;

    अन्तर् = स्मृतिं_देहि(परिमाण(bin));
    यदि (अन्तर् == रिक्त)
        प्रत्यागम रिक्त;
    अन्तर्->buf = स्मृतिं_देहि(CHUNK);
    अन्तर्->fd = उद्घाटय(name, O_RDONLY, 0);
    यदि (अन्तर्->buf == रिक्त || अन्तर्->fd == -1) {
        bclose(अन्तर्);
        प्रत्यागम रिक्त;
    }
    अन्तर्->वामः = 0;
    अन्तर्->अग्र = अन्तर्->buf;
    अन्तर्->name = name;
    प्रत्यागम अन्तर्;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local पूर्णाङ्क bload(bin *अन्तर्)
{
    दीर्घ len;

    यदि (अन्तर् == रिक्त)
        प्रत्यागम -1;
    यदि (अन्तर्->वामः != 0)
        प्रत्यागम 0;
    अन्तर्->अग्र = अन्तर्->buf;
    कुरु {
        len = (दीर्घ)पठ(अन्तर्->fd, अन्तर्->buf + अन्तर्->वामः, CHUNK - अन्तर्->वामः);
        यदि (len < 0)
            प्रत्यागम -1;
        अन्तर्->वामः += (अचिह्नित)len;
    } यावत् (len != 0 && अन्तर्->वामः < CHUNK);
    प्रत्यागम len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#परिभाषय bget(अन्तर्) (अन्तर्->वामः ? 0 : bload(अन्तर्), \
                  अन्तर्->वामः ? (अन्तर्->वामः--, *(अन्तर्->अग्र)++) : \
                    bail("unexpected end of file on ", अन्तर्->name))

/* get a four-byte little-endian unsigned integer from file */
local अचिह्नित दीर्घ bget4(bin *अन्तर्)
{
    अचिह्नित दीर्घ val;

    val = bget(अन्तर्);
    val += (अचिह्नित दीर्घ)(bget(अन्तर्)) << 8;
    val += (अचिह्नित दीर्घ)(bget(अन्तर्)) << 16;
    val += (अचिह्नित दीर्घ)(bget(अन्तर्)) << 24;
    प्रत्यागम val;
}

/* skip bytes in file */
local शून्य bskip(bin *अन्तर्, अचिह्नित skip)
{
    /* check pointer */
    यदि (अन्तर् == रिक्त)
        प्रत्यागम;

    /* easy case -- skip bytes in buffer */
    यदि (skip <= अन्तर्->वामः) {
        अन्तर्->वामः -= skip;
        अन्तर्->अग्र += skip;
        प्रत्यागम;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= अन्तर्->वामः;
    अन्तर्->वामः = 0;

    /* seek past multiples of CHUNK bytes */
    यदि (skip > CHUNK) {
        अचिह्नित वामः;

        वामः = skip & (CHUNK - 1);
        यदि (वामः == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(अन्तर्->fd, skip - 1, SEEK_CUR);
            यदि (पठ(अन्तर्->fd, अन्तर्->buf, 1) != 1)
                bail("unexpected end of file on ", अन्तर्->name);
            प्रत्यागम;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(अन्तर्->fd, skip - वामः, SEEK_CUR);
        skip = वामः;
    }

    /* read more input and skip remainder */
    bload(अन्तर्);
    यदि (skip > अन्तर्->वामः)
        bail("unexpected end of file on ", अन्तर्->name);
    अन्तर्->वामः -= skip;
    अन्तर्->अग्र += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local शून्य gzhead(bin *अन्तर्)
{
    पूर्णाङ्क flags;

    /* verify gzip magic header and compression method */
    यदि (bget(अन्तर्) != 0x1f || bget(अन्तर्) != 0x8b || bget(अन्तर्) != 8)
        bail(अन्तर्->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(अन्तर्);
    यदि ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", अन्तर्->name);

    /* skip modification time, extra flags, and os */
    bskip(अन्तर्, 6);

    /* skip extra field if present */
    यदि (flags & 4) {
        अचिह्नित len;

        len = bget(अन्तर्);
        len += (अचिह्नित)(bget(अन्तर्)) << 8;
        bskip(अन्तर्, len);
    }

    /* skip file name if present */
    यदि (flags & 8)
        यावत् (bget(अन्तर्) != 0)
            ;

    /* skip comment if present */
    यदि (flags & 16)
        यावत् (bget(अन्तर्) != 0)
            ;

    /* skip header crc if present */
    यदि (flags & 2)
        bskip(अन्तर्, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local शून्य put4(अचिह्नित दीर्घ val, FILE *बहिर्गमनम्)
{
    putc(val & 0xff, बहिर्गमनम्);
    putc((val >> 8) & 0xff, बहिर्गमनम्);
    putc((val >> 16) & 0xff, बहिर्गमनम्);
    putc((val >> 24) & 0xff, बहिर्गमनम्);
}

/* Load up zlib stream from buffered input, bail if end of file */
local शून्य zpull(z_streamp strm, bin *अन्तर्)
{
    यदि (अन्तर्->वामः == 0)
        bload(अन्तर्);
    यदि (अन्तर्->वामः == 0)
        bail("unexpected end of file on ", अन्तर्->name);
    strm->avail_अन्तर् = अन्तर्->वामः;
    strm->अग्र_अन्तर् = अन्तर्->अग्र;
}

/* Write header for gzip file to out and initialize trailer. */
local शून्य gzinit(अचिह्नित दीर्घ *crc, अचिह्नित दीर्घ *tot, FILE *बहिर्गमनम्)
{
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, बहिर्गमनम्);
    *crc = चक्रीयपरीक्षा_32_2(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local शून्य gzcopy(वर्ण *name, पूर्णाङ्क clr, अचिह्नित दीर्घ *crc, अचिह्नित दीर्घ *tot,
                  FILE *बहिर्गमनम्)
{
    पूर्णाङ्क ret;                /* return value from zlib functions */
    पूर्णाङ्क pos;                /* where the "last block" bit is in byte */
    पूर्णाङ्क चरमः;               /* true if processing the last block */
    bin *अन्तर्;                /* buffered input file */
    अचिह्नित वर्ण *start;   /* start of compressed data in buffer */
    अचिह्नित वर्ण *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_प्रवाहः strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    अन्तर् = bopen(name);
    यदि (अन्तर् == रिक्त)
        bail("could not open ", name);
    gzhead(अन्तर्);

    /* allocate buffer for uncompressed data and initialize raw inflate
प्रवाहः*/
    junk = स्मृतिं_देहि(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit2(&strm, -15);
    यदि (junk == रिक्त || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, अन्तर्);
    start = अन्तर्->अग्र;
    चरमः = start[0] & 1;
    यदि (चरमः && clr)
        start[0] &= ~1;
    strm.avail_बहिर्गमनम् = 0;
    प्रति (;;) {
        /* if input used and output done, write used input and get more */
        यदि (strm.avail_अन्तर् == 0 && strm.avail_बहिर्गमनम् != 0) {
            fwrite(start, 1, strm.अग्र_अन्तर् - start, बहिर्गमनम्);
            start = अन्तर्->buf;
            अन्तर्->वामः = 0;
            zpull(&strm, अन्तर्);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_बहिर्गमनम् = CHUNK;
        strm.अग्र_बहिर्गमनम् = junk;
        ret = inflate(&strm, Z_खण्डम्);
        विकल्प (ret) {
        पक्ष Z_MEM_ERROR:
            bail("out of memory", "");
        पक्ष Z_दत्त_ERROR:
            bail("invalid compressed data in ", अन्तर्->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_बहिर्गमनम्;

        /* check for block boundary (only get this when block copied out) */
        यदि (strm.दत्त_type & 128) {
            /* if that was the last block, then done */
            यदि (चरमः)
                भङ्ग;

            /* number of unused bits in last byte */
            pos = strm.दत्त_type & 7;

            /* find the next last-block bit */
            यदि (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                चरमः = strm.अग्र_अन्तर्[-1] & pos;
                यदि (चरमः && clr)
                    अन्तर्->buf[strm.अग्र_अन्तर् - अन्तर्->buf - 1] &= ~pos;
            }
            अन्यथा {
                /* next last-block bit is in next unused byte */
                यदि (strm.avail_अन्तर् == 0) {
                    /* don't have that byte yet -- get it */
                    fwrite(start, 1, strm.अग्र_अन्तर् - start, बहिर्गमनम्);
                    start = अन्तर्->buf;
                    अन्तर्->वामः = 0;
                    zpull(&strm, अन्तर्);
                }
                चरमः = strm.अग्र_अन्तर्[0] & 1;
                यदि (चरमः && clr)
                    अन्तर्->buf[strm.अग्र_अन्तर् - अन्तर्->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    अन्तर्->वामः = strm.avail_अन्तर्;
    अन्तर्->अग्र = अन्तर्->buf + (strm.अग्र_अन्तर् - अन्तर्->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.दत्त_type & 7;
    fwrite(start, 1, अन्तर्->अग्र - start - 1, बहिर्गमनम्);
    चरमः = अन्तर्->अग्र[-1];
    यदि (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        putc(चरमः, बहिर्गमनम्);
    अन्यथा {
        /* append empty blocks to last byte */
        चरमः &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        यदि (pos & 1) {
            /* odd -- append an empty stored block */
            putc(चरमः, बहिर्गमनम्);
            यदि (pos == 1)
                putc(0, बहिर्गमनम्);               /* two more bits in block header */
            fwrite("\0\0\xff\xff", 1, 4, बहिर्गमनम्);
        }
        अन्यथा {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            विकल्प (pos) {
            पक्ष 6:
                putc(चरमः | 8, बहिर्गमनम्);
                चरमः = 0;
            पक्ष 4:
                putc(चरमः | 0x20, बहिर्गमनम्);
                चरमः = 0;
            पक्ष 2:
                putc(चरमः | 0x80, बहिर्गमनम्);
                putc(0, बहिर्गमनम्);
            }
        }
    }

    /* update crc and tot */
    *crc = चक्रीयपरीक्षा_32_combine(*crc, bget4(अन्तर्), len);
    *tot += (अचिह्नित दीर्घ)len;

    /* clean up */
    inflate_अन्तः(&strm);
    मुक्त(junk);
    bclose(अन्तर्);

    /* write trailer if this is the last gzip file */
    यदि (!clr) {
        put4(*crc, बहिर्गमनम्);
        put4(*tot, बहिर्गमनम्);
    }
}

/* join the gzip files on the command line, write result to stdout */
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
    अचिह्नित दीर्घ crc, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    यदि (argc == 0) {
        fputs("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              stderr);
        प्रत्यागम 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&crc, &tot, stdout);
    यावत् (argc--)
        gzcopy(*argv++, argc, &crc, &tot, stdout);

    /*समाप्तम्*/
    प्रत्यागम 0;
}
