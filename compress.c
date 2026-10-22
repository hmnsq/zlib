/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#وصف_ڪريو ZLIB_اندروني
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
صحيح ZEXPORT compress2_z(Bytef *dest, z_سائيز_t *destLen, ثابت Bytef *ذريعو,
                        z_سائيز_t ذريعو_Len, صحيح level) {
    z_وهڪرو وهڪرو;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_سائيز_t کاٻو;

    إذا ((ذريعو_Len > 0 && ذريعو == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_وهڪرو_ERROR;

    کاٻو = *destLen;
    *destLen = 0;

    وهڪرو.zalloc = (alloc_func)0;
    وهڪرو.zfree = (free_func)0;
    وهڪرو.opaque = (voidpf)0;

    err = deflateInit(&وهڪرو, level);
    إذا (err != Z_OK) أرجع err;

    وهڪرو.ايندڙ_ٻاهريون = dest;
    وهڪرو.avail_ٻاهريون = 0;
    وهڪرو.ايندڙ_اندريون = (z_const Bytef *)ذريعو;
    وهڪرو.avail_اندريون = 0;

    افعل {
        إذا (وهڪرو.avail_ٻاهريون == 0) {
            وهڪرو.avail_ٻاهريون = کاٻو > (z_سائيز_t)max ? max : (uInt)کاٻو;
            کاٻو -= وهڪرو.avail_ٻاهريون;
        }
        إذا (وهڪرو.avail_اندريون == 0) {
            وهڪرو.avail_اندريون = ذريعو_Len > (z_سائيز_t)max ? max :
                                                          (uInt)ذريعو_Len;
            ذريعو_Len -= وهڪرو.avail_اندريون;
        }
        err = deflate(&وهڪرو, ذريعو_Len ? Z_NO_FLUSH : Z_پڄاڻي);
    } طالما (err == Z_OK);

    *destLen = (z_سائيز_t)(وهڪرو.ايندڙ_ٻاهريون - dest);
    deflate_پڇاڙي(&وهڪرو);
    أرجع err == Z_وهڪرو_پڇاڙي ? Z_OK : err;
}
صحيح ZEXPORT compress2(Bytef *dest, uLongf *destLen, ثابت Bytef *ذريعو,
                      uLong ذريعو_Len, صحيح level) {
    صحيح ret;
    z_سائيز_t got = *destLen;
    ret = compress2_z(dest, &got, ذريعو, ذريعو_Len, level);
    *destLen = (uLong)got;
    أرجع ret;
}
/* ===========================================================================
 */
صحيح ZEXPORT compress_z(Bytef *dest, z_سائيز_t *destLen, ثابت Bytef *ذريعو,
                       z_سائيز_t ذريعو_Len) {
    أرجع compress2_z(dest, destLen, ذريعو, ذريعو_Len,
                       Z_DEFAULT_COMPRESSION);
}
صحيح ZEXPORT compress(Bytef *dest, uLongf *destLen, ثابت Bytef *ذريعو,
                     uLong ذريعو_Len) {
    أرجع compress2(dest, destLen, ذريعو, ذريعو_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_سائيز_t ZEXPORT compressBound_z(z_سائيز_t ذريعو_Len) {
    z_سائيز_t bound = ذريعو_Len + (ذريعو_Len >> 12) + (ذريعو_Len >> 14) +
                     (ذريعو_Len >> 25) + 13;
    أرجع bound < ذريعو_Len ? (z_سائيز_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong ذريعو_Len) {
    z_سائيز_t bound = compressBound_z(ذريعو_Len);
    أرجع (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
