#شامل "stdio.h"
#شامل "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
صحیح مین(خالی) {
    حرف hexStr[3]; // Two hex digits + null terminator
    صحیح ch;

    // Read characters until EOF
    جب((ch = getchar()) != EOF) {
        hexStr[0] = (حرف)ch;
        hexStr[1] = (حرف)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        حرف *endptr;
        غیرعلامتی حرف byte = (غیرعلامتی حرف)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        اگر((ch = getchar()) == EOF) // Read seaparating space
          توڑو;
    }

    واپس 0;
}
