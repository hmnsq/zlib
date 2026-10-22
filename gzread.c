/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#অন্তর্ভাব "gzguts.h"

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
local পূর্ণসংখ্যা gz_load(gz_statep state, unsigned অক্ষর *buf, unsigned len,
                  unsigned *have) {
    পূর্ণসংখ্যা ret;
    unsigned get, max = ((unsigned)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    করো {
        get = len - *have;
        যদি (get > max)
            get = max;
        ret = (পূর্ণসংখ্যা)পঠ(state->fd, buf + *have, get);
        যদি (ret <= 0)
            বিরতি;
        *have += (unsigned)ret;
    } যখন (*have < len);
    যদি (ret < 0) {
        যদি (BLOCKED(errno)) {
            state->again = 1;
            যদি (*have != 0)
                ফেরত 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        ফেরত -1;
    }
    যদি (ret == 0)
        state->eof = 1;
    ফেরত 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local পূর্ণসংখ্যা gz_avail(gz_statep state) {
    unsigned got;
    z_streamp strm = &(state->strm);

    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ফেরত -1;
    যদি (state->eof == 0) {
        যদি (strm->avail_in) {       /* copy what's there to the start */
            unsigned অক্ষর *p = state->in;
            unsigned ধ্রুবক অক্ষর *q = strm->পরবর্তী_in;

            যদি (q != p) {
                unsigned n = strm->avail_in;

                করো {
                    *p++ = *q++;
                } যখন (--n);
            }
        }
        যদি (gz_load(state, state->in + strm->avail_in,
                    state->আকার_2 - strm->avail_in, &got) == -1)
            ফেরত -1;
        strm->avail_in += got;
        strm->পরবর্তী_in = state->in;
    }
    ফেরত 0;
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
local পূর্ণসংখ্যা gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    যদি (state->আকার_2 == 0) {
        /* allocate buffers */
        state->in = (unsigned অক্ষর *)malloc(state->want);
        state->out = (unsigned অক্ষর *)malloc(state->want << 1);
        যদি (state->in == ফাঁকা || state->out == ফাঁকা) {
            মুক্ত(state->out);
            মুক্ত(state->in);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ফেরত -1;
        }
        state->আকার_2 = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_in = 0;
        state->strm.পরবর্তী_in = Z_NULL;
        যদি (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            মুক্ত(state->out);
            মুক্ত(state->in);
            state->আকার_2 = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ফেরত -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    যদি (state->direct == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        যদি (state->junk == 0 && strm->avail_in == 0) {
            যদি (gz_avail(state) == -1)
                ফেরত -1;
            যদি (strm->avail_in == 0)
                ফেরত 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->direct = 0;
        ফেরত 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    যদি (gz_avail(state) == -1)
        ফেরত -1;
    যদি (strm->avail_in == 0 || (state->again && strm->avail_in < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        ফেরত 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    যদি (strm->avail_in > 3 &&
            strm->পরবর্তী_in[0] == 31 && strm->পরবর্তী_in[1] == 139 &&
            strm->পরবর্তী_in[2] == 8 && strm->পরবর্তী_in[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->direct = 0;
        ফেরত 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.পরবর্তী = state->out;
    memcpy(state->x.পরবর্তী, strm->পরবর্তী_in, strm->avail_in);
    state->x.have = strm->avail_in;
    strm->avail_in = 0;
    state->how = COPY;
    ফেরত 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local পূর্ণসংখ্যা gz_decomp(gz_statep state) {
    পূর্ণসংখ্যা ret = Z_OK;
    unsigned had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_out;
    করো {
        /* get more input for inflate() */
        যদি (strm->avail_in == 0 && gz_avail(state) == -1) {
            ret = state->err;
            বিরতি;
        }
        যদি (strm->avail_in == 0) {
            যদি (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            বিরতি;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        যদি (strm->avail_out < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        যদি (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_STREAM_ERROR,
                     "internal error: inflate stream corrupt");
            বিরতি;
        }
        যদি (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            বিরতি;
        }
        যদি (ret == Z_তথ্য_ERROR) {              /* deflate stream invalid */
            যদি (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_in = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                বিরতি;
            }
            gz_error(state, Z_তথ্য_ERROR,
                     strm->বার্তা == ফাঁকা ? "compressed data error" : strm->বার্তা);
            বিরতি;
        }
    } যখন (strm->avail_out && ret != Z_STREAM_END);

    /* update available output */
    state->x.have = had - strm->avail_out;
    state->x.পরবর্তী = strm->পরবর্তী_out - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    যদি (ret == Z_STREAM_END) {
        state->junk = 0;
        state->how = LOOK;
        ফেরত 0;
    }

    /* return decompression status */
    ফেরত ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local পূর্ণসংখ্যা gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    করো {
        নির্বাচন(state->how) {
        ক্ষেত্র LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            যদি (gz_look(state) == -1)
                ফেরত -1;
            যদি (state->how == LOOK)
                ফেরত 0;
            বিরতি;
        ক্ষেত্র COPY:      /* -> COPY */
            যদি (gz_load(state, state->out, state->আকার_2 << 1, &(state->x.have))
                    == -1)
                ফেরত -1;
            state->x.পরবর্তী = state->out;
            ফেরত 0;
        ক্ষেত্র GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_out = state->আকার_2 << 1;
            strm->পরবর্তী_out = state->out;
            যদি (gz_decomp(state) == -1)
                ফেরত -1;
            বিরতি;
        পূর্বনির্ধারিত:
            gz_error(state, Z_STREAM_ERROR, "state corrupt");
            ফেরত -1;
        }
    } যখন (state->x.have == 0 && (!state->eof || strm->avail_in));
    ফেরত 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local পূর্ণসংখ্যা gz_skip(gz_statep state) {
    unsigned n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    করো {
        /* skip over whatever is in output buffer */
        যদি (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (unsigned)state->skip : state->x.have;
            state->x.have -= n;
            state->x.পরবর্তী += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        অন্যথা যদি (state->eof && state->strm.avail_in == 0) {
            যদি (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            বিরতি;
        }

        /* need more data to skip -- load up output buffer */
        অন্যথা {
            /* get more output, looking for header if required */
            যদি (gz_fetch(state) == -1)
                ফেরত -1;
        }
    } যখন (state->skip);
    ফেরত 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_আকার_t gz_read(gz_statep state, voidp buf, z_আকার_t len) {
    z_আকার_t got;
    unsigned n;
    পূর্ণসংখ্যা err;

    /* if len is zero, avoid unnecessary operations */
    যদি (len == 0)
        ফেরত 0;

    /* process a skip request */
    যদি (state->skip && gz_skip(state) == -1)
        ফেরত 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    করো {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (unsigned)-1;
        যদি (n > len)
            n = (unsigned)len;

        /* first just try copying data from the output buffer */
        যদি (state->x.have) {
            যদি (state->x.have < n)
                n = state->x.have;
            memcpy(buf, state->x.পরবর্তী, n);
            state->x.পরবর্তী += n;
            state->x.have -= n;
            যদি (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        অন্যথা যদি (state->eof && state->strm.avail_in == 0) {
            যদি (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            বিরতি;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        অন্যথা যদি (state->how == LOOK || n < (state->আকার_2 << 1)) {
            /* get more output, looking for header if required */
            যদি (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            চালু;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        অন্যথা যদি (state->how == COPY)        /* read directly */
            err = gz_load(state, (unsigned অক্ষর *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        অন্যথা {  /* state->how == GZIP */
            state->strm.avail_out = n;
            state->strm.পরবর্তী_out = (unsigned অক্ষর *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (অক্ষর *)buf + n;
        got += n;
        state->x.pos += n;
    } যখন (len && !err);

    /* note read past eof */
    যদি (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    ফেরত got;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzread(gzFile file, voidp buf, unsigned len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ)
        ফেরত -1;

    /* check that there was no (serious) error */
    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ফেরত -1;
    যদি (state->again)
        gz_error(state, Z_OK, ফাঁকা);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    যদি ((পূর্ণসংখ্যা)len < 0) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in an int");
        ফেরত -1;
    }

    /* read len or fewer bytes to buf */
    len = (unsigned)gz_read(state, buf, len);

    /* check for an error */
    যদি (len == 0) {
        যদি (state->err != Z_OK && state->err != Z_BUF_ERROR)
            ফেরত -1;
        যদি (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            ফেরত -1;
        }
    }

    /* return the number of bytes read */
    ফেরত (পূর্ণসংখ্যা)len;
}

/* -- see zlib.h -- */
z_আকার_t ZEXPORT gzfread(voidp buf, z_আকার_t আকার_2, z_আকার_t nitems,
                         gzFile file) {
    z_আকার_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    যদি (file == ফাঁকা)
        ফেরত 0;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ)
        ফেরত 0;

    /* check that there was no (serious) error */
    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ফেরত 0;
    যদি (state->again)
        gz_error(state, Z_OK, ফাঁকা);

    /* compute bytes to read -- error on overflow */
    len = nitems * আকার_2;
    যদি (আকার_2 && len / আকার_2 != nitems) {
        gz_error(state, Z_STREAM_ERROR, "request does not fit in a size_t");
        ফেরত 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    ফেরত len ? gz_read(state, buf, len) / আকার_2 : 0;
}

/* -- see zlib.h -- */
#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#অন্যথা
#  undef gzgetc
#endif
পূর্ণসংখ্যা ZEXPORT gzgetc(gzFile file) {
    unsigned অক্ষর buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ)
        ফেরত -1;

    /* check that there was no (serious) error */
    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ফেরত -1;
    যদি (state->again)
        gz_error(state, Z_OK, ফাঁকা);

    /* try output buffer (no need to check for skip request) */
    যদি (state->x.have) {
        state->x.have--;
        state->x.pos++;
        ফেরত *(state->x.পরবর্তী)++;
    }

    /* nothing there -- try gz_read() */
    ফেরত gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

পূর্ণসংখ্যা ZEXPORT gzgetc_(gzFile file) {
    ফেরত gzgetc(file);
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzungetc(পূর্ণসংখ্যা c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ)
        ফেরত -1;

    /* in case this was just opened, set up the input buffer */
    যদি (state->how == LOOK && state->x.have == 0)
        (শূন্য)gz_look(state);

    /* check that there was no (serious) error */
    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ফেরত -1;
    যদি (state->again)
        gz_error(state, Z_OK, ফাঁকা);

    /* process a skip request */
    যদি (state->skip && gz_skip(state) == -1)
        ফেরত -1;

    /* can't push EOF */
    যদি (c < 0)
        ফেরত -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    যদি (state->x.have == 0) {
        state->x.have = 1;
        state->x.পরবর্তী = state->out + (state->আকার_2 << 1) - 1;
        state->x.পরবর্তী[0] = (unsigned অক্ষর)c;
        state->x.pos--;
        state->past = 0;
        ফেরত c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    যদি (state->x.have == (state->আকার_2 << 1)) {
        gz_error(state, Z_তথ্য_ERROR, "out of room to push characters");
        ফেরত -1;
    }

    /* slide output data if needed and insert byte before existing data */
    যদি (state->x.পরবর্তী == state->out) {
        unsigned অক্ষর *src = state->out + state->x.have;
        unsigned অক্ষর *dest = state->out + (state->আকার_2 << 1);

        যখন (src > state->out)
            *--dest = *--src;
        state->x.পরবর্তী = dest;
    }
    state->x.have++;
    state->x.পরবর্তী--;
    state->x.পরবর্তী[0] = (unsigned অক্ষর)c;
    state->x.pos--;
    state->past = 0;
    ফেরত c;
}

/* -- see zlib.h -- */
অক্ষর * ZEXPORT gzgets(gzFile file, অক্ষর *buf, পূর্ণসংখ্যা len) {
    unsigned বাম, n;
    অক্ষর *str;
    unsigned অক্ষর *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    যদি (file == ফাঁকা || buf == ফাঁকা || len < 1)
        ফেরত ফাঁকা;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ)
        ফেরত ফাঁকা;

    /* check that there was no (serious) error */
    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        ফেরত ফাঁকা;
    যদি (state->again)
        gz_error(state, Z_OK, ফাঁকা);

    /* process a skip request */
    যদি (state->skip && gz_skip(state) == -1)
        ফেরত ফাঁকা;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    বাম = (unsigned)len - 1;
    যদি (বাম) করো {
        /* assure that something is in the output buffer */
        যদি (state->x.have == 0 && gz_fetch(state) == -1)
            বিরতি;                      /* error */
        যদি (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            বিরতি;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > বাম ? বাম : state->x.have;
        eol = (unsigned অক্ষর *)memchr(state->x.পরবর্তী, '\n', n);
        যদি (eol != ফাঁকা)
            n = (unsigned)(eol - state->x.পরবর্তী) + 1;

        /* copy through end-of-line, or remainder if not found */
        memcpy(buf, state->x.পরবর্তী, n);
        state->x.have -= n;
        state->x.পরবর্তী += n;
        state->x.pos += n;
        বাম -= n;
        buf += n;
    } যখন (বাম && eol == ফাঁকা);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    যদি (buf == str)
        ফেরত ফাঁকা;
    buf[0] = 0;
    ফেরত str;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    যদি (state->mode == GZ_READ && state->how == LOOK && state->x.have == 0)
        (শূন্য)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    ফেরত state->direct == 1;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzclose_r(gzFile file) {
    পূর্ণসংখ্যা ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    যদি (file == ফাঁকা)
        ফেরত Z_STREAM_ERROR;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ)
        ফেরত Z_STREAM_ERROR;

    /* free memory and close file */
    যদি (state->আকার_2) {
        inflateEnd(&(state->strm));
        মুক্ত(state->out);
        মুক্ত(state->in);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, ফাঁকা);
    মুক্ত(state->path);
    ret = close(state->fd);
    মুক্ত(state);
    ফেরত ret ? Z_ERRNO : err;
}
