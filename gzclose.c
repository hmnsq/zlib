/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#അന്തര്ഭാവ "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
പൂർണ്ണസംഖ്യ ZEXPORT gzclose(gzFile file) {
#ifndef NO_GZCOMPRESS
    gz_statep state;

    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ Z_STREAM_ERROR;
    state = (gz_statep)file;

    തിരികെ state->mode == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#അല്ലെങ്കിൽ
    തിരികെ gzclose_r(file);
#endif
}
