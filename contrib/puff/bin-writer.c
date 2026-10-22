#अन्तर्भाव "stdio.h"
#अन्तर्भाव "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
पूर्णाङ्क main(शून्य) {
    वर्ण hexStr[3]; // Two hex digits + null terminator
    पूर्णाङ्क ch;

    // Read characters until EOF
    यावत्((ch = अक्षर_लो()) != खातापूर्ण) {
        hexStr[0] = (वर्ण)ch;
        hexStr[1] = (वर्ण)अक्षर_लो();
        hexStr[2] = '\0'; // Null-terminate string
        वर्ण *endptr;
        अचिन्हित वर्ण byte = (अचिन्हित वर्ण)म_से_दीर्घ(hexStr, &endptr, 16);
        ख_डालो(&byte, 1, 1, मानक_निकास);
        यदि((ch = अक्षर_लो()) == खातापूर्ण) // Read seaparating space
          भङ्ग;
    }

    प्रत्यागम 0;
}
