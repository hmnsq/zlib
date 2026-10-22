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

#എങ്കിൽ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ "puff.h"

#എങ്കിൽ defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_ദ്വിമാന_MODE(file) setmode(fileno(file), O_ദ്വിമാന)
#അല്ലെങ്കിൽ
#  define SET_ദ്വിമാന_MODE(file)
#endif

#define local സ്ഥിതം

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    പൂർണ്ണസംഖ്യ n;
    size_t m;

    m = size;
    വേണ്ടി (n = 0; m; n++)
        m >>= 1;
    എങ്കിൽ (n < 3)
        തിരികെ size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    തിരികെ m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local ശൂന്യം *load(സ്ഥിരം അക്ഷരം *name, size_t *len)
{
    size_t size;
    ശൂന്യം *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    എങ്കിൽ (buf == ശൂന്യ)
        തിരികെ ശൂന്യ;
    in = name == ശൂന്യ ? stdin : fopen(name, "rb");
    എങ്കിൽ (in != ശൂന്യ) {
        വേണ്ടി (;;) {
            *len += fread((അക്ഷരം *)buf + *len, 1, size - *len, in);
            എങ്കിൽ (*len < size) നിർത്തുക;
            size = bythirds(size);
            എങ്കിൽ (size == *len || (swap = realloc(buf, size)) == ശൂന്യ) {
                സ്വതന്ത്രം(buf);
                buf = ശൂന്യ;
                നിർത്തുക;
            }
            buf = swap;
        }
        fclose(in);
    }
    തിരികെ buf;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
    പൂർണ്ണസംഖ്യ ret, put = 0, fail = 0;
    unsigned skip = 0;
    അക്ഷരം *arg, *name = ശൂന്യ;
    unsigned അക്ഷരം *source = ശൂന്യ, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    ആകുമ്പോൾ (arg = *++argv, --argc)
        എങ്കിൽ (arg[0] == '-') {
            എങ്കിൽ (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            അല്ലെങ്കിൽ എങ്കിൽ (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            അല്ലെങ്കിൽ എങ്കിൽ (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            അല്ലെങ്കിൽ {
                fprintf(stderr, "invalid option %s\n", arg);
                തിരികെ 3;
            }
        }
        അല്ലെങ്കിൽ എങ്കിൽ (name != ശൂന്യ) {
            fprintf(stderr, "only one file name allowed\n");
            തിരികെ 3;
        }
        അല്ലെങ്കിൽ
            name = arg;
    source = load(name, &len);
    എങ്കിൽ (source == ശൂന്യ) {
        fprintf(stderr, "memory allocation failure\n");
        തിരികെ 4;
    }
    എങ്കിൽ (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == ശൂന്യ ? "<stdin>" : name);
        സ്വതന്ത്രം(source);
        തിരികെ 3;
    }
    എങ്കിൽ (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        സ്വതന്ത്രം(source);
        തിരികെ 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    എങ്കിൽ (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    അല്ലെങ്കിൽ {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        എങ്കിൽ (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    എങ്കിൽ (put && ret == 0) {
        എങ്കിൽ (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        എങ്കിൽ (dest == ശൂന്യ) {
            fprintf(stderr, "memory allocation failure\n");
            സ്വതന്ത്രം(source);
            തിരികെ 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_ദ്വിമാന_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        സ്വതന്ത്രം(dest);
    }

    /* clean up */
    സ്വതന്ത്രം(source);
    തിരികെ ret;
}
