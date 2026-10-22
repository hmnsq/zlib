#અન્તર્ભાવ "stdio.h"
#અન્તર્ભાવ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
પૂર્ણાંક મુખ્ય(શૂન્ય) {
    અક્ષર hexStr[3]; // Two hex digits + null terminator
    પૂર્ણાંક ch;

    // Read characters until EOF
    જ્યારે((ch = getchar()) != EOF) {
        hexStr[0] = (અક્ષર)ch;
        hexStr[1] = (અક્ષર)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        અક્ષર *endptr;
        unsigned અક્ષર byte = (unsigned અક્ષર)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        જો((ch = getchar()) == EOF) // Read seaparating space
          તોડો;
    }

    પાછા 0;
}
