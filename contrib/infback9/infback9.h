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

#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः

ZEXTERN पूर्णाङ्क ZEXPORT inflateBack9(z_प्रवाहः FAR *strm,
                                 अन्तर्_func अन्तर्, शून्य FAR *अन्तर्_desc,
                                 बहिर्गमनम्_func बहिर्गमनम्, शून्य FAR *बहिर्गमनम्_desc);
ZEXTERN पूर्णाङ्क ZEXPORT inflate_Back9_अन्तः(z_प्रवाहः FAR *strm);
ZEXTERN पूर्णाङ्क ZEXPORT inflateBack9Init_(z_प्रवाहः FAR *strm,
                                      अचिह्नित वर्ण FAR *गवाक्षः,
                                      ध्रुव वर्ण *संस्करणम्,
                                      पूर्णाङ्क प्रवाहः_आकार);
#परिभाषय inflateBack9Init(strm, गवाक्षः) \
        inflateBack9Init_((strm), (गवाक्षः), \
        ZLIB_संस्करणम्, परिमाण(z_प्रवाहः))

#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः
