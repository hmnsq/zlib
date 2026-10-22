#ಅನ್ತರ್ಭಾವ "zstream.h"
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <iomanip.h>

ಶೂನ್ಯ ಮುಖ್ಯ() {
    ಅಕ್ಷರ h[256] = "Hello";
    ಅಕ್ಷರ* g = "Goodbye";
    ozstream out("temp.gz");
    out < "This works well" < h < g;
    out.close();

    izstream in("temp.gz"); // read it back
    ಅಕ್ಷರ *x = read_string(in), *y = ಹೊಸ ಅಕ್ಷರ[256], z[256];
    in > y > z;
    in.close();
    ಹೊರಗೆ << x << ಸಾಲು_ಅಂತ್ಯ << y << ಸಾಲು_ಅಂತ್ಯ << z << ಸಾಲು_ಅಂತ್ಯ;

    out.open("temp.gz"); // try ascii output; zcat temp.gz to see the results
    out << setw(50) << setfill('#') << setprecision(20) << x << ಸಾಲು_ಅಂತ್ಯ << y << ಸಾಲು_ಅಂತ್ಯ << z << ಸಾಲು_ಅಂತ್ಯ;
    out << z << ಸಾಲು_ಅಂತ್ಯ << y << ಸಾಲು_ಅಂತ್ಯ << x << ಸಾಲು_ಅಂತ್ಯ;
    out << 1.1234567890123456789 << ಸಾಲು_ಅಂತ್ಯ;

    ಅಳಿಸು[] x; ಅಳಿಸು[] y;
}
