#شامل "zstream.h"
#شامل <math.h>
#شامل <stdlib.h>
#شامل <iomanip.h>

تهی اصلی() {
    نویسه h[256] = "Hello";
    نویسه* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    نویسه *x = read_string(in), *y = جدید نویسه[256], z[256];
    in > y > z;
    in.close();
    cout << x << endl << y << endl << z << endl;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << endl << y << endl << z << endl;
    out << z << endl << y << endl << x << endl;
    out << 1.1234567890123456789 << endl;

    delete[] x; delete[] y;
}
