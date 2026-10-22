/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#ifndef INTS_H
#define INTS_H
#അന്തര്ഭാവ <limits.h>
#എങ്കിൽ defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    തരനിർവചനം signed അക്ഷരം i8_t;
    തരനിർവചനം unsigned അക്ഷരം ui8_t;
#അല്ലെങ്കിൽ
#   error "no 8-bit integer"
#endif
#എങ്കിൽ defined(USHRT_MAX) && USHRT_MAX == 0xffff
    തരനിർവചനം short i16_t;
    തരനിർവചനം unsigned short ui16_t;
#അല്ലെങ്കിലെങ്കിൽ defined(UINT_MAX) && UINT_MAX == 0xffff
    തരനിർവചനം പൂർണ്ണസംഖ്യ i16_t;
    തരനിർവചനം unsigned ui16_t;
#അല്ലെങ്കിൽ
#   error "no 16-bit integer"
#endif
#എങ്കിൽ defined(UINT_MAX) && UINT_MAX == 0xffffffff
    തരനിർവചനം പൂർണ്ണസംഖ്യ i32_t;
    തരനിർവചനം unsigned ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#അല്ലെങ്കിലെങ്കിൽ defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    തരനിർവചനം long i32_t;
    തരനിർവചനം unsigned long ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#അല്ലെങ്കിൽ
#   error "no 32-bit integer"
#endif
#എങ്കിൽ defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    തരനിർവചനം long i64_t;
    തരനിർവചനം unsigned long ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#അല്ലെങ്കിലെങ്കിൽ defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    തരനിർവചനം long long i64_t;
    തരനിർവചനം unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#അല്ലെങ്കിലെങ്കിൽ defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    തരനിർവചനം long long i64_t;
    തരനിർവചനം unsigned long long ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#അല്ലെങ്കിൽ
#   error "no 64-bit integer"
#endif
#endif
