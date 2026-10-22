/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ຈຳນວນເຕັມ gz_init(gz_statep state) {
    ຈຳນວນເຕັມ ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned var_na *)malloc(state->want << 1);
    ຖ້າ (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ສົ່ງຄືນ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ຖ້າ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned var_na *)malloc(state->want);
        ຖ້າ (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ສົ່ງຄືນ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ຖ້າ (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ສົ່ງຄືນ -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ຖ້າ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ສົ່ງຄືນ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ຈຳນວນເຕັມ gz_comp(gz_statep state, ຈຳນວນເຕັມ flush) {
    ຈຳນວນເຕັມ ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ຖ້າ (state->size == 0 && gz_init(state) == -1)
        ສົ່ງຄືນ -1;

    /* write directly if requested */
    ຖ້າ (state->direct) {
        ໃນຂະນະທີ່ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (ຈຳນວນເຕັມ)write(state->fd, strm->next_in, put);
            ຖ້າ (writ < 0) {
                ຖ້າ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ສົ່ງຄືນ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ສົ່ງຄືນ 0;
    }

    /* check for a pending reset */
    ຖ້າ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ຖ້າ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ສົ່ງຄືນ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ຖ້າ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ໃນຂະນະທີ່ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (ຈຳນວນເຕັມ)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (ຈຳນວນເຕັມ)write(state->fd, state->x.next, put);
                ຖ້າ (writ < 0) {
                    ຖ້າ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ສົ່ງຄືນ -1;
                }
                state->x.next += writ;
            }
            ຖ້າ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ຖ້າ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ສົ່ງຄືນ -1;
        }
        have -= strm->avail_out;
    } ໃນຂະນະທີ່ (have);

    /* if that completed a deflate stream, allow another to start */
    ຖ້າ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ສົ່ງຄືນ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ຈຳນວນເຕັມ gz_zero(gz_statep state) {
    ຈຳນວນເຕັມ first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ຖ້າ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ສົ່ງຄືນ -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ຖ້າ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ຖ້າ (ret == -1)
            ສົ່ງຄືນ -1;
    } ໃນຂະນະທີ່ (state->skip);
    ສົ່ງຄືນ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    ຈຳນວນເຕັມ ret;

    /* if len is zero, avoid unnecessary operations */
    ຖ້າ (len == 0)
        ສົ່ງຄືນ 0;

    /* allocate memory if this is the first time through */
    ຖ້າ (state->size == 0 && gz_init(state) == -1)
        ສົ່ງຄືນ 0;

    /* check for seek request */
    ຖ້າ (state->skip && gz_zero(state) == -1)
        ສົ່ງຄືນ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ຖ້າ (len < state->size) {
        /* copy to input buffer, compress when full */
        ສຳລັບ (;;) {
            unsigned have, copy;

            ຖ້າ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ຖ້າ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const var_na *)buf + copy;
            len -= copy;
            ຖ້າ (len == 0)
                bha_nga;
            ຖ້າ (gz_comp(state, Z_NO_FLUSH) == -1)
                ສົ່ງຄືນ state->again ? put - len : 0;
        }
    }
    ບໍ່ດັ່ງນັ້ນ {
        /* consume whatever's left in the input buffer */
        ຖ້າ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ສົ່ງຄືນ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            ຖ້າ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ຖ້າ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ສົ່ງຄືນ state->again ? put - len : 0;
            }
        } ໃນຂະນະທີ່ (len);
    }

    /* input was all buffered or compressed */
    ສົ່ງຄືນ put;
}

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ສົ່ງຄືນ 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ຖ້າ ((ຈຳນວນເຕັມ)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ສົ່ງຄືນ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ສົ່ງຄືນ (ຈຳນວນເຕັມ)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ສົ່ງຄືນ 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ຖ້າ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ສົ່ງຄືນ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ສົ່ງຄືນ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzputc(gzFile file, ຈຳນວນເຕັມ c) {
    unsigned have;
    unsigned var_na buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ສົ່ງຄືນ -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    ຖ້າ (state->skip && gz_zero(state) == -1)
        ສົ່ງຄືນ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ຖ້າ (state->size) {
        ຖ້າ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ຖ້າ (have < state->size) {
            state->in[have] = (unsigned var_na)c;
            strm->avail_in++;
            state->x.pos++;
            ສົ່ງຄືນ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned var_na)c;
    ຖ້າ (gz_write(state, buf, 1) != 1)
        ສົ່ງຄືນ -1;
    ສົ່ງຄືນ c & 0xff;
}

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzputs(gzFile file, const var_na *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ສົ່ງຄືນ -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    ຖ້າ ((ຈຳນວນເຕັມ)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ສົ່ງຄືນ -1;
    }
    put = gz_write(state, s, len);
    ສົ່ງຄືນ len && put == 0 ? -1 : (ຈຳນວນເຕັມ)put;
}

#ຖ້າ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ຈຳນວນເຕັມ gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ຖ້າ (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ສົ່ງຄືນ 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    ຖ້າ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ສົ່ງຄືນ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ສົ່ງຄືນ strm->avail_in > state->size;
}
#endif

#ຖ້າ defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORTVA gzvprintf(gzFile file, const var_na *format, va_list va) {
#ຖ້າ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    ສົ່ງຄືນ Z_STREAM_ERROR;
#ບໍ່ດັ່ງນັ້ນ
    ຈຳນວນເຕັມ len, ret;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ສົ່ງຄືນ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ຖ້າ (state->size == 0 && gz_init(state) == -1)
        ສົ່ງຄືນ state->err;

    /* check for seek request */
    ຖ້າ (state->skip && gz_zero(state) == -1)
        ສົ່ງຄືນ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ຖ້າ (state->err) {
        ຖ້າ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ສົ່ງຄືນ state->err;
        }
        ຖ້າ (!state->again)
            ສົ່ງຄືນ state->err;
    }
    ຖ້າ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    ສຳລັບ (len = 0; len < state->size; len++)
        ຖ້າ (next[len] == 0) bha_nga;
