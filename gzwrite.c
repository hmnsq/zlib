/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ਪੂਰਨ gz_init(gz_statep state) {
    ਪੂਰਨ ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->ਵਿੱਚ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(state->want << 1);
    ਜੇ (state->ਵਿੱਚ == ਨਿਰਾ) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ਵਾਪਸ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ਜੇ (!state->ਸਿੱਧਾ) {
        /* allocate output buffer */
        state->ਬਾਹਰਲਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(state->want);
        ਜੇ (state->ਬਾਹਰਲਾ == ਨਿਰਾ) {
            ਮੁਕਤ(state->ਵਿੱਚ);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ਵਾਪਸ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ਜੇ (ret != Z_OK) {
            ਮੁਕਤ(state->ਬਾਹਰਲਾ);
            ਮੁਕਤ(state->ਵਿੱਚ);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ਵਾਪਸ -1;
        }
        strm->ਅਗਲਾ_ਵਿੱਚ = ਨਿਰਾ;
    }

    /* mark state as initialized */
    state->ਮਾਪ = state->want;

    /* initialize write buffer if compressing */
    ਜੇ (!state->ਸਿੱਧਾ) {
        strm->avail_ਬਾਹਰਲਾ = state->ਮਾਪ;
        strm->ਅਗਲਾ_ਬਾਹਰਲਾ = state->ਬਾਹਰਲਾ;
        state->x.ਅਗਲਾ = strm->ਅਗਲਾ_ਬਾਹਰਲਾ;
    }
    ਵਾਪਸ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ਪੂਰਨ gz_comp(gz_statep state, ਪੂਰਨ flush) {
    ਪੂਰਨ ret, writ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ have, put, max = ((ਬਿਨਾਨਿਸ਼ਾਨੀ)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ਜੇ (state->ਮਾਪ == 0 && gz_init(state) == -1)
        ਵਾਪਸ -1;

    /* write directly if requested */
    ਜੇ (state->ਸਿੱਧਾ) {
        ਜਦੋਂ (strm->avail_ਵਿੱਚ) {
            errno = 0;
            state->again = 0;
            put = strm->avail_ਵਿੱਚ > max ? max : strm->avail_ਵਿੱਚ;
            writ = (ਪੂਰਨ)ਲਿਖ(state->fd, strm->ਅਗਲਾ_ਵਿੱਚ, put);
            ਜੇ (writ < 0) {
                ਜੇ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ਵਾਪਸ -1;
            }
            strm->avail_ਵਿੱਚ -= (ਬਿਨਾਨਿਸ਼ਾਨੀ)writ;
            strm->ਅਗਲਾ_ਵਿੱਚ += writ;
        }
        ਵਾਪਸ 0;
    }

    /* check for a pending reset */
    ਜੇ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ਜੇ (strm->avail_ਵਿੱਚ == 0 && flush == Z_NO_FLUSH)
            ਵਾਪਸ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    ਕਰੋ {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ਜੇ (strm->avail_ਬਾਹਰਲਾ == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_ਅੰਤ || ret == Z_ਧਾਰਾ_ਸਿਰਾ))) {
            ਜਦੋਂ (strm->ਅਗਲਾ_ਬਾਹਰਲਾ > state->x.ਅਗਲਾ) {
                errno = 0;
                state->again = 0;
                put = strm->ਅਗਲਾ_ਬਾਹਰਲਾ - state->x.ਅਗਲਾ > (ਪੂਰਨ)max ? max :
                      (ਬਿਨਾਨਿਸ਼ਾਨੀ)(strm->ਅਗਲਾ_ਬਾਹਰਲਾ - state->x.ਅਗਲਾ);
                writ = (ਪੂਰਨ)ਲਿਖ(state->fd, state->x.ਅਗਲਾ, put);
                ਜੇ (writ < 0) {
                    ਜੇ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ਵਾਪਸ -1;
                }
                state->x.ਅਗਲਾ += writ;
            }
            ਜੇ (strm->avail_ਬਾਹਰਲਾ == 0) {
                strm->avail_ਬਾਹਰਲਾ = state->ਮਾਪ;
                strm->ਅਗਲਾ_ਬਾਹਰਲਾ = state->ਬਾਹਰਲਾ;
                state->x.ਅਗਲਾ = state->ਬਾਹਰਲਾ;
            }
        }

        /* compress */
        have = strm->avail_ਬਾਹਰਲਾ;
        ret = deflate(strm, flush);
        ਜੇ (ret == Z_ਧਾਰਾ_ERROR) {
            gz_error(state, Z_ਧਾਰਾ_ERROR,
                      "internal error: deflate stream corrupt");
            ਵਾਪਸ -1;
        }
        have -= strm->avail_ਬਾਹਰਲਾ;
    } ਜਦੋਂ (have);

    /* if that completed a deflate stream, allow another to start */
    ਜੇ (flush == Z_ਅੰਤ)
        state->reset = 1;

    /* all done, no errors */
    ਵਾਪਸ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ਪੂਰਨ gz_zero(gz_statep state) {
    ਪੂਰਨ ਪਹਿਲਾ, ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ਜੇ (strm->avail_ਵਿੱਚ && gz_comp(state, Z_NO_FLUSH) == -1)
        ਵਾਪਸ -1;

    /* compress state->skip zeros */
    ਪਹਿਲਾ = 1;
    ਕਰੋ {
        n = GT_OFF(state->ਮਾਪ) || (z_off64_t)state->ਮਾਪ > state->skip ?
            (ਬਿਨਾਨਿਸ਼ਾਨੀ)state->skip : state->ਮਾਪ;
        ਜੇ (ਪਹਿਲਾ) {
            memset(state->ਵਿੱਚ, 0, n);
            ਪਹਿਲਾ = 0;
        }
        strm->avail_ਵਿੱਚ = n;
        strm->ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_ਵਿੱਚ;
        state->x.pos += n;
        state->skip -= n;
        ਜੇ (ret == -1)
            ਵਾਪਸ -1;
    } ਜਦੋਂ (state->skip);
    ਵਾਪਸ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_ਮਾਪ_t gz_write(gz_statep state, voidpc buf, z_ਮਾਪ_t len) {
    z_ਮਾਪ_t put = len;
    ਪੂਰਨ ret;

    /* if len is zero, avoid unnecessary operations */
    ਜੇ (len == 0)
        ਵਾਪਸ 0;

    /* allocate memory if this is the first time through */
    ਜੇ (state->ਮਾਪ == 0 && gz_init(state) == -1)
        ਵਾਪਸ 0;

    /* check for seek request */
    ਜੇ (state->skip && gz_zero(state) == -1)
        ਵਾਪਸ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ਜੇ (len < state->ਮਾਪ) {
        /* copy to input buffer, compress when full */
        ਲਈ (;;) {
            ਬਿਨਾਨਿਸ਼ਾਨੀ have, copy;

            ਜੇ (state->strm.avail_ਵਿੱਚ == 0)
                state->strm.ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
            have = (ਬਿਨਾਨਿਸ਼ਾਨੀ)((state->strm.ਅਗਲਾ_ਵਿੱਚ + state->strm.avail_ਵਿੱਚ) -
                              state->ਵਿੱਚ);
            copy = state->ਮਾਪ - have;
            ਜੇ (copy > len)
                copy = (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(state->ਵਿੱਚ + have, buf, copy);
            state->strm.avail_ਵਿੱਚ += copy;
            state->x.pos += copy;
            buf = (ਅਚਲ ਅੱਖਰ *)buf + copy;
            len -= copy;
            ਜੇ (len == 0)
                ਰੋਕੋ;
            ਜੇ (gz_comp(state, Z_NO_FLUSH) == -1)
                ਵਾਪਸ state->again ? put - len : 0;
        }
    }
    ਨਹੀਂਤਾਂ {
        /* consume whatever's left in the input buffer */
        ਜੇ (state->strm.avail_ਵਿੱਚ && gz_comp(state, Z_NO_FLUSH) == -1)
            ਵਾਪਸ 0;

        /* directly compress user buffer to file */
        state->strm.ਅਗਲਾ_ਵਿੱਚ = (z_const Bytef *)buf;
        ਕਰੋ {
            ਬਿਨਾਨਿਸ਼ਾਨੀ n = (ਬਿਨਾਨਿਸ਼ਾਨੀ)-1;

            ਜੇ (n > len)
                n = (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
            state->strm.avail_ਵਿੱਚ = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_ਵਿੱਚ;
            state->x.pos += n;
            len -= n;
            ਜੇ (ret == -1) {
                state->strm.avail_ਵਿੱਚ = 0;
                state->strm.ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
                ਵਾਪਸ state->again ? put - len : 0;
            }
        } ਜਦੋਂ (len);
    }

    /* input was all buffered or compressed */
    ਵਾਪਸ put;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzwrite(gzFile file, voidpc buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len) {
    gz_statep state;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ਵਾਪਸ 0;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ਜੇ ((ਪੂਰਨ)len < 0) {
        gz_error(state, Z_ਅੰਕੜੇ_ERROR, "requested length does not fit in int");
        ਵਾਪਸ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ਵਾਪਸ (ਪੂਰਨ)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_ਮਾਪ_t ZEXPORT gzfwrite(voidpc buf, z_ਮਾਪ_t ਮਾਪ, z_ਮਾਪ_t nitems,
                          gzFile file) {
    z_ਮਾਪ_t len;
    gz_statep state;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ਵਾਪਸ 0;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* compute bytes to read -- error on overflow */
    len = nitems * ਮਾਪ;
    ਜੇ (ਮਾਪ && len / ਮਾਪ != nitems) {
        gz_error(state, Z_ਧਾਰਾ_ERROR, "request does not fit in a size_t");
        ਵਾਪਸ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ਵਾਪਸ len ? gz_write(state, buf, len) / ਮਾਪ : 0;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzputc(gzFile file, ਪੂਰਨ c) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ਵਾਪਸ -1;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* check for seek request */
    ਜੇ (state->skip && gz_zero(state) == -1)
        ਵਾਪਸ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ਜੇ (state->ਮਾਪ) {
        ਜੇ (strm->avail_ਵਿੱਚ == 0)
            strm->ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
        have = (ਬਿਨਾਨਿਸ਼ਾਨੀ)((strm->ਅਗਲਾ_ਵਿੱਚ + strm->avail_ਵਿੱਚ) - state->ਵਿੱਚ);
        ਜੇ (have < state->ਮਾਪ) {
            state->ਵਿੱਚ[have] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)c;
            strm->avail_ਵਿੱਚ++;
            state->x.pos++;
            ਵਾਪਸ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)c;
    ਜੇ (gz_write(state, buf, 1) != 1)
        ਵਾਪਸ -1;
    ਵਾਪਸ c & 0xff;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzputs(gzFile file, ਅਚਲ ਅੱਖਰ *s) {
    z_ਮਾਪ_t len, put;
    gz_statep state;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ਵਾਪਸ -1;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* write string */
    len = ਲੜੀ_ਲੰਬਾਈ(s);
    ਜੇ ((ਪੂਰਨ)len < 0 || (ਬਿਨਾਨਿਸ਼ਾਨੀ)len != len) {
        gz_error(state, Z_ਧਾਰਾ_ERROR, "string length does not fit in int");
        ਵਾਪਸ -1;
    }
    put = gz_write(state, s, len);
    ਵਾਪਸ len && put == 0 ? -1 : (ਪੂਰਨ)put;
}

#ਜੇ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ਪੂਰਨ gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ਜੇ (strm->ਅਗਲਾ_ਵਿੱਚ == ਨਿਰਾ ||
        strm->ਅਗਲਾ_ਵਿੱਚ + strm->avail_ਵਿੱਚ <= state->ਵਿੱਚ + state->ਮਾਪ)
        ਵਾਪਸ 0;
    (ਖਾਲੀ)gz_comp(state, Z_NO_FLUSH);
    ਜੇ (strm->avail_ਵਿੱਚ == 0) {
        strm->ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
        ਵਾਪਸ 0;
    }
    memmove(state->ਵਿੱਚ, strm->ਅਗਲਾ_ਵਿੱਚ, strm->avail_ਵਿੱਚ);
    strm->ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
    ਵਾਪਸ strm->avail_ਵਿੱਚ > state->ਮਾਪ;
}
#ਜੇ_ਅੰਤ

#ਜੇ defined(STDC) || defined(Z_HAVE_STDARG_H)
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORTVA gzvprintf(gzFile file, ਅਚਲ ਅੱਖਰ *format, va_list va) {
#ਜੇ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (ਖਾਲੀ)file, (ਖਾਲੀ)format, (ਖਾਲੀ)va;
    ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
#ਨਹੀਂਤਾਂ
    ਪੂਰਨ len, ret;
    ਅੱਖਰ *ਅਗਲਾ;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* make sure we have some buffer space */
    ਜੇ (state->ਮਾਪ == 0 && gz_init(state) == -1)
        ਵਾਪਸ state->err;

    /* check for seek request */
    ਜੇ (state->skip && gz_zero(state) == -1)
        ਵਾਪਸ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ਜੇ (state->err) {
        ਜੇ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ਵਾਪਸ state->err;
        }
        ਜੇ (!state->again)
            ਵਾਪਸ state->err;
    }
    ਜੇ (strm->avail_ਵਿੱਚ == 0)
        strm->ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
    ਅਗਲਾ = (ਅੱਖਰ *)(state->ਵਿੱਚ + (strm->ਅਗਲਾ_ਵਿੱਚ - state->ਵਿੱਚ) + strm->avail_ਵਿੱਚ);
    ਅਗਲਾ[state->ਮਾਪ - 1] = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (ਖਾਲੀ)vsprintf(ਅਗਲਾ, format, va);
    ਲਈ (len = 0; len < state->ਮਾਪ; len++)
        ਜੇ (ਅਗਲਾ[len] == 0) ਰੋਕੋ;
#  ਨਹੀਂਤਾਂ
    len = vsprintf(ਅਗਲਾ, format, va);
#  endif
#ਨਹੀਂਤਾਂ
#  ifdef HAS_vsnprintf_void
    (ਖਾਲੀ)vsnprintf(ਅਗਲਾ, state->ਮਾਪ, format, va);
    len = ਲੜੀ_ਲੰਬਾਈ(ਅਗਲਾ);
#  ਨਹੀਂਤਾਂ
    len = vsnprintf(ਅਗਲਾ, state->ਮਾਪ, format, va);
#  endif
#ਜੇ_ਅੰਤ

    /* check that printf() results fit in buffer */
    ਜੇ (len == 0 || (ਬਿਨਾਨਿਸ਼ਾਨੀ)len >= state->ਮਾਪ || ਅਗਲਾ[state->ਮਾਪ - 1] != 0)
        ਵਾਪਸ 0;

    /* update buffer and position */
    strm->avail_ਵਿੱਚ += (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ਜੇ (ret) {
        /* discard unwritten formatted output */
        strm->avail_ਵਿੱਚ -= (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
        state->x.pos -= len;
        ਜੇ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ਵਾਪਸ state->err;
    }
    ਜੇ (state->err && !state->again)
        ਵਾਪਸ state->err;
    ਵਾਪਸ len;
#ਜੇ_ਅੰਤ
}

ਪੂਰਨ ZEXPORTVA gzprintf(gzFile file, ਅਚਲ ਅੱਖਰ *format, ...) {
    va_list va;
    ਪੂਰਨ ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ਵਾਪਸ ret;
}

#ਨਹੀਂਤਾਂ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORTVA gzprintf(gzFile file, ਅਚਲ ਅੱਖਰ *format, ਪੂਰਨ a1, ਪੂਰਨ a2, ਪੂਰਨ a3,
                       ਪੂਰਨ a4, ਪੂਰਨ a5, ਪੂਰਨ a6, ਪੂਰਨ a7, ਪੂਰਨ a8, ਪੂਰਨ a9, ਪੂਰਨ a10,
                       ਪੂਰਨ a11, ਪੂਰਨ a12, ਪੂਰਨ a13, ਪੂਰਨ a14, ਪੂਰਨ a15, ਪੂਰਨ a16,
                       ਪੂਰਨ a17, ਪੂਰਨ a18, ਪੂਰਨ a19, ਪੂਰਨ a20) {
#ਜੇ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (ਖਾਲੀ)file, (ਖਾਲੀ)format, (ਖਾਲੀ)a1, (ਖਾਲੀ)a2, (ਖਾਲੀ)a3, (ਖਾਲੀ)a4, (ਖਾਲੀ)a5,
    (ਖਾਲੀ)a6, (ਖਾਲੀ)a7, (ਖਾਲੀ)a8, (ਖਾਲੀ)a9, (ਖਾਲੀ)a10, (ਖਾਲੀ)a11, (ਖਾਲੀ)a12,
    (ਖਾਲੀ)a13, (ਖਾਲੀ)a14, (ਖਾਲੀ)a15, (ਖਾਲੀ)a16, (ਖਾਲੀ)a17, (ਖਾਲੀ)a18,
    (ਖਾਲੀ)a19, (ਖਾਲੀ)a20;
    ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
#ਨਹੀਂਤਾਂ
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ len, ਖੱਬਾ;
    ਅੱਖਰ *ਅਗਲਾ;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ਜੇ (ਆਕਾਰ(ਪੂਰਨ) != ਆਕਾਰ(ਖਾਲੀ *))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* make sure we have some buffer space */
    ਜੇ (state->ਮਾਪ == 0 && gz_init(state) == -1)
        ਵਾਪਸ state->err;

    /* check for seek request */
    ਜੇ (state->skip && gz_zero(state) == -1)
        ਵਾਪਸ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ਜੇ (state->err) {
        ਜੇ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ਵਾਪਸ state->err;
        }
        ਜੇ (!state->again)
            ਵਾਪਸ state->err;
    }
    ਜੇ (strm->avail_ਵਿੱਚ == 0)
        strm->ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
    ਅਗਲਾ = (ਅੱਖਰ *)(strm->ਅਗਲਾ_ਵਿੱਚ + strm->avail_ਵਿੱਚ);
    ਅਗਲਾ[state->ਮਾਪ - 1] = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(ਅਗਲਾ, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    ਲਈ (len = 0; len < ਮਾਪ; len++)
        ਜੇ (ਅਗਲਾ[len] == 0)
            ਰੋਕੋ;
#  ਨਹੀਂਤਾਂ
    len = sprintf(ਅਗਲਾ, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ਨਹੀਂਤਾਂ
#  ifdef HAS_snprintf_void
    snprintf(ਅਗਲਾ, state->ਮਾਪ, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = ਲੜੀ_ਲੰਬਾਈ(ਅਗਲਾ);
#  ਨਹੀਂਤਾਂ
    len = snprintf(ਅਗਲਾ, state->ਮਾਪ, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ਜੇ_ਅੰਤ

    /* check that printf() results fit in buffer */
    ਜੇ (len == 0 || len >= state->ਮਾਪ || ਅਗਲਾ[state->ਮਾਪ - 1] != 0)
        ਵਾਪਸ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_ਵਿੱਚ += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ਜੇ (ret) {
        /* discard unwritten formatted output */
        strm->avail_ਵਿੱਚ -= (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;
        state->x.pos -= len;
        ਜੇ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ਵਾਪਸ state->err;
    }
    ਜੇ (state->err && !state->again)
        ਵਾਪਸ state->err;
    ਵਾਪਸ (ਪੂਰਨ)len;
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਅੰਤ

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzflush(gzFile file, ਪੂਰਨ flush) {
    gz_statep state;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again))
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* check flush parameter */
    ਜੇ (flush < 0 || flush > Z_ਅੰਤ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    /* check for seek request */
    ਜੇ (state->skip && gz_zero(state) == -1)
        ਵਾਪਸ state->err;

    /* compress remaining data with requested flush */
    (ਖਾਲੀ)gz_comp(state, flush);
    ਵਾਪਸ state->err;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzsetparams(gzFile file, ਪੂਰਨ level, ਪੂਰਨ strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->ਸਿੱਧਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    gz_error(state, Z_OK, ਨਿਰਾ);

    /* if no change is requested, then do nothing */
    ਜੇ (level == state->level && strategy == state->strategy)
        ਵਾਪਸ Z_OK;

    /* check for seek request */
    ਜੇ (state->skip && gz_zero(state) == -1)
        ਵਾਪਸ state->err;

    /* change compression parameters for subsequent input */
    ਜੇ (state->ਮਾਪ) {
        /* flush previous input with previous parameters before changing */
        ਜੇ (strm->avail_ਵਿੱਚ && gz_comp(state, Z_ਖੰਡ) == -1)
            ਵਾਪਸ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ਵਾਪਸ Z_OK;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzclose_w(gzFile file) {
    ਪੂਰਨ ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ਜੇ (state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    /* check for seek request */
    ਜੇ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ਜੇ (gz_comp(state, Z_ਅੰਤ) == -1)
        ret = state->err;
    ਜੇ (state->ਮਾਪ) {
        ਜੇ (!state->ਸਿੱਧਾ) {
            (ਖਾਲੀ)deflate_ਸਿਰਾ(&(state->strm));
            ਮੁਕਤ(state->ਬਾਹਰਲਾ);
        }
        ਮੁਕਤ(state->ਵਿੱਚ);
    }
    gz_error(state, Z_OK, ਨਿਰਾ);
    ਮੁਕਤ(state->ਰਸਤਾ);
    ਜੇ (close(state->fd) == -1)
        ret = Z_ERRNO;
    ਮੁਕਤ(state);
    ਵਾਪਸ ret;
}
