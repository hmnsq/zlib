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

#எனில் defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ "puff.h"

#எனில் defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_இருமம்_MODE(file) setmode(fileno(file), O_இருமம்)
#இல்லையெனில்
#  define SET_இருமம்_MODE(file)
#endif

#define local நிலையான

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    முழு n;
    size_t m;

    m = size;
    க்கு (n = 0; m; n++)
        m >>= 1;
    எனில் (n < 3)
        திரும்பு size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    திரும்பு m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local வெறுமை *load(மாறிலி எழுத்து *name, size_t *len)
{
    size_t size;
    வெறுமை *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    எனில் (buf == வெற்று)
        திரும்பு வெற்று;
    in = name == வெற்று ? stdin : fopen(name, "rb");
    எனில் (in != வெற்று) {
        க்கு (;;) {
            *len += fread((எழுத்து *)buf + *len, 1, size - *len, in);
            எனில் (*len < size) நிறுத்து;
            size = bythirds(size);
            எனில் (size == *len || (swap = realloc(buf, size)) == வெற்று) {
                விடுவி(buf);
                buf = வெற்று;
                நிறுத்து;
            }
            buf = swap;
        }
        fclose(in);
    }
    திரும்பு buf;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
    முழு ret, put = 0, fail = 0;
    unsigned skip = 0;
    எழுத்து *arg, *name = வெற்று;
    unsigned எழுத்து *source = வெற்று, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    போது (arg = *++argv, --argc)
        எனில் (arg[0] == '-') {
            எனில் (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            இல்லையெனில் எனில் (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            இல்லையெனில் எனில் (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            இல்லையெனில் {
                fprintf(stderr, "invalid option %s\n", arg);
                திரும்பு 3;
            }
        }
        இல்லையெனில் எனில் (name != வெற்று) {
            fprintf(stderr, "only one file name allowed\n");
            திரும்பு 3;
        }
        இல்லையெனில்
            name = arg;
    source = load(name, &len);
    எனில் (source == வெற்று) {
        fprintf(stderr, "memory allocation failure\n");
        திரும்பு 4;
    }
    எனில் (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == வெற்று ? "<stdin>" : name);
        விடுவி(source);
        திரும்பு 3;
    }
    எனில் (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        விடுவி(source);
        திரும்பு 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    எனில் (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    இல்லையெனில் {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        எனில் (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    எனில் (put && ret == 0) {
        எனில் (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        எனில் (dest == வெற்று) {
            fprintf(stderr, "memory allocation failure\n");
            விடுவி(source);
            திரும்பு 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_இருமம்_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        விடுவி(dest);
    }

    /* clean up */
    விடுவி(source);
    திரும்பு ret;
}
