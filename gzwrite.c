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
    ᱡᱩᱫᱤ (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ᱨᱩᱣᱟᱹᱲ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ᱡᱩᱫᱤ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned var_na *)malloc(state->want);
        ᱡᱩᱫᱤ (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ᱨᱩᱣᱟᱹᱲ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ᱡᱩᱫᱤ (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ᱨᱩᱣᱟᱹᱲ -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ᱡᱩᱫᱤ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ᱨᱩᱣᱟᱹᱲ 0;
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
    ᱡᱩᱫᱤ (state->size == 0 && gz_init(state) == -1)
        ᱨᱩᱣᱟᱹᱲ -1;

    /* write directly if requested */
    ᱡᱩᱫᱤ (state->direct) {
        ᱡᱚᱠᱷᱚᱱ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (puur_naa_nka)write(state->fd, strm->next_in, put);
            ᱡᱩᱫᱤ (writ < 0) {
                ᱡᱩᱫᱤ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ᱨᱩᱣᱟᱹᱲ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ᱨᱩᱣᱟᱹᱲ 0;
    }

    /* check for a pending reset */
    ᱡᱩᱫᱤ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ᱡᱩᱫᱤ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ᱨᱩᱣᱟᱹᱲ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ᱡᱩᱫᱤ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ᱡᱚᱠᱷᱚᱱ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (puur_naa_nka)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (puur_naa_nka)write(state->fd, state->x.next, put);
                ᱡᱩᱫᱤ (writ < 0) {
                    ᱡᱩᱫᱤ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ᱨᱩᱣᱟᱹᱲ -1;
                }
                state->x.next += writ;
            }
            ᱡᱩᱫᱤ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ᱡᱩᱫᱤ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ᱨᱩᱣᱟᱹᱲ -1;
        }
        have -= strm->avail_out;
    } ᱡᱚᱠᱷᱚᱱ (have);

    /* if that completed a deflate stream, allow another to start */
    ᱡᱩᱫᱤ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ᱨᱩᱣᱟᱹᱲ 0;
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
    ᱡᱩᱫᱤ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ᱨᱩᱣᱟᱹᱲ -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ᱡᱩᱫᱤ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ᱡᱩᱫᱤ (ret == -1)
            ᱨᱩᱣᱟᱹᱲ -1;
    } ᱡᱚᱠᱷᱚᱱ (state->skip);
    ᱨᱩᱣᱟᱹᱲ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    puur_naa_nka ret;

    /* if len is zero, avoid unnecessary operations */
    ᱡᱩᱫᱤ (len == 0)
        ᱨᱩᱣᱟᱹᱲ 0;

    /* allocate memory if this is the first time through */
    ᱡᱩᱫᱤ (state->size == 0 && gz_init(state) == -1)
        ᱨᱩᱣᱟᱹᱲ 0;

    /* check for seek request */
    ᱡᱩᱫᱤ (state->skip && gz_zero(state) == -1)
        ᱨᱩᱣᱟᱹᱲ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ᱡᱩᱫᱤ (len < state->size) {
        /* copy to input buffer, compress when full */
        for (;;) {
            unsigned have, copy;

            ᱡᱩᱫᱤ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ᱡᱩᱫᱤ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const var_na *)buf + copy;
            len -= copy;
            ᱡᱩᱫᱤ (len == 0)
                bha_nga;
            ᱡᱩᱫᱤ (gz_comp(state, Z_NO_FLUSH) == -1)
                ᱨᱩᱣᱟᱹᱲ state->again ? put - len : 0;
        }
    }
    ᱵᱟᱝ {
        /* consume whatever's left in the input buffer */
        ᱡᱩᱫᱤ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ᱨᱩᱣᱟᱹᱲ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            ᱡᱩᱫᱤ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ᱡᱩᱫᱤ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ᱨᱩᱣᱟᱹᱲ state->again ? put - len : 0;
            }
        } ᱡᱚᱠᱷᱚᱱ (len);
    }

    /* input was all buffered or compressed */
    ᱨᱩᱣᱟᱹᱲ put;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᱨᱩᱣᱟᱹᱲ 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ᱡᱩᱫᱤ ((puur_naa_nka)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ᱨᱩᱣᱟᱹᱲ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ᱨᱩᱣᱟᱹᱲ (puur_naa_nka)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᱨᱩᱣᱟᱹᱲ 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ᱡᱩᱫᱤ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ᱨᱩᱣᱟᱹᱲ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ᱨᱩᱣᱟᱹᱲ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzputc(gzFile file, puur_naa_nka c) {
    unsigned have;
    unsigned var_na buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᱨᱩᱣᱟᱹᱲ -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    ᱡᱩᱫᱤ (state->skip && gz_zero(state) == -1)
        ᱨᱩᱣᱟᱹᱲ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ᱡᱩᱫᱤ (state->size) {
        ᱡᱩᱫᱤ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ᱡᱩᱫᱤ (have < state->size) {
            state->in[have] = (unsigned var_na)c;
            strm->avail_in++;
            state->x.pos++;
            ᱨᱩᱣᱟᱹᱲ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned var_na)c;
    ᱡᱩᱫᱤ (gz_write(state, buf, 1) != 1)
        ᱨᱩᱣᱟᱹᱲ -1;
    ᱨᱩᱣᱟᱹᱲ c & 0xff;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzputs(gzFile file, const var_na *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᱨᱩᱣᱟᱹᱲ -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    ᱡᱩᱫᱤ ((puur_naa_nka)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ᱨᱩᱣᱟᱹᱲ -1;
    }
    put = gz_write(state, s, len);
    ᱨᱩᱣᱟᱹᱲ len && put == 0 ? -1 : (puur_naa_nka)put;
}

#ᱡᱩᱫᱤ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
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
    ᱡᱩᱫᱤ (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ᱨᱩᱣᱟᱹᱲ 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    ᱡᱩᱫᱤ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ᱨᱩᱣᱟᱹᱲ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ᱨᱩᱣᱟᱹᱲ strm->avail_in > state->size;
}
#endif

#ᱡᱩᱫᱤ defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
puur_naa_nka ZEXPORTVA gzvprintf(gzFile file, const var_na *format, va_list va) {
#ᱡᱩᱫᱤ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
#ᱵᱟᱝ
    puur_naa_nka len, ret;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ᱡᱩᱫᱤ (state->size == 0 && gz_init(state) == -1)
        ᱨᱩᱣᱟᱹᱲ state->err;

    /* check for seek request */
    ᱡᱩᱫᱤ (state->skip && gz_zero(state) == -1)
        ᱨᱩᱣᱟᱹᱲ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ᱡᱩᱫᱤ (state->err) {
        ᱡᱩᱫᱤ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ᱨᱩᱣᱟᱹᱲ state->err;
        }
        ᱡᱩᱫᱤ (!state->again)
            ᱨᱩᱣᱟᱹᱲ state->err;
    }
    ᱡᱩᱫᱤ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    for (len = 0; len < state->size; len++)
        ᱡᱩᱫᱤ (next[len] == 0) bha_nga;
#  ᱵᱟᱝ
    len = vsprintf(next, format, va);
#  endif
#ᱵᱟᱝ
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  ᱵᱟᱝ
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ᱡᱩᱫᱤ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ᱨᱩᱣᱟᱹᱲ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ᱡᱩᱫᱤ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ᱡᱩᱫᱤ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ᱨᱩᱣᱟᱹᱲ state->err;
    }
    ᱡᱩᱫᱤ (state->err && !state->again)
        ᱨᱩᱣᱟᱹᱲ state->err;
    ᱨᱩᱣᱟᱹᱲ len;
#endif
}

puur_naa_nka ZEXPORTVA gzprintf(gzFile file, const var_na *format, ...) {
    va_list va;
    puur_naa_nka ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ᱨᱩᱣᱟᱹᱲ ret;
}

#ᱵᱟᱝ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
puur_naa_nka ZEXPORTVA gzprintf(gzFile file, const var_na *format, puur_naa_nka a1, puur_naa_nka a2, puur_naa_nka a3,
                       puur_naa_nka a4, puur_naa_nka a5, puur_naa_nka a6, puur_naa_nka a7, puur_naa_nka a8, puur_naa_nka a9, puur_naa_nka a10,
                       puur_naa_nka a11, puur_naa_nka a12, puur_naa_nka a13, puur_naa_nka a14, puur_naa_nka a15, puur_naa_nka a16,
                       puur_naa_nka a17, puur_naa_nka a18, puur_naa_nka a19, puur_naa_nka a20) {
#ᱡᱩᱫᱤ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
#ᱵᱟᱝ
    puur_naa_nka ret;
    unsigned len, left;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ᱡᱩᱫᱤ (parimaa_na(puur_naa_nka) != parimaa_na(void *))
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ᱡᱩᱫᱤ (state->size == 0 && gz_init(state) == -1)
        ᱨᱩᱣᱟᱹᱲ state->err;

    /* check for seek request */
    ᱡᱩᱫᱤ (state->skip && gz_zero(state) == -1)
        ᱨᱩᱣᱟᱹᱲ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ᱡᱩᱫᱤ (state->err) {
        ᱡᱩᱫᱤ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ᱨᱩᱣᱟᱹᱲ state->err;
        }
        ᱡᱩᱫᱤ (!state->again)
            ᱨᱩᱣᱟᱹᱲ state->err;
    }
    ᱡᱩᱫᱤ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    for (len = 0; len < size; len++)
        ᱡᱩᱫᱤ (next[len] == 0)
            bha_nga;
#  ᱵᱟᱝ
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ᱵᱟᱝ
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  ᱵᱟᱝ
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ᱡᱩᱫᱤ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ᱨᱩᱣᱟᱹᱲ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ᱡᱩᱫᱤ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ᱡᱩᱫᱤ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ᱨᱩᱣᱟᱹᱲ state->err;
    }
    ᱡᱩᱫᱤ (state->err && !state->again)
        ᱨᱩᱣᱟᱹᱲ state->err;
    ᱨᱩᱣᱟᱹᱲ (puur_naa_nka)len;
