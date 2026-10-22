#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned var_na **buf)
{
    static unsigned var_na hold[CHUNK];

    *buf = hold;
    ආපසු fread(hold, 1, CHUNK, (FILE *)how);
}

local පූර්ණ_සංඛ්‍යා outf(void *how, unsigned var_na *buf, unsigned len)
{
    ආපසු fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
පූර්ණ_සංඛ්‍යා main(void)
{
    පූර්ණ_සංඛ්‍යා ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, NULL);
    නම් (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    අතරතුර (getchar() != EOF)
        left++;
    නම් (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    ආපසු ret;
}
