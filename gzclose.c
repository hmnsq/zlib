/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
صحیح ZEXPORT gzclose(gzFile file) {
#اگر_غیر_تعریف_شدہ NO_GZCOMPRESS
    gz_statep state;

    اگر (file == تہی)
        واپس Z_دھارا_ERROR;
    state = (gz_statep)file;

    واپس state->منوال == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#ورنہ
    واپس gzclose_r(file);
#اگر_ختم
}
