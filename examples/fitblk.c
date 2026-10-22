/* fitblk.c: example of fitting compressed output to a specified size
   Not copyrighted -- provided to the public domain
   Version 1.1  25 November 2004  Mark Adler */

/* Version history:
   1.0  24 Nov 2004  First version
   1.1  25 Nov 2004  Change deflateInit2() to deflateInit()
                     Use fixed-size, stack-allocated raw buffers
                     Simplify code moving compression to subroutines
                     Use assert() for internal errors
                     Add detailed description of approach
 */

/* Approach to just fitting a requested compressed size:

   fitblk performs three compression passes on a portion of the input
   data in order to determine how much of that input will compress to
   nearly the requested output block size.  The first pass generates
   enough deflate blocks to produce output to fill the requested
   output size plus a specified excess amount (see the EXCESS define
   below).  The last deflate block may go quite a bit past that, but
   is discarded.  The second pass decompresses and recompresses just
   the compressed data that fit in the requested plus excess sized
   buffer.  The deflate process is terminated after that amount of
   input, which is less than the amount consumed on the first pass.
   The last deflate block of the result will be of a comparable size
   to the final product, so that the header for that deflate block and
   the compression ratio for that block will be about the same as in
   the final product.  The third compression pass decompresses the
   result of the second step, but only the compressed data up to the
   requested size minus an amount to allow the compressed stream to
   complete (see the MARGIN define below).  That will result in a
   final compressed stream whose length is less than or equal to the
   requested size.  Assuming sufficient input and a requested size
   greater than a few hundred bytes, the shortfall will typically be
   less than ten bytes.

   If the input is short enough that the first compression completes
   before filling the requested output size, then that compressed
   stream is return with no recompression.

   EXCESS is chosen to be just greater than the shortfall seen in a
   two pass approach similar to the above.  That shortfall is due to
   the last deflate block compressing more efficiently with a smaller
   header on the second pass.  EXCESS is set to be large enough so
   that there is enough uncompressed data for the second pass to fill
   out the requested size, and small enough so that the final deflate
   block of the second pass will be close in size to the final deflate
   block of the third and final pass.  MARGIN is chosen to be just
   large enough to assure that the final compression has enough room
   to complete in all cases.
 */

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

#ವ್ಯಾಖ್ಯಾನಿಸು local ಸ್ಥಿರ

/* print nastygram and leave */
local ಶೂನ್ಯ quit(ಅಕ್ಷರ *why)
{
    fprintf(stderr, "fitblk abort: %s\n", why);
    ನಿರ್ಗಮಿಸು(1);
}

#ವ್ಯಾಖ್ಯಾನಿಸು RAWLEN 4096    /* intermediate uncompressed buffer size */

/* compress from file to def until provided buffer is full or end of
   input reached; return last deflate() return value, or Z_ERRNO if
   there was read error on the file */
local ಪೂರ್ಣಾಂಕ partcompress(FILE *ಒಳ, z_streamp def)
{
    ಪೂರ್ಣಾಂಕ ret, flush;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ raw[RAWLEN];

    flush = Z_NO_FLUSH;
    ಮಾಡು {
        def->avail_ಒಳ = fread(raw, 1, RAWLEN, ಒಳ);
        ಆದರೆ (ferror(ಒಳ))
            ಹಿಂತಿರುಗಿ Z_ERRNO;
        def->ಮುಂದಿನ_ಒಳ = raw;
        ಆದರೆ (feof(ಒಳ))
            flush = Z_ಮುಕ್ತಾಯ;
        ret = deflate(def, flush);
        ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);
    } ಆಗ (def->avail_ಹೊರಹೋಗು != 0 && flush == Z_NO_FLUSH);
    ಹಿಂತಿರುಗಿ ret;
}

/* recompress from inf's input to def's output; the input for inf and
   the output for def are set in those structures before calling;
   return last deflate() return value, or Z_MEM_ERROR if inflate()
   was not able to allocate enough memory when it needed to */
local ಪೂರ್ಣಾಂಕ recompress(z_streamp inf, z_streamp def)
{
    ಪೂರ್ಣಾಂಕ ret, flush;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ raw[RAWLEN];

    flush = Z_NO_FLUSH;
    ಮಾಡು {
        /* decompress */
        inf->avail_ಹೊರಹೋಗು = RAWLEN;
        inf->ಮುಂದಿನ_ಹೊರಹೋಗು = raw;
        ret = inflate(inf, Z_NO_FLUSH);
        ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR && ret != Z_ದತ್ತಾಂಶ_ERROR &&
               ret != Z_NEED_DICT);
        ಆದರೆ (ret == Z_MEM_ERROR)
            ಹಿಂತಿರುಗಿ ret;

        /* compress what was decompressed until done or no room */
        def->avail_ಒಳ = RAWLEN - inf->avail_ಹೊರಹೋಗು;
        def->ಮುಂದಿನ_ಒಳ = raw;
        ಆದರೆ (inf->avail_ಹೊರಹೋಗು != 0)
            flush = Z_ಮುಕ್ತಾಯ;
        ret = deflate(def, flush);
        ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);
    } ಆಗ (ret != Z_ಹರಿವು_ಕೊನೆ && def->avail_ಹೊರಹೋಗು != 0);
    ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿಸು EXCESS 256      /* empirically determined stream overage */
