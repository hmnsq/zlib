/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INTS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INTS_H
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਜੇ defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ i8_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ui8_t;
#ਨਹੀਂਤਾਂ
#   error "no 8-bit integer"
#ਜੇ_ਅੰਤ
#ਜੇ defined(USHRT_MAX) && USHRT_MAX == 0xffff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਛੋਟਾ i16_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ui16_t;
#ਜਾਂਜੇ defined(UINT_MAX) && UINT_MAX == 0xffff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ i16_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ui16_t;
#ਨਹੀਂਤਾਂ
#   error "no 16-bit integer"
#ਜੇ_ਅੰਤ
#ਜੇ defined(UINT_MAX) && UINT_MAX == 0xffffffff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ i32_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#ਜਾਂਜੇ defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ i32_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#ਨਹੀਂਤਾਂ
#   error "no 32-bit integer"
#ਜੇ_ਅੰਤ
#ਜੇ defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ i64_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#ਜਾਂਜੇ defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ ਲੰਮਾ i64_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ਜਾਂਜੇ defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ ਲੰਮਾ i64_t;
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ਨਹੀਂਤਾਂ
#   error "no 64-bit integer"
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
