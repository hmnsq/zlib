/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "gzguts.h"

#यदि defined(__DJGPP__)
#  define LSEEK llseek
#अथवायदि defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#अथवायदि defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#अन्यथा
#  define LSEEK lseek
#endif

#यदि defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
वर्ण ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    स्थिर वर्ण buf[1024];

    ब_अक्षर_प्रकार *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        रिक्त,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        रिक्त);
    यदि (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        यदि (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        यदि (chars > परिमाण (buf) - 1) {
            chars = परिमाण (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    अन्यथा {
        प्र_लिखो(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    फर्काऊ buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local शून्य gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    यदि (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    अन्यथा                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, रिक्त);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(अचर शून्य *path, पूर्णाङ्क fd, अचर वर्ण *mode) {
    gz_statep state;
    z_size_t len;
    पूर्णाङ्क oflag = 0;
#ifdef O_EXCL
    पूर्णाङ्क exclusive = 0;
#endif

    /* check input */
    यदि (path == रिक्त || mode == रिक्त)
        फर्काऊ रिक्त;

    /* allocate gzFile structure to return */
    state = (gz_statep)दो_स्मृति(परिमाण(gz_state));
    यदि (state == रिक्त)
        फर्काऊ रिक्त;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->सन्देश = रिक्त;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    जबसम्म (*mode) {
        यदि (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        अन्यथा
            विकल्प (*mode) {
            पक्ष 'r':
                state->mode = GZ_READ;
                रोक;
#ifndef NO_GZCOMPRESS
            पक्ष 'w':
                state->mode = GZ_WRITE;
                रोक;
            पक्ष 'a':
                state->mode = GZ_APPEND;
                रोक;
#endif
            पक्ष '+':       /* can't read and write at the same time */
                मुक्त(state);
                फर्काऊ रिक्त;
            पक्ष 'b':       /* ignore -- will request binary anyway */
                रोक;
#ifdef O_CLOEXEC
            पक्ष 'e':
                oflag |= O_CLOEXEC;
                रोक;
#endif
#ifdef O_EXCL
            पक्ष 'x':
                exclusive = 1;
                रोक;
#endif
            पक्ष 'f':
                state->strategy = Z_FILTERED;
                रोक;
            पक्ष 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                रोक;
            पक्ष 'R':
                state->strategy = Z_RLE;
                रोक;
            पक्ष 'F':
                state->strategy = Z_FIXED;
                रोक;
            पक्ष 'G':
                state->direct = -1;
                रोक;
#ifdef O_NONBLOCK
            पक्ष 'N':
                oflag |= O_NONBLOCK;
                रोक;
#endif
            पक्ष 'T':
                state->direct = 1;
                रोक;
            स्वभाव:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    यदि (state->mode == GZ_NONE) {
        मुक्त(state);
        फर्काऊ रिक्त;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    यदि (state->mode == GZ_READ) {
        यदि (state->direct == 1) {
            /* can't force a transparent read */
            मुक्त(state);
            फर्काऊ रिक्त;
        }
        यदि (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    अन्यथा यदि (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        मुक्त(state);
        फर्काऊ रिक्त;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    यदि (fd == -2)
        len = wcstombs(रिक्त, path, 0);
    अन्यथा
#endif
        len = म_माप((अचर वर्ण *)path);
    state->path = (वर्ण *)दो_स्मृति(len + 1);
    यदि (state->path == रिक्त) {
        मुक्त(state);
        फर्काऊ रिक्त;
    }
#ifdef WIDECHAR
    यदि (fd == -2) {
        यदि (len)
            wcstombs(state->path, path, len + 1);
        अन्यथा
            *(state->path) = 0;
    }
    अन्यथा
#endif
    {
#यदि !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (शून्य)snprintf(state->path, len + 1, "%s", (अचर वर्ण *)path);
#अन्यथा
        म_नकल(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_द्विआधारी
        O_द्विआधारी |
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
    यदि (fd == -1)
        state->fd = खोलो((अचर वर्ण *)path, oflag, 0666);
#ifdef WIDECHAR
    अन्यथा यदि (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    अन्यथा {
#ifdef O_NONBLOCK
        यदि (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        यदि (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    यदि (state->fd == -1) {
        मुक्त(state->path);
        मुक्त(state);
        फर्काऊ रिक्त;
    }
    यदि (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, अंत_से);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    यदि (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, प्रस्तुत_से);
        यदि (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    फर्काऊ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(अचर वर्ण *path, अचर वर्ण *mode) {
    फर्काऊ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(अचर वर्ण *path, अचर वर्ण *mode) {
    फर्काऊ gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(पूर्णाङ्क fd, अचर वर्ण *mode) {
    वर्ण *path;         /* identifier for error messages */
    gzFile gz;

    यदि (fd == -1 || (path = (वर्ण *)दो_स्मृति(7 + 3 * परिमाण(पूर्णाङ्क))) == रिक्त)
        फर्काऊ रिक्त;
#यदि !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (शून्य)snprintf(path, 7 + 3 * परिमाण(पूर्णाङ्क), "<fd:%d>", fd);
#अन्यथा
    प्र_लिखो(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    मुक्त(path);
    फर्काऊ gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(अचर ब_अक्षर_प्रकार *path, अचर वर्ण *mode) {
    फर्काऊ gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzbuffer(gzFile file, अचिन्हित size) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ && state->mode != GZ_WRITE)
        फर्काऊ -1;

    /* make sure we haven't already allocated memory */
    यदि (state->size != 0)
        फर्काऊ -1;

    /* check and set requested size */
    यदि ((size << 1) < size)
        फर्काऊ -1;              /* need to be able to double it */
    यदि (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    फर्काऊ 0;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    यदि (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        फर्काऊ -1;

    /* back up and start over */
    यदि (LSEEK(state->fd, state->start, शुरू_से) == -1)
        फर्काऊ -1;
    gz_reset(state);
    फर्काऊ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, पूर्णाङ्क whence) {
    अचिन्हित n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ && state->mode != GZ_WRITE)
        फर्काऊ -1;

    /* check that there's no error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR)
        फर्काऊ -1;

    /* can only seek from start or relative to current position */
    यदि (whence != शुरू_से && whence != प्रस्तुत_से)
        फर्काऊ -1;

    /* compute the absolute position */
    pos = whence == शुरू_से ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    यदि (offset > GZ_OFF_MAX - pos)
        फर्काऊ -1;                          /* would overflow z_off64_t */
    यदि (offset < -pos)
        फर्काऊ -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    यदि (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    प्रस्तुत_से);
        यदि (ret == -1)
            फर्काऊ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, रिक्त);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        फर्काऊ pos;
    }

    /* rewind if needed for back seek when reading */
    यदि (pos < state->x.pos) {
        यदि (state->mode != GZ_READ)         /* writing -- can't go backwards */
            फर्काऊ -1;
        यदि (gzrewind(file) == -1)           /* rewind, then skip to pos */
            फर्काऊ -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    यदि (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (अचिन्हित)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    फर्काऊ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, पूर्णाङ्क whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    फर्काऊ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ && state->mode != GZ_WRITE)
        फर्काऊ -1;

    /* return position */
    फर्काऊ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    फर्काऊ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        फर्काऊ -1;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ && state->mode != GZ_WRITE)
        फर्काऊ -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, प्रस्तुत_से);
    यदि (offset == -1)
        फर्काऊ -1;
    यदि (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    फर्काऊ offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    फर्काऊ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        फर्काऊ 0;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ && state->mode != GZ_WRITE)
        फर्काऊ 0;

    /* return end-of-file state */
    फर्काऊ state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
अचर वर्ण * ZEXPORT gzerror(gzFile file, पूर्णाङ्क *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        फर्काऊ रिक्त;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ && state->mode != GZ_WRITE)
        फर्काऊ रिक्त;

    /* return error information */
    यदि (errnum != रिक्त)
        *errnum = state->err;
    फर्काऊ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->सन्देश == रिक्त ? "" : state->सन्देश);
}

/* -- see zlib.h -- */
शून्य ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        फर्काऊ;
    state = (gz_statep)file;
    यदि (state->mode != GZ_READ && state->mode != GZ_WRITE)
        फर्काऊ;

    /* clear error and end-of-file */
    यदि (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, रिक्त);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
शून्य ZLIB_INTERNAL gz_error(gz_statep state, पूर्णाङ्क err, अचर वर्ण *सन्देश) {
    /* free previously allocated message and clear */
    यदि (state->सन्देश != रिक्त) {
        यदि (state->err != Z_MEM_ERROR)
            मुक्त(state->सन्देश);
        state->सन्देश = रिक्त;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    यदि (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    यदि (सन्देश == रिक्त)
        फर्काऊ;

    /* for an out of memory error, return literal string when requested */
    यदि (err == Z_MEM_ERROR)
        फर्काऊ;

    /* construct error message with path */
    यदि ((state->सन्देश = (वर्ण *)दो_स्मृति(म_माप(state->path) + म_माप(सन्देश) + 3)) ==
            रिक्त) {
        state->err = Z_MEM_ERROR;
        फर्काऊ;
    }
#यदि !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (शून्य)snprintf(state->सन्देश, म_माप(state->path) + म_माप(सन्देश) + 3,
                   "%s%s%s", state->path, ": ", सन्देश);
#अन्यथा
    म_नकल(state->सन्देश, state->path);
    म_जोड़ो(state->सन्देश, ": ");
    म_जोड़ो(state->सन्देश, सन्देश);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
अचिन्हित ZLIB_INTERNAL gz_intmax(शून्य) {
#ifdef पूर्णांक_उच्च
    फर्काऊ पूर्णांक_उच्च;
#अन्यथा
    अचिन्हित p = 1, q;

    गर {
        q = p;
        p <<= 1;
        p++;
    } जबसम्म (p > q);
    फर्काऊ q >> 1;
#endif
}
