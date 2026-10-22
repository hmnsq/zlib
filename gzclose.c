/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
ಪೂರ್ಣಾಂಕ ZEXPORT gzclose(gzFile file) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_GZCOMPRESS
    gz_statep state;

    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (gz_statep)file;

    ಹಿಂತಿರುಗಿ state->ಬಹುಲಕ == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ gzclose_r(file);
#ಆದರೆ_ಅಂತ್ಯ
}
