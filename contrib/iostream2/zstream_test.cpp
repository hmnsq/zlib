#ଅନ୍ତର୍ଭାଵ "zstream.h"
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <iomanip.h>

ଶୂନ୍ୟ ମୁଖ୍ୟ() {
    ଅକ୍ଷର h[256] = "Hello";
    ଅକ୍ଷର* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    ଅକ୍ଷର *x = read_string(in), *y = ନୂତନ ଅକ୍ଷର[256], z[256];
    in > y > z;
    in.close();
    cout << x << endl << y << endl << z << endl;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << endl << y << endl << z << endl;
    out << z << endl << y << endl << x << endl;
    out << 1.1234567890123456789 << endl;

    delete[] x; delete[] y;
}
