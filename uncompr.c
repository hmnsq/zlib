/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_ಆಂತರಿಕ
#ಅನ್ತರ್ಭಾವ "zlib.h"

/* ===========================================================================
     Decompresses the source buffer into the destination buffer.  *sourceLen is
   the byte length of the source buffer. Upon entry, *destLen is the total size
   of the destination buffer, which must be large enough to hold the entire
   uncompressed data. (The size of the uncompressed data must have been saved
   previously by the compressor and transmitted to the decompressor by some
   mechanism outside the scope of this compression library.) Upon exit,
   *destLen is the size of the decompressed data and *sourceLen is the number
   of source bytes consumed. Upon return, source + *sourceLen points to the
   first unused input byte.

     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer, or
   Z_DATA_ERROR if the input data was corrupted, including if the input data is
   an incomplete zlib stream.

     The _z versions of the functions take size_t length arguments.
*/
ಪೂರ್ಣಾಂಕ ZEXPORT uncompress2_z(Bytef *dest, z_ಗಾತ್ರ_t *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                          z_ಗಾತ್ರ_t *ಮೂಲ_Len) {
    z_ಹರಿವು ಹರಿವು;
    ಪೂರ್ಣಾಂಕ err;
    ಸ್ಥಿರಾಂಕ uInt max = (uInt)-1;
    z_ಗಾತ್ರ_t len, ಎಡ;

    ಆದರೆ (ಮೂಲ_Len == ಖಾಲಿ || (*ಮೂಲ_Len > 0 && ಮೂಲ == ಖಾಲಿ) ||
        destLen == ಖಾಲಿ || (*destLen > 0 && dest == ಖಾಲಿ))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    len = *ಮೂಲ_Len;
    ಎಡ = *destLen;
    ಆದರೆ (ಎಡ == 0 && dest == Z_NULL)
        dest = (Bytef *)&ಹರಿವು.reserved;       /* next_out cannot be NULL */

    ಹರಿವು.ಮುಂದಿನ_ಒಳ = (z_const Bytef *)ಮೂಲ;
    ಹರಿವು.avail_ಒಳ = 0;
    ಹರಿವು.zalloc = (alloc_func)0;
    ಹರಿವು.zfree = (free_func)0;
    ಹರಿವು.opaque = (voidpf)0;

    err = inflateInit(&ಹರಿವು);
    ಆದರೆ (err != Z_OK) ಹಿಂತಿರುಗಿ err;

    ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = dest;
    ಹರಿವು.avail_ಹೊರಹೋಗು = 0;

    ಮಾಡು {
        ಆದರೆ (ಹರಿವು.avail_ಹೊರಹೋಗು == 0) {
            ಹರಿವು.avail_ಹೊರಹೋಗು = ಎಡ > (z_ಗಾತ್ರ_t)max ? max : (uInt)ಎಡ;
            ಎಡ -= ಹರಿವು.avail_ಹೊರಹೋಗು;
        }
        ಆದರೆ (ಹರಿವು.avail_ಒಳ == 0) {
            ಹರಿವು.avail_ಒಳ = len > (z_ಗಾತ್ರ_t)max ? max : (uInt)len;
            len -= ಹರಿವು.avail_ಒಳ;
        }
        err = inflate(&ಹರಿವು, Z_NO_FLUSH);
    } ಆಗ (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += ಹರಿವು.avail_ಒಳ;
    ಎಡ += ಹರಿವು.avail_ಹೊರಹೋಗು;
    *ಮೂಲ_Len -= len;
    *destLen -= ಎಡ;

    inflate_ಕೊನೆ(&ಹರಿವು);
    ಹಿಂತಿರುಗಿ err == Z_ಹರಿವು_ಕೊನೆ ? Z_OK :
           err == Z_NEED_DICT ? Z_ದತ್ತಾಂಶ_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_ದತ್ತಾಂಶ_ERROR :
           err;
}
ಪೂರ್ಣಾಂಕ ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                        uLong *ಮೂಲ_Len) {
    ಪೂರ್ಣಾಂಕ ret;
    z_ಗಾತ್ರ_t got = *destLen, ಬಳಸಿದ = *ಮೂಲ_Len;
    ret = uncompress2_z(dest, &got, ಮೂಲ, &ಬಳಸಿದ);
    *ಮೂಲ_Len = (uLong)ಬಳಸಿದ;
    *destLen = (uLong)got;
    ಹಿಂತಿರುಗಿ ret;
}
ಪೂರ್ಣಾಂಕ ZEXPORT uncompress_z(Bytef *dest, z_ಗಾತ್ರ_t *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                         z_ಗಾತ್ರ_t ಮೂಲ_Len) {
    z_ಗಾತ್ರ_t ಬಳಸಿದ = ಮೂಲ_Len;
    ಹಿಂತಿರುಗಿ uncompress2_z(dest, destLen, ಮೂಲ, &ಬಳಸಿದ);
}
ಪೂರ್ಣಾಂಕ ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ಸ್ಥಿರಾಂಕ Bytef *ಮೂಲ,
                       uLong ಮೂಲ_Len) {
    uLong ಬಳಸಿದ = ಮೂಲ_Len;
    ಹಿಂತಿರುಗಿ uncompress2(dest, destLen, ಮೂಲ, &ಬಳಸಿದ);
}
