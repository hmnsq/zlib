/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

#إذا defined(__DJGPP__)
#  define LSEEK llseek
#وإلاإذا defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#وإلاإذا defined(LARGEFILE64_ذريعو) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#وإلا
#  define LSEEK lseek
#جيڪڏهن_پورو

#إذا defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
حرف ZLIB_اندروني *gz_strwinerror(DWORD error) {
    ساكن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_آخري_Error();
    DWORD chars = Format_پيغام(FORMAT_پيغام_FROM_SYSTEM
        | FORMAT_پيغام_ALLOCATE_بفر,
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

    سيٽ_آخري_Error(lasterr);
    أرجع buf;
}

#جيڪڏهن_پورو /* UNDER_CE */

/* Reset gzip file state */
local فراغ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    إذا (state->گهڻائي == GZ_READ) {   /* for reading ... */
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
    state->strm.avail_اندريون = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ثابت فراغ *رستو, صحيح fd, ثابت حرف *گهڻائي) {
    gz_statep state;
    z_سائيز_t len;
    صحيح oflag = 0;
#جيڪڏهن_وصف_ٿيل O_EXCL
    صحيح exclusive = 0;
#جيڪڏهن_پورو

    /* check input */
    إذا (رستو == عدم || گهڻائي == عدم)
        أرجع عدم;

    /* allocate gzFile structure to return */
    state = (gz_statep)يادگيري_ڏيو(حجم(gz_state));
    إذا (state == عدم)
        أرجع عدم;
    state->سائيز = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->نياپو = عدم;          /* no error message yet */

    /* interpret mode */
    state->گهڻائي = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->سڌو = 0;
    طالما (*گهڻائي) {
        إذا (*گهڻائي >= '0' && *گهڻائي <= '9')
            state->level = *گهڻائي - '0';
        وإلا
            بدل (*گهڻائي) {
            حالة 'r':
                state->گهڻائي = GZ_READ;
                اكسر;
#جيڪڏهن_وصف_نه_ٿيل NO_GZCOMPRESS
            حالة 'w':
                state->گهڻائي = GZ_WRITE;
                اكسر;
            حالة 'a':
                state->گهڻائي = GZ_APPEND;
                اكسر;
#جيڪڏهن_پورو
            حالة '+':       /* can't read and write at the same time */
                حرر(state);
                أرجع عدم;
            حالة 'b':       /* ignore -- will request binary anyway */
                اكسر;
#جيڪڏهن_وصف_ٿيل O_CLOEXEC
            حالة 'e':
                oflag |= O_CLOEXEC;
                اكسر;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل O_EXCL
            حالة 'x':
                exclusive = 1;
                اكسر;
#جيڪڏهن_پورو
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
                state->سڌو = -1;
                اكسر;
#جيڪڏهن_وصف_ٿيل O_NONBLOCK
            حالة 'N':
                oflag |= O_NONBLOCK;
                اكسر;
#جيڪڏهن_پورو
            حالة 'T':
                state->سڌو = 1;
                اكسر;
            افتراضي:        /* could consider as an error, but just ignore */
                ;
            }
        گهڻائي++;
    }

    /* must provide an "r", "w", or "a" */
    إذا (state->گهڻائي == GZ_NONE) {
        حرر(state);
        أرجع عدم;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    إذا (state->گهڻائي == GZ_READ) {
        إذا (state->سڌو == 1) {
            /* can't force a transparent read */
            حرر(state);
            أرجع عدم;
        }
        إذا (state->سڌو == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->سڌو = 1;
    }
    وإلا إذا (state->سڌو == -1) {
        /* "G" has no meaning when writing -- disallow it */
        حرر(state);
        أرجع عدم;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#جيڪڏهن_وصف_ٿيل WIDECHAR
    إذا (fd == -2)
        len = wcstombs(عدم, رستو, 0);
    وإلا
#جيڪڏهن_پورو
        len = لڙي_ڊيگهه((ثابت حرف *)رستو);
    state->رستو = (حرف *)يادگيري_ڏيو(len + 1);
    إذا (state->رستو == عدم) {
        حرر(state);
        أرجع عدم;
    }
#جيڪڏهن_وصف_ٿيل WIDECHAR
    إذا (fd == -2) {
        إذا (len)
            wcstombs(state->رستو, رستو, len + 1);
        وإلا
            *(state->رستو) = 0;
    }
    وإلا
#جيڪڏهن_پورو
    {
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (فراغ)snprintf(state->رستو, len + 1, "%s", (ثابت حرف *)رستو);
#وإلا
        strcpy(state->رستو, رستو);
#جيڪڏهن_پورو
    }

    /* compute the flags for open() */
    oflag |=
#جيڪڏهن_وصف_ٿيل O_LARGEFILE
        O_LARGEFILE |
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل O_ٻٽو
        O_ٻٽو |
#جيڪڏهن_پورو
        (state->گهڻائي == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#جيڪڏهن_وصف_ٿيل O_EXCL
          (exclusive ? O_EXCL : 0) |
#جيڪڏهن_پورو
          (state->گهڻائي == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    إذا (fd == -1)
        state->fd = open((ثابت حرف *)رستو, oflag, 0666);
#جيڪڏهن_وصف_ٿيل WIDECHAR
    وإلا إذا (fd == -2)
        state->fd = _wopen(رستو, oflag, _S_IREAD | _S_IWRITE);
#جيڪڏهن_پورو
    وإلا {
#جيڪڏهن_وصف_ٿيل O_NONBLOCK
        إذا (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل O_CLOEXEC
        إذا (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#جيڪڏهن_پورو
        state->fd = fd;
    }
    إذا (state->fd == -1) {
        حرر(state->رستو);
        حرر(state);
        أرجع عدم;
    }
    إذا (state->گهڻائي == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->گهڻائي = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    إذا (state->گهڻائي == GZ_READ) {
        state->شروعات = LSEEK(state->fd, 0, SEEK_CUR);
        إذا (state->شروعات == -1) state->شروعات = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    أرجع (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ثابت حرف *رستو, ثابت حرف *گهڻائي) {
    أرجع gz_open(رستو, -1, گهڻائي);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ثابت حرف *رستو, ثابت حرف *گهڻائي) {
    أرجع gz_open(رستو, -1, گهڻائي);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(صحيح fd, ثابت حرف *گهڻائي) {
    حرف *رستو;         /* identifier for error messages */
    gzFile gz;

    إذا (fd == -1 || (رستو = (حرف *)يادگيري_ڏيو(7 + 3 * حجم(صحيح))) == عدم)
        أرجع عدم;
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(رستو, 7 + 3 * حجم(صحيح), "<fd:%d>", fd);
#وإلا
    sprintf(رستو, "<fd:%d>", fd);   /* for debugging */
#جيڪڏهن_پورو
    gz = gz_open(رستو, fd, گهڻائي);
    حرر(رستو);
    أرجع gz;
}

/* -- see zlib.h -- */
#جيڪڏهن_وصف_ٿيل WIDECHAR
gzFile ZEXPORT gzopen_w(ثابت wchar_t *رستو, ثابت حرف *گهڻائي) {
    أرجع gz_open(رستو, -2, گهڻائي);
}
#جيڪڏهن_پورو

/* -- see zlib.h -- */
صحيح ZEXPORT gzbuffer(gzFile file, بنانشان سائيز) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->گهڻائي != GZ_READ && state->گهڻائي != GZ_WRITE)
        أرجع -1;

    /* make sure we haven't already allocated memory */
    إذا (state->سائيز != 0)
        أرجع -1;

    /* check and set requested size */
    إذا ((سائيز << 1) < سائيز)
        أرجع -1;              /* need to be able to double it */
    إذا (سائيز < 8)
        سائيز = 8;               /* needed to behave well with flushing */
    state->want = سائيز;
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
    إذا (state->گهڻائي != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        أرجع -1;

    /* back up and start over */
    إذا (LSEEK(state->fd, state->شروعات, SEEK_SET) == -1)
        أرجع -1;
    gz_reset(state);
    أرجع 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t وٿي, صحيح whence) {
    بنانشان n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->گهڻائي != GZ_READ && state->گهڻائي != GZ_WRITE)
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
    إذا (وٿي > GZ_OFF_MAX - pos)
        أرجع -1;                          /* would overflow z_off64_t */
    إذا (وٿي < -pos)
        أرجع -1;                          /* before start of file */
    pos += وٿي;

    /* if within raw area while reading, just go there */
    إذا (state->گهڻائي == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        إذا (ret == -1)
            أرجع -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, عدم);
        state->strm.avail_اندريون = 0;
        state->x.pos = pos;
        أرجع pos;
    }

    /* rewind if needed for back seek when reading */
    إذا (pos < state->x.pos) {
        إذا (state->گهڻائي != GZ_READ)         /* writing -- can't go backwards */
            أرجع -1;
        إذا (gzrewind(file) == -1)           /* rewind, then skip to pos */
            أرجع -1;
    }
    وٿي = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    إذا (state->گهڻائي == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > وٿي ?
            (بنانشان)وٿي : state->x.have;
        state->x.have -= n;
        state->x.ايندڙ += n;
        state->x.pos += n;
        وٿي -= n;
    }

    /* request skip (if not zero) */
    state->skip = وٿي;
    أرجع pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t وٿي, صحيح whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)وٿي, whence);
    أرجع ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->گهڻائي != GZ_READ && state->گهڻائي != GZ_WRITE)
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
    z_off64_t وٿي;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->گهڻائي != GZ_READ && state->گهڻائي != GZ_WRITE)
        أرجع -1;

    /* compute and return effective offset in file */
    وٿي = LSEEK(state->fd, 0, SEEK_CUR);
    إذا (وٿي == -1)
        أرجع -1;
    إذا (state->گهڻائي == GZ_READ)             /* reading */
        وٿي -= state->strm.avail_اندريون;     /* don't count buffered input */
    أرجع وٿي;
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
    إذا (state->گهڻائي != GZ_READ && state->گهڻائي != GZ_WRITE)
        أرجع 0;

    /* return end-of-file state */
    أرجع state->گهڻائي == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ثابت حرف * ZEXPORT gzerror(gzFile file, صحيح *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع عدم;
    state = (gz_statep)file;
    إذا (state->گهڻائي != GZ_READ && state->گهڻائي != GZ_WRITE)
        أرجع عدم;

    /* return error information */
    إذا (errnum != عدم)
        *errnum = state->err;
    أرجع state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->نياپو == عدم ? "" : state->نياپو);
}

/* -- see zlib.h -- */
فراغ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع;
    state = (gz_statep)file;
    إذا (state->گهڻائي != GZ_READ && state->گهڻائي != GZ_WRITE)
        أرجع;

    /* clear error and end-of-file */
    إذا (state->گهڻائي == GZ_READ) {
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
فراغ ZLIB_اندروني gz_error(gz_statep state, صحيح err, ثابت حرف *نياپو) {
    /* free previously allocated message and clear */
    إذا (state->نياپو != عدم) {
        إذا (state->err != Z_MEM_ERROR)
            حرر(state->نياپو);
        state->نياپو = عدم;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    إذا (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    إذا (نياپو == عدم)
        أرجع;

    /* for an out of memory error, return literal string when requested */
    إذا (err == Z_MEM_ERROR)
        أرجع;

    /* construct error message with path */
    إذا ((state->نياپو = (حرف *)يادگيري_ڏيو(لڙي_ڊيگهه(state->رستو) + لڙي_ڊيگهه(نياپو) + 3)) ==
            عدم) {
        state->err = Z_MEM_ERROR;
        أرجع;
    }
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(state->نياپو, لڙي_ڊيگهه(state->رستو) + لڙي_ڊيگهه(نياپو) + 3,
                   "%s%s%s", state->رستو, ": ", نياپو);
#وإلا
    strcpy(state->نياپو, state->رستو);
    strcat(state->نياپو, ": ");
    strcat(state->نياپو, نياپو);
#جيڪڏهن_پورو
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
بنانشان ZLIB_اندروني gz_intmax(فراغ) {
#جيڪڏهن_وصف_ٿيل وڌ_کان_وڌ_عدد
    أرجع وڌ_کان_وڌ_عدد;
#وإلا
    بنانشان p = 1, q;

    افعل {
        q = p;
        p <<= 1;
        p++;
    } طالما (p > q);
    أرجع q >> 1;
#جيڪڏهن_پورو
}
