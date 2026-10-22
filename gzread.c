/* gzread.c -- zlib functions for reading gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#అన్తర్భావ "gzguts.h"

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
local పూర్ణాంకం gz_ఎక్కించు(gz_statep state, గుర్తులేని అక్షరం *buf, గుర్తులేని len,
                  గుర్తులేని *have) {
    పూర్ణాంకం ret;
    గుర్తులేని get, max = ((గుర్తులేని)-1 >> 2) + 1;

    state->again = 0;
    errno = 0;
    *have = 0;
    చేయి {
        get = len - *have;
        ఒకవేళ (get > max)
            get = max;
        ret = (పూర్ణాంకం)పఠ(state->fd, buf + *have, get);
        ఒకవేళ (ret <= 0)
            ఆపు;
        *have += (గుర్తులేని)ret;
    } అయితే (*have < len);
    ఒకవేళ (ret < 0) {
        ఒకవేళ (BLOCKED(errno)) {
            state->again = 1;
            ఒకవేళ (*have != 0)
                తిరిగి 0;
        }
        gz_error(state, Z_ERRNO, zstrerror());
        తిరిగి -1;
    }
    ఒకవేళ (ret == 0)
        state->eof = 1;
    తిరిగి 0;
}

/* Load up input buffer and set eof flag if last data loaded -- return -1 on
   error, 0 otherwise.  Note that the eof flag is set when the end of the input
   file is reached, even though there may be unused data in the buffer.  Once
   that data has been used, no more attempts will be made to read the file.
   If strm->avail_in != 0, then the current data is moved to the beginning of
   the input buffer, and then the remainder of the buffer is loaded with the
   available data from the input file. */
