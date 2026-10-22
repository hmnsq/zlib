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
    افعل {
        get = len - *have;
        إذا (get > max)
            get = max;
        ret = (صحيح)اقرأ(state->fd, buf + *have, get);
        إذا (ret <= 0)
            اكسر;
        *have += (unsigned)ret;
    } طالما (*have < len);
    إذا (ret < 0) {
        إذا (BLOCKED(errno)) {
            state->again = 1;
            إذا (*have != 0)
                أرجع 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        أرجع -1;
    }
    إذا (ret == 0)
        state->eof = 1;
    أرجع 0;
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

    إذا (state->err != Z_OK && state->err != Z_BUF_ERROR)
        أرجع -1;
    إذا (state->eof == 0) {
        إذا (strm->avail_in) {       /* copy what's there to the start */
            unsigned حرف *p = state->in;
            unsigned ثابت حرف *q = strm->next_in;

            إذا (q != p) {
                unsigned n = strm->avail_in;

                افعل {
                    *p++ = *q++;
                } طالما (--n);
            }
        }
        إذا (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            أرجع -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    أرجع 0;
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
    إذا (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned حرف *)malloc(state->want);
        state->out = (unsigned حرف *)malloc(state->want << 1);
        إذا (state->in == عدم || state->out == عدم) {
            حرر(state->out);
            حرر(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            أرجع -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        إذا (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            حرر(state->out);
            حرر(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            أرجع -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    إذا (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        إذا (state->junk == 0 && strm->avail_in == 0) {
            إذا (gz_avail(state) == -1)
                أرجع -1;
            إذا (strm->avail_in == 0)
                أرجع 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        أرجع 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    إذا (gz_avail(state) == -1)
        أرجع -1;
    إذا (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        أرجع 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    إذا (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        أرجع 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    أرجع 0;
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
    افعل {
        /* get more input for inflate() */
        إذا (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            اكسر;
        }
        إذا (strm->avail_in == 0) {
            إذا (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            اكسر;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        إذا (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        إذا (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            اكسر;
        }
        إذا (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            اكسر;
        }
        إذا (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            إذا (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                اكسر;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->رسالة == عدم ? "compressed data error" : strm->رسالة);
            اكسر;
        }
    } طالما (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    إذا (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        أرجع 0;
    }

    /* return decompression status */
    أرجع ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local صحيح gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    افعل {
        بدل(state->how) {
        حالة LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            إذا (gz_look(state) == -1)
                أرجع -1;
            إذا (state->how == LOOK)
                أرجع 0;
            اكسر;
        حالة COPY:      /* -> COPY */
            إذا (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                أرجع -1;
            state->x.next = state->out;
            أرجع 0;
        حالة GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            إذا (gz_decomp(state) == -1)
                أرجع -1;
            اكسر;
        افتراضي:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            أرجع -1;
        }
    } طالما (state->x.have == 0 && (!state->eof || strm->avail_in));
    أرجع 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local صحيح gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    افعل {
        /* skip over whatever is in output buffer */
        إذا (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        وإلا إذا (state->eof && state->strm.avail_in == 0) {
            إذا (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            اكسر;
        }

        /* need more data to skip -- load up output buffer */
        وإلا {
            /* get more output, looking for header if required */
            إذا (gz_fetch(state) == -1)
                أرجع -1;
        }
    } طالما (state->skip);
    أرجع 0;
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
    إذا (len == 0)
        أرجع 0;

    /* process a skip request */
    إذا (state->skip && gz_skip(state) == -1)
        أرجع 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    افعل {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        إذا (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        إذا (state->x.have) {
            إذا (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            إذا (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        وإلا إذا (state->eof && state->strm.avail_in == 0) {
            إذا (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            اكسر;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        وإلا إذا (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            إذا (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            استمر;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        وإلا إذا (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned حرف *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        وإلا {  /* state->how == GZIP */
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
    } طالما (len && !err);

    /* note read past eof */
    إذا (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    أرجع got;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ)
        أرجع -1;

    /* check that there was no (serious) error */
    إذا (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        أرجع -1;
    إذا (state->again)
        gz_error(state, Z_OK, عدم);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    إذا ((صحيح)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        أرجع -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    إذا (len == 0) {
        إذا (state->err != Z_OK && state->err != Z_BUF_ERROR)
            أرجع -1;
        إذا (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            أرجع -1;
        }
    }

    /* return the number of bytes read */
    أرجع (صحيح)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    إذا (file == عدم)
        أرجع 0;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ)
        أرجع 0;

    /* check that there was no (serious) error */
    إذا (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        أرجع 0;
    إذا (state->again)
        gz_error(state, Z_OK, عدم);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    إذا (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        أرجع 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    أرجع len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#وإلا
#  undef gzgetc
#endif
صحيح ZEXPORT gzgetc(gzFile file) {
    unsigned حرف buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ)
        أرجع -1;

    /* check that there was no (serious) error */
    إذا (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        أرجع -1;
    إذا (state->again)
        gz_error(state, Z_OK, عدم);

    /* try output buffer (no need to check for skip request) */
    إذا (state->x.have) {
        state->x.have--;
        state->x.pos++;
        أرجع *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    أرجع gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

صحيح ZEXPORT gzgetc_(gzFile file) {
    أرجع gzgetc(file);
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzungetc(صحيح c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ)
        أرجع -1;

    /* in case this was just opened, set up the input buffer */
    إذا (state->how == LOOK && state->x.have == 0)
        (فراغ)gz_look(state);

    /* check that there was no (serious) error */
    إذا (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        أرجع -1;
    إذا (state->again)
        gz_error(state, Z_OK, عدم);

    /* process a skip request */
    إذا (state->skip && gz_skip(state) == -1)
        أرجع -1;

    /* can't push EOF */
    إذا (c < 0)
        أرجع -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    إذا (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned حرف)c;
        state->x.pos--;
        state->past = 0;
        أرجع c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    إذا (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        أرجع -1;
    }

    /* slide output data if needed and insert byte before existing data */
    إذا (state->x.next == state->out) {
        unsigned حرف *src = state->out + state->x.have;
        unsigned حرف *dest = state->out + (state->size << 1);

        طالما (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned حرف)c;
    state->x.pos--;
    state->past = 0;
    أرجع c;
}

/* -- see zlib.h -- */
حرف * ZEXPORT gzgets(gzFile file, حرف *buf, صحيح len) {
    unsigned يسار, n;
    حرف *str;
    unsigned حرف *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    إذا (file == عدم || buf == عدم || len < 1)
        أرجع عدم;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ)
        أرجع عدم;

    /* check that there was no (serious) error */
    إذا (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        أرجع عدم;
    إذا (state->again)
        gz_error(state, Z_OK, عدم);

    /* process a skip request */
    إذا (state->skip && gz_skip(state) == -1)
        أرجع عدم;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    يسار = (unsigned)len - 1;
    إذا (يسار) افعل {
        /* assure that something is in the output buffer */
        إذا (state->x.have == 0 && gz_fetch(state) == -1)
            اكسر;                      /* error */
        إذا (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            اكسر;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > يسار ? يسار : state->x.have;
        eol = (unsigned حرف *)memchr(state->x.next, '\n', n);
        إذا (eol != عدم)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        يسار -= n;
        buf += n;
    } طالما (يسار && eol == عدم);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    إذا (buf == str)
        أرجع عدم;
    buf[0] = 0;
    أرجع str;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    إذا (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (فراغ)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    أرجع state->direct == 1;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzclose_r(gzFile file) {
    صحيح ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    إذا (file == عدم)
        أرجع Z_STREAM_ERROR;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ)
        أرجع Z_STREAM_ERROR;

    /* free memory and close file */
    إذا (state->size) {
        inflateEnd(&(state->strm));
        حرر(state->out);
        حرر(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, عدم);
    حرر(state->path);
    ret = close(state->fd);
    حرر(state);
    أرجع ret ? Z_ERRNO : err;
}
