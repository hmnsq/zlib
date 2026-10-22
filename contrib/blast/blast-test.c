#अन्तर्भाव "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

#define CHUNK 16384

local unsigned inf(शून्य *how, unsigned वर्ण **buf)
{
    स्थिर unsigned वर्ण hold[CHUNK];

    *buf = hold;
    प्रत्यागम fread(hold, 1, CHUNK, (FILE *)how);
}

local पूर्णाङ्क outf(शून्य *how, unsigned वर्ण *buf, unsigned len)
{
    प्रत्यागम fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
पूर्णाङ्क main(शून्य)
{
    पूर्णाङ्क ret;
    unsigned डावा;

    /* decompress to stdout */
    डावा = 0;
    ret = blast(inf, stdin, outf, stdout, &डावा, रिक्त);
    यदि (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    यावत् (getchar() != EOF)
        डावा++;
    यदि (डावा)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", डावा);

    /* return blast() error code */
    प्रत्यागम ret;
}
