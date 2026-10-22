#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned var_na **buf)
{
    static unsigned var_na hold[CHUNK];

    *buf = hold;
    ត្រឡប់ fread(hold, 1, CHUNK, (FILE *)how);
}

local ចំនួនគត់ outf(void *how, unsigned var_na *buf, unsigned len)
{
    ត្រឡប់ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
ចំនួនគត់ main(void)
{
    ចំនួនគត់ ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, NULL);
    ប្រសិនបើ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ខណៈពេល (getchar() != EOF)
        left++;
    ប្រសិនបើ (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    ត្រឡប់ ret;
}
