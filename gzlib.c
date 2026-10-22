/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ଅନ୍ତର୍ଭାଵ "gzguts.h"

#ଯଦି defined(__DJGPP__)
#  define LSEEK llseek
#ନଚେତଯଦି defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#ନଚେତଯଦି defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#ଅନ୍ୟଥା
#  define LSEEK lseek
#endif

#ଯଦି defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
ଅକ୍ଷର ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    ସ୍ଥିର ଅକ୍ଷର buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ଖାଲି,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ଖାଲି);
    ଯଦି (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ଯଦି (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ଯଦି (chars > ଆକାର (buf) - 1) {
            chars = ଆକାର (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    ଅନ୍ୟଥା {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    ଫେରନ୍ତୁ buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local ଶୂନ୍ୟ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    ଯଦି (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    ଅନ୍ୟଥା                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ଖାଲି);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ଅଚଳ ଶୂନ୍ୟ *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଅକ୍ଷର *mode) {
    gz_statep state;
    z_size_t len;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା oflag = 0;
#ifdef O_EXCL
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା exclusive = 0;
#endif

    /* check input */
    ଯଦି (path == ଖାଲି || mode == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(ଆକାର(gz_state));
    ଯଦି (state == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->ସନ୍ଦେଶ = ଖାଲି;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    ଯେତେବେଳେ (*mode) {
        ଯଦି (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        ଅନ୍ୟଥା
            ବାଛ (*mode) {
            ସ୍ଥିତି 'r':
                state->mode = GZ_READ;
                ଭାଙ୍ଗ;
#ifndef NO_GZCOMPRESS
            ସ୍ଥିତି 'w':
                state->mode = GZ_WRITE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 'a':
                state->mode = GZ_APPEND;
                ଭାଙ୍ଗ;
#endif
            ସ୍ଥିତି '+':       /* can't read and write at the same time */
                ମୁକ୍ତ(state);
                ଫେରନ୍ତୁ ଖାଲି;
            ସ୍ଥିତି 'b':       /* ignore -- will request binary anyway */
                ଭାଙ୍ଗ;
#ifdef O_CLOEXEC
            ସ୍ଥିତି 'e':
                oflag |= O_CLOEXEC;
                ଭାଙ୍ଗ;
#endif
#ifdef O_EXCL
            ସ୍ଥିତି 'x':
                exclusive = 1;
                ଭାଙ୍ଗ;
#endif
            ସ୍ଥିତି 'f':
                state->strategy = Z_FILTERED;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 'R':
                state->strategy = Z_RLE;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 'F':
                state->strategy = Z_FIXED;
                ଭାଙ୍ଗ;
            ସ୍ଥିତି 'G':
                state->direct = -1;
                ଭାଙ୍ଗ;
#ifdef O_NONBLOCK
            ସ୍ଥିତି 'N':
                oflag |= O_NONBLOCK;
                ଭାଙ୍ଗ;
#endif
            ସ୍ଥିତି 'T':
                state->direct = 1;
                ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    ଯଦି (state->mode == GZ_NONE) {
        ମୁକ୍ତ(state);
        ଫେରନ୍ତୁ ଖାଲି;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    ଯଦି (state->mode == GZ_READ) {
        ଯଦି (state->direct == 1) {
            /* can't force a transparent read */
            ମୁକ୍ତ(state);
            ଫେରନ୍ତୁ ଖାଲି;
        }
        ଯଦି (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    ଅନ୍ୟଥା ଯଦି (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        ମୁକ୍ତ(state);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    ଯଦି (fd == -2)
        len = wcstombs(ଖାଲି, path, 0);
    ଅନ୍ୟଥା
#endif
        len = strlen((ଅଚଳ ଅକ୍ଷର *)path);
    state->path = (ଅକ୍ଷର *)malloc(len + 1);
    ଯଦି (state->path == ଖାଲି) {
        ମୁକ୍ତ(state);
        ଫେରନ୍ତୁ ଖାଲି;
    }
#ifdef WIDECHAR
    ଯଦି (fd == -2) {
        ଯଦି (len)
            wcstombs(state->path, path, len + 1);
        ଅନ୍ୟଥା
            *(state->path) = 0;
    }
    ଅନ୍ୟଥା
#endif
    {
#ଯଦି !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (ଶୂନ୍ୟ)snprintf(state->path, len + 1, "%s", (ଅଚଳ ଅକ୍ଷର *)path);
#ଅନ୍ୟଥା
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_ଦ୍ୱିମିକ
        O_ଦ୍ୱିମିକ |
#endif
        (state->mode == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#ifdef O_EXCL
          (exclusive ? O_EXCL : 0) |
#endif
          (state->mode == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    ଯଦି (fd == -1)
        state->fd = open((ଅଚଳ ଅକ୍ଷର *)path, oflag, 0666);
#ifdef WIDECHAR
    ଅନ୍ୟଥା ଯଦି (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    ଅନ୍ୟଥା {
#ifdef O_NONBLOCK
        ଯଦି (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        ଯଦି (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    ଯଦି (state->fd == -1) {
        ମୁକ୍ତ(state->path);
        ମୁକ୍ତ(state);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ଯଦି (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    ଯଦି (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        ଯଦି (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ଫେରନ୍ତୁ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ଅଚଳ ଅକ୍ଷର *path, ଅଚଳ ଅକ୍ଷର *mode) {
    ଫେରନ୍ତୁ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ଅଚଳ ଅକ୍ଷର *path, ଅଚଳ ଅକ୍ଷର *mode) {
    ଫେରନ୍ତୁ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଅକ୍ଷର *mode) {
    ଅକ୍ଷର *path;         /* identifier for error messages */
    gzFile gz;

    ଯଦି (fd == -1 || (path = (ଅକ୍ଷର *)malloc(7 + 3 * ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))) == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;
#ଯଦି !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ଶୂନ୍ୟ)snprintf(path, 7 + 3 * ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା), "<fd:%d>", fd);
#ଅନ୍ୟଥା
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    ମୁକ୍ତ(path);
    ଫେରନ୍ତୁ gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(ଅଚଳ wchar_t *path, ଅଚଳ ଅକ୍ଷର *mode) {
    ଫେରନ୍ତୁ gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ -1;

    /* make sure we haven't already allocated memory */
    ଯଦି (state->size != 0)
        ଫେରନ୍ତୁ -1;

    /* check and set requested size */
    ଯଦି ((size << 1) < size)
        ଫେରନ୍ତୁ -1;              /* need to be able to double it */
    ଯଦି (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    ଫେରନ୍ତୁ 0;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    ଯଦି (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ଫେରନ୍ତୁ -1;

    /* back up and start over */
    ଯଦି (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        ଫେରନ୍ତୁ -1;
    gz_reset(state);
    ଫେରନ୍ତୁ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ -1;

    /* check that there's no error */
    ଯଦି (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ଫେରନ୍ତୁ -1;

    /* can only seek from start or relative to current position */
    ଯଦି (whence != SEEK_SET && whence != SEEK_CUR)
        ଫେରନ୍ତୁ -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    ଯଦି (offset > GZ_OFF_MAX - pos)
        ଫେରନ୍ତୁ -1;                          /* would overflow z_off64_t */
    ଯଦି (offset < -pos)
        ଫେରନ୍ତୁ -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    ଯଦି (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        ଯଦି (ret == -1)
            ଫେରନ୍ତୁ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ଖାଲି);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        ଫେରନ୍ତୁ pos;
    }

    /* rewind if needed for back seek when reading */
    ଯଦି (pos < state->x.pos) {
        ଯଦି (state->mode != GZ_READ)         /* writing -- can't go backwards */
            ଫେରନ୍ତୁ -1;
        ଯଦି (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ଫେରନ୍ତୁ -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    ଯଦି (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    ଫେରନ୍ତୁ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    ଫେରନ୍ତୁ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ -1;

    /* return position */
    ଫେରନ୍ତୁ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ଫେରନ୍ତୁ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ -1;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    ଯଦି (offset == -1)
        ଫେରନ୍ତୁ -1;
    ଯଦି (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    ଫେରନ୍ତୁ offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ଫେରନ୍ତୁ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ 0;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ 0;

    /* return end-of-file state */
    ଫେରନ୍ତୁ state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ଅଚଳ ଅକ୍ଷର * ZEXPORT gzerror(gzFile file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ ଖାଲି;

    /* return error information */
    ଯଦି (errnum != ଖାଲି)
        *errnum = state->err;
    ଫେରନ୍ତୁ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->ସନ୍ଦେଶ == ଖାଲି ? "" : state->ସନ୍ଦେଶ);
}

/* -- see zlib.h -- */
ଶୂନ୍ୟ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ;
    state = (gz_statep)file;
    ଯଦି (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ଫେରନ୍ତୁ;

    /* clear error and end-of-file */
    ଯଦି (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ଖାଲି);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
ଶୂନ୍ୟ ZLIB_INTERNAL gz_error(gz_statep state, ପୂର୍ଣ୍ଣସଂଖ୍ୟା err, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
    /* free previously allocated message and clear */
    ଯଦି (state->ସନ୍ଦେଶ != ଖାଲି) {
        ଯଦି (state->err != Z_MEM_ERROR)
            ମୁକ୍ତ(state->ସନ୍ଦେଶ);
        state->ସନ୍ଦେଶ = ଖାଲି;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    ଯଦି (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    ଯଦି (ସନ୍ଦେଶ == ଖାଲି)
        ଫେରନ୍ତୁ;

    /* for an out of memory error, return literal string when requested */
    ଯଦି (err == Z_MEM_ERROR)
        ଫେରନ୍ତୁ;

    /* construct error message with path */
    ଯଦି ((state->ସନ୍ଦେଶ = (ଅକ୍ଷର *)malloc(strlen(state->path) + strlen(ସନ୍ଦେଶ) + 3)) ==
            ଖାଲି) {
        state->err = Z_MEM_ERROR;
        ଫେରନ୍ତୁ;
    }
#ଯଦି !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ଶୂନ୍ୟ)snprintf(state->ସନ୍ଦେଶ, strlen(state->path) + strlen(ସନ୍ଦେଶ) + 3,
                   "%s%s%s", state->path, ": ", ସନ୍ଦେଶ);
#ଅନ୍ୟଥା
    strcpy(state->ସନ୍ଦେଶ, state->path);
    strcat(state->ସନ୍ଦେଶ, ": ");
    strcat(state->ସନ୍ଦେଶ, ସନ୍ଦେଶ);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(ଶୂନ୍ୟ) {
#ifdef INT_MAX
    ଫେରନ୍ତୁ INT_MAX;
#ଅନ୍ୟଥା
    unsigned p = 1, q;

    କର {
        q = p;
        p <<= 1;
        p++;
    } ଯେତେବେଳେ (p > q);
    ଫେରନ୍ତୁ q >> 1;
#endif
}
