/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "gzguts.h"

#ਜੇ defined(__DJGPP__)
#  define LSEEK llseek
#ਜਾਂਜੇ defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#ਜਾਂਜੇ defined(LARGEFILE64_ਸਰੋਤ) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#ਨਹੀਂਤਾਂ
#  define LSEEK lseek
#ਜੇ_ਅੰਤ

#ਜੇ defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
ਅੱਖਰ ZLIB_ਅੰਦਰੂਨੀ *gz_strwinerror(DWORD error) {
    ਸਥਿਰ ਅੱਖਰ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_ਆਖਰੀ_Error();
    DWORD chars = Format_ਸੁਨੇਹਾ(FORMAT_ਸੁਨੇਹਾ_FROM_SYSTEM
        | FORMAT_ਸੁਨੇਹਾ_ALLOCATE_ਬਫਰ,
        ਨਿਰਾ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ਨਿਰਾ);
    ਜੇ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ਜੇ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ਜੇ (chars > ਆਕਾਰ (buf) - 1) {
            chars = ਆਕਾਰ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    ਨਹੀਂਤਾਂ {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    ਸੈੱਟ_ਆਖਰੀ_Error(lasterr);
    ਵਾਪਸ buf;
}

#ਜੇ_ਅੰਤ /* UNDER_CE */

/* Reset gzip file state */
local ਖਾਲੀ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    ਨਹੀਂਤਾਂ                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ਨਿਰਾ);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_ਵਿੱਚ = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ਅਚਲ ਖਾਲੀ *ਰਸਤਾ, ਪੂਰਨ fd, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    gz_statep state;
    z_ਮਾਪ_t len;
    ਪੂਰਨ oflag = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_EXCL
    ਪੂਰਨ exclusive = 0;
#ਜੇ_ਅੰਤ

    /* check input */
    ਜੇ (ਰਸਤਾ == ਨਿਰਾ || ਬਹੁਲਕ == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;

    /* allocate gzFile structure to return */
    state = (gz_statep)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(gz_state));
    ਜੇ (state == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    state->ਮਾਪ = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->ਸੁਨੇਹਾ = ਨਿਰਾ;          /* no error message yet */

    /* interpret mode */
    state->ਬਹੁਲਕ = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->ਸਿੱਧਾ = 0;
    ਜਦੋਂ (*ਬਹੁਲਕ) {
        ਜੇ (*ਬਹੁਲਕ >= '0' && *ਬਹੁਲਕ <= '9')
            state->level = *ਬਹੁਲਕ - '0';
        ਨਹੀਂਤਾਂ
            ਚੋਣ (*ਬਹੁਲਕ) {
            ਹਾਲਤ 'r':
                state->ਬਹੁਲਕ = GZ_READ;
                ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_GZCOMPRESS
            ਹਾਲਤ 'w':
                state->ਬਹੁਲਕ = GZ_WRITE;
                ਰੋਕੋ;
            ਹਾਲਤ 'a':
                state->ਬਹੁਲਕ = GZ_APPEND;
                ਰੋਕੋ;
#ਜੇ_ਅੰਤ
            ਹਾਲਤ '+':       /* can't read and write at the same time */
                ਮੁਕਤ(state);
                ਵਾਪਸ ਨਿਰਾ;
            ਹਾਲਤ 'b':       /* ignore -- will request binary anyway */
                ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_CLOEXEC
            ਹਾਲਤ 'e':
                oflag |= O_CLOEXEC;
                ਰੋਕੋ;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_EXCL
            ਹਾਲਤ 'x':
                exclusive = 1;
                ਰੋਕੋ;
#ਜੇ_ਅੰਤ
            ਹਾਲਤ 'f':
                state->strategy = Z_FILTERED;
                ਰੋਕੋ;
            ਹਾਲਤ 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                ਰੋਕੋ;
            ਹਾਲਤ 'R':
                state->strategy = Z_RLE;
                ਰੋਕੋ;
            ਹਾਲਤ 'F':
                state->strategy = Z_FIXED;
                ਰੋਕੋ;
            ਹਾਲਤ 'G':
                state->ਸਿੱਧਾ = -1;
                ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_NONBLOCK
            ਹਾਲਤ 'N':
                oflag |= O_NONBLOCK;
                ਰੋਕੋ;
#ਜੇ_ਅੰਤ
            ਹਾਲਤ 'T':
                state->ਸਿੱਧਾ = 1;
                ਰੋਕੋ;
            ਮੂਲ:        /* could consider as an error, but just ignore */
                ;
            }
        ਬਹੁਲਕ++;
    }

    /* must provide an "r", "w", or "a" */
    ਜੇ (state->ਬਹੁਲਕ == GZ_NONE) {
        ਮੁਕਤ(state);
        ਵਾਪਸ ਨਿਰਾ;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ) {
        ਜੇ (state->ਸਿੱਧਾ == 1) {
            /* can't force a transparent read */
            ਮੁਕਤ(state);
            ਵਾਪਸ ਨਿਰਾ;
        }
        ਜੇ (state->ਸਿੱਧਾ == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->ਸਿੱਧਾ = 1;
    }
    ਨਹੀਂਤਾਂ ਜੇ (state->ਸਿੱਧਾ == -1) {
        /* "G" has no meaning when writing -- disallow it */
        ਮੁਕਤ(state);
        ਵਾਪਸ ਨਿਰਾ;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIDECHAR
    ਜੇ (fd == -2)
        len = wcstombs(ਨਿਰਾ, ਰਸਤਾ, 0);
    ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
        len = ਲੜੀ_ਲੰਬਾਈ((ਅਚਲ ਅੱਖਰ *)ਰਸਤਾ);
    state->ਰਸਤਾ = (ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(len + 1);
    ਜੇ (state->ਰਸਤਾ == ਨਿਰਾ) {
        ਮੁਕਤ(state);
        ਵਾਪਸ ਨਿਰਾ;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIDECHAR
    ਜੇ (fd == -2) {
        ਜੇ (len)
            wcstombs(state->ਰਸਤਾ, ਰਸਤਾ, len + 1);
        ਨਹੀਂਤਾਂ
            *(state->ਰਸਤਾ) = 0;
    }
    ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
#ਜੇ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (ਖਾਲੀ)snprintf(state->ਰਸਤਾ, len + 1, "%s", (ਅਚਲ ਅੱਖਰ *)ਰਸਤਾ);
#ਨਹੀਂਤਾਂ
        strcpy(state->ਰਸਤਾ, ਰਸਤਾ);
#ਜੇ_ਅੰਤ
    }

    /* compute the flags for open() */
    oflag |=
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_LARGEFILE
        O_LARGEFILE |
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_ਦੁਆਧਾਰੀ
        O_ਦੁਆਧਾਰੀ |
#ਜੇ_ਅੰਤ
        (state->ਬਹੁਲਕ == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_EXCL
          (exclusive ? O_EXCL : 0) |
#ਜੇ_ਅੰਤ
          (state->ਬਹੁਲਕ == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    ਜੇ (fd == -1)
        state->fd = open((ਅਚਲ ਅੱਖਰ *)ਰਸਤਾ, oflag, 0666);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIDECHAR
    ਨਹੀਂਤਾਂ ਜੇ (fd == -2)
        state->fd = _wopen(ਰਸਤਾ, oflag, _S_IREAD | _S_IWRITE);
#ਜੇ_ਅੰਤ
    ਨਹੀਂਤਾਂ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_NONBLOCK
        ਜੇ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_CLOEXEC
        ਜੇ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#ਜੇ_ਅੰਤ
        state->fd = fd;
    }
    ਜੇ (state->fd == -1) {
        ਮੁਕਤ(state->ਰਸਤਾ);
        ਮੁਕਤ(state);
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਜੇ (state->ਬਹੁਲਕ == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->ਬਹੁਲਕ = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ) {
        state->ਸ਼ੁਰੂ = LSEEK(state->fd, 0, SEEK_CUR);
        ਜੇ (state->ਸ਼ੁਰੂ == -1) state->ਸ਼ੁਰੂ = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ਵਾਪਸ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    ਵਾਪਸ gz_open(ਰਸਤਾ, -1, ਬਹੁਲਕ);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    ਵਾਪਸ gz_open(ਰਸਤਾ, -1, ਬਹੁਲਕ);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(ਪੂਰਨ fd, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    ਅੱਖਰ *ਰਸਤਾ;         /* identifier for error messages */
    gzFile gz;

    ਜੇ (fd == -1 || (ਰਸਤਾ = (ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(7 + 3 * ਆਕਾਰ(ਪੂਰਨ))) == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
#ਜੇ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ਖਾਲੀ)snprintf(ਰਸਤਾ, 7 + 3 * ਆਕਾਰ(ਪੂਰਨ), "<fd:%d>", fd);
#ਨਹੀਂਤਾਂ
    sprintf(ਰਸਤਾ, "<fd:%d>", fd);   /* for debugging */
#ਜੇ_ਅੰਤ
    gz = gz_open(ਰਸਤਾ, fd, ਬਹੁਲਕ);
    ਮੁਕਤ(ਰਸਤਾ);
    ਵਾਪਸ gz;
}

/* -- see zlib.h -- */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIDECHAR
gzFile ZEXPORT gzopen_w(ਅਚਲ wchar_t *ਰਸਤਾ, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
    ਵਾਪਸ gz_open(ਰਸਤਾ, -2, ਬਹੁਲਕ);
}
#ਜੇ_ਅੰਤ

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzbuffer(gzFile file, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ) {
    gz_statep state;

    /* get internal structure and check integrity */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ && state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ -1;

    /* make sure we haven't already allocated memory */
    ਜੇ (state->ਮਾਪ != 0)
        ਵਾਪਸ -1;

    /* check and set requested size */
    ਜੇ ((ਮਾਪ << 1) < ਮਾਪ)
        ਵਾਪਸ -1;              /* need to be able to double it */
    ਜੇ (ਮਾਪ < 8)
        ਮਾਪ = 8;               /* needed to behave well with flushing */
    state->want = ਮਾਪ;
    ਵਾਪਸ 0;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ਵਾਪਸ -1;

    /* back up and start over */
    ਜੇ (LSEEK(state->fd, state->ਸ਼ੁਰੂ, SEEK_SET) == -1)
        ਵਾਪਸ -1;
    gz_reset(state);
    ਵਾਪਸ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t ਵਿੱਥ, ਪੂਰਨ whence) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ && state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ -1;

    /* check that there's no error */
    ਜੇ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ਵਾਪਸ -1;

    /* can only seek from start or relative to current position */
    ਜੇ (whence != SEEK_SET && whence != SEEK_CUR)
        ਵਾਪਸ -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    ਜੇ (ਵਿੱਥ > GZ_OFF_MAX - pos)
        ਵਾਪਸ -1;                          /* would overflow z_off64_t */
    ਜੇ (ਵਿੱਥ < -pos)
        ਵਾਪਸ -1;                          /* before start of file */
    pos += ਵਿੱਥ;

    /* if within raw area while reading, just go there */
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        ਜੇ (ret == -1)
            ਵਾਪਸ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ਨਿਰਾ);
        state->strm.avail_ਵਿੱਚ = 0;
        state->x.pos = pos;
        ਵਾਪਸ pos;
    }

    /* rewind if needed for back seek when reading */
    ਜੇ (pos < state->x.pos) {
        ਜੇ (state->ਬਹੁਲਕ != GZ_READ)         /* writing -- can't go backwards */
            ਵਾਪਸ -1;
        ਜੇ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ਵਾਪਸ -1;
    }
    ਵਿੱਥ = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > ਵਿੱਥ ?
            (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਵਿੱਥ : state->x.have;
        state->x.have -= n;
        state->x.ਅਗਲਾ += n;
        state->x.pos += n;
        ਵਿੱਥ -= n;
    }

    /* request skip (if not zero) */
    state->skip = ਵਿੱਥ;
    ਵਾਪਸ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t ਵਿੱਥ, ਪੂਰਨ whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)ਵਿੱਥ, whence);
    ਵਾਪਸ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ && state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ -1;

    /* return position */
    ਵਾਪਸ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ਵਾਪਸ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t ਵਿੱਥ;
    gz_statep state;

    /* get internal structure and check integrity */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ -1;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ && state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ -1;

    /* compute and return effective offset in file */
    ਵਿੱਥ = LSEEK(state->fd, 0, SEEK_CUR);
    ਜੇ (ਵਿੱਥ == -1)
        ਵਾਪਸ -1;
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ)             /* reading */
        ਵਿੱਥ -= state->strm.avail_ਵਿੱਚ;     /* don't count buffered input */
    ਵਾਪਸ ਵਿੱਥ;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ਵਾਪਸ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
ਪੂਰਨ ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ 0;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ && state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ 0;

    /* return end-of-file state */
    ਵਾਪਸ state->ਬਹੁਲਕ == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ਅਚਲ ਅੱਖਰ * ZEXPORT gzerror(gzFile file, ਪੂਰਨ *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ && state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ ਨਿਰਾ;

    /* return error information */
    ਜੇ (errnum != ਨਿਰਾ)
        *errnum = state->err;
    ਵਾਪਸ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->ਸੁਨੇਹਾ == ਨਿਰਾ ? "" : state->ਸੁਨੇਹਾ);
}

/* -- see zlib.h -- */
ਖਾਲੀ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ;
    state = (gz_statep)file;
    ਜੇ (state->ਬਹੁਲਕ != GZ_READ && state->ਬਹੁਲਕ != GZ_WRITE)
        ਵਾਪਸ;

    /* clear error and end-of-file */
    ਜੇ (state->ਬਹੁਲਕ == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ਨਿਰਾ);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
ਖਾਲੀ ZLIB_ਅੰਦਰੂਨੀ gz_error(gz_statep state, ਪੂਰਨ err, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
    /* free previously allocated message and clear */
    ਜੇ (state->ਸੁਨੇਹਾ != ਨਿਰਾ) {
        ਜੇ (state->err != Z_MEM_ERROR)
            ਮੁਕਤ(state->ਸੁਨੇਹਾ);
        state->ਸੁਨੇਹਾ = ਨਿਰਾ;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    ਜੇ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    ਜੇ (ਸੁਨੇਹਾ == ਨਿਰਾ)
        ਵਾਪਸ;

    /* for an out of memory error, return literal string when requested */
    ਜੇ (err == Z_MEM_ERROR)
        ਵਾਪਸ;

    /* construct error message with path */
    ਜੇ ((state->ਸੁਨੇਹਾ = (ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਲੜੀ_ਲੰਬਾਈ(state->ਰਸਤਾ) + ਲੜੀ_ਲੰਬਾਈ(ਸੁਨੇਹਾ) + 3)) ==
            ਨਿਰਾ) {
        state->err = Z_MEM_ERROR;
        ਵਾਪਸ;
    }
#ਜੇ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ਖਾਲੀ)snprintf(state->ਸੁਨੇਹਾ, ਲੜੀ_ਲੰਬਾਈ(state->ਰਸਤਾ) + ਲੜੀ_ਲੰਬਾਈ(ਸੁਨੇਹਾ) + 3,
                   "%s%s%s", state->ਰਸਤਾ, ": ", ਸੁਨੇਹਾ);
#ਨਹੀਂਤਾਂ
    strcpy(state->ਸੁਨੇਹਾ, state->ਰਸਤਾ);
    strcat(state->ਸੁਨੇਹਾ, ": ");
    strcat(state->ਸੁਨੇਹਾ, ਸੁਨੇਹਾ);
#ਜੇ_ਅੰਤ
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
ਬਿਨਾਨਿਸ਼ਾਨੀ ZLIB_ਅੰਦਰੂਨੀ gz_intmax(ਖਾਲੀ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ
    ਵਾਪਸ ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
#ਨਹੀਂਤਾਂ
    ਬਿਨਾਨਿਸ਼ਾਨੀ p = 1, q;

    ਕਰੋ {
        q = p;
        p <<= 1;
        p++;
    } ਜਦੋਂ (p > q);
    ਵਾਪਸ q >> 1;
#ਜੇ_ਅੰਤ
}
