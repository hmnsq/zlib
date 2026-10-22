/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
#अन्तर्भाव "zlib.h"

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
पूर्णाङ्क ZEXPORT uncompress2_z(Bytef *dest, z_size_t *destLen, ध्रुव Bytef *source,
                          z_size_t *sourceLen) {
    z_stream stream;
    पूर्णाङ्क err;
    ध्रुव uInt max = (uInt)-1;
    z_size_t len, वामः;

    जइ (sourceLen == रिक्त || (*sourceLen > 0 && source == रिक्त) ||
        destLen == रिक्त || (*destLen > 0 && dest == रिक्त))
        प्रत्यागम Z_STREAM_ERROR;

    len = *sourceLen;
    वामः = *destLen;
    जइ (वामः == 0 && dest == Z_NULL)
        dest = (Bytef *)&stream.reserved;       /* next_out cannot be NULL */

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = inflateInit(&stream);
    जइ (err != Z_OK) प्रत्यागम err;

    stream.next_out = dest;
    stream.avail_out = 0;

    करेहि {
        जइ (stream.avail_out == 0) {
            stream.avail_out = वामः > (z_size_t)max ? max : (uInt)वामः;
            वामः -= stream.avail_out;
        }
        जइ (stream.avail_in == 0) {
            stream.avail_in = len > (z_size_t)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } जाव (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += stream.avail_in;
    वामः += stream.avail_out;
    *sourceLen -= len;
    *destLen -= वामः;

    inflateEnd(&stream);
    प्रत्यागम err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_DATA_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_DATA_ERROR :
           err;
}
पूर्णाङ्क ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ध्रुव Bytef *source,
                        uLong *sourceLen) {
    पूर्णाङ्क ret;
    z_size_t got = *destLen, used = *sourceLen;
    ret = uncompress2_z(dest, &got, source, &used);
    *sourceLen = (uLong)used;
    *destLen = (uLong)got;
    प्रत्यागम ret;
}
पूर्णाङ्क ZEXPORT uncompress_z(Bytef *dest, z_size_t *destLen, ध्रुव Bytef *source,
                         z_size_t sourceLen) {
    z_size_t used = sourceLen;
    प्रत्यागम uncompress2_z(dest, destLen, source, &used);
}
पूर्णाङ्क ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ध्रुव Bytef *source,
                       uLong sourceLen) {
    uLong used = sourceLen;
    प्रत्यागम uncompress2(dest, destLen, source, &used);
}
