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


#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>
#ifndef ZLIB_CONST
#  define ZLIB_CONST
#endif
#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#அந்தர்பாவ "zlib.h"
#அந்தர்பாவ "zip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    வெளி முழு errno;
#இல்லையெனில்
#   include <errno.h>
#endif


#ifndef local
#  define local நிலையான
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
#எனில் MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#இல்லையெனில்
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
மாறிலி எழுத்து zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x6064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

வகைவரை கட்டமைப்பு linkedlist_datablock_internal_s
{
  கட்டமைப்பு linkedlist_datablock_internal_s* next_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignment */
  unsigned எழுத்து data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

வகைவரை கட்டமைப்பு linkedlist_data_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_data;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#define SKIPSET_EXPORT நிலையான
வகைவரை எழுத்து *set_சாவி_t;
#define set_cmp(a, b) strcmp(a, b)
#define set_drop(s, k) set_free(s, k)
#அந்தர்பாவ "skipset.h"
வகைவரை கட்டமைப்பு {
    unsigned எழுத்து *next;        /* next byte in datablock data */
    size_t இடது;                /* number of bytes left in data (at least) */
    linkedlist_datablock_internal *முனை;    /* current datablock */
} block_t;


வகைவரை கட்டமைப்பு
{
    z_stream stream;            /* zLib stream structure for inflate */
#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    முழு  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_data;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_header;     /* offset of the local header of the file
                                     currently writing */
    எழுத்து* central_header;       /* central header data for the current file */
    uLong size_centralExtra;
    uLong size_centralheader;   /* size of the central header for cur file */
    uLong size_centralExtraFree; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    முழு  method;                /* compression method of file currently wr.*/
    முழு  raw;                   /* 1 for directly writing raw data */
    Byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
    முழு  encrypt;
    முழு  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T totalCompressedData;
    ZPOS64_T totalUncompressedData;
#ifndef NOCRYPT
    unsigned long சாவிகள்[3];     /* keys defining the pseudo-random sequence */
    மாறிலி z_crc_t* pcrc_32_tab;
    unsigned crypt_header_size;
#endif
} curfile64_info;

வகைவரை கட்டமைப்பு
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_data central_dir;/* datablock with central dir in construction*/
    முழு  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T add_position_when_writing_offset;
    ZPOS64_T எண்_entry;

#ifndef NO_ADDFILEINEXISTINGZIP
    எழுத்து *globalcomment;
#endif

    /* Support for zipAlreadyThere(). */
    set_t set;              /* set for detecting name collisions */
    block_t block;          /* block for reading the central directory */

} zip64_internal;


#ifndef NOCRYPT
#define INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#அந்தர்பாவ "crypt.h"
#endif

local linkedlist_datablock_internal* allocate_new_datablock(வெறுமை) {
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(அளவு(linkedlist_datablock_internal));
    எனில் (ldi!=வெற்று)
    {
        ldi->next_datablock = வெற்று ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    திரும்பு ldi;
}

local வெறுமை free_datablock(linkedlist_datablock_internal* ldi) {
    போது (ldi!=வெற்று)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        விடுவி(ldi);
        ldi = ldinext;
    }
}

local வெறுமை init_linkedlist(linkedlist_data* ll) {
    ll->first_block = ll->last_block = வெற்று;
}

local வெறுமை free_linkedlist(linkedlist_data* ll) {
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = வெற்று;
}


