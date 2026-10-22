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

#ਜੇ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "puff.h"

#ਜੇ defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file) setmode(fileno(file), O_ਦੁਆਧਾਰੀ)
#ਨਹੀਂਤਾਂ
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ local ਸਥਿਰ

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t ਮਾਪ)
{
    ਪੂਰਨ n;
    size_t m;

    m = ਮਾਪ;
    ਲਈ (n = 0; m; n++)
        m >>= 1;
    ਜੇ (n < 3)
        ਵਾਪਸ ਮਾਪ + 1;
    n -= 3;
    m = ਮਾਪ >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    ਵਾਪਸ m > ਮਾਪ ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local ਖਾਲੀ *ਲੱਦੋ(ਅਚਲ ਅੱਖਰ *name, size_t *len)
{
    size_t ਮਾਪ;
    ਖਾਲੀ *buf, *ਅਦਲਾ_ਬਦਲੀ;
    FILE *ਵਿੱਚ;

    *len = 0;
    buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ = 4096);
    ਜੇ (buf == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਿੱਚ = name == ਨਿਰਾ ? stdin : fopen(name, "rb");
    ਜੇ (ਵਿੱਚ != ਨਿਰਾ) {
        ਲਈ (;;) {
            *len += fread((ਅੱਖਰ *)buf + *len, 1, ਮਾਪ - *len, ਵਿੱਚ);
            ਜੇ (*len < ਮਾਪ) ਰੋਕੋ;
            ਮਾਪ = bythirds(ਮਾਪ);
            ਜੇ (ਮਾਪ == *len || (ਅਦਲਾ_ਬਦਲੀ = realloc(buf, ਮਾਪ)) == ਨਿਰਾ) {
                ਮੁਕਤ(buf);
                buf = ਨਿਰਾ;
                ਰੋਕੋ;
            }
            buf = ਅਦਲਾ_ਬਦਲੀ;
        }
        fclose(ਵਿੱਚ);
    }
    ਵਾਪਸ buf;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
    ਪੂਰਨ ret, put = 0, fail = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ skip = 0;
    ਅੱਖਰ *arg, *name = ਨਿਰਾ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਸਰੋਤ = ਨਿਰਾ, *dest;
    size_t len = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sourcelen, destlen;

    /* process arguments */
    ਜਦੋਂ (arg = *++argv, --argc)
        ਜੇ (arg[0] == '-') {
            ਜੇ (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            ਨਹੀਂਤਾਂ ਜੇ (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            ਨਹੀਂਤਾਂ ਜੇ (arg[1] >= '0' && arg[1] <= '9')
                skip = (ਬਿਨਾਨਿਸ਼ਾਨੀ)atoi(arg + 1);
            ਨਹੀਂਤਾਂ {
                fprintf(stderr, "invalid option %s\n", arg);
                ਵਾਪਸ 3;
            }
        }
        ਨਹੀਂਤਾਂ ਜੇ (name != ਨਿਰਾ) {
            fprintf(stderr, "only one file name allowed\n");
            ਵਾਪਸ 3;
        }
        ਨਹੀਂਤਾਂ
            name = arg;
    ਸਰੋਤ = ਲੱਦੋ(name, &len);
    ਜੇ (ਸਰੋਤ == ਨਿਰਾ) {
        fprintf(stderr, "memory allocation failure\n");
        ਵਾਪਸ 4;
    }
    ਜੇ (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == ਨਿਰਾ ? "<stdin>" : name);
        ਮੁਕਤ(ਸਰੋਤ);
        ਵਾਪਸ 3;
    }
    ਜੇ (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        ਮੁਕਤ(ਸਰੋਤ);
        ਵਾਪਸ 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)len;
    ret = puff(NIL, &destlen, ਸਰੋਤ + skip, &sourcelen);
    ਜੇ (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    ਨਹੀਂਤਾਂ {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        ਜੇ (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    ਜੇ (put && ret == 0) {
        ਜੇ (fail)
            destlen >>= 1;
        dest = ਸਮ੍ਰਿਤੀ_ਦਿਓ(destlen);
        ਜੇ (dest == ਨਿਰਾ) {
            fprintf(stderr, "memory allocation failure\n");
            ਮੁਕਤ(ਸਰੋਤ);
            ਵਾਪਸ 4;
        }
        puff(dest, &destlen, ਸਰੋਤ + skip, &sourcelen);
        ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdout);
        fwrite(dest, 1, destlen, stdout);
        ਮੁਕਤ(dest);
    }

    /* clean up */
    ਮੁਕਤ(ਸਰੋਤ);
    ਵਾਪਸ ret;
}
