/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ចំនួនគត់ gz_init(gz_statep state) {
    ចំនួនគត់ ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned var_na *)malloc(state->want << 1);
    ប្រសិនបើ (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ត្រឡប់ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ប្រសិនបើ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned var_na *)malloc(state->want);
        ប្រសិនបើ (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ត្រឡប់ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ប្រសិនបើ (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ត្រឡប់ -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ប្រសិនបើ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ត្រឡប់ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ចំនួនគត់ gz_comp(gz_statep state, ចំនួនគត់ flush) {
    ចំនួនគត់ ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ប្រសិនបើ (state->size == 0 && gz_init(state) == -1)
        ត្រឡប់ -1;

    /* write directly if requested */
    ប្រសិនបើ (state->direct) {
        ខណៈពេល (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (ចំនួនគត់)write(state->fd, strm->next_in, put);
            ប្រសិនបើ (writ < 0) {
                ប្រសិនបើ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ត្រឡប់ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ត្រឡប់ 0;
    }

    /* check for a pending reset */
    ប្រសិនបើ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ប្រសិនបើ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ត្រឡប់ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ប្រសិនបើ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ខណៈពេល (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (ចំនួនគត់)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (ចំនួនគត់)write(state->fd, state->x.next, put);
                ប្រសិនបើ (writ < 0) {
                    ប្រសិនបើ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ត្រឡប់ -1;
                }
                state->x.next += writ;
            }
            ប្រសិនបើ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ប្រសិនបើ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ត្រឡប់ -1;
        }
        have -= strm->avail_out;
    } ខណៈពេល (have);

    /* if that completed a deflate stream, allow another to start */
    ប្រសិនបើ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ត្រឡប់ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ចំនួនគត់ gz_zero(gz_statep state) {
    ចំនួនគត់ first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ប្រសិនបើ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ត្រឡប់ -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ប្រសិនបើ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ប្រសិនបើ (ret == -1)
            ត្រឡប់ -1;
    } ខណៈពេល (state->skip);
    ត្រឡប់ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    ចំនួនគត់ ret;

    /* if len is zero, avoid unnecessary operations */
    ប្រសិនបើ (len == 0)
        ត្រឡប់ 0;

    /* allocate memory if this is the first time through */
    ប្រសិនបើ (state->size == 0 && gz_init(state) == -1)
        ត្រឡប់ 0;

    /* check for seek request */
    ប្រសិនបើ (state->skip && gz_zero(state) == -1)
        ត្រឡប់ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ប្រសិនបើ (len < state->size) {
        /* copy to input buffer, compress when full */
        សម្រាប់ (;;) {
            unsigned have, copy;

            ប្រសិនបើ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ប្រសិនបើ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const var_na *)buf + copy;
            len -= copy;
            ប្រសិនបើ (len == 0)
                bha_nga;
            ប្រសិនបើ (gz_comp(state, Z_NO_FLUSH) == -1)
                ត្រឡប់ state->again ? put - len : 0;
        }
    }
    បើមិនដូច្នេះ {
        /* consume whatever's left in the input buffer */
        ប្រសិនបើ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ត្រឡប់ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            ប្រសិនបើ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ប្រសិនបើ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ត្រឡប់ state->again ? put - len : 0;
            }
        } ខណៈពេល (len);
    }

    /* input was all buffered or compressed */
    ត្រឡប់ put;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ត្រឡប់ 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ប្រសិនបើ ((ចំនួនគត់)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ត្រឡប់ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ត្រឡប់ (ចំនួនគត់)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ត្រឡប់ 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ប្រសិនបើ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ត្រឡប់ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ត្រឡប់ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzputc(gzFile file, ចំនួនគត់ c) {
    unsigned have;
    unsigned var_na buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ត្រឡប់ -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    ប្រសិនបើ (state->skip && gz_zero(state) == -1)
        ត្រឡប់ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ប្រសិនបើ (state->size) {
        ប្រសិនបើ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ប្រសិនបើ (have < state->size) {
            state->in[have] = (unsigned var_na)c;
            strm->avail_in++;
            state->x.pos++;
            ត្រឡប់ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned var_na)c;
    ប្រសិនបើ (gz_write(state, buf, 1) != 1)
        ត្រឡប់ -1;
    ត្រឡប់ c & 0xff;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzputs(gzFile file, const var_na *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ត្រឡប់ -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    ប្រសិនបើ ((ចំនួនគត់)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ត្រឡប់ -1;
    }
    put = gz_write(state, s, len);
    ត្រឡប់ len && put == 0 ? -1 : (ចំនួនគត់)put;
}

#ប្រសិនបើ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ចំនួនគត់ gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ប្រសិនបើ (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ត្រឡប់ 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    ប្រសិនបើ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ត្រឡប់ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ត្រឡប់ strm->avail_in > state->size;
}
#endif

#ប្រសិនបើ defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORTVA gzvprintf(gzFile file, const var_na *format, va_list va) {
#ប្រសិនបើ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    ត្រឡប់ Z_STREAM_ERROR;
#បើមិនដូច្នេះ
    ចំនួនគត់ len, ret;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ត្រឡប់ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ប្រសិនបើ (state->size == 0 && gz_init(state) == -1)
        ត្រឡប់ state->err;

    /* check for seek request */
    ប្រសិនបើ (state->skip && gz_zero(state) == -1)
        ត្រឡប់ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ប្រសិនបើ (state->err) {
        ប្រសិនបើ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ត្រឡប់ state->err;
        }
        ប្រសិនបើ (!state->again)
            ត្រឡប់ state->err;
    }
    ប្រសិនបើ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    សម្រាប់ (len = 0; len < state->size; len++)
        ប្រសិនបើ (next[len] == 0) bha_nga;
#  បើមិនដូច្នេះ
    len = vsprintf(next, format, va);
#  endif
#បើមិនដូច្នេះ
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  បើមិនដូច្នេះ
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ប្រសិនបើ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ត្រឡប់ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ប្រសិនបើ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ប្រសិនបើ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ត្រឡប់ state->err;
    }
    ប្រសិនបើ (state->err && !state->again)
        ត្រឡប់ state->err;
    ត្រឡប់ len;
