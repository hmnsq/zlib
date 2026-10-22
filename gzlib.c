/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#अन्तर्भाव "gzguts.h"

#यदि defined(__DJGPP__)
#  define LSEEK llseek
#नोचेद्यदि defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#नोचेद्यदि defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#अन्यथा
#  define LSEEK lseek
#यदि_अन्तः

#यदि defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
वर्ण ZLIB_आन्तरिकम् *gz_strwinerror(DWORD error) {
    स्थिर वर्ण buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_चरमः_Error();
    DWORD chars = Format_सन्देशः(FORMAT_सन्देशः_FROM_SYSTEM
        | FORMAT_सन्देशः_ALLOCATE_बफर,
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
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    समुच्चयः_चरमः_Error(lasterr);
    प्रत्यागम buf;
}

#यदि_अन्तः /* UNDER_CE */

/* Reset gzip file state */
local शून्य gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    यदि (state->बहुलकम् == GZ_READ) {   /* for reading ... */
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
    state->strm.avail_अन्तर् = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ध्रुव शून्य *मार्गः, पूर्णाङ्क fd, ध्रुव वर्ण *बहुलकम्) {
    gz_statep state;
    z_आकार_t len;
    पूर्णाङ्क oflag = 0;
#यदि_परिभाषितम् O_EXCL
    पूर्णाङ्क exclusive = 0;
#यदि_अन्तः

    /* check input */
    यदि (मार्गः == रिक्त || बहुलकम् == रिक्त)
        प्रत्यागम रिक्त;

    /* allocate gzFile structure to return */
    state = (gz_statep)स्मृतिं_देहि(परिमाण(gz_state));
    यदि (state == रिक्त)
        प्रत्यागम रिक्त;
    state->आकार = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->सन्देशः = रिक्त;          /* no error message yet */

    /* interpret mode */
    state->बहुलकम् = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->प्रत्यक्षम् = 0;
    यावत् (*बहुलकम्) {
        यदि (*बहुलकम् >= '0' && *बहुलकम् <= '9')
            state->level = *बहुलकम् - '0';
        अन्यथा
            विकल्प (*बहुलकम्) {
            पक्ष 'r':
                state->बहुलकम् = GZ_READ;
                भङ्ग;
#यदि_अपरिभाषितम् NO_GZCOMPRESS
            पक्ष 'w':
                state->बहुलकम् = GZ_WRITE;
                भङ्ग;
            पक्ष 'a':
                state->बहुलकम् = GZ_APPEND;
                भङ्ग;
#यदि_अन्तः
            पक्ष '+':       /* can't read and write at the same time */
                मुक्त(state);
                प्रत्यागम रिक्त;
            पक्ष 'b':       /* ignore -- will request binary anyway */
                भङ्ग;
#यदि_परिभाषितम् O_CLOEXEC
            पक्ष 'e':
                oflag |= O_CLOEXEC;
                भङ्ग;
#यदि_अन्तः
#यदि_परिभाषितम् O_EXCL
            पक्ष 'x':
                exclusive = 1;
                भङ्ग;
#यदि_अन्तः
            पक्ष 'f':
                state->strategy = Z_FILTERED;
                भङ्ग;
            पक्ष 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                भङ्ग;
            पक्ष 'R':
                state->strategy = Z_RLE;
                भङ्ग;
            पक्ष 'F':
                state->strategy = Z_FIXED;
                भङ्ग;
            पक्ष 'G':
                state->प्रत्यक्षम् = -1;
                भङ्ग;
#यदि_परिभाषितम् O_NONBLOCK
            पक्ष 'N':
                oflag |= O_NONBLOCK;
                भङ्ग;
#यदि_अन्तः
            पक्ष 'T':
                state->प्रत्यक्षम् = 1;
                भङ्ग;
            स्वभाव:        /* could consider as an error, but just ignore */
                ;
            }
        बहुलकम्++;
    }

    /* must provide an "r", "w", or "a" */
    यदि (state->बहुलकम् == GZ_NONE) {
        मुक्त(state);
        प्रत्यागम रिक्त;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    यदि (state->बहुलकम् == GZ_READ) {
        यदि (state->प्रत्यक्षम् == 1) {
            /* can't force a transparent read */
            मुक्त(state);
            प्रत्यागम रिक्त;
        }
        यदि (state->प्रत्यक्षम् == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->प्रत्यक्षम् = 1;
    }
    अन्यथा यदि (state->प्रत्यक्षम् == -1) {
        /* "G" has no meaning when writing -- disallow it */
        मुक्त(state);
        प्रत्यागम रिक्त;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#यदि_परिभाषितम् WIDECHAR
    यदि (fd == -2)
        len = wcstombs(रिक्त, मार्गः, 0);
    अन्यथा
#यदि_अन्तः
        len = वाक्यदैर्घ्यम्((ध्रुव वर्ण *)मार्गः);
    state->मार्गः = (वर्ण *)स्मृतिं_देहि(len + 1);
    यदि (state->मार्गः == रिक्त) {
        मुक्त(state);
        प्रत्यागम रिक्त;
    }
#यदि_परिभाषितम् WIDECHAR
    यदि (fd == -2) {
        यदि (len)
            wcstombs(state->मार्गः, मार्गः, len + 1);
        अन्यथा
            *(state->मार्गः) = 0;
    }
    अन्यथा
#यदि_अन्तः
    {
#यदि !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (शून्य)snprintf(state->मार्गः, len + 1, "%s", (ध्रुव वर्ण *)मार्गः);
#अन्यथा
        वाक्यं_प्रतिलिख(state->मार्गः, मार्गः);
#यदि_अन्तः
    }

    /* compute the flags for open() */
    oflag |=
#यदि_परिभाषितम् O_LARGEFILE
        O_LARGEFILE |
#यदि_अन्तः
#यदि_परिभाषितम् O_द्विमान
        O_द्विमान |
#यदि_अन्तः
        (state->बहुलकम् == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#यदि_परिभाषितम् O_EXCL
          (exclusive ? O_EXCL : 0) |
#यदि_अन्तः
          (state->बहुलकम् == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    यदि (fd == -1)
        state->fd = उद्घाटय((ध्रुव वर्ण *)मार्गः, oflag, 0666);
#यदि_परिभाषितम् WIDECHAR
    अन्यथा यदि (fd == -2)
        state->fd = _wopen(मार्गः, oflag, _S_IREAD | _S_IWRITE);
#यदि_अन्तः
    अन्यथा {
#यदि_परिभाषितम् O_NONBLOCK
        यदि (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#यदि_अन्तः
#यदि_परिभाषितम् O_CLOEXEC
        यदि (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#यदि_अन्तः
        state->fd = fd;
    }
    यदि (state->fd == -1) {
        मुक्त(state->मार्गः);
        मुक्त(state);
        प्रत्यागम रिक्त;
    }
    यदि (state->बहुलकम् == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->बहुलकम् = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    यदि (state->बहुलकम् == GZ_READ) {
        state->start = LSEEK(state->fd, 0, SEEK_CUR);
        यदि (state->start == -1) state->start = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    प्रत्यागम (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ध्रुव वर्ण *मार्गः, ध्रुव वर्ण *बहुलकम्) {
    प्रत्यागम gz_open(मार्गः, -1, बहुलकम्);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ध्रुव वर्ण *मार्गः, ध्रुव वर्ण *बहुलकम्) {
    प्रत्यागम gz_open(मार्गः, -1, बहुलकम्);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(पूर्णाङ्क fd, ध्रुव वर्ण *बहुलकम्) {
    वर्ण *मार्गः;         /* identifier for error messages */
    gzFile gz;

    यदि (fd == -1 || (मार्गः = (वर्ण *)स्मृतिं_देहि(7 + 3 * परिमाण(पूर्णाङ्क))) == रिक्त)
        प्रत्यागम रिक्त;
#यदि !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (शून्य)snprintf(मार्गः, 7 + 3 * परिमाण(पूर्णाङ्क), "<fd:%d>", fd);
#अन्यथा
    sprintf(मार्गः, "<fd:%d>", fd);   /* for debugging */
#यदि_अन्तः
    gz = gz_open(मार्गः, fd, बहुलकम्);
    मुक्त(मार्गः);
    प्रत्यागम gz;
}

/* -- see zlib.h -- */
#यदि_परिभाषितम् WIDECHAR
gzFile ZEXPORT gzopen_w(ध्रुव wchar_t *मार्गः, ध्रुव वर्ण *बहुलकम्) {
    प्रत्यागम gz_open(मार्गः, -2, बहुलकम्);
}
#यदि_अन्तः

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzbuffer(gzFile file, अचिह्नित आकार) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    यदि (state->बहुलकम् != GZ_READ && state->बहुलकम् != GZ_WRITE)
        प्रत्यागम -1;

    /* make sure we haven't already allocated memory */
    यदि (state->आकार != 0)
        प्रत्यागम -1;

    /* check and set requested size */
    यदि ((आकार << 1) < आकार)
        प्रत्यागम -1;              /* need to be able to double it */
    यदि (आकार < 8)
        आकार = 8;               /* needed to behave well with flushing */
    state->want = आकार;
    प्रत्यागम 0;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    यदि (state->बहुलकम् != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        प्रत्यागम -1;

    /* back up and start over */
    यदि (LSEEK(state->fd, state->start, SEEK_SET) == -1)
        प्रत्यागम -1;
    gz_reset(state);
    प्रत्यागम 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t विस्थापनम्, पूर्णाङ्क whence) {
    अचिह्नित n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    यदि (state->बहुलकम् != GZ_READ && state->बहुलकम् != GZ_WRITE)
        प्रत्यागम -1;

    /* check that there's no error */
    यदि (state->err != Z_OK && state->err != Z_BUF_ERROR)
        प्रत्यागम -1;

    /* can only seek from start or relative to current position */
    यदि (whence != SEEK_SET && whence != SEEK_CUR)
        प्रत्यागम -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    यदि (विस्थापनम् > GZ_OFF_MAX - pos)
        प्रत्यागम -1;                          /* would overflow z_off64_t */
    यदि (विस्थापनम् < -pos)
        प्रत्यागम -1;                          /* before start of file */
    pos += विस्थापनम्;

    /* if within raw area while reading, just go there */
    यदि (state->बहुलकम् == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        यदि (ret == -1)
            प्रत्यागम -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, रिक्त);
        state->strm.avail_अन्तर् = 0;
        state->x.pos = pos;
        प्रत्यागम pos;
    }

    /* rewind if needed for back seek when reading */
    यदि (pos < state->x.pos) {
        यदि (state->बहुलकम् != GZ_READ)         /* writing -- can't go backwards */
            प्रत्यागम -1;
        यदि (gzrewind(file) == -1)           /* rewind, then skip to pos */
            प्रत्यागम -1;
    }
    विस्थापनम् = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    यदि (state->बहुलकम् == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > विस्थापनम् ?
            (अचिह्नित)विस्थापनम् : state->x.have;
        state->x.have -= n;
        state->x.अग्र += n;
        state->x.pos += n;
        विस्थापनम् -= n;
    }

    /* request skip (if not zero) */
    state->skip = विस्थापनम्;
    प्रत्यागम pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t विस्थापनम्, पूर्णाङ्क whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)विस्थापनम्, whence);
    प्रत्यागम ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    यदि (state->बहुलकम् != GZ_READ && state->बहुलकम् != GZ_WRITE)
        प्रत्यागम -1;

    /* return position */
    प्रत्यागम state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    प्रत्यागम ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t विस्थापनम्;
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        प्रत्यागम -1;
    state = (gz_statep)file;
    यदि (state->बहुलकम् != GZ_READ && state->बहुलकम् != GZ_WRITE)
        प्रत्यागम -1;

    /* compute and return effective offset in file */
    विस्थापनम् = LSEEK(state->fd, 0, SEEK_CUR);
    यदि (विस्थापनम् == -1)
        प्रत्यागम -1;
    यदि (state->बहुलकम् == GZ_READ)             /* reading */
        विस्थापनम् -= state->strm.avail_अन्तर्;     /* don't count buffered input */
    प्रत्यागम विस्थापनम्;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    प्रत्यागम ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
पूर्णाङ्क ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        प्रत्यागम 0;
    state = (gz_statep)file;
    यदि (state->बहुलकम् != GZ_READ && state->बहुलकम् != GZ_WRITE)
        प्रत्यागम 0;

    /* return end-of-file state */
    प्रत्यागम state->बहुलकम् == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ध्रुव वर्ण * ZEXPORT gzerror(gzFile file, पूर्णाङ्क *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        प्रत्यागम रिक्त;
    state = (gz_statep)file;
    यदि (state->बहुलकम् != GZ_READ && state->बहुलकम् != GZ_WRITE)
        प्रत्यागम रिक्त;

    /* return error information */
    यदि (errnum != रिक्त)
        *errnum = state->err;
    प्रत्यागम state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->सन्देशः == रिक्त ? "" : state->सन्देशः);
}

/* -- see zlib.h -- */
शून्य ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    यदि (file == रिक्त)
        प्रत्यागम;
    state = (gz_statep)file;
    यदि (state->बहुलकम् != GZ_READ && state->बहुलकम् != GZ_WRITE)
        प्रत्यागम;

    /* clear error and end-of-file */
    यदि (state->बहुलकम् == GZ_READ) {
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
शून्य ZLIB_आन्तरिकम् gz_error(gz_statep state, पूर्णाङ्क err, ध्रुव वर्ण *सन्देशः) {
    /* free previously allocated message and clear */
    यदि (state->सन्देशः != रिक्त) {
        यदि (state->err != Z_MEM_ERROR)
            मुक्त(state->सन्देशः);
        state->सन्देशः = रिक्त;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    यदि (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    यदि (सन्देशः == रिक्त)
        प्रत्यागम;

    /* for an out of memory error, return literal string when requested */
    यदि (err == Z_MEM_ERROR)
        प्रत्यागम;

    /* construct error message with path */
    यदि ((state->सन्देशः = (वर्ण *)स्मृतिं_देहि(वाक्यदैर्घ्यम्(state->मार्गः) + वाक्यदैर्घ्यम्(सन्देशः) + 3)) ==
            रिक्त) {
        state->err = Z_MEM_ERROR;
        प्रत्यागम;
    }
#यदि !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (शून्य)snprintf(state->सन्देशः, वाक्यदैर्घ्यम्(state->मार्गः) + वाक्यदैर्घ्यम्(सन्देशः) + 3,
                   "%s%s%s", state->मार्गः, ": ", सन्देशः);
#अन्यथा
    वाक्यं_प्रतिलिख(state->सन्देशः, state->मार्गः);
    वाक्यं_योजय(state->सन्देशः, ": ");
    वाक्यं_योजय(state->सन्देशः, सन्देशः);
#यदि_अन्तः
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
अचिह्नित ZLIB_आन्तरिकम् gz_intmax(शून्य) {
#यदि_परिभाषितम् अधिकतम_पूर्णाङ्कः
    प्रत्यागम अधिकतम_पूर्णाङ्कः;
#अन्यथा
    अचिह्नित p = 1, q;

    कुरु {
        q = p;
        p <<= 1;
        p++;
    } यावत् (p > q);
    प्रत्यागम q >> 1;
#यदि_अन्तः
}
