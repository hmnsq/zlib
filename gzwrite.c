/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#అన్తర్భావ "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local పూర్ణాంకం gz_init(gz_statep state) {
    పూర్ణాంకం ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->లోపల = (గుర్తులేని అక్షరం *)జ్ఞాపకం_ఇవ్వు(state->want << 1);
    ఒకవేళ (state->లోపల == ఖాళీ) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        తిరిగి -1;
    }

    /* only need output buffer and deflate state if compressing */
    ఒకవేళ (!state->ప్రత్యక్ష) {
        /* allocate output buffer */
        state->బహిర్గతం = (గుర్తులేని అక్షరం *)జ్ఞాపకం_ఇవ్వు(state->want);
        ఒకవేళ (state->బహిర్గతం == ఖాళీ) {
            విడుదల(state->లోపల);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            తిరిగి -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        ఒకవేళ (ret != Z_OK) {
            విడుదల(state->బహిర్గతం);
            విడుదల(state->లోపల);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            తిరిగి -1;
        }
        strm->తదుపరి_లోపల = ఖాళీ;
    }

    /* mark state as initialized */
    state->కొలత = state->want;

    /* initialize write buffer if compressing */
    ఒకవేళ (!state->ప్రత్యక్ష) {
        strm->avail_బహిర్గతం = state->కొలత;
        strm->తదుపరి_బహిర్గతం = state->బహిర్గతం;
        state->x.తదుపరి = strm->తదుపరి_బహిర్గతం;
    }
    తిరిగి 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local పూర్ణాంకం gz_comp(gz_statep state, పూర్ణాంకం flush) {
    పూర్ణాంకం ret, writ;
    గుర్తులేని have, put, max = ((గుర్తులేని)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    ఒకవేళ (state->కొలత == 0 && gz_init(state) == -1)
        తిరిగి -1;

    /* write directly if requested */
    ఒకవేళ (state->ప్రత్యక్ష) {
        అయితే (strm->avail_లోపల) {
            errno = 0;
            state->again = 0;
            put = strm->avail_లోపల > max ? max : strm->avail_లోపల;
            writ = (పూర్ణాంకం)లిఖ(state->fd, strm->తదుపరి_లోపల, put);
            ఒకవేళ (writ < 0) {
                ఒకవేళ (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                తిరిగి -1;
            }
            strm->avail_లోపల -= (గుర్తులేని)writ;
            strm->తదుపరి_లోపల += writ;
        }
        తిరిగి 0;
    }

    /* check for a pending reset */
    ఒకవేళ (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        ఒకవేళ (strm->avail_లోపల == 0 && flush == Z_NO_FLUSH)
            తిరిగి 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    చేయి {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        ఒకవేళ (strm->avail_బహిర్గతం == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_ముగింపు || ret == Z_ప్రవాహం_చివర))) {
            అయితే (strm->తదుపరి_బహిర్గతం > state->x.తదుపరి) {
                errno = 0;
                state->again = 0;
                put = strm->తదుపరి_బహిర్గతం - state->x.తదుపరి > (పూర్ణాంకం)max ? max :
                      (గుర్తులేని)(strm->తదుపరి_బహిర్గతం - state->x.తదుపరి);
                writ = (పూర్ణాంకం)లిఖ(state->fd, state->x.తదుపరి, put);
                ఒకవేళ (writ < 0) {
                    ఒకవేళ (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    తిరిగి -1;
                }
                state->x.తదుపరి += writ;
            }
            ఒకవేళ (strm->avail_బహిర్గతం == 0) {
                strm->avail_బహిర్గతం = state->కొలత;
                strm->తదుపరి_బహిర్గతం = state->బహిర్గతం;
                state->x.తదుపరి = state->బహిర్గతం;
            }
        }

        /* compress */
        have = strm->avail_బహిర్గతం;
        ret = deflate(strm, flush);
        ఒకవేళ (ret == Z_ప్రవాహం_ERROR) {
            gz_error(state, Z_ప్రవాహం_ERROR,
                      "internal error: deflate stream corrupt");
            తిరిగి -1;
        }
        have -= strm->avail_బహిర్గతం;
    } అయితే (have);

    /* if that completed a deflate stream, allow another to start */
    ఒకవేళ (flush == Z_ముగింపు)
        state->reset = 1;

    /* all done, no errors */
    తిరిగి 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local పూర్ణాంకం gz_zero(gz_statep state) {
    పూర్ణాంకం మొదటి, ret;
    గుర్తులేని n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    ఒకవేళ (strm->avail_లోపల && gz_comp(state, Z_NO_FLUSH) == -1)
        తిరిగి -1;

    /* compress state->skip zeros */
    మొదటి = 1;
    చేయి {
        n = GT_OFF(state->కొలత) || (z_off64_t)state->కొలత > state->skip ?
            (గుర్తులేని)state->skip : state->కొలత;
        ఒకవేళ (మొదటి) {
            memset(state->లోపల, 0, n);
            మొదటి = 0;
        }
        strm->avail_లోపల = n;
        strm->తదుపరి_లోపల = state->లోపల;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_లోపల;
        state->x.pos += n;
        state->skip -= n;
        ఒకవేళ (ret == -1)
            తిరిగి -1;
    } అయితే (state->skip);
    తిరిగి 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_కొలత_t gz_write(gz_statep state, voidpc buf, z_కొలత_t len) {
    z_కొలత_t put = len;
    పూర్ణాంకం ret;

    /* if len is zero, avoid unnecessary operations */
    ఒకవేళ (len == 0)
        తిరిగి 0;

    /* allocate memory if this is the first time through */
    ఒకవేళ (state->కొలత == 0 && gz_init(state) == -1)
        తిరిగి 0;

    /* check for seek request */
    ఒకవేళ (state->skip && gz_zero(state) == -1)
        తిరిగి 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    ఒకవేళ (len < state->కొలత) {
        /* copy to input buffer, compress when full */
        కొరకు (;;) {
            గుర్తులేని have, copy;

            ఒకవేళ (state->strm.avail_లోపల == 0)
                state->strm.తదుపరి_లోపల = state->లోపల;
            have = (గుర్తులేని)((state->strm.తదుపరి_లోపల + state->strm.avail_లోపల) -
                              state->లోపల);
            copy = state->కొలత - have;
            ఒకవేళ (copy > len)
                copy = (గుర్తులేని)len;
            జ్ఞాపకం_నకలు(state->లోపల + have, buf, copy);
            state->strm.avail_లోపల += copy;
            state->x.pos += copy;
            buf = (స్థిరరాశి అక్షరం *)buf + copy;
            len -= copy;
            ఒకవేళ (len == 0)
                ఆపు;
            ఒకవేళ (gz_comp(state, Z_NO_FLUSH) == -1)
                తిరిగి state->again ? put - len : 0;
        }
    }
    లేకపోతే {
        /* consume whatever's left in the input buffer */
        ఒకవేళ (state->strm.avail_లోపల && gz_comp(state, Z_NO_FLUSH) == -1)
            తిరిగి 0;

        /* directly compress user buffer to file */
        state->strm.తదుపరి_లోపల = (z_const Bytef *)buf;
        చేయి {
            గుర్తులేని n = (గుర్తులేని)-1;

            ఒకవేళ (n > len)
                n = (గుర్తులేని)len;
            state->strm.avail_లోపల = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_లోపల;
            state->x.pos += n;
            len -= n;
            ఒకవేళ (ret == -1) {
                state->strm.avail_లోపల = 0;
                state->strm.తదుపరి_లోపల = state->లోపల;
                తిరిగి state->again ? put - len : 0;
            }
        } అయితే (len);
    }

    /* input was all buffered or compressed */
    తిరిగి put;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzwrite(gzFile file, voidpc buf, గుర్తులేని len) {
    gz_statep state;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again))
        తిరిగి 0;
    gz_error(state, Z_OK, ఖాళీ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ఒకవేళ ((పూర్ణాంకం)len < 0) {
        gz_error(state, Z_దత్తాంశం_ERROR, "requested length does not fit in int");
        తిరిగి 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    తిరిగి (పూర్ణాంకం)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_కొలత_t ZEXPORT gzfwrite(voidpc buf, z_కొలత_t కొలత, z_కొలత_t nitems,
                          gzFile file) {
    z_కొలత_t len;
    gz_statep state;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again))
        తిరిగి 0;
    gz_error(state, Z_OK, ఖాళీ);

    /* compute bytes to read -- error on overflow */
    len = nitems * కొలత;
    ఒకవేళ (కొలత && len / కొలత != nitems) {
        gz_error(state, Z_ప్రవాహం_ERROR, "request does not fit in a size_t");
        తిరిగి 0;
    }

    /* write len bytes to buf, return the number of full items written */
    తిరిగి len ? gz_write(state, buf, len) / కొలత : 0;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzputc(gzFile file, పూర్ణాంకం c) {
    గుర్తులేని have;
    గుర్తులేని అక్షరం buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again))
        తిరిగి -1;
    gz_error(state, Z_OK, ఖాళీ);

    /* check for seek request */
    ఒకవేళ (state->skip && gz_zero(state) == -1)
        తిరిగి -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    ఒకవేళ (state->కొలత) {
        ఒకవేళ (strm->avail_లోపల == 0)
            strm->తదుపరి_లోపల = state->లోపల;
        have = (గుర్తులేని)((strm->తదుపరి_లోపల + strm->avail_లోపల) - state->లోపల);
        ఒకవేళ (have < state->కొలత) {
            state->లోపల[have] = (గుర్తులేని అక్షరం)c;
            strm->avail_లోపల++;
            state->x.pos++;
            తిరిగి c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (గుర్తులేని అక్షరం)c;
    ఒకవేళ (gz_write(state, buf, 1) != 1)
        తిరిగి -1;
    తిరిగి c & 0xff;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzputs(gzFile file, స్థిరరాశి అక్షరం *s) {
    z_కొలత_t len, put;
    gz_statep state;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again))
        తిరిగి -1;
    gz_error(state, Z_OK, ఖాళీ);

    /* write string */
    len = అక్షరశ్రేణి_పొడవు(s);
    ఒకవేళ ((పూర్ణాంకం)len < 0 || (గుర్తులేని)len != len) {
        gz_error(state, Z_ప్రవాహం_ERROR, "string length does not fit in int");
        తిరిగి -1;
    }
    put = gz_write(state, s, len);
    తిరిగి len && put == 0 ? -1 : (పూర్ణాంకం)put;
}

#ఒకవేళ (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local పూర్ణాంకం gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    ఒకవేళ (strm->తదుపరి_లోపల == ఖాళీ ||
        strm->తదుపరి_లోపల + strm->avail_లోపల <= state->లోపల + state->కొలత)
        తిరిగి 0;
    (శూన్య)gz_comp(state, Z_NO_FLUSH);
    ఒకవేళ (strm->avail_లోపల == 0) {
        strm->తదుపరి_లోపల = state->లోపల;
        తిరిగి 0;
    }
    memmove(state->లోపల, strm->తదుపరి_లోపల, strm->avail_లోపల);
    strm->తదుపరి_లోపల = state->లోపల;
    తిరిగి strm->avail_లోపల > state->కొలత;
}
#అయితే_ముగింపు

#ఒకవేళ defined(STDC) || defined(Z_HAVE_STDARG_H)
#అన్తర్భావ <stdarg.h>

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORTVA gzvprintf(gzFile file, స్థిరరాశి అక్షరం *format, va_list va) {
#ఒకవేళ defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (శూన్య)file, (శూన్య)format, (శూన్య)va;
    తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
    పూర్ణాంకం len, ret;
    అక్షరం *తదుపరి;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again))
        తిరిగి Z_ప్రవాహం_ERROR;
    gz_error(state, Z_OK, ఖాళీ);

    /* make sure we have some buffer space */
    ఒకవేళ (state->కొలత == 0 && gz_init(state) == -1)
        తిరిగి state->err;

    /* check for seek request */
    ఒకవేళ (state->skip && gz_zero(state) == -1)
        తిరిగి state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ఒకవేళ (state->err) {
        ఒకవేళ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            తిరిగి state->err;
        }
        ఒకవేళ (!state->again)
            తిరిగి state->err;
    }
    ఒకవేళ (strm->avail_లోపల == 0)
        strm->తదుపరి_లోపల = state->లోపల;
    తదుపరి = (అక్షరం *)(state->లోపల + (strm->తదుపరి_లోపల - state->లోపల) + strm->avail_లోపల);
    తదుపరి[state->కొలత - 1] = 0;
#నిర్వచితమైతే NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (శూన్య)vsprintf(తదుపరి, format, va);
    కొరకు (len = 0; len < state->కొలత; len++)
        ఒకవేళ (తదుపరి[len] == 0) ఆపు;
#  లేకపోతే
    len = vsprintf(తదుపరి, format, va);
#  endif
#లేకపోతే
#  ifdef HAS_vsnprintf_void
    (శూన్య)vsnprintf(తదుపరి, state->కొలత, format, va);
    len = అక్షరశ్రేణి_పొడవు(తదుపరి);
#  లేకపోతే
    len = vsnprintf(తదుపరి, state->కొలత, format, va);
#  endif
#అయితే_ముగింపు

    /* check that printf() results fit in buffer */
    ఒకవేళ (len == 0 || (గుర్తులేని)len >= state->కొలత || తదుపరి[state->కొలత - 1] != 0)
        తిరిగి 0;

    /* update buffer and position */
    strm->avail_లోపల += (గుర్తులేని)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ఒకవేళ (ret) {
        /* discard unwritten formatted output */
        strm->avail_లోపల -= (గుర్తులేని)len;
        state->x.pos -= len;
        ఒకవేళ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        తిరిగి state->err;
    }
    ఒకవేళ (state->err && !state->again)
        తిరిగి state->err;
    తిరిగి len;
#అయితే_ముగింపు
}

