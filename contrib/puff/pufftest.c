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

#ఒకవేళ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#అయితే_ముగింపు

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ "puff.h"

#ఒకవేళ defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define సమితి_ద్విమాన_బహుళకం(file) setmode(fileno(file), O_ద్విమాన)
#లేకపోతే
#  define సమితి_ద్విమాన_బహుళకం(file)
#అయితే_ముగింపు

#నిర్వచించు local స్థిర

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local size_t bythirds(size_t కొలత)
{
    పూర్ణాంకం n;
    size_t m;

    m = కొలత;
    కొరకు (n = 0; m; n++)
        m >>= 1;
    ఒకవేళ (n < 3)
        తిరిగి కొలత + 1;
    n -= 3;
    m = కొలత >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    తిరిగి m > కొలత ? m : (size_t)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local శూన్య *ఎక్కించు(స్థిరరాశి అక్షరం *name, size_t *len)
{
    size_t కొలత;
    శూన్య *buf, *మార్పిడి;
    FILE *లోపల;

    *len = 0;
    buf = జ్ఞాపకం_ఇవ్వు(కొలత = 4096);
    ఒకవేళ (buf == ఖాళీ)
        తిరిగి ఖాళీ;
    లోపల = name == ఖాళీ ? stdin : fopen(name, "rb");
    ఒకవేళ (లోపల != ఖాళీ) {
        కొరకు (;;) {
            *len += fread((అక్షరం *)buf + *len, 1, కొలత - *len, లోపల);
            ఒకవేళ (*len < కొలత) ఆపు;
            కొలత = bythirds(కొలత);
            ఒకవేళ (కొలత == *len || (మార్పిడి = realloc(buf, కొలత)) == ఖాళీ) {
                విడుదల(buf);
                buf = ఖాళీ;
                ఆపు;
            }
            buf = మార్పిడి;
        }
        fclose(లోపల);
    }
    తిరిగి buf;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
    పూర్ణాంకం ret, put = 0, fail = 0;
    గుర్తులేని skip = 0;
    అక్షరం *arg, *name = ఖాళీ;
    గుర్తులేని అక్షరం *ఆధారం = ఖాళీ, *dest;
    size_t len = 0;
    గుర్తులేని దీర్ఘ sourcelen, destlen;

    /* process arguments */
    అయితే (arg = *++argv, --argc)
        ఒకవేళ (arg[0] == '-') {
            ఒకవేళ (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            లేకపోతే ఒకవేళ (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            లేకపోతే ఒకవేళ (arg[1] >= '0' && arg[1] <= '9')
                skip = (గుర్తులేని)atoi(arg + 1);
            లేకపోతే {
                fprintf(stderr, "invalid option %s\n", arg);
                తిరిగి 3;
            }
        }
        లేకపోతే ఒకవేళ (name != ఖాళీ) {
            fprintf(stderr, "only one file name allowed\n");
            తిరిగి 3;
        }
        లేకపోతే
            name = arg;
    ఆధారం = ఎక్కించు(name, &len);
    ఒకవేళ (ఆధారం == ఖాళీ) {
        fprintf(stderr, "memory allocation failure\n");
        తిరిగి 4;
    }
    ఒకవేళ (len == 0) {
        fprintf(stderr, "could not read %s, or it was empty\n",
                name == ఖాళీ ? "<stdin>" : name);
        విడుదల(ఆధారం);
        తిరిగి 3;
    }
    ఒకవేళ (skip >= len) {
        fprintf(stderr, "skip request of %d leaves no input\n", skip);
        విడుదల(ఆధారం);
        తిరిగి 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (గుర్తులేని దీర్ఘ)len;
    ret = puff(NIL, &destlen, ఆధారం + skip, &sourcelen);
    ఒకవేళ (ret)
        fprintf(stderr, "puff() failed with return code %d\n", ret);
    లేకపోతే {
        fprintf(stderr, "puff() succeeded uncompressing %lu bytes\n", destlen);
        ఒకవేళ (sourcelen < len) fprintf(stderr, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    ఒకవేళ (put && ret == 0) {
        ఒకవేళ (fail)
            destlen >>= 1;
        dest = జ్ఞాపకం_ఇవ్వు(destlen);
        ఒకవేళ (dest == ఖాళీ) {
            fprintf(stderr, "memory allocation failure\n");
            విడుదల(ఆధారం);
            తిరిగి 4;
        }
        puff(dest, &destlen, ఆధారం + skip, &sourcelen);
        సమితి_ద్విమాన_బహుళకం(stdout);
        fwrite(dest, 1, destlen, stdout);
        విడుదల(dest);
    }

    /* clean up */
    విడుదల(ఆధారం);
    తిరిగి ret;
}
