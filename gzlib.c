/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "gzguts.h"

#اگر defined(__DJGPP__)
#  define LSEEK llseek
#وگرنهاگر defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#وگرنهاگر defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#وگرنه
#  define LSEEK lseek
#endif

#اگر defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
نویسه ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    ایستا نویسه buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        پوچ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        پوچ);
    اگر (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        اگر (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        اگر (chars > اندازه (buf) - 1) {
            chars = اندازه (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    وگرنه {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    بازگشت buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local تهی gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    اگر (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    وگرنه                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, پوچ);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ثابت تهی *path, صحیح fd, ثابت نویسه *mode) {
    gz_statep state;
    z_size_t len;
    صحیح oflag = 0;
#ifdef O_EXCL
    صحیح exclusive = 0;
#endif

    /* check input */
    اگر (path == پوچ || mode == پوچ)
        بازگشت پوچ;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(اندازه(gz_state));
    اگر (state == پوچ)
        بازگشت پوچ;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->پیام = پوچ;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    وقتی (*mode) {
        اگر (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        وگرنه
            انتخاب (*mode) {
            حالت 'r':
                state->mode = GZ_READ;
                بشکن;
#ifndef NO_GZCOMPRESS
            حالت 'w':
                state->mode = GZ_WRITE;
                بشکن;
            حالت 'a':
                state->mode = GZ_APPEND;
                بشکن;
#endif
            حالت '+':       /* can't read and write at the same time */
                آزاد(state);
                بازگشت پوچ;
            حالت 'b':       /* ignore -- will request binary anyway */
                بشکن;
#ifdef O_CLOEXEC
            حالت 'e':
                oflag |= O_CLOEXEC;
                بشکن;
#endif
#ifdef O_EXCL
            حالت 'x':
                exclusive = 1;
                بشکن;
#endif
            حالت 'f':
                state->strategy = Z_FILTERED;
                بشکن;
            حالت 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                بشکن;
            حالت 'R':
                state->strategy = Z_RLE;
                بشکن;
            حالت 'F':
                state->strategy = Z_FIXED;
                بشکن;
            حالت 'G':
                state->direct = -1;
                بشکن;
#ifdef O_NONBLOCK
            حالت 'N':
                oflag |= O_NONBLOCK;
                بشکن;
#endif
            حالت 'T':
                state->direct = 1;
                بشکن;
            پیشفرض:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    اگر (state->mode == GZ_NONE) {
        آزاد(state);
        بازگشت پوچ;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    اگر (state->mode == GZ_READ) {
        اگر (state->direct == 1) {
            /* can't force a transparent read */
            آزاد(state);
            بازگشت پوچ;
        }
        اگر (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    وگرنه اگر (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        آزاد(state);
        بازگشت پوچ;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    اگر (fd == -2)
        len = wcstombs(پوچ, path, 0);
    وگرنه
#endif
        len = strlen((ثابت نویسه *)path);
    state->path = (نویسه *)malloc(len + 1);
    اگر (state->path == پوچ) {
        آزاد(state);
        بازگشت پوچ;
    }
#ifdef WIDECHAR
    اگر (fd == -2) {
        اگر (len)
            wcstombs(state->path, path, len + 1);
        وگرنه
            *(state->path) = 0;
    }
    وگرنه
#endif
    {
#اگر !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (تهی)snprintf(state->path, len + 1, "%s", (ثابت نویسه *)path);
#وگرنه
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_دودویی
        O_دودویی |
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
    اگر (fd == -1)
        state->fd = open((ثابت نویسه *)path, oflag, 0666);
#ifdef WIDECHAR
    وگرنه اگر (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    وگرنه {
#ifdef O_NONBLOCK
        اگر (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        اگر (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    اگر (state->fd == -1) {
        آزاد(state->path);
        آزاد(state);
        بازگشت پوچ;
    }
    اگر (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    اگر (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        اگر (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    بازگشت (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ثابت نویسه *path, ثابت نویسه *mode) {
    بازگشت gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ثابت نویسه *path, ثابت نویسه *mode) {
    بازگشت gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(صحیح fd, ثابت نویسه *mode) {
    نویسه *path;         /* identifier for error messages */
    gzFile gz;

    اگر (fd == -1 || (path = (نویسه *)malloc(7 + 3 * اندازه(صحیح))) == پوچ)
        بازگشت پوچ;
#اگر !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (تهی)snprintf(path, 7 + 3 * اندازه(صحیح), "<fd:%d>", fd);
#وگرنه
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    آزاد(path);
    بازگشت gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(ثابت wchar_t *path, ثابت نویسه *mode) {
    بازگشت gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
صحیح ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ && state->mode != GZ_WRITE)
        بازگشت -1;

    /* make sure we haven't already allocated memory */
    اگر (state->size != 0)
        بازگشت -1;

    /* check and set requested size */
    اگر ((size << 1) < size)
        بازگشت -1;              /* need to be able to double it */
    اگر (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    بازگشت 0;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    اگر (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        بازگشت -1;

    /* back up and start over */
    اگر (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        بازگشت -1;
    gz_reset(state);
    بازگشت 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, صحیح whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ && state->mode != GZ_WRITE)
        بازگشت -1;

    /* check that there's no error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR)
        بازگشت -1;

    /* can only seek from start or relative to current position */
    اگر (whence != SEEK_SET && whence != SEEK_CUR)
        بازگشت -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    اگر (offset > GZ_OFF_MAX - pos)
        بازگشت -1;                          /* would overflow z_off64_t */
    اگر (offset < -pos)
        بازگشت -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    اگر (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        اگر (ret == -1)
            بازگشت -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, پوچ);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        بازگشت pos;
    }

    /* rewind if needed for back seek when reading */
    اگر (pos < state->x.pos) {
        اگر (state->mode != GZ_READ)         /* writing -- can't go backwards */
            بازگشت -1;
        اگر (gzrewind(file) == -1)           /* rewind, then skip to pos */
            بازگشت -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    اگر (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    بازگشت pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, صحیح whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    بازگشت ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ && state->mode != GZ_WRITE)
        بازگشت -1;

    /* return position */
    بازگشت state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    بازگشت ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == پوچ)
        بازگشت -1;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ && state->mode != GZ_WRITE)
        بازگشت -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    اگر (offset == -1)
        بازگشت -1;
    اگر (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    بازگشت offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    بازگشت ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == پوچ)
        بازگشت 0;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ && state->mode != GZ_WRITE)
        بازگشت 0;

    /* return end-of-file state */
    بازگشت state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ثابت نویسه * ZEXPORT gzerror(gzFile file, صحیح *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == پوچ)
        بازگشت پوچ;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ && state->mode != GZ_WRITE)
        بازگشت پوچ;

    /* return error information */
    اگر (errnum != پوچ)
        *errnum = state->err;
    بازگشت state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->پیام == پوچ ? "" : state->پیام);
}

/* -- see zlib.h -- */
تهی ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == پوچ)
        بازگشت;
    state = (gz_statep)file;
    اگر (state->mode != GZ_READ && state->mode != GZ_WRITE)
        بازگشت;

    /* clear error and end-of-file */
    اگر (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, پوچ);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
تهی ZLIB_INTERNAL gz_error(gz_statep state, صحیح err, ثابت نویسه *پیام) {
    /* free previously allocated message and clear */
    اگر (state->پیام != پوچ) {
        اگر (state->err != Z_MEM_ERROR)
            آزاد(state->پیام);
        state->پیام = پوچ;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    اگر (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    اگر (پیام == پوچ)
        بازگشت;

    /* for an out of memory error, return literal string when requested */
    اگر (err == Z_MEM_ERROR)
        بازگشت;

    /* construct error message with path */
    اگر ((state->پیام = (نویسه *)malloc(strlen(state->path) + strlen(پیام) + 3)) ==
            پوچ) {
        state->err = Z_MEM_ERROR;
        بازگشت;
    }
#اگر !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (تهی)snprintf(state->پیام, strlen(state->path) + strlen(پیام) + 3,
                   "%s%s%s", state->path, ": ", پیام);
#وگرنه
    strcpy(state->پیام, state->path);
    strcat(state->پیام, ": ");
    strcat(state->پیام, پیام);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(تهی) {
#ifdef INT_MAX
    بازگشت INT_MAX;
#وگرنه
    unsigned p = 1, q;

    انجامبده {
        q = p;
        p <<= 1;
        p++;
    } وقتی (p > q);
    بازگشت q >> 1;
#endif
}
