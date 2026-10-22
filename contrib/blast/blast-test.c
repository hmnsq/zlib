#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned var_na **buf)
{
    static unsigned var_na hold[CHUNK];

    *buf = hold;
    ပြန်ပေး fread(hold, 1, CHUNK, (FILE *)how);
}

local ကိန်းပြည့် outf(void *how, unsigned var_na *buf, unsigned len)
{
    ပြန်ပေး fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
ကိန်းပြည့် main(void)
{
    ကိန်းပြည့် ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, NULL);
    အကယ်၍ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    နေစဉ် (getchar() != EOF)
        left++;
    အကယ်၍ (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    ပြန်ပေး ret;
}
