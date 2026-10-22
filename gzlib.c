/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#شامل "gzguts.h"

#اگر defined(__DJGPP__)
#  define LSEEK llseek
#وگرنہ_اگر defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#وگرنہ_اگر defined(LARGEFILE64_ماخذ) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#ورنہ
#  define LSEEK lseek
#اگر_ختم

#اگر defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
حرف ZLIB_اندرونی *gz_strwinerror(DWORD error) {
    ساکن حرف buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_آخری_Error();
    DWORD chars = Format_پیغام(FORMAT_پیغام_FROM_SYSTEM
        | FORMAT_پیغام_ALLOCATE_بفر,
        تہی,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        تہی);
    اگر (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        اگر (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        اگر (chars > حجم (buf) - 1) {
            chars = حجم (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    ورنہ {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    سیٹ_آخری_Error(lasterr);
    واپس buf;
}

#اگر_ختم /* UNDER_CE */

/* Reset gzip file state */
local خالی gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    اگر (state->منوال == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    ورنہ                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, تہی);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_اندر = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ثابت خالی *راستہ, صحیح fd, ثابت حرف *منوال) {
    gz_statep state;
    z_سائز_t len;
    صحیح oflag = 0;
#اگر_تعریف_شدہ O_EXCL
    صحیح exclusive = 0;
#اگر_ختم

    /* check input */
    اگر (راستہ == تہی || منوال == تہی)
        واپس تہی;

    /* allocate gzFile structure to return */
    state = (gz_statep)حافظہ_دو(حجم(gz_state));
    اگر (state == تہی)
        واپس تہی;
    state->سائز = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->پیغام = تہی;          /* no error message yet */

    /* interpret mode */
    state->منوال = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->براہ_راست = 0;
    جب (*منوال) {
        اگر (*منوال >= '0' && *منوال <= '9')
            state->level = *منوال - '0';
        ورنہ
            انتخاب (*منوال) {
            حالت 'r':
                state->منوال = GZ_READ;
                توڑو;
#اگر_غیر_تعریف_شدہ NO_GZCOMPRESS
            حالت 'w':
                state->منوال = GZ_WRITE;
                توڑو;
            حالت 'a':
                state->منوال = GZ_APPEND;
                توڑو;
#اگر_ختم
            حالت '+':       /* can't read and write at the same time */
                آزاد(state);
                واپس تہی;
            حالت 'b':       /* ignore -- will request binary anyway */
                توڑو;
#اگر_تعریف_شدہ O_CLOEXEC
            حالت 'e':
                oflag |= O_CLOEXEC;
                توڑو;
#اگر_ختم
#اگر_تعریف_شدہ O_EXCL
            حالت 'x':
                exclusive = 1;
                توڑو;
#اگر_ختم
            حالت 'f':
                state->strategy = Z_FILTERED;
                توڑو;
            حالت 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                توڑو;
            حالت 'R':
                state->strategy = Z_RLE;
                توڑو;
            حالت 'F':
                state->strategy = Z_FIXED;
                توڑو;
            حالت 'G':
                state->براہ_راست = -1;
                توڑو;
#اگر_تعریف_شدہ O_NONBLOCK
            حالت 'N':
                oflag |= O_NONBLOCK;
                توڑو;
#اگر_ختم
            حالت 'T':
                state->براہ_راست = 1;
                توڑو;
            طےشدہ:        /* could consider as an error, but just ignore */
                ;
            }
        منوال++;
    }

    /* must provide an "r", "w", or "a" */
    اگر (state->منوال == GZ_NONE) {
        آزاد(state);
        واپس تہی;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    اگر (state->منوال == GZ_READ) {
        اگر (state->براہ_راست == 1) {
            /* can't force a transparent read */
            آزاد(state);
            واپس تہی;
        }
        اگر (state->براہ_راست == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->براہ_راست = 1;
    }
    ورنہ اگر (state->براہ_راست == -1) {
        /* "G" has no meaning when writing -- disallow it */
        آزاد(state);
        واپس تہی;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#اگر_تعریف_شدہ WIDECHAR
    اگر (fd == -2)
        len = wcstombs(تہی, راستہ, 0);
    ورنہ
#اگر_ختم
        len = لڑی_لمبائی((ثابت حرف *)راستہ);
    state->راستہ = (حرف *)حافظہ_دو(len + 1);
    اگر (state->راستہ == تہی) {
        آزاد(state);
        واپس تہی;
    }
#اگر_تعریف_شدہ WIDECHAR
    اگر (fd == -2) {
        اگر (len)
            wcstombs(state->راستہ, راستہ, len + 1);
        ورنہ
            *(state->راستہ) = 0;
    }
    ورنہ
#اگر_ختم
    {
#اگر !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (خالی)snprintf(state->راستہ, len + 1, "%s", (ثابت حرف *)راستہ);
#ورنہ
        strcpy(state->راستہ, راستہ);
#اگر_ختم
    }

    /* compute the flags for open() */
    oflag |=
#اگر_تعریف_شدہ O_LARGEFILE
        O_LARGEFILE |
#اگر_ختم
#اگر_تعریف_شدہ O_ثنائی
        O_ثنائی |
#اگر_ختم
        (state->منوال == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#اگر_تعریف_شدہ O_EXCL
          (exclusive ? O_EXCL : 0) |
#اگر_ختم
          (state->منوال == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    اگر (fd == -1)
        state->fd = open((ثابت حرف *)راستہ, oflag, 0666);
#اگر_تعریف_شدہ WIDECHAR
    ورنہ اگر (fd == -2)
        state->fd = _wopen(راستہ, oflag, _S_IREAD | _S_IWRITE);
#اگر_ختم
    ورنہ {
#اگر_تعریف_شدہ O_NONBLOCK
        اگر (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#اگر_ختم
#اگر_تعریف_شدہ O_CLOEXEC
        اگر (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#اگر_ختم
        state->fd = fd;
    }
    اگر (state->fd == -1) {
        آزاد(state->راستہ);
        آزاد(state);
        واپس تہی;
    }
    اگر (state->منوال == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->منوال = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    اگر (state->منوال == GZ_READ) {
        state->آغاز = LSEEK(state->fd, 0, SEEK_CUR);
        اگر (state->آغاز == -1) state->آغاز = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    واپس (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ثابت حرف *راستہ, ثابت حرف *منوال) {
    واپس gz_open(راستہ, -1, منوال);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ثابت حرف *راستہ, ثابت حرف *منوال) {
    واپس gz_open(راستہ, -1, منوال);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(صحیح fd, ثابت حرف *منوال) {
    حرف *راستہ;         /* identifier for error messages */
    gzFile gz;

    اگر (fd == -1 || (راستہ = (حرف *)حافظہ_دو(7 + 3 * حجم(صحیح))) == تہی)
        واپس تہی;
#اگر !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (خالی)snprintf(راستہ, 7 + 3 * حجم(صحیح), "<fd:%d>", fd);
#ورنہ
    sprintf(راستہ, "<fd:%d>", fd);   /* for debugging */
#اگر_ختم
    gz = gz_open(راستہ, fd, منوال);
    آزاد(راستہ);
    واپس gz;
}

/* -- see zlib.h -- */
#اگر_تعریف_شدہ WIDECHAR
gzFile ZEXPORT gzopen_w(ثابت wchar_t *راستہ, ثابت حرف *منوال) {
    واپس gz_open(راستہ, -2, منوال);
}
#اگر_ختم

/* -- see zlib.h -- */
صحیح ZEXPORT gzbuffer(gzFile file, غیرعلامتی سائز) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        واپس -1;

    /* make sure we haven't already allocated memory */
    اگر (state->سائز != 0)
        واپس -1;

    /* check and set requested size */
    اگر ((سائز << 1) < سائز)
        واپس -1;              /* need to be able to double it */
    اگر (سائز < 8)
        سائز = 8;               /* needed to behave well with flushing */
    state->want = سائز;
    واپس 0;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    اگر (state->منوال != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        واپس -1;

    /* back up and start over */
    اگر (LSEEK(state->fd, state->آغاز, SEEK_SET) == -1)
        واپس -1;
    gz_reset(state);
    واپس 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t انحراف, صحیح whence) {
    غیرعلامتی n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        واپس -1;

    /* check that there's no error */
    اگر (state->err != Z_OK && state->err != Z_BUF_ERROR)
        واپس -1;

    /* can only seek from start or relative to current position */
    اگر (whence != SEEK_SET && whence != SEEK_CUR)
        واپس -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    اگر (انحراف > GZ_OFF_MAX - pos)
        واپس -1;                          /* would overflow z_off64_t */
    اگر (انحراف < -pos)
        واپس -1;                          /* before start of file */
    pos += انحراف;

    /* if within raw area while reading, just go there */
    اگر (state->منوال == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        اگر (ret == -1)
            واپس -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, تہی);
        state->strm.avail_اندر = 0;
        state->x.pos = pos;
        واپس pos;
    }

    /* rewind if needed for back seek when reading */
    اگر (pos < state->x.pos) {
        اگر (state->منوال != GZ_READ)         /* writing -- can't go backwards */
            واپس -1;
        اگر (gzrewind(file) == -1)           /* rewind, then skip to pos */
            واپس -1;
    }
    انحراف = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    اگر (state->منوال == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > انحراف ?
            (غیرعلامتی)انحراف : state->x.have;
        state->x.have -= n;
        state->x.اگلا += n;
        state->x.pos += n;
        انحراف -= n;
    }

    /* request skip (if not zero) */
    state->skip = انحراف;
    واپس pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t انحراف, صحیح whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)انحراف, whence);
    واپس ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        واپس -1;

    /* return position */
    واپس state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    واپس ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t انحراف;
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == تہی)
        واپس -1;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        واپس -1;

    /* compute and return effective offset in file */
    انحراف = LSEEK(state->fd, 0, SEEK_CUR);
    اگر (انحراف == -1)
        واپس -1;
    اگر (state->منوال == GZ_READ)             /* reading */
        انحراف -= state->strm.avail_اندر;     /* don't count buffered input */
    واپس انحراف;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    واپس ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
صحیح ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == تہی)
        واپس 0;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        واپس 0;

    /* return end-of-file state */
    واپس state->منوال == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ثابت حرف * ZEXPORT gzerror(gzFile file, صحیح *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == تہی)
        واپس تہی;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        واپس تہی;

    /* return error information */
    اگر (errnum != تہی)
        *errnum = state->err;
    واپس state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->پیغام == تہی ? "" : state->پیغام);
}

/* -- see zlib.h -- */
خالی ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    اگر (file == تہی)
        واپس;
    state = (gz_statep)file;
    اگر (state->منوال != GZ_READ && state->منوال != GZ_WRITE)
        واپس;

    /* clear error and end-of-file */
    اگر (state->منوال == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, تہی);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
خالی ZLIB_اندرونی gz_error(gz_statep state, صحیح err, ثابت حرف *پیغام) {
    /* free previously allocated message and clear */
    اگر (state->پیغام != تہی) {
        اگر (state->err != Z_MEM_ERROR)
            آزاد(state->پیغام);
        state->پیغام = تہی;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    اگر (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    اگر (پیغام == تہی)
        واپس;

    /* for an out of memory error, return literal string when requested */
    اگر (err == Z_MEM_ERROR)
        واپس;

    /* construct error message with path */
    اگر ((state->پیغام = (حرف *)حافظہ_دو(لڑی_لمبائی(state->راستہ) + لڑی_لمبائی(پیغام) + 3)) ==
            تہی) {
        state->err = Z_MEM_ERROR;
        واپس;
    }
#اگر !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (خالی)snprintf(state->پیغام, لڑی_لمبائی(state->راستہ) + لڑی_لمبائی(پیغام) + 3,
                   "%s%s%s", state->راستہ, ": ", پیغام);
#ورنہ
    strcpy(state->پیغام, state->راستہ);
    strcat(state->پیغام, ": ");
    strcat(state->پیغام, پیغام);
#اگر_ختم
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
غیرعلامتی ZLIB_اندرونی gz_intmax(خالی) {
#اگر_تعریف_شدہ زیادہ_سے_زیادہ_عدد
    واپس زیادہ_سے_زیادہ_عدد;
#ورنہ
    غیرعلامتی p = 1, q;

    کرو {
        q = p;
        p <<= 1;
        p++;
    } جب (p > q);
    واپس q >> 1;
#اگر_ختم
}
