/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#समावेश "gzguts.h"

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
local पूर्णांक gz_load(gz_statep state, अचिन्हित अक्षर *buf, अचिन्हित len,
                  अचिन्हित *have) {
    पूर्णांक ret;
    अचिन्हित get, max = ((अचिन्हित)-1 >> 2) + 1;

    state->again = 0;
    त्रुटि_सं = 0;
    *have = 0;
    करो {
        get = len - *have;
        अगर (get > max)
            get = max;
        ret = (पूर्णांक)पढ़ो(state->fd, buf + *have, get);
        अगर (ret <= 0)
            अवरोध;
        *have += (अचिन्हित)ret;
    } जबतक (*have < len);
    अगर (ret < 0) {
        अगर (BLOCKED(त्रुटि_सं)) {
            state->again = 1;
            अगर (*have != 0)
                वापस 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        वापस -1;
    }
    अगर (ret == 0)
        state->eof = 1;
    वापस 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local पूर्णांक gz_avail(gz_statep state) {
    अचिन्हित got;
    z_streamp strm = &(state->strm);

    अगर (state->err != Z_OK && state->err != Z_BUF_ERROR)
        वापस -1;
    अगर (state->eof == 0) {
        अगर (strm->avail_in) {       /* copy what's there to the start */
            अचिन्हित अक्षर *p = state->in;
            अचिन्हित स्थिर अक्षर *q = strm->अग्र_in;

            अगर (q != p) {
                अचिन्हित n = strm->avail_in;

                करो {
                    *p++ = *q++;
                } जबतक (--n);
            }
        }
        अगर (gz_load(state, state->in + strm->avail_in,
                    state->आकार - strm->avail_in, &got) == -1)
            वापस -1;
        strm->avail_in += got;
        strm->अग्र_in = state->in;
    }
    वापस 0;
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
local पूर्णांक gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    अगर (state->आकार == 0) {
        /* allocate buffers */
        state->in = (अचिन्हित अक्षर *)दो_स्मृति(state->want);
        state->out = (अचिन्हित अक्षर *)दो_स्मृति(state->want << 1);
        अगर (state->in == शून्य || state->out == शून्य) {
            मुक्त(state->out);
            मुक्त(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            वापस -1;
        }
        state->आकार = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.अग्र_in = Z_NULL;
        अगर (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            मुक्त(state->out);
            मुक्त(state->in);
            state->आकार = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            वापस -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    अगर (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        अगर (state->junk == 0 && strm->avail_in == 0) {
            अगर (gz_avail(state) == -1)
                वापस -1;
            अगर (strm->avail_in == 0)
                वापस 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        वापस 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    अगर (gz_avail(state) == -1)
        वापस -1;
    अगर (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        वापस 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    अगर (strm->avail_in > 3 &&
            strm->अग्र_in[0] == 31 && strm->अग्र_in[1] == 139 &&
            strm->अग्र_in[2] == 8 && strm->अग्र_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        वापस 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.अग्र = state->out;
    स_नकल(state->x.अग्र, strm->अग्र_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    वापस 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local पूर्णांक gz_decomp(gz_statep state) {
    पूर्णांक ret = Z_OK;
    अचिन्हित had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    करो {
        /* get more input for inflate() */
        अगर (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            अवरोध;
        }
        अगर (strm->avail_in == 0) {
            अगर (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            अवरोध;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        अगर (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        अगर (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            अवरोध;
        }
        अगर (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            अवरोध;
        }
        अगर (ret == Z_तथ्य_ERROR) {              /* deflate stream invalid */
            अगर (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                अवरोध;
            }
            gz_error(state, Z_तथ्य_ERROR,
                     strm->संदेश == शून्य ? "compressed data error" : strm->संदेश);
            अवरोध;
        }
    } जबतक (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.अग्र = strm->अग्र_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    अगर (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        वापस 0;
    }

    /* return decompression status */
    वापस ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local पूर्णांक gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    करो {
        चयन(state->how) {
        हाल LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            अगर (gz_look(state) == -1)
                वापस -1;
            अगर (state->how == LOOK)
                वापस 0;
            अवरोध;
        हाल COPY:      /* -> COPY */
            अगर (gz_load(state, state->out, state->आकार << 1, &(state->x.have))
                    == -1)
                वापस -1;
            state->x.अग्र = state->out;
            वापस 0;
        हाल GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->आकार << 1;
            strm->अग्र_out = state->out;
            अगर (gz_decomp(state) == -1)
                वापस -1;
            अवरोध;
        शेष:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            वापस -1;
        }
    } जबतक (state->x.have == 0 && (!state->eof || strm->avail_in));
    वापस 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local पूर्णांक gz_skip(gz_statep state) {
    अचिन्हित n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    करो {
        /* skip over whatever is in output buffer */
        अगर (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (अचिन्हित)state->skip : state->x.have;
            state->x.have -= n;
            state->x.अग्र += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        अन्यथा अगर (state->eof && state->strm.avail_in == 0) {
            अगर (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            अवरोध;
        }

        /* need more data to skip -- load up output buffer */
        अन्यथा {
            /* get more output, looking for header if required */
            अगर (gz_fetch(state) == -1)
                वापस -1;
        }
    } जबतक (state->skip);
    वापस 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_आकार_t gz_read(gz_statep state, voidp buf, z_आकार_t len) {
    z_आकार_t got;
    अचिन्हित n;
    पूर्णांक err;

    /* if len is zero, avoid unnecessary operations */
    अगर (len == 0)
        वापस 0;

    /* process a skip request */
    अगर (state->skip && gz_skip(state) == -1)
        वापस 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    करो {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (अचिन्हित)-1;
        अगर (n > len)
            n = (अचिन्हित)len;

        /* first just try copying data from the output buffer */
        अगर (state->x.have) {
            अगर (state->x.have < n)
                n = state->x.have;
            स_नकल(buf, state->x.अग्र, n);
            state->x.अग्र += n;
            state->x.have -= n;
            अगर (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        अन्यथा अगर (state->eof && state->strm.avail_in == 0) {
            अगर (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            अवरोध;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        अन्यथा अगर (state->how == LOOK || n < (state->आकार << 1)) {
            /* get more output, looking for header if required */
            अगर (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            जारी;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        अन्यथा अगर (state->how == COPY)        /* read directly */
            err = gz_load(state, (अचिन्हित अक्षर *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        अन्यथा {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.अग्र_out = (अचिन्हित अक्षर *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (अक्षर *)buf + n;
        got += n;
        state->x.pos += n;
    } जबतक (len && !err);

    /* note read past eof */
    अगर (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    वापस got;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzread(gzFile file, voidp buf, अचिन्हित len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ)
        वापस -1;

    /* check that there was no (serious) error */
    अगर (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        वापस -1;
    अगर (state->again)
        gz_error(state, Z_OK, शून्य);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    अगर ((पूर्णांक)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        वापस -1;
    }

    /* read len or fewer bytes to buf */
    len = (अचिन्हित)gz_read(state, buf, len);

    /* check for an error */
    अगर (len == 0) {
        अगर (state->err != Z_OK && state->err != Z_BUF_ERROR)
            वापस -1;
        अगर (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            वापस -1;
        }
    }

    /* return the number of bytes read */
    वापस (पूर्णांक)len;
}

/* -- see zlib.h -- */
z_आकार_t ZEXPORT gzfread(voidp buf, z_आकार_t आकार, z_आकार_t nitems,
                         gzFile file) {
    z_आकार_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    अगर (file == शून्य)
        वापस 0;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ)
        वापस 0;

    /* check that there was no (serious) error */
    अगर (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        वापस 0;
    अगर (state->again)
        gz_error(state, Z_OK, शून्य);

    /* compute bytes to read -- error on overflow */
    len = nitems * आकार;
    अगर (आकार && len / आकार != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        वापस 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    वापस len ? gz_read(state, buf, len) / आकार : 0;
}

/* -- see zlib.h -- */
#अगर_घोषित Z_PREFIX_SET
#  undef z_gzgetc
#अन्यथा
#  undef gzgetc
#पूर्ण_अगर
पूर्णांक ZEXPORT gzgetc(gzFile file) {
    अचिन्हित अक्षर buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ)
        वापस -1;

    /* check that there was no (serious) error */
    अगर (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        वापस -1;
    अगर (state->again)
        gz_error(state, Z_OK, शून्य);

    /* try output buffer (no need to check for skip request) */
    अगर (state->x.have) {
        state->x.have--;
        state->x.pos++;
        वापस *(state->x.अग्र)++;
    }

    /* nothing there -- try gz_read() */
    वापस gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

पूर्णांक ZEXPORT gzgetc_(gzFile file) {
    वापस gzgetc(file);
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzungetc(पूर्णांक c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ)
        वापस -1;

    /* in case this was just opened, set up the input buffer */
    अगर (state->how == LOOK && state->x.have == 0)
        (व्योम)gz_look(state);

    /* check that there was no (serious) error */
    अगर (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        वापस -1;
    अगर (state->again)
        gz_error(state, Z_OK, शून्य);

    /* process a skip request */
    अगर (state->skip && gz_skip(state) == -1)
        वापस -1;

    /* can't push EOF */
    अगर (c < 0)
        वापस -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    अगर (state->x.have == 0) {
        state->x.have = 1;
        state->x.अग्र = state->out + (state->आकार << 1) - 1;
        state->x.अग्र[0] = (अचिन्हित अक्षर)c;
        state->x.pos--;
        state->past = 0;
        वापस c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    अगर (state->x.have == (state->आकार << 1)) {
        gz_error(state, Z_तथ्य_ERROR, "out of room to push characters");
        वापस -1;
    }

    /* slide output data if needed and insert byte before existing data */
    अगर (state->x.अग्र == state->out) {
        अचिन्हित अक्षर *src = state->out + state->x.have;
        अचिन्हित अक्षर *dest = state->out + (state->आकार << 1);

        जबतक (src > state->out)
            *--dest = *--src;
        state->x.अग्र = dest;
    }
    state->x.have++;
    state->x.अग्र--;
    state->x.अग्र[0] = (अचिन्हित अक्षर)c;
    state->x.pos--;
    state->past = 0;
    वापस c;
}

/* -- see zlib.h -- */
अक्षर * ZEXPORT gzgets(gzFile file, अक्षर *buf, पूर्णांक len) {
    अचिन्हित बायाँ, n;
    अक्षर *str;
    अचिन्हित अक्षर *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    अगर (file == शून्य || buf == शून्य || len < 1)
        वापस शून्य;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ)
        वापस शून्य;

    /* check that there was no (serious) error */
    अगर (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        वापस शून्य;
    अगर (state->again)
        gz_error(state, Z_OK, शून्य);

    /* process a skip request */
    अगर (state->skip && gz_skip(state) == -1)
        वापस शून्य;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    बायाँ = (अचिन्हित)len - 1;
    अगर (बायाँ) करो {
        /* assure that something is in the output buffer */
        अगर (state->x.have == 0 && gz_fetch(state) == -1)
            अवरोध;                      /* error */
        अगर (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            अवरोध;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > बायाँ ? बायाँ : state->x.have;
        eol = (अचिन्हित अक्षर *)स_प्रथम(state->x.अग्र, '\n', n);
        अगर (eol != शून्य)
            n = (अचिन्हित)(eol - state->x.अग्र) + 1;

        /* copy through end-of-line, or remainder if not found */
        स_नकल(buf, state->x.अग्र, n);
        state->x.have -= n;
        state->x.अग्र += n;
        state->x.pos += n;
        बायाँ -= n;
        buf += n;
    } जबतक (बायाँ && eol == शून्य);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    अगर (buf == str)
        वापस शून्य;
    buf[0] = 0;
    वापस str;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    अगर (file == शून्य)
        वापस 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    अगर (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (व्योम)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    वापस state->direct == 1;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzclose_r(gzFile file) {
    पूर्णांक ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    अगर (file == शून्य)
        वापस Z_STREAM_ERROR;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ)
        वापस Z_STREAM_ERROR;

    /* free memory and close file */
    अगर (state->आकार) {
        inflateEnd(&(state->strm));
        मुक्त(state->out);
        मुक्त(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, शून्य);
    मुक्त(state->path);
    ret = बंद(state->fd);
    मुक्त(state);
    वापस ret ? Z_ERRNO : err;
}
