/* infcover.c -- test zlib's inflate routines with full code coverage
 * Copyright (C) 2011, 2016, 2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* to use, do: ./configure --cover && make cover */

#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>
#समावेश "zlib.h"

/* get definition of internal structure so we can mess with it (see pull()),
   and so we can call inflate_trees() (see cover5()) */
#घोषणा ZLIB_INTERNAL
#समावेश "inftrees.h"
#समावेश "inflate.h"

#घोषणा local अटल

/* -- memory tracking routines -- */

/*
   These memory tracking routines are provided to zlib and track all of zlib's
   allocations and deallocations, check for LIFO operations, keep a current
   and high water mark of total bytes requested, optionally set a limit on the
   total memory that can be allocated, and when done check for memory leaks.

   They are used as follows:

   z_stream strm;
   mem_setup(&strm)         initializes the memory tracking and sets the
                            zalloc, zfree, and opaque members of strm to use
                            memory tracking for all zlib operations on strm
   mem_limit(&strm, limit)  sets a limit on the total bytes requested -- a
                            request that exceeds this limit will result in an
                            allocation failure (returns NULL) -- setting the
                            limit to zero means no limit, which is the default
                            after mem_setup()
   mem_used(&strm, "msg")   prints to stderr "msg" and the total bytes used
   mem_high(&strm, "msg")   prints to stderr "msg" and the high water mark
   mem_done(&strm, "msg")   ends memory tracking, releases all allocations
                            for the tracking as well as leaked zlib blocks, if
                            any.  If there was anything unusual, such as leaked
                            blocks, non-FIFO frees, or frees of addresses not
                            allocated, then "msg" and information about the
                            problem is printed to stderr.  If everything is
                            normal, nothing is printed. mem_done resets the
                            strm members to Z_NULL to use the default memory
                            allocation routines on the next zlib initialization
                            using strm.
 */

