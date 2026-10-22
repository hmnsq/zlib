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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ local ਸਥਿਰ

/* print nastygram and leave */
local ਖਾਲੀ quit(ਅੱਖਰ *why)
{
    fprintf(stderr, "fitblk abort: %s\n", why);
    ਬਾਹਰ_ਨਿਕਲੋ(1);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RAWLEN 4096    /* intermediate uncompressed buffer size */

/* compress from file to def until provided buffer is full or end of
   input reached; return last deflate() return value, or Z_ERRNO if
   there was read error on the file */
local ਪੂਰਨ partcompress(FILE *ਵਿੱਚ, z_streamp def)
{
    ਪੂਰਨ ret, flush;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ raw[RAWLEN];

    flush = Z_NO_FLUSH;
    ਕਰੋ {
        def->avail_ਵਿੱਚ = fread(raw, 1, RAWLEN, ਵਿੱਚ);
        ਜੇ (ferror(ਵਿੱਚ))
            ਵਾਪਸ Z_ERRNO;
        def->ਅਗਲਾ_ਵਿੱਚ = raw;
        ਜੇ (feof(ਵਿੱਚ))
            flush = Z_ਅੰਤ;
        ret = deflate(def, flush);
        ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);
    } ਜਦੋਂ (def->avail_ਬਾਹਰਲਾ != 0 && flush == Z_NO_FLUSH);
    ਵਾਪਸ ret;
}

/* recompress from inf's input to def's output; the input for inf and
   the output for def are set in those structures before calling;
   return last deflate() return value, or Z_MEM_ERROR if inflate()
   was not able to allocate enough memory when it needed to */
local ਪੂਰਨ recompress(z_streamp inf, z_streamp def)
{
    ਪੂਰਨ ret, flush;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ raw[RAWLEN];

    flush = Z_NO_FLUSH;
    ਕਰੋ {
        /* decompress */
        inf->avail_ਬਾਹਰਲਾ = RAWLEN;
        inf->ਅਗਲਾ_ਬਾਹਰਲਾ = raw;
        ret = inflate(inf, Z_NO_FLUSH);
        ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR && ret != Z_ਅੰਕੜੇ_ERROR &&
               ret != Z_NEED_DICT);
        ਜੇ (ret == Z_MEM_ERROR)
            ਵਾਪਸ ret;

        /* compress what was decompressed until done or no room */
        def->avail_ਵਿੱਚ = RAWLEN - inf->avail_ਬਾਹਰਲਾ;
        def->ਅਗਲਾ_ਵਿੱਚ = raw;
        ਜੇ (inf->avail_ਬਾਹਰਲਾ != 0)
            flush = Z_ਅੰਤ;
        ret = deflate(def, flush);
        ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);
    } ਜਦੋਂ (ret != Z_ਧਾਰਾ_ਸਿਰਾ && def->avail_ਬਾਹਰਲਾ != 0);
    ਵਾਪਸ ret;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXCESS 256      /* empirically determined stream overage */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MARGIN 8        /* amount to back off for completion */

/* compress from stdin to fixed-size block on stdout */
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
    ਪੂਰਨ ret;                /* return code */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ;          /* requested fixed output block size */
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;          /* bytes written by deflate() call */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *blk;     /* intermediate and final stream */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *tmp;     /* close to desired size stream */
    z_ਧਾਰਾ def, inf;      /* zlib deflate and inflate states */

    /* get requested output size */
    ਜੇ (argc != 2)
        quit("need one argument: size of output block");
    ret = strtol(argv[1], argv + 1, 10);
    ਜੇ (argv[1][0] != 0)
        quit("argument must be a number");
    ਜੇ (ret < 8)            /* 8 is minimum zlib stream size */
        quit("need positive size of 8 or greater");
    ਮਾਪ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ret;

    /* allocate memory for buffers and compression engine */
    blk = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ + EXCESS);
    def.zalloc = Z_NULL;
    def.zfree = Z_NULL;
    def.opaque = Z_NULL;
    ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
    ਜੇ (ret != Z_OK || blk == ਨਿਰਾ)
        quit("out of memory");

    /* compress from stdin until output full, or no more input */
    def.avail_ਬਾਹਰਲਾ = ਮਾਪ + EXCESS;
    def.ਅਗਲਾ_ਬਾਹਰਲਾ = blk;
    ret = partcompress(stdin, &def);
    ਜੇ (ret == Z_ERRNO)
        quit("error reading input");

    /* if it all fit, then size was undersubscribed -- done! */
    ਜੇ (ret == Z_ਧਾਰਾ_ਸਿਰਾ && def.avail_ਬਾਹਰਲਾ >= EXCESS) {
        /* write block to stdout */
        have = ਮਾਪ + EXCESS - def.avail_ਬਾਹਰਲਾ;
        ਜੇ (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
            quit("error writing output");

        /* clean up and print results to stderr */
        ret = deflate_ਸਿਰਾ(&def);
        ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);
        ਮੁਕਤ(blk);
        fprintf(stderr,
                "%u bytes unused out of %u requested (all input)\n",
                ਮਾਪ - have, ਮਾਪ);
        ਵਾਪਸ 0;
    }

    /* it didn't all fit -- set up for recompression */
    inf.zalloc = Z_NULL;
    inf.zfree = Z_NULL;
    inf.opaque = Z_NULL;
    inf.avail_ਵਿੱਚ = 0;
    inf.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit(&inf);
    tmp = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ + EXCESS);
    ਜੇ (ret != Z_OK || tmp == ਨਿਰਾ)
        quit("out of memory");
    ret = deflateReset(&def);
    ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);

    /* do first recompression close to the right amount */
    inf.avail_ਵਿੱਚ = ਮਾਪ + EXCESS;
    inf.ਅਗਲਾ_ਵਿੱਚ = blk;
    def.avail_ਬਾਹਰਲਾ = ਮਾਪ + EXCESS;
    def.ਅਗਲਾ_ਬਾਹਰਲਾ = tmp;
    ret = recompress(&inf, &def);
    ਜੇ (ret == Z_MEM_ERROR)
        quit("out of memory");

    /* set up for next recompression */
    ret = inflateReset(&inf);
    ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);
    ret = deflateReset(&def);
    ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);

    /* do second and final recompression (third compression) */
    inf.avail_ਵਿੱਚ = ਮਾਪ - MARGIN;   /* assure stream will complete */
    inf.ਅਗਲਾ_ਵਿੱਚ = tmp;
    def.avail_ਬਾਹਰਲਾ = ਮਾਪ;
    def.ਅਗਲਾ_ਬਾਹਰਲਾ = blk;
    ret = recompress(&inf, &def);
    ਜੇ (ret == Z_MEM_ERROR)
        quit("out of memory");
    ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ਸਿਰਾ);    /* otherwise MARGIN too small */

    /* done -- write block to stdout */
    have = ਮਾਪ - def.avail_ਬਾਹਰਲਾ;
    ਜੇ (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
        quit("error writing output");

    /* clean up and print results to stderr */
    ਮੁਕਤ(tmp);
    ret = inflate_ਸਿਰਾ(&inf);
    ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);
    ret = deflate_ਸਿਰਾ(&def);
    ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);
    ਮੁਕਤ(blk);
    fprintf(stderr,
            "%u bytes unused out of %u requested (%lu input)\n",
            ਮਾਪ - have, ਮਾਪ, def.ਕੁੱਲ_ਵਿੱਚ);
    ਵਾਪਸ 0;
}
