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

#ಆದರೆ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "puff.h"

#ಆದರೆ defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file) setmode(fileno(file), O_ದ್ವಿಮಾನ)
#ಇಲ್ಲದಿದ್ದರೆ
#  define ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(file)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು local ಸ್ಥಿರ

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t ಗಾತ್ರ_2)
{
    ಪೂರ್ಣಾಂಕ n;
    size_t m;

    m = ಗಾತ್ರ_2;
    ಗಾಗಿ (n = 0; m; n++)
        m >>= 1;
    ಆದರೆ (n < 3)
        ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2 + 1;
    n -= 3;
    m = ಗಾತ್ರ_2 >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    ಹಿಂತಿರುಗಿ m > ಗಾತ್ರ_2 ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local ಶೂನ್ಯ *ತುಂಬು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, size_t *len)
{
    size_t ಗಾತ್ರ_2;
    ಶೂನ್ಯ *buf, *ಅದಲುಬದಲು;
    FILE *ಒಳ;

    *len = 0;
    buf = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2 = 4096);
    ಆದರೆ (buf == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಒಳ = name == ಖಾಲಿ ? stdin : fopen(name, "rb");
    ಆದರೆ (ಒಳ != ಖಾಲಿ) {
        ಗಾಗಿ (;;) {
            *len += fread((ಅಕ್ಷರ *)buf + *len, 1, ಗಾತ್ರ_2 - *len, ಒಳ);
            ಆದರೆ (*len < ಗಾತ್ರ_2) ನಿಲ್ಲಿಸು;
            ಗಾತ್ರ_2 = bythirds(ಗಾತ್ರ_2);
            ಆದರೆ (ಗಾತ್ರ_2 == *len || (ಅದಲುಬದಲು = realloc(buf, ಗಾತ್ರ_2)) == ಖಾಲಿ) {
                ಬಿಡುಗಡೆ(buf);
                buf = ಖಾಲಿ;
                ನಿಲ್ಲಿಸು;
            }
            buf = ಅದಲುಬದಲು;
        }
        fclose(ಒಳ);
    }
    ಹಿಂತಿರುಗಿ buf;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
    ಪೂರ್ಣಾಂಕ ret, put = 0, fail = 0;
    ಚಿಹ್ನೆರಹಿತ skip = 0;
    ಅಕ್ಷರ *arg, *name = ಖಾಲಿ;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮೂಲ = ಖಾಲಿ, *dest;
    size_t len = 0;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sourcelen, destlen;

    /* process arguments */
    ಆಗ (arg = *++argv, --argc)
        ಆದರೆ (arg[0] == '-') {
            ಆದರೆ (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg[1] >= '0' && arg[1] <= '9')
                skip = (ಚಿಹ್ನೆರಹಿತ)atoi(arg + 1);
            ಇಲ್ಲದಿದ್ದರೆ {
                fprintf(stderr, "invalid option %s\n", arg);
                ಹಿಂತಿರುಗಿ 3;
            }
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (name != ಖಾಲಿ) {
            fprintf(stderr, "only one file name allowed\n");
            ಹಿಂತಿರುಗಿ 3;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            name = arg;
    ಮೂಲ = ತುಂಬು(name, &len);
    ಆದರೆ (ಮೂಲ == ಖಾಲಿ) {
        fprintf(stderr, "memory allocation failure\n");
        ಹಿಂತಿರುಗಿ 4;
    }
    ಆದರೆ (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == ಖಾಲಿ ? "<stdin>" : name);
        ಬಿಡುಗಡೆ(ಮೂಲ);
        ಹಿಂತಿರುಗಿ 3;
    }
    ಆದರೆ (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        ಬಿಡುಗಡೆ(ಮೂಲ);
        ಹಿಂತಿರುಗಿ 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)len;
    ret = puff(NIL, &destlen, ಮೂಲ + skip, &sourcelen);
    ಆದರೆ (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    ಇಲ್ಲದಿದ್ದರೆ {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        ಆದರೆ (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    ಆದರೆ (put && ret == 0) {
        ಆದರೆ (fail)
            destlen >>= 1;
        dest = ಸ್ಮರಣೆ_ಕೊಡು(destlen);
        ಆದರೆ (dest == ಖಾಲಿ) {
            fprintf(stderr, "memory allocation failure\n");
            ಬಿಡುಗಡೆ(ಮೂಲ);
            ಹಿಂತಿರುಗಿ 4;
        }
        puff(dest, &destlen, ಮೂಲ + skip, &sourcelen);
        ಗಣ_ದ್ವಿಮಾನ_ಬಹುಲಕ(stdout);
        fwrite(dest, 1, destlen, stdout);
        ಬಿಡುಗಡೆ(dest);
    }

    /* clean up */
    ಬಿಡುಗಡೆ(ಮೂಲ);
    ಹಿಂತಿರುಗಿ ret;
}
