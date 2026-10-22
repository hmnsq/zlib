/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "gzguts.h"

#जर defined(__DJGPP__)
#  define LSEEK llseek
#किंवाजर defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#किंवाजर defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#अन्यथा
#  define LSEEK lseek
#endif

#जर defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
अक्षर ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    स्थिर अक्षर buf[1024];

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
    जर (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        जर (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        जर (chars > आकार (buf) - 1) {
            chars = आकार (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    अन्यथा {
        प्र_लिखो(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    परत buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local शून्य gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    जर (state->mode == GZ_READ) {   /* for reading ... */
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
local gzFile gz_open(स्थिरांक शून्य *path, पूर्णांक fd, स्थिरांक अक्षर *mode) {
    gz_statep state;
    z_size_t len;
    पूर्णांक oflag = 0;
#ifdef O_EXCL
    पूर्णांक exclusive = 0;
#endif

    /* check input */
    जर (path == रिक्त || mode == रिक्त)
        परत रिक्त;

    /* allocate gzFile structure to return */
    state = (gz_statep)दो_स्मृति(आकार(gz_state));
    जर (state == रिक्त)
        परत रिक्त;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->संदेश = रिक्त;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    पर्यंत (*mode) {
        जर (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        अन्यथा
            निवड (*mode) {
            स्थिती 'r':
                state->mode = GZ_READ;
                थांबा;
#ifndef NO_GZCOMPRESS
            स्थिती 'w':
                state->mode = GZ_WRITE;
                थांबा;
            स्थिती 'a':
                state->mode = GZ_APPEND;
                थांबा;
#endif
            स्थिती '+':       /* can't read and write at the same time */
                मुक्त(state);
                परत रिक्त;
            स्थिती 'b':       /* ignore -- will request binary anyway */
                थांबा;
#ifdef O_CLOEXEC
            स्थिती 'e':
                oflag |= O_CLOEXEC;
                थांबा;
#endif
#ifdef O_EXCL
            स्थिती 'x':
                exclusive = 1;
                थांबा;
#endif
            स्थिती 'f':
                state->strategy = Z_FILTERED;
                थांबा;
            स्थिती 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                थांबा;
            स्थिती 'R':
                state->strategy = Z_RLE;
                थांबा;
            स्थिती 'F':
                state->strategy = Z_FIXED;
                थांबा;
            स्थिती 'G':
                state->direct = -1;
                थांबा;
#ifdef O_NONBLOCK
            स्थिती 'N':
                oflag |= O_NONBLOCK;
                थांबा;
#endif
            स्थिती 'T':
                state->direct = 1;
                थांबा;
            पूर्वनिर्धारित:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    जर (state->mode == GZ_NONE) {
        मुक्त(state);
        परत रिक्त;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    जर (state->mode == GZ_READ) {
        जर (state->direct == 1) {
            /* can't force a transparent read */
            मुक्त(state);
            परत रिक्त;
        }
        जर (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    अन्यथा जर (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        मुक्त(state);
        परत रिक्त;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    जर (fd == -2)
        len = wcstombs(रिक्त, path, 0);
    अन्यथा
#endif
        len = म_माप((स्थिरांक अक्षर *)path);
    state->path = (अक्षर *)दो_स्मृति(len + 1);
    जर (state->path == रिक्त) {
        मुक्त(state);
        परत रिक्त;
    }
#ifdef WIDECHAR
    जर (fd == -2) {
        जर (len)
            wcstombs(state->path, path, len + 1);
        अन्यथा
            *(state->path) = 0;
    }
    अन्यथा
#endif
    {
#जर !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (शून्य)snprintf(state->path, len + 1, "%s", (स्थिरांक अक्षर *)path);
#अन्यथा
        म_नकल(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_द्विमान
        O_द्विमान |
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
    जर (fd == -1)
        state->fd = खोलो((स्थिरांक अक्षर *)path, oflag, 0666);
#ifdef WIDECHAR
    अन्यथा जर (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    अन्यथा {
#ifdef O_NONBLOCK
        जर (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        जर (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    जर (state->fd == -1) {
        मुक्त(state->path);
        मुक्त(state);
        परत रिक्त;
    }
    जर (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, अंत_से);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    जर (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, प्रस्तुत_से);
        जर (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    परत (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(स्थिरांक अक्षर *path, स्थिरांक अक्षर *mode) {
    परत gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(स्थिरांक अक्षर *path, स्थिरांक अक्षर *mode) {
    परत gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(पूर्णांक fd, स्थिरांक अक्षर *mode) {
    अक्षर *path;         /* identifier for error messages */
    gzFile gz;

    जर (fd == -1 || (path = (अक्षर *)दो_स्मृति(7 + 3 * आकार(पूर्णांक))) == रिक्त)
        परत रिक्त;
#जर !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (शून्य)snprintf(path, 7 + 3 * आकार(पूर्णांक), "<fd:%d>", fd);
#अन्यथा
    प्र_लिखो(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    मुक्त(path);
    परत gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(स्थिरांक ब_अक्षर_प्रकार *path, स्थिरांक अक्षर *mode) {
    परत gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzbuffer(gzFile file, अचिन्हित size) {
    gz_statep state;

    /* get internal structure and check integrity */
    जर (file == रिक्त)
        परत -1;
    state = (gz_statep)file;
    जर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        परत -1;

    /* make sure we haven't already allocated memory */
    जर (state->size != 0)
        परत -1;

    /* check and set requested size */
    जर ((size << 1) < size)
        परत -1;              /* need to be able to double it */
    जर (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    परत 0;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    जर (file == रिक्त)
        परत -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    जर (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        परत -1;

    /* back up and start over */
    जर (LSEEK(state->fd, state->start, शुरू_से) == -1)
        परत -1;
    gz_reset(state);
    परत 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, पूर्णांक whence) {
    अचिन्हित n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    जर (file == रिक्त)
        परत -1;
    state = (gz_statep)file;
    जर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        परत -1;

    /* check that there's no error */
    जर (state->err != Z_OK && state->err != Z_BUF_ERROR)
        परत -1;

    /* can only seek from start or relative to current position */
    जर (whence != शुरू_से && whence != प्रस्तुत_से)
        परत -1;

    /* compute the absolute position */
    pos = whence == शुरू_से ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    जर (offset > GZ_OFF_MAX - pos)
        परत -1;                          /* would overflow z_off64_t */
    जर (offset < -pos)
        परत -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    जर (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    प्रस्तुत_से);
        जर (ret == -1)
            परत -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, रिक्त);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        परत pos;
    }

    /* rewind if needed for back seek when reading */
    जर (pos < state->x.pos) {
        जर (state->mode != GZ_READ)         /* writing -- can't go backwards */
            परत -1;
        जर (gzrewind(file) == -1)           /* rewind, then skip to pos */
            परत -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    जर (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (अचिन्हित)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    परत pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, पूर्णांक whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    परत ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    जर (file == रिक्त)
        परत -1;
    state = (gz_statep)file;
    जर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        परत -1;

    /* return position */
    परत state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    परत ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    जर (file == रिक्त)
        परत -1;
    state = (gz_statep)file;
    जर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        परत -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, प्रस्तुत_से);
    जर (offset == -1)
        परत -1;
    जर (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    परत offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    परत ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    जर (file == रिक्त)
        परत 0;
    state = (gz_statep)file;
    जर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        परत 0;

    /* return end-of-file state */
    परत state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
स्थिरांक अक्षर * ZEXPORT gzerror(gzFile file, पूर्णांक *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    जर (file == रिक्त)
        परत रिक्त;
    state = (gz_statep)file;
    जर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        परत रिक्त;

    /* return error information */
    जर (errnum != रिक्त)
        *errnum = state->err;
    परत state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->संदेश == रिक्त ? "" : state->संदेश);
}

/* -- see zlib.h -- */
शून्य ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    जर (file == रिक्त)
        परत;
    state = (gz_statep)file;
    जर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        परत;

    /* clear error and end-of-file */
    जर (state->mode == GZ_READ) {
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
शून्य ZLIB_INTERNAL gz_error(gz_statep state, पूर्णांक err, स्थिरांक अक्षर *संदेश) {
    /* free previously allocated message and clear */
    जर (state->संदेश != रिक्त) {
        जर (state->err != Z_MEM_ERROR)
            मुक्त(state->संदेश);
        state->संदेश = रिक्त;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    जर (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    जर (संदेश == रिक्त)
        परत;

    /* for an out of memory error, return literal string when requested */
    जर (err == Z_MEM_ERROR)
        परत;

    /* construct error message with path */
    जर ((state->संदेश = (अक्षर *)दो_स्मृति(म_माप(state->path) + म_माप(संदेश) + 3)) ==
            रिक्त) {
        state->err = Z_MEM_ERROR;
        परत;
    }
#जर !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (शून्य)snprintf(state->संदेश, म_माप(state->path) + म_माप(संदेश) + 3,
                   "%s%s%s", state->path, ": ", संदेश);
#अन्यथा
    म_नकल(state->संदेश, state->path);
    म_जोड़ो(state->संदेश, ": ");
    म_जोड़ो(state->संदेश, संदेश);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
अचिन्हित ZLIB_INTERNAL gz_intmax(शून्य) {
#ifdef पूर्णांक_उच्च
    परत पूर्णांक_उच्च;
#अन्यथा
    अचिन्हित p = 1, q;

    करा {
        q = p;
        p <<= 1;
        p++;
    } पर्यंत (p > q);
    परत q >> 1;
#endif
}
