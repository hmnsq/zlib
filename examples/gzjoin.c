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

#समावेश <stdio.h>      /* fputs(), fprintf(), fwrite(), putc() */
#समावेश <stdlib.h>     /* exit(), malloc(), free() */
#समावेश <fcntl.h>      /* open() */
#समावेश <unistd.h>     /* close(), read(), lseek() */
#समावेश "zlib.h"
    /* crc32(), crc32_combine(), inflateInit2(), inflate(), inflateEnd() */

#घोषणा local अटल

/* exit with an error (return a value to allow use in an expression) */
local पूर्णांक bail(अक्षर *why1, अक्षर *why2)
{
    ख_लिखो(मानक_त्रुटि, "gzjoin error: %s%s, output incomplete\n", why1, why2);
    निकास(1);
    वापस 0;
}

/* -- simple buffered file input with access to the buffer -- */

#घोषणा CHUNK 32768         /* must be a power of two and fit in unsigned */

/* bin buffered input file type */
प्रकार काष्ठा {
    अक्षर *name;             /* name of file for error messages */
    पूर्णांक fd;                 /* file descriptor */
    अचिन्हित बायाँ;          /* bytes remaining at next */
    अचिन्हित अक्षर *अग्र;    /* next byte to read */
    अचिन्हित अक्षर *buf;     /* allocated buffer of length CHUNK */
} bin;

/* close a buffered file and free allocated memory */
local व्योम bclose(bin *in)
{
    अगर (in != शून्य) {
        अगर (in->fd != -1)
            बंद(in->fd);
        अगर (in->buf != शून्य)
            मुक्त(in->buf);
        मुक्त(in);
    }
}

/* open a buffered file for input, return a pointer to type bin, or NULL on
   failure */
local bin *bopen(अक्षर *name)
{
    bin *in;

    in = दो_स्मृति(माप(bin));
    अगर (in == शून्य)
        वापस शून्य;
    in->buf = दो_स्मृति(CHUNK);
    in->fd = खोलो(name, O_RDONLY, 0);
    अगर (in->buf == शून्य || in->fd == -1) {
        bclose(in);
        वापस शून्य;
    }
    in->बायाँ = 0;
    in->अग्र = in->buf;
    in->name = name;
    वापस in;
}

/* load buffer from file, return -1 on read error, 0 or 1 on success, with
   1 indicating that end-of-file was reached */
local पूर्णांक bload(bin *in)
{
    दीर्घ len;

    अगर (in == शून्य)
        वापस -1;
    अगर (in->बायाँ != 0)
        वापस 0;
    in->अग्र = in->buf;
    करो {
        len = (दीर्घ)पढ़ो(in->fd, in->buf + in->बायाँ, CHUNK - in->बायाँ);
        अगर (len < 0)
            वापस -1;
        in->बायाँ += (अचिन्हित)len;
    } जबतक (len != 0 && in->बायाँ < CHUNK);
    वापस len == 0 ? 1 : 0;
}

/* get a byte from the file, bail if end of file */
#घोषणा bget(in) (in->बायाँ ? 0 : bload(in), \
                  in->बायाँ ? (in->बायाँ--, *(in->अग्र)++) : \
                    bail("unexpected end of file on ", in->name))

/* get a four-byte little-endian unsigned integer from file */
local अचिन्हित दीर्घ bget4(bin *in)
{
    अचिन्हित दीर्घ val;

    val = bget(in);
    val += (अचिन्हित दीर्घ)(bget(in)) << 8;
    val += (अचिन्हित दीर्घ)(bget(in)) << 16;
    val += (अचिन्हित दीर्घ)(bget(in)) << 24;
    वापस val;
}

/* skip bytes in file */
local व्योम bskip(bin *in, अचिन्हित skip)
{
    /* check pointer */
    अगर (in == शून्य)
        वापस;

    /* easy case -- skip bytes in buffer */
    अगर (skip <= in->बायाँ) {
        in->बायाँ -= skip;
        in->अग्र += skip;
        वापस;
    }

    /* skip what's in buffer, discard buffer contents */
    skip -= in->बायाँ;
    in->बायाँ = 0;

    /* seek past multiples of CHUNK bytes */
    अगर (skip > CHUNK) {
        अचिन्हित बायाँ;

        बायाँ = skip & (CHUNK - 1);
        अगर (बायाँ == 0) {
            /* exact number of chunks: seek all the way minus one byte to check
               for end-of-file with a read */
            lseek(in->fd, skip - 1, प्रस्तुत_से);
            अगर (पढ़ो(in->fd, in->buf, 1) != 1)
                bail("unexpected end of file on ", in->name);
            वापस;
        }

        /* skip the integral chunks, update skip with remainder */
        lseek(in->fd, skip - बायाँ, प्रस्तुत_से);
        skip = बायाँ;
    }

    /* read more input and skip remainder */
    bload(in);
    अगर (skip > in->बायाँ)
        bail("unexpected end of file on ", in->name);
    in->बायाँ -= skip;
    in->अग्र += skip;
}

