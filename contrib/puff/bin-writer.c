#അന്തര്ഭാവ "stdio.h"
#അന്തര്ഭാവ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം) {
    അക്ഷരം hexStr[3]; // Two hex digits + null terminator
    പൂർണ്ണസംഖ്യ ch;

    // Read characters until EOF
    ആകുമ്പോൾ((ch = getchar()) != EOF) {
        hexStr[0] = (അക്ഷരം)ch;
        hexStr[1] = (അക്ഷരം)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        അക്ഷരം *endptr;
        unsigned അക്ഷരം byte = (unsigned അക്ഷരം)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        എങ്കിൽ((ch = getchar()) == EOF) // Read seaparating space
          നിർത്തുക;
    }

    തിരികെ 0;
}
