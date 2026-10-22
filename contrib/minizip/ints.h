/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#અન્તર્ભાવ <limits.h>
#જો defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    પ્રકારવ્યાખ્યા signed અક્ષર i8_t;
    પ્રકારવ્યાખ્યા unsigned અક્ષર ui8_t;
#નહીંતર
#   error "no 8-bit integer"
#endif
#જો defined(USHRT_MAX) && USHRT_MAX == 0xffff
    પ્રકારવ્યાખ્યા short i16_t;
    પ્રકારવ્યાખ્યા unsigned short ui16_t;
#અથવાજો defined(UINT_MAX) && UINT_MAX == 0xffff
    પ્રકારવ્યાખ્યા પૂર્ણાંક i16_t;
    પ્રકારવ્યાખ્યા unsigned ui16_t;
#નહીંતર
#   error "no 16-bit integer"
#endif
#જો defined(UINT_MAX) && UINT_MAX == 0xffffffff
    પ્રકારવ્યાખ્યા પૂર્ણાંક i32_t;
    પ્રકારવ્યાખ્યા unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#અથવાજો defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    પ્રકારવ્યાખ્યા long i32_t;
    પ્રકારવ્યાખ્યા unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#નહીંતર
#   error "no 32-bit integer"
#endif
#જો defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    પ્રકારવ્યાખ્યા long i64_t;
    પ્રકારવ્યાખ્યા unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#અથવાજો defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    પ્રકારવ્યાખ્યા long long i64_t;
    પ્રકારવ્યાખ્યા unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#અથવાજો defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    પ્રકારવ્યાખ્યા long long i64_t;
    પ્રકારવ્યાખ્યા unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#નહીંતર
#   error "no 64-bit integer"
#endif
#endif
