#అన్తర్భావ "stdio.h"
#అన్తర్భావ "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
పూర్ణాంకం ప్రధాన(శూన్య) {
    అక్షరం hexStr[3]; // Two hex digits + null terminator
    పూర్ణాంకం ch;

    // Read characters until EOF
    అయితే((ch = getchar()) != EOF) {
        hexStr[0] = (అక్షరం)ch;
        hexStr[1] = (అక్షరం)getchar();
        hexStr[2] = '\0'; // Null-terminate string
        అక్షరం *endptr;
        గుర్తులేని అక్షరం byte = (గుర్తులేని అక్షరం)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        ఒకవేళ((ch = getchar()) == EOF) // Read seaparating space
          ఆపు;
    }

    తిరిగి 0;
}
