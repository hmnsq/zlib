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
#اگر_ختم

#شامل <stdio.h>
#شامل <stdlib.h>
#شامل "puff.h"

#اگر defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define سیٹ_ثنائی_منوال(file) setmode(fileno(file), O_ثنائی)
#ورنہ
#  define سیٹ_ثنائی_منوال(file)
#اگر_ختم

#تعریف_کرو local ساکن

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t سائز)
{
    صحیح n;
    size_t m;

    m = سائز;
    برائے (n = 0; m; n++)
        m >>= 1;
    اگر (n < 3)
        واپس سائز + 1;
    n -= 3;
    m = سائز >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    واپس m > سائز ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local خالی *بھرو(ثابت حرف *name, size_t *len)
{
    size_t سائز;
    خالی *buf, *تبادلہ;
    FILE *اندر;

    *len = 0;
    buf = حافظہ_دو(سائز = 4096);
    اگر (buf == تہی)
        واپس تہی;
    اندر = name == تہی ? stdin : fopen(name, "rb");
    اگر (اندر != تہی) {
        برائے (;;) {
            *len += fread((حرف *)buf + *len, 1, سائز - *len, اندر);
            اگر (*len < سائز) توڑو;
            سائز = bythirds(سائز);
            اگر (سائز == *len || (تبادلہ = realloc(buf, سائز)) == تہی) {
                آزاد(buf);
                buf = تہی;
                توڑو;
            }
            buf = تبادلہ;
        }
        fclose(اندر);
    }
    واپس buf;
}

صحیح مین(صحیح argc, حرف **argv)
{
    صحیح ret, put = 0, fail = 0;
    غیرعلامتی skip = 0;
    حرف *arg, *name = تہی;
    غیرعلامتی حرف *ماخذ = تہی, *dest;
    size_t len = 0;
    غیرعلامتی طویل sourcelen, destlen;

    /* process arguments */
    جب (arg = *++argv, --argc)
        اگر (arg[0] == '-') {
            اگر (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            ورنہ اگر (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            ورنہ اگر (arg[1] >= '0' && arg[1] <= '9')
                skip = (غیرعلامتی)atoi(arg + 1);
            ورنہ {
                fprintf(stderr, "invalid option %s\n", arg);
                واپس 3;
            }
        }
        ورنہ اگر (name != تہی) {
            fprintf(stderr, "only one file name allowed\n");
            واپس 3;
        }
        ورنہ
            name = arg;
    ماخذ = بھرو(name, &len);
    اگر (ماخذ == تہی) {
        fprintf(stderr, "memory allocation failure\n");
        واپس 4;
    }
    اگر (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == تہی ? "<stdin>" : name);
        آزاد(ماخذ);
        واپس 3;
    }
    اگر (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        آزاد(ماخذ);
        واپس 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (غیرعلامتی طویل)len;
    ret = puff(NIL, &destlen, ماخذ + skip, &sourcelen);
    اگر (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    ورنہ {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        اگر (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    اگر (put && ret == 0) {
        اگر (fail)
            destlen >>= 1;
        dest = حافظہ_دو(destlen);
        اگر (dest == تہی) {
            fprintf(stderr, "memory allocation failure\n");
            آزاد(ماخذ);
            واپس 4;
        }
        puff(dest, &destlen, ماخذ + skip, &sourcelen);
        سیٹ_ثنائی_منوال(stdout);
        fwrite(dest, 1, destlen, stdout);
        آزاد(dest);
    }

    /* clean up */
    آزاد(ماخذ);
    واپس ret;
}
