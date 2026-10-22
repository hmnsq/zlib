#अन्तर्भाव "stdio.h"
#अन्तर्भाव "stdlib.h"

/**
 * Reads hexadecimal values from stdin and writes binary bytes to stdout.
 * Accepts hex values separated by spaces, newlines, commas, etc.
 * Handles both uppercase and lowercase hex digits.
 */
पूर्णांक main(शून्य) {
    अक्षर hexStr[3]; // Two hex digits + null terminator
    पूर्णांक ch;

    // Read characters until EOF
    पर्यंत((ch = अक्षर_लो()) != खातापूर्ण) {
        hexStr[0] = (अक्षर)ch;
        hexStr[1] = (अक्षर)अक्षर_लो();
        hexStr[2] = '\0'; // Null-terminate string
        अक्षर *endptr;
        अचिन्हित अक्षर byte = (अचिन्हित अक्षर)म_से_दीर्घ(hexStr, &endptr, 16);
        ख_डालो(&byte, 1, 1, मानक_निकास);
        जर((ch = अक्षर_लो()) == खातापूर्ण) // Read seaparating space
          थांबा;
    }

    परत 0;
}
