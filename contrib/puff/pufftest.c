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

#اگر defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#شامل <stdio.h>
#شامل <stdlib.h>
#شامل "puff.h"

#اگر defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_دودویی_MODE(file) setmode(fileno(file), O_دودویی)
#وگرنه
#  define SET_دودویی_MODE(file)
#endif

#define local ایستا

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t size)
{
    صحیح n;
    size_t m;

    m = size;
    برای (n = 0; m; n++)
        m >>= 1;
    اگر (n < 3)
        بازگشت size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    بازگشت m > size ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local تهی *load(ثابت نویسه *name, size_t *len)
{
    size_t size;
    تهی *buf, *swap;
    FILE *in;

    *len = 0;
    buf = malloc(size = 4096);
    اگر (buf == پوچ)
        بازگشت پوچ;
    in = name == پوچ ? stdin : fopen(name, "rb");
    اگر (in != پوچ) {
        برای (;;) {
            *len += fread((نویسه *)buf + *len, 1, size - *len, in);
            اگر (*len < size) بشکن;
            size = bythirds(size);
            اگر (size == *len || (swap = realloc(buf, size)) == پوچ) {
                آزاد(buf);
                buf = پوچ;
                بشکن;
            }
            buf = swap;
        }
        fclose(in);
    }
    بازگشت buf;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
    صحیح ret, put = 0, fail = 0;
    unsigned skip = 0;
    نویسه *arg, *name = پوچ;
    unsigned نویسه *source = پوچ, *dest;
    size_t len = 0;
    unsigned long sourcelen, destlen;

    /* process arguments */
    وقتی (arg = *++argv, --argc)
        اگر (arg[0] == '-') {
            اگر (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            وگرنه اگر (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            وگرنه اگر (arg[1] >= '0' && arg[1] <= '9')
                skip = (unsigned)atoi(arg + 1);
            وگرنه {
                fprintf(stderr, "invalid option %s\n", arg);
                بازگشت 3;
            }
        }
        وگرنه اگر (name != پوچ) {
            fprintf(stderr, "only one file name allowed\n");
            بازگشت 3;
        }
        وگرنه
            name = arg;
    source = load(name, &len);
    اگر (source == پوچ) {
        fprintf(stderr, "memory allocation failure\n");
        بازگشت 4;
    }
    اگر (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == پوچ ? "<stdin>" : name);
        آزاد(source);
        بازگشت 3;
    }
    اگر (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        آزاد(source);
        بازگشت 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (unsigned long)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    اگر (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    وگرنه {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        اگر (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    اگر (put && ret == 0) {
        اگر (fail)
            destlen >>= 1;
        dest = malloc(destlen);
        اگر (dest == پوچ) {
            fprintf(stderr, "memory allocation failure\n");
            آزاد(source);
            بازگشت 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_دودویی_MODE(stdout);
        fwrite(dest, 1, destlen, stdout);
        آزاد(dest);
    }

    /* clean up */
    آزاد(source);
    بازگشت ret;
}
