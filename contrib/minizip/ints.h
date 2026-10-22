/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#ضمن <limits.h>
#ܐܢ defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    تعريفنوع signed حرف i8_t;
    تعريفنوع unsigned حرف ui8_t;
#ܐܚܪܢܐ
#   error "no 8-bit integer"
#endif
#ܐܢ defined(USHRT_MAX) && USHRT_MAX == 0xffff
    تعريفنوع short i16_t;
    تعريفنوع unsigned short ui16_t;
#وإلاإذا defined(UINT_MAX) && UINT_MAX == 0xffff
    تعريفنوع صحيح i16_t;
    تعريفنوع unsigned ui16_t;
#ܐܚܪܢܐ
#   error "no 16-bit integer"
#endif
#ܐܢ defined(UINT_MAX) && UINT_MAX == 0xffffffff
    تعريفنوع صحيح i32_t;
    تعريفنوع unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#وإلاإذا defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    تعريفنوع long i32_t;
    تعريفنوع unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#ܐܚܪܢܐ
#   error "no 32-bit integer"
#endif
#ܐܢ defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    تعريفنوع long i64_t;
    تعريفنوع unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#وإلاإذا defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    تعريفنوع long long i64_t;
    تعريفنوع unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وإلاإذا defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    تعريفنوع long long i64_t;
    تعريفنوع unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ܐܚܪܢܐ
#   error "no 64-bit integer"
#endif
#endif
