/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local puur_naa_nka gz_init(gz_statep state) {
    puur_naa_nka ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned var_na *)malloc(state->want << 1);
    ᯀᯮᯝ (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ᯀᯮᯞ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ᯀᯮᯝ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned var_na *)malloc(state->want);
        ᯀᯮᯝ (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ᯀᯮᯞ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ᯀᯮᯝ (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ᯀᯮᯞ -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ᯀᯮᯝ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ᯀᯮᯞ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local puur_naa_nka gz_comp(gz_statep state, puur_naa_nka flush) {
    puur_naa_nka ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ᯀᯮᯝ (state->size == 0 && gz_init(state) == -1)
        ᯀᯮᯞ -1;

    /* write directly if requested */
    ᯀᯮᯝ (state->direct) {
        ᯀᯬᯝ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (puur_naa_nka)write(state->fd, strm->next_in, put);
            ᯀᯮᯝ (writ < 0) {
                ᯀᯮᯝ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ᯀᯮᯞ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ᯀᯮᯞ 0;
    }

    /* check for a pending reset */
    ᯀᯮᯝ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ᯀᯮᯝ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ᯀᯮᯞ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ᯀᯮᯝ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ᯀᯬᯝ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (puur_naa_nka)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (puur_naa_nka)write(state->fd, state->x.next, put);
                ᯀᯮᯝ (writ < 0) {
                    ᯀᯮᯝ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ᯀᯮᯞ -1;
                }
                state->x.next += writ;
            }
            ᯀᯮᯝ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ᯀᯮᯝ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ᯀᯮᯞ -1;
        }
        have -= strm->avail_out;
    } ᯀᯬᯝ (have);

    /* if that completed a deflate stream, allow another to start */
    ᯀᯮᯝ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ᯀᯮᯞ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local puur_naa_nka gz_zero(gz_statep state) {
    puur_naa_nka first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ᯀᯮᯝ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ᯀᯮᯞ -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ᯀᯮᯝ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ᯀᯮᯝ (ret == -1)
            ᯀᯮᯞ -1;
    } ᯀᯬᯝ (state->skip);
    ᯀᯮᯞ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    puur_naa_nka ret;

    /* if len is zero, avoid unnecessary operations */
    ᯀᯮᯝ (len == 0)
        ᯀᯮᯞ 0;

    /* allocate memory if this is the first time through */
    ᯀᯮᯝ (state->size == 0 && gz_init(state) == -1)
        ᯀᯮᯞ 0;

    /* check for seek request */
    ᯀᯮᯝ (state->skip && gz_zero(state) == -1)
        ᯀᯮᯞ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ᯀᯮᯝ (len < state->size) {
        /* copy to input buffer, compress when full */
        for (;;) {
            unsigned have, copy;

            ᯀᯮᯝ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ᯀᯮᯝ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const var_na *)buf + copy;
            len -= copy;
            ᯀᯮᯝ (len == 0)
                bha_nga;
            ᯀᯮᯝ (gz_comp(state, Z_NO_FLUSH) == -1)
                ᯀᯮᯞ state->again ? put - len : 0;
        }
    }
    ᯀᯮᯝᯂᯪ {
        /* consume whatever's left in the input buffer */
        ᯀᯮᯝ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ᯀᯮᯞ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            ᯀᯮᯝ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ᯀᯮᯝ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ᯀᯮᯞ state->again ? put - len : 0;
            }
        } ᯀᯬᯝ (len);
    }

    /* input was all buffered or compressed */
    ᯀᯮᯞ put;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᯀᯮᯞ 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ᯀᯮᯝ ((puur_naa_nka)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ᯀᯮᯞ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ᯀᯮᯞ (puur_naa_nka)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᯀᯮᯞ 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ᯀᯮᯝ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ᯀᯮᯞ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ᯀᯮᯞ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzputc(gzFile file, puur_naa_nka c) {
    unsigned have;
    unsigned var_na buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᯀᯮᯞ -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    ᯀᯮᯝ (state->skip && gz_zero(state) == -1)
        ᯀᯮᯞ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ᯀᯮᯝ (state->size) {
        ᯀᯮᯝ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ᯀᯮᯝ (have < state->size) {
            state->in[have] = (unsigned var_na)c;
            strm->avail_in++;
            state->x.pos++;
            ᯀᯮᯞ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned var_na)c;
    ᯀᯮᯝ (gz_write(state, buf, 1) != 1)
        ᯀᯮᯞ -1;
    ᯀᯮᯞ c & 0xff;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzputs(gzFile file, const var_na *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᯀᯮᯞ -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    ᯀᯮᯝ ((puur_naa_nka)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ᯀᯮᯞ -1;
    }
    put = gz_write(state, s, len);
    ᯀᯮᯞ len && put == 0 ? -1 : (puur_naa_nka)put;
}

#ᯀᯮᯝ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local puur_naa_nka gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ᯀᯮᯝ (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ᯀᯮᯞ 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    ᯀᯮᯝ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ᯀᯮᯞ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ᯀᯮᯞ strm->avail_in > state->size;
}
#endif

#ᯀᯮᯝ defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
puur_naa_nka ZEXPORTVA gzvprintf(gzFile file, const var_na *format, va_list va) {
#ᯀᯮᯝ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    ᯀᯮᯞ Z_STREAM_ERROR;
#ᯀᯮᯝᯂᯪ
    puur_naa_nka len, ret;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᯀᯮᯞ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ᯀᯮᯝ (state->size == 0 && gz_init(state) == -1)
        ᯀᯮᯞ state->err;

    /* check for seek request */
    ᯀᯮᯝ (state->skip && gz_zero(state) == -1)
        ᯀᯮᯞ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ᯀᯮᯝ (state->err) {
        ᯀᯮᯝ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ᯀᯮᯞ state->err;
        }
        ᯀᯮᯝ (!state->again)
            ᯀᯮᯞ state->err;
    }
    ᯀᯮᯝ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    for (len = 0; len < state->size; len++)
        ᯀᯮᯝ (next[len] == 0) bha_nga;
#  ᯀᯮᯝᯂᯪ
    len = vsprintf(next, format, va);
#  endif
#ᯀᯮᯝᯂᯪ
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  ᯀᯮᯝᯂᯪ
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ᯀᯮᯝ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ᯀᯮᯞ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ᯀᯮᯝ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ᯀᯮᯝ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ᯀᯮᯞ state->err;
    }
    ᯀᯮᯝ (state->err && !state->again)
        ᯀᯮᯞ state->err;
    ᯀᯮᯞ len;
#endif
}

