#شامل "stdio.h"
#شامل "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
صحیح اصلی(تهی) {
    نویسه hexStr[3]; // Two hex digits + null terminator
    صحیح ch;

    // Read characters until EOF
    وقتی((ch = getchar()) != EOF) {
        hexStr[0] = (نویسه)ch;
        hexStr[1] = (نویسه)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        نویسه *endptr;
        unsigned نویسه byte = (unsigned نویسه)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        اگر((ch = getchar()) == EOF) // Read seaparating space
          بشکن;
    }

    بازگشت 0;
}
