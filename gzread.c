/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* Use read() to load a buffer -- return -1 on error, otherwise 0.  Read from
   state->fd, and update state->eof, state->err, and state->msg as appropriate.
   This function needs to loop on read(), since read() is not guaranteed to
   read the number of bytes requested, depending on the type of descriptor. It
   also needs to loop to manage the fact that read() returns an int. If the
   descriptor is non-blocking and read() returns with no data in order to avoid
   blocking, then gz_load() will return 0 if some data has been read, or -1 if
   no data has been read. Either way, state->again is set true to indicate a
   non-blocking event. If errno is non-zero on return, then there was an error
   signaled from read().  *have is set to the number of bytes read. */
local 整数 gz_load(gz_statep state, unsigned char *buf, unsigned len,
                  unsigned *have) {
    整数 ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    do {
        get = len - *have;
        もし (get > max)
            get = max;
        ret = (整数)read(state->fd, buf + *have, get);
        もし (ret <= 0)
            break;
        *have += (unsigned)ret;
    } ながら (*have < len);
    もし (ret < 0) {
        もし (BLOCKED(errno)) {
            state->again = 1;
            もし (*have != 0)
                戻る 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        戻る -1;
    }
    もし (ret == 0)
        state->eof = 1;
    戻る 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local 整数 gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    もし (state->err != Z_OK && state->err != Z_BUF_ERROR)
        戻る -1;
    もし (state->eof == 0) {
        もし (strm->avail_in) {       /* copy what's there to the start */
            unsigned char *p = state->in;
            unsigned const char *q = strm->next_in;

            もし (q != p) {
                unsigned n = strm->avail_in;

                do {
                    *p++ = *q++;
                } ながら (--n);
            }
        }
        もし (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            戻る -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    戻る 0;
}

/* Look for gzip header, set up for inflate or copy.  state->x.have must be 0.
   If this is the first time in, allocate required memory.  state->how will be
   left unchanged if there is no more input data available, will be set to COPY
   if there is no gzip header and direct copying will be performed, or it will
   be set to GZIP for decompression.  If direct copying, then leftover input
   data from the input buffer will be copied to the output buffer.  In that
   case, all further file reads will be directly to either the output buffer or
   a user buffer.  If decompressing, the inflate state will be initialized.
   gz_look() will return 0 on success or -1 on failure. */
local 整数 gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    もし (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned char *)malloc(state->want);
        state->out = (unsigned char *)malloc(state->want << 1);
        もし (state->in == NULL || state->out == NULL) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            戻る -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        もし (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            free(state->out);
            free(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            戻る -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    もし (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        もし (state->junk == 0 && strm->avail_in == 0) {
            もし (gz_avail(state) == -1)
                戻る -1;
            もし (strm->avail_in == 0)
                戻る 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        戻る 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    もし (gz_avail(state) == -1)
        戻る -1;
    もし (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        戻る 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    もし (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        戻る 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    戻る 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local 整数 gz_decomp(gz_statep state) {
    整数 ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    do {
        /* get more input for inflate() */
        もし (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            break;
        }
        もし (strm->avail_in == 0) {
            もし (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            break;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        もし (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        もし (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            break;
        }
        もし (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            break;
        }
        もし (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            もし (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                break;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->msg == NULL ? "compressed data error" : strm->msg);
            break;
        }
    } ながら (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    もし (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        戻る 0;
    }

    /* return decompression status */
    戻る ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local 整数 gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    do {
        switch(state->how) {
        case LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            もし (gz_look(state) == -1)
                戻る -1;
            もし (state->how == LOOK)
                戻る 0;
            break;
        case COPY:      /* -> COPY */
            もし (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                戻る -1;
            state->x.next = state->out;
            戻る 0;
        case GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            もし (gz_decomp(state) == -1)
                戻る -1;
            break;
        default:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            戻る -1;
        }
    } ながら (state->x.have == 0 && (!state->eof || strm->avail_in));
    戻る 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local 整数 gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    do {
        /* skip over whatever is in output buffer */
        もし (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        否则 もし (state->eof && state->strm.avail_in == 0) {
            もし (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            break;
        }

        /* need more data to skip -- load up output buffer */
        否则 {
            /* get more output, looking for header if required */
            もし (gz_fetch(state) == -1)
                戻る -1;
        }
    } ながら (state->skip);
    戻る 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_size_t gz_read(gz_statep state, voidp buf, z_size_t len) {
    z_size_t got;
    unsigned n;
    整数 err;

    /* if len is zero, avoid unnecessary operations */
    もし (len == 0)
        戻る 0;

    /* process a skip request */
    もし (state->skip && gz_skip(state) == -1)
        戻る 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    do {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        もし (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        もし (state->x.have) {
            もし (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            もし (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        否则 もし (state->eof && state->strm.avail_in == 0) {
            もし (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            break;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        否则 もし (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            もし (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            continue;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        否则 もし (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned char *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        否则 {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.next_out = (unsigned char *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (char *)buf + n;
        got += n;
        state->x.pos += n;
    } ながら (len && !err);

    /* note read past eof */
    もし (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    戻る got;
}

/* -- see zlib.h -- */
整数 ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    もし (file == NULL)
        戻る -1;
    state = (gz_statep)file;
    もし (state->mode != GZ_READ)
        戻る -1;

    /* check that there was no (serious) error */
    もし (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        戻る -1;
    もし (state->again)
        gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    もし ((整数)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        戻る -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    もし (len == 0) {
        もし (state->err != Z_OK && state->err != Z_BUF_ERROR)
            戻る -1;
        もし (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            戻る -1;
        }
    }

    /* return the number of bytes read */
    戻る (整数)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    もし (file == NULL)
        戻る 0;
    state = (gz_statep)file;
    もし (state->mode != GZ_READ)
        戻る 0;

    /* check that there was no (serious) error */
    もし (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        戻る 0;
    もし (state->again)
        gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    もし (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        戻る 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    戻る len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#否则
#  undef gzgetc
#endif
整数 ZEXPORT gzgetc(gzFile file) {
    unsigned char buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    もし (file == NULL)
        戻る -1;
    state = (gz_statep)file;
    もし (state->mode != GZ_READ)
        戻る -1;

    /* check that there was no (serious) error */
    もし (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        戻る -1;
    もし (state->again)
        gz_error(state, Z_OK, NULL);

    /* try output buffer (no need to check for skip request) */
    もし (state->x.have) {
        state->x.have--;
        state->x.pos++;
        戻る *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    戻る gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

整数 ZEXPORT gzgetc_(gzFile file) {
    戻る gzgetc(file);
}

/* -- see zlib.h -- */
整数 ZEXPORT gzungetc(整数 c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    もし (file == NULL)
        戻る -1;
    state = (gz_statep)file;
    もし (state->mode != GZ_READ)
        戻る -1;

    /* in case this was just opened, set up the input buffer */
    もし (state->how == LOOK && state->x.have == 0)
        (void)gz_look(state);

    /* check that there was no (serious) error */
    もし (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        戻る -1;
    もし (state->again)
        gz_error(state, Z_OK, NULL);

    /* process a skip request */
    もし (state->skip && gz_skip(state) == -1)
        戻る -1;

    /* can't push EOF */
    もし (c < 0)
        戻る -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    もし (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned char)c;
        state->x.pos--;
        state->past = 0;
        戻る c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    もし (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        戻る -1;
    }

    /* slide output data if needed and insert byte before existing data */
    もし (state->x.next == state->out) {
        unsigned char *src = state->out + state->x.have;
        unsigned char *dest = state->out + (state->size << 1);

        ながら (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned char)c;
    state->x.pos--;
    state->past = 0;
    戻る c;
}

/* -- see zlib.h -- */
char * ZEXPORT gzgets(gzFile file, char *buf, 整数 len) {
    unsigned left, n;
    char *str;
    unsigned char *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    もし (file == NULL || buf == NULL || len < 1)
        戻る NULL;
    state = (gz_statep)file;
    もし (state->mode != GZ_READ)
        戻る NULL;

    /* check that there was no (serious) error */
    もし (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        戻る NULL;
    もし (state->again)
        gz_error(state, Z_OK, NULL);

    /* process a skip request */
    もし (state->skip && gz_skip(state) == -1)
        戻る NULL;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    left = (unsigned)len - 1;
    もし (left) do {
        /* assure that something is in the output buffer */
        もし (state->x.have == 0 && gz_fetch(state) == -1)
            break;                      /* error */
        もし (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            break;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > left ? left : state->x.have;
        eol = (unsigned char *)memchr(state->x.next, '\n', n);
        もし (eol != NULL)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        left -= n;
        buf += n;
    } ながら (left && eol == NULL);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    もし (buf == str)
        戻る NULL;
    buf[0] = 0;
    戻る str;
}

/* -- see zlib.h -- */
整数 ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    もし (file == NULL)
        戻る 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    もし (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (void)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    戻る state->direct == 1;
}

/* -- see zlib.h -- */
整数 ZEXPORT gzclose_r(gzFile file) {
    整数 ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    もし (file == NULL)
        戻る Z_STREAM_ERROR;
    state = (gz_statep)file;
    もし (state->mode != GZ_READ)
        戻る Z_STREAM_ERROR;

    /* free memory and close file */
    もし (state->size) {
        inflateEnd(&(state->strm));
        free(state->out);
        free(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ret = close(state->fd);
    free(state);
    戻る ret ? Z_ERRNO : err;
}
