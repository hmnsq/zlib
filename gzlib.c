/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

#ᤀᤡ᤺ defined(__DJGPP__)
#  define LSEEK llseek
#elif defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#elif defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#ᤀᤡ᤺ᤆᤡ
#  define LSEEK lseek
#endif

#ᤀᤡ᤺ defined UNDER_CE

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
    ᤀᤡ᤺ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ᤀᤡ᤺ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ᤀᤡ᤺ (chars > parimaa_na (buf) - 1) {
            chars = parimaa_na (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    ᤀᤡ᤺ᤆᤡ {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    ᤀᤡᤴ buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local void gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    ᤀᤡ᤺ (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    ᤀᤡ᤺ᤆᤡ                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, NULL);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(const void *path, puur_naa_nka fd, const var_na *mode) {
    gz_statep state;
    z_size_t len;
    puur_naa_nka oflag = 0;
#ifdef O_EXCL
    puur_naa_nka exclusive = 0;
#endif

    /* check input */
    ᤀᤡ᤺ (path == NULL || mode == NULL)
        ᤀᤡᤴ NULL;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(parimaa_na(gz_state));
    ᤀᤡ᤺ (state == NULL)
        ᤀᤡᤴ NULL;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->msg = NULL;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    ᤀᤠ᤺ (*mode) {
        ᤀᤡ᤺ (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        ᤀᤡ᤺ᤆᤡ
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
                ᤀᤡᤴ NULL;
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
    ᤀᤡ᤺ (state->mode == GZ_NONE) {
        free(state);
        ᤀᤡᤴ NULL;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    ᤀᤡ᤺ (state->mode == GZ_READ) {
        ᤀᤡ᤺ (state->direct == 1) {
            /* can't force a transparent read */
            free(state);
            ᤀᤡᤴ NULL;
        }
        ᤀᤡ᤺ (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        free(state);
        ᤀᤡᤴ NULL;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    ᤀᤡ᤺ (fd == -2)
        len = wcstombs(NULL, path, 0);
    ᤀᤡ᤺ᤆᤡ
#endif
        len = strlen((const var_na *)path);
    state->path = (var_na *)malloc(len + 1);
    ᤀᤡ᤺ (state->path == NULL) {
        free(state);
        ᤀᤡᤴ NULL;
    }
#ifdef WIDECHAR
    ᤀᤡ᤺ (fd == -2) {
        ᤀᤡ᤺ (len)
            wcstombs(state->path, path, len + 1);
        ᤀᤡ᤺ᤆᤡ
            *(state->path) = 0;
    }
    ᤀᤡ᤺ᤆᤡ
#endif
    {
#ᤀᤡ᤺ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (void)snprintf(state->path, len + 1, "%s", (const var_na *)path);
#ᤀᤡ᤺ᤆᤡ
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
    ᤀᤡ᤺ (fd == -1)
        state->fd = open((const var_na *)path, oflag, 0666);
#ifdef WIDECHAR
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    ᤀᤡ᤺ᤆᤡ {
#ifdef O_NONBLOCK
        ᤀᤡ᤺ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        ᤀᤡ᤺ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    ᤀᤡ᤺ (state->fd == -1) {
        free(state->path);
        free(state);
        ᤀᤡᤴ NULL;
    }
    ᤀᤡ᤺ (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    ᤀᤡ᤺ (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        ᤀᤡ᤺ (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ᤀᤡᤴ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(const var_na *path, const var_na *mode) {
    ᤀᤡᤴ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(const var_na *path, const var_na *mode) {
    ᤀᤡᤴ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(puur_naa_nka fd, const var_na *mode) {
    var_na *path;         /* identifier for error messages */
    gzFile gz;

    ᤀᤡ᤺ (fd == -1 || (path = (var_na *)malloc(7 + 3 * parimaa_na(puur_naa_nka))) == NULL)
        ᤀᤡᤴ NULL;
#ᤀᤡ᤺ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(path, 7 + 3 * parimaa_na(puur_naa_nka), "<fd:%d>", fd);
#ᤀᤡ᤺ᤆᤡ
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    free(path);
    ᤀᤡᤴ gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(const wchar_t *path, const var_na *mode) {
    ᤀᤡᤴ gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ -1;
    state = (gz_statep)file;
    ᤀᤡ᤺ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ᤀᤡᤴ -1;

    /* make sure we haven't already allocated memory */
    ᤀᤡ᤺ (state->size != 0)
        ᤀᤡᤴ -1;

    /* check and set requested size */
    ᤀᤡ᤺ ((size << 1) < size)
        ᤀᤡᤴ -1;              /* need to be able to double it */
    ᤀᤡ᤺ (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    ᤀᤡᤴ 0;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    ᤀᤡ᤺ (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ᤀᤡᤴ -1;

    /* back up and start over */
    ᤀᤡ᤺ (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        ᤀᤡᤴ -1;
    gz_reset(state);
    ᤀᤡᤴ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, puur_naa_nka whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ -1;
    state = (gz_statep)file;
    ᤀᤡ᤺ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ᤀᤡᤴ -1;

    /* check that there's no error */
    ᤀᤡ᤺ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ᤀᤡᤴ -1;

    /* can only seek from start or relative to current position */
    ᤀᤡ᤺ (whence != SEEK_SET && whence != SEEK_CUR)
        ᤀᤡᤴ -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    ᤀᤡ᤺ (offset > GZ_OFF_MAX - pos)
        ᤀᤡᤴ -1;                          /* would overflow z_off64_t */
    ᤀᤡ᤺ (offset < -pos)
        ᤀᤡᤴ -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    ᤀᤡ᤺ (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        ᤀᤡ᤺ (ret == -1)
            ᤀᤡᤴ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, NULL);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        ᤀᤡᤴ pos;
    }

    /* rewind if needed for back seek when reading */
    ᤀᤡ᤺ (pos < state->x.pos) {
        ᤀᤡ᤺ (state->mode != GZ_READ)         /* writing -- can't go backwards */
            ᤀᤡᤴ -1;
        ᤀᤡ᤺ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ᤀᤡᤴ -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    ᤀᤡ᤺ (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    ᤀᤡᤴ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, puur_naa_nka whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    ᤀᤡᤴ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ -1;
    state = (gz_statep)file;
    ᤀᤡ᤺ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ᤀᤡᤴ -1;

    /* return position */
    ᤀᤡᤴ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ᤀᤡᤴ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ -1;
    state = (gz_statep)file;
    ᤀᤡ᤺ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ᤀᤡᤴ -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    ᤀᤡ᤺ (offset == -1)
        ᤀᤡᤴ -1;
    ᤀᤡ᤺ (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    ᤀᤡᤴ offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ᤀᤡᤴ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
puur_naa_nka ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ 0;
    state = (gz_statep)file;
    ᤀᤡ᤺ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ᤀᤡᤴ 0;

    /* return end-of-file state */
    ᤀᤡᤴ state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
const var_na * ZEXPORT gzerror(gzFile file, puur_naa_nka *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ NULL;
    state = (gz_statep)file;
    ᤀᤡ᤺ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ᤀᤡᤴ NULL;

    /* return error information */
    ᤀᤡ᤺ (errnum != NULL)
        *errnum = state->err;
    ᤀᤡᤴ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->msg == NULL ? "" : state->msg);
}

/* -- see zlib.h -- */
void ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ;
    state = (gz_statep)file;
    ᤀᤡ᤺ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ᤀᤡᤴ;

    /* clear error and end-of-file */
    ᤀᤡ᤺ (state->mode == GZ_READ) {
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
void ZLIB_INTERNAL gz_error(gz_statep state, puur_naa_nka err, const var_na *msg) {
    /* free previously allocated message and clear */
    ᤀᤡ᤺ (state->msg != NULL) {
        ᤀᤡ᤺ (state->err != Z_MEM_ERROR)
            free(state->msg);
        state->msg = NULL;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    ᤀᤡ᤺ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    ᤀᤡ᤺ (msg == NULL)
        ᤀᤡᤴ;

    /* for an out of memory error, return literal string when requested */
    ᤀᤡ᤺ (err == Z_MEM_ERROR)
        ᤀᤡᤴ;

    /* construct error message with path */
    ᤀᤡ᤺ ((state->msg = (var_na *)malloc(strlen(state->path) + strlen(msg) + 3)) ==
            NULL) {
        state->err = Z_MEM_ERROR;
        ᤀᤡᤴ;
    }
#ᤀᤡ᤺ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(state->msg, strlen(state->path) + strlen(msg) + 3,
                   "%s%s%s", state->path, ": ", msg);
#ᤀᤡ᤺ᤆᤡ
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
    ᤀᤡᤴ INT_MAX;
#ᤀᤡ᤺ᤆᤡ
    unsigned p = 1, q;

    do {
        q = p;
        p <<= 1;
        p++;
    } ᤀᤠ᤺ (p > q);
    ᤀᤡᤴ q >> 1;
#endif
}
