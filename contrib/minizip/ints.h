/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#अन्तर्भाव <limits.h>
#यदि defined(अच_अक्षर_उच्च) && अच_अक्षर_उच्च == 0xff
    प्रकारनिर्देश चिन्हित वर्ण i8_t;
    प्रकारनिर्देश अचिन्हित वर्ण ui8_t;
#अन्यथा
#   error "no 8-bit integer"
#endif
#यदि defined(अच_लघु_उच्च) && अच_लघु_उच्च == 0xffff
    प्रकारनिर्देश लघु i16_t;
    प्रकारनिर्देश अचिन्हित लघु ui16_t;
#अथवायदि defined(अच_पूर्णांक_उच्च) && अच_पूर्णांक_उच्च == 0xffff
    प्रकारनिर्देश पूर्णाङ्क i16_t;
    प्रकारनिर्देश अचिन्हित ui16_t;
#अन्यथा
#   error "no 16-bit integer"
#endif
#यदि defined(अच_पूर्णांक_उच्च) && अच_पूर्णांक_उच्च == 0xffffffff
    प्रकारनिर्देश पूर्णाङ्क i32_t;
    प्रकारनिर्देश अचिन्हित ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#अथवायदि defined(अच_दीर्घ_उच्च) && अच_दीर्घ_उच्च == 0xffffffff
    प्रकारनिर्देश दीर्घ i32_t;
    प्रकारनिर्देश अचिन्हित दीर्घ ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#अन्यथा
#   error "no 32-bit integer"
#endif
#यदि defined(अच_दीर्घ_उच्च) && अच_दीर्घ_उच्च == 0xffffffffffffffff
    प्रकारनिर्देश दीर्घ i64_t;
    प्रकारनिर्देश अचिन्हित दीर्घ ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#अथवायदि defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश दीर्घ दीर्घ i64_t;
    प्रकारनिर्देश अचिन्हित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#अथवायदि defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश दीर्घ दीर्घ i64_t;
    प्रकारनिर्देश अचिन्हित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#अन्यथा
#   error "no 64-bit integer"
#endif
#endif
