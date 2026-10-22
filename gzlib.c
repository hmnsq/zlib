/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#समावेश "gzguts.h"

#अगर defined(__DJGPP__)
#  define LSEEK llseek
#या_अगर defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#या_अगर defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#अन्यथा
#  define LSEEK lseek
#पूर्ण_अगर

#अगर defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
अक्षर ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    अटल अक्षर buf[1024];

    ब_अक्षर_प्रकार *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_बफर,
        शून्य,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        शून्य);
    अगर (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        अगर (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        अगर (chars > माप (buf) - 1) {
            chars = माप (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    अन्यथा {
        प्र_लिखो(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    वापस buf;
}

#पूर्ण_अगर /* UNDER_CE */

/* Reset gzip file state */
local व्योम gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    अगर (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    अन्यथा                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, शून्य);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(स्थिर व्योम *path, पूर्णांक fd, स्थिर अक्षर *mode) {
    gz_statep state;
    z_आकार_t len;
    पूर्णांक oflag = 0;
#अगर_घोषित O_EXCL
    पूर्णांक exclusive = 0;
#पूर्ण_अगर

    /* check input */
    अगर (path == शून्य || mode == शून्य)
        वापस शून्य;

    /* allocate gzFile structure to return */
    state = (gz_statep)दो_स्मृति(माप(gz_state));
    अगर (state == शून्य)
        वापस शून्य;
    state->आकार = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->संदेश = शून्य;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    जबतक (*mode) {
        अगर (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        अन्यथा
            चयन (*mode) {
            हाल 'r':
                state->mode = GZ_READ;
                अवरोध;
#अगर_अघोषित NO_GZCOMPRESS
            हाल 'w':
                state->mode = GZ_WRITE;
                अवरोध;
            हाल 'a':
                state->mode = GZ_APPEND;
                अवरोध;
#पूर्ण_अगर
            हाल '+':       /* can't read and write at the same time */
                मुक्त(state);
                वापस शून्य;
            हाल 'b':       /* ignore -- will request binary anyway */
                अवरोध;
#अगर_घोषित O_CLOEXEC
            हाल 'e':
                oflag |= O_CLOEXEC;
                अवरोध;
#पूर्ण_अगर
#अगर_घोषित O_EXCL
            हाल 'x':
                exclusive = 1;
                अवरोध;
#पूर्ण_अगर
            हाल 'f':
                state->strategy = Z_FILTERED;
                अवरोध;
            हाल 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                अवरोध;
            हाल 'R':
                state->strategy = Z_RLE;
                अवरोध;
            हाल 'F':
                state->strategy = Z_FIXED;
                अवरोध;
            हाल 'G':
                state->direct = -1;
                अवरोध;
#अगर_घोषित O_NONBLOCK
            हाल 'N':
                oflag |= O_NONBLOCK;
                अवरोध;
#पूर्ण_अगर
            हाल 'T':
                state->direct = 1;
                अवरोध;
            शेष:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    अगर (state->mode == GZ_NONE) {
        मुक्त(state);
        वापस शून्य;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    अगर (state->mode == GZ_READ) {
        अगर (state->direct == 1) {
            /* can't force a transparent read */
            मुक्त(state);
            वापस शून्य;
        }
        अगर (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    अन्यथा अगर (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        मुक्त(state);
        वापस शून्य;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#अगर_घोषित WIDECHAR
    अगर (fd == -2)
        len = wcstombs(शून्य, path, 0);
    अन्यथा
#पूर्ण_अगर
        len = म_माप((स्थिर अक्षर *)path);
    state->path = (अक्षर *)दो_स्मृति(len + 1);
    अगर (state->path == शून्य) {
        मुक्त(state);
        वापस शून्य;
    }
#अगर_घोषित WIDECHAR
    अगर (fd == -2) {
        अगर (len)
            wcstombs(state->path, path, len + 1);
        अन्यथा
            *(state->path) = 0;
    }
    अन्यथा
#पूर्ण_अगर
    {
#अगर !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (व्योम)snprintf(state->path, len + 1, "%s", (स्थिर अक्षर *)path);
#अन्यथा
        म_नकल(state->path, path);
#पूर्ण_अगर
    }

    /* compute the flags for open() */
    oflag |=
#अगर_घोषित O_LARGEFILE
        O_LARGEFILE |
#पूर्ण_अगर
#अगर_घोषित O_द्विआधारी
        O_द्विआधारी |
#पूर्ण_अगर
        (state->mode == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#अगर_घोषित O_EXCL
          (exclusive ? O_EXCL : 0) |
#पूर्ण_अगर
          (state->mode == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    अगर (fd == -1)
        state->fd = खोलो((स्थिर अक्षर *)path, oflag, 0666);
#अगर_घोषित WIDECHAR
    अन्यथा अगर (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#पूर्ण_अगर
    अन्यथा {
#अगर_घोषित O_NONBLOCK
        अगर (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#पूर्ण_अगर
#अगर_घोषित O_CLOEXEC
        अगर (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#पूर्ण_अगर
        state->fd = fd;
    }
    अगर (state->fd == -1) {
        मुक्त(state->path);
        मुक्त(state);
        वापस शून्य;
    }
    अगर (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, अंत_से);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    अगर (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, प्रस्तुत_से);
        अगर (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    वापस (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(स्थिर अक्षर *path, स्थिर अक्षर *mode) {
    वापस gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(स्थिर अक्षर *path, स्थिर अक्षर *mode) {
    वापस gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(पूर्णांक fd, स्थिर अक्षर *mode) {
    अक्षर *path;         /* identifier for error messages */
    gzFile gz;

    अगर (fd == -1 || (path = (अक्षर *)दो_स्मृति(7 + 3 * माप(पूर्णांक))) == शून्य)
        वापस शून्य;
#अगर !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (व्योम)snprintf(path, 7 + 3 * माप(पूर्णांक), "<fd:%d>", fd);
#अन्यथा
    प्र_लिखो(path, "<fd:%d>", fd);   /* for debugging */
#पूर्ण_अगर
    gz = gz_open(path, fd, mode);
    मुक्त(path);
    वापस gz;
}

/* -- see zlib.h -- */
#अगर_घोषित WIDECHAR
gzFile ZEXPORT gzopen_w(स्थिर ब_अक्षर_प्रकार *path, स्थिर अक्षर *mode) {
    वापस gz_open(path, -2, mode);
}
#पूर्ण_अगर

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzbuffer(gzFile file, अचिन्हित आकार) {
    gz_statep state;

    /* get internal structure and check integrity */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        वापस -1;

    /* make sure we haven't already allocated memory */
    अगर (state->आकार != 0)
        वापस -1;

    /* check and set requested size */
    अगर ((आकार << 1) < आकार)
        वापस -1;              /* need to be able to double it */
    अगर (आकार < 8)
        आकार = 8;               /* needed to behave well with flushing */
    state->want = आकार;
    वापस 0;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    अगर (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        वापस -1;

    /* back up and start over */
    अगर (LSEEK(state->fd, state->start, शुरू_से) == -1)
        वापस -1;
    gz_reset(state);
    वापस 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, पूर्णांक whence) {
    अचिन्हित n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        वापस -1;

    /* check that there's no error */
    अगर (state->err != Z_OK && state->err != Z_BUF_ERROR)
        वापस -1;

    /* can only seek from start or relative to current position */
    अगर (whence != शुरू_से && whence != प्रस्तुत_से)
        वापस -1;

    /* compute the absolute position */
    pos = whence == शुरू_से ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    अगर (offset > GZ_OFF_MAX - pos)
        वापस -1;                          /* would overflow z_off64_t */
    अगर (offset < -pos)
        वापस -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    अगर (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    प्रस्तुत_से);
        अगर (ret == -1)
            वापस -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, शून्य);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        वापस pos;
    }

    /* rewind if needed for back seek when reading */
    अगर (pos < state->x.pos) {
        अगर (state->mode != GZ_READ)         /* writing -- can't go backwards */
            वापस -1;
        अगर (gzrewind(file) == -1)           /* rewind, then skip to pos */
            वापस -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    अगर (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (अचिन्हित)offset : state->x.have;
        state->x.have -= n;
        state->x.अग्र += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    वापस pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, पूर्णांक whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    वापस ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        वापस -1;

    /* return position */
    वापस state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    वापस ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    अगर (file == शून्य)
        वापस -1;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        वापस -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, प्रस्तुत_से);
    अगर (offset == -1)
        वापस -1;
    अगर (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    वापस offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    वापस ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
पूर्णांक ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    अगर (file == शून्य)
        वापस 0;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        वापस 0;

    /* return end-of-file state */
    वापस state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
स्थिर अक्षर * ZEXPORT gzerror(gzFile file, पूर्णांक *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    अगर (file == शून्य)
        वापस शून्य;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        वापस शून्य;

    /* return error information */
    अगर (errnum != शून्य)
        *errnum = state->err;
    वापस state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->संदेश == शून्य ? "" : state->संदेश);
}

/* -- see zlib.h -- */
व्योम ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    अगर (file == शून्य)
        वापस;
    state = (gz_statep)file;
    अगर (state->mode != GZ_READ && state->mode != GZ_WRITE)
        वापस;

    /* clear error and end-of-file */
    अगर (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, शून्य);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
व्योम ZLIB_INTERNAL gz_error(gz_statep state, पूर्णांक err, स्थिर अक्षर *संदेश) {
    /* free previously allocated message and clear */
    अगर (state->संदेश != शून्य) {
        अगर (state->err != Z_MEM_ERROR)
            मुक्त(state->संदेश);
        state->संदेश = शून्य;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    अगर (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    अगर (संदेश == शून्य)
        वापस;

    /* for an out of memory error, return literal string when requested */
    अगर (err == Z_MEM_ERROR)
        वापस;

    /* construct error message with path */
    अगर ((state->संदेश = (अक्षर *)दो_स्मृति(म_माप(state->path) + म_माप(संदेश) + 3)) ==
            शून्य) {
        state->err = Z_MEM_ERROR;
        वापस;
    }
#अगर !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (व्योम)snprintf(state->संदेश, म_माप(state->path) + म_माप(संदेश) + 3,
                   "%s%s%s", state->path, ": ", संदेश);
#अन्यथा
    म_नकल(state->संदेश, state->path);
    म_जोड़ो(state->संदेश, ": ");
    म_जोड़ो(state->संदेश, संदेश);
#पूर्ण_अगर
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
अचिन्हित ZLIB_INTERNAL gz_intmax(व्योम) {
#अगर_घोषित पूर्णांक_उच्च
    वापस पूर्णांक_उच्च;
#अन्यथा
    अचिन्हित p = 1, q;

    करो {
        q = p;
        p <<= 1;
        p++;
    } जबतक (p > q);
    वापस q >> 1;
#पूर्ण_अगर
}
