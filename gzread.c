/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "gzguts.h"

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
local ಪೂರ್ಣಾಂಕ gz_ತುಂಬು(gz_statep state, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ len,
                  ಚಿಹ್ನೆರಹಿತ *have) {
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ get, max = ((ಚಿಹ್ನೆರಹಿತ)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    ಮಾಡು {
        get = len - *have;
        ಆದರೆ (get > max)
            get = max;
        ret = (ಪೂರ್ಣಾಂಕ)ಪಠ(state->fd, buf + *have, get);
        ಆದರೆ (ret <= 0)
            ನಿಲ್ಲಿಸು;
        *have += (ಚಿಹ್ನೆರಹಿತ)ret;
    } ಆಗ (*have < len);
    ಆದರೆ (ret < 0) {
        ಆದರೆ (BLOCKED(errno)) {
            state->again = 1;
            ಆದರೆ (*have != 0)
                ಹಿಂತಿರುಗಿ 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        ಹಿಂತಿರುಗಿ -1;
    }
    ಆದರೆ (ret == 0)
        state->eof = 1;
    ಹಿಂತಿರುಗಿ 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local ಪೂರ್ಣಾಂಕ gz_avail(gz_statep state) {
    ಚಿಹ್ನೆರಹಿತ got;
    z_streamp strm = &(state->strm);

    ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (state->eof == 0) {
        ಆದರೆ (strm->avail_ಒಳ) {       /* copy what's there to the start */
            ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p = state->ಒಳ;
            ಚಿಹ್ನೆರಹಿತ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *q = strm->ಮುಂದಿನ_ಒಳ;

            ಆದರೆ (q != p) {
                ಚಿಹ್ನೆರಹಿತ n = strm->avail_ಒಳ;

                ಮಾಡು {
                    *p++ = *q++;
                } ಆಗ (--n);
            }
        }
        ಆದರೆ (gz_ತುಂಬು(state, state->ಒಳ + strm->avail_ಒಳ,
                    state->ಗಾತ್ರ_2 - strm->avail_ಒಳ, &got) == -1)
            ಹಿಂತಿರುಗಿ -1;
        strm->avail_ಒಳ += got;
        strm->ಮುಂದಿನ_ಒಳ = state->ಒಳ;
    }
    ಹಿಂತಿರುಗಿ 0;
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
local ಪೂರ್ಣಾಂಕ gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    ಆದರೆ (state->ಗಾತ್ರ_2 == 0) {
        /* allocate buffers */
        state->ಒಳ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ಕೊಡು(state->want);
        state->ಹೊರಹೋಗು = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ಕೊಡು(state->want << 1);
        ಆದರೆ (state->ಒಳ == ಖಾಲಿ || state->ಹೊರಹೋಗು == ಖಾಲಿ) {
            ಬಿಡುಗಡೆ(state->ಹೊರಹೋಗು);
            ಬಿಡುಗಡೆ(state->ಒಳ);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ಹಿಂತಿರುಗಿ -1;
        }
        state->ಗಾತ್ರ_2 = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_ಒಳ = 0;
        state->strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
        ಆದರೆ (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            ಬಿಡುಗಡೆ(state->ಹೊರಹೋಗು);
            ಬಿಡುಗಡೆ(state->ಒಳ);
            state->ಗಾತ್ರ_2 = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ಹಿಂತಿರುಗಿ -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    ಆದರೆ (state->ನೇರ == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        ಆದರೆ (state->junk == 0 && strm->avail_ಒಳ == 0) {
            ಆದರೆ (gz_avail(state) == -1)
                ಹಿಂತಿರುಗಿ -1;
            ಆದರೆ (strm->avail_ಒಳ == 0)
                ಹಿಂತಿರುಗಿ 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->ನೇರ = 0;
        ಹಿಂತಿರುಗಿ 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    ಆದರೆ (gz_avail(state) == -1)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (strm->avail_ಒಳ == 0 || (state->again && strm->avail_ಒಳ < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        ಹಿಂತಿರುಗಿ 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    ಆದರೆ (strm->avail_ಒಳ > 3 &&
            strm->ಮುಂದಿನ_ಒಳ[0] == 31 && strm->ಮುಂದಿನ_ಒಳ[1] == 139 &&
            strm->ಮುಂದಿನ_ಒಳ[2] == 8 && strm->ಮುಂದಿನ_ಒಳ[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->ನೇರ = 0;
        ಹಿಂತಿರುಗಿ 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.ಮುಂದಿನ = state->ಹೊರಹೋಗು;
    ಸ್ಮರಣೆ_ನಕಲು(state->x.ಮುಂದಿನ, strm->ಮುಂದಿನ_ಒಳ, strm->avail_ಒಳ);
    state->x.have = strm->avail_ಒಳ;
    strm->avail_ಒಳ = 0;
    state->how = COPY;
    ಹಿಂತಿರುಗಿ 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local ಪೂರ್ಣಾಂಕ gz_decomp(gz_statep state) {
    ಪೂರ್ಣಾಂಕ ret = Z_OK;
    ಚಿಹ್ನೆರಹಿತ had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_ಹೊರಹೋಗು;
    ಮಾಡು {
        /* get more input for inflate() */
        ಆದರೆ (strm->avail_ಒಳ == 0 && gz_avail(state) == -1) {
            ret = state->err;
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (strm->avail_ಒಳ == 0) {
            ಆದರೆ (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ನಿಲ್ಲಿಸು;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        ಆದರೆ (strm->avail_ಹೊರಹೋಗು < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        ಆದರೆ (ret == Z_ಹರಿವು_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_ಹರಿವು_ERROR,
                     "internal error: inflate stream corrupt");
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (ret == Z_ದತ್ತಾಂಶ_ERROR) {              /* deflate stream invalid */
            ಆದರೆ (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_ಒಳ = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                ನಿಲ್ಲಿಸು;
            }
            gz_error(state, Z_ದತ್ತಾಂಶ_ERROR,
                     strm->ಸಂದೇಶ == ಖಾಲಿ ? "compressed data error" : strm->ಸಂದೇಶ);
            ನಿಲ್ಲಿಸು;
        }
    } ಆಗ (strm->avail_ಹೊರಹೋಗು && ret != Z_ಹರಿವು_ಕೊನೆ);

    /* update available output */
    state->x.have = had - strm->avail_ಹೊರಹೋಗು;
    state->x.ಮುಂದಿನ = strm->ಮುಂದಿನ_ಹೊರಹೋಗು - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    ಆದರೆ (ret == Z_ಹರಿವು_ಕೊನೆ) {
        state->junk = 0;
        state->how = LOOK;
        ಹಿಂತಿರುಗಿ 0;
    }

    /* return decompression status */
    ಹಿಂತಿರುಗಿ ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local ಪೂರ್ಣಾಂಕ gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    ಮಾಡು {
        ಆಯ್ಕೆ(state->how) {
        ಸ್ಥಿತಿ LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            ಆದರೆ (gz_look(state) == -1)
                ಹಿಂತಿರುಗಿ -1;
            ಆದರೆ (state->how == LOOK)
                ಹಿಂತಿರುಗಿ 0;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ COPY:      /* -> COPY */
            ಆದರೆ (gz_ತುಂಬು(state, state->ಹೊರಹೋಗು, state->ಗಾತ್ರ_2 << 1, &(state->x.have))
                    == -1)
                ಹಿಂತಿರುಗಿ -1;
            state->x.ಮುಂದಿನ = state->ಹೊರಹೋಗು;
            ಹಿಂತಿರುಗಿ 0;
        ಸ್ಥಿತಿ GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_ಹೊರಹೋಗು = state->ಗಾತ್ರ_2 << 1;
            strm->ಮುಂದಿನ_ಹೊರಹೋಗು = state->ಹೊರಹೋಗು;
            ಆದರೆ (gz_decomp(state) == -1)
                ಹಿಂತಿರುಗಿ -1;
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            gz_error(state, Z_ಹರಿವು_ERROR, "state corrupt");
            ಹಿಂತಿರುಗಿ -1;
        }
    } ಆಗ (state->x.have == 0 && (!state->eof || strm->avail_ಒಳ));
    ಹಿಂತಿರುಗಿ 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local ಪೂರ್ಣಾಂಕ gz_skip(gz_statep state) {
    ಚಿಹ್ನೆರಹಿತ n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    ಮಾಡು {
        /* skip over whatever is in output buffer */
        ಆದರೆ (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (ಚಿಹ್ನೆರಹಿತ)state->skip : state->x.have;
            state->x.have -= n;
            state->x.ಮುಂದಿನ += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->eof && state->strm.avail_ಒಳ == 0) {
            ಆದರೆ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ನಿಲ್ಲಿಸು;
        }

        /* need more data to skip -- load up output buffer */
        ಇಲ್ಲದಿದ್ದರೆ {
            /* get more output, looking for header if required */
            ಆದರೆ (gz_fetch(state) == -1)
                ಹಿಂತಿರುಗಿ -1;
        }
    } ಆಗ (state->skip);
    ಹಿಂತಿರುಗಿ 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_ಗಾತ್ರ_t gz_read(gz_statep state, voidp buf, z_ಗಾತ್ರ_t len) {
    z_ಗಾತ್ರ_t got;
    ಚಿಹ್ನೆರಹಿತ n;
    ಪೂರ್ಣಾಂಕ err;

    /* if len is zero, avoid unnecessary operations */
    ಆದರೆ (len == 0)
        ಹಿಂತಿರುಗಿ 0;

    /* process a skip request */
    ಆದರೆ (state->skip && gz_skip(state) == -1)
        ಹಿಂತಿರುಗಿ 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    ಮಾಡು {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (ಚಿಹ್ನೆರಹಿತ)-1;
        ಆದರೆ (n > len)
            n = (ಚಿಹ್ನೆರಹಿತ)len;

        /* first just try copying data from the output buffer */
        ಆದರೆ (state->x.have) {
            ಆದರೆ (state->x.have < n)
                n = state->x.have;
            ಸ್ಮರಣೆ_ನಕಲು(buf, state->x.ಮುಂದಿನ, n);
            state->x.ಮುಂದಿನ += n;
            state->x.have -= n;
            ಆದರೆ (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->eof && state->strm.avail_ಒಳ == 0) {
            ಆದರೆ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ನಿಲ್ಲಿಸು;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->how == LOOK || n < (state->ಗಾತ್ರ_2 << 1)) {
            /* get more output, looking for header if required */
            ಆದರೆ (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            ಮುಂದುವರಿ;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->how == COPY)        /* read directly */
            err = gz_ತುಂಬು(state, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        ಇಲ್ಲದಿದ್ದರೆ {  /* state->how == GZIP */
            state->strm.avail_ಹೊರಹೋಗು = n;
            state->strm.ಮುಂದಿನ_ಹೊರಹೋಗು = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (ಅಕ್ಷರ *)buf + n;
        got += n;
        state->x.pos += n;
    } ಆಗ (len && !err);

    /* note read past eof */
    ಆದರೆ (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    ಹಿಂತಿರುಗಿ got;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzread(gzFile file, voidp buf, ಚಿಹ್ನೆರಹಿತ len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ)
        ಹಿಂತಿರುಗಿ -1;

    /* check that there was no (serious) error */
    ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (state->again)
        gz_error(state, Z_OK, ಖಾಲಿ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ಆದರೆ ((ಪೂರ್ಣಾಂಕ)len < 0) {
        gz_error(state, Z_ಹರಿವು_ERROR, "request does not fit in an int");
        ಹಿಂತಿರುಗಿ -1;
    }

    /* read len or fewer bytes to buf */
    len = (ಚಿಹ್ನೆರಹಿತ)gz_read(state, buf, len);

    /* check for an error */
    ಆದರೆ (len == 0) {
        ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR)
            ಹಿಂತಿರುಗಿ -1;
        ಆದರೆ (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            ಹಿಂತಿರುಗಿ -1;
        }
    }

    /* return the number of bytes read */
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)len;
}

/* -- see zlib.h -- */
z_ಗಾತ್ರ_t ZEXPORT gzfread(voidp buf, z_ಗಾತ್ರ_t ಗಾತ್ರ_2, z_ಗಾತ್ರ_t nitems,
                         gzFile file) {
    z_ಗಾತ್ರ_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ)
        ಹಿಂತಿರುಗಿ 0;

    /* check that there was no (serious) error */
    ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ (state->again)
        gz_error(state, Z_OK, ಖಾಲಿ);

    /* compute bytes to read -- error on overflow */
    len = nitems * ಗಾತ್ರ_2;
    ಆದರೆ (ಗಾತ್ರ_2 && len / ಗಾತ್ರ_2 != nitems) {
        gz_error(state, Z_ಹರಿವು_ERROR, "request does not fit in a size_t");
        ಹಿಂತಿರುಗಿ 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    ಹಿಂತಿರುಗಿ len ? gz_read(state, buf, len) / ಗಾತ್ರ_2 : 0;
}

/* -- see zlib.h -- */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಣ
#  undef z_gzgetc
#ಇಲ್ಲದಿದ್ದರೆ
#  undef gzgetc
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ ZEXPORT gzgetc(gzFile file) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ)
        ಹಿಂತಿರುಗಿ -1;

    /* check that there was no (serious) error */
    ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (state->again)
        gz_error(state, Z_OK, ಖಾಲಿ);

    /* try output buffer (no need to check for skip request) */
    ಆದರೆ (state->x.have) {
        state->x.have--;
        state->x.pos++;
        ಹಿಂತಿರುಗಿ *(state->x.ಮುಂದಿನ)++;
    }

    /* nothing there -- try gz_read() */
    ಹಿಂತಿರುಗಿ gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

ಪೂರ್ಣಾಂಕ ZEXPORT gzgetc_(gzFile file) {
    ಹಿಂತಿರುಗಿ gzgetc(file);
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzungetc(ಪೂರ್ಣಾಂಕ c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ)
        ಹಿಂತಿರುಗಿ -1;

    /* in case this was just opened, set up the input buffer */
    ಆದರೆ (state->how == LOOK && state->x.have == 0)
        (ಶೂನ್ಯ)gz_look(state);

    /* check that there was no (serious) error */
    ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (state->again)
        gz_error(state, Z_OK, ಖಾಲಿ);

    /* process a skip request */
    ಆದರೆ (state->skip && gz_skip(state) == -1)
        ಹಿಂತಿರುಗಿ -1;

    /* can't push EOF */
    ಆದರೆ (c < 0)
        ಹಿಂತಿರುಗಿ -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    ಆದರೆ (state->x.have == 0) {
        state->x.have = 1;
        state->x.ಮುಂದಿನ = state->ಹೊರಹೋಗು + (state->ಗಾತ್ರ_2 << 1) - 1;
        state->x.ಮುಂದಿನ[0] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)c;
        state->x.pos--;
        state->past = 0;
        ಹಿಂತಿರುಗಿ c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    ಆದರೆ (state->x.have == (state->ಗಾತ್ರ_2 << 1)) {
        gz_error(state, Z_ದತ್ತಾಂಶ_ERROR, "out of room to push characters");
        ಹಿಂತಿರುಗಿ -1;
    }

    /* slide output data if needed and insert byte before existing data */
    ಆದರೆ (state->x.ಮುಂದಿನ == state->ಹೊರಹೋಗು) {
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *src = state->ಹೊರಹೋಗು + state->x.have;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *dest = state->ಹೊರಹೋಗು + (state->ಗಾತ್ರ_2 << 1);

        ಆಗ (src > state->ಹೊರಹೋಗು)
            *--dest = *--src;
        state->x.ಮುಂದಿನ = dest;
    }
    state->x.have++;
    state->x.ಮುಂದಿನ--;
    state->x.ಮುಂದಿನ[0] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)c;
    state->x.pos--;
    state->past = 0;
    ಹಿಂತಿರುಗಿ c;
}

/* -- see zlib.h -- */
ಅಕ್ಷರ * ZEXPORT gzgets(gzFile file, ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ len) {
    ಚಿಹ್ನೆರಹಿತ ಎಡ, n;
    ಅಕ್ಷರ *str;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    ಆದರೆ (file == ಖಾಲಿ || buf == ಖಾಲಿ || len < 1)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    /* check that there was no (serious) error */
    ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಆದರೆ (state->again)
        gz_error(state, Z_OK, ಖಾಲಿ);

    /* process a skip request */
    ಆದರೆ (state->skip && gz_skip(state) == -1)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    ಎಡ = (ಚಿಹ್ನೆರಹಿತ)len - 1;
    ಆದರೆ (ಎಡ) ಮಾಡು {
        /* assure that something is in the output buffer */
        ಆದರೆ (state->x.have == 0 && gz_fetch(state) == -1)
            ನಿಲ್ಲಿಸು;                      /* error */
        ಆದರೆ (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            ನಿಲ್ಲಿಸು;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > ಎಡ ? ಎಡ : state->x.have;
        eol = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)memchr(state->x.ಮುಂದಿನ, '\n', n);
        ಆದರೆ (eol != ಖಾಲಿ)
            n = (ಚಿಹ್ನೆರಹಿತ)(eol - state->x.ಮುಂದಿನ) + 1;

        /* copy through end-of-line, or remainder if not found */
        ಸ್ಮರಣೆ_ನಕಲು(buf, state->x.ಮುಂದಿನ, n);
        state->x.have -= n;
        state->x.ಮುಂದಿನ += n;
        state->x.pos += n;
        ಎಡ -= n;
        buf += n;
    } ಆಗ (ಎಡ && eol == ಖಾಲಿ);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    ಆದರೆ (buf == str)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    buf[0] = 0;
    ಹಿಂತಿರುಗಿ str;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ && state->how == LOOK && state->x.have == 0)
        (ಶೂನ್ಯ)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    ಹಿಂತಿರುಗಿ state->ನೇರ == 1;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzclose_r(gzFile file) {
    ಪೂರ್ಣಾಂಕ ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    /* free memory and close file */
    ಆದರೆ (state->ಗಾತ್ರ_2) {
        inflate_ಕೊನೆ(&(state->strm));
        ಬಿಡುಗಡೆ(state->ಹೊರಹೋಗು);
        ಬಿಡುಗಡೆ(state->ಒಳ);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, ಖಾಲಿ);
    ಬಿಡುಗಡೆ(state->ಮಾರ್ಗ);
    ret = close(state->fd);
    ಬಿಡುಗಡೆ(state);
    ಹಿಂತಿರುಗಿ ret ? Z_ERRNO : err;
}
