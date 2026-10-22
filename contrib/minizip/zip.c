/* zip.c -- IO on .zip files using zlib
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

         Changes
   Oct-2009 - Mathias Svensson - Remove old C style function prototypes
   Oct-2009 - Mathias Svensson - Added Zip64 Support when creating new file archives
   Oct-2009 - Mathias Svensson - Did some code cleanup and refactoring to get better overview of some functions.
   Oct-2009 - Mathias Svensson - Added zipRemoveExtraInfoBlock to strip extra field data from its ZIP64 data
                                 It is used when recreating zip archive with RAW when deleting items from a zip.
                                 ZIP64 data is automatically added to items that needs it, and existing ZIP64 data need to be removed.
   Oct-2009 - Mathias Svensson - Added support for BZIP2 as compression mode (bzip2 lib is required)
   Jan-2010 - back to unzip and minizip 1.0 name scheme, with compatibility layer

*/


#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ifndef ZLIB_CONST
#  define ZLIB_CONST
#endif
#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#ଅନ୍ତର୍ଭାଵ "zlib.h"
#ଅନ୍ତର୍ଭାଵ "zip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା errno;
#ଅନ୍ୟଥା
#   include <errno.h>
#endif


#ifndef local
#  define local ସ୍ଥିର
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#endif

#ifndef Z_BUFSIZE
#define Z_BUFSIZE (64*1024) /* (16384) */
#endif

#ifndef Z_MAXFILENAMEINZIP
#define Z_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#define MAKEULONG64(a, b) ((ZPOS64_T)(((unsigned long)(a)) | ((ZPOS64_T)((unsigned long)(b))) << 32))

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

#ifndef DEF_MEM_LEVEL
#ଯଦି MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ଅନ୍ୟଥା
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
ଅଚଳ ଅକ୍ଷର zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x6064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା linkedlist_datablock_internal_s
{
  ସଂରଚନା linkedlist_datablock_internal_s* next_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignment */
  unsigned ଅକ୍ଷର data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା linkedlist_data_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_data;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#define SKIPSET_EXPORT ସ୍ଥିର
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅକ୍ଷର *set_ଚାବି_t;
#define set_cmp(a, b) strcmp(a, b)
#define set_drop(s, k) set_free(s, k)
#ଅନ୍ତର୍ଭାଵ "skipset.h"
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    unsigned ଅକ୍ଷର *next;        /* next byte in datablock data */
    size_t ବାମ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_internal *ଗଣ୍ଠି;    /* current datablock */
} block_t;


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା
{
    z_stream stream;            /* zLib stream structure for inflate */
#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_data;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_header;     /* offset of the local header of the file
                                     currently writing */
    ଅକ୍ଷର* central_header;       /* central header data for the current file */
    uLong size_centralExtra;
    uLong size_centralheader;   /* size of the central header for cur file */
    uLong size_centralExtraFree; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା  method;                /* compression method of file currently wr.*/
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା  raw;                   /* 1 for directly writing raw data */
    Byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା  encrypt;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T totalCompressedData;
    ZPOS64_T totalUncompressedData;
#ifndef NOCRYPT
    unsigned long ଚାବିଗୁଡ଼ିକ[3];     /* keys defining the pseudo-random sequence */
    ଅଚଳ z_crc_t* pcrc_32_tab;
    unsigned crypt_header_size;
#endif
} curfile64_info;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_data central_dir;/* datablock with central dir in construction*/
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T add_position_when_writing_offset;
    ZPOS64_T ସଂଖ୍ୟା_entry;

#ifndef NO_ADDFILEINEXISTINGZIP
    ଅକ୍ଷର *globalcomment;
#endif

    /* Support for zipAlreadyThere(). */
    set_t set;              /* set for detecting name collisions */
    block_t block;          /* block for reading the central directory */

} zip64_internal;


#ifndef NOCRYPT
#define INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#ଅନ୍ତର୍ଭାଵ "crypt.h"
#endif

local linkedlist_datablock_internal* allocate_new_datablock(ଶୂନ୍ୟ) {
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(ଆକାର(linkedlist_datablock_internal));
    ଯଦି (ldi!=ଖାଲି)
    {
        ldi->next_datablock = ଖାଲି ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    ଫେରନ୍ତୁ ldi;
}

local ଶୂନ୍ୟ free_datablock(linkedlist_datablock_internal* ldi) {
    ଯେତେବେଳେ (ldi!=ଖାଲି)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        ମୁକ୍ତ(ldi);
        ldi = ldinext;
    }
}

local ଶୂନ୍ୟ init_linkedlist(linkedlist_data* ll) {
    ll->first_block = ll->last_block = ଖାଲି;
}

local ଶୂନ୍ୟ free_linkedlist(linkedlist_data* ll) {
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = ଖାଲି;
}


local ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_data_in_datablock(linkedlist_data* ll, ଅଚଳ ଶୂନ୍ୟ* buf, uLong len) {
    linkedlist_datablock_internal* ldi;
    ଅଚଳ unsigned ଅକ୍ଷର* from_copy;

    ଯଦି (ll==ଖାଲି)
        ଫେରନ୍ତୁ ZIP_INTERNALERROR;

    ଯଦି (ll->last_block == ଖାଲି)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        ଯଦି (ll->first_block == ଖାଲି)
            ଫେରନ୍ତୁ ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (ଅଚଳ unsigned ଅକ୍ଷର*)buf;

    ଯେତେବେଳେ (len>0)
    {
        uInt copy_this;
        uInt i;
        unsigned ଅକ୍ଷର* to_copy;

        ଯଦି (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            ଯଦି (ldi->next_datablock == ଖାଲି)
                ଫେରନ୍ତୁ ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        ଯଦି (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        ଅନ୍ୟଥା
            copy_this = (uInt)len;

        to_copy = &(ldi->data[ldi->filled_in_this_block]);

        ପାଇଁ (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    ଫେରନ୍ତୁ ZIP_OK;
}

/* zipAlreadyThere() operations. "set" in the zip internal structure keeps the
   set of names that are in the under-construction central directory so far. A
   skipset provides ~O(log n) time insertion and searching. Central directory
   records, stored in a linked list of allocated memory datablocks, is read
   through "block" in the zip internal structure.

   The block_*() functions support extracting the central directory file names
   from the datablocks. They are designed to support a growing directory by
   automatically continuing once more data has been appended to the linked
   datablocks.

   Initialize *block to the head of list. This should only be called once the
   list has at least some data in it, i.e. list->first_block is not NULL. */
local ଶୂନ୍ୟ block_init(block_t *block, linkedlist_data *list) {
    block->ଗଣ୍ଠି = list->first_block;
    block->next = block->ଗଣ୍ଠି->data;
    block->ବାମ = block->ଗଣ୍ଠି->filled_in_this_block;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local ଶୂନ୍ୟ block_stop(block_t *block) {
    block->ବାମ = 0;
    block->next = ଖାଲି;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା block_end(block_t *block) {
    linkedlist_datablock_internal *ଗଣ୍ଠି = block->ଗଣ୍ଠି;
    ଯଦି (ଗଣ୍ଠି == ଖାଲି)
        /* This block was previously terminated with extreme prejudice. */
        ଫେରନ୍ତୁ 1;
    ଯଦି (block->next < ଗଣ୍ଠି->data + ଗଣ୍ଠି->filled_in_this_block)
        /* There are more bytes to read in the current datablock. */
        ଫେରନ୍ତୁ 0;
    ଯେତେବେଳେ (ଗଣ୍ଠି->next_datablock != ଖାଲି) {
        ଯଦି (ଗଣ୍ଠି->filled_in_this_block != 0)
            /* There are some bytes in a later datablock. */
            ଫେରନ୍ତୁ 0;
        ଗଣ୍ଠି = ଗଣ୍ଠି->next_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    ଫେରନ୍ତୁ 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା block_get(block_t *block) {
    ଯେତେବେଳେ (block->ବାମ == 0) {
        ଯଦି (block->ଗଣ୍ଠି == ଖାଲି)
            /* We've been marked bad. Return end. */
            ଫେରନ୍ତୁ -1;
        /* Update left in case more was filled in since we were last here. */
        block->ବାମ = block->ଗଣ୍ଠି->filled_in_this_block -
                      (size_t)(block->next - block->ଗଣ୍ଠି->data);
        ଯଦି (block->ବାମ != 0)
            /* There was indeed more data appended in the current datablock. */
            ଭାଙ୍ଗ;
        ଯଦି (block->ଗଣ୍ଠି->next_datablock == ଖାଲି)
            /* No more data here, and there is no next datablock. At the end. */
            ଫେରନ୍ତୁ -1;
        /* Try the next datablock for more data. */
        block->ଗଣ୍ଠି = block->ଗଣ୍ଠି->next_datablock;
        block->next = block->ଗଣ୍ଠି->data;
        block->ବାମ = block->ଗଣ୍ଠି->filled_in_this_block;
    }
    /* We have a byte to return. */
    block->ବାମ--;
    ଫେରନ୍ତୁ *block->next++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local long block_get2(block_t *block) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ତଳ = block_get(block);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଉପର = block_get(block);
    ଫେରନ୍ତୁ ତଳ < 0 || ଉପର < 0 ? -1 : ତଳ | ((long)ଉପର << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t block_read(block_t *block, unsigned ଅକ୍ଷର *buf, size_t len) {
    size_t need = len;
    size_t take;
    ଯେତେବେଳେ (need) {
        ଯଦି (block->ବାମ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା got = block_get(block);
            ଯଦି (got == -1)
                /* Reached the end. */
                ଭାଙ୍ଗ;
            *buf++ = (unsigned ଅକ୍ଷର)got;
            need--;
            ଜାରି;
        }
        take = need > block->ବାମ ? block->ବାମ : need;
        memcpy(buf, block->next, take);
        block->next += take;
        block->ବାମ -= take;
        buf += take;
        need -= take;
    }
    ଫେରନ୍ତୁ len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା block_skip(block_t *block, size_t n) {
    ଯେତେବେଳେ (n > block->ବାମ) {
        n -= block->ବାମ;
        block->next += block->ବାମ;
        block->ବାମ = 0;
        ଯଦି (block_get(block) == -1)
            ଫେରନ୍ତୁ -1;
        n--;
    }
    block->next += n;
    block->ବାମ -= n;
    ଫେରନ୍ତୁ 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local ଅକ୍ଷର *block_central_name(block_t *block, set_t *set) {
    unsigned flen, xlen, clen;
    ଅକ୍ଷର *name = ଖାଲି;
    ପାଇଁ (;;) {
        ଯଦି (block_end(block))
            /* At the end of the central directory (so far). */
            ଫେରନ୍ତୁ ଖାଲି;

        /* Check for a central directory record signature. */
        ଯଦି (block_get2(block) != (CENTRALHEADERMAGIC & 0xffff) ||
            block_get2(block) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            ଭାଙ୍ଗ;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        block_skip(block, 24);
        flen = (unsigned)block_get2(block);    /* file name length */
        xlen = (unsigned)block_get2(block);    /* extra length */
        clen = (unsigned)block_get2(block);    /* comment length */
        ଯଦି (block_skip(block, 12) == -1)
            /* Premature end of the record. */
            ଭାଙ୍ଗ;

        /* Extract the name and skip over the extra and comment fields. */
        name = set_alloc(set, ଖାଲି, flen + 1);
        ଯଦି (block_read(block, (unsigned ଅକ୍ଷର *)name, flen) < flen ||
            block_skip(block, xlen + clen) == -1)
            /* Premature end of the record. */
            ଭାଙ୍ଗ;

        /* Check for embedded nuls in the name. */
        ଯଦି (memchr(name, 0, flen) != ଖାଲି) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            set_free(set, name);
            ଜାରି;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        ଫେରନ୍ତୁ name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    set_free(set, name);
    block_stop(block);
    ଫେରନ୍ତୁ ଖାଲି;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipAlreadyThere(zipFile file, ଅକ୍ଷର ଅଚଳ *name) {
    size_t len;
    ଅକ୍ଷର *copy;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା found;
    zip64_internal *zip = file;
    ଯଦି (zip == ଖାଲି)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;
    ଯଦି (zip->central_dir.first_block == ଖାଲି)
        /* No central directory yet, so no, name isn't there. */
        ଫେରନ୍ତୁ 0;
    ଯଦି (setjmp(zip->set.env)) {
        /* Memory allocation failure. */
        set_end(&zip->set);
        ଫେରନ୍ତୁ -2;
    }
    ଯଦି (!set_ok(&zip->set)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        set_start(&zip->set);
        block_init(&zip->block, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    ପାଇଁ (;;) {
        ଅକ୍ଷର *there = block_central_name(&zip->block, &zip->set);
        ଯଦି (there == ଖାଲି) {
            ଯଦି (zip->block.next == ଖାଲି)
                /* The central directory is invalid. */
                ଫେରନ୍ତୁ -1;
            ଭାଙ୍ଗ;
        }

        /* Add there to the set. */
        ଯଦି (set_ଭର୍ତ୍ତି(&zip->set, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            set_free(&zip->set, there);
    }

    /* Return true if name is in the central directory. */
    len = strlen(name);
    copy = set_alloc(&zip->set, ଖାଲି, len + 1);
    memcpy(copy, name, len + 1);
    found = set_found(&zip->set, copy);
    set_free(&zip->set, copy);
    ଫେରନ୍ତୁ found;
}


/****************************************************************************/

#ifndef NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64local_put_ମୂଲ୍ୟ(ଅଚଳ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nbByte) {
    unsigned ଅକ୍ଷର buf[8];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
    ପାଇଁ (n = 0; n < nbByte; n++)
    {
        buf[n] = (unsigned ଅକ୍ଷର)(x & 0xff);
        x >>= 8;
    }
    ଯଦି (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      ପାଇଁ (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    ଯଦି (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        ଫେରନ୍ତୁ ZIP_ERRNO;
    ଅନ୍ୟଥା
        ଫେରନ୍ତୁ ZIP_OK;
}

local ଶୂନ୍ୟ zip64local_put_ମୂଲ୍ୟ_inmemory (ଶୂନ୍ୟ* dest, ZPOS64_T x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nbByte) {
    unsigned ଅକ୍ଷର* buf=(unsigned ଅକ୍ଷର*)dest;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
    ପାଇଁ (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned ଅକ୍ଷର)(x & 0xff);
        x >>= 8;
    }

    ଯଦି (x != 0)
    {     /* data overflow - hack for ZIP64 */
       ପାଇଁ (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_TmzDateToDosDate(ଅଚଳ tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    ଯଦି (year>=1980)
        year-=1980;
    ଅନ୍ୟଥା ଯଦି (year>=80)
        year-=80;
    ଫେରନ୍ତୁ
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64local_getByte(ଅଚଳ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ପୂର୍ଣ୍ଣସଂଖ୍ୟା* pi) {
    unsigned ଅକ୍ଷର c;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    ଯଦି (err==1)
    {
        *pi = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)c;
        ଫେରନ୍ତୁ ZIP_OK;
    }
    ଅନ୍ୟଥା
    {
        ଯଦି (ZERROR64(*pzlib_filefunc_def,filestream))
            ଫେରନ୍ତୁ ZIP_ERRNO;
        ଅନ୍ୟଥା
            ଫେରନ୍ତୁ ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64local_getShort(ଅଚଳ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ଯଦି (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ଯଦି (err==ZIP_OK)
        *pX = x;
    ଅନ୍ୟଥା
        *pX = 0;
    ଫେରନ୍ତୁ err;
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64local_getLong(ଅଚଳ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ଯଦି (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ଯଦି (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    ଯଦି (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    ଯଦି (err==ZIP_OK)
        *pX = x;
    ଅନ୍ୟଥା
        *pX = 0;
    ଫେରନ୍ତୁ err;
}


local ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64local_getLong64(ଅଚଳ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  ଯଦି (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  ଯଦି (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  ଯଦି (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  ଯଦି (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  ଯଦି (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  ଯଦି (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  ଯଦି (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  ଯଦି (err==ZIP_OK)
    *pX = x;
  ଅନ୍ୟଥା
    *pX = 0;

  ଫେରନ୍ତୁ err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_ସନ୍ଧାନ_Central_Dir(ଅଚଳ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned ଅକ୍ଷର* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  ଯଦି (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    ଫେରନ୍ତୁ 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  ଯଦି (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned ଅକ୍ଷର*)ALLOC(BUFREADCOMMENT+4);
  ଯଦି (buf==ଖାଲି)
    ଫେରନ୍ତୁ 0;

  uBackRead = 4;
  ଯେତେବେଳେ (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ଯଦି (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    ଅନ୍ୟଥା
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    ଯଦି (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      ଭାଙ୍ଗ;

    ଯଦି (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      ଭାଙ୍ଗ;

    ପାଇଁ (i=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)uReadSize-3; (i--)>0;)
      ଯଦି (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+(unsigned)i;
        ଭାଙ୍ଗ;
      }

    ଯଦି (uPosFound!=0)
      ଭାଙ୍ଗ;
  }
  ମୁକ୍ତ(buf);
  ଫେରନ୍ତୁ uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_ସନ୍ଧାନ_Central_Dir64(ଅଚଳ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned ଅକ୍ଷର* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  ଯଦି (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    ଫେରନ୍ତୁ 0;

  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  ଯଦି (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned ଅକ୍ଷର*)ALLOC(BUFREADCOMMENT+4);
  ଯଦି (buf==ଖାଲି)
    ଫେରନ୍ତୁ 0;

  uBackRead = 4;
  ଯେତେବେଳେ (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ଯଦି (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    ଅନ୍ୟଥା
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    ଯଦି (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      ଭାଙ୍ଗ;

    ଯଦି (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      ଭାଙ୍ଗ;

    ପାଇଁ (i=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)uReadSize-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      ଯଦି (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+(unsigned)i;
        ଭାଙ୍ଗ;
      }
    }

      ଯଦି (uPosFound!=0)
        ଭାଙ୍ଗ;
  }

  ମୁକ୍ତ(buf);
  ଯଦି (uPosFound == 0)
    ଫେରନ୍ତୁ 0;

  /* Zip64 end of central directory locator */
  ଯଦି (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    ଫେରନ୍ତୁ 0;

  /* the signature, already checked */
  ଯଦି (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ଫେରନ୍ତୁ 0;

  /* number of the disk with the start of the zip64 end of central directory */
  ଯଦି (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ଫେରନ୍ତୁ 0;
  ଯଦି (uL != 0)
    ଫେରନ୍ତୁ 0;

  /* relative offset of the zip64 end of central directory record */
  ଯଦି (zip64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    ଫେରନ୍ତୁ 0;

  /* total number of disks */
  ଯଦି (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ଫେରନ୍ତୁ 0;
  ଯଦି (uL != 1)
    ଫେରନ୍ତୁ 0;

  /* Goto Zip64 end of central directory record */
  ଯଦି (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    ଫେରନ୍ତୁ 0;

  /* the signature */
  ଯଦି (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ଫେରନ୍ତୁ 0;

  ଯଦି (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    ଫେରନ୍ତୁ 0;

  ଫେରନ୍ତୁ relativeOffset;
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା LoadCentralDirectoryRecord(zip64_internal* pziinit) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T size_central_dir;     /* size of the central directory  */
  ZPOS64_T offset_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong ସଂଖ୍ୟା_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong ସଂଖ୍ୟା_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T ସଂଖ୍ୟା_entry;
  ZPOS64_T ସଂଖ୍ୟା_entry_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong VersionMadeBy;
  uLong VersionNeeded;
  uLong size_comment;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hasZIP64Record = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_ସନ୍ଧାନ_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  ଯଦି(central_pos > 0)
  {
    hasZIP64Record = 1;
  }
  ଅନ୍ୟଥା ଯଦି(central_pos == 0)
  {
    central_pos = zip64local_ସନ୍ଧାନ_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  ଯଦି(hasZIP64Record)
  {
    ZPOS64_T sizeEndOfCentralDirectory;
    ଯଦି (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ଯଦି (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    ଯଦି (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &sizeEndOfCentralDirectory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    ଯଦି (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    ଯଦି (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ଯଦି (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&ସଂଖ୍ୟା_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ଯଦି (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&ସଂଖ୍ୟା_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    ଯଦି (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &ସଂଖ୍ୟା_entry)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    ଯଦି (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ସଂଖ୍ୟା_entry_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    ଯଦି ((ସଂଖ୍ୟା_entry_CD!=ସଂଖ୍ୟା_entry) || (ସଂଖ୍ୟା_disk_with_CD!=0) || (ସଂଖ୍ୟା_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ଯଦି (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&size_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    ଯଦି (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&offset_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    size_comment = 0;
  }
  ଅନ୍ୟଥା
  {
    /* Read End of central Directory info */
    ଯଦି (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ଯଦି (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ଯଦି (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&ସଂଖ୍ୟା_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ଯଦି (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&ସଂଖ୍ୟା_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    ସଂଖ୍ୟା_entry = 0;
    ଯଦି (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ଅନ୍ୟଥା
      ସଂଖ୍ୟା_entry = uL;

    /* total number of entries in the central dir */
    ସଂଖ୍ୟା_entry_CD = 0;
    ଯଦି (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ଅନ୍ୟଥା
      ସଂଖ୍ୟା_entry_CD = uL;

    ଯଦି ((ସଂଖ୍ୟା_entry_CD!=ସଂଖ୍ୟା_entry) || (ସଂଖ୍ୟା_disk_with_CD!=0) || (ସଂଖ୍ୟା_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    size_central_dir = 0;
    ଯଦି (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ଅନ୍ୟଥା
      size_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    offset_central_dir = 0;
    ଯଦି (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ଅନ୍ୟଥା
      offset_central_dir = uL;


    /* zipfile global comment length */
    ଯଦି (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &size_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  ଯଦି ((central_pos<offset_central_dir+size_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  ଯଦି (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    ଫେରନ୍ତୁ ZIP_ERRNO;
  }

  ଯଦି (size_comment>0)
  {
    pziinit->globalcomment = (ଅକ୍ଷର*)ALLOC(size_comment+1);
    ଯଦି (pziinit->globalcomment)
    {
      size_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,size_comment);
      pziinit->globalcomment[size_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (offset_central_dir+size_central_dir);
  pziinit->add_position_when_writing_offset = byte_before_the_zipfile;

  {
    ZPOS64_T size_central_dir_to_read = size_central_dir;
    size_t buf_size = SIZEDATA_INDATABLOCK;
    ଶୂନ୍ୟ* buf_read = (ଶୂନ୍ୟ*)ALLOC(buf_size);
    ଯଦି (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    ଯେତେବେଳେ ((size_central_dir_to_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      ଯଦି (read_this > size_central_dir_to_read)
        read_this = size_central_dir_to_read;

      ଯଦି (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      ଯଦି (err==ZIP_OK)
        err = add_data_in_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      size_central_dir_to_read-=read_this;
    }
    ମୁକ୍ତ(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->ସଂଖ୍ୟା_entry = ସଂଖ୍ୟା_entry_CD;

  ଯଦି (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET) != 0)
    err=ZIP_ERRNO;

  ଫେରନ୍ତୁ err;
}


#endif /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
ବାହ୍ୟ zipFile ZEXPORT zipOpen3(ଅଚଳ ଶୂନ୍ୟ *pathname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_internal ziinit;
    zip64_internal* zi;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = ଖାଲି;
    ziinit.z_filefunc.ztell32_file = ଖାଲି;
    ଯଦି (pzlib_filefunc64_32_def==ଖାଲି)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    ଅନ୍ୟଥା
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_STATUS_CREATE) ?
                  (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
                    (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

    ଯଦି (ziinit.filestream == ଖାଲି)
        ଫେରନ୍ତୁ ଖାଲି;

    ଯଦି (append == APPEND_STATUS_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.ସଂଖ୍ୟା_entry = 0;
    ziinit.add_position_when_writing_offset = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.set, 0, ଆକାର(set_t));  /* make sure set appears dormant */



    zi = (zip64_internal*)ALLOC(ଆକାର(zip64_internal));
    ଯଦି (zi==ଖାଲି)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        ଫେରନ୍ତୁ ଖାଲି;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = ଖାଲି;
    ଯଦି (append == APPEND_STATUS_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = LoadCentralDirectoryRecord(&ziinit);
    }

    ଯଦି (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    ଯଦି (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        ମୁକ୍ତ(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        ମୁକ୍ତ(zi);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ଅନ୍ୟଥା
    {
        *zi = ziinit;
        ଫେରନ୍ତୁ (zipFile)zi;
    }
}

ବାହ୍ୟ zipFile ZEXPORT zipOpen2(ଅଚଳ ଅକ୍ଷର *pathname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    ଯଦି (pzlib_filefunc32_def != ଖାଲି)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        ଫେରନ୍ତୁ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ଅନ୍ୟଥା
        ଫେରନ୍ତୁ zipOpen3(pathname, append, globalcomment, ଖାଲି);
}

ବାହ୍ୟ zipFile ZEXPORT zipOpen2_64(ଅଚଳ ଶୂନ୍ୟ *pathname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    ଯଦି (pzlib_filefunc_def != ଖାଲି)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ଖାଲି;
        zlib_filefunc64_32_def_fill.ztell32_file = ଖାଲି;
        zlib_filefunc64_32_def_fill.zseek32_file = ଖାଲି;
        ଫେରନ୍ତୁ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ଅନ୍ୟଥା
        ଫେରନ୍ତୁ zipOpen3(pathname, append, globalcomment, ଖାଲି);
}



ବାହ୍ୟ zipFile ZEXPORT zipOpen(ଅଚଳ ଅକ୍ଷର* pathname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା append) {
    ଫେରନ୍ତୁ zipOpen3((ଅଚଳ ଶୂନ୍ୟ*)pathname,append,ଖାଲି,ଖାଲି);
}

ବାହ୍ୟ zipFile ZEXPORT zipOpen64(ଅଚଳ ଶୂନ୍ୟ* pathname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା append) {
    ଫେରନ୍ତୁ zipOpen3(pathname,append,ଖାଲି,ଖାଲି);
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା Write_LocalFileHeader(zip64_internal* zi, ଅଚଳ ଅକ୍ଷର* filename, uInt size_extrafield_local, ଅଚଳ ଶୂନ୍ୟ* extrafield_local) {
  /* write the local header */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
  uInt size_filename = (uInt)strlen(filename);
  uInt size_extrafield = size_extrafield_local;

  err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  ଯଦି (err==ZIP_OK)
  {
    ଯଦି(zi->ci.zip64)
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    ଅନ୍ୟଥା
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  ଯଦି (err==ZIP_OK)
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  ଯଦି (err==ZIP_OK)
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.method,2);

  ଯଦି (err==ZIP_OK)
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  ଯଦି (err==ZIP_OK)
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  ଯଦି (err==ZIP_OK)
  {
    ଯଦି(zi->ci.zip64)
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    ଅନ୍ୟଥା
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  ଯଦି (err==ZIP_OK)
  {
    ଯଦି(zi->ci.zip64)
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    ଅନ୍ୟଥା
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  ଯଦି (err==ZIP_OK)
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)size_filename,2);

  ଯଦି(zi->ci.zip64)
  {
    size_extrafield += 20;
  }

  ଯଦି (err==ZIP_OK)
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)size_extrafield,2);

  ଯଦି ((err==ZIP_OK) && (size_filename > 0))
  {
    ଯଦି (ZWRITE64(zi->z_filefunc,zi->filestream,filename,size_filename)!=size_filename)
      err = ZIP_ERRNO;
  }

  ଯଦି ((err==ZIP_OK) && (size_extrafield_local > 0))
  {
    ଯଦି (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, size_extrafield_local) != size_extrafield_local)
      err = ZIP_ERRNO;
  }


  ଯଦି ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      short HeaderID = 1;
      short DataSize = 16;
      ZPOS64_T CompressedSize = 0;
      ZPOS64_T UncompressedSize = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)HeaderID,2);
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)DataSize,2);

      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)UncompressedSize,8);
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)CompressedSize,8);
  }

  ଫେରନ୍ତୁ err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା utf8len(unsigned ଅକ୍ଷର ଅଚଳ *str, size_t len) {
    ଫେରନ୍ତୁ
        len == 0 ? -1 :                         /* empty input */
        str[0] < 0x80 ? 1 :                     /* good one-byte */
        str[0] < 0xc0 ? -1 :                    /* bad first byte */
        len < 2 || (str[1] >> 6) != 2 ? -2 :    /* missing or bad 2nd byte */
        str[0] < 0xc2 ? -2 :                    /* overlong code */
        str[0] < 0xe0 ? 2 :                     /* good two-byte */
        len < 3 || (str[2] >> 6) != 2 ? -3 :    /* missing or bad 3rd byte */
        str[0] == 0xe0 && str[1] < 0xa0 ? -3 :  /* overlong code */
        str[0] < 0xf0 ? 3 :                     /* good three-byte */
        len < 4 || (str[3] >> 6) != 2 ? -4 :    /* missing or bad 4th byte */
        str[0] == 0xf0 && str[1] < 0x90 ? -4 :  /* overlong code */
        str[0] < 0xf4 ||
        (str[0] == 0xf4 && str[1] < 0x90) ? 4 : /* good four-byte */
        -4;                                     /* code > 0x10ffff */
}

/* Return true if str[0..len-1] is valid UTF-8 *and* it contains at least one
   code of two or more bytes. This is used to determine whether or not to set
   bit 11 in the zip header flags. */
local ପୂର୍ଣ୍ଣସଂଖ୍ୟା isutf8(ଅକ୍ଷର ଅଚଳ *str, size_t len) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା utf8 = 0;
    ଯେତେବେଳେ (len) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = utf8len((unsigned ଅକ୍ଷର ଅଚଳ *)str, len);
        ଯଦି (code < 0)
            ଫେରନ୍ତୁ 0;
        ଯଦି (code > 1)
            utf8 = 1;
        str += code;
        len -= (unsigned)code;
    }
    ଫେରନ୍ତୁ utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip4_64(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                           ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                           ଅଚଳ ଶୂନ୍ୟ* extrafield_global, uInt size_extrafield_global,
                                           ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw,
                                           ପୂର୍ଣ୍ଣସଂଖ୍ୟା windowBits,ପୂର୍ଣ୍ଣସଂଖ୍ୟା memLevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strategy,
                                           ଅଚଳ ଅକ୍ଷର* password, uLong crcForCrypting,
                                           uLong versionMadeBy, uLong flagBase, ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64) {
    zip64_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = ZIP_OK;

#    ifdef NOCRYPT
    ଯଦି (password != ଖାଲି)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;
#    endif

    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;

#ifdef HAVE_BZIP2
    ଯଦି ((method!=0) && (method!=Z_DEFLATED) && (method!=Z_BZIP2ED))
      ଫେରନ୍ତୁ ZIP_PARAMERROR;
#ଅନ୍ୟଥା
    ଯଦି ((method!=0) && (method!=Z_DEFLATED))
      ଫେରନ୍ତୁ ZIP_PARAMERROR;
#endif

    /* The filename and comment length must fit in 16 bits. */
    ଯଦି ((filename!=ଖାଲି) && (strlen(filename)>0xffff))
        ଫେରନ୍ତୁ ZIP_PARAMERROR;
    ଯଦି ((comment!=ଖାଲି) && (strlen(comment)>0xffff))
        ଫେରନ୍ତୁ ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    ଯଦି ((size_extrafield_local>0xffff) || (size_extrafield_global>0xffff))
        ଫେରନ୍ତୁ ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    ଯଦି (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        ଯଦି (err != ZIP_OK)
            ଫେରନ୍ତୁ err;
    }

    ଯଦି (filename==ଖାଲି)
        filename="-";

    ଯଦି (comment==ଖାଲି)
        size_comment = 0;
    ଅନ୍ୟଥା
        size_comment = (uInt)strlen(comment);

    size_filename = (uInt)strlen(filename);

    ଯଦି (zipfi == ଖାଲି)
        zi->ci.dosDate = 0;
    ଅନ୍ୟଥା
    {
        ଯଦି (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        ଅନ୍ୟଥା
          zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
    }

    zi->ci.flag = flagBase;
    ଯଦି ((level==8) || (level==9))
      zi->ci.flag |= 2;
    ଯଦି (level==2)
      zi->ci.flag |= 4;
    ଯଦି (level==1)
      zi->ci.flag |= 6;
    ଯଦି (password != ଖାଲି)
      zi->ci.flag |= 1;
    ଯଦି (isutf8(filename, size_filename) &&
        (size_comment == 0 || isutf8(comment, size_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.crc32 = 0;
    zi->ci.method = method;
    zi->ci.encrypt = 0;
    zi->ci.stream_initialised = 0;
    zi->ci.pos_in_buffered_data = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_header = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.size_centralheader = SIZECENTRALHEADER + size_filename + size_extrafield_global + size_comment;
    zi->ci.size_centralExtraFree = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_header = (ଅକ୍ଷର*)ALLOC((uInt)zi->ci.size_centralheader + zi->ci.size_centralExtraFree);

    zi->ci.size_centralExtra = size_extrafield_global;
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+4,(uLong)versionMadeBy,2);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+6,(uLong)20,2);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+10,(uLong)zi->ci.method,2);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+20,(uLong)0,4); /*compr size*/
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+28,(uLong)size_filename,2);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+30,(uLong)size_extrafield_global,2);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+32,(uLong)size_comment,2);
    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    ଯଦି (zipfi==ଖାଲି)
        zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+36,(uLong)0,2);
    ଅନ୍ୟଥା
        zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    ଯଦି (zipfi==ଖାଲି)
        zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+38,(uLong)0,4);
    ଅନ୍ୟଥା
        zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    ଯଦି(zi->ci.pos_local_header >= 0xffffffff)
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+42,(uLong)0xffffffff,4);
    ଅନ୍ୟଥା
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header - zi->add_position_when_writing_offset,4);

    ପାଇଁ (i=0;i<size_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

    ପାଇଁ (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((ଅଚଳ ଅକ୍ଷର*)extrafield_global)+i);

    ପାଇଁ (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = *(comment+i);
    ଯଦି (zi->ci.central_header == ଖାଲି)
        ଫେରନ୍ତୁ ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.totalCompressedData = 0;
    zi->ci.totalUncompressedData = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_LocalFileHeader(zi, filename, size_extrafield_local, extrafield_local);

#ifdef HAVE_BZIP2
    zi->ci.bstream.avail_in = (uInt)0;
    zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.bstream.next_out = (ଅକ୍ଷର*)zi->ci.buffered_data;
    zi->ci.bstream.total_in_hi32 = 0;
    zi->ci.bstream.total_in_lo32 = 0;
    zi->ci.bstream.total_out_hi32 = 0;
    zi->ci.bstream.total_out_lo32 = 0;
#endif

    zi->ci.stream.avail_in = (uInt)0;
    zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.stream.next_out = zi->ci.buffered_data;
    zi->ci.stream.total_in = 0;
    zi->ci.stream.total_out = 0;
    zi->ci.stream.data_type = Z_ଦ୍ୱିମିକ;

#ifdef HAVE_BZIP2
    ଯଦି ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED || zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
#ଅନ୍ୟଥା
    ଯଦି ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
#endif
    {
        ଯଦି(zi->ci.method == Z_DEFLATED)
        {
          zi->ci.stream.zalloc = (alloc_func)0;
          zi->ci.stream.zfree = (free_func)0;
          zi->ci.stream.opaque = (voidpf)0;

          ଯଦି (windowBits>0)
              windowBits = -windowBits;

          err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

          ଯଦି (err==Z_OK)
              zi->ci.stream_initialised = Z_DEFLATED;
        }
        ଅନ୍ୟଥା ଯଦି(zi->ci.method == Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          ଯଦି(err == BZ_OK)
            zi->ci.stream_initialised = Z_BZIP2ED;
#endif
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_header_size = 0;
    ଯଦି ((err==Z_OK) && (password != ଖାଲି))
    {
        unsigned ଅକ୍ଷର bufHead[RAND_HEAD_LEN];
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା sizeHead;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_table();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        sizeHead=crypthead(password,bufHead,RAND_HEAD_LEN,zi->ci.ଚାବିଗୁଡ଼ିକ,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_header_size = sizeHead;

        ଯଦି (ZWRITE64(zi->z_filefunc,zi->filestream,bufHead,sizeHead) != sizeHead)
                err = ZIP_ERRNO;
    }
#    endif

    ଯଦି (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    ଫେରନ୍ତୁ err;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip4(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                        ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                        ଅଚଳ ଶୂନ୍ୟ* extrafield_global, uInt size_extrafield_global,
                                        ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw,
                                        ପୂର୍ଣ୍ଣସଂଖ୍ୟା windowBits,ପୂର୍ଣ୍ଣସଂଖ୍ୟା memLevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strategy,
                                        ଅଚଳ ଅକ୍ଷର* password, uLong crcForCrypting,
                                        uLong versionMadeBy, uLong flagBase) {
    ଫେରନ୍ତୁ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, versionMadeBy, flagBase, 0);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip3(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                        ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                        ଅଚଳ ଶୂନ୍ୟ* extrafield_global, uInt size_extrafield_global,
                                        ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw,
                                        ପୂର୍ଣ୍ଣସଂଖ୍ୟା windowBits,ପୂର୍ଣ୍ଣସଂଖ୍ୟା memLevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strategy,
                                        ଅଚଳ ଅକ୍ଷର* password, uLong crcForCrypting) {
    ଫେରନ୍ତୁ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip3_64(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                         ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                         ଅଚଳ ଶୂନ୍ୟ* extrafield_global, uInt size_extrafield_global,
                                         ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw,
                                         ପୂର୍ଣ୍ଣସଂଖ୍ୟା windowBits,ପୂର୍ଣ୍ଣସଂଖ୍ୟା memLevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strategy,
                                         ଅଚଳ ଅକ୍ଷର* password, uLong crcForCrypting, ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64) {
    ଫେରନ୍ତୁ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip2(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                        ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                        ଅଚଳ ଶୂନ୍ୟ* extrafield_global, uInt size_extrafield_global,
                                        ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw) {
    ଫେରନ୍ତୁ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ଖାଲି, 0, VERSIONMADEBY, 0, 0);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip2_64(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                           ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                           ଅଚଳ ଶୂନ୍ୟ* extrafield_global, uInt size_extrafield_global,
                                           ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raw, ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64) {
    ଫେରନ୍ତୁ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ଖାଲି, 0, VERSIONMADEBY, 0, zip64);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip64(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                         ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                         ଅଚଳ ଶୂନ୍ୟ*extrafield_global, uInt size_extrafield_global,
                                         ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64) {
    ଫେରନ୍ତୁ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ଖାଲି, 0, VERSIONMADEBY, 0, zip64);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipOpenNewFileInZip(zipFile file, ଅଚଳ ଅକ୍ଷର* filename, ଅଚଳ zip_fileinfo* zipfi,
                                       ଅଚଳ ଶୂନ୍ୟ* extrafield_local, uInt size_extrafield_local,
                                       ଅଚଳ ଶୂନ୍ୟ*extrafield_global, uInt size_extrafield_global,
                                       ଅଚଳ ଅକ୍ଷର* comment, ପୂର୍ଣ୍ଣସଂଖ୍ୟା method, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level) {
    ଫେରନ୍ତୁ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ଖାଲି, 0, VERSIONMADEBY, 0, 0);
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64FlushWriteBuffer(zip64_internal* zi) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=ZIP_OK;

    ଯଦି (zi->ci.encrypt != 0)
    {
#ifndef NOCRYPT
        uInt i;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା t;
        ପାଇଁ (i=0;i<zi->ci.pos_in_buffered_data;i++)
            zi->ci.buffered_data[i] = zencode(zi->ci.ଚାବିଗୁଡ଼ିକ, zi->ci.pcrc_32_tab, zi->ci.buffered_data[i],t);
#endif
    }

    ଯଦି (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_data,zi->ci.pos_in_buffered_data) != zi->ci.pos_in_buffered_data)
      err = ZIP_ERRNO;

    zi->ci.totalCompressedData += zi->ci.pos_in_buffered_data;

#ifdef HAVE_BZIP2
    ଯଦି(zi->ci.method == Z_BZIP2ED)
    {
      zi->ci.totalUncompressedData += zi->ci.bstream.total_in_lo32;
      zi->ci.bstream.total_in_lo32 = 0;
      zi->ci.bstream.total_in_hi32 = 0;
    }
    ଅନ୍ୟଥା
#endif
    {
      zi->ci.totalUncompressedData += zi->ci.stream.total_in;
      zi->ci.stream.total_in = 0;
    }


    zi->ci.pos_in_buffered_data = 0;

    ଫେରନ୍ତୁ err;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipWriteInFileInZip(zipFile file, ଅଚଳ ଶୂନ୍ୟ* buf, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା len) {
    zip64_internal* zi;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=ZIP_OK;

    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    ଯଦି (zi->in_opened_file_inzip == 0)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;

    zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uInt)len);

#ifdef HAVE_BZIP2
    ଯଦି(zi->ci.method == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.next_in = (ଶୂନ୍ୟ*)buf;
      zi->ci.bstream.avail_in = len;
      err = BZ_RUN_OK;

      ଯେତେବେଳେ ((err==BZ_RUN_OK) && (zi->ci.bstream.avail_in>0))
      {
        ଯଦି (zi->ci.bstream.avail_out == 0)
        {
          ଯଦି (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (ଅକ୍ଷର*)zi->ci.buffered_data;
        }


        ଯଦି(err != BZ_RUN_OK)
          ଭାଙ୍ଗ;

        ଯଦି ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong uTotalOutBefore_lo = zi->ci.bstream.total_out_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_RUN);

          zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore_lo) ;
        }
      }

      ଯଦି(err == BZ_RUN_OK)
        err = ZIP_OK;
    }
    ଅନ୍ୟଥା
#endif
    {
      zi->ci.stream.next_in = buf;
      zi->ci.stream.avail_in = len;

      ଯେତେବେଳେ ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
      {
          ଯଦି (zi->ci.stream.avail_out == 0)
          {
              ଯଦି (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
              zi->ci.stream.next_out = zi->ci.buffered_data;
          }


          ଯଦି(err != ZIP_OK)
              ଭାଙ୍ଗ;

          ଯଦି ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong uTotalOutBefore = zi->ci.stream.total_out;
              err=deflate(&zi->ci.stream,  Z_NO_FLUSH);

              zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
          }
          ଅନ୍ୟଥା
          {
              uInt copy_this,i;
              ଯଦି (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                  copy_this = zi->ci.stream.avail_in;
              ଅନ୍ୟଥା
                  copy_this = zi->ci.stream.avail_out;

              ପାଇଁ (i = 0; i < copy_this; i++)
                  *(((ଅକ୍ଷର*)zi->ci.stream.next_out)+i) =
                      *(((ଅଚଳ ଅକ୍ଷର*)zi->ci.stream.next_in)+i);
              {
                  zi->ci.stream.avail_in -= copy_this;
                  zi->ci.stream.avail_out-= copy_this;
                  zi->ci.stream.next_in+= copy_this;
                  zi->ci.stream.next_out+= copy_this;
                  zi->ci.stream.total_in+= copy_this;
                  zi->ci.stream.total_out+= copy_this;
                  zi->ci.pos_in_buffered_data += copy_this;
              }
          }
      }/* while(...) */
    }

    ଫେରନ୍ତୁ err;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipCloseFileInZipRaw(zipFile file, uLong uncompressed_size, uLong crc32) {
    ଫେରନ୍ତୁ zipCloseFileInZipRaw64 (file, uncompressed_size, crc32);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipCloseFileInZipRaw64(zipFile file, ZPOS64_T uncompressed_size, uLong crc32) {
    zip64_internal* zi;
    ZPOS64_T compressed_size;
    uLong invalid_ମୂଲ୍ୟ = 0xffffffff;
    unsigned datasize = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=ZIP_OK;

    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    ଯଦି (zi->in_opened_file_inzip == 0)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    ଯଦି ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
                {
                        ଯେତେବେଳେ (err==ZIP_OK)
                        {
                                uLong uTotalOutBefore;
                                ଯଦି (zi->ci.stream.avail_out == 0)
                                {
                                        ଯଦି (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                                        zi->ci.stream.next_out = zi->ci.buffered_data;
                                }
                                uTotalOutBefore = zi->ci.stream.total_out;
                                err=deflate(&zi->ci.stream,  Z_FINISH);
                                zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
                        }
                }
    ଅନ୍ୟଥା ଯଦି ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ifdef HAVE_BZIP2
      err = BZ_FINISH_OK;
      ଯେତେବେଳେ (err==BZ_FINISH_OK)
      {
        uLong uTotalOutBefore;
        ଯଦି (zi->ci.bstream.avail_out == 0)
        {
          ଯଦି (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (ଅକ୍ଷର*)zi->ci.buffered_data;
        }
        uTotalOutBefore = zi->ci.bstream.total_out_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        ଯଦି(err == BZ_STREAM_END)
          err = Z_STREAM_END;

        zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore);
      }

      ଯଦି(err == BZ_FINISH_OK)
        err = ZIP_OK;
#endif
    }

    ଯଦି (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    ଯଦି ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
                {
        ଯଦି (zip64FlushWriteBuffer(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    ଯଦି ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
    {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmp_err = deflateEnd(&zi->ci.stream);
        ଯଦି (err == ZIP_OK)
            err = tmp_err;
        zi->ci.stream_initialised = 0;
    }
#ifdef HAVE_BZIP2
    ଅନ୍ୟଥା ଯଦି((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmperr = BZ2_bzCompressEnd(&zi->ci.bstream);
                        ଯଦି (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.stream_initialised = 0;
    }
#endif

    ଯଦି (!zi->ci.raw)
    {
        crc32 = (uLong)zi->ci.crc32;
        uncompressed_size = zi->ci.totalUncompressedData;
    }
    compressed_size = zi->ci.totalCompressedData;

#    ifndef NOCRYPT
    compressed_size += zi->ci.crypt_header_size;
#    endif

    /* update Current Item crc and sizes, */
    ଯଦି(compressed_size >= 0xffffffff || uncompressed_size >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+6,(uLong)45,2);

    }

    zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


    ଯଦି(compressed_size >= 0xffffffff)
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+20, invalid_ମୂଲ୍ୟ,4); /*compr size*/
    ଅନ୍ୟଥା
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+20, compressed_size,4); /*compr size*/

    /* set internal file attributes field */
    ଯଦି (zi->ci.stream.data_type == Z_ASCII)
        zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+36,(uLong)Z_ASCII,2);

    ଯଦି(uncompressed_size >= 0xffffffff)
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+24, invalid_ମୂଲ୍ୟ,4); /*uncompr size*/
    ଅନ୍ୟଥା
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+24, uncompressed_size,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    ଯଦି(uncompressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    ଯଦି(compressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    ଯଦି(zi->ci.pos_local_header >= 0xffffffff)
      datasize += 8;

    ଯଦି(datasize > 0)
    {
      ଅକ୍ଷର* p = ଖାଲି;

      ଯଦି((uLong)(datasize + 4) > zi->ci.size_centralExtraFree)
      {
        /* we cannot write more data to the buffer that we have room for. */
        ଫେରନ୍ତୁ ZIP_BADZIPFILE;
      }

      p = zi->ci.central_header + zi->ci.size_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_ମୂଲ୍ୟ_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_ମୂଲ୍ୟ_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      ଯଦି(uncompressed_size >= 0xffffffff)
      {
        zip64local_put_ମୂଲ୍ୟ_inmemory(p, uncompressed_size, 8);
        p += 8;
      }

      ଯଦି(compressed_size >= 0xffffffff)
      {
        zip64local_put_ମୂଲ୍ୟ_inmemory(p, compressed_size, 8);
        p += 8;
      }

      ଯଦି(zi->ci.pos_local_header >= 0xffffffff)
      {
        zip64local_put_ମୂଲ୍ୟ_inmemory(p, zi->ci.pos_local_header, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.size_centralExtraFree -= datasize + 4;
      zi->ci.size_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.size_centralExtra += datasize + 4;
      zip64local_put_ମୂଲ୍ୟ_inmemory(zi->ci.central_header+30,(uLong)zi->ci.size_centralExtra,2);
    }

    ଯଦି (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir, zi->ci.central_header, (uLong)zi->ci.size_centralheader);

    ମୁକ୍ତ(zi->ci.central_header);

    ଯଦି (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        ଯଦି (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;

        ଯଦି (err==ZIP_OK)
            err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

        ଯଦି(uncompressed_size >= 0xffffffff || compressed_size >= 0xffffffff )
        {
          ଯଦି(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            ଯଦି (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET)!=0)
              err = ZIP_ERRNO;

            ଯଦି (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, uncompressed_size, 8);

            ଯଦି (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, compressed_size, 8);
          }
          ଅନ୍ୟଥା
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        ଅନ୍ୟଥା
        {
          ଯଦି (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,compressed_size,4);

          ଯଦି (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,uncompressed_size,4);
        }

        ଯଦି (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;
    }

    zi->ସଂଖ୍ୟା_entry ++;
    zi->in_opened_file_inzip = 0;

    ଫେରନ୍ତୁ err;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipCloseFileInZip(zipFile file) {
    ଫେରନ୍ତୁ zipCloseFileInZipRaw (file,0,0);
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, ZPOS64_T zip64eocd_pos_inzip) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->add_position_when_writing_offset;

  err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    ଯଦି (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    ଯଦି (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    ଯଦି (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    ଫେରନ୍ତୁ err;
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = ZIP_OK;

  uLong Zip64DataSize = 44;

  err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  ଯଦି (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64DataSize,8); /* why ZPOS64_T of this ? */

  ଯଦି (err==ZIP_OK) /* version made by */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ଯଦି (err==ZIP_OK) /* version needed */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ଯଦି (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ଯଦି (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ଯଦି (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, zi->ସଂଖ୍ୟା_entry, 8);

  ଯଦି (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc, zi->filestream, zi->ସଂଖ୍ୟା_entry, 8);

  ଯଦି (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)size_centraldir,8);

  ଯଦି (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  ଫେରନ୍ତୁ err;
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = ZIP_OK;

  /*signature*/
  err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  ଯଦି (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ଯଦି (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ଯଦି (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      ଯଦି(zi->ସଂଖ୍ୟା_entry >= 0xFFFF)
        err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      ଅନ୍ୟଥା
        err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)zi->ସଂଖ୍ୟା_entry,2);
    }
  }

  ଯଦି (err==ZIP_OK) /* total number of entries in the central dir */
  {
    ଯଦି(zi->ସଂଖ୍ୟା_entry >= 0xFFFF)
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    ଅନ୍ୟଥା
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)zi->ସଂଖ୍ୟା_entry,2);
  }

  ଯଦି (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)size_centraldir,4);

  ଯଦି (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    ଯଦି(pos >= 0xffffffff)
    {
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    ଅନ୍ୟଥା
      err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->add_position_when_writing_offset),4);
  }

   ଫେରନ୍ତୁ err;
}

local ପୂର୍ଣ୍ଣସଂଖ୍ୟା Write_GlobalComment(zip64_internal* zi, ଅଚଳ ଅକ୍ଷର* global_comment) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = ZIP_OK;
  uInt size_global_comment = 0;

  ଯଦି(global_comment != ଖାଲି)
    size_global_comment = (uInt)strlen(global_comment);

  err = zip64local_put_ମୂଲ୍ୟ(&zi->z_filefunc,zi->filestream,(uLong)size_global_comment,2);

  ଯଦି (err == ZIP_OK && size_global_comment > 0)
  {
    ଯଦି (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, size_global_comment) != size_global_comment)
      err = ZIP_ERRNO;
  }
  ଫେରନ୍ତୁ err;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipClose(zipFile file, ଅଚଳ ଅକ୍ଷର* global_comment) {
    zip64_internal* zi;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = 0;
    uLong size_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    ଯଦି (file == ଖାଲି)
        ଫେରନ୍ତୁ ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    ଯଦି (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

#ifndef NO_ADDFILEINEXISTINGZIP
    ଯଦି (global_comment==ଖାଲି)
        global_comment = zi->globalcomment;
#endif

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    ଯଦି (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
        ଯେତେବେଳେ (ldi!=ଖାଲି)
        {
            ଯଦି ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
            {
                ଯଦି (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->data, ldi->filled_in_this_block) != ldi->filled_in_this_block)
                    err = ZIP_ERRNO;
            }

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    set_end(&zi->set);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    ଯଦି(pos >= 0xffffffff || zi->ସଂଖ୍ୟା_entry >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

      Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
    }

    ଯଦି (err==ZIP_OK)
      err = Write_EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

    ଯଦି(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    ଯଦି (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        ଯଦି (err == ZIP_OK)
            err = ZIP_ERRNO;

#ifndef NO_ADDFILEINEXISTINGZIP
    ମୁକ୍ତ(zi->globalcomment);
#endif
    ମୁକ୍ତ(zi);

    ଫେରନ୍ତୁ err;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT zipRemoveExtraInfoBlock(ଅକ୍ଷର* pData, ପୂର୍ଣ୍ଣସଂଖ୍ୟା* dataLen, short sHeader) {
  ଅକ୍ଷର* p = pData;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = 0;
  ଅକ୍ଷର* pNewHeader;
  ଅକ୍ଷର* pTmp;
  short header;
  short dataSize;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା retVal = ZIP_OK;

  ଯଦି(pData == ଖାଲି || dataLen == ଖାଲି || *dataLen < 4)
    ଫେରନ୍ତୁ ZIP_PARAMERROR;

  pNewHeader = (ଅକ୍ଷର*)ALLOC((unsigned)*dataLen);
  pTmp = pNewHeader;

  ଯେତେବେଳେ(p < (pData + *dataLen))
  {
    header = *(short*)p;
    dataSize = *(((short*)p)+1);

    ଯଦି( header == sHeader ) /* Header found. */
    {
      p += dataSize + 4; /* skip it. do not copy to temp buffer */
    }
    ଅନ୍ୟଥା
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      memcpy(pTmp, p, dataSize + 4);
      p += dataSize + 4;
      size += dataSize + 4;
    }

  }

  ଯଦି(size < *dataLen)
  {
    /* clean old extra info block. */
    memset(pData,0, *dataLen);

    /* copy the new extra info block over the old */
    ଯଦି(size > 0)
      memcpy(pData, pNewHeader, size);

    /* set the new extra info size */
    *dataLen = size;

    retVal = ZIP_OK;
  }
  ଅନ୍ୟଥା
    retVal = ZIP_ERRNO;

  ମୁକ୍ତ(pNewHeader);

  ଫେରନ୍ତୁ retVal;
}
