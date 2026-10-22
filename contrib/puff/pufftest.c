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

#אם defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن "puff.h"

#אם defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#אחרת
#  define SET_BINARY_MODE(file)
#endif

#define local ساكن

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    صحيح n;
    size_t m;

    m = size;
    עבור (n = 0; m; n++)
        m >>= 1;
    אם (n < 3)
        החזר size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    החזר m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local فراغ *load(ثابت حرف *name, size_t *len)
{
    size_t size;
    فراغ *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    אם (buf == عدم)
        החזר عدم;
    in = name == عدم ? stdin : fopen(name, "rb");
    אם (in != عدم) {
        עבור (;;) {
            *len += fread((حرف *)buf + *len, 1, size - *len, in);
            אם (*len < size) اكسر;
            size = bythirds(size);
            אם (size == *len || (swap = realloc(buf, size)) == عدم) {
                حرر(buf);
                buf = عدم;
                اكسر;
            }
            buf = swap;
        }
        fclose(in);
    }
    החזר buf;
}

صحيح main(صحيح argc, حرف **argv)
{
    صحيح ret, put = 0, fail = 0;
    unsigned skip = 0;
    حرف *arg, *name = عدم;
    unsigned حرف *source = عدم, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    כלעוד (arg = *++argv, --argc)
        אם (arg[0] == '-') {
            אם (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            אחרת אם (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            אחרת אם (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            אחרת {
                fprintf(stderr, "invalid option %s\n", arg);
                החזר 3;
            }
        }
        אחרת אם (name != عدم) {
            fprintf(stderr, "only one file name allowed\n");
            החזר 3;
        }
        אחרת
            name = arg;
    source = load(name, &len);
    אם (source == عدم) {
        fprintf(stderr, "memory allocation failure\n");
        החזר 4;
    }
    אם (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == عدم ? "<stdin>" : name);
        حرر(source);
        החזר 3;
    }
    אם (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        حرر(source);
        החזר 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    אם (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    אחרת {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        אם (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    אם (put && ret == 0) {
        אם (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        אם (dest == عدم) {
            fprintf(stderr, "memory allocation failure\n");
            حرر(source);
            החזר 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_BINARY_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        حرر(dest);
    }

    /* clean up */
    حرر(source);
    החזר ret;
}
