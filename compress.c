/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کړه ZLIB_دننی
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
صحيح ZEXPORT compress2_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *سرچینه,
                        z_اندازه_t سرچینه_Len, صحيح level) {
    z_بهیر بهیر;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_اندازه_t کیڼ;

    إذا ((سرچینه_Len > 0 && سرچینه == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_بهیر_ERROR;

    کیڼ = *destLen;
    *destLen = 0;

    بهیر.zalloc = (alloc_func)0;
    بهیر.zfree = (free_func)0;
    بهیر.opaque = (voidpf)0;

    err = deflateInit(&بهیر, level);
    إذا (err != Z_OK) أرجع err;

    بهیر.بل_بهر = dest;
    بهیر.avail_بهر = 0;
    بهیر.بل_دننه = (z_const Bytef *)سرچینه;
    بهیر.avail_دننه = 0;

    افعل {
        إذا (بهیر.avail_بهر == 0) {
            بهیر.avail_بهر = کیڼ > (z_اندازه_t)max ? max : (uInt)کیڼ;
            کیڼ -= بهیر.avail_بهر;
        }
        إذا (بهیر.avail_دننه == 0) {
            بهیر.avail_دننه = سرچینه_Len > (z_اندازه_t)max ? max :
                                                          (uInt)سرچینه_Len;
            سرچینه_Len -= بهیر.avail_دننه;
        }
        err = deflate(&بهیر, سرچینه_Len ? Z_NO_FLUSH : Z_پای);
    } طالما (err == Z_OK);

    *destLen = (z_اندازه_t)(بهیر.بل_بهر - dest);
    deflate_پای_ټکی(&بهیر);
    أرجع err == Z_بهیر_پای_ټکی ? Z_OK : err;
}
صحيح ZEXPORT compress2(Bytef *dest, uLongf *destLen, ثابت Bytef *سرچینه,
                      uLong سرچینه_Len, صحيح level) {
    صحيح ret;
    z_اندازه_t got = *destLen;
    ret = compress2_z(dest, &got, سرچینه, سرچینه_Len, level);
    *destLen = (uLong)got;
    أرجع ret;
}
/* ===========================================================================
 */
صحيح ZEXPORT compress_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *سرچینه,
                       z_اندازه_t سرچینه_Len) {
    أرجع compress2_z(dest, destLen, سرچینه, سرچینه_Len,
                       Z_DEFAULT_COMPRESSION);
}
صحيح ZEXPORT compress(Bytef *dest, uLongf *destLen, ثابت Bytef *سرچینه,
                     uLong سرچینه_Len) {
    أرجع compress2(dest, destLen, سرچینه, سرچینه_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_اندازه_t ZEXPORT compressBound_z(z_اندازه_t سرچینه_Len) {
    z_اندازه_t bound = سرچینه_Len + (سرچینه_Len >> 12) + (سرچینه_Len >> 14) +
                     (سرچینه_Len >> 25) + 13;
    أرجع bound < سرچینه_Len ? (z_اندازه_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong سرچینه_Len) {
    z_اندازه_t bound = compressBound_z(سرچینه_Len);
    أرجع (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
