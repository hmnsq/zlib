/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#அந்தர்பாவ "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
முழு ZEXPORT gzclose(gzFile file) {
#ifndef NO_GZCOMPRESS
    gz_statep state;

    எனில் (file == வெற்று)
        திரும்பு Z_STREAM_ERROR;
    state = (gz_statep)file;

    திரும்பு state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#இல்லையெனில்
    திரும்பு gzclose_r(file);
#endif
}
