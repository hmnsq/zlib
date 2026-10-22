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

#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <assert.h>
#समावेश "zlib.h"

#घोषणा local अटल

/* print nastygram and leave */
local व्योम quit(अक्षर *why)
{
    ख_लिखो(मानक_त्रुटि, "fitblk abort: %s\n", why);
    निकास(1);
}

#घोषणा RAWLEN 4096    /* intermediate uncompressed buffer size */

/* compress from file to def until provided buffer is full or end of
   input reached; return last deflate() return value, or Z_ERRNO if
   there was read error on the file */
local पूर्णांक partcompress(खाता *in, z_streamp def)
{
    पूर्णांक ret, flush;
    अचिन्हित अक्षर raw[RAWLEN];

    flush = Z_NO_FLUSH;
    करो {
        def->avail_in = ख_पढ़ो(raw, 1, RAWLEN, in);
        अगर (ख_त्रुटि(in))
            वापस Z_ERRNO;
        def->अग्र_in = raw;
        अगर (ख_पूर्ण(in))
            flush = Z_FINISH;
        ret = deflate(def, flush);
        निश्चित(ret != Z_STREAM_ERROR);
    } जबतक (def->avail_out != 0 && flush == Z_NO_FLUSH);
    वापस ret;
}

/* recompress from inf's input to def's output; the input for inf and
   the output for def are set in those structures before calling;
   return last deflate() return value, or Z_MEM_ERROR if inflate()
   was not able to allocate enough memory when it needed to */
local पूर्णांक recompress(z_streamp inf, z_streamp def)
{
    पूर्णांक ret, flush;
    अचिन्हित अक्षर raw[RAWLEN];

    flush = Z_NO_FLUSH;
    करो {
        /* decompress */
        inf->avail_out = RAWLEN;
        inf->अग्र_out = raw;
        ret = inflate(inf, Z_NO_FLUSH);
        निश्चित(ret != Z_STREAM_ERROR && ret != Z_तथ्य_ERROR &&
               ret != Z_NEED_DICT);
        अगर (ret == Z_MEM_ERROR)
            वापस ret;

        /* compress what was decompressed until done or no room */
        def->avail_in = RAWLEN - inf->avail_out;
        def->अग्र_in = raw;
        अगर (inf->avail_out != 0)
            flush = Z_FINISH;
        ret = deflate(def, flush);
        निश्चित(ret != Z_STREAM_ERROR);
    } जबतक (ret != Z_STREAM_END && def->avail_out != 0);
    वापस ret;
}

#घोषणा EXCESS 256      /* empirically determined stream overage */
#घोषणा MARGIN 8        /* amount to back off for completion */

/* compress from stdin to fixed-size block on stdout */
पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
    पूर्णांक ret;                /* return code */
    अचिन्हित आकार;          /* requested fixed output block size */
    अचिन्हित have;          /* bytes written by deflate() call */
    अचिन्हित अक्षर *blk;     /* intermediate and final stream */
    अचिन्हित अक्षर *tmp;     /* close to desired size stream */
    z_stream def, inf;      /* zlib deflate and inflate states */

    /* get requested output size */
    अगर (argc != 2)
        quit("need one argument: size of output block");
    ret = म_से_दीर्घ(argv[1], argv + 1, 10);
    अगर (argv[1][0] != 0)
        quit("argument must be a number");
    अगर (ret < 8)            /* 8 is minimum zlib stream size */
        quit("need positive size of 8 or greater");
    आकार = (अचिन्हित)ret;

    /* allocate memory for buffers and compression engine */
    blk = दो_स्मृति(आकार + EXCESS);
    def.zalloc = Z_NULL;
    def.zfree = Z_NULL;
    def.opaque = Z_NULL;
    ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
    अगर (ret != Z_OK || blk == शून्य)
        quit("out of memory");

    /* compress from stdin until output full, or no more input */
    def.avail_out = आकार + EXCESS;
    def.अग्र_out = blk;
    ret = partcompress(मानक_निवेश, &def);
    अगर (ret == Z_ERRNO)
        quit("error reading input");

    /* if it all fit, then size was undersubscribed -- done! */
    अगर (ret == Z_STREAM_END && def.avail_out >= EXCESS) {
        /* write block to stdout */
        have = आकार + EXCESS - def.avail_out;
        अगर (ख_डालो(blk, 1, have, मानक_निकास) != have || ख_त्रुटि(मानक_निकास))
            quit("error writing output");

        /* clean up and print results to stderr */
        ret = deflateEnd(&def);
        निश्चित(ret != Z_STREAM_ERROR);
        मुक्त(blk);
        ख_लिखो(मानक_त्रुटि,
                "%u bytes unused out of %u requested (all input)\n",
                आकार - have, आकार);
        वापस 0;
    }

    /* it didn't all fit -- set up for recompression */
    inf.zalloc = Z_NULL;
    inf.zfree = Z_NULL;
    inf.opaque = Z_NULL;
    inf.avail_in = 0;
    inf.अग्र_in = Z_NULL;
    ret = inflateInit(&inf);
    tmp = दो_स्मृति(आकार + EXCESS);
    अगर (ret != Z_OK || tmp == शून्य)
        quit("out of memory");
    ret = deflateReset(&def);
    निश्चित(ret != Z_STREAM_ERROR);

    /* do first recompression close to the right amount */
    inf.avail_in = आकार + EXCESS;
    inf.अग्र_in = blk;
    def.avail_out = आकार + EXCESS;
    def.अग्र_out = tmp;
    ret = recompress(&inf, &def);
    अगर (ret == Z_MEM_ERROR)
        quit("out of memory");

    /* set up for next recompression */
    ret = inflateReset(&inf);
    निश्चित(ret != Z_STREAM_ERROR);
    ret = deflateReset(&def);
    निश्चित(ret != Z_STREAM_ERROR);

    /* do second and final recompression (third compression) */
    inf.avail_in = आकार - MARGIN;   /* assure stream will complete */
    inf.अग्र_in = tmp;
    def.avail_out = आकार;
    def.अग्र_out = blk;
    ret = recompress(&inf, &def);
    अगर (ret == Z_MEM_ERROR)
        quit("out of memory");
    निश्चित(ret == Z_STREAM_END);    /* otherwise MARGIN too small */

    /* done -- write block to stdout */
    have = आकार - def.avail_out;
    अगर (ख_डालो(blk, 1, have, मानक_निकास) != have || ख_त्रुटि(मानक_निकास))
        quit("error writing output");

    /* clean up and print results to stderr */
    मुक्त(tmp);
    ret = inflateEnd(&inf);
    निश्चित(ret != Z_STREAM_ERROR);
    ret = deflateEnd(&def);
    निश्चित(ret != Z_STREAM_ERROR);
    मुक्त(blk);
    ख_लिखो(मानक_त्रुटि,
            "%u bytes unused out of %u requested (%lu input)\n",
            आकार - have, आकार, def.योग_in);
    वापस 0;
}
