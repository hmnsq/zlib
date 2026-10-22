/* infcover.c -- test zlib's inflate routines with full code coverage
 * Copyright (C) 2011, 2016, 2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* to use, do: ./configure --cover && make cover */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

/* get definition of internal structure so we can mess with it (see pull()),
   and so we can call inflate_trees() (see cover5()) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_ਅੰਦਰੂਨੀ
#ਅਨ੍ਤਰ੍ਭਾਵ "inftrees.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "inflate.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ local ਸਥਿਰ

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
ਬਣਤਰ mem_ਵਸਤੂ {
    ਖਾਲੀ *ptr;                  /* pointer to allocated memory */
    size_t ਮਾਪ;                /* requested size of allocation */
    ਬਣਤਰ mem_ਵਸਤੂ *ਅਗਲਾ;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
ਬਣਤਰ mem_zone {
    ਬਣਤਰ mem_ਵਸਤੂ *ਪਹਿਲਾ;     /* pointer to first item in list, or NULL */
    size_t ਕੁੱਲ, highwater;    /* total allocations, and largest total */
    size_t ਹੱਦਬੰਦੀ;               /* memory allocation limit, or 0 if no limit */
    ਪੂਰਨ notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local ਖਾਲੀ *mem_alloc(ਖਾਲੀ *mem, ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ)
{
    ਖਾਲੀ *ptr;
    ਬਣਤਰ mem_ਵਸਤੂ *ਵਸਤੂ;
    ਬਣਤਰ mem_zone *zone = mem;
    size_t len = ਗਿਣਤੀ_2 * (size_t)ਮਾਪ;

    /* induced allocation failure */
    ਜੇ (zone == ਨਿਰਾ || (zone->ਹੱਦਬੰਦੀ && zone->ਕੁੱਲ + len > zone->ਹੱਦਬੰਦੀ))
        ਵਾਪਸ ਨਿਰਾ;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len);
    ਜੇ (ptr == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    memset(ptr, 0xa5, len);

    /* create a new item for the list */
    ਵਸਤੂ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ mem_ਵਸਤੂ));
    ਜੇ (ਵਸਤੂ == ਨਿਰਾ) {
        ਮੁਕਤ(ptr);
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਵਸਤੂ->ptr = ptr;
    ਵਸਤੂ->ਮਾਪ = len;

    /* insert item at the beginning of the list */
    ਵਸਤੂ->ਅਗਲਾ = zone->ਪਹਿਲਾ;
    zone->ਪਹਿਲਾ = ਵਸਤੂ;

    /* update the statistics */
    zone->ਕੁੱਲ += ਵਸਤੂ->ਮਾਪ;
    ਜੇ (zone->ਕੁੱਲ > zone->highwater)
        zone->highwater = zone->ਕੁੱਲ;

    /* return the allocated memory */
    ਵਾਪਸ ptr;
}

