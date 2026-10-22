/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#অন্তর্ভাব <limits.h>
#যদি defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    প্রকারলেখ signed অক্ষর i8_t;
    প্রকারলেখ unsigned অক্ষর ui8_t;
#অন্যথা
#   error "no 8-bit integer"
#endif
#যদি defined(USHRT_MAX) && USHRT_MAX == 0xffff
    প্রকারলেখ short i16_t;
    প্রকারলেখ unsigned short ui16_t;
#নতুবা_যদি defined(UINT_MAX) && UINT_MAX == 0xffff
    প্রকারলেখ পূর্ণসংখ্যা i16_t;
    প্রকারলেখ unsigned ui16_t;
#অন্যথা
#   error "no 16-bit integer"
#endif
#যদি defined(UINT_MAX) && UINT_MAX == 0xffffffff
    প্রকারলেখ পূর্ণসংখ্যা i32_t;
    প্রকারলেখ unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#নতুবা_যদি defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    প্রকারলেখ long i32_t;
    প্রকারলেখ unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#অন্যথা
#   error "no 32-bit integer"
#endif
#যদি defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    প্রকারলেখ long i64_t;
    প্রকারলেখ unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#নতুবা_যদি defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    প্রকারলেখ long long i64_t;
    প্রকারলেখ unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#নতুবা_যদি defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    প্রকারলেখ long long i64_t;
    প্রকারলেখ unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#অন্যথা
#   error "no 64-bit integer"
#endif
#endif