/* -- end of buffered input functions -- */

/* skip the gzip header from file in */
local व्योम gzhead(bin *in)
{
    पूर्णांक flags;

    /* verify gzip magic header and compression method */
    अगर (bget(in) != 0x1f || bget(in) != 0x8b || bget(in) != 8)
        bail(in->name, " is not a valid gzip file");

    /* get and verify flags */
    flags = bget(in);
    अगर ((flags & 0xe0) != 0)
        bail("unknown reserved bits set in ", in->name);

    /* skip modification time, extra flags, and os */
    bskip(in, 6);

    /* skip extra field if present */
    अगर (flags & 4) {
        अचिन्हित len;

        len = bget(in);
        len += (अचिन्हित)(bget(in)) << 8;
        bskip(in, len);
    }

    /* skip file name if present */
    अगर (flags & 8)
        जबतक (bget(in) != 0)
            ;

    /* skip comment if present */
    अगर (flags & 16)
        जबतक (bget(in) != 0)
            ;

    /* skip header crc if present */
    अगर (flags & 2)
        bskip(in, 2);
}

/* write a four-byte little-endian unsigned integer to out */
local व्योम put4(अचिन्हित दीर्घ val, खाता *out)
{
    अ_दो(val & 0xff, out);
    अ_दो((val >> 8) & 0xff, out);
    अ_दो((val >> 16) & 0xff, out);
    अ_दो((val >> 24) & 0xff, out);
}

/* Load up zlib stream from buffered input, bail if end of file */
local व्योम zpull(z_streamp strm, bin *in)
{
    अगर (in->बायाँ == 0)
        bload(in);
    अगर (in->बायाँ == 0)
        bail("unexpected end of file on ", in->name);
    strm->avail_in = in->बायाँ;
    strm->अग्र_in = in->अग्र;
}

