/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local số_nguyên gz_init(gz_statep state) {
    số_nguyên ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned char *)malloc(state->want << 1);
    nếu (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        trả_về -1;
    }

    /* only need output buffer and deflate state if compressing */
    nếu (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned char *)malloc(state->want);
        nếu (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            trả_về -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        nếu (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            trả_về -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    nếu (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    trả_về 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local số_nguyên gz_comp(gz_statep state, số_nguyên flush) {
    số_nguyên ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    nếu (state->size == 0 && gz_init(state) == -1)
        trả_về -1;

    /* write directly if requested */
    nếu (state->direct) {
        trong_khi (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (số_nguyên)write(state->fd, strm->next_in, put);
            nếu (writ < 0) {
                nếu (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                trả_về -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        trả_về 0;
    }

    /* check for a pending reset */
    nếu (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        nếu (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            trả_về 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        nếu (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            trong_khi (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (số_nguyên)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (số_nguyên)write(state->fd, state->x.next, put);
                nếu (writ < 0) {
                    nếu (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    trả_về -1;
                }
                state->x.next += writ;
            }
            nếu (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        nếu (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            trả_về -1;
        }
        have -= strm->avail_out;
    } trong_khi (have);

    /* if that completed a deflate stream, allow another to start */
    nếu (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    trả_về 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local số_nguyên gz_zero(gz_statep state) {
    số_nguyên first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    nếu (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        trả_về -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        nếu (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        nếu (ret == -1)
            trả_về -1;
    } trong_khi (state->skip);
    trả_về 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    số_nguyên ret;

    /* if len is zero, avoid unnecessary operations */
    nếu (len == 0)
        trả_về 0;

    /* allocate memory if this is the first time through */
    nếu (state->size == 0 && gz_init(state) == -1)
        trả_về 0;

    /* check for seek request */
    nếu (state->skip && gz_zero(state) == -1)
        trả_về 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    nếu (len < state->size) {
        /* copy to input buffer, compress when full */
        cho (;;) {
            unsigned have, copy;

            nếu (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            nếu (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const char *)buf + copy;
            len -= copy;
            nếu (len == 0)
                break;
            nếu (gz_comp(state, Z_NO_FLUSH) == -1)
                trả_về state->again ? put - len : 0;
        }
    }
    không_thì {
        /* consume whatever's left in the input buffer */
        nếu (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            trả_về 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            nếu (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            nếu (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                trả_về state->again ? put - len : 0;
            }
        } trong_khi (len);
    }

    /* input was all buffered or compressed */
    trả_về put;
}

/* -- see zlib.h -- */
số_nguyên ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    nếu (file == NULL)
        trả_về 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        trả_về 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    nếu ((số_nguyên)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        trả_về 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    trả_về (số_nguyên)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    nếu (file == NULL)
        trả_về 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        trả_về 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    nếu (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        trả_về 0;
    }

    /* write len bytes to buf, return the number of full items written */
    trả_về len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
số_nguyên ZEXPORT gzputc(gzFile file, số_nguyên c) {
    unsigned have;
    unsigned char buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    nếu (file == NULL)
        trả_về -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        trả_về -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    nếu (state->skip && gz_zero(state) == -1)
        trả_về -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    nếu (state->size) {
        nếu (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        nếu (have < state->size) {
            state->in[have] = (unsigned char)c;
            strm->avail_in++;
            state->x.pos++;
            trả_về c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned char)c;
    nếu (gz_write(state, buf, 1) != 1)
        trả_về -1;
    trả_về c & 0xff;
}

/* -- see zlib.h -- */
số_nguyên ZEXPORT gzputs(gzFile file, const char *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    nếu (file == NULL)
        trả_về -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        trả_về -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    nếu ((số_nguyên)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        trả_về -1;
    }
    put = gz_write(state, s, len);
    trả_về len && put == 0 ? -1 : (số_nguyên)put;
}

#nếu (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local số_nguyên gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    nếu (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        trả_về 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    nếu (strm->avail_in == 0) {
        strm->next_in = state->in;
        trả_về 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    trả_về strm->avail_in > state->size;
}
#endif

#nếu defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
số_nguyên ZEXPORTVA gzvprintf(gzFile file, const char *format, va_list va) {
#nếu defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    trả_về Z_STREAM_ERROR;
#không_thì
    số_nguyên len, ret;
    char *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    nếu (file == NULL)
        trả_về Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        trả_về Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    nếu (state->size == 0 && gz_init(state) == -1)
        trả_về state->err;

    /* check for seek request */
    nếu (state->skip && gz_zero(state) == -1)
        trả_về state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    nếu (state->err) {
        nếu (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            trả_về state->err;
        }
        nếu (!state->again)
            trả_về state->err;
    }
    nếu (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (char *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    cho (len = 0; len < state->size; len++)
        nếu (next[len] == 0) break;
#  không_thì
    len = vsprintf(next, format, va);
#  endif
#không_thì
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  không_thì
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    nếu (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        trả_về 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    nếu (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        nếu (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        trả_về state->err;
    }
    nếu (state->err && !state->again)
        trả_về state->err;
    trả_về len;
#endif
}

số_nguyên ZEXPORTVA gzprintf(gzFile file, const char *format, ...) {
    va_list va;
    số_nguyên ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    trả_về ret;
}

#không_thì /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
số_nguyên ZEXPORTVA gzprintf(gzFile file, const char *format, số_nguyên a1, số_nguyên a2, số_nguyên a3,
                       số_nguyên a4, số_nguyên a5, số_nguyên a6, số_nguyên a7, số_nguyên a8, số_nguyên a9, số_nguyên a10,
                       số_nguyên a11, số_nguyên a12, số_nguyên a13, số_nguyên a14, số_nguyên a15, số_nguyên a16,
                       số_nguyên a17, số_nguyên a18, số_nguyên a19, số_nguyên a20) {
#nếu defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    trả_về Z_STREAM_ERROR;
#không_thì
    số_nguyên ret;
    unsigned len, left;
    char *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    nếu (file == NULL)
        trả_về Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    nếu (sizeof(số_nguyên) != sizeof(void *))
        trả_về Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        trả_về Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    nếu (state->size == 0 && gz_init(state) == -1)
        trả_về state->err;

    /* check for seek request */
    nếu (state->skip && gz_zero(state) == -1)
        trả_về state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    nếu (state->err) {
        nếu (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            trả_về state->err;
        }
        nếu (!state->again)
            trả_về state->err;
    }
    nếu (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (char *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    cho (len = 0; len < size; len++)
        nếu (next[len] == 0)
            break;
#  không_thì
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#không_thì
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  không_thì
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    nếu (len == 0 || len >= state->size || next[state->size - 1] != 0)
        trả_về 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    nếu (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        nếu (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        trả_về state->err;
    }
    nếu (state->err && !state->again)
        trả_về state->err;
    trả_về (số_nguyên)len;
#endif
}

#endif

/* -- see zlib.h -- */
số_nguyên ZEXPORT gzflush(gzFile file, số_nguyên flush) {
    gz_statep state;

    /* get internal structure */
    nếu (file == NULL)
        trả_về Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        trả_về Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    nếu (flush < 0 || flush > Z_FINISH)
        trả_về Z_STREAM_ERROR;

    /* check for seek request */
    nếu (state->skip && gz_zero(state) == -1)
        trả_về state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    trả_về state->err;
}

/* -- see zlib.h -- */
số_nguyên ZEXPORT gzsetparams(gzFile file, số_nguyên level, số_nguyên strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    nếu (file == NULL)
        trả_về Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    nếu (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        trả_về Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    nếu (level == state->level && strategy == state->strategy)
        trả_về Z_OK;

    /* check for seek request */
    nếu (state->skip && gz_zero(state) == -1)
        trả_về state->err;

    /* change compression parameters for subsequent input */
    nếu (state->size) {
        /* flush previous input with previous parameters before changing */
        nếu (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            trả_về state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    trả_về Z_OK;
}

/* -- see zlib.h -- */
số_nguyên ZEXPORT gzclose_w(gzFile file) {
    số_nguyên ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    nếu (file == NULL)
        trả_về Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    nếu (state->mode != GZ_WRITE)
        trả_về Z_STREAM_ERROR;

    /* check for seek request */
    nếu (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    nếu (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    nếu (state->size) {
        nếu (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    nếu (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    trả_về ret;
}
