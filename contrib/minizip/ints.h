/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ INTS_H
#ವ್ಯಾಖ್ಯಾನಿಸು INTS_H
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಆದರೆ defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ i8_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ui8_t;
#ಇಲ್ಲದಿದ್ದರೆ
#   error "no 8-bit integer"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(USHRT_MAX) && USHRT_MAX == 0xffff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಗಿಡ್ಡ i16_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ui16_t;
#ಅಥವಾಆದರೆ defined(UINT_MAX) && UINT_MAX == 0xffff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ i16_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ui16_t;
#ಇಲ್ಲದಿದ್ದರೆ
#   error "no 16-bit integer"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(UINT_MAX) && UINT_MAX == 0xffffffff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ i32_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#ಅಥವಾಆದರೆ defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ i32_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#ಇಲ್ಲದಿದ್ದರೆ
#   error "no 32-bit integer"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ i64_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#ಅಥವಾಆದರೆ defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ ದೀರ್ಘ i64_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ಅಥವಾಆದರೆ defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ ದೀರ್ಘ i64_t;
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#ಇಲ್ಲದಿದ್ದರೆ
#   error "no 64-bit integer"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
