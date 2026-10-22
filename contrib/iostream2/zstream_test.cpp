#समावेश "zstream.h"
#समावेश <math.h>
#समावेश <stdlib.h>
#समावेश <iomanip.h>

व्योम मुख्य() {
    अक्षर h[256] = "Hello";
    अक्षर* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.बंद();

    izstream in("temp.gz"); // read it back
    अक्षर *x = read_string(in), *y = नया अक्षर[256], z[256];
    in > y > z;
    in.बंद();
    अ_बाहर << x << प_पूर्ण << y << प_पूर्ण << z << प_पूर्ण;

    out.खोलो("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << रखो_चौड़ाई(50) << रखो_भरो('#') << रखो_सटीक(20) << x << प_पूर्ण << y << प_पूर्ण << z << प_पूर्ण;
    out << z << प_पूर्ण << y << प_पूर्ण << x << प_पूर्ण;
    out << 1.1234567890123456789 << प_पूर्ण;

    मिटाओ[] x; मिटाओ[] y;
}
