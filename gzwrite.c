/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local पूर्णांक gz_init(gz_statep state) {
    पूर्णांक ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (अचिन्हित अक्षर *)दो_स्मृति(state->want << 1);
    जर (state->in == रिक्त) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        परत -1;
    }

    /* only need output buffer and deflate state if compressing */
    जर (!state->direct) {
        /* allocate output buffer */
        state->out = (अचिन्हित अक्षर *)दो_स्मृति(state->want);
        जर (state->out == रिक्त) {
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            परत -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        जर (ret != Z_OK) {
            मुक्त(state->out);
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            परत -1;
        }
        strm->next_in = रिक्त;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    जर (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    परत 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local पूर्णांक gz_comp(gz_statep state, पूर्णांक flush) {
    पूर्णांक ret, writ;
    अचिन्हित have, put, max = ((अचिन्हित)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    जर (state->size == 0 && gz_init(state) == -1)
        परत -1;

    /* write directly if requested */
    जर (state->direct) {
        पर्यंत (strm->avail_in) {
            त्रुटि_सं = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (पूर्णांक)लिख(state->fd, strm->next_in, put);
            जर (writ < 0) {
                जर (BLOCKED(त्रुटि_सं))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                परत -1;
            }
            strm->avail_in -= (अचिन्हित)writ;
            strm->next_in += writ;
        }
        परत 0;
    }

    /* check for a pending reset */
    जर (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        जर (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            परत 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    करा {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        जर (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            पर्यंत (strm->next_out > state->x.next) {
                त्रुटि_सं = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (पूर्णांक)max ? max :
                      (अचिन्हित)(strm->next_out - state->x.next);
                writ = (पूर्णांक)लिख(state->fd, state->x.next, put);
                जर (writ < 0) {
                    जर (BLOCKED(त्रुटि_सं))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    परत -1;
                }
                state->x.next += writ;
            }
            जर (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        जर (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            परत -1;
        }
        have -= strm->avail_out;
    } पर्यंत (have);

    /* if that completed a deflate stream, allow another to start */
    जर (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    परत 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local पूर्णांक gz_zero(gz_statep state) {
    पूर्णांक first, ret;
    अचिन्हित n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    जर (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        परत -1;

    /* compress state->skip zeros */
    first = 1;
    करा {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (अचिन्हित)state->skip : state->size;
        जर (first) {
            स_रखो(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        जर (ret == -1)
            परत -1;
    } पर्यंत (state->skip);
    परत 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    पूर्णांक ret;

    /* if len is zero, avoid unnecessary operations */
    जर (len == 0)
        परत 0;

    /* allocate memory if this is the first time through */
    जर (state->size == 0 && gz_init(state) == -1)
        परत 0;

    /* check for seek request */
    जर (state->skip && gz_zero(state) == -1)
        परत 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    जर (len < state->size) {
        /* copy to input buffer, compress when full */
        साठी (;;) {
            अचिन्हित have, copy;

            जर (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (अचिन्हित)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            जर (copy > len)
                copy = (अचिन्हित)len;
            स_नकल(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (स्थिरांक अक्षर *)buf + copy;
            len -= copy;
            जर (len == 0)
                थांबा;
            जर (gz_comp(state, Z_NO_FLUSH) == -1)
                परत state->again ? put - len : 0;
        }
    }
    अन्यथा {
        /* consume whatever's left in the input buffer */
        जर (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            परत 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        करा {
            अचिन्हित n = (अचिन्हित)-1;

            जर (n > len)
                n = (अचिन्हित)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            जर (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                परत state->again ? put - len : 0;
            }
        } पर्यंत (len);
    }

    /* input was all buffered or compressed */
    परत put;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzwrite(gzFile file, voidpc buf, अचिन्हित len) {
    gz_statep state;

    /* get internal structure */
    जर (file == रिक्त)
        परत 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        परत 0;
    gz_error(state, Z_OK, रिक्त);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    जर ((पूर्णांक)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        परत 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    परत (पूर्णांक)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    जर (file == रिक्त)
        परत 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        परत 0;
    gz_error(state, Z_OK, रिक्त);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    जर (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        परत 0;
    }

    /* write len bytes to buf, return the number of full items written */
    परत len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzputc(gzFile file, पूर्णांक c) {
    अचिन्हित have;
    अचिन्हित अक्षर buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    जर (file == रिक्त)
        परत -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        परत -1;
    gz_error(state, Z_OK, रिक्त);

    /* check for seek request */
    जर (state->skip && gz_zero(state) == -1)
        परत -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    जर (state->size) {
        जर (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (अचिन्हित)((strm->next_in + strm->avail_in) - state->in);
        जर (have < state->size) {
            state->in[have] = (अचिन्हित अक्षर)c;
            strm->avail_in++;
            state->x.pos++;
            परत c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (अचिन्हित अक्षर)c;
    जर (gz_write(state, buf, 1) != 1)
        परत -1;
    परत c & 0xff;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzputs(gzFile file, स्थिरांक अक्षर *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    जर (file == रिक्त)
        परत -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        परत -1;
    gz_error(state, Z_OK, रिक्त);

    /* write string */
    len = म_माप(s);
    जर ((पूर्णांक)len < 0 || (अचिन्हित)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        परत -1;
    }
    put = gz_write(state, s, len);
    परत len && put == 0 ? -1 : (पूर्णांक)put;
}

#जर (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local पूर्णांक gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    जर (strm->next_in == रिक्त ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        परत 0;
    (शून्य)gz_comp(state, Z_NO_FLUSH);
    जर (strm->avail_in == 0) {
        strm->next_in = state->in;
        परत 0;
    }
    स_हटाओ(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    परत strm->avail_in > state->size;
}
#endif

#जर defined(STDC) || defined(Z_HAVE_STDARG_H)
#अन्तर्भाव <stdarg.h>

/* -- see zlib.h -- */
पूर्णांक ZEXPORTVA gzvprintf(gzFile file, स्थिरांक अक्षर *format, बहु_सूची va) {
#जर defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (शून्य)file, (शून्य)format, (शून्य)va;
    परत Z_STREAM_ERROR;
#अन्यथा
    पूर्णांक len, ret;
    अक्षर *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    जर (file == रिक्त)
        परत Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        परत Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* make sure we have some buffer space */
    जर (state->size == 0 && gz_init(state) == -1)
        परत state->err;

    /* check for seek request */
    जर (state->skip && gz_zero(state) == -1)
        परत state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    जर (state->err) {
        जर (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            परत state->err;
        }
        जर (!state->again)
            परत state->err;
    }
    जर (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (अक्षर *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (शून्य)भम_लिखो(next, format, va);
    साठी (len = 0; len < state->size; len++)
        जर (next[len] == 0) थांबा;
#  अन्यथा
    len = भम_लिखो(next, format, va);
#  endif
#अन्यथा
#  ifdef HAS_vsnprintf_void
    (शून्य)vsnprintf(next, state->size, format, va);
    len = म_माप(next);
#  अन्यथा
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    जर (len == 0 || (अचिन्हित)len >= state->size || next[state->size - 1] != 0)
        परत 0;

    /* update buffer and position */
    strm->avail_in += (अचिन्हित)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    जर (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (अचिन्हित)len;
        state->x.pos -= len;
        जर (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        परत state->err;
    }
    जर (state->err && !state->again)
        परत state->err;
    परत len;
#endif
}

पूर्णांक ZEXPORTVA gzprintf(gzFile file, स्थिरांक अक्षर *format, ...) {
    बहु_सूची va;
    पूर्णांक ret;

    बहु_शुरू(va, format);
    ret = gzvprintf(file, format, va);
    बहु_पूर्ण(va);
    परत ret;
}

#अन्यथा /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
पूर्णांक ZEXPORTVA gzprintf(gzFile file, स्थिरांक अक्षर *format, पूर्णांक a1, पूर्णांक a2, पूर्णांक a3,
                       पूर्णांक a4, पूर्णांक a5, पूर्णांक a6, पूर्णांक a7, पूर्णांक a8, पूर्णांक a9, पूर्णांक a10,
                       पूर्णांक a11, पूर्णांक a12, पूर्णांक a13, पूर्णांक a14, पूर्णांक a15, पूर्णांक a16,
                       पूर्णांक a17, पूर्णांक a18, पूर्णांक a19, पूर्णांक a20) {
#जर defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (शून्य)file, (शून्य)format, (शून्य)a1, (शून्य)a2, (शून्य)a3, (शून्य)a4, (शून्य)a5,
    (शून्य)a6, (शून्य)a7, (शून्य)a8, (शून्य)a9, (शून्य)a10, (शून्य)a11, (शून्य)a12,
    (शून्य)a13, (शून्य)a14, (शून्य)a15, (शून्य)a16, (शून्य)a17, (शून्य)a18,
    (शून्य)a19, (शून्य)a20;
    परत Z_STREAM_ERROR;
#अन्यथा
    पूर्णांक ret;
    अचिन्हित len, डावा;
    अक्षर *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    जर (file == रिक्त)
        परत Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    जर (आकार(पूर्णांक) != आकार(शून्य *))
        परत Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        परत Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* make sure we have some buffer space */
    जर (state->size == 0 && gz_init(state) == -1)
        परत state->err;

    /* check for seek request */
    जर (state->skip && gz_zero(state) == -1)
        परत state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    जर (state->err) {
        जर (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            परत state->err;
        }
        जर (!state->again)
            परत state->err;
    }
    जर (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (अक्षर *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    प्र_लिखो(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    साठी (len = 0; len < size; len++)
        जर (next[len] == 0)
            थांबा;
#  अन्यथा
    len = प्र_लिखो(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#अन्यथा
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = म_माप(next);
#  अन्यथा
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    जर (len == 0 || len >= state->size || next[state->size - 1] != 0)
        परत 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    जर (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (अचिन्हित)len;
        state->x.pos -= len;
        जर (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        परत state->err;
    }
    जर (state->err && !state->again)
        परत state->err;
    परत (पूर्णांक)len;
#endif
}

#endif

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzflush(gzFile file, पूर्णांक flush) {
    gz_statep state;

    /* get internal structure */
    जर (file == रिक्त)
        परत Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        परत Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* check flush parameter */
    जर (flush < 0 || flush > Z_FINISH)
        परत Z_STREAM_ERROR;

    /* check for seek request */
    जर (state->skip && gz_zero(state) == -1)
        परत state->err;

    /* compress remaining data with requested flush */
    (शून्य)gz_comp(state, flush);
    परत state->err;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzsetparams(gzFile file, पूर्णांक level, पूर्णांक strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    जर (file == रिक्त)
        परत Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    जर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        परत Z_STREAM_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* if no change is requested, then do nothing */
    जर (level == state->level && strategy == state->strategy)
        परत Z_OK;

    /* check for seek request */
    जर (state->skip && gz_zero(state) == -1)
        परत state->err;

    /* change compression parameters for subsequent input */
    जर (state->size) {
        /* flush previous input with previous parameters before changing */
        जर (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            परत state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    परत Z_OK;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzclose_w(gzFile file) {
    पूर्णांक ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    जर (file == रिक्त)
        परत Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    जर (state->mode != GZ_WRITE)
        परत Z_STREAM_ERROR;

    /* check for seek request */
    जर (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    जर (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    जर (state->size) {
        जर (!state->direct) {
            (शून्य)deflateEnd(&(state->strm));
            मुक्त(state->out);
        }
        मुक्त(state->in);
    }
    gz_error(state, Z_OK, रिक्त);
    मुक्त(state->path);
    जर (बंद(state->fd) == -1)
        ret = Z_ERRNO;
    मुक्त(state);
    परत ret;
}