#endif
}

#endif

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzflush(gzFile file, puur_naa_nka flush) {
    gz_statep state;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    ᱡᱩᱫᱤ (flush < 0 || flush > Z_FINISH)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;

    /* check for seek request */
    ᱡᱩᱫᱤ (state->skip && gz_zero(state) == -1)
        ᱨᱩᱣᱟᱹᱲ state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    ᱨᱩᱣᱟᱹᱲ state->err;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzsetparams(gzFile file, puur_naa_nka level, puur_naa_nka strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    ᱡᱩᱫᱤ (level == state->level && strategy == state->strategy)
        ᱨᱩᱣᱟᱹᱲ Z_OK;

    /* check for seek request */
    ᱡᱩᱫᱤ (state->skip && gz_zero(state) == -1)
        ᱨᱩᱣᱟᱹᱲ state->err;

    /* change compression parameters for subsequent input */
    ᱡᱩᱫᱤ (state->size) {
        /* flush previous input with previous parameters before changing */
        ᱡᱩᱫᱤ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ᱨᱩᱣᱟᱹᱲ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ᱨᱩᱣᱟᱹᱲ Z_OK;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzclose_w(gzFile file) {
    puur_naa_nka ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ᱡᱩᱫᱤ (file == NULL)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ᱡᱩᱫᱤ (state->mode != GZ_WRITE)
        ᱨᱩᱣᱟᱹᱲ Z_STREAM_ERROR;

    /* check for seek request */
    ᱡᱩᱫᱤ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ᱡᱩᱫᱤ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ᱡᱩᱫᱤ (state->size) {
        ᱡᱩᱫᱤ (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ᱡᱩᱫᱤ (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    ᱨᱩᱣᱟᱹᱲ ret;
}
