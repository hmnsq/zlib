/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#اگر_غیر_تعریف_شدہ INTS_H
#تعریف_کرو INTS_H
#شامل <limits.h>
#اگر defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    قسمتعریف علامتی حرف i8_t;
    قسمتعریف غیرعلامتی حرف ui8_t;
#ورنہ
#   error "no 8-bit integer"
#اگر_ختم
#اگر defined(USHRT_MAX) && USHRT_MAX == 0xffff
    قسمتعریف مختصر i16_t;
    قسمتعریف غیرعلامتی مختصر ui16_t;
#وگرنہ_اگر defined(UINT_MAX) && UINT_MAX == 0xffff
    قسمتعریف صحیح i16_t;
    قسمتعریف غیرعلامتی ui16_t;
#ورنہ
#   error "no 16-bit integer"
#اگر_ختم
#اگر defined(UINT_MAX) && UINT_MAX == 0xffffffff
    قسمتعریف صحیح i32_t;
    قسمتعریف غیرعلامتی ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#وگرنہ_اگر defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    قسمتعریف طویل i32_t;
    قسمتعریف غیرعلامتی طویل ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#ورنہ
#   error "no 32-bit integer"
#اگر_ختم
#اگر defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    قسمتعریف طویل i64_t;
    قسمتعریف غیرعلامتی طویل ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#وگرنہ_اگر defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    قسمتعریف طویل طویل i64_t;
    قسمتعریف غیرعلامتی طویل طویل ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وگرنہ_اگر defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    قسمتعریف طویل طویل i64_t;
    قسمتعریف غیرعلامتی طویل طویل ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ورنہ
#   error "no 64-bit integer"
#اگر_ختم
#اگر_ختم
