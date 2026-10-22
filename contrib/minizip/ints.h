/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#অন্তর্ভাৱ <limits.h>
#যদি defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    প্রকারনির্দেশ signed ৱর্ণ i8_t;
    প্রকারনির্দেশ unsigned ৱর্ণ ui8_t;
#অন্যথা
#   error "no 8-bit integer"
#endif
#যদি defined(USHRT_MAX) && USHRT_MAX == 0xffff
    প্রকারনির্দেশ short i16_t;
    প্রকারনির্দেশ unsigned short ui16_t;
#নোচেদ্যদি defined(UINT_MAX) && UINT_MAX == 0xffff
    প্রকারনির্দেশ পূর্ণাঙ্ক i16_t;
    প্রকারনির্দেশ unsigned ui16_t;
#অন্যথা
#   error "no 16-bit integer"
#endif
#যদি defined(UINT_MAX) && UINT_MAX == 0xffffffff
    প্রকারনির্দেশ পূর্ণাঙ্ক i32_t;
    প্রকারনির্দেশ unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#নোচেদ্যদি defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    প্রকারনির্দেশ long i32_t;
    প্রকারনির্দেশ unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#অন্যথা
#   error "no 32-bit integer"
#endif
#যদি defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    প্রকারনির্দেশ long i64_t;
    প্রকারনির্দেশ unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#নোচেদ্যদি defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    প্রকারনির্দেশ long long i64_t;
    প্রকারনির্দেশ unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#নোচেদ্যদি defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    প্রকারনির্দেশ long long i64_t;
    প্রকারনির্দেশ unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#অন্যথা
#   error "no 64-bit integer"
#endif
#endif
