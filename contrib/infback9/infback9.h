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

#که_تعریف_شوی __cplusplus
خارجي "C" {
#د_که_پای

ZEXTERN صحيح ZEXPORT inflateBack9(z_بهیر FAR *strm,
                                 دننه_func دننه, فراغ FAR *دننه_desc,
                                 بهر_func بهر, فراغ FAR *بهر_desc);
ZEXTERN صحيح ZEXPORT inflate_Back9_پای_ټکی(z_بهیر FAR *strm);
ZEXTERN صحيح ZEXPORT inflateBack9Init_(z_بهیر FAR *strm,
                                      بینښه حرف FAR *کړکۍ,
                                      ثابت حرف *نسخه,
                                      صحيح بهیر_اندازه);
#تعریف_کړه inflateBack9Init(strm, کړکۍ) \
        inflateBack9Init_((strm), (کړکۍ), \
        ZLIB_نسخه, حجم(z_بهیر))

#که_تعریف_شوی __cplusplus
}
#د_که_پای
