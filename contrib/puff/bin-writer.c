#ضمن "stdio.h"
#ضمن "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
صحيح main(فراغ) {
    حرف hexStr[3]; // Two hex digits + null terminator
    صحيح ch;

    // Read characters until EOF
    طالما((ch = getchar()) != EOF) {
        hexStr[0] = (حرف)ch;
        hexStr[1] = (حرف)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        حرف *endptr;
        unsigned حرف byte = (unsigned حرف)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        إذا((ch = getchar()) == EOF) // Read seaparating space
          اكسر;
    }

    أرجع 0;
}
