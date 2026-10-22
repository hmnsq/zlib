#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned char **buf)
{
    static unsigned char hold[CHUNK];

    *buf = hold;
    return fread(hold, 1, CHUNK, (FILE *)how);
}

local int outf(void *how, unsigned char *buf, unsigned len)
{
    return fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
int main(void)
{
    int ret;
    unsigned कालिवा;

    /* decompress to stdout */
    कालिवा = 0;
    ret = blast(inf, stdin, outf, stdout, &कालिवा, NULL);
    if (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    while (getchar() != EOF)
        कालिवा++;
    if (कालिवा)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", कालिवा);

    /* return blast() error code */
    return ret;
}
