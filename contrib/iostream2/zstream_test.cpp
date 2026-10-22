#অন্তর্ভাব "zstream.h"
#অন্তর্ভাব <math.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <iomanip.h>

শূন্য main() {
    বর্ণ h[256] = "Hello";
    বর্ণ* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    বর্ণ *x = read_string(in), *y = নূতন বর্ণ[256], z[256];
    in > y > z;
    in.close();
    cout << x << endl << y << endl << z << endl;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << endl << y << endl << z << endl;
    out << z << endl << y << endl << x << endl;
    out << 1.1234567890123456789 << endl;

    delete[] x; delete[] y;
}
