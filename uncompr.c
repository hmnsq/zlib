/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
#অন্তর্ভাব "zlib.h"

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
পূর্ণসংখ্যা ZEXPORT uncompress2_z(Bytef *dest, z_আকার_t *destLen, ধ্রুবক Bytef *source,
                          z_আকার_t *sourceLen) {
    z_stream stream;
    পূর্ণসংখ্যা err;
    ধ্রুবক uInt max = (uInt)-1;
    z_আকার_t len, বাম;

    যদি (sourceLen == ফাঁকা || (*sourceLen > 0 && source == ফাঁকা) ||
        destLen == ফাঁকা || (*destLen > 0 && dest == ফাঁকা))
        ফেরত Z_STREAM_ERROR;

    len = *sourceLen;
    বাম = *destLen;
    যদি (বাম == 0 && dest == Z_NULL)
        dest = (Bytef *)&stream.reserved;       /* next_out cannot be NULL */

    stream.পরবর্তী_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = inflateInit(&stream);
    যদি (err != Z_OK) ফেরত err;

    stream.পরবর্তী_out = dest;
    stream.avail_out = 0;

    করো {
        যদি (stream.avail_out == 0) {
            stream.avail_out = বাম > (z_আকার_t)max ? max : (uInt)বাম;
            বাম -= stream.avail_out;
        }
        যদি (stream.avail_in == 0) {
            stream.avail_in = len > (z_আকার_t)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } যখন (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += stream.avail_in;
    বাম += stream.avail_out;
    *sourceLen -= len;
    *destLen -= বাম;

    inflateEnd(&stream);
    ফেরত err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_তথ্য_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_তথ্য_ERROR :
           err;
}
পূর্ণসংখ্যা ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ধ্রুবক Bytef *source,
                        uLong *sourceLen) {
    পূর্ণসংখ্যা ret;
    z_আকার_t got = *destLen, used = *sourceLen;
    ret = uncompress2_z(dest, &got, source, &used);
    *sourceLen = (uLong)used;
    *destLen = (uLong)got;
    ফেরত ret;
}
পূর্ণসংখ্যা ZEXPORT uncompress_z(Bytef *dest, z_আকার_t *destLen, ধ্রুবক Bytef *source,
                         z_আকার_t sourceLen) {
    z_আকার_t used = sourceLen;
    ফেরত uncompress2_z(dest, destLen, source, &used);
}
পূর্ণসংখ্যা ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ধ্রুবক Bytef *source,
                       uLong sourceLen) {
    uLong used = sourceLen;
    ফেরত uncompress2(dest, destLen, source, &used);
}