puur_naa_nka ZEXPORTVA gzprintf(gzFile file, const var_na *format, ...) {
    va_list va;
    puur_naa_nka ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ᯀᯮᯞ ret;
}

#ᯀᯮᯝᯂᯪ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
puur_naa_nka ZEXPORTVA gzprintf(gzFile file, const var_na *format, puur_naa_nka a1, puur_naa_nka a2, puur_naa_nka a3,
                       puur_naa_nka a4, puur_naa_nka a5, puur_naa_nka a6, puur_naa_nka a7, puur_naa_nka a8, puur_naa_nka a9, puur_naa_nka a10,
                       puur_naa_nka a11, puur_naa_nka a12, puur_naa_nka a13, puur_naa_nka a14, puur_naa_nka a15, puur_naa_nka a16,
                       puur_naa_nka a17, puur_naa_nka a18, puur_naa_nka a19, puur_naa_nka a20) {
#ᯀᯮᯝ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    ᯀᯮᯞ Z_STREAM_ERROR;
#ᯀᯮᯝᯂᯪ
    puur_naa_nka ret;
    unsigned len, left;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ᯀᯮᯝ (parimaa_na(puur_naa_nka) != parimaa_na(void *))
        ᯀᯮᯞ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᯀᯮᯞ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ᯀᯮᯝ (state->size == 0 && gz_init(state) == -1)
        ᯀᯮᯞ state->err;

    /* check for seek request */
    ᯀᯮᯝ (state->skip && gz_zero(state) == -1)
        ᯀᯮᯞ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ᯀᯮᯝ (state->err) {
        ᯀᯮᯝ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ᯀᯮᯞ state->err;
        }
        ᯀᯮᯝ (!state->again)
            ᯀᯮᯞ state->err;
    }
    ᯀᯮᯝ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    for (len = 0; len < size; len++)
        ᯀᯮᯝ (next[len] == 0)
            bha_nga;
#  ᯀᯮᯝᯂᯪ
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ᯀᯮᯝᯂᯪ
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  ᯀᯮᯝᯂᯪ
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ᯀᯮᯝ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ᯀᯮᯞ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ᯀᯮᯝ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ᯀᯮᯝ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ᯀᯮᯞ state->err;
    }
    ᯀᯮᯝ (state->err && !state->again)
        ᯀᯮᯞ state->err;
    ᯀᯮᯞ (puur_naa_nka)len;
#endif
}

#endif

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzflush(gzFile file, puur_naa_nka flush) {
    gz_statep state;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᯀᯮᯞ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    ᯀᯮᯝ (flush < 0 || flush > Z_FINISH)
        ᯀᯮᯞ Z_STREAM_ERROR;

    /* check for seek request */
    ᯀᯮᯝ (state->skip && gz_zero(state) == -1)
        ᯀᯮᯞ state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    ᯀᯮᯞ state->err;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzsetparams(gzFile file, puur_naa_nka level, puur_naa_nka strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ᯀᯮᯝ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ᯀᯮᯞ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    ᯀᯮᯝ (level == state->level && strategy == state->strategy)
        ᯀᯮᯞ Z_OK;

    /* check for seek request */
    ᯀᯮᯝ (state->skip && gz_zero(state) == -1)
        ᯀᯮᯞ state->err;

    /* change compression parameters for subsequent input */
    ᯀᯮᯝ (state->size) {
        /* flush previous input with previous parameters before changing */
        ᯀᯮᯝ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ᯀᯮᯞ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ᯀᯮᯞ Z_OK;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzclose_w(gzFile file) {
    puur_naa_nka ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ᯀᯮᯝ (file == NULL)
        ᯀᯮᯞ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ᯀᯮᯝ (state->mode != GZ_WRITE)
        ᯀᯮᯞ Z_STREAM_ERROR;

    /* check for seek request */
    ᯀᯮᯝ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ᯀᯮᯝ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ᯀᯮᯝ (state->size) {
        ᯀᯮᯝ (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ᯀᯮᯝ (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    ᯀᯮᯞ ret;
}
