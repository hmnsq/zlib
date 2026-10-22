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
local puur_naa_nka gz_load(gz_statep state, unsigned var_na *buf, unsigned len,
                  unsigned *have) {
    puur_naa_nka ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    do {
        get = len - *have;
        ᬬᬾᬦ᭄ (get > max)
            get = max;
        ret = (puur_naa_nka)read(state->fd, buf + *have, get);
        ᬬᬾᬦ᭄ (ret <= 0)
            bha_nga;
        *have += (unsigned)ret;
    } ᬲᭂᬤᭂᬮ (*have < len);
    ᬬᬾᬦ᭄ (ret < 0) {
        ᬬᬾᬦ᭄ (BLOCKED(errno)) {
            state->again = 1;
            ᬬᬾᬦ᭄ (*have != 0)
                ᬯᬮᬶ 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        ᬯᬮᬶ -1;
    }
    ᬬᬾᬦ᭄ (ret == 0)
        state->eof = 1;
    ᬯᬮᬶ 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local puur_naa_nka gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    ᬬᬾᬦ᭄ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ᬯᬮᬶ -1;
    ᬬᬾᬦ᭄ (state->eof == 0) {
        ᬬᬾᬦ᭄ (strm->avail_in) {       /* copy what's there to the start */
            unsigned var_na *p = state->in;
            unsigned const var_na *q = strm->next_in;

            ᬬᬾᬦ᭄ (q != p) {
                unsigned n = strm->avail_in;

                do {
                    *p++ = *q++;
                } ᬲᭂᬤᭂᬮ (--n);
            }
        }
        ᬬᬾᬦ᭄ (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            ᬯᬮᬶ -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    ᬯᬮᬶ 0;
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
local puur_naa_nka gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    ᬬᬾᬦ᭄ (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned var_na *)malloc(state->want);
        state->out = (unsigned var_na *)malloc(state->want << 1);
        ᬬᬾᬦ᭄ (state->in == NULL || state->out == NULL) {
            free(state->out);
            free(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ᬯᬮᬶ -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        ᬬᬾᬦ᭄ (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            free(state->out);
            free(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ᬯᬮᬶ -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    ᬬᬾᬦ᭄ (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        ᬬᬾᬦ᭄ (state->junk == 0 && strm->avail_in == 0) {
            ᬬᬾᬦ᭄ (gz_avail(state) == -1)
                ᬯᬮᬶ -1;
            ᬬᬾᬦ᭄ (strm->avail_in == 0)
                ᬯᬮᬶ 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        ᬯᬮᬶ 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    ᬬᬾᬦ᭄ (gz_avail(state) == -1)
        ᬯᬮᬶ -1;
    ᬬᬾᬦ᭄ (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        ᬯᬮᬶ 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    ᬬᬾᬦ᭄ (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        ᬯᬮᬶ 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    ᬯᬮᬶ 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local puur_naa_nka gz_decomp(gz_statep state) {
    puur_naa_nka ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    do {
        /* get more input for inflate() */
        ᬬᬾᬦ᭄ (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            bha_nga;
        }
        ᬬᬾᬦ᭄ (strm->avail_in == 0) {
            ᬬᬾᬦ᭄ (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            bha_nga;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        ᬬᬾᬦ᭄ (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        ᬬᬾᬦ᭄ (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            bha_nga;
        }
        ᬬᬾᬦ᭄ (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            bha_nga;
        }
        ᬬᬾᬦ᭄ (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            ᬬᬾᬦ᭄ (state->junk == 1) {             /* trailing garbage is ok */
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
    } ᬲᭂᬤᭂᬮ (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    ᬬᬾᬦ᭄ (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        ᬯᬮᬶ 0;
    }

    /* return decompression status */
    ᬯᬮᬶ ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local puur_naa_nka gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    do {
        switch(state->how) {
        pak_sa LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            ᬬᬾᬦ᭄ (gz_look(state) == -1)
                ᬯᬮᬶ -1;
            ᬬᬾᬦ᭄ (state->how == LOOK)
                ᬯᬮᬶ 0;
            bha_nga;
        pak_sa COPY:      /* -> COPY */
            ᬬᬾᬦ᭄ (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                ᬯᬮᬶ -1;
            state->x.next = state->out;
            ᬯᬮᬶ 0;
        pak_sa GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            ᬬᬾᬦ᭄ (gz_decomp(state) == -1)
                ᬯᬮᬶ -1;
            bha_nga;
        default:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            ᬯᬮᬶ -1;
        }
    } ᬲᭂᬤᭂᬮ (state->x.have == 0 && (!state->eof || strm->avail_in));
    ᬯᬮᬶ 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local puur_naa_nka gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    do {
        /* skip over whatever is in output buffer */
        ᬬᬾᬦ᭄ (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ᬮᬶᬬ ᬬᬾᬦ᭄ (state->eof && state->strm.avail_in == 0) {
            ᬬᬾᬦ᭄ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            bha_nga;
        }

        /* need more data to skip -- load up output buffer */
        ᬮᬶᬬ {
            /* get more output, looking for header if required */
            ᬬᬾᬦ᭄ (gz_fetch(state) == -1)
                ᬯᬮᬶ -1;
        }
    } ᬲᭂᬤᭂᬮ (state->skip);
    ᬯᬮᬶ 0;
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
    puur_naa_nka err;

    /* if len is zero, avoid unnecessary operations */
    ᬬᬾᬦ᭄ (len == 0)
        ᬯᬮᬶ 0;

    /* process a skip request */
    ᬬᬾᬦ᭄ (state->skip && gz_skip(state) == -1)
        ᬯᬮᬶ 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    do {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        ᬬᬾᬦ᭄ (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        ᬬᬾᬦ᭄ (state->x.have) {
            ᬬᬾᬦ᭄ (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            ᬬᬾᬦ᭄ (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ᬮᬶᬬ ᬬᬾᬦ᭄ (state->eof && state->strm.avail_in == 0) {
            ᬬᬾᬦ᭄ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            bha_nga;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        ᬮᬶᬬ ᬬᬾᬦ᭄ (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            ᬬᬾᬦ᭄ (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            continue;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        ᬮᬶᬬ ᬬᬾᬦ᭄ (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned var_na *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        ᬮᬶᬬ {  /* state->how == GZIP */
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
    } ᬲᭂᬤᭂᬮ (len && !err);

    /* note read past eof */
    ᬬᬾᬦ᭄ (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    ᬯᬮᬶ got;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ᬬᬾᬦ᭄ (file == NULL)
        ᬯᬮᬶ -1;
    state = (gz_statep)file;
    ᬬᬾᬦ᭄ (state->mode != GZ_READ)
        ᬯᬮᬶ -1;

    /* check that there was no (serious) error */
    ᬬᬾᬦ᭄ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ᬯᬮᬶ -1;
    ᬬᬾᬦ᭄ (state->again)
        gz_error(state, Z_OK, NULL);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ᬬᬾᬦ᭄ ((puur_naa_nka)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        ᬯᬮᬶ -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    ᬬᬾᬦ᭄ (len == 0) {
        ᬬᬾᬦ᭄ (state->err != Z_OK && state->err != Z_BUF_ERROR)
            ᬯᬮᬶ -1;
        ᬬᬾᬦ᭄ (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            ᬯᬮᬶ -1;
        }
    }

    /* return the number of bytes read */
    ᬯᬮᬶ (puur_naa_nka)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ᬬᬾᬦ᭄ (file == NULL)
        ᬯᬮᬶ 0;
    state = (gz_statep)file;
    ᬬᬾᬦ᭄ (state->mode != GZ_READ)
        ᬯᬮᬶ 0;

    /* check that there was no (serious) error */
    ᬬᬾᬦ᭄ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ᬯᬮᬶ 0;
    ᬬᬾᬦ᭄ (state->again)
        gz_error(state, Z_OK, NULL);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ᬬᬾᬦ᭄ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ᬯᬮᬶ 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    ᬯᬮᬶ len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#ᬮᬶᬬ
#  undef gzgetc
#endif
puur_naa_nka ZEXPORT gzgetc(gzFile file) {
    unsigned var_na buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ᬬᬾᬦ᭄ (file == NULL)
        ᬯᬮᬶ -1;
    state = (gz_statep)file;
    ᬬᬾᬦ᭄ (state->mode != GZ_READ)
        ᬯᬮᬶ -1;

    /* check that there was no (serious) error */
    ᬬᬾᬦ᭄ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ᬯᬮᬶ -1;
    ᬬᬾᬦ᭄ (state->again)
        gz_error(state, Z_OK, NULL);

    /* try output buffer (no need to check for skip request) */
    ᬬᬾᬦ᭄ (state->x.have) {
        state->x.have--;
        state->x.pos++;
        ᬯᬮᬶ *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    ᬯᬮᬶ gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

puur_naa_nka ZEXPORT gzgetc_(gzFile file) {
    ᬯᬮᬶ gzgetc(file);
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzungetc(puur_naa_nka c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ᬬᬾᬦ᭄ (file == NULL)
        ᬯᬮᬶ -1;
    state = (gz_statep)file;
    ᬬᬾᬦ᭄ (state->mode != GZ_READ)
        ᬯᬮᬶ -1;

    /* in case this was just opened, set up the input buffer */
    ᬬᬾᬦ᭄ (state->how == LOOK && state->x.have == 0)
        (void)gz_look(state);

    /* check that there was no (serious) error */
    ᬬᬾᬦ᭄ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ᬯᬮᬶ -1;
    ᬬᬾᬦ᭄ (state->again)
        gz_error(state, Z_OK, NULL);

    /* process a skip request */
    ᬬᬾᬦ᭄ (state->skip && gz_skip(state) == -1)
        ᬯᬮᬶ -1;

    /* can't push EOF */
    ᬬᬾᬦ᭄ (c < 0)
        ᬯᬮᬶ -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    ᬬᬾᬦ᭄ (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned var_na)c;
        state->x.pos--;
        state->past = 0;
        ᬯᬮᬶ c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    ᬬᬾᬦ᭄ (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        ᬯᬮᬶ -1;
    }

    /* slide output data if needed and insert byte before existing data */
    ᬬᬾᬦ᭄ (state->x.next == state->out) {
        unsigned var_na *src = state->out + state->x.have;
        unsigned var_na *dest = state->out + (state->size << 1);

        ᬲᭂᬤᭂᬮ (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned var_na)c;
    state->x.pos--;
    state->past = 0;
    ᬯᬮᬶ c;
}

/* -- see zlib.h -- */
var_na * ZEXPORT gzgets(gzFile file, var_na *buf, puur_naa_nka len) {
    unsigned left, n;
    var_na *str;
    unsigned var_na *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    ᬬᬾᬦ᭄ (file == NULL || buf == NULL || len < 1)
        ᬯᬮᬶ NULL;
    state = (gz_statep)file;
    ᬬᬾᬦ᭄ (state->mode != GZ_READ)
        ᬯᬮᬶ NULL;

    /* check that there was no (serious) error */
    ᬬᬾᬦ᭄ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ᬯᬮᬶ NULL;
    ᬬᬾᬦ᭄ (state->again)
        gz_error(state, Z_OK, NULL);

    /* process a skip request */
    ᬬᬾᬦ᭄ (state->skip && gz_skip(state) == -1)
        ᬯᬮᬶ NULL;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    left = (unsigned)len - 1;
    ᬬᬾᬦ᭄ (left) do {
        /* assure that something is in the output buffer */
        ᬬᬾᬦ᭄ (state->x.have == 0 && gz_fetch(state) == -1)
            bha_nga;                      /* error */
        ᬬᬾᬦ᭄ (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            bha_nga;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > left ? left : state->x.have;
        eol = (unsigned var_na *)memchr(state->x.next, '\n', n);
        ᬬᬾᬦ᭄ (eol != NULL)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        left -= n;
        buf += n;
    } ᬲᭂᬤᭂᬮ (left && eol == NULL);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    ᬬᬾᬦ᭄ (buf == str)
        ᬯᬮᬶ NULL;
    buf[0] = 0;
    ᬯᬮᬶ str;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ᬬᬾᬦ᭄ (file == NULL)
        ᬯᬮᬶ 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    ᬬᬾᬦ᭄ (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (void)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    ᬯᬮᬶ state->direct == 1;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzclose_r(gzFile file) {
    puur_naa_nka ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ᬬᬾᬦ᭄ (file == NULL)
        ᬯᬮᬶ Z_STREAM_ERROR;
    state = (gz_statep)file;
    ᬬᬾᬦ᭄ (state->mode != GZ_READ)
        ᬯᬮᬶ Z_STREAM_ERROR;

    /* free memory and close file */
    ᬬᬾᬦ᭄ (state->size) {
        inflateEnd(&(state->strm));
        free(state->out);
        free(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, NULL);
    free(state->path);
    ret = close(state->fd);
    free(state);
    ᬯᬮᬶ ret ? Z_ERRNO : err;
}
