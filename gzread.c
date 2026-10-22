/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

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
local صحيح gz_load(gz_statep state, unsigned حرف *buf, unsigned len,
                  unsigned *have) {
    صحيح ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    ܥܒܕ {
        get = len - *have;
        ܐܢ (get > max)
            get = max;
        ret = (صحيح)اقرأ(state->fd, buf + *have, get);
        ܐܢ (ret <= 0)
            اكسر;
        *have += (unsigned)ret;
    } ܟܡܐ (*have < len);
    ܐܢ (ret < 0) {
        ܐܢ (BLOCKED(errno)) {
            state->again = 1;
            ܐܢ (*have != 0)
                ܦܢܐ 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        ܦܢܐ -1;
    }
    ܐܢ (ret == 0)
        state->eof = 1;
    ܦܢܐ 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local صحيح gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    ܐܢ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ܦܢܐ -1;
    ܐܢ (state->eof == 0) {
        ܐܢ (strm->avail_in) {       /* copy what's there to the start */
            unsigned حرف *p = state->in;
            unsigned ثابت حرف *q = strm->next_in;

            ܐܢ (q != p) {
                unsigned n = strm->avail_in;

                ܥܒܕ {
                    *p++ = *q++;
                } ܟܡܐ (--n);
            }
        }
        ܐܢ (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            ܦܢܐ -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    ܦܢܐ 0;
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
local صحيح gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    ܐܢ (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned حرف *)malloc(state->want);
        state->out = (unsigned حرف *)malloc(state->want << 1);
        ܐܢ (state->in == عدم || state->out == عدم) {
            حرر(state->out);
            حرر(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ܦܢܐ -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        ܐܢ (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            حرر(state->out);
            حرر(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ܦܢܐ -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    ܐܢ (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        ܐܢ (state->junk == 0 && strm->avail_in == 0) {
            ܐܢ (gz_avail(state) == -1)
                ܦܢܐ -1;
            ܐܢ (strm->avail_in == 0)
                ܦܢܐ 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        ܦܢܐ 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    ܐܢ (gz_avail(state) == -1)
        ܦܢܐ -1;
    ܐܢ (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        ܦܢܐ 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    ܐܢ (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        ܦܢܐ 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    ܦܢܐ 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local صحيح gz_decomp(gz_statep state) {
    صحيح ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    ܥܒܕ {
        /* get more input for inflate() */
        ܐܢ (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            اكسر;
        }
        ܐܢ (strm->avail_in == 0) {
            ܐܢ (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            اكسر;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        ܐܢ (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        ܐܢ (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            اكسر;
        }
        ܐܢ (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            اكسر;
        }
        ܐܢ (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            ܐܢ (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                اكسر;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->msg == عدم ? "compressed data error" : strm->msg);
            اكسر;
        }
    } ܟܡܐ (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    ܐܢ (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        ܦܢܐ 0;
    }

    /* return decompression status */
    ܦܢܐ ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local صحيح gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    ܥܒܕ {
        بدل(state->how) {
        حالة LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            ܐܢ (gz_look(state) == -1)
                ܦܢܐ -1;
            ܐܢ (state->how == LOOK)
                ܦܢܐ 0;
            اكسر;
        حالة COPY:      /* -> COPY */
            ܐܢ (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                ܦܢܐ -1;
            state->x.next = state->out;
            ܦܢܐ 0;
        حالة GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            ܐܢ (gz_decomp(state) == -1)
                ܦܢܐ -1;
            اكسر;
        افتراضي:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            ܦܢܐ -1;
        }
    } ܟܡܐ (state->x.have == 0 && (!state->eof || strm->avail_in));
    ܦܢܐ 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local صحيح gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    ܥܒܕ {
        /* skip over whatever is in output buffer */
        ܐܢ (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ܐܚܪܢܐ ܐܢ (state->eof && state->strm.avail_in == 0) {
            ܐܢ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            اكسر;
        }

        /* need more data to skip -- load up output buffer */
        ܐܚܪܢܐ {
            /* get more output, looking for header if required */
            ܐܢ (gz_fetch(state) == -1)
                ܦܢܐ -1;
        }
    } ܟܡܐ (state->skip);
    ܦܢܐ 0;
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
    صحيح err;

    /* if len is zero, avoid unnecessary operations */
    ܐܢ (len == 0)
        ܦܢܐ 0;

    /* process a skip request */
    ܐܢ (state->skip && gz_skip(state) == -1)
        ܦܢܐ 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    ܥܒܕ {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        ܐܢ (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        ܐܢ (state->x.have) {
            ܐܢ (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            ܐܢ (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ܐܚܪܢܐ ܐܢ (state->eof && state->strm.avail_in == 0) {
            ܐܢ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            اكسر;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        ܐܚܪܢܐ ܐܢ (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            ܐܢ (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            استمر;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        ܐܚܪܢܐ ܐܢ (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned حرف *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        ܐܚܪܢܐ {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.next_out = (unsigned حرف *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (حرف *)buf + n;
        got += n;
        state->x.pos += n;
    } ܟܡܐ (len && !err);

    /* note read past eof */
    ܐܢ (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    ܦܢܐ got;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ܐܢ (file == عدم)
        ܦܢܐ -1;
    state = (gz_statep)file;
    ܐܢ (state->mode != GZ_READ)
        ܦܢܐ -1;

    /* check that there was no (serious) error */
    ܐܢ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ܦܢܐ -1;
    ܐܢ (state->again)
        gz_error(state, Z_OK, عدم);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ܐܢ ((صحيح)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        ܦܢܐ -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    ܐܢ (len == 0) {
        ܐܢ (state->err != Z_OK && state->err != Z_BUF_ERROR)
            ܦܢܐ -1;
        ܐܢ (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            ܦܢܐ -1;
        }
    }

    /* return the number of bytes read */
    ܦܢܐ (صحيح)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ܐܢ (file == عدم)
        ܦܢܐ 0;
    state = (gz_statep)file;
    ܐܢ (state->mode != GZ_READ)
        ܦܢܐ 0;

    /* check that there was no (serious) error */
    ܐܢ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ܦܢܐ 0;
    ܐܢ (state->again)
        gz_error(state, Z_OK, عدم);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ܐܢ (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ܦܢܐ 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    ܦܢܐ len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#ܐܚܪܢܐ
#  undef gzgetc
#endif
صحيح ZEXPORT gzgetc(gzFile file) {
    unsigned حرف buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ܐܢ (file == عدم)
        ܦܢܐ -1;
    state = (gz_statep)file;
    ܐܢ (state->mode != GZ_READ)
        ܦܢܐ -1;

    /* check that there was no (serious) error */
    ܐܢ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ܦܢܐ -1;
    ܐܢ (state->again)
        gz_error(state, Z_OK, عدم);

    /* try output buffer (no need to check for skip request) */
    ܐܢ (state->x.have) {
        state->x.have--;
        state->x.pos++;
        ܦܢܐ *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    ܦܢܐ gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

صحيح ZEXPORT gzgetc_(gzFile file) {
    ܦܢܐ gzgetc(file);
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzungetc(صحيح c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ܐܢ (file == عدم)
        ܦܢܐ -1;
    state = (gz_statep)file;
    ܐܢ (state->mode != GZ_READ)
        ܦܢܐ -1;

    /* in case this was just opened, set up the input buffer */
    ܐܢ (state->how == LOOK && state->x.have == 0)
        (فراغ)gz_look(state);

    /* check that there was no (serious) error */
    ܐܢ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ܦܢܐ -1;
    ܐܢ (state->again)
        gz_error(state, Z_OK, عدم);

    /* process a skip request */
    ܐܢ (state->skip && gz_skip(state) == -1)
        ܦܢܐ -1;

    /* can't push EOF */
    ܐܢ (c < 0)
        ܦܢܐ -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    ܐܢ (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned حرف)c;
        state->x.pos--;
        state->past = 0;
        ܦܢܐ c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    ܐܢ (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        ܦܢܐ -1;
    }

    /* slide output data if needed and insert byte before existing data */
    ܐܢ (state->x.next == state->out) {
        unsigned حرف *src = state->out + state->x.have;
        unsigned حرف *dest = state->out + (state->size << 1);

        ܟܡܐ (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned حرف)c;
    state->x.pos--;
    state->past = 0;
    ܦܢܐ c;
}

/* -- see zlib.h -- */
حرف * ZEXPORT gzgets(gzFile file, حرف *buf, صحيح len) {
    unsigned left, n;
    حرف *str;
    unsigned حرف *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    ܐܢ (file == عدم || buf == عدم || len < 1)
        ܦܢܐ عدم;
    state = (gz_statep)file;
    ܐܢ (state->mode != GZ_READ)
        ܦܢܐ عدم;

    /* check that there was no (serious) error */
    ܐܢ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ܦܢܐ عدم;
    ܐܢ (state->again)
        gz_error(state, Z_OK, عدم);

    /* process a skip request */
    ܐܢ (state->skip && gz_skip(state) == -1)
        ܦܢܐ عدم;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    left = (unsigned)len - 1;
    ܐܢ (left) ܥܒܕ {
        /* assure that something is in the output buffer */
        ܐܢ (state->x.have == 0 && gz_fetch(state) == -1)
            اكسر;                      /* error */
        ܐܢ (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            اكسر;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > left ? left : state->x.have;
        eol = (unsigned حرف *)memchr(state->x.next, '\n', n);
        ܐܢ (eol != عدم)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        left -= n;
        buf += n;
    } ܟܡܐ (left && eol == عدم);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    ܐܢ (buf == str)
        ܦܢܐ عدم;
    buf[0] = 0;
    ܦܢܐ str;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ܐܢ (file == عدم)
        ܦܢܐ 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    ܐܢ (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (فراغ)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    ܦܢܐ state->direct == 1;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzclose_r(gzFile file) {
    صحيح ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ܐܢ (file == عدم)
        ܦܢܐ Z_STREAM_ERROR;
    state = (gz_statep)file;
    ܐܢ (state->mode != GZ_READ)
        ܦܢܐ Z_STREAM_ERROR;

    /* free memory and close file */
    ܐܢ (state->size) {
        inflateEnd(&(state->strm));
        حرر(state->out);
        حرر(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, عدم);
    حرر(state->path);
    ret = close(state->fd);
    حرر(state);
    ܦܢܐ ret ? Z_ERRNO : err;
}
