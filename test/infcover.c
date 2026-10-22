/* infcover.c -- test zlib's inflate routines with full code coverage
 * Copyright (C) 2011, 2016, 2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* to use, do: ./configure --cover && make cover */

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ "zlib.h"

/* get definition of internal structure so we can mess with it (see pull()),
   and so we can call inflate_trees() (see cover5()) */
#define ZLIB_INTERNAL
#അന്തര്ഭാവ "inftrees.h"
#അന്തര്ഭാവ "inflate.h"

#define local സ്ഥിതം

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
ഘടന mem_item {
    ശൂന്യം *ptr;                  /* pointer to allocated memory */
    size_t size;                /* requested size of allocation */
    ഘടന mem_item *next;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
ഘടന mem_zone {
    ഘടന mem_item *first;     /* pointer to first item in list, or NULL */
    size_t total, highwater;    /* total allocations, and largest total */
    size_t limit;               /* memory allocation limit, or 0 if no limit */
    പൂർണ്ണസംഖ്യ notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local ശൂന്യം *mem_alloc(ശൂന്യം *mem, unsigned എണ്ണം, unsigned size)
{
    ശൂന്യം *ptr;
    ഘടന mem_item *item;
    ഘടന mem_zone *zone = mem;
    size_t len = എണ്ണം * (size_t)size;

    /* induced allocation failure */
    എങ്കിൽ (zone == ശൂന്യ || (zone->limit && zone->total + len > zone->limit))
        തിരികെ ശൂന്യ;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = malloc(len);
    എങ്കിൽ (ptr == ശൂന്യ)
        തിരികെ ശൂന്യ;
    memset(ptr, 0xa5, len);

    /* create a new item for the list */
    item = malloc(വലുപ്പം(ഘടന mem_item));
    എങ്കിൽ (item == ശൂന്യ) {
        സ്വതന്ത്രം(ptr);
        തിരികെ ശൂന്യ;
    }
    item->ptr = ptr;
    item->size = len;

    /* insert item at the beginning of the list */
    item->next = zone->first;
    zone->first = item;

    /* update the statistics */
    zone->total += item->size;
    എങ്കിൽ (zone->total > zone->highwater)
        zone->highwater = zone->total;

    /* return the allocated memory */
    തിരികെ ptr;
}

/* memory free routine to pass to zlib */
local ശൂന്യം mem_free(ശൂന്യം *mem, ശൂന്യം *ptr)
{
    ഘടന mem_item *item, *next;
    ഘടന mem_zone *zone = mem;

    /* if no zone, just do a free */
    എങ്കിൽ (zone == ശൂന്യ) {
        സ്വതന്ത്രം(ptr);
        തിരികെ;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    next = zone->first;
    എങ്കിൽ (next) {
        എങ്കിൽ (next->ptr == ptr)
            zone->first = next->next;   /* first one is it, remove from list */
        അല്ലെങ്കിൽ {
            ചെയ്യുക {                        /* search the linked list */
                item = next;
                next = item->next;
            } ആകുമ്പോൾ (next != ശൂന്യ && next->ptr != ptr);
            എങ്കിൽ (next) {                 /* if found, remove from linked list */
                item->next = next->next;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    എങ്കിൽ (next) {
        zone->total -= next->size;
        സ്വതന്ത്രം(next);
    }

    /* if not found, update the rogue count */
    അല്ലെങ്കിൽ
        zone->rogue++;

    /* in any case, do the requested free with the standard library function */
    സ്വതന്ത്രം(ptr);
}

/* set up a controlled memory allocation space for monitoring, set the stream
   parameters to the controlled routines, with opaque pointing to the space */
local ശൂന്യം mem_setup(z_stream *strm)
{
    ഘടന mem_zone *zone;

    zone = malloc(വലുപ്പം(ഘടന mem_zone));
    പ്രതിജ്ഞാ(zone != ശൂന്യ);
    zone->first = ശൂന്യ;
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
local ശൂന്യം mem_limit(z_stream *strm, size_t limit)
{
    ഘടന mem_zone *zone = strm->opaque;

    zone->limit = limit;
}

/* show the current total requested allocations in bytes */
local ശൂന്യം mem_used(z_stream *strm, അക്ഷരം *prefix)
{
    ഘടന mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu allocated\n", prefix, zone->total);
}

/* show the high water allocation in bytes */
local ശൂന്യം mem_മുകളിലെ(z_stream *strm, അക്ഷരം *prefix)
{
    ഘടന mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu high water mark\n", prefix, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local ശൂന്യം mem_done(z_stream *strm, അക്ഷരം *prefix)
{
    പൂർണ്ണസംഖ്യ എണ്ണം = 0;
    ഘടന mem_item *item, *next;
    ഘടന mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_മുകളിലെ(strm, prefix);

    /* free leftover allocations and item structures, if any */
    item = zone->first;
    ആകുമ്പോൾ (item != ശൂന്യ) {
        സ്വതന്ത്രം(item->ptr);
        next = item->next;
        സ്വതന്ത്രം(item);
        item = next;
        എണ്ണം++;
    }

    /* issue alerts about anything unexpected */
    എങ്കിൽ (എണ്ണം || zone->total)
        fprintf(stderr, "** %s: %zu bytes in %d blocks not freed\n",
                prefix, zone->total, എണ്ണം);
    എങ്കിൽ (zone->notlifo)
        fprintf(stderr, "** %s: %d frees not LIFO\n", prefix, zone->notlifo);
    എങ്കിൽ (zone->rogue)
        fprintf(stderr, "** %s: %d frees not recognized\n",
                prefix, zone->rogue);

    /* free the zone and delete from the stream */
    സ്വതന്ത്രം(zone);
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
local unsigned അക്ഷരം *h2b(സ്ഥിരം അക്ഷരം *hex, unsigned *len)
{
    unsigned അക്ഷരം *in, *re;
    unsigned next, val;

    in = malloc((strlen(hex) + 1) >> 1);
    എങ്കിൽ (in == ശൂന്യ)
        തിരികെ ശൂന്യ;
    next = 0;
    val = 1;
    ചെയ്യുക {
        എങ്കിൽ (*hex >= '0' && *hex <= '9')
            val = (val << 4) + *hex - '0';
        അല്ലെങ്കിൽ എങ്കിൽ (*hex >= 'A' && *hex <= 'F')
            val = (val << 4) + *hex - 'A' + 10;
        അല്ലെങ്കിൽ എങ്കിൽ (*hex >= 'a' && *hex <= 'f')
            val = (val << 4) + *hex - 'a' + 10;
        അല്ലെങ്കിൽ എങ്കിൽ (val != 1 && val < 32)  /* one digit followed by delimiter */
            val += 240;                 /* make it look like two digits */
        എങ്കിൽ (val > 255) {                /* have two digits */
            in[next++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } ആകുമ്പോൾ (*hex++);       /* go through the loop with the terminating null */
    എങ്കിൽ (len != ശൂന്യ)
        *len = next;
    re = realloc(in, next);
    തിരികെ re == ശൂന്യ ? in : re;
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
local ശൂന്യം inf(അക്ഷരം *hex, അക്ഷരം *what, unsigned step, പൂർണ്ണസംഖ്യ win, unsigned len,
               പൂർണ്ണസംഖ്യ err)
{
    പൂർണ്ണസംഖ്യ ret;
    unsigned have;
    unsigned അക്ഷരം *in, *out;
    z_stream strm, copy;
    gz_header head;

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, win);
    എങ്കിൽ (ret != Z_OK) {
        mem_done(&strm, what);
        തിരികെ;
    }
    out = malloc(len);                          പ്രതിജ്ഞാ(out != ശൂന്യ);
    എങ്കിൽ (win == 47) {
        head.extra = out;
        head.extra_max = len;
        head.name = out;
        head.name_max = len;
        head.comment = out;
        head.comm_max = len;
        ret = inflateGetHeader(&strm, &head);   പ്രതിജ്ഞാ(ret == Z_OK);
    }
    in = h2b(hex, &have);                       പ്രതിജ്ഞാ(in != ശൂന്യ);
    എങ്കിൽ (step == 0 || step > have)
        step = have;
    strm.avail_in = step;
    have -= step;
    strm.next_in = in;
    ചെയ്യുക {
        strm.avail_out = len;
        strm.next_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);       പ്രതിജ്ഞാ(err == 9 || ret == err);
        എങ്കിൽ (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            നിർത്തുക;
        എങ്കിൽ (ret == Z_NEED_DICT) {
            ret = inflateSetDictionary(&strm, in, 1);
                                                പ്രതിജ്ഞാ(ret == Z_DATA_ERROR);
            mem_limit(&strm, 1);
            ret = inflateSetDictionary(&strm, out, 0);
                                                പ്രതിജ്ഞാ(ret == Z_MEM_ERROR);
            mem_limit(&strm, 0);
            ((ഘടന inflate_state *)strm.state)->mode = DICT;
            ret = inflateSetDictionary(&strm, out, 0);
                                                പ്രതിജ്ഞാ(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   പ്രതിജ്ഞാ(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        പ്രതിജ്ഞാ(ret == Z_OK);
        ret = inflateEnd(&copy);                പ്രതിജ്ഞാ(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_in;
        strm.avail_in = step > have ? have : step;
        have -= strm.avail_in;
    } ആകുമ്പോൾ (strm.avail_in);
    സ്വതന്ത്രം(in);
    സ്വതന്ത്രം(out);
    ret = inflateReset2(&strm, -8);             പ്രതിജ്ഞാ(ret == Z_OK);
    ret = inflateEnd(&strm);                    പ്രതിജ്ഞാ(ret == Z_OK);
    mem_done(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local ശൂന്യം cover_support(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ ret;
    z_stream strm;

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);                   പ്രതിജ്ഞാ(ret == Z_OK);
    mem_used(&strm, "inflate init");
    ret = inflate_അഭാജ്യം(&strm, 5, 31);           പ്രതിജ്ഞാ(ret == Z_OK);
    ret = inflate_അഭാജ്യം(&strm, -1, 0);           പ്രതിജ്ഞാ(ret == Z_OK);
    ret = inflateSetDictionary(&strm, Z_NULL, 0);
                                                പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    ret = inflateEnd(&strm);                    പ്രതിജ്ഞാ(ret == Z_OK);
    mem_done(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_STREAM_END);
    inf("", "bad window size", 0, 1, 0, Z_STREAM_ERROR);

    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit_(&strm, "!", (പൂർണ്ണസംഖ്യ)വലുപ്പം(z_stream));
                                                പ്രതിജ്ഞാ(ret == Z_VERSION_ERROR);
    mem_done(&strm, "wrong version");

    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);                   പ്രതിജ്ഞാ(ret == Z_OK);
    ret = inflateEnd(&strm);                    പ്രതിജ്ഞാ(ret == Z_OK);
    fputs("inflate built-in memory routines\n", stderr);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local ശൂന്യം cover_wrap(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ ret;
    z_stream strm, copy;
    unsigned അക്ഷരം dict[257];

    ret = inflate(Z_NULL, 0);                   പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    ret = inflateEnd(Z_NULL);                   പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
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
    strm.next_in = (ശൂന്യം *)"\x63";
    strm.avail_out = 1;
    strm.next_out = (ശൂന്യം *)&ret;
    mem_limit(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           പ്രതിജ്ഞാ(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           പ്രതിജ്ഞാ(ret == Z_MEM_ERROR);
    mem_limit(&strm, 0);
    memset(dict, 0, 257);
    ret = inflateSetDictionary(&strm, dict, 257);
                                                പ്രതിജ്ഞാ(ret == Z_OK);
    mem_limit(&strm, (വലുപ്പം(ഘടന inflate_state) << 1) + 256);
    ret = inflate_അഭാജ്യം(&strm, 16, 0);           പ്രതിജ്ഞാ(ret == Z_OK);
    strm.avail_in = 2;
    strm.next_in = (ശൂന്യം *)"\x80";
    ret = inflateSync(&strm);                   പ്രതിജ്ഞാ(ret == Z_DATA_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    strm.avail_in = 4;
    strm.next_in = (ശൂന്യം *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   പ്രതിജ്ഞാ(ret == Z_OK);
    (ശൂന്യം)inflateSyncPoint(&strm);
    ret = inflateCopy(&copy, &strm);            പ്രതിജ്ഞാ(ret == Z_MEM_ERROR);
    mem_limit(&strm, 0);
    ret = inflateUndermine(&strm, 1);           പ്രതിജ്ഞാ(ret == Z_DATA_ERROR);
    (ശൂന്യം)inflateMark(&strm);
    ret = inflateEnd(&strm);                    പ്രതിജ്ഞാ(ret == Z_OK);
    mem_done(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local unsigned pull(ശൂന്യം *desc, unsigned അക്ഷരം z_const **buf)
{
    സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ next = 0;
    സ്ഥിതം unsigned അക്ഷരം dat[] = {0x63, 0, 2, 0};
    ഘടന inflate_state *state;

    എങ്കിൽ (desc == Z_NULL) {
        next = 0;
        തിരികെ 0;   /* no input (already provided at next_in) */
    }
    state = (ശൂന്യം *)((z_stream *)desc)->state;
    എങ്കിൽ (state != Z_NULL)
        state->mode = SYNC;     /* force an otherwise impossible situation */
    തിരികെ next < വലുപ്പം(dat) ? (*buf = dat + next++, 1) : 0;
}

local പൂർണ്ണസംഖ്യ തള്ളുക(ശൂന്യം *desc, unsigned അക്ഷരം *buf, unsigned len)
{
    (ശൂന്യം)buf;
    (ശൂന്യം)len;
    തിരികെ desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local ശൂന്യം cover_back(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ ret;
    z_stream strm;
    unsigned അക്ഷരം win[32768];

    ret = inflateBackInit_(Z_NULL, 0, win, 0, 0);
                                                പ്രതിജ്ഞാ(ret == Z_VERSION_ERROR);
    ret = inflateBackInit(Z_NULL, 0, win);      പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    ret = inflateBack(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    ret = inflateBackEnd(Z_NULL);               പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    fputs("inflateBack bad parameters\n", stderr);

    mem_setup(&strm);
    ret = inflateBackInit(&strm, 15, win);      പ്രതിജ്ഞാ(ret == Z_OK);
    strm.avail_in = 2;
    strm.next_in = (ശൂന്യം *)"\x03";
    ret = inflateBack(&strm, pull, Z_NULL, തള്ളുക, Z_NULL);
                                                പ്രതിജ്ഞാ(ret == Z_STREAM_END);
        /* force output error */
    strm.avail_in = 3;
    strm.next_in = (ശൂന്യം *)"\x63\x00";
    ret = inflateBack(&strm, pull, Z_NULL, തള്ളുക, &strm);
                                                പ്രതിജ്ഞാ(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflateBack(&strm, pull, &strm, തള്ളുക, Z_NULL);
                                                പ്രതിജ്ഞാ(ret == Z_STREAM_ERROR);
    ret = inflateBackEnd(&strm);                പ്രതിജ്ഞാ(ret == Z_OK);
    mem_done(&strm, "inflateBack bad state");

    ret = inflateBackInit(&strm, 15, win);      പ്രതിജ്ഞാ(ret == Z_OK);
    ret = inflateBackEnd(&strm);                പ്രതിജ്ഞാ(ret == Z_OK);
    fputs("inflateBack built-in memory routines\n", stderr);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local പൂർണ്ണസംഖ്യ try(അക്ഷരം *hex, അക്ഷരം *id, പൂർണ്ണസംഖ്യ err)
{
    പൂർണ്ണസംഖ്യ ret;
    unsigned len, size;
    unsigned അക്ഷരം *in, *out, *win;
    അക്ഷരം *prefix;
    z_stream strm;

    /* convert to hex */
    in = h2b(hex, &len);
    പ്രതിജ്ഞാ(in != ശൂന്യ);

    /* allocate work areas */
    size = len << 3;
    out = malloc(size);
    പ്രതിജ്ഞാ(out != ശൂന്യ);
    win = malloc(32768);
    പ്രതിജ്ഞാ(win != ശൂന്യ);
    prefix = malloc(strlen(id) + 6);
    പ്രതിജ്ഞാ(prefix != ശൂന്യ);

    /* first with inflate */
    strcpy(prefix, id);
    strcat(prefix, "-late");
    mem_setup(&strm);
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    പ്രതിജ്ഞാ(ret == Z_OK);
    strm.avail_in = len;
    strm.next_in = in;
    ചെയ്യുക {
        strm.avail_out = size;
        strm.next_out = out;
        ret = inflate(&strm, Z_TREES);
        പ്രതിജ്ഞാ(ret != Z_STREAM_ERROR && ret != Z_MEM_ERROR);
        എങ്കിൽ (ret == Z_DATA_ERROR || ret == Z_NEED_DICT)
            നിർത്തുക;
    } ആകുമ്പോൾ (strm.avail_in || strm.avail_out == 0);
    എങ്കിൽ (err) {
        പ്രതിജ്ഞാ(ret == Z_DATA_ERROR);
        പ്രതിജ്ഞാ(strcmp(id, strm.സന്ദേശം) == 0);
    }
    inflateEnd(&strm);
    mem_done(&strm, prefix);

    /* then with inflateBack */
    എങ്കിൽ (err >= 0) {
        strcpy(prefix, id);
        strcat(prefix, "-back");
        mem_setup(&strm);
        ret = inflateBackInit(&strm, 15, win);
        പ്രതിജ്ഞാ(ret == Z_OK);
        strm.avail_in = len;
        strm.next_in = in;
        ret = inflateBack(&strm, pull, Z_NULL, തള്ളുക, Z_NULL);
        പ്രതിജ്ഞാ(ret != Z_STREAM_ERROR);
        എങ്കിൽ (err) {
            പ്രതിജ്ഞാ(ret == Z_DATA_ERROR);
            പ്രതിജ്ഞാ(strcmp(id, strm.സന്ദേശം) == 0);
        }
        inflateBackEnd(&strm);
        mem_done(&strm, prefix);
    }

    /* clean up */
    സ്വതന്ത്രം(prefix);
    സ്വതന്ത്രം(win);
    സ്വതന്ത്രം(out);
    സ്വതന്ത്രം(in);
    തിരികെ ret;
}

/* cover deflate data cases in both inflate() and inflateBack() */
local ശൂന്യം cover_inflate(ശൂന്യം)
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
local ശൂന്യം cover_trees(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ ret;
    unsigned bits;
    unsigned short lens[16], work[16];
    code *next, table[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    വേണ്ടി (bits = 0; bits < 15; bits++)
        lens[bits] = (unsigned short)(bits + 1);
    lens[15] = 15;
    next = table;
    bits = 15;
    ret = inflate_table(DISTS, lens, 16, &next, &bits, work);
                                                പ്രതിജ്ഞാ(ret == 1);
    next = table;
    bits = 1;
    ret = inflate_table(DISTS, lens, 16, &next, &bits, work);
                                                പ്രതിജ്ഞാ(ret == 1);
    fputs("inflate_table not enough errors\n", stderr);
}

/* cover remaining inffast.c decoding and window copying */
local ശൂന്യം cover_fast(ശൂന്യം)
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

പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം)
{
    fprintf(stderr, "%s\n", zlibVersion());
    cover_support();
    cover_wrap();
    cover_back();
    cover_inflate();
    cover_trees();
    cover_fast();
    തിരികെ 0;
}
