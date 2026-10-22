#समावेश "blast.h"              /* prototype for blast() */

/* Example of how to use blast() */
#समावेश <stdio.h>
#समावेश <stdlib.h>

#घोषणा CHUNK 16384

local अचिन्हित inf(व्योम *how, अचिन्हित अक्षर **buf)
{
    अटल अचिन्हित अक्षर hold[CHUNK];

    *buf = hold;
    वापस ख_पढ़ो(hold, 1, CHUNK, (खाता *)how);
}

local पूर्णांक outf(व्योम *how, अचिन्हित अक्षर *buf, अचिन्हित len)
{
    वापस ख_डालो(buf, 1, len, (खाता *)how) != len;
}

/* Decompress a PKWare Compression Library stream from stdin to stdout */
पूर्णांक मुख्य(व्योम)
{
    पूर्णांक ret;
    अचिन्हित बायाँ;

    /* decompress to stdout */
    बायाँ = 0;
    ret = blast(inf, मानक_निवेश, outf, मानक_निकास, &बायाँ, शून्य);
    अगर (ret != 0)
        ख_लिखो(मानक_त्रुटि, "blast error: %d\n", ret);

    /* count any leftover bytes */
    जबतक (अक्षर_लो() != खातापूर्ण)
        बायाँ++;
    अगर (बायाँ)
        ख_लिखो(मानक_त्रुटि, "blast warning: %u unused bytes of input\n", बायाँ);

    /* return blast() error code */
    वापस ret;
}
