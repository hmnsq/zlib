/* gzwrite.c -- zlib functions for writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Initialize state for writing a gzip file.  Mark initialization by setting
   state->size to non-zero.  Return -1 on a memory allocation failure, or 0 on
   success. */
local целое gz_init(gz_statep state) {
    целое ret;
    z_streamp strm = &(state->strm);

    /* allocate input buffer (double size for gzprintf) */
    state->in = (unsigned char *)malloc(state->want << 1);
    если (state->in == NULL) {
        gz_error(state, Z_MEM_ERROR, "out of memory");
        вернуть -1;
    }

    /* only need output buffer and deflate state if compressing */
    если (!state->direct) {
        /* allocate output buffer */
        state->out = (unsigned char *)malloc(state->want);
        если (state->out == NULL) {
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            вернуть -1;
        }

        /* allocate deflate memory, set up for gzip compression */
        strm->zalloc = Z_NULL;
        strm->zfree = Z_NULL;
        strm->opaque = Z_NULL;
        ret = deflateInit2(strm, state->level, Z_DEFLATED,
                           MAX_WBITS + 16, DEF_MEM_LEVEL, state->strategy);
        если (ret != Z_OK) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            вернуть -1;
        }
        strm->next_in = NULL;
    }

    /* mark state as initialized */
    state->size = state->want;

    /* initialize write buffer if compressing */
    если (!state->direct) {
        strm->avail_out = state->size;
        strm->next_out = state->out;
        state->x.next = strm->next_out;
    }
    вернуть 0;
}

/* Compress whatever is at avail_in and next_in and write to the output file.
   Return -1 if there is an error writing to the output file or if gz_init()
   fails to allocate memory, otherwise 0.  flush is assumed to be a valid
   deflate() flush value.  If flush is Z_FINISH, then the deflate() state is
   reset to start a new gzip stream.  If gz->direct is true, then simply write
   to the output file without compressing, and ignore flush. */
local целое gz_comp(gz_statep state, целое flush) {
    целое ret, writ;
    unsigned have, put, max = ((unsigned)-1 >> 2) + 1;
    z_streamp strm = &(state->strm);

    /* allocate memory if this is the first time through */
    если (state->size == 0 && gz_init(state) == -1)
        вернуть -1;

    /* write directly if requested */
    если (state->direct) {
        пока (strm->avail_in) {
            errno = 0;
            state->again = 0;
            put = strm->avail_in > max ? max : strm->avail_in;
            writ = (целое)write(state->fd, strm->next_in, put);
            если (writ < 0) {
                если (BLOCKED(errno))
                    state->again = 1;
                gz_error(state, Z_ERRNO, zstrerror());
                вернуть -1;
            }
            strm->avail_in -= (unsigned)writ;
            strm->next_in += writ;
        }
        вернуть 0;
    }

    /* check for a pending reset */
    если (state->reset) {
        /* don't start a new gzip member unless there is data to write and
           we're not flushing */
        если (strm->avail_in == 0 && flush == Z_NO_FLUSH)
            вернуть 0;
        deflateReset(strm);
        state->reset = 0;
    }

    /* run deflate() on provided input until it produces no more output */
    ret = Z_OK;
    do {
        /* write out current buffer contents if full, or if flushing, but if
           doing Z_FINISH then don't write until we get to Z_STREAM_END */
        если (strm->avail_out == 0 || (flush != Z_NO_FLUSH &&
            (flush != Z_FINISH || ret == Z_STREAM_END))) {
            пока (strm->next_out > state->x.next) {
                errno = 0;
                state->again = 0;
                put = strm->next_out - state->x.next > (целое)max ? max :
                      (unsigned)(strm->next_out - state->x.next);
                writ = (целое)write(state->fd, state->x.next, put);
                если (writ < 0) {
                    если (BLOCKED(errno))
                        state->again = 1;
                    gz_error(state, Z_ERRNO, zstrerror());
                    вернуть -1;
                }
                state->x.next += writ;
            }
            если (strm->avail_out == 0) {
                strm->avail_out = state->size;
                strm->next_out = state->out;
                state->x.next = state->out;
            }
        }

        /* compress */
        have = strm->avail_out;
        ret = deflate(strm, flush);
        если (ret == Z_STREAM_ERROR) {
            gz_error(state, Z_STREAM_ERROR,
                      "internal error: deflate stream corrupt");
            вернуть -1;
        }
        have -= strm->avail_out;
    } пока (have);

    /* if that completed a deflate stream, allow another to start */
    если (flush == Z_FINISH)
        state->reset = 1;

    /* all done, no errors */
    вернуть 0;
}

