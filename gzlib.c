/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#அந்தர்பாவ "gzguts.h"

#எனில் defined(__DJGPP__)
#  define LSEEK llseek
#அல்லதெனில் defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#அல்லதெனில் defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#இல்லையெனில்
#  define LSEEK lseek
#endif

#எனில் defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
எழுத்து ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    நிலையான எழுத்து buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        வெற்று,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        வெற்று);
    எனில் (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        எனில் (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        எனில் (chars > அளவு (buf) - 1) {
            chars = அளவு (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    இல்லையெனில் {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    திரும்பு buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local வெறுமை gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    எனில் (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    இல்லையெனில்                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, வெற்று);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(மாறிலி வெறுமை *path, முழு fd, மாறிலி எழுத்து *mode) {
    gz_statep state;
    z_size_t len;
    முழு oflag = 0;
#ifdef O_EXCL
    முழு exclusive = 0;
#endif

    /* check input */
    எனில் (path == வெற்று || mode == வெற்று)
        திரும்பு வெற்று;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(அளவு(gz_state));
    எனில் (state == வெற்று)
        திரும்பு வெற்று;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->செய்தி = வெற்று;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    போது (*mode) {
        எனில் (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        இல்லையெனில்
            தேர்வு (*mode) {
            நிலை 'r':
                state->mode = GZ_READ;
                நிறுத்து;
#ifndef NO_GZCOMPRESS
            நிலை 'w':
                state->mode = GZ_WRITE;
                நிறுத்து;
            நிலை 'a':
                state->mode = GZ_APPEND;
                நிறுத்து;
#endif
            நிலை '+':       /* can't read and write at the same time */
                விடுவி(state);
                திரும்பு வெற்று;
            நிலை 'b':       /* ignore -- will request binary anyway */
                நிறுத்து;
#ifdef O_CLOEXEC
            நிலை 'e':
                oflag |= O_CLOEXEC;
                நிறுத்து;
#endif
#ifdef O_EXCL
            நிலை 'x':
                exclusive = 1;
                நிறுத்து;
#endif
            நிலை 'f':
                state->strategy = Z_FILTERED;
                நிறுத்து;
            நிலை 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                நிறுத்து;
            நிலை 'R':
                state->strategy = Z_RLE;
                நிறுத்து;
            நிலை 'F':
                state->strategy = Z_FIXED;
                நிறுத்து;
            நிலை 'G':
                state->direct = -1;
                நிறுத்து;
#ifdef O_NONBLOCK
            நிலை 'N':
                oflag |= O_NONBLOCK;
                நிறுத்து;
#endif
            நிலை 'T':
                state->direct = 1;
                நிறுத்து;
            இயல்பு:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    எனில் (state->mode == GZ_NONE) {
        விடுவி(state);
        திரும்பு வெற்று;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    எனில் (state->mode == GZ_READ) {
        எனில் (state->direct == 1) {
            /* can't force a transparent read */
            விடுவி(state);
            திரும்பு வெற்று;
        }
        எனில் (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    இல்லையெனில் எனில் (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        விடுவி(state);
        திரும்பு வெற்று;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    எனில் (fd == -2)
        len = wcstombs(வெற்று, path, 0);
    இல்லையெனில்
#endif
        len = strlen((மாறிலி எழுத்து *)path);
    state->path = (எழுத்து *)malloc(len + 1);
    எனில் (state->path == வெற்று) {
        விடுவி(state);
        திரும்பு வெற்று;
    }
#ifdef WIDECHAR
    எனில் (fd == -2) {
        எனில் (len)
            wcstombs(state->path, path, len + 1);
        இல்லையெனில்
            *(state->path) = 0;
    }
    இல்லையெனில்
#endif
    {
#எனில் !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (வெறுமை)snprintf(state->path, len + 1, "%s", (மாறிலி எழுத்து *)path);
#இல்லையெனில்
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_இருமம்
        O_இருமம் |
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
    எனில் (fd == -1)
        state->fd = open((மாறிலி எழுத்து *)path, oflag, 0666);
#ifdef WIDECHAR
    இல்லையெனில் எனில் (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    இல்லையெனில் {
#ifdef O_NONBLOCK
        எனில் (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        எனில் (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    எனில் (state->fd == -1) {
        விடுவி(state->path);
        விடுவி(state);
        திரும்பு வெற்று;
    }
    எனில் (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    எனில் (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        எனில் (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    திரும்பு (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(மாறிலி எழுத்து *path, மாறிலி எழுத்து *mode) {
    திரும்பு gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(மாறிலி எழுத்து *path, மாறிலி எழுத்து *mode) {
    திரும்பு gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(முழு fd, மாறிலி எழுத்து *mode) {
    எழுத்து *path;         /* identifier for error messages */
    gzFile gz;

    எனில் (fd == -1 || (path = (எழுத்து *)malloc(7 + 3 * அளவு(முழு))) == வெற்று)
        திரும்பு வெற்று;
#எனில் !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (வெறுமை)snprintf(path, 7 + 3 * அளவு(முழு), "<fd:%d>", fd);
#இல்லையெனில்
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    விடுவி(path);
    திரும்பு gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(மாறிலி wchar_t *path, மாறிலி எழுத்து *mode) {
    திரும்பு gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
முழு ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    எனில் (file == வெற்று)
        திரும்பு -1;
    state = (gz_statep)file;
    எனில் (state->mode != GZ_READ && state->mode != GZ_WRITE)
        திரும்பு -1;

    /* make sure we haven't already allocated memory */
    எனில் (state->size != 0)
        திரும்பு -1;

    /* check and set requested size */
    எனில் ((size << 1) < size)
        திரும்பு -1;              /* need to be able to double it */
    எனில் (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    திரும்பு 0;
}

/* -- see zlib.h -- */
முழு ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    எனில் (file == வெற்று)
        திரும்பு -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    எனில் (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        திரும்பு -1;

    /* back up and start over */
    எனில் (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        திரும்பு -1;
    gz_reset(state);
    திரும்பு 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, முழு whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    எனில் (file == வெற்று)
        திரும்பு -1;
    state = (gz_statep)file;
    எனில் (state->mode != GZ_READ && state->mode != GZ_WRITE)
        திரும்பு -1;

    /* check that there's no error */
    எனில் (state->err != Z_OK && state->err != Z_BUF_ERROR)
        திரும்பு -1;

    /* can only seek from start or relative to current position */
    எனில் (whence != SEEK_SET && whence != SEEK_CUR)
        திரும்பு -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    எனில் (offset > GZ_OFF_MAX - pos)
        திரும்பு -1;                          /* would overflow z_off64_t */
    எனில் (offset < -pos)
        திரும்பு -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    எனில் (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        எனில் (ret == -1)
            திரும்பு -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, வெற்று);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        திரும்பு pos;
    }

    /* rewind if needed for back seek when reading */
    எனில் (pos < state->x.pos) {
        எனில் (state->mode != GZ_READ)         /* writing -- can't go backwards */
            திரும்பு -1;
        எனில் (gzrewind(file) == -1)           /* rewind, then skip to pos */
            திரும்பு -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    எனில் (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    திரும்பு pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, முழு whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    திரும்பு ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    எனில் (file == வெற்று)
        திரும்பு -1;
    state = (gz_statep)file;
    எனில் (state->mode != GZ_READ && state->mode != GZ_WRITE)
        திரும்பு -1;

    /* return position */
    திரும்பு state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    திரும்பு ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    எனில் (file == வெற்று)
        திரும்பு -1;
    state = (gz_statep)file;
    எனில் (state->mode != GZ_READ && state->mode != GZ_WRITE)
        திரும்பு -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    எனில் (offset == -1)
        திரும்பு -1;
    எனில் (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    திரும்பு offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    திரும்பு ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
முழு ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    எனில் (file == வெற்று)
        திரும்பு 0;
    state = (gz_statep)file;
    எனில் (state->mode != GZ_READ && state->mode != GZ_WRITE)
        திரும்பு 0;

    /* return end-of-file state */
    திரும்பு state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
மாறிலி எழுத்து * ZEXPORT gzerror(gzFile file, முழு *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    எனில் (file == வெற்று)
        திரும்பு வெற்று;
    state = (gz_statep)file;
    எனில் (state->mode != GZ_READ && state->mode != GZ_WRITE)
        திரும்பு வெற்று;

    /* return error information */
    எனில் (errnum != வெற்று)
        *errnum = state->err;
    திரும்பு state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->செய்தி == வெற்று ? "" : state->செய்தி);
}

/* -- see zlib.h -- */
வெறுமை ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    எனில் (file == வெற்று)
        திரும்பு;
    state = (gz_statep)file;
    எனில் (state->mode != GZ_READ && state->mode != GZ_WRITE)
        திரும்பு;

    /* clear error and end-of-file */
    எனில் (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, வெற்று);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
வெறுமை ZLIB_INTERNAL gz_error(gz_statep state, முழு err, மாறிலி எழுத்து *செய்தி) {
    /* free previously allocated message and clear */
    எனில் (state->செய்தி != வெற்று) {
        எனில் (state->err != Z_MEM_ERROR)
            விடுவி(state->செய்தி);
        state->செய்தி = வெற்று;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    எனில் (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    எனில் (செய்தி == வெற்று)
        திரும்பு;

    /* for an out of memory error, return literal string when requested */
    எனில் (err == Z_MEM_ERROR)
        திரும்பு;

    /* construct error message with path */
    எனில் ((state->செய்தி = (எழுத்து *)malloc(strlen(state->path) + strlen(செய்தி) + 3)) ==
            வெற்று) {
        state->err = Z_MEM_ERROR;
        திரும்பு;
    }
#எனில் !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (வெறுமை)snprintf(state->செய்தி, strlen(state->path) + strlen(செய்தி) + 3,
                   "%s%s%s", state->path, ": ", செய்தி);
#இல்லையெனில்
    strcpy(state->செய்தி, state->path);
    strcat(state->செய்தி, ": ");
    strcat(state->செய்தி, செய்தி);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(வெறுமை) {
#ifdef INT_MAX
    திரும்பு INT_MAX;
#இல்லையெனில்
    unsigned p = 1, q;

    செய் {
        q = p;
        p <<= 1;
        p++;
    } போது (p > q);
    திரும்பு q >> 1;
#endif
}