/* Write header for gzip file to out and initialize trailer. */
local व्योम gzinit(अचिन्हित दीर्घ *crc, अचिन्हित दीर्घ *tot, खाता *out)
{
    ख_डालो("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, out);
    *crc = crc32(0L, Z_NULL, 0);
    *tot = 0;
}

/* Copy the compressed data from name, zeroing the last block bit of the last
   block if clr is true, and adding empty blocks as needed to get to a byte
   boundary.  If clr is false, then the last block becomes the last block of
   the output, and the gzip trailer is written.  crc and tot maintains the
   crc and length (modulo 2^32) of the output for the trailer.  The resulting
   gzip file is written to out.  gzinit() must be called before the first call
   of gzcopy() to write the gzip header and to initialize crc and tot. */
local व्योम gzcopy(अक्षर *name, पूर्णांक clr, अचिन्हित दीर्घ *crc, अचिन्हित दीर्घ *tot,
                  खाता *out)
{
    पूर्णांक ret;                /* return value from zlib functions */
    पूर्णांक pos;                /* where the "last block" bit is in byte */
    पूर्णांक last;               /* true if processing the last block */
    bin *in;                /* buffered input file */
    अचिन्हित अक्षर *start;   /* start of compressed data in buffer */
    अचिन्हित अक्षर *junk;    /* buffer for uncompressed data -- discarded */
    z_off_t len;            /* length of uncompressed data (support > 4 GB) */
    z_stream strm;          /* zlib inflate stream */

    /* open gzip file and skip header */
    in = bopen(name);
    अगर (in == शून्य)
        bail("could not open ", name);
    gzhead(in);

    /* allocate buffer for uncompressed data and initialize raw inflate
       stream */
    junk = दो_स्मृति(CHUNK);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.अग्र_in = Z_NULL;
    ret = inflateInit2(&strm, -15);
    अगर (junk == शून्य || ret != Z_OK)
        bail("out of memory", "");

    /* inflate and copy compressed data, clear last-block bit if requested */
    len = 0;
    zpull(&strm, in);
    start = in->अग्र;
    last = start[0] & 1;
    अगर (last && clr)
        start[0] &= ~1;
    strm.avail_out = 0;
    क्रम (;;) {
        /* if input used and output done, write used input and get more */
        अगर (strm.avail_in == 0 && strm.avail_out != 0) {
            ख_डालो(start, 1, strm.अग्र_in - start, out);
            start = in->buf;
            in->बायाँ = 0;
            zpull(&strm, in);
        }

        /* decompress -- return early when end-of-block reached */
        strm.avail_out = CHUNK;
        strm.अग्र_out = junk;
        ret = inflate(&strm, Z_BLOCK);
        चयन (ret) {
        हाल Z_MEM_ERROR:
            bail("out of memory", "");
        हाल Z_तथ्य_ERROR:
            bail("invalid compressed data in ", in->name);
        }

        /* update length of uncompressed data */
        len += CHUNK - strm.avail_out;

        /* check for block boundary (only get this when block copied out) */
        अगर (strm.तथ्य_type & 128) {
            /* if that was the last block, then done */
            अगर (last)
                अवरोध;

            /* number of unused bits in last byte */
            pos = strm.तथ्य_type & 7;

            /* find the next last-block bit */
            अगर (pos != 0) {
                /* next last-block bit is in last used byte */
                pos = 0x100 >> pos;
                last = strm.अग्र_in[-1] & pos;
                अगर (last && clr)
                    in->buf[strm.अग्र_in - in->buf - 1] &= ~pos;
            }
            अन्यथा {
                /* next last-block bit is in next unused byte */
                अगर (strm.avail_in == 0) {
                    /* don't have that byte yet -- get it */
                    ख_डालो(start, 1, strm.अग्र_in - start, out);
                    start = in->buf;
                    in->बायाँ = 0;
                    zpull(&strm, in);
                }
                last = strm.अग्र_in[0] & 1;
                अगर (last && clr)
                    in->buf[strm.अग्र_in - in->buf] &= ~1;
            }
        }
    }

    /* update buffer with unused input */
    in->बायाँ = strm.avail_in;
    in->अग्र = in->buf + (strm.अग्र_in - in->buf);

    /* copy used input, write empty blocks to get to byte boundary */
    pos = strm.तथ्य_type & 7;
    ख_डालो(start, 1, in->अग्र - start - 1, out);
    last = in->अग्र[-1];
    अगर (pos == 0 || !clr)
        /* already at byte boundary, or last file: write last byte */
        अ_दो(last, out);
    अन्यथा {
        /* append empty blocks to last byte */
        last &= ((0x100 >> pos) - 1);       /* assure unused bits are zero */
        अगर (pos & 1) {
            /* odd -- append an empty stored block */
            अ_दो(last, out);
            अगर (pos == 1)
                अ_दो(0, out);               /* two more bits in block header */
            ख_डालो("\0\0\xff\xff", 1, 4, out);
        }
        अन्यथा {
            /* even -- append 1, 2, or 3 empty fixed blocks */
            चयन (pos) {
            हाल 6:
                अ_दो(last | 8, out);
                last = 0;
            हाल 4:
                अ_दो(last | 0x20, out);
                last = 0;
            हाल 2:
                अ_दो(last | 0x80, out);
                अ_दो(0, out);
            }
        }
    }

    /* update crc and tot */
    *crc = crc32_combine(*crc, bget4(in), len);
    *tot += (अचिन्हित दीर्घ)len;

    /* clean up */
    inflateEnd(&strm);
    मुक्त(junk);
    bclose(in);

    /* write trailer if this is the last gzip file */
    अगर (!clr) {
        put4(*crc, out);
        put4(*tot, out);
    }
}

/* join the gzip files on the command line, write result to stdout */
पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
    अचिन्हित दीर्घ crc, tot;     /* running crc and total uncompressed length */

    /* skip command name */
    argc--;
    argv++;

    /* show usage if no arguments */
    अगर (argc == 0) {
        ख_माला_दो("gzjoin usage: gzjoin f1.gz [f2.gz [f3.gz ...]] > fjoin.gz\n",
              मानक_त्रुटि);
        वापस 0;
    }

    /* join gzip files on command line and write to stdout */
    gzinit(&crc, &tot, मानक_निकास);
    जबतक (argc--)
        gzcopy(*argv++, argc, &crc, &tot, मानक_निकास);

    /* done */
    वापस 0;
}
