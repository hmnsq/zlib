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
    state->अन्तर् = (अचिह्नित वर्ण *)स्मृतिं_देहि(state->want << 1);
    यदि (state->अन्तर् == रिक्त) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        प्रत्यागम -1;
    }

    /* only need output buffer and deflate state if compressing */
    यदि (!state->प्रत्यक्षम्) {
        /* allocate output buffer */
        state->बहिर्गमनम् = (अचिह्नित वर्ण *)स्मृतिं_देहि(state->want);
        यदि (state->बहिर्गमनम् == रिक्त) {
            मुक्त(state->अन्तर्);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            प्रत्यागम -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        यदि (ret != Z_OK) {
            मुक्त(state->बहिर्गमनम्);
            मुक्त(state->अन्तर्);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            प्रत्यागम -1;
        }
        strm->अग्र_अन्तर् = रिक्त;
    }

    /* mark state as initialized */
    state->आकार = state->want;

    /* initialize write buffer if compressing */
    यदि (!state->प्रत्यक्षम्) {
        strm->avail_बहिर्गमनम् = state->आकार;
        strm->अग्र_बहिर्गमनम् = state->बहिर्गमनम्;
        state->x.अग्र = strm->अग्र_बहिर्गमनम्;
    }
    प्रत्यागम 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local पूर्णाङ्क gz_comp(gz_statep state, पूर्णाङ्क flush) {
    पूर्णाङ्क ret, writ;
    अचिह्नित have, put, max = ((अचिह्नित)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    यदि (state->आकार == 0 && gz_init(state) == -1)
        प्रत्यागम -1;

    /* write directly if requested */
    यदि (state->प्रत्यक्षम्) {
        यावत् (strm->avail_अन्तर्) {
            errno = 0;
            state->again = 0;
            put = strm->avail_अन्तर् > max ? max : strm->avail_अन्तर्;
            writ = (पूर्णाङ्क)लिख(state->fd, strm->अग्र_अन्तर्, put);
            यदि (writ < 0) {
                यदि (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                प्रत्यागम -1;
            }
            strm->avail_अन्तर् -= (अचिह्नित)writ;
            strm->अग्र_अन्तर् += writ;
        }
        प्रत्यागम 0;
    }

    /* check for a pending reset */
    यदि (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        यदि (strm->avail_अन्तर् == 0 && flush == Z_NO_FLUSH)
            प्रत्यागम 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    कुरु {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        यदि (strm->avail_बहिर्गमनम् == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_प्रवाहः_अन्तः))) {
            यावत् (strm->अग्र_बहिर्गमनम् > state->x.अग्र) {
                errno = 0;
                state->again = 0;
                put = strm->अग्र_बहिर्गमनम् - state->x.अग्र > (पूर्णाङ्क)max ? max :
                      (अचिह्नित)(strm->अग्र_बहिर्गमनम् - state->x.अग्र);
                writ = (पूर्णाङ्क)लिख(state->fd, state->x.अग्र, put);
                यदि (writ < 0) {
                    यदि (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    प्रत्यागम -1;
                }
                state->x.अग्र += writ;
            }
            यदि (strm->avail_बहिर्गमनम् == 0) {
                strm->avail_बहिर्गमनम् = state->आकार;
                strm->अग्र_बहिर्गमनम् = state->बहिर्गमनम्;
                state->x.अग्र = state->बहिर्गमनम्;
            }
        }

        /* compress */
        have = strm->avail_बहिर्गमनम्;
        ret = deflate(strm, flush);
        यदि (ret == Z_प्रवाहः_ERROR) {
            gz_error(state, Z_प्रवाहः_ERROR,
                      "internal error: deflate stream corrupt");
            प्रत्यागम -1;
        }
        have -= strm->avail_बहिर्गमनम्;
    } यावत् (have);

    /* if that completed a deflate stream, allow another to start */
    यदि (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    प्रत्यागम 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local पूर्णाङ्क gz_zero(gz_statep state) {
    पूर्णाङ्क first, ret;
    अचिह्नित n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    यदि (strm->avail_अन्तर् && gz_comp(state, Z_NO_FLUSH) == -1)
        प्रत्यागम -1;

    /* compress state->skip zeros */
    first = 1;
    कुरु {
        n = GT_OFF(state->आकार) || (z_off64_t)state->आकार > state->skip ?
            (अचिह्नित)state->skip : state->आकार;
        यदि (first) {
            स्मृतिं_पूरय(state->अन्तर्, 0, n);
            first = 0;
        }
        strm->avail_अन्तर् = n;
        strm->अग्र_अन्तर् = state->अन्तर्;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_अन्तर्;
        state->x.pos += n;
        state->skip -= n;
        यदि (ret == -1)
            प्रत्यागम -1;
    } यावत् (state->skip);
    प्रत्यागम 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_आकार_t gz_write(gz_statep state, voidpc buf, z_आकार_t len) {
    z_आकार_t put = len;
    पूर्णाङ्क ret;

    /* if len is zero, avoid unnecessary operations */
    यदि (len == 0)
        प्रत्यागम 0;

    /* allocate memory if this is the first time through */
    यदि (state->आकार == 0 && gz_init(state) == -1)
        प्रत्यागम 0;

    /* check for seek request */
    यदि (state->skip && gz_zero(state) == -1)
        प्रत्यागम 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    यदि (len < state->आकार) {
        /* copy to input buffer, compress when full */
        प्रति (;;) {
            अचिह्नित have, copy;

            यदि (state->strm.avail_अन्तर् == 0)
                state->strm.अग्र_अन्तर् = state->अन्तर्;
            have = (अचिह्नित)((state->strm.अग्र_अन्तर् + state->strm.avail_अन्तर्) -
                              state->अन्तर्);
            copy = state->आकार - have;
            यदि (copy > len)
                copy = (अचिह्नित)len;
            स्मृतिं_प्रतिलिख(state->अन्तर् + have, buf, copy);
            state->strm.avail_अन्तर् += copy;
            state->x.pos += copy;
            buf = (ध्रुव वर्ण *)buf + copy;
            len -= copy;
            यदि (len == 0)
                भङ्ग;
            यदि (gz_comp(state, Z_NO_FLUSH) == -1)
                प्रत्यागम state->again ? put - len : 0;
        }
    }
    अन्यथा {
        /* consume whatever's left in the input buffer */
        यदि (state->strm.avail_अन्तर् && gz_comp(state, Z_NO_FLUSH) == -1)
            प्रत्यागम 0;

        /* directly compress user buffer to file */
        state->strm.अग्र_अन्तर् = (z_const Bytef *)buf;
        कुरु {
            अचिह्नित n = (अचिह्नित)-1;

            यदि (n > len)
                n = (अचिह्नित)len;
            state->strm.avail_अन्तर् = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_अन्तर्;
            state->x.pos += n;
            len -= n;
            यदि (ret == -1) {
                state->strm.avail_अन्तर् = 0;
                state->strm.अग्र_अन्तर् = state->अन्तर्;
                प्रत्यागम state->again ? put - len : 0;
            }
        } यावत् (len);
    }

    /* input was all buffered or compressed */
    प्रत्यागम put;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzwrite(gzFile file, voidpc buf, अचिह्नित len) {
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again))
        प्रत्यागम 0;
    gz_error(state, Z_OK, रिक्त);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    यदि ((पूर्णाङ्क)len < 0) {
        gz_error(state, Z_दत्त_ERROR, "requested length does not fit in int");
        प्रत्यागम 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    प्रत्यागम (पूर्णाङ्क)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_आकार_t ZEXPORT gzfwrite(voidpc buf, z_आकार_t आकार, z_आकार_t nitems,
                          gzFile file) {
    z_आकार_t len;
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again))
        प्रत्यागम 0;
    gz_error(state, Z_OK, रिक्त);

    /* compute bytes to read -- error on overflow */
    len = nitems * आकार;
    यदि (आकार && len / आकार != nitems) {
        gz_error(state, Z_प्रवाहः_ERROR, "request does not fit in a size_t");
        प्रत्यागम 0;
    }

    /* write len bytes to buf, return the number of full items written */
    प्रत्यागम len ? gz_write(state, buf, len) / आकार : 0;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzputc(gzFile file, पूर्णाङ्क c) {
    अचिह्नित have;
    अचिह्नित वर्ण buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again))
        प्रत्यागम -1;
    gz_error(state, Z_OK, रिक्त);

    /* check for seek request */
    यदि (state->skip && gz_zero(state) == -1)
        प्रत्यागम -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    यदि (state->आकार) {
        यदि (strm->avail_अन्तर् == 0)
            strm->अग्र_अन्तर् = state->अन्तर्;
        have = (अचिह्नित)((strm->अग्र_अन्तर् + strm->avail_अन्तर्) - state->अन्तर्);
        यदि (have < state->आकार) {
            state->अन्तर्[have] = (अचिह्नित वर्ण)c;
            strm->avail_अन्तर्++;
            state->x.pos++;
            प्रत्यागम c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (अचिह्नित वर्ण)c;
    यदि (gz_write(state, buf, 1) != 1)
        प्रत्यागम -1;
    प्रत्यागम c & 0xff;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzputs(gzFile file, ध्रुव वर्ण *s) {
    z_आकार_t len, put;
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again))
        प्रत्यागम -1;
    gz_error(state, Z_OK, रिक्त);

    /* write string */
    len = वाक्यदैर्घ्यम्(s);
    यदि ((पूर्णाङ्क)len < 0 || (अचिह्नित)len != len) {
        gz_error(state, Z_प्रवाहः_ERROR, "string length does not fit in int");
        प्रत्यागम -1;
    }
    put = gz_write(state, s, len);
    प्रत्यागम len && put == 0 ? -1 : (पूर्णाङ्क)put;
}

#यदि (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
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
    यदि (strm->अग्र_अन्तर् == रिक्त ||
        strm->अग्र_अन्तर् + strm->avail_अन्तर् <= state->अन्तर् + state->आकार)
        प्रत्यागम 0;
    (शून्य)gz_comp(state, Z_NO_FLUSH);
    यदि (strm->avail_अन्तर् == 0) {
        strm->अग्र_अन्तर् = state->अन्तर्;
        प्रत्यागम 0;
    }
    memmove(state->अन्तर्, strm->अग्र_अन्तर्, strm->avail_अन्तर्);
    strm->अग्र_अन्तर् = state->अन्तर्;
    प्रत्यागम strm->avail_अन्तर् > state->आकार;
}
#यदि_अन्तः

#यदि defined(STDC) || defined(Z_HAVE_STDARG_H)
#अन्तर्भाव <stdarg.h>

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORTVA gzvprintf(gzFile file, ध्रुव वर्ण *format, va_list va) {
#यदि defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (शून्य)file, (शून्य)format, (शून्य)va;
    प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
    पूर्णाङ्क len, ret;
    वर्ण *अग्र;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again))
        प्रत्यागम Z_प्रवाहः_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* make sure we have some buffer space */
    यदि (state->आकार == 0 && gz_init(state) == -1)
        प्रत्यागम state->err;

    /* check for seek request */
    यदि (state->skip && gz_zero(state) == -1)
        प्रत्यागम state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    यदि (state->err) {
        यदि (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            प्रत्यागम state->err;
        }
        यदि (!state->again)
            प्रत्यागम state->err;
    }
    यदि (strm->avail_अन्तर् == 0)
        strm->अग्र_अन्तर् = state->अन्तर्;
    अग्र = (वर्ण *)(state->अन्तर् + (strm->अग्र_अन्तर् - state->अन्तर्) + strm->avail_अन्तर्);
    अग्र[state->आकार - 1] = 0;
#यदि_परिभाषितम् NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (शून्य)vsprintf(अग्र, format, va);
    प्रति (len = 0; len < state->आकार; len++)
        यदि (अग्र[len] == 0) भङ्ग;
#  अन्यथा
    len = vsprintf(अग्र, format, va);
#  endif
#अन्यथा
#  ifdef HAS_vsnprintf_void
    (शून्य)vsnprintf(अग्र, state->आकार, format, va);
    len = वाक्यदैर्घ्यम्(अग्र);
#  अन्यथा
    len = vsnprintf(अग्र, state->आकार, format, va);
#  endif
#यदि_अन्तः

    /* check that printf() results fit in buffer */
    यदि (len == 0 || (अचिह्नित)len >= state->आकार || अग्र[state->आकार - 1] != 0)
        प्रत्यागम 0;

    /* update buffer and position */
    strm->avail_अन्तर् += (अचिह्नित)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    यदि (ret) {
        /* discard unwritten formatted output */
        strm->avail_अन्तर् -= (अचिह्नित)len;
        state->x.pos -= len;
        यदि (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        प्रत्यागम state->err;
    }
    यदि (state->err && !state->again)
        प्रत्यागम state->err;
    प्रत्यागम len;
#यदि_अन्तः
}

पूर्णाङ्क ZEXPORTVA gzprintf(gzFile file, ध्रुव वर्ण *format, ...) {
    va_list va;
    पूर्णाङ्क ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    प्रत्यागम ret;
}

#अन्यथा /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORTVA gzprintf(gzFile file, ध्रुव वर्ण *format, पूर्णाङ्क a1, पूर्णाङ्क a2, पूर्णाङ्क a3,
                       पूर्णाङ्क a4, पूर्णाङ्क a5, पूर्णाङ्क a6, पूर्णाङ्क a7, पूर्णाङ्क a8, पूर्णाङ्क a9, पूर्णाङ्क a10,
                       पूर्णाङ्क a11, पूर्णाङ्क a12, पूर्णाङ्क a13, पूर्णाङ्क a14, पूर्णाङ्क a15, पूर्णाङ्क a16,
                       पूर्णाङ्क a17, पूर्णाङ्क a18, पूर्णाङ्क a19, पूर्णाङ्क a20) {
#यदि defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (शून्य)file, (शून्य)format, (शून्य)a1, (शून्य)a2, (शून्य)a3, (शून्य)a4, (शून्य)a5,
    (शून्य)a6, (शून्य)a7, (शून्य)a8, (शून्य)a9, (शून्य)a10, (शून्य)a11, (शून्य)a12,
    (शून्य)a13, (शून्य)a14, (शून्य)a15, (शून्य)a16, (शून्य)a17, (शून्य)a18,
    (शून्य)a19, (शून्य)a20;
    प्रत्यागम Z_प्रवाहः_ERROR;
#अन्यथा
    पूर्णाङ्क ret;
    अचिह्नित len, वामः;
    वर्ण *अग्र;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    यदि (परिमाण(पूर्णाङ्क) != परिमाण(शून्य *))
        प्रत्यागम Z_प्रवाहः_ERROR;

    /* check that we're writing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again))
        प्रत्यागम Z_प्रवाहः_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* make sure we have some buffer space */
    यदि (state->आकार == 0 && gz_init(state) == -1)
        प्रत्यागम state->err;

    /* check for seek request */
    यदि (state->skip && gz_zero(state) == -1)
        प्रत्यागम state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    यदि (state->err) {
        यदि (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            प्रत्यागम state->err;
        }
        यदि (!state->again)
            प्रत्यागम state->err;
    }
    यदि (strm->avail_अन्तर् == 0)
        strm->अग्र_अन्तर् = state->अन्तर्;
    अग्र = (वर्ण *)(strm->अग्र_अन्तर् + strm->avail_अन्तर्);
    अग्र[state->आकार - 1] = 0;
#यदि_परिभाषितम् NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(अग्र, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    प्रति (len = 0; len < आकार; len++)
        यदि (अग्र[len] == 0)
            भङ्ग;
#  अन्यथा
    len = sprintf(अग्र, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#अन्यथा
#  ifdef HAS_snprintf_void
    snprintf(अग्र, state->आकार, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = वाक्यदैर्घ्यम्(अग्र);
#  अन्यथा
    len = snprintf(अग्र, state->आकार, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#यदि_अन्तः

    /* check that printf() results fit in buffer */
    यदि (len == 0 || len >= state->आकार || अग्र[state->आकार - 1] != 0)
        प्रत्यागम 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_अन्तर् += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    यदि (ret) {
        /* discard unwritten formatted output */
        strm->avail_अन्तर् -= (अचिह्नित)len;
        state->x.pos -= len;
        यदि (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        प्रत्यागम state->err;
    }
    यदि (state->err && !state->again)
        प्रत्यागम state->err;
    प्रत्यागम (पूर्णाङ्क)len;
#यदि_अन्तः
}

#यदि_अन्तः

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzflush(gzFile file, पूर्णाङ्क flush) {
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again))
        प्रत्यागम Z_प्रवाहः_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* check flush parameter */
    यदि (flush < 0 || flush > Z_FINISH)
        प्रत्यागम Z_प्रवाहः_ERROR;

    /* check for seek request */
    यदि (state->skip && gz_zero(state) == -1)
        प्रत्यागम state->err;

    /* compress remaining data with requested flush */
    (शून्य)gz_comp(state, flush);
    प्रत्यागम state->err;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzsetparams(gzFile file, पूर्णाङ्क level, पूर्णाङ्क strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    यदि (state->बहुलकम् != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->प्रत्यक्षम्)
        प्रत्यागम Z_प्रवाहः_ERROR;
    gz_error(state, Z_OK, रिक्त);

    /* if no change is requested, then do nothing */
    यदि (level == state->level && strategy == state->strategy)
        प्रत्यागम Z_OK;

    /* check for seek request */
    यदि (state->skip && gz_zero(state) == -1)
        प्रत्यागम state->err;

    /* change compression parameters for subsequent input */
    यदि (state->आकार) {
        /* flush previous input with previous parameters before changing */
        यदि (strm->avail_अन्तर् && gz_comp(state, Z_खण्डम्) == -1)
            प्रत्यागम state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    प्रत्यागम Z_OK;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzclose_w(gzFile file) {
    पूर्णाङ्क ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम Z_प्रवाहः_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    यदि (state->बहुलकम् != GZ_WRITE)
        प्रत्यागम Z_प्रवाहः_ERROR;

    /* check for seek request */
    यदि (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    यदि (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    यदि (state->आकार) {
        यदि (!state->प्रत्यक्षम्) {
            (शून्य)deflate_अन्तः(&(state->strm));
            मुक्त(state->बहिर्गमनम्);
        }
        मुक्त(state->अन्तर्);
    }
    gz_error(state, Z_OK, रिक्त);
    मुक्त(state->मार्गः);
    यदि (close(state->fd) == -1)
        ret = Z_ERRNO;
    मुक्त(state);
    प्रत्यागम ret;
}