/* these items are strung together in a linked list, one for each allocation */
काष्ठा mem_item {
    व्योम *ptr;                  /* pointer to allocated memory */
    माप_प्रकार आकार;                /* requested size of allocation */
    काष्ठा mem_item *अग्र;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
काष्ठा mem_zone {
    काष्ठा mem_item *first;     /* pointer to first item in list, or NULL */
    माप_प्रकार योग_2, highwater;    /* total allocations, and largest total */
    माप_प्रकार limit;               /* memory allocation limit, or 0 if no limit */
    पूर्णांक notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local व्योम *mem_alloc(व्योम *mem, अचिन्हित गिनती, अचिन्हित आकार)
{
    व्योम *ptr;
    काष्ठा mem_item *item;
    काष्ठा mem_zone *zone = mem;
    माप_प्रकार len = गिनती * (माप_प्रकार)आकार;

    /* induced allocation failure */
    अगर (zone == शून्य || (zone->limit && zone->योग_2 + len > zone->limit))
        वापस शून्य;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = दो_स्मृति(len);
    अगर (ptr == शून्य)
        वापस शून्य;
    स_रखो(ptr, 0xa5, len);

    /* create a new item for the list */
    item = दो_स्मृति(माप(काष्ठा mem_item));
    अगर (item == शून्य) {
        मुक्त(ptr);
        वापस शून्य;
    }
    item->ptr = ptr;
    item->आकार = len;

    /* insert item at the beginning of the list */
    item->अग्र = zone->first;
    zone->first = item;

    /* update the statistics */
    zone->योग_2 += item->आकार;
    अगर (zone->योग_2 > zone->highwater)
        zone->highwater = zone->योग_2;

    /* return the allocated memory */
    वापस ptr;
}

/* memory free routine to pass to zlib */
local व्योम mem_free(व्योम *mem, व्योम *ptr)
{
    काष्ठा mem_item *item, *अग्र;
    काष्ठा mem_zone *zone = mem;

    /* if no zone, just do a free */
    अगर (zone == शून्य) {
        मुक्त(ptr);
        वापस;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    अग्र = zone->first;
    अगर (अग्र) {
        अगर (अग्र->ptr == ptr)
            zone->first = अग्र->अग्र;   /* first one is it, remove from list */
        अन्यथा {
            करो {                        /* search the linked list */
                item = अग्र;
                अग्र = item->अग्र;
            } जबतक (अग्र != शून्य && अग्र->ptr != ptr);
            अगर (अग्र) {                 /* if found, remove from linked list */
                item->अग्र = अग्र->अग्र;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    अगर (अग्र) {
        zone->योग_2 -= अग्र->आकार;
        मुक्त(अग्र);
    }

    /* if not found, update the rogue count */
    अन्यथा
        zone->rogue++;

    /* in any case, do the requested free with the standard library function */
    मुक्त(ptr);
}

/* set up a controlled memory allocation space for monitoring, set the stream
   parameters to the controlled routines, with opaque pointing to the space */
local व्योम mem_setup(z_stream *strm)
{
    काष्ठा mem_zone *zone;

    zone = दो_स्मृति(माप(काष्ठा mem_zone));
    निश्चित(zone != शून्य);
    zone->first = शून्य;
    zone->योग_2 = 0;
    zone->highwater = 0;
    zone->limit = 0;
    zone->notlifo = 0;
    zone->rogue = 0;
    strm->opaque = zone;
    strm->zalloc = mem_alloc;
    strm->zfree = mem_free;
}

/* set a limit on the total memory allocation, or 0 to remove the limit */
local व्योम mem_limit(z_stream *strm, माप_प्रकार limit)
{
    काष्ठा mem_zone *zone = strm->opaque;

    zone->limit = limit;
}

/* show the current total requested allocations in bytes */
local व्योम mem_used(z_stream *strm, अक्षर *prefix)
{
    काष्ठा mem_zone *zone = strm->opaque;

    ख_लिखो(मानक_त्रुटि, "%s: %zu allocated\n", prefix, zone->योग_2);
}

/* show the high water allocation in bytes */
local व्योम mem_ऊपरी(z_stream *strm, अक्षर *prefix)
{
    काष्ठा mem_zone *zone = strm->opaque;

    ख_लिखो(मानक_त्रुटि, "%s: %zu high water mark\n", prefix, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local व्योम mem_done(z_stream *strm, अक्षर *prefix)
{
    पूर्णांक गिनती = 0;
    काष्ठा mem_item *item, *अग्र;
    काष्ठा mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_ऊपरी(strm, prefix);

    /* free leftover allocations and item structures, if any */
    item = zone->first;
    जबतक (item != शून्य) {
        मुक्त(item->ptr);
        अग्र = item->अग्र;
        मुक्त(item);
        item = अग्र;
        गिनती++;
    }

    /* issue alerts about anything unexpected */
    अगर (गिनती || zone->योग_2)
        ख_लिखो(मानक_त्रुटि, "** %s: %zu bytes in %d blocks not freed\n",
                prefix, zone->योग_2, गिनती);
    अगर (zone->notlifo)
        ख_लिखो(मानक_त्रुटि, "** %s: %d frees not LIFO\n", prefix, zone->notlifo);
    अगर (zone->rogue)
        ख_लिखो(मानक_त्रुटि, "** %s: %d frees not recognized\n",
                prefix, zone->rogue);

    /* free the zone and delete from the stream */
    मुक्त(zone);
    strm->opaque = Z_NULL;
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
}

/* -- inflate test routines -- */

/* Decode a hexadecimal string, set *len to length, in[] to the bytes.  This
   decodes liberally, in that hex digits can be adjacent, in which case two in
   a row writes a byte.  Or they can be delimited by any non-hex character,
   where the delimiters are ignored except when a single hex digit is followed
   by a delimiter, where that single digit writes a byte.  The returned data is
   allocated and must eventually be freed.  NULL is returned if out of memory.
   If the length is not needed, then len can be NULL. */
local अचिन्हित अक्षर *h2b(स्थिर अक्षर *hex, अचिन्हित *len)
{
    अचिन्हित अक्षर *in, *re;
    अचिन्हित अग्र, val;

    in = दो_स्मृति((म_माप(hex) + 1) >> 1);
    अगर (in == शून्य)
        वापस शून्य;
    अग्र = 0;
    val = 1;
    करो {
        अगर (*hex >= '0' && *hex <= '9')
            val = (val << 4) + *hex - '0';
        अन्यथा अगर (*hex >= 'A' && *hex <= 'F')
            val = (val << 4) + *hex - 'A' + 10;
        अन्यथा अगर (*hex >= 'a' && *hex <= 'f')
            val = (val << 4) + *hex - 'a' + 10;
        अन्यथा अगर (val != 1 && val < 32)  /* one digit followed by delimiter */
            val += 240;                 /* make it look like two digits */
        अगर (val > 255) {                /* have two digits */
            in[अग्र++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } जबतक (*hex++);       /* go through the loop with the terminating null */
    अगर (len != शून्य)
        *len = अग्र;
    re = पुनः_स्मृति(in, अग्र);
    वापस re == शून्य ? in : re;
}

/* generic inflate() run, where hex is the hexadecimal input data, what is the
   text to include in an error message, step is how much input data to feed
   inflate() on each call, or zero to feed it all, win is the window bits
   parameter to inflateInit2(), len is the size of the output buffer, and err
   is the error code expected from the first inflate() call (the second
   inflate() call is expected to return Z_STREAM_END).  If win is 47, then
   header information is collected with inflateGetHeader().  If a zlib stream
   is looking for a dictionary, then an empty dictionary is provided.
   inflate() is run until all of the input data is consumed. */
local व्योम inf(अक्षर *hex, अक्षर *what, अचिन्हित step, पूर्णांक win, अचिन्हित len,
               पूर्णांक err)
{
    पूर्णांक ret;
    अचिन्हित have;
    अचिन्हित अक्षर *in, *out;
    z_stream strm, copy;
    gz_header शीर्ष;

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.अग्र_in = Z_NULL;
    ret = inflateInit2(&strm, win);
    अगर (ret != Z_OK) {
        mem_done(&strm, what);
        वापस;
    }
    out = दो_स्मृति(len);                          निश्चित(out != शून्य);
    अगर (win == 47) {
        शीर्ष.extra = out;
        शीर्ष.extra_max = len;
        शीर्ष.name = out;
        शीर्ष.name_max = len;
        शीर्ष.comment = out;
        शीर्ष.comm_max = len;
        ret = inflateGetHeader(&strm, &शीर्ष);   निश्चित(ret == Z_OK);
    }
    in = h2b(hex, &have);                       निश्चित(in != शून्य);
    अगर (step == 0 || step > have)
        step = have;
    strm.avail_in = step;
    have -= step;
    strm.अग्र_in = in;
    करो {
        strm.avail_out = len;
        strm.अग्र_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);       निश्चित(err == 9 || ret == err);
        अगर (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            अवरोध;
        अगर (ret == Z_NEED_DICT) {
            ret = inflateSetDictionary(&strm, in, 1);
                                                निश्चित(ret == Z_तथ्य_ERROR);
            mem_limit(&strm, 1);
            ret = inflateSetDictionary(&strm, out, 0);
                                                निश्चित(ret == Z_MEM_ERROR);
            mem_limit(&strm, 0);
            ((काष्ठा inflate_state *)strm.state)->mode = DICT;
            ret = inflateSetDictionary(&strm, out, 0);
                                                निश्चित(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   निश्चित(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        निश्चित(ret == Z_OK);
        ret = inflateEnd(&copy);                निश्चित(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_in;
        strm.avail_in = step > have ? have : step;
        have -= strm.avail_in;
    } जबतक (strm.avail_in);
    मुक्त(in);
    मुक्त(out);
    ret = inflateReset2(&strm, -8);             निश्चित(ret == Z_OK);
    ret = inflateEnd(&strm);                    निश्चित(ret == Z_OK);
    mem_done(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local व्योम cover_support(व्योम)
{
    पूर्णांक ret;
    z_stream strm;

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.अग्र_in = Z_NULL;
    ret = inflateInit(&strm);                   निश्चित(ret == Z_OK);
    mem_used(&strm, "inflate init");
    ret = inflate_अभाज्य(&strm, 5, 31);           निश्चित(ret == Z_OK);
    ret = inflate_अभाज्य(&strm, -1, 0);           निश्चित(ret == Z_OK);
    ret = inflateSetDictionary(&strm, Z_NULL, 0);
                                                निश्चित(ret == Z_STREAM_ERROR);
    ret = inflateEnd(&strm);                    निश्चित(ret == Z_OK);
    mem_done(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_STREAM_END);
    inf("", "bad window size", 0, 1, 0, Z_STREAM_ERROR);

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.अग्र_in = Z_NULL;
    ret = inflateInit_(&strm, "!", (पूर्णांक)माप(z_stream));
                                                निश्चित(ret == Z_VERSION_ERROR);
    mem_done(&strm, "wrong version");

    strm.avail_in = 0;
    strm.अग्र_in = Z_NULL;
    ret = inflateInit(&strm);                   निश्चित(ret == Z_OK);
    ret = inflateEnd(&strm);                    निश्चित(ret == Z_OK);
    ख_माला_दो("inflate built-in memory routines\n", मानक_त्रुटि);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local व्योम cover_wrap(व्योम)
{
    पूर्णांक ret;
    z_stream strm, copy;
    अचिन्हित अक्षर dict[257];

    ret = inflate(Z_NULL, 0);                   निश्चित(ret == Z_STREAM_ERROR);
    ret = inflateEnd(Z_NULL);                   निश्चित(ret == Z_STREAM_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          निश्चित(ret == Z_STREAM_ERROR);
    ख_माला_दो("inflate bad parameters\n", मानक_त्रुटि);

    inf("1f 8b 0 0", "bad gzip method", 0, 31, 0, Z_तथ्य_ERROR);
    inf("1f 8b 8 80", "bad gzip flags", 0, 31, 0, Z_तथ्य_ERROR);
    inf("77 85", "bad zlib method", 0, 15, 0, Z_तथ्य_ERROR);
    inf("8 99", "set window size from header", 0, 0, 0, Z_OK);
    inf("78 9c", "bad zlib window size", 0, 8, 0, Z_तथ्य_ERROR);
    inf("78 9c 63 0 0 0 1 0 1", "check adler32", 0, 15, 1, Z_STREAM_END);
    inf("1f 8b 8 1e 0 0 0 0 0 0 1 0 0 0 0 0 0", "bad header crc", 0, 47, 1,
        Z_तथ्य_ERROR);
    inf("1f 8b 8 2 0 0 0 0 0 0 1d 26 3 0 0 0 0 0 0 0 0 0", "check gzip length",
        0, 47, 0, Z_STREAM_END);
    inf("78 90", "bad zlib header check", 0, 47, 0, Z_तथ्य_ERROR);
    inf("8 b8 0 0 0 1", "need dictionary", 0, 8, 0, Z_NEED_DICT);
    inf("78 9c 63 0", "compute adler32", 0, 15, 1, Z_OK);

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.अग्र_in = Z_NULL;
    ret = inflateInit2(&strm, -8);
    strm.avail_in = 2;
    strm.अग्र_in = (व्योम *)"\x63";
    strm.avail_out = 1;
    strm.अग्र_out = (व्योम *)&ret;
    mem_limit(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           निश्चित(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           निश्चित(ret == Z_MEM_ERROR);
    mem_limit(&strm, 0);
    स_रखो(dict, 0, 257);
    ret = inflateSetDictionary(&strm, dict, 257);
                                                निश्चित(ret == Z_OK);
    mem_limit(&strm, (माप(काष्ठा inflate_state) << 1) + 256);
    ret = inflate_अभाज्य(&strm, 16, 0);           निश्चित(ret == Z_OK);
    strm.avail_in = 2;
    strm.अग्र_in = (व्योम *)"\x80";
    ret = inflateSync(&strm);                   निश्चित(ret == Z_तथ्य_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           निश्चित(ret == Z_STREAM_ERROR);
    strm.avail_in = 4;
    strm.अग्र_in = (व्योम *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   निश्चित(ret == Z_OK);
    (व्योम)inflateSyncPoint(&strm);
    ret = inflateCopy(&copy, &strm);            निश्चित(ret == Z_MEM_ERROR);
    mem_limit(&strm, 0);
    ret = inflateUndermine(&strm, 1);           निश्चित(ret == Z_तथ्य_ERROR);
    (व्योम)inflateMark(&strm);
    ret = inflateEnd(&strm);                    निश्चित(ret == Z_OK);
    mem_done(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local अचिन्हित pull(व्योम *desc, अचिन्हित अक्षर z_const **buf)
{
    अटल अचिन्हित पूर्णांक अग्र = 0;
    अटल अचिन्हित अक्षर dat[] = {0x63, 0, 2, 0};
    काष्ठा inflate_state *state;

    अगर (desc == Z_NULL) {
        अग्र = 0;
        वापस 0;   /* no input (already provided at next_in) */
    }
    state = (व्योम *)((z_stream *)desc)->state;
    अगर (state != Z_NULL)
        state->mode = SYNC;     /* force an otherwise impossible situation */
    वापस अग्र < माप(dat) ? (*buf = dat + अग्र++, 1) : 0;
}

local पूर्णांक धकेलो(व्योम *desc, अचिन्हित अक्षर *buf, अचिन्हित len)
{
    (व्योम)buf;
    (व्योम)len;
    वापस desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local व्योम cover_back(व्योम)
{
    पूर्णांक ret;
    z_stream strm;
    अचिन्हित अक्षर win[32768];

    ret = inflateBackInit_(Z_NULL, 0, win, 0, 0);
                                                निश्चित(ret == Z_VERSION_ERROR);
    ret = inflateBackInit(Z_NULL, 0, win);      निश्चित(ret == Z_STREAM_ERROR);
    ret = inflateBack(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                निश्चित(ret == Z_STREAM_ERROR);
    ret = inflateBackEnd(Z_NULL);               निश्चित(ret == Z_STREAM_ERROR);
    ख_माला_दो("inflateBack bad parameters\n", मानक_त्रुटि);

    mem_setup(&strm);
    ret = inflateBackInit(&strm, 15, win);      निश्चित(ret == Z_OK);
    strm.avail_in = 2;
    strm.अग्र_in = (व्योम *)"\x03";
    ret = inflateBack(&strm, pull, Z_NULL, धकेलो, Z_NULL);
                                                निश्चित(ret == Z_STREAM_END);
        /* force output error */
    strm.avail_in = 3;
    strm.अग्र_in = (व्योम *)"\x63\x00";
    ret = inflateBack(&strm, pull, Z_NULL, धकेलो, &strm);
                                                निश्चित(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflateBack(&strm, pull, &strm, धकेलो, Z_NULL);
                                                निश्चित(ret == Z_STREAM_ERROR);
    ret = inflateBackEnd(&strm);                निश्चित(ret == Z_OK);
    mem_done(&strm, "inflateBack bad state");

    ret = inflateBackInit(&strm, 15, win);      निश्चित(ret == Z_OK);
    ret = inflateBackEnd(&strm);                निश्चित(ret == Z_OK);
    ख_माला_दो("inflateBack built-in memory routines\n", मानक_त्रुटि);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local पूर्णांक try(अक्षर *hex, अक्षर *id, पूर्णांक err)
{
    पूर्णांक ret;
    अचिन्हित len, आकार;
    अचिन्हित अक्षर *in, *out, *win;
    अक्षर *prefix;
    z_stream strm;

    /* convert to hex */
    in = h2b(hex, &len);
    निश्चित(in != शून्य);

    /* allocate work areas */
    आकार = len << 3;
    out = दो_स्मृति(आकार);
    निश्चित(out != शून्य);
    win = दो_स्मृति(32768);
    निश्चित(win != शून्य);
    prefix = दो_स्मृति(म_माप(id) + 6);
    निश्चित(prefix != शून्य);

    /* first with inflate */
    म_नकल(prefix, id);
    म_जोड़ो(prefix, "-late");
    mem_setup(&strm);
    strm.avail_in = 0;
    strm.अग्र_in = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    निश्चित(ret == Z_OK);
    strm.avail_in = len;
    strm.अग्र_in = in;
    करो {
        strm.avail_out = आकार;
        strm.अग्र_out = out;
        ret = inflate(&strm, Z_TREES);
        निश्चित(ret != Z_STREAM_ERROR && ret != Z_MEM_ERROR);
        अगर (ret == Z_तथ्य_ERROR || ret == Z_NEED_DICT)
            अवरोध;
    } जबतक (strm.avail_in || strm.avail_out == 0);
    अगर (err) {
        निश्चित(ret == Z_तथ्य_ERROR);
        निश्चित(म_भेद(id, strm.संदेश) == 0);
    }
    inflateEnd(&strm);
    mem_done(&strm, prefix);

    /* then with inflateBack */
    अगर (err >= 0) {
        म_नकल(prefix, id);
        म_जोड़ो(prefix, "-back");
        mem_setup(&strm);
        ret = inflateBackInit(&strm, 15, win);
        निश्चित(ret == Z_OK);
        strm.avail_in = len;
        strm.अग्र_in = in;
        ret = inflateBack(&strm, pull, Z_NULL, धकेलो, Z_NULL);
        निश्चित(ret != Z_STREAM_ERROR);
        अगर (err) {
            निश्चित(ret == Z_तथ्य_ERROR);
            निश्चित(म_भेद(id, strm.संदेश) == 0);
        }
        inflateBackEnd(&strm);
        mem_done(&strm, prefix);
    }

    /* clean up */
    मुक्त(prefix);
    मुक्त(win);
    मुक्त(out);
    मुक्त(in);
    वापस ret;
}

/* cover deflate data cases in both inflate() and inflateBack() */
local व्योम cover_inflate(व्योम)
{
    try("0 0 0 0 0", "invalid stored block lengths", 1);
    try("3 0", "fixed", 0);
    try("6", "invalid block type", 1);
    try("1 1 0 fe ff 0", "stored", 0);
    try("fc 0 0", "too many length or distance symbols", 1);
    try("4 0 fe ff", "invalid code lengths set", 1);
    try("4 0 24 49 0", "invalid bit length repeat", 1);
    try("4 0 24 e9 ff ff", "invalid bit length repeat", 1);
    try("4 0 24 e9 ff 6d", "invalid code -- missing end-of-block", 1);
    try("4 80 49 92 24 49 92 24 71 ff ff 93 11 0",
        "invalid literal/lengths set", 1);
    try("4 80 49 92 24 49 92 24 f b4 ff ff c3 84", "invalid distances set", 1);
    try("4 c0 81 8 0 0 0 0 20 7f eb b 0 0", "invalid literal/length code", 1);
    try("2 7e ff ff", "invalid distance code", 1);
    try("c c0 81 0 0 0 0 0 90 ff 6b 4 0", "invalid distance too far back", 1);

    /* also trailer mismatch just in inflate() */
    try("1f 8b 8 0 0 0 0 0 0 0 3 0 0 0 0 1", "incorrect data check", -1);
    try("1f 8b 8 0 0 0 0 0 0 0 3 0 0 0 0 0 0 0 0 1",
        "incorrect length check", -1);
    try("5 c0 21 d 0 0 0 80 b0 fe 6d 2f 91 6c", "pull 17", 0);
    try("5 e0 81 91 24 cb b2 2c 49 e2 f 2e 8b 9a 47 56 9f fb fe ec d2 ff 1f",
        "long code", 0);
    try("ed c0 1 1 0 0 0 40 20 ff 57 1b 42 2c 4f", "length extra", 0);
    try("ed cf c1 b1 2c 47 10 c4 30 fa 6f 35 1d 1 82 59 3d fb be 2e 2a fc f c",
        "long distance and extra", 0);
    try("ed c0 81 0 0 0 0 80 a0 fd a9 17 a9 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
        "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6", "window end", 0);
    inf("2 8 20 80 0 3 0", "inflate_fast TYPE return", 0, -15, 258,
        Z_STREAM_END);
    inf("63 18 5 40 c 0", "window wrap", 3, -8, 300, Z_OK);
}

/* cover remaining lines in inftrees.c */
local व्योम cover_trees(व्योम)
{
    पूर्णांक ret;
    अचिन्हित bits;
    अचिन्हित लघु lens[16], work[16];
    code *अग्र, table[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    क्रम (bits = 0; bits < 15; bits++)
        lens[bits] = (अचिन्हित लघु)(bits + 1);
    lens[15] = 15;
    अग्र = table;
    bits = 15;
    ret = inflate_table(DISTS, lens, 16, &अग्र, &bits, work);
                                                निश्चित(ret == 1);
    अग्र = table;
    bits = 1;
    ret = inflate_table(DISTS, lens, 16, &अग्र, &bits, work);
                                                निश्चित(ret == 1);
    ख_माला_दो("inflate_table not enough errors\n", मानक_त्रुटि);
}

/* cover remaining inffast.c decoding and window copying */
local व्योम cover_fast(व्योम)
{
    inf("e5 e0 81 ad 6d cb b2 2c c9 01 1e 59 63 ae 7d ee fb 4d fd b5 35 41 68"
        " ff 7f 0f 0 0 0", "fast length extra bits", 0, -8, 258, Z_तथ्य_ERROR);
    inf("25 fd 81 b5 6d 59 b6 6a 49 ea af 35 6 34 eb 8c b9 f6 b9 1e ef 67 49"
        " 50 fe ff ff 3f 0 0", "fast distance extra bits", 0, -8, 258,
        Z_तथ्य_ERROR);
    inf("3 7e 0 0 0 0 0", "fast invalid distance code", 0, -8, 258,
        Z_तथ्य_ERROR);
    inf("1b 7 0 0 0 0 0", "fast invalid literal/length code", 0, -8, 258,
        Z_तथ्य_ERROR);
    inf("d c7 1 ae eb 38 c 4 41 a0 87 72 de df fb 1f b8 36 b1 38 5d ff ff 0",
        "fast 2nd level codes and too far back", 0, -8, 258, Z_तथ्य_ERROR);
    inf("63 18 5 8c 10 8 0 0 0 0", "very common case", 0, -8, 259, Z_OK);
    inf("63 60 60 18 c9 0 8 18 18 18 26 c0 28 0 29 0 0 0",
        "contiguous and wrap around window", 6, -8, 259, Z_OK);
    inf("63 0 3 0 0 0 0 0", "copy direct from output", 0, -8, 259,
        Z_STREAM_END);
}

पूर्णांक मुख्य(व्योम)
{
    ख_लिखो(मानक_त्रुटि, "%s\n", zlibVersion());
    cover_support();
    cover_wrap();
    cover_back();
    cover_inflate();
    cover_trees();
    cover_fast();
    वापस 0;
}
