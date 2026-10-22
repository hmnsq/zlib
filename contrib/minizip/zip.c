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


#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>
#ifndef ZLIB_CONST
#  define ZLIB_CONST
#endif
#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#അന്തര്ഭാവ "zlib.h"
#അന്തര്ഭാവ "zip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    ബാഹ്യ പൂർണ്ണസംഖ്യ errno;
#അല്ലെങ്കിൽ
#   include <errno.h>
#endif


#ifndef local
#  define local സ്ഥിതം
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
#എങ്കിൽ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#അല്ലെങ്കിൽ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
സ്ഥിരം അക്ഷരം zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x6064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

തരനിർവചനം ഘടന linkedlist_datablock_internal_s
{
  ഘടന linkedlist_datablock_internal_s* next_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignment */
  unsigned അക്ഷരം data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

തരനിർവചനം ഘടന linkedlist_data_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_data;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#define SKIPSET_EXPORT സ്ഥിതം
തരനിർവചനം അക്ഷരം *set_താക്കോൽ_t;
#define set_cmp(a, b) strcmp(a, b)
#define set_drop(s, k) set_free(s, k)
#അന്തര്ഭാവ "skipset.h"
തരനിർവചനം ഘടന {
    unsigned അക്ഷരം *next;        /* next byte in datablock data */
    size_t ഇടത്;                /* number of bytes left in data (at least) */
    linkedlist_datablock_internal *കണ്ണി;    /* current datablock */
} block_t;


തരനിർവചനം ഘടന
{
    z_stream stream;            /* zLib stream structure for inflate */
#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    പൂർണ്ണസംഖ്യ  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_data;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_header;     /* offset of the local header of the file
                                     currently writing */
    അക്ഷരം* central_header;       /* central header data for the current file */
    uLong size_centralExtra;
    uLong size_centralheader;   /* size of the central header for cur file */
    uLong size_centralExtraFree; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    പൂർണ്ണസംഖ്യ  method;                /* compression method of file currently wr.*/
    പൂർണ്ണസംഖ്യ  raw;                   /* 1 for directly writing raw data */
    Byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
    പൂർണ്ണസംഖ്യ  encrypt;
    പൂർണ്ണസംഖ്യ  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T totalCompressedData;
    ZPOS64_T totalUncompressedData;
#ifndef NOCRYPT
    unsigned long താക്കോലുകൾ[3];     /* keys defining the pseudo-random sequence */
    സ്ഥിരം z_crc_t* pcrc_32_tab;
    unsigned crypt_header_size;
#endif
} curfile64_info;

തരനിർവചനം ഘടന
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_data central_dir;/* datablock with central dir in construction*/
    പൂർണ്ണസംഖ്യ  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T add_position_when_writing_offset;
    ZPOS64_T സംഖ്യ_entry;

#ifndef NO_ADDFILEINEXISTINGZIP
    അക്ഷരം *globalcomment;
#endif

    /* Support for zipAlreadyThere(). */
    set_t set;              /* set for detecting name collisions */
    block_t block;          /* block for reading the central directory */

} zip64_internal;


#ifndef NOCRYPT
#define INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#അന്തര്ഭാവ "crypt.h"
#endif

local linkedlist_datablock_internal* allocate_new_datablock(ശൂന്യം) {
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(വലുപ്പം(linkedlist_datablock_internal));
    എങ്കിൽ (ldi!=ശൂന്യ)
    {
        ldi->next_datablock = ശൂന്യ ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    തിരികെ ldi;
}

local ശൂന്യം free_datablock(linkedlist_datablock_internal* ldi) {
    ആകുമ്പോൾ (ldi!=ശൂന്യ)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        സ്വതന്ത്രം(ldi);
        ldi = ldinext;
    }
}

local ശൂന്യം init_linkedlist(linkedlist_data* ll) {
    ll->first_block = ll->last_block = ശൂന്യ;
}

local ശൂന്യം free_linkedlist(linkedlist_data* ll) {
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = ശൂന്യ;
}


