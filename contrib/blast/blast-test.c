#অন্তর্ভাব "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>

#define CHUNK 16384

local unsigned inf(শূন্য *how, unsigned বর্ণ **buf)
{
    স্থির unsigned বর্ণ hold[CHUNK];

    *buf = hold;
    প্রত্যাগম fread(hold, 1, CHUNK, (FILE *)how);
}

local পূর্ণাঙ্ক outf(শূন্য *how, unsigned বর্ণ *buf, unsigned len)
{
    প্রত্যাগম fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
পূর্ণাঙ্ক main(শূন্য)
{
    পূর্ণাঙ্ক ret;
    unsigned বাম;

    /* decompress to stdout */
    বাম = 0;
    ret = blast(inf, stdin, outf, stdout, &বাম, রিক্ত);
    যদি (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    যাবত্ (getchar() != EOF)
        বাম++;
    যদি (বাম)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", বাম);

    /* return blast() error code */
    প্রত্যাগম ret;
}
