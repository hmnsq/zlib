#ਅਨ੍ਤਰ੍ਭਾਵ "zstream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <iomanip.h>

ਖਾਲੀ ਮੁੱਖ() {
    ਅੱਖਰ h[256] = "Hello";
    ਅੱਖਰ* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    ਅੱਖਰ *x = read_string(in), *y = ਨਵਾਂ ਅੱਖਰ[256], z[256];
    in > y > z;
    in.close();
    ਬਾਹਰ << x << ਸਤਰ_ਅੰਤ << y << ਸਤਰ_ਅੰਤ << z << ਸਤਰ_ਅੰਤ;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << ਸਤਰ_ਅੰਤ << y << ਸਤਰ_ਅੰਤ << z << ਸਤਰ_ਅੰਤ;
    out << z << ਸਤਰ_ਅੰਤ << y << ਸਤਰ_ਅੰਤ << x << ਸਤਰ_ਅੰਤ;
    out << 1.1234567890123456789 << ਸਤਰ_ਅੰਤ;

    ਮਿਟਾਓ[] x; ਮਿਟਾਓ[] y;
}
