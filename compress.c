/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#घोषणा ZLIB_INTERNAL
#समावेश "zlib.h"

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
पूर्णांक ZEXPORT compress2_z(Bytef *dest, z_आकार_t *destLen, स्थिर Bytef *source,
                        z_आकार_t sourceLen, पूर्णांक level) {
    z_stream stream;
    पूर्णांक err;
    स्थिर uInt max = (uInt)-1;
    z_आकार_t बायाँ;

    अगर ((sourceLen > 0 && source == शून्य) ||
        destLen == शून्य || (*destLen > 0 && dest == शून्य))
        वापस Z_STREAM_ERROR;

    बायाँ = *destLen;
    *destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    अगर (err != Z_OK) वापस err;

    stream.अग्र_out = dest;
    stream.avail_out = 0;
    stream.अग्र_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    करो {
        अगर (stream.avail_out == 0) {
            stream.avail_out = बायाँ > (z_आकार_t)max ? max : (uInt)बायाँ;
            बायाँ -= stream.avail_out;
        }
        अगर (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (z_आकार_t)max ? max :
                                                          (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } जबतक (err == Z_OK);

    *destLen = (z_आकार_t)(stream.अग्र_out - dest);
    deflateEnd(&stream);
    वापस err == Z_STREAM_END ? Z_OK : err;
}
पूर्णांक ZEXPORT compress2(Bytef *dest, uLongf *destLen, स्थिर Bytef *source,
                      uLong sourceLen, पूर्णांक level) {
    पूर्णांक ret;
    z_आकार_t got = *destLen;
    ret = compress2_z(dest, &got, source, sourceLen, level);
    *destLen = (uLong)got;
    वापस ret;
}
/* ===========================================================================
 */
पूर्णांक ZEXPORT compress_z(Bytef *dest, z_आकार_t *destLen, स्थिर Bytef *source,
                       z_आकार_t sourceLen) {
    वापस compress2_z(dest, destLen, source, sourceLen,
                       Z_DEFAULT_COMPRESSION);
}
पूर्णांक ZEXPORT compress(Bytef *dest, uLongf *destLen, स्थिर Bytef *source,
                     uLong sourceLen) {
    वापस compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_आकार_t ZEXPORT compressBound_z(z_आकार_t sourceLen) {
    z_आकार_t bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
                     (sourceLen >> 25) + 13;
    वापस bound < sourceLen ? (z_आकार_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong sourceLen) {
    z_आकार_t bound = compressBound_z(sourceLen);
    वापस (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
