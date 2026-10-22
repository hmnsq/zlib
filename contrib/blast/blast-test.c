#ਅਨ੍ਤਰ੍ਭਾਵ "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK 16384

local ਬਿਨਾਨਿਸ਼ਾਨੀ inf(ਖਾਲੀ *how, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **buf)
{
    ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ hold[CHUNK];

    *buf = hold;
    ਵਾਪਸ fread(hold, 1, CHUNK, (FILE *)how);
}

local ਪੂਰਨ outf(ਖਾਲੀ *how, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len)
{
    ਵਾਪਸ fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ)
{
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ;

    /* decompress to stdout */
    ਖੱਬਾ = 0;
    ret = blast(inf, stdin, outf, stdout, &ਖੱਬਾ, ਨਿਰਾ);
    ਜੇ (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    ਜਦੋਂ (getchar() != EOF)
        ਖੱਬਾ++;
    ਜੇ (ਖੱਬਾ)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", ਖੱਬਾ);

    /* return blast() error code */
    ਵਾਪਸ ret;
}
