/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کن ZLIB_داخلی
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
صحيح ZEXPORT uncompress2_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *منبع,
                          z_اندازه_t *منبع_Len) {
    z_جریان جریان;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_اندازه_t len, چپ;

    إذا (منبع_Len == عدم || (*منبع_Len > 0 && منبع == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_جریان_ERROR;

    len = *منبع_Len;
    چپ = *destLen;
    إذا (چپ == 0 && dest == Z_NULL)
        dest = (Bytef *)&جریان.reserved;       /* next_out cannot be NULL */

    جریان.بعدی_درون = (z_const Bytef *)منبع;
    جریان.avail_درون = 0;
    جریان.zalloc = (alloc_func)0;
    جریان.zfree = (free_func)0;
    جریان.opaque = (voidpf)0;

    err = inflateInit(&جریان);
    إذا (err != Z_OK) أرجع err;

    جریان.بعدی_بیرون = dest;
    جریان.avail_بیرون = 0;

    افعل {
        إذا (جریان.avail_بیرون == 0) {
            جریان.avail_بیرون = چپ > (z_اندازه_t)max ? max : (uInt)چپ;
            چپ -= جریان.avail_بیرون;
        }
        إذا (جریان.avail_درون == 0) {
            جریان.avail_درون = len > (z_اندازه_t)max ? max : (uInt)len;
            len -= جریان.avail_درون;
        }
        err = inflate(&جریان, Z_NO_FLUSH);
    } طالما (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += جریان.avail_درون;
    چپ += جریان.avail_بیرون;
    *منبع_Len -= len;
    *destLen -= چپ;

    inflate_انتها(&جریان);
    أرجع err == Z_جریان_انتها ? Z_OK :
           err == Z_NEED_DICT ? Z_داده_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_داده_ERROR :
           err;
}
صحيح ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ثابت Bytef *منبع,
                        uLong *منبع_Len) {
    صحيح ret;
    z_اندازه_t got = *destLen, استفاده_شده = *منبع_Len;
    ret = uncompress2_z(dest, &got, منبع, &استفاده_شده);
    *منبع_Len = (uLong)استفاده_شده;
    *destLen = (uLong)got;
    أرجع ret;
}
صحيح ZEXPORT uncompress_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *منبع,
                         z_اندازه_t منبع_Len) {
    z_اندازه_t استفاده_شده = منبع_Len;
    أرجع uncompress2_z(dest, destLen, منبع, &استفاده_شده);
}
صحيح ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ثابت Bytef *منبع,
                       uLong منبع_Len) {
    uLong استفاده_شده = منبع_Len;
    أرجع uncompress2(dest, destLen, منبع, &استفاده_شده);
}
