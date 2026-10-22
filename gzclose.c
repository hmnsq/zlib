/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
ཧྲིལ ZEXPORT gzclose(gzFile file) {
#ifndef NO_GZCOMPRESS
    gz_statep state;

    གལ་ཏེ (file == NULL)
        ཕྱིར Z_STREAM_ERROR;
    state = (gz_statep)file;

    ཕྱིར state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#དེ་མིན
    ཕྱིར gzclose_r(file);
#endif
}
