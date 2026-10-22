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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ

ZEXTERN ಪೂರ್ಣಾಂಕ ZEXPORT inflateBack9(z_ಹರಿವು FAR *strm,
                                 ಒಳ_func ಒಳ, ಶೂನ್ಯ FAR *ಒಳ_desc,
                                 ಹೊರಹೋಗು_func ಹೊರಹೋಗು, ಶೂನ್ಯ FAR *ಹೊರಹೋಗು_desc);
ZEXTERN ಪೂರ್ಣಾಂಕ ZEXPORT inflate_Back9_ಕೊನೆ(z_ಹರಿವು FAR *strm);
ZEXTERN ಪೂರ್ಣಾಂಕ ZEXPORT inflateBack9Init_(z_ಹರಿವು FAR *strm,
                                      ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ,
                                      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ,
                                      ಪೂರ್ಣಾಂಕ ಹರಿವು_ಗಾತ್ರ);
#ವ್ಯಾಖ್ಯಾನಿಸು inflateBack9Init(strm, ಕಿಟಕಿ) \
        inflateBack9Init_((strm), (ಕಿಟಕಿ), \
        ZLIB_ಆವೃತ್ತಿ, ಗಾತ್ರ(z_ಹರಿವು))

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ
