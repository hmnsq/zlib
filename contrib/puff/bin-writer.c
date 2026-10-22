#অন্তর্ভাব "stdio.h"
#অন্তর্ভাব "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
পূর্ণাঙ্ক main(শূন্য) {
    বর্ণ hexStr[3]; // Two hex digits + null terminator
    পূর্ণাঙ্ক ch;

    // Read characters until EOF
    যাবত্((ch = getchar()) != EOF) {
        hexStr[0] = (বর্ণ)ch;
        hexStr[1] = (বর্ণ)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        বর্ণ *endptr;
        unsigned বর্ণ byte = (unsigned বর্ণ)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        যদি((ch = getchar()) == EOF) // Read seaparating space
          ভঙ্গ;
    }

    প্রত্যাগম 0;
}
