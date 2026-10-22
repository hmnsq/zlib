#অন্তর্ভাৱ "stdio.h"
#অন্তর্ভাৱ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
পূর্ণাঙ্ক main(শূন্য) {
    ৱর্ণ hexStr[3]; // Two hex digits + null terminator
    পূর্ণাঙ্ক ch;

    // Read characters until EOF
    যেতিয়ালৈ((ch = getchar()) != EOF) {
        hexStr[0] = (ৱর্ণ)ch;
        hexStr[1] = (ৱর্ণ)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        ৱর্ণ *endptr;
        unsigned ৱর্ণ byte = (unsigned ৱর্ণ)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        যদি((ch = getchar()) == EOF) // Read seaparating space
          ভঙ্গ;
    }

    ঘূৰাই 0;
}
