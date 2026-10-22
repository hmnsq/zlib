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
#تعریف_کړه ZLIB_دننی
#ضمن "inftrees.h"
#ضمن "inflate.h"

#تعریف_کړه local ساكن

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
بنية mem_توکی {
    فراغ *ptr;                  /* pointer to allocated memory */
    size_t اندازه;                /* requested size of allocation */
    بنية mem_توکی *بل;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
بنية mem_zone {
    بنية mem_توکی *لومړی;     /* pointer to first item in list, or NULL */
    size_t ټول, highwater;    /* total allocations, and largest total */
    size_t حد;               /* memory allocation limit, or 0 if no limit */
    صحيح notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local فراغ *mem_alloc(فراغ *mem, بینښه شمېر, بینښه اندازه)
{
    فراغ *ptr;
    بنية mem_توکی *توکی;
    بنية mem_zone *zone = mem;
    size_t len = شمېر * (size_t)اندازه;

    /* induced allocation failure */
    إذا (zone == عدم || (zone->حد && zone->ټول + len > zone->حد))
        أرجع عدم;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = حافظه_ورکړه(len);
    إذا (ptr == عدم)
        أرجع عدم;
    memset(ptr, 0xa5, len);

    /* create a new item for the list */
    توکی = حافظه_ورکړه(حجم(بنية mem_توکی));
    إذا (توکی == عدم) {
        حرر(ptr);
        أرجع عدم;
    }
    توکی->ptr = ptr;
    توکی->اندازه = len;

    /* insert item at the beginning of the list */
    توکی->بل = zone->لومړی;
    zone->لومړی = توکی;

    /* update the statistics */
    zone->ټول += توکی->اندازه;
    إذا (zone->ټول > zone->highwater)
        zone->highwater = zone->ټول;

    /* return the allocated memory */
    أرجع ptr;
}

/* memory free routine to pass to zlib */
local فراغ mem_free(فراغ *mem, فراغ *ptr)
{
    بنية mem_توکی *توکی, *بل;
    بنية mem_zone *zone = mem;

    /* if no zone, just do a free */
    إذا (zone == عدم) {
        حرر(ptr);
        أرجع;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    بل = zone->لومړی;
    إذا (بل) {
        إذا (بل->ptr == ptr)
            zone->لومړی = بل->بل;   /* first one is it, remove from list */
        وإلا {
            افعل {                        /* search the linked list */
                توکی = بل;
                بل = توکی->بل;
            } طالما (بل != عدم && بل->ptr != ptr);
            إذا (بل) {                 /* if found, remove from linked list */
                توکی->بل = بل->بل;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    إذا (بل) {
        zone->ټول -= بل->اندازه;
        حرر(بل);
    }

    /* if not found, update the rogue count */
    وإلا
        zone->rogue++;

    /* in any case, do the requested free with the standard library function */
    حرر(ptr);
}

/* set up a controlled memory allocation space for monitoring, set the stream
   parameters to the controlled routines, with opaque pointing to the space */
local فراغ mem_تیاری(z_بهیر *strm)
{
    بنية mem_zone *zone;

    zone = حافظه_ورکړه(حجم(بنية mem_zone));
    أكد(zone != عدم);
    zone->لومړی = عدم;
    zone->ټول = 0;
    zone->highwater = 0;
    zone->حد = 0;
    zone->notlifo = 0;
    zone->rogue = 0;
    strm->opaque = zone;
    strm->zalloc = mem_alloc;
    strm->zfree = mem_free;
}

/* set a limit on the total memory allocation, or 0 to remove the limit */
local فراغ mem_حد(z_بهیر *strm, size_t حد)
{
    بنية mem_zone *zone = strm->opaque;

    zone->حد = حد;
}

/* show the current total requested allocations in bytes */
local فراغ mem_کارول_شوی(z_بهیر *strm, حرف *مختاړی)
{
    بنية mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu allocated\n", مختاړی, zone->ټول);
}

/* show the high water allocation in bytes */
local فراغ mem_لوړ(z_بهیر *strm, حرف *مختاړی)
{
    بنية mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu high water mark\n", مختاړی, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local فراغ mem_بشپړ(z_بهیر *strm, حرف *مختاړی)
{
    صحيح شمېر = 0;
    بنية mem_توکی *توکی, *بل;
    بنية mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_لوړ(strm, مختاړی);

    /* free leftover allocations and item structures, if any */
    توکی = zone->لومړی;
    طالما (توکی != عدم) {
        حرر(توکی->ptr);
        بل = توکی->بل;
        حرر(توکی);
        توکی = بل;
        شمېر++;
    }

    /* issue alerts about anything unexpected */
    إذا (شمېر || zone->ټول)
        fprintf(stderr, "** %s: %zu bytes in %d blocks not freed\n",
                مختاړی, zone->ټول, شمېر);
    إذا (zone->notlifo)
        fprintf(stderr, "** %s: %d frees not LIFO\n", مختاړی, zone->notlifo);
    إذا (zone->rogue)
        fprintf(stderr, "** %s: %d frees not recognized\n",
                مختاړی, zone->rogue);

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
local بینښه حرف *h2b(ثابت حرف *hex, بینښه *len)
{
    بینښه حرف *دننه, *re;
    بینښه بل, val;

    دننه = حافظه_ورکړه((مزي_اوږدوالی(hex) + 1) >> 1);
    إذا (دننه == عدم)
        أرجع عدم;
    بل = 0;
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
            دننه[بل++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } طالما (*hex++);       /* go through the loop with the terminating null */
    إذا (len != عدم)
        *len = بل;
    re = realloc(دننه, بل);
    أرجع re == عدم ? دننه : re;
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
local فراغ inf(حرف *hex, حرف *what, بینښه ګام, صحيح win, بینښه len,
               صحيح err)
{
    صحيح ret;
    بینښه have;
    بینښه حرف *دننه, *بهر;
    z_بهیر strm, copy;
    gz_سرلیک سر;

    mem_تیاری(&strm);
    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    ret = inflateInit2(&strm, win);
    إذا (ret != Z_OK) {
        mem_بشپړ(&strm, what);
        أرجع;
    }
    بهر = حافظه_ورکړه(len);                          أكد(بهر != عدم);
    إذا (win == 47) {
        سر.extra = بهر;
        سر.extra_max = len;
        سر.name = بهر;
        سر.name_max = len;
        سر.comment = بهر;
        سر.comm_max = len;
        ret = inflate_Get_سرلیک(&strm, &سر);   أكد(ret == Z_OK);
    }
    دننه = h2b(hex, &have);                       أكد(دننه != عدم);
    إذا (ګام == 0 || ګام > have)
        ګام = have;
    strm.avail_دننه = ګام;
    have -= ګام;
    strm.بل_دننه = دننه;
    افعل {
        strm.avail_بهر = len;
        strm.بل_بهر = بهر;
        ret = inflate(&strm, Z_NO_FLUSH);       أكد(err == 9 || ret == err);
        إذا (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            اكسر;
        إذا (ret == Z_NEED_DICT) {
            ret = inflate_سېټ_Dictionary(&strm, دننه, 1);
                                                أكد(ret == Z_معلومات_ERROR);
            mem_حد(&strm, 1);
            ret = inflate_سېټ_Dictionary(&strm, بهر, 0);
                                                أكد(ret == Z_MEM_ERROR);
            mem_حد(&strm, 0);
            ((بنية inflate_state *)strm.state)->ډېرپېښ = DICT;
            ret = inflate_سېټ_Dictionary(&strm, بهر, 0);
                                                أكد(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   أكد(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        أكد(ret == Z_OK);
        ret = inflate_پای_ټکی(&copy);                أكد(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_دننه;
        strm.avail_دننه = ګام > have ? have : ګام;
        have -= strm.avail_دننه;
    } طالما (strm.avail_دننه);
    حرر(دننه);
    حرر(بهر);
    ret = inflateReset2(&strm, -8);             أكد(ret == Z_OK);
    ret = inflate_پای_ټکی(&strm);                    أكد(ret == Z_OK);
    mem_بشپړ(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local فراغ cover_support(فراغ)
{
    صحيح ret;
    z_بهیر strm;

    mem_تیاری(&strm);
    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    ret = inflateInit(&strm);                   أكد(ret == Z_OK);
    mem_کارول_شوی(&strm, "inflate init");
    ret = inflate_اولي(&strm, 5, 31);           أكد(ret == Z_OK);
    ret = inflate_اولي(&strm, -1, 0);           أكد(ret == Z_OK);
    ret = inflate_سېټ_Dictionary(&strm, Z_NULL, 0);
                                                أكد(ret == Z_بهیر_ERROR);
    ret = inflate_پای_ټکی(&strm);                    أكد(ret == Z_OK);
    mem_بشپړ(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_بهیر_پای_ټکی);
    inf("", "bad window size", 0, 1, 0, Z_بهیر_ERROR);

    mem_تیاری(&strm);
    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    ret = inflateInit_(&strm, "!", (صحيح)حجم(z_بهیر));
                                                أكد(ret == Z_نسخه_ERROR);
    mem_بشپړ(&strm, "wrong version");

    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    ret = inflateInit(&strm);                   أكد(ret == Z_OK);
    ret = inflate_پای_ټکی(&strm);                    أكد(ret == Z_OK);
    fputs("inflate built-in memory routines\n", stderr);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local فراغ cover_wrap(فراغ)
{
    صحيح ret;
    z_بهیر strm, copy;
    بینښه حرف dict[257];

    ret = inflate(Z_NULL, 0);                   أكد(ret == Z_بهیر_ERROR);
    ret = inflate_پای_ټکی(Z_NULL);                   أكد(ret == Z_بهیر_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          أكد(ret == Z_بهیر_ERROR);
    fputs("inflate bad parameters\n", stderr);

    inf("1f 8b 0 0", "bad gzip method", 0, 31, 0, Z_معلومات_ERROR);
    inf("1f 8b 8 80", "bad gzip flags", 0, 31, 0, Z_معلومات_ERROR);
    inf("77 85", "bad zlib method", 0, 15, 0, Z_معلومات_ERROR);
    inf("8 99", "set window size from header", 0, 0, 0, Z_OK);
    inf("78 9c", "bad zlib window size", 0, 8, 0, Z_معلومات_ERROR);
    inf("78 9c 63 0 0 0 1 0 1", "check adler32", 0, 15, 1, Z_بهیر_پای_ټکی);
    inf("1f 8b 8 1e 0 0 0 0 0 0 1 0 0 0 0 0 0", "bad header crc", 0, 47, 1,
        Z_معلومات_ERROR);
    inf("1f 8b 8 2 0 0 0 0 0 0 1d 26 3 0 0 0 0 0 0 0 0 0", "check gzip length",
        0, 47, 0, Z_بهیر_پای_ټکی);
    inf("78 90", "bad zlib header check", 0, 47, 0, Z_معلومات_ERROR);
    inf("8 b8 0 0 0 1", "need dictionary", 0, 8, 0, Z_NEED_DICT);
    inf("78 9c 63 0", "compute adler32", 0, 15, 1, Z_OK);

    mem_تیاری(&strm);
    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    ret = inflateInit2(&strm, -8);
    strm.avail_دننه = 2;
    strm.بل_دننه = (فراغ *)"\x63";
    strm.avail_بهر = 1;
    strm.بل_بهر = (فراغ *)&ret;
    mem_حد(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           أكد(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           أكد(ret == Z_MEM_ERROR);
    mem_حد(&strm, 0);
    memset(dict, 0, 257);
    ret = inflate_سېټ_Dictionary(&strm, dict, 257);
                                                أكد(ret == Z_OK);
    mem_حد(&strm, (حجم(بنية inflate_state) << 1) + 256);
    ret = inflate_اولي(&strm, 16, 0);           أكد(ret == Z_OK);
    strm.avail_دننه = 2;
    strm.بل_دننه = (فراغ *)"\x80";
    ret = inflateSync(&strm);                   أكد(ret == Z_معلومات_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           أكد(ret == Z_بهیر_ERROR);
    strm.avail_دننه = 4;
    strm.بل_دننه = (فراغ *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   أكد(ret == Z_OK);
    (فراغ)inflate_Sync_ټکی(&strm);
    ret = inflateCopy(&copy, &strm);            أكد(ret == Z_MEM_ERROR);
    mem_حد(&strm, 0);
    ret = inflateUndermine(&strm, 1);           أكد(ret == Z_معلومات_ERROR);
    (فراغ)inflateMark(&strm);
    ret = inflate_پای_ټکی(&strm);                    أكد(ret == Z_OK);
    mem_بشپړ(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local بینښه pull(فراغ *desc, بینښه حرف z_const **buf)
{
    ساكن بینښه صحيح بل = 0;
    ساكن بینښه حرف dat[] = {0x63, 0, 2, 0};
    بنية inflate_state *state;

    إذا (desc == Z_NULL) {
        بل = 0;
        أرجع 0;   /* no input (already provided at next_in) */
    }
    state = (فراغ *)((z_بهیر *)desc)->state;
    إذا (state != Z_NULL)
        state->ډېرپېښ = SYNC;     /* force an otherwise impossible situation */
    أرجع بل < حجم(dat) ? (*buf = dat + بل++, 1) : 0;
}

local صحيح ټېل(فراغ *desc, بینښه حرف *buf, بینښه len)
{
    (فراغ)buf;
    (فراغ)len;
    أرجع desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local فراغ cover_شا(فراغ)
{
    صحيح ret;
    z_بهیر strm;
    بینښه حرف win[32768];

    ret = inflate_شا_Init_2(Z_NULL, 0, win, 0, 0);
                                                أكد(ret == Z_نسخه_ERROR);
    ret = inflate_شا_Init(Z_NULL, 0, win);      أكد(ret == Z_بهیر_ERROR);
    ret = inflate_شا(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                أكد(ret == Z_بهیر_ERROR);
    ret = inflate_شا_پای_ټکی(Z_NULL);               أكد(ret == Z_بهیر_ERROR);
    fputs("inflateBack bad parameters\n", stderr);

    mem_تیاری(&strm);
    ret = inflate_شا_Init(&strm, 15, win);      أكد(ret == Z_OK);
    strm.avail_دننه = 2;
    strm.بل_دننه = (فراغ *)"\x03";
    ret = inflate_شا(&strm, pull, Z_NULL, ټېل, Z_NULL);
                                                أكد(ret == Z_بهیر_پای_ټکی);
        /* force output error */
    strm.avail_دننه = 3;
    strm.بل_دننه = (فراغ *)"\x63\x00";
    ret = inflate_شا(&strm, pull, Z_NULL, ټېل, &strm);
                                                أكد(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflate_شا(&strm, pull, &strm, ټېل, Z_NULL);
                                                أكد(ret == Z_بهیر_ERROR);
    ret = inflate_شا_پای_ټکی(&strm);                أكد(ret == Z_OK);
    mem_بشپړ(&strm, "inflateBack bad state");

    ret = inflate_شا_Init(&strm, 15, win);      أكد(ret == Z_OK);
    ret = inflate_شا_پای_ټکی(&strm);                أكد(ret == Z_OK);
    fputs("inflateBack built-in memory routines\n", stderr);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local صحيح try(حرف *hex, حرف *id, صحيح err)
{
    صحيح ret;
    بینښه len, اندازه;
    بینښه حرف *دننه, *بهر, *win;
    حرف *مختاړی;
    z_بهیر strm;

    /* convert to hex */
    دننه = h2b(hex, &len);
    أكد(دننه != عدم);

    /* allocate work areas */
    اندازه = len << 3;
    بهر = حافظه_ورکړه(اندازه);
    أكد(بهر != عدم);
    win = حافظه_ورکړه(32768);
    أكد(win != عدم);
    مختاړی = حافظه_ورکړه(مزي_اوږدوالی(id) + 6);
    أكد(مختاړی != عدم);

    /* first with inflate */
    strcpy(مختاړی, id);
    strcat(مختاړی, "-late");
    mem_تیاری(&strm);
    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    أكد(ret == Z_OK);
    strm.avail_دننه = len;
    strm.بل_دننه = دننه;
    افعل {
        strm.avail_بهر = اندازه;
        strm.بل_بهر = بهر;
        ret = inflate(&strm, Z_TREES);
        أكد(ret != Z_بهیر_ERROR && ret != Z_MEM_ERROR);
        إذا (ret == Z_معلومات_ERROR || ret == Z_NEED_DICT)
            اكسر;
    } طالما (strm.avail_دننه || strm.avail_بهر == 0);
    إذا (err) {
        أكد(ret == Z_معلومات_ERROR);
        أكد(strcmp(id, strm.پیغام) == 0);
    }
    inflate_پای_ټکی(&strm);
    mem_بشپړ(&strm, مختاړی);

    /* then with inflateBack */
    إذا (err >= 0) {
        strcpy(مختاړی, id);
        strcat(مختاړی, "-back");
        mem_تیاری(&strm);
        ret = inflate_شا_Init(&strm, 15, win);
        أكد(ret == Z_OK);
        strm.avail_دننه = len;
        strm.بل_دننه = دننه;
        ret = inflate_شا(&strm, pull, Z_NULL, ټېل, Z_NULL);
        أكد(ret != Z_بهیر_ERROR);
        إذا (err) {
            أكد(ret == Z_معلومات_ERROR);
            أكد(strcmp(id, strm.پیغام) == 0);
        }
        inflate_شا_پای_ټکی(&strm);
        mem_بشپړ(&strm, مختاړی);
    }

    /* clean up */
    حرر(مختاړی);
    حرر(win);
    حرر(بهر);
    حرر(دننه);
    أرجع ret;
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
        Z_بهیر_پای_ټکی);
    inf("63 18 5 40 c 0", "window wrap", 3, -8, 300, Z_OK);
}

/* cover remaining lines in inftrees.c */
local فراغ cover_trees(فراغ)
{
    صحيح ret;
    بینښه بټونه;
    بینښه لنډ lens[16], work[16];
    کوډ *بل, جدول_2[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    لكل (بټونه = 0; بټونه < 15; بټونه++)
        lens[بټونه] = (بینښه لنډ)(بټونه + 1);
    lens[15] = 15;
    بل = جدول_2;
    بټونه = 15;
    ret = inflate_جدول(DISTS, lens, 16, &بل, &بټونه, work);
                                                أكد(ret == 1);
    بل = جدول_2;
    بټونه = 1;
    ret = inflate_جدول(DISTS, lens, 16, &بل, &بټونه, work);
                                                أكد(ret == 1);
    fputs("inflate_table not enough errors\n", stderr);
}

/* cover remaining inffast.c decoding and window copying */
local فراغ cover_fast(فراغ)
{
    inf("e5 e0 81 ad 6d cb b2 2c c9 01 1e 59 63 ae 7d ee fb 4d fd b5 35 41 68"
        " ff 7f 0f 0 0 0", "fast length extra bits", 0, -8, 258, Z_معلومات_ERROR);
    inf("25 fd 81 b5 6d 59 b6 6a 49 ea af 35 6 34 eb 8c b9 f6 b9 1e ef 67 49"
        " 50 fe ff ff 3f 0 0", "fast distance extra bits", 0, -8, 258,
        Z_معلومات_ERROR);
    inf("3 7e 0 0 0 0 0", "fast invalid distance code", 0, -8, 258,
        Z_معلومات_ERROR);
    inf("1b 7 0 0 0 0 0", "fast invalid literal/length code", 0, -8, 258,
        Z_معلومات_ERROR);
    inf("d c7 1 ae eb 38 c 4 41 a0 87 72 de df fb 1f b8 36 b1 38 5d ff ff 0",
        "fast 2nd level codes and too far back", 0, -8, 258, Z_معلومات_ERROR);
    inf("63 18 5 8c 10 8 0 0 0 0", "very common case", 0, -8, 259, Z_OK);
    inf("63 60 60 18 c9 0 8 18 18 18 26 c0 28 0 29 0 0 0",
        "contiguous and wrap around window", 6, -8, 259, Z_OK);
    inf("63 0 3 0 0 0 0 0", "copy direct from output", 0, -8, 259,
        Z_بهیر_پای_ټکی);
}

صحيح اصلي(فراغ)
{
    fprintf(stderr, "%s\n", zlib_نسخه());
    cover_support();
    cover_wrap();
    cover_شا();
    cover_inflate();
    cover_trees();
    cover_fast();
    أرجع 0;
}
