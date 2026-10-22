/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#અન્તર્ભાવ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local પૂર્ણાંક gz_init(gz_statep state) {
    પૂર્ણાંક ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned અક્ષર *)malloc(state->want << 1);
    જો (state->in == ખાલી) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        પાછા -1;
    }

    /* only need output buffer and deflate state if compressing */
    જો (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned અક્ષર *)malloc(state->want);
        જો (state->out == ખાલી) {
            મુક્ત(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            પાછા -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        જો (ret != Z_OK) {
            મુક્ત(state->out);
            મુક્ત(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            પાછા -1;
        }
        strm->next_in = ખાલી;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    જો (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    પાછા 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local પૂર્ણાંક gz_comp(gz_statep state, પૂર્ણાંક flush) {
    પૂર્ણાંક ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    જો (state->size == 0 && gz_init(state) == -1)
        પાછા -1;

    /* write directly if requested */
    જો (state->direct) {
        જ્યારે (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (પૂર્ણાંક)લિખ(state->fd, strm->next_in, put);
            જો (writ < 0) {
                જો (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                પાછા -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        પાછા 0;
    }

    /* check for a pending reset */
    જો (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        જો (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            પાછા 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    કરો {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        જો (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            જ્યારે (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (પૂર્ણાંક)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (પૂર્ણાંક)લિખ(state->fd, state->x.next, put);
                જો (writ < 0) {
                    જો (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    પાછા -1;
                }
                state->x.next += writ;
            }
            જો (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        જો (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            પાછા -1;
        }
        have -= strm->avail_out;
    } જ્યારે (have);

    /* if that completed a deflate stream, allow another to start */
    જો (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    પાછા 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local પૂર્ણાંક gz_zero(gz_statep state) {
    પૂર્ણાંક first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    જો (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        પાછા -1;

    /* compress state->skip zeros */
    first = 1;
    કરો {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        જો (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        જો (ret == -1)
            પાછા -1;
    } જ્યારે (state->skip);
    પાછા 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    પૂર્ણાંક ret;

    /* if len is zero, avoid unnecessary operations */
    જો (len == 0)
        પાછા 0;

    /* allocate memory if this is the first time through */
    જો (state->size == 0 && gz_init(state) == -1)
        પાછા 0;

    /* check for seek request */
    જો (state->skip && gz_zero(state) == -1)
        પાછા 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    જો (len < state->size) {
        /* copy to input buffer, compress when full */
        માટે (;;) {
            unsigned have, copy;

            જો (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            જો (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (અચલ અક્ષર *)buf + copy;
            len -= copy;
            જો (len == 0)
                તોડો;
            જો (gz_comp(state, Z_NO_FLUSH) == -1)
                પાછા state->again ? put - len : 0;
        }
    }
    નહીંતર {
        /* consume whatever's left in the input buffer */
        જો (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            પાછા 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        કરો {
            unsigned n = (unsigned)-1;

            જો (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            જો (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                પાછા state->again ? put - len : 0;
            }
        } જ્યારે (len);
    }

    /* input was all buffered or compressed */
    પાછા put;
}

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        પાછા 0;
    gz_error(state, Z_OK, ખાલી);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    જો ((પૂર્ણાંક)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        પાછા 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    પાછા (પૂર્ણાંક)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        પાછા 0;
    gz_error(state, Z_OK, ખાલી);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    જો (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        પાછા 0;
    }

    /* write len bytes to buf, return the number of full items written */
    પાછા len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzputc(gzFile file, પૂર્ણાંક c) {
    unsigned have;
    unsigned અક્ષર buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        પાછા -1;
    gz_error(state, Z_OK, ખાલી);

    /* check for seek request */
    જો (state->skip && gz_zero(state) == -1)
        પાછા -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    જો (state->size) {
        જો (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        જો (have < state->size) {
            state->in[have] = (unsigned અક્ષર)c;
            strm->avail_in++;
            state->x.pos++;
            પાછા c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned અક્ષર)c;
    જો (gz_write(state, buf, 1) != 1)
        પાછા -1;
    પાછા c & 0xff;
}

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzputs(gzFile file, અચલ અક્ષર *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        પાછા -1;
    gz_error(state, Z_OK, ખાલી);

    /* write string */
    len = strlen(s);
    જો ((પૂર્ણાંક)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        પાછા -1;
    }
    put = gz_write(state, s, len);
    પાછા len && put == 0 ? -1 : (પૂર્ણાંક)put;
}

#જો (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local પૂર્ણાંક gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    જો (strm->next_in == ખાલી ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        પાછા 0;
    (શૂન્ય)gz_comp(state, Z_NO_FLUSH);
    જો (strm->avail_in == 0) {
        strm->next_in = state->in;
        પાછા 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    પાછા strm->avail_in > state->size;
}
#endif

#જો defined(STDC) || defined(Z_HAVE_STDARG_H)
#અન્તર્ભાવ <stdarg.h>

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORTVA gzvprintf(gzFile file, અચલ અક્ષર *format, va_list va) {
#જો defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (શૂન્ય)file, (શૂન્ય)format, (શૂન્ય)va;
    પાછા Z_STREAM_ERROR;
#નહીંતર
    પૂર્ણાંક len, ret;
    અક્ષર *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        પાછા Z_STREAM_ERROR;
    gz_error(state, Z_OK, ખાલી);

    /* make sure we have some buffer space */
    જો (state->size == 0 && gz_init(state) == -1)
        પાછા state->err;

    /* check for seek request */
    જો (state->skip && gz_zero(state) == -1)
        પાછા state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    જો (state->err) {
        જો (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            પાછા state->err;
        }
        જો (!state->again)
            પાછા state->err;
    }
    જો (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (અક્ષર *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (શૂન્ય)vsprintf(next, format, va);
    માટે (len = 0; len < state->size; len++)
        જો (next[len] == 0) તોડો;
#  નહીંતર
    len = vsprintf(next, format, va);
#  endif
#નહીંતર
#  ifdef HAS_vsnprintf_void
    (શૂન્ય)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  નહીંતર
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    જો (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        પાછા 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    જો (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        જો (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        પાછા state->err;
    }
    જો (state->err && !state->again)
        પાછા state->err;
    પાછા len;
#endif
}

પૂર્ણાંક ZEXPORTVA gzprintf(gzFile file, અચલ અક્ષર *format, ...) {
    va_list va;
    પૂર્ણાંક ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    પાછા ret;
}

#નહીંતર /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORTVA gzprintf(gzFile file, અચલ અક્ષર *format, પૂર્ણાંક a1, પૂર્ણાંક a2, પૂર્ણાંક a3,
                       પૂર્ણાંક a4, પૂર્ણાંક a5, પૂર્ણાંક a6, પૂર્ણાંક a7, પૂર્ણાંક a8, પૂર્ણાંક a9, પૂર્ણાંક a10,
                       પૂર્ણાંક a11, પૂર્ણાંક a12, પૂર્ણાંક a13, પૂર્ણાંક a14, પૂર્ણાંક a15, પૂર્ણાંક a16,
                       પૂર્ણાંક a17, પૂર્ણાંક a18, પૂર્ણાંક a19, પૂર્ણાંક a20) {
#જો defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (શૂન્ય)file, (શૂન્ય)format, (શૂન્ય)a1, (શૂન્ય)a2, (શૂન્ય)a3, (શૂન્ય)a4, (શૂન્ય)a5,
    (શૂન્ય)a6, (શૂન્ય)a7, (શૂન્ય)a8, (શૂન્ય)a9, (શૂન્ય)a10, (શૂન્ય)a11, (શૂન્ય)a12,
    (શૂન્ય)a13, (શૂન્ય)a14, (શૂન્ય)a15, (શૂન્ય)a16, (શૂન્ય)a17, (શૂન્ય)a18,
    (શૂન્ય)a19, (શૂન્ય)a20;
    પાછા Z_STREAM_ERROR;
#નહીંતર
    પૂર્ણાંક ret;
    unsigned len, ડાબું;
    અક્ષર *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    જો (કદ(પૂર્ણાંક) != કદ(શૂન્ય *))
        પાછા Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        પાછા Z_STREAM_ERROR;
    gz_error(state, Z_OK, ખાલી);

    /* make sure we have some buffer space */
    જો (state->size == 0 && gz_init(state) == -1)
        પાછા state->err;

    /* check for seek request */
    જો (state->skip && gz_zero(state) == -1)
        પાછા state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    જો (state->err) {
        જો (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            પાછા state->err;
        }
        જો (!state->again)
            પાછા state->err;
    }
    જો (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (અક્ષર *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    માટે (len = 0; len < size; len++)
        જો (next[len] == 0)
            તોડો;
#  નહીંતર
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#નહીંતર
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  નહીંતર
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    જો (len == 0 || len >= state->size || next[state->size - 1] != 0)
        પાછા 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    જો (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        જો (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        પાછા state->err;
    }
    જો (state->err && !state->again)
        પાછા state->err;
    પાછા (પૂર્ણાંક)len;
#endif
}

#endif

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzflush(gzFile file, પૂર્ણાંક flush) {
    gz_statep state;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        પાછા Z_STREAM_ERROR;
    gz_error(state, Z_OK, ખાલી);

    /* check flush parameter */
    જો (flush < 0 || flush > Z_FINISH)
        પાછા Z_STREAM_ERROR;

    /* check for seek request */
    જો (state->skip && gz_zero(state) == -1)
        પાછા state->err;

    /* compress remaining data with requested flush */
    (શૂન્ય)gz_comp(state, flush);
    પાછા state->err;
}

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzsetparams(gzFile file, પૂર્ણાંક level, પૂર્ણાંક strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    જો (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        પાછા Z_STREAM_ERROR;
    gz_error(state, Z_OK, ખાલી);

    /* if no change is requested, then do nothing */
    જો (level == state->level && strategy == state->strategy)
        પાછા Z_OK;

    /* check for seek request */
    જો (state->skip && gz_zero(state) == -1)
        પાછા state->err;

    /* change compression parameters for subsequent input */
    જો (state->size) {
        /* flush previous input with previous parameters before changing */
        જો (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            પાછા state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    પાછા Z_OK;
}

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzclose_w(gzFile file) {
    પૂર્ણાંક ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    જો (state->mode != GZ_WRITE)
        પાછા Z_STREAM_ERROR;

    /* check for seek request */
    જો (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    જો (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    જો (state->size) {
        જો (!state->direct) {
            (શૂન્ય)deflateEnd(&(state->strm));
            મુક્ત(state->out);
        }
        મુક્ત(state->in);
    }
    gz_error(state, Z_OK, ખાલી);
    મુક્ત(state->path);
    જો (close(state->fd) == -1)
        ret = Z_ERRNO;
    મુક્ત(state);
    પાછા ret;
}
