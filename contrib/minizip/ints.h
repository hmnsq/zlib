/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#నిర్వచితం_కాకపోతే INTS_H
#నిర్వచించు INTS_H
#అన్తర్భావ <limits.h>
#ఒకవేళ defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    రకనిర్వచనం గుర్తుగల అక్షరం i8_t;
    రకనిర్వచనం గుర్తులేని అక్షరం ui8_t;
#లేకపోతే
#   error "no 8-bit integer"
#అయితే_ముగింపు
#ఒకవేళ defined(USHRT_MAX) && USHRT_MAX == 0xffff
    రకనిర్వచనం పొట్టి i16_t;
    రకనిర్వచనం గుర్తులేని పొట్టి ui16_t;
#లేదావేళ defined(UINT_MAX) && UINT_MAX == 0xffff
    రకనిర్వచనం పూర్ణాంకం i16_t;
    రకనిర్వచనం గుర్తులేని ui16_t;
#లేకపోతే
#   error "no 16-bit integer"
#అయితే_ముగింపు
#ఒకవేళ defined(UINT_MAX) && UINT_MAX == 0xffffffff
    రకనిర్వచనం పూర్ణాంకం i32_t;
    రకనిర్వచనం గుర్తులేని ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#లేదావేళ defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    రకనిర్వచనం దీర్ఘ i32_t;
    రకనిర్వచనం గుర్తులేని దీర్ఘ ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#లేకపోతే
#   error "no 32-bit integer"
#అయితే_ముగింపు
#ఒకవేళ defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    రకనిర్వచనం దీర్ఘ i64_t;
    రకనిర్వచనం గుర్తులేని దీర్ఘ ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#లేదావేళ defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    రకనిర్వచనం దీర్ఘ దీర్ఘ i64_t;
    రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#లేదావేళ defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    రకనిర్వచనం దీర్ఘ దీర్ఘ i64_t;
    రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#లేకపోతే
#   error "no 64-bit integer"
#అయితే_ముగింపు
#అయితే_ముగింపు
