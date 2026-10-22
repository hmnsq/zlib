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

#सचे defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "puff.h"

#सचे defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_द्विमान_MODE(file) setmode(fileno(file), O_द्विमान)
#अञ्ञथा
#  define SET_द्विमान_MODE(file)
#endif

#define local स्थिर

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    पूर्णाङ्क n;
    size_t m;

    m = size;
    अत्थाय (n = 0; m; n++)
        m >>= 1;
    सचे (n < 3)
        पटिनिवत्त size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    पटिनिवत्त m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local शून्य *load(ध्रुव वर्ण *name, size_t *len)
{
    size_t size;
    शून्य *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    सचे (buf == रिक्त)
        पटिनिवत्त रिक्त;
    in = name == रिक्त ? stdin : fopen(name, "rb");
    सचे (in != रिक्त) {
        अत्थाय (;;) {
            *len += fread((वर्ण *)buf + *len, 1, size - *len, in);
            सचे (*len < size) भङ्ग;
            size = bythirds(size);
            सचे (size == *len || (swap = realloc(buf, size)) == रिक्त) {
                मुक्त(buf);
                buf = रिक्त;
                भङ्ग;
            }
            buf = swap;
        }
        fclose(in);
    }
    पटिनिवत्त buf;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
    पूर्णाङ्क ret, put = 0, fail = 0;
    unsigned skip = 0;
    वर्ण *arg, *name = रिक्त;
    unsigned वर्ण *source = रिक्त, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    याव (arg = *++argv, --argc)
        सचे (arg[0] == '-') {
            सचे (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            अञ्ञथा सचे (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            अञ्ञथा सचे (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            अञ्ञथा {
                fprintf(stderr, "invalid option %s\n", arg);
                पटिनिवत्त 3;
            }
        }
        अञ्ञथा सचे (name != रिक्त) {
            fprintf(stderr, "only one file name allowed\n");
            पटिनिवत्त 3;
        }
        अञ्ञथा
            name = arg;
    source = load(name, &len);
    सचे (source == रिक्त) {
        fprintf(stderr, "memory allocation failure\n");
        पटिनिवत्त 4;
    }
    सचे (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == रिक्त ? "<stdin>" : name);
        मुक्त(source);
        पटिनिवत्त 3;
    }
    सचे (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        मुक्त(source);
        पटिनिवत्त 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    सचे (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    अञ्ञथा {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        सचे (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    सचे (put && ret == 0) {
        सचे (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        सचे (dest == रिक्त) {
            fprintf(stderr, "memory allocation failure\n");
            मुक्त(source);
            पटिनिवत्त 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_द्विमान_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        मुक्त(dest);
    }

    /* clean up */
    मुक्त(source);
    पटिनिवत्त ret;
}
