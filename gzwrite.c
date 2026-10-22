/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local صحیح gz_init(gz_statep state) {
    صحیح ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->اندر = (غیرعلامتی حرف *)حافظہ_دو(state->want << 1);
    اگر (state->اندر == تہی) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        واپس -1;
    }

    /* only need output buffer and deflate state if compressing */
    اگر (!state->براہ_راست) {
        /* allocate output buffer */
        state->برآمد = (غیرعلامتی حرف *)حافظہ_دو(state->want);
        اگر (state->برآمد == تہی) {
            آزاد(state->اندر);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            واپس -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        اگر (ret != Z_OK) {
            آزاد(state->برآمد);
            آزاد(state->اندر);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            واپس -1;
        }
        strm->اگلا_اندر = تہی;
    }

    /* mark state as initialized */
    state->سائز = state->want;

    /* initialize write buffer if compressing */
    اگر (!state->براہ_راست) {
        strm->avail_برآمد = state->سائز;
        strm->اگلا_برآمد = state->برآمد;
        state->x.اگلا = strm->اگلا_برآمد;
    }
    واپس 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local صحیح gz_comp(gz_statep state, صحیح flush) {
    صحیح ret, writ;
    غیرعلامتی have, put, max = ((غیرعلامتی)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    اگر (state->سائز == 0 && gz_init(state) == -1)
        واپس -1;

    /* write directly if requested */
    اگر (state->براہ_راست) {
        جب (strm->avail_اندر) {
            errno = 0;
            state->again = 0;
            put = strm->avail_اندر > max ? max : strm->avail_اندر;
            writ = (صحیح)اكتب(state->fd, strm->اگلا_اندر, put);
            اگر (writ < 0) {
                اگر (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                واپس -1;
            }
            strm->avail_اندر -= (غیرعلامتی)writ;
            strm->اگلا_اندر += writ;
        }
        واپس 0;
    }

    /* check for a pending reset */
    اگر (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        اگر (strm->avail_اندر == 0 && flush == Z_NO_FLUSH)
            واپس 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    کرو {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        اگر (strm->avail_برآمد == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_اختتام || ret == Z_دھارا_انتہا))) {
            جب (strm->اگلا_برآمد > state->x.اگلا) {
                errno = 0;
                state->again = 0;
                put = strm->اگلا_برآمد - state->x.اگلا > (صحیح)max ? max :
                      (غیرعلامتی)(strm->اگلا_برآمد - state->x.اگلا);
                writ = (صحیح)اكتب(state->fd, state->x.اگلا, put);
                اگر (writ < 0) {
                    اگر (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    واپس -1;
                }
                state->x.اگلا += writ;
            }
            اگر (strm->avail_برآمد == 0) {
                strm->avail_برآمد = state->سائز;
                strm->اگلا_برآمد = state->برآمد;
                state->x.اگلا = state->برآمد;
            }
        }

        /* compress */
        have = strm->avail_برآمد;
        ret = deflate(strm, flush);
        اگر (ret == Z_دھارا_ERROR) {
            gz_error(state, Z_دھارا_ERROR,
                      "internal error: deflate stream corrupt");
            واپس -1;
        }
        have -= strm->avail_برآمد;
    } جب (have);

    /* if that completed a deflate stream, allow another to start */
    اگر (flush == Z_اختتام)
        state->reset = 1;

    /* all done, no errors */
    واپس 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local صحیح gz_zero(gz_statep state) {
    صحیح پہلا, ret;
    غیرعلامتی n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    اگر (strm->avail_اندر && gz_comp(state, Z_NO_FLUSH) == -1)
        واپس -1;

    /* compress state->skip zeros */
    پہلا = 1;
    کرو {
        n = GT_OFF(state->سائز) || (z_off64_t)state->سائز > state->skip ?
            (غیرعلامتی)state->skip : state->سائز;
        اگر (پہلا) {
            memset(state->اندر, 0, n);
            پہلا = 0;
        }
        strm->avail_اندر = n;
        strm->اگلا_اندر = state->اندر;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_اندر;
        state->x.pos += n;
        state->skip -= n;
        اگر (ret == -1)
            واپس -1;
    } جب (state->skip);
    واپس 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_سائز_t gz_write(gz_statep state, voidpc buf, z_سائز_t len) {
    z_سائز_t put = len;
    صحیح ret;

    /* if len is zero, avoid unnecessary operations */
    اگر (len == 0)
        واپس 0;

    /* allocate memory if this is the first time through */
    اگر (state->سائز == 0 && gz_init(state) == -1)
        واپس 0;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        واپس 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    اگر (len < state->سائز) {
        /* copy to input buffer, compress when full */
        برائے (;;) {
            غیرعلامتی have, copy;

            اگر (state->strm.avail_اندر == 0)
                state->strm.اگلا_اندر = state->اندر;
            have = (غیرعلامتی)((state->strm.اگلا_اندر + state->strm.avail_اندر) -
                              state->اندر);
            copy = state->سائز - have;
            اگر (copy > len)
                copy = (غیرعلامتی)len;
            حافظہ_نقل(state->اندر + have, buf, copy);
            state->strm.avail_اندر += copy;
            state->x.pos += copy;
            buf = (ثابت حرف *)buf + copy;
            len -= copy;
            اگر (len == 0)
                توڑو;
            اگر (gz_comp(state, Z_NO_FLUSH) == -1)
                واپس state->again ? put - len : 0;
        }
    }
    ورنہ {
        /* consume whatever's left in the input buffer */
        اگر (state->strm.avail_اندر && gz_comp(state, Z_NO_FLUSH) == -1)
            واپس 0;

        /* directly compress user buffer to file */
        state->strm.اگلا_اندر = (z_const Bytef *)buf;
        کرو {
            غیرعلامتی n = (غیرعلامتی)-1;

            اگر (n > len)
                n = (غیرعلامتی)len;
            state->strm.avail_اندر = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_اندر;
            state->x.pos += n;
            len -= n;
            اگر (ret == -1) {
                state->strm.avail_اندر = 0;
                state->strm.اگلا_اندر = state->اندر;
                واپس state->again ? put - len : 0;
            }
        } جب (len);
    }

    /* input was all buffered or compressed */
    واپس put;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzwrite(gzFile file, voidpc buf, غیرعلامتی len) {
    gz_statep state;

    /* get internal structure */
    اگر (file == تہی)
        واپس 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again))
        واپس 0;
    gz_error(state, Z_OK, تہی);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    اگر ((صحیح)len < 0) {
        gz_error(state, Z_ڈیٹا_ERROR, "requested length does not fit in int");
        واپس 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    واپس (صحیح)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_سائز_t ZEXPORT gzfwrite(voidpc buf, z_سائز_t سائز, z_سائز_t nitems,
                          gzFile file) {
    z_سائز_t len;
    gz_statep state;

    /* get internal structure */
    اگر (file == تہی)
        واپس 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again))
        واپس 0;
    gz_error(state, Z_OK, تہی);

    /* compute bytes to read -- error on overflow */
    len = nitems * سائز;
    اگر (سائز && len / سائز != nitems) {
        gz_error(state, Z_دھارا_ERROR, "request does not fit in a size_t");
        واپس 0;
    }

    /* write len bytes to buf, return the number of full items written */
    واپس len ? gz_write(state, buf, len) / سائز : 0;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzputc(gzFile file, صحیح c) {
    غیرعلامتی have;
    غیرعلامتی حرف buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again))
        واپس -1;
    gz_error(state, Z_OK, تہی);

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        واپس -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    اگر (state->سائز) {
        اگر (strm->avail_اندر == 0)
            strm->اگلا_اندر = state->اندر;
        have = (غیرعلامتی)((strm->اگلا_اندر + strm->avail_اندر) - state->اندر);
        اگر (have < state->سائز) {
            state->اندر[have] = (غیرعلامتی حرف)c;
            strm->avail_اندر++;
            state->x.pos++;
            واپس c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (غیرعلامتی حرف)c;
    اگر (gz_write(state, buf, 1) != 1)
        واپس -1;
    واپس c & 0xff;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzputs(gzFile file, ثابت حرف *s) {
    z_سائز_t len, put;
    gz_statep state;

    /* get internal structure */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again))
        واپس -1;
    gz_error(state, Z_OK, تہی);

    /* write string */
    len = لڑی_لمبائی(s);
    اگر ((صحیح)len < 0 || (غیرعلامتی)len != len) {
        gz_error(state, Z_دھارا_ERROR, "string length does not fit in int");
        واپس -1;
    }
    put = gz_write(state, s, len);
    واپس len && put == 0 ? -1 : (صحیح)put;
}

#اگر (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local صحیح gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    اگر (strm->اگلا_اندر == تہی ||
        strm->اگلا_اندر + strm->avail_اندر <= state->اندر + state->سائز)
        واپس 0;
    (خالی)gz_comp(state, Z_NO_FLUSH);
    اگر (strm->avail_اندر == 0) {
        strm->اگلا_اندر = state->اندر;
        واپس 0;
    }
    memmove(state->اندر, strm->اگلا_اندر, strm->avail_اندر);
    strm->اگلا_اندر = state->اندر;
    واپس strm->avail_اندر > state->سائز;
}
#اگر_ختم

#اگر defined(STDC) || defined(Z_HAVE_STDARG_H)
#شامل <stdarg.h>

/* -- see zlib.h -- */
صحیح ZEXPORTVA gzvprintf(gzFile file, ثابت حرف *format, va_list va) {
#اگر defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (خالی)file, (خالی)format, (خالی)va;
    واپس Z_دھارا_ERROR;
#ورنہ
    صحیح len, ret;
    حرف *اگلا;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == تہی)
        واپس Z_دھارا_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again))
        واپس Z_دھارا_ERROR;
    gz_error(state, Z_OK, تہی);

    /* make sure we have some buffer space */
    اگر (state->سائز == 0 && gz_init(state) == -1)
        واپس state->err;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        واپس state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    اگر (state->err) {
        اگر (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            واپس state->err;
        }
        اگر (!state->again)
            واپس state->err;
    }
    اگر (strm->avail_اندر == 0)
        strm->اگلا_اندر = state->اندر;
    اگلا = (حرف *)(state->اندر + (strm->اگلا_اندر - state->اندر) + strm->avail_اندر);
    اگلا[state->سائز - 1] = 0;
