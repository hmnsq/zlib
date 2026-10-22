/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#यदि_अपरिभाषितम् INTS_H
#परिभाषय INTS_H
#अन्तर्भाव <limits.h>
#यदि defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    प्रकारनिर्देश चिह्नित वर्ण i8_t;
    प्रकारनिर्देश अचिह्नित वर्ण ui8_t;
#अन्यथा
#   error "no 8-bit integer"
#यदि_अन्तः
#यदि defined(USHRT_MAX) && USHRT_MAX == 0xffff
    प्रकारनिर्देश ह्रस्व i16_t;
    प्रकारनिर्देश अचिह्नित ह्रस्व ui16_t;
#नोचेद्यदि defined(UINT_MAX) && UINT_MAX == 0xffff
    प्रकारनिर्देश पूर्णाङ्क i16_t;
    प्रकारनिर्देश अचिह्नित ui16_t;
#अन्यथा
#   error "no 16-bit integer"
#यदि_अन्तः
#यदि defined(UINT_MAX) && UINT_MAX == 0xffffffff
    प्रकारनिर्देश पूर्णाङ्क i32_t;
    प्रकारनिर्देश अचिह्नित ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#नोचेद्यदि defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    प्रकारनिर्देश दीर्घ i32_t;
    प्रकारनिर्देश अचिह्नित दीर्घ ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#अन्यथा
#   error "no 32-bit integer"
#यदि_अन्तः
#यदि defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश दीर्घ i64_t;
    प्रकारनिर्देश अचिह्नित दीर्घ ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#नोचेद्यदि defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश दीर्घ दीर्घ i64_t;
    प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#नोचेद्यदि defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    प्रकारनिर्देश दीर्घ दीर्घ i64_t;
    प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#अन्यथा
#   error "no 64-bit integer"
#यदि_अन्तः
#यदि_अन्तः
