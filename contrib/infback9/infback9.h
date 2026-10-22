/* infback9.h -- header for using inflateBack9 functions
 * Copyright (C) 2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * This header file and associated patches provide a decoder for PKWare's
 * undocumented deflate64 compression method (method 9).  Use with infback9.c,
 * inftree9.h, inftree9.c, and inffix9.h.  These patches are not supported.
 * This should be compiled with zlib, since it uses zutil.h and zutil.o.
 * This code has not yet been tested on 16-bit architectures.  See the
 * comments in zlib.h for inflateBack() usage.  These functions are used
 * identically, except that there is no windowBits parameter, and a 64K
 * window must be provided.  Also if int's are 16 bits, then a zero for
 * the third parameter of the "out" function actually means 65536UL.
 * zlib.h must be included before this header file.
 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

ZEXTERN ਪੂਰਨ ZEXPORT inflateBack9(z_ਧਾਰਾ FAR *strm,
                                 ਵਿੱਚ_func ਵਿੱਚ, ਖਾਲੀ FAR *ਵਿੱਚ_desc,
                                 ਬਾਹਰਲਾ_func ਬਾਹਰਲਾ, ਖਾਲੀ FAR *ਬਾਹਰਲਾ_desc);
ZEXTERN ਪੂਰਨ ZEXPORT inflate_Back9_ਸਿਰਾ(z_ਧਾਰਾ FAR *strm);
ZEXTERN ਪੂਰਨ ZEXPORT inflateBack9Init_(z_ਧਾਰਾ FAR *strm,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ,
                                      ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ,
                                      ਪੂਰਨ ਧਾਰਾ_ਮਾਪ);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ inflateBack9Init(strm, ਖਿੜਕੀ) \
        inflateBack9Init_((strm), (ਖਿੜਕੀ), \
        ZLIB_ਸੰਸਕਰਣ, ਆਕਾਰ(z_ਧਾਰਾ))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ
