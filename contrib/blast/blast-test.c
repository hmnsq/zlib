#شامل "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#شامل <stdio.h>
#شامل <stdlib.h>

#تعریف_کرو CHUNK 16384

local غیرعلامتی inf(خالی *how, غیرعلامتی حرف **buf)
{
    ساکن غیرعلامتی حرف hold[CHUNK];

    *buf = hold;
    واپس fread(hold, 1, CHUNK, (FILE *)how);
}

local صحیح outf(خالی *how, غیرعلامتی حرف *buf, غیرعلامتی len)
{
    واپس fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
صحیح مین(خالی)
{
    صحیح ret;
    غیرعلامتی بایاں;

    /* decompress to stdout */
    بایاں = 0;
    ret = blast(inf, stdin, outf, stdout, &بایاں, تہی);
    اگر (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    جب (getchar() != EOF)
        بایاں++;
    اگر (بایاں)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", بایاں);

    /* return blast() error code */
    واپس ret;
}
