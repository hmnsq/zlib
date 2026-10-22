/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#অন্তর্ভাব "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local পূর্ণসংখ্যা gz_init(gz_statep state) {
    পূর্ণসংখ্যা ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned অক্ষর *)malloc(state->want << 1);
    যদি (state->in == ফাঁকা) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        ফেরত -1;
    }

    /* only need output buffer and deflate state if compressing */
    যদি (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned অক্ষর *)malloc(state->want);
        যদি (state->out == ফাঁকা) {
            মুক্ত(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ফেরত -1;
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
            ফেরত -1;
        }
        strm->পরবর্তী_in = ফাঁকা;
    }

    /* mark state as initialized */
    state->আকার_2 = state->want;

    /* initialize write buffer if compressing */
    যদি (!state->direct) {
        strm->avail_out = state->আকার_2;
        strm->পরবর্তী_out = state->out;
        state->x.পরবর্তী = strm->পরবর্তী_out;
    }
    ফেরত 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local পূর্ণসংখ্যা gz_comp(gz_statep state, পূর্ণসংখ্যা flush) {
    পূর্ণসংখ্যা ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    যদি (state->আকার_2 == 0 && gz_init(state) == -1)
        ফেরত -1;

    /* write directly if requested */
    যদি (state->direct) {
        যখন (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (পূর্ণসংখ্যা)লিখ(state->fd, strm->পরবর্তী_in, put);
            যদি (writ < 0) {
                যদি (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                ফেরত -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->পরবর্তী_in += writ;
        }
        ফেরত 0;
    }

    /* check for a pending reset */
    যদি (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        যদি (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            ফেরত 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    করো {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        যদি (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            যখন (strm->পরবর্তী_out > state->x.পরবর্তী) {
                errno = 0;
                state->again = 0;
                put = strm->পরবর্তী_out - state->x.পরবর্তী > (পূর্ণসংখ্যা)max ? max :
                      (unsigned)(strm->পরবর্তী_out - state->x.পরবর্তী);
                writ = (পূর্ণসংখ্যা)লিখ(state->fd, state->x.পরবর্তী, put);
                যদি (writ < 0) {
                    যদি (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    ফেরত -1;
                }
                state->x.পরবর্তী += writ;
            }
            যদি (strm->avail_out == 0) {
                strm->avail_out = state->আকার_2;
                strm->পরবর্তী_out = state->out;
                state->x.পরবর্তী = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        যদি (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            ফেরত -1;
        }
        have -= strm->avail_out;
    } যখন (have);

    /* if that completed a deflate stream, allow another to start */
    যদি (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    ফেরত 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local পূর্ণসংখ্যা gz_zero(gz_statep state) {
    পূর্ণসংখ্যা first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    যদি (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        ফেরত -1;

    /* compress state->skip zeros */
    first = 1;
    করো {
        n = GT_OFF(state->আকার_2) || (z_off64_t)state->আকার_2 > state->skip ?
            (unsigned)state->skip : state->আকার_2;
        যদি (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->পরবর্তী_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        যদি (ret == -1)
            ফেরত -1;
    } যখন (state->skip);
    ফেরত 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_আকার_t gz_write(gz_statep state, voidpc buf, z_আকার_t len) {
    z_আকার_t put = len;
    পূর্ণসংখ্যা ret;

    /* if len is zero, avoid unnecessary operations */
    যদি (len == 0)
        ফেরত 0;

    /* allocate memory if this is the first time through */
    যদি (state->আকার_2 == 0 && gz_init(state) == -1)
        ফেরত 0;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ফেরত 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    যদি (len < state->আকার_2) {
        /* copy to input buffer, compress when full */
        জন্য (;;) {
            unsigned have, copy;

            যদি (state->strm.avail_in == 0)
                state->strm.পরবর্তী_in = state->in;
            have = (unsigned)((state->strm.পরবর্তী_in + state->strm.avail_in) -
                              state->in);
            copy = state->আকার_2 - have;
            যদি (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ধ্রুবক অক্ষর *)buf + copy;
            len -= copy;
            যদি (len == 0)
                বিরতি;
            যদি (gz_comp(state, Z_NO_FLUSH) == -1)
                ফেরত state->again ? put - len : 0;
        }
    }
    অন্যথা {
        /* consume whatever's left in the input buffer */
        যদি (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            ফেরত 0;

        /* directly compress user buffer to file */
        state->strm.পরবর্তী_in = (z_const Bytef *)buf;
        করো {
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
                state->strm.পরবর্তী_in = state->in;
                ফেরত state->again ? put - len : 0;
            }
        } যখন (len);
    }

    /* input was all buffered or compressed */
    ফেরত put;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ফেরত 0;
    gz_error(state, Z_OK, ফাঁকা);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    যদি ((পূর্ণসংখ্যা)len < 0) {
        gz_error(state, Z_তথ্য_ERROR, "requested length does not fit in int");
        ফেরত 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    ফেরত (পূর্ণসংখ্যা)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_আকার_t ZEXPORT gzfwrite(voidpc buf, z_আকার_t আকার_2, z_আকার_t nitems,
                          gzFile file) {
    z_আকার_t len;
    gz_statep state;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ফেরত 0;
    gz_error(state, Z_OK, ফাঁকা);

    /* compute bytes to read -- error on overflow */
    len = nitems * আকার_2;
    যদি (আকার_2 && len / আকার_2 != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ফেরত 0;
    }

    /* write len bytes to buf, return the number of full items written */
    ফেরত len ? gz_write(state, buf, len) / আকার_2 : 0;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzputc(gzFile file, পূর্ণসংখ্যা c) {
    unsigned have;
    unsigned অক্ষর buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ফেরত -1;
    gz_error(state, Z_OK, ফাঁকা);

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ফেরত -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    যদি (state->আকার_2) {
        যদি (strm->avail_in == 0)
            strm->পরবর্তী_in = state->in;
        have = (unsigned)((strm->পরবর্তী_in + strm->avail_in) - state->in);
        যদি (have < state->আকার_2) {
            state->in[have] = (unsigned অক্ষর)c;
            strm->avail_in++;
            state->x.pos++;
            ফেরত c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned অক্ষর)c;
    যদি (gz_write(state, buf, 1) != 1)
        ফেরত -1;
    ফেরত c & 0xff;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzputs(gzFile file, ধ্রুবক অক্ষর *s) {
    z_আকার_t len, put;
    gz_statep state;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ফেরত -1;
    gz_error(state, Z_OK, ফাঁকা);

    /* write string */
    len = strlen(s);
    যদি ((পূর্ণসংখ্যা)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        ফেরত -1;
    }
    put = gz_write(state, s, len);
    ফেরত len && put == 0 ? -1 : (পূর্ণসংখ্যা)put;
}

#যদি (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local পূর্ণসংখ্যা gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    যদি (strm->পরবর্তী_in == ফাঁকা ||
        strm->পরবর্তী_in + strm->avail_in <= state->in + state->আকার_2)
        ফেরত 0;
    (শূন্য)gz_comp(state, Z_NO_FLUSH);
    যদি (strm->avail_in == 0) {
        strm->পরবর্তী_in = state->in;
        ফেরত 0;
    }
    memmove(state->in, strm->পরবর্তী_in, strm->avail_in);
    strm->পরবর্তী_in = state->in;
    ফেরত strm->avail_in > state->আকার_2;
}
#endif

#যদি defined(STDC) || defined(Z_HAVE_STDARG_H)
#অন্তর্ভাব <stdarg.h>

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORTVA gzvprintf(gzFile file, ধ্রুবক অক্ষর *format, va_list va) {
#যদি defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (শূন্য)file, (শূন্য)format, (শূন্য)va;
    ফেরত Z_STREAM_ERROR;
#অন্যথা
    পূর্ণসংখ্যা len, ret;
    অক্ষর *পরবর্তী;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ফেরত Z_STREAM_ERROR;
    gz_error(state, Z_OK, ফাঁকা);

    /* make sure we have some buffer space */
    যদি (state->আকার_2 == 0 && gz_init(state) == -1)
        ফেরত state->err;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ফেরত state->err;

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
            ফেরত state->err;
        }
        যদি (!state->again)
            ফেরত state->err;
    }
    যদি (strm->avail_in == 0)
        strm->পরবর্তী_in = state->in;
    পরবর্তী = (অক্ষর *)(state->in + (strm->পরবর্তী_in - state->in) + strm->avail_in);
    পরবর্তী[state->আকার_2 - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (শূন্য)vsprintf(পরবর্তী, format, va);
    জন্য (len = 0; len < state->আকার_2; len++)
        যদি (পরবর্তী[len] == 0) বিরতি;
#  অন্যথা
    len = vsprintf(পরবর্তী, format, va);
#  endif
#অন্যথা
#  ifdef HAS_vsnprintf_void
    (শূন্য)vsnprintf(পরবর্তী, state->আকার_2, format, va);
    len = strlen(পরবর্তী);
#  অন্যথা
    len = vsnprintf(পরবর্তী, state->আকার_2, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    যদি (len == 0 || (unsigned)len >= state->আকার_2 || পরবর্তী[state->আকার_2 - 1] != 0)
        ফেরত 0;

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
        ফেরত state->err;
    }
    যদি (state->err && !state->again)
        ফেরত state->err;
    ফেরত len;
#endif
}

পূর্ণসংখ্যা ZEXPORTVA gzprintf(gzFile file, ধ্রুবক অক্ষর *format, ...) {
    va_list va;
    পূর্ণসংখ্যা ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    ফেরত ret;
}

#অন্যথা /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORTVA gzprintf(gzFile file, ধ্রুবক অক্ষর *format, পূর্ণসংখ্যা a1, পূর্ণসংখ্যা a2, পূর্ণসংখ্যা a3,
                       পূর্ণসংখ্যা a4, পূর্ণসংখ্যা a5, পূর্ণসংখ্যা a6, পূর্ণসংখ্যা a7, পূর্ণসংখ্যা a8, পূর্ণসংখ্যা a9, পূর্ণসংখ্যা a10,
                       পূর্ণসংখ্যা a11, পূর্ণসংখ্যা a12, পূর্ণসংখ্যা a13, পূর্ণসংখ্যা a14, পূর্ণসংখ্যা a15, পূর্ণসংখ্যা a16,
                       পূর্ণসংখ্যা a17, পূর্ণসংখ্যা a18, পূর্ণসংখ্যা a19, পূর্ণসংখ্যা a20) {
#যদি defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (শূন্য)file, (শূন্য)format, (শূন্য)a1, (শূন্য)a2, (শূন্য)a3, (শূন্য)a4, (শূন্য)a5,
    (শূন্য)a6, (শূন্য)a7, (শূন্য)a8, (শূন্য)a9, (শূন্য)a10, (শূন্য)a11, (শূন্য)a12,
    (শূন্য)a13, (শূন্য)a14, (শূন্য)a15, (শূন্য)a16, (শূন্য)a17, (শূন্য)a18,
    (শূন্য)a19, (শূন্য)a20;
    ফেরত Z_STREAM_ERROR;
#অন্যথা
    পূর্ণসংখ্যা ret;
    unsigned len, বাম;
    অক্ষর *পরবর্তী;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    যদি (আকার(পূর্ণসংখ্যা) != আকার(শূন্য *))
        ফেরত Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ফেরত Z_STREAM_ERROR;
    gz_error(state, Z_OK, ফাঁকা);

    /* make sure we have some buffer space */
    যদি (state->আকার_2 == 0 && gz_init(state) == -1)
        ফেরত state->err;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ফেরত state->err;

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
            ফেরত state->err;
        }
        যদি (!state->again)
            ফেরত state->err;
    }
    যদি (strm->avail_in == 0)
        strm->পরবর্তী_in = state->in;
    পরবর্তী = (অক্ষর *)(strm->পরবর্তী_in + strm->avail_in);
    পরবর্তী[state->আকার_2 - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(পরবর্তী, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    জন্য (len = 0; len < আকার_2; len++)
        যদি (পরবর্তী[len] == 0)
            বিরতি;
#  অন্যথা
    len = sprintf(পরবর্তী, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#অন্যথা
#  ifdef HAS_snprintf_void
    snprintf(পরবর্তী, state->আকার_2, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(পরবর্তী);
#  অন্যথা
    len = snprintf(পরবর্তী, state->আকার_2, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    যদি (len == 0 || len >= state->আকার_2 || পরবর্তী[state->আকার_2 - 1] != 0)
        ফেরত 0;

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
        ফেরত state->err;
    }
    যদি (state->err && !state->again)
        ফেরত state->err;
    ফেরত (পূর্ণসংখ্যা)len;
#endif
}

#endif

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzflush(gzFile file, পূর্ণসংখ্যা flush) {
    gz_statep state;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        ফেরত Z_STREAM_ERROR;
    gz_error(state, Z_OK, ফাঁকা);

    /* check flush parameter */
    যদি (flush < 0 || flush > Z_FINISH)
        ফেরত Z_STREAM_ERROR;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ফেরত state->err;

    /* compress remaining data with requested flush */
    (শূন্য)gz_comp(state, flush);
    ফেরত state->err;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzsetparams(gzFile file, পূর্ণসংখ্যা level, পূর্ণসংখ্যা strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    যদি (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        ফেরত Z_STREAM_ERROR;
    gz_error(state, Z_OK, ফাঁকা);

    /* if no change is requested, then do nothing */
    যদি (level == state->level && strategy == state->strategy)
        ফেরত Z_OK;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ফেরত state->err;

    /* change compression parameters for subsequent input */
    যদি (state->আকার_2) {
        /* flush previous input with previous parameters before changing */
        যদি (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            ফেরত state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    ফেরত Z_OK;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzclose_w(gzFile file) {
    পূর্ণসংখ্যা ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    যদি (state->mode != GZ_WRITE)
        ফেরত Z_STREAM_ERROR;

    /* check for seek request */
    যদি (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    যদি (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    যদি (state->আকার_2) {
        যদি (!state->direct) {
            (শূন্য)deflateEnd(&(state->strm));
            মুক্ত(state->out);
        }
        মুক্ত(state->in);
    }
    gz_error(state, Z_OK, ফাঁকা);
    মুক্ত(state->path);
    যদি (close(state->fd) == -1)
        ret = Z_ERRNO;
    মুক্ত(state);
    ফেরত ret;
}
