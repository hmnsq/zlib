#ضمن "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ضمن <stdio.h>
#ضمن <stdlib.h>

#تعریف_کړه CHUNK 16384

local بینښه inf(فراغ *how, بینښه حرف **buf)
{
    ساكن بینښه حرف hold[CHUNK];

    *buf = hold;
    أرجع fread(hold, 1, CHUNK, (FILE *)how);
}

local صحيح outf(فراغ *how, بینښه حرف *buf, بینښه len)
{
    أرجع fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحيح اصلي(فراغ)
{
    صحيح ret;
    بینښه کیڼ;

    /* decompress to stdout */
    کیڼ = 0;
    ret = blast(inf, stdin, outf, stdout, &کیڼ, عدم);
    إذا (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    طالما (getchar() != EOF)
        کیڼ++;
    إذا (کیڼ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", کیڼ);

    /* return blast() error code */
    أرجع ret;
}
