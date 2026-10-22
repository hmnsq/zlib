/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#समावेश "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local पूर्णांक gz_init(gz_statep state) {
    पूर्णांक ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (अचिन्हित अक्षर *)दो_स्मृति(state->want << 1);
    अगर (state->in == शून्य) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        वापस -1;
    }

    /* only need output buffer and deflate state if compressing */
    अगर (!state->direct) {
        /* allocate output buffer */
        state->out = (अचिन्हित अक्षर *)दो_स्मृति(state->want);
        अगर (state->out == शून्य) {
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            वापस -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        अगर (ret != Z_OK) {
            मुक्त(state->out);
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            वापस -1;
        }
        strm->अग्र_in = शून्य;
    }

    /* mark state as initialized */
    state->आकार = state->want;

    /* initialize write buffer if compressing */
    अगर (!state->direct) {
        strm->avail_out = state->आकार;
        strm->अग्र_out = state->out;
        state->x.अग्र = strm->अग्र_out;
    }
    वापस 0;
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
    अगर (state->आकार == 0 && gz_init(state) == -1)
        वापस -1;

    /* write directly if requested */
    अगर (state->direct) {
        जबतक (strm->avail_in) {
            त्रुटि_सं = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (पूर्णांक)ग_लिखो(state->fd, strm->अग्र_in, put);
            अगर (writ < 0) {
                अगर (BLOCKED(त्रुटि_सं))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                वापस -1;
            }
            strm->avail_in -= (अचिन्हित)writ;
            strm->अग्र_in += writ;
        }
        वापस 0;
    }

    /* check for a pending reset */
    अगर (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        अगर (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            वापस 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    करो {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        अगर (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            जबतक (strm->अग्र_out > state->x.अग्र) {
                त्रुटि_सं = 0;
                state->again = 0;
                put = strm->अग्र_out - state->x.अग्र > (पूर्णांक)max ? max :
                      (अचिन्हित)(strm->अग्र_out - state->x.अग्र);
                writ = (पूर्णांक)ग_लिखो(state->fd, state->x.अग्र, put);
                अगर (writ < 0) {
                    अगर (BLOCKED(त्रुटि_सं))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    वापस -1;
                }
                state->x.अग्र += writ;
            }
            अगर (strm->avail_out == 0) {
                strm->avail_out = state->आकार;
                strm->अग्र_out = state->out;
                state->x.अग्र = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        अगर (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            वापस -1;
        }
        have -= strm->avail_out;
    } जबतक (have);

    /* if that completed a deflate stream, allow another to start */
    अगर (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    वापस 0;
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
    अगर (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        वापस -1;

    /* compress state->skip zeros */
    first = 1;
    करो {
        n = GT_OFF(state->आकार) || (z_off64_t)state->आकार > state->skip ?
            (अचिन्हित)state->skip : state->आकार;
        अगर (first) {
            स_रखो(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->अग्र_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        अगर (ret == -1)
            वापस -1;
    } जबतक (state->skip);
    वापस 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_आकार_t gz_write(gz_statep state, voidpc buf, z_आकार_t len) {
    z_आकार_t put = len;
    पूर्णांक ret;

    /* if len is zero, avoid unnecessary operations */
    अगर (len == 0)
        वापस 0;

    /* allocate memory if this is the first time through */
    अगर (state->आकार == 0 && gz_init(state) == -1)
        वापस 0;

    /* check for seek request */
    अगर (state->skip && gz_zero(state) == -1)
        वापस 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    अगर (len < state->आकार) {
        /* copy to input buffer, compress when full */
        क्रम (;;) {
            अचिन्हित have, copy;

            अगर (state->strm.avail_in == 0)
                state->strm.अग्र_in = state->in;
            have = (अचिन्हित)((state->strm.अग्र_in + state->strm.avail_in) -
                              state->in);
            copy = state->आकार - have;
            अगर (copy > len)
                copy = (अचिन्हित)len;
            स_नकल(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (स्थिर अक्षर *)buf + copy;
            len -= copy;
            अगर (len == 0)
                अवरोध;
            अगर (gz_comp(state, Z_NO_FLUSH) == -1)
                वापस state->again ? put - len : 0;
        }
    }
    अन्यथा {
        /* consume whatever's left in the input buffer */
        अगर (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            वापस 0;

        /* directly compress user buffer to file */
        state->strm.अग्र_in = (z_const Bytef *)buf;
        करो {
            अचिन्हित n = (अचिन्हित)-1;

            अगर (n > len)
                n = (अचिन्हित)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            अगर (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.अग्र_in = state->in;
                वापस state->again ? put - len : 0;
            }
        } जबतक (len);
    }

    /* input was all buffered or compressed */
    वापस put;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzwrite(gzFile file, voidpc buf, अचिन्हित len) {
    gz_statep state;

    /* get internal structure */
    अगर (file == शून्य)
        वापस 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        वापस 0;
    gz_error(state, Z_OK, शून्य);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    अगर ((पूर्णांक)len < 0) {
        gz_error(state, Z_तथ्य_ERROR, "requested length does not fit in int");
        वापस 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    वापस (पूर्णांक)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_आकार_t ZEXPORT gzfwrite(voidpc buf, z_आकार_t आकार, z_आकार_t nitems,
                          gzFile file) {
    z_आकार_t len;
    gz_statep state;

    /* get internal structure */
    अगर (file == शून्य)
        वापस 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        वापस 0;
    gz_error(state, Z_OK, शून्य);

    /* compute bytes to read -- error on overflow */
    len = nitems * आकार;
    अगर (आकार && len / आकार != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        वापस 0;
    }

    /* write len bytes to buf, return the number of full items written */
    वापस len ? gz_write(state, buf, len) / आकार : 0;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzputc(gzFile file, पूर्णांक c) {
    अचिन्हित have;
    अचिन्हित अक्षर buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        वापस -1;
    gz_error(state, Z_OK, शून्य);

    /* check for seek request */
    अगर (state->skip && gz_zero(state) == -1)
        वापस -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    अगर (state->आकार) {
        अगर (strm->avail_in == 0)
            strm->अग्र_in = state->in;
        have = (अचिन्हित)((strm->अग्र_in + strm->avail_in) - state->in);
        अगर (have < state->आकार) {
            state->in[have] = (अचिन्हित अक्षर)c;
            strm->avail_in++;
            state->x.pos++;
            वापस c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (अचिन्हित अक्षर)c;
    अगर (gz_write(state, buf, 1) != 1)
        वापस -1;
    वापस c & 0xff;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzputs(gzFile file, स्थिर अक्षर *s) {
    z_आकार_t len, put;
    gz_statep state;

    /* get internal structure */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        वापस -1;
    gz_error(state, Z_OK, शून्य);

    /* write string */
    len = म_माप(s);
    अगर ((पूर्णांक)len < 0 || (अचिन्हित)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        वापस -1;
    }
    put = gz_write(state, s, len);
    वापस len && put == 0 ? -1 : (पूर्णांक)put;
}

#अगर (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
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
    अगर (strm->अग्र_in == शून्य ||
        strm->अग्र_in + strm->avail_in <= state->in + state->आकार)
        वापस 0;
    (व्योम)gz_comp(state, Z_NO_FLUSH);
    अगर (strm->avail_in == 0) {
        strm->अग्र_in = state->in;
        वापस 0;
    }
    स_हटाओ(state->in, strm->अग्र_in, strm->avail_in);
    strm->अग्र_in = state->in;
    वापस strm->avail_in > state->आकार;
}
#पूर्ण_अगर

#अगर defined(STDC) || defined(Z_HAVE_STDARG_H)
#समावेश <stdarg.h>

/* -- see zlib.h -- */
पूर्णांक ZEXPORTVA gzvprintf(gzFile file, स्थिर अक्षर *format, बहु_सूची va) {
#अगर defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (व्योम)file, (व्योम)format, (व्योम)va;
    वापस Z_STREAM_ERROR;
#अन्यथा
    पूर्णांक len, ret;
    अक्षर *अग्र;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    अगर (file == शून्य)
        वापस Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        वापस Z_STREAM_ERROR;
    gz_error(state, Z_OK, शून्य);

    /* make sure we have some buffer space */
    अगर (state->आकार == 0 && gz_init(state) == -1)
        वापस state->err;

    /* check for seek request */
    अगर (state->skip && gz_zero(state) == -1)
        वापस state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    अगर (state->err) {
        अगर (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            वापस state->err;
        }
        अगर (!state->again)
            वापस state->err;
    }
    अगर (strm->avail_in == 0)
        strm->अग्र_in = state->in;
    अग्र = (अक्षर *)(state->in + (strm->अग्र_in - state->in) + strm->avail_in);
    अग्र[state->आकार - 1] = 0;
#अगर_घोषित NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (व्योम)भम_लिखो(अग्र, format, va);
    क्रम (len = 0; len < state->आकार; len++)
        अगर (अग्र[len] == 0) अवरोध;
#  अन्यथा
    len = भम_लिखो(अग्र, format, va);
#  endif
#अन्यथा
#  ifdef HAS_vsnprintf_void
    (व्योम)vsnprintf(अग्र, state->आकार, format, va);
    len = म_माप(अग्र);
#  अन्यथा
    len = vsnprintf(अग्र, state->आकार, format, va);
#  endif
#पूर्ण_अगर

    /* check that printf() results fit in buffer */
    अगर (len == 0 || (अचिन्हित)len >= state->आकार || अग्र[state->आकार - 1] != 0)
        वापस 0;

    /* update buffer and position */
    strm->avail_in += (अचिन्हित)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    अगर (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (अचिन्हित)len;
        state->x.pos -= len;
        अगर (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        वापस state->err;
    }
    अगर (state->err && !state->again)
        वापस state->err;
    वापस len;
#पूर्ण_अगर
}

पूर्णांक ZEXPORTVA gzprintf(gzFile file, स्थिर अक्षर *format, ...) {
    बहु_सूची va;
    पूर्णांक ret;

    बहु_शुरू(va, format);
    ret = gzvprintf(file, format, va);
    बहु_पूर्ण(va);
    वापस ret;
}

#अन्यथा /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
पूर्णांक ZEXPORTVA gzprintf(gzFile file, स्थिर अक्षर *format, पूर्णांक a1, पूर्णांक a2, पूर्णांक a3,
                       पूर्णांक a4, पूर्णांक a5, पूर्णांक a6, पूर्णांक a7, पूर्णांक a8, पूर्णांक a9, पूर्णांक a10,
                       पूर्णांक a11, पूर्णांक a12, पूर्णांक a13, पूर्णांक a14, पूर्णांक a15, पूर्णांक a16,
                       पूर्णांक a17, पूर्णांक a18, पूर्णांक a19, पूर्णांक a20) {
#अगर defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (व्योम)file, (व्योम)format, (व्योम)a1, (व्योम)a2, (व्योम)a3, (व्योम)a4, (व्योम)a5,
    (व्योम)a6, (व्योम)a7, (व्योम)a8, (व्योम)a9, (व्योम)a10, (व्योम)a11, (व्योम)a12,
    (व्योम)a13, (व्योम)a14, (व्योम)a15, (व्योम)a16, (व्योम)a17, (व्योम)a18,
    (व्योम)a19, (व्योम)a20;
    वापस Z_STREAM_ERROR;
#अन्यथा
    पूर्णांक ret;
    अचिन्हित len, बायाँ;
    अक्षर *अग्र;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    अगर (file == शून्य)
        वापस Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    अगर (माप(पूर्णांक) != माप(व्योम *))
        वापस Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        वापस Z_STREAM_ERROR;
    gz_error(state, Z_OK, शून्य);

    /* make sure we have some buffer space */
    अगर (state->आकार == 0 && gz_init(state) == -1)
        वापस state->err;

    /* check for seek request */
    अगर (state->skip && gz_zero(state) == -1)
        वापस state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    अगर (state->err) {
        अगर (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            वापस state->err;
        }
        अगर (!state->again)
            वापस state->err;
    }
    अगर (strm->avail_in == 0)
        strm->अग्र_in = state->in;
    अग्र = (अक्षर *)(strm->अग्र_in + strm->avail_in);
    अग्र[state->आकार - 1] = 0;
#अगर_घोषित NO_snprintf
#  ifdef HAS_sprintf_void
    प्र_लिखो(अग्र, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    क्रम (len = 0; len < आकार; len++)
        अगर (अग्र[len] == 0)
            अवरोध;
#  अन्यथा
    len = प्र_लिखो(अग्र, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#अन्यथा
#  ifdef HAS_snprintf_void
    snprintf(अग्र, state->आकार, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = म_माप(अग्र);
#  अन्यथा
    len = snprintf(अग्र, state->आकार, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#पूर्ण_अगर

    /* check that printf() results fit in buffer */
    अगर (len == 0 || len >= state->आकार || अग्र[state->आकार - 1] != 0)
        वापस 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    अगर (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (अचिन्हित)len;
        state->x.pos -= len;
        अगर (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        वापस state->err;
    }
    अगर (state->err && !state->again)
        वापस state->err;
    वापस (पूर्णांक)len;
#पूर्ण_अगर
}

#पूर्ण_अगर

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzflush(gzFile file, पूर्णांक flush) {
    gz_statep state;

    /* get internal structure */
    अगर (file == शून्य)
        वापस Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        वापस Z_STREAM_ERROR;
    gz_error(state, Z_OK, शून्य);

    /* check flush parameter */
    अगर (flush < 0 || flush > Z_FINISH)
        वापस Z_STREAM_ERROR;

    /* check for seek request */
    अगर (state->skip && gz_zero(state) == -1)
        वापस state->err;

    /* compress remaining data with requested flush */
    (व्योम)gz_comp(state, flush);
    वापस state->err;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzsetparams(gzFile file, पूर्णांक level, पूर्णांक strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    अगर (file == शून्य)
        वापस Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    अगर (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        वापस Z_STREAM_ERROR;
    gz_error(state, Z_OK, शून्य);

    /* if no change is requested, then do nothing */
    अगर (level == state->level && strategy == state->strategy)
        वापस Z_OK;

    /* check for seek request */
    अगर (state->skip && gz_zero(state) == -1)
        वापस state->err;

    /* change compression parameters for subsequent input */
    अगर (state->आकार) {
        /* flush previous input with previous parameters before changing */
        अगर (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            वापस state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    वापस Z_OK;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzclose_w(gzFile file) {
    पूर्णांक ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    अगर (file == शून्य)
        वापस Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    अगर (state->mode != GZ_WRITE)
        वापस Z_STREAM_ERROR;

    /* check for seek request */
    अगर (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    अगर (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    अगर (state->आकार) {
        अगर (!state->direct) {
            (व्योम)deflateEnd(&(state->strm));
            मुक्त(state->out);
        }
        मुक्त(state->in);
    }
    gz_error(state, Z_OK, शून्य);
    मुक्त(state->path);
    अगर (बंद(state->fd) == -1)
        ret = Z_ERRNO;
    मुक्त(state);
    वापस ret;
}
