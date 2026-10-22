/* infcover.c -- test zlib's inflate routines with full code coverage
 * Copyright (C) 2011, 2016, 2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* to use, do: ./configure --cover && make cover */

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ "zlib.h"

/* get definition of internal structure so we can mess with it (see pull()),
   and so we can call inflate_trees() (see cover5()) */
#నిర్వచించు ZLIB_అంతర్గత
#అన్తర్భావ "inftrees.h"
#అన్తర్భావ "inflate.h"

#నిర్వచించు local స్థిర

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
నిర్మాణం mem_అంశం {
    శూన్య *ptr;                  /* pointer to allocated memory */
    size_t కొలత;                /* requested size of allocation */
    నిర్మాణం mem_అంశం *తదుపరి;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
నిర్మాణం mem_zone {
    నిర్మాణం mem_అంశం *మొదటి;     /* pointer to first item in list, or NULL */
    size_t మొత్తం, highwater;    /* total allocations, and largest total */
    size_t హద్దు;               /* memory allocation limit, or 0 if no limit */
    పూర్ణాంకం notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local శూన్య *mem_alloc(శూన్య *mem, గుర్తులేని లెక్క, గుర్తులేని కొలత)
{
    శూన్య *ptr;
    నిర్మాణం mem_అంశం *అంశం;
    నిర్మాణం mem_zone *zone = mem;
    size_t len = లెక్క * (size_t)కొలత;

    /* induced allocation failure */
    ఒకవేళ (zone == ఖాళీ || (zone->హద్దు && zone->మొత్తం + len > zone->హద్దు))
        తిరిగి ఖాళీ;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = జ్ఞాపకం_ఇవ్వు(len);
    ఒకవేళ (ptr == ఖాళీ)
        తిరిగి ఖాళీ;
    memset(ptr, 0xa5, len);

    /* create a new item for the list */
    అంశం = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం mem_అంశం));
    ఒకవేళ (అంశం == ఖాళీ) {
        విడుదల(ptr);
        తిరిగి ఖాళీ;
    }
    అంశం->ptr = ptr;
    అంశం->కొలత = len;

    /* insert item at the beginning of the list */
    అంశం->తదుపరి = zone->మొదటి;
    zone->మొదటి = అంశం;

    /* update the statistics */
    zone->మొత్తం += అంశం->కొలత;
    ఒకవేళ (zone->మొత్తం > zone->highwater)
        zone->highwater = zone->మొత్తం;

    /* return the allocated memory */
    తిరిగి ptr;
}

