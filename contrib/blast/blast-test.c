#அந்தர்பாவ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>

#define CHUNK 16384

local unsigned inf(வெறுமை *how, unsigned எழுத்து **buf)
{
    நிலையான unsigned எழுத்து hold[CHUNK];

    *buf = hold;
    திரும்பு fread(hold, 1, CHUNK, (FILE *)how);
}

local முழு outf(வெறுமை *how, unsigned எழுத்து *buf, unsigned len)
{
    திரும்பு fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
முழு முதன்மை(வெறுமை)
{
    முழு ret;
    unsigned இடது;

    /* decompress to stdout */
    இடது = 0;
    ret = blast(inf, stdin, outf, stdout, &இடது, வெற்று);
    எனில் (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    போது (getchar() != EOF)
        இடது++;
    எனில் (இடது)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", இடது);

    /* return blast() error code */
    திரும்பு ret;
}
