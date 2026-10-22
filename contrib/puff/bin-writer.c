#ಅನ್ತರ್ಭಾವ "stdio.h"
#ಅನ್ತರ್ಭಾವ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ) {
    ಅಕ್ಷರ hexStr[3]; // Two hex digits + null terminator
    ಪೂರ್ಣಾಂಕ ch;

    // Read characters until EOF
    ಆಗ((ch = getchar()) != EOF) {
        hexStr[0] = (ಅಕ್ಷರ)ch;
        hexStr[1] = (ಅಕ್ಷರ)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        ಅಕ್ಷರ *endptr;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ byte = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        ಆದರೆ((ch = getchar()) == EOF) // Read seaparating space
          ನಿಲ್ಲಿಸು;
    }

    ಹಿಂತಿರುಗಿ 0;
}
