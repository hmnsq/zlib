/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ଅନ୍ତର୍ଭାଵ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_init(gz_statep state) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned ଅକ୍ଷର *)malloc(state->want << 1);
    ଯଦି (state->in == ଖାଲି) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ଫେରନ୍ତୁ -1;
    }

    /* only need output buffer and deflate state if compressing */
    ଯଦି (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned ଅକ୍ଷର *)malloc(state->want);
        ଯଦି (state->out == ଖାଲି) {
            ମୁକ୍ତ(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ଫେରନ୍ତୁ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ଯଦି (ret != Z_OK) {
            ମୁକ୍ତ(state->out);
            ମୁକ୍ତ(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ଫେରନ୍ତୁ -1;
        }
        strm->next_in = ଖାଲି;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    ଯଦି (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ଫେରନ୍ତୁ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_comp(gz_statep state, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flush) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ଯଦି (state->size == 0 && gz_init(state) == -1)
        ଫେରନ୍ତୁ -1;

    /* write directly if requested */
    ଯଦି (state->direct) {
        ଯେତେବେଳେ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଲିଖ(state->fd, strm->next_in, put);
            ଯଦି (writ < 0) {
                ଯଦି (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ଫେରନ୍ତୁ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ଫେରନ୍ତୁ 0;
    }

    /* check for a pending reset */
    ଯଦି (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ଯଦି (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ଫେରନ୍ତୁ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    କର {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ଯଦି (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ଯେତେବେଳେ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଲିଖ(state->fd, state->x.next, put);
                ଯଦି (writ < 0) {
                    ଯଦି (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ଫେରନ୍ତୁ -1;
                }
                state->x.next += writ;
            }
            ଯଦି (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        ଯଦି (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ଫେରନ୍ତୁ -1;
        }
        have -= strm->avail_out;
    } ଯେତେବେଳେ (have);

    /* if that completed a deflate stream, allow another to start */
    ଯଦି (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ଫେରନ୍ତୁ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_zero(gz_statep state) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ଯଦି (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ଫେରନ୍ତୁ -1;

    /* compress state->skip zeros */
    first = 1;
    କର {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        ଯଦି (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        ଯଦି (ret == -1)
            ଫେରନ୍ତୁ -1;
    } ଯେତେବେଳେ (state->skip);
    ଫେରନ୍ତୁ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

    /* if len is zero, avoid unnecessary operations */
    ଯଦି (len == 0)
        ଫେରନ୍ତୁ 0;

    /* allocate memory if this is the first time through */
    ଯଦି (state->size == 0 && gz_init(state) == -1)
        ଫେରନ୍ତୁ 0;

    /* check for seek request */
    ଯଦି (state->skip && gz_zero(state) == -1)
        ଫେରନ୍ତୁ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ଯଦି (len < state->size) {
        /* copy to input buffer, compress when full */
        ପାଇଁ (;;) {
            unsigned have, copy;

            ଯଦି (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            ଯଦି (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ଅଚଳ ଅକ୍ଷର *)buf + copy;
            len -= copy;
            ଯଦି (len == 0)
                ଭାଙ୍ଗ;
            ଯଦି (gz_comp(state, Z_NO_FLUSH) == -1)
                ଫେରନ୍ତୁ state->again ? put - len : 0;
        }
    }
    ଅନ୍ୟଥା {
        /* consume whatever's left in the input buffer */
        ଯଦି (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ଫେରନ୍ତୁ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        କର {
            unsigned n = (unsigned)-1;

            ଯଦି (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            ଯଦି (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ଫେରନ୍ତୁ state->again ? put - len : 0;
            }
        } ଯେତେବେଳେ (len);
    }

    /* input was all buffered or compressed */
    ଫେରନ୍ତୁ put;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ଫେରନ୍ତୁ 0;
    gz_error(state, Z_OK, ଖାଲି);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ଫେରନ୍ତୁ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ଫେରନ୍ତୁ 0;
    gz_error(state, Z_OK, ଖାଲି);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ଯଦି (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ଫେରନ୍ତୁ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ଫେରନ୍ତୁ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzputc(gzFile file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
    unsigned have;
    unsigned ଅକ୍ଷର buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ଫେରନ୍ତୁ -1;
    gz_error(state, Z_OK, ଖାଲି);

    /* check for seek request */
    ଯଦି (state->skip && gz_zero(state) == -1)
        ଫେରନ୍ତୁ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ଯଦି (state->size) {
        ଯଦି (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        ଯଦି (have < state->size) {
            state->in[have] = (unsigned ଅକ୍ଷର)c;
            strm->avail_in++;
            state->x.pos++;
            ଫେରନ୍ତୁ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned ଅକ୍ଷର)c;
    ଯଦି (gz_write(state, buf, 1) != 1)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ c & 0xff;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzputs(gzFile file, ଅଚଳ ଅକ୍ଷର *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ଫେରନ୍ତୁ -1;
    gz_error(state, Z_OK, ଖାଲି);

    /* write string */
    len = strlen(s);
    ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ଫେରନ୍ତୁ -1;
    }
    put = gz_write(state, s, len);
    ଫେରନ୍ତୁ len && put == 0 ? -1 : (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)put;
}

#ଯଦି (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ଯଦି (strm->next_in == ଖାଲି ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ଫେରନ୍ତୁ 0;
    (ଶୂନ୍ୟ)gz_comp(state, Z_NO_FLUSH);
    ଯଦି (strm->avail_in == 0) {
        strm->next_in = state->in;
        ଫେରନ୍ତୁ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ଫେରନ୍ତୁ strm->avail_in > state->size;
}
#endif

#ଯଦି defined(STDC) || defined(Z_HAVE_STDARG_H)
#ଅନ୍ତର୍ଭାଵ <stdarg.h>

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORTVA gzvprintf(gzFile file, ଅଚଳ ଅକ୍ଷର *format, va_list va) {
#ଯଦି defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (ଶୂନ୍ୟ)file, (ଶୂନ୍ୟ)format, (ଶୂନ୍ୟ)va;
    ଫେରନ୍ତୁ Z_STREAM_ERROR;
#ଅନ୍ୟଥା
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, ret;
    ଅକ୍ଷର *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ଖାଲି);

    /* make sure we have some buffer space */
    ଯଦି (state->size == 0 && gz_init(state) == -1)
        ଫେରନ୍ତୁ state->err;

    /* check for seek request */
    ଯଦି (state->skip && gz_zero(state) == -1)
        ଫେରନ୍ତୁ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ଯଦି (state->err) {
        ଯଦି (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ଫେରନ୍ତୁ state->err;
        }
        ଯଦି (!state->again)
            ଫେରନ୍ତୁ state->err;
    }
    ଯଦି (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (ଅକ୍ଷର *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (ଶୂନ୍ୟ)vsprintf(next, format, va);
    ପାଇଁ (len = 0; len < state->size; len++)
        ଯଦି (next[len] == 0) ଭାଙ୍ଗ;
#  ଅନ୍ୟଥା
    len = vsprintf(next, format, va);
#  endif
#ଅନ୍ୟଥା
#  ifdef HAS_vsnprintf_void
    (ଶୂନ୍ୟ)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  ଅନ୍ୟଥା
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ଯଦି (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ଫେରନ୍ତୁ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ଯଦି (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ଯଦି (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ଫେରନ୍ତୁ state->err;
    }
    ଯଦି (state->err && !state->again)
        ଫେରନ୍ତୁ state->err;
    ଫେରନ୍ତୁ len;
#endif
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORTVA gzprintf(gzFile file, ଅଚଳ ଅକ୍ଷର *format, ...) {
    va_list va;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ଫେରନ୍ତୁ ret;
}

#ଅନ୍ୟଥା /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORTVA gzprintf(gzFile file, ଅଚଳ ଅକ୍ଷର *format, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a3,
                       ପୂର୍ଣ୍ଣସଂଖ୍ୟା a4, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a5, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a6, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a7, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a8, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a9, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a10,
                       ପୂର୍ଣ୍ଣସଂଖ୍ୟା a11, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a12, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a13, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a14, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a15, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a16,
                       ପୂର୍ଣ୍ଣସଂଖ୍ୟା a17, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a18, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a19, ପୂର୍ଣ୍ଣସଂଖ୍ୟା a20) {
#ଯଦି defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (ଶୂନ୍ୟ)file, (ଶୂନ୍ୟ)format, (ଶୂନ୍ୟ)a1, (ଶୂନ୍ୟ)a2, (ଶୂନ୍ୟ)a3, (ଶୂନ୍ୟ)a4, (ଶୂନ୍ୟ)a5,
    (ଶୂନ୍ୟ)a6, (ଶୂନ୍ୟ)a7, (ଶୂନ୍ୟ)a8, (ଶୂନ୍ୟ)a9, (ଶୂନ୍ୟ)a10, (ଶୂନ୍ୟ)a11, (ଶୂନ୍ୟ)a12,
    (ଶୂନ୍ୟ)a13, (ଶୂନ୍ୟ)a14, (ଶୂନ୍ୟ)a15, (ଶୂନ୍ୟ)a16, (ଶୂନ୍ୟ)a17, (ଶୂନ୍ୟ)a18,
    (ଶୂନ୍ୟ)a19, (ଶୂନ୍ୟ)a20;
    ଫେରନ୍ତୁ Z_STREAM_ERROR;
#ଅନ୍ୟଥା
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    unsigned len, ବାମ;
    ଅକ୍ଷର *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ଯଦି (ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) != ଆକାର(ଶୂନ୍ୟ *))
        ଫେରନ୍ତୁ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ଖାଲି);

    /* make sure we have some buffer space */
    ଯଦି (state->size == 0 && gz_init(state) == -1)
        ଫେରନ୍ତୁ state->err;

    /* check for seek request */
    ଯଦି (state->skip && gz_zero(state) == -1)
        ଫେରନ୍ତୁ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ଯଦି (state->err) {
        ଯଦି (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ଫେରନ୍ତୁ state->err;
        }
        ଯଦି (!state->again)
            ଫେରନ୍ତୁ state->err;
    }
    ଯଦି (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (ଅକ୍ଷର *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    ପାଇଁ (len = 0; len < size; len++)
        ଯଦି (next[len] == 0)
            ଭାଙ୍ଗ;
#  ଅନ୍ୟଥା
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ଅନ୍ୟଥା
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  ଅନ୍ୟଥା
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    ଯଦି (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ଫେରନ୍ତୁ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ଯଦି (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        ଯଦି (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ଫେରନ୍ତୁ state->err;
    }
    ଯଦି (state->err && !state->again)
        ଫେରନ୍ତୁ state->err;
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len;
#endif
}

#endif

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzflush(gzFile file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flush) {
    gz_statep state;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ଖାଲି);

    /* check flush parameter */
    ଯଦି (flush < 0 || flush > Z_FINISH)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;

    /* check for seek request */
    ଯଦି (state->skip && gz_zero(state) == -1)
        ଫେରନ୍ତୁ state->err;

    /* compress remaining data with requested flush */
    (ଶୂନ୍ୟ)gz_comp(state, flush);
    ଫେରନ୍ତୁ state->err;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzsetparams(gzFile file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ଯଦି (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ଖାଲି);

    /* if no change is requested, then do nothing */
    ଯଦି (level == state->level && strategy == state->strategy)
        ଫେରନ୍ତୁ Z_OK;

    /* check for seek request */
    ଯଦି (state->skip && gz_zero(state) == -1)
        ଫେରନ୍ତୁ state->err;

    /* change compression parameters for subsequent input */
    ଯଦି (state->size) {
        /* flush previous input with previous parameters before changing */
        ଯଦି (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ଫେରନ୍ତୁ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ଫେରନ୍ତୁ Z_OK;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzclose_w(gzFile file) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ଯଦି (state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;

    /* check for seek request */
    ଯଦି (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ଯଦି (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    ଯଦି (state->size) {
        ଯଦି (!state->direct) {
            (ଶୂନ୍ୟ)deflateEnd(&(state->strm));
            ମୁକ୍ତ(state->out);
        }
        ମୁକ୍ତ(state->in);
    }
    gz_error(state, Z_OK, ଖାଲି);
    ମୁକ୍ତ(state->path);
    ଯଦି (close(state->fd) == -1)
        ret = Z_ERRNO;
    ମୁକ୍ତ(state);
    ଫେରନ୍ତୁ ret;
}
