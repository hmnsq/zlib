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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef ZLIB_CONST
#  define ZLIB_CONST
#endif
#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#include "zlib.h"
#include "zip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    extern puur_naa_nka errno;
#ᤀᤡ᤺ᤆᤡ
#   include <errno.h>
#endif


#ifndef local
#  define local static
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
#ᤀᤡ᤺ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ᤀᤡ᤺ᤆᤡ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
const var_na zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x6064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

typedef sa_nracanaa linkedlist_datablock_internal_s
{
  sa_nracanaa linkedlist_datablock_internal_s* next_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignment */
  unsigned var_na data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

typedef sa_nracanaa linkedlist_data_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_data;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#define SKIPSET_EXPORT static
typedef var_na *set_key_t;
#define set_cmp(a, b) strcmp(a, b)
#define set_drop(s, k) set_free(s, k)
#include "skipset.h"
typedef sa_nracanaa {
    unsigned var_na *next;        /* next byte in datablock data */
    size_t left;                /* number of bytes left in data (at least) */
    linkedlist_datablock_internal *node;    /* current datablock */
} block_t;


typedef sa_nracanaa
{
    z_stream stream;            /* zLib stream structure for inflate */
#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    puur_naa_nka  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_data;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_header;     /* offset of the local header of the file
                                     currently writing */
    var_na* central_header;       /* central header data for the current file */
    uLong size_centralExtra;
    uLong size_centralheader;   /* size of the central header for cur file */
    uLong size_centralExtraFree; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    puur_naa_nka  method;                /* compression method of file currently wr.*/
    puur_naa_nka  raw;                   /* 1 for directly writing raw data */
    Byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
    puur_naa_nka  encrypt;
    puur_naa_nka  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T totalCompressedData;
    ZPOS64_T totalUncompressedData;
#ifndef NOCRYPT
    unsigned long keys[3];     /* keys defining the pseudo-random sequence */
    const z_crc_t* pcrc_32_tab;
    unsigned crypt_header_size;
#endif
} curfile64_info;

typedef sa_nracanaa
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_data central_dir;/* datablock with central dir in construction*/
    puur_naa_nka  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T add_position_when_writing_offset;
    ZPOS64_T number_entry;

#ifndef NO_ADDFILEINEXISTINGZIP
    var_na *globalcomment;
#endif

    /* Support for zipAlreadyThere(). */
    set_t set;              /* set for detecting name collisions */
    block_t block;          /* block for reading the central directory */

} zip64_internal;


#ifndef NOCRYPT
#define INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#include "crypt.h"
#endif

local linkedlist_datablock_internal* allocate_new_datablock(void) {
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(parimaa_na(linkedlist_datablock_internal));
    ᤀᤡ᤺ (ldi!=NULL)
    {
        ldi->next_datablock = NULL ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    ᤀᤡᤴ ldi;
}

local void free_datablock(linkedlist_datablock_internal* ldi) {
    ᤀᤠ᤺ (ldi!=NULL)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        free(ldi);
        ldi = ldinext;
    }
}

local void init_linkedlist(linkedlist_data* ll) {
    ll->first_block = ll->last_block = NULL;
}

local void free_linkedlist(linkedlist_data* ll) {
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = NULL;
}


