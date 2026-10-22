#ਅਨ੍ਤਰ੍ਭਾਵ "stdio.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ) {
    ਅੱਖਰ hexStr[3]; // Two hex digits + null terminator
    ਪੂਰਨ ch;

    // Read characters until EOF
    ਜਦੋਂ((ch = getchar()) != EOF) {
        hexStr[0] = (ਅੱਖਰ)ch;
        hexStr[1] = (ਅੱਖਰ)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        ਅੱਖਰ *endptr;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ byte = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        ਜੇ((ch = getchar()) == EOF) // Read seaparating space
          ਰੋਕੋ;
    }

    ਵਾਪਸ 0;
}