#اگر_تعریف_شدہ NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (خالی)vsprintf(اگلا, format, va);
    برائے (len = 0; len < state->سائز; len++)
        اگر (اگلا[len] == 0) توڑو;
#  ورنہ
    len = vsprintf(اگلا, format, va);
#  endif
#ورنہ
#  ifdef HAS_vsnprintf_void
    (خالی)vsnprintf(اگلا, state->سائز, format, va);
    len = لڑی_لمبائی(اگلا);
#  ورنہ
    len = vsnprintf(اگلا, state->سائز, format, va);
#  endif
#اگر_ختم

    /* check that printf() results fit in buffer */
    اگر (len == 0 || (غیرعلامتی)len >= state->سائز || اگلا[state->سائز - 1] != 0)
        واپس 0;

    /* update buffer and position */
    strm->avail_اندر += (غیرعلامتی)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    اگر (ret) {
        /* discard unwritten formatted output */
        strm->avail_اندر -= (غیرعلامتی)len;
        state->x.pos -= len;
        اگر (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        واپس state->err;
    }
    اگر (state->err && !state->again)
        واپس state->err;
    واپس len;
#اگر_ختم
}

صحیح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, ...) {
    va_list va;
    صحیح ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    واپس ret;
}

#ورنہ /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
صحیح ZEXPORTVA gzprintf(gzFile file, ثابت حرف *format, صحیح a1, صحیح a2, صحیح a3,
                       صحیح a4, صحیح a5, صحیح a6, صحیح a7, صحیح a8, صحیح a9, صحیح a10,
                       صحیح a11, صحیح a12, صحیح a13, صحیح a14, صحیح a15, صحیح a16,
                       صحیح a17, صحیح a18, صحیح a19, صحیح a20) {
#اگر defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (خالی)file, (خالی)format, (خالی)a1, (خالی)a2, (خالی)a3, (خالی)a4, (خالی)a5,
    (خالی)a6, (خالی)a7, (خالی)a8, (خالی)a9, (خالی)a10, (خالی)a11, (خالی)a12,
    (خالی)a13, (خالی)a14, (خالی)a15, (خالی)a16, (خالی)a17, (خالی)a18,
    (خالی)a19, (خالی)a20;
    واپس Z_دھارا_ERROR;
#ورنہ
    صحیح ret;
    غیرعلامتی len, بایاں;
    حرف *اگلا;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == تہی)
        واپس Z_دھارا_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    اگر (حجم(صحیح) != حجم(خالی *))
        واپس Z_دھارا_ERROR;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again))
        واپس Z_دھارا_ERROR;
    gz_error(state, Z_OK, تہی);

    /* make sure we have some buffer space */
    اگر (state->سائز == 0 && gz_init(state) == -1)
        واپس state->err;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        واپس state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    اگر (state->err) {
        اگر (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            واپس state->err;
        }
        اگر (!state->again)
            واپس state->err;
    }
    اگر (strm->avail_اندر == 0)
        strm->اگلا_اندر = state->اندر;
    اگلا = (حرف *)(strm->اگلا_اندر + strm->avail_اندر);
    اگلا[state->سائز - 1] = 0;
