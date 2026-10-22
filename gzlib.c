/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#অন্তর্ভাব "gzguts.h"

#যদি defined(__DJGPP__)
#  define LSEEK llseek
#নতুবা_যদি defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#নতুবা_যদি defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#অন্যথা
#  define LSEEK lseek
#endif

#যদি defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
অক্ষর ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    স্থির অক্ষর buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_বাফার,
        ফাঁকা,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ফাঁকা);
    যদি (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        যদি (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        যদি (chars > আকার (buf) - 1) {
            chars = আকার (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    অন্যথা {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    ফেরত buf;
}

#endif /* UNDER_CE */

/* Reset gzip file state */
local শূন্য gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    যদি (state->mode == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    অন্যথা                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ফাঁকা);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ধ্রুবক শূন্য *path, পূর্ণসংখ্যা fd, ধ্রুবক অক্ষর *mode) {
    gz_statep state;
    z_আকার_t len;
    পূর্ণসংখ্যা oflag = 0;
#ifdef O_EXCL
    পূর্ণসংখ্যা exclusive = 0;
#endif

    /* check input */
    যদি (path == ফাঁকা || mode == ফাঁকা)
        ফেরত ফাঁকা;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(আকার(gz_state));
    যদি (state == ফাঁকা)
        ফেরত ফাঁকা;
    state->আকার_2 = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->বার্তা = ফাঁকা;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    যখন (*mode) {
        যদি (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        অন্যথা
            নির্বাচন (*mode) {
            ক্ষেত্র 'r':
                state->mode = GZ_READ;
                বিরতি;
#ifndef NO_GZCOMPRESS
            ক্ষেত্র 'w':
                state->mode = GZ_WRITE;
                বিরতি;
            ক্ষেত্র 'a':
                state->mode = GZ_APPEND;
                বিরতি;
#endif
            ক্ষেত্র '+':       /* can't read and write at the same time */
                মুক্ত(state);
                ফেরত ফাঁকা;
            ক্ষেত্র 'b':       /* ignore -- will request binary anyway */
                বিরতি;
#ifdef O_CLOEXEC
            ক্ষেত্র 'e':
                oflag |= O_CLOEXEC;
                বিরতি;
#endif
#ifdef O_EXCL
            ক্ষেত্র 'x':
                exclusive = 1;
                বিরতি;
#endif
            ক্ষেত্র 'f':
                state->strategy = Z_FILTERED;
                বিরতি;
            ক্ষেত্র 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                বিরতি;
            ক্ষেত্র 'R':
                state->strategy = Z_RLE;
                বিরতি;
            ক্ষেত্র 'F':
                state->strategy = Z_FIXED;
                বিরতি;
            ক্ষেত্র 'G':
                state->direct = -1;
                বিরতি;
#ifdef O_NONBLOCK
            ক্ষেত্র 'N':
                oflag |= O_NONBLOCK;
                বিরতি;
#endif
            ক্ষেত্র 'T':
                state->direct = 1;
                বিরতি;
            পূর্বনির্ধারিত:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    যদি (state->mode == GZ_NONE) {
        মুক্ত(state);
        ফেরত ফাঁকা;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    যদি (state->mode == GZ_READ) {
        যদি (state->direct == 1) {
            /* can't force a transparent read */
            মুক্ত(state);
            ফেরত ফাঁকা;
        }
        যদি (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    অন্যথা যদি (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        মুক্ত(state);
        ফেরত ফাঁকা;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    যদি (fd == -2)
        len = wcstombs(ফাঁকা, path, 0);
    অন্যথা
#endif
        len = strlen((ধ্রুবক অক্ষর *)path);
    state->path = (অক্ষর *)malloc(len + 1);
    যদি (state->path == ফাঁকা) {
        মুক্ত(state);
        ফেরত ফাঁকা;
    }
#ifdef WIDECHAR
    যদি (fd == -2) {
        যদি (len)
            wcstombs(state->path, path, len + 1);
        অন্যথা
            *(state->path) = 0;
    }
    অন্যথা
#endif
    {
#যদি !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (শূন্য)snprintf(state->path, len + 1, "%s", (ধ্রুবক অক্ষর *)path);
#অন্যথা
        strcpy(state->path, path);
#endif
    }

    /* compute the flags for open() */
    oflag |=
#ifdef O_LARGEFILE
        O_LARGEFILE |
#endif
#ifdef O_দ্বিমিক
        O_দ্বিমিক |
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
    যদি (fd == -1)
        state->fd = open((ধ্রুবক অক্ষর *)path, oflag, 0666);
#ifdef WIDECHAR
    অন্যথা যদি (fd == -2)
        state->fd = _wopen(path, oflag, _S_IREAD | _S_IWRITE);
#endif
    অন্যথা {
#ifdef O_NONBLOCK
        যদি (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
#ifdef O_CLOEXEC
        যদি (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#endif
        state->fd = fd;
    }
    যদি (state->fd == -1) {
        মুক্ত(state->path);
        মুক্ত(state);
        ফেরত ফাঁকা;
    }
    যদি (state->mode == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->mode = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    যদি (state->mode == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        যদি (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ফেরত (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ধ্রুবক অক্ষর *path, ধ্রুবক অক্ষর *mode) {
    ফেরত gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ধ্রুবক অক্ষর *path, ধ্রুবক অক্ষর *mode) {
    ফেরত gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(পূর্ণসংখ্যা fd, ধ্রুবক অক্ষর *mode) {
    অক্ষর *path;         /* identifier for error messages */
    gzFile gz;

    যদি (fd == -1 || (path = (অক্ষর *)malloc(7 + 3 * আকার(পূর্ণসংখ্যা))) == ফাঁকা)
        ফেরত ফাঁকা;
#যদি !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (শূন্য)snprintf(path, 7 + 3 * আকার(পূর্ণসংখ্যা), "<fd:%d>", fd);
#অন্যথা
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    মুক্ত(path);
    ফেরত gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(ধ্রুবক wchar_t *path, ধ্রুবক অক্ষর *mode) {
    ফেরত gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzbuffer(gzFile file, unsigned আকার_2) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ফেরত -1;

    /* make sure we haven't already allocated memory */
    যদি (state->আকার_2 != 0)
        ফেরত -1;

    /* check and set requested size */
    যদি ((আকার_2 << 1) < আকার_2)
        ফেরত -1;              /* need to be able to double it */
    যদি (আকার_2 < 8)
        আকার_2 = 8;               /* needed to behave well with flushing */
    state->want = আকার_2;
    ফেরত 0;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    যদি (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ফেরত -1;

    /* back up and start over */
    যদি (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        ফেরত -1;
    gz_reset(state);
    ফেরত 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, পূর্ণসংখ্যা whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ফেরত -1;

    /* check that there's no error */
    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ফেরত -1;

    /* can only seek from start or relative to current position */
    যদি (whence != SEEK_SET && whence != SEEK_CUR)
        ফেরত -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    যদি (offset > GZ_OFF_MAX - pos)
        ফেরত -1;                          /* would overflow z_off64_t */
    যদি (offset < -pos)
        ফেরত -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    যদি (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        যদি (ret == -1)
            ফেরত -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ফাঁকা);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        ফেরত pos;
    }

    /* rewind if needed for back seek when reading */
    যদি (pos < state->x.pos) {
        যদি (state->mode != GZ_READ)         /* writing -- can't go backwards */
            ফেরত -1;
        যদি (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ফেরত -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    যদি (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.পরবর্তী += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    ফেরত pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, পূর্ণসংখ্যা whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    ফেরত ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ফেরত -1;

    /* return position */
    ফেরত state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ফেরত ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == ফাঁকা)
        ফেরত -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ফেরত -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    যদি (offset == -1)
        ফেরত -1;
    যদি (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    ফেরত offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ফেরত ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
পূর্ণসংখ্যা ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == ফাঁকা)
        ফেরত 0;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ফেরত 0;

    /* return end-of-file state */
    ফেরত state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ধ্রুবক অক্ষর * ZEXPORT gzerror(gzFile file, পূর্ণসংখ্যা *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == ফাঁকা)
        ফেরত ফাঁকা;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ফেরত ফাঁকা;

    /* return error information */
    যদি (errnum != ফাঁকা)
        *errnum = state->err;
    ফেরত state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->বার্তা == ফাঁকা ? "" : state->বার্তা);
}

/* -- see zlib.h -- */
শূন্য ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == ফাঁকা)
        ফেরত;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        ফেরত;

    /* clear error and end-of-file */
    যদি (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ফাঁকা);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
শূন্য ZLIB_INTERNAL gz_error(gz_statep state, পূর্ণসংখ্যা err, ধ্রুবক অক্ষর *বার্তা) {
    /* free previously allocated message and clear */
    যদি (state->বার্তা != ফাঁকা) {
        যদি (state->err != Z_MEM_ERROR)
            মুক্ত(state->বার্তা);
        state->বার্তা = ফাঁকা;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    যদি (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    যদি (বার্তা == ফাঁকা)
        ফেরত;

    /* for an out of memory error, return literal string when requested */
    যদি (err == Z_MEM_ERROR)
        ফেরত;

    /* construct error message with path */
    যদি ((state->বার্তা = (অক্ষর *)malloc(strlen(state->path) + strlen(বার্তা) + 3)) ==
            ফাঁকা) {
        state->err = Z_MEM_ERROR;
        ফেরত;
    }
#যদি !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (শূন্য)snprintf(state->বার্তা, strlen(state->path) + strlen(বার্তা) + 3,
                   "%s%s%s", state->path, ": ", বার্তা);
#অন্যথা
    strcpy(state->বার্তা, state->path);
    strcat(state->বার্তা, ": ");
    strcat(state->বার্তা, বার্তা);
#endif
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
unsigned ZLIB_INTERNAL gz_intmax(শূন্য) {
#ifdef INT_MAX
    ফেরত INT_MAX;
#অন্যথা
    unsigned p = 1, q;

    করো {
        q = p;
        p <<= 1;
        p++;
    } যখন (p > q);
    ফেরত q >> 1;
#endif
}
