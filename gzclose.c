/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
ਪੂਰਨ ZEXPORT gzclose(gzFile file) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_GZCOMPRESS
    gz_statep state;

    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (gz_statep)file;

    ਵਾਪਸ state->ਬਹੁਲਕ == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#ਨਹੀਂਤਾਂ
    ਵਾਪਸ gzclose_r(file);
#ਜੇ_ਅੰਤ
}
