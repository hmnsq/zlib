/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کرو ZLIB_اندرونی
#شامل "zlib.h"

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
صحیح ZEXPORT compress2_z(Bytef *dest, z_سائز_t *destLen, ثابت Bytef *ماخذ,
                        z_سائز_t ماخذ_Len, صحیح level) {
    z_دھارا دھارا;
    صحیح err;
    ثابت uInt max = (uInt)-1;
    z_سائز_t بایاں;

    اگر ((ماخذ_Len > 0 && ماخذ == تہی) ||
        destLen == تہی || (*destLen > 0 && dest == تہی))
        واپس Z_دھارا_ERROR;

    بایاں = *destLen;
    *destLen = 0;

    دھارا.zalloc = (alloc_func)0;
    دھارا.zfree = (free_func)0;
    دھارا.opaque = (voidpf)0;

    err = deflateInit(&دھارا, level);
    اگر (err != Z_OK) واپس err;

    دھارا.اگلا_برآمد = dest;
    دھارا.avail_برآمد = 0;
    دھارا.اگلا_اندر = (z_const Bytef *)ماخذ;
    دھارا.avail_اندر = 0;

    کرو {
        اگر (دھارا.avail_برآمد == 0) {
            دھارا.avail_برآمد = بایاں > (z_سائز_t)max ? max : (uInt)بایاں;
            بایاں -= دھارا.avail_برآمد;
        }
        اگر (دھارا.avail_اندر == 0) {
            دھارا.avail_اندر = ماخذ_Len > (z_سائز_t)max ? max :
                                                          (uInt)ماخذ_Len;
            ماخذ_Len -= دھارا.avail_اندر;
        }
        err = deflate(&دھارا, ماخذ_Len ? Z_NO_FLUSH : Z_اختتام);
    } جب (err == Z_OK);

    *destLen = (z_سائز_t)(دھارا.اگلا_برآمد - dest);
    deflate_انتہا(&دھارا);
    واپس err == Z_دھارا_انتہا ? Z_OK : err;
}
صحیح ZEXPORT compress2(Bytef *dest, uLongf *destLen, ثابت Bytef *ماخذ,
                      uLong ماخذ_Len, صحیح level) {
    صحیح ret;
    z_سائز_t got = *destLen;
    ret = compress2_z(dest, &got, ماخذ, ماخذ_Len, level);
    *destLen = (uLong)got;
    واپس ret;
}
/* ===========================================================================
 */
صحیح ZEXPORT compress_z(Bytef *dest, z_سائز_t *destLen, ثابت Bytef *ماخذ,
                       z_سائز_t ماخذ_Len) {
    واپس compress2_z(dest, destLen, ماخذ, ماخذ_Len,
                       Z_DEFAULT_COMPRESSION);
}
صحیح ZEXPORT compress(Bytef *dest, uLongf *destLen, ثابت Bytef *ماخذ,
                     uLong ماخذ_Len) {
    واپس compress2(dest, destLen, ماخذ, ماخذ_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_سائز_t ZEXPORT compressBound_z(z_سائز_t ماخذ_Len) {
    z_سائز_t bound = ماخذ_Len + (ماخذ_Len >> 12) + (ماخذ_Len >> 14) +
                     (ماخذ_Len >> 25) + 13;
    واپس bound < ماخذ_Len ? (z_سائز_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong ماخذ_Len) {
    z_سائز_t bound = compressBound_z(ماخذ_Len);
    واپس (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
