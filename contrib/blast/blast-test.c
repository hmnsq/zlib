#अन्तर्भाव "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

#define CHUNK 16384

local unsigned inf(शून्य *how, unsigned वर्ण **buf)
{
    स्थिर unsigned वर्ण hold[CHUNK];

    *buf = hold;
    पटिनिवत्त fread(hold, 1, CHUNK, (FILE *)how);
}

local पूर्णाङ्क outf(शून्य *how, unsigned वर्ण *buf, unsigned len)
{
    पटिनिवत्त fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
पूर्णाङ्क main(शून्य)
{
    पूर्णाङ्क ret;
    unsigned वामः;

    /* decompress to stdout */
    वामः = 0;
    ret = blast(inf, stdin, outf, stdout, &वामः, रिक्त);
    सचे (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    याव (getchar() != EOF)
        वामः++;
    सचे (वामः)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", वामः);

    /* return blast() error code */
    पटिनिवत्त ret;
}
