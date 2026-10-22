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

#যদি defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব "puff.h"

#যদি defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_দ্বিমিক_MODE(file) setmode(fileno(file), O_দ্বিমিক)
#অন্যথা
#  define SET_দ্বিমিক_MODE(file)
#endif

#define local স্থির

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    পূর্ণাঙ্ক n;
    size_t m;

    m = size;
    প্রতি (n = 0; m; n++)
        m >>= 1;
    যদি (n < 3)
        প্রত্যাগম size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    প্রত্যাগম m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local শূন্য *load(ধ্রুব বর্ণ *name, size_t *len)
{
    size_t size;
    শূন্য *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    যদি (buf == রিক্ত)
        প্রত্যাগম রিক্ত;
    in = name == রিক্ত ? stdin : fopen(name, "rb");
    যদি (in != রিক্ত) {
        প্রতি (;;) {
            *len += fread((বর্ণ *)buf + *len, 1, size - *len, in);
            যদি (*len < size) ভঙ্গ;
            size = bythirds(size);
            যদি (size == *len || (swap = realloc(buf, size)) == রিক্ত) {
                মুক্ত(buf);
                buf = রিক্ত;
                ভঙ্গ;
            }
            buf = swap;
        }
        fclose(in);
    }
    প্রত্যাগম buf;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
    পূর্ণাঙ্ক ret, put = 0, fail = 0;
    unsigned skip = 0;
    বর্ণ *arg, *name = রিক্ত;
    unsigned বর্ণ *source = রিক্ত, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    যাবত্ (arg = *++argv, --argc)
        যদি (arg[0] == '-') {
            যদি (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            অন্যথা যদি (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            অন্যথা যদি (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            অন্যথা {
                fprintf(stderr, "invalid option %s\n", arg);
                প্রত্যাগম 3;
            }
        }
        অন্যথা যদি (name != রিক্ত) {
            fprintf(stderr, "only one file name allowed\n");
            প্রত্যাগম 3;
        }
        অন্যথা
            name = arg;
    source = load(name, &len);
    যদি (source == রিক্ত) {
        fprintf(stderr, "memory allocation failure\n");
        প্রত্যাগম 4;
    }
    যদি (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == রিক্ত ? "<stdin>" : name);
        মুক্ত(source);
        প্রত্যাগম 3;
    }
    যদি (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        মুক্ত(source);
        প্রত্যাগম 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    যদি (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    অন্যথা {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        যদি (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    যদি (put && ret == 0) {
        যদি (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        যদি (dest == রিক্ত) {
            fprintf(stderr, "memory allocation failure\n");
            মুক্ত(source);
            প্রত্যাগম 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_দ্বিমিক_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        মুক্ত(dest);
    }

    /* clean up */
    মুক্ত(source);
    প্রত্যাগম ret;
}
