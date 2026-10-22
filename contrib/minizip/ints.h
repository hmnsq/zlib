/* ints.h -- create integer types for 8, 16, 32, and 64 bits
 * Copyright (C) 2024 Mark Adler
 * For conditions of distribution and use, see the copyright notice in zlib.h
 *
 * There exist compilers with limits.h, but not stdint.h or inttypes.h.
 */

#جيڪڏهن_وصف_نه_ٿيل INTS_H
#وصف_ڪريو INTS_H
#ضمن <limits.h>
#إذا defined(UCHAR_MAX) && UCHAR_MAX == 0xff
    تعريفنوع نشانوارو حرف i8_t;
    تعريفنوع بنانشان حرف ui8_t;
#وإلا
#   error "no 8-bit integer"
#جيڪڏهن_پورو
#إذا defined(USHRT_MAX) && USHRT_MAX == 0xffff
    تعريفنوع ننڍو i16_t;
    تعريفنوع بنانشان ننڍو ui16_t;
#وإلاإذا defined(UINT_MAX) && UINT_MAX == 0xffff
    تعريفنوع صحيح i16_t;
    تعريفنوع بنانشان ui16_t;
#وإلا
#   error "no 16-bit integer"
#جيڪڏهن_پورو
#إذا defined(UINT_MAX) && UINT_MAX == 0xffffffff
    تعريفنوع صحيح i32_t;
    تعريفنوع بنانشان ui32_t;
#   define PI32 "d"
#   define PUI32 "u"
#وإلاإذا defined(ULONG_MAX) && ULONG_MAX == 0xffffffff
    تعريفنوع ڊگهو i32_t;
    تعريفنوع بنانشان ڊگهو ui32_t;
#   define PI32 "ld"
#   define PUI32 "lu"
#وإلا
#   error "no 32-bit integer"
#جيڪڏهن_پورو
#إذا defined(ULONG_MAX) && ULONG_MAX == 0xffffffffffffffff
    تعريفنوع ڊگهو i64_t;
    تعريفنوع بنانشان ڊگهو ui64_t;
#   define PI64 "ld"
#   define PUI64 "lu"
#وإلاإذا defined(ULLONG_MAX) && ULLONG_MAX == 0xffffffffffffffff
    تعريفنوع ڊگهو ڊگهو i64_t;
    تعريفنوع بنانشان ڊگهو ڊگهو ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وإلاإذا defined(ULONG_LONG_MAX) && ULONG_LONG_MAX == 0xffffffffffffffff
    تعريفنوع ڊگهو ڊگهو i64_t;
    تعريفنوع بنانشان ڊگهو ڊگهو ui64_t;
#   define PI64 "lld"
#   define PUI64 "llu"
#وإلا
#   error "no 64-bit integer"
#جيڪڏهن_پورو
#جيڪڏهن_پورو