/* memory free routine to pass to zlib */
local ਖਾਲੀ mem_free(ਖਾਲੀ *mem, ਖਾਲੀ *ptr)
{
    ਬਣਤਰ mem_ਵਸਤੂ *ਵਸਤੂ, *ਅਗਲਾ;
    ਬਣਤਰ mem_zone *zone = mem;

    /* if no zone, just do a free */
    ਜੇ (zone == ਨਿਰਾ) {
        ਮੁਕਤ(ptr);
        ਵਾਪਸ;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    ਅਗਲਾ = zone->ਪਹਿਲਾ;
    ਜੇ (ਅਗਲਾ) {
        ਜੇ (ਅਗਲਾ->ptr == ptr)
            zone->ਪਹਿਲਾ = ਅਗਲਾ->ਅਗਲਾ;   /* first one is it, remove from list */
        ਨਹੀਂਤਾਂ {
            ਕਰੋ {                        /* search the linked list */
                ਵਸਤੂ = ਅਗਲਾ;
                ਅਗਲਾ = ਵਸਤੂ->ਅਗਲਾ;
            } ਜਦੋਂ (ਅਗਲਾ != ਨਿਰਾ && ਅਗਲਾ->ptr != ptr);
            ਜੇ (ਅਗਲਾ) {                 /* if found, remove from linked list */
                ਵਸਤੂ->ਅਗਲਾ = ਅਗਲਾ->ਅਗਲਾ;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    ਜੇ (ਅਗਲਾ) {
        zone->ਕੁੱਲ -= ਅਗਲਾ->ਮਾਪ;
        ਮੁਕਤ(ਅਗਲਾ);
    }

    /* if not found, update the rogue count */
    ਨਹੀਂਤਾਂ
        zone->rogue++;

    /* in any case, do the requested free with the standard library function */
    ਮੁਕਤ(ptr);
}

/* set up a controlled memory allocation space for monitoring, set the stream
   parameters to the controlled routines, with opaque pointing to the space */
local ਖਾਲੀ mem_ਤਿਆਰੀ(z_ਧਾਰਾ *strm)
{
    ਬਣਤਰ mem_zone *zone;

    zone = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ mem_zone));
    ਪ੍ਰਤਿਜ੍ਞਾ(zone != ਨਿਰਾ);
    zone->ਪਹਿਲਾ = ਨਿਰਾ;
    zone->ਕੁੱਲ = 0;
    zone->highwater = 0;
    zone->ਹੱਦਬੰਦੀ = 0;
    zone->notlifo = 0;
    zone->rogue = 0;
    strm->opaque = zone;
    strm->zalloc = mem_alloc;
    strm->zfree = mem_free;
}

/* set a limit on the total memory allocation, or 0 to remove the limit */
local ਖਾਲੀ mem_ਹੱਦਬੰਦੀ(z_ਧਾਰਾ *strm, size_t ਹੱਦਬੰਦੀ)
{
    ਬਣਤਰ mem_zone *zone = strm->opaque;

    zone->ਹੱਦਬੰਦੀ = ਹੱਦਬੰਦੀ;
}

/* show the current total requested allocations in bytes */
local ਖਾਲੀ mem_ਵਰਤਿਆ(z_ਧਾਰਾ *strm, ਅੱਖਰ *ਅਗੇਤਰ)
{
    ਬਣਤਰ mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu allocated\n", ਅਗੇਤਰ, zone->ਕੁੱਲ);
}

/* show the high water allocation in bytes */
local ਖਾਲੀ mem_ਉੱਚਾ(z_ਧਾਰਾ *strm, ਅੱਖਰ *ਅਗੇਤਰ)
{
    ਬਣਤਰ mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu high water mark\n", ਅਗੇਤਰ, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local ਖਾਲੀ mem_ਪੂਰਾ(z_ਧਾਰਾ *strm, ਅੱਖਰ *ਅਗੇਤਰ)
{
    ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
    ਬਣਤਰ mem_ਵਸਤੂ *ਵਸਤੂ, *ਅਗਲਾ;
    ਬਣਤਰ mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_ਉੱਚਾ(strm, ਅਗੇਤਰ);

    /* free leftover allocations and item structures, if any */
    ਵਸਤੂ = zone->ਪਹਿਲਾ;
    ਜਦੋਂ (ਵਸਤੂ != ਨਿਰਾ) {
        ਮੁਕਤ(ਵਸਤੂ->ptr);
        ਅਗਲਾ = ਵਸਤੂ->ਅਗਲਾ;
        ਮੁਕਤ(ਵਸਤੂ);
        ਵਸਤੂ = ਅਗਲਾ;
        ਗਿਣਤੀ_2++;
    }

    /* issue alerts about anything unexpected */
    ਜੇ (ਗਿਣਤੀ_2 || zone->ਕੁੱਲ)
        fprintf(stderr, "** %s: %zu bytes in %d blocks not freed\n",
                ਅਗੇਤਰ, zone->ਕੁੱਲ, ਗਿਣਤੀ_2);
    ਜੇ (zone->notlifo)
        fprintf(stderr, "** %s: %d frees not LIFO\n", ਅਗੇਤਰ, zone->notlifo);
    ਜੇ (zone->rogue)
        fprintf(stderr, "** %s: %d frees not recognized\n",
                ਅਗੇਤਰ, zone->rogue);

    /* free the zone and delete from the stream */
    ਮੁਕਤ(zone);
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
local ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *h2b(ਅਚਲ ਅੱਖਰ *hex, ਬਿਨਾਨਿਸ਼ਾਨੀ *len)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ, *re;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅਗਲਾ, val;

    ਵਿੱਚ = ਸਮ੍ਰਿਤੀ_ਦਿਓ((ਲੜੀ_ਲੰਬਾਈ(hex) + 1) >> 1);
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    ਅਗਲਾ = 0;
    val = 1;
    ਕਰੋ {
        ਜੇ (*hex >= '0' && *hex <= '9')
            val = (val << 4) + *hex - '0';
        ਨਹੀਂਤਾਂ ਜੇ (*hex >= 'A' && *hex <= 'F')
            val = (val << 4) + *hex - 'A' + 10;
        ਨਹੀਂਤਾਂ ਜੇ (*hex >= 'a' && *hex <= 'f')
            val = (val << 4) + *hex - 'a' + 10;
        ਨਹੀਂਤਾਂ ਜੇ (val != 1 && val < 32)  /* one digit followed by delimiter */
            val += 240;                 /* make it look like two digits */
        ਜੇ (val > 255) {                /* have two digits */
            ਵਿੱਚ[ਅਗਲਾ++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } ਜਦੋਂ (*hex++);       /* go through the loop with the terminating null */
    ਜੇ (len != ਨਿਰਾ)
        *len = ਅਗਲਾ;
    re = realloc(ਵਿੱਚ, ਅਗਲਾ);
    ਵਾਪਸ re == ਨਿਰਾ ? ਵਿੱਚ : re;
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
local ਖਾਲੀ inf(ਅੱਖਰ *hex, ਅੱਖਰ *what, ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਦਮ, ਪੂਰਨ win, ਬਿਨਾਨਿਸ਼ਾਨੀ len,
               ਪੂਰਨ err)
{
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ, *ਬਾਹਰਲਾ;
    z_ਧਾਰਾ strm, copy;
    gz_ਸਿਰਲੇਖ ਸਿਰ;

    mem_ਤਿਆਰੀ(&strm);
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit2(&strm, win);
    ਜੇ (ret != Z_OK) {
        mem_ਪੂਰਾ(&strm, what);
        ਵਾਪਸ;
    }
    ਬਾਹਰਲਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len);                          ਪ੍ਰਤਿਜ੍ਞਾ(ਬਾਹਰਲਾ != ਨਿਰਾ);
    ਜੇ (win == 47) {
        ਸਿਰ.extra = ਬਾਹਰਲਾ;
        ਸਿਰ.extra_max = len;
        ਸਿਰ.name = ਬਾਹਰਲਾ;
        ਸਿਰ.name_max = len;
        ਸਿਰ.comment = ਬਾਹਰਲਾ;
        ਸਿਰ.comm_max = len;
        ret = inflate_Get_ਸਿਰਲੇਖ(&strm, &ਸਿਰ);   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    }
    ਵਿੱਚ = h2b(hex, &have);                       ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਚ != ਨਿਰਾ);
    ਜੇ (ਕਦਮ == 0 || ਕਦਮ > have)
        ਕਦਮ = have;
    strm.avail_ਵਿੱਚ = ਕਦਮ;
    have -= ਕਦਮ;
    strm.ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;
    ਕਰੋ {
        strm.avail_ਬਾਹਰਲਾ = len;
        strm.ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
        ret = inflate(&strm, Z_NO_FLUSH);       ਪ੍ਰਤਿਜ੍ਞਾ(err == 9 || ret == err);
        ਜੇ (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            ਰੋਕੋ;
        ਜੇ (ret == Z_NEED_DICT) {
            ret = inflate_ਸੈੱਟ_Dictionary(&strm, ਵਿੱਚ, 1);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਅੰਕੜੇ_ERROR);
            mem_ਹੱਦਬੰਦੀ(&strm, 1);
            ret = inflate_ਸੈੱਟ_Dictionary(&strm, ਬਾਹਰਲਾ, 0);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_MEM_ERROR);
            mem_ਹੱਦਬੰਦੀ(&strm, 0);
            ((ਬਣਤਰ inflate_state *)strm.state)->ਬਹੁਲਕ = DICT;
            ret = inflate_ਸੈੱਟ_Dictionary(&strm, ਬਾਹਰਲਾ, 0);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
        ret = inflate_ਸਿਰਾ(&copy);                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_ਵਿੱਚ;
        strm.avail_ਵਿੱਚ = ਕਦਮ > have ? have : ਕਦਮ;
        have -= strm.avail_ਵਿੱਚ;
    } ਜਦੋਂ (strm.avail_ਵਿੱਚ);
    ਮੁਕਤ(ਵਿੱਚ);
    ਮੁਕਤ(ਬਾਹਰਲਾ);
    ret = inflateReset2(&strm, -8);             ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    ret = inflate_ਸਿਰਾ(&strm);                    ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    mem_ਪੂਰਾ(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local ਖਾਲੀ cover_support(ਖਾਲੀ)
{
    ਪੂਰਨ ret;
    z_ਧਾਰਾ strm;

    mem_ਤਿਆਰੀ(&strm);
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit(&strm);                   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    mem_ਵਰਤਿਆ(&strm, "inflate init");
    ret = inflate_ਅਭਾਜ(&strm, 5, 31);           ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    ret = inflate_ਅਭਾਜ(&strm, -1, 0);           ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    ret = inflate_ਸੈੱਟ_Dictionary(&strm, Z_NULL, 0);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    ret = inflate_ਸਿਰਾ(&strm);                    ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    mem_ਪੂਰਾ(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_ਧਾਰਾ_ਸਿਰਾ);
    inf("", "bad window size", 0, 1, 0, Z_ਧਾਰਾ_ERROR);

    mem_ਤਿਆਰੀ(&strm);
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit_(&strm, "!", (ਪੂਰਨ)ਆਕਾਰ(z_ਧਾਰਾ));
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਸੰਸਕਰਣ_ERROR);
    mem_ਪੂਰਾ(&strm, "wrong version");

    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit(&strm);                   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    ret = inflate_ਸਿਰਾ(&strm);                    ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    fputs("inflate built-in memory routines\n", stderr);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local ਖਾਲੀ cover_wrap(ਖਾਲੀ)
{
    ਪੂਰਨ ret;
    z_ਧਾਰਾ strm, copy;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ dict[257];

    ret = inflate(Z_NULL, 0);                   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    ret = inflate_ਸਿਰਾ(Z_NULL);                   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    fputs("inflate bad parameters\n", stderr);

    inf("1f 8b 0 0", "bad gzip method", 0, 31, 0, Z_ਅੰਕੜੇ_ERROR);
    inf("1f 8b 8 80", "bad gzip flags", 0, 31, 0, Z_ਅੰਕੜੇ_ERROR);
    inf("77 85", "bad zlib method", 0, 15, 0, Z_ਅੰਕੜੇ_ERROR);
    inf("8 99", "set window size from header", 0, 0, 0, Z_OK);
    inf("78 9c", "bad zlib window size", 0, 8, 0, Z_ਅੰਕੜੇ_ERROR);
    inf("78 9c 63 0 0 0 1 0 1", "check adler32", 0, 15, 1, Z_ਧਾਰਾ_ਸਿਰਾ);
    inf("1f 8b 8 1e 0 0 0 0 0 0 1 0 0 0 0 0 0", "bad header crc", 0, 47, 1,
        Z_ਅੰਕੜੇ_ERROR);
    inf("1f 8b 8 2 0 0 0 0 0 0 1d 26 3 0 0 0 0 0 0 0 0 0", "check gzip length",
        0, 47, 0, Z_ਧਾਰਾ_ਸਿਰਾ);
    inf("78 90", "bad zlib header check", 0, 47, 0, Z_ਅੰਕੜੇ_ERROR);
    inf("8 b8 0 0 0 1", "need dictionary", 0, 8, 0, Z_NEED_DICT);
    inf("78 9c 63 0", "compute adler32", 0, 15, 1, Z_OK);

    mem_ਤਿਆਰੀ(&strm);
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit2(&strm, -8);
    strm.avail_ਵਿੱਚ = 2;
    strm.ਅਗਲਾ_ਵਿੱਚ = (ਖਾਲੀ *)"\x63";
    strm.avail_ਬਾਹਰਲਾ = 1;
    strm.ਅਗਲਾ_ਬਾਹਰਲਾ = (ਖਾਲੀ *)&ret;
    mem_ਹੱਦਬੰਦੀ(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_MEM_ERROR);
    mem_ਹੱਦਬੰਦੀ(&strm, 0);
    memset(dict, 0, 257);
    ret = inflate_ਸੈੱਟ_Dictionary(&strm, dict, 257);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    mem_ਹੱਦਬੰਦੀ(&strm, (ਆਕਾਰ(ਬਣਤਰ inflate_state) << 1) + 256);
    ret = inflate_ਅਭਾਜ(&strm, 16, 0);           ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    strm.avail_ਵਿੱਚ = 2;
    strm.ਅਗਲਾ_ਵਿੱਚ = (ਖਾਲੀ *)"\x80";
    ret = inflateSync(&strm);                   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਅੰਕੜੇ_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    strm.avail_ਵਿੱਚ = 4;
    strm.ਅਗਲਾ_ਵਿੱਚ = (ਖਾਲੀ *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    (ਖਾਲੀ)inflate_Sync_ਬਿੰਦੂ(&strm);
    ret = inflateCopy(&copy, &strm);            ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_MEM_ERROR);
    mem_ਹੱਦਬੰਦੀ(&strm, 0);
    ret = inflateUndermine(&strm, 1);           ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਅੰਕੜੇ_ERROR);
    (ਖਾਲੀ)inflateMark(&strm);
    ret = inflate_ਸਿਰਾ(&strm);                    ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    mem_ਪੂਰਾ(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local ਬਿਨਾਨਿਸ਼ਾਨੀ pull(ਖਾਲੀ *desc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ z_const **buf)
{
    ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅਗਲਾ = 0;
    ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ dat[] = {0x63, 0, 2, 0};
    ਬਣਤਰ inflate_state *state;

    ਜੇ (desc == Z_NULL) {
        ਅਗਲਾ = 0;
        ਵਾਪਸ 0;   /* no input (already provided at next_in) */
    }
    state = (ਖਾਲੀ *)((z_ਧਾਰਾ *)desc)->state;
    ਜੇ (state != Z_NULL)
        state->ਬਹੁਲਕ = SYNC;     /* force an otherwise impossible situation */
    ਵਾਪਸ ਅਗਲਾ < ਆਕਾਰ(dat) ? (*buf = dat + ਅਗਲਾ++, 1) : 0;
}

local ਪੂਰਨ ਧੱਕੋ(ਖਾਲੀ *desc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len)
{
    (ਖਾਲੀ)buf;
    (ਖਾਲੀ)len;
    ਵਾਪਸ desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local ਖਾਲੀ cover_ਪਿੱਠ(ਖਾਲੀ)
{
    ਪੂਰਨ ret;
    z_ਧਾਰਾ strm;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ win[32768];

    ret = inflate_ਪਿੱਠ_Init_2(Z_NULL, 0, win, 0, 0);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਸੰਸਕਰਣ_ERROR);
    ret = inflate_ਪਿੱਠ_Init(Z_NULL, 0, win);      ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    ret = inflate_ਪਿੱਠ(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    ret = inflate_ਪਿੱਠ_ਸਿਰਾ(Z_NULL);               ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    fputs("inflateBack bad parameters\n", stderr);

    mem_ਤਿਆਰੀ(&strm);
    ret = inflate_ਪਿੱਠ_Init(&strm, 15, win);      ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    strm.avail_ਵਿੱਚ = 2;
    strm.ਅਗਲਾ_ਵਿੱਚ = (ਖਾਲੀ *)"\x03";
    ret = inflate_ਪਿੱਠ(&strm, pull, Z_NULL, ਧੱਕੋ, Z_NULL);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ਸਿਰਾ);
        /* force output error */
    strm.avail_ਵਿੱਚ = 3;
    strm.ਅਗਲਾ_ਵਿੱਚ = (ਖਾਲੀ *)"\x63\x00";
    ret = inflate_ਪਿੱਠ(&strm, pull, Z_NULL, ਧੱਕੋ, &strm);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflate_ਪਿੱਠ(&strm, pull, &strm, ਧੱਕੋ, Z_NULL);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਧਾਰਾ_ERROR);
    ret = inflate_ਪਿੱਠ_ਸਿਰਾ(&strm);                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    mem_ਪੂਰਾ(&strm, "inflateBack bad state");

    ret = inflate_ਪਿੱਠ_Init(&strm, 15, win);      ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    ret = inflate_ਪਿੱਠ_ਸਿਰਾ(&strm);                ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    fputs("inflateBack built-in memory routines\n", stderr);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local ਪੂਰਨ try(ਅੱਖਰ *hex, ਅੱਖਰ *id, ਪੂਰਨ err)
{
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ len, ਮਾਪ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ, *ਬਾਹਰਲਾ, *win;
    ਅੱਖਰ *ਅਗੇਤਰ;
    z_ਧਾਰਾ strm;

    /* convert to hex */
    ਵਿੱਚ = h2b(hex, &len);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਚ != ਨਿਰਾ);

    /* allocate work areas */
    ਮਾਪ = len << 3;
    ਬਾਹਰਲਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਬਾਹਰਲਾ != ਨਿਰਾ);
    win = ਸਮ੍ਰਿਤੀ_ਦਿਓ(32768);
    ਪ੍ਰਤਿਜ੍ਞਾ(win != ਨਿਰਾ);
    ਅਗੇਤਰ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਲੜੀ_ਲੰਬਾਈ(id) + 6);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਅਗੇਤਰ != ਨਿਰਾ);

    /* first with inflate */
    strcpy(ਅਗੇਤਰ, id);
    strcat(ਅਗੇਤਰ, "-late");
    mem_ਤਿਆਰੀ(&strm);
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
    strm.avail_ਵਿੱਚ = len;
    strm.ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;
    ਕਰੋ {
        strm.avail_ਬਾਹਰਲਾ = ਮਾਪ;
        strm.ਅਗਲਾ_ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
        ret = inflate(&strm, Z_TREES);
        ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR && ret != Z_MEM_ERROR);
        ਜੇ (ret == Z_ਅੰਕੜੇ_ERROR || ret == Z_NEED_DICT)
            ਰੋਕੋ;
    } ਜਦੋਂ (strm.avail_ਵਿੱਚ || strm.avail_ਬਾਹਰਲਾ == 0);
    ਜੇ (err) {
        ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਅੰਕੜੇ_ERROR);
        ਪ੍ਰਤਿਜ੍ਞਾ(strcmp(id, strm.ਸੁਨੇਹਾ) == 0);
    }
    inflate_ਸਿਰਾ(&strm);
    mem_ਪੂਰਾ(&strm, ਅਗੇਤਰ);

    /* then with inflateBack */
    ਜੇ (err >= 0) {
        strcpy(ਅਗੇਤਰ, id);
        strcat(ਅਗੇਤਰ, "-back");
        mem_ਤਿਆਰੀ(&strm);
        ret = inflate_ਪਿੱਠ_Init(&strm, 15, win);
        ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_OK);
        strm.avail_ਵਿੱਚ = len;
        strm.ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;
        ret = inflate_ਪਿੱਠ(&strm, pull, Z_NULL, ਧੱਕੋ, Z_NULL);
        ਪ੍ਰਤਿਜ੍ਞਾ(ret != Z_ਧਾਰਾ_ERROR);
        ਜੇ (err) {
            ਪ੍ਰਤਿਜ੍ਞਾ(ret == Z_ਅੰਕੜੇ_ERROR);
            ਪ੍ਰਤਿਜ੍ਞਾ(strcmp(id, strm.ਸੁਨੇਹਾ) == 0);
        }
        inflate_ਪਿੱਠ_ਸਿਰਾ(&strm);
        mem_ਪੂਰਾ(&strm, ਅਗੇਤਰ);
    }

    /* clean up */
    ਮੁਕਤ(ਅਗੇਤਰ);
    ਮੁਕਤ(win);
    ਮੁਕਤ(ਬਾਹਰਲਾ);
    ਮੁਕਤ(ਵਿੱਚ);
    ਵਾਪਸ ret;
}

