/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#परिभाषय ZLIB_आन्तरिकम्
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
पूर्णाङ्क ZEXPORT uncompress2_z(Bytef *dest, z_आकार_t *destLen, ध्रुव Bytef *source,
                          z_आकार_t *sourceLen) {
    z_प्रवाहः प्रवाहः;
    पूर्णाङ्क err;
    ध्रुव uInt max = (uInt)-1;
    z_आकार_t len, वामः;

    यदि (sourceLen == रिक्त || (*sourceLen > 0 && source == रिक्त) ||
        destLen == रिक्त || (*destLen > 0 && dest == रिक्त))
        प्रत्यागम Z_प्रवाहः_ERROR;

    len = *sourceLen;
    वामः = *destLen;
    यदि (वामः == 0 && dest == Z_NULL)
        dest = (Bytef *)&प्रवाहः.reserved;       /* next_out cannot be NULL */

    प्रवाहः.अग्र_अन्तर् = (z_const Bytef *)source;
    प्रवाहः.avail_अन्तर् = 0;
    प्रवाहः.zalloc = (alloc_func)0;
    प्रवाहः.zfree = (free_func)0;
    प्रवाहः.opaque = (voidpf)0;

    err = inflateInit(&प्रवाहः);
    यदि (err != Z_OK) प्रत्यागम err;

    प्रवाहः.अग्र_बहिर्गमनम् = dest;
    प्रवाहः.avail_बहिर्गमनम् = 0;

    कुरु {
        यदि (प्रवाहः.avail_बहिर्गमनम् == 0) {
            प्रवाहः.avail_बहिर्गमनम् = वामः > (z_आकार_t)max ? max : (uInt)वामः;
            वामः -= प्रवाहः.avail_बहिर्गमनम्;
        }
        यदि (प्रवाहः.avail_अन्तर् == 0) {
            प्रवाहः.avail_अन्तर् = len > (z_आकार_t)max ? max : (uInt)len;
            len -= प्रवाहः.avail_अन्तर्;
        }
        err = inflate(&प्रवाहः, Z_NO_FLUSH);
    } यावत् (err == Z_OK);

    /* Set len and left to the unused input data and unused output space. Set
       *sourceLen to the amount of input consumed. Set *destLen to the amount
       of data produced. */
    len += प्रवाहः.avail_अन्तर्;
    वामः += प्रवाहः.avail_बहिर्गमनम्;
    *sourceLen -= len;
    *destLen -= वामः;

    inflate_अन्तः(&प्रवाहः);
    प्रत्यागम err == Z_प्रवाहः_अन्तः ? Z_OK :
           err == Z_NEED_DICT ? Z_दत्त_ERROR  :
           err == Z_BUF_ERROR && len == 0 ? Z_दत्त_ERROR :
           err;
}
पूर्णाङ्क ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, ध्रुव Bytef *source,
                        uLong *sourceLen) {
    पूर्णाङ्क ret;
    z_आकार_t got = *destLen, प्रयुक्तम् = *sourceLen;
    ret = uncompress2_z(dest, &got, source, &प्रयुक्तम्);
    *sourceLen = (uLong)प्रयुक्तम्;
    *destLen = (uLong)got;
    प्रत्यागम ret;
}
पूर्णाङ्क ZEXPORT uncompress_z(Bytef *dest, z_आकार_t *destLen, ध्रुव Bytef *source,
                         z_आकार_t sourceLen) {
    z_आकार_t प्रयुक्तम् = sourceLen;
    प्रत्यागम uncompress2_z(dest, destLen, source, &प्रयुक्तम्);
}
पूर्णाङ्क ZEXPORT uncompress(Bytef *dest, uLongf *destLen, ध्रुव Bytef *source,
                       uLong sourceLen) {
    uLong प्रयुक्तम् = sourceLen;
    प्रत्यागम uncompress2(dest, destLen, source, &प्रयुक्तम्);
}
