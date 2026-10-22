/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#अन्तर्भाव <limits.h>
#जर defined(अच_अक्षर_उच्च) && अच_अक्षर_उच्च == 0xff
    प्रकारव्याख्या चिन्हित अक्षर i8_t;
    प्रकारव्याख्या अचिन्हित अक्षर ui8_t;
#अन्यथा
#   error "no 8-bit integer"
#endif
#जर defined(अच_लघु_उच्च) && अच_लघु_उच्च == 0xffff
    प्रकारव्याख्या लघु i16_t;
    प्रकारव्याख्या अचिन्हित लघु ui16_t;
#किंवाजर defined(अच_पूर्णांक_उच्च) && अच_पूर्णांक_उच्च == 0xffff
    प्रकारव्याख्या पूर्णांक i16_t;
    प्रकारव्याख्या अचिन्हित ui16_t;
#अन्यथा
#   error "no 16-bit integer"
#endif
#जर defined(अच_पूर्णांक_उच्च) && अच_पूर्णांक_उच्च == 0xffffffff
    प्रकारव्याख्या पूर्णांक i32_t;
    प्रकारव्याख्या अचिन्हित ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#किंवाजर defined(अच_दीर्घ_उच्च) && अच_दीर्घ_उच्च == 0xffffffff
    प्रकारव्याख्या दीर्घ i32_t;
    प्रकारव्याख्या अचिन्हित दीर्घ ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#अन्यथा
#   error "no 32-bit integer"
#endif
#जर defined(अच_दीर्घ_उच्च) && अच_दीर्घ_उच्च == 0xffffffffffffffff
    प्रकारव्याख्या दीर्घ i64_t;
    प्रकारव्याख्या अचिन्हित दीर्घ ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#किंवाजर defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    प्रकारव्याख्या दीर्घ दीर्घ i64_t;
    प्रकारव्याख्या अचिन्हित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#किंवाजर defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    प्रकारव्याख्या दीर्घ दीर्घ i64_t;
    प्रकारव्याख्या अचिन्हित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#अन्यथा
#   error "no 64-bit integer"
#endif
#endif
