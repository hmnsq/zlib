/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#నిర్వచించు ZLIB_అంతర్గత
#అన్తర్భావ "zlib.h"

/* ===========================================================================
     Decompresses the source buffer into the destination buffer.  *sourceLen is
   the byte length of the source buffer. Upon entry, *destLen is the total size
   of the destination buffer, which must be large enough to hold the entire
   uncompressed data. (The size of the uncompressed data must have been saved
   previously by the compressor and transmitted to the decompressor by some
   mechanism outside the scope of this compression library.) Upon exit,
   *destLen is the size of the decompressed data and *sourceLen is the number
   of source bytes consumed. Upon return, source + *sourceLen points to the
   first unused input byte.

     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer, or
   Z_DATA_ERROR if the input data was corrupted, including if the input data is
   an incomplete zlib stream.

     The _z versions of the functions take size_t length arguments.
*/
పూర్ణాంకం ZEXPORT uncompress2_z(Bytef *dest, z_కొలత_t *destLen, స్థిరరాశి Bytef *ఆధారం,
                          z_కొలత_t *ఆధారం_Len) {
    z_ప్రవాహం ప్రవాహం;
    పూర్ణాంకం err;
    స్థిరరాశి uInt max = (uInt)-1;
    z_కొలత_t len, ఎడమ;

    ఒకవేళ (ఆధారం_Len == ఖాళీ || (*ఆధారం_Len > 0 && ఆధారం == ఖాళీ) ||
        destLen == ఖాళీ || (*destLen > 0 && dest == ఖాళీ))
        తిరిగి Z_ప్రవాహం_ERROR;

    len = *ఆధారం_Len;
    ఎడమ = *destLen;
    ఒకవేళ (ఎడమ == 0 && dest == Z_NULL)
        dest = (Bytef *)&ప్రవాహం.reserved;       /* next_out cannot be NULL */

    ప్రవాహం.తదుపరి_లోపల = (z_const Bytef *)ఆధారం;
    ప్రవాహం.avail_లోపల = 0;
    ప్రవాహం.zalloc = (alloc_func)0;
    ప్రవాహం.zfree = (free_func)0;
    ప్రవాహం.opaque = (voidpf)0;

    err = inflateInit(&ప్రవాహం);
    ఒకవేళ (err != Z_OK) తిరిగి err;

    ప్రవాహం.తదుపరి_బహిర్గతం = dest;
    ప్రవాహం.avail_బహిర్గతం = 0;

    చేయి {
        ఒకవేళ (ప్రవాహం.avail_బహిర్గతం == 0) {
            ప్రవాహం.avail_బహిర్గతం = ఎడమ > (z_కొలత_t)max ? max : (uInt)ఎడమ;
            ఎడమ -= ప్రవాహం.avail_బహిర్గతం;
        }
        ఒకవేళ (ప్రవాహం.avail_లోపల == 0) {
            ప్రవాహం.avail_లోపల = len > (z_కొలత_t)max ? max : (uInt)len;
            len -= ప్రవాహం.avail_లోపల;
        }
        err = inflate(&ప్రవాహం, Z_NO_FLUSH);
    } అయితే (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += ప్రవాహం.avail_లోపల;
    ఎడమ += ప్రవాహం.avail_బహిర్గతం;
    *ఆధారం_Len -= len;
    *destLen -= ఎడమ;

    inflate_చివర(&ప్రవాహం);
    తిరిగి err == Z_ప్రవాహం_చివర ? Z_OK :
           err == Z_NEED_DICT ? Z_దత్తాంశం_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_దత్తాంశం_ERROR :
           err;
}
పూర్ణాంకం ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, స్థిరరాశి Bytef *ఆధారం,
                        uLong *ఆధారం_Len) {
    పూర్ణాంకం ret;
    z_కొలత_t got = *destLen, వాడిన = *ఆధారం_Len;
    ret = uncompress2_z(dest, &got, ఆధారం, &వాడిన);
    *ఆధారం_Len = (uLong)వాడిన;
    *destLen = (uLong)got;
    తిరిగి ret;
}
పూర్ణాంకం ZEXPORT uncompress_z(Bytef *dest, z_కొలత_t *destLen, స్థిరరాశి Bytef *ఆధారం,
                         z_కొలత_t ఆధారం_Len) {
    z_కొలత_t వాడిన = ఆధారం_Len;
    తిరిగి uncompress2_z(dest, destLen, ఆధారం, &వాడిన);
}
పూర్ణాంకం ZEXPORT uncompress(Bytef *dest, uLongf *destLen, స్థిరరాశి Bytef *ఆధారం,
                       uLong ఆధారం_Len) {
    uLong వాడిన = ఆధారం_Len;
    తిరిగి uncompress2(dest, destLen, ఆధారం, &వాడిన);
}
