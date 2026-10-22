/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#अन्तर्भाव <limits.h>
#जइ defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    प्रकारनिर्देश signed वर्ण i8_t;
    प्रकारनिर्देश unsigned वर्ण ui8_t;
#अण्णहा
#   error "no 8-bit integer"
#endif
#जइ defined(USHRT_MAX) && USHRT_MAX == 0xffff
    प्रकारनिर्देश short i16_t;
    प्रकारनिर्देश unsigned short ui16_t;
#नोचेद्यदि defined(UINT_MAX) && UINT_MAX == 0xffff
    प्रकारनिर्देश पूर्णाङ्क i16_t;
    प्रकारनिर्देश unsigned ui16_t;
#अण्णहा
#   error "no 16-bit integer"
#endif
#जइ defined(UINT_MAX) && UINT_MAX == 0xffffffff
    प्रकारनिर्देश पूर्णाङ्क i32_t;
    प्रकारनिर्देश unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#नोचेद्यदि defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    प्रकारनिर्देश long i32_t;
    प्रकारनिर्देश unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#अण्णहा
#   error "no 32-bit integer"
#endif
#जइ defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश long i64_t;
    प्रकारनिर्देश unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#नोचेद्यदि defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश long long i64_t;
    प्रकारनिर्देश unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#नोचेद्यदि defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश long long i64_t;
    प्रकारनिर्देश unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#अण्णहा
#   error "no 64-bit integer"
#endif
#endif
