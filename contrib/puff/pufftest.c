/*
 * pufftest.c
 * Copyright (C) 2002-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in puff.h
 */

/* Example of how to use puff().

   Usage: puff [-w] [-f] [-nnn] file
          ... | puff [-w] [-f] [-nnn]

   where file is the input file with deflate data, nnn is the number of bytes
   of input to skip before inflating (e.g. to skip a zlib or gzip header), and
   -w is used to write the decompressed data to stdout.  -f is for coverage
   testing, and causes pufftest to fail with not enough output space (-f does
   a write like -w, so -w is not required). */

#ಯದಿ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "puff.h"

#ಯದಿ defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_ದ್ವಿಮಾನ_MODE(file) setmode(fileno(file), O_ದ್ವಿಮಾನ)
#ಅನ್ಯಥಾ
#  define SET_ದ್ವಿಮಾನ_MODE(file)
#endif

#define local ಸ್ಥಿರ

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    ಪೂರ್ಣಾಙ್ಕ n;
    size_t m;

    m = size;
    ಪ್ರತಿ (n = 0; m; n++)
        m >>= 1;
    ಯದಿ (n < 3)
        ಪ್ರತ್ಯಾಗಮ size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    ಪ್ರತ್ಯಾಗಮ m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local ಶೂನ್ಯ *load(ಧ್ರುವ ವರ್ಣ *name, size_t *len)
{
    size_t size;
    ಶೂನ್ಯ *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    ಯದಿ (buf == ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    in = name == ರಿಕ್ತ ? stdin : fopen(name, "rb");
    ಯದಿ (in != ರಿಕ್ತ) {
        ಪ್ರತಿ (;;) {
            *len += fread((ವರ್ಣ *)buf + *len, 1, size - *len, in);
            ಯದಿ (*len < size) ಭಙ್ಗ;
            size = bythirds(size);
            ಯದಿ (size == *len || (swap = realloc(buf, size)) == ರಿಕ್ತ) {
                ಮುಕ್ತ(buf);
                buf = ರಿಕ್ತ;
                ಭಙ್ಗ;
            }
            buf = swap;
        }
        fclose(in);
    }
    ಪ್ರತ್ಯಾಗಮ buf;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
    ಪೂರ್ಣಾಙ್ಕ ret, put = 0, fail = 0;
    unsigned skip = 0;
    ವರ್ಣ *arg, *name = ರಿಕ್ತ;
    unsigned ವರ್ಣ *source = ರಿಕ್ತ, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    ಯಾವತ್ (arg = *++argv, --argc)
        ಯದಿ (arg[0] == '-') {
            ಯದಿ (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            ಅನ್ಯಥಾ ಯದಿ (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            ಅನ್ಯಥಾ ಯದಿ (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            ಅನ್ಯಥಾ {
                fprintf(stderr, "invalid option %s\n", arg);
                ಪ್ರತ್ಯಾಗಮ 3;
            }
        }
        ಅನ್ಯಥಾ ಯದಿ (name != ರಿಕ್ತ) {
            fprintf(stderr, "only one file name allowed\n");
            ಪ್ರತ್ಯಾಗಮ 3;
        }
        ಅನ್ಯಥಾ
            name = arg;
    source = load(name, &len);
    ಯದಿ (source == ರಿಕ್ತ) {
        fprintf(stderr, "memory allocation failure\n");
        ಪ್ರತ್ಯಾಗಮ 4;
    }
    ಯದಿ (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == ರಿಕ್ತ ? "<stdin>" : name);
        ಮುಕ್ತ(source);
        ಪ್ರತ್ಯಾಗಮ 3;
    }
    ಯದಿ (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        ಮುಕ್ತ(source);
        ಪ್ರತ್ಯಾಗಮ 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    ಯದಿ (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    ಅನ್ಯಥಾ {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        ಯದಿ (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    ಯದಿ (put && ret == 0) {
        ಯದಿ (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        ಯದಿ (dest == ರಿಕ್ತ) {
            fprintf(stderr, "memory allocation failure\n");
            ಮುಕ್ತ(source);
            ಪ್ರತ್ಯಾಗಮ 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_ದ್ವಿಮಾನ_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        ಮುಕ್ತ(dest);
    }

    /* clean up */
    ಮುಕ್ತ(source);
    ಪ್ರತ್ಯಾಗಮ ret;
}
