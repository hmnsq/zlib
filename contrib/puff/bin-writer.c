#include "stdio.h"
#include "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
整数 主要(void) {
    char hexStr[3]; // Two hex digits + null terminator
    整数 ch;

    // Read characters until EOF
    当((ch = getchar()) != EOF) {
        hexStr[0] = (char)ch;
        hexStr[1] = (char)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        char *endptr;
        unsigned char byte = (unsigned char)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        如果((ch = getchar()) == EOF) // Read seaparating space
          break;
    }

    返回 0;
}
