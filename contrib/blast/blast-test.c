#अन्तर्भाव "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

#define CHUNK 16384

local अचिन्हित inf(शून्य *how, अचिन्हित वर्ण **buf)
{
    स्थिर अचिन्हित वर्ण hold[CHUNK];

    *buf = hold;
    प्रत्यागम ख_पढ़ो(hold, 1, CHUNK, (खाता *)how);
}

local पूर्णाङ्क outf(शून्य *how, अचिन्हित वर्ण *buf, अचिन्हित len)
{
    प्रत्यागम ख_डालो(buf, 1, len, (खाता *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
पूर्णाङ्क main(शून्य)
{
    पूर्णाङ्क ret;
    अचिन्हित बायाँ;

    /* decompress to stdout */
    बायाँ = 0;
    ret = blast(inf, मानक_निवेश, outf, मानक_निकास, &बायाँ, रिक्त);
    यदि (ret != 0)
        ख_लिखो(मानक_त्रुटि, "blast error: %d\n", ret);

    /* count any leftover bytes */
    यावत् (अक्षर_लो() != खातापूर्ण)
        बायाँ++;
    यदि (बायाँ)
        ख_लिखो(मानक_त्रुटि, "blast warning: %u unused bytes of input\n", बायाँ);

    /* return blast() error code */
    प्रत्यागम ret;
}
