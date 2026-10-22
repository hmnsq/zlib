#અન્તર્ભાવ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>

#define CHUNK 16384

local unsigned inf(શૂન્ય *how, unsigned અક્ષર **buf)
{
    સ્થિર unsigned અક્ષર hold[CHUNK];

    *buf = hold;
    પાછા fread(hold, 1, CHUNK, (FILE *)how);
}

local પૂર્ણાંક outf(શૂન્ય *how, unsigned અક્ષર *buf, unsigned len)
{
    પાછા fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
પૂર્ણાંક મુખ્ય(શૂન્ય)
{
    પૂર્ણાંક ret;
    unsigned ડાબું;

    /* decompress to stdout */
    ડાબું = 0;
    ret = blast(inf, stdin, outf, stdout, &ડાબું, ખાલી);
    જો (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    જ્યારે (getchar() != EOF)
        ડાબું++;
    જો (ડાબું)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", ડાબું);

    /* return blast() error code */
    પાછા ret;
}
