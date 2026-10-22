#অন্তর্ভাব "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>

#define CHUNK 16384

local unsigned inf(শূন্য *how, unsigned অক্ষর **buf)
{
    স্থির unsigned অক্ষর hold[CHUNK];

    *buf = hold;
    ফেরত fread(hold, 1, CHUNK, (FILE *)how);
}

local পূর্ণসংখ্যা outf(শূন্য *how, unsigned অক্ষর *buf, unsigned len)
{
    ফেরত fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
পূর্ণসংখ্যা প্রধান(শূন্য)
{
    পূর্ণসংখ্যা ret;
    unsigned বাম;

    /* decompress to stdout */
    বাম = 0;
    ret = blast(inf, stdin, outf, stdout, &বাম, ফাঁকা);
    যদি (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    যখন (getchar() != EOF)
        বাম++;
    যদি (বাম)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", বাম);

    /* return blast() error code */
    ফেরত ret;
}
