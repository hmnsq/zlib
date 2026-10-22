/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "gzguts.h"

#ಯದಿ defined(__DJGPP__)
#  define LSEEK llseek
#ನೋಚೇದ್ಯದಿ defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#ನೋಚೇದ್ಯದಿ defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#ಅನ್ಯಥಾ
#  define LSEEK lseek
#endif

#ಯದಿ defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
ವರ್ಣ ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    ಸ್ಥಿರ ವರ್ಣ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ರಿಕ್ತ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ರಿಕ್ತ);
    ಯದಿ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ಯದಿ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ಯದಿ (chars > ಪರಿಮಾಣ (buf) - 1) {
            chars = ಪರಿಮಾಣ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    ಅನ್ಯಥಾ {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    ಪ್ರತ್ಯಾಗಮ buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local ಶೂನ್ಯ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    ಯದಿ (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    ಅನ್ಯಥಾ                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ರಿಕ್ತ);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ಧ್ರುವ ಶೂನ್ಯ *path, ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ವರ್ಣ *mode) {
    gz_statep state;
    z_size_t len;
    ಪೂರ್ಣಾಙ್ಕ oflag = 0;
#ifdef O_EXCL
    ಪೂರ್ಣಾಙ್ಕ exclusive = 0;
#endif

    /* check input */
    ಯದಿ (path == ರಿಕ್ತ || mode == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(ಪರಿಮಾಣ(gz_state));
    ಯದಿ (state == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->ಸಂದೇಶ = ರಿಕ್ತ;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    ಯಾವತ್ (*mode) {
        ಯದಿ (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        ಅನ್ಯಥಾ
            ವಿಕಲ್ಪ (*mode) {
            ಪಕ್ಷ 'r':
                state->mode = GZ_READ;
                ಭಙ್ಗ;
#ifndef NO_GZCOMPRESS
            ಪಕ್ಷ 'w':
                state->mode = GZ_WRITE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 'a':
                state->mode = GZ_APPEND;
                ಭಙ್ಗ;
#endif
            ಪಕ್ಷ '+':       /* can't read and write at the same time */
                ಮುಕ್ತ(state);
                ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
            ಪಕ್ಷ 'b':       /* ignore -- will request binary anyway */
                ಭಙ್ಗ;
#ifdef O_CLOEXEC
            ಪಕ್ಷ 'e':
                oflag |= O_CLOEXEC;
                ಭಙ್ಗ;
#endif
#ifdef O_EXCL
            ಪಕ್ಷ 'x':
                exclusive = 1;
                ಭಙ್ಗ;
#endif
            ಪಕ್ಷ 'f':
                state->strategy = Z_FILTERED;
                ಭಙ್ಗ;
            ಪಕ್ಷ 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                ಭಙ್ಗ;
            ಪಕ್ಷ 'R':
                state->strategy = Z_RLE;
                ಭಙ್ಗ;
            ಪಕ್ಷ 'F':
                state->strategy = Z_FIXED;
                ಭಙ್ಗ;
            ಪಕ್ಷ 'G':
                state->direct = -1;
                ಭಙ್ಗ;
#ifdef O_NONBLOCK
            ಪಕ್ಷ 'N':
                oflag |= O_NONBLOCK;
                ಭಙ್ಗ;
#endif
            ಪಕ್ಷ 'T':
                state->direct = 1;
                ಭಙ್ಗ;
            ಸ್ವಭಾವ:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    ಯದಿ (state->mode == GZ_NONE) {
        ಮುಕ್ತ(state);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    ಯದಿ (state->mode == GZ_READ) {
        ಯದಿ (state->direct == 1) {
            /* can't force a transparent read */
            ಮುಕ್ತ(state);
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        }
        ಯದಿ (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    ಅನ್ಯಥಾ ಯದಿ (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        ಮುಕ್ತ(state);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    ಯದಿ (fd == -2)
        len = wcstombs(ರಿಕ್ತ, path, 0);
    ಅನ್ಯಥಾ
#endif
        len = strlen((ಧ್ರುವ ವರ್ಣ *)path);
    state->path = (ವರ್ಣ *)malloc(len + 1);
    ಯದಿ (state->path == ರಿಕ್ತ) {
        ಮುಕ್ತ(state);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
#ifdef WIDECHAR
    ಯದಿ (fd == -2) {
        ಯದಿ (len)
            wcstombs(state->path, path, len + 1);
        ಅನ್ಯಥಾ
            *(state->path) = 0;
    }
    ಅನ್ಯಥಾ
#endif
    {
#ಯದಿ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (ಶೂನ್ಯ)snprintf(state->path, len + 1, "%s", (ಧ್ರುವ ವರ್ಣ *)path);
#ಅನ್ಯಥಾ
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_ದ್ವಿಮಾನ
        O_ದ್ವಿಮಾನ |
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
    ಯದಿ (fd == -1)
        state->fd = open((ಧ್ರುವ ವರ್ಣ *)path, oflag, 0666);
#ifdef WIDECHAR
    ಅನ್ಯಥಾ ಯದಿ (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    ಅನ್ಯಥಾ {
#ifdef O_NONBLOCK
        ಯದಿ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        ಯದಿ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    ಯದಿ (state->fd == -1) {
        ಮುಕ್ತ(state->path);
        ಮುಕ್ತ(state);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    ಯದಿ (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    ಯದಿ (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        ಯದಿ (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ಪ್ರತ್ಯಾಗಮ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ಧ್ರುವ ವರ್ಣ *path, ಧ್ರುವ ವರ್ಣ *mode) {
    ಪ್ರತ್ಯಾಗಮ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ಧ್ರುವ ವರ್ಣ *path, ಧ್ರುವ ವರ್ಣ *mode) {
    ಪ್ರತ್ಯಾಗಮ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ವರ್ಣ *mode) {
    ವರ್ಣ *path;         /* identifier for error messages */
    gzFile gz;

    ಯದಿ (fd == -1 || (path = (ವರ್ಣ *)malloc(7 + 3 * ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))) == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#ಯದಿ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ಶೂನ್ಯ)snprintf(path, 7 + 3 * ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ), "<fd:%d>", fd);
#ಅನ್ಯಥಾ
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    ಮುಕ್ತ(path);
    ಪ್ರತ್ಯಾಗಮ gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(ಧ್ರುವ wchar_t *path, ಧ್ರುವ ವರ್ಣ *mode) {
    ಪ್ರತ್ಯಾಗಮ gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;
    state = (gz_statep)file;
    ಯದಿ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* make sure we haven't already allocated memory */
    ಯದಿ (state->size != 0)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* check and set requested size */
    ಯದಿ ((size << 1) < size)
        ಪ್ರತ್ಯಾಗಮ -1;              /* need to be able to double it */
    ಯದಿ (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    ಪ್ರತ್ಯಾಗಮ 0;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    ಯದಿ (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ಪ್ರತ್ಯಾಗಮ -1;

    /* back up and start over */
    ಯದಿ (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        ಪ್ರತ್ಯಾಗಮ -1;
    gz_reset(state);
    ಪ್ರತ್ಯಾಗಮ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, ಪೂರ್ಣಾಙ್ಕ whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;
    state = (gz_statep)file;
    ಯದಿ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* check that there's no error */
    ಯದಿ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* can only seek from start or relative to current position */
    ಯದಿ (whence != SEEK_SET && whence != SEEK_CUR)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    ಯದಿ (offset > GZ_OFF_MAX - pos)
        ಪ್ರತ್ಯಾಗಮ -1;                          /* would overflow z_off64_t */
    ಯದಿ (offset < -pos)
        ಪ್ರತ್ಯಾಗಮ -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    ಯದಿ (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        ಯದಿ (ret == -1)
            ಪ್ರತ್ಯಾಗಮ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ರಿಕ್ತ);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        ಪ್ರತ್ಯಾಗಮ pos;
    }

    /* rewind if needed for back seek when reading */
    ಯದಿ (pos < state->x.pos) {
        ಯದಿ (state->mode != GZ_READ)         /* writing -- can't go backwards */
            ಪ್ರತ್ಯಾಗಮ -1;
        ಯದಿ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ಪ್ರತ್ಯಾಗಮ -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    ಯದಿ (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    ಪ್ರತ್ಯಾಗಮ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, ಪೂರ್ಣಾಙ್ಕ whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    ಪ್ರತ್ಯಾಗಮ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;
    state = (gz_statep)file;
    ಯದಿ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* return position */
    ಪ್ರತ್ಯಾಗಮ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ಪ್ರತ್ಯಾಗಮ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ -1;
    state = (gz_statep)file;
    ಯದಿ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    ಯದಿ (offset == -1)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    ಪ್ರತ್ಯಾಗಮ offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ಪ್ರತ್ಯಾಗಮ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಙ್ಕ ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ 0;
    state = (gz_statep)file;
    ಯದಿ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* return end-of-file state */
    ಪ್ರತ್ಯಾಗಮ state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ಧ್ರುವ ವರ್ಣ * ZEXPORT gzerror(gzFile file, ಪೂರ್ಣಾಙ್ಕ *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    state = (gz_statep)file;
    ಯದಿ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    /* return error information */
    ಯದಿ (errnum != ರಿಕ್ತ)
        *errnum = state->err;
    ಪ್ರತ್ಯಾಗಮ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->ಸಂದೇಶ == ರಿಕ್ತ ? "" : state->ಸಂದೇಶ);
}

/* -- see zlib.h -- */
ಶೂನ್ಯ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಯದಿ (file == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ;
    state = (gz_statep)file;
    ಯದಿ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ಪ್ರತ್ಯಾಗಮ;

    /* clear error and end-of-file */
    ಯದಿ (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ರಿಕ್ತ);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
ಶೂನ್ಯ ZLIB_INTERNAL gz_error(gz_statep state, ಪೂರ್ಣಾಙ್ಕ err, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) {
    /* free previously allocated message and clear */
    ಯದಿ (state->ಸಂದೇಶ != ರಿಕ್ತ) {
        ಯದಿ (state->err != Z_MEM_ERROR)
            ಮುಕ್ತ(state->ಸಂದೇಶ);
        state->ಸಂದೇಶ = ರಿಕ್ತ;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    ಯದಿ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    ಯದಿ (ಸಂದೇಶ == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ;

    /* for an out of memory error, return literal string when requested */
    ಯದಿ (err == Z_MEM_ERROR)
        ಪ್ರತ್ಯಾಗಮ;

    /* construct error message with path */
    ಯದಿ ((state->ಸಂದೇಶ = (ವರ್ಣ *)malloc(strlen(state->path) + strlen(ಸಂದೇಶ) + 3)) ==
            ರಿಕ್ತ) {
        state->err = Z_MEM_ERROR;
        ಪ್ರತ್ಯಾಗಮ;
    }
#ಯದಿ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ಶೂನ್ಯ)snprintf(state->ಸಂದೇಶ, strlen(state->path) + strlen(ಸಂದೇಶ) + 3,
                   "%s%s%s", state->path, ": ", ಸಂದೇಶ);
#ಅನ್ಯಥಾ
    strcpy(state->ಸಂದೇಶ, state->path);
    strcat(state->ಸಂದೇಶ, ": ");
    strcat(state->ಸಂದೇಶ, ಸಂದೇಶ);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(ಶೂನ್ಯ) {
#ifdef INT_MAX
    ಪ್ರತ್ಯಾಗಮ INT_MAX;
#ಅನ್ಯಥಾ
    unsigned p = 1, q;

    ಕುರು {
        q = p;
        p <<= 1;
        p++;
    } ಯಾವತ್ (p > q);
    ಪ್ರತ್ಯಾಗಮ q >> 1;
#endif
}
