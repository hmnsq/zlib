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

#𐡄𐡍 defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن "puff.h"

#𐡄𐡍 defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#وإلا
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
    لكل (n = 0; m; n++)
        m >>= 1;
    𐡄𐡍 (n < 3)
        أرجع size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    أرجع m > size ? m : (size_t)(-1);
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
    𐡄𐡍 (buf == عدم)
        أرجع عدم;
    in = name == عدم ? stdin : fopen(name, "rb");
    𐡄𐡍 (in != عدم) {
        لكل (;;) {
            *len += fread((حرف *)buf + *len, 1, size - *len, in);
            𐡄𐡍 (*len < size) اكسر;
            size = bythirds(size);
            𐡄𐡍 (size == *len || (swap = realloc(buf, size)) == عدم) {
                حرر(buf);
                buf = عدم;
                اكسر;
            }
            buf = swap;
        }
        fclose(in);
    }
    أرجع buf;
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
    طالما (arg = *++argv, --argc)
        𐡄𐡍 (arg[0] == '-') {
            𐡄𐡍 (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            وإلا 𐡄𐡍 (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            وإلا 𐡄𐡍 (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            وإلا {
                fprintf(stderr, "invalid option %s\n", arg);
                أرجع 3;
            }
        }
        وإلا 𐡄𐡍 (name != عدم) {
            fprintf(stderr, "only one file name allowed\n");
            أرجع 3;
        }
        وإلا
            name = arg;
    source = load(name, &len);
    𐡄𐡍 (source == عدم) {
        fprintf(stderr, "memory allocation failure\n");
        أرجع 4;
    }
    𐡄𐡍 (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == عدم ? "<stdin>" : name);
        حرر(source);
        أرجع 3;
    }
    𐡄𐡍 (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        حرر(source);
        أرجع 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    𐡄𐡍 (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    وإلا {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        𐡄𐡍 (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    𐡄𐡍 (put && ret == 0) {
        𐡄𐡍 (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        𐡄𐡍 (dest == عدم) {
            fprintf(stderr, "memory allocation failure\n");
            حرر(source);
            أرجع 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_BINARY_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        حرر(dest);
    }

    /* clean up */
    حرر(source);
    أرجع ret;
}
