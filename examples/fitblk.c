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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव "zlib.h"

#परिभाषय local स्थिर

/* print nastygram and leave */
local शून्य quit(वर्ण *why)
{
    fprintf(stderr, "fitblk abort: %s\n", why);
    निर्गच्छ(1);
}

#परिभाषय RAWLEN 4096    /* intermediate uncompressed buffer size */

/* compress from file to def until provided buffer is full or end of
   input reached; return last deflate() return value, or Z_ERRNO if
   there was read error on the file */
local पूर्णाङ्क partcompress(FILE *अन्तर्, z_streamp def)
{
    पूर्णाङ्क ret, flush;
    अचिह्नित वर्ण raw[RAWLEN];

    flush = Z_NO_FLUSH;
    कुरु {
        def->avail_अन्तर् = fread(raw, 1, RAWLEN, अन्तर्);
        यदि (ferror(अन्तर्))
            प्रत्यागम Z_ERRNO;
        def->अग्र_अन्तर् = raw;
        यदि (feof(अन्तर्))
            flush = Z_FINISH;
        ret = deflate(def, flush);
        प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);
    } यावत् (def->avail_बहिर्गमनम् != 0 && flush == Z_NO_FLUSH);
    प्रत्यागम ret;
}

/* recompress from inf's input to def's output; the input for inf and
   the output for def are set in those structures before calling;
   return last deflate() return value, or Z_MEM_ERROR if inflate()
   was not able to allocate enough memory when it needed to */
local पूर्णाङ्क recompress(z_streamp inf, z_streamp def)
{
    पूर्णाङ्क ret, flush;
    अचिह्नित वर्ण raw[RAWLEN];

    flush = Z_NO_FLUSH;
    कुरु {
        /* decompress */
        inf->avail_बहिर्गमनम् = RAWLEN;
        inf->अग्र_बहिर्गमनम् = raw;
        ret = inflate(inf, Z_NO_FLUSH);
        प्रतिज्ञा(ret != Z_प्रवाहः_ERROR && ret != Z_दत्त_ERROR &&
               ret != Z_NEED_DICT);
        यदि (ret == Z_MEM_ERROR)
            प्रत्यागम ret;

        /* compress what was decompressed until done or no room */
        def->avail_अन्तर् = RAWLEN - inf->avail_बहिर्गमनम्;
        def->अग्र_अन्तर् = raw;
        यदि (inf->avail_बहिर्गमनम् != 0)
            flush = Z_FINISH;
        ret = deflate(def, flush);
        प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);
    } यावत् (ret != Z_प्रवाहः_अन्तः && def->avail_बहिर्गमनम् != 0);
    प्रत्यागम ret;
}

#परिभाषय EXCESS 256      /* empirically determined stream overage */
#परिभाषय MARGIN 8        /* amount to back off for completion */

/* compress from stdin to fixed-size block on stdout */
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
    पूर्णाङ्क ret;                /* return code */
    अचिह्नित आकार;          /* requested fixed output block size */
    अचिह्नित have;          /* bytes written by deflate() call */
    अचिह्नित वर्ण *blk;     /* intermediate and final stream */
    अचिह्नित वर्ण *tmp;     /* close to desired size stream */
    z_प्रवाहः def, inf;      /* zlib deflate and inflate states */

    /* get requested output size */
    यदि (argc != 2)
        quit("need one argument: size of output block");
    ret = strtol(argv[1], argv + 1, 10);
    यदि (argv[1][0] != 0)
        quit("argument must be a number");
    यदि (ret < 8)            /* 8 is minimum zlib stream size */
        quit("need positive size of 8 or greater");
    आकार = (अचिह्नित)ret;

    /* allocate memory for buffers and compression engine */
    blk = स्मृतिं_देहि(आकार + EXCESS);
    def.zalloc = Z_NULL;
    def.zfree = Z_NULL;
    def.opaque = Z_NULL;
    ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
    यदि (ret != Z_OK || blk == रिक्त)
        quit("out of memory");

    /* compress from stdin until output full, or no more input */
    def.avail_बहिर्गमनम् = आकार + EXCESS;
    def.अग्र_बहिर्गमनम् = blk;
    ret = partcompress(stdin, &def);
    यदि (ret == Z_ERRNO)
        quit("error reading input");

    /* if it all fit, then size was undersubscribed -- done! */
    यदि (ret == Z_प्रवाहः_अन्तः && def.avail_बहिर्गमनम् >= EXCESS) {
        /* write block to stdout */
        have = आकार + EXCESS - def.avail_बहिर्गमनम्;
        यदि (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
            quit("error writing output");

        /* clean up and print results to stderr */
        ret = deflate_अन्तः(&def);
        प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);
        मुक्त(blk);
        fprintf(stderr,
                "%u bytes unused out of %u requested (all input)\n",
                आकार - have, आकार);
        प्रत्यागम 0;
    }

    /* it didn't all fit -- set up for recompression */
    inf.zalloc = Z_NULL;
    inf.zfree = Z_NULL;
    inf.opaque = Z_NULL;
    inf.avail_अन्तर् = 0;
    inf.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit(&inf);
    tmp = स्मृतिं_देहि(आकार + EXCESS);
    यदि (ret != Z_OK || tmp == रिक्त)
        quit("out of memory");
    ret = deflateReset(&def);
    प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);

    /* do first recompression close to the right amount */
    inf.avail_अन्तर् = आकार + EXCESS;
    inf.अग्र_अन्तर् = blk;
    def.avail_बहिर्गमनम् = आकार + EXCESS;
    def.अग्र_बहिर्गमनम् = tmp;
    ret = recompress(&inf, &def);
    यदि (ret == Z_MEM_ERROR)
        quit("out of memory");

    /* set up for next recompression */
    ret = inflateReset(&inf);
    प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);
    ret = deflateReset(&def);
    प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);

    /* do second and final recompression (third compression) */
    inf.avail_अन्तर् = आकार - MARGIN;   /* assure stream will complete */
    inf.अग्र_अन्तर् = tmp;
    def.avail_बहिर्गमनम् = आकार;
    def.अग्र_बहिर्गमनम् = blk;
    ret = recompress(&inf, &def);
    यदि (ret == Z_MEM_ERROR)
        quit("out of memory");
    प्रतिज्ञा(ret == Z_प्रवाहः_अन्तः);    /* otherwise MARGIN too small */

    /* done -- write block to stdout */
    have = आकार - def.avail_बहिर्गमनम्;
    यदि (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
        quit("error writing output");

    /* clean up and print results to stderr */
    मुक्त(tmp);
    ret = inflate_अन्तः(&inf);
    प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);
    ret = deflate_अन्तः(&def);
    प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);
    मुक्त(blk);
    fprintf(stderr,
            "%u bytes unused out of %u requested (%lu input)\n",
            आकार - have, आकार, def.योग_अन्तर्);
    प्रत्यागम 0;
}
