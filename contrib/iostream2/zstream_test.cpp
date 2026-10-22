#अन्तर्भाव "zstream.h"
#अन्तर्भाव <math.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <iomanip.h>

शून्य मुख्यम्() {
    वर्ण h[256] = "Hello";
    वर्ण* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    वर्ण *x = read_string(in), *y = नूतन वर्ण[256], z[256];
    in > y > z;
    in.close();
    निर्गमः << x << पङ्क्त्यन्तः << y << पङ्क्त्यन्तः << z << पङ्क्त्यन्तः;

    out.उद्घाटय("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << पङ्क्त्यन्तः << y << पङ्क्त्यन्तः << z << पङ्क्त्यन्तः;
    out << z << पङ्क्त्यन्तः << y << पङ्क्त्यन्तः << x << पङ्क्त्यन्तः;
    out << 1.1234567890123456789 << पङ्क्त्यन्तः;

    विलोपय[] x; विलोपय[] y;
}
