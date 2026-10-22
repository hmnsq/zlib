/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#समावेश "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
पूर्णांक ZEXPORT gzclose(gzFile file) {
#अगर_अघोषित NO_GZCOMPRESS
    gz_statep state;

    अगर (file == शून्य)
        वापस Z_STREAM_ERROR;
    state = (gz_statep)file;

    वापस state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#अन्यथा
    वापस gzclose_r(file);
#पूर्ण_अगर
}
