#अन्तर्भाव "stdio.h"
#अन्तर्भाव "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
पूर्णाङ्क मुख्यम्(शून्य) {
    वर्ण hexStr[3]; // Two hex digits + null terminator
    पूर्णाङ्क ch;

    // Read characters until EOF
    यावत्((ch = अक्षरं_पठ()) != EOF) {
        hexStr[0] = (वर्ण)ch;
        hexStr[1] = (वर्ण)अक्षरं_पठ();
        hexStr[2] = '\0'; // Null-terminate string
        वर्ण *endptr;
        अचिह्नित वर्ण byte = (अचिह्नित वर्ण)strtol(hexStr, &endptr, 16);
        fwrite(&byte, 1, 1, stdout);
        यदि((ch = अक्षरं_पठ()) == EOF) // Read seaparating space
          भङ्ग;
    }

    प्रत्यागम 0;
}
