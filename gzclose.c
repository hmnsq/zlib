/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
정수 ZEXPORT gzclose(gzFile file) {
#ifndef NO_GZCOMPRESS
    gz_statep state;

    만약 (file == NULL)
        반환 Z_STREAM_ERROR;
    state = (gz_statep)file;

    반환 state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#아니면
    반환 gzclose_r(file);
#endif
}
