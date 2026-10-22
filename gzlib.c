/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ضمن "gzguts.h"

#إذا defined(__DJGPP__)
#  define LSEEK llseek
#وإلاإذا defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#وإلاإذا defined(LARGEFILE64_منبع) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#وإلا
#  define LSEEK lseek
#پایان_اگر

#إذا defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
حرف ZLIB_داخلی *gz_strwinerror(DWORD error) {
    ساكن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_آخرین_Error();
    DWORD chars = Format_پیام(FORMAT_پیام_FROM_SYSTEM
        | FORMAT_پیام_ALLOCATE_میانگیر,
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

    مجموعه_آخرین_Error(lasterr);
    أرجع buf;
}

#پایان_اگر /* UNDER_CE */

/* Reset gzip file state */
local فراغ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    إذا (state->نما == GZ_READ) {   /* for reading ... */
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
    state->strm.avail_درون = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ثابت فراغ *مسیر, صحيح fd, ثابت حرف *نما) {
    gz_statep state;
    z_اندازه_t len;
    صحيح oflag = 0;
#اگر_تعریف_شده O_EXCL
    صحيح exclusive = 0;
#پایان_اگر

    /* check input */
    إذا (مسیر == عدم || نما == عدم)
        أرجع عدم;

    /* allocate gzFile structure to return */
    state = (gz_statep)حافظه_بده(حجم(gz_state));
    إذا (state == عدم)
        أرجع عدم;
    state->اندازه = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->پیام = عدم;          /* no error message yet */

    /* interpret mode */
    state->نما = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->مستقیم = 0;
    طالما (*نما) {
        إذا (*نما >= '0' && *نما <= '9')
            state->level = *نما - '0';
        وإلا
            بدل (*نما) {
            حالة 'r':
                state->نما = GZ_READ;
                اكسر;
#اگر_تعریف_نشده NO_GZCOMPRESS
            حالة 'w':
                state->نما = GZ_WRITE;
                اكسر;
            حالة 'a':
                state->نما = GZ_APPEND;
                اكسر;
#پایان_اگر
            حالة '+':       /* can't read and write at the same time */
                حرر(state);
                أرجع عدم;
            حالة 'b':       /* ignore -- will request binary anyway */
                اكسر;
#اگر_تعریف_شده O_CLOEXEC
            حالة 'e':
                oflag |= O_CLOEXEC;
                اكسر;
#پایان_اگر
#اگر_تعریف_شده O_EXCL
            حالة 'x':
                exclusive = 1;
                اكسر;
#پایان_اگر
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
#اگر_تعریف_شده O_NONBLOCK
            حالة 'N':
                oflag |= O_NONBLOCK;
                اكسر;
#پایان_اگر
            حالة 'T':
                state->مستقیم = 1;
                اكسر;
            افتراضي:        /* could consider as an error, but just ignore */
                ;
            }
        نما++;
    }

    /* must provide an "r", "w", or "a" */
    إذا (state->نما == GZ_NONE) {
        حرر(state);
        أرجع عدم;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    إذا (state->نما == GZ_READ) {
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
#اگر_تعریف_شده WIDECHAR
    إذا (fd == -2)
        len = wcstombs(عدم, مسیر, 0);
    وإلا
#پایان_اگر
        len = طول_رشته((ثابت حرف *)مسیر);
    state->مسیر = (حرف *)حافظه_بده(len + 1);
    إذا (state->مسیر == عدم) {
        حرر(state);
        أرجع عدم;
    }
#اگر_تعریف_شده WIDECHAR
    إذا (fd == -2) {
        إذا (len)
            wcstombs(state->مسیر, مسیر, len + 1);
        وإلا
            *(state->مسیر) = 0;
    }
    وإلا
#پایان_اگر
    {
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (فراغ)snprintf(state->مسیر, len + 1, "%s", (ثابت حرف *)مسیر);
#وإلا
        strcpy(state->مسیر, مسیر);
#پایان_اگر
    }

    /* compute the flags for open() */
    oflag |=
#اگر_تعریف_شده O_LARGEFILE
        O_LARGEFILE |
#پایان_اگر
#اگر_تعریف_شده O_دودویی
        O_دودویی |
#پایان_اگر
        (state->نما == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#اگر_تعریف_شده O_EXCL
          (exclusive ? O_EXCL : 0) |
#پایان_اگر
          (state->نما == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    إذا (fd == -1)
        state->fd = open((ثابت حرف *)مسیر, oflag, 0666);
#اگر_تعریف_شده WIDECHAR
    وإلا إذا (fd == -2)
        state->fd = _wopen(مسیر, oflag, _S_IREAD | _S_IWRITE);
#پایان_اگر
    وإلا {
#اگر_تعریف_شده O_NONBLOCK
        إذا (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#پایان_اگر
#اگر_تعریف_شده O_CLOEXEC
        إذا (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#پایان_اگر
        state->fd = fd;
    }
    إذا (state->fd == -1) {
        حرر(state->مسیر);
        حرر(state);
        أرجع عدم;
    }
    إذا (state->نما == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->نما = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    إذا (state->نما == GZ_READ) {
        state->شروع = LSEEK(state->fd, 0, SEEK_CUR);
        إذا (state->شروع == -1) state->شروع = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    أرجع (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ثابت حرف *مسیر, ثابت حرف *نما) {
    أرجع gz_open(مسیر, -1, نما);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ثابت حرف *مسیر, ثابت حرف *نما) {
    أرجع gz_open(مسیر, -1, نما);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(صحيح fd, ثابت حرف *نما) {
    حرف *مسیر;         /* identifier for error messages */
    gzFile gz;

    إذا (fd == -1 || (مسیر = (حرف *)حافظه_بده(7 + 3 * حجم(صحيح))) == عدم)
        أرجع عدم;
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(مسیر, 7 + 3 * حجم(صحيح), "<fd:%d>", fd);
#وإلا
    sprintf(مسیر, "<fd:%d>", fd);   /* for debugging */
#پایان_اگر
    gz = gz_open(مسیر, fd, نما);
    حرر(مسیر);
    أرجع gz;
}

/* -- see zlib.h -- */
#اگر_تعریف_شده WIDECHAR
gzFile ZEXPORT gzopen_w(ثابت wchar_t *مسیر, ثابت حرف *نما) {
    أرجع gz_open(مسیر, -2, نما);
}
#پایان_اگر

/* -- see zlib.h -- */
صحيح ZEXPORT gzbuffer(gzFile file, بیعلامت اندازه) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->نما != GZ_READ && state->نما != GZ_WRITE)
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
    إذا (state->نما != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        أرجع -1;

    /* back up and start over */
    إذا (LSEEK(state->fd, state->شروع, SEEK_SET) == -1)
        أرجع -1;
    gz_reset(state);
    أرجع 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t جابه_جایی, صحيح whence) {
    بیعلامت n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->نما != GZ_READ && state->نما != GZ_WRITE)
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
    إذا (جابه_جایی > GZ_OFF_MAX - pos)
        أرجع -1;                          /* would overflow z_off64_t */
    إذا (جابه_جایی < -pos)
        أرجع -1;                          /* before start of file */
    pos += جابه_جایی;

    /* if within raw area while reading, just go there */
    إذا (state->نما == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        إذا (ret == -1)
            أرجع -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, عدم);
        state->strm.avail_درون = 0;
        state->x.pos = pos;
        أرجع pos;
    }

    /* rewind if needed for back seek when reading */
    إذا (pos < state->x.pos) {
        إذا (state->نما != GZ_READ)         /* writing -- can't go backwards */
            أرجع -1;
        إذا (gzrewind(file) == -1)           /* rewind, then skip to pos */
            أرجع -1;
    }
    جابه_جایی = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    إذا (state->نما == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > جابه_جایی ?
            (بیعلامت)جابه_جایی : state->x.have;
        state->x.have -= n;
        state->x.بعدی += n;
        state->x.pos += n;
        جابه_جایی -= n;
    }

    /* request skip (if not zero) */
    state->skip = جابه_جایی;
    أرجع pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t جابه_جایی, صحيح whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)جابه_جایی, whence);
    أرجع ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->نما != GZ_READ && state->نما != GZ_WRITE)
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
    z_off64_t جابه_جایی;
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع -1;
    state = (gz_statep)file;
    إذا (state->نما != GZ_READ && state->نما != GZ_WRITE)
        أرجع -1;

    /* compute and return effective offset in file */
    جابه_جایی = LSEEK(state->fd, 0, SEEK_CUR);
    إذا (جابه_جایی == -1)
        أرجع -1;
    إذا (state->نما == GZ_READ)             /* reading */
        جابه_جایی -= state->strm.avail_درون;     /* don't count buffered input */
    أرجع جابه_جایی;
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
    إذا (state->نما != GZ_READ && state->نما != GZ_WRITE)
        أرجع 0;

    /* return end-of-file state */
    أرجع state->نما == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ثابت حرف * ZEXPORT gzerror(gzFile file, صحيح *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع عدم;
    state = (gz_statep)file;
    إذا (state->نما != GZ_READ && state->نما != GZ_WRITE)
        أرجع عدم;

    /* return error information */
    إذا (errnum != عدم)
        *errnum = state->err;
    أرجع state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->پیام == عدم ? "" : state->پیام);
}

/* -- see zlib.h -- */
فراغ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    إذا (file == عدم)
        أرجع;
    state = (gz_statep)file;
    إذا (state->نما != GZ_READ && state->نما != GZ_WRITE)
        أرجع;

    /* clear error and end-of-file */
    إذا (state->نما == GZ_READ) {
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
فراغ ZLIB_داخلی gz_error(gz_statep state, صحيح err, ثابت حرف *پیام) {
    /* free previously allocated message and clear */
    إذا (state->پیام != عدم) {
        إذا (state->err != Z_MEM_ERROR)
            حرر(state->پیام);
        state->پیام = عدم;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    إذا (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    إذا (پیام == عدم)
        أرجع;

    /* for an out of memory error, return literal string when requested */
    إذا (err == Z_MEM_ERROR)
        أرجع;

    /* construct error message with path */
    إذا ((state->پیام = (حرف *)حافظه_بده(طول_رشته(state->مسیر) + طول_رشته(پیام) + 3)) ==
            عدم) {
        state->err = Z_MEM_ERROR;
        أرجع;
    }
#إذا !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (فراغ)snprintf(state->پیام, طول_رشته(state->مسیر) + طول_رشته(پیام) + 3,
                   "%s%s%s", state->مسیر, ": ", پیام);
#وإلا
    strcpy(state->پیام, state->مسیر);
    strcat(state->پیام, ": ");
    strcat(state->پیام, پیام);
#پایان_اگر
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
بیعلامت ZLIB_داخلی gz_intmax(فراغ) {
#اگر_تعریف_شده بیشترین_صحیح
    أرجع بیشترین_صحیح;
#وإلا
    بیعلامت p = 1, q;

    افعل {
        q = p;
        p <<= 1;
        p++;
    } طالما (p > q);
    أرجع q >> 1;
#پایان_اگر
}
