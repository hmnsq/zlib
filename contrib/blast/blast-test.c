#ضمن "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ضمن <stdio.h>
#ضمن <stdlib.h>

#وصف_ڪريو CHUNK 16384

local بنانشان inf(فراغ *how, بنانشان حرف **buf)
{
    ساكن بنانشان حرف hold[CHUNK];

    *buf = hold;
    أرجع fread(hold, 1, CHUNK, (FILE *)how);
}

local صحيح outf(فراغ *how, بنانشان حرف *buf, بنانشان len)
{
    أرجع fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحيح مکيه(فراغ)
{
    صحيح ret;
    بنانشان کاٻو;

    /* decompress to stdout */
    کاٻو = 0;
    ret = blast(inf, stdin, outf, stdout, &کاٻو, عدم);
    إذا (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    طالما (getchar() != EOF)
        کاٻو++;
    إذا (کاٻو)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", کاٻو);

    /* return blast() error code */
    أرجع ret;
}
