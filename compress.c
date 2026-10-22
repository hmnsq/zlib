/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کن ZLIB_داخلی
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
صحيح ZEXPORT compress2_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *منبع,
                        z_اندازه_t منبع_Len, صحيح level) {
    z_جریان جریان;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_اندازه_t چپ;

    إذا ((منبع_Len > 0 && منبع == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_جریان_ERROR;

    چپ = *destLen;
    *destLen = 0;

    جریان.zalloc = (alloc_func)0;
    جریان.zfree = (free_func)0;
    جریان.opaque = (voidpf)0;

    err = deflateInit(&جریان, level);
    إذا (err != Z_OK) أرجع err;

    جریان.بعدی_بیرون = dest;
    جریان.avail_بیرون = 0;
    جریان.بعدی_درون = (z_const Bytef *)منبع;
    جریان.avail_درون = 0;

    افعل {
        إذا (جریان.avail_بیرون == 0) {
            جریان.avail_بیرون = چپ > (z_اندازه_t)max ? max : (uInt)چپ;
            چپ -= جریان.avail_بیرون;
        }
        إذا (جریان.avail_درون == 0) {
            جریان.avail_درون = منبع_Len > (z_اندازه_t)max ? max :
                                                          (uInt)منبع_Len;
            منبع_Len -= جریان.avail_درون;
        }
        err = deflate(&جریان, منبع_Len ? Z_NO_FLUSH : Z_پایان);
    } طالما (err == Z_OK);

    *destLen = (z_اندازه_t)(جریان.بعدی_بیرون - dest);
    deflate_انتها(&جریان);
    أرجع err == Z_جریان_انتها ? Z_OK : err;
}
صحيح ZEXPORT compress2(Bytef *dest, uLongf *destLen, ثابت Bytef *منبع,
                      uLong منبع_Len, صحيح level) {
    صحيح ret;
    z_اندازه_t got = *destLen;
    ret = compress2_z(dest, &got, منبع, منبع_Len, level);
    *destLen = (uLong)got;
    أرجع ret;
}
/* ===========================================================================
 */
صحيح ZEXPORT compress_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *منبع,
                       z_اندازه_t منبع_Len) {
    أرجع compress2_z(dest, destLen, منبع, منبع_Len,
                       Z_DEFAULT_COMPRESSION);
}
صحيح ZEXPORT compress(Bytef *dest, uLongf *destLen, ثابت Bytef *منبع,
                     uLong منبع_Len) {
    أرجع compress2(dest, destLen, منبع, منبع_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_اندازه_t ZEXPORT compressBound_z(z_اندازه_t منبع_Len) {
    z_اندازه_t bound = منبع_Len + (منبع_Len >> 12) + (منبع_Len >> 14) +
                     (منبع_Len >> 25) + 13;
    أرجع bound < منبع_Len ? (z_اندازه_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong منبع_Len) {
    z_اندازه_t bound = compressBound_z(منبع_Len);
    أرجع (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
