/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local पूर्णाङ्क gz_init(gz_statep state) {
    पूर्णाङ्क ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned वर्ण *)malloc(state->want << 1);
    सचे (state->in == रिक्त) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        पटिनिवत्त -1;
    }

    /* only need output buffer and deflate state if compressing */
    सचे (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned वर्ण *)malloc(state->want);
        सचे (state->out == रिक्त) {
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            पटिनिवत्त -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        सचे (ret != Z_OK) {
            मुक्त(state->out);
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            पटिनिवत्त -1;
        }
        strm->next_in = रिक्त;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    सचे (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    पटिनिवत्त 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local पूर्णाङ्क gz_comp(gz_statep state, पूर्णाङ्क flush) {
    पूर्णाङ्क ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    सचे (state->size == 0 && gz_init(state) == -1)
        पटिनिवत्त -1;

    /* write directly if requested */
    सचे (state->direct) {
        याव (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (पूर्णाङ्क)लिख(state->fd, strm->next_in, put);
            सचे (writ < 0) {
                सचे (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                पटिनिवत्त -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        पटिनिवत्त 0;
    }

    /* check for a pending reset */
    सचे (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        सचे (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            पटिनिवत्त 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    करोहि {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        सचे (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            याव (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (पूर्णाङ्क)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (पूर्णाङ्क)लिख(state->fd, state->x.next, put);
                सचे (writ < 0) {
                    सचे (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    पटिनिवत्त -1;
                }
                state->x.next += writ;
            }
            सचे (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        सचे (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            पटिनिवत्त -1;
        }
        have -= strm->avail_out;
    } याव (have);

    /* if that completed a deflate stream, allow another to start */
    सचे (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    पटिनिवत्त 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local पूर्णाङ्क gz_zero(gz_statep state) {
    पूर्णाङ्क first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    सचे (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        पटिनिवत्त -1;

    /* compress state->skip zeros */
    first = 1;
    करोहि {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        सचे (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        सचे (ret == -1)
            पटिनिवत्त -1;
    } याव (state->skip);
    पटिनिवत्त 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    पूर्णाङ्क ret;

    /* if len is zero, avoid unnecessary operations */
    सचे (len == 0)
        पटिनिवत्त 0;

    /* allocate memory if this is the first time through */
    सचे (state->size == 0 && gz_init(state) == -1)
        पटिनिवत्त 0;

    /* check for seek request */
    सचे (state->skip && gz_zero(state) == -1)
        पटिनिवत्त 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    सचे (len < state->size) {
        /* copy to input buffer, compress when full */
        अत्थाय (;;) {
            unsigned have, copy;

            सचे (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            सचे (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (ध्रुव वर्ण *)buf + copy;
            len -= copy;
            सचे (len == 0)
                भङ्ग;
            सचे (gz_comp(state, Z_NO_FLUSH) == -1)
                पटिनिवत्त state->again ? put - len : 0;
        }
    }
    अञ्ञथा {
        /* consume whatever's left in the input buffer */
        सचे (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            पटिनिवत्त 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        करोहि {
            unsigned n = (unsigned)-1;

            सचे (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            सचे (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                पटिनिवत्त state->again ? put - len : 0;
            }
        } याव (len);
    }

    /* input was all buffered or compressed */
    पटिनिवत्त put;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        पटिनिवत्त 0;
    gz_error(state, Z_OK, रिक्त);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    सचे ((पूर्णाङ्क)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        पटिनिवत्त 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    पटिनिवत्त (पूर्णाङ्क)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        पटिनिवत्त 0;
    gz_error(state, Z_OK, रिक्त);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    सचे (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        पटिनिवत्त 0;
    }

    /* write len bytes to buf, return the number of full items written */
    पटिनिवत्त len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzputc(gzFile file, पूर्णाङ्क c) {
    unsigned have;
    unsigned वर्ण buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        पटिनिवत्त -1;
    gz_error(state, Z_OK, रिक्त);

    /* check for seek request */
    सचे (state->skip && gz_zero(state) == -1)
        पटिनिवत्त -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    सचे (state->size) {
        सचे (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        सचे (have < state->size) {
            state->in[have] = (unsigned वर्ण)c;
            strm->avail_in++;
            state->x.pos++;
            पटिनिवत्त c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned वर्ण)c;
    सचे (gz_write(state, buf, 1) != 1)
        पटिनिवत्त -1;
    पटिनिवत्त c & 0xff;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzputs(gzFile file, ध्रुव वर्ण *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        पटिनिवत्त -1;
    gz_error(state, Z_OK, रिक्त);

    /* write string */
    len = strlen(s);
    सचे ((पूर्णाङ्क)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        पटिनिवत्त -1;
    }
    put = gz_write(state, s, len);
    पटिनिवत्त len && put == 0 ? -1 : (पूर्णाङ्क)put;
}

#सचे (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local पूर्णाङ्क gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    सचे (strm->next_in == रिक्त ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        पटिनिवत्त 0;
    (शून्य)gz_comp(state, Z_NO_FLUSH);
    सचे (strm->avail_in == 0) {
        strm->next_in = state->in;
        पटिनिवत्त 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    पटिनिवत्त strm->avail_in > state->size;
}
#endif

#सचे defined(STDC) || defined(Z_HAVE_STDARG_H)
#अन्तर्भाव <stdarg.h>

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORTVA gzvprintf(gzFile file, ध्रुव वर्ण *format, va_list va) {
#सचे defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (शून्य)file, (शून्य)format, (शून्य)va;
    पटिनिवत्त Z_STREAM_ERROR;
#अञ्ञथा
    पूर्णाङ्क len, ret;
    वर्ण *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        पटिनिवत्त Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* make sure we have some buffer space */
    सचे (state->size == 0 && gz_init(state) == -1)
        पटिनिवत्त state->err;

    /* check for seek request */
    सचे (state->skip && gz_zero(state) == -1)
        पटिनिवत्त state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    सचे (state->err) {
        सचे (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            पटिनिवत्त state->err;
        }
        सचे (!state->again)
            पटिनिवत्त state->err;
    }
    सचे (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (वर्ण *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (शून्य)vsprintf(next, format, va);
    अत्थाय (len = 0; len < state->size; len++)
        सचे (next[len] == 0) भङ्ग;
#  अञ्ञथा
    len = vsprintf(next, format, va);
#  endif
#अञ्ञथा
#  ifdef HAS_vsnprintf_void
    (शून्य)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  अञ्ञथा
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    सचे (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        पटिनिवत्त 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    सचे (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        सचे (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        पटिनिवत्त state->err;
    }
    सचे (state->err && !state->again)
        पटिनिवत्त state->err;
    पटिनिवत्त len;
#endif
}

पूर्णाङ्क ZEXPORTVA gzprintf(gzFile file, ध्रुव वर्ण *format, ...) {
    va_list va;
    पूर्णाङ्क ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    पटिनिवत्त ret;
}

#अञ्ञथा /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORTVA gzprintf(gzFile file, ध्रुव वर्ण *format, पूर्णाङ्क a1, पूर्णाङ्क a2, पूर्णाङ्क a3,
                       पूर्णाङ्क a4, पूर्णाङ्क a5, पूर्णाङ्क a6, पूर्णाङ्क a7, पूर्णाङ्क a8, पूर्णाङ्क a9, पूर्णाङ्क a10,
                       पूर्णाङ्क a11, पूर्णाङ्क a12, पूर्णाङ्क a13, पूर्णाङ्क a14, पूर्णाङ्क a15, पूर्णाङ्क a16,
                       पूर्णाङ्क a17, पूर्णाङ्क a18, पूर्णाङ्क a19, पूर्णाङ्क a20) {
#सचे defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (शून्य)file, (शून्य)format, (शून्य)a1, (शून्य)a2, (शून्य)a3, (शून्य)a4, (शून्य)a5,
    (शून्य)a6, (शून्य)a7, (शून्य)a8, (शून्य)a9, (शून्य)a10, (शून्य)a11, (शून्य)a12,
    (शून्य)a13, (शून्य)a14, (शून्य)a15, (शून्य)a16, (शून्य)a17, (शून्य)a18,
    (शून्य)a19, (शून्य)a20;
    पटिनिवत्त Z_STREAM_ERROR;
#अञ्ञथा
    पूर्णाङ्क ret;
    unsigned len, वामः;
    वर्ण *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    सचे (परिमाण(पूर्णाङ्क) != परिमाण(शून्य *))
        पटिनिवत्त Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        पटिनिवत्त Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* make sure we have some buffer space */
    सचे (state->size == 0 && gz_init(state) == -1)
        पटिनिवत्त state->err;

    /* check for seek request */
    सचे (state->skip && gz_zero(state) == -1)
        पटिनिवत्त state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    सचे (state->err) {
        सचे (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            पटिनिवत्त state->err;
        }
        सचे (!state->again)
            पटिनिवत्त state->err;
    }
    सचे (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (वर्ण *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    अत्थाय (len = 0; len < size; len++)
        सचे (next[len] == 0)
            भङ्ग;
#  अञ्ञथा
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#अञ्ञथा
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  अञ्ञथा
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    सचे (len == 0 || len >= state->size || next[state->size - 1] != 0)
        पटिनिवत्त 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    सचे (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        सचे (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        पटिनिवत्त state->err;
    }
    सचे (state->err && !state->again)
        पटिनिवत्त state->err;
    पटिनिवत्त (पूर्णाङ्क)len;
#endif
}

#endif

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzflush(gzFile file, पूर्णाङ्क flush) {
    gz_statep state;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        पटिनिवत्त Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* check flush parameter */
    सचे (flush < 0 || flush > Z_FINISH)
        पटिनिवत्त Z_STREAM_ERROR;

    /* check for seek request */
    सचे (state->skip && gz_zero(state) == -1)
        पटिनिवत्त state->err;

    /* compress remaining data with requested flush */
    (शून्य)gz_comp(state, flush);
    पटिनिवत्त state->err;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzsetparams(gzFile file, पूर्णाङ्क level, पूर्णाङ्क strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    सचे (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        पटिनिवत्त Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* if no change is requested, then do nothing */
    सचे (level == state->level && strategy == state->strategy)
        पटिनिवत्त Z_OK;

    /* check for seek request */
    सचे (state->skip && gz_zero(state) == -1)
        पटिनिवत्त state->err;

    /* change compression parameters for subsequent input */
    सचे (state->size) {
        /* flush previous input with previous parameters before changing */
        सचे (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            पटिनिवत्त state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    पटिनिवत्त Z_OK;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzclose_w(gzFile file) {
    पूर्णाङ्क ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    सचे (file == रिक्त)
        पटिनिवत्त Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    सचे (state->mode != GZ_WRITE)
        पटिनिवत्त Z_STREAM_ERROR;

    /* check for seek request */
    सचे (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    सचे (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    सचे (state->size) {
        सचे (!state->direct) {
            (शून्य)deflateEnd(&(state->strm));
            मुक्त(state->out);
        }
        मुक्त(state->in);
    }
    gz_error(state, Z_OK, रिक्त);
    मुक्त(state->path);
    सचे (close(state->fd) == -1)
        ret = Z_ERRNO;
    मुक्त(state);
    पटिनिवत्त ret;
}
