/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

#إذا defined(__DJGPP__)
#  define LSEEK llseek
#وإلاإذا defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#وإلاإذا defined(LARGEFILE64_سرچینه) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#وإلا
#  define LSEEK lseek
#د_که_پای

#إذا defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
حرف ZLIB_دننی *gz_strwinerror(DWORD error) {
    ساكن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_وروستی_Error();
    DWORD chars = Format_پیغام(FORMAT_پیغام_FROM_SYSTEM
        | FORMAT_پیغام_ALLOCATE_بفر,
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

    سېټ_وروستی_Error(lasterr);
    أرجع buf;
}

#د_که_پای /* UNDER_CE */

/* Reset gzip file state */
local فراغ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    إذا (state->ډېرپېښ == GZ_READ) {   /* for reading ... */
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
    state->strm.avail_دننه = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ثابت فراغ *لار, صحيح fd, ثابت حرف *ډېرپېښ) {
    gz_statep state;
    z_اندازه_t len;
    صحيح oflag = 0;
#که_تعریف_شوی O_EXCL
    صحيح exclusive = 0;
#د_که_پای

    /* check input */
    إذا (لار == عدم || ډېرپېښ == عدم)
        أرجع عدم;

    /* allocate gzFile structure to return */
    state = (gz_statep)حافظه_ورکړه(حجم(gz_state));
    إذا (state == عدم)
        أرجع عدم;
    state->اندازه = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->پیغام = عدم;          /* no error message yet */

    /* interpret mode */
    state->ډېرپېښ = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->مستقیم = 0;
    طالما (*ډېرپېښ) {
        إذا (*ډېرپېښ >= '0' && *ډېرپېښ <= '9')
            state->level = *ډېرپېښ - '0';
        وإلا
            بدل (*ډېرپېښ) {
            حالة 'r':
                state->ډېرپېښ = GZ_READ;
                اكسر;
#که_نه_تعریف_شوی NO_GZCOMPRESS
            حالة 'w':
                state->ډېرپېښ = GZ_WRITE;
                اكسر;
            حالة 'a':
                state->ډېرپېښ = GZ_APPEND;
                اكسر;
#د_که_پای
            حالة '+':       /* can't read and write at the same time */
                حرر(state);
                أرجع عدم;
            حالة 'b':       /* ignore -- will request binary anyway */
                اكسر;
#که_تعریف_شوی O_CLOEXEC
            حالة 'e':
                oflag |= O_CLOEXEC;
                اكسر;
#د_که_پای
#که_تعریف_شوی O_EXCL
            حالة 'x':
                exclusive = 1;
                اكسر;
#د_که_پای
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
                state->مستقیم = -1;
                اكسر;
#که_تعریف_شوی O_NONBLOCK
            حالة 'N':
                oflag |= O_NONBLOCK;
                اكسر;
#د_که_پای
            حالة 'T':
                state->مستقیم = 1;
                اكسر;
            افتراضي:        /* could consider as an error, but just ignore */
                ;
            }
        ډېرپېښ++;
    }

    /* must provide an "r", "w", or "a" */
    إذا (state->ډېرپېښ == GZ_NONE) {
        حرر(state);
        أرجع عدم;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    إذا (state->ډېرپېښ == GZ_READ) {
        إذا (state->مستقیم == 1) {
            /* can't force a transparent read */
            حرر(state);
            أرجع عدم;
        }
        إذا (state->مستقیم == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->مستقیم = 1;
    }
    وإلا إذا (state->مستقیم == -1) {
        /* "G" has no meaning when writing -- disallow it */
        حرر(state);
        أرجع عدم;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#که_تعریف_شوی WIDECHAR
    إذا (fd == -2)
        len = wcstombs(عدم, لار, 0);
    وإلا
#د_که_پای
        len = مزي_اوږدوالی((ثابت حرف *)لار);
    state->لار = (حرف *)حافظه_ورکړه(len + 1);
    إذا (state->لار == عدم) {
        حرر(state);
        أرجع عدم;
    }
#که_تعریف_شوی WIDECHAR
    إذا (fd == -2) {
        إذا (len)
            wcstombs(state->لار, لار, len + 1);
        وإلا
            *(state->لار) = 0;
    }
    وإلا
#د_که_پای
    {
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (فراغ)snprintf(state->لار, len + 1, "%s", (ثابت حرف *)لار);
#وإلا
        strcpy(state->لار, لار);
#د_که_پای
    }

    /* compute the flags for open() */
    oflag |=
#که_تعریف_شوی O_LARGEFILE
        O_LARGEFILE |
#د_که_پای
#که_تعریف_شوی O_دوګونی
        O_دوګونی |
#د_که_پای
        (state->ډېرپېښ == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#که_تعریف_شوی O_EXCL
          (exclusive ? O_EXCL : 0) |
#د_که_پای
          (state->ډېرپېښ == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    إذا (fd == -1)
        state->fd = open((ثابت حرف *)لار, oflag, 0666);
#که_تعریف_شوی WIDECHAR
    وإلا إذا (fd == -2)
        state->fd = _wopen(لار, oflag, _S_IREAD | _S_IWRITE);
#د_که_پای
    وإلا {
#که_تعریف_شوی O_NONBLOCK
        إذا (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#د_که_پای
#که_تعریف_شوی O_CLOEXEC
        إذا (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#د_که_پای
        state->fd = fd;
    }
    إذا (state->fd == -1) {
        حرر(state->لار);
        حرر(state);
        أرجع عدم;
    }
    إذا (state->ډېرپېښ == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->ډېرپېښ = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    إذا (state->ډېرپېښ == GZ_READ) {
        state->پیل = LSEEK(state->fd, 0, SEEK_CUR);
        إذا (state->پیل == -1) state->پیل = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    أرجع (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ثابت حرف *لار, ثابت حرف *ډېرپېښ) {
    أرجع gz_open(لار, -1, ډېرپېښ);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ثابت حرف *لار, ثابت حرف *ډېرپېښ) {
    أرجع gz_open(لار, -1, ډېرپېښ);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(صحيح fd, ثابت حرف *ډېرپېښ) {
    حرف *لار;         /* identifier for error messages */
    gzFile gz;

    إذا (fd == -1 || (لار = (حرف *)حافظه_ورکړه(7 + 3 * حجم(صحيح))) == عدم)
        أرجع عدم;
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(لار, 7 + 3 * حجم(صحيح), "<fd:%d>", fd);
#وإلا
    sprintf(لار, "<fd:%d>", fd);   /* for debugging */
#د_که_پای
    gz = gz_open(لار, fd, ډېرپېښ);
    حرر(لار);
    أرجع gz;
}

/* -- see zlib.h -- */
#که_تعریف_شوی WIDECHAR
gzFile ZEXPORT gzopen_w(ثابت wchar_t *لار, ثابت حرف *ډېرپېښ) {
    أرجع gz_open(لار, -2, ډېرپېښ);
}
#د_که_پای

/* -- see zlib.h -- */
صحيح ZEXPORT gzbuffer(gzFile file, بینښه اندازه) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->ډېرپېښ != GZ_READ && state->ډېرپېښ != GZ_WRITE)
        أرجع -1;

    /* make sure we haven't already allocated memory */
    إذا (state->اندازه != 0)
        أرجع -1;

    /* check and set requested size */
    إذا ((اندازه << 1) < اندازه)
        أرجع -1;              /* need to be able to double it */
    إذا (اندازه < 8)
        اندازه = 8;               /* needed to behave well with flushing */
    state->want = اندازه;
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
    إذا (state->ډېرپېښ != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        أرجع -1;

    /* back up and start over */
    إذا (LSEEK(state->fd, state->پیل, SEEK_SET) == -1)
        أرجع -1;
    gz_reset(state);
    أرجع 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t دننه_واټن, صحيح whence) {
    بینښه n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->ډېرپېښ != GZ_READ && state->ډېرپېښ != GZ_WRITE)
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
    إذا (دننه_واټن > GZ_OFF_MAX - pos)
        أرجع -1;                          /* would overflow z_off64_t */
    إذا (دننه_واټن < -pos)
        أرجع -1;                          /* before start of file */
    pos += دننه_واټن;

    /* if within raw area while reading, just go there */
    إذا (state->ډېرپېښ == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        إذا (ret == -1)
            أرجع -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, عدم);
        state->strm.avail_دننه = 0;
        state->x.pos = pos;
        أرجع pos;
    }

    /* rewind if needed for back seek when reading */
    إذا (pos < state->x.pos) {
        إذا (state->ډېرپېښ != GZ_READ)         /* writing -- can't go backwards */
            أرجع -1;
        إذا (gzrewind(file) == -1)           /* rewind, then skip to pos */
            أرجع -1;
    }
    دننه_واټن = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    إذا (state->ډېرپېښ == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > دننه_واټن ?
            (بینښه)دننه_واټن : state->x.have;
        state->x.have -= n;
        state->x.بل += n;
        state->x.pos += n;
        دننه_واټن -= n;
    }

    /* request skip (if not zero) */
    state->skip = دننه_واټن;
    أرجع pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t دننه_واټن, صحيح whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)دننه_واټن, whence);
    أرجع ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->ډېرپېښ != GZ_READ && state->ډېرپېښ != GZ_WRITE)
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
    z_off64_t دننه_واټن;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->ډېرپېښ != GZ_READ && state->ډېرپېښ != GZ_WRITE)
        أرجع -1;

    /* compute and return effective offset in file */
    دننه_واټن = LSEEK(state->fd, 0, SEEK_CUR);
    إذا (دننه_واټن == -1)
        أرجع -1;
    إذا (state->ډېرپېښ == GZ_READ)             /* reading */
        دننه_واټن -= state->strm.avail_دننه;     /* don't count buffered input */
    أرجع دننه_واټن;
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
    إذا (state->ډېرپېښ != GZ_READ && state->ډېرپېښ != GZ_WRITE)
        أرجع 0;

    /* return end-of-file state */
    أرجع state->ډېرپېښ == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ثابت حرف * ZEXPORT gzerror(gzFile file, صحيح *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع عدم;
    state = (gz_statep)file;
    إذا (state->ډېرپېښ != GZ_READ && state->ډېرپېښ != GZ_WRITE)
        أرجع عدم;

    /* return error information */
    إذا (errnum != عدم)
        *errnum = state->err;
    أرجع state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->پیغام == عدم ? "" : state->پیغام);
}

/* -- see zlib.h -- */
فراغ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع;
    state = (gz_statep)file;
    إذا (state->ډېرپېښ != GZ_READ && state->ډېرپېښ != GZ_WRITE)
        أرجع;

    /* clear error and end-of-file */
    إذا (state->ډېرپېښ == GZ_READ) {
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
فراغ ZLIB_دننی gz_error(gz_statep state, صحيح err, ثابت حرف *پیغام) {
    /* free previously allocated message and clear */
    إذا (state->پیغام != عدم) {
        إذا (state->err != Z_MEM_ERROR)
            حرر(state->پیغام);
        state->پیغام = عدم;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    إذا (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    إذا (پیغام == عدم)
        أرجع;

    /* for an out of memory error, return literal string when requested */
    إذا (err == Z_MEM_ERROR)
        أرجع;

    /* construct error message with path */
    إذا ((state->پیغام = (حرف *)حافظه_ورکړه(مزي_اوږدوالی(state->لار) + مزي_اوږدوالی(پیغام) + 3)) ==
            عدم) {
        state->err = Z_MEM_ERROR;
        أرجع;
    }
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(state->پیغام, مزي_اوږدوالی(state->لار) + مزي_اوږدوالی(پیغام) + 3,
                   "%s%s%s", state->لار, ": ", پیغام);
#وإلا
    strcpy(state->پیغام, state->لار);
    strcat(state->پیغام, ": ");
    strcat(state->پیغام, پیغام);
#د_که_پای
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
بینښه ZLIB_دننی gz_intmax(فراغ) {
#که_تعریف_شوی لوی_عدد
    أرجع لوی_عدد;
#وإلا
    بینښه p = 1, q;

    افعل {
        q = p;
        p <<= 1;
        p++;
    } طالما (p > q);
    أرجع q >> 1;
#د_که_پای
}
