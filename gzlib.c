/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#અન્તર્ભાવ "gzguts.h"

#જો defined(__DJGPP__)
#  define LSEEK llseek
#અથવાજો defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#અથવાજો defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#નહીંતર
#  define LSEEK lseek
#endif

#જો defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
અક્ષર ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    સ્થિર અક્ષર buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        ખાલી,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ખાલી);
    જો (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        જો (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        જો (chars > કદ (buf) - 1) {
            chars = કદ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    નહીંતર {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    પાછા buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local શૂન્ય gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    જો (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    નહીંતર                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ખાલી);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(અચલ શૂન્ય *path, પૂર્ણાંક fd, અચલ અક્ષર *mode) {
    gz_statep state;
    z_size_t len;
    પૂર્ણાંક oflag = 0;
#ifdef O_EXCL
    પૂર્ણાંક exclusive = 0;
#endif

    /* check input */
    જો (path == ખાલી || mode == ખાલી)
        પાછા ખાલી;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(કદ(gz_state));
    જો (state == ખાલી)
        પાછા ખાલી;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->સંદેશ = ખાલી;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    જ્યારે (*mode) {
        જો (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        નહીંતર
            પસંદગી (*mode) {
            સ્થિતિ 'r':
                state->mode = GZ_READ;
                તોડો;
#ifndef NO_GZCOMPRESS
            સ્થિતિ 'w':
                state->mode = GZ_WRITE;
                તોડો;
            સ્થિતિ 'a':
                state->mode = GZ_APPEND;
                તોડો;
#endif
            સ્થિતિ '+':       /* can't read and write at the same time */
                મુક્ત(state);
                પાછા ખાલી;
            સ્થિતિ 'b':       /* ignore -- will request binary anyway */
                તોડો;
#ifdef O_CLOEXEC
            સ્થિતિ 'e':
                oflag |= O_CLOEXEC;
                તોડો;
#endif
#ifdef O_EXCL
            સ્થિતિ 'x':
                exclusive = 1;
                તોડો;
#endif
            સ્થિતિ 'f':
                state->strategy = Z_FILTERED;
                તોડો;
            સ્થિતિ 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                તોડો;
            સ્થિતિ 'R':
                state->strategy = Z_RLE;
                તોડો;
            સ્થિતિ 'F':
                state->strategy = Z_FIXED;
                તોડો;
            સ્થિતિ 'G':
                state->direct = -1;
                તોડો;
#ifdef O_NONBLOCK
            સ્થિતિ 'N':
                oflag |= O_NONBLOCK;
                તોડો;
#endif
            સ્થિતિ 'T':
                state->direct = 1;
                તોડો;
            મૂળભૂત:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    જો (state->mode == GZ_NONE) {
        મુક્ત(state);
        પાછા ખાલી;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    જો (state->mode == GZ_READ) {
        જો (state->direct == 1) {
            /* can't force a transparent read */
            મુક્ત(state);
            પાછા ખાલી;
        }
        જો (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    નહીંતર જો (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        મુક્ત(state);
        પાછા ખાલી;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    જો (fd == -2)
        len = wcstombs(ખાલી, path, 0);
    નહીંતર
#endif
        len = strlen((અચલ અક્ષર *)path);
    state->path = (અક્ષર *)malloc(len + 1);
    જો (state->path == ખાલી) {
        મુક્ત(state);
        પાછા ખાલી;
    }
#ifdef WIDECHAR
    જો (fd == -2) {
        જો (len)
            wcstombs(state->path, path, len + 1);
        નહીંતર
            *(state->path) = 0;
    }
    નહીંતર
#endif
    {
#જો !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (શૂન્ય)snprintf(state->path, len + 1, "%s", (અચલ અક્ષર *)path);
#નહીંતર
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_દ્વિઅંકી
        O_દ્વિઅંકી |
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
    જો (fd == -1)
        state->fd = open((અચલ અક્ષર *)path, oflag, 0666);
#ifdef WIDECHAR
    નહીંતર જો (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    નહીંતર {
#ifdef O_NONBLOCK
        જો (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        જો (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    જો (state->fd == -1) {
        મુક્ત(state->path);
        મુક્ત(state);
        પાછા ખાલી;
    }
    જો (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    જો (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        જો (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    પાછા (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(અચલ અક્ષર *path, અચલ અક્ષર *mode) {
    પાછા gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(અચલ અક્ષર *path, અચલ અક્ષર *mode) {
    પાછા gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(પૂર્ણાંક fd, અચલ અક્ષર *mode) {
    અક્ષર *path;         /* identifier for error messages */
    gzFile gz;

    જો (fd == -1 || (path = (અક્ષર *)malloc(7 + 3 * કદ(પૂર્ણાંક))) == ખાલી)
        પાછા ખાલી;
#જો !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (શૂન્ય)snprintf(path, 7 + 3 * કદ(પૂર્ણાંક), "<fd:%d>", fd);
#નહીંતર
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    મુક્ત(path);
    પાછા gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(અચલ wchar_t *path, અચલ અક્ષર *mode) {
    પાછા gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    જો (file == ખાલી)
        પાછા -1;
    state = (gz_statep)file;
    જો (state->mode != GZ_READ && state->mode != GZ_WRITE)
        પાછા -1;

    /* make sure we haven't already allocated memory */
    જો (state->size != 0)
        પાછા -1;

    /* check and set requested size */
    જો ((size << 1) < size)
        પાછા -1;              /* need to be able to double it */
    જો (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    પાછા 0;
}

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    જો (file == ખાલી)
        પાછા -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    જો (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        પાછા -1;

    /* back up and start over */
    જો (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        પાછા -1;
    gz_reset(state);
    પાછા 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, પૂર્ણાંક whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    જો (file == ખાલી)
        પાછા -1;
    state = (gz_statep)file;
    જો (state->mode != GZ_READ && state->mode != GZ_WRITE)
        પાછા -1;

    /* check that there's no error */
    જો (state->err != Z_OK && state->err != Z_BUF_ERROR)
        પાછા -1;

    /* can only seek from start or relative to current position */
    જો (whence != SEEK_SET && whence != SEEK_CUR)
        પાછા -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    જો (offset > GZ_OFF_MAX - pos)
        પાછા -1;                          /* would overflow z_off64_t */
    જો (offset < -pos)
        પાછા -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    જો (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        જો (ret == -1)
            પાછા -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ખાલી);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        પાછા pos;
    }

    /* rewind if needed for back seek when reading */
    જો (pos < state->x.pos) {
        જો (state->mode != GZ_READ)         /* writing -- can't go backwards */
            પાછા -1;
        જો (gzrewind(file) == -1)           /* rewind, then skip to pos */
            પાછા -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    જો (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    પાછા pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, પૂર્ણાંક whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    પાછા ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    જો (file == ખાલી)
        પાછા -1;
    state = (gz_statep)file;
    જો (state->mode != GZ_READ && state->mode != GZ_WRITE)
        પાછા -1;

    /* return position */
    પાછા state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    પાછા ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    જો (file == ખાલી)
        પાછા -1;
    state = (gz_statep)file;
    જો (state->mode != GZ_READ && state->mode != GZ_WRITE)
        પાછા -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    જો (offset == -1)
        પાછા -1;
    જો (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    પાછા offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    પાછા ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
પૂર્ણાંક ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    જો (file == ખાલી)
        પાછા 0;
    state = (gz_statep)file;
    જો (state->mode != GZ_READ && state->mode != GZ_WRITE)
        પાછા 0;

    /* return end-of-file state */
    પાછા state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
અચલ અક્ષર * ZEXPORT gzerror(gzFile file, પૂર્ણાંક *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    જો (file == ખાલી)
        પાછા ખાલી;
    state = (gz_statep)file;
    જો (state->mode != GZ_READ && state->mode != GZ_WRITE)
        પાછા ખાલી;

    /* return error information */
    જો (errnum != ખાલી)
        *errnum = state->err;
    પાછા state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->સંદેશ == ખાલી ? "" : state->સંદેશ);
}

/* -- see zlib.h -- */
શૂન્ય ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    જો (file == ખાલી)
        પાછા;
    state = (gz_statep)file;
    જો (state->mode != GZ_READ && state->mode != GZ_WRITE)
        પાછા;

    /* clear error and end-of-file */
    જો (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ખાલી);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
શૂન્ય ZLIB_INTERNAL gz_error(gz_statep state, પૂર્ણાંક err, અચલ અક્ષર *સંદેશ) {
    /* free previously allocated message and clear */
    જો (state->સંદેશ != ખાલી) {
        જો (state->err != Z_MEM_ERROR)
            મુક્ત(state->સંદેશ);
        state->સંદેશ = ખાલી;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    જો (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    જો (સંદેશ == ખાલી)
        પાછા;

    /* for an out of memory error, return literal string when requested */
    જો (err == Z_MEM_ERROR)
        પાછા;

    /* construct error message with path */
    જો ((state->સંદેશ = (અક્ષર *)malloc(strlen(state->path) + strlen(સંદેશ) + 3)) ==
            ખાલી) {
        state->err = Z_MEM_ERROR;
        પાછા;
    }
#જો !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (શૂન્ય)snprintf(state->સંદેશ, strlen(state->path) + strlen(સંદેશ) + 3,
                   "%s%s%s", state->path, ": ", સંદેશ);
#નહીંતર
    strcpy(state->સંદેશ, state->path);
    strcat(state->સંદેશ, ": ");
    strcat(state->સંદેશ, સંદેશ);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(શૂન્ય) {
#ifdef INT_MAX
    પાછા INT_MAX;
#નહીંતર
    unsigned p = 1, q;

    કરો {
        q = p;
        p <<= 1;
        p++;
    } જ્યારે (p > q);
    પાછા q >> 1;
#endif
}
