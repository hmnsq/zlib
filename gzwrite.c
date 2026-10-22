/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ཧྲིལ gz_init(gz_statep state) {
    ཧྲིལ ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned var_na *)malloc(state->want << 1);
    གལ་ཏེ (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ཕྱིར -1;
    }

    /* only need output buffer and deflate state if compressing */
    གལ་ཏེ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned var_na *)malloc(state->want);
        གལ་ཏེ (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ཕྱིར -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        གལ་ཏེ (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ཕྱིར -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    གལ་ཏེ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ཕྱིར 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ཧྲིལ gz_comp(gz_statep state, ཧྲིལ flush) {
    ཧྲིལ ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    གལ་ཏེ (state->size == 0 && gz_init(state) == -1)
        ཕྱིར -1;

    /* write directly if requested */
    གལ་ཏེ (state->direct) {
        སྐབས (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (ཧྲིལ)write(state->fd, strm->next_in, put);
            གལ་ཏེ (writ < 0) {
                གལ་ཏེ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ཕྱིར -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ཕྱིར 0;
    }

    /* check for a pending reset */
    གལ་ཏེ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        གལ་ཏེ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ཕྱིར 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        གལ་ཏེ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            སྐབས (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (ཧྲིལ)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (ཧྲིལ)write(state->fd, state->x.next, put);
                གལ་ཏེ (writ < 0) {
                    གལ་ཏེ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ཕྱིར -1;
                }
                state->x.next += writ;
            }
            གལ་ཏེ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        གལ་ཏེ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ཕྱིར -1;
        }
        have -= strm->avail_out;
    } སྐབས (have);

    /* if that completed a deflate stream, allow another to start */
    གལ་ཏེ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ཕྱིར 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ཧྲིལ gz_zero(gz_statep state) {
    ཧྲིལ first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    གལ་ཏེ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ཕྱིར -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        གལ་ཏེ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        གལ་ཏེ (ret == -1)
            ཕྱིར -1;
    } སྐབས (state->skip);
    ཕྱིར 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    ཧྲིལ ret;

    /* if len is zero, avoid unnecessary operations */
    གལ་ཏེ (len == 0)
        ཕྱིར 0;

    /* allocate memory if this is the first time through */
    གལ་ཏེ (state->size == 0 && gz_init(state) == -1)
        ཕྱིར 0;

    /* check for seek request */
    གལ་ཏེ (state->skip && gz_zero(state) == -1)
        ཕྱིར 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    གལ་ཏེ (len < state->size) {
        /* copy to input buffer, compress when full */
        ཆེད་དུ (;;) {
            unsigned have, copy;

            གལ་ཏེ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            གལ་ཏེ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const var_na *)buf + copy;
            len -= copy;
            གལ་ཏེ (len == 0)
                bha_nga;
            གལ་ཏེ (gz_comp(state, Z_NO_FLUSH) == -1)
                ཕྱིར state->again ? put - len : 0;
        }
    }
    དེ་མིན {
        /* consume whatever's left in the input buffer */
        གལ་ཏེ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ཕྱིར 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            གལ་ཏེ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            གལ་ཏེ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ཕྱིར state->again ? put - len : 0;
            }
        } སྐབས (len);
    }

    /* input was all buffered or compressed */
    ཕྱིར put;
}

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ཕྱིར 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    གལ་ཏེ ((ཧྲིལ)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ཕྱིར 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ཕྱིར (ཧྲིལ)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ཕྱིར 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    གལ་ཏེ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ཕྱིར 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ཕྱིར len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzputc(gzFile file, ཧྲིལ c) {
    unsigned have;
    unsigned var_na buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ཕྱིར -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    གལ་ཏེ (state->skip && gz_zero(state) == -1)
        ཕྱིར -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    གལ་ཏེ (state->size) {
        གལ་ཏེ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        གལ་ཏེ (have < state->size) {
            state->in[have] = (unsigned var_na)c;
            strm->avail_in++;
            state->x.pos++;
            ཕྱིར c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned var_na)c;
    གལ་ཏེ (gz_write(state, buf, 1) != 1)
        ཕྱིར -1;
    ཕྱིར c & 0xff;
}

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzputs(gzFile file, const var_na *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ཕྱིར -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    གལ་ཏེ ((ཧྲིལ)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ཕྱིར -1;
    }
    put = gz_write(state, s, len);
    ཕྱིར len && put == 0 ? -1 : (ཧྲིལ)put;
}

#གལ་ཏེ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ཧྲིལ gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    གལ་ཏེ (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ཕྱིར 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    གལ་ཏེ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ཕྱིར 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ཕྱིར strm->avail_in > state->size;
}
#endif

#གལ་ཏེ defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
ཧྲིལ ZEXPORTVA gzvprintf(gzFile file, const var_na *format, va_list va) {
#གལ་ཏེ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    ཕྱིར Z_STREAM_ERROR;
#དེ་མིན
    ཧྲིལ len, ret;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ཕྱིར Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    གལ་ཏེ (state->size == 0 && gz_init(state) == -1)
        ཕྱིར state->err;

    /* check for seek request */
    གལ་ཏེ (state->skip && gz_zero(state) == -1)
        ཕྱིར state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    གལ་ཏེ (state->err) {
        གལ་ཏེ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ཕྱིར state->err;
        }
        གལ་ཏེ (!state->again)
            ཕྱིར state->err;
    }
    གལ་ཏེ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    ཆེད་དུ (len = 0; len < state->size; len++)
        གལ་ཏེ (next[len] == 0) bha_nga;
#  དེ་མིན
    len = vsprintf(next, format, va);
#  endif
#དེ་མིན
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  དེ་མིན
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    གལ་ཏེ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ཕྱིར 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    གལ་ཏེ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        གལ་ཏེ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ཕྱིར state->err;
    }
    གལ་ཏེ (state->err && !state->again)
        ཕྱིར state->err;
    ཕྱིར len;
