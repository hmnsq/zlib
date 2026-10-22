#अन्तर्भाव "zstream.h"
#अन्तर्भाव <math.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <iomanip.h>

शून्य main() {
    अक्षर h[256] = "Hello";
    अक्षर* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.बंद();

    izstream in("temp.gz"); // read it back
    अक्षर *x = read_string(in), *y = नवीन अक्षर[256], z[256];
    in > y > z;
    in.बंद();
    cout << x << endl << y << endl << z << endl;

    out.खोलो("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << endl << y << endl << z << endl;
    out << z << endl << y << endl << x << endl;
    out << 1.1234567890123456789 << endl;

    delete[] x; delete[] y;
}
