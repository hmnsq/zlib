/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಯದಿ defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    ಪ್ರಕಾರನಿರ್ದೇಶ signed ವರ್ಣ i8_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ವರ್ಣ ui8_t;
#ಅನ್ಯಥಾ
#   error "no 8-bit integer"
#endif
#ಯದಿ defined(USHRT_MAX) && USHRT_MAX == 0xffff
    ಪ್ರಕಾರನಿರ್ದೇಶ short i16_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned short ui16_t;
#ನೋಚೇದ್ಯದಿ defined(UINT_MAX) && UINT_MAX == 0xffff
    ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ i16_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ui16_t;
#ಅನ್ಯಥಾ
#   error "no 16-bit integer"
#endif
#ಯದಿ defined(UINT_MAX) && UINT_MAX == 0xffffffff
    ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ i32_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#ನೋಚೇದ್ಯದಿ defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    ಪ್ರಕಾರನಿರ್ದೇಶ long i32_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#ಅನ್ಯಥಾ
#   error "no 32-bit integer"
#endif
#ಯದಿ defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    ಪ್ರಕಾರನಿರ್ದೇಶ long i64_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#ನೋಚೇದ್ಯದಿ defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    ಪ್ರಕಾರನಿರ್ದೇಶ long long i64_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ನೋಚೇದ್ಯದಿ defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    ಪ್ರಕಾರನಿರ್ದೇಶ long long i64_t;
    ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ಅನ್ಯಥಾ
#   error "no 64-bit integer"
#endif
#endif
