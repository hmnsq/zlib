/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ကိန်းပြည့် gz_init(gz_statep state) {
    ကိန်းပြည့် ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned var_na *)malloc(state->want << 1);
    အကယ်၍ (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ပြန်ပေး -1;
    }

    /* only need output buffer and deflate state if compressing */
    အကယ်၍ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned var_na *)malloc(state->want);
        အကယ်၍ (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ပြန်ပေး -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        အကယ်၍ (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ပြန်ပေး -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    အကယ်၍ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ပြန်ပေး 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ကိန်းပြည့် gz_comp(gz_statep state, ကိန်းပြည့် flush) {
    ကိန်းပြည့် ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    အကယ်၍ (state->size == 0 && gz_init(state) == -1)
        ပြန်ပေး -1;

    /* write directly if requested */
    အကယ်၍ (state->direct) {
        နေစဉ် (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (ကိန်းပြည့်)write(state->fd, strm->next_in, put);
            အကယ်၍ (writ < 0) {
                အကယ်၍ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ပြန်ပေး -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ပြန်ပေး 0;
    }

    /* check for a pending reset */
    အကယ်၍ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        အကယ်၍ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ပြန်ပေး 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        အကယ်၍ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            နေစဉ် (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (ကိန်းပြည့်)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (ကိန်းပြည့်)write(state->fd, state->x.next, put);
                အကယ်၍ (writ < 0) {
                    အကယ်၍ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ပြန်ပေး -1;
                }
                state->x.next += writ;
            }
            အကယ်၍ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        အကယ်၍ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ပြန်ပေး -1;
        }
        have -= strm->avail_out;
    } နေစဉ် (have);

    /* if that completed a deflate stream, allow another to start */
    အကယ်၍ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ပြန်ပေး 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ကိန်းပြည့် gz_zero(gz_statep state) {
    ကိန်းပြည့် first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    အကယ်၍ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ပြန်ပေး -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        အကယ်၍ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        အကယ်၍ (ret == -1)
            ပြန်ပေး -1;
    } နေစဉ် (state->skip);
    ပြန်ပေး 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    ကိန်းပြည့် ret;

    /* if len is zero, avoid unnecessary operations */
    အကယ်၍ (len == 0)
        ပြန်ပေး 0;

    /* allocate memory if this is the first time through */
    အကယ်၍ (state->size == 0 && gz_init(state) == -1)
        ပြန်ပေး 0;

    /* check for seek request */
    အကယ်၍ (state->skip && gz_zero(state) == -1)
        ပြန်ပေး 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    အကယ်၍ (len < state->size) {
        /* copy to input buffer, compress when full */
        အတွက် (;;) {
            unsigned have, copy;

            အကယ်၍ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            အကယ်၍ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const var_na *)buf + copy;
            len -= copy;
            အကယ်၍ (len == 0)
                bha_nga;
            အကယ်၍ (gz_comp(state, Z_NO_FLUSH) == -1)
                ပြန်ပေး state->again ? put - len : 0;
        }
    }
    မဟုတ်ပါက {
        /* consume whatever's left in the input buffer */
        အကယ်၍ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ပြန်ပေး 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            အကယ်၍ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            အကယ်၍ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ပြန်ပေး state->again ? put - len : 0;
            }
        } နေစဉ် (len);
    }

    /* input was all buffered or compressed */
    ပြန်ပေး put;
}

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ပြန်ပေး 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    အကယ်၍ ((ကိန်းပြည့်)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ပြန်ပေး 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ပြန်ပေး (ကိန်းပြည့်)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ပြန်ပေး 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    အကယ်၍ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ပြန်ပေး 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ပြန်ပေး len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORT gzputc(gzFile file, ကိန်းပြည့် c) {
    unsigned have;
    unsigned var_na buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ပြန်ပေး -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    အကယ်၍ (state->skip && gz_zero(state) == -1)
        ပြန်ပေး -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    အကယ်၍ (state->size) {
        အကယ်၍ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        အကယ်၍ (have < state->size) {
            state->in[have] = (unsigned var_na)c;
            strm->avail_in++;
            state->x.pos++;
            ပြန်ပေး c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned var_na)c;
    အကယ်၍ (gz_write(state, buf, 1) != 1)
        ပြန်ပေး -1;
    ပြန်ပေး c & 0xff;
}

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORT gzputs(gzFile file, const var_na *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ပြန်ပေး -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    အကယ်၍ ((ကိန်းပြည့်)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ပြန်ပေး -1;
    }
    put = gz_write(state, s, len);
    ပြန်ပေး len && put == 0 ? -1 : (ကိန်းပြည့်)put;
}

#အကယ်၍ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ကိန်းပြည့် gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    အကယ်၍ (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ပြန်ပေး 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    အကယ်၍ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ပြန်ပေး 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ပြန်ပေး strm->avail_in > state->size;
}
#endif

#အကယ်၍ defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORTVA gzvprintf(gzFile file, const var_na *format, va_list va) {
#အကယ်၍ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    ပြန်ပေး Z_STREAM_ERROR;
#မဟုတ်ပါက
    ကိန်းပြည့် len, ret;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ပြန်ပေး Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    အကယ်၍ (state->size == 0 && gz_init(state) == -1)
        ပြန်ပေး state->err;

    /* check for seek request */
    အကယ်၍ (state->skip && gz_zero(state) == -1)
        ပြန်ပေး state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    အကယ်၍ (state->err) {
        အကယ်၍ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ပြန်ပေး state->err;
        }
        အကယ်၍ (!state->again)
            ပြန်ပေး state->err;
    }
    အကယ်၍ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    အတွက် (len = 0; len < state->size; len++)
        အကယ်၍ (next[len] == 0) bha_nga;
