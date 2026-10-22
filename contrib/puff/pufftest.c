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

#ଯଦି defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ "puff.h"

#ଯଦି defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_ଦ୍ୱିମିକ_MODE(file) setmode(fileno(file), O_ଦ୍ୱିମିକ)
#ଅନ୍ୟଥା
#  define SET_ଦ୍ୱିମିକ_MODE(file)
#endif

#define local ସ୍ଥିର

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
    size_t m;

    m = size;
    ପାଇଁ (n = 0; m; n++)
        m >>= 1;
    ଯଦି (n < 3)
        ଫେରନ୍ତୁ size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    ଫେରନ୍ତୁ m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local ଶୂନ୍ୟ *load(ଅଚଳ ଅକ୍ଷର *name, size_t *len)
{
    size_t size;
    ଶୂନ୍ୟ *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    ଯଦି (buf == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;
    in = name == ଖାଲି ? stdin : fopen(name, "rb");
    ଯଦି (in != ଖାଲି) {
        ପାଇଁ (;;) {
            *len += fread((ଅକ୍ଷର *)buf + *len, 1, size - *len, in);
            ଯଦି (*len < size) ଭାଙ୍ଗ;
            size = bythirds(size);
            ଯଦି (size == *len || (swap = realloc(buf, size)) == ଖାଲି) {
                ମୁକ୍ତ(buf);
                buf = ଖାଲି;
                ଭାଙ୍ଗ;
            }
            buf = swap;
        }
        fclose(in);
    }
    ଫେରନ୍ତୁ buf;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, put = 0, fail = 0;
    unsigned skip = 0;
    ଅକ୍ଷର *arg, *name = ଖାଲି;
    unsigned ଅକ୍ଷର *source = ଖାଲି, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    ଯେତେବେଳେ (arg = *++argv, --argc)
        ଯଦି (arg[0] == '-') {
            ଯଦି (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            ଅନ୍ୟଥା ଯଦି (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            ଅନ୍ୟଥା ଯଦି (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            ଅନ୍ୟଥା {
                fprintf(stderr, "invalid option %s\n", arg);
                ଫେରନ୍ତୁ 3;
            }
        }
        ଅନ୍ୟଥା ଯଦି (name != ଖାଲି) {
            fprintf(stderr, "only one file name allowed\n");
            ଫେରନ୍ତୁ 3;
        }
        ଅନ୍ୟଥା
            name = arg;
    source = load(name, &len);
    ଯଦି (source == ଖାଲି) {
        fprintf(stderr, "memory allocation failure\n");
        ଫେରନ୍ତୁ 4;
    }
    ଯଦି (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == ଖାଲି ? "<stdin>" : name);
        ମୁକ୍ତ(source);
        ଫେରନ୍ତୁ 3;
    }
    ଯଦି (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        ମୁକ୍ତ(source);
        ଫେରନ୍ତୁ 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    ଯଦି (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    ଅନ୍ୟଥା {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        ଯଦି (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    ଯଦି (put && ret == 0) {
        ଯଦି (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        ଯଦି (dest == ଖାଲି) {
            fprintf(stderr, "memory allocation failure\n");
            ମୁକ୍ତ(source);
            ଫେରନ୍ତୁ 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_ଦ୍ୱିମିକ_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        ମୁକ୍ତ(dest);
    }

    /* clean up */
    ମୁକ୍ତ(source);
    ଫେରନ୍ତୁ ret;
}
