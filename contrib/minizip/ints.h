/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଯଦି defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା signed ଅକ୍ଷର i8_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର ui8_t;
#ଅନ୍ୟଥା
#   error "no 8-bit integer"
#endif
#ଯଦି defined(USHRT_MAX) && USHRT_MAX == 0xffff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା short i16_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned short ui16_t;
#ନଚେତଯଦି defined(UINT_MAX) && UINT_MAX == 0xffff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା i16_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ui16_t;
#ଅନ୍ୟଥା
#   error "no 16-bit integer"
#endif
#ଯଦି defined(UINT_MAX) && UINT_MAX == 0xffffffff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା i32_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#ନଚେତଯଦି defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା long i32_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#ଅନ୍ୟଥା
#   error "no 32-bit integer"
#endif
#ଯଦି defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା long i64_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#ନଚେତଯଦି defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା long long i64_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ନଚେତଯଦି defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    ପ୍ରକାରବ୍ୟାଖ୍ୟା long long i64_t;
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ଅନ୍ୟଥା
#   error "no 64-bit integer"
#endif
#endif
