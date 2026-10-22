/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#अगर_अघोषित INTS_H
#घोषणा INTS_H
#समावेश <limits.h>
#अगर defined(अच_अक्षर_उच्च) && अच_अक्षर_उच्च == 0xff
    प्रकार चिन्हित अक्षर i8_t;
    प्रकार अचिन्हित अक्षर ui8_t;
#अन्यथा
#   error "no 8-bit integer"
#पूर्ण_अगर
#अगर defined(अच_लघु_उच्च) && अच_लघु_उच्च == 0xffff
    प्रकार लघु i16_t;
    प्रकार अचिन्हित लघु ui16_t;
#या_अगर defined(अच_पूर्णांक_उच्च) && अच_पूर्णांक_उच्च == 0xffff
    प्रकार पूर्णांक i16_t;
    प्रकार अचिन्हित ui16_t;
#अन्यथा
#   error "no 16-bit integer"
#पूर्ण_अगर
#अगर defined(अच_पूर्णांक_उच्च) && अच_पूर्णांक_उच्च == 0xffffffff
    प्रकार पूर्णांक i32_t;
    प्रकार अचिन्हित ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#या_अगर defined(अच_दीर्घ_उच्च) && अच_दीर्घ_उच्च == 0xffffffff
    प्रकार दीर्घ i32_t;
    प्रकार अचिन्हित दीर्घ ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#अन्यथा
#   error "no 32-bit integer"
#पूर्ण_अगर
#अगर defined(अच_दीर्घ_उच्च) && अच_दीर्घ_उच्च == 0xffffffffffffffff
    प्रकार दीर्घ i64_t;
    प्रकार अचिन्हित दीर्घ ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#या_अगर defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    प्रकार दीर्घ दीर्घ i64_t;
    प्रकार अचिन्हित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#या_अगर defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    प्रकार दीर्घ दीर्घ i64_t;
    प्रकार अचिन्हित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#अन्यथा
#   error "no 64-bit integer"
#पूर्ण_अगर
#पूर्ण_अगर
