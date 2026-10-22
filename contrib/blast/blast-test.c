#ضمن "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ضمن <stdio.h>
#ضمن <stdlib.h>

#تعریف_کن CHUNK 16384

local بیعلامت inf(فراغ *how, بیعلامت حرف **buf)
{
    ساكن بیعلامت حرف hold[CHUNK];

    *buf = hold;
    أرجع fread(hold, 1, CHUNK, (FILE *)how);
}

local صحيح outf(فراغ *how, بیعلامت حرف *buf, بیعلامت len)
{
    أرجع fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحيح اصلی(فراغ)
{
    صحيح ret;
    بیعلامت چپ;

    /* decompress to stdout */
    چپ = 0;
    ret = blast(inf, stdin, outf, stdout, &چپ, عدم);
    إذا (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    طالما (getchar() != EOF)
        چپ++;
    إذا (چپ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", چپ);

    /* return blast() error code */
    أرجع ret;
}