#  မဟုတ်ပါက
    len = vsprintf(next, format, va);
#  endif
#မဟုတ်ပါက
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  မဟုတ်ပါက
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    အကယ်၍ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ပြန်ပေး 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    အကယ်၍ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        အကယ်၍ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ပြန်ပေး state->err;
    }
    အကယ်၍ (state->err && !state->again)
        ပြန်ပေး state->err;
    ပြန်ပေး len;
#endif
}

ကိန်းပြည့် ZEXPORTVA gzprintf(gzFile file, const var_na *format, ...) {
    va_list va;
    ကိန်းပြည့် ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ပြန်ပေး ret;
}

#မဟုတ်ပါက /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORTVA gzprintf(gzFile file, const var_na *format, ကိန်းပြည့် a1, ကိန်းပြည့် a2, ကိန်းပြည့် a3,
                       ကိန်းပြည့် a4, ကိန်းပြည့် a5, ကိန်းပြည့် a6, ကိန်းပြည့် a7, ကိန်းပြည့် a8, ကိန်းပြည့် a9, ကိန်းပြည့် a10,
                       ကိန်းပြည့် a11, ကိန်းပြည့် a12, ကိန်းပြည့် a13, ကိန်းပြည့် a14, ကိန်းပြည့် a15, ကိန်းပြည့် a16,
                       ကိန်းပြည့် a17, ကိန်းပြည့် a18, ကိန်းပြည့် a19, ကိန်းပြည့် a20) {
#အကယ်၍ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    ပြန်ပေး Z_STREAM_ERROR;
#မဟုတ်ပါက
    ကိန်းပြည့် ret;
    unsigned len, left;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    အကယ်၍ (parimaa_na(ကိန်းပြည့်) != parimaa_na(void *))
        ပြန်ပေး Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ပြန်ပေး Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    အကယ်၍ (state->size == 0 && gz_init(state) == -1)
        ပြန်ပေး state->err;

    /* check for seek request */
    အကယ်၍ (state->skip && gz_zero(state) == -1)
        ပြန်ပေး state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    အကယ်၍ (state->err) {
        အကယ်၍ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ပြန်ပေး state->err;
        }
        အကယ်၍ (!state->again)
            ပြန်ပေး state->err;
    }
    အကယ်၍ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    အတွက် (len = 0; len < size; len++)
        အကယ်၍ (next[len] == 0)
            bha_nga;
#  မဟုတ်ပါက
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#မဟုတ်ပါက
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  မဟုတ်ပါက
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    အကယ်၍ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ပြန်ပေး 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    အကယ်၍ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        အကယ်၍ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ပြန်ပေး state->err;
    }
    အကယ်၍ (state->err && !state->again)
        ပြန်ပေး state->err;
    ပြန်ပေး (ကိန်းပြည့်)len;
#endif
}

#endif

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORT gzflush(gzFile file, ကိန်းပြည့် flush) {
    gz_statep state;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ပြန်ပေး Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    အကယ်၍ (flush < 0 || flush > Z_FINISH)
        ပြန်ပေး Z_STREAM_ERROR;

    /* check for seek request */
    အကယ်၍ (state->skip && gz_zero(state) == -1)
        ပြန်ပေး state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    ပြန်ပေး state->err;
}

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORT gzsetparams(gzFile file, ကိန်းပြည့် level, ကိန်းပြည့် strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    အကယ်၍ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ပြန်ပေး Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    အကယ်၍ (level == state->level && strategy == state->strategy)
        ပြန်ပေး Z_OK;

    /* check for seek request */
    အကယ်၍ (state->skip && gz_zero(state) == -1)
        ပြန်ပေး state->err;

    /* change compression parameters for subsequent input */
    အကယ်၍ (state->size) {
        /* flush previous input with previous parameters before changing */
        အကယ်၍ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ပြန်ပေး state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ပြန်ပေး Z_OK;
}

/* -- see zlib.h -- */
ကိန်းပြည့် ZEXPORT gzclose_w(gzFile file) {
    ကိန်းပြည့် ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    အကယ်၍ (file == NULL)
        ပြန်ပေး Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    အကယ်၍ (state->mode != GZ_WRITE)
        ပြန်ပေး Z_STREAM_ERROR;

    /* check for seek request */
    အကယ်၍ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    အကယ်၍ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    အကယ်၍ (state->size) {
        အကယ်၍ (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    အကယ်၍ (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    ပြန်ပေး ret;
}
