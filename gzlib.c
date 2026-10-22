/* gzlib.c -- zlib functions common to reading and writing gzip files
 * Copyright (C) 2004-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ಅನ್ತರ್ಭಾವ "gzguts.h"

#ಆದರೆ defined(__DJGPP__)
#  define LSEEK llseek
#ಅಥವಾಆದರೆ defined(_WIN32) && !defined(__BORLANDC__) && !defined(UNDER_CE)
#  define LSEEK _lseeki64
#ಅಥವಾಆದರೆ defined(LARGEFILE64_ಮೂಲ) && _LFS64_LARGEFILE-0
#  define LSEEK lseek64
#ಇಲ್ಲದಿದ್ದರೆ
#  define LSEEK lseek
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined UNDER_CE

/* Map the Windows error number in ERROR to a locale-dependent error message
   string and return a pointer to it.  Typically, the values for ERROR come
   from GetLastError.

   The string pointed to shall not be modified by the application, but may be
   overwritten by a subsequent call to gz_strwinerror

   The gz_strwinerror function does not change the current setting of
   GetLastError. */
ಅಕ್ಷರ ZLIB_ಆಂತರಿಕ *gz_strwinerror(DWORD error) {
    ಸ್ಥಿರ ಅಕ್ಷರ buf[1024];

    wchar_t *msgbuf;
    DWORD lasterr = Get_ಕೊನೆಯ_Error();
    DWORD chars = Format_ಸಂದೇಶ(FORMAT_ಸಂದೇಶ_FROM_SYSTEM
        | FORMAT_ಸಂದೇಶ_ALLOCATE_ಬಫರ್,
        ಖಾಲಿ,
        error,
        0, /* Default language */
        (LPVOID)&msgbuf,
        0,
        ಖಾಲಿ);
    ಆದರೆ (chars != 0) {
        /* If there is an \r\n appended, zap it.  */
        ಆದರೆ (chars >= 2
            && msgbuf[chars - 2] == '\r' && msgbuf[chars - 1] == '\n') {
            chars -= 2;
            msgbuf[chars] = 0;
        }

        ಆದರೆ (chars > ಗಾತ್ರ (buf) - 1) {
            chars = ಗಾತ್ರ (buf) - 1;
            msgbuf[chars] = 0;
        }

        wcstombs(buf, msgbuf, chars + 1);       /* assumes buf is big enough */
        LocalFree(msgbuf);
    }
    ಇಲ್ಲದಿದ್ದರೆ {
        sprintf(buf, "unknown win32 error (%ld)", error);
    }

    ಗಣ_ಕೊನೆಯ_Error(lasterr);
    ಹಿಂತಿರುಗಿ buf;
}

#ಆದರೆ_ಅಂತ್ಯ /* UNDER_CE */

/* Reset gzip file state */
local ಶೂನ್ಯ gz_reset(gz_statep state) {
    state->x.have = 0;              /* no output data available */
    state->past = 0;                /* have not read past end yet */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ) {   /* for reading ... */
        state->eof = 0;             /* not at end of file */
        state->how = LOOK;          /* look for gzip header */
        state->junk = -1;           /* mark first member */
    }
    ಇಲ್ಲದಿದ್ದರೆ                            /* for writing ... */
        state->reset = 0;           /* no deflateReset pending */
    state->again = 0;               /* no stalled i/o yet */
    state->skip = 0;                /* no seek request pending */
    gz_error(state, Z_OK, ಖಾಲಿ);    /* clear error */
    state->x.pos = 0;               /* no uncompressed data yet */
    state->strm.avail_ಒಳ = 0;       /* no input data yet */
}

