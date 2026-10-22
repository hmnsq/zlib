#ଅନ୍ତର୍ଭାଵ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#define CHUNK 16384

local unsigned inf(ଶୂନ୍ୟ *how, unsigned ଅକ୍ଷର **buf)
{
    ସ୍ଥିର unsigned ଅକ୍ଷର hold[CHUNK];

    *buf = hold;
    ଫେରନ୍ତୁ fread(hold, 1, CHUNK, (FILE *)how);
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା outf(ଶୂନ୍ୟ *how, unsigned ଅକ୍ଷର *buf, unsigned len)
{
    ଫେରନ୍ତୁ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    unsigned ବାମ;

    /* decompress to stdout */
    ବାମ = 0;
    ret = blast(inf, stdin, outf, stdout, &ବାମ, ଖାଲି);
    ଯଦି (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ଯେତେବେଳେ (getchar() != EOF)
        ବାମ++;
    ଯଦି (ବାମ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", ବାମ);

    /* return blast() error code */
    ଫେରନ୍ତୁ ret;
}
