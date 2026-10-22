/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#അന്തര്ഭാവ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local പൂർണ്ണസംഖ്യ gz_init(gz_statep state) {
    പൂർണ്ണസംഖ്യ ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned അക്ഷരം *)malloc(state->want << 1);
    എങ്കിൽ (state->in == ശൂന്യ) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        തിരികെ -1;
    }

    /* only need output buffer and deflate state if compressing */
    എങ്കിൽ (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned അക്ഷരം *)malloc(state->want);
        എങ്കിൽ (state->out == ശൂന്യ) {
            സ്വതന്ത്രം(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            തിരികെ -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        എങ്കിൽ (ret != Z_OK) {
            സ്വതന്ത്രം(state->out);
            സ്വതന്ത്രം(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            തിരികെ -1;
        }
        strm->next_in = ശൂന്യ;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    എങ്കിൽ (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    തിരികെ 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local പൂർണ്ണസംഖ്യ gz_comp(gz_statep state, പൂർണ്ണസംഖ്യ flush) {
    പൂർണ്ണസംഖ്യ ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    എങ്കിൽ (state->size == 0 && gz_init(state) == -1)
        തിരികെ -1;

    /* write directly if requested */
    എങ്കിൽ (state->direct) {
        ആകുമ്പോൾ (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (പൂർണ്ണസംഖ്യ)ലിഖ(state->fd, strm->next_in, put);
            എങ്കിൽ (writ < 0) {
                എങ്കിൽ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                തിരികെ -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        തിരികെ 0;
    }

    /* check for a pending reset */
    എങ്കിൽ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        എങ്കിൽ (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            തിരികെ 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    ചെയ്യുക {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        എങ്കിൽ (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            ആകുമ്പോൾ (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (പൂർണ്ണസംഖ്യ)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (പൂർണ്ണസംഖ്യ)ലിഖ(state->fd, state->x.next, put);
                എങ്കിൽ (writ < 0) {
                    എങ്കിൽ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    തിരികെ -1;
                }
                state->x.next += writ;
            }
            എങ്കിൽ (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        എങ്കിൽ (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            തിരികെ -1;
        }
        have -= strm->avail_out;
    } ആകുമ്പോൾ (have);

    /* if that completed a deflate stream, allow another to start */
    എങ്കിൽ (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    തിരികെ 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local പൂർണ്ണസംഖ്യ gz_zero(gz_statep state) {
    പൂർണ്ണസംഖ്യ first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    എങ്കിൽ (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        തിരികെ -1;

    /* compress state->skip zeros */
    first = 1;
    ചെയ്യുക {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        എങ്കിൽ (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        എങ്കിൽ (ret == -1)
            തിരികെ -1;
    } ആകുമ്പോൾ (state->skip);
    തിരികെ 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    പൂർണ്ണസംഖ്യ ret;

    /* if len is zero, avoid unnecessary operations */
    എങ്കിൽ (len == 0)
        തിരികെ 0;

    /* allocate memory if this is the first time through */
    എങ്കിൽ (state->size == 0 && gz_init(state) == -1)
        തിരികെ 0;

    /* check for seek request */
    എങ്കിൽ (state->skip && gz_zero(state) == -1)
        തിരികെ 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    എങ്കിൽ (len < state->size) {
        /* copy to input buffer, compress when full */
        വേണ്ടി (;;) {
            unsigned have, copy;

            എങ്കിൽ (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            എങ്കിൽ (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (സ്ഥിരം അക്ഷരം *)buf + copy;
            len -= copy;
            എങ്കിൽ (len == 0)
                നിർത്തുക;
            എങ്കിൽ (gz_comp(state, Z_NO_FLUSH) == -1)
                തിരികെ state->again ? put - len : 0;
        }
    }
    അല്ലെങ്കിൽ {
        /* consume whatever's left in the input buffer */
        എങ്കിൽ (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            തിരികെ 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        ചെയ്യുക {
            unsigned n = (unsigned)-1;

            എങ്കിൽ (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            എങ്കിൽ (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                തിരികെ state->again ? put - len : 0;
            }
        } ആകുമ്പോൾ (len);
    }

    /* input was all buffered or compressed */
    തിരികെ put;
}

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        തിരികെ 0;
    gz_error(state, Z_OK, ശൂന്യ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    എങ്കിൽ ((പൂർണ്ണസംഖ്യ)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        തിരികെ 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    തിരികെ (പൂർണ്ണസംഖ്യ)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        തിരികെ 0;
    gz_error(state, Z_OK, ശൂന്യ);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    എങ്കിൽ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        തിരികെ 0;
    }

    /* write len bytes to buf, return the number of full items written */
    തിരികെ len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzputc(gzFile file, പൂർണ്ണസംഖ്യ c) {
    unsigned have;
    unsigned അക്ഷരം buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        തിരികെ -1;
    gz_error(state, Z_OK, ശൂന്യ);

    /* check for seek request */
    എങ്കിൽ (state->skip && gz_zero(state) == -1)
        തിരികെ -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    എങ്കിൽ (state->size) {
        എങ്കിൽ (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        എങ്കിൽ (have < state->size) {
            state->in[have] = (unsigned അക്ഷരം)c;
            strm->avail_in++;
            state->x.pos++;
            തിരികെ c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned അക്ഷരം)c;
    എങ്കിൽ (gz_write(state, buf, 1) != 1)
        തിരികെ -1;
    തിരികെ c & 0xff;
}

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzputs(gzFile file, സ്ഥിരം അക്ഷരം *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        തിരികെ -1;
    gz_error(state, Z_OK, ശൂന്യ);

    /* write string */
    len = strlen(s);
    എങ്കിൽ ((പൂർണ്ണസംഖ്യ)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        തിരികെ -1;
    }
    put = gz_write(state, s, len);
    തിരികെ len && put == 0 ? -1 : (പൂർണ്ണസംഖ്യ)put;
}

#എങ്കിൽ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local പൂർണ്ണസംഖ്യ gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    എങ്കിൽ (strm->next_in == ശൂന്യ ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        തിരികെ 0;
    (ശൂന്യം)gz_comp(state, Z_NO_FLUSH);
    എങ്കിൽ (strm->avail_in == 0) {
        strm->next_in = state->in;
        തിരികെ 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    തിരികെ strm->avail_in > state->size;
}
#endif

#എങ്കിൽ defined(STDC) || defined(Z_HAVE_STDARG_H)
#അന്തര്ഭാവ <stdarg.h>

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORTVA gzvprintf(gzFile file, സ്ഥിരം അക്ഷരം *format, va_list va) {
#എങ്കിൽ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (ശൂന്യം)file, (ശൂന്യം)format, (ശൂന്യം)va;
    തിരികെ Z_STREAM_ERROR;
#അല്ലെങ്കിൽ
    പൂർണ്ണസംഖ്യ len, ret;
    അക്ഷരം *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        തിരികെ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ശൂന്യ);

    /* make sure we have some buffer space */
    എങ്കിൽ (state->size == 0 && gz_init(state) == -1)
        തിരികെ state->err;

    /* check for seek request */
    എങ്കിൽ (state->skip && gz_zero(state) == -1)
        തിരികെ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    എങ്കിൽ (state->err) {
        എങ്കിൽ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            തിരികെ state->err;
        }
        എങ്കിൽ (!state->again)
            തിരികെ state->err;
    }
    എങ്കിൽ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (അക്ഷരം *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (ശൂന്യം)vsprintf(next, format, va);
    വേണ്ടി (len = 0; len < state->size; len++)
        എങ്കിൽ (next[len] == 0) നിർത്തുക;
#  അല്ലെങ്കിൽ
    len = vsprintf(next, format, va);
#  endif
#അല്ലെങ്കിൽ
#  ifdef HAS_vsnprintf_void
    (ശൂന്യം)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  അല്ലെങ്കിൽ
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    എങ്കിൽ (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        തിരികെ 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    എങ്കിൽ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        എങ്കിൽ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        തിരികെ state->err;
    }
    എങ്കിൽ (state->err && !state->again)
        തിരികെ state->err;
    തിരികെ len;
#endif
}

പൂർണ്ണസംഖ്യ ZEXPORTVA gzprintf(gzFile file, സ്ഥിരം അക്ഷരം *format, ...) {
    va_list va;
    പൂർണ്ണസംഖ്യ ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    തിരികെ ret;
}

#അല്ലെങ്കിൽ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORTVA gzprintf(gzFile file, സ്ഥിരം അക്ഷരം *format, പൂർണ്ണസംഖ്യ a1, പൂർണ്ണസംഖ്യ a2, പൂർണ്ണസംഖ്യ a3,
                       പൂർണ്ണസംഖ്യ a4, പൂർണ്ണസംഖ്യ a5, പൂർണ്ണസംഖ്യ a6, പൂർണ്ണസംഖ്യ a7, പൂർണ്ണസംഖ്യ a8, പൂർണ്ണസംഖ്യ a9, പൂർണ്ണസംഖ്യ a10,
                       പൂർണ്ണസംഖ്യ a11, പൂർണ്ണസംഖ്യ a12, പൂർണ്ണസംഖ്യ a13, പൂർണ്ണസംഖ്യ a14, പൂർണ്ണസംഖ്യ a15, പൂർണ്ണസംഖ്യ a16,
                       പൂർണ്ണസംഖ്യ a17, പൂർണ്ണസംഖ്യ a18, പൂർണ്ണസംഖ്യ a19, പൂർണ്ണസംഖ്യ a20) {
#എങ്കിൽ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (ശൂന്യം)file, (ശൂന്യം)format, (ശൂന്യം)a1, (ശൂന്യം)a2, (ശൂന്യം)a3, (ശൂന്യം)a4, (ശൂന്യം)a5,
    (ശൂന്യം)a6, (ശൂന്യം)a7, (ശൂന്യം)a8, (ശൂന്യം)a9, (ശൂന്യം)a10, (ശൂന്യം)a11, (ശൂന്യം)a12,
    (ശൂന്യം)a13, (ശൂന്യം)a14, (ശൂന്യം)a15, (ശൂന്യം)a16, (ശൂന്യം)a17, (ശൂന്യം)a18,
    (ശൂന്യം)a19, (ശൂന്യം)a20;
    തിരികെ Z_STREAM_ERROR;
#അല്ലെങ്കിൽ
    പൂർണ്ണസംഖ്യ ret;
    unsigned len, ഇടത്;
    അക്ഷരം *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    എങ്കിൽ (വലുപ്പം(പൂർണ്ണസംഖ്യ) != വലുപ്പം(ശൂന്യം *))
        തിരികെ Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        തിരികെ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ശൂന്യ);

    /* make sure we have some buffer space */
    എങ്കിൽ (state->size == 0 && gz_init(state) == -1)
        തിരികെ state->err;

    /* check for seek request */
    എങ്കിൽ (state->skip && gz_zero(state) == -1)
        തിരികെ state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    എങ്കിൽ (state->err) {
        എങ്കിൽ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            തിരികെ state->err;
        }
        എങ്കിൽ (!state->again)
            തിരികെ state->err;
    }
    എങ്കിൽ (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (അക്ഷരം *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    വേണ്ടി (len = 0; len < size; len++)
        എങ്കിൽ (next[len] == 0)
            നിർത്തുക;
#  അല്ലെങ്കിൽ
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#അല്ലെങ്കിൽ
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  അല്ലെങ്കിൽ
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    എങ്കിൽ (len == 0 || len >= state->size || next[state->size - 1] != 0)
        തിരികെ 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    എങ്കിൽ (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        എങ്കിൽ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        തിരികെ state->err;
    }
    എങ്കിൽ (state->err && !state->again)
        തിരികെ state->err;
    തിരികെ (പൂർണ്ണസംഖ്യ)len;
#endif
}

#endif

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzflush(gzFile file, പൂർണ്ണസംഖ്യ flush) {
    gz_statep state;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        തിരികെ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ശൂന്യ);

    /* check flush parameter */
    എങ്കിൽ (flush < 0 || flush > Z_FINISH)
        തിരികെ Z_STREAM_ERROR;

    /* check for seek request */
    എങ്കിൽ (state->skip && gz_zero(state) == -1)
        തിരികെ state->err;

    /* compress remaining data with requested flush */
    (ശൂന്യം)gz_comp(state, flush);
    തിരികെ state->err;
}

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzsetparams(gzFile file, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    എങ്കിൽ (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        തിരികെ Z_STREAM_ERROR;
    gz_error(state, Z_OK, ശൂന്യ);

    /* if no change is requested, then do nothing */
    എങ്കിൽ (level == state->level && strategy == state->strategy)
        തിരികെ Z_OK;

    /* check for seek request */
    എങ്കിൽ (state->skip && gz_zero(state) == -1)
        തിരികെ state->err;

    /* change compression parameters for subsequent input */
    എങ്കിൽ (state->size) {
        /* flush previous input with previous parameters before changing */
        എങ്കിൽ (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            തിരികെ state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    തിരികെ Z_OK;
}

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzclose_w(gzFile file) {
    പൂർണ്ണസംഖ്യ ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    എങ്കിൽ (state->mode != GZ_WRITE)
        തിരികെ Z_STREAM_ERROR;

    /* check for seek request */
    എങ്കിൽ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    എങ്കിൽ (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    എങ്കിൽ (state->size) {
        എങ്കിൽ (!state->direct) {
            (ശൂന്യം)deflateEnd(&(state->strm));
            സ്വതന്ത്രം(state->out);
        }
        സ്വതന്ത്രം(state->in);
    }
    gz_error(state, Z_OK, ശൂന്യ);
    സ്വതന്ത്രം(state->path);
    എങ്കിൽ (close(state->fd) == -1)
        ret = Z_ERRNO;
    സ്വതന്ത്രം(state);
    തിരികെ ret;
}
