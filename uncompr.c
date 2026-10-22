/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#تعریف_کړه ZLIB_دننی
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
صحيح ZEXPORT uncompress2_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *سرچینه,
                          z_اندازه_t *سرچینه_Len) {
    z_بهیر بهیر;
    صحيح err;
    ثابت uInt max = (uInt)-1;
    z_اندازه_t len, کیڼ;

    إذا (سرچینه_Len == عدم || (*سرچینه_Len > 0 && سرچینه == عدم) ||
        destLen == عدم || (*destLen > 0 && dest == عدم))
        أرجع Z_بهیر_ERROR;

    len = *سرچینه_Len;
    کیڼ = *destLen;
    إذا (کیڼ == 0 && dest == Z_NULL)
        dest = (Bytef *)&بهیر.reserved;       /* next_out cannot be NULL */

    بهیر.بل_دننه = (z_const Bytef *)سرچینه;
    بهیر.avail_دننه = 0;
    بهیر.zalloc = (alloc_func)0;
    بهیر.zfree = (free_func)0;
    بهیر.opaque = (voidpf)0;

    err = inflateInit(&بهیر);
    إذا (err != Z_OK) أرجع err;

    بهیر.بل_بهر = dest;
    بهیر.avail_بهر = 0;

    افعل {
        إذا (بهیر.avail_بهر == 0) {
            بهیر.avail_بهر = کیڼ > (z_اندازه_t)max ? max : (uInt)کیڼ;
            کیڼ -= بهیر.avail_بهر;
        }
        إذا (بهیر.avail_دننه == 0) {
            بهیر.avail_دننه = len > (z_اندازه_t)max ? max : (uInt)len;
            len -= بهیر.avail_دننه;
        }
        err = inflate(&بهیر, Z_NO_FLUSH);
    } طالما (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += بهیر.avail_دننه;
    کیڼ += بهیر.avail_بهر;
    *سرچینه_Len -= len;
    *destLen -= کیڼ;

    inflate_پای_ټکی(&بهیر);
    أرجع err == Z_بهیر_پای_ټکی ? Z_OK :
           err == Z_NEED_DICT ? Z_معلومات_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_معلومات_ERROR :
           err;
}
صحيح ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ثابت Bytef *سرچینه,
                        uLong *سرچینه_Len) {
    صحيح ret;
    z_اندازه_t got = *destLen, کارول_شوی = *سرچینه_Len;
    ret = uncompress2_z(dest, &got, سرچینه, &کارول_شوی);
    *سرچینه_Len = (uLong)کارول_شوی;
    *destLen = (uLong)got;
    أرجع ret;
}
صحيح ZEXPORT uncompress_z(Bytef *dest, z_اندازه_t *destLen, ثابت Bytef *سرچینه,
                         z_اندازه_t سرچینه_Len) {
    z_اندازه_t کارول_شوی = سرچینه_Len;
    أرجع uncompress2_z(dest, destLen, سرچینه, &کارول_شوی);
}
صحيح ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ثابت Bytef *سرچینه,
                       uLong سرچینه_Len) {
    uLong کارول_شوی = سرچینه_Len;
    أرجع uncompress2(dest, destLen, سرچینه, &کارول_شوی);
}
