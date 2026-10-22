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

#अगर defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#पूर्ण_अगर

#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश "puff.h"

#अगर defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_द्विआधारी_MODE(file) setmode(fileno(file), O_द्विआधारी)
#अन्यथा
#  define SET_द्विआधारी_MODE(file)
#पूर्ण_अगर

#घोषणा local अटल

/* Return size times approximately the cube root of 2, keeping the result as 1,
   3, or 5 times a power of 2 -- the result is always > size, until the result
   is the maximum value of an unsigned long, where it remains.  This is useful
   to keep reallocations less than ~33% over the actual data. */
local माप_प्रकार bythirds(माप_प्रकार आकार)
{
    पूर्णांक n;
    माप_प्रकार m;

    m = आकार;
    क्रम (n = 0; m; n++)
        m >>= 1;
    अगर (n < 3)
        वापस आकार + 1;
    n -= 3;
    m = आकार >> n;
    m += m == 6 ? 2 : 1;
    m <<= n;
    वापस m > आकार ? m : (माप_प्रकार)(-1);
}

/* Read the input file *name, or stdin if name is NULL, into allocated memory.
   Reallocate to larger buffers until the entire file is read in.  Return a
   pointer to the allocated data, or NULL if there was a memory allocation
   failure.  *len is the number of bytes of data read from the input file (even
   if load() returns NULL).  If the input file was empty or could not be opened
   or read, *len is zero. */
local व्योम *load(स्थिर अक्षर *name, माप_प्रकार *len)
{
    माप_प्रकार आकार;
    व्योम *buf, *swap;
    खाता *in;

    *len = 0;
    buf = दो_स्मृति(आकार = 4096);
    अगर (buf == शून्य)
        वापस शून्य;
    in = name == शून्य ? मानक_निवेश : ख_खोलो(name, "rb");
    अगर (in != शून्य) {
        क्रम (;;) {
            *len += ख_पढ़ो((अक्षर *)buf + *len, 1, आकार - *len, in);
            अगर (*len < आकार) अवरोध;
            आकार = bythirds(आकार);
            अगर (आकार == *len || (swap = पुनः_स्मृति(buf, आकार)) == शून्य) {
                मुक्त(buf);
                buf = शून्य;
                अवरोध;
            }
            buf = swap;
        }
        ख_बंद(in);
    }
    वापस buf;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
    पूर्णांक ret, put = 0, fail = 0;
    अचिन्हित skip = 0;
    अक्षर *arg, *name = शून्य;
    अचिन्हित अक्षर *source = शून्य, *dest;
    माप_प्रकार len = 0;
    अचिन्हित दीर्घ sourcelen, destlen;

    /* process arguments */
    जबतक (arg = *++argv, --argc)
        अगर (arg[0] == '-') {
            अगर (arg[1] == 'w' && arg[2] == 0)
                put = 1;
            अन्यथा अगर (arg[1] == 'f' && arg[2] == 0)
                fail = 1, put = 1;
            अन्यथा अगर (arg[1] >= '0' && arg[1] <= '9')
                skip = (अचिन्हित)म_से_प(arg + 1);
            अन्यथा {
                ख_लिखो(मानक_त्रुटि, "invalid option %s\n", arg);
                वापस 3;
            }
        }
        अन्यथा अगर (name != शून्य) {
            ख_लिखो(मानक_त्रुटि, "only one file name allowed\n");
            वापस 3;
        }
        अन्यथा
            name = arg;
    source = load(name, &len);
    अगर (source == शून्य) {
        ख_लिखो(मानक_त्रुटि, "memory allocation failure\n");
        वापस 4;
    }
    अगर (len == 0) {
        ख_लिखो(मानक_त्रुटि, "could not read %s, or it was empty\n",
                name == शून्य ? "<stdin>" : name);
        मुक्त(source);
        वापस 3;
    }
    अगर (skip >= len) {
        ख_लिखो(मानक_त्रुटि, "skip request of %d leaves no input\n", skip);
        मुक्त(source);
        वापस 3;
    }

    /* test inflate data with offset skip */
    len -= skip;
    sourcelen = (अचिन्हित दीर्घ)len;
    ret = puff(NIL, &destlen, source + skip, &sourcelen);
    अगर (ret)
        ख_लिखो(मानक_त्रुटि, "puff() failed with return code %d\n", ret);
    अन्यथा {
        ख_लिखो(मानक_त्रुटि, "puff() succeeded uncompressing %lu bytes\n", destlen);
        अगर (sourcelen < len) ख_लिखो(मानक_त्रुटि, "%lu compressed bytes unused\n",
                                     len - sourcelen);
    }

    /* if requested, inflate again and write decompressed data to stdout */
    अगर (put && ret == 0) {
        अगर (fail)
            destlen >>= 1;
        dest = दो_स्मृति(destlen);
        अगर (dest == शून्य) {
            ख_लिखो(मानक_त्रुटि, "memory allocation failure\n");
            मुक्त(source);
            वापस 4;
        }
        puff(dest, &destlen, source + skip, &sourcelen);
        SET_द्विआधारी_MODE(मानक_निकास);
        ख_डालो(dest, 1, destlen, मानक_निकास);
        मुक्त(dest);
    }

    /* clean up */
    मुक्त(source);
    वापस ret;
}