పూర్ణాంకం ZEXPORTVA gzprintf(gzFile file, స్థిరరాశి అక్షరం *format, ...) {
    va_list va;
    పూర్ణాంకం ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    తిరిగి ret;
}

#లేకపోతే /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORTVA gzprintf(gzFile file, స్థిరరాశి అక్షరం *format, పూర్ణాంకం a1, పూర్ణాంకం a2, పూర్ణాంకం a3,
                       పూర్ణాంకం a4, పూర్ణాంకం a5, పూర్ణాంకం a6, పూర్ణాంకం a7, పూర్ణాంకం a8, పూర్ణాంకం a9, పూర్ణాంకం a10,
                       పూర్ణాంకం a11, పూర్ణాంకం a12, పూర్ణాంకం a13, పూర్ణాంకం a14, పూర్ణాంకం a15, పూర్ణాంకం a16,
                       పూర్ణాంకం a17, పూర్ణాంకం a18, పూర్ణాంకం a19, పూర్ణాంకం a20) {
#ఒకవేళ defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (శూన్య)file, (శూన్య)format, (శూన్య)a1, (శూన్య)a2, (శూన్య)a3, (శూన్య)a4, (శూన్య)a5,
    (శూన్య)a6, (శూన్య)a7, (శూన్య)a8, (శూన్య)a9, (శూన్య)a10, (శూన్య)a11, (శూన్య)a12,
    (శూన్య)a13, (శూన్య)a14, (శూన్య)a15, (శూన్య)a16, (శూన్య)a17, (శూన్య)a18,
    (శూన్య)a19, (శూన్య)a20;
    తిరిగి Z_ప్రవాహం_ERROR;
#లేకపోతే
    పూర్ణాంకం ret;
    గుర్తులేని len, ఎడమ;
    అక్షరం *తదుపరి;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    ఒకవేళ (పరిమాణం(పూర్ణాంకం) != పరిమాణం(శూన్య *))
        తిరిగి Z_ప్రవాహం_ERROR;

    /* check that we're writing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again))
        తిరిగి Z_ప్రవాహం_ERROR;
    gz_error(state, Z_OK, ఖాళీ);

    /* make sure we have some buffer space */
    ఒకవేళ (state->కొలత == 0 && gz_init(state) == -1)
        తిరిగి state->err;

    /* check for seek request */
    ఒకవేళ (state->skip && gz_zero(state) == -1)
        తిరిగి state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    ఒకవేళ (state->err) {
        ఒకవేళ (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            తిరిగి state->err;
        }
        ఒకవేళ (!state->again)
            తిరిగి state->err;
    }
    ఒకవేళ (strm->avail_లోపల == 0)
        strm->తదుపరి_లోపల = state->లోపల;
    తదుపరి = (అక్షరం *)(strm->తదుపరి_లోపల + strm->avail_లోపల);
    తదుపరి[state->కొలత - 1] = 0;
#నిర్వచితమైతే NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(తదుపరి, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    కొరకు (len = 0; len < కొలత; len++)
        ఒకవేళ (తదుపరి[len] == 0)
            ఆపు;
#  లేకపోతే
    len = sprintf(తదుపరి, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#లేకపోతే
#  ifdef HAS_snprintf_void
    snprintf(తదుపరి, state->కొలత, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = అక్షరశ్రేణి_పొడవు(తదుపరి);
#  లేకపోతే
    len = snprintf(తదుపరి, state->కొలత, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#అయితే_ముగింపు

    /* check that printf() results fit in buffer */
    ఒకవేళ (len == 0 || len >= state->కొలత || తదుపరి[state->కొలత - 1] != 0)
        తిరిగి 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_లోపల += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    ఒకవేళ (ret) {
        /* discard unwritten formatted output */
        strm->avail_లోపల -= (గుర్తులేని)len;
        state->x.pos -= len;
        ఒకవేళ (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        తిరిగి state->err;
    }
    ఒకవేళ (state->err && !state->again)
        తిరిగి state->err;
    తిరిగి (పూర్ణాంకం)len;
#అయితే_ముగింపు
}

#అయితే_ముగింపు

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzflush(gzFile file, పూర్ణాంకం flush) {
    gz_statep state;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again))
        తిరిగి Z_ప్రవాహం_ERROR;
    gz_error(state, Z_OK, ఖాళీ);

    /* check flush parameter */
    ఒకవేళ (flush < 0 || flush > Z_ముగింపు)
        తిరిగి Z_ప్రవాహం_ERROR;

    /* check for seek request */
    ఒకవేళ (state->skip && gz_zero(state) == -1)
        తిరిగి state->err;

    /* compress remaining data with requested flush */
    (శూన్య)gz_comp(state, flush);
    తిరిగి state->err;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzsetparams(gzFile file, పూర్ణాంకం level, పూర్ణాంకం strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    ఒకవేళ (state->బహుళకం != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->ప్రత్యక్ష)
        తిరిగి Z_ప్రవాహం_ERROR;
    gz_error(state, Z_OK, ఖాళీ);

    /* if no change is requested, then do nothing */
    ఒకవేళ (level == state->level && strategy == state->strategy)
        తిరిగి Z_OK;

    /* check for seek request */
    ఒకవేళ (state->skip && gz_zero(state) == -1)
        తిరిగి state->err;

    /* change compression parameters for subsequent input */
    ఒకవేళ (state->కొలత) {
        /* flush previous input with previous parameters before changing */
        ఒకవేళ (strm->avail_లోపల && gz_comp(state, Z_ఖండం) == -1)
            తిరిగి state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    తిరిగి Z_OK;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzclose_w(gzFile file) {
    పూర్ణాంకం ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    ఒకవేళ (state->బహుళకం != GZ_WRITE)
        తిరిగి Z_ప్రవాహం_ERROR;

    /* check for seek request */
    ఒకవేళ (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    ఒకవేళ (gz_comp(state, Z_ముగింపు) == -1)
        ret = state->err;
    ఒకవేళ (state->కొలత) {
        ఒకవేళ (!state->ప్రత్యక్ష) {
            (శూన్య)deflate_చివర(&(state->strm));
            విడుదల(state->బహిర్గతం);
        }
        విడుదల(state->లోపల);
    }
    gz_error(state, Z_OK, ఖాళీ);
    విడుదల(state->మార్గం);
    ఒకవేళ (close(state->fd) == -1)
        ret = Z_ERRNO;
    విడుదల(state);
    తిరిగి ret;
}
