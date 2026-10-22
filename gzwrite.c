/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local صحيح gz_init(gz_statep state) {
    صحيح ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->دننه = (بینښه حرف *)حافظه_ورکړه(state->want << 1);
    إذا (state->دننه == عدم) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        أرجع -1;
    }

    /* only need output buffer and deflate state if compressing */
    إذا (!state->مستقیم) {
        /* allocate output buffer */
        state->بهر = (بینښه حرف *)حافظه_ورکړه(state->want);
        إذا (state->بهر == عدم) {
            حرر(state->دننه);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            أرجع -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        إذا (ret != Z_OK) {
            حرر(state->بهر);
            حرر(state->دننه);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            أرجع -1;
        }
        strm->بل_دننه = عدم;
    }

    /* mark state as initialized */
    state->اندازه = state->want;

    /* initialize write buffer if compressing */
    إذا (!state->مستقیم) {
        strm->avail_بهر = state->اندازه;
        strm->بل_بهر = state->بهر;
        state->x.بل = strm->بل_بهر;
    }
    أرجع 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local صحيح gz_comp(gz_statep state, صحيح flush) {
    صحيح ret, writ;
    بینښه have, put, max = ((بینښه)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    إذا (state->اندازه == 0 && gz_init(state) == -1)
        أرجع -1;

    /* write directly if requested */
    إذا (state->مستقیم) {
        طالما (strm->avail_دننه) {
            errno = 0;
            state->again = 0;
            put = strm->avail_دننه > max ? max : strm->avail_دننه;
            writ = (صحيح)اكتب(state->fd, strm->بل_دننه, put);
            إذا (writ < 0) {
                إذا (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                أرجع -1;
            }
            strm->avail_دننه -= (بینښه)writ;
            strm->بل_دننه += writ;
        }
        أرجع 0;
    }

    /* check for a pending reset */
    إذا (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        إذا (strm->avail_دننه == 0 && flush == Z_NO_FLUSH)
            أرجع 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    افعل {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        إذا (strm->avail_بهر == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_پای || ret == Z_بهیر_پای_ټکی))) {
            طالما (strm->بل_بهر > state->x.بل) {
                errno = 0;
                state->again = 0;
                put = strm->بل_بهر - state->x.بل > (صحيح)max ? max :
                      (بینښه)(strm->بل_بهر - state->x.بل);
                writ = (صحيح)اكتب(state->fd, state->x.بل, put);
                إذا (writ < 0) {
                    إذا (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    أرجع -1;
                }
                state->x.بل += writ;
            }
            إذا (strm->avail_بهر == 0) {
                strm->avail_بهر = state->اندازه;
                strm->بل_بهر = state->بهر;
                state->x.بل = state->بهر;
            }
        }

        /* compress */
        have = strm->avail_بهر;
        ret = deflate(strm, flush);
        إذا (ret == Z_بهیر_ERROR) {
            gz_error(state, Z_بهیر_ERROR,
                      "internal error: deflate stream corrupt");
            أرجع -1;
        }
        have -= strm->avail_بهر;
    } طالما (have);

    /* if that completed a deflate stream, allow another to start */
    إذا (flush == Z_پای)
        state->reset = 1;

    /* all done, no errors */
    أرجع 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local صحيح gz_zero(gz_statep state) {
    صحيح لومړی, ret;
    بینښه n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    إذا (strm->avail_دننه && gz_comp(state, Z_NO_FLUSH) == -1)
        أرجع -1;

    /* compress state->skip zeros */
    لومړی = 1;
    افعل {
        n = GT_OFF(state->اندازه) || (z_off64_t)state->اندازه > state->skip ?
            (بینښه)state->skip : state->اندازه;
        إذا (لومړی) {
            memset(state->دننه, 0, n);
            لومړی = 0;
        }
        strm->avail_دننه = n;
        strm->بل_دننه = state->دننه;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_دننه;
        state->x.pos += n;
        state->skip -= n;
        إذا (ret == -1)
            أرجع -1;
    } طالما (state->skip);
    أرجع 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_اندازه_t gz_write(gz_statep state, voidpc buf, z_اندازه_t len) {
    z_اندازه_t put = len;
    صحيح ret;

    /* if len is zero, avoid unnecessary operations */
    إذا (len == 0)
        أرجع 0;

    /* allocate memory if this is the first time through */
    إذا (state->اندازه == 0 && gz_init(state) == -1)
        أرجع 0;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    إذا (len < state->اندازه) {
        /* copy to input buffer, compress when full */
        لكل (;;) {
            بینښه have, copy;

            إذا (state->strm.avail_دننه == 0)
                state->strm.بل_دننه = state->دننه;
            have = (بینښه)((state->strm.بل_دننه + state->strm.avail_دننه) -
                              state->دننه);
            copy = state->اندازه - have;
            إذا (copy > len)
                copy = (بینښه)len;
            حافظه_نقل(state->دننه + have, buf, copy);
            state->strm.avail_دننه += copy;
            state->x.pos += copy;
            buf = (ثابت حرف *)buf + copy;
            len -= copy;
            إذا (len == 0)
                اكسر;
            إذا (gz_comp(state, Z_NO_FLUSH) == -1)
                أرجع state->again ? put - len : 0;
        }
    }
    وإلا {
        /* consume whatever's left in the input buffer */
        إذا (state->strm.avail_دننه && gz_comp(state, Z_NO_FLUSH) == -1)
            أرجع 0;

        /* directly compress user buffer to file */
        state->strm.بل_دننه = (z_const Bytef *)buf;
        افعل {
            بینښه n = (بینښه)-1;

            إذا (n > len)
                n = (بینښه)len;
            state->strm.avail_دننه = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_دننه;
            state->x.pos += n;
            len -= n;
            إذا (ret == -1) {
                state->strm.avail_دننه = 0;
                state->strm.بل_دننه = state->دننه;
                أرجع state->again ? put - len : 0;
            }
        } طالما (len);
    }

    /* input was all buffered or compressed */
    أرجع put;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzwrite(gzFile file, voidpc buf, بینښه len) {
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع 0;
    gz_error(state, Z_OK, عدم);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    إذا ((صحيح)len < 0) {
        gz_error(state, Z_معلومات_ERROR, "requested length does not fit in int");
        أرجع 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    أرجع (صحيح)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_اندازه_t ZEXPORT gzfwrite(voidpc buf, z_اندازه_t اندازه, z_اندازه_t nitems,
                          gzFile file) {
    z_اندازه_t len;
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع 0;
    gz_error(state, Z_OK, عدم);

    /* compute bytes to read -- error on overflow */
    len = nitems * اندازه;
    إذا (اندازه && len / اندازه != nitems) {
        gz_error(state, Z_بهیر_ERROR, "request does not fit in a size_t");
        أرجع 0;
    }

    /* write len bytes to buf, return the number of full items written */
    أرجع len ? gz_write(state, buf, len) / اندازه : 0;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzputc(gzFile file, صحيح c) {
    بینښه have;
    بینښه حرف buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع -1;
    gz_error(state, Z_OK, عدم);

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    إذا (state->اندازه) {
        إذا (strm->avail_دننه == 0)
            strm->بل_دننه = state->دننه;
        have = (بینښه)((strm->بل_دننه + strm->avail_دننه) - state->دننه);
        إذا (have < state->اندازه) {
            state->دننه[have] = (بینښه حرف)c;
            strm->avail_دننه++;
            state->x.pos++;
            أرجع c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (بینښه حرف)c;
    إذا (gz_write(state, buf, 1) != 1)
        أرجع -1;
    أرجع c & 0xff;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzputs(gzFile file, ثابت حرف *s) {
    z_اندازه_t len, put;
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع -1;
    gz_error(state, Z_OK, عدم);

    /* write string */
    len = مزي_اوږدوالی(s);
    إذا ((صحيح)len < 0 || (بینښه)len != len) {
        gz_error(state, Z_بهیر_ERROR, "string length does not fit in int");
        أرجع -1;
    }
    put = gz_write(state, s, len);
    أرجع len && put == 0 ? -1 : (صحيح)put;
}

#إذا (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local صحيح gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    إذا (strm->بل_دننه == عدم ||
        strm->بل_دننه + strm->avail_دننه <= state->دننه + state->اندازه)
        أرجع 0;
    (فراغ)gz_comp(state, Z_NO_FLUSH);
    إذا (strm->avail_دننه == 0) {
        strm->بل_دننه = state->دننه;
        أرجع 0;
    }
    memmove(state->دننه, strm->بل_دننه, strm->avail_دننه);
    strm->بل_دننه = state->دننه;
    أرجع strm->avail_دننه > state->اندازه;
}
#د_که_پای

#إذا defined(STDC) || defined(Z_HAVE_STDARG_H)
#ضمن <stdarg.h>

/* -- see zlib.h -- */
صحيح ZEXPORTVA gzvprintf(gzFile file, ثابت حرف *format, va_list va) {
#إذا defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (فراغ)file, (فراغ)format, (فراغ)va;
    أرجع Z_بهیر_ERROR;
#وإلا
    صحيح len, ret;
    حرف *بل;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_بهیر_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع Z_بهیر_ERROR;
    gz_error(state, Z_OK, عدم);

    /* make sure we have some buffer space */
    إذا (state->اندازه == 0 && gz_init(state) == -1)
        أرجع state->err;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    إذا (state->err) {
        إذا (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            أرجع state->err;
        }
        إذا (!state->again)
            أرجع state->err;
    }
    إذا (strm->avail_دننه == 0)
        strm->بل_دننه = state->دننه;
    بل = (حرف *)(state->دننه + (strm->بل_دننه - state->دننه) + strm->avail_دننه);
    بل[state->اندازه - 1] = 0;
#که_تعریف_شوی NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (فراغ)vsprintf(بل, format, va);
    لكل (len = 0; len < state->اندازه; len++)
        إذا (بل[len] == 0) اكسر;
#  وإلا
    len = vsprintf(بل, format, va);
#  endif
#وإلا
#  ifdef HAS_vsnprintf_void
    (فراغ)vsnprintf(بل, state->اندازه, format, va);
    len = مزي_اوږدوالی(بل);
#  وإلا
    len = vsnprintf(بل, state->اندازه, format, va);
#  endif
#د_که_پای

    /* check that printf() results fit in buffer */
    إذا (len == 0 || (بینښه)len >= state->اندازه || بل[state->اندازه - 1] != 0)
        أرجع 0;

    /* update buffer and position */
    strm->avail_دننه += (بینښه)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    إذا (ret) {
        /* discard unwritten formatted output */
        strm->avail_دننه -= (بینښه)len;
        state->x.pos -= len;
        إذا (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        أرجع state->err;
    }
    إذا (state->err && !state->again)
        أرجع state->err;
    أرجع len;
#د_که_پای
}

صحيح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, ...) {
    va_list va;
    صحيح ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    أرجع ret;
}

#وإلا /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
صحيح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, صحيح a1, صحيح a2, صحيح a3,
                       صحيح a4, صحيح a5, صحيح a6, صحيح a7, صحيح a8, صحيح a9, صحيح a10,
                       صحيح a11, صحيح a12, صحيح a13, صحيح a14, صحيح a15, صحيح a16,
                       صحيح a17, صحيح a18, صحيح a19, صحيح a20) {
#إذا defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (فراغ)file, (فراغ)format, (فراغ)a1, (فراغ)a2, (فراغ)a3, (فراغ)a4, (فراغ)a5,
    (فراغ)a6, (فراغ)a7, (فراغ)a8, (فراغ)a9, (فراغ)a10, (فراغ)a11, (فراغ)a12,
    (فراغ)a13, (فراغ)a14, (فراغ)a15, (فراغ)a16, (فراغ)a17, (فراغ)a18,
    (فراغ)a19, (فراغ)a20;
    أرجع Z_بهیر_ERROR;
#وإلا
    صحيح ret;
    بینښه len, کیڼ;
    حرف *بل;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_بهیر_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    إذا (حجم(صحيح) != حجم(فراغ *))
        أرجع Z_بهیر_ERROR;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع Z_بهیر_ERROR;
    gz_error(state, Z_OK, عدم);

    /* make sure we have some buffer space */
    إذا (state->اندازه == 0 && gz_init(state) == -1)
        أرجع state->err;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    إذا (state->err) {
        إذا (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            أرجع state->err;
        }
        إذا (!state->again)
            أرجع state->err;
    }
    إذا (strm->avail_دننه == 0)
        strm->بل_دننه = state->دننه;
    بل = (حرف *)(strm->بل_دننه + strm->avail_دننه);
    بل[state->اندازه - 1] = 0;
#که_تعریف_شوی NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(بل, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    لكل (len = 0; len < اندازه; len++)
        إذا (بل[len] == 0)
            اكسر;
#  وإلا
    len = sprintf(بل, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#وإلا
#  ifdef HAS_snprintf_void
    snprintf(بل, state->اندازه, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = مزي_اوږدوالی(بل);
#  وإلا
    len = snprintf(بل, state->اندازه, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#د_که_پای

    /* check that printf() results fit in buffer */
    إذا (len == 0 || len >= state->اندازه || بل[state->اندازه - 1] != 0)
        أرجع 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_دننه += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    إذا (ret) {
        /* discard unwritten formatted output */
        strm->avail_دننه -= (بینښه)len;
        state->x.pos -= len;
        إذا (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        أرجع state->err;
    }
    إذا (state->err && !state->again)
        أرجع state->err;
    أرجع (صحيح)len;
#د_که_پای
}

#د_که_پای

/* -- see zlib.h -- */
صحيح ZEXPORT gzflush(gzFile file, صحيح flush) {
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_بهیر_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again))
        أرجع Z_بهیر_ERROR;
    gz_error(state, Z_OK, عدم);

    /* check flush parameter */
    إذا (flush < 0 || flush > Z_پای)
        أرجع Z_بهیر_ERROR;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* compress remaining data with requested flush */
    (فراغ)gz_comp(state, flush);
    أرجع state->err;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzsetparams(gzFile file, صحيح level, صحيح strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_بهیر_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    إذا (state->ډېرپېښ != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->مستقیم)
        أرجع Z_بهیر_ERROR;
    gz_error(state, Z_OK, عدم);

    /* if no change is requested, then do nothing */
    إذا (level == state->level && strategy == state->strategy)
        أرجع Z_OK;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        أرجع state->err;

    /* change compression parameters for subsequent input */
    إذا (state->اندازه) {
        /* flush previous input with previous parameters before changing */
        إذا (strm->avail_دننه && gz_comp(state, Z_بلاک) == -1)
            أرجع state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    أرجع Z_OK;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzclose_w(gzFile file) {
    صحيح ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع Z_بهیر_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    إذا (state->ډېرپېښ != GZ_WRITE)
        أرجع Z_بهیر_ERROR;

    /* check for seek request */
    إذا (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    إذا (gz_comp(state, Z_پای) == -1)
        ret = state->err;
    إذا (state->اندازه) {
        إذا (!state->مستقیم) {
            (فراغ)deflate_پای_ټکی(&(state->strm));
            حرر(state->بهر);
        }
        حرر(state->دننه);
    }
    gz_error(state, Z_OK, عدم);
    حرر(state->لار);
    إذا (close(state->fd) == -1)
        ret = Z_ERRNO;
    حرر(state);
    أرجع ret;
}
