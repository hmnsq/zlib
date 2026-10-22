#include "stdio.h"
#include "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
ཧྲིལ main(void) {
    var_na hexStr[3]; // Two hex digits + null terminator
    ཧྲིལ ch;

    // Read characters until EOF
    སྐབས((ch = getchar()) != EOF) {
        hexStr[0] = (var_na)ch;
        hexStr[1] = (var_na)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        var_na *endptr;
        unsigned var_na byte = (unsigned var_na)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        གལ་སྲིད((ch = getchar()) == EOF) // Read seaparating space
          bha_nga;
    }

    ལོག 0;
}