local పూర్ణాంకం gz_avail(gz_statep state) {
    గుర్తులేని got;
    z_streamp strm = &(state->strm);

    ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        తిరిగి -1;
    ఒకవేళ (state->eof == 0) {
        ఒకవేళ (strm->avail_లోపల) {       /* copy what's there to the start */
            గుర్తులేని అక్షరం *p = state->లోపల;
            గుర్తులేని స్థిరరాశి అక్షరం *q = strm->తదుపరి_లోపల;

            ఒకవేళ (q != p) {
                గుర్తులేని n = strm->avail_లోపల;

                చేయి {
                    *p++ = *q++;
                } అయితే (--n);
            }
        }
        ఒకవేళ (gz_ఎక్కించు(state, state->లోపల + strm->avail_లోపల,
                    state->కొలత - strm->avail_లోపల, &got) == -1)
            తిరిగి -1;
        strm->avail_లోపల += got;
        strm->తదుపరి_లోపల = state->లోపల;
    }
    తిరిగి 0;
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
local పూర్ణాంకం gz_look(gz_statep state) {
    z_streamp strm = &(state->strm);

    /* allocate read buffers and inflate memory */
    ఒకవేళ (state->కొలత == 0) {
        /* allocate buffers */
        state->లోపల = (గుర్తులేని అక్షరం *)జ్ఞాపకం_ఇవ్వు(state->want);
        state->బహిర్గతం = (గుర్తులేని అక్షరం *)జ్ఞాపకం_ఇవ్వు(state->want << 1);
        ఒకవేళ (state->లోపల == ఖాళీ || state->బహిర్గతం == ఖాళీ) {
            విడుదల(state->బహిర్గతం);
            విడుదల(state->లోపల);
            gz_error(state, Z_MEM_ERROR, "out of memory");
            తిరిగి -1;
        }
        state->కొలత = state->want;

        /* allocate inflate memory */
        state->strm.zalloc = Z_NULL;
        state->strm.zfree = Z_NULL;
        state->strm.opaque = Z_NULL;
        state->strm.avail_లోపల = 0;
        state->strm.తదుపరి_లోపల = Z_NULL;
        ఒకవేళ (inflateInit2(&(state->strm), 15 + 16) != Z_OK) {    /* gunzip */
            విడుదల(state->బహిర్గతం);
            విడుదల(state->లోపల);
            state->కొలత = 0;
            gz_error(state, Z_MEM_ERROR, "out of memory");
            తిరిగి -1;
        }
    }

    /* if transparent reading is disabled, which would only be at the start, or
       if we're looking for a gzip member after the first one, which is not at
       the start, then proceed directly to look for a gzip member next */
    ఒకవేళ (state->ప్రత్యక్ష == -1 || state->junk == 0) {
        /* wait for input before committing to another gzip member */
        ఒకవేళ (state->junk == 0 && strm->avail_లోపల == 0) {
            ఒకవేళ (gz_avail(state) == -1)
                తిరిగి -1;
            ఒకవేళ (strm->avail_లోపల == 0)
                తిరిగి 0;
        }
        inflateReset(strm);
        state->how = GZIP;
        state->junk = state->junk != -1;
        state->ప్రత్యక్ష = 0;
        తిరిగి 0;
    }

    /* otherwise we're at the start with auto-detect -- we check to see if the
       first four bytes could be gzip header in order to decide whether or not
       this will be a transparent read */

    /* load any header bytes into the input buffer -- if the input is empty,
       then it's not an error as this is a transparent read of zero bytes */
    ఒకవేళ (gz_avail(state) == -1)
        తిరిగి -1;
    ఒకవేళ (strm->avail_లోపల == 0 || (state->again && strm->avail_లోపల < 4))
        /* if non-blocking input stalled before getting four bytes, then
           return and wait until a later call has accumulated enough */
        తిరిగి 0;

    /* see if this is (likely) gzip input -- if the first four bytes are
       consistent with a gzip header, then go look for the first gzip member,
       otherwise proceed to copy the input transparently */
    ఒకవేళ (strm->avail_లోపల > 3 &&
            strm->తదుపరి_లోపల[0] == 31 && strm->తదుపరి_లోపల[1] == 139 &&
            strm->తదుపరి_లోపల[2] == 8 && strm->తదుపరి_లోపల[3] < 32) {
        inflateReset(strm);
        state->how = GZIP;
        state->junk = 1;
        state->ప్రత్యక్ష = 0;
        తిరిగి 0;
    }

    /* doing raw i/o: copy any leftover input to output -- this assumes that
       the output buffer is larger than the input buffer, which also assures
       space for gzungetc() */
    state->x.తదుపరి = state->బహిర్గతం;
    జ్ఞాపకం_నకలు(state->x.తదుపరి, strm->తదుపరి_లోపల, strm->avail_లోపల);
    state->x.have = strm->avail_లోపల;
    strm->avail_లోపల = 0;
    state->how = COPY;
    తిరిగి 0;
}

/* Decompress from input to the provided next_out and avail_out in the state.
   On return, state->x.have and state->x.next point to the just decompressed
   data. If the gzip stream completes, state->how is reset to LOOK to look for
   the next gzip stream or raw data, once state->x.have is depleted. Returns 0
   on success, -1 on failure. If EOF is reached when looking for more input to
   complete the gzip member, then an unexpected end of file error is raised.
   If there is no more input, but state->again is true, then EOF has not been
   reached, and no error is raised. */
local పూర్ణాంకం gz_decomp(gz_statep state) {
    పూర్ణాంకం ret = Z_OK;
    గుర్తులేని had;
    z_streamp strm = &(state->strm);

    /* fill output buffer up to end of deflate stream */
    had = strm->avail_బహిర్గతం;
    చేయి {
        /* get more input for inflate() */
        ఒకవేళ (strm->avail_లోపల == 0 && gz_avail(state) == -1) {
            ret = state->err;
            ఆపు;
        }
        ఒకవేళ (strm->avail_లోపల == 0) {
            ఒకవేళ (!state->again)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ఆపు;
        }

        /* decompress and handle errors */
        ret = inflate(strm, Z_NO_FLUSH);
        ఒకవేళ (strm->avail_బహిర్గతం < had)
            /* any decompressed data marks this as a real gzip stream */
            state->junk = 0;
        ఒకవేళ (ret == Z_ప్రవాహం_ERROR || ret == Z_NEED_DICT) {
            gz_error(state, Z_ప్రవాహం_ERROR,
                     "internal error: inflate stream corrupt");
            ఆపు;
        }
        ఒకవేళ (ret == Z_MEM_ERROR) {
            gz_error(state, Z_MEM_ERROR, "out of memory");
            ఆపు;
        }
        ఒకవేళ (ret == Z_దత్తాంశం_ERROR) {              /* deflate stream invalid */
            ఒకవేళ (state->junk == 1) {             /* trailing garbage is ok */
                strm->avail_లోపల = 0;
                state->eof = 1;
                state->how = LOOK;
                ret = Z_OK;
                ఆపు;
            }
            gz_error(state, Z_దత్తాంశం_ERROR,
                     strm->సందేశం == ఖాళీ ? "compressed data error" : strm->సందేశం);
            ఆపు;
        }
    } అయితే (strm->avail_బహిర్గతం && ret != Z_ప్రవాహం_చివర);

    /* update available output */
    state->x.have = had - strm->avail_బహిర్గతం;
    state->x.తదుపరి = strm->తదుపరి_బహిర్గతం - state->x.have;

    /* if the gzip stream completed successfully, look for another */
    ఒకవేళ (ret == Z_ప్రవాహం_చివర) {
        state->junk = 0;
        state->how = LOOK;
        తిరిగి 0;
    }

    /* return decompression status */
    తిరిగి ret != Z_OK ? -1 : 0;
}

/* Fetch data and put it in the output buffer.  Assumes state->x.have is 0.
   Data is either copied from the input file or decompressed from the input
   file depending on state->how.  If state->how is LOOK, then a gzip header is
   looked for to determine whether to copy or decompress.  Returns -1 on error,
   otherwise 0.  gz_fetch() will leave state->how as COPY or GZIP unless the
   end of the input file has been reached and all data has been processed.  */
local పూర్ణాంకం gz_fetch(gz_statep state) {
    z_streamp strm = &(state->strm);

    చేయి {
        ఎంపిక(state->how) {
        స్థితి LOOK:      /* -> LOOK, COPY (only if never GZIP), or GZIP */
            ఒకవేళ (gz_look(state) == -1)
                తిరిగి -1;
            ఒకవేళ (state->how == LOOK)
                తిరిగి 0;
            ఆపు;
        స్థితి COPY:      /* -> COPY */
            ఒకవేళ (gz_ఎక్కించు(state, state->బహిర్గతం, state->కొలత << 1, &(state->x.have))
                    == -1)
                తిరిగి -1;
            state->x.తదుపరి = state->బహిర్గతం;
            తిరిగి 0;
        స్థితి GZIP:      /* -> GZIP or LOOK (if end of gzip stream) */
            strm->avail_బహిర్గతం = state->కొలత << 1;
            strm->తదుపరి_బహిర్గతం = state->బహిర్గతం;
            ఒకవేళ (gz_decomp(state) == -1)
                తిరిగి -1;
            ఆపు;
        అప్రమేయ:
            gz_error(state, Z_ప్రవాహం_ERROR, "state corrupt");
            తిరిగి -1;
        }
    } అయితే (state->x.have == 0 && (!state->eof || strm->avail_లోపల));
    తిరిగి 0;
}

/* Skip state->skip (> 0) uncompressed bytes of output.  Return -1 on error, 0
   on success. */
local పూర్ణాంకం gz_skip(gz_statep state) {
    గుర్తులేని n;

    /* skip over len bytes or reach end-of-file, whichever comes first */
    చేయి {
        /* skip over whatever is in output buffer */
        ఒకవేళ (state->x.have) {
            n = GT_OFF(state->x.have) ||
                (z_off64_t)state->x.have > state->skip ?
                (గుర్తులేని)state->skip : state->x.have;
            state->x.have -= n;
            state->x.తదుపరి += n;
            state->x.pos += n;
            state->skip -= n;
        }

        /* output buffer empty -- return if we're at the end of the input */
        లేకపోతే ఒకవేళ (state->eof && state->strm.avail_లోపల == 0) {
            ఒకవేళ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ఆపు;
        }

        /* need more data to skip -- load up output buffer */
        లేకపోతే {
            /* get more output, looking for header if required */
            ఒకవేళ (gz_fetch(state) == -1)
                తిరిగి -1;
        }
    } అయితే (state->skip);
    తిరిగి 0;
}

/* Read len bytes into buf from file, or less than len up to the end of the
   input. Return the number of bytes read. If zero is returned, either the end
   of file was reached, or there was an error. state->err must be consulted in
   that case to determine which. If there was an error, but some uncompressed
   bytes were read before the error, then that count is returned. The error is
   still recorded, and so is deferred until the next call. */
local z_కొలత_t gz_read(gz_statep state, voidp buf, z_కొలత_t len) {
    z_కొలత_t got;
    గుర్తులేని n;
    పూర్ణాంకం err;

    /* if len is zero, avoid unnecessary operations */
    ఒకవేళ (len == 0)
        తిరిగి 0;

    /* process a skip request */
    ఒకవేళ (state->skip && gz_skip(state) == -1)
        తిరిగి 0;

    /* get len bytes to buf, or less than len if at the end */
    got = 0;
    err = 0;
    చేయి {
        /* set n to the maximum amount of len that fits in an unsigned int */
        n = (గుర్తులేని)-1;
        ఒకవేళ (n > len)
            n = (గుర్తులేని)len;

        /* first just try copying data from the output buffer */
        ఒకవేళ (state->x.have) {
            ఒకవేళ (state->x.have < n)
                n = state->x.have;
            జ్ఞాపకం_నకలు(buf, state->x.తదుపరి, n);
            state->x.తదుపరి += n;
            state->x.have -= n;
            ఒకవేళ (state->err != Z_OK)
                /* caught deferred error from gz_fetch() */
                err = -1;
        }

        /* output buffer empty -- return if we're at the end of the input */
        లేకపోతే ఒకవేళ (state->eof && state->strm.avail_లోపల == 0) {
            ఒకవేళ (state->how == GZIP && state->err == Z_OK)
                gz_error(state, Z_BUF_ERROR, "unexpected end of file");
            ఆపు;
        }

        /* need output data -- for small len or new stream load up our output
           buffer, so that gzgetc() can be fast */
        లేకపోతే ఒకవేళ (state->how == LOOK || n < (state->కొలత << 1)) {
            /* get more output, looking for header if required */
            ఒకవేళ (gz_fetch(state) == -1 && state->x.have == 0)
                /* if state->x.have != 0, error will be caught after copy */
                err = -1;
            కొనసాగు;       /* no progress yet -- go back to copy above */
            /* the copy above assures that we will leave with space in the
               output buffer, allowing at least one gzungetc() to succeed */
        }

        /* large len -- read directly into user buffer */
        లేకపోతే ఒకవేళ (state->how == COPY)        /* read directly */
            err = gz_ఎక్కించు(state, (గుర్తులేని అక్షరం *)buf, n, &n);

        /* large len -- decompress directly into user buffer */
        లేకపోతే {  /* state->how == GZIP */
            state->strm.avail_బహిర్గతం = n;
            state->strm.తదుపరి_బహిర్గతం = (గుర్తులేని అక్షరం *)buf;
            err = gz_decomp(state);
            n = state->x.have;
            state->x.have = 0;
        }

        /* update progress */
        len -= n;
        buf = (అక్షరం *)buf + n;
        got += n;
        state->x.pos += n;
    } అయితే (len && !err);

    /* note read past eof */
    ఒకవేళ (len && state->eof)
        state->past = 1;

    /* return number of bytes read into user buffer */
    తిరిగి got;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzread(gzFile file, voidp buf, గుర్తులేని len) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ)
        తిరిగి -1;

    /* check that there was no (serious) error */
    ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        తిరిగి -1;
    ఒకవేళ (state->again)
        gz_error(state, Z_OK, ఖాళీ);

    /* since an int is returned, make sure len fits in one, otherwise return
       with an error (this avoids a flaw in the interface) */
    ఒకవేళ ((పూర్ణాంకం)len < 0) {
        gz_error(state, Z_ప్రవాహం_ERROR, "request does not fit in an int");
        తిరిగి -1;
    }

    /* read len or fewer bytes to buf */
    len = (గుర్తులేని)gz_read(state, buf, len);

    /* check for an error */
    ఒకవేళ (len == 0) {
        ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR)
            తిరిగి -1;
        ఒకవేళ (state->again) {
            /* non-blocking input stalled after some input was read, but no
               uncompressed bytes were produced -- let the application know
               this isn't EOF */
            gz_error(state, Z_ERRNO, zstrerror());
            తిరిగి -1;
        }
    }

    /* return the number of bytes read */
    తిరిగి (పూర్ణాంకం)len;
}

