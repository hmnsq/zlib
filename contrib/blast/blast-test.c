#ضمن "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ضمن <stdio.h>
#ضمن <stdlib.h>

#define CHUNK 16384

local unsigned inf(فراغ *how, unsigned حرف **buf)
{
    ساكن unsigned حرف hold[CHUNK];

    *buf = hold;
    إرجاع fread(hold, 1, CHUNK, (FILE *)how);
}

local صحيح outf(فراغ *how, unsigned حرف *buf, unsigned len)
{
    إرجاع fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحيح الرئيسية(فراغ)
{
    صحيح ret;
    unsigned يسار;

    /* decompress to stdout */
    يسار = 0;
    ret = blast(inf, stdin, outf, stdout, &يسار, عدم);
    إذا (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    بينما (getchar() != EOF)
        يسار++;
    إذا (يسار)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", يسار);

    /* return blast() error code */
    إرجاع ret;
}
