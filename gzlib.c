/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

#إذا defined(__DJGPP__)
#  define LSEEK llseek
#وإلاإذا defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#وإلاإذا defined(LARGEFILE64_سروت) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#وإلا
#  define LSEEK lseek
#جے_انت

#إذا defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
حرف ZLIB_اندرونی *gz_strwinerror(DWORD error) {
    ساكن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_آخری_Error();
    DWORD chars = Format_سنیہا(FORMAT_سنیہا_FROM_SYSTEM
        | FORMAT_سنیہا_ALLOCATE_بفر,
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

    سیٹ_آخری_Error(lasterr);
    أرجع buf;
}

#جے_انت /* UNDER_CE */

/* Reset gzip file state */
local فراغ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    إذا (state->منوال == GZ_READ) {   /* for reading ... */
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
    state->strm.avail_وچ = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ثابت فراغ *رستہ, صحيح fd, ثابت حرف *منوال) {
    gz_statep state;
    z_ناپ_t len;
    صحيح oflag = 0;
#جے_تعریف_شدہ O_EXCL
    صحيح exclusive = 0;
#جے_انت

    /* check input */
    إذا (رستہ == عدم || منوال == عدم)
        أرجع عدم;

    /* allocate gzFile structure to return */
    state = (gz_statep)حافظہ_دیو(حجم(gz_state));
    إذا (state == عدم)
        أرجع عدم;
    state->ناپ = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->پیغام = عدم;          /* no error message yet */

    /* interpret mode */
    state->منوال = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->سدھا = 0;
    طالما (*منوال) {
        إذا (*منوال >= '0' && *منوال <= '9')
            state->level = *منوال - '0';
        وإلا
            بدل (*منوال) {
            حالة 'r':
                state->منوال = GZ_READ;
                اكسر;
#جے_بنا_تعریف NO_GZCOMPRESS
            حالة 'w':
                state->منوال = GZ_WRITE;
                اكسر;
            حالة 'a':
                state->منوال = GZ_APPEND;
                اكسر;
#جے_انت
            حالة '+':       /* can't read and write at the same time */
                حرر(state);
                أرجع عدم;
            حالة 'b':       /* ignore -- will request binary anyway */
                اكسر;
#جے_تعریف_شدہ O_CLOEXEC
            حالة 'e':
                oflag |= O_CLOEXEC;
                اكسر;
#جے_انت
#جے_تعریف_شدہ O_EXCL
            حالة 'x':
                exclusive = 1;
                اكسر;
#جے_انت
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
                state->سدھا = -1;
                اكسر;
#جے_تعریف_شدہ O_NONBLOCK
            حالة 'N':
                oflag |= O_NONBLOCK;
                اكسر;
#جے_انت
            حالة 'T':
                state->سدھا = 1;
                اكسر;
            افتراضي:        /* could consider as an error, but just ignore */
                ;
            }
        منوال++;
    }

    /* must provide an "r", "w", or "a" */
    إذا (state->منوال == GZ_NONE) {
        حرر(state);
        أرجع عدم;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    إذا (state->منوال == GZ_READ) {
        إذا (state->سدھا == 1) {
            /* can't force a transparent read */
            حرر(state);
            أرجع عدم;
        }
        إذا (state->سدھا == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->سدھا = 1;
    }
    وإلا إذا (state->سدھا == -1) {
        /* "G" has no meaning when writing -- disallow it */
        حرر(state);
        أرجع عدم;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#جے_تعریف_شدہ WIDECHAR
    إذا (fd == -2)
        len = wcstombs(عدم, رستہ, 0);
    وإلا
#جے_انت
        len = لڑی_لمبائی((ثابت حرف *)رستہ);
    state->رستہ = (حرف *)حافظہ_دیو(len + 1);
    إذا (state->رستہ == عدم) {
        حرر(state);
        أرجع عدم;
    }
#جے_تعریف_شدہ WIDECHAR
    إذا (fd == -2) {
        إذا (len)
            wcstombs(state->رستہ, رستہ, len + 1);
        وإلا
            *(state->رستہ) = 0;
    }
    وإلا
#جے_انت
    {
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (فراغ)snprintf(state->رستہ, len + 1, "%s", (ثابت حرف *)رستہ);
#وإلا
        strcpy(state->رستہ, رستہ);
#جے_انت
    }

    /* compute the flags for open() */
    oflag |=
#جے_تعریف_شدہ O_LARGEFILE
        O_LARGEFILE |
#جے_انت
#جے_تعریف_شدہ O_ثنائی
        O_ثنائی |
#جے_انت
        (state->منوال == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#جے_تعریف_شدہ O_EXCL
          (exclusive ? O_EXCL : 0) |
#جے_انت
          (state->منوال == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    إذا (fd == -1)
        state->fd = open((ثابت حرف *)رستہ, oflag, 0666);
#جے_تعریف_شدہ WIDECHAR
    وإلا إذا (fd == -2)
        state->fd = _wopen(رستہ, oflag, _S_IREAD | _S_IWRITE);
#جے_انت
    وإلا {
#جے_تعریف_شدہ O_NONBLOCK
        إذا (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#جے_انت
#جے_تعریف_شدہ O_CLOEXEC
        إذا (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#جے_انت
        state->fd = fd;
    }
    إذا (state->fd == -1) {
        حرر(state->رستہ);
        حرر(state);
        أرجع عدم;
    }
    إذا (state->منوال == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->منوال = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    إذا (state->منوال == GZ_READ) {
        state->شروع = LSEEK(state->fd, 0, SEEK_CUR);
        إذا (state->شروع == -1) state->شروع = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    أرجع (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ثابت حرف *رستہ, ثابت حرف *منوال) {
    أرجع gz_open(رستہ, -1, منوال);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ثابت حرف *رستہ, ثابت حرف *منوال) {
    أرجع gz_open(رستہ, -1, منوال);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(صحيح fd, ثابت حرف *منوال) {
    حرف *رستہ;         /* identifier for error messages */
    gzFile gz;

    إذا (fd == -1 || (رستہ = (حرف *)حافظہ_دیو(7 + 3 * حجم(صحيح))) == عدم)
        أرجع عدم;
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(رستہ, 7 + 3 * حجم(صحيح), "<fd:%d>", fd);
#وإلا
    sprintf(رستہ, "<fd:%d>", fd);   /* for debugging */
#جے_انت
    gz = gz_open(رستہ, fd, منوال);
    حرر(رستہ);
    أرجع gz;
}

/* -- see zlib.h -- */
#جے_تعریف_شدہ WIDECHAR
gzFile ZEXPORT gzopen_w(ثابت wchar_t *رستہ, ثابت حرف *منوال) {
    أرجع gz_open(رستہ, -2, منوال);
}
#جے_انت

/* -- see zlib.h -- */
صحيح ZEXPORT gzbuffer(gzFile file, بنانشانی ناپ) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        أرجع -1;

    /* make sure we haven't already allocated memory */
    إذا (state->ناپ != 0)
        أرجع -1;

    /* check and set requested size */
    إذا ((ناپ << 1) < ناپ)
        أرجع -1;              /* need to be able to double it */
    إذا (ناپ < 8)
        ناپ = 8;               /* needed to behave well with flushing */
    state->want = ناپ;
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
    إذا (state->منوال != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        أرجع -1;

    /* back up and start over */
    إذا (LSEEK(state->fd, state->شروع, SEEK_SET) == -1)
        أرجع -1;
    gz_reset(state);
    أرجع 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t وتھ, صحيح whence) {
    بنانشانی n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
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
    إذا (وتھ > GZ_OFF_MAX - pos)
        أرجع -1;                          /* would overflow z_off64_t */
    إذا (وتھ < -pos)
        أرجع -1;                          /* before start of file */
    pos += وتھ;

    /* if within raw area while reading, just go there */
    إذا (state->منوال == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        إذا (ret == -1)
            أرجع -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, عدم);
        state->strm.avail_وچ = 0;
        state->x.pos = pos;
        أرجع pos;
    }

    /* rewind if needed for back seek when reading */
    إذا (pos < state->x.pos) {
        إذا (state->منوال != GZ_READ)         /* writing -- can't go backwards */
            أرجع -1;
        إذا (gzrewind(file) == -1)           /* rewind, then skip to pos */
            أرجع -1;
    }
    وتھ = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    إذا (state->منوال == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > وتھ ?
            (بنانشانی)وتھ : state->x.have;
        state->x.have -= n;
        state->x.اگلا += n;
        state->x.pos += n;
        وتھ -= n;
    }

    /* request skip (if not zero) */
    state->skip = وتھ;
    أرجع pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t وتھ, صحيح whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)وتھ, whence);
    أرجع ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
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
    z_off64_t وتھ;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        أرجع -1;

    /* compute and return effective offset in file */
    وتھ = LSEEK(state->fd, 0, SEEK_CUR);
    إذا (وتھ == -1)
        أرجع -1;
    إذا (state->منوال == GZ_READ)             /* reading */
        وتھ -= state->strm.avail_وچ;     /* don't count buffered input */
    أرجع وتھ;
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
    إذا (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        أرجع 0;

    /* return end-of-file state */
    أرجع state->منوال == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ثابت حرف * ZEXPORT gzerror(gzFile file, صحيح *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع عدم;
    state = (gz_statep)file;
    إذا (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
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
    إذا (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        أرجع;

    /* clear error and end-of-file */
    إذا (state->منوال == GZ_READ) {
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
فراغ ZLIB_اندرونی gz_error(gz_statep state, صحيح err, ثابت حرف *پیغام) {
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
    إذا ((state->پیغام = (حرف *)حافظہ_دیو(لڑی_لمبائی(state->رستہ) + لڑی_لمبائی(پیغام) + 3)) ==
            عدم) {
        state->err = Z_MEM_ERROR;
        أرجع;
    }
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(state->پیغام, لڑی_لمبائی(state->رستہ) + لڑی_لمبائی(پیغام) + 3,
                   "%s%s%s", state->رستہ, ": ", پیغام);
#وإلا
    strcpy(state->پیغام, state->رستہ);
    strcat(state->پیغام, ": ");
    strcat(state->پیغام, پیغام);
#جے_انت
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
بنانشانی ZLIB_اندرونی gz_intmax(فراغ) {
#جے_تعریف_شدہ ودھ_توں_ودھ_عدد
    أرجع ودھ_توں_ودھ_عدد;
#وإلا
    بنانشانی p = 1, q;

    افعل {
        q = p;
        p <<= 1;
        p++;
    } طالما (p > q);
    أرجع q >> 1;
#جے_انت
}