/* -- see zlib.h -- */
z_కొలత_t ZEXPORT gzfread(voidp buf, z_కొలత_t కొలత, z_కొలత_t nitems,
                         gzFile file) {
    z_కొలత_t len;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి 0;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ)
        తిరిగి 0;

    /* check that there was no (serious) error */
    ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        తిరిగి 0;
    ఒకవేళ (state->again)
        gz_error(state, Z_OK, ఖాళీ);

    /* compute bytes to read -- error on overflow */
    len = nitems * కొలత;
    ఒకవేళ (కొలత && len / కొలత != nitems) {
        gz_error(state, Z_ప్రవాహం_ERROR, "request does not fit in a size_t");
        తిరిగి 0;
    }

    /* read len or fewer bytes to buf, return the number of full items read */
    తిరిగి len ? gz_read(state, buf, len) / కొలత : 0;
}

/* -- see zlib.h -- */
#నిర్వచితమైతే Z_ఉపసర్గ_సమితి
#  undef z_gzgetc
#లేకపోతే
#  undef gzgetc
#అయితే_ముగింపు
పూర్ణాంకం ZEXPORT gzgetc(gzFile file) {
    గుర్తులేని అక్షరం buf[1];
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ)
        తిరిగి -1;

    /* check that there was no (serious) error */
    ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        తిరిగి -1;
    ఒకవేళ (state->again)
        gz_error(state, Z_OK, ఖాళీ);

    /* try output buffer (no need to check for skip request) */
    ఒకవేళ (state->x.have) {
        state->x.have--;
        state->x.pos++;
        తిరిగి *(state->x.తదుపరి)++;
    }

    /* nothing there -- try gz_read() */
    తిరిగి gz_read(state, buf, 1) < 1 ? -1 : buf[0];
}

