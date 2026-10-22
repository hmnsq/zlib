/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
#അന്തര്ഭാവ "zlib.h"

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
പൂർണ്ണസംഖ്യ ZEXPORT compress2_z(Bytef *dest, z_size_t *destLen, സ്ഥിരം Bytef *source,
                        z_size_t sourceLen, പൂർണ്ണസംഖ്യ level) {
    z_stream stream;
    പൂർണ്ണസംഖ്യ err;
    സ്ഥിരം uInt max = (uInt)-1;
    z_size_t ഇടത്;

    എങ്കിൽ ((sourceLen > 0 && source == ശൂന്യ) ||
        destLen == ശൂന്യ || (*destLen > 0 && dest == ശൂന്യ))
        തിരികെ Z_STREAM_ERROR;

    ഇടത് = *destLen;
    *destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    എങ്കിൽ (err != Z_OK) തിരികെ err;

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    ചെയ്യുക {
        എങ്കിൽ (stream.avail_out == 0) {
            stream.avail_out = ഇടത് > (z_size_t)max ? max : (uInt)ഇടത്;
            ഇടത് -= stream.avail_out;
        }
        എങ്കിൽ (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (z_size_t)max ? max :
                                                          (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } ആകുമ്പോൾ (err == Z_OK);

    *destLen = (z_size_t)(stream.next_out - dest);
    deflateEnd(&stream);
    തിരികെ err == Z_STREAM_END ? Z_OK : err;
}
പൂർണ്ണസംഖ്യ ZEXPORT compress2(Bytef *dest, uLongf *destLen, സ്ഥിരം Bytef *source,
                      uLong sourceLen, പൂർണ്ണസംഖ്യ level) {
    പൂർണ്ണസംഖ്യ ret;
    z_size_t got = *destLen;
    ret = compress2_z(dest, &got, source, sourceLen, level);
    *destLen = (uLong)got;
    തിരികെ ret;
}
/* ===========================================================================
 */
പൂർണ്ണസംഖ്യ ZEXPORT compress_z(Bytef *dest, z_size_t *destLen, സ്ഥിരം Bytef *source,
                       z_size_t sourceLen) {
    തിരികെ compress2_z(dest, destLen, source, sourceLen,
                       Z_DEFAULT_COMPRESSION);
}
പൂർണ്ണസംഖ്യ ZEXPORT compress(Bytef *dest, uLongf *destLen, സ്ഥിരം Bytef *source,
                     uLong sourceLen) {
    തിരികെ compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_size_t ZEXPORT compressBound_z(z_size_t sourceLen) {
    z_size_t bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
                     (sourceLen >> 25) + 13;
    തിരികെ bound < sourceLen ? (z_size_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong sourceLen) {
    z_size_t bound = compressBound_z(sourceLen);
    തിരികെ (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
