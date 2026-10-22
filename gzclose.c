/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
पूर्णाङ्क ZEXPORT gzclose(gzFile file) {
#ifndef NO_GZCOMPRESS
    gz_statep state;

    यदि (file == रिक्त)
        प्रत्यागम Z_STREAM_ERROR;
    state = (gz_statep)file;

    प्रत्यागम state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#अन्यथा
    प्रत्यागम gzclose_r(file);
#endif
}
