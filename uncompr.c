/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#وصف_ڪريو ZLIB_اندروني
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
صحيح ZEXPORT uncompress2_z(Bytef *dest, z_سائيز_t *destLen, ثابت Bytef *ذريعو,
                          z_سائيز_t *ذريعو_Len) {
    z_وهڪرو وهڪرو;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_سائيز_t len, کاٻو;

    إذا (ذريعو_Len == عدم || (*ذريعو_Len > 0 && ذريعو == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_وهڪرو_ERROR;

    len = *ذريعو_Len;
    کاٻو = *destLen;
    إذا (کاٻو == 0 && dest == Z_NULL)
        dest = (Bytef *)&وهڪرو.reserved;       /* next_out cannot be NULL */

    وهڪرو.ايندڙ_اندريون = (z_const Bytef *)ذريعو;
    وهڪرو.avail_اندريون = 0;
    وهڪرو.zalloc = (alloc_func)0;
    وهڪرو.zfree = (free_func)0;
    وهڪرو.opaque = (voidpf)0;

    err = inflateInit(&وهڪرو);
    إذا (err != Z_OK) أرجع err;

    وهڪرو.ايندڙ_ٻاهريون = dest;
    وهڪرو.avail_ٻاهريون = 0;

    افعل {
        إذا (وهڪرو.avail_ٻاهريون == 0) {
            وهڪرو.avail_ٻاهريون = کاٻو > (z_سائيز_t)max ? max : (uInt)کاٻو;
            کاٻو -= وهڪرو.avail_ٻاهريون;
        }
        إذا (وهڪرو.avail_اندريون == 0) {
            وهڪرو.avail_اندريون = len > (z_سائيز_t)max ? max : (uInt)len;
            len -= وهڪرو.avail_اندريون;
        }
        err = inflate(&وهڪرو, Z_NO_FLUSH);
    } طالما (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += وهڪرو.avail_اندريون;
    کاٻو += وهڪرو.avail_ٻاهريون;
    *ذريعو_Len -= len;
    *destLen -= کاٻو;

    inflate_پڇاڙي(&وهڪرو);
    أرجع err == Z_وهڪرو_پڇاڙي ? Z_OK :
           err == Z_NEED_DICT ? Z_معلومات_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_معلومات_ERROR :
           err;
}
صحيح ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ثابت Bytef *ذريعو,
                        uLong *ذريعو_Len) {
    صحيح ret;
    z_سائيز_t got = *destLen, استعمال_ٿيل = *ذريعو_Len;
    ret = uncompress2_z(dest, &got, ذريعو, &استعمال_ٿيل);
    *ذريعو_Len = (uLong)استعمال_ٿيل;
    *destLen = (uLong)got;
    أرجع ret;
}
صحيح ZEXPORT uncompress_z(Bytef *dest, z_سائيز_t *destLen, ثابت Bytef *ذريعو,
                         z_سائيز_t ذريعو_Len) {
    z_سائيز_t استعمال_ٿيل = ذريعو_Len;
    أرجع uncompress2_z(dest, destLen, ذريعو, &استعمال_ٿيل);
}
صحيح ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ثابت Bytef *ذريعو,
                       uLong ذريعو_Len) {
    uLong استعمال_ٿيل = ذريعو_Len;
    أرجع uncompress2(dest, destLen, ذريعو, &استعمال_ٿيل);
}
