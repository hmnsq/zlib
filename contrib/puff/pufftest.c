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

#如果 defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdlib.h>
#include "puff.h"

#如果 defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#否则
#  define SET_BINARY_MODE(file)
#endif

#define local static

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    整数 n;
    size_t m;

    m = size;
    为 (n = 0; m; n++)
        m >>= 1;
    如果 (n < 3)
        返回 size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    返回 m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local void *load(const char *name, size_t *len)
{
    size_t size;
    void *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    如果 (buf == NULL)
        返回 NULL;
    in = name == NULL ? stdin : fopen(name, "rb");
    如果 (in != NULL) {
        为 (;;) {
            *len += fread((char *)buf + *len, 1, size - *len, in);
            如果 (*len < size) break;
            size = bythirds(size);
            如果 (size == *len || (swap = realloc(buf, size)) == NULL) {
                free(buf);
                buf = NULL;
                break;
            }
            buf = swap;
        }
        fclose(in);
    }
    返回 buf;
}

整数 主要(整数 argc, char **argv)
{
    整数 ret, put = 0, fail = 0;
    unsigned skip = 0;
    char *arg, *name = NULL;
    unsigned char *source = NULL, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    当 (arg = *++argv, --argc)
        如果 (arg[0] == '-') {
            如果 (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            否则 如果 (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            否则 如果 (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            否则 {
                fprintf(stderr, "invalid option %s\n", arg);
                返回 3;
            }
        }
        否则 如果 (name != NULL) {
            fprintf(stderr, "only one file name allowed\n");
            返回 3;
        }
        否则
            name = arg;
    source = load(name, &len);
    如果 (source == NULL) {
        fprintf(stderr, "memory allocation failure\n");
        返回 4;
    }
    如果 (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == NULL ? "<stdin>" : name);
        free(source);
        返回 3;
    }
    如果 (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        free(source);
        返回 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    如果 (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    否则 {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        如果 (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    如果 (put && ret == 0) {
        如果 (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        如果 (dest == NULL) {
            fprintf(stderr, "memory allocation failure\n");
            free(source);
            返回 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_BINARY_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        free(dest);
    }

    /* clean up */
    free(source);
    返回 ret;
}
