/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کرو ZLIB_اندرونی
#ضمن "zlib.h"

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
صحيح ZEXPORT uncompress2_z(Bytef *dest, z_ناپ_t *destLen, ثابت Bytef *سروت,
                          z_ناپ_t *سروت_Len) {
    z_دھارا دھارا;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_ناپ_t len, بایاں;

    إذا (سروت_Len == عدم || (*سروت_Len > 0 && سروت == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_دھارا_ERROR;

    len = *سروت_Len;
    بایاں = *destLen;
    إذا (بایاں == 0 && dest == Z_NULL)
        dest = (Bytef *)&دھارا.reserved;       /* next_out cannot be NULL */

    دھارا.اگلا_وچ = (z_const Bytef *)سروت;
    دھارا.avail_وچ = 0;
    دھارا.zalloc = (alloc_func)0;
    دھارا.zfree = (free_func)0;
    دھارا.opaque = (voidpf)0;

    err = inflateInit(&دھارا);
    إذا (err != Z_OK) أرجع err;

    دھارا.اگلا_باہرلا = dest;
    دھارا.avail_باہرلا = 0;

    افعل {
        إذا (دھارا.avail_باہرلا == 0) {
            دھارا.avail_باہرلا = بایاں > (z_ناپ_t)max ? max : (uInt)بایاں;
            بایاں -= دھارا.avail_باہرلا;
        }
        إذا (دھارا.avail_وچ == 0) {
            دھارا.avail_وچ = len > (z_ناپ_t)max ? max : (uInt)len;
            len -= دھارا.avail_وچ;
        }
        err = inflate(&دھارا, Z_NO_FLUSH);
    } طالما (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += دھارا.avail_وچ;
    بایاں += دھارا.avail_باہرلا;
    *سروت_Len -= len;
    *destLen -= بایاں;

    inflate_سرا(&دھارا);
    أرجع err == Z_دھارا_سرا ? Z_OK :
           err == Z_NEED_DICT ? Z_انکڑے_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_انکڑے_ERROR :
           err;
}
صحيح ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ثابت Bytef *سروت,
                        uLong *سروت_Len) {
    صحيح ret;
    z_ناپ_t got = *destLen, ورتیا = *سروت_Len;
    ret = uncompress2_z(dest, &got, سروت, &ورتیا);
    *سروت_Len = (uLong)ورتیا;
    *destLen = (uLong)got;
    أرجع ret;
}
صحيح ZEXPORT uncompress_z(Bytef *dest, z_ناپ_t *destLen, ثابت Bytef *سروت,
                         z_ناپ_t سروت_Len) {
    z_ناپ_t ورتیا = سروت_Len;
    أرجع uncompress2_z(dest, destLen, سروت, &ورتیا);
}
صحيح ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ثابت Bytef *سروت,
                       uLong سروت_Len) {
    uLong ورتیا = سروت_Len;
    أرجع uncompress2(dest, destLen, سروت, &ورتیا);
}
