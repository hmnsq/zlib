/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#অন্তর্ভাব "gzguts.h"

#যদি defined(__DJGPP__)
#  define LSEEK llseek
#নোচেদ্যদি defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#নোচেদ্যদি defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
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
বর্ণ ZLIB_INTERNAL *gz_strwinerror(DWORD error) {
    স্থির বর্ণ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = GetLastError();
    DWORD chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        রিক্ত,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        রিক্ত);
    যদি (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        যদি (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        যদি (chars > পরিমাণ (buf) - 1) {
            chars = পরিমাণ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    অন্যথা {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    SetLastError(lasterr);
    প্রত্যাগম buf;
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
    gz_error(state, Z_OK, রিক্ত);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_in = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ধ্রুব শূন্য *path, পূর্ণাঙ্ক fd, ধ্রুব বর্ণ *mode) {
    gz_statep state;
    z_size_t len;
    পূর্ণাঙ্ক oflag = 0;
#ifdef O_EXCL
    পূর্ণাঙ্ক exclusive = 0;
#endif

    /* check input */
    যদি (path == রিক্ত || mode == রিক্ত)
        প্রত্যাগম রিক্ত;

    /* allocate gzFile structure to return */
    state = (gz_statep)malloc(পরিমাণ(gz_state));
    যদি (state == রিক্ত)
        প্রত্যাগম রিক্ত;
    state->size = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->বার্তা = রিক্ত;          /* no error message yet */

    /* interpret mode */
    state->mode = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->direct = 0;
    যাবত্ (*mode) {
        যদি (*mode >= '0' && *mode <= '9')
            state->level = *mode - '0';
        অন্যথা
            বিকল্প (*mode) {
            পক্ষ 'r':
                state->mode = GZ_READ;
                ভঙ্গ;
#ifndef NO_GZCOMPRESS
            পক্ষ 'w':
                state->mode = GZ_WRITE;
                ভঙ্গ;
            পক্ষ 'a':
                state->mode = GZ_APPEND;
                ভঙ্গ;
#endif
            পক্ষ '+':       /* can't read and write at the same time */
                মুক্ত(state);
                প্রত্যাগম রিক্ত;
            পক্ষ 'b':       /* ignore -- will request binary anyway */
                ভঙ্গ;
#ifdef O_CLOEXEC
            পক্ষ 'e':
                oflag |= O_CLOEXEC;
                ভঙ্গ;
#endif
#ifdef O_EXCL
            পক্ষ 'x':
                exclusive = 1;
                ভঙ্গ;
#endif
            পক্ষ 'f':
                state->strategy = Z_FILTERED;
                ভঙ্গ;
            পক্ষ 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                ভঙ্গ;
            পক্ষ 'R':
                state->strategy = Z_RLE;
                ভঙ্গ;
            পক্ষ 'F':
                state->strategy = Z_FIXED;
                ভঙ্গ;
            পক্ষ 'G':
                state->direct = -1;
                ভঙ্গ;
#ifdef O_NONBLOCK
            পক্ষ 'N':
                oflag |= O_NONBLOCK;
                ভঙ্গ;
#endif
            পক্ষ 'T':
                state->direct = 1;
                ভঙ্গ;
            স্বভাব:        /* could consider as an error, but just ignore */
                ;
            }
        mode++;
    }

    /* must provide an "r", "w", or "a" */
    যদি (state->mode == GZ_NONE) {
        মুক্ত(state);
        প্রত্যাগম রিক্ত;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    যদি (state->mode == GZ_READ) {
        যদি (state->direct == 1) {
            /* can't force a transparent read */
            মুক্ত(state);
            প্রত্যাগম রিক্ত;
        }
        যদি (state->direct == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->direct = 1;
    }
    অন্যথা যদি (state->direct == -1) {
        /* "G" has no meaning when writing -- disallow it */
        মুক্ত(state);
        প্রত্যাগম রিক্ত;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ifdef WIDECHAR
    যদি (fd == -2)
        len = wcstombs(রিক্ত, path, 0);
    অন্যথা
#endif
        len = strlen((ধ্রুব বর্ণ *)path);
    state->path = (বর্ণ *)malloc(len + 1);
    যদি (state->path == রিক্ত) {
        মুক্ত(state);
        প্রত্যাগম রিক্ত;
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
        (শূন্য)snprintf(state->path, len + 1, "%s", (ধ্রুব বর্ণ *)path);
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
        state->fd = open((ধ্রুব বর্ণ *)path, oflag, 0666);
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
        প্রত্যাগম রিক্ত;
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
    প্রত্যাগম (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ধ্রুব বর্ণ *path, ধ্রুব বর্ণ *mode) {
    প্রত্যাগম gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ধ্রুব বর্ণ *path, ধ্রুব বর্ণ *mode) {
    প্রত্যাগম gz_open(path, -1, mode);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(পূর্ণাঙ্ক fd, ধ্রুব বর্ণ *mode) {
    বর্ণ *path;         /* identifier for error messages */
    gzFile gz;

    যদি (fd == -1 || (path = (বর্ণ *)malloc(7 + 3 * পরিমাণ(পূর্ণাঙ্ক))) == রিক্ত)
        প্রত্যাগম রিক্ত;
#যদি !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (শূন্য)snprintf(path, 7 + 3 * পরিমাণ(পূর্ণাঙ্ক), "<fd:%d>", fd);
#অন্যথা
    sprintf(path, "<fd:%d>", fd);   /* for debugging */
#endif
    gz = gz_open(path, fd, mode);
    মুক্ত(path);
    প্রত্যাগম gz;
}

/* -- see zlib.h -- */
#ifdef WIDECHAR
gzFile ZEXPORT gzopen_w(ধ্রুব wchar_t *path, ধ্রুব বর্ণ *mode) {
    প্রত্যাগম gz_open(path, -2, mode);
}
#endif

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzbuffer(gzFile file, unsigned size) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == রিক্ত)
        প্রত্যাগম -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        প্রত্যাগম -1;

    /* make sure we haven't already allocated memory */
    যদি (state->size != 0)
        প্রত্যাগম -1;

    /* check and set requested size */
    যদি ((size << 1) < size)
        প্রত্যাগম -1;              /* need to be able to double it */
    যদি (size < 8)
        size = 8;               /* needed to behave well with flushing */
    state->want = size;
    প্রত্যাগম 0;
}

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    যদি (file == রিক্ত)
        প্রত্যাগম -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    যদি (state->mode != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        প্রত্যাগম -1;

    /* back up and start over */
    যদি (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        প্রত্যাগম -1;
    gz_reset(state);
    প্রত্যাগম 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t offset, পূর্ণাঙ্ক whence) {
    unsigned n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == রিক্ত)
        প্রত্যাগম -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        প্রত্যাগম -1;

    /* check that there's no error */
    যদি (state->err != Z_OK && state->err != Z_BUF_ERROR)
        প্রত্যাগম -1;

    /* can only seek from start or relative to current position */
    যদি (whence != SEEK_SET && whence != SEEK_CUR)
        প্রত্যাগম -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    যদি (offset > GZ_OFF_MAX - pos)
        প্রত্যাগম -1;                          /* would overflow z_off64_t */
    যদি (offset < -pos)
        প্রত্যাগম -1;                          /* before start of file */
    pos += offset;

    /* if within raw area while reading, just go there */
    যদি (state->mode == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        যদি (ret == -1)
            প্রত্যাগম -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, রিক্ত);
        state->strm.avail_in = 0;
        state->x.pos = pos;
        প্রত্যাগম pos;
    }

    /* rewind if needed for back seek when reading */
    যদি (pos < state->x.pos) {
        যদি (state->mode != GZ_READ)         /* writing -- can't go backwards */
            প্রত্যাগম -1;
        যদি (gzrewind(file) == -1)           /* rewind, then skip to pos */
            প্রত্যাগম -1;
    }
    offset = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    যদি (state->mode == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > offset ?
            (unsigned)offset : state->x.have;
        state->x.have -= n;
        state->x.next += n;
        state->x.pos += n;
        offset -= n;
    }

    /* request skip (if not zero) */
    state->skip = offset;
    প্রত্যাগম pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t offset, পূর্ণাঙ্ক whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)offset, whence);
    প্রত্যাগম ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == রিক্ত)
        প্রত্যাগম -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        প্রত্যাগম -1;

    /* return position */
    প্রত্যাগম state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    প্রত্যাগম ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t offset;
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == রিক্ত)
        প্রত্যাগম -1;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        প্রত্যাগম -1;

    /* compute and return effective offset in file */
    offset = LSEEK(state->fd, 0, SEEK_CUR);
    যদি (offset == -1)
        প্রত্যাগম -1;
    যদি (state->mode == GZ_READ)             /* reading */
        offset -= state->strm.avail_in;     /* don't count buffered input */
    প্রত্যাগম offset;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    প্রত্যাগম ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
পূর্ণাঙ্ক ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == রিক্ত)
        প্রত্যাগম 0;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        প্রত্যাগম 0;

    /* return end-of-file state */
    প্রত্যাগম state->mode == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ধ্রুব বর্ণ * ZEXPORT gzerror(gzFile file, পূর্ণাঙ্ক *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == রিক্ত)
        প্রত্যাগম রিক্ত;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        প্রত্যাগম রিক্ত;

    /* return error information */
    যদি (errnum != রিক্ত)
        *errnum = state->err;
    প্রত্যাগম state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->বার্তা == রিক্ত ? "" : state->বার্তা);
}

