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

#གལ་སྲིད defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdlib.h>
#include "puff.h"

#གལ་སྲིད defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#དེ་མིན
#  define SET_BINARY_MODE(file)
#endif

#define local static

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    ཧྲིལ n;
    size_t m;

    m = size;
    དོན་ལུ (n = 0; m; n++)
        m >>= 1;
    གལ་སྲིད (n < 3)
        ལོག size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    ལོག m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local void *load(const var_na *name, size_t *len)
{
    size_t size;
    void *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    གལ་སྲིད (buf == NULL)
        ལོག NULL;
    in = name == NULL ? stdin : fopen(name, "rb");
    གལ་སྲིད (in != NULL) {
        དོན་ལུ (;;) {
            *len += fread((var_na *)buf + *len, 1, size - *len, in);
            གལ་སྲིད (*len < size) bha_nga;
            size = bythirds(size);
            གལ་སྲིད (size == *len || (swap = realloc(buf, size)) == NULL) {
                free(buf);
                buf = NULL;
                bha_nga;
            }
            buf = swap;
        }
        fclose(in);
    }
    ལོག buf;
}

ཧྲིལ main(ཧྲིལ argc, var_na **argv)
{
    ཧྲིལ ret, put = 0, fail = 0;
    unsigned skip = 0;
    var_na *arg, *name = NULL;
    unsigned var_na *source = NULL, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    སྐབས (arg = *++argv, --argc)
        གལ་སྲིད (arg[0] == '-') {
            གལ་སྲིད (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            དེ་མིན གལ་སྲིད (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            དེ་མིན གལ་སྲིད (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            དེ་མིན {
                fprintf(stderr, "invalid option %s\n", arg);
                ལོག 3;
            }
        }
        དེ་མིན གལ་སྲིད (name != NULL) {
            fprintf(stderr, "only one file name allowed\n");
            ལོག 3;
        }
        དེ་མིན
            name = arg;
    source = load(name, &len);
    གལ་སྲིད (source == NULL) {
        fprintf(stderr, "memory allocation failure\n");
        ལོག 4;
    }
    གལ་སྲིད (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == NULL ? "<stdin>" : name);
        free(source);
        ལོག 3;
    }
    གལ་སྲིད (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        free(source);
        ལོག 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    གལ་སྲིད (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    དེ་མིན {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        གལ་སྲིད (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    གལ་སྲིད (put && ret == 0) {
        གལ་སྲིད (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        གལ་སྲིད (dest == NULL) {
            fprintf(stderr, "memory allocation failure\n");
            free(source);
            ལོག 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_BINARY_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        free(dest);
    }

    /* clean up */
    free(source);
    ལོག ret;
}