/* Open a gzip file either by name or file descriptor. */
local gzFile gz_open(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    gz_statep state;
    z_ಗಾತ್ರ_t len;
    ಪೂರ್ಣಾಂಕ oflag = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_EXCL
    ಪೂರ್ಣಾಂಕ exclusive = 0;
#ಆದರೆ_ಅಂತ್ಯ

    /* check input */
    ಆದರೆ (ಮಾರ್ಗ == ಖಾಲಿ || ಬಹುಲಕ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    /* allocate gzFile structure to return */
    state = (gz_statep)ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(gz_state));
    ಆದರೆ (state == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    state->ಗಾತ್ರ_2 = 0;            /* no buffers allocated yet */
    state->want = GZBUFSIZE;    /* requested buffer size */
    state->err = Z_OK;          /* no error yet */
    state->ಸಂದೇಶ = ಖಾಲಿ;          /* no error message yet */

    /* interpret mode */
    state->ಬಹುಲಕ = GZ_NONE;
    state->level = Z_DEFAULT_COMPRESSION;
    state->strategy = Z_DEFAULT_STRATEGY;
    state->ನೇರ = 0;
    ಆಗ (*ಬಹುಲಕ) {
        ಆದರೆ (*ಬಹುಲಕ >= '0' && *ಬಹುಲಕ <= '9')
            state->level = *ಬಹುಲಕ - '0';
        ಇಲ್ಲದಿದ್ದರೆ
            ಆಯ್ಕೆ (*ಬಹುಲಕ) {
            ಸ್ಥಿತಿ 'r':
                state->ಬಹುಲಕ = GZ_READ;
                ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_GZCOMPRESS
            ಸ್ಥಿತಿ 'w':
                state->ಬಹುಲಕ = GZ_WRITE;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 'a':
                state->ಬಹುಲಕ = GZ_APPEND;
                ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
            ಸ್ಥಿತಿ '+':       /* can't read and write at the same time */
                ಬಿಡುಗಡೆ(state);
                ಹಿಂತಿರುಗಿ ಖಾಲಿ;
            ಸ್ಥಿತಿ 'b':       /* ignore -- will request binary anyway */
                ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_CLOEXEC
            ಸ್ಥಿತಿ 'e':
                oflag |= O_CLOEXEC;
                ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_EXCL
            ಸ್ಥಿತಿ 'x':
                exclusive = 1;
                ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
            ಸ್ಥಿತಿ 'f':
                state->strategy = Z_FILTERED;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 'h':
                state->strategy = Z_HUFFMAN_ONLY;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 'R':
                state->strategy = Z_RLE;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 'F':
                state->strategy = Z_FIXED;
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 'G':
                state->ನೇರ = -1;
                ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_NONBLOCK
            ಸ್ಥಿತಿ 'N':
                oflag |= O_NONBLOCK;
                ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
            ಸ್ಥಿತಿ 'T':
                state->ನೇರ = 1;
                ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:        /* could consider as an error, but just ignore */
                ;
            }
        ಬಹುಲಕ++;
    }

    /* must provide an "r", "w", or "a" */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_NONE) {
        ಬಿಡುಗಡೆ(state);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    /* direct is 0, 1 if "T", or -1 if "G" (last "G" or "T" wins) */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ) {
        ಆದರೆ (state->ನೇರ == 1) {
            /* can't force a transparent read */
            ಬಿಡುಗಡೆ(state);
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        }
        ಆದರೆ (state->ನೇರ == 0)
            /* default when reading is auto-detect of gzip vs. transparent --
               start with a transparent assumption in case of an empty file */
            state->ನೇರ = 1;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state->ನೇರ == -1) {
        /* "G" has no meaning when writing -- disallow it */
        ಬಿಡುಗಡೆ(state);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    /* if reading, direct == 1 for auto-detect, -1 for gzip only; if writing or
       appending, direct == 0 for gzip, 1 for transparent (copy in to out) */

    /* save the path name for error messages */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WIDECHAR
    ಆದರೆ (fd == -2)
        len = wcstombs(ಖಾಲಿ, ಮಾರ್ಗ, 0);
    ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
        len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ಮಾರ್ಗ);
    state->ಮಾರ್ಗ = (ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ಕೊಡು(len + 1);
    ಆದರೆ (state->ಮಾರ್ಗ == ಖಾಲಿ) {
        ಬಿಡುಗಡೆ(state);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WIDECHAR
    ಆದರೆ (fd == -2) {
        ಆದರೆ (len)
            wcstombs(state->ಮಾರ್ಗ, ಮಾರ್ಗ, len + 1);
        ಇಲ್ಲದಿದ್ದರೆ
            *(state->ಮಾರ್ಗ) = 0;
    }
    ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
#ಆದರೆ !defined(NO_snprintf) && !defined(NO_vsnprintf)
        (ಶೂನ್ಯ)snprintf(state->ಮಾರ್ಗ, len + 1, "%s", (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ಮಾರ್ಗ);
#ಇಲ್ಲದಿದ್ದರೆ
        strcpy(state->ಮಾರ್ಗ, ಮಾರ್ಗ);
#ಆದರೆ_ಅಂತ್ಯ
    }

    /* compute the flags for open() */
    oflag |=
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_LARGEFILE
        O_LARGEFILE |
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_ದ್ವಿಮಾನ
        O_ದ್ವಿಮಾನ |
#ಆದರೆ_ಅಂತ್ಯ
        (state->ಬಹುಲಕ == GZ_READ ?
         O_RDONLY :
         (O_WRONLY | O_CREAT |
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_EXCL
          (exclusive ? O_EXCL : 0) |
#ಆದರೆ_ಅಂತ್ಯ
          (state->ಬಹುಲಕ == GZ_WRITE ?
           O_TRUNC :
           O_APPEND)));

    /* open the file with the appropriate flags (or just use fd) */
    ಆದರೆ (fd == -1)
        state->fd = open((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ಮಾರ್ಗ, oflag, 0666);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WIDECHAR
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fd == -2)
        state->fd = _wopen(ಮಾರ್ಗ, oflag, _S_IREAD | _S_IWRITE);
#ಆದರೆ_ಅಂತ್ಯ
    ಇಲ್ಲದಿದ್ದರೆ {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_NONBLOCK
        ಆದರೆ (oflag & O_NONBLOCK)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_CLOEXEC
        ಆದರೆ (oflag & O_CLOEXEC)
            fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | O_CLOEXEC);
#ಆದರೆ_ಅಂತ್ಯ
        state->fd = fd;
    }
    ಆದರೆ (state->fd == -1) {
        ಬಿಡುಗಡೆ(state->ಮಾರ್ಗ);
        ಬಿಡುಗಡೆ(state);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಆದರೆ (state->ಬಹುಲಕ == GZ_APPEND) {
        LSEEK(state->fd, 0, SEEK_END);  /* so gzoffset() is correct */
        state->ಬಹುಲಕ = GZ_WRITE;         /* simplify later checks */
    }

    /* save the current position for rewinding (only if reading) */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ) {
        state->ಆರಂಭ = LSEEK(state->fd, 0, SEEK_CUR);
        ಆದರೆ (state->ಆರಂಭ == -1) state->ಆರಂಭ = 0;
    }

    /* initialize stream */
    gz_reset(state);

    /* return stream */
    ಹಿಂತಿರುಗಿ (gzFile)state;
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    ಹಿಂತಿರುಗಿ gz_open(ಮಾರ್ಗ, -1, ಬಹುಲಕ);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzopen64(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    ಹಿಂತಿರುಗಿ gz_open(ಮಾರ್ಗ, -1, ಬಹುಲಕ);
}

/* -- see zlib.h -- */
gzFile ZEXPORT gzdopen(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    ಅಕ್ಷರ *ಮಾರ್ಗ;         /* identifier for error messages */
    gzFile gz;

    ಆದರೆ (fd == -1 || (ಮಾರ್ಗ = (ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ಕೊಡು(7 + 3 * ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))) == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ಶೂನ್ಯ)snprintf(ಮಾರ್ಗ, 7 + 3 * ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ), "<fd:%d>", fd);
#ಇಲ್ಲದಿದ್ದರೆ
    sprintf(ಮಾರ್ಗ, "<fd:%d>", fd);   /* for debugging */
#ಆದರೆ_ಅಂತ್ಯ
    gz = gz_open(ಮಾರ್ಗ, fd, ಬಹುಲಕ);
    ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
    ಹಿಂತಿರುಗಿ gz;
}

/* -- see zlib.h -- */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WIDECHAR
gzFile ZEXPORT gzopen_w(ಸ್ಥಿರಾಂಕ wchar_t *ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
    ಹಿಂತಿರುಗಿ gz_open(ಮಾರ್ಗ, -2, ಬಹುಲಕ);
}
#ಆದರೆ_ಅಂತ್ಯ

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzbuffer(gzFile file, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ && state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ -1;

    /* make sure we haven't already allocated memory */
    ಆದರೆ (state->ಗಾತ್ರ_2 != 0)
        ಹಿಂತಿರುಗಿ -1;

    /* check and set requested size */
    ಆದರೆ ((ಗಾತ್ರ_2 << 1) < ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ -1;              /* need to be able to double it */
    ಆದರೆ (ಗಾತ್ರ_2 < 8)
        ಗಾತ್ರ_2 = 8;               /* needed to behave well with flushing */
    state->want = ಗಾತ್ರ_2;
    ಹಿಂತಿರುಗಿ 0;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzrewind(gzFile file) {
    gz_statep state;

    /* get internal structure */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;

    /* check that we're reading and that there's no error */
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ ||
            (state->err != Z_OK && state->err != Z_BUF_ERROR))
        ಹಿಂತಿರುಗಿ -1;

    /* back up and start over */
    ಆದರೆ (LSEEK(state->fd, state->ಆರಂಭ, SEEK_SET) == -1)
        ಹಿಂತಿರುಗಿ -1;
    gz_reset(state);
    ಹಿಂತಿರುಗಿ 0;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzseek64(gzFile file, z_off64_t ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ whence) {
    ಚಿಹ್ನೆರಹಿತ n;
    z_off64_t ret, pos;
    gz_statep state;

    /* get internal structure and check integrity */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ && state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ -1;

    /* check that there's no error */
    ಆದರೆ (state->err != Z_OK && state->err != Z_BUF_ERROR)
        ಹಿಂತಿರುಗಿ -1;

    /* can only seek from start or relative to current position */
    ಆದರೆ (whence != SEEK_SET && whence != SEEK_CUR)
        ಹಿಂತಿರುಗಿ -1;

    /* compute the absolute position */
    pos = whence == SEEK_SET ? 0 :
          state->x.pos + (state->past ? 0 : state->skip);
    ಆದರೆ (ಸ್ಥಾನಾಂತರ > GZ_OFF_MAX - pos)
        ಹಿಂತಿರುಗಿ -1;                          /* would overflow z_off64_t */
    ಆದರೆ (ಸ್ಥಾನಾಂತರ < -pos)
        ಹಿಂತಿರುಗಿ -1;                          /* before start of file */
    pos += ಸ್ಥಾನಾಂತರ;

    /* if within raw area while reading, just go there */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ && state->how == COPY) {
        ret = LSEEK(state->fd, pos - state->x.pos - (z_off64_t)state->x.have,
                    SEEK_CUR);
        ಆದರೆ (ret == -1)
            ಹಿಂತಿರುಗಿ -1;
        state->x.have = 0;
        state->eof = 0;
        state->past = 0;
        state->skip = 0;
        gz_error(state, Z_OK, ಖಾಲಿ);
        state->strm.avail_ಒಳ = 0;
        state->x.pos = pos;
        ಹಿಂತಿರುಗಿ pos;
    }

    /* rewind if needed for back seek when reading */
    ಆದರೆ (pos < state->x.pos) {
        ಆದರೆ (state->ಬಹುಲಕ != GZ_READ)         /* writing -- can't go backwards */
            ಹಿಂತಿರುಗಿ -1;
        ಆದರೆ (gzrewind(file) == -1)           /* rewind, then skip to pos */
            ಹಿಂತಿರುಗಿ -1;
    }
    ಸ್ಥಾನಾಂತರ = pos - state->x.pos;

    /* if reading, skip what's in output buffer (one less gzgetc() check) */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ) {
        n = GT_OFF(state->x.have) || (z_off64_t)state->x.have > ಸ್ಥಾನಾಂತರ ?
            (ಚಿಹ್ನೆರಹಿತ)ಸ್ಥಾನಾಂತರ : state->x.have;
        state->x.have -= n;
        state->x.ಮುಂದಿನ += n;
        state->x.pos += n;
        ಸ್ಥಾನಾಂತರ -= n;
    }

    /* request skip (if not zero) */
    state->skip = ಸ್ಥಾನಾಂತರ;
    ಹಿಂತಿರುಗಿ pos;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzseek(gzFile file, z_off_t ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ whence) {
    z_off64_t ret;

    ret = gzseek64(file, (z_off64_t)ಸ್ಥಾನಾಂತರ, whence);
    ಹಿಂತಿರುಗಿ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gztell64(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ && state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ -1;

    /* return position */
    ಹಿಂತಿರುಗಿ state->x.pos + (state->past ? 0 : state->skip);
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gztell(gzFile file) {
    z_off64_t ret;

    ret = gztell64(file);
    ಹಿಂತಿರುಗಿ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
z_off64_t ZEXPORT gzoffset64(gzFile file) {
    z_off64_t ಸ್ಥಾನಾಂತರ;
    gz_statep state;

    /* get internal structure and check integrity */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ -1;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ && state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ -1;

    /* compute and return effective offset in file */
    ಸ್ಥಾನಾಂತರ = LSEEK(state->fd, 0, SEEK_CUR);
    ಆದರೆ (ಸ್ಥಾನಾಂತರ == -1)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ)             /* reading */
        ಸ್ಥಾನಾಂತರ -= state->strm.avail_ಒಳ;     /* don't count buffered input */
    ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ;
}

/* -- see zlib.h -- */
z_off_t ZEXPORT gzoffset(gzFile file) {
    z_off64_t ret;

    ret = gzoffset64(file);
    ಹಿಂತಿರುಗಿ ret == (z_off_t)ret ? (z_off_t)ret : -1;
}

/* -- see zlib.h -- */
ಪೂರ್ಣಾಂಕ ZEXPORT gzeof(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ && state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ 0;

    /* return end-of-file state */
    ಹಿಂತಿರುಗಿ state->ಬಹುಲಕ == GZ_READ ? state->past : 0;
}

/* -- see zlib.h -- */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ZEXPORT gzerror(gzFile file, ಪೂರ್ಣಾಂಕ *errnum) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ && state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    /* return error information */
    ಆದರೆ (errnum != ಖಾಲಿ)
        *errnum = state->err;
    ಹಿಂತಿರುಗಿ state->err == Z_MEM_ERROR ? "out of memory" :
                                       (state->ಸಂದೇಶ == ಖಾಲಿ ? "" : state->ಸಂದೇಶ);
}

/* -- see zlib.h -- */
ಶೂನ್ಯ ZEXPORT gzclearerr(gzFile file) {
    gz_statep state;

    /* get internal structure and check integrity */
    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ;
    state = (gz_statep)file;
    ಆದರೆ (state->ಬಹುಲಕ != GZ_READ && state->ಬಹುಲಕ != GZ_WRITE)
        ಹಿಂತಿರುಗಿ;

    /* clear error and end-of-file */
    ಆದರೆ (state->ಬಹುಲಕ == GZ_READ) {
        state->eof = 0;
        state->past = 0;
    }
    gz_error(state, Z_OK, ಖಾಲಿ);
}

/* Create an error message in allocated memory and set state->err and
   state->msg accordingly.  Free any previous error message already there.  Do
   not try to free or allocate space if the error is Z_MEM_ERROR (out of
   memory).  Simply save the error message as a static string.  If there is an
   allocation failure constructing the error message, then convert the error to
   out of memory. */
ಶೂನ್ಯ ZLIB_ಆಂತರಿಕ gz_error(gz_statep state, ಪೂರ್ಣಾಂಕ err, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
    /* free previously allocated message and clear */
    ಆದರೆ (state->ಸಂದೇಶ != ಖಾಲಿ) {
        ಆದರೆ (state->err != Z_MEM_ERROR)
            ಬಿಡುಗಡೆ(state->ಸಂದೇಶ);
        state->ಸಂದೇಶ = ಖಾಲಿ;
    }

    /* if fatal, set state->x.have to 0 so that the gzgetc() macro fails */
    ಆದರೆ (err != Z_OK && err != Z_BUF_ERROR && !state->again)
        state->x.have = 0;

    /* set error code, and if no message, then done */
    state->err = err;
    ಆದರೆ (ಸಂದೇಶ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ;

    /* for an out of memory error, return literal string when requested */
    ಆದರೆ (err == Z_MEM_ERROR)
        ಹಿಂತಿರುಗಿ;

    /* construct error message with path */
    ಆದರೆ ((state->ಸಂದೇಶ = (ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ಕೊಡು(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(state->ಮಾರ್ಗ) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಂದೇಶ) + 3)) ==
            ಖಾಲಿ) {
        state->err = Z_MEM_ERROR;
        ಹಿಂತಿರುಗಿ;
    }
#ಆದರೆ !defined(NO_snprintf) && !defined(NO_vsnprintf)
    (ಶೂನ್ಯ)snprintf(state->ಸಂದೇಶ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(state->ಮಾರ್ಗ) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಂದೇಶ) + 3,
                   "%s%s%s", state->ಮಾರ್ಗ, ": ", ಸಂದೇಶ);
#ಇಲ್ಲದಿದ್ದರೆ
    strcpy(state->ಸಂದೇಶ, state->ಮಾರ್ಗ);
    strcat(state->ಸಂದೇಶ, ": ");
    strcat(state->ಸಂದೇಶ, ಸಂದೇಶ);
#ಆದರೆ_ಅಂತ್ಯ
}

/* portably return maximum value for an int (when limits.h presumed not
   available) -- we need to do this to cover cases where 2's complement not
   used, since C standard permits 1's complement and sign-bit representations,
   otherwise we could just use ((unsigned)-1) >> 1 */
ಚಿಹ್ನೆರಹಿತ ZLIB_ಆಂತರಿಕ gz_intmax(ಶೂನ್ಯ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ
    ಹಿಂತಿರುಗಿ ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
#ಇಲ್ಲದಿದ್ದರೆ
    ಚಿಹ್ನೆರಹಿತ p = 1, q;

    ಮಾಡು {
        q = p;
        p <<= 1;
        p++;
    } ಆಗ (p > q);
    ಹಿಂತಿರುಗಿ q >> 1;
#ಆದರೆ_ಅಂತ್ಯ
}
