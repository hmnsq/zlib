/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#అన్తర్భావ "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
పూర్ణాంకం ZEXPORT gzclose(gzFile file) {
#నిర్వచితం_కాకపోతే NO_GZCOMPRESS
    gz_statep state;

    ఒకవేళ (file == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (gz_statep)file;

    తిరిగి state->బహుళకం == GZ_READ ? gzclose_r(file) : gzclose_w(file);
#లేకపోతే
    తిరిగి gzclose_r(file);
#అయితే_ముగింపు
}
