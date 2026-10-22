/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

#إذا defined(__DJGPP__)
#  define LSEEK llseek
#وإلاإذا defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#وإلاإذا defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#وإلا
#  define LSEEK lseek
#endif

#إذا defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
حرف ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    ساكن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        عدم,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        عدم);
    إذا (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        إذا (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        إذا (chars > حجم (buf) - 1) {
            chars = حجم (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    وإلا {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    أرجع buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local فراغ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    إذا (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    وإلا                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, عدم);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ثابت فراغ *path, صحيح fd, ثابت حرف *mode) {
    gz_statep state;
    z_size_t len;
    صحيح oflag = 0;
#ifdef O_EXCL
    صحيح exclusive = 0;
#endif

    /* check input */
    إذا (path == عدم || mode == عدم)
        أرجع عدم;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(حجم(gz_state));
    إذا (state == عدم)
        أرجع عدم;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->رسالة = عدم;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    طالما (*mode) {
        إذا (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        وإلا
            بدل (*mode) {
            حالة 'r':
                state->mode = GZ_READ;
                اكسر;
#ifndef NO_GZCOMPRESS
            حالة 'w':
                state->mode = GZ_WRITE;
                اكسر;
            حالة 'a':
                state->mode = GZ_APPEND;
                اكسر;
#endif
            حالة '+':       /* can't read and write at the same time */
                حرر(state);
                أرجع عدم;
            حالة 'b':       /* ignore -- will request binary anyway */
                اكسر;
#ifdef O_CLOEXEC
            حالة 'e':
                oflag |= O_CLOEXEC;
                اكسر;
#endif
#ifdef O_EXCL
            حالة 'x':
                exclusive = 1;
                اكسر;
#endif
            حالة 'f':
                state->strategy = Z_FILTERED;
                اكسر;
            حالة 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                اكسر;
            حالة 'R':
                state->strategy = Z_RLE;
                اكسر;
            حالة 'F':
                state->strategy = Z_FIXED;
                اكسر;
            حالة 'G':
                state->direct = -1;
                اكسر;
#ifdef O_NONBLOCK
            حالة 'N':
                oflag |= O_NONBLOCK;
                اكسر;
#endif
            حالة 'T':
                state->direct = 1;
                اكسر;
            افتراضي:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    إذا (state->mode == GZ_NONE) {
        حرر(state);
        أرجع عدم;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    إذا (state->mode == GZ_READ) {
        إذا (state->direct == 1) {
            /* can't force a transparent read */
            حرر(state);
            أرجع عدم;
        }
        إذا (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    وإلا إذا (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        حرر(state);
        أرجع عدم;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    إذا (fd == -2)
        len = wcstombs(عدم, path, 0);
    وإلا
#endif
        len = strlen((ثابت حرف *)path);
    state->path = (حرف *)malloc(len + 1);
    إذا (state->path == عدم) {
        حرر(state);
        أرجع عدم;
    }
#ifdef WIDECHAR
    إذا (fd == -2) {
        إذا (len)
            wcstombs(state->path, path, len + 1);
        وإلا
            *(state->path) = 0;
    }
    وإلا
#endif
    {
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (فراغ)snprintf(state->path, len + 1, "%s", (ثابت حرف *)path);
#وإلا
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_ثنائي
        O_ثنائي |
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
    إذا (fd == -1)
        state->fd = open((ثابت حرف *)path, oflag, 0666);
#ifdef WIDECHAR
    وإلا إذا (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    وإلا {
#ifdef O_NONBLOCK
        إذا (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        إذا (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    إذا (state->fd == -1) {
        حرر(state->path);
        حرر(state);
        أرجع عدم;
    }
    إذا (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    إذا (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        إذا (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    أرجع (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ثابت حرف *path, ثابت حرف *mode) {
    أرجع gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ثابت حرف *path, ثابت حرف *mode) {
    أرجع gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(صحيح fd, ثابت حرف *mode) {
    حرف *path;         /* identifier for error messages */
    gzFile gz;

    إذا (fd == -1 || (path = (حرف *)malloc(7 + 3 * حجم(صحيح))) == عدم)
        أرجع عدم;
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(path, 7 + 3 * حجم(صحيح), "<fd:%d>", fd);
#وإلا
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    حرر(path);
    أرجع gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(ثابت wchar_t *path, ثابت حرف *mode) {
    أرجع gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
صحيح ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ && state->mode != GZ_WRITE)
        أرجع -1;

    /* make sure we haven't already allocated memory */
    إذا (state->size != 0)
        أرجع -1;

    /* check and set requested size */
    إذا ((size << 1) < size)
        أرجع -1;              /* need to be able to double it */
    إذا (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    أرجع 0;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    إذا (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        أرجع -1;

    /* back up and start over */
    إذا (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        أرجع -1;
    gz_reset(state);
    أرجع 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, صحيح whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ && state->mode != GZ_WRITE)
        أرجع -1;

    /* check that there's no error */
    إذا (state->err != Z_OK && state->err != Z_BUF_ERROR)
        أرجع -1;

    /* can only seek from start or relative to current position */
    إذا (whence != SEEK_SET && whence != SEEK_CUR)
        أرجع -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    إذا (offset > GZ_OFF_MAX - pos)
        أرجع -1;                          /* would overflow z_off64_t */
    إذا (offset < -pos)
        أرجع -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    إذا (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        إذا (ret == -1)
            أرجع -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, عدم);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        أرجع pos;
    }

    /* rewind if needed for back seek when reading */
    إذا (pos < state->x.pos) {
        إذا (state->mode != GZ_READ)         /* writing -- can't go backwards */
            أرجع -1;
        إذا (gzrewind(file) == -1)           /* rewind, then skip to pos */
            أرجع -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    إذا (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    أرجع pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, صحيح whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    أرجع ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ && state->mode != GZ_WRITE)
        أرجع -1;

    /* return position */
    أرجع state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    أرجع ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ && state->mode != GZ_WRITE)
        أرجع -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    إذا (offset == -1)
        أرجع -1;
    إذا (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    أرجع offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    أرجع ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
صحيح ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع 0;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ && state->mode != GZ_WRITE)
        أرجع 0;

    /* return end-of-file state */
    أرجع state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ثابت حرف * ZEXPORT gzerror(gzFile file, صحيح *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع عدم;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ && state->mode != GZ_WRITE)
        أرجع عدم;

    /* return error information */
    إذا (errnum != عدم)
        *errnum = state->err;
    أرجع state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->رسالة == عدم ? "" : state->رسالة);
}

/* -- see zlib.h -- */
فراغ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع;
    state = (gz_statep)file;
    إذا (state->mode != GZ_READ && state->mode != GZ_WRITE)
        أرجع;

    /* clear error and end-of-file */
    إذا (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, عدم);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
فراغ ZLIB_INTERNAL gz_error(gz_statep state, صحيح err, ثابت حرف *رسالة) {
    /* free previously allocated message and clear */
    إذا (state->رسالة != عدم) {
        إذا (state->err != Z_MEM_ERROR)
            حرر(state->رسالة);
        state->رسالة = عدم;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    إذا (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    إذا (رسالة == عدم)
        أرجع;

    /* for an out of memory error, return literal string when requested */
    إذا (err == Z_MEM_ERROR)
        أرجع;

    /* construct error message with path */
    إذا ((state->رسالة = (حرف *)malloc(strlen(state->path) + strlen(رسالة) + 3)) ==
            عدم) {
        state->err = Z_MEM_ERROR;
        أرجع;
    }
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(state->رسالة, strlen(state->path) + strlen(رسالة) + 3,
                   "%s%s%s", state->path, ": ", رسالة);
#وإلا
    strcpy(state->رسالة, state->path);
    strcat(state->رسالة, ": ");
    strcat(state->رسالة, رسالة);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(فراغ) {
#ifdef INT_MAX
    أرجع INT_MAX;
#وإلا
    unsigned p = 1, q;

    افعل {
        q = p;
        p <<= 1;
        p++;
    } طالما (p > q);
    أرجع q >> 1;
#endif
}