/* memory free routine to pass to zlib */
local శూన్య mem_free(శూన్య *mem, శూన్య *ptr)
{
    నిర్మాణం mem_అంశం *అంశం, *తదుపరి;
    నిర్మాణం mem_zone *zone = mem;

    /* if no zone, just do a free */
    ఒకవేళ (zone == ఖాళీ) {
        విడుదల(ptr);
        తిరిగి;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    తదుపరి = zone->మొదటి;
    ఒకవేళ (తదుపరి) {
        ఒకవేళ (తదుపరి->ptr == ptr)
            zone->మొదటి = తదుపరి->తదుపరి;   /* first one is it, remove from list */
        లేకపోతే {
            చేయి {                        /* search the linked list */
                అంశం = తదుపరి;
                తదుపరి = అంశం->తదుపరి;
            } అయితే (తదుపరి != ఖాళీ && తదుపరి->ptr != ptr);
            ఒకవేళ (తదుపరి) {                 /* if found, remove from linked list */
                అంశం->తదుపరి = తదుపరి->తదుపరి;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    ఒకవేళ (తదుపరి) {
        zone->మొత్తం -= తదుపరి->కొలత;
        విడుదల(తదుపరి);
    }

    /* if not found, update the rogue count */
    లేకపోతే
        zone->rogue++;

    /* in any case, do the requested free with the standard library function */
    విడుదల(ptr);
}

/* set up a controlled memory allocation space for monitoring, set the stream
   parameters to the controlled routines, with opaque pointing to the space */
local శూన్య mem_అమరిక(z_ప్రవాహం *strm)
{
    నిర్మాణం mem_zone *zone;

    zone = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం mem_zone));
    ప్రతిజ్ఞా(zone != ఖాళీ);
    zone->మొదటి = ఖాళీ;
    zone->మొత్తం = 0;
    zone->highwater = 0;
    zone->హద్దు = 0;
    zone->notlifo = 0;
    zone->rogue = 0;
    strm->opaque = zone;
    strm->zalloc = mem_alloc;
    strm->zfree = mem_free;
}

/* set a limit on the total memory allocation, or 0 to remove the limit */
local శూన్య mem_హద్దు(z_ప్రవాహం *strm, size_t హద్దు)
{
    నిర్మాణం mem_zone *zone = strm->opaque;

    zone->హద్దు = హద్దు;
}

/* show the current total requested allocations in bytes */
local శూన్య mem_వాడిన(z_ప్రవాహం *strm, అక్షరం *ఉపసర్గ)
{
    నిర్మాణం mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu allocated\n", ఉపసర్గ, zone->మొత్తం);
}

/* show the high water allocation in bytes */
local శూన్య mem_ఎగువ(z_ప్రవాహం *strm, అక్షరం *ఉపసర్గ)
{
    నిర్మాణం mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu high water mark\n", ఉపసర్గ, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local శూన్య mem_పూర్తి(z_ప్రవాహం *strm, అక్షరం *ఉపసర్గ)
{
    పూర్ణాంకం లెక్క = 0;
    నిర్మాణం mem_అంశం *అంశం, *తదుపరి;
    నిర్మాణం mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_ఎగువ(strm, ఉపసర్గ);

    /* free leftover allocations and item structures, if any */
    అంశం = zone->మొదటి;
    అయితే (అంశం != ఖాళీ) {
        విడుదల(అంశం->ptr);
        తదుపరి = అంశం->తదుపరి;
        విడుదల(అంశం);
        అంశం = తదుపరి;
        లెక్క++;
    }

    /* issue alerts about anything unexpected */
    ఒకవేళ (లెక్క || zone->మొత్తం)
        fprintf(stderr, "** %s: %zu bytes in %d blocks not freed\n",
                ఉపసర్గ, zone->మొత్తం, లెక్క);
    ఒకవేళ (zone->notlifo)
        fprintf(stderr, "** %s: %d frees not LIFO\n", ఉపసర్గ, zone->notlifo);
    ఒకవేళ (zone->rogue)
        fprintf(stderr, "** %s: %d frees not recognized\n",
                ఉపసర్గ, zone->rogue);

    /* free the zone and delete from the stream */
    విడుదల(zone);
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
local గుర్తులేని అక్షరం *h2b(స్థిరరాశి అక్షరం *hex, గుర్తులేని *len)
{
    గుర్తులేని అక్షరం *లోపల, *re;
    గుర్తులేని తదుపరి, val;

    లోపల = జ్ఞాపకం_ఇవ్వు((అక్షరశ్రేణి_పొడవు(hex) + 1) >> 1);
    ఒకవేళ (లోపల == ఖాళీ)
        తిరిగి ఖాళీ;
    తదుపరి = 0;
    val = 1;
    చేయి {
        ఒకవేళ (*hex >= '0' && *hex <= '9')
            val = (val << 4) + *hex - '0';
        లేకపోతే ఒకవేళ (*hex >= 'A' && *hex <= 'F')
            val = (val << 4) + *hex - 'A' + 10;
        లేకపోతే ఒకవేళ (*hex >= 'a' && *hex <= 'f')
            val = (val << 4) + *hex - 'a' + 10;
        లేకపోతే ఒకవేళ (val != 1 && val < 32)  /* one digit followed by delimiter */
            val += 240;                 /* make it look like two digits */
        ఒకవేళ (val > 255) {                /* have two digits */
            లోపల[తదుపరి++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } అయితే (*hex++);       /* go through the loop with the terminating null */
    ఒకవేళ (len != ఖాళీ)
        *len = తదుపరి;
    re = realloc(లోపల, తదుపరి);
    తిరిగి re == ఖాళీ ? లోపల : re;
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
local శూన్య inf(అక్షరం *hex, అక్షరం *what, గుర్తులేని అడుగు, పూర్ణాంకం win, గుర్తులేని len,
               పూర్ణాంకం err)
{
    పూర్ణాంకం ret;
    గుర్తులేని have;
    గుర్తులేని అక్షరం *లోపల, *బహిర్గతం;
    z_ప్రవాహం strm, copy;
    gz_శీర్షిక తల;

    mem_అమరిక(&strm);
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit2(&strm, win);
    ఒకవేళ (ret != Z_OK) {
        mem_పూర్తి(&strm, what);
        తిరిగి;
    }
    బహిర్గతం = జ్ఞాపకం_ఇవ్వు(len);                          ప్రతిజ్ఞా(బహిర్గతం != ఖాళీ);
    ఒకవేళ (win == 47) {
        తల.extra = బహిర్గతం;
        తల.extra_max = len;
        తల.name = బహిర్గతం;
        తల.name_max = len;
        తల.comment = బహిర్గతం;
        తల.comm_max = len;
        ret = inflate_Get_శీర్షిక(&strm, &తల);   ప్రతిజ్ఞా(ret == Z_OK);
    }
    లోపల = h2b(hex, &have);                       ప్రతిజ్ఞా(లోపల != ఖాళీ);
    ఒకవేళ (అడుగు == 0 || అడుగు > have)
        అడుగు = have;
    strm.avail_లోపల = అడుగు;
    have -= అడుగు;
    strm.తదుపరి_లోపల = లోపల;
    చేయి {
        strm.avail_బహిర్గతం = len;
        strm.తదుపరి_బహిర్గతం = బహిర్గతం;
        ret = inflate(&strm, Z_NO_FLUSH);       ప్రతిజ్ఞా(err == 9 || ret == err);
        ఒకవేళ (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            ఆపు;
        ఒకవేళ (ret == Z_NEED_DICT) {
            ret = inflate_సమితి_Dictionary(&strm, లోపల, 1);
                                                ప్రతిజ్ఞా(ret == Z_దత్తాంశం_ERROR);
            mem_హద్దు(&strm, 1);
            ret = inflate_సమితి_Dictionary(&strm, బహిర్గతం, 0);
                                                ప్రతిజ్ఞా(ret == Z_MEM_ERROR);
            mem_హద్దు(&strm, 0);
            ((నిర్మాణం inflate_state *)strm.state)->బహుళకం = DICT;
            ret = inflate_సమితి_Dictionary(&strm, బహిర్గతం, 0);
                                                ప్రతిజ్ఞా(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   ప్రతిజ్ఞా(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        ప్రతిజ్ఞా(ret == Z_OK);
        ret = inflate_చివర(&copy);                ప్రతిజ్ఞా(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_లోపల;
        strm.avail_లోపల = అడుగు > have ? have : అడుగు;
        have -= strm.avail_లోపల;
    } అయితే (strm.avail_లోపల);
    విడుదల(లోపల);
    విడుదల(బహిర్గతం);
    ret = inflateReset2(&strm, -8);             ప్రతిజ్ఞా(ret == Z_OK);
    ret = inflate_చివర(&strm);                    ప్రతిజ్ఞా(ret == Z_OK);
    mem_పూర్తి(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local శూన్య cover_support(శూన్య)
{
    పూర్ణాంకం ret;
    z_ప్రవాహం strm;

    mem_అమరిక(&strm);
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit(&strm);                   ప్రతిజ్ఞా(ret == Z_OK);
    mem_వాడిన(&strm, "inflate init");
    ret = inflate_ప్రధానసంఖ్య(&strm, 5, 31);           ప్రతిజ్ఞా(ret == Z_OK);
    ret = inflate_ప్రధానసంఖ్య(&strm, -1, 0);           ప్రతిజ్ఞా(ret == Z_OK);
    ret = inflate_సమితి_Dictionary(&strm, Z_NULL, 0);
                                                ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    ret = inflate_చివర(&strm);                    ప్రతిజ్ఞా(ret == Z_OK);
    mem_పూర్తి(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_ప్రవాహం_చివర);
    inf("", "bad window size", 0, 1, 0, Z_ప్రవాహం_ERROR);

    mem_అమరిక(&strm);
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit_(&strm, "!", (పూర్ణాంకం)పరిమాణం(z_ప్రవాహం));
                                                ప్రతిజ్ఞా(ret == Z_సంస్కరణ_ERROR);
    mem_పూర్తి(&strm, "wrong version");

    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit(&strm);                   ప్రతిజ్ఞా(ret == Z_OK);
    ret = inflate_చివర(&strm);                    ప్రతిజ్ఞా(ret == Z_OK);
    fputs("inflate built-in memory routines\n", stderr);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local శూన్య cover_wrap(శూన్య)
{
    పూర్ణాంకం ret;
    z_ప్రవాహం strm, copy;
    గుర్తులేని అక్షరం dict[257];

    ret = inflate(Z_NULL, 0);                   ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    ret = inflate_చివర(Z_NULL);                   ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    fputs("inflate bad parameters\n", stderr);

    inf("1f 8b 0 0", "bad gzip method", 0, 31, 0, Z_దత్తాంశం_ERROR);
    inf("1f 8b 8 80", "bad gzip flags", 0, 31, 0, Z_దత్తాంశం_ERROR);
    inf("77 85", "bad zlib method", 0, 15, 0, Z_దత్తాంశం_ERROR);
    inf("8 99", "set window size from header", 0, 0, 0, Z_OK);
    inf("78 9c", "bad zlib window size", 0, 8, 0, Z_దత్తాంశం_ERROR);
    inf("78 9c 63 0 0 0 1 0 1", "check adler32", 0, 15, 1, Z_ప్రవాహం_చివర);
    inf("1f 8b 8 1e 0 0 0 0 0 0 1 0 0 0 0 0 0", "bad header crc", 0, 47, 1,
        Z_దత్తాంశం_ERROR);
    inf("1f 8b 8 2 0 0 0 0 0 0 1d 26 3 0 0 0 0 0 0 0 0 0", "check gzip length",
        0, 47, 0, Z_ప్రవాహం_చివర);
    inf("78 90", "bad zlib header check", 0, 47, 0, Z_దత్తాంశం_ERROR);
    inf("8 b8 0 0 0 1", "need dictionary", 0, 8, 0, Z_NEED_DICT);
    inf("78 9c 63 0", "compute adler32", 0, 15, 1, Z_OK);

    mem_అమరిక(&strm);
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit2(&strm, -8);
    strm.avail_లోపల = 2;
    strm.తదుపరి_లోపల = (శూన్య *)"\x63";
    strm.avail_బహిర్గతం = 1;
    strm.తదుపరి_బహిర్గతం = (శూన్య *)&ret;
    mem_హద్దు(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           ప్రతిజ్ఞా(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           ప్రతిజ్ఞా(ret == Z_MEM_ERROR);
    mem_హద్దు(&strm, 0);
    memset(dict, 0, 257);
    ret = inflate_సమితి_Dictionary(&strm, dict, 257);
                                                ప్రతిజ్ఞా(ret == Z_OK);
    mem_హద్దు(&strm, (పరిమాణం(నిర్మాణం inflate_state) << 1) + 256);
    ret = inflate_ప్రధానసంఖ్య(&strm, 16, 0);           ప్రతిజ్ఞా(ret == Z_OK);
    strm.avail_లోపల = 2;
    strm.తదుపరి_లోపల = (శూన్య *)"\x80";
    ret = inflateSync(&strm);                   ప్రతిజ్ఞా(ret == Z_దత్తాంశం_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    strm.avail_లోపల = 4;
    strm.తదుపరి_లోపల = (శూన్య *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   ప్రతిజ్ఞా(ret == Z_OK);
    (శూన్య)inflate_Sync_బిందువు(&strm);
    ret = inflateCopy(&copy, &strm);            ప్రతిజ్ఞా(ret == Z_MEM_ERROR);
    mem_హద్దు(&strm, 0);
    ret = inflateUndermine(&strm, 1);           ప్రతిజ్ఞా(ret == Z_దత్తాంశం_ERROR);
    (శూన్య)inflateMark(&strm);
    ret = inflate_చివర(&strm);                    ప్రతిజ్ఞా(ret == Z_OK);
    mem_పూర్తి(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local గుర్తులేని pull(శూన్య *desc, గుర్తులేని అక్షరం z_const **buf)
{
    స్థిర గుర్తులేని పూర్ణాంకం తదుపరి = 0;
    స్థిర గుర్తులేని అక్షరం dat[] = {0x63, 0, 2, 0};
    నిర్మాణం inflate_state *state;

    ఒకవేళ (desc == Z_NULL) {
        తదుపరి = 0;
        తిరిగి 0;   /* no input (already provided at next_in) */
    }
    state = (శూన్య *)((z_ప్రవాహం *)desc)->state;
    ఒకవేళ (state != Z_NULL)
        state->బహుళకం = SYNC;     /* force an otherwise impossible situation */
    తిరిగి తదుపరి < పరిమాణం(dat) ? (*buf = dat + తదుపరి++, 1) : 0;
}

local పూర్ణాంకం నెట్టు(శూన్య *desc, గుర్తులేని అక్షరం *buf, గుర్తులేని len)
{
    (శూన్య)buf;
    (శూన్య)len;
    తిరిగి desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local శూన్య cover_వెనుక(శూన్య)
{
    పూర్ణాంకం ret;
    z_ప్రవాహం strm;
    గుర్తులేని అక్షరం win[32768];

    ret = inflate_వెనుక_Init_2(Z_NULL, 0, win, 0, 0);
                                                ప్రతిజ్ఞా(ret == Z_సంస్కరణ_ERROR);
    ret = inflate_వెనుక_Init(Z_NULL, 0, win);      ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    ret = inflate_వెనుక(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    ret = inflate_వెనుక_చివర(Z_NULL);               ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    fputs("inflateBack bad parameters\n", stderr);

    mem_అమరిక(&strm);
    ret = inflate_వెనుక_Init(&strm, 15, win);      ప్రతిజ్ఞా(ret == Z_OK);
    strm.avail_లోపల = 2;
    strm.తదుపరి_లోపల = (శూన్య *)"\x03";
    ret = inflate_వెనుక(&strm, pull, Z_NULL, నెట్టు, Z_NULL);
                                                ప్రతిజ్ఞా(ret == Z_ప్రవాహం_చివర);
        /* force output error */
    strm.avail_లోపల = 3;
    strm.తదుపరి_లోపల = (శూన్య *)"\x63\x00";
    ret = inflate_వెనుక(&strm, pull, Z_NULL, నెట్టు, &strm);
                                                ప్రతిజ్ఞా(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflate_వెనుక(&strm, pull, &strm, నెట్టు, Z_NULL);
                                                ప్రతిజ్ఞా(ret == Z_ప్రవాహం_ERROR);
    ret = inflate_వెనుక_చివర(&strm);                ప్రతిజ్ఞా(ret == Z_OK);
    mem_పూర్తి(&strm, "inflateBack bad state");

    ret = inflate_వెనుక_Init(&strm, 15, win);      ప్రతిజ్ఞా(ret == Z_OK);
    ret = inflate_వెనుక_చివర(&strm);                ప్రతిజ్ఞా(ret == Z_OK);
    fputs("inflateBack built-in memory routines\n", stderr);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local పూర్ణాంకం try(అక్షరం *hex, అక్షరం *id, పూర్ణాంకం err)
{
    పూర్ణాంకం ret;
    గుర్తులేని len, కొలత;
    గుర్తులేని అక్షరం *లోపల, *బహిర్గతం, *win;
    అక్షరం *ఉపసర్గ;
    z_ప్రవాహం strm;

    /* convert to hex */
    లోపల = h2b(hex, &len);
    ప్రతిజ్ఞా(లోపల != ఖాళీ);

    /* allocate work areas */
    కొలత = len << 3;
    బహిర్గతం = జ్ఞాపకం_ఇవ్వు(కొలత);
    ప్రతిజ్ఞా(బహిర్గతం != ఖాళీ);
    win = జ్ఞాపకం_ఇవ్వు(32768);
    ప్రతిజ్ఞా(win != ఖాళీ);
    ఉపసర్గ = జ్ఞాపకం_ఇవ్వు(అక్షరశ్రేణి_పొడవు(id) + 6);
    ప్రతిజ్ఞా(ఉపసర్గ != ఖాళీ);

    /* first with inflate */
    strcpy(ఉపసర్గ, id);
    strcat(ఉపసర్గ, "-late");
    mem_అమరిక(&strm);
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    ప్రతిజ్ఞా(ret == Z_OK);
    strm.avail_లోపల = len;
    strm.తదుపరి_లోపల = లోపల;
    చేయి {
        strm.avail_బహిర్గతం = కొలత;
        strm.తదుపరి_బహిర్గతం = బహిర్గతం;
        ret = inflate(&strm, Z_TREES);
        ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR && ret != Z_MEM_ERROR);
        ఒకవేళ (ret == Z_దత్తాంశం_ERROR || ret == Z_NEED_DICT)
            ఆపు;
    } అయితే (strm.avail_లోపల || strm.avail_బహిర్గతం == 0);
    ఒకవేళ (err) {
        ప్రతిజ్ఞా(ret == Z_దత్తాంశం_ERROR);
        ప్రతిజ్ఞా(strcmp(id, strm.సందేశం) == 0);
    }
    inflate_చివర(&strm);
    mem_పూర్తి(&strm, ఉపసర్గ);

    /* then with inflateBack */
    ఒకవేళ (err >= 0) {
        strcpy(ఉపసర్గ, id);
        strcat(ఉపసర్గ, "-back");
        mem_అమరిక(&strm);
        ret = inflate_వెనుక_Init(&strm, 15, win);
        ప్రతిజ్ఞా(ret == Z_OK);
        strm.avail_లోపల = len;
        strm.తదుపరి_లోపల = లోపల;
        ret = inflate_వెనుక(&strm, pull, Z_NULL, నెట్టు, Z_NULL);
        ప్రతిజ్ఞా(ret != Z_ప్రవాహం_ERROR);
        ఒకవేళ (err) {
            ప్రతిజ్ఞా(ret == Z_దత్తాంశం_ERROR);
            ప్రతిజ్ఞా(strcmp(id, strm.సందేశం) == 0);
        }
        inflate_వెనుక_చివర(&strm);
        mem_పూర్తి(&strm, ఉపసర్గ);
    }

    /* clean up */
    విడుదల(ఉపసర్గ);
    విడుదల(win);
    విడుదల(బహిర్గతం);
    విడుదల(లోపల);
    తిరిగి ret;
}

/* cover deflate data cases in both inflate() and inflateBack() */
local శూన్య cover_inflate(శూన్య)
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
        Z_ప్రవాహం_చివర);
    inf("63 18 5 40 c 0", "window wrap", 3, -8, 300, Z_OK);
}

/* cover remaining lines in inftrees.c */
local శూన్య cover_trees(శూన్య)
{
    పూర్ణాంకం ret;
    గుర్తులేని బిట్లు;
    గుర్తులేని పొట్టి lens[16], work[16];
    సంకేతం_2 *తదుపరి, పట్టిక_2[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    కొరకు (బిట్లు = 0; బిట్లు < 15; బిట్లు++)
        lens[బిట్లు] = (గుర్తులేని పొట్టి)(బిట్లు + 1);
    lens[15] = 15;
    తదుపరి = పట్టిక_2;
    బిట్లు = 15;
    ret = inflate_పట్టిక(DISTS, lens, 16, &తదుపరి, &బిట్లు, work);
                                                ప్రతిజ్ఞా(ret == 1);
    తదుపరి = పట్టిక_2;
    బిట్లు = 1;
    ret = inflate_పట్టిక(DISTS, lens, 16, &తదుపరి, &బిట్లు, work);
                                                ప్రతిజ్ఞా(ret == 1);
    fputs("inflate_table not enough errors\n", stderr);
}

/* cover remaining inffast.c decoding and window copying */
local శూన్య cover_fast(శూన్య)
{
    inf("e5 e0 81 ad 6d cb b2 2c c9 01 1e 59 63 ae 7d ee fb 4d fd b5 35 41 68"
        " ff 7f 0f 0 0 0", "fast length extra bits", 0, -8, 258, Z_దత్తాంశం_ERROR);
    inf("25 fd 81 b5 6d 59 b6 6a 49 ea af 35 6 34 eb 8c b9 f6 b9 1e ef 67 49"
        " 50 fe ff ff 3f 0 0", "fast distance extra bits", 0, -8, 258,
        Z_దత్తాంశం_ERROR);
    inf("3 7e 0 0 0 0 0", "fast invalid distance code", 0, -8, 258,
        Z_దత్తాంశం_ERROR);
    inf("1b 7 0 0 0 0 0", "fast invalid literal/length code", 0, -8, 258,
        Z_దత్తాంశం_ERROR);
    inf("d c7 1 ae eb 38 c 4 41 a0 87 72 de df fb 1f b8 36 b1 38 5d ff ff 0",
        "fast 2nd level codes and too far back", 0, -8, 258, Z_దత్తాంశం_ERROR);
    inf("63 18 5 8c 10 8 0 0 0 0", "very common case", 0, -8, 259, Z_OK);
    inf("63 60 60 18 c9 0 8 18 18 18 26 c0 28 0 29 0 0 0",
        "contiguous and wrap around window", 6, -8, 259, Z_OK);
    inf("63 0 3 0 0 0 0 0", "copy direct from output", 0, -8, 259,
        Z_ప్రవాహం_చివర);
}

పూర్ణాంకం ప్రధాన(శూన్య)
{
    fprintf(stderr, "%s\n", zlib_సంస్కరణ());
    cover_support();
    cover_wrap();
    cover_వెనుక();
    cover_inflate();
    cover_trees();
    cover_fast();
    తిరిగి 0;
}
