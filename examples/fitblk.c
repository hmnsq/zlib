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

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <assert.h>
#ضمن "zlib.h"

#تعریف_کن local ساكن

/* print nastygram and leave */
local فراغ quit(حرف *why)
{
    fprintf(stderr, "fitblk abort: %s\n", why);
    خروج(1);
}

#تعریف_کن RAWLEN 4096    /* intermediate uncompressed buffer size */

/* compress from file to def until provided buffer is full or end of
   input reached; return last deflate() return value, or Z_ERRNO if
   there was read error on the file */
local صحيح partcompress(FILE *درون, z_streamp def)
{
    صحيح ret, flush;
    بیعلامت حرف raw[RAWLEN];

    flush = Z_NO_FLUSH;
    افعل {
        def->avail_درون = fread(raw, 1, RAWLEN, درون);
        إذا (ferror(درون))
            أرجع Z_ERRNO;
        def->بعدی_درون = raw;
        إذا (feof(درون))
            flush = Z_پایان;
        ret = deflate(def, flush);
        أكد(ret != Z_جریان_ERROR);
    } طالما (def->avail_بیرون != 0 && flush == Z_NO_FLUSH);
    أرجع ret;
}

/* recompress from inf's input to def's output; the input for inf and
   the output for def are set in those structures before calling;
   return last deflate() return value, or Z_MEM_ERROR if inflate()
   was not able to allocate enough memory when it needed to */
local صحيح recompress(z_streamp inf, z_streamp def)
{
    صحيح ret, flush;
    بیعلامت حرف raw[RAWLEN];

    flush = Z_NO_FLUSH;
    افعل {
        /* decompress */
        inf->avail_بیرون = RAWLEN;
        inf->بعدی_بیرون = raw;
        ret = inflate(inf, Z_NO_FLUSH);
        أكد(ret != Z_جریان_ERROR && ret != Z_داده_ERROR &&
               ret != Z_NEED_DICT);
        إذا (ret == Z_MEM_ERROR)
            أرجع ret;

        /* compress what was decompressed until done or no room */
        def->avail_درون = RAWLEN - inf->avail_بیرون;
        def->بعدی_درون = raw;
        إذا (inf->avail_بیرون != 0)
            flush = Z_پایان;
        ret = deflate(def, flush);
        أكد(ret != Z_جریان_ERROR);
    } طالما (ret != Z_جریان_انتها && def->avail_بیرون != 0);
    أرجع ret;
}

#تعریف_کن EXCESS 256      /* empirically determined stream overage */
#تعریف_کن MARGIN 8        /* amount to back off for completion */

/* compress from stdin to fixed-size block on stdout */
صحيح اصلی(صحيح argc, حرف **argv)
{
    صحيح ret;                /* return code */
    بیعلامت اندازه;          /* requested fixed output block size */
    بیعلامت have;          /* bytes written by deflate() call */
    بیعلامت حرف *blk;     /* intermediate and final stream */
    بیعلامت حرف *tmp;     /* close to desired size stream */
    z_جریان def, inf;      /* zlib deflate and inflate states */

    /* get requested output size */
    إذا (argc != 2)
        quit("need one argument: size of output block");
    ret = strtol(argv[1], argv + 1, 10);
    إذا (argv[1][0] != 0)
        quit("argument must be a number");
    إذا (ret < 8)            /* 8 is minimum zlib stream size */
        quit("need positive size of 8 or greater");
    اندازه = (بیعلامت)ret;

    /* allocate memory for buffers and compression engine */
    blk = حافظه_بده(اندازه + EXCESS);
    def.zalloc = Z_NULL;
    def.zfree = Z_NULL;
    def.opaque = Z_NULL;
    ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
    إذا (ret != Z_OK || blk == عدم)
        quit("out of memory");

    /* compress from stdin until output full, or no more input */
    def.avail_بیرون = اندازه + EXCESS;
    def.بعدی_بیرون = blk;
    ret = partcompress(stdin, &def);
    إذا (ret == Z_ERRNO)
        quit("error reading input");

    /* if it all fit, then size was undersubscribed -- done! */
    إذا (ret == Z_جریان_انتها && def.avail_بیرون >= EXCESS) {
        /* write block to stdout */
        have = اندازه + EXCESS - def.avail_بیرون;
        إذا (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
            quit("error writing output");

        /* clean up and print results to stderr */
        ret = deflate_انتها(&def);
        أكد(ret != Z_جریان_ERROR);
        حرر(blk);
        fprintf(stderr,
                "%u bytes unused out of %u requested (all input)\n",
                اندازه - have, اندازه);
        أرجع 0;
    }

    /* it didn't all fit -- set up for recompression */
    inf.zalloc = Z_NULL;
    inf.zfree = Z_NULL;
    inf.opaque = Z_NULL;
    inf.avail_درون = 0;
    inf.بعدی_درون = Z_NULL;
    ret = inflateInit(&inf);
    tmp = حافظه_بده(اندازه + EXCESS);
    إذا (ret != Z_OK || tmp == عدم)
        quit("out of memory");
    ret = deflateReset(&def);
    أكد(ret != Z_جریان_ERROR);

    /* do first recompression close to the right amount */
    inf.avail_درون = اندازه + EXCESS;
    inf.بعدی_درون = blk;
    def.avail_بیرون = اندازه + EXCESS;
    def.بعدی_بیرون = tmp;
    ret = recompress(&inf, &def);
    إذا (ret == Z_MEM_ERROR)
        quit("out of memory");

    /* set up for next recompression */
    ret = inflateReset(&inf);
    أكد(ret != Z_جریان_ERROR);
    ret = deflateReset(&def);
    أكد(ret != Z_جریان_ERROR);

    /* do second and final recompression (third compression) */
    inf.avail_درون = اندازه - MARGIN;   /* assure stream will complete */
    inf.بعدی_درون = tmp;
    def.avail_بیرون = اندازه;
    def.بعدی_بیرون = blk;
    ret = recompress(&inf, &def);
    إذا (ret == Z_MEM_ERROR)
        quit("out of memory");
    أكد(ret == Z_جریان_انتها);    /* otherwise MARGIN too small */

    /* done -- write block to stdout */
    have = اندازه - def.avail_بیرون;
    إذا (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
        quit("error writing output");

    /* clean up and print results to stderr */
    حرر(tmp);
    ret = inflate_انتها(&inf);
    أكد(ret != Z_جریان_ERROR);
    ret = deflate_انتها(&def);
    أكد(ret != Z_جریان_ERROR);
    حرر(blk);
    fprintf(stderr,
            "%u bytes unused out of %u requested (%lu input)\n",
            اندازه - have, اندازه, def.مجموع_درون);
    أرجع 0;
}
