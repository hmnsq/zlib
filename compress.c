/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کرو ZLIB_اندرونی
#ضمن "zlib.h"

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
صحيح ZEXPORT compress2_z(Bytef *dest, z_ناپ_t *destLen, ثابت Bytef *سروت,
                        z_ناپ_t سروت_Len, صحيح level) {
    z_دھارا دھارا;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_ناپ_t بایاں;

    إذا ((سروت_Len > 0 && سروت == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_دھارا_ERROR;

    بایاں = *destLen;
    *destLen = 0;

    دھارا.zalloc = (alloc_func)0;
    دھارا.zfree = (free_func)0;
    دھارا.opaque = (voidpf)0;

    err = deflateInit(&دھارا, level);
    إذا (err != Z_OK) أرجع err;

    دھارا.اگلا_باہرلا = dest;
    دھارا.avail_باہرلا = 0;
    دھارا.اگلا_وچ = (z_const Bytef *)سروت;
    دھارا.avail_وچ = 0;

    افعل {
        إذا (دھارا.avail_باہرلا == 0) {
            دھارا.avail_باہرلا = بایاں > (z_ناپ_t)max ? max : (uInt)بایاں;
            بایاں -= دھارا.avail_باہرلا;
        }
        إذا (دھارا.avail_وچ == 0) {
            دھارا.avail_وچ = سروت_Len > (z_ناپ_t)max ? max :
                                                          (uInt)سروت_Len;
            سروت_Len -= دھارا.avail_وچ;
        }
        err = deflate(&دھارا, سروت_Len ? Z_NO_FLUSH : Z_انت);
    } طالما (err == Z_OK);

    *destLen = (z_ناپ_t)(دھارا.اگلا_باہرلا - dest);
    deflate_سرا(&دھارا);
    أرجع err == Z_دھارا_سرا ? Z_OK : err;
}
صحيح ZEXPORT compress2(Bytef *dest, uLongf *destLen, ثابت Bytef *سروت,
                      uLong سروت_Len, صحيح level) {
    صحيح ret;
    z_ناپ_t got = *destLen;
    ret = compress2_z(dest, &got, سروت, سروت_Len, level);
    *destLen = (uLong)got;
    أرجع ret;
}
/* ===========================================================================
 */
صحيح ZEXPORT compress_z(Bytef *dest, z_ناپ_t *destLen, ثابت Bytef *سروت,
                       z_ناپ_t سروت_Len) {
    أرجع compress2_z(dest, destLen, سروت, سروت_Len,
                       Z_DEFAULT_COMPRESSION);
}
صحيح ZEXPORT compress(Bytef *dest, uLongf *destLen, ثابت Bytef *سروت,
                     uLong سروت_Len) {
    أرجع compress2(dest, destLen, سروت, سروت_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_ناپ_t ZEXPORT compressBound_z(z_ناپ_t سروت_Len) {
    z_ناپ_t bound = سروت_Len + (سروت_Len >> 12) + (سروت_Len >> 14) +
                     (سروت_Len >> 25) + 13;
    أرجع bound < سروت_Len ? (z_ناپ_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong سروت_Len) {
    z_ناپ_t bound = compressBound_z(سروت_Len);
    أرجع (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
