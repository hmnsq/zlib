/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#include <limits.h>
#如果 defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    typedef signed char i8_t;
    typedef unsigned char ui8_t;
#否则
#   error "no 8-bit integer"
#endif
#如果 defined(USHRT_MAX) && USHRT_MAX == 0xffff
    typedef short i16_t;
    typedef unsigned short ui16_t;
#elif defined(UINT_MAX) && UINT_MAX == 0xffff
    typedef 整数 i16_t;
    typedef unsigned ui16_t;
#否则
#   error "no 16-bit integer"
#endif
#如果 defined(UINT_MAX) && UINT_MAX == 0xffffffff
    typedef 整数 i32_t;
    typedef unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#elif defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    typedef long i32_t;
    typedef unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#否则
#   error "no 32-bit integer"
#endif
#如果 defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    typedef long i64_t;
    typedef unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#elif defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    typedef long long i64_t;
    typedef unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#elif defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    typedef long long i64_t;
    typedef unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#否则
#   error "no 64-bit integer"
#endif
#endif
