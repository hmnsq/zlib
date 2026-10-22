/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_ਅੰਦਰੂਨੀ
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

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
ਪੂਰਨ ZEXPORT compress2_z(Bytef *dest, z_ਮਾਪ_t *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                        z_ਮਾਪ_t ਸਰੋਤ_Len, ਪੂਰਨ level) {
    z_ਧਾਰਾ ਧਾਰਾ;
    ਪੂਰਨ err;
    ਅਚਲ uInt max = (uInt)-1;
    z_ਮਾਪ_t ਖੱਬਾ;

    ਜੇ ((ਸਰੋਤ_Len > 0 && ਸਰੋਤ == ਨਿਰਾ) ||
        destLen == ਨਿਰਾ || (*destLen > 0 && dest == ਨਿਰਾ))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    ਖੱਬਾ = *destLen;
    *destLen = 0;

    ਧਾਰਾ.zalloc = (alloc_func)0;
    ਧਾਰਾ.zfree = (free_func)0;
    ਧਾਰਾ.opaque = (voidpf)0;

    err = deflateInit(&ਧਾਰਾ, level);
    ਜੇ (err != Z_OK) ਵਾਪਸ err;

    ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = dest;
    ਧਾਰਾ.avail_ਬਾਹਰਲਾ = 0;
    ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = (z_const Bytef *)ਸਰੋਤ;
    ਧਾਰਾ.avail_ਵਿੱਚ = 0;

    ਕਰੋ {
        ਜੇ (ਧਾਰਾ.avail_ਬਾਹਰਲਾ == 0) {
            ਧਾਰਾ.avail_ਬਾਹਰਲਾ = ਖੱਬਾ > (z_ਮਾਪ_t)max ? max : (uInt)ਖੱਬਾ;
            ਖੱਬਾ -= ਧਾਰਾ.avail_ਬਾਹਰਲਾ;
        }
        ਜੇ (ਧਾਰਾ.avail_ਵਿੱਚ == 0) {
            ਧਾਰਾ.avail_ਵਿੱਚ = ਸਰੋਤ_Len > (z_ਮਾਪ_t)max ? max :
                                                          (uInt)ਸਰੋਤ_Len;
            ਸਰੋਤ_Len -= ਧਾਰਾ.avail_ਵਿੱਚ;
        }
        err = deflate(&ਧਾਰਾ, ਸਰੋਤ_Len ? Z_NO_FLUSH : Z_ਅੰਤ);
    } ਜਦੋਂ (err == Z_OK);

    *destLen = (z_ਮਾਪ_t)(ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ - dest);
    deflate_ਸਿਰਾ(&ਧਾਰਾ);
    ਵਾਪਸ err == Z_ਧਾਰਾ_ਸਿਰਾ ? Z_OK : err;
}
ਪੂਰਨ ZEXPORT compress2(Bytef *dest, uLongf *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                      uLong ਸਰੋਤ_Len, ਪੂਰਨ level) {
    ਪੂਰਨ ret;
    z_ਮਾਪ_t got = *destLen;
    ret = compress2_z(dest, &got, ਸਰੋਤ, ਸਰੋਤ_Len, level);
    *destLen = (uLong)got;
    ਵਾਪਸ ret;
}
/* ===========================================================================
 */
ਪੂਰਨ ZEXPORT compress_z(Bytef *dest, z_ਮਾਪ_t *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                       z_ਮਾਪ_t ਸਰੋਤ_Len) {
    ਵਾਪਸ compress2_z(dest, destLen, ਸਰੋਤ, ਸਰੋਤ_Len,
                       Z_DEFAULT_COMPRESSION);
}
ਪੂਰਨ ZEXPORT compress(Bytef *dest, uLongf *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                     uLong ਸਰੋਤ_Len) {
    ਵਾਪਸ compress2(dest, destLen, ਸਰੋਤ, ਸਰੋਤ_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_ਮਾਪ_t ZEXPORT compressBound_z(z_ਮਾਪ_t ਸਰੋਤ_Len) {
    z_ਮਾਪ_t bound = ਸਰੋਤ_Len + (ਸਰੋਤ_Len >> 12) + (ਸਰੋਤ_Len >> 14) +
                     (ਸਰੋਤ_Len >> 25) + 13;
    ਵਾਪਸ bound < ਸਰੋਤ_Len ? (z_ਮਾਪ_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong ਸਰੋਤ_Len) {
    z_ਮਾਪ_t bound = compressBound_z(ਸਰੋਤ_Len);
    ਵਾਪਸ (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
