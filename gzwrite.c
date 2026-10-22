/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#அந்தர்பாவ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local முழு gz_init(gz_statep state) {
    முழு ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned எழுத்து *)malloc(state->want << 1);
    எனில் (state->in == வெற்று) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        திரும்பு -1;
    }

    /* only need output buffer and deflate state if compressing */
    எனில் (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned எழுத்து *)malloc(state->want);
        எனில் (state->out == வெற்று) {
            விடுவி(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            திரும்பு -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        எனில் (ret != Z_OK) {
            விடுவி(state->out);
            விடுவி(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            திரும்பு -1;
        }
        strm->next_in = வெற்று;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    எனில் (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    திரும்பு 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local முழு gz_comp(gz_statep state, முழு flush) {
    முழு ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    எனில் (state->size == 0 && gz_init(state) == -1)
        திரும்பு -1;

    /* write directly if requested */
    எனில் (state->direct) {
        போது (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (முழு)லிக(state->fd, strm->next_in, put);
            எனில் (writ < 0) {
                எனில் (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                திரும்பு -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        திரும்பு 0;
    }

    /* check for a pending reset */
    எனில் (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        எனில் (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            திரும்பு 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    செய் {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        எனில் (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            போது (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (முழு)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (முழு)லிக(state->fd, state->x.next, put);
                எனில் (writ < 0) {
                    எனில் (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    திரும்பு -1;
                }
                state->x.next += writ;
            }
            எனில் (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        எனில் (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            திரும்பு -1;
        }
        have -= strm->avail_out;
    } போது (have);

    /* if that completed a deflate stream, allow another to start */
    எனில் (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    திரும்பு 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local முழு gz_zero(gz_statep state) {
    முழு first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    எனில் (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        திரும்பு -1;

    /* compress state->skip zeros */
    first = 1;
    செய் {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        எனில் (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        எனில் (ret == -1)
            திரும்பு -1;
    } போது (state->skip);
    திரும்பு 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    முழு ret;

    /* if len is zero, avoid unnecessary operations */
    எனில் (len == 0)
        திரும்பு 0;

    /* allocate memory if this is the first time through */
    எனில் (state->size == 0 && gz_init(state) == -1)
        திரும்பு 0;

    /* check for seek request */
    எனில் (state->skip && gz_zero(state) == -1)
        திரும்பு 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    எனில் (len < state->size) {
        /* copy to input buffer, compress when full */
        க்கு (;;) {
            unsigned have, copy;

            எனில் (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            எனில் (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (மாறிலி எழுத்து *)buf + copy;
            len -= copy;
            எனில் (len == 0)
                நிறுத்து;
            எனில் (gz_comp(state, Z_NO_FLUSH) == -1)
                திரும்பு state->again ? put - len : 0;
        }
    }
    இல்லையெனில் {
        /* consume whatever's left in the input buffer */
        எனில் (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            திரும்பு 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        செய் {
            unsigned n = (unsigned)-1;

            எனில் (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            எனில் (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                திரும்பு state->again ? put - len : 0;
            }
        } போது (len);
    }

    /* input was all buffered or compressed */
    திரும்பு put;
}

/* -- see zlib.h -- */
முழு ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        திரும்பு 0;
    gz_error(state, Z_OK, வெற்று);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    எனில் ((முழு)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        திரும்பு 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    திரும்பு (முழு)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        திரும்பு 0;
    gz_error(state, Z_OK, வெற்று);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    எனில் (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        திரும்பு 0;
    }

    /* write len bytes to buf, return the number of full items written */
    திரும்பு len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
முழு ZEXPORT gzputc(gzFile file, முழு c) {
    unsigned have;
    unsigned எழுத்து buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        திரும்பு -1;
    gz_error(state, Z_OK, வெற்று);

    /* check for seek request */
    எனில் (state->skip && gz_zero(state) == -1)
        திரும்பு -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    எனில் (state->size) {
        எனில் (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        எனில் (have < state->size) {
            state->in[have] = (unsigned எழுத்து)c;
            strm->avail_in++;
            state->x.pos++;
            திரும்பு c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned எழுத்து)c;
    எனில் (gz_write(state, buf, 1) != 1)
        திரும்பு -1;
    திரும்பு c & 0xff;
}

/* -- see zlib.h -- */
முழு ZEXPORT gzputs(gzFile file, மாறிலி எழுத்து *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        திரும்பு -1;
    gz_error(state, Z_OK, வெற்று);

    /* write string */
    len = strlen(s);
    எனில் ((முழு)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        திரும்பு -1;
    }
    put = gz_write(state, s, len);
    திரும்பு len && put == 0 ? -1 : (முழு)put;
}

#எனில் (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local முழு gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    எனில் (strm->next_in == வெற்று ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        திரும்பு 0;
    (வெறுமை)gz_comp(state, Z_NO_FLUSH);
    எனில் (strm->avail_in == 0) {
        strm->next_in = state->in;
        திரும்பு 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    திரும்பு strm->avail_in > state->size;
}
#endif

#எனில் defined(STDC) || defined(Z_HAVE_STDARG_H)
#அந்தர்பாவ <stdarg.h>

/* -- see zlib.h -- */
முழு ZEXPORTVA gzvprintf(gzFile file, மாறிலி எழுத்து *format, va_list va) {
#எனில் defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (வெறுமை)file, (வெறுமை)format, (வெறுமை)va;
    திரும்பு Z_STREAM_ERROR;
#இல்லையெனில்
    முழு len, ret;
    எழுத்து *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        திரும்பு Z_STREAM_ERROR;
    gz_error(state, Z_OK, வெற்று);

    /* make sure we have some buffer space */
    எனில் (state->size == 0 && gz_init(state) == -1)
        திரும்பு state->err;

    /* check for seek request */
    எனில் (state->skip && gz_zero(state) == -1)
        திரும்பு state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    எனில் (state->err) {
        எனில் (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            திரும்பு state->err;
        }
        எனில் (!state->again)
            திரும்பு state->err;
    }
    எனில் (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (எழுத்து *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (வெறுமை)vsprintf(next, format, va);
    க்கு (len = 0; len < state->size; len++)
        எனில் (next[len] == 0) நிறுத்து;
#  இல்லையெனில்
    len = vsprintf(next, format, va);
#  endif
#இல்லையெனில்
#  ifdef HAS_vsnprintf_void
    (வெறுமை)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  இல்லையெனில்
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    எனில் (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        திரும்பு 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    எனில் (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        எனில் (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        திரும்பு state->err;
    }
    எனில் (state->err && !state->again)
        திரும்பு state->err;
    திரும்பு len;
#endif
}

முழு ZEXPORTVA gzprintf(gzFile file, மாறிலி எழுத்து *format, ...) {
    va_list va;
    முழு ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    திரும்பு ret;
}

#இல்லையெனில் /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
முழு ZEXPORTVA gzprintf(gzFile file, மாறிலி எழுத்து *format, முழு a1, முழு a2, முழு a3,
                       முழு a4, முழு a5, முழு a6, முழு a7, முழு a8, முழு a9, முழு a10,
                       முழு a11, முழு a12, முழு a13, முழு a14, முழு a15, முழு a16,
                       முழு a17, முழு a18, முழு a19, முழு a20) {
#எனில் defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (வெறுமை)file, (வெறுமை)format, (வெறுமை)a1, (வெறுமை)a2, (வெறுமை)a3, (வெறுமை)a4, (வெறுமை)a5,
    (வெறுமை)a6, (வெறுமை)a7, (வெறுமை)a8, (வெறுமை)a9, (வெறுமை)a10, (வெறுமை)a11, (வெறுமை)a12,
    (வெறுமை)a13, (வெறுமை)a14, (வெறுமை)a15, (வெறுமை)a16, (வெறுமை)a17, (வெறுமை)a18,
    (வெறுமை)a19, (வெறுமை)a20;
    திரும்பு Z_STREAM_ERROR;
#இல்லையெனில்
    முழு ret;
    unsigned len, இடது;
    எழுத்து *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    எனில் (அளவு(முழு) != அளவு(வெறுமை *))
        திரும்பு Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        திரும்பு Z_STREAM_ERROR;
    gz_error(state, Z_OK, வெற்று);

    /* make sure we have some buffer space */
    எனில் (state->size == 0 && gz_init(state) == -1)
        திரும்பு state->err;

    /* check for seek request */
    எனில் (state->skip && gz_zero(state) == -1)
        திரும்பு state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    எனில் (state->err) {
        எனில் (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            திரும்பு state->err;
        }
        எனில் (!state->again)
            திரும்பு state->err;
    }
    எனில் (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (எழுத்து *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    க்கு (len = 0; len < size; len++)
        எனில் (next[len] == 0)
            நிறுத்து;
#  இல்லையெனில்
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#இல்லையெனில்
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  இல்லையெனில்
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    எனில் (len == 0 || len >= state->size || next[state->size - 1] != 0)
        திரும்பு 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    எனில் (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        எனில் (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        திரும்பு state->err;
    }
    எனில் (state->err && !state->again)
        திரும்பு state->err;
    திரும்பு (முழு)len;
#endif
}

#endif

/* -- see zlib.h -- */
முழு ZEXPORT gzflush(gzFile file, முழு flush) {
    gz_statep state;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        திரும்பு Z_STREAM_ERROR;
    gz_error(state, Z_OK, வெற்று);

    /* check flush parameter */
    எனில் (flush < 0 || flush > Z_FINISH)
        திரும்பு Z_STREAM_ERROR;

    /* check for seek request */
    எனில் (state->skip && gz_zero(state) == -1)
        திரும்பு state->err;

    /* compress remaining data with requested flush */
    (வெறுமை)gz_comp(state, flush);
    திரும்பு state->err;
}

/* -- see zlib.h -- */
முழு ZEXPORT gzsetparams(gzFile file, முழு level, முழு strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    எனில் (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        திரும்பு Z_STREAM_ERROR;
    gz_error(state, Z_OK, வெற்று);

    /* if no change is requested, then do nothing */
    எனில் (level == state->level && strategy == state->strategy)
        திரும்பு Z_OK;

    /* check for seek request */
    எனில் (state->skip && gz_zero(state) == -1)
        திரும்பு state->err;

    /* change compression parameters for subsequent input */
    எனில் (state->size) {
        /* flush previous input with previous parameters before changing */
        எனில் (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            திரும்பு state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    திரும்பு Z_OK;
}

/* -- see zlib.h -- */
முழு ZEXPORT gzclose_w(gzFile file) {
    முழு ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    எனில் (state->mode != GZ_WRITE)
        திரும்பு Z_STREAM_ERROR;

    /* check for seek request */
    எனில் (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    எனில் (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    எனில் (state->size) {
        எனில் (!state->direct) {
            (வெறுமை)deflateEnd(&(state->strm));
            விடுவி(state->out);
        }
        விடுவி(state->in);
    }
    gz_error(state, Z_OK, வெற்று);
    விடுவி(state->path);
    எனில் (close(state->fd) == -1)
        ret = Z_ERRNO;
    விடுவி(state);
    திரும்பு ret;
}