#endif
}

ចំនួនគត់ ZEXPORTVA gzprintf(gzFile file, const var_na *format, ...) {
    va_list va;
    ចំនួនគត់ ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ត្រឡប់ ret;
}

#បើមិនដូច្នេះ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORTVA gzprintf(gzFile file, const var_na *format, ចំនួនគត់ a1, ចំនួនគត់ a2, ចំនួនគត់ a3,
                       ចំនួនគត់ a4, ចំនួនគត់ a5, ចំនួនគត់ a6, ចំនួនគត់ a7, ចំនួនគត់ a8, ចំនួនគត់ a9, ចំនួនគត់ a10,
                       ចំនួនគត់ a11, ចំនួនគត់ a12, ចំនួនគត់ a13, ចំនួនគត់ a14, ចំនួនគត់ a15, ចំនួនគត់ a16,
                       ចំនួនគត់ a17, ចំនួនគត់ a18, ចំនួនគត់ a19, ចំនួនគត់ a20) {
#ប្រសិនបើ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    ត្រឡប់ Z_STREAM_ERROR;
#បើមិនដូច្នេះ
    ចំនួនគត់ ret;
    unsigned len, left;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ប្រសិនបើ (parimaa_na(ចំនួនគត់) != parimaa_na(void *))
        ត្រឡប់ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ត្រឡប់ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ប្រសិនបើ (state->size == 0 && gz_init(state) == -1)
        ត្រឡប់ state->err;

    /* check for seek request */
    ប្រសិនបើ (state->skip && gz_zero(state) == -1)
        ត្រឡប់ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ប្រសិនបើ (state->err) {
        ប្រសិនបើ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ត្រឡប់ state->err;
        }
        ប្រសិនបើ (!state->again)
            ត្រឡប់ state->err;
    }
    ប្រសិនបើ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    សម្រាប់ (len = 0; len < size; len++)
        ប្រសិនបើ (next[len] == 0)
            bha_nga;
#  បើមិនដូច្នេះ
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#បើមិនដូច្នេះ
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  បើមិនដូច្នេះ
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ប្រសិនបើ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ត្រឡប់ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ប្រសិនបើ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ប្រសិនបើ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ត្រឡប់ state->err;
    }
    ប្រសិនបើ (state->err && !state->again)
        ត្រឡប់ state->err;
    ត្រឡប់ (ចំនួនគត់)len;
#endif
}

#endif

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzflush(gzFile file, ចំនួនគត់ flush) {
    gz_statep state;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ត្រឡប់ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    ប្រសិនបើ (flush < 0 || flush > Z_FINISH)
        ត្រឡប់ Z_STREAM_ERROR;

    /* check for seek request */
    ប្រសិនបើ (state->skip && gz_zero(state) == -1)
        ត្រឡប់ state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    ត្រឡប់ state->err;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzsetparams(gzFile file, ចំនួនគត់ level, ចំនួនគត់ strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ប្រសិនបើ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ត្រឡប់ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    ប្រសិនបើ (level == state->level && strategy == state->strategy)
        ត្រឡប់ Z_OK;

    /* check for seek request */
    ប្រសិនបើ (state->skip && gz_zero(state) == -1)
        ត្រឡប់ state->err;

    /* change compression parameters for subsequent input */
    ប្រសិនបើ (state->size) {
        /* flush previous input with previous parameters before changing */
        ប្រសិនបើ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ត្រឡប់ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ត្រឡប់ Z_OK;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzclose_w(gzFile file) {
    ចំនួនគត់ ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ប្រសិនបើ (state->mode != GZ_WRITE)
        ត្រឡប់ Z_STREAM_ERROR;

    /* check for seek request */
    ប្រសិនបើ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ប្រសិនបើ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ប្រសិនបើ (state->size) {
        ប្រសិនបើ (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ប្រសិនបើ (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    ត្រឡប់ ret;
}
