#ಅನ್ತರ್ಭಾವ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#define CHUNK 16384

local unsigned inf(ಶೂನ್ಯ *how, unsigned ವರ್ಣ **buf)
{
    ಸ್ಥಿರ unsigned ವರ್ಣ hold[CHUNK];

    *buf = hold;
    ಪ್ರತ್ಯಾಗಮ fread(hold, 1, CHUNK, (FILE *)how);
}

local ಪೂರ್ಣಾಙ್ಕ outf(ಶೂನ್ಯ *how, unsigned ವರ್ಣ *buf, unsigned len)
{
    ಪ್ರತ್ಯಾಗಮ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
ಪೂರ್ಣಾಙ್ಕ main(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ ret;
    unsigned ಎಡ;

    /* decompress to stdout */
    ಎಡ = 0;
    ret = blast(inf, stdin, outf, stdout, &ಎಡ, ರಿಕ್ತ);
    ಯದಿ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ಯಾವತ್ (getchar() != EOF)
        ಎಡ++;
    ಯದಿ (ಎಡ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", ಎಡ);

    /* return blast() error code */
    ಪ್ರತ್ಯಾಗಮ ret;
}
