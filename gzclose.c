/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
จำนวนเต็ม ZEXPORT gzclose(gzFile file) {
#ifndef NO_GZCOMPRESS
    gz_statep state;

    ถ้า (file == NULL)
        คืน Z_STREAM_ERROR;
    state = (gz_statep)file;

    คืน state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#มิฉะนั้น
    คืน gzclose_r(file);
#endif
}
