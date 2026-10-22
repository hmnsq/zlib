/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
#அந்தர்பாவ "zlib.h"

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
முழு ZEXPORT compress2_z(Bytef *dest, z_size_t *destLen, மாறிலி Bytef *source,
                        z_size_t sourceLen, முழு level) {
    z_stream stream;
    முழு err;
    மாறிலி uInt max = (uInt)-1;
    z_size_t இடது;

    எனில் ((sourceLen > 0 && source == வெற்று) ||
        destLen == வெற்று || (*destLen > 0 && dest == வெற்று))
        திரும்பு Z_STREAM_ERROR;

    இடது = *destLen;
    *destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    எனில் (err != Z_OK) திரும்பு err;

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    செய் {
        எனில் (stream.avail_out == 0) {
            stream.avail_out = இடது > (z_size_t)max ? max : (uInt)இடது;
            இடது -= stream.avail_out;
        }
        எனில் (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (z_size_t)max ? max :
                                                          (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } போது (err == Z_OK);

    *destLen = (z_size_t)(stream.next_out - dest);
    deflateEnd(&stream);
    திரும்பு err == Z_STREAM_END ? Z_OK : err;
}
முழு ZEXPORT compress2(Bytef *dest, uLongf *destLen, மாறிலி Bytef *source,
                      uLong sourceLen, முழு level) {
    முழு ret;
    z_size_t got = *destLen;
    ret = compress2_z(dest, &got, source, sourceLen, level);
    *destLen = (uLong)got;
    திரும்பு ret;
}
/* ===========================================================================
 */
முழு ZEXPORT compress_z(Bytef *dest, z_size_t *destLen, மாறிலி Bytef *source,
                       z_size_t sourceLen) {
    திரும்பு compress2_z(dest, destLen, source, sourceLen,
                       Z_DEFAULT_COMPRESSION);
}
முழு ZEXPORT compress(Bytef *dest, uLongf *destLen, மாறிலி Bytef *source,
                     uLong sourceLen) {
    திரும்பு compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_size_t ZEXPORT compressBound_z(z_size_t sourceLen) {
    z_size_t bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
                     (sourceLen >> 25) + 13;
    திரும்பு bound < sourceLen ? (z_size_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong sourceLen) {
    z_size_t bound = compressBound_z(sourceLen);
    திரும்பு (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
