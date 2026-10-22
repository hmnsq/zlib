/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#அந்தர்பாவ <limits.h>
#எனில் defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    வகைவரை signed எழுத்து i8_t;
    வகைவரை unsigned எழுத்து ui8_t;
#இல்லையெனில்
#   error "no 8-bit integer"
#endif
#எனில் defined(USHRT_MAX) && USHRT_MAX == 0xffff
    வகைவரை short i16_t;
    வகைவரை unsigned short ui16_t;
#அல்லதெனில் defined(UINT_MAX) && UINT_MAX == 0xffff
    வகைவரை முழு i16_t;
    வகைவரை unsigned ui16_t;
#இல்லையெனில்
#   error "no 16-bit integer"
#endif
#எனில் defined(UINT_MAX) && UINT_MAX == 0xffffffff
    வகைவரை முழு i32_t;
    வகைவரை unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#அல்லதெனில் defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    வகைவரை long i32_t;
    வகைவரை unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#இல்லையெனில்
#   error "no 32-bit integer"
#endif
#எனில் defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    வகைவரை long i64_t;
    வகைவரை unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#அல்லதெனில் defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    வகைவரை long long i64_t;
    வகைவரை unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#அல்லதெனில் defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    வகைவரை long long i64_t;
    வகைவரை unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#இல்லையெனில்
#   error "no 64-bit integer"
#endif
#endif