/* -- see zlib.h -- */
শূন্য ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    যদি (file == রিক্ত)
        প্রত্যাগম;
    state = (gz_statep)file;
    যদি (state->mode != GZ_READ && state->mode != GZ_WRITE)
        প্রত্যাগম;

    /* clear error and end-of-file */
    যদি (state->mode == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, রিক্ত);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
শূন্য ZLIB_INTERNAL gz_error(gz_statep state, পূর্ণাঙ্ক err, ধ্রুব বর্ণ *বার্তা) {
    /* free previously allocated message and clear */
    যদি (state->বার্তা != রিক্ত) {
        যদি (state->err != Z_MEM_ERROR)
            মুক্ত(state->বার্তা);
        state->বার্তা = রিক্ত;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    যদি (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    যদি (বার্তা == রিক্ত)
        প্রত্যাগম;

    /* for an out of memory error, return literal string when requested */
    যদি (err == Z_MEM_ERROR)
        প্রত্যাগম;

    /* construct error message with path */
    যদি ((state->বার্তা = (বর্ণ *)malloc(strlen(state->path) + strlen(বার্তা) + 3)) ==
            রিক্ত) {
        state->err = Z_MEM_ERROR;
        প্রত্যাগম;
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
    প্রত্যাগম INT_MAX;
#অন্যথা
    unsigned p = 1, q;

    কুরু {
        q = p;
        p <<= 1;
        p++;
    } যাবত্ (p > q);
    প্রত্যাগম q >> 1;
#endif
}
