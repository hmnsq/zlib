/* infcover.c -- test zlib's inflate routines with full code coverage
 * Copyright (C) 2011, 2016, 2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* to use, do: ./configure --cover && make cover */

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव "zlib.h"

/* get definition of internal structure so we can mess with it (see pull()),
   and so we can call inflate_trees() (see cover5()) */
#परिभाषय ZLIB_आन्तरिकम्
#अन्तर्भाव "inftrees.h"
#अन्तर्भाव "inflate.h"

#परिभाषय local स्थिर

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
संरचना mem_item {
    शून्य *ptr;                  /* pointer to allocated memory */
    size_t आकार;                /* requested size of allocation */
    संरचना mem_item *अग्र;      /* pointer to next item in list, or NULL */
};

/* this structure is at the root of the linked list, and tracks statistics */
संरचना mem_zone {
    संरचना mem_item *first;     /* pointer to first item in list, or NULL */
    size_t योग_2, highwater;    /* total allocations, and largest total */
    size_t सीमा;               /* memory allocation limit, or 0 if no limit */
    पूर्णाङ्क notlifo, rogue;         /* counts of non-LIFO frees and rogue frees */
};

/* memory allocation routine to pass to zlib */
local शून्य *mem_alloc(शून्य *mem, अचिह्नित गणनम्, अचिह्नित आकार)
{
    शून्य *ptr;
    संरचना mem_item *item;
    संरचना mem_zone *zone = mem;
    size_t len = गणनम् * (size_t)आकार;

    /* induced allocation failure */
    यदि (zone == रिक्त || (zone->सीमा && zone->योग_2 + len > zone->सीमा))
        प्रत्यागम रिक्त;

    /* perform allocation using the standard library, fill memory with a
       non-zero value to make sure that the code isn't depending on zeros */
    ptr = स्मृतिं_देहि(len);
    यदि (ptr == रिक्त)
        प्रत्यागम रिक्त;
    स्मृतिं_पूरय(ptr, 0xa5, len);

    /* create a new item for the list */
    item = स्मृतिं_देहि(परिमाण(संरचना mem_item));
    यदि (item == रिक्त) {
        मुक्त(ptr);
        प्रत्यागम रिक्त;
    }
    item->ptr = ptr;
    item->आकार = len;

    /* insert item at the beginning of the list */
    item->अग्र = zone->first;
    zone->first = item;

    /* update the statistics */
    zone->योग_2 += item->आकार;
    यदि (zone->योग_2 > zone->highwater)
        zone->highwater = zone->योग_2;

    /* return the allocated memory */
    प्रत्यागम ptr;
}

