/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local صحیح gz_init(gz_statep state) {
    صحیح ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned نویسه *)malloc(state->want << 1);
    اگر (state->in == پوچ) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        بازگشت -1;
    }

    /* only need output buffer and deflate state if compressing */
    اگر (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned نویسه *)malloc(state->want);
        اگر (state->out == پوچ) {
            آزاد(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            بازگشت -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        اگر (ret != Z_OK) {
            آزاد(state->out);
            آزاد(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            بازگشت -1;
        }
        strm->next_in = پوچ;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    اگر (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    بازگشت 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local صحیح gz_comp(gz_statep state, صحیح flush) {
    صحیح ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    اگر (state->size == 0 && gz_init(state) == -1)
        بازگشت -1;

    /* write directly if requested */
    اگر (state->direct) {
        وقتی (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (صحیح)اكتب(state->fd, strm->next_in, put);
            اگر (writ < 0) {
                اگر (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                بازگشت -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        بازگشت 0;
    }

    /* check for a pending reset */
    اگر (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        اگر (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            بازگشت 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    انجامبده {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        اگر (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            وقتی (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (صحیح)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (صحیح)اكتب(state->fd, state->x.next, put);
                اگر (writ < 0) {
                    اگر (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    بازگشت -1;
                }
                state->x.next += writ;
            }
            اگر (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        اگر (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            بازگشت -1;
        }
        have -= strm->avail_out;
    } وقتی (have);

    /* if that completed a deflate stream, allow another to start */
    اگر (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    بازگشت 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local صحیح gz_zero(gz_statep state) {
    صحیح first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    اگر (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        بازگشت -1;

    /* compress state->skip zeros */
    first = 1;
    انجامبده {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        اگر (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        اگر (ret == -1)
            بازگشت -1;
    } وقتی (state->skip);
    بازگشت 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    صحیح ret;

    /* if len is zero, avoid unnecessary operations */
    اگر (len == 0)
        بازگشت 0;

    /* allocate memory if this is the first time through */
    اگر (state->size == 0 && gz_init(state) == -1)
        بازگشت 0;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        بازگشت 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    اگر (len < state->size) {
        /* copy to input buffer, compress when full */
        برای (;;) {
            unsigned have, copy;

            اگر (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            اگر (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ثابت نویسه *)buf + copy;
            len -= copy;
            اگر (len == 0)
                بشکن;
            اگر (gz_comp(state, Z_NO_FLUSH) == -1)
                بازگشت state->again ? put - len : 0;
        }
    }
    وگرنه {
        /* consume whatever's left in the input buffer */
        اگر (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            بازگشت 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        انجامبده {
            unsigned n = (unsigned)-1;

            اگر (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            اگر (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                بازگشت state->again ? put - len : 0;
            }
        } وقتی (len);
    }

    /* input was all buffered or compressed */
    بازگشت put;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        بازگشت 0;
    gz_error(state, Z_OK, پوچ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    اگر ((صحیح)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        بازگشت 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    بازگشت (صحیح)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        بازگشت 0;
    gz_error(state, Z_OK, پوچ);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    اگر (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        بازگشت 0;
    }

    /* write len bytes to buf, return the number of full items written */
    بازگشت len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzputc(gzFile file, صحیح c) {
    unsigned have;
    unsigned نویسه buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        بازگشت -1;
    gz_error(state, Z_OK, پوچ);

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        بازگشت -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    اگر (state->size) {
        اگر (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        اگر (have < state->size) {
            state->in[have] = (unsigned نویسه)c;
            strm->avail_in++;
            state->x.pos++;
            بازگشت c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned نویسه)c;
    اگر (gz_write(state, buf, 1) != 1)
        بازگشت -1;
    بازگشت c & 0xff;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzputs(gzFile file, ثابت نویسه *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        بازگشت -1;
    gz_error(state, Z_OK, پوچ);

    /* write string */
    len = strlen(s);
    اگر ((صحیح)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        بازگشت -1;
    }
    put = gz_write(state, s, len);
    بازگشت len && put == 0 ? -1 : (صحیح)put;
}

#اگر (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local صحیح gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    اگر (strm->next_in == پوچ ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        بازگشت 0;
    (تهی)gz_comp(state, Z_NO_FLUSH);
    اگر (strm->avail_in == 0) {
        strm->next_in = state->in;
        بازگشت 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    بازگشت strm->avail_in > state->size;
}
#endif

#اگر defined(STDC) || defined(Z_HAVE_STDARG_H)
#شامل <stdarg.h>

/* -- see zlib.h -- */
صحیح ZEXPORTVA gzvprintf(gzFile file, ثابت نویسه *format, va_list va) {
#اگر defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (تهی)file, (تهی)format, (تهی)va;
    بازگشت Z_STREAM_ERROR;
#وگرنه
    صحیح len, ret;
    نویسه *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        بازگشت Z_STREAM_ERROR;
    gz_error(state, Z_OK, پوچ);

    /* make sure we have some buffer space */
    اگر (state->size == 0 && gz_init(state) == -1)
        بازگشت state->err;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        بازگشت state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    اگر (state->err) {
        اگر (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            بازگشت state->err;
        }
        اگر (!state->again)
            بازگشت state->err;
    }
    اگر (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (نویسه *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (تهی)vsprintf(next, format, va);
    برای (len = 0; len < state->size; len++)
        اگر (next[len] == 0) بشکن;
#  وگرنه
    len = vsprintf(next, format, va);
#  endif
#وگرنه
#  ifdef HAS_vsnprintf_void
    (تهی)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  وگرنه
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    اگر (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        بازگشت 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    اگر (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        اگر (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        بازگشت state->err;
    }
    اگر (state->err && !state->again)
        بازگشت state->err;
    بازگشت len;
#endif
}

صحیح ZEXPORTVA gzprintf(gzFile file, ثابت نویسه *format, ...) {
    va_list va;
    صحیح ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    بازگشت ret;
}

#وگرنه /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
صحیح ZEXPORTVA gzprintf(gzFile file, ثابت نویسه *format, صحیح a1, صحیح a2, صحیح a3,
                       صحیح a4, صحیح a5, صحیح a6, صحیح a7, صحیح a8, صحیح a9, صحیح a10,
                       صحیح a11, صحیح a12, صحیح a13, صحیح a14, صحیح a15, صحیح a16,
                       صحیح a17, صحیح a18, صحیح a19, صحیح a20) {
#اگر defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (تهی)file, (تهی)format, (تهی)a1, (تهی)a2, (تهی)a3, (تهی)a4, (تهی)a5,
    (تهی)a6, (تهی)a7, (تهی)a8, (تهی)a9, (تهی)a10, (تهی)a11, (تهی)a12,
    (تهی)a13, (تهی)a14, (تهی)a15, (تهی)a16, (تهی)a17, (تهی)a18,
    (تهی)a19, (تهی)a20;
    بازگشت Z_STREAM_ERROR;
#وگرنه
    صحیح ret;
    unsigned len, چپ;
    نویسه *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    اگر (اندازه(صحیح) != اندازه(تهی *))
        بازگشت Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        بازگشت Z_STREAM_ERROR;
    gz_error(state, Z_OK, پوچ);

    /* make sure we have some buffer space */
    اگر (state->size == 0 && gz_init(state) == -1)
        بازگشت state->err;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        بازگشت state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    اگر (state->err) {
        اگر (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            بازگشت state->err;
        }
        اگر (!state->again)
            بازگشت state->err;
    }
    اگر (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (نویسه *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    برای (len = 0; len < size; len++)
        اگر (next[len] == 0)
            بشکن;
#  وگرنه
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#وگرنه
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  وگرنه
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    اگر (len == 0 || len >= state->size || next[state->size - 1] != 0)
        بازگشت 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    اگر (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        اگر (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        بازگشت state->err;
    }
    اگر (state->err && !state->again)
        بازگشت state->err;
    بازگشت (صحیح)len;
#endif
}

#endif

/* -- see zlib.h -- */
صحیح ZEXPORT gzflush(gzFile file, صحیح flush) {
    gz_statep state;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        بازگشت Z_STREAM_ERROR;
    gz_error(state, Z_OK, پوچ);

    /* check flush parameter */
    اگر (flush < 0 || flush > Z_FINISH)
        بازگشت Z_STREAM_ERROR;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        بازگشت state->err;

    /* compress remaining data with requested flush */
    (تهی)gz_comp(state, flush);
    بازگشت state->err;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzsetparams(gzFile file, صحیح level, صحیح strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    اگر (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        بازگشت Z_STREAM_ERROR;
    gz_error(state, Z_OK, پوچ);

    /* if no change is requested, then do nothing */
    اگر (level == state->level && strategy == state->strategy)
        بازگشت Z_OK;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        بازگشت state->err;

    /* change compression parameters for subsequent input */
    اگر (state->size) {
        /* flush previous input with previous parameters before changing */
        اگر (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            بازگشت state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    بازگشت Z_OK;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzclose_w(gzFile file) {
    صحیح ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    اگر (state->mode != GZ_WRITE)
        بازگشت Z_STREAM_ERROR;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    اگر (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    اگر (state->size) {
        اگر (!state->direct) {
            (تهی)deflateEnd(&(state->strm));
            آزاد(state->out);
        }
        آزاد(state->in);
    }
    gz_error(state, Z_OK, پوچ);
    آزاد(state->path);
    اگر (close(state->fd) == -1)
        ret = Z_ERRNO;
    آزاد(state);
    بازگشت ret;
}