#اگر_تعریف_شدہ NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(اگلا, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    برائے (len = 0; len < سائز; len++)
        اگر (اگلا[len] == 0)
            توڑو;
#  ورنہ
    len = sprintf(اگلا, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#ورنہ
#  ifdef HAS_snprintf_void
    snprintf(اگلا, state->سائز, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = لڑی_لمبائی(اگلا);
#  ورنہ
    len = snprintf(اگلا, state->سائز, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#اگر_ختم

    /* check that printf() results fit in buffer */
    اگر (len == 0 || len >= state->سائز || اگلا[state->سائز - 1] != 0)
        واپس 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_اندر += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    اگر (ret) {
        /* discard unwritten formatted output */
        strm->avail_اندر -= (غیرعلامتی)len;
        state->x.pos -= len;
        اگر (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        واپس state->err;
    }
    اگر (state->err && !state->again)
        واپس state->err;
    واپس (صحیح)len;
#اگر_ختم
}

#اگر_ختم

/* -- see zlib.h -- */
صحیح ZEXPORT gzflush(gzFile file, صحیح flush) {
    gz_statep state;

    /* get internal structure */
    اگر (file == تہی)
        واپس Z_دھارا_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again))
        واپس Z_دھارا_ERROR;
    gz_error(state, Z_OK, تہی);

    /* check flush parameter */
    اگر (flush < 0 || flush > Z_اختتام)
        واپس Z_دھارا_ERROR;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        واپس state->err;

    /* compress remaining data with requested flush */
    (خالی)gz_comp(state, flush);
    واپس state->err;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzsetparams(gzFile file, صحیح level, صحیح strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    اگر (file == تہی)
        واپس Z_دھارا_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    اگر (state->منوال != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->براہ_راست)
        واپس Z_دھارا_ERROR;
    gz_error(state, Z_OK, تہی);

    /* if no change is requested, then do nothing */
    اگر (level == state->level && strategy == state->strategy)
        واپس Z_OK;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        واپس state->err;

    /* change compression parameters for subsequent input */
    اگر (state->سائز) {
        /* flush previous input with previous parameters before changing */
        اگر (strm->avail_اندر && gz_comp(state, Z_بلاک) == -1)
            واپس state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    واپس Z_OK;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzclose_w(gzFile file) {
    صحیح ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    اگر (file == تہی)
        واپس Z_دھارا_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    اگر (state->منوال != GZ_WRITE)
        واپس Z_دھارا_ERROR;

    /* check for seek request */
    اگر (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    اگر (gz_comp(state, Z_اختتام) == -1)
        ret = state->err;
    اگر (state->سائز) {
        اگر (!state->براہ_راست) {
            (خالی)deflate_انتہا(&(state->strm));
            آزاد(state->برآمد);
        }
        آزاد(state->اندر);
    }
    gz_error(state, Z_OK, تہی);
    آزاد(state->راستہ);
    اگر (close(state->fd) == -1)
        ret = Z_ERRNO;
    آزاد(state);
    واپس ret;
}
