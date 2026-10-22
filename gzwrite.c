/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ಪೂರ್ಣಾಙ್ಕ gz_init(gz_statep state) {
    ಪೂರ್ಣಾಙ್ಕ ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned ವರ್ಣ *)malloc(state->want << 1);
    ಯದಿ (state->in == ರಿಕ್ತ) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ಯದಿ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned ವರ್ಣ *)malloc(state->want);
        ಯದಿ (state->out == ರಿಕ್ತ) {
            ಮುಕ್ತ(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ಯದಿ (ret != Z_OK) {
            ಮುಕ್ತ(state->out);
            ಮುಕ್ತ(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        strm->next_in = ರಿಕ್ತ;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ಯದಿ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ಪೂರ್ಣಾಙ್ಕ gz_comp(gz_statep state, ಪೂರ್ಣಾಙ್ಕ flush) {
    ಪೂರ್ಣಾಙ್ಕ ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ಯದಿ (state->size == 0 && gz_init(state) == -1)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* write directly if requested */
    ಯದಿ (state->direct) {
        ಯಾವತ್ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (ಪೂರ್ಣಾಙ್ಕ)ಲಿಖ(state->fd, strm->next_in, put);
            ಯದಿ (writ < 0) {
                ಯದಿ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ಪ್ರತ್ಯಾಗಮ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    /* check for a pending reset */
    ಯದಿ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ಯದಿ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ಪ್ರತ್ಯಾಗಮ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    ಕುರು {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ಯದಿ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ಯಾವತ್ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (ಪೂರ್ಣಾಙ್ಕ)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (ಪೂರ್ಣಾಙ್ಕ)ಲಿಖ(state->fd, state->x.next, put);
                ಯದಿ (writ < 0) {
                    ಯದಿ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ಪ್ರತ್ಯಾಗಮ -1;
                }
                state->x.next += writ;
            }
            ಯದಿ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ಯದಿ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        have -= strm->avail_out;
    } ಯಾವತ್ (have);

    /* if that completed a deflate stream, allow another to start */
    ಯದಿ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ಪ್ರತ್ಯಾಗಮ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ಪೂರ್ಣಾಙ್ಕ gz_zero(gz_statep state) {
    ಪೂರ್ಣಾಙ್ಕ first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ಯದಿ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* compress state->skip zeros */
    first = 1;
    ಕುರು {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ಯದಿ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ಯದಿ (ret == -1)
            ಪ್ರತ್ಯಾಗಮ -1;
    } ಯಾವತ್ (state->skip);
    ಪ್ರತ್ಯಾಗಮ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    ಪೂರ್ಣಾಙ್ಕ ret;

    /* if len is zero, avoid unnecessary operations */
    ಯದಿ (len == 0)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* allocate memory if this is the first time through */
    ಯದಿ (state->size == 0 && gz_init(state) == -1)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* check for seek request */
    ಯದಿ (state->skip && gz_zero(state) == -1)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ಯದಿ (len < state->size) {
        /* copy to input buffer, compress when full */
        ಪ್ರತಿ (;;) {
            unsigned have, copy;

            ಯದಿ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ಯದಿ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ಧ್ರುವ ವರ್ಣ *)buf + copy;
            len -= copy;
            ಯದಿ (len == 0)
                ಭಙ್ಗ;
            ಯದಿ (gz_comp(state, Z_NO_FLUSH) == -1)
                ಪ್ರತ್ಯಾಗಮ state->again ? put - len : 0;
        }
    }
    ಅನ್ಯಥಾ {
        /* consume whatever's left in the input buffer */
        ಯದಿ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ಪ್ರತ್ಯಾಗಮ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        ಕುರು {
            unsigned n = (unsigned)-1;

            ಯದಿ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ಯದಿ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ಪ್ರತ್ಯಾಗಮ state->again ? put - len : 0;
            }
        } ಯಾವತ್ (len);
    }

    /* input was all buffered or compressed */
    ಪ್ರತ್ಯಾಗಮ put;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಪ್ರತ್ಯಾಗಮ 0;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಪ್ರತ್ಯಾಗಮ 0;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ಯದಿ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ಪ್ರತ್ಯಾಗಮ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzputc(gzFile file, ಪೂರ್ಣಾಙ್ಕ c) {
    unsigned have;
    unsigned ವರ್ಣ buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಪ್ರತ್ಯಾಗಮ -1;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* check for seek request */
    ಯದಿ (state->skip && gz_zero(state) == -1)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ಯದಿ (state->size) {
        ಯದಿ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ಯದಿ (have < state->size) {
            state->in[have] = (unsigned ವರ್ಣ)c;
            strm->avail_in++;
            state->x.pos++;
            ಪ್ರತ್ಯಾಗಮ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned ವರ್ಣ)c;
    ಯದಿ (gz_write(state, buf, 1) != 1)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ c & 0xff;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzputs(gzFile file, ಧ್ರುವ ವರ್ಣ *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಪ್ರತ್ಯಾಗಮ -1;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* write string */
    len = strlen(s);
    ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    put = gz_write(state, s, len);
    ಪ್ರತ್ಯಾಗಮ len && put == 0 ? -1 : (ಪೂರ್ಣಾಙ್ಕ)put;
}

#ಯದಿ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ಪೂರ್ಣಾಙ್ಕ gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ಯದಿ (strm->next_in == ರಿಕ್ತ ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ಪ್ರತ್ಯಾಗಮ 0;
    (ಶೂನ್ಯ)gz_comp(state, Z_NO_FLUSH);
    ಯದಿ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ಪ್ರತ್ಯಾಗಮ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ಪ್ರತ್ಯಾಗಮ strm->avail_in > state->size;
}
#endif

#ಯದಿ defined(STDC) || defined(Z_HAVE_STDARG_H)
#ಅನ್ತರ್ಭಾವ <stdarg.h>

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORTVA gzvprintf(gzFile file, ಧ್ರುವ ವರ್ಣ *format, va_list va) {
#ಯದಿ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (ಶೂನ್ಯ)file, (ಶೂನ್ಯ)format, (ಶೂನ್ಯ)va;
    ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
#ಅನ್ಯಥಾ
    ಪೂರ್ಣಾಙ್ಕ len, ret;
    ವರ್ಣ *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* make sure we have some buffer space */
    ಯದಿ (state->size == 0 && gz_init(state) == -1)
        ಪ್ರತ್ಯಾಗಮ state->err;

    /* check for seek request */
    ಯದಿ (state->skip && gz_zero(state) == -1)
        ಪ್ರತ್ಯಾಗಮ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ಯದಿ (state->err) {
        ಯದಿ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ಪ್ರತ್ಯಾಗಮ state->err;
        }
        ಯದಿ (!state->again)
            ಪ್ರತ್ಯಾಗಮ state->err;
    }
    ಯದಿ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (ವರ್ಣ *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (ಶೂನ್ಯ)vsprintf(next, format, va);
    ಪ್ರತಿ (len = 0; len < state->size; len++)
        ಯದಿ (next[len] == 0) ಭಙ್ಗ;
#  ಅನ್ಯಥಾ
    len = vsprintf(next, format, va);
#  endif
#ಅನ್ಯಥಾ
#  ifdef HAS_vsnprintf_void
    (ಶೂನ್ಯ)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  ಅನ್ಯಥಾ
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ಯದಿ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ಯದಿ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ಯದಿ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ಪ್ರತ್ಯಾಗಮ state->err;
    }
    ಯದಿ (state->err && !state->again)
        ಪ್ರತ್ಯಾಗಮ state->err;
    ಪ್ರತ್ಯಾಗಮ len;
#endif
}

ಪೂರ್ಣಾಙ್ಕ ZEXPORTVA gzprintf(gzFile file, ಧ್ರುವ ವರ್ಣ *format, ...) {
    va_list va;
    ಪೂರ್ಣಾಙ್ಕ ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ಪ್ರತ್ಯಾಗಮ ret;
}

#ಅನ್ಯಥಾ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORTVA gzprintf(gzFile file, ಧ್ರುವ ವರ್ಣ *format, ಪೂರ್ಣಾಙ್ಕ a1, ಪೂರ್ಣಾಙ್ಕ a2, ಪೂರ್ಣಾಙ್ಕ a3,
                       ಪೂರ್ಣಾಙ್ಕ a4, ಪೂರ್ಣಾಙ್ಕ a5, ಪೂರ್ಣಾಙ್ಕ a6, ಪೂರ್ಣಾಙ್ಕ a7, ಪೂರ್ಣಾಙ್ಕ a8, ಪೂರ್ಣಾಙ್ಕ a9, ಪೂರ್ಣಾಙ್ಕ a10,
                       ಪೂರ್ಣಾಙ್ಕ a11, ಪೂರ್ಣಾಙ್ಕ a12, ಪೂರ್ಣಾಙ್ಕ a13, ಪೂರ್ಣಾಙ್ಕ a14, ಪೂರ್ಣಾಙ್ಕ a15, ಪೂರ್ಣಾಙ್ಕ a16,
                       ಪೂರ್ಣಾಙ್ಕ a17, ಪೂರ್ಣಾಙ್ಕ a18, ಪೂರ್ಣಾಙ್ಕ a19, ಪೂರ್ಣಾಙ್ಕ a20) {
#ಯದಿ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (ಶೂನ್ಯ)file, (ಶೂನ್ಯ)format, (ಶೂನ್ಯ)a1, (ಶೂನ್ಯ)a2, (ಶೂನ್ಯ)a3, (ಶೂನ್ಯ)a4, (ಶೂನ್ಯ)a5,
    (ಶೂನ್ಯ)a6, (ಶೂನ್ಯ)a7, (ಶೂನ್ಯ)a8, (ಶೂನ್ಯ)a9, (ಶೂನ್ಯ)a10, (ಶೂನ್ಯ)a11, (ಶೂನ್ಯ)a12,
    (ಶೂನ್ಯ)a13, (ಶೂನ್ಯ)a14, (ಶೂನ್ಯ)a15, (ಶೂನ್ಯ)a16, (ಶೂನ್ಯ)a17, (ಶೂನ್ಯ)a18,
    (ಶೂನ್ಯ)a19, (ಶೂನ್ಯ)a20;
    ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
#ಅನ್ಯಥಾ
    ಪೂರ್ಣಾಙ್ಕ ret;
    unsigned len, ಎಡ;
    ವರ್ಣ *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ಯದಿ (ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) != ಪರಿಮಾಣ(ಶೂನ್ಯ *))
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* make sure we have some buffer space */
    ಯದಿ (state->size == 0 && gz_init(state) == -1)
        ಪ್ರತ್ಯಾಗಮ state->err;

    /* check for seek request */
    ಯದಿ (state->skip && gz_zero(state) == -1)
        ಪ್ರತ್ಯಾಗಮ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ಯದಿ (state->err) {
        ಯದಿ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ಪ್ರತ್ಯಾಗಮ state->err;
        }
        ಯದಿ (!state->again)
            ಪ್ರತ್ಯಾಗಮ state->err;
    }
    ಯದಿ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (ವರ್ಣ *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    ಪ್ರತಿ (len = 0; len < size; len++)
        ಯದಿ (next[len] == 0)
            ಭಙ್ಗ;
#  ಅನ್ಯಥಾ
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ಅನ್ಯಥಾ
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  ಅನ್ಯಥಾ
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ಯದಿ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ಯದಿ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ಯದಿ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ಪ್ರತ್ಯಾಗಮ state->err;
    }
    ಯದಿ (state->err && !state->again)
        ಪ್ರತ್ಯಾಗಮ state->err;
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)len;
#endif
}

#endif

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzflush(gzFile file, ಪೂರ್ಣಾಙ್ಕ flush) {
    gz_statep state;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* check flush parameter */
    ಯದಿ (flush < 0 || flush > Z_FINISH)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;

    /* check for seek request */
    ಯದಿ (state->skip && gz_zero(state) == -1)
        ಪ್ರತ್ಯಾಗಮ state->err;

    /* compress remaining data with requested flush */
    (ಶೂನ್ಯ)gz_comp(state, flush);
    ಪ್ರತ್ಯಾಗಮ state->err;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzsetparams(gzFile file, ಪೂರ್ಣಾಙ್ಕ level, ಪೂರ್ಣಾಙ್ಕ strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ಯದಿ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ರಿಕ್ತ);

    /* if no change is requested, then do nothing */
    ಯದಿ (level == state->level && strategy == state->strategy)
        ಪ್ರತ್ಯಾಗಮ Z_OK;

    /* check for seek request */
    ಯದಿ (state->skip && gz_zero(state) == -1)
        ಪ್ರತ್ಯಾಗಮ state->err;

    /* change compression parameters for subsequent input */
    ಯದಿ (state->size) {
        /* flush previous input with previous parameters before changing */
        ಯದಿ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ಪ್ರತ್ಯಾಗಮ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ಪ್ರತ್ಯಾಗಮ Z_OK;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzclose_w(gzFile file) {
    ಪೂರ್ಣಾಙ್ಕ ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ಯದಿ (state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ Z_STREAM_ERROR;

    /* check for seek request */
    ಯದಿ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ಯದಿ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ಯದಿ (state->size) {
        ಯದಿ (!state->direct) {
            (ಶೂನ್ಯ)deflateEnd(&(state->strm));
            ಮುಕ್ತ(state->out);
        }
        ಮುಕ್ತ(state->in);
    }
    gz_error(state, Z_OK, ರಿಕ್ತ);
    ಮುಕ್ತ(state->path);
    ಯದಿ (close(state->fd) == -1)
        ret = Z_ERRNO;
    ಮುಕ್ತ(state);
    ಪ್ರತ್ಯಾಗಮ ret;
}
