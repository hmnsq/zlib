/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ଅନ୍ତର୍ଭାଵ "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzclose(gzFile file) {
#ifndef NO_GZCOMPRESS
    gz_statep state;

    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (gz_statep)file;

    ଫେରନ୍ତୁ state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#ଅନ୍ୟଥା
    ଫେରନ୍ତୁ gzclose_r(file);
#endif
}
