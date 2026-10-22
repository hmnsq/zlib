/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کرو ZLIB_اندرونی
#شامل "zlib.h"

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
صحیح ZEXPORT uncompress2_z(Bytef *dest, z_سائز_t *destLen, ثابت Bytef *ماخذ,
                          z_سائز_t *ماخذ_Len) {
    z_دھارا دھارا;
    صحیح err;
    ثابت uInt max = (uInt)-1;
    z_سائز_t len, بایاں;

    اگر (ماخذ_Len == تہی || (*ماخذ_Len > 0 && ماخذ == تہی) ||
        destLen == تہی || (*destLen > 0 && dest == تہی))
        واپس Z_دھارا_ERROR;

    len = *ماخذ_Len;
    بایاں = *destLen;
    اگر (بایاں == 0 && dest == Z_NULL)
        dest = (Bytef *)&دھارا.reserved;       /* next_out cannot be NULL */

    دھارا.اگلا_اندر = (z_const Bytef *)ماخذ;
    دھارا.avail_اندر = 0;
    دھارا.zalloc = (alloc_func)0;
    دھارا.zfree = (free_func)0;
    دھارا.opaque = (voidpf)0;

    err = inflateInit(&دھارا);
    اگر (err != Z_OK) واپس err;

    دھارا.اگلا_برآمد = dest;
    دھارا.avail_برآمد = 0;

    کرو {
        اگر (دھارا.avail_برآمد == 0) {
            دھارا.avail_برآمد = بایاں > (z_سائز_t)max ? max : (uInt)بایاں;
            بایاں -= دھارا.avail_برآمد;
        }
        اگر (دھارا.avail_اندر == 0) {
            دھارا.avail_اندر = len > (z_سائز_t)max ? max : (uInt)len;
            len -= دھارا.avail_اندر;
        }
        err = inflate(&دھارا, Z_NO_FLUSH);
    } جب (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += دھارا.avail_اندر;
    بایاں += دھارا.avail_برآمد;
    *ماخذ_Len -= len;
    *destLen -= بایاں;

    inflate_انتہا(&دھارا);
    واپس err == Z_دھارا_انتہا ? Z_OK :
           err == Z_NEED_DICT ? Z_ڈیٹا_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_ڈیٹا_ERROR :
           err;
}
صحیح ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ثابت Bytef *ماخذ,
                        uLong *ماخذ_Len) {
    صحیح ret;
    z_سائز_t got = *destLen, مستعمل = *ماخذ_Len;
    ret = uncompress2_z(dest, &got, ماخذ, &مستعمل);
    *ماخذ_Len = (uLong)مستعمل;
    *destLen = (uLong)got;
    واپس ret;
}
صحیح ZEXPORT uncompress_z(Bytef *dest, z_سائز_t *destLen, ثابت Bytef *ماخذ,
                         z_سائز_t ماخذ_Len) {
    z_سائز_t مستعمل = ماخذ_Len;
    واپس uncompress2_z(dest, destLen, ماخذ, &مستعمل);
}
صحیح ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ثابت Bytef *ماخذ,
                       uLong ماخذ_Len) {
    uLong مستعمل = ماخذ_Len;
    واپس uncompress2(dest, destLen, ماخذ, &مستعمل);
}
