#ضمن "zstream.h"
#ضمن <math.h>
#ضمن <stdlib.h>
#ضمن <iomanip.h>

فراغ اصلی() {
    حرف h[256] = "Hello";
    حرف* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    حرف *x = read_string(in), *y = جديد حرف[256], z[256];
    in > y > z;
    in.close();
    خروجی << x << پایان_سطر << y << پایان_سطر << z << پایان_سطر;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << پایان_سطر << y << پایان_سطر << z << پایان_سطر;
    out << z << پایان_سطر << y << پایان_سطر << x << پایان_سطر;
    out << 1.1234567890123456789 << پایان_سطر;

    حذف[] x; حذف[] y;
}
