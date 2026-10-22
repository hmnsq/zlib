/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local 정수 gz_init(gz_statep state) {
    정수 ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned char *)malloc(state->want << 1);
    만약 (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        반환 -1;
    }

    /* only need output buffer and deflate state if compressing */
    만약 (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned char *)malloc(state->want);
        만약 (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            반환 -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        만약 (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            반환 -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    만약 (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    반환 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local 정수 gz_comp(gz_statep state, 정수 flush) {
    정수 ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    만약 (state->size == 0 && gz_init(state) == -1)
        반환 -1;

    /* write directly if requested */
    만약 (state->direct) {
        동안 (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (정수)write(state->fd, strm->next_in, put);
            만약 (writ < 0) {
                만약 (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                반환 -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        반환 0;
    }

    /* check for a pending reset */
    만약 (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        만약 (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            반환 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        만약 (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            동안 (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (정수)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (정수)write(state->fd, state->x.next, put);
                만약 (writ < 0) {
                    만약 (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    반환 -1;
                }
                state->x.next += writ;
            }
            만약 (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        만약 (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            반환 -1;
        }
        have -= strm->avail_out;
    } 동안 (have);

    /* if that completed a deflate stream, allow another to start */
    만약 (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    반환 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local 정수 gz_zero(gz_statep state) {
    정수 first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    만약 (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        반환 -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        만약 (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        만약 (ret == -1)
            반환 -1;
    } 동안 (state->skip);
    반환 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    정수 ret;

    /* if len is zero, avoid unnecessary operations */
    만약 (len == 0)
        반환 0;

    /* allocate memory if this is the first time through */
    만약 (state->size == 0 && gz_init(state) == -1)
        반환 0;

    /* check for seek request */
    만약 (state->skip && gz_zero(state) == -1)
        반환 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    만약 (len < state->size) {
        /* copy to input buffer, compress when full */
        위해 (;;) {
            unsigned have, copy;

            만약 (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            만약 (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const char *)buf + copy;
            len -= copy;
            만약 (len == 0)
                break;
            만약 (gz_comp(state, Z_NO_FLUSH) == -1)
                반환 state->again ? put - len : 0;
        }
    }
    아니면 {
        /* consume whatever's left in the input buffer */
        만약 (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            반환 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            만약 (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            만약 (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                반환 state->again ? put - len : 0;
            }
        } 동안 (len);
    }

    /* input was all buffered or compressed */
    반환 put;
}

/* -- see zlib.h -- */
정수 ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    만약 (file == NULL)
        반환 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        반환 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    만약 ((정수)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        반환 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    반환 (정수)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    만약 (file == NULL)
        반환 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        반환 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    만약 (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        반환 0;
    }

    /* write len bytes to buf, return the number of full items written */
    반환 len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
정수 ZEXPORT gzputc(gzFile file, 정수 c) {
    unsigned have;
    unsigned char buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    만약 (file == NULL)
        반환 -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        반환 -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    만약 (state->skip && gz_zero(state) == -1)
        반환 -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    만약 (state->size) {
        만약 (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        만약 (have < state->size) {
            state->in[have] = (unsigned char)c;
            strm->avail_in++;
            state->x.pos++;
            반환 c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned char)c;
    만약 (gz_write(state, buf, 1) != 1)
        반환 -1;
    반환 c & 0xff;
}

/* -- see zlib.h -- */
정수 ZEXPORT gzputs(gzFile file, const char *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    만약 (file == NULL)
        반환 -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        반환 -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    만약 ((정수)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        반환 -1;
    }
    put = gz_write(state, s, len);
    반환 len && put == 0 ? -1 : (정수)put;
}

#만약 (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local 정수 gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    만약 (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        반환 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    만약 (strm->avail_in == 0) {
        strm->next_in = state->in;
        반환 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    반환 strm->avail_in > state->size;
}
#endif

#만약 defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
정수 ZEXPORTVA gzvprintf(gzFile file, const char *format, va_list va) {
#만약 defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    반환 Z_STREAM_ERROR;
#아니면
    정수 len, ret;
    char *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    만약 (file == NULL)
        반환 Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        반환 Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    만약 (state->size == 0 && gz_init(state) == -1)
        반환 state->err;

    /* check for seek request */
    만약 (state->skip && gz_zero(state) == -1)
        반환 state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    만약 (state->err) {
        만약 (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            반환 state->err;
        }
        만약 (!state->again)
            반환 state->err;
    }
    만약 (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (char *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    위해 (len = 0; len < state->size; len++)
        만약 (next[len] == 0) break;
#  아니면
    len = vsprintf(next, format, va);
#  endif
#아니면
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  아니면
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    만약 (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        반환 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    만약 (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        만약 (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        반환 state->err;
    }
    만약 (state->err && !state->again)
        반환 state->err;
    반환 len;
#endif
}

정수 ZEXPORTVA gzprintf(gzFile file, const char *format, ...) {
    va_list va;
    정수 ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    반환 ret;
}

#아니면 /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
정수 ZEXPORTVA gzprintf(gzFile file, const char *format, 정수 a1, 정수 a2, 정수 a3,
                       정수 a4, 정수 a5, 정수 a6, 정수 a7, 정수 a8, 정수 a9, 정수 a10,
                       정수 a11, 정수 a12, 정수 a13, 정수 a14, 정수 a15, 정수 a16,
                       정수 a17, 정수 a18, 정수 a19, 정수 a20) {
#만약 defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    반환 Z_STREAM_ERROR;
#아니면
    정수 ret;
    unsigned len, left;
    char *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    만약 (file == NULL)
        반환 Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    만약 (sizeof(정수) != sizeof(void *))
        반환 Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        반환 Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    만약 (state->size == 0 && gz_init(state) == -1)
        반환 state->err;

    /* check for seek request */
    만약 (state->skip && gz_zero(state) == -1)
        반환 state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    만약 (state->err) {
        만약 (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            반환 state->err;
        }
        만약 (!state->again)
            반환 state->err;
    }
    만약 (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (char *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    위해 (len = 0; len < size; len++)
        만약 (next[len] == 0)
            break;
#  아니면
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#아니면
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  아니면
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    만약 (len == 0 || len >= state->size || next[state->size - 1] != 0)
        반환 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    만약 (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        만약 (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        반환 state->err;
    }
    만약 (state->err && !state->again)
        반환 state->err;
    반환 (정수)len;
#endif
}

#endif

/* -- see zlib.h -- */
정수 ZEXPORT gzflush(gzFile file, 정수 flush) {
    gz_statep state;

    /* get internal structure */
    만약 (file == NULL)
        반환 Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        반환 Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    만약 (flush < 0 || flush > Z_FINISH)
        반환 Z_STREAM_ERROR;

    /* check for seek request */
    만약 (state->skip && gz_zero(state) == -1)
        반환 state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    반환 state->err;
}

/* -- see zlib.h -- */
정수 ZEXPORT gzsetparams(gzFile file, 정수 level, 정수 strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    만약 (file == NULL)
        반환 Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    만약 (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        반환 Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    만약 (level == state->level && strategy == state->strategy)
        반환 Z_OK;

    /* check for seek request */
    만약 (state->skip && gz_zero(state) == -1)
        반환 state->err;

    /* change compression parameters for subsequent input */
    만약 (state->size) {
        /* flush previous input with previous parameters before changing */
        만약 (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            반환 state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    반환 Z_OK;
}

/* -- see zlib.h -- */
정수 ZEXPORT gzclose_w(gzFile file) {
    정수 ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    만약 (file == NULL)
        반환 Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    만약 (state->mode != GZ_WRITE)
        반환 Z_STREAM_ERROR;

    /* check for seek request */
    만약 (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    만약 (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    만약 (state->size) {
        만약 (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    만약 (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    반환 ret;
}
