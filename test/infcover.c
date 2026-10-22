/* infcover.c -- test zlib's inflate routines with full code coverage
 * Copyright (C) 2011, 2016, 2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* to use, do: ./configure --cover && make cover */

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

/* get definition of internal structure so we can mess with it (see pull()),
   and so we can call inflate_trees() (see cover5()) */
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_ಆಂತರಿಕ
#ಅನ್ತರ್ಭಾವ "inftrees.h"
#ಅನ್ತರ್ಭಾವ "inflate.h"

#ವ್ಯಾಖ್ಯಾನಿಸು local ಸ್ಥಿರ

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
ರಚನೆ mem_ಅಂಶ {
    ಶೂನ್ಯ *ptr;                  /* pointer to allocated memory */
    size_t ಗಾತ್ರ_2;                /* requested size of allocation */
    ರಚನೆ mem_ಅಂಶ *ಮುಂದಿನ;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
ರಚನೆ mem_zone {
    ರಚನೆ mem_ಅಂಶ *ಮೊದಲ;     /* pointer to first item in list, or NULL */
    size_t ಒಟ್ಟು, highwater;    /* total allocations, and largest total */
    size_t ಎಲ್ಲೆ;               /* memory allocation limit, or 0 if no limit */
    ಪೂರ್ಣಾಂಕ notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local ಶೂನ್ಯ *mem_alloc(ಶೂನ್ಯ *mem, ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2)
{
    ಶೂನ್ಯ *ptr;
    ರಚನೆ mem_ಅಂಶ *ಅಂಶ;
    ರಚನೆ mem_zone *zone = mem;
    size_t len = ಎಣಿಕೆ_2 * (size_t)ಗಾತ್ರ_2;

    /* induced allocation failure */
    ಆದರೆ (zone == ಖಾಲಿ || (zone->ಎಲ್ಲೆ && zone->ಒಟ್ಟು + len > zone->ಎಲ್ಲೆ))
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = ಸ್ಮರಣೆ_ಕೊಡು(len);
    ಆದರೆ (ptr == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    memset(ptr, 0xa5, len);

    /* create a new item for the list */
    ಅಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ mem_ಅಂಶ));
    ಆದರೆ (ಅಂಶ == ಖಾಲಿ) {
        ಬಿಡುಗಡೆ(ptr);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಅಂಶ->ptr = ptr;
    ಅಂಶ->ಗಾತ್ರ_2 = len;

    /* insert item at the beginning of the list */
    ಅಂಶ->ಮುಂದಿನ = zone->ಮೊದಲ;
    zone->ಮೊದಲ = ಅಂಶ;

    /* update the statistics */
    zone->ಒಟ್ಟು += ಅಂಶ->ಗಾತ್ರ_2;
    ಆದರೆ (zone->ಒಟ್ಟು > zone->highwater)
        zone->highwater = zone->ಒಟ್ಟು;

    /* return the allocated memory */
    ಹಿಂತಿರುಗಿ ptr;
}

/* memory free routine to pass to zlib */
local ಶೂನ್ಯ mem_free(ಶೂನ್ಯ *mem, ಶೂನ್ಯ *ptr)
{
    ರಚನೆ mem_ಅಂಶ *ಅಂಶ, *ಮುಂದಿನ;
    ರಚನೆ mem_zone *zone = mem;

    /* if no zone, just do a free */
    ಆದರೆ (zone == ಖಾಲಿ) {
        ಬಿಡುಗಡೆ(ptr);
        ಹಿಂತಿರುಗಿ;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    ಮುಂದಿನ = zone->ಮೊದಲ;
    ಆದರೆ (ಮುಂದಿನ) {
        ಆದರೆ (ಮುಂದಿನ->ptr == ptr)
            zone->ಮೊದಲ = ಮುಂದಿನ->ಮುಂದಿನ;   /* first one is it, remove from list */
        ಇಲ್ಲದಿದ್ದರೆ {
            ಮಾಡು {                        /* search the linked list */
                ಅಂಶ = ಮುಂದಿನ;
                ಮುಂದಿನ = ಅಂಶ->ಮುಂದಿನ;
            } ಆಗ (ಮುಂದಿನ != ಖಾಲಿ && ಮುಂದಿನ->ptr != ptr);
            ಆದರೆ (ಮುಂದಿನ) {                 /* if found, remove from linked list */
                ಅಂಶ->ಮುಂದಿನ = ಮುಂದಿನ->ಮುಂದಿನ;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    ಆದರೆ (ಮುಂದಿನ) {
        zone->ಒಟ್ಟು -= ಮುಂದಿನ->ಗಾತ್ರ_2;
        ಬಿಡುಗಡೆ(ಮುಂದಿನ);
    }

    /* if not found, update the rogue count */
    ಇಲ್ಲದಿದ್ದರೆ
        zone->rogue++;

    /* in any case, do the requested free with the standard library function */
    ಬಿಡುಗಡೆ(ptr);
}

/* set up a controlled memory allocation space for monitoring, set the stream
   parameters to the controlled routines, with opaque pointing to the space */
local ಶೂನ್ಯ mem_ಸಿದ್ಧತೆ(z_ಹರಿವು *strm)
{
    ರಚನೆ mem_zone *zone;

    zone = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ mem_zone));
    ಪ್ರತಿಜ್ಞಾ(zone != ಖಾಲಿ);
    zone->ಮೊದಲ = ಖಾಲಿ;
    zone->ಒಟ್ಟು = 0;
    zone->highwater = 0;
    zone->ಎಲ್ಲೆ = 0;
    zone->notlifo = 0;
    zone->rogue = 0;
    strm->opaque = zone;
    strm->zalloc = mem_alloc;
    strm->zfree = mem_free;
}

/* set a limit on the total memory allocation, or 0 to remove the limit */
local ಶೂನ್ಯ mem_ಎಲ್ಲೆ(z_ಹರಿವು *strm, size_t ಎಲ್ಲೆ)
{
    ರಚನೆ mem_zone *zone = strm->opaque;

    zone->ಎಲ್ಲೆ = ಎಲ್ಲೆ;
}

/* show the current total requested allocations in bytes */
local ಶೂನ್ಯ mem_ಬಳಸಿದ(z_ಹರಿವು *strm, ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
    ರಚನೆ mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu allocated\n", ಪೂರ್ವಪ್ರತ್ಯಯ, zone->ಒಟ್ಟು);
}

/* show the high water allocation in bytes */
local ಶೂನ್ಯ mem_ಮೇಲಿನ(z_ಹರಿವು *strm, ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
    ರಚನೆ mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu high water mark\n", ಪೂರ್ವಪ್ರತ್ಯಯ, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local ಶೂನ್ಯ mem_ಮುಗಿದ(z_ಹರಿವು *strm, ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
    ರಚನೆ mem_ಅಂಶ *ಅಂಶ, *ಮುಂದಿನ;
    ರಚನೆ mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_ಮೇಲಿನ(strm, ಪೂರ್ವಪ್ರತ್ಯಯ);

    /* free leftover allocations and item structures, if any */
    ಅಂಶ = zone->ಮೊದಲ;
    ಆಗ (ಅಂಶ != ಖಾಲಿ) {
        ಬಿಡುಗಡೆ(ಅಂಶ->ptr);
        ಮುಂದಿನ = ಅಂಶ->ಮುಂದಿನ;
        ಬಿಡುಗಡೆ(ಅಂಶ);
        ಅಂಶ = ಮುಂದಿನ;
        ಎಣಿಕೆ_2++;
    }

    /* issue alerts about anything unexpected */
    ಆದರೆ (ಎಣಿಕೆ_2 || zone->ಒಟ್ಟು)
        fprintf(stderr, "** %s: %zu bytes in %d blocks not freed\n",
                ಪೂರ್ವಪ್ರತ್ಯಯ, zone->ಒಟ್ಟು, ಎಣಿಕೆ_2);
    ಆದರೆ (zone->notlifo)
        fprintf(stderr, "** %s: %d frees not LIFO\n", ಪೂರ್ವಪ್ರತ್ಯಯ, zone->notlifo);
    ಆದರೆ (zone->rogue)
        fprintf(stderr, "** %s: %d frees not recognized\n",
                ಪೂರ್ವಪ್ರತ್ಯಯ, zone->rogue);

    /* free the zone and delete from the stream */
    ಬಿಡುಗಡೆ(zone);
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
local ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *h2b(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hex, ಚಿಹ್ನೆರಹಿತ *len)
{
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ, *re;
    ಚಿಹ್ನೆರಹಿತ ಮುಂದಿನ, val;

    ಒಳ = ಸ್ಮರಣೆ_ಕೊಡು((ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(hex) + 1) >> 1);
    ಆದರೆ (ಒಳ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಮುಂದಿನ = 0;
    val = 1;
    ಮಾಡು {
        ಆದರೆ (*hex >= '0' && *hex <= '9')
            val = (val << 4) + *hex - '0';
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*hex >= 'A' && *hex <= 'F')
            val = (val << 4) + *hex - 'A' + 10;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*hex >= 'a' && *hex <= 'f')
            val = (val << 4) + *hex - 'a' + 10;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (val != 1 && val < 32)  /* one digit followed by delimiter */
            val += 240;                 /* make it look like two digits */
        ಆದರೆ (val > 255) {                /* have two digits */
            ಒಳ[ಮುಂದಿನ++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } ಆಗ (*hex++);       /* go through the loop with the terminating null */
    ಆದರೆ (len != ಖಾಲಿ)
        *len = ಮುಂದಿನ;
    re = realloc(ಒಳ, ಮುಂದಿನ);
    ಹಿಂತಿರುಗಿ re == ಖಾಲಿ ? ಒಳ : re;
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
local ಶೂನ್ಯ inf(ಅಕ್ಷರ *hex, ಅಕ್ಷರ *what, ಚಿಹ್ನೆರಹಿತ ಹೆಜ್ಜೆ, ಪೂರ್ಣಾಂಕ win, ಚಿಹ್ನೆರಹಿತ len,
               ಪೂರ್ಣಾಂಕ err)
{
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ have;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ, *ಹೊರಹೋಗು;
    z_ಹರಿವು strm, copy;
    gz_ಶೀರ್ಷಿಕೆ ತಲೆ;

    mem_ಸಿದ್ಧತೆ(&strm);
    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit2(&strm, win);
    ಆದರೆ (ret != Z_OK) {
        mem_ಮುಗಿದ(&strm, what);
        ಹಿಂತಿರುಗಿ;
    }
    ಹೊರಹೋಗು = ಸ್ಮರಣೆ_ಕೊಡು(len);                          ಪ್ರತಿಜ್ಞಾ(ಹೊರಹೋಗು != ಖಾಲಿ);
    ಆದರೆ (win == 47) {
        ತಲೆ.extra = ಹೊರಹೋಗು;
        ತಲೆ.extra_max = len;
        ತಲೆ.name = ಹೊರಹೋಗು;
        ತಲೆ.name_max = len;
        ತಲೆ.comment = ಹೊರಹೋಗು;
        ತಲೆ.comm_max = len;
        ret = inflate_Get_ಶೀರ್ಷಿಕೆ(&strm, &ತಲೆ);   ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    }
    ಒಳ = h2b(hex, &have);                       ಪ್ರತಿಜ್ಞಾ(ಒಳ != ಖಾಲಿ);
    ಆದರೆ (ಹೆಜ್ಜೆ == 0 || ಹೆಜ್ಜೆ > have)
        ಹೆಜ್ಜೆ = have;
    strm.avail_ಒಳ = ಹೆಜ್ಜೆ;
    have -= ಹೆಜ್ಜೆ;
    strm.ಮುಂದಿನ_ಒಳ = ಒಳ;
    ಮಾಡು {
        strm.avail_ಹೊರಹೋಗು = len;
        strm.ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
        ret = inflate(&strm, Z_NO_FLUSH);       ಪ್ರತಿಜ್ಞಾ(err == 9 || ret == err);
        ಆದರೆ (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (ret == Z_NEED_DICT) {
            ret = inflate_ಗಣ_Dictionary(&strm, ಒಳ, 1);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_ದತ್ತಾಂಶ_ERROR);
            mem_ಎಲ್ಲೆ(&strm, 1);
            ret = inflate_ಗಣ_Dictionary(&strm, ಹೊರಹೋಗು, 0);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_MEM_ERROR);
            mem_ಎಲ್ಲೆ(&strm, 0);
            ((ರಚನೆ inflate_state *)strm.state)->ಬಹುಲಕ = DICT;
            ret = inflate_ಗಣ_Dictionary(&strm, ಹೊರಹೋಗು, 0);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   ಪ್ರತಿಜ್ಞಾ(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
        ret = inflate_ಕೊನೆ(&copy);                ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_ಒಳ;
        strm.avail_ಒಳ = ಹೆಜ್ಜೆ > have ? have : ಹೆಜ್ಜೆ;
        have -= strm.avail_ಒಳ;
    } ಆಗ (strm.avail_ಒಳ);
    ಬಿಡುಗಡೆ(ಒಳ);
    ಬಿಡುಗಡೆ(ಹೊರಹೋಗು);
    ret = inflateReset2(&strm, -8);             ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    ret = inflate_ಕೊನೆ(&strm);                    ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    mem_ಮುಗಿದ(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local ಶೂನ್ಯ cover_support(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ret;
    z_ಹರಿವು strm;

    mem_ಸಿದ್ಧತೆ(&strm);
    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit(&strm);                   ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    mem_ಬಳಸಿದ(&strm, "inflate init");
    ret = inflate_ಅವಿಭಾಜ್ಯ(&strm, 5, 31);           ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    ret = inflate_ಅವಿಭಾಜ್ಯ(&strm, -1, 0);           ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    ret = inflate_ಗಣ_Dictionary(&strm, Z_NULL, 0);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    ret = inflate_ಕೊನೆ(&strm);                    ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    mem_ಮುಗಿದ(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_ಹರಿವು_ಕೊನೆ);
    inf("", "bad window size", 0, 1, 0, Z_ಹರಿವು_ERROR);

    mem_ಸಿದ್ಧತೆ(&strm);
    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit_(&strm, "!", (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(z_ಹರಿವು));
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_ಆವೃತ್ತಿ_ERROR);
    mem_ಮುಗಿದ(&strm, "wrong version");

    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit(&strm);                   ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    ret = inflate_ಕೊನೆ(&strm);                    ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    fputs("inflate built-in memory routines\n", stderr);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local ಶೂನ್ಯ cover_wrap(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ret;
    z_ಹರಿವು strm, copy;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ dict[257];

    ret = inflate(Z_NULL, 0);                   ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    ret = inflate_ಕೊನೆ(Z_NULL);                   ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    fputs("inflate bad parameters\n", stderr);

    inf("1f 8b 0 0", "bad gzip method", 0, 31, 0, Z_ದತ್ತಾಂಶ_ERROR);
    inf("1f 8b 8 80", "bad gzip flags", 0, 31, 0, Z_ದತ್ತಾಂಶ_ERROR);
    inf("77 85", "bad zlib method", 0, 15, 0, Z_ದತ್ತಾಂಶ_ERROR);
    inf("8 99", "set window size from header", 0, 0, 0, Z_OK);
    inf("78 9c", "bad zlib window size", 0, 8, 0, Z_ದತ್ತಾಂಶ_ERROR);
    inf("78 9c 63 0 0 0 1 0 1", "check adler32", 0, 15, 1, Z_ಹರಿವು_ಕೊನೆ);
    inf("1f 8b 8 1e 0 0 0 0 0 0 1 0 0 0 0 0 0", "bad header crc", 0, 47, 1,
        Z_ದತ್ತಾಂಶ_ERROR);
    inf("1f 8b 8 2 0 0 0 0 0 0 1d 26 3 0 0 0 0 0 0 0 0 0", "check gzip length",
        0, 47, 0, Z_ಹರಿವು_ಕೊನೆ);
    inf("78 90", "bad zlib header check", 0, 47, 0, Z_ದತ್ತಾಂಶ_ERROR);
    inf("8 b8 0 0 0 1", "need dictionary", 0, 8, 0, Z_NEED_DICT);
    inf("78 9c 63 0", "compute adler32", 0, 15, 1, Z_OK);

    mem_ಸಿದ್ಧತೆ(&strm);
    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit2(&strm, -8);
    strm.avail_ಒಳ = 2;
    strm.ಮುಂದಿನ_ಒಳ = (ಶೂನ್ಯ *)"\x63";
    strm.avail_ಹೊರಹೋಗು = 1;
    strm.ಮುಂದಿನ_ಹೊರಹೋಗು = (ಶೂನ್ಯ *)&ret;
    mem_ಎಲ್ಲೆ(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           ಪ್ರತಿಜ್ಞಾ(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           ಪ್ರತಿಜ್ಞಾ(ret == Z_MEM_ERROR);
    mem_ಎಲ್ಲೆ(&strm, 0);
    memset(dict, 0, 257);
    ret = inflate_ಗಣ_Dictionary(&strm, dict, 257);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    mem_ಎಲ್ಲೆ(&strm, (ಗಾತ್ರ(ರಚನೆ inflate_state) << 1) + 256);
    ret = inflate_ಅವಿಭಾಜ್ಯ(&strm, 16, 0);           ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    strm.avail_ಒಳ = 2;
    strm.ಮುಂದಿನ_ಒಳ = (ಶೂನ್ಯ *)"\x80";
    ret = inflateSync(&strm);                   ಪ್ರತಿಜ್ಞಾ(ret == Z_ದತ್ತಾಂಶ_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    strm.avail_ಒಳ = 4;
    strm.ಮುಂದಿನ_ಒಳ = (ಶೂನ್ಯ *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    (ಶೂನ್ಯ)inflate_Sync_ಬಿಂದು(&strm);
    ret = inflateCopy(&copy, &strm);            ಪ್ರತಿಜ್ಞಾ(ret == Z_MEM_ERROR);
    mem_ಎಲ್ಲೆ(&strm, 0);
    ret = inflateUndermine(&strm, 1);           ಪ್ರತಿಜ್ಞಾ(ret == Z_ದತ್ತಾಂಶ_ERROR);
    (ಶೂನ್ಯ)inflateMark(&strm);
    ret = inflate_ಕೊನೆ(&strm);                    ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    mem_ಮುಗಿದ(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local ಚಿಹ್ನೆರಹಿತ pull(ಶೂನ್ಯ *desc, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ z_const **buf)
{
    ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಂದಿನ = 0;
    ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ dat[] = {0x63, 0, 2, 0};
    ರಚನೆ inflate_state *state;

    ಆದರೆ (desc == Z_NULL) {
        ಮುಂದಿನ = 0;
        ಹಿಂತಿರುಗಿ 0;   /* no input (already provided at next_in) */
    }
    state = (ಶೂನ್ಯ *)((z_ಹರಿವು *)desc)->state;
    ಆದರೆ (state != Z_NULL)
        state->ಬಹುಲಕ = SYNC;     /* force an otherwise impossible situation */
    ಹಿಂತಿರುಗಿ ಮುಂದಿನ < ಗಾತ್ರ(dat) ? (*buf = dat + ಮುಂದಿನ++, 1) : 0;
}

local ಪೂರ್ಣಾಂಕ ತಳ್ಳು(ಶೂನ್ಯ *desc, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ len)
{
    (ಶೂನ್ಯ)buf;
    (ಶೂನ್ಯ)len;
    ಹಿಂತಿರುಗಿ desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local ಶೂನ್ಯ cover_ಹಿಂಭಾಗ(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ret;
    z_ಹರಿವು strm;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ win[32768];

    ret = inflate_ಹಿಂಭಾಗ_Init_2(Z_NULL, 0, win, 0, 0);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_ಆವೃತ್ತಿ_ERROR);
    ret = inflate_ಹಿಂಭಾಗ_Init(Z_NULL, 0, win);      ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    ret = inflate_ಹಿಂಭಾಗ(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    ret = inflate_ಹಿಂಭಾಗ_ಕೊನೆ(Z_NULL);               ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    fputs("inflateBack bad parameters\n", stderr);

    mem_ಸಿದ್ಧತೆ(&strm);
    ret = inflate_ಹಿಂಭಾಗ_Init(&strm, 15, win);      ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    strm.avail_ಒಳ = 2;
    strm.ಮುಂದಿನ_ಒಳ = (ಶೂನ್ಯ *)"\x03";
    ret = inflate_ಹಿಂಭಾಗ(&strm, pull, Z_NULL, ತಳ್ಳು, Z_NULL);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ಕೊನೆ);
        /* force output error */
    strm.avail_ಒಳ = 3;
    strm.ಮುಂದಿನ_ಒಳ = (ಶೂನ್ಯ *)"\x63\x00";
    ret = inflate_ಹಿಂಭಾಗ(&strm, pull, Z_NULL, ತಳ್ಳು, &strm);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflate_ಹಿಂಭಾಗ(&strm, pull, &strm, ತಳ್ಳು, Z_NULL);
                                                ಪ್ರತಿಜ್ಞಾ(ret == Z_ಹರಿವು_ERROR);
    ret = inflate_ಹಿಂಭಾಗ_ಕೊನೆ(&strm);                ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    mem_ಮುಗಿದ(&strm, "inflateBack bad state");

    ret = inflate_ಹಿಂಭಾಗ_Init(&strm, 15, win);      ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    ret = inflate_ಹಿಂಭಾಗ_ಕೊನೆ(&strm);                ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    fputs("inflateBack built-in memory routines\n", stderr);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local ಪೂರ್ಣಾಂಕ try(ಅಕ್ಷರ *hex, ಅಕ್ಷರ *id, ಪೂರ್ಣಾಂಕ err)
{
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ len, ಗಾತ್ರ_2;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ, *ಹೊರಹೋಗು, *win;
    ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ;
    z_ಹರಿವು strm;

    /* convert to hex */
    ಒಳ = h2b(hex, &len);
    ಪ್ರತಿಜ್ಞಾ(ಒಳ != ಖಾಲಿ);

    /* allocate work areas */
    ಗಾತ್ರ_2 = len << 3;
    ಹೊರಹೋಗು = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
    ಪ್ರತಿಜ್ಞಾ(ಹೊರಹೋಗು != ಖಾಲಿ);
    win = ಸ್ಮರಣೆ_ಕೊಡು(32768);
    ಪ್ರತಿಜ್ಞಾ(win != ಖಾಲಿ);
    ಪೂರ್ವಪ್ರತ್ಯಯ = ಸ್ಮರಣೆ_ಕೊಡು(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(id) + 6);
    ಪ್ರತಿಜ್ಞಾ(ಪೂರ್ವಪ್ರತ್ಯಯ != ಖಾಲಿ);

    /* first with inflate */
    strcpy(ಪೂರ್ವಪ್ರತ್ಯಯ, id);
    strcat(ಪೂರ್ವಪ್ರತ್ಯಯ, "-late");
    mem_ಸಿದ್ಧತೆ(&strm);
    strm.avail_ಒಳ = 0;
    strm.ಮುಂದಿನ_ಒಳ = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
    strm.avail_ಒಳ = len;
    strm.ಮುಂದಿನ_ಒಳ = ಒಳ;
    ಮಾಡು {
        strm.avail_ಹೊರಹೋಗು = ಗಾತ್ರ_2;
        strm.ಮುಂದಿನ_ಹೊರಹೋಗು = ಹೊರಹೋಗು;
        ret = inflate(&strm, Z_TREES);
        ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR && ret != Z_MEM_ERROR);
        ಆದರೆ (ret == Z_ದತ್ತಾಂಶ_ERROR || ret == Z_NEED_DICT)
            ನಿಲ್ಲಿಸು;
    } ಆಗ (strm.avail_ಒಳ || strm.avail_ಹೊರಹೋಗು == 0);
    ಆದರೆ (err) {
        ಪ್ರತಿಜ್ಞಾ(ret == Z_ದತ್ತಾಂಶ_ERROR);
        ಪ್ರತಿಜ್ಞಾ(strcmp(id, strm.ಸಂದೇಶ) == 0);
    }
    inflate_ಕೊನೆ(&strm);
    mem_ಮುಗಿದ(&strm, ಪೂರ್ವಪ್ರತ್ಯಯ);

    /* then with inflateBack */
    ಆದರೆ (err >= 0) {
        strcpy(ಪೂರ್ವಪ್ರತ್ಯಯ, id);
        strcat(ಪೂರ್ವಪ್ರತ್ಯಯ, "-back");
        mem_ಸಿದ್ಧತೆ(&strm);
        ret = inflate_ಹಿಂಭಾಗ_Init(&strm, 15, win);
        ಪ್ರತಿಜ್ಞಾ(ret == Z_OK);
        strm.avail_ಒಳ = len;
        strm.ಮುಂದಿನ_ಒಳ = ಒಳ;
        ret = inflate_ಹಿಂಭಾಗ(&strm, pull, Z_NULL, ತಳ್ಳು, Z_NULL);
        ಪ್ರತಿಜ್ಞಾ(ret != Z_ಹರಿವು_ERROR);
        ಆದರೆ (err) {
            ಪ್ರತಿಜ್ಞಾ(ret == Z_ದತ್ತಾಂಶ_ERROR);
            ಪ್ರತಿಜ್ಞಾ(strcmp(id, strm.ಸಂದೇಶ) == 0);
        }
        inflate_ಹಿಂಭಾಗ_ಕೊನೆ(&strm);
        mem_ಮುಗಿದ(&strm, ಪೂರ್ವಪ್ರತ್ಯಯ);
    }

    /* clean up */
    ಬಿಡುಗಡೆ(ಪೂರ್ವಪ್ರತ್ಯಯ);
    ಬಿಡುಗಡೆ(win);
    ಬಿಡುಗಡೆ(ಹೊರಹೋಗು);
    ಬಿಡುಗಡೆ(ಒಳ);
    ಹಿಂತಿರುಗಿ ret;
}

/* cover deflate data cases in both inflate() and inflateBack() */
local ಶೂನ್ಯ cover_inflate(ಶೂನ್ಯ)
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
        Z_ಹರಿವು_ಕೊನೆ);
    inf("63 18 5 40 c 0", "window wrap", 3, -8, 300, Z_OK);
}

/* cover remaining lines in inftrees.c */
local ಶೂನ್ಯ cover_trees(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ ret;
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು;
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ lens[16], work[16];
    ಸಂಕೇತ_2 *ಮುಂದಿನ, ಕೋಷ್ಟಕ_2[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    ಗಾಗಿ (ಬಿಟ್ಟುಗಳು = 0; ಬಿಟ್ಟುಗಳು < 15; ಬಿಟ್ಟುಗಳು++)
        lens[ಬಿಟ್ಟುಗಳು] = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ)(ಬಿಟ್ಟುಗಳು + 1);
    lens[15] = 15;
    ಮುಂದಿನ = ಕೋಷ್ಟಕ_2;
    ಬಿಟ್ಟುಗಳು = 15;
    ret = inflate_ಕೋಷ್ಟಕ(DISTS, lens, 16, &ಮುಂದಿನ, &ಬಿಟ್ಟುಗಳು, work);
                                                ಪ್ರತಿಜ್ಞಾ(ret == 1);
    ಮುಂದಿನ = ಕೋಷ್ಟಕ_2;
    ಬಿಟ್ಟುಗಳು = 1;
    ret = inflate_ಕೋಷ್ಟಕ(DISTS, lens, 16, &ಮುಂದಿನ, &ಬಿಟ್ಟುಗಳು, work);
                                                ಪ್ರತಿಜ್ಞಾ(ret == 1);
    fputs("inflate_table not enough errors\n", stderr);
}

/* cover remaining inffast.c decoding and window copying */
local ಶೂನ್ಯ cover_fast(ಶೂನ್ಯ)
{
    inf("e5 e0 81 ad 6d cb b2 2c c9 01 1e 59 63 ae 7d ee fb 4d fd b5 35 41 68"
        " ff 7f 0f 0 0 0", "fast length extra bits", 0, -8, 258, Z_ದತ್ತಾಂಶ_ERROR);
    inf("25 fd 81 b5 6d 59 b6 6a 49 ea af 35 6 34 eb 8c b9 f6 b9 1e ef 67 49"
        " 50 fe ff ff 3f 0 0", "fast distance extra bits", 0, -8, 258,
        Z_ದತ್ತಾಂಶ_ERROR);
    inf("3 7e 0 0 0 0 0", "fast invalid distance code", 0, -8, 258,
        Z_ದತ್ತಾಂಶ_ERROR);
    inf("1b 7 0 0 0 0 0", "fast invalid literal/length code", 0, -8, 258,
        Z_ದತ್ತಾಂಶ_ERROR);
    inf("d c7 1 ae eb 38 c 4 41 a0 87 72 de df fb 1f b8 36 b1 38 5d ff ff 0",
        "fast 2nd level codes and too far back", 0, -8, 258, Z_ದತ್ತಾಂಶ_ERROR);
    inf("63 18 5 8c 10 8 0 0 0 0", "very common case", 0, -8, 259, Z_OK);
    inf("63 60 60 18 c9 0 8 18 18 18 26 c0 28 0 29 0 0 0",
        "contiguous and wrap around window", 6, -8, 259, Z_OK);
    inf("63 0 3 0 0 0 0 0", "copy direct from output", 0, -8, 259,
        Z_ಹರಿವು_ಕೊನೆ);
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ)
{
    fprintf(stderr, "%s\n", zlib_ಆವೃತ್ತಿ());
    cover_support();
    cover_wrap();
    cover_ಹಿಂಭಾಗ();
    cover_inflate();
    cover_trees();
    cover_fast();
    ಹಿಂತಿರುಗಿ 0;
}