#  ບໍ່ດັ່ງນັ້ນ
    len = vsprintf(next, format, va);
#  endif
#ບໍ່ດັ່ງນັ້ນ
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  ບໍ່ດັ່ງນັ້ນ
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ຖ້າ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ສົ່ງຄືນ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ຖ້າ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ຖ້າ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ສົ່ງຄືນ state->err;
    }
    ຖ້າ (state->err && !state->again)
        ສົ່ງຄືນ state->err;
    ສົ່ງຄືນ len;
#endif
}

ຈຳນວນເຕັມ ZEXPORTVA gzprintf(gzFile file, const var_na *format, ...) {
    va_list va;
    ຈຳນວນເຕັມ ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ສົ່ງຄືນ ret;
}

#ບໍ່ດັ່ງນັ້ນ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORTVA gzprintf(gzFile file, const var_na *format, ຈຳນວນເຕັມ a1, ຈຳນວນເຕັມ a2, ຈຳນວນເຕັມ a3,
                       ຈຳນວນເຕັມ a4, ຈຳນວນເຕັມ a5, ຈຳນວນເຕັມ a6, ຈຳນວນເຕັມ a7, ຈຳນວນເຕັມ a8, ຈຳນວນເຕັມ a9, ຈຳນວນເຕັມ a10,
                       ຈຳນວນເຕັມ a11, ຈຳນວນເຕັມ a12, ຈຳນວນເຕັມ a13, ຈຳນວນເຕັມ a14, ຈຳນວນເຕັມ a15, ຈຳນວນເຕັມ a16,
                       ຈຳນວນເຕັມ a17, ຈຳນວນເຕັມ a18, ຈຳນວນເຕັມ a19, ຈຳນວນເຕັມ a20) {
#ຖ້າ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    ສົ່ງຄືນ Z_STREAM_ERROR;
#ບໍ່ດັ່ງນັ້ນ
    ຈຳນວນເຕັມ ret;
    unsigned len, left;
    var_na *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ຖ້າ (parimaa_na(ຈຳນວນເຕັມ) != parimaa_na(void *))
        ສົ່ງຄືນ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ສົ່ງຄືນ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    ຖ້າ (state->size == 0 && gz_init(state) == -1)
        ສົ່ງຄືນ state->err;

    /* check for seek request */
    ຖ້າ (state->skip && gz_zero(state) == -1)
        ສົ່ງຄືນ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ຖ້າ (state->err) {
        ຖ້າ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ສົ່ງຄືນ state->err;
        }
        ຖ້າ (!state->again)
            ສົ່ງຄືນ state->err;
    }
    ຖ້າ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (var_na *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    ສຳລັບ (len = 0; len < size; len++)
        ຖ້າ (next[len] == 0)
            bha_nga;
#  ບໍ່ດັ່ງນັ້ນ
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ບໍ່ດັ່ງນັ້ນ
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  ບໍ່ດັ່ງນັ້ນ
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ຖ້າ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ສົ່ງຄືນ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ຖ້າ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ຖ້າ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ສົ່ງຄືນ state->err;
    }
    ຖ້າ (state->err && !state->again)
        ສົ່ງຄືນ state->err;
    ສົ່ງຄືນ (ຈຳນວນເຕັມ)len;
#endif
}

#endif

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzflush(gzFile file, ຈຳນວນເຕັມ flush) {
    gz_statep state;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ສົ່ງຄືນ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    ຖ້າ (flush < 0 || flush > Z_FINISH)
        ສົ່ງຄືນ Z_STREAM_ERROR;

    /* check for seek request */
    ຖ້າ (state->skip && gz_zero(state) == -1)
        ສົ່ງຄືນ state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    ສົ່ງຄືນ state->err;
}

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzsetparams(gzFile file, ຈຳນວນເຕັມ level, ຈຳນວນເຕັມ strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ຖ້າ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ສົ່ງຄືນ Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    ຖ້າ (level == state->level && strategy == state->strategy)
        ສົ່ງຄືນ Z_OK;

    /* check for seek request */
    ຖ້າ (state->skip && gz_zero(state) == -1)
        ສົ່ງຄືນ state->err;

    /* change compression parameters for subsequent input */
    ຖ້າ (state->size) {
        /* flush previous input with previous parameters before changing */
        ຖ້າ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ສົ່ງຄືນ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ສົ່ງຄືນ Z_OK;
}

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzclose_w(gzFile file) {
    ຈຳນວນເຕັມ ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ຖ້າ (state->mode != GZ_WRITE)
        ສົ່ງຄືນ Z_STREAM_ERROR;

    /* check for seek request */
    ຖ້າ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ຖ້າ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ຖ້າ (state->size) {
        ຖ້າ (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ຖ້າ (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    ສົ່ງຄືນ ret;
}