local പൂർണ്ണസംഖ്യ add_data_in_datablock(linkedlist_data* ll, സ്ഥിരം ശൂന്യം* buf, uLong len) {
    linkedlist_datablock_internal* ldi;
    സ്ഥിരം unsigned അക്ഷരം* from_copy;

    എങ്കിൽ (ll==ശൂന്യ)
        തിരികെ ZIP_INTERNALERROR;

    എങ്കിൽ (ll->last_block == ശൂന്യ)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        എങ്കിൽ (ll->first_block == ശൂന്യ)
            തിരികെ ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (സ്ഥിരം unsigned അക്ഷരം*)buf;

    ആകുമ്പോൾ (len>0)
    {
        uInt copy_this;
        uInt i;
        unsigned അക്ഷരം* to_copy;

        എങ്കിൽ (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            എങ്കിൽ (ldi->next_datablock == ശൂന്യ)
                തിരികെ ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        എങ്കിൽ (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        അല്ലെങ്കിൽ
            copy_this = (uInt)len;

        to_copy = &(ldi->data[ldi->filled_in_this_block]);

        വേണ്ടി (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    തിരികെ ZIP_OK;
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
local ശൂന്യം block_init(block_t *block, linkedlist_data *list) {
    block->കണ്ണി = list->first_block;
    block->next = block->കണ്ണി->data;
    block->ഇടത് = block->കണ്ണി->filled_in_this_block;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local ശൂന്യം block_stop(block_t *block) {
    block->ഇടത് = 0;
    block->next = ശൂന്യ;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local പൂർണ്ണസംഖ്യ block_end(block_t *block) {
    linkedlist_datablock_internal *കണ്ണി = block->കണ്ണി;
    എങ്കിൽ (കണ്ണി == ശൂന്യ)
        /* This block was previously terminated with extreme prejudice. */
        തിരികെ 1;
    എങ്കിൽ (block->next < കണ്ണി->data + കണ്ണി->filled_in_this_block)
        /* There are more bytes to read in the current datablock. */
        തിരികെ 0;
    ആകുമ്പോൾ (കണ്ണി->next_datablock != ശൂന്യ) {
        എങ്കിൽ (കണ്ണി->filled_in_this_block != 0)
            /* There are some bytes in a later datablock. */
            തിരികെ 0;
        കണ്ണി = കണ്ണി->next_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    തിരികെ 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local പൂർണ്ണസംഖ്യ block_get(block_t *block) {
    ആകുമ്പോൾ (block->ഇടത് == 0) {
        എങ്കിൽ (block->കണ്ണി == ശൂന്യ)
            /* We've been marked bad. Return end. */
            തിരികെ -1;
        /* Update left in case more was filled in since we were last here. */
        block->ഇടത് = block->കണ്ണി->filled_in_this_block -
                      (size_t)(block->next - block->കണ്ണി->data);
        എങ്കിൽ (block->ഇടത് != 0)
            /* There was indeed more data appended in the current datablock. */
            നിർത്തുക;
        എങ്കിൽ (block->കണ്ണി->next_datablock == ശൂന്യ)
            /* No more data here, and there is no next datablock. At the end. */
            തിരികെ -1;
        /* Try the next datablock for more data. */
        block->കണ്ണി = block->കണ്ണി->next_datablock;
        block->next = block->കണ്ണി->data;
        block->ഇടത് = block->കണ്ണി->filled_in_this_block;
    }
    /* We have a byte to return. */
    block->ഇടത്--;
    തിരികെ *block->next++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local long block_get2(block_t *block) {
    പൂർണ്ണസംഖ്യ താഴെയുള്ള = block_get(block);
    പൂർണ്ണസംഖ്യ മുകളിലെ = block_get(block);
    തിരികെ താഴെയുള്ള < 0 || മുകളിലെ < 0 ? -1 : താഴെയുള്ള | ((long)മുകളിലെ << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t block_read(block_t *block, unsigned അക്ഷരം *buf, size_t len) {
    size_t need = len;
    size_t take;
    ആകുമ്പോൾ (need) {
        എങ്കിൽ (block->ഇടത് == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            പൂർണ്ണസംഖ്യ got = block_get(block);
            എങ്കിൽ (got == -1)
                /* Reached the end. */
                നിർത്തുക;
            *buf++ = (unsigned അക്ഷരം)got;
            need--;
            തുടരുക;
        }
        take = need > block->ഇടത് ? block->ഇടത് : need;
        memcpy(buf, block->next, take);
        block->next += take;
        block->ഇടത് -= take;
        buf += take;
        need -= take;
    }
    തിരികെ len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local പൂർണ്ണസംഖ്യ block_skip(block_t *block, size_t n) {
    ആകുമ്പോൾ (n > block->ഇടത്) {
        n -= block->ഇടത്;
        block->next += block->ഇടത്;
        block->ഇടത് = 0;
        എങ്കിൽ (block_get(block) == -1)
            തിരികെ -1;
        n--;
    }
    block->next += n;
    block->ഇടത് -= n;
    തിരികെ 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local അക്ഷരം *block_central_name(block_t *block, set_t *set) {
    unsigned flen, xlen, clen;
    അക്ഷരം *name = ശൂന്യ;
    വേണ്ടി (;;) {
        എങ്കിൽ (block_end(block))
            /* At the end of the central directory (so far). */
            തിരികെ ശൂന്യ;

        /* Check for a central directory record signature. */
        എങ്കിൽ (block_get2(block) != (CENTRALHEADERMAGIC & 0xffff) ||
            block_get2(block) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            നിർത്തുക;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        block_skip(block, 24);
        flen = (unsigned)block_get2(block);    /* file name length */
        xlen = (unsigned)block_get2(block);    /* extra length */
        clen = (unsigned)block_get2(block);    /* comment length */
        എങ്കിൽ (block_skip(block, 12) == -1)
            /* Premature end of the record. */
            നിർത്തുക;

        /* Extract the name and skip over the extra and comment fields. */
        name = set_alloc(set, ശൂന്യ, flen + 1);
        എങ്കിൽ (block_read(block, (unsigned അക്ഷരം *)name, flen) < flen ||
            block_skip(block, xlen + clen) == -1)
            /* Premature end of the record. */
            നിർത്തുക;

        /* Check for embedded nuls in the name. */
        എങ്കിൽ (memchr(name, 0, flen) != ശൂന്യ) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            set_free(set, name);
            തുടരുക;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        തിരികെ name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    set_free(set, name);
    block_stop(block);
    തിരികെ ശൂന്യ;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipAlreadyThere(zipFile file, അക്ഷരം സ്ഥിരം *name) {
    size_t len;
    അക്ഷരം *copy;
    പൂർണ്ണസംഖ്യ found;
    zip64_internal *zip = file;
    എങ്കിൽ (zip == ശൂന്യ)
        തിരികെ ZIP_PARAMERROR;
    എങ്കിൽ (zip->central_dir.first_block == ശൂന്യ)
        /* No central directory yet, so no, name isn't there. */
        തിരികെ 0;
    എങ്കിൽ (setjmp(zip->set.env)) {
        /* Memory allocation failure. */
        set_end(&zip->set);
        തിരികെ -2;
    }
    എങ്കിൽ (!set_ok(&zip->set)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        set_start(&zip->set);
        block_init(&zip->block, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    വേണ്ടി (;;) {
        അക്ഷരം *there = block_central_name(&zip->block, &zip->set);
        എങ്കിൽ (there == ശൂന്യ) {
            എങ്കിൽ (zip->block.next == ശൂന്യ)
                /* The central directory is invalid. */
                തിരികെ -1;
            നിർത്തുക;
        }

        /* Add there to the set. */
        എങ്കിൽ (set_തിരുകുക(&zip->set, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            set_free(&zip->set, there);
    }

    /* Return true if name is in the central directory. */
    len = strlen(name);
    copy = set_alloc(&zip->set, ശൂന്യ, len + 1);
    memcpy(copy, name, len + 1);
    found = set_found(&zip->set, copy);
    set_free(&zip->set, copy);
    തിരികെ found;
}


/****************************************************************************/

#ifndef NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local പൂർണ്ണസംഖ്യ zip64local_put_മൂല്യം(സ്ഥിരം zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, പൂർണ്ണസംഖ്യ nbByte) {
    unsigned അക്ഷരം buf[8];
    പൂർണ്ണസംഖ്യ n;
    വേണ്ടി (n = 0; n < nbByte; n++)
    {
        buf[n] = (unsigned അക്ഷരം)(x & 0xff);
        x >>= 8;
    }
    എങ്കിൽ (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      വേണ്ടി (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    എങ്കിൽ (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        തിരികെ ZIP_ERRNO;
    അല്ലെങ്കിൽ
        തിരികെ ZIP_OK;
}

local ശൂന്യം zip64local_put_മൂല്യം_inmemory (ശൂന്യം* dest, ZPOS64_T x, പൂർണ്ണസംഖ്യ nbByte) {
    unsigned അക്ഷരം* buf=(unsigned അക്ഷരം*)dest;
    പൂർണ്ണസംഖ്യ n;
    വേണ്ടി (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned അക്ഷരം)(x & 0xff);
        x >>= 8;
    }

    എങ്കിൽ (x != 0)
    {     /* data overflow - hack for ZIP64 */
       വേണ്ടി (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_TmzDateToDosDate(സ്ഥിരം tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    എങ്കിൽ (year>=1980)
        year-=1980;
    അല്ലെങ്കിൽ എങ്കിൽ (year>=80)
        year-=80;
    തിരികെ
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local പൂർണ്ണസംഖ്യ zip64local_getByte(സ്ഥിരം zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, പൂർണ്ണസംഖ്യ* pi) {
    unsigned അക്ഷരം c;
    പൂർണ്ണസംഖ്യ err = (പൂർണ്ണസംഖ്യ)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    എങ്കിൽ (err==1)
    {
        *pi = (പൂർണ്ണസംഖ്യ)c;
        തിരികെ ZIP_OK;
    }
    അല്ലെങ്കിൽ
    {
        എങ്കിൽ (ZERROR64(*pzlib_filefunc_def,filestream))
            തിരികെ ZIP_ERRNO;
        അല്ലെങ്കിൽ
            തിരികെ ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local പൂർണ്ണസംഖ്യ zip64local_getShort(സ്ഥിരം zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    പൂർണ്ണസംഖ്യ i = 0;
    പൂർണ്ണസംഖ്യ err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    എങ്കിൽ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    എങ്കിൽ (err==ZIP_OK)
        *pX = x;
    അല്ലെങ്കിൽ
        *pX = 0;
    തിരികെ err;
}

local പൂർണ്ണസംഖ്യ zip64local_getLong(സ്ഥിരം zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    പൂർണ്ണസംഖ്യ i = 0;
    പൂർണ്ണസംഖ്യ err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    എങ്കിൽ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    എങ്കിൽ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    എങ്കിൽ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    എങ്കിൽ (err==ZIP_OK)
        *pX = x;
    അല്ലെങ്കിൽ
        *pX = 0;
    തിരികെ err;
}


local പൂർണ്ണസംഖ്യ zip64local_getLong64(സ്ഥിരം zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  പൂർണ്ണസംഖ്യ i = 0;
  പൂർണ്ണസംഖ്യ err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  എങ്കിൽ (err==ZIP_OK)
    *pX = x;
  അല്ലെങ്കിൽ
    *pX = 0;

  തിരികെ err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_തിരയൽ_Central_Dir(സ്ഥിരം zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned അക്ഷരം* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  എങ്കിൽ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    തിരികെ 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  എങ്കിൽ (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned അക്ഷരം*)ALLOC(BUFREADCOMMENT+4);
  എങ്കിൽ (buf==ശൂന്യ)
    തിരികെ 0;

  uBackRead = 4;
  ആകുമ്പോൾ (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    പൂർണ്ണസംഖ്യ i;
    എങ്കിൽ (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    അല്ലെങ്കിൽ
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    എങ്കിൽ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      നിർത്തുക;

    എങ്കിൽ (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      നിർത്തുക;

    വേണ്ടി (i=(പൂർണ്ണസംഖ്യ)uReadSize-3; (i--)>0;)
      എങ്കിൽ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+(unsigned)i;
        നിർത്തുക;
      }

    എങ്കിൽ (uPosFound!=0)
      നിർത്തുക;
  }
  സ്വതന്ത്രം(buf);
  തിരികെ uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_തിരയൽ_Central_Dir64(സ്ഥിരം zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned അക്ഷരം* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  എങ്കിൽ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    തിരികെ 0;

  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  എങ്കിൽ (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned അക്ഷരം*)ALLOC(BUFREADCOMMENT+4);
  എങ്കിൽ (buf==ശൂന്യ)
    തിരികെ 0;

  uBackRead = 4;
  ആകുമ്പോൾ (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    പൂർണ്ണസംഖ്യ i;
    എങ്കിൽ (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    അല്ലെങ്കിൽ
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    എങ്കിൽ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      നിർത്തുക;

    എങ്കിൽ (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      നിർത്തുക;

    വേണ്ടി (i=(പൂർണ്ണസംഖ്യ)uReadSize-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      എങ്കിൽ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+(unsigned)i;
        നിർത്തുക;
      }
    }

      എങ്കിൽ (uPosFound!=0)
        നിർത്തുക;
  }

  സ്വതന്ത്രം(buf);
  എങ്കിൽ (uPosFound == 0)
    തിരികെ 0;

  /* Zip64 end of central directory locator */
  എങ്കിൽ (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    തിരികെ 0;

  /* the signature, already checked */
  എങ്കിൽ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    തിരികെ 0;

  /* number of the disk with the start of the zip64 end of central directory */
  എങ്കിൽ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    തിരികെ 0;
  എങ്കിൽ (uL != 0)
    തിരികെ 0;

  /* relative offset of the zip64 end of central directory record */
  എങ്കിൽ (zip64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    തിരികെ 0;

  /* total number of disks */
  എങ്കിൽ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    തിരികെ 0;
  എങ്കിൽ (uL != 1)
    തിരികെ 0;

  /* Goto Zip64 end of central directory record */
  എങ്കിൽ (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    തിരികെ 0;

  /* the signature */
  എങ്കിൽ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    തിരികെ 0;

  എങ്കിൽ (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    തിരികെ 0;

  തിരികെ relativeOffset;
}

local പൂർണ്ണസംഖ്യ LoadCentralDirectoryRecord(zip64_internal* pziinit) {
  പൂർണ്ണസംഖ്യ err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T size_central_dir;     /* size of the central directory  */
  ZPOS64_T offset_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong സംഖ്യ_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong സംഖ്യ_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T സംഖ്യ_entry;
  ZPOS64_T സംഖ്യ_entry_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong VersionMadeBy;
  uLong VersionNeeded;
  uLong size_comment;

  പൂർണ്ണസംഖ്യ hasZIP64Record = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_തിരയൽ_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  എങ്കിൽ(central_pos > 0)
  {
    hasZIP64Record = 1;
  }
  അല്ലെങ്കിൽ എങ്കിൽ(central_pos == 0)
  {
    central_pos = zip64local_തിരയൽ_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  എങ്കിൽ(hasZIP64Record)
  {
    ZPOS64_T sizeEndOfCentralDirectory;
    എങ്കിൽ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    എങ്കിൽ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    എങ്കിൽ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &sizeEndOfCentralDirectory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    എങ്കിൽ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    എങ്കിൽ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    എങ്കിൽ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&സംഖ്യ_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    എങ്കിൽ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&സംഖ്യ_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    എങ്കിൽ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &സംഖ്യ_entry)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    എങ്കിൽ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&സംഖ്യ_entry_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    എങ്കിൽ ((സംഖ്യ_entry_CD!=സംഖ്യ_entry) || (സംഖ്യ_disk_with_CD!=0) || (സംഖ്യ_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    എങ്കിൽ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&size_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    എങ്കിൽ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&offset_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    size_comment = 0;
  }
  അല്ലെങ്കിൽ
  {
    /* Read End of central Directory info */
    എങ്കിൽ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    എങ്കിൽ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    എങ്കിൽ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&സംഖ്യ_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    എങ്കിൽ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&സംഖ്യ_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    സംഖ്യ_entry = 0;
    എങ്കിൽ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    അല്ലെങ്കിൽ
      സംഖ്യ_entry = uL;

    /* total number of entries in the central dir */
    സംഖ്യ_entry_CD = 0;
    എങ്കിൽ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    അല്ലെങ്കിൽ
      സംഖ്യ_entry_CD = uL;

    എങ്കിൽ ((സംഖ്യ_entry_CD!=സംഖ്യ_entry) || (സംഖ്യ_disk_with_CD!=0) || (സംഖ്യ_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    size_central_dir = 0;
    എങ്കിൽ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    അല്ലെങ്കിൽ
      size_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    offset_central_dir = 0;
    എങ്കിൽ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    അല്ലെങ്കിൽ
      offset_central_dir = uL;


    /* zipfile global comment length */
    എങ്കിൽ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &size_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  എങ്കിൽ ((central_pos<offset_central_dir+size_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  എങ്കിൽ (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    തിരികെ ZIP_ERRNO;
  }

  എങ്കിൽ (size_comment>0)
  {
    pziinit->globalcomment = (അക്ഷരം*)ALLOC(size_comment+1);
    എങ്കിൽ (pziinit->globalcomment)
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
    ശൂന്യം* buf_read = (ശൂന്യം*)ALLOC(buf_size);
    എങ്കിൽ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    ആകുമ്പോൾ ((size_central_dir_to_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      എങ്കിൽ (read_this > size_central_dir_to_read)
        read_this = size_central_dir_to_read;

      എങ്കിൽ (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      എങ്കിൽ (err==ZIP_OK)
        err = add_data_in_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      size_central_dir_to_read-=read_this;
    }
    സ്വതന്ത്രം(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->സംഖ്യ_entry = സംഖ്യ_entry_CD;

  എങ്കിൽ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET) != 0)
    err=ZIP_ERRNO;

  തിരികെ err;
}


#endif /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
ബാഹ്യ zipFile ZEXPORT zipOpen3(സ്ഥിരം ശൂന്യം *pathname, പൂർണ്ണസംഖ്യ append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_internal ziinit;
    zip64_internal* zi;
    പൂർണ്ണസംഖ്യ err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = ശൂന്യ;
    ziinit.z_filefunc.ztell32_file = ശൂന്യ;
    എങ്കിൽ (pzlib_filefunc64_32_def==ശൂന്യ)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    അല്ലെങ്കിൽ
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_STATUS_CREATE) ?
                  (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
                    (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

    എങ്കിൽ (ziinit.filestream == ശൂന്യ)
        തിരികെ ശൂന്യ;

    എങ്കിൽ (append == APPEND_STATUS_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.സംഖ്യ_entry = 0;
    ziinit.add_position_when_writing_offset = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.set, 0, വലുപ്പം(set_t));  /* make sure set appears dormant */



    zi = (zip64_internal*)ALLOC(വലുപ്പം(zip64_internal));
    എങ്കിൽ (zi==ശൂന്യ)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        തിരികെ ശൂന്യ;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = ശൂന്യ;
    എങ്കിൽ (append == APPEND_STATUS_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = LoadCentralDirectoryRecord(&ziinit);
    }

    എങ്കിൽ (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    എങ്കിൽ (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        സ്വതന്ത്രം(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        സ്വതന്ത്രം(zi);
        തിരികെ ശൂന്യ;
    }
    അല്ലെങ്കിൽ
    {
        *zi = ziinit;
        തിരികെ (zipFile)zi;
    }
}

ബാഹ്യ zipFile ZEXPORT zipOpen2(സ്ഥിരം അക്ഷരം *pathname, പൂർണ്ണസംഖ്യ append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    എങ്കിൽ (pzlib_filefunc32_def != ശൂന്യ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        തിരികെ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    അല്ലെങ്കിൽ
        തിരികെ zipOpen3(pathname, append, globalcomment, ശൂന്യ);
}

ബാഹ്യ zipFile ZEXPORT zipOpen2_64(സ്ഥിരം ശൂന്യം *pathname, പൂർണ്ണസംഖ്യ append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    എങ്കിൽ (pzlib_filefunc_def != ശൂന്യ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ശൂന്യ;
        zlib_filefunc64_32_def_fill.ztell32_file = ശൂന്യ;
        zlib_filefunc64_32_def_fill.zseek32_file = ശൂന്യ;
        തിരികെ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    അല്ലെങ്കിൽ
        തിരികെ zipOpen3(pathname, append, globalcomment, ശൂന്യ);
}



ബാഹ്യ zipFile ZEXPORT zipOpen(സ്ഥിരം അക്ഷരം* pathname, പൂർണ്ണസംഖ്യ append) {
    തിരികെ zipOpen3((സ്ഥിരം ശൂന്യം*)pathname,append,ശൂന്യ,ശൂന്യ);
}

ബാഹ്യ zipFile ZEXPORT zipOpen64(സ്ഥിരം ശൂന്യം* pathname, പൂർണ്ണസംഖ്യ append) {
    തിരികെ zipOpen3(pathname,append,ശൂന്യ,ശൂന്യ);
}

local പൂർണ്ണസംഖ്യ Write_LocalFileHeader(zip64_internal* zi, സ്ഥിരം അക്ഷരം* filename, uInt size_extrafield_local, സ്ഥിരം ശൂന്യം* extrafield_local) {
  /* write the local header */
  പൂർണ്ണസംഖ്യ err;
  uInt size_filename = (uInt)strlen(filename);
  uInt size_extrafield = size_extrafield_local;

  err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  എങ്കിൽ (err==ZIP_OK)
  {
    എങ്കിൽ(zi->ci.zip64)
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    അല്ലെങ്കിൽ
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.method,2);

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  എങ്കിൽ (err==ZIP_OK)
  {
    എങ്കിൽ(zi->ci.zip64)
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    അല്ലെങ്കിൽ
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  എങ്കിൽ (err==ZIP_OK)
  {
    എങ്കിൽ(zi->ci.zip64)
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    അല്ലെങ്കിൽ
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)size_filename,2);

  എങ്കിൽ(zi->ci.zip64)
  {
    size_extrafield += 20;
  }

  എങ്കിൽ (err==ZIP_OK)
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)size_extrafield,2);

  എങ്കിൽ ((err==ZIP_OK) && (size_filename > 0))
  {
    എങ്കിൽ (ZWRITE64(zi->z_filefunc,zi->filestream,filename,size_filename)!=size_filename)
      err = ZIP_ERRNO;
  }

  എങ്കിൽ ((err==ZIP_OK) && (size_extrafield_local > 0))
  {
    എങ്കിൽ (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, size_extrafield_local) != size_extrafield_local)
      err = ZIP_ERRNO;
  }


  എങ്കിൽ ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      short HeaderID = 1;
      short DataSize = 16;
      ZPOS64_T CompressedSize = 0;
      ZPOS64_T UncompressedSize = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, (ZPOS64_T)HeaderID,2);
      err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, (ZPOS64_T)DataSize,2);

      err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, (ZPOS64_T)UncompressedSize,8);
      err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, (ZPOS64_T)CompressedSize,8);
  }

  തിരികെ err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local പൂർണ്ണസംഖ്യ utf8len(unsigned അക്ഷരം സ്ഥിരം *str, size_t len) {
    തിരികെ
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
local പൂർണ്ണസംഖ്യ isutf8(അക്ഷരം സ്ഥിരം *str, size_t len) {
    പൂർണ്ണസംഖ്യ utf8 = 0;
    ആകുമ്പോൾ (len) {
        പൂർണ്ണസംഖ്യ code = utf8len((unsigned അക്ഷരം സ്ഥിരം *)str, len);
        എങ്കിൽ (code < 0)
            തിരികെ 0;
        എങ്കിൽ (code > 1)
            utf8 = 1;
        str += code;
        len -= (unsigned)code;
    }
    തിരികെ utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip4_64(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                           സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                           സ്ഥിരം ശൂന്യം* extrafield_global, uInt size_extrafield_global,
                                           സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ raw,
                                           പൂർണ്ണസംഖ്യ windowBits,പൂർണ്ണസംഖ്യ memLevel, പൂർണ്ണസംഖ്യ strategy,
                                           സ്ഥിരം അക്ഷരം* password, uLong crcForCrypting,
                                           uLong versionMadeBy, uLong flagBase, പൂർണ്ണസംഖ്യ zip64) {
    zip64_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    പൂർണ്ണസംഖ്യ err = ZIP_OK;

#    ifdef NOCRYPT
    എങ്കിൽ (password != ശൂന്യ)
        തിരികെ ZIP_PARAMERROR;
#    endif

    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ ZIP_PARAMERROR;

#ifdef HAVE_BZIP2
    എങ്കിൽ ((method!=0) && (method!=Z_DEFLATED) && (method!=Z_BZIP2ED))
      തിരികെ ZIP_PARAMERROR;
#അല്ലെങ്കിൽ
    എങ്കിൽ ((method!=0) && (method!=Z_DEFLATED))
      തിരികെ ZIP_PARAMERROR;
#endif

    /* The filename and comment length must fit in 16 bits. */
    എങ്കിൽ ((filename!=ശൂന്യ) && (strlen(filename)>0xffff))
        തിരികെ ZIP_PARAMERROR;
    എങ്കിൽ ((comment!=ശൂന്യ) && (strlen(comment)>0xffff))
        തിരികെ ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    എങ്കിൽ ((size_extrafield_local>0xffff) || (size_extrafield_global>0xffff))
        തിരികെ ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    എങ്കിൽ (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        എങ്കിൽ (err != ZIP_OK)
            തിരികെ err;
    }

    എങ്കിൽ (filename==ശൂന്യ)
        filename="-";

    എങ്കിൽ (comment==ശൂന്യ)
        size_comment = 0;
    അല്ലെങ്കിൽ
        size_comment = (uInt)strlen(comment);

    size_filename = (uInt)strlen(filename);

    എങ്കിൽ (zipfi == ശൂന്യ)
        zi->ci.dosDate = 0;
    അല്ലെങ്കിൽ
    {
        എങ്കിൽ (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        അല്ലെങ്കിൽ
          zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
    }

    zi->ci.flag = flagBase;
    എങ്കിൽ ((level==8) || (level==9))
      zi->ci.flag |= 2;
    എങ്കിൽ (level==2)
      zi->ci.flag |= 4;
    എങ്കിൽ (level==1)
      zi->ci.flag |= 6;
    എങ്കിൽ (password != ശൂന്യ)
      zi->ci.flag |= 1;
    എങ്കിൽ (isutf8(filename, size_filename) &&
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

    zi->ci.central_header = (അക്ഷരം*)ALLOC((uInt)zi->ci.size_centralheader + zi->ci.size_centralExtraFree);

    zi->ci.size_centralExtra = size_extrafield_global;
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+4,(uLong)versionMadeBy,2);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+6,(uLong)20,2);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+10,(uLong)zi->ci.method,2);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+20,(uLong)0,4); /*compr size*/
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+28,(uLong)size_filename,2);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+30,(uLong)size_extrafield_global,2);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+32,(uLong)size_comment,2);
    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    എങ്കിൽ (zipfi==ശൂന്യ)
        zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+36,(uLong)0,2);
    അല്ലെങ്കിൽ
        zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    എങ്കിൽ (zipfi==ശൂന്യ)
        zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+38,(uLong)0,4);
    അല്ലെങ്കിൽ
        zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    എങ്കിൽ(zi->ci.pos_local_header >= 0xffffffff)
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+42,(uLong)0xffffffff,4);
    അല്ലെങ്കിൽ
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header - zi->add_position_when_writing_offset,4);

    വേണ്ടി (i=0;i<size_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

    വേണ്ടി (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((സ്ഥിരം അക്ഷരം*)extrafield_global)+i);

    വേണ്ടി (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = *(comment+i);
    എങ്കിൽ (zi->ci.central_header == ശൂന്യ)
        തിരികെ ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.totalCompressedData = 0;
    zi->ci.totalUncompressedData = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_LocalFileHeader(zi, filename, size_extrafield_local, extrafield_local);

#ifdef HAVE_BZIP2
    zi->ci.bstream.avail_in = (uInt)0;
    zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.bstream.next_out = (അക്ഷരം*)zi->ci.buffered_data;
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
    zi->ci.stream.data_type = Z_ദ്വിമാന;

#ifdef HAVE_BZIP2
    എങ്കിൽ ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED || zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
#അല്ലെങ്കിൽ
    എങ്കിൽ ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
#endif
    {
        എങ്കിൽ(zi->ci.method == Z_DEFLATED)
        {
          zi->ci.stream.zalloc = (alloc_func)0;
          zi->ci.stream.zfree = (free_func)0;
          zi->ci.stream.opaque = (voidpf)0;

          എങ്കിൽ (windowBits>0)
              windowBits = -windowBits;

          err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

          എങ്കിൽ (err==Z_OK)
              zi->ci.stream_initialised = Z_DEFLATED;
        }
        അല്ലെങ്കിൽ എങ്കിൽ(zi->ci.method == Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          എങ്കിൽ(err == BZ_OK)
            zi->ci.stream_initialised = Z_BZIP2ED;
#endif
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_header_size = 0;
    എങ്കിൽ ((err==Z_OK) && (password != ശൂന്യ))
    {
        unsigned അക്ഷരം bufHead[RAND_HEAD_LEN];
        unsigned പൂർണ്ണസംഖ്യ sizeHead;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_table();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        sizeHead=crypthead(password,bufHead,RAND_HEAD_LEN,zi->ci.താക്കോലുകൾ,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_header_size = sizeHead;

        എങ്കിൽ (ZWRITE64(zi->z_filefunc,zi->filestream,bufHead,sizeHead) != sizeHead)
                err = ZIP_ERRNO;
    }
#    endif

    എങ്കിൽ (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    തിരികെ err;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip4(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                        സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                        സ്ഥിരം ശൂന്യം* extrafield_global, uInt size_extrafield_global,
                                        സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ raw,
                                        പൂർണ്ണസംഖ്യ windowBits,പൂർണ്ണസംഖ്യ memLevel, പൂർണ്ണസംഖ്യ strategy,
                                        സ്ഥിരം അക്ഷരം* password, uLong crcForCrypting,
                                        uLong versionMadeBy, uLong flagBase) {
    തിരികെ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, versionMadeBy, flagBase, 0);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip3(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                        സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                        സ്ഥിരം ശൂന്യം* extrafield_global, uInt size_extrafield_global,
                                        സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ raw,
                                        പൂർണ്ണസംഖ്യ windowBits,പൂർണ്ണസംഖ്യ memLevel, പൂർണ്ണസംഖ്യ strategy,
                                        സ്ഥിരം അക്ഷരം* password, uLong crcForCrypting) {
    തിരികെ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip3_64(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                         സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                         സ്ഥിരം ശൂന്യം* extrafield_global, uInt size_extrafield_global,
                                         സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ raw,
                                         പൂർണ്ണസംഖ്യ windowBits,പൂർണ്ണസംഖ്യ memLevel, പൂർണ്ണസംഖ്യ strategy,
                                         സ്ഥിരം അക്ഷരം* password, uLong crcForCrypting, പൂർണ്ണസംഖ്യ zip64) {
    തിരികെ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip2(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                        സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                        സ്ഥിരം ശൂന്യം* extrafield_global, uInt size_extrafield_global,
                                        സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ raw) {
    തിരികെ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ശൂന്യ, 0, VERSIONMADEBY, 0, 0);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip2_64(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                           സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                           സ്ഥിരം ശൂന്യം* extrafield_global, uInt size_extrafield_global,
                                           സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ raw, പൂർണ്ണസംഖ്യ zip64) {
    തിരികെ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ശൂന്യ, 0, VERSIONMADEBY, 0, zip64);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip64(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                         സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                         സ്ഥിരം ശൂന്യം*extrafield_global, uInt size_extrafield_global,
                                         സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level, പൂർണ്ണസംഖ്യ zip64) {
    തിരികെ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ശൂന്യ, 0, VERSIONMADEBY, 0, zip64);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipOpenNewFileInZip(zipFile file, സ്ഥിരം അക്ഷരം* filename, സ്ഥിരം zip_fileinfo* zipfi,
                                       സ്ഥിരം ശൂന്യം* extrafield_local, uInt size_extrafield_local,
                                       സ്ഥിരം ശൂന്യം*extrafield_global, uInt size_extrafield_global,
                                       സ്ഥിരം അക്ഷരം* comment, പൂർണ്ണസംഖ്യ method, പൂർണ്ണസംഖ്യ level) {
    തിരികെ zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ശൂന്യ, 0, VERSIONMADEBY, 0, 0);
}

local പൂർണ്ണസംഖ്യ zip64FlushWriteBuffer(zip64_internal* zi) {
    പൂർണ്ണസംഖ്യ err=ZIP_OK;

    എങ്കിൽ (zi->ci.encrypt != 0)
    {
#ifndef NOCRYPT
        uInt i;
        പൂർണ്ണസംഖ്യ t;
        വേണ്ടി (i=0;i<zi->ci.pos_in_buffered_data;i++)
            zi->ci.buffered_data[i] = zencode(zi->ci.താക്കോലുകൾ, zi->ci.pcrc_32_tab, zi->ci.buffered_data[i],t);
#endif
    }

    എങ്കിൽ (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_data,zi->ci.pos_in_buffered_data) != zi->ci.pos_in_buffered_data)
      err = ZIP_ERRNO;

    zi->ci.totalCompressedData += zi->ci.pos_in_buffered_data;

#ifdef HAVE_BZIP2
    എങ്കിൽ(zi->ci.method == Z_BZIP2ED)
    {
      zi->ci.totalUncompressedData += zi->ci.bstream.total_in_lo32;
      zi->ci.bstream.total_in_lo32 = 0;
      zi->ci.bstream.total_in_hi32 = 0;
    }
    അല്ലെങ്കിൽ
#endif
    {
      zi->ci.totalUncompressedData += zi->ci.stream.total_in;
      zi->ci.stream.total_in = 0;
    }


    zi->ci.pos_in_buffered_data = 0;

    തിരികെ err;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipWriteInFileInZip(zipFile file, സ്ഥിരം ശൂന്യം* buf, unsigned പൂർണ്ണസംഖ്യ len) {
    zip64_internal* zi;
    പൂർണ്ണസംഖ്യ err=ZIP_OK;

    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    എങ്കിൽ (zi->in_opened_file_inzip == 0)
        തിരികെ ZIP_PARAMERROR;

    zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uInt)len);

#ifdef HAVE_BZIP2
    എങ്കിൽ(zi->ci.method == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.next_in = (ശൂന്യം*)buf;
      zi->ci.bstream.avail_in = len;
      err = BZ_RUN_OK;

      ആകുമ്പോൾ ((err==BZ_RUN_OK) && (zi->ci.bstream.avail_in>0))
      {
        എങ്കിൽ (zi->ci.bstream.avail_out == 0)
        {
          എങ്കിൽ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (അക്ഷരം*)zi->ci.buffered_data;
        }


        എങ്കിൽ(err != BZ_RUN_OK)
          നിർത്തുക;

        എങ്കിൽ ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong uTotalOutBefore_lo = zi->ci.bstream.total_out_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_RUN);

          zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore_lo) ;
        }
      }

      എങ്കിൽ(err == BZ_RUN_OK)
        err = ZIP_OK;
    }
    അല്ലെങ്കിൽ
#endif
    {
      zi->ci.stream.next_in = buf;
      zi->ci.stream.avail_in = len;

      ആകുമ്പോൾ ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
      {
          എങ്കിൽ (zi->ci.stream.avail_out == 0)
          {
              എങ്കിൽ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
              zi->ci.stream.next_out = zi->ci.buffered_data;
          }


          എങ്കിൽ(err != ZIP_OK)
              നിർത്തുക;

          എങ്കിൽ ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong uTotalOutBefore = zi->ci.stream.total_out;
              err=deflate(&zi->ci.stream,  Z_NO_FLUSH);

              zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
          }
          അല്ലെങ്കിൽ
          {
              uInt copy_this,i;
              എങ്കിൽ (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                  copy_this = zi->ci.stream.avail_in;
              അല്ലെങ്കിൽ
                  copy_this = zi->ci.stream.avail_out;

              വേണ്ടി (i = 0; i < copy_this; i++)
                  *(((അക്ഷരം*)zi->ci.stream.next_out)+i) =
                      *(((സ്ഥിരം അക്ഷരം*)zi->ci.stream.next_in)+i);
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

    തിരികെ err;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipCloseFileInZipRaw(zipFile file, uLong uncompressed_size, uLong crc32) {
    തിരികെ zipCloseFileInZipRaw64 (file, uncompressed_size, crc32);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipCloseFileInZipRaw64(zipFile file, ZPOS64_T uncompressed_size, uLong crc32) {
    zip64_internal* zi;
    ZPOS64_T compressed_size;
    uLong invalid_മൂല്യം = 0xffffffff;
    unsigned datasize = 0;
    പൂർണ്ണസംഖ്യ err=ZIP_OK;

    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    എങ്കിൽ (zi->in_opened_file_inzip == 0)
        തിരികെ ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    എങ്കിൽ ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
                {
                        ആകുമ്പോൾ (err==ZIP_OK)
                        {
                                uLong uTotalOutBefore;
                                എങ്കിൽ (zi->ci.stream.avail_out == 0)
                                {
                                        എങ്കിൽ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                                        zi->ci.stream.next_out = zi->ci.buffered_data;
                                }
                                uTotalOutBefore = zi->ci.stream.total_out;
                                err=deflate(&zi->ci.stream,  Z_FINISH);
                                zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
                        }
                }
    അല്ലെങ്കിൽ എങ്കിൽ ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ifdef HAVE_BZIP2
      err = BZ_FINISH_OK;
      ആകുമ്പോൾ (err==BZ_FINISH_OK)
      {
        uLong uTotalOutBefore;
        എങ്കിൽ (zi->ci.bstream.avail_out == 0)
        {
          എങ്കിൽ (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (അക്ഷരം*)zi->ci.buffered_data;
        }
        uTotalOutBefore = zi->ci.bstream.total_out_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        എങ്കിൽ(err == BZ_STREAM_END)
          err = Z_STREAM_END;

        zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore);
      }

      എങ്കിൽ(err == BZ_FINISH_OK)
        err = ZIP_OK;
#endif
    }

    എങ്കിൽ (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    എങ്കിൽ ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
                {
        എങ്കിൽ (zip64FlushWriteBuffer(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    എങ്കിൽ ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
    {
        പൂർണ്ണസംഖ്യ tmp_err = deflateEnd(&zi->ci.stream);
        എങ്കിൽ (err == ZIP_OK)
            err = tmp_err;
        zi->ci.stream_initialised = 0;
    }
#ifdef HAVE_BZIP2
    അല്ലെങ്കിൽ എങ്കിൽ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
      പൂർണ്ണസംഖ്യ tmperr = BZ2_bzCompressEnd(&zi->ci.bstream);
                        എങ്കിൽ (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.stream_initialised = 0;
    }
#endif

    എങ്കിൽ (!zi->ci.raw)
    {
        crc32 = (uLong)zi->ci.crc32;
        uncompressed_size = zi->ci.totalUncompressedData;
    }
    compressed_size = zi->ci.totalCompressedData;

#    ifndef NOCRYPT
    compressed_size += zi->ci.crypt_header_size;
#    endif

    /* update Current Item crc and sizes, */
    എങ്കിൽ(compressed_size >= 0xffffffff || uncompressed_size >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+6,(uLong)45,2);

    }

    zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


    എങ്കിൽ(compressed_size >= 0xffffffff)
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+20, invalid_മൂല്യം,4); /*compr size*/
    അല്ലെങ്കിൽ
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+20, compressed_size,4); /*compr size*/

    /* set internal file attributes field */
    എങ്കിൽ (zi->ci.stream.data_type == Z_ASCII)
        zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+36,(uLong)Z_ASCII,2);

    എങ്കിൽ(uncompressed_size >= 0xffffffff)
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+24, invalid_മൂല്യം,4); /*uncompr size*/
    അല്ലെങ്കിൽ
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+24, uncompressed_size,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    എങ്കിൽ(uncompressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    എങ്കിൽ(compressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    എങ്കിൽ(zi->ci.pos_local_header >= 0xffffffff)
      datasize += 8;

    എങ്കിൽ(datasize > 0)
    {
      അക്ഷരം* p = ശൂന്യ;

      എങ്കിൽ((uLong)(datasize + 4) > zi->ci.size_centralExtraFree)
      {
        /* we cannot write more data to the buffer that we have room for. */
        തിരികെ ZIP_BADZIPFILE;
      }

      p = zi->ci.central_header + zi->ci.size_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_മൂല്യം_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_മൂല്യം_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      എങ്കിൽ(uncompressed_size >= 0xffffffff)
      {
        zip64local_put_മൂല്യം_inmemory(p, uncompressed_size, 8);
        p += 8;
      }

      എങ്കിൽ(compressed_size >= 0xffffffff)
      {
        zip64local_put_മൂല്യം_inmemory(p, compressed_size, 8);
        p += 8;
      }

      എങ്കിൽ(zi->ci.pos_local_header >= 0xffffffff)
      {
        zip64local_put_മൂല്യം_inmemory(p, zi->ci.pos_local_header, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.size_centralExtraFree -= datasize + 4;
      zi->ci.size_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.size_centralExtra += datasize + 4;
      zip64local_put_മൂല്യം_inmemory(zi->ci.central_header+30,(uLong)zi->ci.size_centralExtra,2);
    }

    എങ്കിൽ (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir, zi->ci.central_header, (uLong)zi->ci.size_centralheader);

    സ്വതന്ത്രം(zi->ci.central_header);

    എങ്കിൽ (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        എങ്കിൽ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;

        എങ്കിൽ (err==ZIP_OK)
            err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

        എങ്കിൽ(uncompressed_size >= 0xffffffff || compressed_size >= 0xffffffff )
        {
          എങ്കിൽ(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            എങ്കിൽ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET)!=0)
              err = ZIP_ERRNO;

            എങ്കിൽ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, uncompressed_size, 8);

            എങ്കിൽ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, compressed_size, 8);
          }
          അല്ലെങ്കിൽ
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        അല്ലെങ്കിൽ
        {
          എങ്കിൽ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,compressed_size,4);

          എങ്കിൽ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,uncompressed_size,4);
        }

        എങ്കിൽ (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;
    }

    zi->സംഖ്യ_entry ++;
    zi->in_opened_file_inzip = 0;

    തിരികെ err;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipCloseFileInZip(zipFile file) {
    തിരികെ zipCloseFileInZipRaw (file,0,0);
}

local പൂർണ്ണസംഖ്യ Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, ZPOS64_T zip64eocd_pos_inzip) {
  പൂർണ്ണസംഖ്യ err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->add_position_when_writing_offset;

  err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    എങ്കിൽ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    എങ്കിൽ (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    എങ്കിൽ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    തിരികെ err;
}

local പൂർണ്ണസംഖ്യ Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  പൂർണ്ണസംഖ്യ err = ZIP_OK;

  uLong Zip64DataSize = 44;

  err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  എങ്കിൽ (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64DataSize,8); /* why ZPOS64_T of this ? */

  എങ്കിൽ (err==ZIP_OK) /* version made by */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  എങ്കിൽ (err==ZIP_OK) /* version needed */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  എങ്കിൽ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  എങ്കിൽ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  എങ്കിൽ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, zi->സംഖ്യ_entry, 8);

  എങ്കിൽ (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc, zi->filestream, zi->സംഖ്യ_entry, 8);

  എങ്കിൽ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(ZPOS64_T)size_centraldir,8);

  എങ്കിൽ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  തിരികെ err;
}

local പൂർണ്ണസംഖ്യ Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  പൂർണ്ണസംഖ്യ err = ZIP_OK;

  /*signature*/
  err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  എങ്കിൽ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  എങ്കിൽ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  എങ്കിൽ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      എങ്കിൽ(zi->സംഖ്യ_entry >= 0xFFFF)
        err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      അല്ലെങ്കിൽ
        err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)zi->സംഖ്യ_entry,2);
    }
  }

  എങ്കിൽ (err==ZIP_OK) /* total number of entries in the central dir */
  {
    എങ്കിൽ(zi->സംഖ്യ_entry >= 0xFFFF)
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    അല്ലെങ്കിൽ
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)zi->സംഖ്യ_entry,2);
  }

  എങ്കിൽ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)size_centraldir,4);

  എങ്കിൽ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    എങ്കിൽ(pos >= 0xffffffff)
    {
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    അല്ലെങ്കിൽ
      err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->add_position_when_writing_offset),4);
  }

   തിരികെ err;
}

local പൂർണ്ണസംഖ്യ Write_GlobalComment(zip64_internal* zi, സ്ഥിരം അക്ഷരം* global_comment) {
  പൂർണ്ണസംഖ്യ err = ZIP_OK;
  uInt size_global_comment = 0;

  എങ്കിൽ(global_comment != ശൂന്യ)
    size_global_comment = (uInt)strlen(global_comment);

  err = zip64local_put_മൂല്യം(&zi->z_filefunc,zi->filestream,(uLong)size_global_comment,2);

  എങ്കിൽ (err == ZIP_OK && size_global_comment > 0)
  {
    എങ്കിൽ (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, size_global_comment) != size_global_comment)
      err = ZIP_ERRNO;
  }
  തിരികെ err;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipClose(zipFile file, സ്ഥിരം അക്ഷരം* global_comment) {
    zip64_internal* zi;
    പൂർണ്ണസംഖ്യ err = 0;
    uLong size_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    എങ്കിൽ (file == ശൂന്യ)
        തിരികെ ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    എങ്കിൽ (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

#ifndef NO_ADDFILEINEXISTINGZIP
    എങ്കിൽ (global_comment==ശൂന്യ)
        global_comment = zi->globalcomment;
#endif

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    എങ്കിൽ (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
        ആകുമ്പോൾ (ldi!=ശൂന്യ)
        {
            എങ്കിൽ ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
            {
                എങ്കിൽ (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->data, ldi->filled_in_this_block) != ldi->filled_in_this_block)
                    err = ZIP_ERRNO;
            }

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    set_end(&zi->set);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    എങ്കിൽ(pos >= 0xffffffff || zi->സംഖ്യ_entry >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

      Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
    }

    എങ്കിൽ (err==ZIP_OK)
      err = Write_EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

    എങ്കിൽ(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    എങ്കിൽ (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        എങ്കിൽ (err == ZIP_OK)
            err = ZIP_ERRNO;

#ifndef NO_ADDFILEINEXISTINGZIP
    സ്വതന്ത്രം(zi->globalcomment);
#endif
    സ്വതന്ത്രം(zi);

    തിരികെ err;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT zipRemoveExtraInfoBlock(അക്ഷരം* pData, പൂർണ്ണസംഖ്യ* dataLen, short sHeader) {
  അക്ഷരം* p = pData;
  പൂർണ്ണസംഖ്യ size = 0;
  അക്ഷരം* pNewHeader;
  അക്ഷരം* pTmp;
  short header;
  short dataSize;

  പൂർണ്ണസംഖ്യ retVal = ZIP_OK;

  എങ്കിൽ(pData == ശൂന്യ || dataLen == ശൂന്യ || *dataLen < 4)
    തിരികെ ZIP_PARAMERROR;

  pNewHeader = (അക്ഷരം*)ALLOC((unsigned)*dataLen);
  pTmp = pNewHeader;

  ആകുമ്പോൾ(p < (pData + *dataLen))
  {
    header = *(short*)p;
    dataSize = *(((short*)p)+1);

    എങ്കിൽ( header == sHeader ) /* Header found. */
    {
      p += dataSize + 4; /* skip it. do not copy to temp buffer */
    }
    അല്ലെങ്കിൽ
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      memcpy(pTmp, p, dataSize + 4);
      p += dataSize + 4;
      size += dataSize + 4;
    }

  }

  എങ്കിൽ(size < *dataLen)
  {
    /* clean old extra info block. */
    memset(pData,0, *dataLen);

    /* copy the new extra info block over the old */
    എങ്കിൽ(size > 0)
      memcpy(pData, pNewHeader, size);

    /* set the new extra info size */
    *dataLen = size;

    retVal = ZIP_OK;
  }
  അല്ലെങ്കിൽ
    retVal = ZIP_ERRNO;

  സ്വതന്ത്രം(pNewHeader);

  തിരികെ retVal;
}
