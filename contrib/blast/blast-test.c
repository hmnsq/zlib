#ضمن "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ضمن <stdio.h>
#ضمن <stdlib.h>

#define CHUNK 16384

local unsigned inf(فراغ *how, unsigned حرف **buf)
{
    ساكن unsigned حرف hold[CHUNK];

    *buf = hold;
    החזר fread(hold, 1, CHUNK, (FILE *)how);
}

local صحيح outf(فراغ *how, unsigned حرف *buf, unsigned len)
{
    החזר fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحيح main(فراغ)
{
    صحيح ret;
    unsigned left;

    /* decompress to stdout */
    left = 0;
    ret = blast(inf, stdin, outf, stdout, &left, عدم);
    אם (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    כלעוד (getchar() != EOF)
        left++;
    אם (left)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", left);

    /* return blast() error code */
    החזר ret;
}
