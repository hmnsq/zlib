/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#అన్తర్భావ "gzguts.h"

#ఒకవేళ defined(__DJGPP__)
#  define LSEEK llseek
#లేదావేళ defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#లేదావేళ defined(LARGEFILE64_ఆధారం) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#లేకపోతే
#  define LSEEK lseek
#అయితే_ముగింపు

#ఒకవేళ defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
అక్షరం ZLIB_అంతర్గత *gz_strwinerror(DWORD error) {
    స్థిర అక్షరం buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_చివరి_Error();
    DWORD chars = Format_సందేశం(FORMAT_సందేశం_FROM_SYSTEM
        | FORMAT_సందేశం_ALLOCATE_బఫరు,
        ఖాళీ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ఖాళీ);
    ఒకవేళ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ఒకవేళ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ఒకవేళ (chars > పరిమాణం (buf) - 1) {
            chars = పరిమాణం (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    లేకపోతే {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    సమితి_చివరి_Error(lasterr);
    తిరిగి buf;
}

#అయితే_ముగింపు /* UNDER_CE */

/* Reset gzip file state */
local శూన్య gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    ఒకవేళ (state->బహుళకం == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    లేకపోతే                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ఖాళీ);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_లోపల = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(స్థిరరాశి శూన్య *మార్గం, పూర్ణాంకం fd, స్థిరరాశి అక్షరం *బహుళకం) {
    gz_statep state;
    z_కొలత_t len;
    పూర్ణాంకం oflag = 0;
#నిర్వచితమైతే O_EXCL
    పూర్ణాంకం exclusive = 0;
#అయితే_ముగింపు

    /* check input */
    ఒకవేళ (మార్గం == ఖాళీ || బహుళకం == ఖాళీ)
        తిరిగి ఖాళీ;

    /* allocate gzFile structure to return */
    state = (gz_statep)జ్ఞాపకం_ఇవ్వు(పరిమాణం(gz_state));
    ఒకవేళ (state == ఖాళీ)
        తిరిగి ఖాళీ;
    state->కొలత = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->సందేశం = ఖాళీ;          /* no error message yet */

    /* interpret mode */
    state->బహుళకం = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->ప్రత్యక్ష = 0;
    అయితే (*బహుళకం) {
        ఒకవేళ (*బహుళకం >= '0' && *బహుళకం <= '9')
            state->level = *బహుళకం - '0';
        లేకపోతే
            ఎంపిక (*బహుళకం) {
            స్థితి 'r':
                state->బహుళకం = GZ_READ;
                ఆపు;
#నిర్వచితం_కాకపోతే NO_GZCOMPRESS
            స్థితి 'w':
                state->బహుళకం = GZ_WRITE;
                ఆపు;
            స్థితి 'a':
                state->బహుళకం = GZ_APPEND;
                ఆపు;
#అయితే_ముగింపు
            స్థితి '+':       /* can't read and write at the same time */
                విడుదల(state);
                తిరిగి ఖాళీ;
            స్థితి 'b':       /* ignore -- will request binary anyway */
                ఆపు;
#నిర్వచితమైతే O_CLOEXEC
            స్థితి 'e':
                oflag |= O_CLOEXEC;
                ఆపు;
#అయితే_ముగింపు
#నిర్వచితమైతే O_EXCL
            స్థితి 'x':
                exclusive = 1;
                ఆపు;
#అయితే_ముగింపు
            స్థితి 'f':
                state->strategy = Z_FILTERED;
                ఆపు;
            స్థితి 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                ఆపు;
            స్థితి 'R':
                state->strategy = Z_RLE;
                ఆపు;
            స్థితి 'F':
                state->strategy = Z_FIXED;
                ఆపు;
            స్థితి 'G':
                state->ప్రత్యక్ష = -1;
                ఆపు;
#నిర్వచితమైతే O_NONBLOCK
            స్థితి 'N':
                oflag |= O_NONBLOCK;
                ఆపు;
#అయితే_ముగింపు
            స్థితి 'T':
                state->ప్రత్యక్ష = 1;
                ఆపు;
            అప్రమేయ:        /* could consider as an error, but just ignore */
                ;
            }
        బహుళకం++;
    }

    /* must provide an "r", "w", or "a" */
    ఒకవేళ (state->బహుళకం == GZ_NONE) {
        విడుదల(state);
        తిరిగి ఖాళీ;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    ఒకవేళ (state->బహుళకం == GZ_READ) {
        ఒకవేళ (state->ప్రత్యక్ష == 1) {
            /* can't force a transparent read */
            విడుదల(state);
            తిరిగి ఖాళీ;
        }
        ఒకవేళ (state->ప్రత్యక్ష == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->ప్రత్యక్ష = 1;
    }
    లేకపోతే ఒకవేళ (state->ప్రత్యక్ష == -1) {
        /* "G" has no meaning when writing -- disallow it */
        విడుదల(state);
        తిరిగి ఖాళీ;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#నిర్వచితమైతే WIDECHAR
    ఒకవేళ (fd == -2)
        len = wcstombs(ఖాళీ, మార్గం, 0);
    లేకపోతే
#అయితే_ముగింపు
        len = అక్షరశ్రేణి_పొడవు((స్థిరరాశి అక్షరం *)మార్గం);
    state->మార్గం = (అక్షరం *)జ్ఞాపకం_ఇవ్వు(len + 1);
    ఒకవేళ (state->మార్గం == ఖాళీ) {
        విడుదల(state);
        తిరిగి ఖాళీ;
    }
#నిర్వచితమైతే WIDECHAR
    ఒకవేళ (fd == -2) {
        ఒకవేళ (len)
            wcstombs(state->మార్గం, మార్గం, len + 1);
        లేకపోతే
            *(state->మార్గం) = 0;
    }
    లేకపోతే
#అయితే_ముగింపు
    {
#ఒకవేళ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (శూన్య)snprintf(state->మార్గం, len + 1, "%s", (స్థిరరాశి అక్షరం *)మార్గం);
#లేకపోతే
        strcpy(state->మార్గం, మార్గం);
#అయితే_ముగింపు
    }

    /* compute the flags for open() */
    oflag |=
#నిర్వచితమైతే O_LARGEFILE
        O_LARGEFILE |
#అయితే_ముగింపు
#నిర్వచితమైతే O_ద్విమాన
        O_ద్విమాన |
#అయితే_ముగింపు
        (state->బహుళకం == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#నిర్వచితమైతే O_EXCL
          (exclusive ? O_EXCL : 0) |
#అయితే_ముగింపు
          (state->బహుళకం == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    ఒకవేళ (fd == -1)
        state->fd = open((స్థిరరాశి అక్షరం *)మార్గం, oflag, 0666);
#నిర్వచితమైతే WIDECHAR
    లేకపోతే ఒకవేళ (fd == -2)
        state->fd = _wopen(మార్గం, oflag, _S_IREAD | _S_IWRITE);
#అయితే_ముగింపు
    లేకపోతే {
#నిర్వచితమైతే O_NONBLOCK
        ఒకవేళ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#అయితే_ముగింపు
#నిర్వచితమైతే O_CLOEXEC
        ఒకవేళ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#అయితే_ముగింపు
        state->fd = fd;
    }
    ఒకవేళ (state->fd == -1) {
        విడుదల(state->మార్గం);
        విడుదల(state);
        తిరిగి ఖాళీ;
    }
    ఒకవేళ (state->బహుళకం == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->బహుళకం = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    ఒకవేళ (state->బహుళకం == GZ_READ) {
        state->ప్రారంభం = LSEEK(state->fd, 0, SEEK_CUR);
        ఒకవేళ (state->ప్రారంభం == -1) state->ప్రారంభం = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    తిరిగి (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(స్థిరరాశి అక్షరం *మార్గం, స్థిరరాశి అక్షరం *బహుళకం) {
    తిరిగి gz_open(మార్గం, -1, బహుళకం);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(స్థిరరాశి అక్షరం *మార్గం, స్థిరరాశి అక్షరం *బహుళకం) {
    తిరిగి gz_open(మార్గం, -1, బహుళకం);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(పూర్ణాంకం fd, స్థిరరాశి అక్షరం *బహుళకం) {
    అక్షరం *మార్గం;         /* identifier for error messages */
    gzFile gz;

    ఒకవేళ (fd == -1 || (మార్గం = (అక్షరం *)జ్ఞాపకం_ఇవ్వు(7 + 3 * పరిమాణం(పూర్ణాంకం))) == ఖాళీ)
        తిరిగి ఖాళీ;
#ఒకవేళ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (శూన్య)snprintf(మార్గం, 7 + 3 * పరిమాణం(పూర్ణాంకం), "<fd:%d>", fd);
#లేకపోతే
    sprintf(మార్గం, "<fd:%d>", fd);   /* for debugging */
#అయితే_ముగింపు
    gz = gz_open(మార్గం, fd, బహుళకం);
    విడుదల(మార్గం);
    తిరిగి gz;
}

/* -- see zlib.h -- */
#నిర్వచితమైతే WIDECHAR
gzFile ZEXPORT gzopen_w(స్థిరరాశి wchar_t *మార్గం, స్థిరరాశి అక్షరం *బహుళకం) {
    తిరిగి gz_open(మార్గం, -2, బహుళకం);
}
#అయితే_ముగింపు

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzbuffer(gzFile file, గుర్తులేని కొలత) {
    gz_statep state;

    /* get internal structure and check integrity */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ && state->బహుళకం != GZ_WRITE)
        తిరిగి -1;

    /* make sure we haven't already allocated memory */
    ఒకవేళ (state->కొలత != 0)
        తిరిగి -1;

    /* check and set requested size */
    ఒకవేళ ((కొలత << 1) < కొలత)
        తిరిగి -1;              /* need to be able to double it */
    ఒకవేళ (కొలత < 8)
        కొలత = 8;               /* needed to behave well with flushing */
    state->want = కొలత;
    తిరిగి 0;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    ఒకవేళ (state->బహుళకం != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        తిరిగి -1;

    /* back up and start over */
    ఒకవేళ (LSEEK(state->fd, state->ప్రారంభం, SEEK_SET) == -1)
        తిరిగి -1;
    gz_reset(state);
    తిరిగి 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t స్థానభ్రంశం, పూర్ణాంకం whence) {
    గుర్తులేని n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ && state->బహుళకం != GZ_WRITE)
        తిరిగి -1;

    /* check that there's no error */
    ఒకవేళ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        తిరిగి -1;

    /* can only seek from start or relative to current position */
    ఒకవేళ (whence != SEEK_SET && whence != SEEK_CUR)
        తిరిగి -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    ఒకవేళ (స్థానభ్రంశం > GZ_OFF_MAX - pos)
        తిరిగి -1;                          /* would overflow z_off64_t */
    ఒకవేళ (స్థానభ్రంశం < -pos)
        తిరిగి -1;                          /* before start of file */
    pos += స్థానభ్రంశం;

    /* if within raw area while reading, just go there */
    ఒకవేళ (state->బహుళకం == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        ఒకవేళ (ret == -1)
            తిరిగి -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ఖాళీ);
        state->strm.avail_లోపల = 0;
        state->x.pos = pos;
        తిరిగి pos;
    }

    /* rewind if needed for back seek when reading */
    ఒకవేళ (pos < state->x.pos) {
        ఒకవేళ (state->బహుళకం != GZ_READ)         /* writing -- can't go backwards */
            తిరిగి -1;
        ఒకవేళ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            తిరిగి -1;
    }
    స్థానభ్రంశం = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    ఒకవేళ (state->బహుళకం == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > స్థానభ్రంశం ?
            (గుర్తులేని)స్థానభ్రంశం : state->x.have;
        state->x.have -= n;
        state->x.తదుపరి += n;
        state->x.pos += n;
        స్థానభ్రంశం -= n;
    }

    /* request skip (if not zero) */
    state->skip = స్థానభ్రంశం;
    తిరిగి pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t స్థానభ్రంశం, పూర్ణాంకం whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)స్థానభ్రంశం, whence);
    తిరిగి ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ && state->బహుళకం != GZ_WRITE)
        తిరిగి -1;

    /* return position */
    తిరిగి state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    తిరిగి ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t స్థానభ్రంశం;
    gz_statep state;

    /* get internal structure and check integrity */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి -1;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ && state->బహుళకం != GZ_WRITE)
        తిరిగి -1;

    /* compute and return effective offset in file */
    స్థానభ్రంశం = LSEEK(state->fd, 0, SEEK_CUR);
    ఒకవేళ (స్థానభ్రంశం == -1)
        తిరిగి -1;
    ఒకవేళ (state->బహుళకం == GZ_READ)             /* reading */
        స్థానభ్రంశం -= state->strm.avail_లోపల;     /* don't count buffered input */
    తిరిగి స్థానభ్రంశం;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    తిరిగి ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
పూర్ణాంకం ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి 0;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ && state->బహుళకం != GZ_WRITE)
        తిరిగి 0;

    /* return end-of-file state */
    తిరిగి state->బహుళకం == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
స్థిరరాశి అక్షరం * ZEXPORT gzerror(gzFile file, పూర్ణాంకం *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి ఖాళీ;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ && state->బహుళకం != GZ_WRITE)
        తిరిగి ఖాళీ;

    /* return error information */
    ఒకవేళ (errnum != ఖాళీ)
        *errnum = state->err;
    తిరిగి state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->సందేశం == ఖాళీ ? "" : state->సందేశం);
}

/* -- see zlib.h -- */
శూన్య ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ఒకవేళ (file == ఖాళీ)
        తిరిగి;
    state = (gz_statep)file;
    ఒకవేళ (state->బహుళకం != GZ_READ && state->బహుళకం != GZ_WRITE)
        తిరిగి;

    /* clear error and end-of-file */
    ఒకవేళ (state->బహుళకం == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ఖాళీ);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
శూన్య ZLIB_అంతర్గత gz_error(gz_statep state, పూర్ణాంకం err, స్థిరరాశి అక్షరం *సందేశం) {
    /* free previously allocated message and clear */
    ఒకవేళ (state->సందేశం != ఖాళీ) {
        ఒకవేళ (state->err != Z_MEM_ERROR)
            విడుదల(state->సందేశం);
        state->సందేశం = ఖాళీ;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    ఒకవేళ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    ఒకవేళ (సందేశం == ఖాళీ)
        తిరిగి;

    /* for an out of memory error, return literal string when requested */
    ఒకవేళ (err == Z_MEM_ERROR)
        తిరిగి;

    /* construct error message with path */
    ఒకవేళ ((state->సందేశం = (అక్షరం *)జ్ఞాపకం_ఇవ్వు(అక్షరశ్రేణి_పొడవు(state->మార్గం) + అక్షరశ్రేణి_పొడవు(సందేశం) + 3)) ==
            ఖాళీ) {
        state->err = Z_MEM_ERROR;
        తిరిగి;
    }
#ఒకవేళ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (శూన్య)snprintf(state->సందేశం, అక్షరశ్రేణి_పొడవు(state->మార్గం) + అక్షరశ్రేణి_పొడవు(సందేశం) + 3,
                   "%s%s%s", state->మార్గం, ": ", సందేశం);
#లేకపోతే
    strcpy(state->సందేశం, state->మార్గం);
    strcat(state->సందేశం, ": ");
    strcat(state->సందేశం, సందేశం);
#అయితే_ముగింపు
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
గుర్తులేని ZLIB_అంతర్గత gz_intmax(శూన్య) {
#నిర్వచితమైతే గరిష్ఠ_పూర్ణాంకం
    తిరిగి గరిష్ఠ_పూర్ణాంకం;
#లేకపోతే
    గుర్తులేని p = 1, q;

    చేయి {
        q = p;
        p <<= 1;
        p++;
    } అయితే (p > q);
    తిరిగి q >> 1;
#అయితే_ముగింపు
}
