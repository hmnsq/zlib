#অন্তর্ভাব "stdio.h"
#অন্তর্ভাব "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
পূর্ণসংখ্যা প্রধান(শূন্য) {
    অক্ষর hexStr[3]; // Two hex digits + null terminator
    পূর্ণসংখ্যা ch;

    // Read characters until EOF
    যখন((ch = getchar()) != EOF) {
        hexStr[0] = (অক্ষর)ch;
        hexStr[1] = (অক্ষর)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        অক্ষর *endptr;
        unsigned অক্ষর byte = (unsigned অক্ষর)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        যদি((ch = getchar()) == EOF) // Read seaparating space
          বিরতি;
    }

    ফেরত 0;
}
