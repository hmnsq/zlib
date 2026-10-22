#ಅನ್ತರ್ಭಾವ "stdio.h"
#ಅನ್ತರ್ಭಾವ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
ಪೂರ್ಣಾಙ್ಕ main(ಶೂನ್ಯ) {
    ವರ್ಣ hexStr[3]; // Two hex digits + null terminator
    ಪೂರ್ಣಾಙ್ಕ ch;

    // Read characters until EOF
    ಯಾವತ್((ch = getchar()) != EOF) {
        hexStr[0] = (ವರ್ಣ)ch;
        hexStr[1] = (ವರ್ಣ)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        ವರ್ಣ *endptr;
        unsigned ವರ್ಣ byte = (unsigned ವರ್ಣ)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        ಯದಿ((ch = getchar()) == EOF) // Read seaparating space
          ಭಙ್ಗ;
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}
