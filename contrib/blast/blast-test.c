#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned var_na **buf)
{
    static unsigned var_na hold[CHUNK];

    *buf = hold;
    ᮝᮜᮤ fread(hold, 1, CHUNK, (FILE *)how);
}

local puur_naa_nka outf(void *how, unsigned var_na *buf, unsigned len)
{
    ᮝᮜᮤ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
puur_naa_nka main(void)
{
    puur_naa_nka ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, NULL);
    ᮚᮦᮔ᮪ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ᮞᮨᮓᮨᮜ (getchar() != EOF)
        left++;
    ᮚᮦᮔ᮪ (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    ᮝᮜᮤ ret;
}
