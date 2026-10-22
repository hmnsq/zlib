/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_ಆಂತರಿಕ
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
ಪೂರ್ಣಾಂಕ ZEXPORT compress2_z(Bytef *dest, z_ಗಾತ್ರ_t *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                        z_ಗಾತ್ರ_t ಮೂಲ_Len, ಪೂರ್ಣಾಂಕ level) {
    z_ಹರಿವು ಹರಿವು;
    ಪೂರ್ಣಾಂಕ err;
    ಸ್ಥಿರಾಂಕ uInt max = (uInt)-1;
    z_ಗಾತ್ರ_t ಎಡ;

    ಆದರೆ ((ಮೂಲ_Len > 0 && ಮೂಲ == ಖಾಲಿ) ||
        destLen == ಖಾಲಿ || (*destLen > 0 && dest == ಖಾಲಿ))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    ಎಡ = *destLen;
    *destLen = 0;

    ಹರಿವು.zalloc = (alloc_func)0;
    ಹರಿವು.zfree = (free_func)0;
    ಹರಿವು.opaque = (voidpf)0;

    err = deflateInit(&ಹರಿವು, level);
    ಆದರೆ (err != Z_OK) ಹಿಂತಿರುಗಿ err;

    ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = dest;
    ಹರಿವು.avail_ಹೊರಹೋಗು = 0;
    ಹರಿವು.ಮುಂದಿನ_ಒಳ = (z_const Bytef *)ಮೂಲ;
    ಹರಿವು.avail_ಒಳ = 0;

    ಮಾಡು {
        ಆದರೆ (ಹರಿವು.avail_ಹೊರಹೋಗು == 0) {
            ಹರಿವು.avail_ಹೊರಹೋಗು = ಎಡ > (z_ಗಾತ್ರ_t)max ? max : (uInt)ಎಡ;
            ಎಡ -= ಹರಿವು.avail_ಹೊರಹೋಗು;
        }
        ಆದರೆ (ಹರಿವು.avail_ಒಳ == 0) {
            ಹರಿವು.avail_ಒಳ = ಮೂಲ_Len > (z_ಗಾತ್ರ_t)max ? max :
                                                          (uInt)ಮೂಲ_Len;
            ಮೂಲ_Len -= ಹರಿವು.avail_ಒಳ;
        }
        err = deflate(&ಹರಿವು, ಮೂಲ_Len ? Z_NO_FLUSH : Z_ಮುಕ್ತಾಯ);
    } ಆಗ (err == Z_OK);

    *destLen = (z_ಗಾತ್ರ_t)(ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು - dest);
    deflate_ಕೊನೆ(&ಹರಿವು);
    ಹಿಂತಿರುಗಿ err == Z_ಹರಿವು_ಕೊನೆ ? Z_OK : err;
}
ಪೂರ್ಣಾಂಕ ZEXPORT compress2(Bytef *dest, uLongf *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                      uLong ಮೂಲ_Len, ಪೂರ್ಣಾಂಕ level) {
    ಪೂರ್ಣಾಂಕ ret;
    z_ಗಾತ್ರ_t got = *destLen;
    ret = compress2_z(dest, &got, ಮೂಲ, ಮೂಲ_Len, level);
    *destLen = (uLong)got;
    ಹಿಂತಿರುಗಿ ret;
}
/* ===========================================================================
 */
ಪೂರ್ಣಾಂಕ ZEXPORT compress_z(Bytef *dest, z_ಗಾತ್ರ_t *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                       z_ಗಾತ್ರ_t ಮೂಲ_Len) {
    ಹಿಂತಿರುಗಿ compress2_z(dest, destLen, ಮೂಲ, ಮೂಲ_Len,
                       Z_DEFAULT_COMPRESSION);
}
ಪೂರ್ಣಾಂಕ ZEXPORT compress(Bytef *dest, uLongf *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                     uLong ಮೂಲ_Len) {
    ಹಿಂತಿರುಗಿ compress2(dest, destLen, ಮೂಲ, ಮೂಲ_Len, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
z_ಗಾತ್ರ_t ZEXPORT compressBound_z(z_ಗಾತ್ರ_t ಮೂಲ_Len) {
    z_ಗಾತ್ರ_t bound = ಮೂಲ_Len + (ಮೂಲ_Len >> 12) + (ಮೂಲ_Len >> 14) +
                     (ಮೂಲ_Len >> 25) + 13;
    ಹಿಂತಿರುಗಿ bound < ಮೂಲ_Len ? (z_ಗಾತ್ರ_t)-1 : bound;
}
uLong ZEXPORT compressBound(uLong ಮೂಲ_Len) {
    z_ಗಾತ್ರ_t bound = compressBound_z(ಮೂಲ_Len);
    ಹಿಂತಿರುಗಿ (uLong)bound != bound ? (uLong)-1 : (uLong)bound;
}
