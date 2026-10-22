/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "gzguts.h"

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
local صحیح gz_load(gz_statep state, unsigned نویسه *buf, unsigned len,
                  unsigned *have) {
    صحیح ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    انجامبده {
        get = len - *have;
        اگر (get > max)
            get = max;
        ret = (صحیح)اقرأ(state->fd, buf + *have, get);
        اگر (ret <= 0)
            بشکن;
        *have += (unsigned)ret;
    } وقتی (*have < len);
    اگر (ret < 0) {
        اگر (BLOCKED(errno)) {
            state->again = 1;
            اگر (*have != 0)
                بازگشت 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        بازگشت -1;
    }
    اگر (ret == 0)
        state->eof = 1;
    بازگشت 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local صحیح gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR)
        بازگشت -1;
    اگر (state->eof == 0) {
        اگر (strm->avail_in) {       /* copy what's there to the start */
            unsigned نویسه *p = state->in;
            unsigned ثابت نویسه *q = strm->next_in;

            اگر (q != p) {
                unsigned n = strm->avail_in;

                انجامبده {
                    *p++ = *q++;
                } وقتی (--n);
            }
        }
        اگر (gz_load(state, state->in + strm->avail_in,
                    state->size - strm->avail_in, &got) == -1)
            بازگشت -1;
        strm->avail_in += got;
        strm->next_in = state->in;
    }
    بازگشت 0;
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
local صحیح gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    اگر (state->size == 0) {
        /* allocate buffers */
        state->in = (unsigned نویسه *)malloc(state->want);
        state->out = (unsigned نویسه *)malloc(state->want << 1);
        اگر (state->in == پوچ || state->out == پوچ) {
            آزاد(state->out);
            آزاد(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            بازگشت -1;
        }
        state->size = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.next_in = Z_NULL;
        اگر (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            آزاد(state->out);
            آزاد(state->in);
            state->size = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            بازگشت -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    اگر (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        اگر (state->junk == 0 && strm->avail_in == 0) {
            اگر (gz_avail(state) == -1)
                بازگشت -1;
            اگر (strm->avail_in == 0)
                بازگشت 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        بازگشت 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    اگر (gz_avail(state) == -1)
        بازگشت -1;
    اگر (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        بازگشت 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    اگر (strm->avail_in > 3 &&
            strm->next_in[0] == 31 && strm->next_in[1] == 139 &&
            strm->next_in[2] == 8 && strm->next_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        بازگشت 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.next = state->out;
    memcpy(state->x.next, strm->next_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    بازگشت 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local صحیح gz_decomp(gz_statep state) {
    صحیح ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    انجامبده {
        /* get more input for inflate() */
        اگر (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            بشکن;
        }
        اگر (strm->avail_in == 0) {
            اگر (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            بشکن;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        اگر (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        اگر (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            بشکن;
        }
        اگر (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            بشکن;
        }
        اگر (ret == Z_DATA_ERROR) {              /* deflate stream invalid */
            اگر (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                بشکن;
            }
            gz_error(state, Z_DATA_ERROR,
                     strm->پیام == پوچ ? "compressed data error" : strm->پیام);
            بشکن;
        }
    } وقتی (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.next = strm->next_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    اگر (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        بازگشت 0;
    }

    /* return decompression status */
    بازگشت ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local صحیح gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    انجامبده {
        انتخاب(state->how) {
        حالت LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            اگر (gz_look(state) == -1)
                بازگشت -1;
            اگر (state->how == LOOK)
                بازگشت 0;
            بشکن;
        حالت COPY:      /* -> COPY */
            اگر (gz_load(state, state->out, state->size << 1, &(state->x.have))
                    == -1)
                بازگشت -1;
            state->x.next = state->out;
            بازگشت 0;
        حالت GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->size << 1;
            strm->next_out = state->out;
            اگر (gz_decomp(state) == -1)
                بازگشت -1;
            بشکن;
        پیشفرض:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            بازگشت -1;
        }
    } وقتی (state->x.have == 0 && (!state->eof || strm->avail_in));
    بازگشت 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local صحیح gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    انجامبده {
        /* skip over whatever is in output buffer */
        اگر (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.next += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        وگرنه اگر (state->eof && state->strm.avail_in == 0) {
            اگر (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            بشکن;
        }

        /* need more data to skip -- load up output buffer */
        وگرنه {
            /* get more output, looking for header if required */
            اگر (gz_fetch(state) == -1)
                بازگشت -1;
        }
    } وقتی (state->skip);
    بازگشت 0;
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
    صحیح err;

    /* if len is zero, avoid unnecessary operations */
    اگر (len == 0)
        بازگشت 0;

    /* process a skip request */
    اگر (state->skip && gz_skip(state) == -1)
        بازگشت 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    انجامبده {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        اگر (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        اگر (state->x.have) {
            اگر (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.next, n);
            state->x.next += n;
            state->x.have -= n;
            اگر (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        وگرنه اگر (state->eof && state->strm.avail_in == 0) {
            اگر (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            بشکن;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        وگرنه اگر (state->how == LOOK || n < (state->size << 1)) {
            /* get more output, looking for header if required */
            اگر (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            ادامه;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        وگرنه اگر (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned نویسه *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        وگرنه {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.next_out = (unsigned نویسه *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (نویسه *)buf + n;
        got += n;
        state->x.pos += n;
    } وقتی (len && !err);

    /* note read past eof */
    اگر (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    بازگشت got;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ)
        بازگشت -1;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        بازگشت -1;
    اگر (state->again)
        gz_error(state, Z_OK, پوچ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    اگر ((صحیح)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        بازگشت -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    اگر (len == 0) {
        اگر (state->err != Z_OK && state->err != Z_BUF_ERROR)
            بازگشت -1;
        اگر (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            بازگشت -1;
        }
    }

    /* return the number of bytes read */
    بازگشت (صحیح)len;
}

/* -- see zlib.h -- */
z_size_t ZEXPORT gzfread(voidp buf, z_size_t size, z_size_t nitems,
                         gzFile file) {
    z_size_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == پوچ)
        بازگشت 0;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ)
        بازگشت 0;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        بازگشت 0;
    اگر (state->again)
        gz_error(state, Z_OK, پوچ);

    /* compute bytes to read -- error on overflow */
    len = nitems * size;
    اگر (size && len / size != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        بازگشت 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    بازگشت len ? gz_read(state, buf, len) / size : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#وگرنه
#  undef gzgetc
#endif
صحیح ZEXPORT gzgetc(gzFile file) {
    unsigned نویسه buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ)
        بازگشت -1;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        بازگشت -1;
    اگر (state->again)
        gz_error(state, Z_OK, پوچ);

    /* try output buffer (no need to check for skip request) */
    اگر (state->x.have) {
        state->x.have--;
        state->x.pos++;
        بازگشت *(state->x.next)++;
    }

    /* nothing there -- try gz_read() */
    بازگشت gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

صحیح ZEXPORT gzgetc_(gzFile file) {
    بازگشت gzgetc(file);
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzungetc(صحیح c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ)
        بازگشت -1;

    /* in case this was just opened, set up the input buffer */
    اگر (state->how == LOOK && state->x.have == 0)
        (تهی)gz_look(state);

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        بازگشت -1;
    اگر (state->again)
        gz_error(state, Z_OK, پوچ);

    /* process a skip request */
    اگر (state->skip && gz_skip(state) == -1)
        بازگشت -1;

    /* can't push EOF */
    اگر (c < 0)
        بازگشت -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    اگر (state->x.have == 0) {
        state->x.have = 1;
        state->x.next = state->out + (state->size << 1) - 1;
        state->x.next[0] = (unsigned نویسه)c;
        state->x.pos--;
        state->past = 0;
        بازگشت c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    اگر (state->x.have == (state->size << 1)) {
        gz_error(state, Z_DATA_ERROR, "out of room to push characters");
        بازگشت -1;
    }

    /* slide output data if needed and insert byte before existing data */
    اگر (state->x.next == state->out) {
        unsigned نویسه *src = state->out + state->x.have;
        unsigned نویسه *dest = state->out + (state->size << 1);

        وقتی (src > state->out)
            *--dest = *--src;
        state->x.next = dest;
    }
    state->x.have++;
    state->x.next--;
    state->x.next[0] = (unsigned نویسه)c;
    state->x.pos--;
    state->past = 0;
    بازگشت c;
}

/* -- see zlib.h -- */
نویسه * ZEXPORT gzgets(gzFile file, نویسه *buf, صحیح len) {
    unsigned چپ, n;
    نویسه *str;
    unsigned نویسه *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    اگر (file == پوچ || buf == پوچ || len < 1)
        بازگشت پوچ;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ)
        بازگشت پوچ;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        بازگشت پوچ;
    اگر (state->again)
        gz_error(state, Z_OK, پوچ);

    /* process a skip request */
    اگر (state->skip && gz_skip(state) == -1)
        بازگشت پوچ;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    چپ = (unsigned)len - 1;
    اگر (چپ) انجامبده {
        /* assure that something is in the output buffer */
        اگر (state->x.have == 0 && gz_fetch(state) == -1)
            بشکن;                      /* error */
        اگر (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            بشکن;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > چپ ? چپ : state->x.have;
        eol = (unsigned نویسه *)memchr(state->x.next, '\n', n);
        اگر (eol != پوچ)
            n = (unsigned)(eol - state->x.next) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.next, n);
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        چپ -= n;
        buf += n;
    } وقتی (چپ && eol == پوچ);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    اگر (buf == str)
        بازگشت پوچ;
    buf[0] = 0;
    بازگشت str;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    اگر (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (تهی)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    بازگشت state->direct == 1;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzclose_r(gzFile file) {
    صحیح ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == پوچ)
        بازگشت Z_STREAM_ERROR;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ)
        بازگشت Z_STREAM_ERROR;

    /* free memory and close file */
    اگر (state->size) {
        inflateEnd(&(state->strm));
        آزاد(state->out);
        آزاد(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, پوچ);
    آزاد(state->path);
    ret = close(state->fd);
    آزاد(state);
    بازگشت ret ? Z_ERRNO : err;
}