/* Compress state->skip (> 0) zeros to output.  Return -1 on a write error or
   memory allocation failure by gz_comp(), or 0 on success. state->skip is
   updated with the number of successfully written zeros, in case there is a
   stall on a non-blocking write destination. */
local целое gz_zero(gz_statep state) {
    целое first, ret;
    unsigned n;
    z_streamp strm = &(state->strm);

    /* consume whatever's left in the input buffer */
    если (strm->avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
        вернуть -1;

    /* compress state->skip zeros */
    first = 1;
    do {
        n = GT_OFF(state->size) || (z_off64_t)state->size > state->skip ?
            (unsigned)state->skip : state->size;
        если (first) {
            memset(state->in, 0, n);
            first = 0;
        }
        strm->avail_in = n;
        strm->next_in = state->in;
        ret = gz_comp(state, Z_NO_FLUSH);
        n -= strm->avail_in;
        state->x.pos += n;
        state->skip -= n;
        если (ret == -1)
            вернуть -1;
    } пока (state->skip);
    вернуть 0;
}

/* Write len bytes from buf to file.  Return the number of bytes written.  If
   the returned value is less than len, then there was an error. If the error
   was a non-blocking stall, then the number of bytes consumed is returned.
   For any other error, 0 is returned. */
local z_size_t gz_write(gz_statep state, voidpc buf, z_size_t len) {
    z_size_t put = len;
    целое ret;

    /* if len is zero, avoid unnecessary operations */
    если (len == 0)
        вернуть 0;

    /* allocate memory if this is the first time through */
    если (state->size == 0 && gz_init(state) == -1)
        вернуть 0;

    /* check for seek request */
    если (state->skip && gz_zero(state) == -1)
        вернуть 0;

    /* for small len, copy to input buffer, otherwise compress directly */
    если (len < state->size) {
        /* copy to input buffer, compress when full */
        для (;;) {
            unsigned have, copy;

            если (state->strm.avail_in == 0)
                state->strm.next_in = state->in;
            have = (unsigned)((state->strm.next_in + state->strm.avail_in) -
                              state->in);
            copy = state->size - have;
            если (copy > len)
                copy = (unsigned)len;
            memcpy(state->in + have, buf, copy);
            state->strm.avail_in += copy;
            state->x.pos += copy;
            buf = (const char *)buf + copy;
            len -= copy;
            если (len == 0)
                break;
            если (gz_comp(state, Z_NO_FLUSH) == -1)
                вернуть state->again ? put - len : 0;
        }
    }
    иначе {
        /* consume whatever's left in the input buffer */
        если (state->strm.avail_in && gz_comp(state, Z_NO_FLUSH) == -1)
            вернуть 0;

        /* directly compress user buffer to file */
        state->strm.next_in = (z_const Bytef *)buf;
        do {
            unsigned n = (unsigned)-1;

            если (n > len)
                n = (unsigned)len;
            state->strm.avail_in = n;
            ret = gz_comp(state, Z_NO_FLUSH);
            n -= state->strm.avail_in;
            state->x.pos += n;
            len -= n;
            если (ret == -1) {
                state->strm.avail_in = 0;
                state->strm.next_in = state->in;
                вернуть state->again ? put - len : 0;
            }
        } пока (len);
    }

    /* input was all buffered or compressed */
    вернуть put;
}

/* -- see zlib.h -- */
целое ZEXPORT gzwrite(gzFile file, voidpc buf, unsigned len) {
    gz_statep state;

    /* get internal structure */
    если (file == NULL)
        вернуть 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        вернуть 0;
    gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    если ((целое)len < 0) {
        gz_error(state, Z_DATA_ERROR, "requested length does not fit in int");
        вернуть 0;
    }

    /* write len bytes from buf (the return value will fit in an int) */
    вернуть (целое)gz_write(state, buf, len);
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfwrite(voidpc buf, z_size_t size, z_size_t nitems,
                          gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure */
    если (file == NULL)
        вернуть 0;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        вернуть 0;
    gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    если (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        вернуть 0;
    }

    /* write len bytes to buf, return the number of full items written */
    вернуть len ? gz_write(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
целое ZEXPORT gzputc(gzFile file, целое c) {
    unsigned have;
    unsigned char buf[1];
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    если (file == NULL)
        вернуть -1;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        вернуть -1;
    gz_error(state, Z_OK, NULL);

    /* check for seek request */
    если (state->skip && gz_zero(state) == -1)
        вернуть -1;

    /* try writing to input buffer for speed (state->size == 0 if buffer not
       initialized) */
    если (state->size) {
        если (strm->avail_in == 0)
            strm->next_in = state->in;
        have = (unsigned)((strm->next_in + strm->avail_in) - state->in);
        если (have < state->size) {
            state->in[have] = (unsigned char)c;
            strm->avail_in++;
            state->x.pos++;
            вернуть c & 0xff;
        }
    }

    /* no room in buffer or not initialized, use gz_write() */
    buf[0] = (unsigned char)c;
    если (gz_write(state, buf, 1) != 1)
        вернуть -1;
    вернуть c & 0xff;
}

/* -- see zlib.h -- */
целое ZEXPORT gzputs(gzFile file, const char *s) {
    z_size_t len, put;
    gz_statep state;

    /* get internal structure */
    если (file == NULL)
        вернуть -1;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        вернуть -1;
    gz_error(state, Z_OK, NULL);

    /* write string */
    len = strlen(s);
    если ((целое)len < 0 || (unsigned)len != len) {
        gz_error(state, Z_STREAM_ERROR, "string length does not fit in int");
        вернуть -1;
    }
    put = gz_write(state, s, len);
    вернуть len && put == 0 ? -1 : (целое)put;
}

#если (((!defined(STDC) && !defined(Z_HAVE_STDARG_H)) || !defined(NO_vsnprintf)) && \
     (defined(STDC) || defined(Z_HAVE_STDARG_H) || !defined(NO_snprintf))) || \
    defined(ZLIB_INSECURE)
/* If the second half of the input buffer is occupied, write out the contents.
   If there is any input remaining due to a non-blocking stall on write, move
   it to the start of the buffer. Return true if this did not open up the
   second half of the buffer.  state->err should be checked after this to
   handle a gz_comp() error. */
local целое gz_vacate(gz_statep state) {
    z_streamp strm;

    strm = &(state->strm);
    если (strm->next_in == NULL ||
        strm->next_in + strm->avail_in <= state->in + state->size)
        вернуть 0;
    (void)gz_comp(state, Z_NO_FLUSH);
    если (strm->avail_in == 0) {
        strm->next_in = state->in;
        вернуть 0;
    }
    memmove(state->in, strm->next_in, strm->avail_in);
    strm->next_in = state->in;
    вернуть strm->avail_in > state->size;
}
#endif

#если defined(STDC) || defined(Z_HAVE_STDARG_H)
#include <stdarg.h>

/* -- see zlib.h -- */
целое ZEXPORTVA gzvprintf(gzFile file, const char *format, va_list va) {
#если defined(NO_vsnprintf) && !defined(ZLIB_INSECURE)
#warning "vsnprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use vsprintf()"
    /* prevent use of insecure vsprintf(), unless purposefully requested */
    (void)file, (void)format, (void)va;
    вернуть Z_STREAM_ERROR;
#иначе
    целое len, ret;
    char *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    если (file == NULL)
        вернуть Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're writing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        вернуть Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    если (state->size == 0 && gz_init(state) == -1)
        вернуть state->err;

    /* check for seek request */
    если (state->skip && gz_zero(state) == -1)
        вернуть state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there should be
       state->size bytes available after the current contents */
    ret = gz_vacate(state);
    если (state->err) {
        если (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            вернуть state->err;
        }
        если (!state->again)
            вернуть state->err;
    }
    если (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (char *)(state->in + (strm->next_in - state->in) + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(next, format, va);
    для (len = 0; len < state->size; len++)
        если (next[len] == 0) break;
#  иначе
    len = vsprintf(next, format, va);
#  endif
#иначе
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(next, state->size, format, va);
    len = strlen(next);
#  иначе
    len = vsnprintf(next, state->size, format, va);
#  endif
#endif

    /* check that printf() results fit in buffer */
    если (len == 0 || (unsigned)len >= state->size || next[state->size - 1] != 0)
        вернуть 0;

    /* update buffer and position */
    strm->avail_in += (unsigned)len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    если (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        если (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        вернуть state->err;
    }
    если (state->err && !state->again)
        вернуть state->err;
    вернуть len;
#endif
}

целое ZEXPORTVA gzprintf(gzFile file, const char *format, ...) {
    va_list va;
    целое ret;

    va_start(va, format);
    ret = gzvprintf(file, format, va);
    va_end(va);
    вернуть ret;
}

#иначе /* !STDC && !Z_HAVE_STDARG_H */

/* -- see zlib.h -- */
целое ZEXPORTVA gzprintf(gzFile file, const char *format, целое a1, целое a2, целое a3,
                       целое a4, целое a5, целое a6, целое a7, целое a8, целое a9, целое a10,
                       целое a11, целое a12, целое a13, целое a14, целое a15, целое a16,
                       целое a17, целое a18, целое a19, целое a20) {
#если defined(NO_snprintf) && !defined(ZLIB_INSECURE)
#warning "snprintf() not available -- gzprintf() stub returns Z_STREAM_ERROR"
#warning "you can recompile with ZLIB_INSECURE defined to use sprintf()"
    /* prevent use of insecure sprintf(), unless purposefully requested */
    (void)file, (void)format, (void)a1, (void)a2, (void)a3, (void)a4, (void)a5,
    (void)a6, (void)a7, (void)a8, (void)a9, (void)a10, (void)a11, (void)a12,
    (void)a13, (void)a14, (void)a15, (void)a16, (void)a17, (void)a18,
    (void)a19, (void)a20;
    вернуть Z_STREAM_ERROR;
#иначе
    целое ret;
    unsigned len, left;
    char *next;
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    если (file == NULL)
        вернуть Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that can really pass pointer in ints */
    если (sizeof(целое) != sizeof(void *))
        вернуть Z_STREAM_ERROR;

    /* check that we're writing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        вернуть Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* make sure we have some buffer space */
    если (state->size == 0 && gz_init(state) == -1)
        вернуть state->err;

    /* check for seek request */
    если (state->skip && gz_zero(state) == -1)
        вернуть state->err;

    /* do the printf() into the input buffer, put length in len -- the input
       buffer is double-sized just for this function, so there is guaranteed to
       be state->size bytes available after the current contents */
    ret = gz_vacate(state);
    если (state->err) {
        если (ret && state->again) {
            /* There was a non-blocking stall on write, resulting in the part
               of the second half of the output buffer being occupied.  Return
               a Z_BUF_ERROR to let the application know that this gzprintf()
               needs to be retried. */
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
            вернуть state->err;
        }
        если (!state->again)
            вернуть state->err;
    }
    если (strm->avail_in == 0)
        strm->next_in = state->in;
    next = (char *)(strm->next_in + strm->avail_in);
    next[state->size - 1] = 0;
#ifdef NO_snprintf
#  ifdef HAS_sprintf_void
    sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
            a13, a14, a15, a16, a17, a18, a19, a20);
    для (len = 0; len < size; len++)
        если (next[len] == 0)
            break;
#  иначе
    len = sprintf(next, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11,
                  a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#иначе
#  ifdef HAS_snprintf_void
    snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8, a9,
             a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
    len = strlen(next);
#  иначе
    len = snprintf(next, state->size, format, a1, a2, a3, a4, a5, a6, a7, a8,
                   a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#  endif
#endif

    /* check that printf() results fit in buffer */
    если (len == 0 || len >= state->size || next[state->size - 1] != 0)
        вернуть 0;

    /* update buffer and position, compress first half if past that */
    strm->avail_in += len;
    state->x.pos += len;

    /* write out buffer if more than half is occupied */
    ret = gz_vacate(state);
    если (ret) {
        /* discard unwritten formatted output */
        strm->avail_in -= (unsigned)len;
        state->x.pos -= len;
        если (state->again)
            gz_error(state, Z_BUF_ERROR, "stalled write on gzprintf");
        вернуть state->err;
    }
    если (state->err && !state->again)
        вернуть state->err;
    вернуть (целое)len;
#endif
}

#endif

/* -- see zlib.h -- */
целое ZEXPORT gzflush(gzFile file, целое flush) {
    gz_statep state;

    /* get internal structure */
    если (file == NULL)
        вернуть Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again))
        вернуть Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* check flush parameter */
    если (flush < 0 || flush > Z_FINISH)
        вернуть Z_STREAM_ERROR;

    /* check for seek request */
    если (state->skip && gz_zero(state) == -1)
        вернуть state->err;

    /* compress remaining data with requested flush */
    (void)gz_comp(state, flush);
    вернуть state->err;
}

/* -- see zlib.h -- */
целое ZEXPORT gzsetparams(gzFile file, целое level, целое strategy) {
    gz_statep state;
    z_streamp strm;

    /* get internal structure */
    если (file == NULL)
        вернуть Z_STREAM_ERROR;
    state = (gz_statep)file;
    strm = &(state->strm);

    /* check that we're compressing and that there's no (serious) error */
    если (state->mode != GZ_WRITE || (state->err != Z_OK && !state->again) ||
            state->direct)
        вернуть Z_STREAM_ERROR;
    gz_error(state, Z_OK, NULL);

    /* if no change is requested, then do nothing */
    если (level == state->level && strategy == state->strategy)
        вернуть Z_OK;

    /* check for seek request */
    если (state->skip && gz_zero(state) == -1)
        вернуть state->err;

    /* change compression parameters for subsequent input */
    если (state->size) {
        /* flush previous input with previous parameters before changing */
        если (strm->avail_in && gz_comp(state, Z_BLOCK) == -1)
            вернуть state->err;
        deflateParams(strm, level, strategy);
    }
    state->level = level;
    state->strategy = strategy;
    вернуть Z_OK;
}

/* -- see zlib.h -- */
целое ZEXPORT gzclose_w(gzFile file) {
    целое ret = Z_OK;
    gz_statep state;

    /* get internal structure */
    если (file == NULL)
        вернуть Z_STREAM_ERROR;
    state = (gz_statep)file;

    /* check that we're writing */
    если (state->mode != GZ_WRITE)
        вернуть Z_STREAM_ERROR;

    /* check for seek request */
    если (state->skip && gz_zero(state) == -1)
        ret = state->err;

    /* flush, free memory, and close file */
    если (gz_comp(state, Z_FINISH) == -1)
        ret = state->err;
    если (state->size) {
        если (!state->direct) {
            (void)deflateEnd(&(state->strm));
            free(state->out);
        }
        free(state->in);
    }
    gz_error(state, Z_OK, NULL);
    free(state->path);
    если (close(state->fd) == -1)
        ret = Z_ERRNO;
    free(state);
    вернуть ret;
}
