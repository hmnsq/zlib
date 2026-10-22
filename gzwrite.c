/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local صحيح gz_init(gz_statep state) {
    صحيح ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned حرف *)malloc(state->want << 1);
    ܐܢ (state->in == عدم) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ܦܢܐ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ܐܢ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned حرف *)malloc(state->want);
        ܐܢ (state->out == عدم) {
            حرر(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ܦܢܐ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ܐܢ (ret != Z_OK) {
            حرر(state->out);
            حرر(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ܦܢܐ -1;
        }
        strm->next_in = عدم;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ܐܢ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ܦܢܐ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local صحيح gz_comp(gz_statep state, صحيح flush) {
    صحيح ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ܐܢ (state->size == 0 && gz_init(state) == -1)
        ܦܢܐ -1;

    /* write directly if requested */
    ܐܢ (state->direct) {
        ܟܡܐ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (صحيح)اكتب(state->fd, strm->next_in, put);
            ܐܢ (writ < 0) {
                ܐܢ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ܦܢܐ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ܦܢܐ 0;
    }

    /* check for a pending reset */
    ܐܢ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ܐܢ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ܦܢܐ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    ܥܒܕ {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ܐܢ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ܟܡܐ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (صحيح)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (صحيح)اكتب(state->fd, state->x.next, put);
                ܐܢ (writ < 0) {
                    ܐܢ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ܦܢܐ -1;
                }
                state->x.next += writ;
            }
            ܐܢ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ܐܢ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ܦܢܐ -1;
        }
        have -= strm->avail_out;
    } ܟܡܐ (have);

    /* if that completed a deflate stream, allow another to start */
    ܐܢ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ܦܢܐ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local صحيح gz_zero(gz_statep state) {
    صحيح first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ܐܢ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ܦܢܐ -1;

    /* compress state->skip zeros */
    first = 1;
    ܥܒܕ {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ܐܢ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ܐܢ (ret == -1)
            ܦܢܐ -1;
    } ܟܡܐ (state->skip);
    ܦܢܐ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    صحيح ret;

    /* if len is zero, avoid unnecessary operations */
    ܐܢ (len == 0)
        ܦܢܐ 0;

    /* allocate memory if this is the first time through */
    ܐܢ (state->size == 0 && gz_init(state) == -1)
        ܦܢܐ 0;

    /* check for seek request */
    ܐܢ (state->skip && gz_zero(state) == -1)
        ܦܢܐ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ܐܢ (len < state->size) {
        /* copy to input buffer, compress when full */
        لكل (;;) {
            unsigned have, copy;

            ܐܢ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ܐܢ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ثابت حرف *)buf + copy;
            len -= copy;
            ܐܢ (len == 0)
                اكسر;
            ܐܢ (gz_comp(state, Z_NO_FLUSH) == -1)
                ܦܢܐ state->again ? put - len : 0;
        }
    }
    ܐܚܪܢܐ {
        /* consume whatever's left in the input buffer */
        ܐܢ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ܦܢܐ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        ܥܒܕ {
            unsigned n = (unsigned)-1;

            ܐܢ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ܐܢ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ܦܢܐ state->again ? put - len : 0;
            }
        } ܟܡܐ (len);
    }

    /* input was all buffered or compressed */
    ܦܢܐ put;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ܦܢܐ 0;
    gz_error(state, Z_OK, عدم);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ܐܢ ((صحيح)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ܦܢܐ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ܦܢܐ (صحيح)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ܦܢܐ 0;
    gz_error(state, Z_OK, عدم);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ܐܢ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ܦܢܐ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ܦܢܐ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzputc(gzFile file, صحيح c) {
    unsigned have;
    unsigned حرف buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ܦܢܐ -1;
    gz_error(state, Z_OK, عدم);

    /* check for seek request */
    ܐܢ (state->skip && gz_zero(state) == -1)
        ܦܢܐ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ܐܢ (state->size) {
        ܐܢ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ܐܢ (have < state->size) {
            state->in[have] = (unsigned حرف)c;
            strm->avail_in++;
            state->x.pos++;
            ܦܢܐ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned حرف)c;
    ܐܢ (gz_write(state, buf, 1) != 1)
        ܦܢܐ -1;
    ܦܢܐ c & 0xff;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzputs(gzFile file, ثابت حرف *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ܦܢܐ -1;
    gz_error(state, Z_OK, عدم);

    /* write string */
    len = strlen(s);
    ܐܢ ((صحيح)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ܦܢܐ -1;
    }
    put = gz_write(state, s, len);
    ܦܢܐ len && put == 0 ? -1 : (صحيح)put;
}

#ܐܢ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local صحيح gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ܐܢ (strm->next_in == عدم ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ܦܢܐ 0;
    (فراغ)gz_comp(state, Z_NO_FLUSH);
    ܐܢ (strm->avail_in == 0) {
        strm->next_in = state->in;
        ܦܢܐ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ܦܢܐ strm->avail_in > state->size;
}
#endif

#ܐܢ defined(STDC) || defined(Z_HAVE_STDARG_H)
#ضمن <stdarg.h>

/* -- see zlib.h -- */
صحيح ZEXPORTVA gzvprintf(gzFile file, ثابت حرف *format, va_list va) {
#ܐܢ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (فراغ)file, (فراغ)format, (فراغ)va;
    ܦܢܐ Z_STREAM_ERROR;
#ܐܚܪܢܐ
    صحيح len, ret;
    حرف *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ܦܢܐ Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* make sure we have some buffer space */
    ܐܢ (state->size == 0 && gz_init(state) == -1)
        ܦܢܐ state->err;

    /* check for seek request */
    ܐܢ (state->skip && gz_zero(state) == -1)
        ܦܢܐ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ܐܢ (state->err) {
        ܐܢ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ܦܢܐ state->err;
        }
        ܐܢ (!state->again)
            ܦܢܐ state->err;
    }
    ܐܢ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (حرف *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (فراغ)vsprintf(next, format, va);
    لكل (len = 0; len < state->size; len++)
        ܐܢ (next[len] == 0) اكسر;
#  ܐܚܪܢܐ
    len = vsprintf(next, format, va);
#  endif
#ܐܚܪܢܐ
#  ifdef HAS_vsnprintf_void
    (فراغ)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  ܐܚܪܢܐ
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ܐܢ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ܦܢܐ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ܐܢ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ܐܢ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ܦܢܐ state->err;
    }
    ܐܢ (state->err && !state->again)
        ܦܢܐ state->err;
    ܦܢܐ len;
#endif
}

صحيح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, ...) {
    va_list va;
    صحيح ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ܦܢܐ ret;
}

#ܐܚܪܢܐ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
صحيح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, صحيح a1, صحيح a2, صحيح a3,
                       صحيح a4, صحيح a5, صحيح a6, صحيح a7, صحيح a8, صحيح a9, صحيح a10,
                       صحيح a11, صحيح a12, صحيح a13, صحيح a14, صحيح a15, صحيح a16,
                       صحيح a17, صحيح a18, صحيح a19, صحيح a20) {
#ܐܢ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (فراغ)file, (فراغ)format, (فراغ)a1, (فراغ)a2, (فراغ)a3, (فراغ)a4, (فراغ)a5,
    (فراغ)a6, (فراغ)a7, (فراغ)a8, (فراغ)a9, (فراغ)a10, (فراغ)a11, (فراغ)a12,
    (فراغ)a13, (فراغ)a14, (فراغ)a15, (فراغ)a16, (فراغ)a17, (فراغ)a18,
    (فراغ)a19, (فراغ)a20;
    ܦܢܐ Z_STREAM_ERROR;
#ܐܚܪܢܐ
    صحيح ret;
    unsigned len, left;
    حرف *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ܐܢ (حجم(صحيح) != حجم(فراغ *))
        ܦܢܐ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ܦܢܐ Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* make sure we have some buffer space */
    ܐܢ (state->size == 0 && gz_init(state) == -1)
        ܦܢܐ state->err;

    /* check for seek request */
    ܐܢ (state->skip && gz_zero(state) == -1)
        ܦܢܐ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ܐܢ (state->err) {
        ܐܢ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ܦܢܐ state->err;
        }
        ܐܢ (!state->again)
            ܦܢܐ state->err;
    }
    ܐܢ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (حرف *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    لكل (len = 0; len < size; len++)
        ܐܢ (next[len] == 0)
            اكسر;
#  ܐܚܪܢܐ
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ܐܚܪܢܐ
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  ܐܚܪܢܐ
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ܐܢ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ܦܢܐ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ܐܢ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ܐܢ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ܦܢܐ state->err;
    }
    ܐܢ (state->err && !state->again)
        ܦܢܐ state->err;
    ܦܢܐ (صحيح)len;
#endif
}

#endif

/* -- see zlib.h -- */
صحيح ZEXPORT gzflush(gzFile file, صحيح flush) {
    gz_statep state;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ܦܢܐ Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* check flush parameter */
    ܐܢ (flush < 0 || flush > Z_FINISH)
        ܦܢܐ Z_STREAM_ERROR;

    /* check for seek request */
    ܐܢ (state->skip && gz_zero(state) == -1)
        ܦܢܐ state->err;

    /* compress remaining data with requested flush */
    (فراغ)gz_comp(state, flush);
    ܦܢܐ state->err;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzsetparams(gzFile file, صحيح level, صحيح strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ܐܢ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ܦܢܐ Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* if no change is requested, then do nothing */
    ܐܢ (level == state->level && strategy == state->strategy)
        ܦܢܐ Z_OK;

    /* check for seek request */
    ܐܢ (state->skip && gz_zero(state) == -1)
        ܦܢܐ state->err;

    /* change compression parameters for subsequent input */
    ܐܢ (state->size) {
        /* flush previous input with previous parameters before changing */
        ܐܢ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ܦܢܐ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ܦܢܐ Z_OK;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzclose_w(gzFile file) {
    صحيح ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ܐܢ (state->mode != GZ_WRITE)
        ܦܢܐ Z_STREAM_ERROR;

    /* check for seek request */
    ܐܢ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ܐܢ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ܐܢ (state->size) {
        ܐܢ (!state->direct) {
            (فراغ)deflateEnd(&(state->strm));
            حرر(state->out);
        }
        حرر(state->in);
    }
    gz_error(state, Z_OK, عدم);
    حرر(state->path);
    ܐܢ (close(state->fd) == -1)
        ret = Z_ERRNO;
    حرر(state);
    ܦܢܐ ret;
}
