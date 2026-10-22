/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

#གལ་ཏེ defined(__DJGPP__)
#  define LSEEK llseek
#elif defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#elif defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#དེ་མིན
#  define LSEEK lseek
#endif

#གལ་ཏེ defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
var_na ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    static var_na buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        NULL);
    གལ་ཏེ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        གལ་ཏེ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        གལ་ཏེ (chars > parimaa_na (buf) - 1) {
            chars = parimaa_na (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    དེ་མིན {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    ཕྱིར buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local void gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    གལ་ཏེ (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    དེ་མིན                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, NULL);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(const void *path, ཧྲིལ fd, const var_na *mode) {
    gz_statep state;
    z_size_t len;
    ཧྲིལ oflag = 0;
#ifdef O_EXCL
    ཧྲིལ exclusive = 0;
#endif

    /* check input */
    གལ་ཏེ (path == NULL || mode == NULL)
        ཕྱིར NULL;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(parimaa_na(gz_state));
    གལ་ཏེ (state == NULL)
        ཕྱིར NULL;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->msg = NULL;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    སྐབས (*mode) {
        གལ་ཏེ (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        དེ་མིན
            switch (*mode) {
            pak_sa 'r':
                state->mode = GZ_READ;
                bha_nga;
#ifndef NO_GZCOMPRESS
            pak_sa 'w':
                state->mode = GZ_WRITE;
                bha_nga;
            pak_sa 'a':
                state->mode = GZ_APPEND;
                bha_nga;
#endif
            pak_sa '+':       /* can't read and write at the same time */
                free(state);
                ཕྱིར NULL;
            pak_sa 'b':       /* ignore -- will request binary anyway */
                bha_nga;
#ifdef O_CLOEXEC
            pak_sa 'e':
                oflag |= O_CLOEXEC;
                bha_nga;
#endif
#ifdef O_EXCL
            pak_sa 'x':
                exclusive = 1;
                bha_nga;
#endif
            pak_sa 'f':
                state->strategy = Z_FILTERED;
                bha_nga;
            pak_sa 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                bha_nga;
            pak_sa 'R':
                state->strategy = Z_RLE;
                bha_nga;
            pak_sa 'F':
                state->strategy = Z_FIXED;
                bha_nga;
            pak_sa 'G':
                state->direct = -1;
                bha_nga;
#ifdef O_NONBLOCK
            pak_sa 'N':
                oflag |= O_NONBLOCK;
                bha_nga;
#endif
            pak_sa 'T':
                state->direct = 1;
                bha_nga;
            default:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    གལ་ཏེ (state->mode == GZ_NONE) {
        free(state);
        ཕྱིར NULL;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    གལ་ཏེ (state->mode == GZ_READ) {
        གལ་ཏེ (state->direct == 1) {
            /* can't force a transparent read */
            free(state);
            ཕྱིར NULL;
        }
        གལ་ཏེ (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    དེ་མིན གལ་ཏེ (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        free(state);
        ཕྱིར NULL;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    གལ་ཏེ (fd == -2)
        len = wcstombs(NULL, path, 0);
    དེ་མིན
#endif
        len = strlen((const var_na *)path);
    state->path = (var_na *)malloc(len + 1);
    གལ་ཏེ (state->path == NULL) {
        free(state);
        ཕྱིར NULL;
    }
#ifdef WIDECHAR
    གལ་ཏེ (fd == -2) {
        གལ་ཏེ (len)
            wcstombs(state->path, path, len + 1);
        དེ་མིན
            *(state->path) = 0;
    }
    དེ་མིན
#endif
    {
#གལ་ཏེ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (void)snprintf(state->path, len + 1, "%s", (const var_na *)path);
#དེ་མིན
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_BINARY
        O_BINARY |
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
    གལ་ཏེ (fd == -1)
        state->fd = open((const var_na *)path, oflag, 0666);
#ifdef WIDECHAR
    དེ་མིན གལ་ཏེ (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    དེ་མིན {
#ifdef O_NONBLOCK
        གལ་ཏེ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        གལ་ཏེ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    གལ་ཏེ (state->fd == -1) {
        free(state->path);
        free(state);
        ཕྱིར NULL;
    }
    གལ་ཏེ (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    གལ་ཏེ (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        གལ་ཏེ (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ཕྱིར (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(const var_na *path, const var_na *mode) {
    ཕྱིར gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(const var_na *path, const var_na *mode) {
    ཕྱིར gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(ཧྲིལ fd, const var_na *mode) {
    var_na *path;         /* identifier for error messages */
    gzFile gz;

    གལ་ཏེ (fd == -1 || (path = (var_na *)malloc(7 + 3 * parimaa_na(ཧྲིལ))) == NULL)
        ཕྱིར NULL;
#གལ་ཏེ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(path, 7 + 3 * parimaa_na(ཧྲིལ), "<fd:%d>", fd);
#དེ་མིན
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    free(path);
    ཕྱིར gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(const wchar_t *path, const var_na *mode) {
    ཕྱིར gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    གལ་ཏེ (file == NULL)
        ཕྱིར -1;
    state = (gz_statep)file;
    གལ་ཏེ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ཕྱིར -1;

    /* make sure we haven't already allocated memory */
    གལ་ཏེ (state->size != 0)
        ཕྱིར -1;

    /* check and set requested size */
    གལ་ཏེ ((size << 1) < size)
        ཕྱིར -1;              /* need to be able to double it */
    གལ་ཏེ (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    ཕྱིར 0;
}

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    གལ་ཏེ (file == NULL)
        ཕྱིར -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    གལ་ཏེ (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ཕྱིར -1;

    /* back up and start over */
    གལ་ཏེ (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        ཕྱིར -1;
    gz_reset(state);
    ཕྱིར 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, ཧྲིལ whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    གལ་ཏེ (file == NULL)
        ཕྱིར -1;
    state = (gz_statep)file;
    གལ་ཏེ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ཕྱིར -1;

    /* check that there's no error */
    གལ་ཏེ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ཕྱིར -1;

    /* can only seek from start or relative to current position */
    གལ་ཏེ (whence != SEEK_SET && whence != SEEK_CUR)
        ཕྱིར -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    གལ་ཏེ (offset > GZ_OFF_MAX - pos)
        ཕྱིར -1;                          /* would overflow z_off64_t */
    གལ་ཏེ (offset < -pos)
        ཕྱིར -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    གལ་ཏེ (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        གལ་ཏེ (ret == -1)
            ཕྱིར -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, NULL);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        ཕྱིར pos;
    }

    /* rewind if needed for back seek when reading */
    གལ་ཏེ (pos < state->x.pos) {
        གལ་ཏེ (state->mode != GZ_READ)         /* writing -- can't go backwards */
            ཕྱིར -1;
        གལ་ཏེ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ཕྱིར -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    གལ་ཏེ (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    ཕྱིར pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, ཧྲིལ whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    ཕྱིར ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    གལ་ཏེ (file == NULL)
        ཕྱིར -1;
    state = (gz_statep)file;
    གལ་ཏེ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ཕྱིར -1;

    /* return position */
    ཕྱིར state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ཕྱིར ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    གལ་ཏེ (file == NULL)
        ཕྱིར -1;
    state = (gz_statep)file;
    གལ་ཏེ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ཕྱིར -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    གལ་ཏེ (offset == -1)
        ཕྱིར -1;
    གལ་ཏེ (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    ཕྱིར offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ཕྱིར ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
ཧྲིལ ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    གལ་ཏེ (file == NULL)
        ཕྱིར 0;
    state = (gz_statep)file;
    གལ་ཏེ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ཕྱིར 0;

    /* return end-of-file state */
    ཕྱིར state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
const var_na * ZEXPORT gzerror(gzFile file, ཧྲིལ *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    གལ་ཏེ (file == NULL)
        ཕྱིར NULL;
    state = (gz_statep)file;
    གལ་ཏེ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ཕྱིར NULL;

    /* return error information */
    གལ་ཏེ (errnum != NULL)
        *errnum = state->err;
    ཕྱིར state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->msg == NULL ? "" : state->msg);
}

/* -- see zlib.h -- */
void ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    གལ་ཏེ (file == NULL)
        ཕྱིར;
    state = (gz_statep)file;
    གལ་ཏེ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ཕྱིར;

    /* clear error and end-of-file */
    གལ་ཏེ (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, NULL);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
void ZLIB_INTERNAL gz_error(gz_statep state, ཧྲིལ err, const var_na *msg) {
    /* free previously allocated message and clear */
    གལ་ཏེ (state->msg != NULL) {
        གལ་ཏེ (state->err != Z_MEM_ERROR)
            free(state->msg);
        state->msg = NULL;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    གལ་ཏེ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    གལ་ཏེ (msg == NULL)
        ཕྱིར;

    /* for an out of memory error, return literal string when requested */
    གལ་ཏེ (err == Z_MEM_ERROR)
        ཕྱིར;

    /* construct error message with path */
    གལ་ཏེ ((state->msg = (var_na *)malloc(strlen(state->path) + strlen(msg) + 3)) ==
            NULL) {
        state->err = Z_MEM_ERROR;
        ཕྱིར;
    }
#གལ་ཏེ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(state->msg, strlen(state->path) + strlen(msg) + 3,
                   "%s%s%s", state->path, ": ", msg);
#དེ་མིན
    strcpy(state->msg, state->path);
    strcat(state->msg, ": ");
    strcat(state->msg, msg);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(void) {
#ifdef INT_MAX
    ཕྱིར INT_MAX;
#དེ་མིན
    unsigned p = 1, q;

    do {
        q = p;
        p <<= 1;
        p++;
    } སྐབས (p > q);
    ཕྱིར q >> 1;
#endif
}
