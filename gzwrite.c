/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ಪೂರ್ಣಾಂಕ gz_init(gz_statep state) {
    ಪೂರ್ಣಾಂಕ ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->ಒಳ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ಕೊಡು(state->want << 1);
    ಆದರೆ (state->ಒಳ == ಖಾಲಿ) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ಹಿಂತಿರುಗಿ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ಆದರೆ (!state->ನೇರ) {
        /* allocate output buffer */
        state->ಹೊರಹೋಗು = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ಕೊಡು(state->want);
        ಆದರೆ (state->ಹೊರಹೋಗು == ಖಾಲಿ) {
            ಬಿಡುಗಡೆ(state->ಒಳ);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ಹಿಂತಿರುಗಿ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ಆದರೆ (ret != Z_OK) {
            ಬಿಡುಗಡೆ(state->ಹೊರಹೋಗು);
            ಬಿಡುಗಡೆ(state->ಒಳ);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ಹಿಂತಿರುಗಿ -1;
        }
        strm->ಮುಂದಿನ_ಒಳ = ಖಾಲಿ;
    }

    /* mark state as initialized */
    state->ಗಾತ್ರ_2 = state->want;

    /* initialize write buffer if compressing */
    ಆದರೆ (!state->ನೇರ) {
        strm->avail_ಹೊರಹೋಗು = state->ಗಾತ್ರ_2;
        strm->ಮುಂದಿನ_ಹೊರಹೋಗು = state->ಹೊರಹೋಗು;
        state->x.ಮುಂದಿನ = strm->ಮುಂದಿನ_ಹೊರಹೋಗು;
    }
    ಹಿಂತಿರುಗಿ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ಪೂರ್ಣಾಂಕ gz_comp(gz_statep state, ಪೂರ್ಣಾಂಕ flush) {
    ಪೂರ್ಣಾಂಕ ret, writ;
    ಚಿಹ್ನೆರಹಿತ have, put, max = ((ಚಿಹ್ನೆರಹಿತ)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ಆದರೆ (state->ಗಾತ್ರ_2 == 0 && gz_init(state) == -1)
        ಹಿಂತಿರುಗಿ -1;

    /* write directly if requested */
    ಆದರೆ (state->ನೇರ) {
        ಆಗ (strm->avail_ಒಳ) {
            errno = 0;
            state->again = 0;
            put = strm->avail_ಒಳ > max ? max : strm->avail_ಒಳ;
            writ = (ಪೂರ್ಣಾಂಕ)ಲಿಖ(state->fd, strm->ಮುಂದಿನ_ಒಳ, put);
            ಆದರೆ (writ < 0) {
                ಆದರೆ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ಹಿಂತಿರುಗಿ -1;
            }
            strm->avail_ಒಳ -= (ಚಿಹ್ನೆರಹಿತ)writ;
            strm->ಮುಂದಿನ_ಒಳ += writ;
        }
        ಹಿಂತಿರುಗಿ 0;
    }

    /* check for a pending reset */
    ಆದರೆ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ಆದರೆ (strm->avail_ಒಳ == 0 && flush == Z_NO_FLUSH)
            ಹಿಂತಿರುಗಿ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    ಮಾಡು {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ಆದರೆ (strm->avail_ಹೊರಹೋಗು == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_ಮುಕ್ತಾಯ || ret == Z_ಹರಿವು_ಕೊನೆ))) {
            ಆಗ (strm->ಮುಂದಿನ_ಹೊರಹೋಗು > state->x.ಮುಂದಿನ) {
                errno = 0;
                state->again = 0;
                put = strm->ಮುಂದಿನ_ಹೊರಹೋಗು - state->x.ಮುಂದಿನ > (ಪೂರ್ಣಾಂಕ)max ? max :
                      (ಚಿಹ್ನೆರಹಿತ)(strm->ಮುಂದಿನ_ಹೊರಹೋಗು - state->x.ಮುಂದಿನ);
                writ = (ಪೂರ್ಣಾಂಕ)ಲಿಖ(state->fd, state->x.ಮುಂದಿನ, put);
                ಆದರೆ (writ < 0) {
                    ಆದರೆ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ಹಿಂತಿರುಗಿ -1;
                }
                state->x.ಮುಂದಿನ += writ;
            }
            ಆದರೆ (strm->avail_ಹೊರಹೋಗು == 0) {
                strm->avail_ಹೊರಹೋಗು = state->ಗಾತ್ರ_2;
                strm->ಮುಂದಿನ_ಹೊರಹೋಗು = state->ಹೊರಹೋಗು;
                state->x.ಮುಂದಿನ = state->ಹೊರಹೋಗು;
            }
        }

        /* compress */
        have = strm->avail_ಹೊರಹೋಗು;
        ret = deflate(strm, flush);
        ಆದರೆ (ret == Z_ಹರಿವು_ERROR) {
            gz_error(state, Z_ಹರಿವು_ERROR,
                      "internal error: deflate stream corrupt");
            ಹಿಂತಿರುಗಿ -1;
        }
        have -= strm->avail_ಹೊರಹೋಗು;
    } ಆಗ (have);

    /* if that completed a deflate stream, allow another to start */
    ಆದರೆ (flush == Z_ಮುಕ್ತಾಯ)
        state->reset = 1;

    /* all done, no errors */
    ಹಿಂತಿರುಗಿ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ಪೂರ್ಣಾಂಕ gz_zero(gz_statep state) {
    ಪೂರ್ಣಾಂಕ ಮೊದಲ, ret;
    ಚಿಹ್ನೆರಹಿತ n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ಆದರೆ (strm->avail_ಒಳ && gz_comp(state, Z_NO_FLUSH) == -1)
        ಹಿಂತಿರುಗಿ -1;

    /* compress state->skip zeros */
    ಮೊದಲ = 1;
    ಮಾಡು {
        n = GT_OFF(state->ಗಾತ್ರ_2) || (z_off64_t)state->ಗಾತ್ರ_2 > state->skip ?
            (ಚಿಹ್ನೆರಹಿತ)state->skip : state->ಗಾತ್ರ_2;
        ಆದರೆ (ಮೊದಲ) {
            memset(state->ಒಳ, 0, n);
            ಮೊದಲ = 0;
        }
        strm->avail_ಒಳ = n;
        strm->ಮುಂದಿನ_ಒಳ = state->ಒಳ;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_ಒಳ;
        state->x.pos += n;
        state->skip -= n;
        ಆದರೆ (ret == -1)
            ಹಿಂತಿರುಗಿ -1;
    } ಆಗ (state->skip);
    ಹಿಂತಿರುಗಿ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_ಗಾತ್ರ_t gz_write(gz_statep state, voidpc buf, z_ಗಾತ್ರ_t len) {
    z_ಗಾತ್ರ_t put = len;
    ಪೂರ್ಣಾಂಕ ret;

    /* if len is zero, avoid unnecessary operations */
    ಆದರೆ (len == 0)
        ಹಿಂತಿರುಗಿ 0;

    /* allocate memory if this is the first time through */
    ಆದರೆ (state->ಗಾತ್ರ_2 == 0 && gz_init(state) == -1)
        ಹಿಂತಿರುಗಿ 0;

    /* check for seek request */
    ಆದರೆ (state->skip && gz_zero(state) == -1)
        ಹಿಂತಿರುಗಿ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ಆದರೆ (len < state->ಗಾತ್ರ_2) {
        /* copy to input buffer, compress when full */
        ಗಾಗಿ (;;) {
            ಚಿಹ್ನೆರಹಿತ have, copy;

            ಆದರೆ (state->strm.avail_ಒಳ == 0)
                state->strm.ಮುಂದಿನ_ಒಳ = state->ಒಳ;
            have = (ಚಿಹ್ನೆರಹಿತ)((state->strm.ಮುಂದಿನ_ಒಳ + state->strm.avail_ಒಳ) -
                              state->ಒಳ);
            copy = state->ಗಾತ್ರ_2 - have;
            ಆದರೆ (copy > len)
                copy = (ಚಿಹ್ನೆರಹಿತ)len;
            ಸ್ಮರಣೆ_ನಕಲು(state->ಒಳ + have, buf, copy);
            state->strm.avail_ಒಳ += copy;
            state->x.pos += copy;
            buf = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)buf + copy;
            len -= copy;
            ಆದರೆ (len == 0)
                ನಿಲ್ಲಿಸು;
            ಆದರೆ (gz_comp(state, Z_NO_FLUSH) == -1)
                ಹಿಂತಿರುಗಿ state->again ? put - len : 0;
        }
    }
    ಇಲ್ಲದಿದ್ದರೆ {
        /* consume whatever's left in the input buffer */
        ಆದರೆ (state->strm.avail_ಒಳ && gz_comp(state, Z_NO_FLUSH) == -1)
            ಹಿಂತಿರುಗಿ 0;

        /* directly compress user buffer to file */
        state->strm.ಮುಂದಿನ_ಒಳ = (z_const Bytef *)buf;
        ಮಾಡು {
            ಚಿಹ್ನೆರಹಿತ n = (ಚಿಹ್ನೆರಹಿತ)-1;

            ಆದರೆ (n > len)
                n = (ಚಿಹ್ನೆರಹಿತ)len;
            state->strm.avail_ಒಳ = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_ಒಳ;
            state->x.pos += n;
            len -= n;
            ಆದರೆ (ret == -1) {
                state->strm.avail_ಒಳ = 0;
                state->strm.ಮುಂದಿನ_ಒಳ = state->ಒಳ;
                ಹಿಂತಿರುಗಿ state->again ? put - len : 0;
            }
        } ಆಗ (len);
    }

    /* input was all buffered or compressed */
    ಹಿಂತಿರುಗಿ put;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzwrite(gzFile file, voidpc buf, ಚಿಹ್ನೆರಹಿತ len) {
    gz_statep state;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಹಿಂತಿರುಗಿ 0;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ಆದರೆ ((ಪೂರ್ಣಾಂಕ)len < 0) {
        gz_error(state, Z_ದತ್ತಾಂಶ_ERROR, "requested length does not fit in int");
        ಹಿಂತಿರುಗಿ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_ಗಾತ್ರ_t ZEXPORT gzfwrite(voidpc buf, z_ಗಾತ್ರ_t ಗಾತ್ರ_2, z_ಗಾತ್ರ_t nitems,
                          gzFile file) {
    z_ಗಾತ್ರ_t len;
    gz_statep state;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಹಿಂತಿರುಗಿ 0;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* compute bytes to read -- error on overflow */
    len = nitems * ಗಾತ್ರ_2;
    ಆದರೆ (ಗಾತ್ರ_2 && len / ಗಾತ್ರ_2 != nitems) {
        gz_error(state, Z_ಹರಿವು_ERROR, "request does not fit in a size_t");
        ಹಿಂತಿರುಗಿ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ಹಿಂತಿರುಗಿ len ? gz_write(state, buf, len) / ಗಾತ್ರ_2 : 0;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzputc(gzFile file, ಪೂರ್ಣಾಂಕ c) {
    ಚಿಹ್ನೆರಹಿತ have;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಹಿಂತಿರುಗಿ -1;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* check for seek request */
    ಆದರೆ (state->skip && gz_zero(state) == -1)
        ಹಿಂತಿರುಗಿ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ಆದರೆ (state->ಗಾತ್ರ_2) {
        ಆದರೆ (strm->avail_ಒಳ == 0)
            strm->ಮುಂದಿನ_ಒಳ = state->ಒಳ;
        have = (ಚಿಹ್ನೆರಹಿತ)((strm->ಮುಂದಿನ_ಒಳ + strm->avail_ಒಳ) - state->ಒಳ);
        ಆದರೆ (have < state->ಗಾತ್ರ_2) {
            state->ಒಳ[have] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)c;
            strm->avail_ಒಳ++;
            state->x.pos++;
            ಹಿಂತಿರುಗಿ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)c;
    ಆದರೆ (gz_write(state, buf, 1) != 1)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ c & 0xff;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzputs(gzFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s) {
    z_ಗಾತ್ರ_t len, put;
    gz_statep state;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಹಿಂತಿರುಗಿ -1;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* write string */
    len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s);
    ಆದರೆ ((ಪೂರ್ಣಾಂಕ)len < 0 || (ಚಿಹ್ನೆರಹಿತ)len != len) {
        gz_error(state, Z_ಹರಿವು_ERROR, "string length does not fit in int");
        ಹಿಂತಿರುಗಿ -1;
    }
    put = gz_write(state, s, len);
    ಹಿಂತಿರುಗಿ len && put == 0 ? -1 : (ಪೂರ್ಣಾಂಕ)put;
}

#ಆದರೆ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ಪೂರ್ಣಾಂಕ gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ಆದರೆ (strm->ಮುಂದಿನ_ಒಳ == ಖಾಲಿ ||
        strm->ಮುಂದಿನ_ಒಳ + strm->avail_ಒಳ <= state->ಒಳ + state->ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ 0;
    (ಶೂನ್ಯ)gz_comp(state, Z_NO_FLUSH);
    ಆದರೆ (strm->avail_ಒಳ == 0) {
        strm->ಮುಂದಿನ_ಒಳ = state->ಒಳ;
        ಹಿಂತಿರುಗಿ 0;
    }
    memmove(state->ಒಳ, strm->ಮುಂದಿನ_ಒಳ, strm->avail_ಒಳ);
    strm->ಮುಂದಿನ_ಒಳ = state->ಒಳ;
    ಹಿಂತಿರುಗಿ strm->avail_ಒಳ > state->ಗಾತ್ರ_2;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(STDC) || defined(Z_HAVE_STDARG_H)
#ಅನ್ತರ್ಭಾವ <stdarg.h>

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORTVA gzvprintf(gzFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, va_list va) {
#ಆದರೆ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (ಶೂನ್ಯ)file, (ಶೂನ್ಯ)format, (ಶೂನ್ಯ)va;
    ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಪೂರ್ಣಾಂಕ len, ret;
    ಅಕ್ಷರ *ಮುಂದಿನ;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* make sure we have some buffer space */
    ಆದರೆ (state->ಗಾತ್ರ_2 == 0 && gz_init(state) == -1)
        ಹಿಂತಿರುಗಿ state->err;

    /* check for seek request */
    ಆದರೆ (state->skip && gz_zero(state) == -1)
        ಹಿಂತಿರುಗಿ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ಆದರೆ (state->err) {
        ಆದರೆ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ಹಿಂತಿರುಗಿ state->err;
        }
        ಆದರೆ (!state->again)
            ಹಿಂತಿರುಗಿ state->err;
    }
    ಆದರೆ (strm->avail_ಒಳ == 0)
        strm->ಮುಂದಿನ_ಒಳ = state->ಒಳ;
    ಮುಂದಿನ = (ಅಕ್ಷರ *)(state->ಒಳ + (strm->ಮುಂದಿನ_ಒಳ - state->ಒಳ) + strm->avail_ಒಳ);
    ಮುಂದಿನ[state->ಗಾತ್ರ_2 - 1] = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (ಶೂನ್ಯ)vsprintf(ಮುಂದಿನ, format, va);
    ಗಾಗಿ (len = 0; len < state->ಗಾತ್ರ_2; len++)
        ಆದರೆ (ಮುಂದಿನ[len] == 0) ನಿಲ್ಲಿಸು;
#  ಇಲ್ಲದಿದ್ದರೆ
    len = vsprintf(ಮುಂದಿನ, format, va);
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
#  ifdef HAS_vsnprintf_void
    (ಶೂನ್ಯ)vsnprintf(ಮುಂದಿನ, state->ಗಾತ್ರ_2, format, va);
    len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮುಂದಿನ);
#  ಇಲ್ಲದಿದ್ದರೆ
    len = vsnprintf(ಮುಂದಿನ, state->ಗಾತ್ರ_2, format, va);
#  endif
#ಆದರೆ_ಅಂತ್ಯ

    /* check that printf() results fit in buffer */
    ಆದರೆ (len == 0 || (ಚಿಹ್ನೆರಹಿತ)len >= state->ಗಾತ್ರ_2 || ಮುಂದಿನ[state->ಗಾತ್ರ_2 - 1] != 0)
        ಹಿಂತಿರುಗಿ 0;

    /* update buffer and position */
    strm->avail_ಒಳ += (ಚಿಹ್ನೆರಹಿತ)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ಆದರೆ (ret) {
        /* discard unwritten formatted output */
        strm->avail_ಒಳ -= (ಚಿಹ್ನೆರಹಿತ)len;
        state->x.pos -= len;
        ಆದರೆ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ಹಿಂತಿರುಗಿ state->err;
    }
    ಆದರೆ (state->err && !state->again)
        ಹಿಂತಿರುಗಿ state->err;
    ಹಿಂತಿರುಗಿ len;
#ಆದರೆ_ಅಂತ್ಯ
}

ಪೂರ್ಣಾಂಕ ZEXPORTVA gzprintf(gzFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...) {
    va_list va;
    ಪೂರ್ಣಾಂಕ ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ಹಿಂತಿರುಗಿ ret;
}

#ಇಲ್ಲದಿದ್ದರೆ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORTVA gzprintf(gzFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ಪೂರ್ಣಾಂಕ a1, ಪೂರ್ಣಾಂಕ a2, ಪೂರ್ಣಾಂಕ a3,
                       ಪೂರ್ಣಾಂಕ a4, ಪೂರ್ಣಾಂಕ a5, ಪೂರ್ಣಾಂಕ a6, ಪೂರ್ಣಾಂಕ a7, ಪೂರ್ಣಾಂಕ a8, ಪೂರ್ಣಾಂಕ a9, ಪೂರ್ಣಾಂಕ a10,
                       ಪೂರ್ಣಾಂಕ a11, ಪೂರ್ಣಾಂಕ a12, ಪೂರ್ಣಾಂಕ a13, ಪೂರ್ಣಾಂಕ a14, ಪೂರ್ಣಾಂಕ a15, ಪೂರ್ಣಾಂಕ a16,
                       ಪೂರ್ಣಾಂಕ a17, ಪೂರ್ಣಾಂಕ a18, ಪೂರ್ಣಾಂಕ a19, ಪೂರ್ಣಾಂಕ a20) {
#ಆದರೆ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (ಶೂನ್ಯ)file, (ಶೂನ್ಯ)format, (ಶೂನ್ಯ)a1, (ಶೂನ್ಯ)a2, (ಶೂನ್ಯ)a3, (ಶೂನ್ಯ)a4, (ಶೂನ್ಯ)a5,
    (ಶೂನ್ಯ)a6, (ಶೂನ್ಯ)a7, (ಶೂನ್ಯ)a8, (ಶೂನ್ಯ)a9, (ಶೂನ್ಯ)a10, (ಶೂನ್ಯ)a11, (ಶೂನ್ಯ)a12,
    (ಶೂನ್ಯ)a13, (ಶೂನ್ಯ)a14, (ಶೂನ್ಯ)a15, (ಶೂನ್ಯ)a16, (ಶೂನ್ಯ)a17, (ಶೂನ್ಯ)a18,
    (ಶೂನ್ಯ)a19, (ಶೂನ್ಯ)a20;
    ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ len, ಎಡ;
    ಅಕ್ಷರ *ಮುಂದಿನ;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ಆದರೆ (ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) != ಗಾತ್ರ(ಶೂನ್ಯ *))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* make sure we have some buffer space */
    ಆದರೆ (state->ಗಾತ್ರ_2 == 0 && gz_init(state) == -1)
        ಹಿಂತಿರುಗಿ state->err;

    /* check for seek request */
    ಆದರೆ (state->skip && gz_zero(state) == -1)
        ಹಿಂತಿರುಗಿ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ಆದರೆ (state->err) {
        ಆದರೆ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ಹಿಂತಿರುಗಿ state->err;
        }
        ಆದರೆ (!state->again)
            ಹಿಂತಿರುಗಿ state->err;
    }
    ಆದರೆ (strm->avail_ಒಳ == 0)
        strm->ಮುಂದಿನ_ಒಳ = state->ಒಳ;
    ಮುಂದಿನ = (ಅಕ್ಷರ *)(strm->ಮುಂದಿನ_ಒಳ + strm->avail_ಒಳ);
    ಮುಂದಿನ[state->ಗಾತ್ರ_2 - 1] = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(ಮುಂದಿನ, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    ಗಾಗಿ (len = 0; len < ಗಾತ್ರ_2; len++)
        ಆದರೆ (ಮುಂದಿನ[len] == 0)
            ನಿಲ್ಲಿಸು;
#  ಇಲ್ಲದಿದ್ದರೆ
    len = sprintf(ಮುಂದಿನ, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
#  ifdef HAS_snprintf_void
    snprintf(ಮುಂದಿನ, state->ಗಾತ್ರ_2, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮುಂದಿನ);
#  ಇಲ್ಲದಿದ್ದರೆ
    len = snprintf(ಮುಂದಿನ, state->ಗಾತ್ರ_2, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ಆದರೆ_ಅಂತ್ಯ

    /* check that printf() results fit in buffer */
    ಆದರೆ (len == 0 || len >= state->ಗಾತ್ರ_2 || ಮುಂದಿನ[state->ಗಾತ್ರ_2 - 1] != 0)
        ಹಿಂತಿರುಗಿ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_ಒಳ += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ಆದರೆ (ret) {
        /* discard unwritten formatted output */
        strm->avail_ಒಳ -= (ಚಿಹ್ನೆರಹಿತ)len;
        state->x.pos -= len;
        ಆದರೆ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ಹಿಂತಿರುಗಿ state->err;
    }
    ಆದರೆ (state->err && !state->again)
        ಹಿಂತಿರುಗಿ state->err;
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)len;
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ_ಅಂತ್ಯ

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzflush(gzFile file, ಪೂರ್ಣಾಂಕ flush) {
    gz_statep state;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* check flush parameter */
    ಆದರೆ (flush < 0 || flush > Z_ಮುಕ್ತಾಯ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    /* check for seek request */
    ಆದರೆ (state->skip && gz_zero(state) == -1)
        ಹಿಂತಿರುಗಿ state->err;

    /* compress remaining data with requested flush */
    (ಶೂನ್ಯ)gz_comp(state, flush);
    ಹಿಂತಿರುಗಿ state->err;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzsetparams(gzFile file, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->ನೇರ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    gz_error(state, Z_OK, ಖಾಲಿ);

    /* if no change is requested, then do nothing */
    ಆದರೆ (level == state->level && strategy == state->strategy)
        ಹಿಂತಿರುಗಿ Z_OK;

    /* check for seek request */
    ಆದರೆ (state->skip && gz_zero(state) == -1)
        ಹಿಂತಿರುಗಿ state->err;

    /* change compression parameters for subsequent input */
    ಆದರೆ (state->ಗಾತ್ರ_2) {
        /* flush previous input with previous parameters before changing */
        ಆದರೆ (strm->avail_ಒಳ && gz_comp(state, Z_ಖಂಡ) == -1)
            ಹಿಂತಿರುಗಿ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ಹಿಂತಿರುಗಿ Z_OK;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzclose_w(gzFile file) {
    ಪೂರ್ಣಾಂಕ ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    /* check for seek request */
    ಆದರೆ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ಆದರೆ (gz_comp(state, Z_ಮುಕ್ತಾಯ) == -1)
        ret = state->err;
    ಆದರೆ (state->ಗಾತ್ರ_2) {
        ಆದರೆ (!state->ನೇರ) {
            (ಶೂನ್ಯ)deflate_ಕೊನೆ(&(state->strm));
            ಬಿಡುಗಡೆ(state->ಹೊರಹೋಗು);
        }
        ಬಿಡುಗಡೆ(state->ಒಳ);
    }
    gz_error(state, Z_OK, ಖಾಲಿ);
    ಬಿಡುಗಡೆ(state->ಮಾರ್ಗ);
    ಆದರೆ (close(state->fd) == -1)
        ret = Z_ERRNO;
    ಬಿಡುಗಡೆ(state);
    ಹಿಂತಿರುಗಿ ret;
}
