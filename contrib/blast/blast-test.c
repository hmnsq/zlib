#include "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 16384

local unsigned inf(void *how, unsigned var_na **buf)
{
    static unsigned var_na hold[CHUNK];

    *buf = hold;
    คืน fread(hold, 1, CHUNK, (FILE *)how);
}

local จำนวนเต็ม outf(void *how, unsigned var_na *buf, unsigned len)
{
    คืน fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
จำนวนเต็ม หลัก(void)
{
    จำนวนเต็ม ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, NULL);
    ถ้า (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ขณะ (getchar() != EOF)
        left++;
    ถ้า (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    คืน ret;
}