/* cover deflate data cases in both inflate() and inflateBack() */
local ਖਾਲੀ cover_inflate(ਖਾਲੀ)
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
        Z_ਧਾਰਾ_ਸਿਰਾ);
    inf("63 18 5 40 c 0", "window wrap", 3, -8, 300, Z_OK);
}

/* cover remaining lines in inftrees.c */
local ਖਾਲੀ cover_trees(ਖਾਲੀ)
{
    ਪੂਰਨ ret;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ lens[16], work[16];
    ਕੋਡ *ਅਗਲਾ, ਸਾਰਣੀ_2[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    ਲਈ (ਬਿੱਟ = 0; ਬਿੱਟ < 15; ਬਿੱਟ++)
        lens[ਬਿੱਟ] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ)(ਬਿੱਟ + 1);
    lens[15] = 15;
    ਅਗਲਾ = ਸਾਰਣੀ_2;
    ਬਿੱਟ = 15;
    ret = inflate_ਸਾਰਣੀ(DISTS, lens, 16, &ਅਗਲਾ, &ਬਿੱਟ, work);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == 1);
    ਅਗਲਾ = ਸਾਰਣੀ_2;
    ਬਿੱਟ = 1;
    ret = inflate_ਸਾਰਣੀ(DISTS, lens, 16, &ਅਗਲਾ, &ਬਿੱਟ, work);
                                                ਪ੍ਰਤਿਜ੍ਞਾ(ret == 1);
    fputs("inflate_table not enough errors\n", stderr);
}

