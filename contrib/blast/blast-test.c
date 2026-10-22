#अन्तर्भाव "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

#परिभाषय CHUNK 16384

local अचिह्नित inf(शून्य *how, अचिह्नित वर्ण **buf)
{
    स्थिर अचिह्नित वर्ण hold[CHUNK];

    *buf = hold;
    प्रत्यागम fread(hold, 1, CHUNK, (FILE *)how);
}

local पूर्णाङ्क outf(शून्य *how, अचिह्नित वर्ण *buf, अचिह्नित len)
{
    प्रत्यागम fwrite(buf, 1, len, (FILE *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
पूर्णाङ्क मुख्यम्(शून्य)
{
    पूर्णाङ्क ret;
    अचिह्नित वामः;

    /* decompress to stdout */
    वामः = 0;
    ret = blast(inf, stdin, outf, stdout, &वामः, रिक्त);
    यदि (ret != 0)
        fprintf(stderr, "blast error: %d\n", ret);

    /* count any leftover bytes */
    यावत् (अक्षरं_पठ() != EOF)
        वामः++;
    यदि (वामः)
        fprintf(stderr, "blast warning: %u unused bytes of input\n", वामः);

    /* return blast() error code */
    प्रत्यागम ret;
}
