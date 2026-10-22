#ଅନ୍ତର୍ଭାଵ "stdio.h"
#ଅନ୍ତର୍ଭାଵ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ଶୂନ୍ୟ) {
    ଅକ୍ଷର hexStr[3]; // Two hex digits + null terminator
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

    // Read characters until EOF
    ଯେତେବେଳେ((ch = getchar()) != EOF) {
        hexStr[0] = (ଅକ୍ଷର)ch;
        hexStr[1] = (ଅକ୍ଷର)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        ଅକ୍ଷର *endptr;
        unsigned ଅକ୍ଷର byte = (unsigned ଅକ୍ଷର)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        ଯଦି((ch = getchar()) == EOF) // Read seaparating space
          ଭାଙ୍ଗ;
    }

    ଫେରନ୍ତୁ 0;
}
