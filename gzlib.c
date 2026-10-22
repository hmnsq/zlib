/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

#如果 defined(__DJGPP__)
#  define LSEEK llseek
#elif defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#elif defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#否则
#  define LSEEK lseek
#endif

#如果 defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
char ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    static char buf[1024];

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
    如果 (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        如果 (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        如果 (chars > sizeof (buf) - 1) {
            chars = sizeof (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    否则 {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    返回 buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local void gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    如果 (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    否则                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, NULL);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(const void *path, 整数 fd, const char *mode) {
    gz_statep state;
    z_size_t len;
    整数 oflag = 0;
#ifdef O_EXCL
    整数 exclusive = 0;
#endif

    /* check input */
    如果 (path == NULL || mode == NULL)
        返回 NULL;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(sizeof(gz_state));
    如果 (state == NULL)
        返回 NULL;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->msg = NULL;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    当 (*mode) {
        如果 (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        否则
            switch (*mode) {
            case 'r':
                state->mode = GZ_READ;
                break;
#ifndef NO_GZCOMPRESS
            case 'w':
                state->mode = GZ_WRITE;
                break;
            case 'a':
                state->mode = GZ_APPEND;
                break;
#endif
            case '+':       /* can't read and write at the same time */
                free(state);
                返回 NULL;
            case 'b':       /* ignore -- will request binary anyway */
                break;
#ifdef O_CLOEXEC
            case 'e':
                oflag |= O_CLOEXEC;
                break;
#endif
#ifdef O_EXCL
            case 'x':
                exclusive = 1;
                break;
#endif
            case 'f':
                state->strategy = Z_FILTERED;
                break;
            case 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                break;
            case 'R':
                state->strategy = Z_RLE;
                break;
            case 'F':
                state->strategy = Z_FIXED;
                break;
            case 'G':
                state->direct = -1;
                break;
#ifdef O_NONBLOCK
            case 'N':
                oflag |= O_NONBLOCK;
                break;
#endif
            case 'T':
                state->direct = 1;
                break;
            default:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    如果 (state->mode == GZ_NONE) {
        free(state);
        返回 NULL;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    如果 (state->mode == GZ_READ) {
        如果 (state->direct == 1) {
            /* can't force a transparent read */
            free(state);
            返回 NULL;
        }
        如果 (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    否则 如果 (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        free(state);
        返回 NULL;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    如果 (fd == -2)
        len = wcstombs(NULL, path, 0);
    否则
#endif
        len = strlen((const char *)path);
    state->path = (char *)malloc(len + 1);
    如果 (state->path == NULL) {
        free(state);
        返回 NULL;
    }
#ifdef WIDECHAR
    如果 (fd == -2) {
        如果 (len)
            wcstombs(state->path, path, len + 1);
        否则
            *(state->path) = 0;
    }
    否则
#endif
    {
#如果 !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (void)snprintf(state->path, len + 1, "%s", (const char *)path);
#否则
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
    如果 (fd == -1)
        state->fd = open((const char *)path, oflag, 0666);
#ifdef WIDECHAR
    否则 如果 (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    否则 {
#ifdef O_NONBLOCK
        如果 (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        如果 (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    如果 (state->fd == -1) {
        free(state->path);
        free(state);
        返回 NULL;
    }
    如果 (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    如果 (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        如果 (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    返回 (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(const char *path, const char *mode) {
    返回 gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(const char *path, const char *mode) {
    返回 gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(整数 fd, const char *mode) {
    char *path;         /* identifier for error messages */
    gzFile gz;

    如果 (fd == -1 || (path = (char *)malloc(7 + 3 * sizeof(整数))) == NULL)
        返回 NULL;
#如果 !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(path, 7 + 3 * sizeof(整数), "<fd:%d>", fd);
#否则
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    free(path);
    返回 gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(const wchar_t *path, const char *mode) {
    返回 gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
整数 ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    如果 (file == NULL)
        返回 -1;
    state = (gz_statep)file;
    如果 (state->mode != GZ_READ && state->mode != GZ_WRITE)
        返回 -1;

    /* make sure we haven't already allocated memory */
    如果 (state->size != 0)
        返回 -1;

    /* check and set requested size */
    如果 ((size << 1) < size)
        返回 -1;              /* need to be able to double it */
    如果 (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    返回 0;
}

/* -- see zlib.h -- */
整数 ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    如果 (file == NULL)
        返回 -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    如果 (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        返回 -1;

    /* back up and start over */
    如果 (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        返回 -1;
    gz_reset(state);
    返回 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, 整数 whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    如果 (file == NULL)
        返回 -1;
    state = (gz_statep)file;
    如果 (state->mode != GZ_READ && state->mode != GZ_WRITE)
        返回 -1;

    /* check that there's no error */
    如果 (state->err != Z_OK && state->err != Z_BUF_ERROR)
        返回 -1;

    /* can only seek from start or relative to current position */
    如果 (whence != SEEK_SET && whence != SEEK_CUR)
        返回 -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    如果 (offset > GZ_OFF_MAX - pos)
        返回 -1;                          /* would overflow z_off64_t */
    如果 (offset < -pos)
        返回 -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    如果 (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        如果 (ret == -1)
            返回 -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, NULL);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        返回 pos;
    }

    /* rewind if needed for back seek when reading */
    如果 (pos < state->x.pos) {
        如果 (state->mode != GZ_READ)         /* writing -- can't go backwards */
            返回 -1;
        如果 (gzrewind(file) == -1)           /* rewind, then skip to pos */
            返回 -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    如果 (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    返回 pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, 整数 whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    返回 ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    如果 (file == NULL)
        返回 -1;
    state = (gz_statep)file;
    如果 (state->mode != GZ_READ && state->mode != GZ_WRITE)
        返回 -1;

    /* return position */
    返回 state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    返回 ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    如果 (file == NULL)
        返回 -1;
    state = (gz_statep)file;
    如果 (state->mode != GZ_READ && state->mode != GZ_WRITE)
        返回 -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    如果 (offset == -1)
        返回 -1;
    如果 (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    返回 offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    返回 ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
整数 ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    如果 (file == NULL)
        返回 0;
    state = (gz_statep)file;
    如果 (state->mode != GZ_READ && state->mode != GZ_WRITE)
        返回 0;

    /* return end-of-file state */
    返回 state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
const char * ZEXPORT gzerror(gzFile file, 整数 *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    如果 (file == NULL)
        返回 NULL;
    state = (gz_statep)file;
    如果 (state->mode != GZ_READ && state->mode != GZ_WRITE)
        返回 NULL;

    /* return error information */
    如果 (errnum != NULL)
        *errnum = state->err;
    返回 state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->msg == NULL ? "" : state->msg);
}

/* -- see zlib.h -- */
void ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    如果 (file == NULL)
        返回;
    state = (gz_statep)file;
    如果 (state->mode != GZ_READ && state->mode != GZ_WRITE)
        返回;

    /* clear error and end-of-file */
    如果 (state->mode == GZ_READ) {
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
void ZLIB_INTERNAL gz_error(gz_statep state, 整数 err, const char *msg) {
    /* free previously allocated message and clear */
    如果 (state->msg != NULL) {
        如果 (state->err != Z_MEM_ERROR)
            free(state->msg);
        state->msg = NULL;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    如果 (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    如果 (msg == NULL)
        返回;

    /* for an out of memory error, return literal string when requested */
    如果 (err == Z_MEM_ERROR)
        返回;

    /* construct error message with path */
    如果 ((state->msg = (char *)malloc(strlen(state->path) + strlen(msg) + 3)) ==
            NULL) {
        state->err = Z_MEM_ERROR;
        返回;
    }
#如果 !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(state->msg, strlen(state->path) + strlen(msg) + 3,
                   "%s%s%s", state->path, ": ", msg);
#否则
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
    返回 INT_MAX;
#否则
    unsigned p = 1, q;

    do {
        q = p;
        p <<= 1;
        p++;
    } 当 (p > q);
    返回 q >> 1;
#endif
}
