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
local पूर्णाङ्क gz_load(gz_statep state, अचिन्हित वर्ण *buf, अचिन्हित len,
                  अचिन्हित *have) {
    पूर्णाङ्क ret;
    अचिन्हित get, max = ((अचिन्हित)-1 >> 2) + 1;

    state->again = 0;
    त्रुटि_सं = 0;
    *have = 0;
    गर {
        get = len - *have;
        यदि (get > max)
            get = max;
        ret = (पूर्णाङ्क)पठ(state->fd, buf + *have, get);
        यदि (ret <= 0)
            रोक;
        *have += (अचिन्हित)ret;
    } जबसम्म (*have < len);
    यदि (ret < 0) {
        यदि (BLOCKED(त्रुटि_सं)) {
            state->again = 1;
            यदि (*have != 0)
                फर्काऊ 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        फर्काऊ -1;
    }
    यदि (ret == 0)
        state->eof = 1;
    फर्काऊ 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local पूर्णाङ्क gz_avail(gz_statep state) {
    अचिन्हित got;
    z_streamp strm = &(state->strm);

    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR)
        फर्काऊ -1;
    यदि (state->eof == 0) {
        यदि (strm->avail_in) {       /* copy what's there to the start */
            अचिन्हित वर्ण *p = state->in;
            अचिन्हित अचर वर्ण *q = strm->next_in;

            यदि (q != p) {
                अचिन्हित n = strm->avail_in;

                गर {
                    *p++ = *q++;
                } जबसम्म (--n);
            }
        }
        यदि (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            फर्काऊ -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    फर्काऊ 0;
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
        state->in = (अचिन्हित वर्ण *)दो_स्मृति(state->want);
        state->out = (अचिन्हित वर्ण *)दो_स्मृति(state->want << 1);
        यदि (state->in == रिक्त || state->out == रिक्त) {
            मुक्त(state->out);
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            फर्काऊ -1;
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
            फर्काऊ -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    यदि (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        यदि (state->junk == 0 && strm->avail_in == 0) {
            यदि (gz_avail(state) == -1)
                फर्काऊ -1;
            यदि (strm->avail_in == 0)
                फर्काऊ 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        फर्काऊ 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    यदि (gz_avail(state) == -1)
        फर्काऊ -1;
    यदि (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        फर्काऊ 0;

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
        फर्काऊ 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    स_नकल(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    फर्काऊ 0;
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
    अचिन्हित had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    गर {
        /* get more input for inflate() */
        यदि (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            रोक;
        }
        यदि (strm->avail_in == 0) {
            यदि (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            रोक;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        यदि (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        यदि (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            रोक;
        }
        यदि (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            रोक;
        }
        यदि (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            यदि (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                रोक;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->सन्देश == रिक्त ? "compressed data error" : strm->सन्देश);
            रोक;
        }
    } जबसम्म (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    यदि (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        फर्काऊ 0;
    }

    /* return decompression status */
    फर्काऊ ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local पूर्णाङ्क gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    गर {
        विकल्प(state->how) {
        पक्ष LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            यदि (gz_look(state) == -1)
                फर्काऊ -1;
            यदि (state->how == LOOK)
                फर्काऊ 0;
            रोक;
        पक्ष COPY:      /* -> COPY */
            यदि (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                फर्काऊ -1;
            state->x.next = state->out;
            फर्काऊ 0;
        पक्ष GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            यदि (gz_decomp(state) == -1)
                फर्काऊ -1;
            रोक;
        स्वभाव:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            फर्काऊ -1;
        }
    } जबसम्म (state->x.have == 0 && (!state->eof || strm->avail_in));
    फर्काऊ 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local पूर्णाङ्क gz_skip(gz_statep state) {
    अचिन्हित n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    गर {
        /* skip over whatever is in output buffer */
        यदि (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (अचिन्हित)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        अन्यथा यदि (state->eof && state->strm.avail_in == 0) {
            यदि (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            रोक;
        }

        /* need more data to skip -- load up output buffer */
        अन्यथा {
            /* get more output, looking for header if required */
            यदि (gz_fetch(state) == -1)
                फर्काऊ -1;
        }
    } जबसम्म (state->skip);
    फर्काऊ 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_size_t gz_read(gz_statep state, voidp buf, z_size_t len) {
    z_size_t got;
    अचिन्हित n;
    पूर्णाङ्क err;

    /* if len is zero, avoid unnecessary operations */
    यदि (len == 0)
        फर्काऊ 0;

    /* process a skip request */
    यदि (state->skip && gz_skip(state) == -1)
        फर्काऊ 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    गर {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (अचिन्हित)-1;
        यदि (n > len)
            n = (अचिन्हित)len;

        /* first just try copying data from the output buffer */
        यदि (state->x.have) {
            यदि (state->x.have < n)
                n = state->x.have;
            स_नकल(buf, state->x.next, n);
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
            रोक;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        अन्यथा यदि (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            यदि (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            जारी;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        अन्यथा यदि (state->how == COPY)        /* read directly */
            err = gz_load(state, (अचिन्हित वर्ण *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        अन्यथा {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.next_out = (अचिन्हित वर्ण *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (वर्ण *)buf + n;
        got += n;
        state->x.pos += n;
    } जबसम्म (len && !err);

    /* note read past eof */
    यदि (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    फर्काऊ got;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzread(gzFile file, voidp buf, अचिन्हित len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        फर्काऊ -1;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        फर्काऊ -1;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    यदि ((पूर्णाङ्क)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        फर्काऊ -1;
    }

    /* read len or fewer bytes to buf */
    len = (अचिन्हित)gz_read(state, buf, len);

    /* check for an error */
    यदि (len == 0) {
        यदि (state->err != Z_OK && state->err != Z_BUF_ERROR)
            फर्काऊ -1;
        यदि (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            फर्काऊ -1;
        }
    }

    /* return the number of bytes read */
    फर्काऊ (पूर्णाङ्क)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        फर्काऊ 0;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        फर्काऊ 0;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        फर्काऊ 0;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    यदि (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        फर्काऊ 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    फर्काऊ len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#अन्यथा
#  undef gzgetc
#endif
पूर्णाङ्क ZEXPORT gzgetc(gzFile file) {
    अचिन्हित वर्ण buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        फर्काऊ -1;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        फर्काऊ -1;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* try output buffer (no need to check for skip request) */
    यदि (state->x.have) {
        state->x.have--;
        state->x.pos++;
        फर्काऊ *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    फर्काऊ gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

पूर्णाङ्क ZEXPORT gzgetc_(gzFile file) {
    फर्काऊ gzgetc(file);
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzungetc(पूर्णाङ्क c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        फर्काऊ -1;

    /* in case this was just opened, set up the input buffer */
    यदि (state->how == LOOK && state->x.have == 0)
        (शून्य)gz_look(state);

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        फर्काऊ -1;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* process a skip request */
    यदि (state->skip && gz_skip(state) == -1)
        फर्काऊ -1;

    /* can't push EOF */
    यदि (c < 0)
        फर्काऊ -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    यदि (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (अचिन्हित वर्ण)c;
        state->x.pos--;
        state->past = 0;
        फर्काऊ c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    यदि (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        फर्काऊ -1;
    }

    /* slide output data if needed and insert byte before existing data */
    यदि (state->x.next == state->out) {
        अचिन्हित वर्ण *src = state->out + state->x.have;
        अचिन्हित वर्ण *dest = state->out + (state->size << 1);

        जबसम्म (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (अचिन्हित वर्ण)c;
    state->x.pos--;
    state->past = 0;
    फर्काऊ c;
}

/* -- see zlib.h -- */
वर्ण * ZEXPORT gzgets(gzFile file, वर्ण *buf, पूर्णाङ्क len) {
    अचिन्हित बायाँ, n;
    वर्ण *str;
    अचिन्हित वर्ण *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    यदि (file == रिक्त || buf == रिक्त || len < 1)
        फर्काऊ रिक्त;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        फर्काऊ रिक्त;

    /* check that there was no (serious) error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        फर्काऊ रिक्त;
    यदि (state->again)
        gz_error(state, Z_OK, रिक्त);

    /* process a skip request */
    यदि (state->skip && gz_skip(state) == -1)
        फर्काऊ रिक्त;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    बायाँ = (अचिन्हित)len - 1;
    यदि (बायाँ) गर {
        /* assure that something is in the output buffer */
        यदि (state->x.have == 0 && gz_fetch(state) == -1)
            रोक;                      /* error */
        यदि (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            रोक;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > बायाँ ? बायाँ : state->x.have;
        eol = (अचिन्हित वर्ण *)स_प्रथम(state->x.next, '\n', n);
        यदि (eol != रिक्त)
            n = (अचिन्हित)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        स_नकल(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        बायाँ -= n;
        buf += n;
    } जबसम्म (बायाँ && eol == रिक्त);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    यदि (buf == str)
        फर्काऊ रिक्त;
    buf[0] = 0;
    फर्काऊ str;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        फर्काऊ 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    यदि (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (शून्य)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    फर्काऊ state->direct == 1;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzclose_r(gzFile file) {
    पूर्णाङ्क ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    यदि (file == रिक्त)
        फर्काऊ Z_STREAM_ERROR;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ)
        फर्काऊ Z_STREAM_ERROR;

    /* free memory and close file */
    यदि (state->size) {
        inflateEnd(&(state->strm));
        मुक्त(state->out);
        मुक्त(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, रिक्त);
    मुक्त(state->path);
    ret = बंद(state->fd);
    मुक्त(state);
    फर्काऊ ret ? Z_ERRNO : err;
}
