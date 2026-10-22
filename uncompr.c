/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#घोषणा ZLIB_INTERNAL
#समावेश "zlib.h"

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
पूर्णांक ZEXPORT uncompress2_z(Bytef *dest, z_आकार_t *destLen, स्थिर Bytef *source,
                          z_आकार_t *sourceLen) {
    z_stream stream;
    पूर्णांक err;
    स्थिर uInt max = (uInt)-1;
    z_आकार_t len, बायाँ;

    अगर (sourceLen == शून्य || (*sourceLen > 0 && source == शून्य) ||
        destLen == शून्य || (*destLen > 0 && dest == शून्य))
        वापस Z_STREAM_ERROR;

    len = *sourceLen;
    बायाँ = *destLen;
    अगर (बायाँ == 0 && dest == Z_NULL)
        dest = (Bytef *)&stream.reserved;       /* next_out cannot be NULL */

    stream.अग्र_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = inflateInit(&stream);
    अगर (err != Z_OK) वापस err;

    stream.अग्र_out = dest;
    stream.avail_out = 0;

    करो {
        अगर (stream.avail_out == 0) {
            stream.avail_out = बायाँ > (z_आकार_t)max ? max : (uInt)बायाँ;
            बायाँ -= stream.avail_out;
        }
        अगर (stream.avail_in == 0) {
            stream.avail_in = len > (z_आकार_t)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } जबतक (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += stream.avail_in;
    बायाँ += stream.avail_out;
    *sourceLen -= len;
    *destLen -= बायाँ;

    inflateEnd(&stream);
    वापस err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_तथ्य_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_तथ्य_ERROR :
           err;
}
पूर्णांक ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, स्थिर Bytef *source,
                        uLong *sourceLen) {
    पूर्णांक ret;
    z_आकार_t got = *destLen, used = *sourceLen;
    ret = uncompress2_z(dest, &got, source, &used);
    *sourceLen = (uLong)used;
    *destLen = (uLong)got;
    वापस ret;
}
पूर्णांक ZEXPORT uncompress_z(Bytef *dest, z_आकार_t *destLen, स्थिर Bytef *source,
                         z_आकार_t sourceLen) {
    z_आकार_t used = sourceLen;
    वापस uncompress2_z(dest, destLen, source, &used);
}
पूर्णांक ZEXPORT uncompress(Bytef *dest, uLongf *destLen, स्थिर Bytef *source,
                       uLong sourceLen) {
    uLong used = sourceLen;
    वापस uncompress2(dest, destLen, source, &used);
}
