/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ଅନ୍ତର୍ଭାଵ "gzguts.h"

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
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_load(gz_statep state, unsigned ଅକ୍ଷର *buf, unsigned len,
                  unsigned *have) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    କର {
        get = len - *have;
        ଯଦି (get > max)
            get = max;
        ret = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ପଠ(state->fd, buf + *have, get);
        ଯଦି (ret <= 0)
            ଭାଙ୍ଗ;
        *have += (unsigned)ret;
    } ଯେତେବେଳେ (*have < len);
    ଯଦି (ret < 0) {
        ଯଦି (BLOCKED(errno)) {
            state->again = 1;
            ଯଦି (*have != 0)
                ଫେରନ୍ତୁ 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        ଫେରନ୍ତୁ -1;
    }
    ଯଦି (ret == 0)
        state->eof = 1;
    ଫେରନ୍ତୁ 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ଫେରନ୍ତୁ -1;
    ଯଦି (state->eof == 0) {
        ଯଦି (strm->avail_in) {       /* copy what's there to the start */
            unsigned ଅକ୍ଷର *p = state->in;
            unsigned ଅଚଳ ଅକ୍ଷର *q = strm->next_in;

            ଯଦି (q != p) {
                unsigned n = strm->avail_in;

                କର {
                    *p++ = *q++;
                } ଯେତେବେଳେ (--n);
            }
        }
        ଯଦି (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            ଫେରନ୍ତୁ -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    ଫେରନ୍ତୁ 0;
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
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    ଯଦି (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned ଅକ୍ଷର *)malloc(state->want);
        state->out = (unsigned ଅକ୍ଷର *)malloc(state->want << 1);
        ଯଦି (state->in == ଖାଲି || state->out == ଖାଲି) {
            ମୁକ୍ତ(state->out);
            ମୁକ୍ତ(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ଫେରନ୍ତୁ -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        ଯଦି (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            ମୁକ୍ତ(state->out);
            ମୁକ୍ତ(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ଫେରନ୍ତୁ -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    ଯଦି (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        ଯଦି (state->junk == 0 && strm->avail_in == 0) {
            ଯଦି (gz_avail(state) == -1)
                ଫେରନ୍ତୁ -1;
            ଯଦି (strm->avail_in == 0)
                ଫେରନ୍ତୁ 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        ଫେରନ୍ତୁ 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    ଯଦି (gz_avail(state) == -1)
        ଫେରନ୍ତୁ -1;
    ଯଦି (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        ଫେରନ୍ତୁ 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    ଯଦି (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        ଫେରନ୍ତୁ 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    ଫେରନ୍ତୁ 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_decomp(gz_statep state) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    କର {
        /* get more input for inflate() */
        ଯଦି (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            ଭାଙ୍ଗ;
        }
        ଯଦି (strm->avail_in == 0) {
            ଯଦି (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ଭାଙ୍ଗ;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        ଯଦି (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        ଯଦି (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            ଭାଙ୍ଗ;
        }
        ଯଦି (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ଭାଙ୍ଗ;
        }
        ଯଦି (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            ଯଦି (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                ଭାଙ୍ଗ;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->ସନ୍ଦେଶ == ଖାଲି ? "compressed data error" : strm->ସନ୍ଦେଶ);
            ଭାଙ୍ଗ;
        }
    } ଯେତେବେଳେ (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    ଯଦି (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        ଫେରନ୍ତୁ 0;
    }

    /* return decompression status */
    ଫେରନ୍ତୁ ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    କର {
        ବାଛ(state->how) {
        ସ୍ଥିତି LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            ଯଦି (gz_look(state) == -1)
                ଫେରନ୍ତୁ -1;
            ଯଦି (state->how == LOOK)
                ଫେରନ୍ତୁ 0;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି COPY:      /* -> COPY */
            ଯଦି (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                ଫେରନ୍ତୁ -1;
            state->x.next = state->out;
            ଫେରନ୍ତୁ 0;
        ସ୍ଥିତି GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            ଯଦି (gz_decomp(state) == -1)
                ଫେରନ୍ତୁ -1;
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            ଫେରନ୍ତୁ -1;
        }
    } ଯେତେବେଳେ (state->x.have == 0 && (!state->eof || strm->avail_in));
    ଫେରନ୍ତୁ 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    କର {
        /* skip over whatever is in output buffer */
        ଯଦି (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ଅନ୍ୟଥା ଯଦି (state->eof && state->strm.avail_in == 0) {
            ଯଦି (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ଭାଙ୍ଗ;
        }

        /* need more data to skip -- load up output buffer */
        ଅନ୍ୟଥା {
            /* get more output, looking for header if required */
            ଯଦି (gz_fetch(state) == -1)
                ଫେରନ୍ତୁ -1;
        }
    } ଯେତେବେଳେ (state->skip);
    ଫେରନ୍ତୁ 0;
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

    /* if len is zero, avoid unnecessary operations */
    ଯଦି (len == 0)
        ଫେରନ୍ତୁ 0;

    /* process a skip request */
    ଯଦି (state->skip && gz_skip(state) == -1)
        ଫେରନ୍ତୁ 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    କର {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        ଯଦି (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        ଯଦି (state->x.have) {
            ଯଦି (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            ଯଦି (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ଅନ୍ୟଥା ଯଦି (state->eof && state->strm.avail_in == 0) {
            ଯଦି (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ଭାଙ୍ଗ;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        ଅନ୍ୟଥା ଯଦି (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            ଯଦି (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            ଜାରି;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        ଅନ୍ୟଥା ଯଦି (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned ଅକ୍ଷର *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        ଅନ୍ୟଥା {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.next_out = (unsigned ଅକ୍ଷର *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (ଅକ୍ଷର *)buf + n;
        got += n;
        state->x.pos += n;
    } ଯେତେବେଳେ (len && !err);

    /* note read past eof */
    ଯଦି (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    ଫେରନ୍ତୁ got;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ)
        ଫେରନ୍ତୁ -1;

    /* check that there was no (serious) error */
    ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ଫେରନ୍ତୁ -1;
    ଯଦି (state->again)
        gz_error(state, Z_OK, ଖାଲି);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        ଫେରନ୍ତୁ -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    ଯଦି (len == 0) {
        ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR)
            ଫେରନ୍ତୁ -1;
        ଯଦି (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            ଫେରନ୍ତୁ -1;
        }
    }

    /* return the number of bytes read */
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ 0;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ)
        ଫେରନ୍ତୁ 0;

    /* check that there was no (serious) error */
    ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ଫେରନ୍ତୁ 0;
    ଯଦି (state->again)
        gz_error(state, Z_OK, ଖାଲି);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    ଯଦି (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ଫେରନ୍ତୁ 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    ଫେରନ୍ତୁ len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#ଅନ୍ୟଥା
#  undef gzgetc
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzgetc(gzFile file) {
    unsigned ଅକ୍ଷର buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ)
        ଫେରନ୍ତୁ -1;

    /* check that there was no (serious) error */
    ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ଫେରନ୍ତୁ -1;
    ଯଦି (state->again)
        gz_error(state, Z_OK, ଖାଲି);

    /* try output buffer (no need to check for skip request) */
    ଯଦି (state->x.have) {
        state->x.have--;
        state->x.pos++;
        ଫେରନ୍ତୁ *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    ଫେରନ୍ତୁ gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzgetc_(gzFile file) {
    ଫେରନ୍ତୁ gzgetc(file);
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzungetc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ)
        ଫେରନ୍ତୁ -1;

    /* in case this was just opened, set up the input buffer */
    ଯଦି (state->how == LOOK && state->x.have == 0)
        (ଶୂନ୍ୟ)gz_look(state);

    /* check that there was no (serious) error */
    ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ଫେରନ୍ତୁ -1;
    ଯଦି (state->again)
        gz_error(state, Z_OK, ଖାଲି);

    /* process a skip request */
    ଯଦି (state->skip && gz_skip(state) == -1)
        ଫେରନ୍ତୁ -1;

    /* can't push EOF */
    ଯଦି (c < 0)
        ଫେରନ୍ତୁ -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    ଯଦି (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned ଅକ୍ଷର)c;
        state->x.pos--;
        state->past = 0;
        ଫେରନ୍ତୁ c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    ଯଦି (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        ଫେରନ୍ତୁ -1;
    }

    /* slide output data if needed and insert byte before existing data */
    ଯଦି (state->x.next == state->out) {
        unsigned ଅକ୍ଷର *src = state->out + state->x.have;
        unsigned ଅକ୍ଷର *dest = state->out + (state->size << 1);

        ଯେତେବେଳେ (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned ଅକ୍ଷର)c;
    state->x.pos--;
    state->past = 0;
    ଫେରନ୍ତୁ c;
}

/* -- see zlib.h -- */
ଅକ୍ଷର * ZEXPORT gzgets(gzFile file, ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len) {
    unsigned ବାମ, n;
    ଅକ୍ଷର *str;
    unsigned ଅକ୍ଷର *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    ଯଦି (file == ଖାଲି || buf == ଖାଲି || len < 1)
        ଫେରନ୍ତୁ ଖାଲି;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ)
        ଫେରନ୍ତୁ ଖାଲି;

    /* check that there was no (serious) error */
    ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ଫେରନ୍ତୁ ଖାଲି;
    ଯଦି (state->again)
        gz_error(state, Z_OK, ଖାଲି);

    /* process a skip request */
    ଯଦି (state->skip && gz_skip(state) == -1)
        ଫେରନ୍ତୁ ଖାଲି;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    ବାମ = (unsigned)len - 1;
    ଯଦି (ବାମ) କର {
        /* assure that something is in the output buffer */
        ଯଦି (state->x.have == 0 && gz_fetch(state) == -1)
            ଭାଙ୍ଗ;                      /* error */
        ଯଦି (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            ଭାଙ୍ଗ;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > ବାମ ? ବାମ : state->x.have;
        eol = (unsigned ଅକ୍ଷର *)memchr(state->x.next, '\n', n);
        ଯଦି (eol != ଖାଲି)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        ବାମ -= n;
        buf += n;
    } ଯେତେବେଳେ (ବାମ && eol == ଖାଲି);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    ଯଦି (buf == str)
        ଫେରନ୍ତୁ ଖାଲି;
    buf[0] = 0;
    ଫେରନ୍ତୁ str;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    ଯଦି (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (ଶୂନ୍ୟ)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    ଫେରନ୍ତୁ state->direct == 1;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzclose_r(gzFile file) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ)
        ଫେରନ୍ତୁ Z_STREAM_ERROR;

    /* free memory and close file */
    ଯଦି (state->size) {
        inflateEnd(&(state->strm));
        ମୁକ୍ତ(state->out);
        ମୁକ୍ତ(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, ଖାଲି);
    ମୁକ୍ତ(state->path);
    ret = close(state->fd);
    ମୁକ୍ତ(state);
    ଫେରନ୍ତୁ ret ? Z_ERRNO : err;
}