#ವ್ಯಾಖ್ಯಾನಿಸು MARGIN 8        /* amount to back off for completion */

/* compress from stdin to fixed-size block on stdout */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
    ಪೂರ್ಣಾಂಕ ret;                /* return code */
    ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2;          /* requested fixed output block size */
    ಚಿಹ್ನೆರಹಿತ have;          /* bytes written by deflate() call */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *blk;     /* intermediate and final stream */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *tmp;     /* close to desired size stream */
    z_ಹರಿವು def, inf;      /* zlib deflate and inflate states */

    /* get requested output size */
    ಆದರೆ (argc != 2)
        quit("need one argument: size of output block");
    ret = strtol(argv[1], argv + 1, 10);
    ಆದರೆ (argv[1][0] != 0)
        quit("argument must be a number");
    ಆದರೆ (ret < 8)            /* 8 is minimum zlib stream size */
        quit("need positive size of 8 or greater");
    ಗಾತ್ರ_2 = (ಚಿಹ್ನೆರಹಿತ)ret;

    /* allocate memory for buffers and compression engine */
    blk = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2 + EXCESS);
    def.zalloc = Z_NULL;
    def.zfree = Z_NULL;
    def.opaque = Z_NULL;
    ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
    ಆದರೆ (ret != Z_OK || blk == ಖಾಲಿ)
        quit("out of memory");

    /* compress from stdin until output full, or no more input */
    def.avail_ಹೊರಹೋಗು = ಗಾತ್ರ_2 + EXCESS;
    def.ಮುಂದಿನ_ಹೊರಹೋಗು = blk;
    ret = partcompress(stdin, &def);
    ಆದರೆ (ret == Z_ERRNO)
        quit("error reading input");

    /* if it all fit, then size was undersubscribed -- done! */
    ಆದರೆ (ret == Z_ಹರಿವು_ಕೊನೆ && def.avail_ಹೊರಹೋಗು >= EXCESS) {
        /* write block to stdout */
        have = ಗಾತ್ರ_2 + EXCESS - def.avail_ಹೊರಹೋಗು;
        ಆದರೆ (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
            quit("error writing output");

        /* clean up and print results to stderr */
        ret = deflate_ಕೊನೆ(&def);
        ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);
        ಬಿಡುಗಡೆ(blk);
        fprintf(stderr,
                "%u bytes unused out of %u requested (all input)\n",
                ಗಾತ್ರ_2 - have, ಗಾತ್ರ_2);
        ಹಿಂತಿರುಗಿ 0;
    }

    /* it didn't all fit -- set up for recompression */
    inf.zalloc = Z_NULL;
    inf.zfree = Z_NULL;
    inf.opaque = Z_NULL;
    inf.avail_ಒಳ = 0;
    inf.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit(&inf);
    tmp = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2 + EXCESS);
    ಆದರೆ (ret != Z_OK || tmp == ಖಾಲಿ)
        quit("out of memory");
    ret = deflateReset(&def);
    ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);

    /* do first recompression close to the right amount */
    inf.avail_ಒಳ = ಗಾತ್ರ_2 + EXCESS;
    inf.ಮುಂದಿನ_ಒಳ = blk;
    def.avail_ಹೊರಹೋಗು = ಗಾತ್ರ_2 + EXCESS;
    def.ಮುಂದಿನ_ಹೊರಹೋಗು = tmp;
    ret = recompress(&inf, &def);
    ಆದರೆ (ret == Z_MEM_ERROR)
        quit("out of memory");

    /* set up for next recompression */
    ret = inflateReset(&inf);
    ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);
    ret = deflateReset(&def);
    ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);

    /* do second and final recompression (third compression) */
    inf.avail_ಒಳ = ಗಾತ್ರ_2 - MARGIN;   /* assure stream will complete */
    inf.ಮುಂದಿನ_ಒಳ = tmp;
    def.avail_ಹೊರಹೋಗು = ಗಾತ್ರ_2;
    def.ಮುಂದಿನ_ಹೊರಹೋಗು = blk;
    ret = recompress(&inf, &def);
    ಆದರೆ (ret == Z_MEM_ERROR)
        quit("out of memory");
    ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ಕೊನೆ);    /* otherwise MARGIN too small */

    /* done -- write block to stdout */
    have = ಗಾತ್ರ_2 - def.avail_ಹೊರಹೋಗು;
    ಆದರೆ (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
        quit("error writing output");

    /* clean up and print results to stderr */
    ಬಿಡುಗಡೆ(tmp);
    ret = inflate_ಕೊನೆ(&inf);
    ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);
    ret = deflate_ಕೊನೆ(&def);
    ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);
    ಬಿಡುಗಡೆ(blk);
    fprintf(stderr,
            "%u bytes unused out of %u requested (%lu input)\n",
            ಗಾತ್ರ_2 - have, ಗಾತ್ರ_2, def.ಒಟ್ಟು_ಒಳ);
    ಹಿಂತಿರುಗಿ 0;
}