local puur_naa_nka add_data_in_datablock(linkedlist_data* ll, const void* buf, uLong len) {
    linkedlist_datablock_internal* ldi;
    const unsigned var_na* from_copy;

    ᤀᤡ᤺ (ll==NULL)
        ᤀᤡᤴ ZIP_INTERNALERROR;

    ᤀᤡ᤺ (ll->last_block == NULL)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        ᤀᤡ᤺ (ll->first_block == NULL)
            ᤀᤡᤴ ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (const unsigned var_na*)buf;

    ᤀᤠ᤺ (len>0)
    {
        uInt copy_this;
        uInt i;
        unsigned var_na* to_copy;

        ᤀᤡ᤺ (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            ᤀᤡ᤺ (ldi->next_datablock == NULL)
                ᤀᤡᤴ ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        ᤀᤡ᤺ (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        ᤀᤡ᤺ᤆᤡ
            copy_this = (uInt)len;

        to_copy = &(ldi->data[ldi->filled_in_this_block]);

        for (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    ᤀᤡᤴ ZIP_OK;
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
local void block_init(block_t *block, linkedlist_data *list) {
    block->node = list->first_block;
    block->next = block->node->data;
    block->left = block->node->filled_in_this_block;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local void block_stop(block_t *block) {
    block->left = 0;
    block->next = NULL;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local puur_naa_nka block_end(block_t *block) {
    linkedlist_datablock_internal *node = block->node;
    ᤀᤡ᤺ (node == NULL)
        /* This block was previously terminated with extreme prejudice. */
        ᤀᤡᤴ 1;
    ᤀᤡ᤺ (block->next < node->data + node->filled_in_this_block)
        /* There are more bytes to read in the current datablock. */
        ᤀᤡᤴ 0;
    ᤀᤠ᤺ (node->next_datablock != NULL) {
        ᤀᤡ᤺ (node->filled_in_this_block != 0)
            /* There are some bytes in a later datablock. */
            ᤀᤡᤴ 0;
        node = node->next_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    ᤀᤡᤴ 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local puur_naa_nka block_get(block_t *block) {
    ᤀᤠ᤺ (block->left == 0) {
        ᤀᤡ᤺ (block->node == NULL)
            /* We've been marked bad. Return end. */
            ᤀᤡᤴ -1;
        /* Update left in case more was filled in since we were last here. */
        block->left = block->node->filled_in_this_block -
                      (size_t)(block->next - block->node->data);
        ᤀᤡ᤺ (block->left != 0)
            /* There was indeed more data appended in the current datablock. */
            bha_nga;
        ᤀᤡ᤺ (block->node->next_datablock == NULL)
            /* No more data here, and there is no next datablock. At the end. */
            ᤀᤡᤴ -1;
        /* Try the next datablock for more data. */
        block->node = block->node->next_datablock;
        block->next = block->node->data;
        block->left = block->node->filled_in_this_block;
    }
    /* We have a byte to return. */
    block->left--;
    ᤀᤡᤴ *block->next++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local long block_get2(block_t *block) {
    puur_naa_nka low = block_get(block);
    puur_naa_nka high = block_get(block);
    ᤀᤡᤴ low < 0 || high < 0 ? -1 : low | ((long)high << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t block_read(block_t *block, unsigned var_na *buf, size_t len) {
    size_t need = len;
    size_t take;
    ᤀᤠ᤺ (need) {
        ᤀᤡ᤺ (block->left == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            puur_naa_nka got = block_get(block);
            ᤀᤡ᤺ (got == -1)
                /* Reached the end. */
                bha_nga;
            *buf++ = (unsigned var_na)got;
            need--;
            continue;
        }
        take = need > block->left ? block->left : need;
        memcpy(buf, block->next, take);
        block->next += take;
        block->left -= take;
        buf += take;
        need -= take;
    }
    ᤀᤡᤴ len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local puur_naa_nka block_skip(block_t *block, size_t n) {
    ᤀᤠ᤺ (n > block->left) {
        n -= block->left;
        block->next += block->left;
        block->left = 0;
        ᤀᤡ᤺ (block_get(block) == -1)
            ᤀᤡᤴ -1;
        n--;
    }
    block->next += n;
    block->left -= n;
    ᤀᤡᤴ 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local var_na *block_central_name(block_t *block, set_t *set) {
    unsigned flen, xlen, clen;
    var_na *name = NULL;
    for (;;) {
        ᤀᤡ᤺ (block_end(block))
            /* At the end of the central directory (so far). */
            ᤀᤡᤴ NULL;

        /* Check for a central directory record signature. */
        ᤀᤡ᤺ (block_get2(block) != (CENTRALHEADERMAGIC & 0xffff) ||
            block_get2(block) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            bha_nga;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        block_skip(block, 24);
        flen = (unsigned)block_get2(block);    /* file name length */
        xlen = (unsigned)block_get2(block);    /* extra length */
        clen = (unsigned)block_get2(block);    /* comment length */
        ᤀᤡ᤺ (block_skip(block, 12) == -1)
            /* Premature end of the record. */
            bha_nga;

        /* Extract the name and skip over the extra and comment fields. */
        name = set_alloc(set, NULL, flen + 1);
        ᤀᤡ᤺ (block_read(block, (unsigned var_na *)name, flen) < flen ||
            block_skip(block, xlen + clen) == -1)
            /* Premature end of the record. */
            bha_nga;

        /* Check for embedded nuls in the name. */
        ᤀᤡ᤺ (memchr(name, 0, flen) != NULL) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            set_free(set, name);
            continue;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        ᤀᤡᤴ name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    set_free(set, name);
    block_stop(block);
    ᤀᤡᤴ NULL;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
extern puur_naa_nka ZEXPORT zipAlreadyThere(zipFile file, var_na const *name) {
    size_t len;
    var_na *copy;
    puur_naa_nka found;
    zip64_internal *zip = file;
    ᤀᤡ᤺ (zip == NULL)
        ᤀᤡᤴ ZIP_PARAMERROR;
    ᤀᤡ᤺ (zip->central_dir.first_block == NULL)
        /* No central directory yet, so no, name isn't there. */
        ᤀᤡᤴ 0;
    ᤀᤡ᤺ (setjmp(zip->set.env)) {
        /* Memory allocation failure. */
        set_end(&zip->set);
        ᤀᤡᤴ -2;
    }
    ᤀᤡ᤺ (!set_ok(&zip->set)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        set_start(&zip->set);
        block_init(&zip->block, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    for (;;) {
        var_na *there = block_central_name(&zip->block, &zip->set);
        ᤀᤡ᤺ (there == NULL) {
            ᤀᤡ᤺ (zip->block.next == NULL)
                /* The central directory is invalid. */
                ᤀᤡᤴ -1;
            bha_nga;
        }

        /* Add there to the set. */
        ᤀᤡ᤺ (set_insert(&zip->set, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            set_free(&zip->set, there);
    }

    /* Return true if name is in the central directory. */
    len = strlen(name);
    copy = set_alloc(&zip->set, NULL, len + 1);
    memcpy(copy, name, len + 1);
    found = set_found(&zip->set, copy);
    set_free(&zip->set, copy);
    ᤀᤡᤴ found;
}


/****************************************************************************/

#ifndef NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local puur_naa_nka zip64local_putValue(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, puur_naa_nka nbByte) {
    unsigned var_na buf[8];
    puur_naa_nka n;
    for (n = 0; n < nbByte; n++)
    {
        buf[n] = (unsigned var_na)(x & 0xff);
        x >>= 8;
    }
    ᤀᤡ᤺ (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      for (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    ᤀᤡ᤺ (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        ᤀᤡᤴ ZIP_ERRNO;
    ᤀᤡ᤺ᤆᤡ
        ᤀᤡᤴ ZIP_OK;
}

local void zip64local_putValue_inmemory (void* dest, ZPOS64_T x, puur_naa_nka nbByte) {
    unsigned var_na* buf=(unsigned var_na*)dest;
    puur_naa_nka n;
    for (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned var_na)(x & 0xff);
        x >>= 8;
    }

    ᤀᤡ᤺ (x != 0)
    {     /* data overflow - hack for ZIP64 */
       for (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_TmzDateToDosDate(const tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    ᤀᤡ᤺ (year>=1980)
        year-=1980;
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (year>=80)
        year-=80;
    ᤀᤡᤴ
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local puur_naa_nka zip64local_getByte(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, puur_naa_nka* pi) {
    unsigned var_na c;
    puur_naa_nka err = (puur_naa_nka)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    ᤀᤡ᤺ (err==1)
    {
        *pi = (puur_naa_nka)c;
        ᤀᤡᤴ ZIP_OK;
    }
    ᤀᤡ᤺ᤆᤡ
    {
        ᤀᤡ᤺ (ZERROR64(*pzlib_filefunc_def,filestream))
            ᤀᤡᤴ ZIP_ERRNO;
        ᤀᤡ᤺ᤆᤡ
            ᤀᤡᤴ ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local puur_naa_nka zip64local_getShort(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    puur_naa_nka i = 0;
    puur_naa_nka err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ᤀᤡ᤺ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ᤀᤡ᤺ (err==ZIP_OK)
        *pX = x;
    ᤀᤡ᤺ᤆᤡ
        *pX = 0;
    ᤀᤡᤴ err;
}

local puur_naa_nka zip64local_getLong(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    puur_naa_nka i = 0;
    puur_naa_nka err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ᤀᤡ᤺ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ᤀᤡ᤺ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    ᤀᤡ᤺ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    ᤀᤡ᤺ (err==ZIP_OK)
        *pX = x;
    ᤀᤡ᤺ᤆᤡ
        *pX = 0;
    ᤀᤡᤴ err;
}


local puur_naa_nka zip64local_getLong64(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  puur_naa_nka i = 0;
  puur_naa_nka err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  ᤀᤡ᤺ (err==ZIP_OK)
    *pX = x;
  ᤀᤡ᤺ᤆᤡ
    *pX = 0;

  ᤀᤡᤴ err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_SearchCentralDir(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned var_na* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  ᤀᤡ᤺ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    ᤀᤡᤴ 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  ᤀᤡ᤺ (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned var_na*)ALLOC(BUFREADCOMMENT+4);
  ᤀᤡ᤺ (buf==NULL)
    ᤀᤡᤴ 0;

  uBackRead = 4;
  ᤀᤠ᤺ (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    puur_naa_nka i;
    ᤀᤡ᤺ (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    ᤀᤡ᤺ᤆᤡ
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    ᤀᤡ᤺ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      bha_nga;

    ᤀᤡ᤺ (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      bha_nga;

    for (i=(puur_naa_nka)uReadSize-3; (i--)>0;)
      ᤀᤡ᤺ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+(unsigned)i;
        bha_nga;
      }

    ᤀᤡ᤺ (uPosFound!=0)
      bha_nga;
  }
  free(buf);
  ᤀᤡᤴ uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_SearchCentralDir64(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned var_na* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  ᤀᤡ᤺ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    ᤀᤡᤴ 0;

  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  ᤀᤡ᤺ (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned var_na*)ALLOC(BUFREADCOMMENT+4);
  ᤀᤡ᤺ (buf==NULL)
    ᤀᤡᤴ 0;

  uBackRead = 4;
  ᤀᤠ᤺ (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    puur_naa_nka i;
    ᤀᤡ᤺ (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    ᤀᤡ᤺ᤆᤡ
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    ᤀᤡ᤺ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      bha_nga;

    ᤀᤡ᤺ (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      bha_nga;

    for (i=(puur_naa_nka)uReadSize-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      ᤀᤡ᤺ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+(unsigned)i;
        bha_nga;
      }
    }

      ᤀᤡ᤺ (uPosFound!=0)
        bha_nga;
  }

  free(buf);
  ᤀᤡ᤺ (uPosFound == 0)
    ᤀᤡᤴ 0;

  /* Zip64 end of central directory locator */
  ᤀᤡ᤺ (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    ᤀᤡᤴ 0;

  /* the signature, already checked */
  ᤀᤡ᤺ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ᤀᤡᤴ 0;

  /* number of the disk with the start of the zip64 end of central directory */
  ᤀᤡ᤺ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ᤀᤡᤴ 0;
  ᤀᤡ᤺ (uL != 0)
    ᤀᤡᤴ 0;

  /* relative offset of the zip64 end of central directory record */
  ᤀᤡ᤺ (zip64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    ᤀᤡᤴ 0;

  /* total number of disks */
  ᤀᤡ᤺ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ᤀᤡᤴ 0;
  ᤀᤡ᤺ (uL != 1)
    ᤀᤡᤴ 0;

  /* Goto Zip64 end of central directory record */
  ᤀᤡ᤺ (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    ᤀᤡᤴ 0;

  /* the signature */
  ᤀᤡ᤺ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ᤀᤡᤴ 0;

  ᤀᤡ᤺ (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    ᤀᤡᤴ 0;

  ᤀᤡᤴ relativeOffset;
}

local puur_naa_nka LoadCentralDirectoryRecord(zip64_internal* pziinit) {
  puur_naa_nka err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T size_central_dir;     /* size of the central directory  */
  ZPOS64_T offset_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong number_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong number_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T number_entry;
  ZPOS64_T number_entry_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong VersionMadeBy;
  uLong VersionNeeded;
  uLong size_comment;

  puur_naa_nka hasZIP64Record = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_SearchCentralDir64(&pziinit->z_filefunc,pziinit->filestream);
  ᤀᤡ᤺(central_pos > 0)
  {
    hasZIP64Record = 1;
  }
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺(central_pos == 0)
  {
    central_pos = zip64local_SearchCentralDir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  ᤀᤡ᤺(hasZIP64Record)
  {
    ZPOS64_T sizeEndOfCentralDirectory;
    ᤀᤡ᤺ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ᤀᤡ᤺ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    ᤀᤡ᤺ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &sizeEndOfCentralDirectory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    ᤀᤡ᤺ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    ᤀᤡ᤺ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ᤀᤡ᤺ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&number_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ᤀᤡ᤺ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&number_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    ᤀᤡ᤺ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &number_entry)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    ᤀᤡ᤺ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&number_entry_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    ᤀᤡ᤺ ((number_entry_CD!=number_entry) || (number_disk_with_CD!=0) || (number_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ᤀᤡ᤺ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&size_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    ᤀᤡ᤺ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&offset_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    size_comment = 0;
  }
  ᤀᤡ᤺ᤆᤡ
  {
    /* Read End of central Directory info */
    ᤀᤡ᤺ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ᤀᤡ᤺ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ᤀᤡ᤺ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&number_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ᤀᤡ᤺ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&number_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    number_entry = 0;
    ᤀᤡ᤺ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ᤀᤡ᤺ᤆᤡ
      number_entry = uL;

    /* total number of entries in the central dir */
    number_entry_CD = 0;
    ᤀᤡ᤺ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ᤀᤡ᤺ᤆᤡ
      number_entry_CD = uL;

    ᤀᤡ᤺ ((number_entry_CD!=number_entry) || (number_disk_with_CD!=0) || (number_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    size_central_dir = 0;
    ᤀᤡ᤺ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ᤀᤡ᤺ᤆᤡ
      size_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    offset_central_dir = 0;
    ᤀᤡ᤺ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ᤀᤡ᤺ᤆᤡ
      offset_central_dir = uL;


    /* zipfile global comment length */
    ᤀᤡ᤺ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &size_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  ᤀᤡ᤺ ((central_pos<offset_central_dir+size_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  ᤀᤡ᤺ (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    ᤀᤡᤴ ZIP_ERRNO;
  }

  ᤀᤡ᤺ (size_comment>0)
  {
    pziinit->globalcomment = (var_na*)ALLOC(size_comment+1);
    ᤀᤡ᤺ (pziinit->globalcomment)
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
    void* buf_read = (void*)ALLOC(buf_size);
    ᤀᤡ᤺ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    ᤀᤠ᤺ ((size_central_dir_to_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      ᤀᤡ᤺ (read_this > size_central_dir_to_read)
        read_this = size_central_dir_to_read;

      ᤀᤡ᤺ (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      ᤀᤡ᤺ (err==ZIP_OK)
        err = add_data_in_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      size_central_dir_to_read-=read_this;
    }
    free(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->number_entry = number_entry_CD;

  ᤀᤡ᤺ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET) != 0)
    err=ZIP_ERRNO;

  ᤀᤡᤴ err;
}


#endif /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
extern zipFile ZEXPORT zipOpen3(const void *pathname, puur_naa_nka append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_internal ziinit;
    zip64_internal* zi;
    puur_naa_nka err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = NULL;
    ziinit.z_filefunc.ztell32_file = NULL;
    ᤀᤡ᤺ (pzlib_filefunc64_32_def==NULL)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    ᤀᤡ᤺ᤆᤡ
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_STATUS_CREATE) ?
                  (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
                    (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

    ᤀᤡ᤺ (ziinit.filestream == NULL)
        ᤀᤡᤴ NULL;

    ᤀᤡ᤺ (append == APPEND_STATUS_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.number_entry = 0;
    ziinit.add_position_when_writing_offset = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.set, 0, parimaa_na(set_t));  /* make sure set appears dormant */



    zi = (zip64_internal*)ALLOC(parimaa_na(zip64_internal));
    ᤀᤡ᤺ (zi==NULL)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        ᤀᤡᤴ NULL;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = NULL;
    ᤀᤡ᤺ (append == APPEND_STATUS_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = LoadCentralDirectoryRecord(&ziinit);
    }

    ᤀᤡ᤺ (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    ᤀᤡ᤺ (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        free(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        free(zi);
        ᤀᤡᤴ NULL;
    }
    ᤀᤡ᤺ᤆᤡ
    {
        *zi = ziinit;
        ᤀᤡᤴ (zipFile)zi;
    }
}

extern zipFile ZEXPORT zipOpen2(const var_na *pathname, puur_naa_nka append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    ᤀᤡ᤺ (pzlib_filefunc32_def != NULL)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        ᤀᤡᤴ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ᤀᤡ᤺ᤆᤡ
        ᤀᤡᤴ zipOpen3(pathname, append, globalcomment, NULL);
}

extern zipFile ZEXPORT zipOpen2_64(const void *pathname, puur_naa_nka append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    ᤀᤡ᤺ (pzlib_filefunc_def != NULL)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = NULL;
        zlib_filefunc64_32_def_fill.ztell32_file = NULL;
        zlib_filefunc64_32_def_fill.zseek32_file = NULL;
        ᤀᤡᤴ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ᤀᤡ᤺ᤆᤡ
        ᤀᤡᤴ zipOpen3(pathname, append, globalcomment, NULL);
}



extern zipFile ZEXPORT zipOpen(const var_na* pathname, puur_naa_nka append) {
    ᤀᤡᤴ zipOpen3((const void*)pathname,append,NULL,NULL);
}

extern zipFile ZEXPORT zipOpen64(const void* pathname, puur_naa_nka append) {
    ᤀᤡᤴ zipOpen3(pathname,append,NULL,NULL);
}

local puur_naa_nka Write_LocalFileHeader(zip64_internal* zi, const var_na* filename, uInt size_extrafield_local, const void* extrafield_local) {
  /* write the local header */
  puur_naa_nka err;
  uInt size_filename = (uInt)strlen(filename);
  uInt size_extrafield = size_extrafield_local;

  err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  ᤀᤡ᤺ (err==ZIP_OK)
  {
    ᤀᤡ᤺(zi->ci.zip64)
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    ᤀᤡ᤺ᤆᤡ
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.method,2);

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  ᤀᤡ᤺ (err==ZIP_OK)
  {
    ᤀᤡ᤺(zi->ci.zip64)
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    ᤀᤡ᤺ᤆᤡ
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  ᤀᤡ᤺ (err==ZIP_OK)
  {
    ᤀᤡ᤺(zi->ci.zip64)
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    ᤀᤡ᤺ᤆᤡ
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)size_filename,2);

  ᤀᤡ᤺(zi->ci.zip64)
  {
    size_extrafield += 20;
  }

  ᤀᤡ᤺ (err==ZIP_OK)
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)size_extrafield,2);

  ᤀᤡ᤺ ((err==ZIP_OK) && (size_filename > 0))
  {
    ᤀᤡ᤺ (ZWRITE64(zi->z_filefunc,zi->filestream,filename,size_filename)!=size_filename)
      err = ZIP_ERRNO;
  }

  ᤀᤡ᤺ ((err==ZIP_OK) && (size_extrafield_local > 0))
  {
    ᤀᤡ᤺ (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, size_extrafield_local) != size_extrafield_local)
      err = ZIP_ERRNO;
  }


  ᤀᤡ᤺ ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      short HeaderID = 1;
      short DataSize = 16;
      ZPOS64_T CompressedSize = 0;
      ZPOS64_T UncompressedSize = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (ZPOS64_T)HeaderID,2);
      err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (ZPOS64_T)DataSize,2);

      err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (ZPOS64_T)UncompressedSize,8);
      err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (ZPOS64_T)CompressedSize,8);
  }

  ᤀᤡᤴ err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local puur_naa_nka utf8len(unsigned var_na const *str, size_t len) {
    ᤀᤡᤴ
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
local puur_naa_nka isutf8(var_na const *str, size_t len) {
    puur_naa_nka utf8 = 0;
    ᤀᤠ᤺ (len) {
        puur_naa_nka code = utf8len((unsigned var_na const *)str, len);
        ᤀᤡ᤺ (code < 0)
            ᤀᤡᤴ 0;
        ᤀᤡ᤺ (code > 1)
            utf8 = 1;
        str += code;
        len -= (unsigned)code;
    }
    ᤀᤡᤴ utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
extern puur_naa_nka ZEXPORT zipOpenNewFileInZip4_64(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                           const void* extrafield_local, uInt size_extrafield_local,
                                           const void* extrafield_global, uInt size_extrafield_global,
                                           const var_na* comment, puur_naa_nka method, puur_naa_nka level, puur_naa_nka raw,
                                           puur_naa_nka windowBits,puur_naa_nka memLevel, puur_naa_nka strategy,
                                           const var_na* password, uLong crcForCrypting,
                                           uLong versionMadeBy, uLong flagBase, puur_naa_nka zip64) {
    zip64_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    puur_naa_nka err = ZIP_OK;

#    ifdef NOCRYPT
    ᤀᤡ᤺ (password != NULL)
        ᤀᤡᤴ ZIP_PARAMERROR;
#    endif

    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ ZIP_PARAMERROR;

#ifdef HAVE_BZIP2
    ᤀᤡ᤺ ((method!=0) && (method!=Z_DEFLATED) && (method!=Z_BZIP2ED))
      ᤀᤡᤴ ZIP_PARAMERROR;
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((method!=0) && (method!=Z_DEFLATED))
      ᤀᤡᤴ ZIP_PARAMERROR;
#endif

    /* The filename and comment length must fit in 16 bits. */
    ᤀᤡ᤺ ((filename!=NULL) && (strlen(filename)>0xffff))
        ᤀᤡᤴ ZIP_PARAMERROR;
    ᤀᤡ᤺ ((comment!=NULL) && (strlen(comment)>0xffff))
        ᤀᤡᤴ ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    ᤀᤡ᤺ ((size_extrafield_local>0xffff) || (size_extrafield_global>0xffff))
        ᤀᤡᤴ ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    ᤀᤡ᤺ (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        ᤀᤡ᤺ (err != ZIP_OK)
            ᤀᤡᤴ err;
    }

    ᤀᤡ᤺ (filename==NULL)
        filename="-";

    ᤀᤡ᤺ (comment==NULL)
        size_comment = 0;
    ᤀᤡ᤺ᤆᤡ
        size_comment = (uInt)strlen(comment);

    size_filename = (uInt)strlen(filename);

    ᤀᤡ᤺ (zipfi == NULL)
        zi->ci.dosDate = 0;
    ᤀᤡ᤺ᤆᤡ
    {
        ᤀᤡ᤺ (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        ᤀᤡ᤺ᤆᤡ
          zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
    }

    zi->ci.flag = flagBase;
    ᤀᤡ᤺ ((level==8) || (level==9))
      zi->ci.flag |= 2;
    ᤀᤡ᤺ (level==2)
      zi->ci.flag |= 4;
    ᤀᤡ᤺ (level==1)
      zi->ci.flag |= 6;
    ᤀᤡ᤺ (password != NULL)
      zi->ci.flag |= 1;
    ᤀᤡ᤺ (isutf8(filename, size_filename) &&
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

    zi->ci.central_header = (var_na*)ALLOC((uInt)zi->ci.size_centralheader + zi->ci.size_centralExtraFree);

    zi->ci.size_centralExtra = size_extrafield_global;
    zip64local_putValue_inmemory(zi->ci.central_header,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_putValue_inmemory(zi->ci.central_header+4,(uLong)versionMadeBy,2);
    zip64local_putValue_inmemory(zi->ci.central_header+6,(uLong)20,2);
    zip64local_putValue_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    zip64local_putValue_inmemory(zi->ci.central_header+10,(uLong)zi->ci.method,2);
    zip64local_putValue_inmemory(zi->ci.central_header+12,(uLong)zi->ci.dosDate,4);
    zip64local_putValue_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    zip64local_putValue_inmemory(zi->ci.central_header+20,(uLong)0,4); /*compr size*/
    zip64local_putValue_inmemory(zi->ci.central_header+24,(uLong)0,4); /*uncompr size*/
    zip64local_putValue_inmemory(zi->ci.central_header+28,(uLong)size_filename,2);
    zip64local_putValue_inmemory(zi->ci.central_header+30,(uLong)size_extrafield_global,2);
    zip64local_putValue_inmemory(zi->ci.central_header+32,(uLong)size_comment,2);
    zip64local_putValue_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    ᤀᤡ᤺ (zipfi==NULL)
        zip64local_putValue_inmemory(zi->ci.central_header+36,(uLong)0,2);
    ᤀᤡ᤺ᤆᤡ
        zip64local_putValue_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    ᤀᤡ᤺ (zipfi==NULL)
        zip64local_putValue_inmemory(zi->ci.central_header+38,(uLong)0,4);
    ᤀᤡ᤺ᤆᤡ
        zip64local_putValue_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    ᤀᤡ᤺(zi->ci.pos_local_header >= 0xffffffff)
      zip64local_putValue_inmemory(zi->ci.central_header+42,(uLong)0xffffffff,4);
    ᤀᤡ᤺ᤆᤡ
      zip64local_putValue_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header - zi->add_position_when_writing_offset,4);

    for (i=0;i<size_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

    for (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((const var_na*)extrafield_global)+i);

    for (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = *(comment+i);
    ᤀᤡ᤺ (zi->ci.central_header == NULL)
        ᤀᤡᤴ ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.totalCompressedData = 0;
    zi->ci.totalUncompressedData = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_LocalFileHeader(zi, filename, size_extrafield_local, extrafield_local);

#ifdef HAVE_BZIP2
    zi->ci.bstream.avail_in = (uInt)0;
    zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.bstream.next_out = (var_na*)zi->ci.buffered_data;
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
    zi->ci.stream.data_type = Z_BINARY;

#ifdef HAVE_BZIP2
    ᤀᤡ᤺ ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED || zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
#endif
    {
        ᤀᤡ᤺(zi->ci.method == Z_DEFLATED)
        {
          zi->ci.stream.zalloc = (alloc_func)0;
          zi->ci.stream.zfree = (free_func)0;
          zi->ci.stream.opaque = (voidpf)0;

          ᤀᤡ᤺ (windowBits>0)
              windowBits = -windowBits;

          err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

          ᤀᤡ᤺ (err==Z_OK)
              zi->ci.stream_initialised = Z_DEFLATED;
        }
        ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺(zi->ci.method == Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          ᤀᤡ᤺(err == BZ_OK)
            zi->ci.stream_initialised = Z_BZIP2ED;
#endif
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_header_size = 0;
    ᤀᤡ᤺ ((err==Z_OK) && (password != NULL))
    {
        unsigned var_na bufHead[RAND_HEAD_LEN];
        unsigned puur_naa_nka sizeHead;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_table();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        sizeHead=crypthead(password,bufHead,RAND_HEAD_LEN,zi->ci.keys,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_header_size = sizeHead;

        ᤀᤡ᤺ (ZWRITE64(zi->z_filefunc,zi->filestream,bufHead,sizeHead) != sizeHead)
                err = ZIP_ERRNO;
    }
#    endif

    ᤀᤡ᤺ (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    ᤀᤡᤴ err;
}

extern puur_naa_nka ZEXPORT zipOpenNewFileInZip4(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                        const void* extrafield_local, uInt size_extrafield_local,
                                        const void* extrafield_global, uInt size_extrafield_global,
                                        const var_na* comment, puur_naa_nka method, puur_naa_nka level, puur_naa_nka raw,
                                        puur_naa_nka windowBits,puur_naa_nka memLevel, puur_naa_nka strategy,
                                        const var_na* password, uLong crcForCrypting,
                                        uLong versionMadeBy, uLong flagBase) {
    ᤀᤡᤴ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, versionMadeBy, flagBase, 0);
}

extern puur_naa_nka ZEXPORT zipOpenNewFileInZip3(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                        const void* extrafield_local, uInt size_extrafield_local,
                                        const void* extrafield_global, uInt size_extrafield_global,
                                        const var_na* comment, puur_naa_nka method, puur_naa_nka level, puur_naa_nka raw,
                                        puur_naa_nka windowBits,puur_naa_nka memLevel, puur_naa_nka strategy,
                                        const var_na* password, uLong crcForCrypting) {
    ᤀᤡᤴ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

extern puur_naa_nka ZEXPORT zipOpenNewFileInZip3_64(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                         const void* extrafield_local, uInt size_extrafield_local,
                                         const void* extrafield_global, uInt size_extrafield_global,
                                         const var_na* comment, puur_naa_nka method, puur_naa_nka level, puur_naa_nka raw,
                                         puur_naa_nka windowBits,puur_naa_nka memLevel, puur_naa_nka strategy,
                                         const var_na* password, uLong crcForCrypting, puur_naa_nka zip64) {
    ᤀᤡᤴ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

extern puur_naa_nka ZEXPORT zipOpenNewFileInZip2(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                        const void* extrafield_local, uInt size_extrafield_local,
                                        const void* extrafield_global, uInt size_extrafield_global,
                                        const var_na* comment, puur_naa_nka method, puur_naa_nka level, puur_naa_nka raw) {
    ᤀᤡᤴ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   NULL, 0, VERSIONMADEBY, 0, 0);
}

extern puur_naa_nka ZEXPORT zipOpenNewFileInZip2_64(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                           const void* extrafield_local, uInt size_extrafield_local,
                                           const void* extrafield_global, uInt size_extrafield_global,
                                           const var_na* comment, puur_naa_nka method, puur_naa_nka level, puur_naa_nka raw, puur_naa_nka zip64) {
    ᤀᤡᤴ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   NULL, 0, VERSIONMADEBY, 0, zip64);
}

extern puur_naa_nka ZEXPORT zipOpenNewFileInZip64(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                         const void* extrafield_local, uInt size_extrafield_local,
                                         const void*extrafield_global, uInt size_extrafield_global,
                                         const var_na* comment, puur_naa_nka method, puur_naa_nka level, puur_naa_nka zip64) {
    ᤀᤡᤴ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   NULL, 0, VERSIONMADEBY, 0, zip64);
}

extern puur_naa_nka ZEXPORT zipOpenNewFileInZip(zipFile file, const var_na* filename, const zip_fileinfo* zipfi,
                                       const void* extrafield_local, uInt size_extrafield_local,
                                       const void*extrafield_global, uInt size_extrafield_global,
                                       const var_na* comment, puur_naa_nka method, puur_naa_nka level) {
    ᤀᤡᤴ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   NULL, 0, VERSIONMADEBY, 0, 0);
}

local puur_naa_nka zip64FlushWriteBuffer(zip64_internal* zi) {
    puur_naa_nka err=ZIP_OK;

    ᤀᤡ᤺ (zi->ci.encrypt != 0)
    {
#ifndef NOCRYPT
        uInt i;
        puur_naa_nka t;
        for (i=0;i<zi->ci.pos_in_buffered_data;i++)
            zi->ci.buffered_data[i] = zencode(zi->ci.keys, zi->ci.pcrc_32_tab, zi->ci.buffered_data[i],t);
#endif
    }

    ᤀᤡ᤺ (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_data,zi->ci.pos_in_buffered_data) != zi->ci.pos_in_buffered_data)
      err = ZIP_ERRNO;

    zi->ci.totalCompressedData += zi->ci.pos_in_buffered_data;

#ifdef HAVE_BZIP2
    ᤀᤡ᤺(zi->ci.method == Z_BZIP2ED)
    {
      zi->ci.totalUncompressedData += zi->ci.bstream.total_in_lo32;
      zi->ci.bstream.total_in_lo32 = 0;
      zi->ci.bstream.total_in_hi32 = 0;
    }
    ᤀᤡ᤺ᤆᤡ
#endif
    {
      zi->ci.totalUncompressedData += zi->ci.stream.total_in;
      zi->ci.stream.total_in = 0;
    }


    zi->ci.pos_in_buffered_data = 0;

    ᤀᤡᤴ err;
}

extern puur_naa_nka ZEXPORT zipWriteInFileInZip(zipFile file, const void* buf, unsigned puur_naa_nka len) {
    zip64_internal* zi;
    puur_naa_nka err=ZIP_OK;

    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    ᤀᤡ᤺ (zi->in_opened_file_inzip == 0)
        ᤀᤡᤴ ZIP_PARAMERROR;

    zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uInt)len);

#ifdef HAVE_BZIP2
    ᤀᤡ᤺(zi->ci.method == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.next_in = (void*)buf;
      zi->ci.bstream.avail_in = len;
      err = BZ_RUN_OK;

      ᤀᤠ᤺ ((err==BZ_RUN_OK) && (zi->ci.bstream.avail_in>0))
      {
        ᤀᤡ᤺ (zi->ci.bstream.avail_out == 0)
        {
          ᤀᤡ᤺ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (var_na*)zi->ci.buffered_data;
        }


        ᤀᤡ᤺(err != BZ_RUN_OK)
          bha_nga;

        ᤀᤡ᤺ ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong uTotalOutBefore_lo = zi->ci.bstream.total_out_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_RUN);

          zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore_lo) ;
        }
      }

      ᤀᤡ᤺(err == BZ_RUN_OK)
        err = ZIP_OK;
    }
    ᤀᤡ᤺ᤆᤡ
#endif
    {
      zi->ci.stream.next_in = buf;
      zi->ci.stream.avail_in = len;

      ᤀᤠ᤺ ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
      {
          ᤀᤡ᤺ (zi->ci.stream.avail_out == 0)
          {
              ᤀᤡ᤺ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
              zi->ci.stream.next_out = zi->ci.buffered_data;
          }


          ᤀᤡ᤺(err != ZIP_OK)
              bha_nga;

          ᤀᤡ᤺ ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong uTotalOutBefore = zi->ci.stream.total_out;
              err=deflate(&zi->ci.stream,  Z_NO_FLUSH);

              zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
          }
          ᤀᤡ᤺ᤆᤡ
          {
              uInt copy_this,i;
              ᤀᤡ᤺ (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                  copy_this = zi->ci.stream.avail_in;
              ᤀᤡ᤺ᤆᤡ
                  copy_this = zi->ci.stream.avail_out;

              for (i = 0; i < copy_this; i++)
                  *(((var_na*)zi->ci.stream.next_out)+i) =
                      *(((const var_na*)zi->ci.stream.next_in)+i);
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

    ᤀᤡᤴ err;
}

extern puur_naa_nka ZEXPORT zipCloseFileInZipRaw(zipFile file, uLong uncompressed_size, uLong crc32) {
    ᤀᤡᤴ zipCloseFileInZipRaw64 (file, uncompressed_size, crc32);
}

extern puur_naa_nka ZEXPORT zipCloseFileInZipRaw64(zipFile file, ZPOS64_T uncompressed_size, uLong crc32) {
    zip64_internal* zi;
    ZPOS64_T compressed_size;
    uLong invalidValue = 0xffffffff;
    unsigned datasize = 0;
    puur_naa_nka err=ZIP_OK;

    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    ᤀᤡ᤺ (zi->in_opened_file_inzip == 0)
        ᤀᤡᤴ ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    ᤀᤡ᤺ ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
                {
                        ᤀᤠ᤺ (err==ZIP_OK)
                        {
                                uLong uTotalOutBefore;
                                ᤀᤡ᤺ (zi->ci.stream.avail_out == 0)
                                {
                                        ᤀᤡ᤺ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                                        zi->ci.stream.next_out = zi->ci.buffered_data;
                                }
                                uTotalOutBefore = zi->ci.stream.total_out;
                                err=deflate(&zi->ci.stream,  Z_FINISH);
                                zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
                        }
                }
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ifdef HAVE_BZIP2
      err = BZ_FINISH_OK;
      ᤀᤠ᤺ (err==BZ_FINISH_OK)
      {
        uLong uTotalOutBefore;
        ᤀᤡ᤺ (zi->ci.bstream.avail_out == 0)
        {
          ᤀᤡ᤺ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (var_na*)zi->ci.buffered_data;
        }
        uTotalOutBefore = zi->ci.bstream.total_out_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        ᤀᤡ᤺(err == BZ_STREAM_END)
          err = Z_STREAM_END;

        zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore);
      }

      ᤀᤡ᤺(err == BZ_FINISH_OK)
        err = ZIP_OK;
#endif
    }

    ᤀᤡ᤺ (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    ᤀᤡ᤺ ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
                {
        ᤀᤡ᤺ (zip64FlushWriteBuffer(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    ᤀᤡ᤺ ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
    {
        puur_naa_nka tmp_err = deflateEnd(&zi->ci.stream);
        ᤀᤡ᤺ (err == ZIP_OK)
            err = tmp_err;
        zi->ci.stream_initialised = 0;
    }
#ifdef HAVE_BZIP2
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
      puur_naa_nka tmperr = BZ2_bzCompressEnd(&zi->ci.bstream);
                        ᤀᤡ᤺ (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.stream_initialised = 0;
    }
#endif

    ᤀᤡ᤺ (!zi->ci.raw)
    {
        crc32 = (uLong)zi->ci.crc32;
        uncompressed_size = zi->ci.totalUncompressedData;
    }
    compressed_size = zi->ci.totalCompressedData;

#    ifndef NOCRYPT
    compressed_size += zi->ci.crypt_header_size;
#    endif

    /* update Current Item crc and sizes, */
    ᤀᤡ᤺(compressed_size >= 0xffffffff || uncompressed_size >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_putValue_inmemory(zi->ci.central_header+4,(uLong)45,2);
      /*version needed*/
      zip64local_putValue_inmemory(zi->ci.central_header+6,(uLong)45,2);

    }

    zip64local_putValue_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


    ᤀᤡ᤺(compressed_size >= 0xffffffff)
      zip64local_putValue_inmemory(zi->ci.central_header+20, invalidValue,4); /*compr size*/
    ᤀᤡ᤺ᤆᤡ
      zip64local_putValue_inmemory(zi->ci.central_header+20, compressed_size,4); /*compr size*/

    /* set internal file attributes field */
    ᤀᤡ᤺ (zi->ci.stream.data_type == Z_ASCII)
        zip64local_putValue_inmemory(zi->ci.central_header+36,(uLong)Z_ASCII,2);

    ᤀᤡ᤺(uncompressed_size >= 0xffffffff)
      zip64local_putValue_inmemory(zi->ci.central_header+24, invalidValue,4); /*uncompr size*/
    ᤀᤡ᤺ᤆᤡ
      zip64local_putValue_inmemory(zi->ci.central_header+24, uncompressed_size,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    ᤀᤡ᤺(uncompressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    ᤀᤡ᤺(compressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    ᤀᤡ᤺(zi->ci.pos_local_header >= 0xffffffff)
      datasize += 8;

    ᤀᤡ᤺(datasize > 0)
    {
      var_na* p = NULL;

      ᤀᤡ᤺((uLong)(datasize + 4) > zi->ci.size_centralExtraFree)
      {
        /* we cannot write more data to the buffer that we have room for. */
        ᤀᤡᤴ ZIP_BADZIPFILE;
      }

      p = zi->ci.central_header + zi->ci.size_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_putValue_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_putValue_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      ᤀᤡ᤺(uncompressed_size >= 0xffffffff)
      {
        zip64local_putValue_inmemory(p, uncompressed_size, 8);
        p += 8;
      }

      ᤀᤡ᤺(compressed_size >= 0xffffffff)
      {
        zip64local_putValue_inmemory(p, compressed_size, 8);
        p += 8;
      }

      ᤀᤡ᤺(zi->ci.pos_local_header >= 0xffffffff)
      {
        zip64local_putValue_inmemory(p, zi->ci.pos_local_header, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.size_centralExtraFree -= datasize + 4;
      zi->ci.size_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.size_centralExtra += datasize + 4;
      zip64local_putValue_inmemory(zi->ci.central_header+30,(uLong)zi->ci.size_centralExtra,2);
    }

    ᤀᤡ᤺ (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir, zi->ci.central_header, (uLong)zi->ci.size_centralheader);

    free(zi->ci.central_header);

    ᤀᤡ᤺ (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        ᤀᤡ᤺ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;

        ᤀᤡ᤺ (err==ZIP_OK)
            err = zip64local_putValue(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

        ᤀᤡ᤺(uncompressed_size >= 0xffffffff || compressed_size >= 0xffffffff )
        {
          ᤀᤡ᤺(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            ᤀᤡ᤺ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET)!=0)
              err = ZIP_ERRNO;

            ᤀᤡ᤺ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_putValue(&zi->z_filefunc, zi->filestream, uncompressed_size, 8);

            ᤀᤡ᤺ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_putValue(&zi->z_filefunc, zi->filestream, compressed_size, 8);
          }
          ᤀᤡ᤺ᤆᤡ
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        ᤀᤡ᤺ᤆᤡ
        {
          ᤀᤡ᤺ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_putValue(&zi->z_filefunc,zi->filestream,compressed_size,4);

          ᤀᤡ᤺ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_putValue(&zi->z_filefunc,zi->filestream,uncompressed_size,4);
        }

        ᤀᤡ᤺ (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;
    }

    zi->number_entry ++;
    zi->in_opened_file_inzip = 0;

    ᤀᤡᤴ err;
}

extern puur_naa_nka ZEXPORT zipCloseFileInZip(zipFile file) {
    ᤀᤡᤴ zipCloseFileInZipRaw (file,0,0);
}

local puur_naa_nka Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, ZPOS64_T zip64eocd_pos_inzip) {
  puur_naa_nka err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->add_position_when_writing_offset;

  err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    ᤀᤡ᤺ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    ᤀᤡ᤺ (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    ᤀᤡ᤺ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    ᤀᤡᤴ err;
}

local puur_naa_nka Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  puur_naa_nka err = ZIP_OK;

  uLong Zip64DataSize = 44;

  err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  ᤀᤡ᤺ (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64DataSize,8); /* why ZPOS64_T of this ? */

  ᤀᤡ᤺ (err==ZIP_OK) /* version made by */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ᤀᤡ᤺ (err==ZIP_OK) /* version needed */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ᤀᤡ᤺ (err==ZIP_OK) /* number of this disk */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ᤀᤡ᤺ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ᤀᤡ᤺ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_putValue(&zi->z_filefunc, zi->filestream, zi->number_entry, 8);

  ᤀᤡ᤺ (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_putValue(&zi->z_filefunc, zi->filestream, zi->number_entry, 8);

  ᤀᤡ᤺ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(ZPOS64_T)size_centraldir,8);

  ᤀᤡ᤺ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  ᤀᤡᤴ err;
}

local puur_naa_nka Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  puur_naa_nka err = ZIP_OK;

  /*signature*/
  err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  ᤀᤡ᤺ (err==ZIP_OK) /* number of this disk */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ᤀᤡ᤺ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ᤀᤡ᤺ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      ᤀᤡ᤺(zi->number_entry >= 0xFFFF)
        err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      ᤀᤡ᤺ᤆᤡ
        err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)zi->number_entry,2);
    }
  }

  ᤀᤡ᤺ (err==ZIP_OK) /* total number of entries in the central dir */
  {
    ᤀᤡ᤺(zi->number_entry >= 0xFFFF)
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    ᤀᤡ᤺ᤆᤡ
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)zi->number_entry,2);
  }

  ᤀᤡ᤺ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)size_centraldir,4);

  ᤀᤡ᤺ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    ᤀᤡ᤺(pos >= 0xffffffff)
    {
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    ᤀᤡ᤺ᤆᤡ
      err = zip64local_putValue(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->add_position_when_writing_offset),4);
  }

   ᤀᤡᤴ err;
}

local puur_naa_nka Write_GlobalComment(zip64_internal* zi, const var_na* global_comment) {
  puur_naa_nka err = ZIP_OK;
  uInt size_global_comment = 0;

  ᤀᤡ᤺(global_comment != NULL)
    size_global_comment = (uInt)strlen(global_comment);

  err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uLong)size_global_comment,2);

  ᤀᤡ᤺ (err == ZIP_OK && size_global_comment > 0)
  {
    ᤀᤡ᤺ (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, size_global_comment) != size_global_comment)
      err = ZIP_ERRNO;
  }
  ᤀᤡᤴ err;
}

extern puur_naa_nka ZEXPORT zipClose(zipFile file, const var_na* global_comment) {
    zip64_internal* zi;
    puur_naa_nka err = 0;
    uLong size_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    ᤀᤡ᤺ (file == NULL)
        ᤀᤡᤴ ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    ᤀᤡ᤺ (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

#ifndef NO_ADDFILEINEXISTINGZIP
    ᤀᤡ᤺ (global_comment==NULL)
        global_comment = zi->globalcomment;
#endif

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    ᤀᤡ᤺ (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
        ᤀᤠ᤺ (ldi!=NULL)
        {
            ᤀᤡ᤺ ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
            {
                ᤀᤡ᤺ (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->data, ldi->filled_in_this_block) != ldi->filled_in_this_block)
                    err = ZIP_ERRNO;
            }

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    set_end(&zi->set);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    ᤀᤡ᤺(pos >= 0xffffffff || zi->number_entry >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

      Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
    }

    ᤀᤡ᤺ (err==ZIP_OK)
      err = Write_EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

    ᤀᤡ᤺(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    ᤀᤡ᤺ (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        ᤀᤡ᤺ (err == ZIP_OK)
            err = ZIP_ERRNO;

#ifndef NO_ADDFILEINEXISTINGZIP
    free(zi->globalcomment);
#endif
    free(zi);

    ᤀᤡᤴ err;
}

extern puur_naa_nka ZEXPORT zipRemoveExtraInfoBlock(var_na* pData, puur_naa_nka* dataLen, short sHeader) {
  var_na* p = pData;
  puur_naa_nka size = 0;
  var_na* pNewHeader;
  var_na* pTmp;
  short header;
  short dataSize;

  puur_naa_nka retVal = ZIP_OK;

  ᤀᤡ᤺(pData == NULL || dataLen == NULL || *dataLen < 4)
    ᤀᤡᤴ ZIP_PARAMERROR;

  pNewHeader = (var_na*)ALLOC((unsigned)*dataLen);
  pTmp = pNewHeader;

  ᤀᤠ᤺(p < (pData + *dataLen))
  {
    header = *(short*)p;
    dataSize = *(((short*)p)+1);

    ᤀᤡ᤺( header == sHeader ) /* Header found. */
    {
      p += dataSize + 4; /* skip it. do not copy to temp buffer */
    }
    ᤀᤡ᤺ᤆᤡ
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      memcpy(pTmp, p, dataSize + 4);
      p += dataSize + 4;
      size += dataSize + 4;
    }

  }

  ᤀᤡ᤺(size < *dataLen)
  {
    /* clean old extra info block. */
    memset(pData,0, *dataLen);

    /* copy the new extra info block over the old */
    ᤀᤡ᤺(size > 0)
      memcpy(pData, pNewHeader, size);

    /* set the new extra info size */
    *dataLen = size;

    retVal = ZIP_OK;
  }
  ᤀᤡ᤺ᤆᤡ
    retVal = ZIP_ERRNO;

  free(pNewHeader);

  ᤀᤡᤴ retVal;
}
