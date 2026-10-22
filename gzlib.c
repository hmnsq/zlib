/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

#ຖ້າ defined(__DJGPP__)
#  define LSEEK llseek
#elif defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#elif defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#ບໍ່ດັ່ງນັ້ນ
#  define LSEEK lseek
#endif

#ຖ້າ defined UNDER_CE

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
    ຖ້າ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ຖ້າ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ຖ້າ (chars > parimaa_na (buf) - 1) {
            chars = parimaa_na (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    ບໍ່ດັ່ງນັ້ນ {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    ສົ່ງຄືນ buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local void gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    ຖ້າ (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    ບໍ່ດັ່ງນັ້ນ                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, NULL);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(const void *path, ຈຳນວນເຕັມ fd, const var_na *mode) {
    gz_statep state;
    z_size_t len;
    ຈຳນວນເຕັມ oflag = 0;
#ifdef O_EXCL
    ຈຳນວນເຕັມ exclusive = 0;
#endif

    /* check input */
    ຖ້າ (path == NULL || mode == NULL)
        ສົ່ງຄືນ NULL;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(parimaa_na(gz_state));
    ຖ້າ (state == NULL)
        ສົ່ງຄືນ NULL;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->msg = NULL;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    ໃນຂະນະທີ່ (*mode) {
        ຖ້າ (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        ບໍ່ດັ່ງນັ້ນ
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
                ສົ່ງຄືນ NULL;
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
    ຖ້າ (state->mode == GZ_NONE) {
        free(state);
        ສົ່ງຄືນ NULL;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    ຖ້າ (state->mode == GZ_READ) {
        ຖ້າ (state->direct == 1) {
            /* can't force a transparent read */
            free(state);
            ສົ່ງຄືນ NULL;
        }
        ຖ້າ (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        free(state);
        ສົ່ງຄືນ NULL;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    ຖ້າ (fd == -2)
        len = wcstombs(NULL, path, 0);
    ບໍ່ດັ່ງນັ້ນ
#endif
        len = strlen((const var_na *)path);
    state->path = (var_na *)malloc(len + 1);
    ຖ້າ (state->path == NULL) {
        free(state);
        ສົ່ງຄືນ NULL;
    }
#ifdef WIDECHAR
    ຖ້າ (fd == -2) {
        ຖ້າ (len)
            wcstombs(state->path, path, len + 1);
        ບໍ່ດັ່ງນັ້ນ
            *(state->path) = 0;
    }
    ບໍ່ດັ່ງນັ້ນ
#endif
    {
#ຖ້າ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (void)snprintf(state->path, len + 1, "%s", (const var_na *)path);
#ບໍ່ດັ່ງນັ້ນ
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
    ຖ້າ (fd == -1)
        state->fd = open((const var_na *)path, oflag, 0666);
#ifdef WIDECHAR
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    ບໍ່ດັ່ງນັ້ນ {
#ifdef O_NONBLOCK
        ຖ້າ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        ຖ້າ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    ຖ້າ (state->fd == -1) {
        free(state->path);
        free(state);
        ສົ່ງຄືນ NULL;
    }
    ຖ້າ (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    ຖ້າ (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        ຖ້າ (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ສົ່ງຄືນ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(const var_na *path, const var_na *mode) {
    ສົ່ງຄືນ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(const var_na *path, const var_na *mode) {
    ສົ່ງຄືນ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(ຈຳນວນເຕັມ fd, const var_na *mode) {
    var_na *path;         /* identifier for error messages */
    gzFile gz;

    ຖ້າ (fd == -1 || (path = (var_na *)malloc(7 + 3 * parimaa_na(ຈຳນວນເຕັມ))) == NULL)
        ສົ່ງຄືນ NULL;
#ຖ້າ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(path, 7 + 3 * parimaa_na(ຈຳນວນເຕັມ), "<fd:%d>", fd);
#ບໍ່ດັ່ງນັ້ນ
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    free(path);
    ສົ່ງຄືນ gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(const wchar_t *path, const var_na *mode) {
    ສົ່ງຄືນ gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ -1;
    state = (gz_statep)file;
    ຖ້າ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ສົ່ງຄືນ -1;

    /* make sure we haven't already allocated memory */
    ຖ້າ (state->size != 0)
        ສົ່ງຄືນ -1;

    /* check and set requested size */
    ຖ້າ ((size << 1) < size)
        ສົ່ງຄືນ -1;              /* need to be able to double it */
    ຖ້າ (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    ສົ່ງຄືນ 0;
}

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    ຖ້າ (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ສົ່ງຄືນ -1;

    /* back up and start over */
    ຖ້າ (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        ສົ່ງຄືນ -1;
    gz_reset(state);
    ສົ່ງຄືນ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, ຈຳນວນເຕັມ whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ -1;
    state = (gz_statep)file;
    ຖ້າ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ສົ່ງຄືນ -1;

    /* check that there's no error */
    ຖ້າ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ສົ່ງຄືນ -1;

    /* can only seek from start or relative to current position */
    ຖ້າ (whence != SEEK_SET && whence != SEEK_CUR)
        ສົ່ງຄືນ -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    ຖ້າ (offset > GZ_OFF_MAX - pos)
        ສົ່ງຄືນ -1;                          /* would overflow z_off64_t */
    ຖ້າ (offset < -pos)
        ສົ່ງຄືນ -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    ຖ້າ (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        ຖ້າ (ret == -1)
            ສົ່ງຄືນ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, NULL);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        ສົ່ງຄືນ pos;
    }

    /* rewind if needed for back seek when reading */
    ຖ້າ (pos < state->x.pos) {
        ຖ້າ (state->mode != GZ_READ)         /* writing -- can't go backwards */
            ສົ່ງຄືນ -1;
        ຖ້າ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ສົ່ງຄືນ -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    ຖ້າ (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    ສົ່ງຄືນ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, ຈຳນວນເຕັມ whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    ສົ່ງຄືນ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ -1;
    state = (gz_statep)file;
    ຖ້າ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ສົ່ງຄືນ -1;

    /* return position */
    ສົ່ງຄືນ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ສົ່ງຄືນ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ -1;
    state = (gz_statep)file;
    ຖ້າ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ສົ່ງຄືນ -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    ຖ້າ (offset == -1)
        ສົ່ງຄືນ -1;
    ຖ້າ (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    ສົ່ງຄືນ offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ສົ່ງຄືນ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
ຈຳນວນເຕັມ ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ 0;
    state = (gz_statep)file;
    ຖ້າ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ສົ່ງຄືນ 0;

    /* return end-of-file state */
    ສົ່ງຄືນ state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
const var_na * ZEXPORT gzerror(gzFile file, ຈຳນວນເຕັມ *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ NULL;
    state = (gz_statep)file;
    ຖ້າ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ສົ່ງຄືນ NULL;

    /* return error information */
    ຖ້າ (errnum != NULL)
        *errnum = state->err;
    ສົ່ງຄືນ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->msg == NULL ? "" : state->msg);
}

/* -- see zlib.h -- */
void ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ຖ້າ (file == NULL)
        ສົ່ງຄືນ;
    state = (gz_statep)file;
    ຖ້າ (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ສົ່ງຄືນ;

    /* clear error and end-of-file */
    ຖ້າ (state->mode == GZ_READ) {
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
void ZLIB_INTERNAL gz_error(gz_statep state, ຈຳນວນເຕັມ err, const var_na *msg) {
    /* free previously allocated message and clear */
    ຖ້າ (state->msg != NULL) {
        ຖ້າ (state->err != Z_MEM_ERROR)
            free(state->msg);
        state->msg = NULL;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    ຖ້າ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    ຖ້າ (msg == NULL)
        ສົ່ງຄືນ;

    /* for an out of memory error, return literal string when requested */
    ຖ້າ (err == Z_MEM_ERROR)
        ສົ່ງຄືນ;

    /* construct error message with path */
    ຖ້າ ((state->msg = (var_na *)malloc(strlen(state->path) + strlen(msg) + 3)) ==
            NULL) {
        state->err = Z_MEM_ERROR;
        ສົ່ງຄືນ;
    }
#ຖ້າ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (void)snprintf(state->msg, strlen(state->path) + strlen(msg) + 3,
                   "%s%s%s", state->path, ": ", msg);
#ບໍ່ດັ່ງນັ້ນ
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
    ສົ່ງຄືນ INT_MAX;
#ບໍ່ດັ່ງນັ້ນ
    unsigned p = 1, q;

    do {
        q = p;
        p <<= 1;
        p++;
    } ໃນຂະນະທີ່ (p > q);
    ສົ່ງຄືນ q >> 1;
#endif
}