local முழு add_data_in_datablock(linkedlist_data* ll, மாறிலி வெறுமை* buf, uLong len) {
    linkedlist_datablock_internal* ldi;
    மாறிலி unsigned எழுத்து* from_copy;

    எனில் (ll==வெற்று)
        திரும்பு ZIP_INTERNALERROR;

    எனில் (ll->last_block == வெற்று)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        எனில் (ll->first_block == வெற்று)
            திரும்பு ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (மாறிலி unsigned எழுத்து*)buf;

    போது (len>0)
    {
        uInt copy_this;
        uInt i;
        unsigned எழுத்து* to_copy;

        எனில் (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            எனில் (ldi->next_datablock == வெற்று)
                திரும்பு ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        எனில் (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        இல்லையெனில்
            copy_this = (uInt)len;

        to_copy = &(ldi->data[ldi->filled_in_this_block]);

        க்கு (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    திரும்பு ZIP_OK;
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
local வெறுமை block_init(block_t *block, linkedlist_data *list) {
    block->முனை = list->first_block;
    block->next = block->முனை->data;
    block->இடது = block->முனை->filled_in_this_block;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local வெறுமை block_stop(block_t *block) {
    block->இடது = 0;
    block->next = வெற்று;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local முழு block_end(block_t *block) {
    linkedlist_datablock_internal *முனை = block->முனை;
    எனில் (முனை == வெற்று)
        /* This block was previously terminated with extreme prejudice. */
        திரும்பு 1;
    எனில் (block->next < முனை->data + முனை->filled_in_this_block)
        /* There are more bytes to read in the current datablock. */
        திரும்பு 0;
    போது (முனை->next_datablock != வெற்று) {
        எனில் (முனை->filled_in_this_block != 0)
            /* There are some bytes in a later datablock. */
            திரும்பு 0;
        முனை = முனை->next_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    திரும்பு 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local முழு block_get(block_t *block) {
    போது (block->இடது == 0) {
        எனில் (block->முனை == வெற்று)
            /* We've been marked bad. Return end. */
            திரும்பு -1;
        /* Update left in case more was filled in since we were last here. */
        block->இடது = block->முனை->filled_in_this_block -
                      (size_t)(block->next - block->முனை->data);
        எனில் (block->இடது != 0)
            /* There was indeed more data appended in the current datablock. */
            நிறுத்து;
        எனில் (block->முனை->next_datablock == வெற்று)
            /* No more data here, and there is no next datablock. At the end. */
            திரும்பு -1;
        /* Try the next datablock for more data. */
        block->முனை = block->முனை->next_datablock;
        block->next = block->முனை->data;
        block->இடது = block->முனை->filled_in_this_block;
    }
    /* We have a byte to return. */
    block->இடது--;
    திரும்பு *block->next++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local long block_get2(block_t *block) {
    முழு கீழ் = block_get(block);
    முழு உயர்வு = block_get(block);
    திரும்பு கீழ் < 0 || உயர்வு < 0 ? -1 : கீழ் | ((long)உயர்வு << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t block_read(block_t *block, unsigned எழுத்து *buf, size_t len) {
    size_t need = len;
    size_t take;
    போது (need) {
        எனில் (block->இடது == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            முழு got = block_get(block);
            எனில் (got == -1)
                /* Reached the end. */
                நிறுத்து;
            *buf++ = (unsigned எழுத்து)got;
            need--;
            தொடர்;
        }
        take = need > block->இடது ? block->இடது : need;
        memcpy(buf, block->next, take);
        block->next += take;
        block->இடது -= take;
        buf += take;
        need -= take;
    }
    திரும்பு len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local முழு block_skip(block_t *block, size_t n) {
    போது (n > block->இடது) {
        n -= block->இடது;
        block->next += block->இடது;
        block->இடது = 0;
        எனில் (block_get(block) == -1)
            திரும்பு -1;
        n--;
    }
    block->next += n;
    block->இடது -= n;
    திரும்பு 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local எழுத்து *block_central_name(block_t *block, set_t *set) {
    unsigned flen, xlen, clen;
    எழுத்து *name = வெற்று;
    க்கு (;;) {
        எனில் (block_end(block))
            /* At the end of the central directory (so far). */
            திரும்பு வெற்று;

        /* Check for a central directory record signature. */
        எனில் (block_get2(block) != (CENTRALHEADERMAGIC & 0xffff) ||
            block_get2(block) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            நிறுத்து;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        block_skip(block, 24);
        flen = (unsigned)block_get2(block);    /* file name length */
        xlen = (unsigned)block_get2(block);    /* extra length */
        clen = (unsigned)block_get2(block);    /* comment length */
        எனில் (block_skip(block, 12) == -1)
            /* Premature end of the record. */
            நிறுத்து;

        /* Extract the name and skip over the extra and comment fields. */
        name = set_alloc(set, வெற்று, flen + 1);
        எனில் (block_read(block, (unsigned எழுத்து *)name, flen) < flen ||
            block_skip(block, xlen + clen) == -1)
            /* Premature end of the record. */
            நிறுத்து;

        /* Check for embedded nuls in the name. */
        எனில் (memchr(name, 0, flen) != வெற்று) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            set_free(set, name);
            தொடர்;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        திரும்பு name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    set_free(set, name);
    block_stop(block);
    திரும்பு வெற்று;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
வெளி முழு ZEXPORT zipAlreadyThere(zipFile file, எழுத்து மாறிலி *name) {
    size_t len;
    எழுத்து *copy;
    முழு found;
    zip64_internal *zip = file;
    எனில் (zip == வெற்று)
        திரும்பு ZIP_PARAMERROR;
    எனில் (zip->central_dir.first_block == வெற்று)
        /* No central directory yet, so no, name isn't there. */
        திரும்பு 0;
    எனில் (setjmp(zip->set.env)) {
        /* Memory allocation failure. */
        set_end(&zip->set);
        திரும்பு -2;
    }
    எனில் (!set_ok(&zip->set)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        set_start(&zip->set);
        block_init(&zip->block, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    க்கு (;;) {
        எழுத்து *there = block_central_name(&zip->block, &zip->set);
        எனில் (there == வெற்று) {
            எனில் (zip->block.next == வெற்று)
                /* The central directory is invalid. */
                திரும்பு -1;
            நிறுத்து;
        }

        /* Add there to the set. */
        எனில் (set_செருகு(&zip->set, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            set_free(&zip->set, there);
    }

    /* Return true if name is in the central directory. */
    len = strlen(name);
    copy = set_alloc(&zip->set, வெற்று, len + 1);
    memcpy(copy, name, len + 1);
    found = set_found(&zip->set, copy);
    set_free(&zip->set, copy);
    திரும்பு found;
}


/****************************************************************************/

#ifndef NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local முழு zip64local_put_மதிப்பு(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, முழு nbByte) {
    unsigned எழுத்து buf[8];
    முழு n;
    க்கு (n = 0; n < nbByte; n++)
    {
        buf[n] = (unsigned எழுத்து)(x & 0xff);
        x >>= 8;
    }
    எனில் (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      க்கு (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    எனில் (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        திரும்பு ZIP_ERRNO;
    இல்லையெனில்
        திரும்பு ZIP_OK;
}

local வெறுமை zip64local_put_மதிப்பு_inmemory (வெறுமை* dest, ZPOS64_T x, முழு nbByte) {
    unsigned எழுத்து* buf=(unsigned எழுத்து*)dest;
    முழு n;
    க்கு (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned எழுத்து)(x & 0xff);
        x >>= 8;
    }

    எனில் (x != 0)
    {     /* data overflow - hack for ZIP64 */
       க்கு (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_TmzDateToDosDate(மாறிலி tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    எனில் (year>=1980)
        year-=1980;
    இல்லையெனில் எனில் (year>=80)
        year-=80;
    திரும்பு
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local முழு zip64local_getByte(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, முழு* pi) {
    unsigned எழுத்து c;
    முழு err = (முழு)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    எனில் (err==1)
    {
        *pi = (முழு)c;
        திரும்பு ZIP_OK;
    }
    இல்லையெனில்
    {
        எனில் (ZERROR64(*pzlib_filefunc_def,filestream))
            திரும்பு ZIP_ERRNO;
        இல்லையெனில்
            திரும்பு ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local முழு zip64local_getShort(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    முழு i = 0;
    முழு err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    எனில் (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    எனில் (err==ZIP_OK)
        *pX = x;
    இல்லையெனில்
        *pX = 0;
    திரும்பு err;
}

local முழு zip64local_getLong(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    முழு i = 0;
    முழு err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    எனில் (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    எனில் (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    எனில் (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    எனில் (err==ZIP_OK)
        *pX = x;
    இல்லையெனில்
        *pX = 0;
    திரும்பு err;
}


local முழு zip64local_getLong64(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  முழு i = 0;
  முழு err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  எனில் (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  எனில் (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  எனில் (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  எனில் (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  எனில் (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  எனில் (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  எனில் (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  எனில் (err==ZIP_OK)
    *pX = x;
  இல்லையெனில்
    *pX = 0;

  திரும்பு err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_தேடல்_Central_Dir(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned எழுத்து* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    திரும்பு 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  எனில் (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned எழுத்து*)ALLOC(BUFREADCOMMENT+4);
  எனில் (buf==வெற்று)
    திரும்பு 0;

  uBackRead = 4;
  போது (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    முழு i;
    எனில் (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    இல்லையெனில்
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      நிறுத்து;

    எனில் (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      நிறுத்து;

    க்கு (i=(முழு)uReadSize-3; (i--)>0;)
      எனில் (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+(unsigned)i;
        நிறுத்து;
      }

    எனில் (uPosFound!=0)
      நிறுத்து;
  }
  விடுவி(buf);
  திரும்பு uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_தேடல்_Central_Dir64(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned எழுத்து* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    திரும்பு 0;

  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  எனில் (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned எழுத்து*)ALLOC(BUFREADCOMMENT+4);
  எனில் (buf==வெற்று)
    திரும்பு 0;

  uBackRead = 4;
  போது (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    முழு i;
    எனில் (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    இல்லையெனில்
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      நிறுத்து;

    எனில் (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      நிறுத்து;

    க்கு (i=(முழு)uReadSize-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      எனில் (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+(unsigned)i;
        நிறுத்து;
      }
    }

      எனில் (uPosFound!=0)
        நிறுத்து;
  }

  விடுவி(buf);
  எனில் (uPosFound == 0)
    திரும்பு 0;

  /* Zip64 end of central directory locator */
  எனில் (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    திரும்பு 0;

  /* the signature, already checked */
  எனில் (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    திரும்பு 0;

  /* number of the disk with the start of the zip64 end of central directory */
  எனில் (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    திரும்பு 0;
  எனில் (uL != 0)
    திரும்பு 0;

  /* relative offset of the zip64 end of central directory record */
  எனில் (zip64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    திரும்பு 0;

  /* total number of disks */
  எனில் (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    திரும்பு 0;
  எனில் (uL != 1)
    திரும்பு 0;

  /* Goto Zip64 end of central directory record */
  எனில் (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    திரும்பு 0;

  /* the signature */
  எனில் (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    திரும்பு 0;

  எனில் (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    திரும்பு 0;

  திரும்பு relativeOffset;
}

local முழு LoadCentralDirectoryRecord(zip64_internal* pziinit) {
  முழு err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T size_central_dir;     /* size of the central directory  */
  ZPOS64_T offset_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong எண்_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong எண்_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T எண்_entry;
  ZPOS64_T எண்_entry_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong VersionMadeBy;
  uLong VersionNeeded;
  uLong size_comment;

  முழு hasZIP64Record = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_தேடல்_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  எனில்(central_pos > 0)
  {
    hasZIP64Record = 1;
  }
  இல்லையெனில் எனில்(central_pos == 0)
  {
    central_pos = zip64local_தேடல்_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  எனில்(hasZIP64Record)
  {
    ZPOS64_T sizeEndOfCentralDirectory;
    எனில் (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    எனில் (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    எனில் (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &sizeEndOfCentralDirectory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    எனில் (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    எனில் (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    எனில் (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&எண்_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    எனில் (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&எண்_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    எனில் (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &எண்_entry)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    எனில் (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&எண்_entry_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    எனில் ((எண்_entry_CD!=எண்_entry) || (எண்_disk_with_CD!=0) || (எண்_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    எனில் (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&size_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    எனில் (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&offset_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    size_comment = 0;
  }
  இல்லையெனில்
  {
    /* Read End of central Directory info */
    எனில் (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    எனில் (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    எனில் (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&எண்_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    எனில் (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&எண்_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    எண்_entry = 0;
    எனில் (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    இல்லையெனில்
      எண்_entry = uL;

    /* total number of entries in the central dir */
    எண்_entry_CD = 0;
    எனில் (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    இல்லையெனில்
      எண்_entry_CD = uL;

    எனில் ((எண்_entry_CD!=எண்_entry) || (எண்_disk_with_CD!=0) || (எண்_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    size_central_dir = 0;
    எனில் (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    இல்லையெனில்
      size_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    offset_central_dir = 0;
    எனில் (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    இல்லையெனில்
      offset_central_dir = uL;


    /* zipfile global comment length */
    எனில் (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &size_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  எனில் ((central_pos<offset_central_dir+size_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  எனில் (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    திரும்பு ZIP_ERRNO;
  }

  எனில் (size_comment>0)
  {
    pziinit->globalcomment = (எழுத்து*)ALLOC(size_comment+1);
    எனில் (pziinit->globalcomment)
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
    வெறுமை* buf_read = (வெறுமை*)ALLOC(buf_size);
    எனில் (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    போது ((size_central_dir_to_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      எனில் (read_this > size_central_dir_to_read)
        read_this = size_central_dir_to_read;

      எனில் (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      எனில் (err==ZIP_OK)
        err = add_data_in_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      size_central_dir_to_read-=read_this;
    }
    விடுவி(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->எண்_entry = எண்_entry_CD;

  எனில் (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET) != 0)
    err=ZIP_ERRNO;

  திரும்பு err;
}


#endif /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
வெளி zipFile ZEXPORT zipOpen3(மாறிலி வெறுமை *pathname, முழு append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_internal ziinit;
    zip64_internal* zi;
    முழு err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = வெற்று;
    ziinit.z_filefunc.ztell32_file = வெற்று;
    எனில் (pzlib_filefunc64_32_def==வெற்று)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    இல்லையெனில்
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_STATUS_CREATE) ?
                  (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
                    (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

    எனில் (ziinit.filestream == வெற்று)
        திரும்பு வெற்று;

    எனில் (append == APPEND_STATUS_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.எண்_entry = 0;
    ziinit.add_position_when_writing_offset = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.set, 0, அளவு(set_t));  /* make sure set appears dormant */



    zi = (zip64_internal*)ALLOC(அளவு(zip64_internal));
    எனில் (zi==வெற்று)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        திரும்பு வெற்று;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = வெற்று;
    எனில் (append == APPEND_STATUS_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = LoadCentralDirectoryRecord(&ziinit);
    }

    எனில் (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    எனில் (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        விடுவி(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        விடுவி(zi);
        திரும்பு வெற்று;
    }
    இல்லையெனில்
    {
        *zi = ziinit;
        திரும்பு (zipFile)zi;
    }
}

வெளி zipFile ZEXPORT zipOpen2(மாறிலி எழுத்து *pathname, முழு append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    எனில் (pzlib_filefunc32_def != வெற்று)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        திரும்பு zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    இல்லையெனில்
        திரும்பு zipOpen3(pathname, append, globalcomment, வெற்று);
}

வெளி zipFile ZEXPORT zipOpen2_64(மாறிலி வெறுமை *pathname, முழு append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    எனில் (pzlib_filefunc_def != வெற்று)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = வெற்று;
        zlib_filefunc64_32_def_fill.ztell32_file = வெற்று;
        zlib_filefunc64_32_def_fill.zseek32_file = வெற்று;
        திரும்பு zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    இல்லையெனில்
        திரும்பு zipOpen3(pathname, append, globalcomment, வெற்று);
}



வெளி zipFile ZEXPORT zipOpen(மாறிலி எழுத்து* pathname, முழு append) {
    திரும்பு zipOpen3((மாறிலி வெறுமை*)pathname,append,வெற்று,வெற்று);
}

வெளி zipFile ZEXPORT zipOpen64(மாறிலி வெறுமை* pathname, முழு append) {
    திரும்பு zipOpen3(pathname,append,வெற்று,வெற்று);
}

local முழு Write_LocalFileHeader(zip64_internal* zi, மாறிலி எழுத்து* filename, uInt size_extrafield_local, மாறிலி வெறுமை* extrafield_local) {
  /* write the local header */
  முழு err;
  uInt size_filename = (uInt)strlen(filename);
  uInt size_extrafield = size_extrafield_local;

  err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  எனில் (err==ZIP_OK)
  {
    எனில்(zi->ci.zip64)
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    இல்லையெனில்
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  எனில் (err==ZIP_OK)
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  எனில் (err==ZIP_OK)
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.method,2);

  எனில் (err==ZIP_OK)
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  எனில் (err==ZIP_OK)
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  எனில் (err==ZIP_OK)
  {
    எனில்(zi->ci.zip64)
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    இல்லையெனில்
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  எனில் (err==ZIP_OK)
  {
    எனில்(zi->ci.zip64)
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    இல்லையெனில்
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  எனில் (err==ZIP_OK)
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)size_filename,2);

  எனில்(zi->ci.zip64)
  {
    size_extrafield += 20;
  }

  எனில் (err==ZIP_OK)
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)size_extrafield,2);

  எனில் ((err==ZIP_OK) && (size_filename > 0))
  {
    எனில் (ZWRITE64(zi->z_filefunc,zi->filestream,filename,size_filename)!=size_filename)
      err = ZIP_ERRNO;
  }

  எனில் ((err==ZIP_OK) && (size_extrafield_local > 0))
  {
    எனில் (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, size_extrafield_local) != size_extrafield_local)
      err = ZIP_ERRNO;
  }


  எனில் ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      short HeaderID = 1;
      short DataSize = 16;
      ZPOS64_T CompressedSize = 0;
      ZPOS64_T UncompressedSize = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, (ZPOS64_T)HeaderID,2);
      err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, (ZPOS64_T)DataSize,2);

      err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, (ZPOS64_T)UncompressedSize,8);
      err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, (ZPOS64_T)CompressedSize,8);
  }

  திரும்பு err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local முழு utf8len(unsigned எழுத்து மாறிலி *str, size_t len) {
    திரும்பு
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
local முழு isutf8(எழுத்து மாறிலி *str, size_t len) {
    முழு utf8 = 0;
    போது (len) {
        முழு code = utf8len((unsigned எழுத்து மாறிலி *)str, len);
        எனில் (code < 0)
            திரும்பு 0;
        எனில் (code > 1)
            utf8 = 1;
        str += code;
        len -= (unsigned)code;
    }
    திரும்பு utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
வெளி முழு ZEXPORT zipOpenNewFileInZip4_64(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                           மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                           மாறிலி வெறுமை* extrafield_global, uInt size_extrafield_global,
                                           மாறிலி எழுத்து* comment, முழு method, முழு level, முழு raw,
                                           முழு windowBits,முழு memLevel, முழு strategy,
                                           மாறிலி எழுத்து* password, uLong crcForCrypting,
                                           uLong versionMadeBy, uLong flagBase, முழு zip64) {
    zip64_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    முழு err = ZIP_OK;

#    ifdef NOCRYPT
    எனில் (password != வெற்று)
        திரும்பு ZIP_PARAMERROR;
#    endif

    எனில் (file == வெற்று)
        திரும்பு ZIP_PARAMERROR;

#ifdef HAVE_BZIP2
    எனில் ((method!=0) && (method!=Z_DEFLATED) && (method!=Z_BZIP2ED))
      திரும்பு ZIP_PARAMERROR;
#இல்லையெனில்
    எனில் ((method!=0) && (method!=Z_DEFLATED))
      திரும்பு ZIP_PARAMERROR;
#endif

    /* The filename and comment length must fit in 16 bits. */
    எனில் ((filename!=வெற்று) && (strlen(filename)>0xffff))
        திரும்பு ZIP_PARAMERROR;
    எனில் ((comment!=வெற்று) && (strlen(comment)>0xffff))
        திரும்பு ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    எனில் ((size_extrafield_local>0xffff) || (size_extrafield_global>0xffff))
        திரும்பு ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    எனில் (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        எனில் (err != ZIP_OK)
            திரும்பு err;
    }

    எனில் (filename==வெற்று)
        filename="-";

    எனில் (comment==வெற்று)
        size_comment = 0;
    இல்லையெனில்
        size_comment = (uInt)strlen(comment);

    size_filename = (uInt)strlen(filename);

    எனில் (zipfi == வெற்று)
        zi->ci.dosDate = 0;
    இல்லையெனில்
    {
        எனில் (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        இல்லையெனில்
          zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
    }

    zi->ci.flag = flagBase;
    எனில் ((level==8) || (level==9))
      zi->ci.flag |= 2;
    எனில் (level==2)
      zi->ci.flag |= 4;
    எனில் (level==1)
      zi->ci.flag |= 6;
    எனில் (password != வெற்று)
      zi->ci.flag |= 1;
    எனில் (isutf8(filename, size_filename) &&
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

    zi->ci.central_header = (எழுத்து*)ALLOC((uInt)zi->ci.size_centralheader + zi->ci.size_centralExtraFree);

    zi->ci.size_centralExtra = size_extrafield_global;
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+4,(uLong)versionMadeBy,2);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+6,(uLong)20,2);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+10,(uLong)zi->ci.method,2);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+20,(uLong)0,4); /*compr size*/
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+28,(uLong)size_filename,2);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+30,(uLong)size_extrafield_global,2);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+32,(uLong)size_comment,2);
    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    எனில் (zipfi==வெற்று)
        zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+36,(uLong)0,2);
    இல்லையெனில்
        zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    எனில் (zipfi==வெற்று)
        zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+38,(uLong)0,4);
    இல்லையெனில்
        zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    எனில்(zi->ci.pos_local_header >= 0xffffffff)
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+42,(uLong)0xffffffff,4);
    இல்லையெனில்
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header - zi->add_position_when_writing_offset,4);

    க்கு (i=0;i<size_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

    க்கு (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((மாறிலி எழுத்து*)extrafield_global)+i);

    க்கு (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = *(comment+i);
    எனில் (zi->ci.central_header == வெற்று)
        திரும்பு ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.totalCompressedData = 0;
    zi->ci.totalUncompressedData = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_LocalFileHeader(zi, filename, size_extrafield_local, extrafield_local);

#ifdef HAVE_BZIP2
    zi->ci.bstream.avail_in = (uInt)0;
    zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.bstream.next_out = (எழுத்து*)zi->ci.buffered_data;
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
    zi->ci.stream.data_type = Z_இருமம்;

#ifdef HAVE_BZIP2
    எனில் ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED || zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
#இல்லையெனில்
    எனில் ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
#endif
    {
        எனில்(zi->ci.method == Z_DEFLATED)
        {
          zi->ci.stream.zalloc = (alloc_func)0;
          zi->ci.stream.zfree = (free_func)0;
          zi->ci.stream.opaque = (voidpf)0;

          எனில் (windowBits>0)
              windowBits = -windowBits;

          err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

          எனில் (err==Z_OK)
              zi->ci.stream_initialised = Z_DEFLATED;
        }
        இல்லையெனில் எனில்(zi->ci.method == Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          எனில்(err == BZ_OK)
            zi->ci.stream_initialised = Z_BZIP2ED;
#endif
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_header_size = 0;
    எனில் ((err==Z_OK) && (password != வெற்று))
    {
        unsigned எழுத்து bufHead[RAND_HEAD_LEN];
        unsigned முழு sizeHead;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_table();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        sizeHead=crypthead(password,bufHead,RAND_HEAD_LEN,zi->ci.சாவிகள்,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_header_size = sizeHead;

        எனில் (ZWRITE64(zi->z_filefunc,zi->filestream,bufHead,sizeHead) != sizeHead)
                err = ZIP_ERRNO;
    }
#    endif

    எனில் (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    திரும்பு err;
}

வெளி முழு ZEXPORT zipOpenNewFileInZip4(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                        மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                        மாறிலி வெறுமை* extrafield_global, uInt size_extrafield_global,
                                        மாறிலி எழுத்து* comment, முழு method, முழு level, முழு raw,
                                        முழு windowBits,முழு memLevel, முழு strategy,
                                        மாறிலி எழுத்து* password, uLong crcForCrypting,
                                        uLong versionMadeBy, uLong flagBase) {
    திரும்பு zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, versionMadeBy, flagBase, 0);
}

வெளி முழு ZEXPORT zipOpenNewFileInZip3(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                        மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                        மாறிலி வெறுமை* extrafield_global, uInt size_extrafield_global,
                                        மாறிலி எழுத்து* comment, முழு method, முழு level, முழு raw,
                                        முழு windowBits,முழு memLevel, முழு strategy,
                                        மாறிலி எழுத்து* password, uLong crcForCrypting) {
    திரும்பு zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

வெளி முழு ZEXPORT zipOpenNewFileInZip3_64(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                         மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                         மாறிலி வெறுமை* extrafield_global, uInt size_extrafield_global,
                                         மாறிலி எழுத்து* comment, முழு method, முழு level, முழு raw,
                                         முழு windowBits,முழு memLevel, முழு strategy,
                                         மாறிலி எழுத்து* password, uLong crcForCrypting, முழு zip64) {
    திரும்பு zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

வெளி முழு ZEXPORT zipOpenNewFileInZip2(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                        மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                        மாறிலி வெறுமை* extrafield_global, uInt size_extrafield_global,
                                        மாறிலி எழுத்து* comment, முழு method, முழு level, முழு raw) {
    திரும்பு zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   வெற்று, 0, VERSIONMADEBY, 0, 0);
}

வெளி முழு ZEXPORT zipOpenNewFileInZip2_64(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                           மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                           மாறிலி வெறுமை* extrafield_global, uInt size_extrafield_global,
                                           மாறிலி எழுத்து* comment, முழு method, முழு level, முழு raw, முழு zip64) {
    திரும்பு zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   வெற்று, 0, VERSIONMADEBY, 0, zip64);
}

வெளி முழு ZEXPORT zipOpenNewFileInZip64(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                         மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                         மாறிலி வெறுமை*extrafield_global, uInt size_extrafield_global,
                                         மாறிலி எழுத்து* comment, முழு method, முழு level, முழு zip64) {
    திரும்பு zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   வெற்று, 0, VERSIONMADEBY, 0, zip64);
}

வெளி முழு ZEXPORT zipOpenNewFileInZip(zipFile file, மாறிலி எழுத்து* filename, மாறிலி zip_fileinfo* zipfi,
                                       மாறிலி வெறுமை* extrafield_local, uInt size_extrafield_local,
                                       மாறிலி வெறுமை*extrafield_global, uInt size_extrafield_global,
                                       மாறிலி எழுத்து* comment, முழு method, முழு level) {
    திரும்பு zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   வெற்று, 0, VERSIONMADEBY, 0, 0);
}

local முழு zip64FlushWriteBuffer(zip64_internal* zi) {
    முழு err=ZIP_OK;

    எனில் (zi->ci.encrypt != 0)
    {
#ifndef NOCRYPT
        uInt i;
        முழு t;
        க்கு (i=0;i<zi->ci.pos_in_buffered_data;i++)
            zi->ci.buffered_data[i] = zencode(zi->ci.சாவிகள், zi->ci.pcrc_32_tab, zi->ci.buffered_data[i],t);
#endif
    }

    எனில் (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_data,zi->ci.pos_in_buffered_data) != zi->ci.pos_in_buffered_data)
      err = ZIP_ERRNO;

    zi->ci.totalCompressedData += zi->ci.pos_in_buffered_data;

#ifdef HAVE_BZIP2
    எனில்(zi->ci.method == Z_BZIP2ED)
    {
      zi->ci.totalUncompressedData += zi->ci.bstream.total_in_lo32;
      zi->ci.bstream.total_in_lo32 = 0;
      zi->ci.bstream.total_in_hi32 = 0;
    }
    இல்லையெனில்
#endif
    {
      zi->ci.totalUncompressedData += zi->ci.stream.total_in;
      zi->ci.stream.total_in = 0;
    }


    zi->ci.pos_in_buffered_data = 0;

    திரும்பு err;
}

வெளி முழு ZEXPORT zipWriteInFileInZip(zipFile file, மாறிலி வெறுமை* buf, unsigned முழு len) {
    zip64_internal* zi;
    முழு err=ZIP_OK;

    எனில் (file == வெற்று)
        திரும்பு ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    எனில் (zi->in_opened_file_inzip == 0)
        திரும்பு ZIP_PARAMERROR;

    zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uInt)len);

#ifdef HAVE_BZIP2
    எனில்(zi->ci.method == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.next_in = (வெறுமை*)buf;
      zi->ci.bstream.avail_in = len;
      err = BZ_RUN_OK;

      போது ((err==BZ_RUN_OK) && (zi->ci.bstream.avail_in>0))
      {
        எனில் (zi->ci.bstream.avail_out == 0)
        {
          எனில் (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (எழுத்து*)zi->ci.buffered_data;
        }


        எனில்(err != BZ_RUN_OK)
          நிறுத்து;

        எனில் ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong uTotalOutBefore_lo = zi->ci.bstream.total_out_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_RUN);

          zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore_lo) ;
        }
      }

      எனில்(err == BZ_RUN_OK)
        err = ZIP_OK;
    }
    இல்லையெனில்
#endif
    {
      zi->ci.stream.next_in = buf;
      zi->ci.stream.avail_in = len;

      போது ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
      {
          எனில் (zi->ci.stream.avail_out == 0)
          {
              எனில் (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
              zi->ci.stream.next_out = zi->ci.buffered_data;
          }


          எனில்(err != ZIP_OK)
              நிறுத்து;

          எனில் ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong uTotalOutBefore = zi->ci.stream.total_out;
              err=deflate(&zi->ci.stream,  Z_NO_FLUSH);

              zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
          }
          இல்லையெனில்
          {
              uInt copy_this,i;
              எனில் (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                  copy_this = zi->ci.stream.avail_in;
              இல்லையெனில்
                  copy_this = zi->ci.stream.avail_out;

              க்கு (i = 0; i < copy_this; i++)
                  *(((எழுத்து*)zi->ci.stream.next_out)+i) =
                      *(((மாறிலி எழுத்து*)zi->ci.stream.next_in)+i);
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

    திரும்பு err;
}

வெளி முழு ZEXPORT zipCloseFileInZipRaw(zipFile file, uLong uncompressed_size, uLong crc32) {
    திரும்பு zipCloseFileInZipRaw64 (file, uncompressed_size, crc32);
}

வெளி முழு ZEXPORT zipCloseFileInZipRaw64(zipFile file, ZPOS64_T uncompressed_size, uLong crc32) {
    zip64_internal* zi;
    ZPOS64_T compressed_size;
    uLong invalid_மதிப்பு = 0xffffffff;
    unsigned datasize = 0;
    முழு err=ZIP_OK;

    எனில் (file == வெற்று)
        திரும்பு ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    எனில் (zi->in_opened_file_inzip == 0)
        திரும்பு ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    எனில் ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
                {
                        போது (err==ZIP_OK)
                        {
                                uLong uTotalOutBefore;
                                எனில் (zi->ci.stream.avail_out == 0)
                                {
                                        எனில் (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                                        zi->ci.stream.next_out = zi->ci.buffered_data;
                                }
                                uTotalOutBefore = zi->ci.stream.total_out;
                                err=deflate(&zi->ci.stream,  Z_FINISH);
                                zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
                        }
                }
    இல்லையெனில் எனில் ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ifdef HAVE_BZIP2
      err = BZ_FINISH_OK;
      போது (err==BZ_FINISH_OK)
      {
        uLong uTotalOutBefore;
        எனில் (zi->ci.bstream.avail_out == 0)
        {
          எனில் (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (எழுத்து*)zi->ci.buffered_data;
        }
        uTotalOutBefore = zi->ci.bstream.total_out_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        எனில்(err == BZ_STREAM_END)
          err = Z_STREAM_END;

        zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore);
      }

      எனில்(err == BZ_FINISH_OK)
        err = ZIP_OK;
#endif
    }

    எனில் (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    எனில் ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
                {
        எனில் (zip64FlushWriteBuffer(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    எனில் ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
    {
        முழு tmp_err = deflateEnd(&zi->ci.stream);
        எனில் (err == ZIP_OK)
            err = tmp_err;
        zi->ci.stream_initialised = 0;
    }
#ifdef HAVE_BZIP2
    இல்லையெனில் எனில்((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
      முழு tmperr = BZ2_bzCompressEnd(&zi->ci.bstream);
                        எனில் (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.stream_initialised = 0;
    }
#endif

    எனில் (!zi->ci.raw)
    {
        crc32 = (uLong)zi->ci.crc32;
        uncompressed_size = zi->ci.totalUncompressedData;
    }
    compressed_size = zi->ci.totalCompressedData;

#    ifndef NOCRYPT
    compressed_size += zi->ci.crypt_header_size;
#    endif

    /* update Current Item crc and sizes, */
    எனில்(compressed_size >= 0xffffffff || uncompressed_size >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+6,(uLong)45,2);

    }

    zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


    எனில்(compressed_size >= 0xffffffff)
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+20, invalid_மதிப்பு,4); /*compr size*/
    இல்லையெனில்
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+20, compressed_size,4); /*compr size*/

    /* set internal file attributes field */
    எனில் (zi->ci.stream.data_type == Z_ASCII)
        zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+36,(uLong)Z_ASCII,2);

    எனில்(uncompressed_size >= 0xffffffff)
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+24, invalid_மதிப்பு,4); /*uncompr size*/
    இல்லையெனில்
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+24, uncompressed_size,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    எனில்(uncompressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    எனில்(compressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    எனில்(zi->ci.pos_local_header >= 0xffffffff)
      datasize += 8;

    எனில்(datasize > 0)
    {
      எழுத்து* p = வெற்று;

      எனில்((uLong)(datasize + 4) > zi->ci.size_centralExtraFree)
      {
        /* we cannot write more data to the buffer that we have room for. */
        திரும்பு ZIP_BADZIPFILE;
      }

      p = zi->ci.central_header + zi->ci.size_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_மதிப்பு_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_மதிப்பு_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      எனில்(uncompressed_size >= 0xffffffff)
      {
        zip64local_put_மதிப்பு_inmemory(p, uncompressed_size, 8);
        p += 8;
      }

      எனில்(compressed_size >= 0xffffffff)
      {
        zip64local_put_மதிப்பு_inmemory(p, compressed_size, 8);
        p += 8;
      }

      எனில்(zi->ci.pos_local_header >= 0xffffffff)
      {
        zip64local_put_மதிப்பு_inmemory(p, zi->ci.pos_local_header, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.size_centralExtraFree -= datasize + 4;
      zi->ci.size_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.size_centralExtra += datasize + 4;
      zip64local_put_மதிப்பு_inmemory(zi->ci.central_header+30,(uLong)zi->ci.size_centralExtra,2);
    }

    எனில் (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir, zi->ci.central_header, (uLong)zi->ci.size_centralheader);

    விடுவி(zi->ci.central_header);

    எனில் (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        எனில் (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;

        எனில் (err==ZIP_OK)
            err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

        எனில்(uncompressed_size >= 0xffffffff || compressed_size >= 0xffffffff )
        {
          எனில்(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            எனில் (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET)!=0)
              err = ZIP_ERRNO;

            எனில் (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, uncompressed_size, 8);

            எனில் (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, compressed_size, 8);
          }
          இல்லையெனில்
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        இல்லையெனில்
        {
          எனில் (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,compressed_size,4);

          எனில் (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,uncompressed_size,4);
        }

        எனில் (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;
    }

    zi->எண்_entry ++;
    zi->in_opened_file_inzip = 0;

    திரும்பு err;
}

வெளி முழு ZEXPORT zipCloseFileInZip(zipFile file) {
    திரும்பு zipCloseFileInZipRaw (file,0,0);
}

local முழு Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, ZPOS64_T zip64eocd_pos_inzip) {
  முழு err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->add_position_when_writing_offset;

  err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    எனில் (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    எனில் (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    எனில் (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    திரும்பு err;
}

local முழு Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  முழு err = ZIP_OK;

  uLong Zip64DataSize = 44;

  err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  எனில் (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64DataSize,8); /* why ZPOS64_T of this ? */

  எனில் (err==ZIP_OK) /* version made by */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  எனில் (err==ZIP_OK) /* version needed */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  எனில் (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  எனில் (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  எனில் (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, zi->எண்_entry, 8);

  எனில் (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc, zi->filestream, zi->எண்_entry, 8);

  எனில் (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(ZPOS64_T)size_centraldir,8);

  எனில் (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  திரும்பு err;
}

local முழு Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  முழு err = ZIP_OK;

  /*signature*/
  err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  எனில் (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  எனில் (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  எனில் (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      எனில்(zi->எண்_entry >= 0xFFFF)
        err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      இல்லையெனில்
        err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)zi->எண்_entry,2);
    }
  }

  எனில் (err==ZIP_OK) /* total number of entries in the central dir */
  {
    எனில்(zi->எண்_entry >= 0xFFFF)
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    இல்லையெனில்
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)zi->எண்_entry,2);
  }

  எனில் (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)size_centraldir,4);

  எனில் (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    எனில்(pos >= 0xffffffff)
    {
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    இல்லையெனில்
      err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->add_position_when_writing_offset),4);
  }

   திரும்பு err;
}

local முழு Write_GlobalComment(zip64_internal* zi, மாறிலி எழுத்து* global_comment) {
  முழு err = ZIP_OK;
  uInt size_global_comment = 0;

  எனில்(global_comment != வெற்று)
    size_global_comment = (uInt)strlen(global_comment);

  err = zip64local_put_மதிப்பு(&zi->z_filefunc,zi->filestream,(uLong)size_global_comment,2);

  எனில் (err == ZIP_OK && size_global_comment > 0)
  {
    எனில் (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, size_global_comment) != size_global_comment)
      err = ZIP_ERRNO;
  }
  திரும்பு err;
}

வெளி முழு ZEXPORT zipClose(zipFile file, மாறிலி எழுத்து* global_comment) {
    zip64_internal* zi;
    முழு err = 0;
    uLong size_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    எனில் (file == வெற்று)
        திரும்பு ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    எனில் (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

#ifndef NO_ADDFILEINEXISTINGZIP
    எனில் (global_comment==வெற்று)
        global_comment = zi->globalcomment;
#endif

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    எனில் (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
        போது (ldi!=வெற்று)
        {
            எனில் ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
            {
                எனில் (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->data, ldi->filled_in_this_block) != ldi->filled_in_this_block)
                    err = ZIP_ERRNO;
            }

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    set_end(&zi->set);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    எனில்(pos >= 0xffffffff || zi->எண்_entry >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

      Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
    }

    எனில் (err==ZIP_OK)
      err = Write_EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

    எனில்(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    எனில் (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        எனில் (err == ZIP_OK)
            err = ZIP_ERRNO;

#ifndef NO_ADDFILEINEXISTINGZIP
    விடுவி(zi->globalcomment);
#endif
    விடுவி(zi);

    திரும்பு err;
}

வெளி முழு ZEXPORT zipRemoveExtraInfoBlock(எழுத்து* pData, முழு* dataLen, short sHeader) {
  எழுத்து* p = pData;
  முழு size = 0;
  எழுத்து* pNewHeader;
  எழுத்து* pTmp;
  short header;
  short dataSize;

  முழு retVal = ZIP_OK;

  எனில்(pData == வெற்று || dataLen == வெற்று || *dataLen < 4)
    திரும்பு ZIP_PARAMERROR;

  pNewHeader = (எழுத்து*)ALLOC((unsigned)*dataLen);
  pTmp = pNewHeader;

  போது(p < (pData + *dataLen))
  {
    header = *(short*)p;
    dataSize = *(((short*)p)+1);

    எனில்( header == sHeader ) /* Header found. */
    {
      p += dataSize + 4; /* skip it. do not copy to temp buffer */
    }
    இல்லையெனில்
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      memcpy(pTmp, p, dataSize + 4);
      p += dataSize + 4;
      size += dataSize + 4;
    }

  }

  எனில்(size < *dataLen)
  {
    /* clean old extra info block. */
    memset(pData,0, *dataLen);

    /* copy the new extra info block over the old */
    எனில்(size > 0)
      memcpy(pData, pNewHeader, size);

    /* set the new extra info size */
    *dataLen = size;

    retVal = ZIP_OK;
  }
  இல்லையெனில்
    retVal = ZIP_ERRNO;

  விடுவி(pNewHeader);

  திரும்பு retVal;
}
