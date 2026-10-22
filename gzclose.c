/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
صحيح ZEXPORT gzclose(gzFile file) {
#جے_بنا_تعریف NO_GZCOMPRESS
    gz_statep state;

    إذا (file == عدم)
        أرجع Z_دھارا_ERROR;
    state = (gz_statep)file;

    أرجع state->منوال == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#وإلا
    أرجع gzclose_r(file);
#جے_انت
}
