#அந்தர்பாவ "stdio.h"
#அந்தர்பாவ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
முழு முதன்மை(வெறுமை) {
    எழுத்து hexStr[3]; // Two hex digits + null terminator
    முழு ch;

    // Read characters until EOF
    போது((ch = getchar()) != EOF) {
        hexStr[0] = (எழுத்து)ch;
        hexStr[1] = (எழுத்து)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        எழுத்து *endptr;
        unsigned எழுத்து byte = (unsigned எழுத்து)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        எனில்((ch = getchar()) == EOF) // Read seaparating space
          நிறுத்து;
    }

    திரும்பு 0;
}
