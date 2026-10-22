#అన్తర్భావ "zstream.h"
#అన్తర్భావ <math.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <iomanip.h>

శూన్య ప్రధాన() {
    అక్షరం h[256] = "Hello";
    అక్షరం* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    అక్షరం *x = read_string(in), *y = కొత్త అక్షరం[256], z[256];
    in > y > z;
    in.close();
    బయటకు << x << పంక్తి_ముగింపు << y << పంక్తి_ముగింపు << z << పంక్తి_ముగింపు;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << పంక్తి_ముగింపు << y << పంక్తి_ముగింపు << z << పంక్తి_ముగింపు;
    out << z << పంక్తి_ముగింపు << y << పంక్తి_ముగింపు << x << పంక్తి_ముగింపు;
    out << 1.1234567890123456789 << పంక్తి_ముగింపు;

    తొలగించు[] x; తొలగించు[] y;
}
