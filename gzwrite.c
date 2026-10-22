/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#অন্তর্ভাৱ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local পূর্ণাঙ্ক gz_init(gz_statep state) {
    পূর্ণাঙ্ক ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned ৱর্ণ *)malloc(state->want << 1);
    যদি (state->in == রিক্ত) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ঘূৰাই -1;
    }

    /* only need output buffer and deflate state if compressing */
    যদি (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned ৱর্ণ *)malloc(state->want);
        যদি (state->out == রিক্ত) {
            মুক্ত(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ঘূৰাই -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        যদি (ret != Z_OK) {
            মুক্ত(state->out);
            মুক্ত(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ঘূৰাই -1;
        }
        strm->next_in = রিক্ত;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    যদি (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    ঘূৰাই 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local পূর্ণাঙ্ক gz_comp(gz_statep state, পূর্ণাঙ্ক flush) {
    পূর্ণাঙ্ক ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    যদি (state->size == 0 && gz_init(state) == -1)
        ঘূৰাই -1;

    /* write directly if requested */
    যদি (state->direct) {
        যেতিয়ালৈ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (পূর্ণাঙ্ক)লিখ(state->fd, strm->next_in, put);
            যদি (writ < 0) {
                যদি (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ঘূৰাই -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        ঘূৰাই 0;
    }

    /* check for a pending reset */
    যদি (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        যদি (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ঘূৰাই 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    কৰা {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        যদি (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            যেতিয়ালৈ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (পূর্ণাঙ্ক)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (পূর্ণাঙ্ক)লিখ(state->fd, state->x.next, put);
                যদি (writ < 0) {
                    যদি (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ঘূৰাই -1;
                }
                state->x.next += writ;
            }
            যদি (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        যদি (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ঘূৰাই -1;
        }
        have -= strm->avail_out;
    } যেতিয়ালৈ (have);

    /* if that completed a deflate stream, allow another to start */
    যদি (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ঘূৰাই 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local পূর্ণাঙ্ক gz_zero(gz_statep state) {
    পূর্ণাঙ্ক first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    যদি (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ঘূৰাই -1;

    /* compress state->skip zeros */
    first = 1;
    কৰা {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        যদি (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        যদি (ret == -1)
            ঘূৰাই -1;
    } যেতিয়ালৈ (state->skip);
    ঘূৰাই 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    পূর্ণাঙ্ক ret;

    /* if len is zero, avoid unnecessary operations */
    যদি (len == 0)
        ঘূৰাই 0;

    /* allocate memory if this is the first time through */
    যদি (state->size == 0 && gz_init(state) == -1)
        ঘূৰাই 0;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ঘূৰাই 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    যদি (len < state->size) {
        /* copy to input buffer, compress when full */
        বাবে (;;) {
            unsigned have, copy;

            যদি (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            যদি (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ধ্রুৱ ৱর্ণ *)buf + copy;
            len -= copy;
            যদি (len == 0)
                ভঙ্গ;
            যদি (gz_comp(state, Z_NO_FLUSH) == -1)
                ঘূৰাই state->again ? put - len : 0;
        }
    }
    অন্যথা {
        /* consume whatever's left in the input buffer */
        যদি (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ঘূৰাই 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        কৰা {
            unsigned n = (unsigned)-1;

            যদি (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            যদি (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                ঘূৰাই state->again ? put - len : 0;
            }
        } যেতিয়ালৈ (len);
    }

    /* input was all buffered or compressed */
    ঘূৰাই put;
}

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ঘূৰাই 0;
    gz_error(state, Z_OK, রিক্ত);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    যদি ((পূর্ণাঙ্ক)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        ঘূৰাই 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ঘূৰাই (পূর্ণাঙ্ক)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ঘূৰাই 0;
    gz_error(state, Z_OK, রিক্ত);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    যদি (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ঘূৰাই 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ঘূৰাই len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzputc(gzFile file, পূর্ণাঙ্ক c) {
    unsigned have;
    unsigned ৱর্ণ buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ঘূৰাই -1;
    gz_error(state, Z_OK, রিক্ত);

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ঘূৰাই -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    যদি (state->size) {
        যদি (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        যদি (have < state->size) {
            state->in[have] = (unsigned ৱর্ণ)c;
            strm->avail_in++;
            state->x.pos++;
            ঘূৰাই c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned ৱর্ণ)c;
    যদি (gz_write(state, buf, 1) != 1)
        ঘূৰাই -1;
    ঘূৰাই c & 0xff;
}

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzputs(gzFile file, ধ্রুৱ ৱর্ণ *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ঘূৰাই -1;
    gz_error(state, Z_OK, রিক্ত);

    /* write string */
    len = strlen(s);
    যদি ((পূর্ণাঙ্ক)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ঘূৰাই -1;
    }
    put = gz_write(state, s, len);
    ঘূৰাই len && put == 0 ? -1 : (পূর্ণাঙ্ক)put;
}

#যদি (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local পূর্ণাঙ্ক gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    যদি (strm->next_in == রিক্ত ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        ঘূৰাই 0;
    (শূন্য)gz_comp(state, Z_NO_FLUSH);
    যদি (strm->avail_in == 0) {
        strm->next_in = state->in;
        ঘূৰাই 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    ঘূৰাই strm->avail_in > state->size;
}
#endif

#যদি defined(STDC) || defined(Z_HAVE_STDARG_H)
#অন্তর্ভাৱ <stdarg.h>

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORTVA gzvprintf(gzFile file, ধ্রুৱ ৱর্ণ *format, va_list va) {
#যদি defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (শূন্য)file, (শূন্য)format, (শূন্য)va;
    ঘূৰাই Z_STREAM_ERROR;
#অন্যথা
    পূর্ণাঙ্ক len, ret;
    ৱর্ণ *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ঘূৰাই Z_STREAM_ERROR;
    gz_error(state, Z_OK, রিক্ত);

    /* make sure we have some buffer space */
    যদি (state->size == 0 && gz_init(state) == -1)
        ঘূৰাই state->err;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ঘূৰাই state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    যদি (state->err) {
        যদি (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ঘূৰাই state->err;
        }
        যদি (!state->again)
            ঘূৰাই state->err;
    }
    যদি (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (ৱর্ণ *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (শূন্য)vsprintf(next, format, va);
    বাবে (len = 0; len < state->size; len++)
        যদি (next[len] == 0) ভঙ্গ;
#  অন্যথা
    len = vsprintf(next, format, va);
#  endif
#অন্যথা
#  ifdef HAS_vsnprintf_void
    (শূন্য)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  অন্যথা
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    যদি (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        ঘূৰাই 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    যদি (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        যদি (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ঘূৰাই state->err;
    }
    যদি (state->err && !state->again)
        ঘূৰাই state->err;
    ঘূৰাই len;
#endif
}

পূর্ণাঙ্ক ZEXPORTVA gzprintf(gzFile file, ধ্রুৱ ৱর্ণ *format, ...) {
    va_list va;
    পূর্ণাঙ্ক ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ঘূৰাই ret;
}

#অন্যথা /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORTVA gzprintf(gzFile file, ধ্রুৱ ৱর্ণ *format, পূর্ণাঙ্ক a1, পূর্ণাঙ্ক a2, পূর্ণাঙ্ক a3,
                       পূর্ণাঙ্ক a4, পূর্ণাঙ্ক a5, পূর্ণাঙ্ক a6, পূর্ণাঙ্ক a7, পূর্ণাঙ্ক a8, পূর্ণাঙ্ক a9, পূর্ণাঙ্ক a10,
                       পূর্ণাঙ্ক a11, পূর্ণাঙ্ক a12, পূর্ণাঙ্ক a13, পূর্ণাঙ্ক a14, পূর্ণাঙ্ক a15, পূর্ণাঙ্ক a16,
                       পূর্ণাঙ্ক a17, পূর্ণাঙ্ক a18, পূর্ণাঙ্ক a19, পূর্ণাঙ্ক a20) {
#যদি defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (শূন্য)file, (শূন্য)format, (শূন্য)a1, (শূন্য)a2, (শূন্য)a3, (শূন্য)a4, (শূন্য)a5,
    (শূন্য)a6, (শূন্য)a7, (শূন্য)a8, (শূন্য)a9, (শূন্য)a10, (শূন্য)a11, (শূন্য)a12,
    (শূন্য)a13, (শূন্য)a14, (শূন্য)a15, (শূন্য)a16, (শূন্য)a17, (শূন্য)a18,
    (শূন্য)a19, (শূন্য)a20;
    ঘূৰাই Z_STREAM_ERROR;
#অন্যথা
    পূর্ণাঙ্ক ret;
    unsigned len, বাম;
    ৱর্ণ *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    যদি (পরিমাণ(পূর্ণাঙ্ক) != পরিমাণ(শূন্য *))
        ঘূৰাই Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ঘূৰাই Z_STREAM_ERROR;
    gz_error(state, Z_OK, রিক্ত);

    /* make sure we have some buffer space */
    যদি (state->size == 0 && gz_init(state) == -1)
        ঘূৰাই state->err;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ঘূৰাই state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    যদি (state->err) {
        যদি (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            ঘূৰাই state->err;
        }
        যদি (!state->again)
            ঘূৰাই state->err;
    }
    যদি (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (ৱর্ণ *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    বাবে (len = 0; len < size; len++)
        যদি (next[len] == 0)
            ভঙ্গ;
#  অন্যথা
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#অন্যথা
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  অন্যথা
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    যদি (len == 0 || len >= state->size || next[state->size - 1] != 0)
        ঘূৰাই 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    যদি (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        যদি (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        ঘূৰাই state->err;
    }
    যদি (state->err && !state->again)
        ঘূৰাই state->err;
    ঘূৰাই (পূর্ণাঙ্ক)len;
#endif
}

#endif

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzflush(gzFile file, পূর্ণাঙ্ক flush) {
    gz_statep state;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ঘূৰাই Z_STREAM_ERROR;
    gz_error(state, Z_OK, রিক্ত);

    /* check flush parameter */
    যদি (flush < 0 || flush > Z_FINISH)
        ঘূৰাই Z_STREAM_ERROR;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ঘূৰাই state->err;

    /* compress remaining data with requested flush */
    (শূন্য)gz_comp(state, flush);
    ঘূৰাই state->err;
}

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzsetparams(gzFile file, পূর্ণাঙ্ক level, পূর্ণাঙ্ক strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ঘূৰাই Z_STREAM_ERROR;
    gz_error(state, Z_OK, রিক্ত);

    /* if no change is requested, then do nothing */
    যদি (level == state->level && strategy == state->strategy)
        ঘূৰাই Z_OK;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ঘূৰাই state->err;

    /* change compression parameters for subsequent input */
    যদি (state->size) {
        /* flush previous input with previous parameters before changing */
        যদি (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ঘূৰাই state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ঘূৰাই Z_OK;
}

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzclose_w(gzFile file) {
    পূর্ণাঙ্ক ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    যদি (file == রিক্ত)
        ঘূৰাই Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    যদি (state->mode != GZ_WRITE)
        ঘূৰাই Z_STREAM_ERROR;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    যদি (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    যদি (state->size) {
        যদি (!state->direct) {
            (শূন্য)deflateEnd(&(state->strm));
            মুক্ত(state->out);
        }
        মুক্ত(state->in);
    }
    gz_error(state, Z_OK, রিক্ত);
    মুক্ত(state->path);
    যদি (close(state->fd) == -1)
        ret = Z_ERRNO;
    মুক্ত(state);
    ঘূৰাই ret;
}
