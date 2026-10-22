#ಅನ್ತರ್ಭಾವ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ವ್ಯಾಖ್ಯಾನಿಸು CHUNK 16384

local ಚಿಹ್ನೆರಹಿತ inf(ಶೂನ್ಯ *how, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **buf)
{
    ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ hold[CHUNK];

    *buf = hold;
    ಹಿಂತಿರುಗಿ fread(hold, 1, CHUNK, (FILE *)how);
}

local ಪೂರ್ಣಾಂಕ outf(ಶೂನ್ಯ *how, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ len)
{
    ಹಿಂತಿರುಗಿ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ ಎಡ;

    /* decompress to stdout */
    ಎಡ = 0;
    ret = blast(inf, stdin, outf, stdout, &ಎಡ, ಖಾಲಿ);
    ಆದರೆ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ಆಗ (getchar() != EOF)
        ಎಡ++;
    ಆದರೆ (ಎಡ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", ಎಡ);

    /* return blast() error code */
    ಹಿಂತಿರುಗಿ ret;
}
