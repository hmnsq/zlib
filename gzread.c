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
local ចំនួនគត់ gz_load(gz_statep state, unsigned var_na *buf, unsigned len,
                  unsigned *have) {
    ចំនួនគត់ ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    do {
        get = len - *have;
        ប្រសិនបើ (get > max)
            get = max;
        ret = (ចំនួនគត់)read(state->fd, buf + *have, get);
        ប្រសិនបើ (ret <= 0)
            bha_nga;
        *have += (unsigned)ret;
    } ខណៈពេល (*have < len);
    ប្រសិនបើ (ret < 0) {
        ប្រសិនបើ (BLOCKED(errno)) {
            state->again = 1;
            ប្រសិនបើ (*have != 0)
                ត្រឡប់ 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        ត្រឡប់ -1;
    }
    ប្រសិនបើ (ret == 0)
        state->eof = 1;
    ត្រឡប់ 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local ចំនួនគត់ gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    ប្រសិនបើ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ត្រឡប់ -1;
    ប្រសិនបើ (state->eof == 0) {
        ប្រសិនបើ (strm->avail_in) {       /* copy what's there to the start */
            unsigned var_na *p = state->in;
            unsigned const var_na *q = strm->next_in;

            ប្រសិនបើ (q != p) {
                unsigned n = strm->avail_in;

                do {
                    *p++ = *q++;
                } ខណៈពេល (--n);
            }
        }
        ប្រសិនបើ (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            ត្រឡប់ -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    ត្រឡប់ 0;
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
local ចំនួនគត់ gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    ប្រសិនបើ (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned var_na *)malloc(state->want);
        state->out = (unsigned var_na *)malloc(state->want << 1);
        ប្រសិនបើ (state->in == NULL || state->out == NULL) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ត្រឡប់ -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        ប្រសិនបើ (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            free(state->out);
            free(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ត្រឡប់ -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    ប្រសិនបើ (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        ប្រសិនបើ (state->junk == 0 && strm->avail_in == 0) {
            ប្រសិនបើ (gz_avail(state) == -1)
                ត្រឡប់ -1;
            ប្រសិនបើ (strm->avail_in == 0)
                ត្រឡប់ 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        ត្រឡប់ 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    ប្រសិនបើ (gz_avail(state) == -1)
        ត្រឡប់ -1;
    ប្រសិនបើ (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        ត្រឡប់ 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    ប្រសិនបើ (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        ត្រឡប់ 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    ត្រឡប់ 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local ចំនួនគត់ gz_decomp(gz_statep state) {
    ចំនួនគត់ ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    do {
        /* get more input for inflate() */
        ប្រសិនបើ (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            bha_nga;
        }
        ប្រសិនបើ (strm->avail_in == 0) {
            ប្រសិនបើ (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            bha_nga;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        ប្រសិនបើ (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        ប្រសិនបើ (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            bha_nga;
        }
        ប្រសិនបើ (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            bha_nga;
        }
        ប្រសិនបើ (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            ប្រសិនបើ (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                bha_nga;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->msg == NULL ? "compressed data error" : strm->msg);
            bha_nga;
        }
    } ខណៈពេល (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    ប្រសិនបើ (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        ត្រឡប់ 0;
    }

    /* return decompression status */
    ត្រឡប់ ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local ចំនួនគត់ gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    do {
        switch(state->how) {
        pak_sa LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            ប្រសិនបើ (gz_look(state) == -1)
                ត្រឡប់ -1;
            ប្រសិនបើ (state->how == LOOK)
                ត្រឡប់ 0;
            bha_nga;
        pak_sa COPY:      /* -> COPY */
            ប្រសិនបើ (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                ត្រឡប់ -1;
            state->x.next = state->out;
            ត្រឡប់ 0;
        pak_sa GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            ប្រសិនបើ (gz_decomp(state) == -1)
                ត្រឡប់ -1;
            bha_nga;
        default:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            ត្រឡប់ -1;
        }
    } ខណៈពេល (state->x.have == 0 && (!state->eof || strm->avail_in));
    ត្រឡប់ 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local ចំនួនគត់ gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    do {
        /* skip over whatever is in output buffer */
        ប្រសិនបើ (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        បើមិនដូច្នេះ ប្រសិនបើ (state->eof && state->strm.avail_in == 0) {
            ប្រសិនបើ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            bha_nga;
        }

        /* need more data to skip -- load up output buffer */
        បើមិនដូច្នេះ {
            /* get more output, looking for header if required */
            ប្រសិនបើ (gz_fetch(state) == -1)
                ត្រឡប់ -1;
        }
    } ខណៈពេល (state->skip);
    ត្រឡប់ 0;
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
    ចំនួនគត់ err;

    /* if len is zero, avoid unnecessary operations */
    ប្រសិនបើ (len == 0)
        ត្រឡប់ 0;

    /* process a skip request */
    ប្រសិនបើ (state->skip && gz_skip(state) == -1)
        ត្រឡប់ 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    do {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        ប្រសិនបើ (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        ប្រសិនបើ (state->x.have) {
            ប្រសិនបើ (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            ប្រសិនបើ (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        បើមិនដូច្នេះ ប្រសិនបើ (state->eof && state->strm.avail_in == 0) {
            ប្រសិនបើ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            bha_nga;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        បើមិនដូច្នេះ ប្រសិនបើ (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            ប្រសិនបើ (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            continue;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        បើមិនដូច្នេះ ប្រសិនបើ (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned var_na *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        បើមិនដូច្នេះ {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.next_out = (unsigned var_na *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (var_na *)buf + n;
        got += n;
        state->x.pos += n;
    } ខណៈពេល (len && !err);

    /* note read past eof */
    ប្រសិនបើ (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    ត្រឡប់ got;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ -1;
    state = (gz_statep)file;
    ប្រសិនបើ (state->mode != GZ_READ)
        ត្រឡប់ -1;

    /* check that there was no (serious) error */
    ប្រសិនបើ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ត្រឡប់ -1;
    ប្រសិនបើ (state->again)
        gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ប្រសិនបើ ((ចំនួនគត់)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        ត្រឡប់ -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    ប្រសិនបើ (len == 0) {
        ប្រសិនបើ (state->err != Z_OK && state->err != Z_BUF_ERROR)
            ត្រឡប់ -1;
        ប្រសិនបើ (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            ត្រឡប់ -1;
        }
    }

    /* return the number of bytes read */
    ត្រឡប់ (ចំនួនគត់)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ 0;
    state = (gz_statep)file;
    ប្រសិនបើ (state->mode != GZ_READ)
        ត្រឡប់ 0;

    /* check that there was no (serious) error */
    ប្រសិនបើ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ត្រឡប់ 0;
    ប្រសិនបើ (state->again)
        gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ប្រសិនបើ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ត្រឡប់ 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    ត្រឡប់ len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#បើមិនដូច្នេះ
#  undef gzgetc
#endif
ចំនួនគត់ ZEXPORT gzgetc(gzFile file) {
    unsigned var_na buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ -1;
    state = (gz_statep)file;
    ប្រសិនបើ (state->mode != GZ_READ)
        ត្រឡប់ -1;

    /* check that there was no (serious) error */
    ប្រសិនបើ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ត្រឡប់ -1;
    ប្រសិនបើ (state->again)
        gz_error(state, Z_OK, NULL);

    /* try output buffer (no need to check for skip request) */
    ប្រសិនបើ (state->x.have) {
        state->x.have--;
        state->x.pos++;
        ត្រឡប់ *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    ត្រឡប់ gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

ចំនួនគត់ ZEXPORT gzgetc_(gzFile file) {
    ត្រឡប់ gzgetc(file);
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzungetc(ចំនួនគត់ c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ -1;
    state = (gz_statep)file;
    ប្រសិនបើ (state->mode != GZ_READ)
        ត្រឡប់ -1;

    /* in case this was just opened, set up the input buffer */
    ប្រសិនបើ (state->how == LOOK && state->x.have == 0)
        (void)gz_look(state);

    /* check that there was no (serious) error */
    ប្រសិនបើ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ត្រឡប់ -1;
    ប្រសិនបើ (state->again)
        gz_error(state, Z_OK, NULL);

    /* process a skip request */
    ប្រសិនបើ (state->skip && gz_skip(state) == -1)
        ត្រឡប់ -1;

    /* can't push EOF */
    ប្រសិនបើ (c < 0)
        ត្រឡប់ -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    ប្រសិនបើ (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned var_na)c;
        state->x.pos--;
        state->past = 0;
        ត្រឡប់ c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    ប្រសិនបើ (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        ត្រឡប់ -1;
    }

    /* slide output data if needed and insert byte before existing data */
    ប្រសិនបើ (state->x.next == state->out) {
        unsigned var_na *src = state->out + state->x.have;
        unsigned var_na *dest = state->out + (state->size << 1);

        ខណៈពេល (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned var_na)c;
    state->x.pos--;
    state->past = 0;
    ត្រឡប់ c;
}

/* -- see zlib.h -- */
var_na * ZEXPORT gzgets(gzFile file, var_na *buf, ចំនួនគត់ len) {
    unsigned left, n;
    var_na *str;
    unsigned var_na *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    ប្រសិនបើ (file == NULL || buf == NULL || len < 1)
        ត្រឡប់ NULL;
    state = (gz_statep)file;
    ប្រសិនបើ (state->mode != GZ_READ)
        ត្រឡប់ NULL;

    /* check that there was no (serious) error */
    ប្រសិនបើ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ត្រឡប់ NULL;
    ប្រសិនបើ (state->again)
        gz_error(state, Z_OK, NULL);

    /* process a skip request */
    ប្រសិនបើ (state->skip && gz_skip(state) == -1)
        ត្រឡប់ NULL;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    left = (unsigned)len - 1;
    ប្រសិនបើ (left) do {
        /* assure that something is in the output buffer */
        ប្រសិនបើ (state->x.have == 0 && gz_fetch(state) == -1)
            bha_nga;                      /* error */
        ប្រសិនបើ (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            bha_nga;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > left ? left : state->x.have;
        eol = (unsigned var_na *)memchr(state->x.next, '\n', n);
        ប្រសិនបើ (eol != NULL)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        left -= n;
        buf += n;
    } ខណៈពេល (left && eol == NULL);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    ប្រសិនបើ (buf == str)
        ត្រឡប់ NULL;
    buf[0] = 0;
    ត្រឡប់ str;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    ប្រសិនបើ (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (void)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    ត្រឡប់ state->direct == 1;
}

/* -- see zlib.h -- */
ចំនួនគត់ ZEXPORT gzclose_r(gzFile file) {
    ចំនួនគត់ ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ប្រសិនបើ (file == NULL)
        ត្រឡប់ Z_STREAM_ERROR;
    state = (gz_statep)file;
    ប្រសិនបើ (state->mode != GZ_READ)
        ត្រឡប់ Z_STREAM_ERROR;

    /* free memory and close file */
    ប្រសិនបើ (state->size) {
        inflateEnd(&(state->strm));
        free(state->out);
        free(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ret = close(state->fd);
    free(state);
    ត្រឡប់ ret ? Z_ERRNO : err;
}