/* cover remaining inffast.c decoding and window copying */
local ਖਾਲੀ cover_fast(ਖਾਲੀ)
{
    inf("e5 e0 81 ad 6d cb b2 2c c9 01 1e 59 63 ae 7d ee fb 4d fd b5 35 41 68"
        " ff 7f 0f 0 0 0", "fast length extra bits", 0, -8, 258, Z_ਅੰਕੜੇ_ERROR);
    inf("25 fd 81 b5 6d 59 b6 6a 49 ea af 35 6 34 eb 8c b9 f6 b9 1e ef 67 49"
        " 50 fe ff ff 3f 0 0", "fast distance extra bits", 0, -8, 258,
        Z_ਅੰਕੜੇ_ERROR);
    inf("3 7e 0 0 0 0 0", "fast invalid distance code", 0, -8, 258,
        Z_ਅੰਕੜੇ_ERROR);
    inf("1b 7 0 0 0 0 0", "fast invalid literal/length code", 0, -8, 258,
        Z_ਅੰਕੜੇ_ERROR);
    inf("d c7 1 ae eb 38 c 4 41 a0 87 72 de df fb 1f b8 36 b1 38 5d ff ff 0",
        "fast 2nd level codes and too far back", 0, -8, 258, Z_ਅੰਕੜੇ_ERROR);
    inf("63 18 5 8c 10 8 0 0 0 0", "very common case", 0, -8, 259, Z_OK);
    inf("63 60 60 18 c9 0 8 18 18 18 26 c0 28 0 29 0 0 0",
        "contiguous and wrap around window", 6, -8, 259, Z_OK);
    inf("63 0 3 0 0 0 0 0", "copy direct from output", 0, -8, 259,
        Z_ਧਾਰਾ_ਸਿਰਾ);
}

ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ)
{
    fprintf(stderr, "%s\n", zlib_ਸੰਸਕਰਣ());
    cover_support();
    cover_wrap();
    cover_ਪਿੱਠ();
    cover_inflate();
    cover_trees();
    cover_fast();
    ਵਾਪਸ 0;
}
