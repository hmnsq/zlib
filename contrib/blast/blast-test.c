#ضمن "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ضمن <stdio.h>
#ضمن <stdlib.h>

#define CHUNK 16384

local unsigned inf(فراغ *how, unsigned حرف **buf)
{
    ساكن unsigned حرف hold[CHUNK];

    *buf = hold;
    أرجع fread(hold, 1, CHUNK, (FILE *)how);
}

local صحيح outf(فراغ *how, unsigned حرف *buf, unsigned len)
{
    أرجع fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحيح main(فراغ)
{
    صحيح ret;
    unsigned سول;

    /* decompress to stdout */
    سول = 0;
    ret = blast(inf, stdin, outf, stdout, &سول, عدم);
    إذا (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    طالما (getchar() != EOF)
        سول++;
    إذا (سول)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", سول);

    /* return blast() error code */
    أرجع ret;
}
