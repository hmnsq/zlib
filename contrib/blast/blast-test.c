#അന്തര്ഭാവ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>

#define CHUNK 16384

local unsigned inf(ശൂന്യം *how, unsigned അക്ഷരം **buf)
{
    സ്ഥിതം unsigned അക്ഷരം hold[CHUNK];

    *buf = hold;
    തിരികെ fread(hold, 1, CHUNK, (FILE *)how);
}

local പൂർണ്ണസംഖ്യ outf(ശൂന്യം *how, unsigned അക്ഷരം *buf, unsigned len)
{
    തിരികെ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ ret;
    unsigned ഇടത്;

    /* decompress to stdout */
    ഇടത് = 0;
    ret = blast(inf, stdin, outf, stdout, &ഇടത്, ശൂന്യ);
    എങ്കിൽ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ആകുമ്പോൾ (getchar() != EOF)
        ഇടത്++;
    എങ്കിൽ (ഇടത്)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", ഇടത്);

    /* return blast() error code */
    തിരികെ ret;
}
