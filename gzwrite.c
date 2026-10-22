/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local จำนวนเต็ม gz_init(gz_statep state) {
    จำนวนเต็ม ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned var_na *)malloc(state->want << 1);
    ถ้า (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        คืน -1;
    }

    /* only need output buffer and deflate state if compressing */
    ถ้า (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned var_na *)malloc(state->want);
        ถ้า (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            คืน -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ถ้า (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            คืน -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ถ้า (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    คืน 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local จำนวนเต็ม gz_comp(gz_statep state, จำนวนเต็ม flush) {
    จำนวนเต็ม ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ถ้า (state->size == 0 && gz_init(state) == -1)
        คืน -1;

    /* write directly if requested */
    ถ้า (state->direct) {
        ขณะ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (จำนวนเต็ม)write(state->fd, strm->next_in, put);
            ถ้า (writ < 0) {
                ถ้า (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                คืน -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        คืน 0;
    }

    /* check for a pending reset */
    ถ้า (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ถ้า (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            คืน 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ถ้า (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ขณะ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (จำนวนเต็ม)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (จำนวนเต็ม)write(state->fd, state->x.next, put);
                ถ้า (writ < 0) {
                    ถ้า (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    คืน -1;
                }
                state->x.next += writ;
            }
            ถ้า (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ถ้า (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            คืน -1;
        }
        have -= strm->avail_out;
    } ขณะ (have);

    /* if that completed a deflate stream, allow another to start */
    ถ้า (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    คืน 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local จำนวนเต็ม gz_zero(gz_statep state) {
    จำนวนเต็ม first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ถ้า (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        คืน -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ถ้า (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ถ้า (ret == -1)
            คืน -1;
    } ขณะ (state->skip);
    คืน 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    จำนวนเต็ม ret;

    /* if len is zero, avoid unnecessary operations */
    ถ้า (len == 0)
        คืน 0;

    /* allocate memory if this is the first time through */
    ถ้า (state->size == 0 && gz_init(state) == -1)
        คืน 0;

    /* check for seek request */
    ถ้า (state->skip && gz_zero(state) == -1)
        คืน 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ถ้า (len < state->size) {
        /* copy to input buffer, compress when full */
        สำหรับ (;;) {
            unsigned have, copy;

            ถ้า (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ถ้า (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const var_na *)buf + copy;
            len -= copy;
            ถ้า (len == 0)
                bha_nga;
            ถ้า (gz_comp(state, Z_NO_FLUSH) == -1)
                คืน state->again ? put - len : 0;
        }
    }
    มิฉะนั้น {
        /* consume whatever's left in the input buffer */
        ถ้า (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            คืน 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            ถ้า (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ถ้า (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                คืน state->again ? put - len : 0;
            }
        } ขณะ (len);
    }

    /* input was all buffered or compressed */
    คืน put;
}

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        คืน 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ถ้า ((จำนวนเต็ม)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        คืน 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    คืน (จำนวนเต็ม)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        คืน 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ถ้า (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        คืน 0;
    }

    /* write len bytes to buf, return the number of full items written */
    คืน len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORT gzputc(gzFile file, จำนวนเต็ม c) {
    unsigned have;
    unsigned var_na buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        คืน -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    ถ้า (state->skip && gz_zero(state) == -1)
        คืน -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ถ้า (state->size) {
        ถ้า (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ถ้า (have < state->size) {
            state->in[have] = (unsigned var_na)c;
            strm->avail_in++;
            state->x.pos++;
            คืน c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned var_na)c;
    ถ้า (gz_write(state, buf, 1) != 1)
        คืน -1;
    คืน c & 0xff;
}

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORT gzputs(gzFile file, const var_na *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        คืน -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    ถ้า ((จำนวนเต็ม)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        คืน -1;
    }
    put = gz_write(state, s, len);
    คืน len && put == 0 ? -1 : (จำนวนเต็ม)put;
}

#ถ้า (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local จำนวนเต็ม gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ถ้า (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        คืน 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    ถ้า (strm->avail_in == 0) {
        strm->next_in = state->in;
        คืน 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    คืน strm->avail_in > state->size;
}
#endif

#ถ้า defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORTVA gzvprintf(gzFile file, const var_na *format, va_list va) {
#ถ้า defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    คืน Z_STREAM_ERROR;
#มิฉะนั้น
    จำนวนเต็ม len, ret;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        คืน Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ถ้า (state->size == 0 && gz_init(state) == -1)
        คืน state->err;

    /* check for seek request */
    ถ้า (state->skip && gz_zero(state) == -1)
        คืน state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ถ้า (state->err) {
        ถ้า (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            คืน state->err;
        }
        ถ้า (!state->again)
            คืน state->err;
    }
    ถ้า (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    สำหรับ (len = 0; len < state->size; len++)
        ถ้า (next[len] == 0) bha_nga;
#  มิฉะนั้น
    len = vsprintf(next, format, va);
#  endif
#มิฉะนั้น
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  มิฉะนั้น
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ถ้า (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        คืน 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ถ้า (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ถ้า (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        คืน state->err;
    }
    ถ้า (state->err && !state->again)
        คืน state->err;
    คืน len;
#endif
}

จำนวนเต็ม ZEXPORTVA gzprintf(gzFile file, const var_na *format, ...) {
    va_list va;
    จำนวนเต็ม ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    คืน ret;
}

#มิฉะนั้น /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORTVA gzprintf(gzFile file, const var_na *format, จำนวนเต็ม a1, จำนวนเต็ม a2, จำนวนเต็ม a3,
                       จำนวนเต็ม a4, จำนวนเต็ม a5, จำนวนเต็ม a6, จำนวนเต็ม a7, จำนวนเต็ม a8, จำนวนเต็ม a9, จำนวนเต็ม a10,
                       จำนวนเต็ม a11, จำนวนเต็ม a12, จำนวนเต็ม a13, จำนวนเต็ม a14, จำนวนเต็ม a15, จำนวนเต็ม a16,
                       จำนวนเต็ม a17, จำนวนเต็ม a18, จำนวนเต็ม a19, จำนวนเต็ม a20) {
#ถ้า defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    คืน Z_STREAM_ERROR;
#มิฉะนั้น
    จำนวนเต็ม ret;
    unsigned len, left;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ถ้า (parimaa_na(จำนวนเต็ม) != parimaa_na(void *))
        คืน Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        คืน Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ถ้า (state->size == 0 && gz_init(state) == -1)
        คืน state->err;

    /* check for seek request */
    ถ้า (state->skip && gz_zero(state) == -1)
        คืน state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ถ้า (state->err) {
        ถ้า (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            คืน state->err;
        }
        ถ้า (!state->again)
            คืน state->err;
    }
    ถ้า (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    สำหรับ (len = 0; len < size; len++)
        ถ้า (next[len] == 0)
            bha_nga;
#  มิฉะนั้น
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#มิฉะนั้น
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  มิฉะนั้น
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ถ้า (len == 0 || len >= state->size || next[state->size - 1] != 0)
        คืน 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ถ้า (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ถ้า (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        คืน state->err;
    }
    ถ้า (state->err && !state->again)
        คืน state->err;
    คืน (จำนวนเต็ม)len;
#endif
}

#endif

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORT gzflush(gzFile file, จำนวนเต็ม flush) {
    gz_statep state;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        คืน Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    ถ้า (flush < 0 || flush > Z_FINISH)
        คืน Z_STREAM_ERROR;

    /* check for seek request */
    ถ้า (state->skip && gz_zero(state) == -1)
        คืน state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    คืน state->err;
}

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORT gzsetparams(gzFile file, จำนวนเต็ม level, จำนวนเต็ม strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ถ้า (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        คืน Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    ถ้า (level == state->level && strategy == state->strategy)
        คืน Z_OK;

    /* check for seek request */
    ถ้า (state->skip && gz_zero(state) == -1)
        คืน state->err;

    /* change compression parameters for subsequent input */
    ถ้า (state->size) {
        /* flush previous input with previous parameters before changing */
        ถ้า (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            คืน state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    คืน Z_OK;
}

/* -- see zlib.h -- */
จำนวนเต็ม ZEXPORT gzclose_w(gzFile file) {
    จำนวนเต็ม ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ถ้า (file == NULL)
        คืน Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ถ้า (state->mode != GZ_WRITE)
        คืน Z_STREAM_ERROR;

    /* check for seek request */
    ถ้า (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ถ้า (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ถ้า (state->size) {
        ถ้า (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ถ้า (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    คืน ret;
}
