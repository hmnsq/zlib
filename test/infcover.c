/* infcover.c -- test zlib's inflate routines with full code coverage
 * Copyright (C) 2011, 2016, 2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* to use, do: ./configure --cover && make cover */

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>
#ضمن "zlib.h"

/* get definition of internal structure so we can mess with it (see pull()),
   and so we can call inflate_trees() (see cover5()) */
#define ZLIB_INTERNAL
#ضمن "inftrees.h"
#ضمن "inflate.h"

#define local ساكن

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
بنية mem_item {
    فراغ *ptr;                  /* pointer to allocated memory */
    size_t size;                /* requested size of allocation */
    بنية mem_item *next;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
بنية mem_zone {
    بنية mem_item *first;     /* pointer to first item in list, or NULL */
    size_t total, highwater;    /* total allocations, and largest total */
    size_t limit;               /* memory allocation limit, or 0 if no limit */
    صحيح notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local فراغ *mem_alloc(فراغ *mem, unsigned تعداد_2, unsigned size)
{
    فراغ *ptr;
    بنية mem_item *item;
    بنية mem_zone *zone = mem;
    size_t len = تعداد_2 * (size_t)size;

    /* induced allocation failure */
    إذا (zone == عدم || (zone->limit && zone->total + len > zone->limit))
        إرجاع عدم;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = malloc(len);
    إذا (ptr == عدم)
        إرجاع عدم;
    memset(ptr, 0xa5, len);

    /* create a new item for the list */
    item = malloc(حجم(بنية mem_item));
    إذا (item == عدم) {
        حرر(ptr);
        إرجاع عدم;
    }
    item->ptr = ptr;
    item->size = len;

    /* insert item at the beginning of the list */
    item->next = zone->first;
    zone->first = item;

    /* update the statistics */
    zone->total += item->size;
    إذا (zone->total > zone->highwater)
        zone->highwater = zone->total;

    /* return the allocated memory */
    إرجاع ptr;
}

/* memory free routine to pass to zlib */
local فراغ mem_free(فراغ *mem, فراغ *ptr)
{
    بنية mem_item *item, *next;
    بنية mem_zone *zone = mem;

    /* if no zone, just do a free */
    إذا (zone == عدم) {
        حرر(ptr);
        إرجاع;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    next = zone->first;
    إذا (next) {
        إذا (next->ptr == ptr)
            zone->first = next->next;   /* first one is it, remove from list */
        وإلا {
            افعل {                        /* search the linked list */
                item = next;
                next = item->next;
            } بينما (next != عدم && next->ptr != ptr);
            إذا (next) {                 /* if found, remove from linked list */
                item->next = next->next;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    إذا (next) {
        zone->total -= next->size;
        حرر(next);
    }

    /* if not found, update the rogue count */
    وإلا
        zone->rogue++;

    /* in any case, do the requested free with the standard library function */
    حرر(ptr);
}

/* set up a controlled memory allocation space for monitoring, set the stream
   parameters to the controlled routines, with opaque pointing to the space */
local فراغ mem_setup(z_stream *strm)
{
    بنية mem_zone *zone;

    zone = malloc(حجم(بنية mem_zone));
    أكد(zone != عدم);
    zone->first = عدم;
    zone->total = 0;
    zone->highwater = 0;
    zone->limit = 0;
    zone->notlifo = 0;
    zone->rogue = 0;
    strm->opaque = zone;
    strm->zalloc = mem_alloc;
    strm->zfree = mem_free;
}

/* set a limit on the total memory allocation, or 0 to remove the limit */
local فراغ mem_limit(z_stream *strm, size_t limit)
{
    بنية mem_zone *zone = strm->opaque;

    zone->limit = limit;
}

/* show the current total requested allocations in bytes */
local فراغ mem_used(z_stream *strm, حرف *prefix)
{
    بنية mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu allocated\n", prefix, zone->total);
}

/* show the high water allocation in bytes */
local فراغ mem_أعلى(z_stream *strm, حرف *prefix)
{
    بنية mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu high water mark\n", prefix, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local فراغ mem_done(z_stream *strm, حرف *prefix)
{
    صحيح تعداد_2 = 0;
    بنية mem_item *item, *next;
    بنية mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_أعلى(strm, prefix);

    /* free leftover allocations and item structures, if any */
    item = zone->first;
    بينما (item != عدم) {
        حرر(item->ptr);
        next = item->next;
        حرر(item);
        item = next;
        تعداد_2++;
    }

    /* issue alerts about anything unexpected */
    إذا (تعداد_2 || zone->total)
        fprintf(stderr, "** %s: %zu bytes in %d blocks not freed\n",
                prefix, zone->total, تعداد_2);
    إذا (zone->notlifo)
        fprintf(stderr, "** %s: %d frees not LIFO\n", prefix, zone->notlifo);
    إذا (zone->rogue)
        fprintf(stderr, "** %s: %d frees not recognized\n",
                prefix, zone->rogue);

    /* free the zone and delete from the stream */
    حرر(zone);
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
local unsigned حرف *h2b(ثابت حرف *hex, unsigned *len)
{
    unsigned حرف *in, *re;
    unsigned next, val;

    in = malloc((strlen(hex) + 1) >> 1);
    إذا (in == عدم)
        إرجاع عدم;
    next = 0;
    val = 1;
    افعل {
        إذا (*hex >= '0' && *hex <= '9')
            val = (val << 4) + *hex - '0';
        وإلا إذا (*hex >= 'A' && *hex <= 'F')
            val = (val << 4) + *hex - 'A' + 10;
        وإلا إذا (*hex >= 'a' && *hex <= 'f')
            val = (val << 4) + *hex - 'a' + 10;
        وإلا إذا (val != 1 && val < 32)  /* one digit followed by delimiter */
            val += 240;                 /* make it look like two digits */
        إذا (val > 255) {                /* have two digits */
            in[next++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } بينما (*hex++);       /* go through the loop with the terminating null */
    إذا (len != عدم)
        *len = next;
    re = realloc(in, next);
    إرجاع re == عدم ? in : re;
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
local فراغ inf(حرف *hex, حرف *what, unsigned step, صحيح win, unsigned len,
               صحيح err)
{
    صحيح ret;
    unsigned have;
    unsigned حرف *in, *out;
    z_stream strm, copy;
    gz_header head;

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, win);
    إذا (ret != Z_OK) {
        mem_done(&strm, what);
        إرجاع;
    }
    out = malloc(len);                          أكد(out != عدم);
    إذا (win == 47) {
        head.extra = out;
        head.extra_max = len;
        head.name = out;
        head.name_max = len;
        head.comment = out;
        head.comm_max = len;
        ret = inflateGetHeader(&strm, &head);   أكد(ret == Z_OK);
    }
    in = h2b(hex, &have);                       أكد(in != عدم);
    إذا (step == 0 || step > have)
        step = have;
    strm.avail_in = step;
    have -= step;
    strm.next_in = in;
    افعل {
        strm.avail_out = len;
        strm.next_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);       أكد(err == 9 || ret == err);
        إذا (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            اكسر;
        إذا (ret == Z_NEED_DICT) {
            ret = inflateSetDictionary(&strm, in, 1);
                                                أكد(ret == Z_DATA_ERROR);
            mem_limit(&strm, 1);
            ret = inflateSetDictionary(&strm, out, 0);
                                                أكد(ret == Z_MEM_ERROR);
            mem_limit(&strm, 0);
            ((بنية inflate_state *)strm.state)->mode = DICT;
            ret = inflateSetDictionary(&strm, out, 0);
                                                أكد(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   أكد(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        أكد(ret == Z_OK);
        ret = inflateEnd(&copy);                أكد(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_in;
        strm.avail_in = step > have ? have : step;
        have -= strm.avail_in;
    } بينما (strm.avail_in);
    حرر(in);
    حرر(out);
    ret = inflateReset2(&strm, -8);             أكد(ret == Z_OK);
    ret = inflateEnd(&strm);                    أكد(ret == Z_OK);
    mem_done(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local فراغ cover_support(فراغ)
{
    صحيح ret;
    z_stream strm;

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);                   أكد(ret == Z_OK);
    mem_used(&strm, "inflate init");
    ret = inflate_أولي(&strm, 5, 31);           أكد(ret == Z_OK);
    ret = inflate_أولي(&strm, -1, 0);           أكد(ret == Z_OK);
    ret = inflateSetDictionary(&strm, Z_NULL, 0);
                                                أكد(ret == Z_STREAM_ERROR);
    ret = inflateEnd(&strm);                    أكد(ret == Z_OK);
    mem_done(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_STREAM_END);
    inf("", "bad window size", 0, 1, 0, Z_STREAM_ERROR);

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit_(&strm, "!", (صحيح)حجم(z_stream));
                                                أكد(ret == Z_VERSION_ERROR);
    mem_done(&strm, "wrong version");

    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);                   أكد(ret == Z_OK);
    ret = inflateEnd(&strm);                    أكد(ret == Z_OK);
    fputs("inflate built-in memory routines\n", stderr);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local فراغ cover_wrap(فراغ)
{
    صحيح ret;
    z_stream strm, copy;
    unsigned حرف dict[257];

    ret = inflate(Z_NULL, 0);                   أكد(ret == Z_STREAM_ERROR);
    ret = inflateEnd(Z_NULL);                   أكد(ret == Z_STREAM_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          أكد(ret == Z_STREAM_ERROR);
    fputs("inflate bad parameters\n", stderr);

    inf("1f 8b 0 0", "bad gzip method", 0, 31, 0, Z_DATA_ERROR);
    inf("1f 8b 8 80", "bad gzip flags", 0, 31, 0, Z_DATA_ERROR);
    inf("77 85", "bad zlib method", 0, 15, 0, Z_DATA_ERROR);
    inf("8 99", "set window size from header", 0, 0, 0, Z_OK);
    inf("78 9c", "bad zlib window size", 0, 8, 0, Z_DATA_ERROR);
    inf("78 9c 63 0 0 0 1 0 1", "check adler32", 0, 15, 1, Z_STREAM_END);
    inf("1f 8b 8 1e 0 0 0 0 0 0 1 0 0 0 0 0 0", "bad header crc", 0, 47, 1,
        Z_DATA_ERROR);
    inf("1f 8b 8 2 0 0 0 0 0 0 1d 26 3 0 0 0 0 0 0 0 0 0", "check gzip length",
        0, 47, 0, Z_STREAM_END);
    inf("78 90", "bad zlib header check", 0, 47, 0, Z_DATA_ERROR);
    inf("8 b8 0 0 0 1", "need dictionary", 0, 8, 0, Z_NEED_DICT);
    inf("78 9c 63 0", "compute adler32", 0, 15, 1, Z_OK);

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -8);
    strm.avail_in = 2;
    strm.next_in = (فراغ *)"\x63";
    strm.avail_out = 1;
    strm.next_out = (فراغ *)&ret;
    mem_limit(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           أكد(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           أكد(ret == Z_MEM_ERROR);
    mem_limit(&strm, 0);
    memset(dict, 0, 257);
    ret = inflateSetDictionary(&strm, dict, 257);
                                                أكد(ret == Z_OK);
    mem_limit(&strm, (حجم(بنية inflate_state) << 1) + 256);
    ret = inflate_أولي(&strm, 16, 0);           أكد(ret == Z_OK);
    strm.avail_in = 2;
    strm.next_in = (فراغ *)"\x80";
    ret = inflateSync(&strm);                   أكد(ret == Z_DATA_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           أكد(ret == Z_STREAM_ERROR);
    strm.avail_in = 4;
    strm.next_in = (فراغ *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   أكد(ret == Z_OK);
    (فراغ)inflateSyncPoint(&strm);
    ret = inflateCopy(&copy, &strm);            أكد(ret == Z_MEM_ERROR);
    mem_limit(&strm, 0);
    ret = inflateUndermine(&strm, 1);           أكد(ret == Z_DATA_ERROR);
    (فراغ)inflateMark(&strm);
    ret = inflateEnd(&strm);                    أكد(ret == Z_OK);
    mem_done(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local unsigned pull(فراغ *desc, unsigned حرف z_const **buf)
{
    ساكن unsigned صحيح next = 0;
    ساكن unsigned حرف dat[] = {0x63, 0, 2, 0};
    بنية inflate_state *state;

    إذا (desc == Z_NULL) {
        next = 0;
        إرجاع 0;   /* no input (already provided at next_in) */
    }
    state = (فراغ *)((z_stream *)desc)->state;
    إذا (state != Z_NULL)
        state->mode = SYNC;     /* force an otherwise impossible situation */
    إرجاع next < حجم(dat) ? (*buf = dat + next++, 1) : 0;
}

local صحيح ادفع(فراغ *desc, unsigned حرف *buf, unsigned len)
{
    (فراغ)buf;
    (فراغ)len;
    إرجاع desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local فراغ cover_back(فراغ)
{
    صحيح ret;
    z_stream strm;
    unsigned حرف win[32768];

    ret = inflateBackInit_(Z_NULL, 0, win, 0, 0);
                                                أكد(ret == Z_VERSION_ERROR);
    ret = inflateBackInit(Z_NULL, 0, win);      أكد(ret == Z_STREAM_ERROR);
    ret = inflateBack(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                أكد(ret == Z_STREAM_ERROR);
    ret = inflateBackEnd(Z_NULL);               أكد(ret == Z_STREAM_ERROR);
    fputs("inflateBack bad parameters\n", stderr);

    mem_setup(&strm);
    ret = inflateBackInit(&strm, 15, win);      أكد(ret == Z_OK);
    strm.avail_in = 2;
    strm.next_in = (فراغ *)"\x03";
    ret = inflateBack(&strm, pull, Z_NULL, ادفع, Z_NULL);
                                                أكد(ret == Z_STREAM_END);
        /* force output error */
    strm.avail_in = 3;
    strm.next_in = (فراغ *)"\x63\x00";
    ret = inflateBack(&strm, pull, Z_NULL, ادفع, &strm);
                                                أكد(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflateBack(&strm, pull, &strm, ادفع, Z_NULL);
                                                أكد(ret == Z_STREAM_ERROR);
    ret = inflateBackEnd(&strm);                أكد(ret == Z_OK);
    mem_done(&strm, "inflateBack bad state");

    ret = inflateBackInit(&strm, 15, win);      أكد(ret == Z_OK);
    ret = inflateBackEnd(&strm);                أكد(ret == Z_OK);
    fputs("inflateBack built-in memory routines\n", stderr);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local صحيح try(حرف *hex, حرف *id, صحيح err)
{
    صحيح ret;
    unsigned len, size;
    unsigned حرف *in, *out, *win;
    حرف *prefix;
    z_stream strm;

    /* convert to hex */
    in = h2b(hex, &len);
    أكد(in != عدم);

    /* allocate work areas */
    size = len << 3;
    out = malloc(size);
    أكد(out != عدم);
    win = malloc(32768);
    أكد(win != عدم);
    prefix = malloc(strlen(id) + 6);
    أكد(prefix != عدم);

    /* first with inflate */
    strcpy(prefix, id);
    strcat(prefix, "-late");
    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    أكد(ret == Z_OK);
    strm.avail_in = len;
    strm.next_in = in;
    افعل {
        strm.avail_out = size;
        strm.next_out = out;
        ret = inflate(&strm, Z_TREES);
        أكد(ret != Z_STREAM_ERROR && ret != Z_MEM_ERROR);
        إذا (ret == Z_DATA_ERROR || ret == Z_NEED_DICT)
            اكسر;
    } بينما (strm.avail_in || strm.avail_out == 0);
    إذا (err) {
        أكد(ret == Z_DATA_ERROR);
        أكد(strcmp(id, strm.رسالة) == 0);
    }
    inflateEnd(&strm);
    mem_done(&strm, prefix);

    /* then with inflateBack */
    إذا (err >= 0) {
        strcpy(prefix, id);
        strcat(prefix, "-back");
        mem_setup(&strm);
        ret = inflateBackInit(&strm, 15, win);
        أكد(ret == Z_OK);
        strm.avail_in = len;
        strm.next_in = in;
        ret = inflateBack(&strm, pull, Z_NULL, ادفع, Z_NULL);
        أكد(ret != Z_STREAM_ERROR);
        إذا (err) {
            أكد(ret == Z_DATA_ERROR);
            أكد(strcmp(id, strm.رسالة) == 0);
        }
        inflateBackEnd(&strm);
        mem_done(&strm, prefix);
    }

    /* clean up */
    حرر(prefix);
    حرر(win);
    حرر(out);
    حرر(in);
    إرجاع ret;
}

/* cover deflate data cases in both inflate() and inflateBack() */
local فراغ cover_inflate(فراغ)
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
local فراغ cover_trees(فراغ)
{
    صحيح ret;
    unsigned bits;
    unsigned short lens[16], work[16];
    code *next, table[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    لكل (bits = 0; bits < 15; bits++)
        lens[bits] = (unsigned short)(bits + 1);
    lens[15] = 15;
    next = table;
    bits = 15;
    ret = inflate_table(DISTS, lens, 16, &next, &bits, work);
                                                أكد(ret == 1);
    next = table;
    bits = 1;
    ret = inflate_table(DISTS, lens, 16, &next, &bits, work);
                                                أكد(ret == 1);
    fputs("inflate_table not enough errors\n", stderr);
}

/* cover remaining inffast.c decoding and window copying */
local فراغ cover_fast(فراغ)
{
    inf("e5 e0 81 ad 6d cb b2 2c c9 01 1e 59 63 ae 7d ee fb 4d fd b5 35 41 68"
        " ff 7f 0f 0 0 0", "fast length extra bits", 0, -8, 258, Z_DATA_ERROR);
    inf("25 fd 81 b5 6d 59 b6 6a 49 ea af 35 6 34 eb 8c b9 f6 b9 1e ef 67 49"
        " 50 fe ff ff 3f 0 0", "fast distance extra bits", 0, -8, 258,
        Z_DATA_ERROR);
    inf("3 7e 0 0 0 0 0", "fast invalid distance code", 0, -8, 258,
        Z_DATA_ERROR);
    inf("1b 7 0 0 0 0 0", "fast invalid literal/length code", 0, -8, 258,
        Z_DATA_ERROR);
    inf("d c7 1 ae eb 38 c 4 41 a0 87 72 de df fb 1f b8 36 b1 38 5d ff ff 0",
        "fast 2nd level codes and too far back", 0, -8, 258, Z_DATA_ERROR);
    inf("63 18 5 8c 10 8 0 0 0 0", "very common case", 0, -8, 259, Z_OK);
    inf("63 60 60 18 c9 0 8 18 18 18 26 c0 28 0 29 0 0 0",
        "contiguous and wrap around window", 6, -8, 259, Z_OK);
    inf("63 0 3 0 0 0 0 0", "copy direct from output", 0, -8, 259,
        Z_STREAM_END);
}

صحيح الرئيسية(فراغ)
{
    fprintf(stderr, "%s\n", zlibVersion());
    cover_support();
    cover_wrap();
    cover_back();
    cover_inflate();
    cover_trees();
    cover_fast();
    إرجاع 0;
}
