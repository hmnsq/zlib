#अन्तर्भाव "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

#define CHUNK 16384

local अचिन्हित inf(शून्य *how, अचिन्हित अक्षर **buf)
{
    स्थिर अचिन्हित अक्षर hold[CHUNK];

    *buf = hold;
    परत ख_पढ़ो(hold, 1, CHUNK, (खाता *)how);
}

local पूर्णांक outf(शून्य *how, अचिन्हित अक्षर *buf, अचिन्हित len)
{
    परत ख_डालो(buf, 1, len, (खाता *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
पूर्णांक main(शून्य)
{
    पूर्णांक ret;
    अचिन्हित डावा;

    /* decompress to stdout */
    डावा = 0;
    ret = blast(inf, मानक_निवेश, outf, मानक_निकास, &डावा, रिक्त);
    जर (ret != 0)
        ख_लिखो(मानक_त्रुटि, "blast error: %d\n", ret);

    /* count any leftover bytes */
    पर्यंत (अक्षर_लो() != खातापूर्ण)
        डावा++;
    जर (डावा)
        ख_लिखो(मानक_त्रुटि, "blast warning: %u unused bytes of input\n", डावा);

    /* return blast() error code */
    परत ret;
}
