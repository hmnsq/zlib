/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#నిర్వచించు ZLIB_అంతర్గత
#అన్తర్భావ "zlib.h"

/* ===========================================================================
     Compresses the source buffer into the destination buffer. The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer. Upon entry, destLen is the total size of the
   destination buffer, which must be at least 0.1% larger than sourceLen plus
   12 bytes. Upon exit, destLen is the actual size of the compressed buffer.

     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.

     The _z versions of the functions take size_t length arguments.
*/
పూర్ణాంకం ZEXPORT compress2_z(Bytef *dest, z_కొలత_t *destLen, స్థిరరాశి Bytef *ఆధారం,
                        z_కొలత_t ఆధారం_Len, పూర్ణాంకం level) {
    z_ప్రవాహం ప్రవాహం;
    పూర్ణాంకం err;
    స్థిరరాశి uInt max = (uInt)-1;
    z_కొలత_t ఎడమ;

    ఒకవేళ ((ఆధారం_Len > 0 && ఆధారం == ఖాళీ) ||
        destLen == ఖాళీ || (*destLen > 0 && dest == ఖాళీ))
        తిరిగి Z_ప్రవాహం_ERROR;

    ఎడమ = *destLen;
    *destLen = 0;

    ప్రవాహం.zalloc = (alloc_func)0;
    ప్రవాహం.zfree = (free_func)0;
    ప్రవాహం.opaque = (voidpf)0;

    err = deflateInit(&ప్రవాహం, level);
    ఒకవేళ (err != Z_OK) తిరిగి err;

    ప్రవాహం.తదుపరి_బహిర్గతం = dest;
    ప్రవాహం.avail_బహిర్గతం = 0;
    ప్రవాహం.తదుపరి_లోపల = (z_const Bytef *)ఆధారం;
    ప్రవాహం.avail_లోపల = 0;

    చేయి {
        ఒకవేళ (ప్రవాహం.avail_బహిర్గతం == 0) {
            ప్రవాహం.avail_బహిర్గతం = ఎడమ > (z_కొలత_t)max ? max : (uInt)ఎడమ;
            ఎడమ -= ప్రవాహం.avail_బహిర్గతం;
        }
        ఒకవేళ (ప్రవాహం.avail_లోపల == 0) {
            ప్రవాహం.avail_లోపల = ఆధారం_Len > (z_కొలత_t)max ? max :
                                                          (uInt)ఆధారం_Len;
            ఆధారం_Len -= ప్రవాహం.avail_లోపల;
        }
        err = deflate(&ప్రవాహం, ఆధారం_Len ? Z_NO_FLUSH : Z_ముగింపు);
    } అయితే (err == Z_OK);

    *destLen = (z_కొలత_t)(ప్రవాహం.తదుపరి_బహిర్గతం - dest);
    deflate_చివర(&ప్రవాహం);
    తిరిగి err == Z_ప్రవాహం_చివర ? Z_OK : err;
}
పూర్ణాంకం ZEXPORT compress2(Bytef *dest, uLongf *destLen, స్థిరరాశి Bytef *ఆధారం,
                      uLong ఆధారం_Len, పూర్ణాంకం level) {
    పూర్ణాంకం ret;
    z_కొలత_t got = *destLen;
    ret = compress2_z(dest, &got, ఆధారం, ఆధారం_Len, level);
    *destLen = (uLong)got;
    తిరిగి ret;
}
/* ===========================================================================
 */
పూర్ణాంకం ZEXPORT compress_z(Bytef *dest, z_కొలత_t *destLen, స్థిరరాశి Bytef *ఆధారం,
                       z_కొలత_t ఆధారం_Len) {
    తిరిగి compress2_z(dest, destLen, ఆధారం, ఆధారం_Len,
                       Z_DEFAULT_COMPRESSION);
}
పూర్ణాంకం ZEXPORT compress(Bytef *dest, uLongf *destLen, స్థిరరాశి Bytef *ఆధారం,
                     uLong ఆధారం_Len) {
    తిరిగి compress2(dest, destLen, ఆధారం, ఆధారం_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_కొలత_t ZEXPORT compressBound_z(z_కొలత_t ఆధారం_Len) {
    z_కొలత_t bound = ఆధారం_Len + (ఆధారం_Len >> 12) + (ఆధారం_Len >> 14) +
                     (ఆధారం_Len >> 25) + 13;
    తిరిగి bound < ఆధారం_Len ? (z_కొలత_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong ఆధారం_Len) {
    z_కొలత_t bound = compressBound_z(ఆధారం_Len);
    తిరిగి (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
