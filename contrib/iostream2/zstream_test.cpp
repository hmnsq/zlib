#شامل "zstream.h"
#شامل <math.h>
#شامل <stdlib.h>
#شامل <iomanip.h>

خالی مین() {
    حرف h[256] = "Hello";
    حرف* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    حرف *x = read_string(in), *y = نیا حرف[256], z[256];
    in > y > z;
    in.close();
    خارج << x << سطر_ختم << y << سطر_ختم << z << سطر_ختم;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << سطر_ختم << y << سطر_ختم << z << سطر_ختم;
    out << z << سطر_ختم << y << سطر_ختم << x << سطر_ختم;
    out << 1.1234567890123456789 << سطر_ختم;

    حذف[] x; حذف[] y;
}
