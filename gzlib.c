/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#അന്തര്ഭാവ "gzguts.h"

#എങ്കിൽ defined(__DJGPP__)
#  define LSEEK llseek
#അല്ലെങ്കിലെങ്കിൽ defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#അല്ലെങ്കിലെങ്കിൽ defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#അല്ലെങ്കിൽ
#  define LSEEK lseek
#endif

#എങ്കിൽ defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
അക്ഷരം ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    സ്ഥിതം അക്ഷരം buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ശൂന്യ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ശൂന്യ);
    എങ്കിൽ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        എങ്കിൽ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        എങ്കിൽ (chars > വലുപ്പം (buf) - 1) {
            chars = വലുപ്പം (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    അല്ലെങ്കിൽ {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    തിരികെ buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local ശൂന്യം gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    എങ്കിൽ (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    അല്ലെങ്കിൽ                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ശൂന്യ);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(സ്ഥിരം ശൂന്യം *path, പൂർണ്ണസംഖ്യ fd, സ്ഥിരം അക്ഷരം *mode) {
    gz_statep state;
    z_size_t len;
    പൂർണ്ണസംഖ്യ oflag = 0;
#ifdef O_EXCL
    പൂർണ്ണസംഖ്യ exclusive = 0;
#endif

    /* check input */
    എങ്കിൽ (path == ശൂന്യ || mode == ശൂന്യ)
        തിരികെ ശൂന്യ;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(വലുപ്പം(gz_state));
    എങ്കിൽ (state == ശൂന്യ)
        തിരികെ ശൂന്യ;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->സന്ദേശം = ശൂന്യ;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    ആകുമ്പോൾ (*mode) {
        എങ്കിൽ (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        അല്ലെങ്കിൽ
            തിരഞ്ഞെടുപ്പ് (*mode) {
            അവസ്ഥ 'r':
                state->mode = GZ_READ;
                നിർത്തുക;
#ifndef NO_GZCOMPRESS
            അവസ്ഥ 'w':
                state->mode = GZ_WRITE;
                നിർത്തുക;
            അവസ്ഥ 'a':
                state->mode = GZ_APPEND;
                നിർത്തുക;
#endif
            അവസ്ഥ '+':       /* can't read and write at the same time */
                സ്വതന്ത്രം(state);
                തിരികെ ശൂന്യ;
            അവസ്ഥ 'b':       /* ignore -- will request binary anyway */
                നിർത്തുക;
#ifdef O_CLOEXEC
            അവസ്ഥ 'e':
                oflag |= O_CLOEXEC;
                നിർത്തുക;
#endif
#ifdef O_EXCL
            അവസ്ഥ 'x':
                exclusive = 1;
                നിർത്തുക;
#endif
            അവസ്ഥ 'f':
                state->strategy = Z_FILTERED;
                നിർത്തുക;
            അവസ്ഥ 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                നിർത്തുക;
            അവസ്ഥ 'R':
                state->strategy = Z_RLE;
                നിർത്തുക;
            അവസ്ഥ 'F':
                state->strategy = Z_FIXED;
                നിർത്തുക;
            അവസ്ഥ 'G':
                state->direct = -1;
                നിർത്തുക;
#ifdef O_NONBLOCK
            അവസ്ഥ 'N':
                oflag |= O_NONBLOCK;
                നിർത്തുക;
#endif
            അവസ്ഥ 'T':
                state->direct = 1;
                നിർത്തുക;
            സ്ഥിരസ്ഥിതി:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    എങ്കിൽ (state->mode == GZ_NONE) {
        സ്വതന്ത്രം(state);
        തിരികെ ശൂന്യ;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    എങ്കിൽ (state->mode == GZ_READ) {
        എങ്കിൽ (state->direct == 1) {
            /* can't force a transparent read */
            സ്വതന്ത്രം(state);
            തിരികെ ശൂന്യ;
        }
        എങ്കിൽ (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    അല്ലെങ്കിൽ എങ്കിൽ (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        സ്വതന്ത്രം(state);
        തിരികെ ശൂന്യ;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    എങ്കിൽ (fd == -2)
        len = wcstombs(ശൂന്യ, path, 0);
    അല്ലെങ്കിൽ
#endif
        len = strlen((സ്ഥിരം അക്ഷരം *)path);
    state->path = (അക്ഷരം *)malloc(len + 1);
    എങ്കിൽ (state->path == ശൂന്യ) {
        സ്വതന്ത്രം(state);
        തിരികെ ശൂന്യ;
    }
#ifdef WIDECHAR
    എങ്കിൽ (fd == -2) {
        എങ്കിൽ (len)
            wcstombs(state->path, path, len + 1);
        അല്ലെങ്കിൽ
            *(state->path) = 0;
    }
    അല്ലെങ്കിൽ
#endif
    {
#എങ്കിൽ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (ശൂന്യം)snprintf(state->path, len + 1, "%s", (സ്ഥിരം അക്ഷരം *)path);
#അല്ലെങ്കിൽ
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_ദ്വിമാന
        O_ദ്വിമാന |
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
    എങ്കിൽ (fd == -1)
        state->fd = open((സ്ഥിരം അക്ഷരം *)path, oflag, 0666);
#ifdef WIDECHAR
    അല്ലെങ്കിൽ എങ്കിൽ (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    അല്ലെങ്കിൽ {
#ifdef O_NONBLOCK
        എങ്കിൽ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        എങ്കിൽ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    എങ്കിൽ (state->fd == -1) {
        സ്വതന്ത്രം(state->path);
        സ്വതന്ത്രം(state);
        തിരികെ ശൂന്യ;
    }
    എങ്കിൽ (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    എങ്കിൽ (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        എങ്കിൽ (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    തിരികെ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(സ്ഥിരം അക്ഷരം *path, സ്ഥിരം അക്ഷരം *mode) {
    തിരികെ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(സ്ഥിരം അക്ഷരം *path, സ്ഥിരം അക്ഷരം *mode) {
    തിരികെ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം അക്ഷരം *mode) {
    അക്ഷരം *path;         /* identifier for error messages */
    gzFile gz;

    എങ്കിൽ (fd == -1 || (path = (അക്ഷരം *)malloc(7 + 3 * വലുപ്പം(പൂർണ്ണസംഖ്യ))) == ശൂന്യ)
        തിരികെ ശൂന്യ;
#എങ്കിൽ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ശൂന്യം)snprintf(path, 7 + 3 * വലുപ്പം(പൂർണ്ണസംഖ്യ), "<fd:%d>", fd);
#അല്ലെങ്കിൽ
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    സ്വതന്ത്രം(path);
    തിരികെ gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(സ്ഥിരം wchar_t *path, സ്ഥിരം അക്ഷരം *mode) {
    തിരികെ gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ -1;
    state = (gz_statep)file;
    എങ്കിൽ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        തിരികെ -1;

    /* make sure we haven't already allocated memory */
    എങ്കിൽ (state->size != 0)
        തിരികെ -1;

    /* check and set requested size */
    എങ്കിൽ ((size << 1) < size)
        തിരികെ -1;              /* need to be able to double it */
    എങ്കിൽ (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    തിരികെ 0;
}

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    എങ്കിൽ (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        തിരികെ -1;

    /* back up and start over */
    എങ്കിൽ (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        തിരികെ -1;
    gz_reset(state);
    തിരികെ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, പൂർണ്ണസംഖ്യ whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ -1;
    state = (gz_statep)file;
    എങ്കിൽ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        തിരികെ -1;

    /* check that there's no error */
    എങ്കിൽ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        തിരികെ -1;

    /* can only seek from start or relative to current position */
    എങ്കിൽ (whence != SEEK_SET && whence != SEEK_CUR)
        തിരികെ -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    എങ്കിൽ (offset > GZ_OFF_MAX - pos)
        തിരികെ -1;                          /* would overflow z_off64_t */
    എങ്കിൽ (offset < -pos)
        തിരികെ -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    എങ്കിൽ (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        എങ്കിൽ (ret == -1)
            തിരികെ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ശൂന്യ);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        തിരികെ pos;
    }

    /* rewind if needed for back seek when reading */
    എങ്കിൽ (pos < state->x.pos) {
        എങ്കിൽ (state->mode != GZ_READ)         /* writing -- can't go backwards */
            തിരികെ -1;
        എങ്കിൽ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            തിരികെ -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    എങ്കിൽ (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    തിരികെ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, പൂർണ്ണസംഖ്യ whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    തിരികെ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ -1;
    state = (gz_statep)file;
    എങ്കിൽ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        തിരികെ -1;

    /* return position */
    തിരികെ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    തിരികെ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ -1;
    state = (gz_statep)file;
    എങ്കിൽ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        തിരികെ -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    എങ്കിൽ (offset == -1)
        തിരികെ -1;
    എങ്കിൽ (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    തിരികെ offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    തിരികെ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
പൂർണ്ണസംഖ്യ ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ 0;
    state = (gz_statep)file;
    എങ്കിൽ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        തിരികെ 0;

    /* return end-of-file state */
    തിരികെ state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
സ്ഥിരം അക്ഷരം * ZEXPORT gzerror(gzFile file, പൂർണ്ണസംഖ്യ *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ ശൂന്യ;
    state = (gz_statep)file;
    എങ്കിൽ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        തിരികെ ശൂന്യ;

    /* return error information */
    എങ്കിൽ (errnum != ശൂന്യ)
        *errnum = state->err;
    തിരികെ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->സന്ദേശം == ശൂന്യ ? "" : state->സന്ദേശം);
}

/* -- see zlib.h -- */
ശൂന്യം ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ;
    state = (gz_statep)file;
    എങ്കിൽ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        തിരികെ;

    /* clear error and end-of-file */
    എങ്കിൽ (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ശൂന്യ);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
ശൂന്യം ZLIB_INTERNAL gz_error(gz_statep state, പൂർണ്ണസംഖ്യ err, സ്ഥിരം അക്ഷരം *സന്ദേശം) {
    /* free previously allocated message and clear */
    എങ്കിൽ (state->സന്ദേശം != ശൂന്യ) {
        എങ്കിൽ (state->err != Z_MEM_ERROR)
            സ്വതന്ത്രം(state->സന്ദേശം);
        state->സന്ദേശം = ശൂന്യ;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    എങ്കിൽ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    എങ്കിൽ (സന്ദേശം == ശൂന്യ)
        തിരികെ;

    /* for an out of memory error, return literal string when requested */
    എങ്കിൽ (err == Z_MEM_ERROR)
        തിരികെ;

    /* construct error message with path */
    എങ്കിൽ ((state->സന്ദേശം = (അക്ഷരം *)malloc(strlen(state->path) + strlen(സന്ദേശം) + 3)) ==
            ശൂന്യ) {
        state->err = Z_MEM_ERROR;
        തിരികെ;
    }
#എങ്കിൽ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ശൂന്യം)snprintf(state->സന്ദേശം, strlen(state->path) + strlen(സന്ദേശം) + 3,
                   "%s%s%s", state->path, ": ", സന്ദേശം);
#അല്ലെങ്കിൽ
    strcpy(state->സന്ദേശം, state->path);
    strcat(state->സന്ദേശം, ": ");
    strcat(state->സന്ദേശം, സന്ദേശം);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(ശൂന്യം) {
#ifdef INT_MAX
    തിരികെ INT_MAX;
#അല്ലെങ്കിൽ
    unsigned p = 1, q;

    ചെയ്യുക {
        q = p;
        p <<= 1;
        p++;
    } ആകുമ്പോൾ (p > q);
    തിരികെ q >> 1;
#endif
}
