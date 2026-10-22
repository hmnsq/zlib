#ضمن "zstream.h"
#ضمن <math.h>
#ضمن <stdlib.h>
#ضمن <iomanip.h>

فراغ مکھ() {
    حرف h[256] = "Hello";
    حرف* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    حرف *x = read_string(in), *y = جديد حرف[256], z[256];
    in > y > z;
    in.close();
    باہر << x << سطر_انت << y << سطر_انت << z << سطر_انت;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << سطر_انت << y << سطر_انت << z << سطر_انت;
    out << z << سطر_انت << y << سطر_انت << x << سطر_انت;
    out << 1.1234567890123456789 << سطر_انت;

    مٹاؤ[] x; مٹاؤ[] y;
}