#endif
}

ཧྲིལ ZEXPORTVA gzprintf(gzFile file, const var_na *format, ...) {
    va_list va;
    ཧྲིལ ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ཕྱིར ret;
}

#དེ་མིན /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ཧྲིལ ZEXPORTVA gzprintf(gzFile file, const var_na *format, ཧྲིལ a1, ཧྲིལ a2, ཧྲིལ a3,
                       ཧྲིལ a4, ཧྲིལ a5, ཧྲིལ a6, ཧྲིལ a7, ཧྲིལ a8, ཧྲིལ a9, ཧྲིལ a10,
                       ཧྲིལ a11, ཧྲིལ a12, ཧྲིལ a13, ཧྲིལ a14, ཧྲིལ a15, ཧྲིལ a16,
                       ཧྲིལ a17, ཧྲིལ a18, ཧྲིལ a19, ཧྲིལ a20) {
#གལ་ཏེ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    ཕྱིར Z_STREAM_ERROR;
#དེ་མིན
    ཧྲིལ ret;
    unsigned len, left;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    གལ་ཏེ (parimaa_na(ཧྲིལ) != parimaa_na(void *))
        ཕྱིར Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ཕྱིར Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    གལ་ཏེ (state->size == 0 && gz_init(state) == -1)
        ཕྱིར state->err;

    /* check for seek request */
    གལ་ཏེ (state->skip && gz_zero(state) == -1)
        ཕྱིར state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    གལ་ཏེ (state->err) {
        གལ་ཏེ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ཕྱིར state->err;
        }
        གལ་ཏེ (!state->again)
            ཕྱིར state->err;
    }
    གལ་ཏེ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    ཆེད་དུ (len = 0; len < size; len++)
        གལ་ཏེ (next[len] == 0)
            bha_nga;
#  དེ་མིན
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#དེ་མིན
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  དེ་མིན
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    གལ་ཏེ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ཕྱིར 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    གལ་ཏེ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        གལ་ཏེ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ཕྱིར state->err;
    }
    གལ་ཏེ (state->err && !state->again)
        ཕྱིར state->err;
    ཕྱིར (ཧྲིལ)len;
#endif
}

#endif

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzflush(gzFile file, ཧྲིལ flush) {
    gz_statep state;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ཕྱིར Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    གལ་ཏེ (flush < 0 || flush > Z_FINISH)
        ཕྱིར Z_STREAM_ERROR;

    /* check for seek request */
    གལ་ཏེ (state->skip && gz_zero(state) == -1)
        ཕྱིར state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    ཕྱིར state->err;
}

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzsetparams(gzFile file, ཧྲིལ level, ཧྲིལ strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    གལ་ཏེ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ཕྱིར Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    གལ་ཏེ (level == state->level && strategy == state->strategy)
        ཕྱིར Z_OK;

    /* check for seek request */
    གལ་ཏེ (state->skip && gz_zero(state) == -1)
        ཕྱིར state->err;

    /* change compression parameters for subsequent input */
    གལ་ཏེ (state->size) {
        /* flush previous input with previous parameters before changing */
        གལ་ཏེ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ཕྱིར state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ཕྱིར Z_OK;
}

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzclose_w(gzFile file) {
    ཧྲིལ ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    གལ་ཏེ (state->mode != GZ_WRITE)
        ཕྱིར Z_STREAM_ERROR;

    /* check for seek request */
    གལ་ཏེ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    གལ་ཏེ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    གལ་ཏེ (state->size) {
        གལ་ཏེ (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    གལ་ཏེ (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    ཕྱིར ret;
}
