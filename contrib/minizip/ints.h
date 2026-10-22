/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#اگر_تعریف_نشده INTS_H
#تعریف_کن INTS_H
#ضمن <limits.h>
#إذا defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    تعريفنوع علامتدار حرف i8_t;
    تعريفنوع بیعلامت حرف ui8_t;
#وإلا
#   error "no 8-bit integer"
#پایان_اگر
#إذا defined(USHRT_MAX) && USHRT_MAX == 0xffff
    تعريفنوع کوتاه i16_t;
    تعريفنوع بیعلامت کوتاه ui16_t;
#وإلاإذا defined(UINT_MAX) && UINT_MAX == 0xffff
    تعريفنوع صحيح i16_t;
    تعريفنوع بیعلامت ui16_t;
#وإلا
#   error "no 16-bit integer"
#پایان_اگر
#إذا defined(UINT_MAX) && UINT_MAX == 0xffffffff
    تعريفنوع صحيح i32_t;
    تعريفنوع بیعلامت ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#وإلاإذا defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    تعريفنوع دراز i32_t;
    تعريفنوع بیعلامت دراز ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#وإلا
#   error "no 32-bit integer"
#پایان_اگر
#إذا defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    تعريفنوع دراز i64_t;
    تعريفنوع بیعلامت دراز ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#وإلاإذا defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    تعريفنوع دراز دراز i64_t;
    تعريفنوع بیعلامت دراز دراز ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وإلاإذا defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    تعريفنوع دراز دراز i64_t;
    تعريفنوع بیعلامت دراز دراز ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وإلا
#   error "no 64-bit integer"
#پایان_اگر
#پایان_اگر