పూర్ణాంకం ZEXPORT gzgetc_(gzFile file) {
    తిరిగి gzgetc(file);
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzungetc(పూర్ణాంకం c, gzFile file) {
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ)
        తిరిగి -1;

    /* in case this was just opened, set up the input buffer */
    ఒకవేళ (state->how == LOOK && state->x.have == 0)
        (శూన్య)gz_look(state);

    /* check that there was no (serious) error */
    ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        తిరిగి -1;
    ఒకవేళ (state->again)
        gz_error(state, Z_OK, ఖాళీ);

    /* process a skip request */
    ఒకవేళ (state->skip && gz_skip(state) == -1)
        తిరిగి -1;

    /* can't push EOF */
    ఒకవేళ (c < 0)
        తిరిగి -1;

    /* if output buffer empty, put byte at end (allows more pushing) */
    ఒకవేళ (state->x.have == 0) {
        state->x.have = 1;
        state->x.తదుపరి = state->బహిర్గతం + (state->కొలత << 1) - 1;
        state->x.తదుపరి[0] = (గుర్తులేని అక్షరం)c;
        state->x.pos--;
        state->past = 0;
        తిరిగి c;
    }

    /* if no room, give up (must have already done a gzungetc()) */
    ఒకవేళ (state->x.have == (state->కొలత << 1)) {
        gz_error(state, Z_దత్తాంశం_ERROR, "out of room to push characters");
        తిరిగి -1;
    }

    /* slide output data if needed and insert byte before existing data */
    ఒకవేళ (state->x.తదుపరి == state->బహిర్గతం) {
        గుర్తులేని అక్షరం *src = state->బహిర్గతం + state->x.have;
        గుర్తులేని అక్షరం *dest = state->బహిర్గతం + (state->కొలత << 1);

        అయితే (src > state->బహిర్గతం)
            *--dest = *--src;
        state->x.తదుపరి = dest;
    }
    state->x.have++;
    state->x.తదుపరి--;
    state->x.తదుపరి[0] = (గుర్తులేని అక్షరం)c;
    state->x.pos--;
    state->past = 0;
    తిరిగి c;
}

