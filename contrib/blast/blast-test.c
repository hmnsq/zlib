#شامل "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#شامل <stdio.h>
#شامل <stdlib.h>

#define CHUNK 16384

local unsigned inf(تهی *how, unsigned نویسه **buf)
{
    ایستا unsigned نویسه hold[CHUNK];

    *buf = hold;
    بازگشت fread(hold, 1, CHUNK, (FILE *)how);
}

local صحیح outf(تهی *how, unsigned نویسه *buf, unsigned len)
{
    بازگشت fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحیح اصلی(تهی)
{
    صحیح ret;
    unsigned چپ;

    /* decompress to stdout */
    چپ = 0;
    ret = blast(inf, stdin, outf, stdout, &چپ, پوچ);
    اگر (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    وقتی (getchar() != EOF)
        چپ++;
    اگر (چپ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", چپ);

    /* return blast() error code */
    بازگشت ret;
}
