/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#परिभाषय ZLIB_आन्तरिकम्
#अन्तर्भाव "zlib.h"

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
पूर्णाङ्क ZEXPORT compress2_z(Bytef *dest, z_आकार_t *destLen, ध्रुव Bytef *source,
                        z_आकार_t sourceLen, पूर्णाङ्क level) {
    z_प्रवाहः प्रवाहः;
    पूर्णाङ्क err;
    ध्रुव uInt max = (uInt)-1;
    z_आकार_t वामः;

    यदि ((sourceLen > 0 && source == रिक्त) ||
        destLen == रिक्त || (*destLen > 0 && dest == रिक्त))
        प्रत्यागम Z_प्रवाहः_ERROR;

    वामः = *destLen;
    *destLen = 0;

    प्रवाहः.zalloc = (alloc_func)0;
    प्रवाहः.zfree = (free_func)0;
    प्रवाहः.opaque = (voidpf)0;

    err = deflateInit(&प्रवाहः, level);
    यदि (err != Z_OK) प्रत्यागम err;

    प्रवाहः.अग्र_बहिर्गमनम् = dest;
    प्रवाहः.avail_बहिर्गमनम् = 0;
    प्रवाहः.अग्र_अन्तर् = (z_const Bytef *)source;
    प्रवाहः.avail_अन्तर् = 0;

    कुरु {
        यदि (प्रवाहः.avail_बहिर्गमनम् == 0) {
            प्रवाहः.avail_बहिर्गमनम् = वामः > (z_आकार_t)max ? max : (uInt)वामः;
            वामः -= प्रवाहः.avail_बहिर्गमनम्;
        }
        यदि (प्रवाहः.avail_अन्तर् == 0) {
            प्रवाहः.avail_अन्तर् = sourceLen > (z_आकार_t)max ? max :
                                                          (uInt)sourceLen;
            sourceLen -= प्रवाहः.avail_अन्तर्;
        }
        err = deflate(&प्रवाहः, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } यावत् (err == Z_OK);

    *destLen = (z_आकार_t)(प्रवाहः.अग्र_बहिर्गमनम् - dest);
    deflate_अन्तः(&प्रवाहः);
    प्रत्यागम err == Z_प्रवाहः_अन्तः ? Z_OK : err;
}
पूर्णाङ्क ZEXPORT compress2(Bytef *dest, uLongf *destLen, ध्रुव Bytef *source,
                      uLong sourceLen, पूर्णाङ्क level) {
    पूर्णाङ्क ret;
    z_आकार_t got = *destLen;
    ret = compress2_z(dest, &got, source, sourceLen, level);
    *destLen = (uLong)got;
    प्रत्यागम ret;
}
/* ===========================================================================
 */
पूर्णाङ्क ZEXPORT compress_z(Bytef *dest, z_आकार_t *destLen, ध्रुव Bytef *source,
                       z_आकार_t sourceLen) {
    प्रत्यागम compress2_z(dest, destLen, source, sourceLen,
                       Z_DEFAULT_COMPRESSION);
}
पूर्णाङ्क ZEXPORT compress(Bytef *dest, uLongf *destLen, ध्रुव Bytef *source,
                     uLong sourceLen) {
    प्रत्यागम compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_आकार_t ZEXPORT compressBound_z(z_आकार_t sourceLen) {
    z_आकार_t bound = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
                     (sourceLen >> 25) + 13;
    प्रत्यागम bound < sourceLen ? (z_आकार_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong sourceLen) {
    z_आकार_t bound = compressBound_z(sourceLen);
    प्रत्यागम (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
