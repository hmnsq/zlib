/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#شامل <limits.h>
#اگر defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    تعریفنوع signed نویسه i8_t;
    تعریفنوع unsigned نویسه ui8_t;
#وگرنه
#   error "no 8-bit integer"
#endif
#اگر defined(USHRT_MAX) && USHRT_MAX == 0xffff
    تعریفنوع short i16_t;
    تعریفنوع unsigned short ui16_t;
#وگرنهاگر defined(UINT_MAX) && UINT_MAX == 0xffff
    تعریفنوع صحیح i16_t;
    تعریفنوع unsigned ui16_t;
#وگرنه
#   error "no 16-bit integer"
#endif
#اگر defined(UINT_MAX) && UINT_MAX == 0xffffffff
    تعریفنوع صحیح i32_t;
    تعریفنوع unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#وگرنهاگر defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    تعریفنوع long i32_t;
    تعریفنوع unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#وگرنه
#   error "no 32-bit integer"
#endif
#اگر defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    تعریفنوع long i64_t;
    تعریفنوع unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#وگرنهاگر defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    تعریفنوع long long i64_t;
    تعریفنوع unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وگرنهاگر defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    تعریفنوع long long i64_t;
    تعریفنوع unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وگرنه
#   error "no 64-bit integer"
#endif
#endif