/* -- see zlib.h -- */
అక్షరం * ZEXPORT gzgets(gzFile file, అక్షరం *buf, పూర్ణాంకం len) {
    గుర్తులేని ఎడమ, n;
    అక్షరం *str;
    గుర్తులేని అక్షరం *eol;
    gz_statep state;

    /* check parameters, get internal structure, and check that it's for
       reading */
    ఒకవేళ (file == ఖాళీ || buf == ఖాళీ || len < 1)
        తిరిగి ఖాళీ;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ)
        తిరిగి ఖాళీ;

    /* check that there was no (serious) error */
    ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR && !state->again)
        తిరిగి ఖాళీ;
    ఒకవేళ (state->again)
        gz_error(state, Z_OK, ఖాళీ);

    /* process a skip request */
    ఒకవేళ (state->skip && gz_skip(state) == -1)
        తిరిగి ఖాళీ;

    /* copy output up to a new line, len-1 bytes, or there is no more output,
       whichever comes first */
    str = buf;
    ఎడమ = (గుర్తులేని)len - 1;
    ఒకవేళ (ఎడమ) చేయి {
        /* assure that something is in the output buffer */
        ఒకవేళ (state->x.have == 0 && gz_fetch(state) == -1)
            ఆపు;                      /* error */
        ఒకవేళ (state->x.have == 0) {       /* end of file */
            state->past = 1;            /* read past end */
            ఆపు;                      /* return what we have */
        }

        /* look for end-of-line in current output buffer */
        n = state->x.have > ఎడమ ? ఎడమ : state->x.have;
        eol = (గుర్తులేని అక్షరం *)memchr(state->x.తదుపరి, '\n', n);
        ఒకవేళ (eol != ఖాళీ)
            n = (గుర్తులేని)(eol - state->x.తదుపరి) + 1;

        /* copy through end-of-line, or remainder if not found */
        జ్ఞాపకం_నకలు(buf, state->x.తదుపరి, n);
        state->x.have -= n;
        state->x.తదుపరి += n;
        state->x.pos += n;
        ఎడమ -= n;
        buf += n;
    } అయితే (ఎడమ && eol == ఖాళీ);

    /* append a terminating zero to the string (we don't check for a zero in
       the contents, let the user worry about that) -- return the terminated
       string, or if nothing was read, NULL */
    ఒకవేళ (buf == str)
        తిరిగి ఖాళీ;
    buf[0] = 0;
    తిరిగి str;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzdirect(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి 0;
    state = (gz_statep)file;

    /* if the state is not known, but we can find out, then do so (this is
       mainly for right after a gzopen() or gzdopen()) */
    ఒకవేళ (state->బహుళకం == GZ_READ && state->how == LOOK && state->x.have == 0)
        (శూన్య)gz_look(state);

    /* return 1 if transparent, 0 if processing a gzip stream */
    తిరిగి state->ప్రత్యక్ష == 1;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzclose_r(gzFile file) {
    పూర్ణాంకం ret, err;
    gz_statep state;

    /* get internal structure and check that it's for reading */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి Z_ప్రవాహం_ERROR;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ)
        తిరిగి Z_ప్రవాహం_ERROR;

    /* free memory and close file */
    ఒకవేళ (state->కొలత) {
        inflate_చివర(&(state->strm));
        విడుదల(state->బహిర్గతం);
        విడుదల(state->లోపల);
    }
    err = state->err == Z_BUF_ERROR ? Z_BUF_ERROR : Z_OK;
    gz_error(state, Z_OK, ఖాళీ);
    విడుదల(state->మార్గం);
    ret = close(state->fd);
    విడుదల(state);
    తిరిగి ret ? Z_ERRNO : err;
}
