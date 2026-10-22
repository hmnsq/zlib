/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_ਅੰਦਰੂਨੀ
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

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
ਪੂਰਨ ZEXPORT uncompress2_z(Bytef *dest, z_ਮਾਪ_t *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                          z_ਮਾਪ_t *ਸਰੋਤ_Len) {
    z_ਧਾਰਾ ਧਾਰਾ;
    ਪੂਰਨ err;
    ਅਚਲ uInt max = (uInt)-1;
    z_ਮਾਪ_t len, ਖੱਬਾ;

    ਜੇ (ਸਰੋਤ_Len == ਨਿਰਾ || (*ਸਰੋਤ_Len > 0 && ਸਰੋਤ == ਨਿਰਾ) ||
        destLen == ਨਿਰਾ || (*destLen > 0 && dest == ਨਿਰਾ))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    len = *ਸਰੋਤ_Len;
    ਖੱਬਾ = *destLen;
    ਜੇ (ਖੱਬਾ == 0 && dest == Z_NULL)
        dest = (Bytef *)&ਧਾਰਾ.reserved;       /* next_out cannot be NULL */

    ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = (z_const Bytef *)ਸਰੋਤ;
    ਧਾਰਾ.avail_ਵਿੱਚ = 0;
    ਧਾਰਾ.zalloc = (alloc_func)0;
    ਧਾਰਾ.zfree = (free_func)0;
    ਧਾਰਾ.opaque = (voidpf)0;

    err = inflateInit(&ਧਾਰਾ);
    ਜੇ (err != Z_OK) ਵਾਪਸ err;

    ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = dest;
    ਧਾਰਾ.avail_ਬਾਹਰਲਾ = 0;

    ਕਰੋ {
        ਜੇ (ਧਾਰਾ.avail_ਬਾਹਰਲਾ == 0) {
            ਧਾਰਾ.avail_ਬਾਹਰਲਾ = ਖੱਬਾ > (z_ਮਾਪ_t)max ? max : (uInt)ਖੱਬਾ;
            ਖੱਬਾ -= ਧਾਰਾ.avail_ਬਾਹਰਲਾ;
        }
        ਜੇ (ਧਾਰਾ.avail_ਵਿੱਚ == 0) {
            ਧਾਰਾ.avail_ਵਿੱਚ = len > (z_ਮਾਪ_t)max ? max : (uInt)len;
            len -= ਧਾਰਾ.avail_ਵਿੱਚ;
        }
        err = inflate(&ਧਾਰਾ, Z_NO_FLUSH);
    } ਜਦੋਂ (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += ਧਾਰਾ.avail_ਵਿੱਚ;
    ਖੱਬਾ += ਧਾਰਾ.avail_ਬਾਹਰਲਾ;
    *ਸਰੋਤ_Len -= len;
    *destLen -= ਖੱਬਾ;

    inflate_ਸਿਰਾ(&ਧਾਰਾ);
    ਵਾਪਸ err == Z_ਧਾਰਾ_ਸਿਰਾ ? Z_OK :
           err == Z_NEED_DICT ? Z_ਅੰਕੜੇ_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_ਅੰਕੜੇ_ERROR :
           err;
}
ਪੂਰਨ ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                        uLong *ਸਰੋਤ_Len) {
    ਪੂਰਨ ret;
    z_ਮਾਪ_t got = *destLen, ਵਰਤਿਆ = *ਸਰੋਤ_Len;
    ret = uncompress2_z(dest, &got, ਸਰੋਤ, &ਵਰਤਿਆ);
    *ਸਰੋਤ_Len = (uLong)ਵਰਤਿਆ;
    *destLen = (uLong)got;
    ਵਾਪਸ ret;
}
ਪੂਰਨ ZEXPORT uncompress_z(Bytef *dest, z_ਮਾਪ_t *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                         z_ਮਾਪ_t ਸਰੋਤ_Len) {
    z_ਮਾਪ_t ਵਰਤਿਆ = ਸਰੋਤ_Len;
    ਵਾਪਸ uncompress2_z(dest, destLen, ਸਰੋਤ, &ਵਰਤਿਆ);
}
ਪੂਰਨ ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ਅਚਲ Bytef *ਸਰੋਤ,
                       uLong ਸਰੋਤ_Len) {
    uLong ਵਰਤਿਆ = ਸਰੋਤ_Len;
    ਵਾਪਸ uncompress2(dest, destLen, ਸਰੋਤ, &ਵਰਤਿਆ);
}
