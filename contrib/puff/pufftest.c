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

#إذا defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#جيڪڏهن_پورو

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن "puff.h"

#إذا defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define سيٽ_ٻٽو_گهڻائي(file) setmode(fileno(file), O_ٻٽو)
#وإلا
#  define سيٽ_ٻٽو_گهڻائي(file)
#جيڪڏهن_پورو

#وصف_ڪريو local ساكن

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t سائيز)
{
    صحيح n;
    size_t m;

    m = سائيز;
    لكل (n = 0; m; n++)
        m >>= 1;
    إذا (n < 3)
        أرجع سائيز + 1;
    n -= 3;
    m = سائيز >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    أرجع m > سائيز ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local فراغ *ڀريو(ثابت حرف *name, size_t *len)
{
    size_t سائيز;
    فراغ *buf, *مٽا_سٽا;
    FILE *اندريون;

    *len = 0;
    buf = يادگيري_ڏيو(سائيز = 4096);
    إذا (buf == عدم)
        أرجع عدم;
    اندريون = name == عدم ? stdin : fopen(name, "rb");
    إذا (اندريون != عدم) {
        لكل (;;) {
            *len += fread((حرف *)buf + *len, 1, سائيز - *len, اندريون);
            إذا (*len < سائيز) اكسر;
            سائيز = bythirds(سائيز);
            إذا (سائيز == *len || (مٽا_سٽا = realloc(buf, سائيز)) == عدم) {
                حرر(buf);
                buf = عدم;
                اكسر;
            }
            buf = مٽا_سٽا;
        }
        fclose(اندريون);
    }
    أرجع buf;
}

صحيح مکيه(صحيح argc, حرف **argv)
{
    صحيح ret, put = 0, fail = 0;
    بنانشان skip = 0;
    حرف *arg, *name = عدم;
    بنانشان حرف *ذريعو = عدم, *dest;
    size_t len = 0;
    بنانشان ڊگهو sourcelen, destlen;

    /* process arguments */
    طالما (arg = *++argv, --argc)
        إذا (arg[0] == '-') {
            إذا (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            وإلا إذا (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            وإلا إذا (arg[1] >= '0' && arg[1] <= '9')
                skip = (بنانشان)atoi(arg + 1);
            وإلا {
                fprintf(stderr, "invalid option %s\n", arg);
                أرجع 3;
            }
        }
        وإلا إذا (name != عدم) {
            fprintf(stderr, "only one file name allowed\n");
            أرجع 3;
        }
        وإلا
            name = arg;
    ذريعو = ڀريو(name, &len);
    إذا (ذريعو == عدم) {
        fprintf(stderr, "memory allocation failure\n");
        أرجع 4;
    }
    إذا (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == عدم ? "<stdin>" : name);
        حرر(ذريعو);
        أرجع 3;
    }
    إذا (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        حرر(ذريعو);
        أرجع 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (بنانشان ڊگهو)len;
    ret = puff(NIL, &destlen, ذريعو + skip, &sourcelen);
    إذا (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    وإلا {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        إذا (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    إذا (put && ret == 0) {
        إذا (fail)
            destlen >>= 1;
        dest = يادگيري_ڏيو(destlen);
        إذا (dest == عدم) {
            fprintf(stderr, "memory allocation failure\n");
            حرر(ذريعو);
            أرجع 4;
        }
        puff(dest, &destlen, ذريعو + skip, &sourcelen);
        سيٽ_ٻٽو_گهڻائي(stdout);
        fwrite(dest, 1, destlen, stdout);
        حرر(dest);
    }

    /* clean up */
    حرر(ذريعو);
    أرجع ret;
}
