/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "gzguts.h"

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
local ਪੂਰਨ gz_ਲੱਦੋ(gz_statep state, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len,
                  ਬਿਨਾਨਿਸ਼ਾਨੀ *have) {
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ get, max = ((ਬਿਨਾਨਿਸ਼ਾਨੀ)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    ਕਰੋ {
        get = len - *have;
        ਜੇ (get > max)
            get = max;
        ret = (ਪੂਰਨ)ਪਠ(state->fd, buf + *have, get);
        ਜੇ (ret <= 0)
            ਰੋਕੋ;
        *have += (ਬਿਨਾਨਿਸ਼ਾਨੀ)ret;
    } ਜਦੋਂ (*have < len);
    ਜੇ (ret < 0) {
        ਜੇ (BLOCKED(errno)) {
            state->again = 1;
            ਜੇ (*have != 0)
                ਵਾਪਸ 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        ਵਾਪਸ -1;
    }
    ਜੇ (ret == 0)
        state->eof = 1;
    ਵਾਪਸ 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local ਪੂਰਨ gz_avail(gz_statep state) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ got;
    z_streamp strm = &(state->strm);

    ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ਵਾਪਸ -1;
    ਜੇ (state->eof == 0) {
        ਜੇ (strm->avail_ਵਿੱਚ) {       /* copy what's there to the start */
            ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p = state->ਵਿੱਚ;
            ਬਿਨਾਨਿਸ਼ਾਨੀ ਅਚਲ ਅੱਖਰ *q = strm->ਅਗਲਾ_ਵਿੱਚ;

            ਜੇ (q != p) {
                ਬਿਨਾਨਿਸ਼ਾਨੀ n = strm->avail_ਵਿੱਚ;

                ਕਰੋ {
                    *p++ = *q++;
                } ਜਦੋਂ (--n);
            }
        }
        ਜੇ (gz_ਲੱਦੋ(state, state->ਵਿੱਚ + strm->avail_ਵਿੱਚ,
                    state->ਮਾਪ - strm->avail_ਵਿੱਚ, &got) == -1)
            ਵਾਪਸ -1;
        strm->avail_ਵਿੱਚ += got;
        strm->ਅਗਲਾ_ਵਿੱਚ = state->ਵਿੱਚ;
    }
    ਵਾਪਸ 0;
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
local ਪੂਰਨ gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    ਜੇ (state->ਮਾਪ == 0) {
        /* allocate buffers */
        state->ਵਿੱਚ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(state->want);
        state->ਬਾਹਰਲਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(state->want << 1);
        ਜੇ (state->ਵਿੱਚ == ਨਿਰਾ || state->ਬਾਹਰਲਾ == ਨਿਰਾ) {
            ਮੁਕਤ(state->ਬਾਹਰਲਾ);
            ਮੁਕਤ(state->ਵਿੱਚ);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ਵਾਪਸ -1;
        }
        state->ਮਾਪ = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_ਵਿੱਚ = 0;
        state->strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
        ਜੇ (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            ਮੁਕਤ(state->ਬਾਹਰਲਾ);
            ਮੁਕਤ(state->ਵਿੱਚ);
            state->ਮਾਪ = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ਵਾਪਸ -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    ਜੇ (state->ਸਿੱਧਾ == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        ਜੇ (state->junk == 0 && strm->avail_ਵਿੱਚ == 0) {
            ਜੇ (gz_avail(state) == -1)
                ਵਾਪਸ -1;
            ਜੇ (strm->avail_ਵਿੱਚ == 0)
                ਵਾਪਸ 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->ਸਿੱਧਾ = 0;
        ਵਾਪਸ 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    ਜੇ (gz_avail(state) == -1)
        ਵਾਪਸ -1;
    ਜੇ (strm->avail_ਵਿੱਚ == 0 || (state->again && strm->avail_ਵਿੱਚ < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        ਵਾਪਸ 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    ਜੇ (strm->avail_ਵਿੱਚ > 3 &&
            strm->ਅਗਲਾ_ਵਿੱਚ[0] == 31 && strm->ਅਗਲਾ_ਵਿੱਚ[1] == 139 &&
            strm->ਅਗਲਾ_ਵਿੱਚ[2] == 8 && strm->ਅਗਲਾ_ਵਿੱਚ[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->ਸਿੱਧਾ = 0;
        ਵਾਪਸ 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.ਅਗਲਾ = state->ਬਾਹਰਲਾ;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(state->x.ਅਗਲਾ, strm->ਅਗਲਾ_ਵਿੱਚ, strm->avail_ਵਿੱਚ);
    state->x.have = strm->avail_ਵਿੱਚ;
    strm->avail_ਵਿੱਚ = 0;
    state->how = COPY;
    ਵਾਪਸ 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local ਪੂਰਨ gz_decomp(gz_statep state) {
    ਪੂਰਨ ret = Z_OK;
    ਬਿਨਾਨਿਸ਼ਾਨੀ had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_ਬਾਹਰਲਾ;
    ਕਰੋ {
        /* get more input for inflate() */
        ਜੇ (strm->avail_ਵਿੱਚ == 0 && gz_avail(state) == -1) {
            ret = state->err;
            ਰੋਕੋ;
        }
        ਜੇ (strm->avail_ਵਿੱਚ == 0) {
            ਜੇ (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ਰੋਕੋ;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        ਜੇ (strm->avail_ਬਾਹਰਲਾ < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        ਜੇ (ret == Z_ਧਾਰਾ_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_ਧਾਰਾ_ERROR,
                     "internal error: inflate stream corrupt");
            ਰੋਕੋ;
        }
        ਜੇ (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ਰੋਕੋ;
        }
        ਜੇ (ret == Z_ਅੰਕੜੇ_ERROR) {              /* deflate stream invalid */
            ਜੇ (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_ਵਿੱਚ = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                ਰੋਕੋ;
            }
            gz_error(state, Z_ਅੰਕੜੇ_ERROR,
                     strm->ਸੁਨੇਹਾ == ਨਿਰਾ ? "compressed data error" : strm->ਸੁਨੇਹਾ);
            ਰੋਕੋ;
        }
    } ਜਦੋਂ (strm->avail_ਬਾਹਰਲਾ && ret != Z_ਧਾਰਾ_ਸਿਰਾ);

    /* update available output */
    state->x.have = had - strm->avail_ਬਾਹਰਲਾ;
    state->x.ਅਗਲਾ = strm->ਅਗਲਾ_ਬਾਹਰਲਾ - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    ਜੇ (ret == Z_ਧਾਰਾ_ਸਿਰਾ) {
        state->junk = 0;
        state->how = LOOK;
        ਵਾਪਸ 0;
    }

    /* return decompression status */
    ਵਾਪਸ ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local ਪੂਰਨ gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    ਕਰੋ {
        ਚੋਣ(state->how) {
        ਹਾਲਤ LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            ਜੇ (gz_look(state) == -1)
                ਵਾਪਸ -1;
            ਜੇ (state->how == LOOK)
                ਵਾਪਸ 0;
            ਰੋਕੋ;
        ਹਾਲਤ COPY:      /* -> COPY */
            ਜੇ (gz_ਲੱਦੋ(state, state->ਬਾਹਰਲਾ, state->ਮਾਪ << 1, &(state->x.have))
                    == -1)
                ਵਾਪਸ -1;
            state->x.ਅਗਲਾ = state->ਬਾਹਰਲਾ;
            ਵਾਪਸ 0;
        ਹਾਲਤ GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_ਬਾਹਰਲਾ = state->ਮਾਪ << 1;
            strm->ਅਗਲਾ_ਬਾਹਰਲਾ = state->ਬਾਹਰਲਾ;
            ਜੇ (gz_decomp(state) == -1)
                ਵਾਪਸ -1;
            ਰੋਕੋ;
        ਮੂਲ:
            gz_error(state, Z_ਧਾਰਾ_ERROR, "state corrupt");
            ਵਾਪਸ -1;
        }
    } ਜਦੋਂ (state->x.have == 0 && (!state->eof || strm->avail_ਵਿੱਚ));
    ਵਾਪਸ 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local ਪੂਰਨ gz_skip(gz_statep state) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    ਕਰੋ {
        /* skip over whatever is in output buffer */
        ਜੇ (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (ਬਿਨਾਨਿਸ਼ਾਨੀ)state->skip : state->x.have;
            state->x.have -= n;
            state->x.ਅਗਲਾ += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ਨਹੀਂਤਾਂ ਜੇ (state->eof && state->strm.avail_ਵਿੱਚ == 0) {
            ਜੇ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ਰੋਕੋ;
        }

        /* need more data to skip -- load up output buffer */
        ਨਹੀਂਤਾਂ {
            /* get more output, looking for header if required */
            ਜੇ (gz_fetch(state) == -1)
                ਵਾਪਸ -1;
        }
    } ਜਦੋਂ (state->skip);
    ਵਾਪਸ 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_ਮਾਪ_t gz_read(gz_statep state, voidp buf, z_ਮਾਪ_t len) {
    z_ਮਾਪ_t got;
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;
    ਪੂਰਨ err;

    /* if len is zero, avoid unnecessary operations */
    ਜੇ (len == 0)
        ਵਾਪਸ 0;

    /* process a skip request */
    ਜੇ (state->skip && gz_skip(state) == -1)
        ਵਾਪਸ 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    ਕਰੋ {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (ਬਿਨਾਨਿਸ਼ਾਨੀ)-1;
        ਜੇ (n > len)
            n = (ਬਿਨਾਨਿਸ਼ਾਨੀ)len;

        /* first just try copying data from the output buffer */
        ਜੇ (state->x.have) {
            ਜੇ (state->x.have < n)
                n = state->x.have;
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, state->x.ਅਗਲਾ, n);
            state->x.ਅਗਲਾ += n;
            state->x.have -= n;
            ਜੇ (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ਨਹੀਂਤਾਂ ਜੇ (state->eof && state->strm.avail_ਵਿੱਚ == 0) {
            ਜੇ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ਰੋਕੋ;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        ਨਹੀਂਤਾਂ ਜੇ (state->how == LOOK || n < (state->ਮਾਪ << 1)) {
            /* get more output, looking for header if required */
            ਜੇ (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            ਜਾਰੀ;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        ਨਹੀਂਤਾਂ ਜੇ (state->how == COPY)        /* read directly */
            err = gz_ਲੱਦੋ(state, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        ਨਹੀਂਤਾਂ {  /* state->how == GZIP */
            state->strm.avail_ਬਾਹਰਲਾ = n;
            state->strm.ਅਗਲਾ_ਬਾਹਰਲਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (ਅੱਖਰ *)buf + n;
        got += n;
        state->x.pos += n;
    } ਜਦੋਂ (len && !err);

    /* note read past eof */
    ਜੇ (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    ਵਾਪਸ got;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzread(gzFile file, voidp buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ)
        ਵਾਪਸ -1;

    /* check that there was no (serious) error */
    ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ਵਾਪਸ -1;
    ਜੇ (state->again)
        gz_error(state, Z_OK, ਨਿਰਾ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ਜੇ ((ਪੂਰਨ)len < 0) {
        gz_error(state, Z_ਧਾਰਾ_ERROR, "request does not fit in an int");
        ਵਾਪਸ -1;
    }

    /* read len or fewer bytes to buf */
    len = (ਬਿਨਾਨਿਸ਼ਾਨੀ)gz_read(state, buf, len);

    /* check for an error */
    ਜੇ (len == 0) {
        ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR)
            ਵਾਪਸ -1;
        ਜੇ (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            ਵਾਪਸ -1;
        }
    }

    /* return the number of bytes read */
    ਵਾਪਸ (ਪੂਰਨ)len;
}

/* -- see zlib.h -- */
z_ਮਾਪ_t ZEXPORT gzfread(voidp buf, z_ਮਾਪ_t ਮਾਪ, z_ਮਾਪ_t nitems,
                         gzFile file) {
    z_ਮਾਪ_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ 0;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ)
        ਵਾਪਸ 0;

    /* check that there was no (serious) error */
    ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ਵਾਪਸ 0;
    ਜੇ (state->again)
        gz_error(state, Z_OK, ਨਿਰਾ);

    /* compute bytes to read -- error on overflow */
    len = nitems * ਮਾਪ;
    ਜੇ (ਮਾਪ && len / ਮਾਪ != nitems) {
        gz_error(state, Z_ਧਾਰਾ_ERROR, "request does not fit in a size_t");
        ਵਾਪਸ 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    ਵਾਪਸ len ? gz_read(state, buf, len) / ਮਾਪ : 0;
}

/* -- see zlib.h -- */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_ਅਗੇਤਰ_ਸੈੱਟ
#  undef z_gzgetc
#ਨਹੀਂਤਾਂ
#  undef gzgetc
#ਜੇ_ਅੰਤ
ਪੂਰਨ ZEXPORT gzgetc(gzFile file) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ)
        ਵਾਪਸ -1;

    /* check that there was no (serious) error */
    ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ਵਾਪਸ -1;
    ਜੇ (state->again)
        gz_error(state, Z_OK, ਨਿਰਾ);

    /* try output buffer (no need to check for skip request) */
    ਜੇ (state->x.have) {
        state->x.have--;
        state->x.pos++;
        ਵਾਪਸ *(state->x.ਅਗਲਾ)++;
    }

    /* nothing there -- try gz_read() */
    ਵਾਪਸ gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

ਪੂਰਨ ZEXPORT gzgetc_(gzFile file) {
    ਵਾਪਸ gzgetc(file);
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzungetc(ਪੂਰਨ c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ)
        ਵਾਪਸ -1;

    /* in case this was just opened, set up the input buffer */
    ਜੇ (state->how == LOOK && state->x.have == 0)
        (ਖਾਲੀ)gz_look(state);

    /* check that there was no (serious) error */
    ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ਵਾਪਸ -1;
    ਜੇ (state->again)
        gz_error(state, Z_OK, ਨਿਰਾ);

    /* process a skip request */
    ਜੇ (state->skip && gz_skip(state) == -1)
        ਵਾਪਸ -1;

    /* can't push EOF */
    ਜੇ (c < 0)
        ਵਾਪਸ -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    ਜੇ (state->x.have == 0) {
        state->x.have = 1;
        state->x.ਅਗਲਾ = state->ਬਾਹਰਲਾ + (state->ਮਾਪ << 1) - 1;
        state->x.ਅਗਲਾ[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)c;
        state->x.pos--;
        state->past = 0;
        ਵਾਪਸ c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    ਜੇ (state->x.have == (state->ਮਾਪ << 1)) {
        gz_error(state, Z_ਅੰਕੜੇ_ERROR, "out of room to push characters");
        ਵਾਪਸ -1;
    }

    /* slide output data if needed and insert byte before existing data */
    ਜੇ (state->x.ਅਗਲਾ == state->ਬਾਹਰਲਾ) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *src = state->ਬਾਹਰਲਾ + state->x.have;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *dest = state->ਬਾਹਰਲਾ + (state->ਮਾਪ << 1);

        ਜਦੋਂ (src > state->ਬਾਹਰਲਾ)
            *--dest = *--src;
        state->x.ਅਗਲਾ = dest;
    }
    state->x.have++;
    state->x.ਅਗਲਾ--;
    state->x.ਅਗਲਾ[0] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)c;
    state->x.pos--;
    state->past = 0;
    ਵਾਪਸ c;
}

/* -- see zlib.h -- */
ਅੱਖਰ * ZEXPORT gzgets(gzFile file, ਅੱਖਰ *buf, ਪੂਰਨ len) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ, n;
    ਅੱਖਰ *str;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    ਜੇ (file == ਨਿਰਾ || buf == ਨਿਰਾ || len < 1)
        ਵਾਪਸ ਨਿਰਾ;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ)
        ਵਾਪਸ ਨਿਰਾ;

    /* check that there was no (serious) error */
    ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ਵਾਪਸ ਨਿਰਾ;
    ਜੇ (state->again)
        gz_error(state, Z_OK, ਨਿਰਾ);

    /* process a skip request */
    ਜੇ (state->skip && gz_skip(state) == -1)
        ਵਾਪਸ ਨਿਰਾ;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    ਖੱਬਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)len - 1;
    ਜੇ (ਖੱਬਾ) ਕਰੋ {
        /* assure that something is in the output buffer */
        ਜੇ (state->x.have == 0 && gz_fetch(state) == -1)
            ਰੋਕੋ;                      /* error */
        ਜੇ (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            ਰੋਕੋ;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > ਖੱਬਾ ? ਖੱਬਾ : state->x.have;
        eol = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)memchr(state->x.ਅਗਲਾ, '\n', n);
        ਜੇ (eol != ਨਿਰਾ)
            n = (ਬਿਨਾਨਿਸ਼ਾਨੀ)(eol - state->x.ਅਗਲਾ) + 1;

        /* copy through end-of-line, or remainder if not found */
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, state->x.ਅਗਲਾ, n);
        state->x.have -= n;
        state->x.ਅਗਲਾ += n;
        state->x.pos += n;
        ਖੱਬਾ -= n;
        buf += n;
    } ਜਦੋਂ (ਖੱਬਾ && eol == ਨਿਰਾ);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    ਜੇ (buf == str)
        ਵਾਪਸ ਨਿਰਾ;
    buf[0] = 0;
    ਵਾਪਸ str;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ && state->how == LOOK && state->x.have == 0)
        (ਖਾਲੀ)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    ਵਾਪਸ state->ਸਿੱਧਾ == 1;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzclose_r(gzFile file) {
    ਪੂਰਨ ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    /* free memory and close file */
    ਜੇ (state->ਮਾਪ) {
        inflate_ਸਿਰਾ(&(state->strm));
        ਮੁਕਤ(state->ਬਾਹਰਲਾ);
        ਮੁਕਤ(state->ਵਿੱਚ);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, ਨਿਰਾ);
    ਮੁਕਤ(state->ਰਸਤਾ);
    ret = close(state->fd);
    ਮੁਕਤ(state);
    ਵਾਪਸ ret ? Z_ERRNO : err;
}
