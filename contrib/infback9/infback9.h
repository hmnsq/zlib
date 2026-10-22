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

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

ZEXTERN పూర్ణాంకం ZEXPORT inflateBack9(z_ప్రవాహం FAR *strm,
                                 లోపల_func లోపల, శూన్య FAR *లోపల_desc,
                                 బహిర్గతం_func బహిర్గతం, శూన్య FAR *బహిర్గతం_desc);
ZEXTERN పూర్ణాంకం ZEXPORT inflate_Back9_చివర(z_ప్రవాహం FAR *strm);
ZEXTERN పూర్ణాంకం ZEXPORT inflateBack9Init_(z_ప్రవాహం FAR *strm,
                                      గుర్తులేని అక్షరం FAR *కిటికీ,
                                      స్థిరరాశి అక్షరం *సంస్కరణ,
                                      పూర్ణాంకం ప్రవాహం_కొలత);
#నిర్వచించు inflateBack9Init(strm, కిటికీ) \
        inflateBack9Init_((strm), (కిటికీ), \
        ZLIB_సంస్కరణ, పరిమాణం(z_ప్రవాహం))

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు
