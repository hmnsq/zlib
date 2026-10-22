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
    إذا (state->in == عدم) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        أرجع -1;
    }

    /* only need output buffer and deflate state if compressing */
    إذا (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned حرف *)malloc(state->want);
        إذا (state->out == عدم) {
            حرر(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            أرجع -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        إذا (ret != Z_OK) {
            حرر(state->out);
            حرر(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            أرجع -1;
        }
        strm->next_in = عدم;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    إذا (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    أرجع 0;
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
    إذا (state->size == 0 && gz_init(state) == -1)
        أرجع -1;

    /* write directly if requested */
    إذا (state->direct) {
        طالما (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (صحيح)اكتب(state->fd, strm->next_in, put);
            إذا (writ < 0) {
                إذا (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                أرجع -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        أرجع 0;
    }

    /* check for a pending reset */
    إذا (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        إذا (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            أرجع 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    افعل {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        إذا (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            طالما (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (صحيح)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (صحيح)اكتب(state->fd, state->x.next, put);
                إذا (writ < 0) {
                    إذا (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    أرجع -1;
                }
                state->x.next += writ;
            }
            إذا (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        إذا (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            أرجع -1;
        }
        have -= strm->avail_out;
    } طالما (have);

    /* if that completed a deflate stream, allow another to start */
    إذا (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    أرجع 0;
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
    إذا (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        أرجع -1;

    /* compress state->skip zeros */
    first = 1;
    افعل {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        إذا (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        إذا (ret == -1)
            أرجع -1;
    } طالما (state->skip);
    أرجع 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    صحيح ret;

    /* if len is zero, avoid unnecessary operations */
    إذا (len == 0)
        أرجع 0;

    /* allocate memory if this is the first time through */
    إذا (state->size == 0 && gz_init(state) == -1)
        أرجع 0;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    إذا (len < state->size) {
        /* copy to input buffer, compress when full */
        لكل (;;) {
            unsigned have, copy;

            إذا (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            إذا (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ثابت حرف *)buf + copy;
            len -= copy;
            إذا (len == 0)
                اكسر;
            إذا (gz_comp(state, Z_NO_FLUSH) == -1)
                أرجع state->again ? put - len : 0;
        }
    }
    وإلا {
        /* consume whatever's left in the input buffer */
        إذا (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            أرجع 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        افعل {
            unsigned n = (unsigned)-1;

            إذا (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            إذا (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                أرجع state->again ? put - len : 0;
            }
        } طالما (len);
    }

    /* input was all buffered or compressed */
    أرجع put;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع 0;
    gz_error(state, Z_OK, عدم);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    إذا ((صحيح)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        أرجع 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    أرجع (صحيح)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع 0;
    gz_error(state, Z_OK, عدم);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    إذا (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        أرجع 0;
    }

    /* write len bytes to buf, return the number of full items written */
    أرجع len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzputc(gzFile file, صحيح c) {
    unsigned have;
    unsigned حرف buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع -1;
    gz_error(state, Z_OK, عدم);

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    إذا (state->size) {
        إذا (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        إذا (have < state->size) {
            state->in[have] = (unsigned حرف)c;
            strm->avail_in++;
            state->x.pos++;
            أرجع c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned حرف)c;
    إذا (gz_write(state, buf, 1) != 1)
        أرجع -1;
    أرجع c & 0xff;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzputs(gzFile file, ثابت حرف *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع -1;
    gz_error(state, Z_OK, عدم);

    /* write string */
    len = strlen(s);
    إذا ((صحيح)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        أرجع -1;
    }
    put = gz_write(state, s, len);
    أرجع len && put == 0 ? -1 : (صحيح)put;
}

#إذا (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
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
    إذا (strm->next_in == عدم ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        أرجع 0;
    (فراغ)gz_comp(state, Z_NO_FLUSH);
    إذا (strm->avail_in == 0) {
        strm->next_in = state->in;
        أرجع 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    أرجع strm->avail_in > state->size;
}
#endif

#إذا defined(STDC) || defined(Z_HAVE_STDARG_H)
#ضمن <stdarg.h>

/* -- see zlib.h -- */
صحيح ZEXPORTVA gzvprintf(gzFile file, ثابت حرف *format, va_list va) {
#إذا defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (فراغ)file, (فراغ)format, (فراغ)va;
    أرجع Z_STREAM_ERROR;
#وإلا
    صحيح len, ret;
    حرف *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* make sure we have some buffer space */
    إذا (state->size == 0 && gz_init(state) == -1)
        أرجع state->err;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    إذا (state->err) {
        إذا (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            أرجع state->err;
        }
        إذا (!state->again)
            أرجع state->err;
    }
    إذا (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (حرف *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (فراغ)vsprintf(next, format, va);
    لكل (len = 0; len < state->size; len++)
        إذا (next[len] == 0) اكسر;
#  وإلا
    len = vsprintf(next, format, va);
#  endif
#وإلا
#  ifdef HAS_vsnprintf_void
    (فراغ)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  وإلا
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    إذا (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        أرجع 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    إذا (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        إذا (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        أرجع state->err;
    }
    إذا (state->err && !state->again)
        أرجع state->err;
    أرجع len;
#endif
}

صحيح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, ...) {
    va_list va;
    صحيح ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    أرجع ret;
}

#وإلا /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
صحيح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, صحيح a1, صحيح a2, صحيح a3,
                       صحيح a4, صحيح a5, صحيح a6, صحيح a7, صحيح a8, صحيح a9, صحيح a10,
                       صحيح a11, صحيح a12, صحيح a13, صحيح a14, صحيح a15, صحيح a16,
                       صحيح a17, صحيح a18, صحيح a19, صحيح a20) {
#إذا defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (فراغ)file, (فراغ)format, (فراغ)a1, (فراغ)a2, (فراغ)a3, (فراغ)a4, (فراغ)a5,
    (فراغ)a6, (فراغ)a7, (فراغ)a8, (فراغ)a9, (فراغ)a10, (فراغ)a11, (فراغ)a12,
    (فراغ)a13, (فراغ)a14, (فراغ)a15, (فراغ)a16, (فراغ)a17, (فراغ)a18,
    (فراغ)a19, (فراغ)a20;
    أرجع Z_STREAM_ERROR;
#وإلا
    صحيح ret;
    unsigned len, سول;
    حرف *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    إذا (حجم(صحيح) != حجم(فراغ *))
        أرجع Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* make sure we have some buffer space */
    إذا (state->size == 0 && gz_init(state) == -1)
        أرجع state->err;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    إذا (state->err) {
        إذا (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            أرجع state->err;
        }
        إذا (!state->again)
            أرجع state->err;
    }
    إذا (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (حرف *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    لكل (len = 0; len < size; len++)
        إذا (next[len] == 0)
            اكسر;
#  وإلا
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#وإلا
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  وإلا
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    إذا (len == 0 || len >= state->size || next[state->size - 1] != 0)
        أرجع 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    إذا (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        إذا (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        أرجع state->err;
    }
    إذا (state->err && !state->again)
        أرجع state->err;
    أرجع (صحيح)len;
#endif
}

#endif

/* -- see zlib.h -- */
صحيح ZEXPORT gzflush(gzFile file, صحيح flush) {
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* check flush parameter */
    إذا (flush < 0 || flush > Z_FINISH)
        أرجع Z_STREAM_ERROR;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* compress remaining data with requested flush */
    (فراغ)gz_comp(state, flush);
    أرجع state->err;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzsetparams(gzFile file, صحيح level, صحيح strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    إذا (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        أرجع Z_STREAM_ERROR;
    gz_error(state, Z_OK, عدم);

    /* if no change is requested, then do nothing */
    إذا (level == state->level && strategy == state->strategy)
        أرجع Z_OK;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* change compression parameters for subsequent input */
    إذا (state->size) {
        /* flush previous input with previous parameters before changing */
        إذا (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            أرجع state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    أرجع Z_OK;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzclose_w(gzFile file) {
    صحيح ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    إذا (state->mode != GZ_WRITE)
        أرجع Z_STREAM_ERROR;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    إذا (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    إذا (state->size) {
        إذا (!state->direct) {
            (فراغ)deflateEnd(&(state->strm));
            حرر(state->out);
        }
        حرر(state->in);
    }
    gz_error(state, Z_OK, عدم);
    حرر(state->path);
    إذا (close(state->fd) == -1)
        ret = Z_ERRNO;
    حرر(state);
    أرجع ret;
}
