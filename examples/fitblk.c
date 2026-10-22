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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ "zlib.h"

#నిర్వచించు local స్థిర

/* print nastygram and leave */
local శూన్య quit(అక్షరం *why)
{
    fprintf(stderr, "fitblk abort: %s\n", why);
    నిష్క్రమించు(1);
}

#నిర్వచించు RAWLEN 4096    /* intermediate uncompressed buffer size */

/* compress from file to def until provided buffer is full or end of
   input reached; return last deflate() return value, or Z_ERRNO if
   there was read error on the file */
local పూర్ణాంకం partcompress(FILE *లోపల, z_streamp def)
{
    పూర్ణాంకం ret, flush;
    గుర్తులేని అక్షరం raw[RAWLEN];

    flush = Z_NO_FLUSH;
    చేయి {
        def->avail_లోపల = fread(raw, 1, RAWLEN, లోపల);
        ఒకవేళ (ferror(లోపల))
            తిరిగి Z_ERRNO;
        def->తదుపరి_లోపల = raw;
        ఒకవేళ (feof(లోపల))
            flush = Z_ముగింపు;
        ret = deflate(def, flush);
        ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);
    } అయితే (def->avail_బహిర్గతం != 0 && flush == Z_NO_FLUSH);
    తిరిగి ret;
}

/* recompress from inf's input to def's output; the input for inf and
   the output for def are set in those structures before calling;
   return last deflate() return value, or Z_MEM_ERROR if inflate()
   was not able to allocate enough memory when it needed to */
local పూర్ణాంకం recompress(z_streamp inf, z_streamp def)
{
    పూర్ణాంకం ret, flush;
    గుర్తులేని అక్షరం raw[RAWLEN];

    flush = Z_NO_FLUSH;
    చేయి {
        /* decompress */
        inf->avail_బహిర్గతం = RAWLEN;
        inf->తదుపరి_బహిర్గతం = raw;
        ret = inflate(inf, Z_NO_FLUSH);
        ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR && ret != Z_దత్తాంశం_ERROR &&
               ret != Z_NEED_DICT);
        ఒకవేళ (ret == Z_MEM_ERROR)
            తిరిగి ret;

        /* compress what was decompressed until done or no room */
        def->avail_లోపల = RAWLEN - inf->avail_బహిర్గతం;
        def->తదుపరి_లోపల = raw;
        ఒకవేళ (inf->avail_బహిర్గతం != 0)
            flush = Z_ముగింపు;
        ret = deflate(def, flush);
        ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);
    } అయితే (ret != Z_ప్రవాహం_చివర && def->avail_బహిర్గతం != 0);
    తిరిగి ret;
}

#నిర్వచించు EXCESS 256      /* empirically determined stream overage */
#నిర్వచించు MARGIN 8        /* amount to back off for completion */

/* compress from stdin to fixed-size block on stdout */
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
    పూర్ణాంకం ret;                /* return code */
    గుర్తులేని కొలత;          /* requested fixed output block size */
    గుర్తులేని have;          /* bytes written by deflate() call */
    గుర్తులేని అక్షరం *blk;     /* intermediate and final stream */
    గుర్తులేని అక్షరం *tmp;     /* close to desired size stream */
    z_ప్రవాహం def, inf;      /* zlib deflate and inflate states */

    /* get requested output size */
    ఒకవేళ (argc != 2)
        quit("need one argument: size of output block");
    ret = strtol(argv[1], argv + 1, 10);
    ఒకవేళ (argv[1][0] != 0)
        quit("argument must be a number");
    ఒకవేళ (ret < 8)            /* 8 is minimum zlib stream size */
        quit("need positive size of 8 or greater");
    కొలత = (గుర్తులేని)ret;

    /* allocate memory for buffers and compression engine */
    blk = జ్ఞాపకం_ఇవ్వు(కొలత + EXCESS);
    def.zalloc = Z_NULL;
    def.zfree = Z_NULL;
    def.opaque = Z_NULL;
    ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
    ఒకవేళ (ret != Z_OK || blk == ఖాళీ)
        quit("out of memory");

    /* compress from stdin until output full, or no more input */
    def.avail_బహిర్గతం = కొలత + EXCESS;
    def.తదుపరి_బహిర్గతం = blk;
    ret = partcompress(stdin, &def);
    ఒకవేళ (ret == Z_ERRNO)
        quit("error reading input");

    /* if it all fit, then size was undersubscribed -- done! */
    ఒకవేళ (ret == Z_ప్రవాహం_చివర && def.avail_బహిర్గతం >= EXCESS) {
        /* write block to stdout */
        have = కొలత + EXCESS - def.avail_బహిర్గతం;
        ఒకవేళ (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
            quit("error writing output");

        /* clean up and print results to stderr */
        ret = deflate_చివర(&def);
        ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);
        విడుదల(blk);
        fprintf(stderr,
                "%u bytes unused out of %u requested (all input)\n",
                కొలత - have, కొలత);
        తిరిగి 0;
    }

    /* it didn't all fit -- set up for recompression */
    inf.zalloc = Z_NULL;
    inf.zfree = Z_NULL;
    inf.opaque = Z_NULL;
    inf.avail_లోపల = 0;
    inf.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit(&inf);
    tmp = జ్ఞాపకం_ఇవ్వు(కొలత + EXCESS);
    ఒకవేళ (ret != Z_OK || tmp == ఖాళీ)
        quit("out of memory");
    ret = deflateReset(&def);
    ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);

    /* do first recompression close to the right amount */
    inf.avail_లోపల = కొలత + EXCESS;
    inf.తదుపరి_లోపల = blk;
    def.avail_బహిర్గతం = కొలత + EXCESS;
    def.తదుపరి_బహిర్గతం = tmp;
    ret = recompress(&inf, &def);
    ఒకవేళ (ret == Z_MEM_ERROR)
        quit("out of memory");

    /* set up for next recompression */
    ret = inflateReset(&inf);
    ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);
    ret = deflateReset(&def);
    ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);

    /* do second and final recompression (third compression) */
    inf.avail_లోపల = కొలత - MARGIN;   /* assure stream will complete */
    inf.తదుపరి_లోపల = tmp;
    def.avail_బహిర్గతం = కొలత;
    def.తదుపరి_బహిర్గతం = blk;
    ret = recompress(&inf, &def);
    ఒకవేళ (ret == Z_MEM_ERROR)
        quit("out of memory");
    ప్రతిజ్ఞా(ret == Z_ప్రవాహం_చివర);    /* otherwise MARGIN too small */

    /* done -- write block to stdout */
    have = కొలత - def.avail_బహిర్గతం;
    ఒకవేళ (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
        quit("error writing output");

    /* clean up and print results to stderr */
    విడుదల(tmp);
    ret = inflate_చివర(&inf);
    ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);
    ret = deflate_చివర(&def);
    ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);
    విడుదల(blk);
    fprintf(stderr,
            "%u bytes unused out of %u requested (%lu input)\n",
            కొలత - have, కొలత, def.మొత్తం_లోపల);
    తిరిగి 0;
}
