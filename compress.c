/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
#ಅನ್ತರ್ಭಾವ "zlib.h"

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
ಪೂರ್ಣಾಙ್ಕ ZEXPORT compress2_z(Bytef *dest, z_size_t *destLen, ಧ್ರುವ Bytef *source,
                        z_size_t sourceLen, ಪೂರ್ಣಾಙ್ಕ level) {
    z_stream stream;
    ಪೂರ್ಣಾಙ್ಕ err;
    ಧ್ರುವ uInt max = (uInt)-1;
    z_size_t ಎಡ;

    ಯದಿ ((sourceLen > 0 && source == ರಿಕ್ತ) ||
        destLen == ರಿಕ್ತ || (*destLen > 0 && dest == ರಿಕ್ತ))
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;

    ಎಡ = *destLen;
    *destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    ಯದಿ (err != Z_OK) ಪ್ರತ್ಯಾಗಮ err;

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    ಕುರು {
        ಯದಿ (stream.avail_out == 0) {
            stream.avail_out = ಎಡ > (z_size_t)max ? max : (uInt)ಎಡ;
            ಎಡ -= stream.avail_out;
        }
        ಯದಿ (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (z_size_t)max ? max :
                                                          (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } ಯಾವತ್ (err == Z_OK);

    *destLen = (z_size_t)(stream.next_out - dest);
    deflateEnd(&stream);
    ಪ್ರತ್ಯಾಗಮ err == Z_STREAM_END ? Z_OK : err;
}
ಪೂರ್ಣಾಙ್ಕ ZEXPORT compress2(Bytef *dest, uLongf *destLen, ಧ್ರುವ Bytef *source,
                      uLong sourceLen, ಪೂರ್ಣಾಙ್ಕ level) {
    ಪೂರ್ಣಾಙ್ಕ ret;
    z_size_t got = *destLen;
    ret = compress2_z(dest, &got, source, sourceLen, level);
    *destLen = (uLong)got;
    ಪ್ರತ್ಯಾಗಮ ret;
}
/* ===========================================================================
 */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT compress_z(Bytef *dest, z_size_t *destLen, ಧ್ರುವ Bytef *source,
                       z_size_t sourceLen) {
    ಪ್ರತ್ಯಾಗಮ compress2_z(dest, destLen, source, sourceLen,
                       Z_DEFAULT_COMPRESSION);
}
ಪೂರ್ಣಾಙ್ಕ ZEXPORT compress(Bytef *dest, uLongf *destLen, ಧ್ರುವ Bytef *source,
                     uLong sourceLen) {
    ಪ್ರತ್ಯಾಗಮ compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_size_t ZEXPORT compressBound_z(z_size_t sourceLen) {
    z_size_t bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
                     (sourceLen >> 25) + 13;
    ಪ್ರತ್ಯಾಗಮ bound < sourceLen ? (z_size_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong sourceLen) {
    z_size_t bound = compressBound_z(sourceLen);
    ಪ್ರತ್ಯಾಗಮ (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
