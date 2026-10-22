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

#જો defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ "puff.h"

#જો defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_દ્વિઅંકી_MODE(file) setmode(fileno(file), O_દ્વિઅંકી)
#નહીંતર
#  define SET_દ્વિઅંકી_MODE(file)
#endif

#define local સ્થિર

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    પૂર્ણાંક n;
    size_t m;

    m = size;
    માટે (n = 0; m; n++)
        m >>= 1;
    જો (n < 3)
        પાછા size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    પાછા m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local શૂન્ય *load(અચલ અક્ષર *name, size_t *len)
{
    size_t size;
    શૂન્ય *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    જો (buf == ખાલી)
        પાછા ખાલી;
    in = name == ખાલી ? stdin : fopen(name, "rb");
    જો (in != ખાલી) {
        માટે (;;) {
            *len += fread((અક્ષર *)buf + *len, 1, size - *len, in);
            જો (*len < size) તોડો;
            size = bythirds(size);
            જો (size == *len || (swap = realloc(buf, size)) == ખાલી) {
                મુક્ત(buf);
                buf = ખાલી;
                તોડો;
            }
            buf = swap;
        }
        fclose(in);
    }
    પાછા buf;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
    પૂર્ણાંક ret, put = 0, fail = 0;
    unsigned skip = 0;
    અક્ષર *arg, *name = ખાલી;
    unsigned અક્ષર *source = ખાલી, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    જ્યારે (arg = *++argv, --argc)
        જો (arg[0] == '-') {
            જો (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            નહીંતર જો (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            નહીંતર જો (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            નહીંતર {
                fprintf(stderr, "invalid option %s\n", arg);
                પાછા 3;
            }
        }
        નહીંતર જો (name != ખાલી) {
            fprintf(stderr, "only one file name allowed\n");
            પાછા 3;
        }
        નહીંતર
            name = arg;
    source = load(name, &len);
    જો (source == ખાલી) {
        fprintf(stderr, "memory allocation failure\n");
        પાછા 4;
    }
    જો (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == ખાલી ? "<stdin>" : name);
        મુક્ત(source);
        પાછા 3;
    }
    જો (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        મુક્ત(source);
        પાછા 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    જો (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    નહીંતર {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        જો (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    જો (put && ret == 0) {
        જો (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        જો (dest == ખાલી) {
            fprintf(stderr, "memory allocation failure\n");
            મુક્ત(source);
            પાછા 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_દ્વિઅંકી_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        મુક્ત(dest);
    }

    /* clean up */
    મુક્ત(source);
    પાછા ret;
}
