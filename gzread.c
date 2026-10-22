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
local صحیح gz_بھرو(gz_statep state, غیرعلامتی حرف *buf, غیرعلامتی len,
                  غیرعلامتی *have) {
    صحیح ret;
    غیرعلامتی get, max = ((غیرعلامتی)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    کرو {
        get = len - *have;
        اگر (get > max)
            get = max;
        ret = (صحیح)اقرأ(state->fd, buf + *have, get);
        اگر (ret <= 0)
            توڑو;
        *have += (غیرعلامتی)ret;
    } جب (*have < len);
    اگر (ret < 0) {
        اگر (BLOCKED(errno)) {
            state->again = 1;
            اگر (*have != 0)
                واپس 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        واپس -1;
    }
    اگر (ret == 0)
        state->eof = 1;
    واپس 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local صحیح gz_avail(gz_statep state) {
    غیرعلامتی got;
    z_streamp strm = &(state->strm);

    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR)
        واپس -1;
    اگر (state->eof == 0) {
        اگر (strm->avail_اندر) {       /* copy what's there to the start */
            غیرعلامتی حرف *p = state->اندر;
            غیرعلامتی ثابت حرف *q = strm->اگلا_اندر;

            اگر (q != p) {
                غیرعلامتی n = strm->avail_اندر;

                کرو {
                    *p++ = *q++;
                } جب (--n);
            }
        }
        اگر (gz_بھرو(state, state->اندر + strm->avail_اندر,
                    state->سائز - strm->avail_اندر, &got) == -1)
            واپس -1;
        strm->avail_اندر += got;
        strm->اگلا_اندر = state->اندر;
    }
    واپس 0;
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
    اگر (state->سائز == 0) {
        /* allocate buffers */
        state->اندر = (غیرعلامتی حرف *)حافظہ_دو(state->want);
        state->برآمد = (غیرعلامتی حرف *)حافظہ_دو(state->want << 1);
        اگر (state->اندر == تہی || state->برآمد == تہی) {
            آزاد(state->برآمد);
            آزاد(state->اندر);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            واپس -1;
        }
        state->سائز = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_اندر = 0;
        state->strm.اگلا_اندر = Z_NULL;
        اگر (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            آزاد(state->برآمد);
            آزاد(state->اندر);
            state->سائز = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            واپس -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    اگر (state->براہ_راست == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        اگر (state->junk == 0 && strm->avail_اندر == 0) {
            اگر (gz_avail(state) == -1)
                واپس -1;
            اگر (strm->avail_اندر == 0)
                واپس 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->براہ_راست = 0;
        واپس 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    اگر (gz_avail(state) == -1)
        واپس -1;
    اگر (strm->avail_اندر == 0 || (state->again && strm->avail_اندر < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        واپس 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    اگر (strm->avail_اندر > 3 &&
            strm->اگلا_اندر[0] == 31 && strm->اگلا_اندر[1] == 139 &&
            strm->اگلا_اندر[2] == 8 && strm->اگلا_اندر[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->براہ_راست = 0;
        واپس 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.اگلا = state->برآمد;
    حافظہ_نقل(state->x.اگلا, strm->اگلا_اندر, strm->avail_اندر);
    state->x.have = strm->avail_اندر;
    strm->avail_اندر = 0;
    state->how = COPY;
    واپس 0;
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
    غیرعلامتی had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_برآمد;
    کرو {
        /* get more input for inflate() */
        اگر (strm->avail_اندر == 0 && gz_avail(state) == -1) {
            ret = state->err;
            توڑو;
        }
        اگر (strm->avail_اندر == 0) {
            اگر (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            توڑو;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        اگر (strm->avail_برآمد < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        اگر (ret == Z_دھارا_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_دھارا_ERROR,
                     "internal error: inflate stream corrupt");
            توڑو;
        }
        اگر (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            توڑو;
        }
        اگر (ret == Z_ڈیٹا_ERROR) {              /* deflate stream invalid */
            اگر (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_اندر = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                توڑو;
            }
            gz_error(state, Z_ڈیٹا_ERROR,
                     strm->پیغام == تہی ? "compressed data error" : strm->پیغام);
            توڑو;
        }
    } جب (strm->avail_برآمد && ret != Z_دھارا_انتہا);

    /* update available output */
    state->x.have = had - strm->avail_برآمد;
    state->x.اگلا = strm->اگلا_برآمد - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    اگر (ret == Z_دھارا_انتہا) {
        state->junk = 0;
        state->how = LOOK;
        واپس 0;
    }

    /* return decompression status */
    واپس ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local صحیح gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    کرو {
        انتخاب(state->how) {
        حالت LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            اگر (gz_look(state) == -1)
                واپس -1;
            اگر (state->how == LOOK)
                واپس 0;
            توڑو;
        حالت COPY:      /* -> COPY */
            اگر (gz_بھرو(state, state->برآمد, state->سائز << 1, &(state->x.have))
                    == -1)
                واپس -1;
            state->x.اگلا = state->برآمد;
            واپس 0;
        حالت GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_برآمد = state->سائز << 1;
            strm->اگلا_برآمد = state->برآمد;
            اگر (gz_decomp(state) == -1)
                واپس -1;
            توڑو;
        طےشدہ:
            gz_error(state, Z_دھارا_ERROR, "state corrupt");
            واپس -1;
        }
    } جب (state->x.have == 0 && (!state->eof || strm->avail_اندر));
    واپس 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local صحیح gz_skip(gz_statep state) {
    غیرعلامتی n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    کرو {
        /* skip over whatever is in output buffer */
        اگر (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (غیرعلامتی)state->skip : state->x.have;
            state->x.have -= n;
            state->x.اگلا += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ورنہ اگر (state->eof && state->strm.avail_اندر == 0) {
            اگر (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            توڑو;
        }

        /* need more data to skip -- load up output buffer */
        ورنہ {
            /* get more output, looking for header if required */
            اگر (gz_fetch(state) == -1)
                واپس -1;
        }
    } جب (state->skip);
    واپس 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_سائز_t gz_read(gz_statep state, voidp buf, z_سائز_t len) {
    z_سائز_t got;
    غیرعلامتی n;
    صحیح err;

    /* if len is zero, avoid unnecessary operations */
    اگر (len == 0)
        واپس 0;

    /* process a skip request */
    اگر (state->skip && gz_skip(state) == -1)
        واپس 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    کرو {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (غیرعلامتی)-1;
        اگر (n > len)
            n = (غیرعلامتی)len;

        /* first just try copying data from the output buffer */
        اگر (state->x.have) {
            اگر (state->x.have < n)
                n = state->x.have;
            حافظہ_نقل(buf, state->x.اگلا, n);
            state->x.اگلا += n;
            state->x.have -= n;
            اگر (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        ورنہ اگر (state->eof && state->strm.avail_اندر == 0) {
            اگر (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            توڑو;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        ورنہ اگر (state->how == LOOK || n < (state->سائز << 1)) {
            /* get more output, looking for header if required */
            اگر (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            جاری;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        ورنہ اگر (state->how == COPY)        /* read directly */
            err = gz_بھرو(state, (غیرعلامتی حرف *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        ورنہ {  /* state->how == GZIP */
            state->strm.avail_برآمد = n;
            state->strm.اگلا_برآمد = (غیرعلامتی حرف *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (حرف *)buf + n;
        got += n;
        state->x.pos += n;
    } جب (len && !err);

    /* note read past eof */
    اگر (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    واپس got;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzread(gzFile file, voidp buf, غیرعلامتی len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ)
        واپس -1;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        واپس -1;
    اگر (state->again)
        gz_error(state, Z_OK, تہی);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    اگر ((صحیح)len < 0) {
        gz_error(state, Z_دھارا_ERROR, "request does not fit in an int");
        واپس -1;
    }

    /* read len or fewer bytes to buf */
    len = (غیرعلامتی)gz_read(state, buf, len);

    /* check for an error */
    اگر (len == 0) {
        اگر (state->err != Z_OK && state->err != Z_BUF_ERROR)
            واپس -1;
        اگر (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            واپس -1;
        }
    }

    /* return the number of bytes read */
    واپس (صحیح)len;
}

/* -- see zlib.h -- */
z_سائز_t ZEXPORT gzfread(voidp buf, z_سائز_t سائز, z_سائز_t nitems,
                         gzFile file) {
    z_سائز_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == تہی)
        واپس 0;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ)
        واپس 0;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        واپس 0;
    اگر (state->again)
        gz_error(state, Z_OK, تہی);

    /* compute bytes to read -- error on overflow */
    len = nitems * سائز;
    اگر (سائز && len / سائز != nitems) {
        gz_error(state, Z_دھارا_ERROR, "request does not fit in a size_t");
        واپس 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    واپس len ? gz_read(state, buf, len) / سائز : 0;
}

/* -- see zlib.h -- */
#اگر_تعریف_شدہ Z_سابقہ_سیٹ
#  undef z_gzgetc
#ورنہ
#  undef gzgetc
#اگر_ختم
صحیح ZEXPORT gzgetc(gzFile file) {
    غیرعلامتی حرف buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ)
        واپس -1;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        واپس -1;
    اگر (state->again)
        gz_error(state, Z_OK, تہی);

    /* try output buffer (no need to check for skip request) */
    اگر (state->x.have) {
        state->x.have--;
        state->x.pos++;
        واپس *(state->x.اگلا)++;
    }

    /* nothing there -- try gz_read() */
    واپس gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

صحیح ZEXPORT gzgetc_(gzFile file) {
    واپس gzgetc(file);
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzungetc(صحیح c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ)
        واپس -1;

    /* in case this was just opened, set up the input buffer */
    اگر (state->how == LOOK && state->x.have == 0)
        (خالی)gz_look(state);

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        واپس -1;
    اگر (state->again)
        gz_error(state, Z_OK, تہی);

    /* process a skip request */
    اگر (state->skip && gz_skip(state) == -1)
        واپس -1;

    /* can't push EOF */
    اگر (c < 0)
        واپس -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    اگر (state->x.have == 0) {
        state->x.have = 1;
        state->x.اگلا = state->برآمد + (state->سائز << 1) - 1;
        state->x.اگلا[0] = (غیرعلامتی حرف)c;
        state->x.pos--;
        state->past = 0;
        واپس c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    اگر (state->x.have == (state->سائز << 1)) {
        gz_error(state, Z_ڈیٹا_ERROR, "out of room to push characters");
        واپس -1;
    }

    /* slide output data if needed and insert byte before existing data */
    اگر (state->x.اگلا == state->برآمد) {
        غیرعلامتی حرف *src = state->برآمد + state->x.have;
        غیرعلامتی حرف *dest = state->برآمد + (state->سائز << 1);

        جب (src > state->برآمد)
            *--dest = *--src;
        state->x.اگلا = dest;
    }
    state->x.have++;
    state->x.اگلا--;
    state->x.اگلا[0] = (غیرعلامتی حرف)c;
    state->x.pos--;
    state->past = 0;
    واپس c;
}

/* -- see zlib.h -- */
حرف * ZEXPORT gzgets(gzFile file, حرف *buf, صحیح len) {
    غیرعلامتی بایاں, n;
    حرف *str;
    غیرعلامتی حرف *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    اگر (file == تہی || buf == تہی || len < 1)
        واپس تہی;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ)
        واپس تہی;

    /* check that there was no (serious) error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        واپس تہی;
    اگر (state->again)
        gz_error(state, Z_OK, تہی);

    /* process a skip request */
    اگر (state->skip && gz_skip(state) == -1)
        واپس تہی;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    بایاں = (غیرعلامتی)len - 1;
    اگر (بایاں) کرو {
        /* assure that something is in the output buffer */
        اگر (state->x.have == 0 && gz_fetch(state) == -1)
            توڑو;                      /* error */
        اگر (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            توڑو;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > بایاں ? بایاں : state->x.have;
        eol = (غیرعلامتی حرف *)memchr(state->x.اگلا, '\n', n);
        اگر (eol != تہی)
            n = (غیرعلامتی)(eol - state->x.اگلا) + 1;

        /* copy through end-of-line, or remainder if not found */
        حافظہ_نقل(buf, state->x.اگلا, n);
        state->x.have -= n;
        state->x.اگلا += n;
        state->x.pos += n;
        بایاں -= n;
        buf += n;
    } جب (بایاں && eol == تہی);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    اگر (buf == str)
        واپس تہی;
    buf[0] = 0;
    واپس str;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    اگر (file == تہی)
        واپس 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    اگر (state->منوال == GZ_READ && state->how == LOOK && state->x.have == 0)
        (خالی)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    واپس state->براہ_راست == 1;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzclose_r(gzFile file) {
    صحیح ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    اگر (file == تہی)
        واپس Z_دھارا_ERROR;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ)
        واپس Z_دھارا_ERROR;

    /* free memory and close file */
    اگر (state->سائز) {
        inflate_انتہا(&(state->strm));
        آزاد(state->برآمد);
        آزاد(state->اندر);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, تہی);
    آزاد(state->راستہ);
    ret = close(state->fd);
    آزاد(state);
    واپس ret ? Z_ERRNO : err;
}
