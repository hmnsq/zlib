#అన్తర్భావ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>

#నిర్వచించు CHUNK 16384

local గుర్తులేని inf(శూన్య *how, గుర్తులేని అక్షరం **buf)
{
    స్థిర గుర్తులేని అక్షరం hold[CHUNK];

    *buf = hold;
    తిరిగి fread(hold, 1, CHUNK, (FILE *)how);
}

local పూర్ణాంకం outf(శూన్య *how, గుర్తులేని అక్షరం *buf, గుర్తులేని len)
{
    తిరిగి fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
పూర్ణాంకం ప్రధాన(శూన్య)
{
    పూర్ణాంకం ret;
    గుర్తులేని ఎడమ;

    /* decompress to stdout */
    ఎడమ = 0;
    ret = blast(inf, stdin, outf, stdout, &ఎడమ, ఖాళీ);
    ఒకవేళ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    అయితే (getchar() != EOF)
        ఎడమ++;
    ఒకవేళ (ఎడమ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", ఎడమ);

    /* return blast() error code */
    తిరిగి ret;
}
