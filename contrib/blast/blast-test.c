#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned var_na **buf)
{
    static unsigned var_na hold[CHUNK];

    *buf = hold;
    ສົ່ງຄືນ fread(hold, 1, CHUNK, (FILE *)how);
}

local ຈຳນວນເຕັມ outf(void *how, unsigned var_na *buf, unsigned len)
{
    ສົ່ງຄືນ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
ຈຳນວນເຕັມ main(void)
{
    ຈຳນວນເຕັມ ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, NULL);
    ຖ້າ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ໃນຂະນະທີ່ (getchar() != EOF)
        left++;
    ຖ້າ (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    ສົ່ງຄືນ ret;
}
