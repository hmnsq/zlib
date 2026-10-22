#ضمن "zstream.h"
#ضمن <math.h>
#ضمن <stdlib.h>
#ضمن <iomanip.h>

فراغ اصلي() {
    حرف h[256] = "Hello";
    حرف* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    حرف *x = read_string(in), *y = جديد حرف[256], z[256];
    in > y > z;
    in.close();
    وتنه << x << کرښه_پای << y << کرښه_پای << z << کرښه_پای;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << کرښه_پای << y << کرښه_پای << z << کرښه_پای;
    out << z << کرښه_پای << y << کرښه_پای << x << کرښه_پای;
    out << 1.1234567890123456789 << کرښه_پای;

    ړنګول[] x; ړنګول[] y;
}