/* memory free routine to pass to zlib */
local शून्य mem_free(शून्य *mem, शून्य *ptr)
{
    संरचना mem_item *item, *अग्र;
    संरचना mem_zone *zone = mem;

    /* if no zone, just do a free */
    यदि (zone == रिक्त) {
        मुक्त(ptr);
        प्रत्यागम;
    }

    /* point next to the item that matches ptr, or NULL if not found -- remove
       the item from the linked list if found */
    अग्र = zone->first;
    यदि (अग्र) {
        यदि (अग्र->ptr == ptr)
            zone->first = अग्र->अग्र;   /* first one is it, remove from list */
        अन्यथा {
            कुरु {                        /* search the linked list */
                item = अग्र;
                अग्र = item->अग्र;
            } यावत् (अग्र != रिक्त && अग्र->ptr != ptr);
            यदि (अग्र) {                 /* if found, remove from linked list */
                item->अग्र = अग्र->अग्र;
                zone->notlifo++;        /* not a LIFO free */
            }

        }
    }

    /* if found, update the statistics and free the item */
    यदि (अग्र) {
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
local शून्य mem_सज्जता(z_प्रवाहः *strm)
{
    संरचना mem_zone *zone;

    zone = स्मृतिं_देहि(परिमाण(संरचना mem_zone));
    प्रतिज्ञा(zone != रिक्त);
    zone->first = रिक्त;
    zone->योग_2 = 0;
    zone->highwater = 0;
    zone->सीमा = 0;
    zone->notlifo = 0;
    zone->rogue = 0;
    strm->opaque = zone;
    strm->zalloc = mem_alloc;
    strm->zfree = mem_free;
}

/* set a limit on the total memory allocation, or 0 to remove the limit */
local शून्य mem_सीमा(z_प्रवाहः *strm, size_t सीमा)
{
    संरचना mem_zone *zone = strm->opaque;

    zone->सीमा = सीमा;
}

/* show the current total requested allocations in bytes */
local शून्य mem_प्रयुक्तम्(z_प्रवाहः *strm, वर्ण *उपसर्गः)
{
    संरचना mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu allocated\n", उपसर्गः, zone->योग_2);
}

/* show the high water allocation in bytes */
local शून्य mem_उच्चः(z_प्रवाहः *strm, वर्ण *उपसर्गः)
{
    संरचना mem_zone *zone = strm->opaque;

    fprintf(stderr, "%s: %zu high water mark\n", उपसर्गः, zone->highwater);
}

/* release the memory allocation zone -- if there are any surprises, notify */
local शून्य mem_समाप्तम्(z_प्रवाहः *strm, वर्ण *उपसर्गः)
{
    पूर्णाङ्क गणनम् = 0;
    संरचना mem_item *item, *अग्र;
    संरचना mem_zone *zone = strm->opaque;

    /* show high water mark */
    mem_उच्चः(strm, उपसर्गः);

    /* free leftover allocations and item structures, if any */
    item = zone->first;
    यावत् (item != रिक्त) {
        मुक्त(item->ptr);
        अग्र = item->अग्र;
        मुक्त(item);
        item = अग्र;
        गणनम्++;
    }

    /* issue alerts about anything unexpected */
    यदि (गणनम् || zone->योग_2)
        fprintf(stderr, "** %s: %zu bytes in %d blocks not freed\n",
                उपसर्गः, zone->योग_2, गणनम्);
    यदि (zone->notlifo)
        fprintf(stderr, "** %s: %d frees not LIFO\n", उपसर्गः, zone->notlifo);
    यदि (zone->rogue)
        fprintf(stderr, "** %s: %d frees not recognized\n",
                उपसर्गः, zone->rogue);

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
local अचिह्नित वर्ण *h2b(ध्रुव वर्ण *hex, अचिह्नित *len)
{
    अचिह्नित वर्ण *अन्तर्, *re;
    अचिह्नित अग्र, val;

    अन्तर् = स्मृतिं_देहि((वाक्यदैर्घ्यम्(hex) + 1) >> 1);
    यदि (अन्तर् == रिक्त)
        प्रत्यागम रिक्त;
    अग्र = 0;
    val = 1;
    कुरु {
        यदि (*hex >= '0' && *hex <= '9')
            val = (val << 4) + *hex - '0';
        अन्यथा यदि (*hex >= 'A' && *hex <= 'F')
            val = (val << 4) + *hex - 'A' + 10;
        अन्यथा यदि (*hex >= 'a' && *hex <= 'f')
            val = (val << 4) + *hex - 'a' + 10;
        अन्यथा यदि (val != 1 && val < 32)  /* one digit followed by delimiter */
            val += 240;                 /* make it look like two digits */
        यदि (val > 255) {                /* have two digits */
            अन्तर्[अग्र++] = val & 0xff;    /* save the decoded byte */
            val = 1;                    /* start over */
        }
    } यावत् (*hex++);       /* go through the loop with the terminating null */
    यदि (len != रिक्त)
        *len = अग्र;
    re = realloc(अन्तर्, अग्र);
    प्रत्यागम re == रिक्त ? अन्तर् : re;
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
local शून्य inf(वर्ण *hex, वर्ण *what, अचिह्नित चरणः, पूर्णाङ्क win, अचिह्नित len,
               पूर्णाङ्क err)
{
    पूर्णाङ्क ret;
    अचिह्नित have;
    अचिह्नित वर्ण *अन्तर्, *बहिर्गमनम्;
    z_प्रवाहः strm, copy;
    gz_शीर्षकम् शीर्ष;

    mem_सज्जता(&strm);
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit2(&strm, win);
    यदि (ret != Z_OK) {
        mem_समाप्तम्(&strm, what);
        प्रत्यागम;
    }
    बहिर्गमनम् = स्मृतिं_देहि(len);                          प्रतिज्ञा(बहिर्गमनम् != रिक्त);
    यदि (win == 47) {
        शीर्ष.extra = बहिर्गमनम्;
        शीर्ष.extra_max = len;
        शीर्ष.name = बहिर्गमनम्;
        शीर्ष.name_max = len;
        शीर्ष.comment = बहिर्गमनम्;
        शीर्ष.comm_max = len;
        ret = inflate_Get_शीर्षकम्(&strm, &शीर्ष);   प्रतिज्ञा(ret == Z_OK);
    }
    अन्तर् = h2b(hex, &have);                       प्रतिज्ञा(अन्तर् != रिक्त);
    यदि (चरणः == 0 || चरणः > have)
        चरणः = have;
    strm.avail_अन्तर् = चरणः;
    have -= चरणः;
    strm.अग्र_अन्तर् = अन्तर्;
    कुरु {
        strm.avail_बहिर्गमनम् = len;
        strm.अग्र_बहिर्गमनम् = बहिर्गमनम्;
        ret = inflate(&strm, Z_NO_FLUSH);       प्रतिज्ञा(err == 9 || ret == err);
        यदि (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_NEED_DICT)
            भङ्ग;
        यदि (ret == Z_NEED_DICT) {
            ret = inflate_समुच्चयः_Dictionary(&strm, अन्तर्, 1);
                                                प्रतिज्ञा(ret == Z_दत्त_ERROR);
            mem_सीमा(&strm, 1);
            ret = inflate_समुच्चयः_Dictionary(&strm, बहिर्गमनम्, 0);
                                                प्रतिज्ञा(ret == Z_MEM_ERROR);
            mem_सीमा(&strm, 0);
            ((संरचना inflate_state *)strm.state)->बहुलकम् = DICT;
            ret = inflate_समुच्चयः_Dictionary(&strm, बहिर्गमनम्, 0);
                                                प्रतिज्ञा(ret == Z_OK);
            ret = inflate(&strm, Z_NO_FLUSH);   प्रतिज्ञा(ret == Z_BUF_ERROR);
        }
        ret = inflateCopy(&copy, &strm);        प्रतिज्ञा(ret == Z_OK);
        ret = inflate_अन्तः(&copy);                प्रतिज्ञा(ret == Z_OK);
        err = 9;                        /* don't care next time around */
        have += strm.avail_अन्तर्;
        strm.avail_अन्तर् = चरणः > have ? have : चरणः;
        have -= strm.avail_अन्तर्;
    } यावत् (strm.avail_अन्तर्);
    मुक्त(अन्तर्);
    मुक्त(बहिर्गमनम्);
    ret = inflateReset2(&strm, -8);             प्रतिज्ञा(ret == Z_OK);
    ret = inflate_अन्तः(&strm);                    प्रतिज्ञा(ret == Z_OK);
    mem_समाप्तम्(&strm, what);
}

/* cover all of the lines in inflate.c up to inflate() */
local शून्य cover_support(शून्य)
{
    पूर्णाङ्क ret;
    z_प्रवाहः strm;

    mem_सज्जता(&strm);
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit(&strm);                   प्रतिज्ञा(ret == Z_OK);
    mem_प्रयुक्तम्(&strm, "inflate init");
    ret = inflate_अविभाज्यः(&strm, 5, 31);           प्रतिज्ञा(ret == Z_OK);
    ret = inflate_अविभाज्यः(&strm, -1, 0);           प्रतिज्ञा(ret == Z_OK);
    ret = inflate_समुच्चयः_Dictionary(&strm, Z_NULL, 0);
                                                प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    ret = inflate_अन्तः(&strm);                    प्रतिज्ञा(ret == Z_OK);
    mem_समाप्तम्(&strm, "prime");

    inf("63 0", "force window allocation", 0, -15, 1, Z_OK);
    inf("63 18 5", "force window replacement", 0, -8, 259, Z_OK);
    inf("63 18 68 30 d0 0 0", "force split window update", 4, -8, 259, Z_OK);
    inf("3 0", "use fixed blocks", 0, -15, 1, Z_प्रवाहः_अन्तः);
    inf("", "bad window size", 0, 1, 0, Z_प्रवाहः_ERROR);

    mem_सज्जता(&strm);
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit_(&strm, "!", (पूर्णाङ्क)परिमाण(z_प्रवाहः));
                                                प्रतिज्ञा(ret == Z_संस्करणम्_ERROR);
    mem_समाप्तम्(&strm, "wrong version");

    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit(&strm);                   प्रतिज्ञा(ret == Z_OK);
    ret = inflate_अन्तः(&strm);                    प्रतिज्ञा(ret == Z_OK);
    fputs("inflate built-in memory routines\n", stderr);
}

/* cover all inflate() header and trailer cases and code after inflate() */
local शून्य cover_wrap(शून्य)
{
    पूर्णाङ्क ret;
    z_प्रवाहः strm, copy;
    अचिह्नित वर्ण dict[257];

    ret = inflate(Z_NULL, 0);                   प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    ret = inflate_अन्तः(Z_NULL);                   प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    ret = inflateCopy(Z_NULL, Z_NULL);          प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    fputs("inflate bad parameters\n", stderr);

    inf("1f 8b 0 0", "bad gzip method", 0, 31, 0, Z_दत्त_ERROR);
    inf("1f 8b 8 80", "bad gzip flags", 0, 31, 0, Z_दत्त_ERROR);
    inf("77 85", "bad zlib method", 0, 15, 0, Z_दत्त_ERROR);
    inf("8 99", "set window size from header", 0, 0, 0, Z_OK);
    inf("78 9c", "bad zlib window size", 0, 8, 0, Z_दत्त_ERROR);
    inf("78 9c 63 0 0 0 1 0 1", "check adler32", 0, 15, 1, Z_प्रवाहः_अन्तः);
    inf("1f 8b 8 1e 0 0 0 0 0 0 1 0 0 0 0 0 0", "bad header crc", 0, 47, 1,
        Z_दत्त_ERROR);
    inf("1f 8b 8 2 0 0 0 0 0 0 1d 26 3 0 0 0 0 0 0 0 0 0", "check gzip length",
        0, 47, 0, Z_प्रवाहः_अन्तः);
    inf("78 90", "bad zlib header check", 0, 47, 0, Z_दत्त_ERROR);
    inf("8 b8 0 0 0 1", "need dictionary", 0, 8, 0, Z_NEED_DICT);
    inf("78 9c 63 0", "compute adler32", 0, 15, 1, Z_OK);

    mem_सज्जता(&strm);
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit2(&strm, -8);
    strm.avail_अन्तर् = 2;
    strm.अग्र_अन्तर् = (शून्य *)"\x63";
    strm.avail_बहिर्गमनम् = 1;
    strm.अग्र_बहिर्गमनम् = (शून्य *)&ret;
    mem_सीमा(&strm, 1);
    ret = inflate(&strm, Z_NO_FLUSH);           प्रतिज्ञा(ret == Z_MEM_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           प्रतिज्ञा(ret == Z_MEM_ERROR);
    mem_सीमा(&strm, 0);
    स्मृतिं_पूरय(dict, 0, 257);
    ret = inflate_समुच्चयः_Dictionary(&strm, dict, 257);
                                                प्रतिज्ञा(ret == Z_OK);
    mem_सीमा(&strm, (परिमाण(संरचना inflate_state) << 1) + 256);
    ret = inflate_अविभाज्यः(&strm, 16, 0);           प्रतिज्ञा(ret == Z_OK);
    strm.avail_अन्तर् = 2;
    strm.अग्र_अन्तर् = (शून्य *)"\x80";
    ret = inflateSync(&strm);                   प्रतिज्ञा(ret == Z_दत्त_ERROR);
    ret = inflate(&strm, Z_NO_FLUSH);           प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    strm.avail_अन्तर् = 4;
    strm.अग्र_अन्तर् = (शून्य *)"\0\0\xff\xff";
    ret = inflateSync(&strm);                   प्रतिज्ञा(ret == Z_OK);
    (शून्य)inflateSyncPoint(&strm);
    ret = inflateCopy(&copy, &strm);            प्रतिज्ञा(ret == Z_MEM_ERROR);
    mem_सीमा(&strm, 0);
    ret = inflateUndermine(&strm, 1);           प्रतिज्ञा(ret == Z_दत्त_ERROR);
    (शून्य)inflateMark(&strm);
    ret = inflate_अन्तः(&strm);                    प्रतिज्ञा(ret == Z_OK);
    mem_समाप्तम्(&strm, "miscellaneous, force memory errors");
}

/* input and output functions for inflateBack() */
local अचिह्नित pull(शून्य *desc, अचिह्नित वर्ण z_const **buf)
{
    स्थिर अचिह्नित पूर्णाङ्क अग्र = 0;
    स्थिर अचिह्नित वर्ण dat[] = {0x63, 0, 2, 0};
    संरचना inflate_state *state;

    यदि (desc == Z_NULL) {
        अग्र = 0;
        प्रत्यागम 0;   /* no input (already provided at next_in) */
    }
    state = (शून्य *)((z_प्रवाहः *)desc)->state;
    यदि (state != Z_NULL)
        state->बहुलकम् = SYNC;     /* force an otherwise impossible situation */
    प्रत्यागम अग्र < परिमाण(dat) ? (*buf = dat + अग्र++, 1) : 0;
}

local पूर्णाङ्क प्रक्षिप(शून्य *desc, अचिह्नित वर्ण *buf, अचिह्नित len)
{
    (शून्य)buf;
    (शून्य)len;
    प्रत्यागम desc != Z_NULL;      /* force error if desc not null */
}

/* cover inflateBack() up to common deflate data cases and after those */
local शून्य cover_पृष्ठभागः(शून्य)
{
    पूर्णाङ्क ret;
    z_प्रवाहः strm;
    अचिह्नित वर्ण win[32768];

    ret = inflate_पृष्ठभागः_Init_2(Z_NULL, 0, win, 0, 0);
                                                प्रतिज्ञा(ret == Z_संस्करणम्_ERROR);
    ret = inflate_पृष्ठभागः_Init(Z_NULL, 0, win);      प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    ret = inflate_पृष्ठभागः(Z_NULL, Z_NULL, Z_NULL, Z_NULL, Z_NULL);
                                                प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    ret = inflate_पृष्ठभागः_अन्तः(Z_NULL);               प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    fputs("inflateBack bad parameters\n", stderr);

    mem_सज्जता(&strm);
    ret = inflate_पृष्ठभागः_Init(&strm, 15, win);      प्रतिज्ञा(ret == Z_OK);
    strm.avail_अन्तर् = 2;
    strm.अग्र_अन्तर् = (शून्य *)"\x03";
    ret = inflate_पृष्ठभागः(&strm, pull, Z_NULL, प्रक्षिप, Z_NULL);
                                                प्रतिज्ञा(ret == Z_प्रवाहः_अन्तः);
        /* force output error */
    strm.avail_अन्तर् = 3;
    strm.अग्र_अन्तर् = (शून्य *)"\x63\x00";
    ret = inflate_पृष्ठभागः(&strm, pull, Z_NULL, प्रक्षिप, &strm);
                                                प्रतिज्ञा(ret == Z_BUF_ERROR);
        /* force mode error by mucking with state */
    ret = inflate_पृष्ठभागः(&strm, pull, &strm, प्रक्षिप, Z_NULL);
                                                प्रतिज्ञा(ret == Z_प्रवाहः_ERROR);
    ret = inflate_पृष्ठभागः_अन्तः(&strm);                प्रतिज्ञा(ret == Z_OK);
    mem_समाप्तम्(&strm, "inflateBack bad state");

    ret = inflate_पृष्ठभागः_Init(&strm, 15, win);      प्रतिज्ञा(ret == Z_OK);
    ret = inflate_पृष्ठभागः_अन्तः(&strm);                प्रतिज्ञा(ret == Z_OK);
    fputs("inflateBack built-in memory routines\n", stderr);
}

/* do a raw inflate of data in hexadecimal with both inflate and inflateBack */
local पूर्णाङ्क try(वर्ण *hex, वर्ण *id, पूर्णाङ्क err)
{
    पूर्णाङ्क ret;
    अचिह्नित len, आकार;
    अचिह्नित वर्ण *अन्तर्, *बहिर्गमनम्, *win;
    वर्ण *उपसर्गः;
    z_प्रवाहः strm;

    /* convert to hex */
    अन्तर् = h2b(hex, &len);
    प्रतिज्ञा(अन्तर् != रिक्त);

    /* allocate work areas */
    आकार = len << 3;
    बहिर्गमनम् = स्मृतिं_देहि(आकार);
    प्रतिज्ञा(बहिर्गमनम् != रिक्त);
    win = स्मृतिं_देहि(32768);
    प्रतिज्ञा(win != रिक्त);
    उपसर्गः = स्मृतिं_देहि(वाक्यदैर्घ्यम्(id) + 6);
    प्रतिज्ञा(उपसर्गः != रिक्त);

    /* first with inflate */
    वाक्यं_प्रतिलिख(उपसर्गः, id);
    वाक्यं_योजय(उपसर्गः, "-late");
    mem_सज्जता(&strm);
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    ret = inflateInit2(&strm, err < 0 ? 47 : -15);
    प्रतिज्ञा(ret == Z_OK);
    strm.avail_अन्तर् = len;
    strm.अग्र_अन्तर् = अन्तर्;
    कुरु {
        strm.avail_बहिर्गमनम् = आकार;
        strm.अग्र_बहिर्गमनम् = बहिर्गमनम्;
        ret = inflate(&strm, Z_TREES);
        प्रतिज्ञा(ret != Z_प्रवाहः_ERROR && ret != Z_MEM_ERROR);
        यदि (ret == Z_दत्त_ERROR || ret == Z_NEED_DICT)
            भङ्ग;
    } यावत् (strm.avail_अन्तर् || strm.avail_बहिर्गमनम् == 0);
    यदि (err) {
        प्रतिज्ञा(ret == Z_दत्त_ERROR);
        प्रतिज्ञा(वाक्यं_तुलय(id, strm.सन्देशः) == 0);
    }
    inflate_अन्तः(&strm);
    mem_समाप्तम्(&strm, उपसर्गः);

    /* then with inflateBack */
    यदि (err >= 0) {
        वाक्यं_प्रतिलिख(उपसर्गः, id);
        वाक्यं_योजय(उपसर्गः, "-back");
        mem_सज्जता(&strm);
        ret = inflate_पृष्ठभागः_Init(&strm, 15, win);
        प्रतिज्ञा(ret == Z_OK);
        strm.avail_अन्तर् = len;
        strm.अग्र_अन्तर् = अन्तर्;
        ret = inflate_पृष्ठभागः(&strm, pull, Z_NULL, प्रक्षिप, Z_NULL);
        प्रतिज्ञा(ret != Z_प्रवाहः_ERROR);
        यदि (err) {
            प्रतिज्ञा(ret == Z_दत्त_ERROR);
            प्रतिज्ञा(वाक्यं_तुलय(id, strm.सन्देशः) == 0);
        }
        inflate_पृष्ठभागः_अन्तः(&strm);
        mem_समाप्तम्(&strm, उपसर्गः);
    }

    /* clean up */
    मुक्त(उपसर्गः);
    मुक्त(win);
    मुक्त(बहिर्गमनम्);
    मुक्त(अन्तर्);
    प्रत्यागम ret;
}

/* cover deflate data cases in both inflate() and inflateBack() */
local शून्य cover_inflate(शून्य)
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
        Z_प्रवाहः_अन्तः);
    inf("63 18 5 40 c 0", "window wrap", 3, -8, 300, Z_OK);
}

/* cover remaining lines in inftrees.c */
local शून्य cover_trees(शून्य)
{
    पूर्णाङ्क ret;
    अचिह्नित द्वयङ्काः;
    अचिह्नित ह्रस्व lens[16], work[16];
    संहिता *अग्र, सारणी_2[ENOUGH_DISTS];

    /* we need to call inflate_table() directly in order to manifest not-
       enough errors, since zlib insures that enough is always enough */
    प्रति (द्वयङ्काः = 0; द्वयङ्काः < 15; द्वयङ्काः++)
        lens[द्वयङ्काः] = (अचिह्नित ह्रस्व)(द्वयङ्काः + 1);
    lens[15] = 15;
    अग्र = सारणी_2;
    द्वयङ्काः = 15;
    ret = inflate_सारणी(DISTS, lens, 16, &अग्र, &द्वयङ्काः, work);
                                                प्रतिज्ञा(ret == 1);
    अग्र = सारणी_2;
    द्वयङ्काः = 1;
    ret = inflate_सारणी(DISTS, lens, 16, &अग्र, &द्वयङ्काः, work);
                                                प्रतिज्ञा(ret == 1);
    fputs("inflate_table not enough errors\n", stderr);
}

/* cover remaining inffast.c decoding and window copying */
local शून्य cover_fast(शून्य)
{
    inf("e5 e0 81 ad 6d cb b2 2c c9 01 1e 59 63 ae 7d ee fb 4d fd b5 35 41 68"
        " ff 7f 0f 0 0 0", "fast length extra bits", 0, -8, 258, Z_दत्त_ERROR);
    inf("25 fd 81 b5 6d 59 b6 6a 49 ea af 35 6 34 eb 8c b9 f6 b9 1e ef 67 49"
        " 50 fe ff ff 3f 0 0", "fast distance extra bits", 0, -8, 258,
        Z_दत्त_ERROR);
    inf("3 7e 0 0 0 0 0", "fast invalid distance code", 0, -8, 258,
        Z_दत्त_ERROR);
    inf("1b 7 0 0 0 0 0", "fast invalid literal/length code", 0, -8, 258,
        Z_दत्त_ERROR);
    inf("d c7 1 ae eb 38 c 4 41 a0 87 72 de df fb 1f b8 36 b1 38 5d ff ff 0",
        "fast 2nd level codes and too far back", 0, -8, 258, Z_दत्त_ERROR);
    inf("63 18 5 8c 10 8 0 0 0 0", "very common case", 0, -8, 259, Z_OK);
    inf("63 60 60 18 c9 0 8 18 18 18 26 c0 28 0 29 0 0 0",
        "contiguous and wrap around window", 6, -8, 259, Z_OK);
    inf("63 0 3 0 0 0 0 0", "copy direct from output", 0, -8, 259,
        Z_प्रवाहः_अन्तः);
}

पूर्णाङ्क मुख्यम्(शून्य)
{
    fprintf(stderr, "%s\n", zlib_संस्करणम्());
    cover_support();
    cover_wrap();
    cover_पृष्ठभागः();
    cover_inflate();
    cover_trees();
    cover_fast();
    प्रत्यागम 0;
}
