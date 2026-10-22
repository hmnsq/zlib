#ضمن "zstream.h"
#ضمن <math.h>
#ضمن <stdlib.h>
#ضمن <iomanip.h>

فراغ مکيه() {
    حرف h[256] = "Hello";
    حرف* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    حرف *x = read_string(in), *y = جديد حرف[256], z[256];
    in > y > z;
    in.close();
    ٻاهر << x << سٽ_پڄاڻي << y << سٽ_پڄاڻي << z << سٽ_پڄاڻي;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << سٽ_پڄاڻي << y << سٽ_پڄاڻي << z << سٽ_پڄاڻي;
    out << z << سٽ_پڄاڻي << y << سٽ_پڄاڻي << x << سٽ_پڄاڻي;
    out << 1.1234567890123456789 << سٽ_پڄاڻي;

    ميٽيو[] x; ميٽيو[] y;
}
