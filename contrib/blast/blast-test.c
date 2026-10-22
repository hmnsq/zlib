#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned char **buf)
{
    static unsigned char hold[CHUNK];

    *buf = hold;
    врати fread(hold, 1, CHUNK, (FILE *)how);
}

local цео outf(void *how, unsigned char *buf, unsigned len)
{
    врати fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
цео main(void)
{
    цео ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, NULL);
    ако (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    док (getchar() != EOF)
        left++;
    ако (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    врати ret;
}
