/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "gzguts.h"

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
local पूर्णाङ्क gz_load(gz_statep state, unsigned वर्ण *buf, unsigned len,
                  unsigned *have) {
    पूर्णाङ्क ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    कुरु {
        get = len - *have;
        यदि (get > max)
            get = max;
        ret = (पूर्णाङ्क)पठ(state->fd, buf + *have, get);
        यदि (ret <= 0)
            भङ्ग;
        *have += (unsigned)ret;
    } यावत् (*have < len);
    यदि (ret < 0) {
        यदि (BLOCKED(errno)) {
            state->again = 1;
            यदि (*have != 0)
                प्रत्यागम 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        प्रत्यागम -1;
    }
    यदि (ret == 0)
        state->eof = 1;
    प्रत्यागम 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local पूर्णाङ्क gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR)
        प्रत्यागम -1;
    यदि (state->eof == 0) {
        यदि (strm->avail_in) {       /* copy what's there to the start */
            unsigned वर्ण *p = state->in;
            unsigned ध्रुव वर्ण *q = strm->next_in;

            यदि (q != p) {
                unsigned n = strm->avail_in;

                कुरु {
                    *p++ = *q++;
                } यावत् (--n);
            }
        }
        यदि (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            प्रत्यागम -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    प्रत्यागम 0;
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
local पूर्णाङ्क gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    यदि (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned वर्ण *)malloc(state->want);
        state->out = (unsigned वर्ण *)malloc(state->want << 1);
        यदि (state->in == रिक्त || state->out == रिक्त) {
            मुक्त(state->out);
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            प्रत्यागम -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        यदि (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            मुक्त(state->out);
            मुक्त(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            प्रत्यागम -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    यदि (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        यदि (state->junk == 0 && strm->avail_in == 0) {
            यदि (gz_avail(state) == -1)
                प्रत्यागम -1;
            यदि (strm->avail_in == 0)
                प्रत्यागम 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        प्रत्यागम 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    यदि (gz_avail(state) == -1)
        प्रत्यागम -1;
    यदि (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        प्रत्यागम 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    यदि (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        प्रत्यागम 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    प्रत्यागम 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local पूर्णाङ्क gz_decomp(gz_statep state) {
    पूर्णाङ्क ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    कुरु {
        /* get more input for inflate() */
        यदि (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            भङ्ग;
        }
        यदि (strm->avail_in == 0) {
            यदि (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            भङ्ग;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        यदि (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        यदि (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            भङ्ग;
        }
        यदि (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            भङ्ग;
        }
        यदि (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            यदि (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                भङ्ग;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->संदेश == रिक्त ? "compressed data error" : strm->संदेश);
            भङ्ग;
        }
    } यावत् (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    यदि (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        प्रत्यागम 0;
    }

    /* return decompression status */
    प्रत्यागम ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local पूर्णाङ्क gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    कुरु {
        विकल्प(state->how) {
        पक्ष LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            यदि (gz_look(state) == -1)
                प्रत्यागम -1;
            यदि (state->how == LOOK)
                प्रत्यागम 0;
            भङ्ग;
        पक्ष COPY:      /* -> COPY */
            यदि (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                प्रत्यागम -1;
            state->x.next = state->out;
            प्रत्यागम 0;
        पक्ष GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            यदि (gz_decomp(state) == -1)
                प्रत्यागम -1;
            भङ्ग;
        स्वभाव:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            प्रत्यागम -1;
        }
    } यावत् (state->x.have == 0 && (!state->eof || strm->avail_in));
    प्रत्यागम 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local पूर्णाङ्क gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    कुरु {
        /* skip over whatever is in output buffer */
        यदि (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        अन्यथा यदि (state->eof && state->strm.avail_in == 0) {
            यदि (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            भङ्ग;
        }

        /* need more data to skip -- load up output buffer */
        अन्यथा {
            /* get more output, looking for header if required */
            यदि (gz_fetch(state) == -1)
                प्रत्यागम -1;
        }
    } यावत् (state->skip);
    प्रत्यागम 0;
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
    पूर्णाङ्क err;

    /* if len is zero, avoid unnecessary operations */
    यदि (len == 0)
        प्रत्यागम 0;

    /* process a skip request */
    यदि (state->skip && gz_skip(state) == -1)
        प्रत्यागम 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    कुरु {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        यदि (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        यदि (state->x.have) {
            यदि (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            यदि (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        अन्यथा यदि (state->eof && state->strm.avail_in == 0) {
            यदि (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            भङ्ग;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        अन्यथा यदि (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            यदि (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            अनुवर्त;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        अन्यथा यदि (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned वर्ण *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        अन्यथा {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.next_out = (unsigned वर्ण *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (वर्ण *)buf + n;
        got += n;
        state->x.pos += n;
    } यावत् (len && !err);

    /* note read past eof */
    यदि (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    प्रत्यागम got;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        प्रत्यागम -1;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        प्रत्यागम -1;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    यदि ((पूर्णाङ्क)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        प्रत्यागम -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    यदि (len == 0) {
        यदि (state->err != Z_OK && state->err != Z_BUF_ERROR)
            प्रत्यागम -1;
        यदि (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            प्रत्यागम -1;
        }
    }

    /* return the number of bytes read */
    प्रत्यागम (पूर्णाङ्क)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        प्रत्यागम 0;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        प्रत्यागम 0;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        प्रत्यागम 0;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    यदि (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        प्रत्यागम 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    प्रत्यागम len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#अन्यथा
#  undef gzgetc
#endif
पूर्णाङ्क ZEXPORT gzgetc(gzFile file) {
    unsigned वर्ण buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        प्रत्यागम -1;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        प्रत्यागम -1;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* try output buffer (no need to check for skip request) */
    यदि (state->x.have) {
        state->x.have--;
        state->x.pos++;
        प्रत्यागम *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    प्रत्यागम gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

पूर्णाङ्क ZEXPORT gzgetc_(gzFile file) {
    प्रत्यागम gzgetc(file);
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzungetc(पूर्णाङ्क c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        प्रत्यागम -1;

    /* in case this was just opened, set up the input buffer */
    यदि (state->how == LOOK && state->x.have == 0)
        (शून्य)gz_look(state);

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        प्रत्यागम -1;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* process a skip request */
    यदि (state->skip && gz_skip(state) == -1)
        प्रत्यागम -1;

    /* can't push EOF */
    यदि (c < 0)
        प्रत्यागम -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    यदि (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned वर्ण)c;
        state->x.pos--;
        state->past = 0;
        प्रत्यागम c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    यदि (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        प्रत्यागम -1;
    }

    /* slide output data if needed and insert byte before existing data */
    यदि (state->x.next == state->out) {
        unsigned वर्ण *src = state->out + state->x.have;
        unsigned वर्ण *dest = state->out + (state->size << 1);

        यावत् (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned वर्ण)c;
    state->x.pos--;
    state->past = 0;
    प्रत्यागम c;
}

/* -- see zlib.h -- */
वर्ण * ZEXPORT gzgets(gzFile file, वर्ण *buf, पूर्णाङ्क len) {
    unsigned डावा, n;
    वर्ण *str;
    unsigned वर्ण *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    यदि (file == रिक्त || buf == रिक्त || len < 1)
        प्रत्यागम रिक्त;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        प्रत्यागम रिक्त;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        प्रत्यागम रिक्त;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* process a skip request */
    यदि (state->skip && gz_skip(state) == -1)
        प्रत्यागम रिक्त;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    डावा = (unsigned)len - 1;
    यदि (डावा) कुरु {
        /* assure that something is in the output buffer */
        यदि (state->x.have == 0 && gz_fetch(state) == -1)
            भङ्ग;                      /* error */
        यदि (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            भङ्ग;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > डावा ? डावा : state->x.have;
        eol = (unsigned वर्ण *)memchr(state->x.next, '\n', n);
        यदि (eol != रिक्त)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        डावा -= n;
        buf += n;
    } यावत् (डावा && eol == रिक्त);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    यदि (buf == str)
        प्रत्यागम रिक्त;
    buf[0] = 0;
    प्रत्यागम str;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    यदि (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (शून्य)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    प्रत्यागम state->direct == 1;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzclose_r(gzFile file) {
    पूर्णाङ्क ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        प्रत्यागम Z_STREAM_ERROR;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        प्रत्यागम Z_STREAM_ERROR;

    /* free memory and close file */
    यदि (state->size) {
        inflateEnd(&(state->strm));
        मुक्त(state->out);
        मुक्त(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, रिक्त);
    मुक्त(state->path);
    ret = close(state->fd);
    मुक्त(state);
    प्रत्यागम ret ? Z_ERRNO : err;
}
