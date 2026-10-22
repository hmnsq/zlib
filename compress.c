/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
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
صحیح ZEXPORT compress2_z(Bytef *dest, z_size_t *destLen, ثابت Bytef *source,
                        z_size_t sourceLen, صحیح level) {
    z_stream stream;
    صحیح err;
    ثابت uInt max = (uInt)-1;
    z_size_t چپ;

    اگر ((sourceLen > 0 && source == پوچ) ||
        destLen == پوچ || (*destLen > 0 && dest == پوچ))
        بازگشت Z_STREAM_ERROR;

    چپ = *destLen;
    *destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    اگر (err != Z_OK) بازگشت err;

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    انجامبده {
        اگر (stream.avail_out == 0) {
            stream.avail_out = چپ > (z_size_t)max ? max : (uInt)چپ;
            چپ -= stream.avail_out;
        }
        اگر (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (z_size_t)max ? max :
                                                          (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } وقتی (err == Z_OK);

    *destLen = (z_size_t)(stream.next_out - dest);
    deflateEnd(&stream);
    بازگشت err == Z_STREAM_END ? Z_OK : err;
}
صحیح ZEXPORT compress2(Bytef *dest, uLongf *destLen, ثابت Bytef *source,
                      uLong sourceLen, صحیح level) {
    صحیح ret;
    z_size_t got = *destLen;
    ret = compress2_z(dest, &got, source, sourceLen, level);
    *destLen = (uLong)got;
    بازگشت ret;
}
/* ===========================================================================
 */
صحیح ZEXPORT compress_z(Bytef *dest, z_size_t *destLen, ثابت Bytef *source,
                       z_size_t sourceLen) {
    بازگشت compress2_z(dest, destLen, source, sourceLen,
                       Z_DEFAULT_COMPRESSION);
}
صحیح ZEXPORT compress(Bytef *dest, uLongf *destLen, ثابت Bytef *source,
                     uLong sourceLen) {
    بازگشت compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_size_t ZEXPORT compressBound_z(z_size_t sourceLen) {
    z_size_t bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
                     (sourceLen >> 25) + 13;
    بازگشت bound < sourceLen ? (z_size_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong sourceLen) {
    z_size_t bound = compressBound_z(sourceLen);
    بازگشت (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
