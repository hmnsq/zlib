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

#यदि defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "puff.h"

#यदि defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_द्विआधारी_MODE(file) setmode(fileno(file), O_द्विआधारी)
#अन्यथा
#  define SET_द्विआधारी_MODE(file)
#endif

#define local स्थिर

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local माप_प्रकार bythirds(माप_प्रकार size)
{
    पूर्णाङ्क n;
    माप_प्रकार m;

    m = size;
    लागि (n = 0; m; n++)
        m >>= 1;
    यदि (n < 3)
        फर्काऊ size + 1;
    n -= 3;
    m = size >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    फर्काऊ m > size ? m : (माप_प्रकार)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local शून्य *load(अचर वर्ण *name, माप_प्रकार *len)
{
    माप_प्रकार size;
    शून्य *buf, *swap;
    खाता *in;

    *len = 0;
    buf = दो_स्मृति(size = 4096);
    यदि (buf == रिक्त)
        फर्काऊ रिक्त;
    in = name == रिक्त ? मानक_निवेश : ख_खोलो(name, "rb");
    यदि (in != रिक्त) {
        लागि (;;) {
            *len += ख_पढ़ो((वर्ण *)buf + *len, 1, size - *len, in);
            यदि (*len < size) रोक;
            size = bythirds(size);
            यदि (size == *len || (swap = पुनः_स्मृति(buf, size)) == रिक्त) {
                मुक्त(buf);
                buf = रिक्त;
                रोक;
            }
            buf = swap;
        }
        ख_बंद(in);
    }
    फर्काऊ buf;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
    पूर्णाङ्क ret, put = 0, fail = 0;
    अचिन्हित skip = 0;
    वर्ण *arg, *name = रिक्त;
    अचिन्हित वर्ण *source = रिक्त, *dest;
    माप_प्रकार len = 0;
    अचिन्हित दीर्घ sourcelen, destlen;

    /* process arguments */
    जबसम्म (arg = *++argv, --argc)
        यदि (arg[0] == '-') {
            यदि (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            अन्यथा यदि (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            अन्यथा यदि (arg[1] >= '0' && arg[1] <= '9')
                skip = (अचिन्हित)म_से_प(arg + 1);
            अन्यथा {
                ख_लिखो(मानक_त्रुटि, "invalid option %s\n", arg);
                फर्काऊ 3;
            }
        }
        अन्यथा यदि (name != रिक्त) {
            ख_लिखो(मानक_त्रुटि, "only one file name allowed\n");
            फर्काऊ 3;
        }
        अन्यथा
            name = arg;
    source = load(name, &len);
    यदि (source == रिक्त) {
        ख_लिखो(मानक_त्रुटि, "memory allocation failure\n");
        फर्काऊ 4;
    }
    यदि (len == 0) {
        ख_लिखो(मानक_त्रुटि, "could not read %s, or it was empty\n",
                name == रिक्त ? "<stdin>" : name);
        मुक्त(source);
        फर्काऊ 3;
    }
    यदि (skip >= len) {
        ख_लिखो(मानक_त्रुटि, "skip request of %d leaves no input\n", skip);
        मुक्त(source);
        फर्काऊ 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (अचिन्हित दीर्घ)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    यदि (ret)
        ख_लिखो(मानक_त्रुटि, "puff() failed with return code %d\n", ret);
    अन्यथा {
        ख_लिखो(मानक_त्रुटि, "puff() succeeded uncompressing %lu bytes\n", destlen);
        यदि (sourcelen < len) ख_लिखो(मानक_त्रुटि, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    यदि (put && ret == 0) {
        यदि (fail)
            destlen >>= 1;
        dest = दो_स्मृति(destlen);
        यदि (dest == रिक्त) {
            ख_लिखो(मानक_त्रुटि, "memory allocation failure\n");
            मुक्त(source);
            फर्काऊ 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_द्विआधारी_MODE(मानक_निकास);
        ख_डालो(dest, 1, destlen, मानक_निकास);
        मुक्त(dest);
    }

    /* clean up */
    मुक्त(source);
    फर्काऊ ret;
}
