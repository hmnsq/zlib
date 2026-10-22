#ضمن "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ضمن <stdio.h>
#ضمن <stdlib.h>

#تعریف_کرو CHUNK 16384

local بنانشانی inf(فراغ *how, بنانشانی حرف **buf)
{
    ساكن بنانشانی حرف hold[CHUNK];

    *buf = hold;
    أرجع fread(hold, 1, CHUNK, (FILE *)how);
}

local صحيح outf(فراغ *how, بنانشانی حرف *buf, بنانشانی len)
{
    أرجع fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحيح مکھ(فراغ)
{
    صحيح ret;
    بنانشانی بایاں;

    /* decompress to stdout */
    بایاں = 0;
    ret = blast(inf, stdin, outf, stdout, &بایاں, عدم);
    إذا (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    طالما (getchar() != EOF)
        بایاں++;
    إذا (بایاں)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", بایاں);

    /* return blast() error code */
    أرجع ret;
}
