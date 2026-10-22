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


#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>
#ifndef ZLIB_CONST
#  define ZLIB_CONST
#endif
#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#अन्तर्भाव "zlib.h"
#अन्तर्भाव "zip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    बाह्य पूर्णाङ्क त्रुटि_सं;
#अन्यथा
#   include <errno.h>
#endif


#ifndef local
#  define local स्थिर
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
# define ALLOC(size) (दो_स्मृति(size))
#endif

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#define MAKEULONG64(a, b) ((ZPOS64_T)(((अचिन्हित दीर्घ)(a)) | ((ZPOS64_T)((अचिन्हित दीर्घ)(b))) << 32))

#ifndef प्रस्तुत_से
#define प्रस्तुत_से    1
#endif

#ifndef अंत_से
#define अंत_से    2
#endif

#ifndef शुरू_से
#define शुरू_से    0
#endif

#ifndef DEF_MEM_LEVEL
#यदि MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
ध्रुव वर्ण zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x6064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

प्रकारनिर्देश संरचना linkedlist_datablock_internal_s
{
  संरचना linkedlist_datablock_internal_s* next_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignment */
  अचिन्हित वर्ण data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

प्रकारनिर्देश संरचना linkedlist_data_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_data;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#define SKIPSET_EXPORT स्थिर
प्रकारनिर्देश वर्ण *set_कुंजी_t;
#define set_cmp(a, b) म_भेद(a, b)
#define set_drop(s, k) set_free(s, k)
#अन्तर्भाव "skipset.h"
प्रकारनिर्देश संरचना {
    अचिन्हित वर्ण *next;        /* next byte in datablock data */
    माप_प्रकार बायाँ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_internal *गाँठ;    /* current datablock */
} block_t;


प्रकारनिर्देश संरचना
{
    z_stream stream;            /* zLib stream structure for inflate */
#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    पूर्णाङ्क  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_data;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_header;     /* offset of the local header of the file
                                     currently writing */
    वर्ण* central_header;       /* central header data for the current file */
    uLong size_centralExtra;
    uLong size_centralheader;   /* size of the central header for cur file */
    uLong size_centralExtraFree; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    पूर्णाङ्क  method;                /* compression method of file currently wr.*/
    पूर्णाङ्क  raw;                   /* 1 for directly writing raw data */
    Byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
    पूर्णाङ्क  encrypt;
    पूर्णाङ्क  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T totalCompressedData;
    ZPOS64_T totalUncompressedData;
#ifndef NOCRYPT
    अचिन्हित दीर्घ कुंजियाँ[3];     /* keys defining the pseudo-random sequence */
    ध्रुव z_crc_t* pcrc_32_tab;
    अचिन्हित crypt_header_size;
#endif
} curfile64_info;

प्रकारनिर्देश संरचना
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_data central_dir;/* datablock with central dir in construction*/
    पूर्णाङ्क  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T add_position_when_writing_offset;
    ZPOS64_T संख्या_entry;

#ifndef NO_ADDFILEINEXISTINGZIP
    वर्ण *globalcomment;
#endif

    /* Support for zipAlreadyThere(). */
    set_t set;              /* set for detecting name collisions */
    block_t block;          /* block for reading the central directory */

} zip64_internal;


#ifndef NOCRYPT
#define INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#अन्तर्भाव "crypt.h"
#endif

local linkedlist_datablock_internal* allocate_new_datablock(शून्य) {
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(परिमाण(linkedlist_datablock_internal));
    यदि (ldi!=रिक्त)
    {
        ldi->next_datablock = रिक्त ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    प्रत्यागम ldi;
}

local शून्य free_datablock(linkedlist_datablock_internal* ldi) {
    यावत् (ldi!=रिक्त)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        मुक्त(ldi);
        ldi = ldinext;
    }
}

local शून्य init_linkedlist(linkedlist_data* ll) {
    ll->first_block = ll->last_block = रिक्त;
}

local शून्य free_linkedlist(linkedlist_data* ll) {
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = रिक्त;
}


local पूर्णाङ्क add_data_in_datablock(linkedlist_data* ll, ध्रुव शून्य* buf, uLong len) {
    linkedlist_datablock_internal* ldi;
    ध्रुव अचिन्हित वर्ण* from_copy;

    यदि (ll==रिक्त)
        प्रत्यागम ZIP_INTERNALERROR;

    यदि (ll->last_block == रिक्त)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        यदि (ll->first_block == रिक्त)
            प्रत्यागम ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (ध्रुव अचिन्हित वर्ण*)buf;

    यावत् (len>0)
    {
        uInt copy_this;
        uInt i;
        अचिन्हित वर्ण* to_copy;

        यदि (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            यदि (ldi->next_datablock == रिक्त)
                प्रत्यागम ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        यदि (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        अन्यथा
            copy_this = (uInt)len;

        to_copy = &(ldi->data[ldi->filled_in_this_block]);

        प्रति (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    प्रत्यागम ZIP_OK;
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
local शून्य block_init(block_t *block, linkedlist_data *list) {
    block->गाँठ = list->first_block;
    block->next = block->गाँठ->data;
    block->बायाँ = block->गाँठ->filled_in_this_block;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local शून्य block_stop(block_t *block) {
    block->बायाँ = 0;
    block->next = रिक्त;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local पूर्णाङ्क block_end(block_t *block) {
    linkedlist_datablock_internal *गाँठ = block->गाँठ;
    यदि (गाँठ == रिक्त)
        /* This block was previously terminated with extreme prejudice. */
        प्रत्यागम 1;
    यदि (block->next < गाँठ->data + गाँठ->filled_in_this_block)
        /* There are more bytes to read in the current datablock. */
        प्रत्यागम 0;
    यावत् (गाँठ->next_datablock != रिक्त) {
        यदि (गाँठ->filled_in_this_block != 0)
            /* There are some bytes in a later datablock. */
            प्रत्यागम 0;
        गाँठ = गाँठ->next_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    प्रत्यागम 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local पूर्णाङ्क block_get(block_t *block) {
    यावत् (block->बायाँ == 0) {
        यदि (block->गाँठ == रिक्त)
            /* We've been marked bad. Return end. */
            प्रत्यागम -1;
        /* Update left in case more was filled in since we were last here. */
        block->बायाँ = block->गाँठ->filled_in_this_block -
                      (माप_प्रकार)(block->next - block->गाँठ->data);
        यदि (block->बायाँ != 0)
            /* There was indeed more data appended in the current datablock. */
            भङ्ग;
        यदि (block->गाँठ->next_datablock == रिक्त)
            /* No more data here, and there is no next datablock. At the end. */
            प्रत्यागम -1;
        /* Try the next datablock for more data. */
        block->गाँठ = block->गाँठ->next_datablock;
        block->next = block->गाँठ->data;
        block->बायाँ = block->गाँठ->filled_in_this_block;
    }
    /* We have a byte to return. */
    block->बायाँ--;
    प्रत्यागम *block->next++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local दीर्घ block_get2(block_t *block) {
    पूर्णाङ्क निचला = block_get(block);
    पूर्णाङ्क ऊपरी = block_get(block);
    प्रत्यागम निचला < 0 || ऊपरी < 0 ? -1 : निचला | ((दीर्घ)ऊपरी << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local माप_प्रकार block_read(block_t *block, अचिन्हित वर्ण *buf, माप_प्रकार len) {
    माप_प्रकार need = len;
    माप_प्रकार take;
    यावत् (need) {
        यदि (block->बायाँ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            पूर्णाङ्क got = block_get(block);
            यदि (got == -1)
                /* Reached the end. */
                भङ्ग;
            *buf++ = (अचिन्हित वर्ण)got;
            need--;
            अनुवर्त;
        }
        take = need > block->बायाँ ? block->बायाँ : need;
        स_नकल(buf, block->next, take);
        block->next += take;
        block->बायाँ -= take;
        buf += take;
        need -= take;
    }
    प्रत्यागम len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local पूर्णाङ्क block_skip(block_t *block, माप_प्रकार n) {
    यावत् (n > block->बायाँ) {
        n -= block->बायाँ;
        block->next += block->बायाँ;
        block->बायाँ = 0;
        यदि (block_get(block) == -1)
            प्रत्यागम -1;
        n--;
    }
    block->next += n;
    block->बायाँ -= n;
    प्रत्यागम 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local वर्ण *block_central_name(block_t *block, set_t *set) {
    अचिन्हित flen, xlen, clen;
    वर्ण *name = रिक्त;
    प्रति (;;) {
        यदि (block_end(block))
            /* At the end of the central directory (so far). */
            प्रत्यागम रिक्त;

        /* Check for a central directory record signature. */
        यदि (block_get2(block) != (CENTRALHEADERMAGIC & 0xffff) ||
            block_get2(block) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            भङ्ग;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        block_skip(block, 24);
        flen = (अचिन्हित)block_get2(block);    /* file name length */
        xlen = (अचिन्हित)block_get2(block);    /* extra length */
        clen = (अचिन्हित)block_get2(block);    /* comment length */
        यदि (block_skip(block, 12) == -1)
            /* Premature end of the record. */
            भङ्ग;

        /* Extract the name and skip over the extra and comment fields. */
        name = set_alloc(set, रिक्त, flen + 1);
        यदि (block_read(block, (अचिन्हित वर्ण *)name, flen) < flen ||
            block_skip(block, xlen + clen) == -1)
            /* Premature end of the record. */
            भङ्ग;

        /* Check for embedded nuls in the name. */
        यदि (स_प्रथम(name, 0, flen) != रिक्त) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            set_free(set, name);
            अनुवर्त;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        प्रत्यागम name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    set_free(set, name);
    block_stop(block);
    प्रत्यागम रिक्त;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
बाह्य पूर्णाङ्क ZEXPORT zipAlreadyThere(zipFile file, वर्ण ध्रुव *name) {
    माप_प्रकार len;
    वर्ण *copy;
    पूर्णाङ्क found;
    zip64_internal *zip = file;
    यदि (zip == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
    यदि (zip->central_dir.first_block == रिक्त)
        /* No central directory yet, so no, name isn't there. */
        प्रत्यागम 0;
    यदि (बनाओ_लाँघ(zip->set.env)) {
        /* Memory allocation failure. */
        set_end(&zip->set);
        प्रत्यागम -2;
    }
    यदि (!set_ok(&zip->set)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        set_start(&zip->set);
        block_init(&zip->block, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    प्रति (;;) {
        वर्ण *there = block_central_name(&zip->block, &zip->set);
        यदि (there == रिक्त) {
            यदि (zip->block.next == रिक्त)
                /* The central directory is invalid. */
                प्रत्यागम -1;
            भङ्ग;
        }

        /* Add there to the set. */
        यदि (set_जोड़ो(&zip->set, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            set_free(&zip->set, there);
    }

    /* Return true if name is in the central directory. */
    len = म_माप(name);
    copy = set_alloc(&zip->set, रिक्त, len + 1);
    स_नकल(copy, name, len + 1);
    found = set_found(&zip->set, copy);
    set_free(&zip->set, copy);
    प्रत्यागम found;
}


/****************************************************************************/

#ifndef NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local पूर्णाङ्क zip64local_put_मूल्य(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, पूर्णाङ्क nbByte) {
    अचिन्हित वर्ण buf[8];
    पूर्णाङ्क n;
    प्रति (n = 0; n < nbByte; n++)
    {
        buf[n] = (अचिन्हित वर्ण)(x & 0xff);
        x >>= 8;
    }
    यदि (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      प्रति (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    यदि (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        प्रत्यागम ZIP_ERRNO;
    अन्यथा
        प्रत्यागम ZIP_OK;
}

local शून्य zip64local_put_मूल्य_inmemory (शून्य* dest, ZPOS64_T x, पूर्णाङ्क nbByte) {
    अचिन्हित वर्ण* buf=(अचिन्हित वर्ण*)dest;
    पूर्णाङ्क n;
    प्रति (n = 0; n < nbByte; n++) {
        buf[n] = (अचिन्हित वर्ण)(x & 0xff);
        x >>= 8;
    }

    यदि (x != 0)
    {     /* data overflow - hack for ZIP64 */
       प्रति (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_TmzDateToDosDate(ध्रुव tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    यदि (year>=1980)
        year-=1980;
    अन्यथा यदि (year>=80)
        year-=80;
    प्रत्यागम
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local पूर्णाङ्क zip64local_getByte(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, पूर्णाङ्क* pi) {
    अचिन्हित वर्ण c;
    पूर्णाङ्क err = (पूर्णाङ्क)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    यदि (err==1)
    {
        *pi = (पूर्णाङ्क)c;
        प्रत्यागम ZIP_OK;
    }
    अन्यथा
    {
        यदि (ZERROR64(*pzlib_filefunc_def,filestream))
            प्रत्यागम ZIP_ERRNO;
        अन्यथा
            प्रत्यागम ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local पूर्णाङ्क zip64local_getShort(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    पूर्णाङ्क i = 0;
    पूर्णाङ्क err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    यदि (err==ZIP_OK)
        *pX = x;
    अन्यथा
        *pX = 0;
    प्रत्यागम err;
}

local पूर्णाङ्क zip64local_getLong(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    पूर्णाङ्क i = 0;
    पूर्णाङ्क err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    यदि (err==ZIP_OK)
        *pX = x;
    अन्यथा
        *pX = 0;
    प्रत्यागम err;
}


local पूर्णाङ्क zip64local_getLong64(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  पूर्णाङ्क i = 0;
  पूर्णाङ्क err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  यदि (err==ZIP_OK)
    *pX = x;
  अन्यथा
    *pX = 0;

  प्रत्यागम err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_खोज_Central_Dir(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  अचिन्हित वर्ण* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  यदि (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    प्रत्यागम 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  यदि (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (अचिन्हित वर्ण*)ALLOC(BUFREADCOMMENT+4);
  यदि (buf==रिक्त)
    प्रत्यागम 0;

  uBackRead = 4;
  यावत् (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    पूर्णाङ्क i;
    यदि (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    अन्यथा
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    यदि (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      भङ्ग;

    यदि (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      भङ्ग;

    प्रति (i=(पूर्णाङ्क)uReadSize-3; (i--)>0;)
      यदि (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+(अचिन्हित)i;
        भङ्ग;
      }

    यदि (uPosFound!=0)
      भङ्ग;
  }
  मुक्त(buf);
  प्रत्यागम uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_खोज_Central_Dir64(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  अचिन्हित वर्ण* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  यदि (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    प्रत्यागम 0;

  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  यदि (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (अचिन्हित वर्ण*)ALLOC(BUFREADCOMMENT+4);
  यदि (buf==रिक्त)
    प्रत्यागम 0;

  uBackRead = 4;
  यावत् (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    पूर्णाङ्क i;
    यदि (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    अन्यथा
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    यदि (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      भङ्ग;

    यदि (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      भङ्ग;

    प्रति (i=(पूर्णाङ्क)uReadSize-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      यदि (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+(अचिन्हित)i;
        भङ्ग;
      }
    }

      यदि (uPosFound!=0)
        भङ्ग;
  }

  मुक्त(buf);
  यदि (uPosFound == 0)
    प्रत्यागम 0;

  /* Zip64 end of central directory locator */
  यदि (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    प्रत्यागम 0;

  /* the signature, already checked */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;

  /* number of the disk with the start of the zip64 end of central directory */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;
  यदि (uL != 0)
    प्रत्यागम 0;

  /* relative offset of the zip64 end of central directory record */
  यदि (zip64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    प्रत्यागम 0;

  /* total number of disks */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;
  यदि (uL != 1)
    प्रत्यागम 0;

  /* Goto Zip64 end of central directory record */
  यदि (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    प्रत्यागम 0;

  /* the signature */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;

  यदि (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    प्रत्यागम 0;

  प्रत्यागम relativeOffset;
}

local पूर्णाङ्क LoadCentralDirectoryRecord(zip64_internal* pziinit) {
  पूर्णाङ्क err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T size_central_dir;     /* size of the central directory  */
  ZPOS64_T offset_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong संख्या_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong संख्या_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T संख्या_entry;
  ZPOS64_T संख्या_entry_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong VersionMadeBy;
  uLong VersionNeeded;
  uLong size_comment;

  पूर्णाङ्क hasZIP64Record = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_खोज_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  यदि(central_pos > 0)
  {
    hasZIP64Record = 1;
  }
  अन्यथा यदि(central_pos == 0)
  {
    central_pos = zip64local_खोज_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  यदि(hasZIP64Record)
  {
    ZPOS64_T sizeEndOfCentralDirectory;
    यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &sizeEndOfCentralDirectory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &संख्या_entry)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&संख्या_entry_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    यदि ((संख्या_entry_CD!=संख्या_entry) || (संख्या_disk_with_CD!=0) || (संख्या_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&size_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&offset_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    size_comment = 0;
  }
  अन्यथा
  {
    /* Read End of central Directory info */
    यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    संख्या_entry = 0;
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      संख्या_entry = uL;

    /* total number of entries in the central dir */
    संख्या_entry_CD = 0;
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      संख्या_entry_CD = uL;

    यदि ((संख्या_entry_CD!=संख्या_entry) || (संख्या_disk_with_CD!=0) || (संख्या_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    size_central_dir = 0;
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      size_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    offset_central_dir = 0;
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      offset_central_dir = uL;


    /* zipfile global comment length */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &size_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  यदि ((central_pos<offset_central_dir+size_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  यदि (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    प्रत्यागम ZIP_ERRNO;
  }

  यदि (size_comment>0)
  {
    pziinit->globalcomment = (वर्ण*)ALLOC(size_comment+1);
    यदि (pziinit->globalcomment)
    {
      size_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,size_comment);
      pziinit->globalcomment[size_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (offset_central_dir+size_central_dir);
  pziinit->add_position_when_writing_offset = byte_before_the_zipfile;

  {
    ZPOS64_T size_central_dir_to_read = size_central_dir;
    माप_प्रकार buf_size = SIZEDATA_INDATABLOCK;
    शून्य* buf_read = (शून्य*)ALLOC(buf_size);
    यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    यावत् ((size_central_dir_to_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      यदि (read_this > size_central_dir_to_read)
        read_this = size_central_dir_to_read;

      यदि (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      यदि (err==ZIP_OK)
        err = add_data_in_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      size_central_dir_to_read-=read_this;
    }
    मुक्त(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->संख्या_entry = संख्या_entry_CD;

  यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET) != 0)
    err=ZIP_ERRNO;

  प्रत्यागम err;
}


#endif /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
बाह्य zipFile ZEXPORT zipOpen3(ध्रुव शून्य *pathname, पूर्णाङ्क append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_internal ziinit;
    zip64_internal* zi;
    पूर्णाङ्क err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = रिक्त;
    ziinit.z_filefunc.ztell32_file = रिक्त;
    यदि (pzlib_filefunc64_32_def==रिक्त)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    अन्यथा
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_STATUS_CREATE) ?
                  (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
                    (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

    यदि (ziinit.filestream == रिक्त)
        प्रत्यागम रिक्त;

    यदि (append == APPEND_STATUS_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,अंत_से);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.संख्या_entry = 0;
    ziinit.add_position_when_writing_offset = 0;
    init_linkedlist(&(ziinit.central_dir));
    स_रखो(&ziinit.set, 0, परिमाण(set_t));  /* make sure set appears dormant */



    zi = (zip64_internal*)ALLOC(परिमाण(zip64_internal));
    यदि (zi==रिक्त)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        प्रत्यागम रिक्त;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = रिक्त;
    यदि (append == APPEND_STATUS_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = LoadCentralDirectoryRecord(&ziinit);
    }

    यदि (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    यदि (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        मुक्त(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        मुक्त(zi);
        प्रत्यागम रिक्त;
    }
    अन्यथा
    {
        *zi = ziinit;
        प्रत्यागम (zipFile)zi;
    }
}

बाह्य zipFile ZEXPORT zipOpen2(ध्रुव वर्ण *pathname, पूर्णाङ्क append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    यदि (pzlib_filefunc32_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        प्रत्यागम zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    अन्यथा
        प्रत्यागम zipOpen3(pathname, append, globalcomment, रिक्त);
}

बाह्य zipFile ZEXPORT zipOpen2_64(ध्रुव शून्य *pathname, पूर्णाङ्क append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    यदि (pzlib_filefunc_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = रिक्त;
        zlib_filefunc64_32_def_fill.ztell32_file = रिक्त;
        zlib_filefunc64_32_def_fill.zseek32_file = रिक्त;
        प्रत्यागम zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    अन्यथा
        प्रत्यागम zipOpen3(pathname, append, globalcomment, रिक्त);
}



बाह्य zipFile ZEXPORT zipOpen(ध्रुव वर्ण* pathname, पूर्णाङ्क append) {
    प्रत्यागम zipOpen3((ध्रुव शून्य*)pathname,append,रिक्त,रिक्त);
}

बाह्य zipFile ZEXPORT zipOpen64(ध्रुव शून्य* pathname, पूर्णाङ्क append) {
    प्रत्यागम zipOpen3(pathname,append,रिक्त,रिक्त);
}

local पूर्णाङ्क Write_LocalFileHeader(zip64_internal* zi, ध्रुव वर्ण* filename, uInt size_extrafield_local, ध्रुव शून्य* extrafield_local) {
  /* write the local header */
  पूर्णाङ्क err;
  uInt size_filename = (uInt)म_माप(filename);
  uInt size_extrafield = size_extrafield_local;

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  यदि (err==ZIP_OK)
  {
    यदि(zi->ci.zip64)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  यदि (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  यदि (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.method,2);

  यदि (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  यदि (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  यदि (err==ZIP_OK)
  {
    यदि(zi->ci.zip64)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  यदि (err==ZIP_OK)
  {
    यदि(zi->ci.zip64)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  यदि (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)size_filename,2);

  यदि(zi->ci.zip64)
  {
    size_extrafield += 20;
  }

  यदि (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)size_extrafield,2);

  यदि ((err==ZIP_OK) && (size_filename > 0))
  {
    यदि (ZWRITE64(zi->z_filefunc,zi->filestream,filename,size_filename)!=size_filename)
      err = ZIP_ERRNO;
  }

  यदि ((err==ZIP_OK) && (size_extrafield_local > 0))
  {
    यदि (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, size_extrafield_local) != size_extrafield_local)
      err = ZIP_ERRNO;
  }


  यदि ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      लघु HeaderID = 1;
      लघु DataSize = 16;
      ZPOS64_T CompressedSize = 0;
      ZPOS64_T UncompressedSize = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)HeaderID,2);
      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)DataSize,2);

      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)UncompressedSize,8);
      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)CompressedSize,8);
  }

  प्रत्यागम err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local पूर्णाङ्क utf8len(अचिन्हित वर्ण ध्रुव *str, माप_प्रकार len) {
    प्रत्यागम
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
local पूर्णाङ्क isutf8(वर्ण ध्रुव *str, माप_प्रकार len) {
    पूर्णाङ्क utf8 = 0;
    यावत् (len) {
        पूर्णाङ्क code = utf8len((अचिन्हित वर्ण ध्रुव *)str, len);
        यदि (code < 0)
            प्रत्यागम 0;
        यदि (code > 1)
            utf8 = 1;
        str += code;
        len -= (अचिन्हित)code;
    }
    प्रत्यागम utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip4_64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                           ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                           ध्रुव शून्य* extrafield_global, uInt size_extrafield_global,
                                           ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level, पूर्णाङ्क raw,
                                           पूर्णाङ्क windowBits,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                           ध्रुव वर्ण* password, uLong crcForCrypting,
                                           uLong versionMadeBy, uLong flagBase, पूर्णाङ्क zip64) {
    zip64_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    पूर्णाङ्क err = ZIP_OK;

#    ifdef NOCRYPT
    यदि (password != रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
#    endif

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;

#ifdef HAVE_BZIP2
    यदि ((method!=0) && (method!=Z_DEFLATED) && (method!=Z_BZIP2ED))
      प्रत्यागम ZIP_PARAMERROR;
#अन्यथा
    यदि ((method!=0) && (method!=Z_DEFLATED))
      प्रत्यागम ZIP_PARAMERROR;
#endif

    /* The filename and comment length must fit in 16 bits. */
    यदि ((filename!=रिक्त) && (म_माप(filename)>0xffff))
        प्रत्यागम ZIP_PARAMERROR;
    यदि ((comment!=रिक्त) && (म_माप(comment)>0xffff))
        प्रत्यागम ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    यदि ((size_extrafield_local>0xffff) || (size_extrafield_global>0xffff))
        प्रत्यागम ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    यदि (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        यदि (err != ZIP_OK)
            प्रत्यागम err;
    }

    यदि (filename==रिक्त)
        filename="-";

    यदि (comment==रिक्त)
        size_comment = 0;
    अन्यथा
        size_comment = (uInt)म_माप(comment);

    size_filename = (uInt)म_माप(filename);

    यदि (zipfi == रिक्त)
        zi->ci.dosDate = 0;
    अन्यथा
    {
        यदि (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        अन्यथा
          zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
    }

    zi->ci.flag = flagBase;
    यदि ((level==8) || (level==9))
      zi->ci.flag |= 2;
    यदि (level==2)
      zi->ci.flag |= 4;
    यदि (level==1)
      zi->ci.flag |= 6;
    यदि (password != रिक्त)
      zi->ci.flag |= 1;
    यदि (isutf8(filename, size_filename) &&
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

    zi->ci.central_header = (वर्ण*)ALLOC((uInt)zi->ci.size_centralheader + zi->ci.size_centralExtraFree);

    zi->ci.size_centralExtra = size_extrafield_global;
    zip64local_put_मूल्य_inmemory(zi->ci.central_header,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+4,(uLong)versionMadeBy,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+6,(uLong)20,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+10,(uLong)zi->ci.method,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+20,(uLong)0,4); /*compr size*/
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+28,(uLong)size_filename,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+30,(uLong)size_extrafield_global,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+32,(uLong)size_comment,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    यदि (zipfi==रिक्त)
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+36,(uLong)0,2);
    अन्यथा
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    यदि (zipfi==रिक्त)
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+38,(uLong)0,4);
    अन्यथा
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    यदि(zi->ci.pos_local_header >= 0xffffffff)
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+42,(uLong)0xffffffff,4);
    अन्यथा
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header - zi->add_position_when_writing_offset,4);

    प्रति (i=0;i<size_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

    प्रति (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((ध्रुव वर्ण*)extrafield_global)+i);

    प्रति (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = *(comment+i);
    यदि (zi->ci.central_header == रिक्त)
        प्रत्यागम ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.totalCompressedData = 0;
    zi->ci.totalUncompressedData = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_LocalFileHeader(zi, filename, size_extrafield_local, extrafield_local);

#ifdef HAVE_BZIP2
    zi->ci.bstream.avail_in = (uInt)0;
    zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.bstream.next_out = (वर्ण*)zi->ci.buffered_data;
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
    zi->ci.stream.data_type = Z_द्विआधारी;

#ifdef HAVE_BZIP2
    यदि ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED || zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
#अन्यथा
    यदि ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
#endif
    {
        यदि(zi->ci.method == Z_DEFLATED)
        {
          zi->ci.stream.zalloc = (alloc_func)0;
          zi->ci.stream.zfree = (free_func)0;
          zi->ci.stream.opaque = (voidpf)0;

          यदि (windowBits>0)
              windowBits = -windowBits;

          err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

          यदि (err==Z_OK)
              zi->ci.stream_initialised = Z_DEFLATED;
        }
        अन्यथा यदि(zi->ci.method == Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          यदि(err == BZ_OK)
            zi->ci.stream_initialised = Z_BZIP2ED;
#endif
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_header_size = 0;
    यदि ((err==Z_OK) && (password != रिक्त))
    {
        अचिन्हित वर्ण bufHead[RAND_HEAD_LEN];
        अचिन्हित पूर्णाङ्क sizeHead;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_table();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        sizeHead=crypthead(password,bufHead,RAND_HEAD_LEN,zi->ci.कुंजियाँ,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_header_size = sizeHead;

        यदि (ZWRITE64(zi->z_filefunc,zi->filestream,bufHead,sizeHead) != sizeHead)
                err = ZIP_ERRNO;
    }
#    endif

    यदि (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip4(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                        ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                        ध्रुव शून्य* extrafield_global, uInt size_extrafield_global,
                                        ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level, पूर्णाङ्क raw,
                                        पूर्णाङ्क windowBits,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                        ध्रुव वर्ण* password, uLong crcForCrypting,
                                        uLong versionMadeBy, uLong flagBase) {
    प्रत्यागम zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, versionMadeBy, flagBase, 0);
}

बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip3(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                        ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                        ध्रुव शून्य* extrafield_global, uInt size_extrafield_global,
                                        ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level, पूर्णाङ्क raw,
                                        पूर्णाङ्क windowBits,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                        ध्रुव वर्ण* password, uLong crcForCrypting) {
    प्रत्यागम zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip3_64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                         ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                         ध्रुव शून्य* extrafield_global, uInt size_extrafield_global,
                                         ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level, पूर्णाङ्क raw,
                                         पूर्णाङ्क windowBits,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                         ध्रुव वर्ण* password, uLong crcForCrypting, पूर्णाङ्क zip64) {
    प्रत्यागम zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip2(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                        ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                        ध्रुव शून्य* extrafield_global, uInt size_extrafield_global,
                                        ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level, पूर्णाङ्क raw) {
    प्रत्यागम zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, 0);
}

बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip2_64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                           ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                           ध्रुव शून्य* extrafield_global, uInt size_extrafield_global,
                                           ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level, पूर्णाङ्क raw, पूर्णाङ्क zip64) {
    प्रत्यागम zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                         ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                         ध्रुव शून्य*extrafield_global, uInt size_extrafield_global,
                                         ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level, पूर्णाङ्क zip64) {
    प्रत्यागम zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णाङ्क ZEXPORT zipOpenNewFileInZip(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                       ध्रुव शून्य* extrafield_local, uInt size_extrafield_local,
                                       ध्रुव शून्य*extrafield_global, uInt size_extrafield_global,
                                       ध्रुव वर्ण* comment, पूर्णाङ्क method, पूर्णाङ्क level) {
    प्रत्यागम zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, 0);
}

local पूर्णाङ्क zip64FlushWriteBuffer(zip64_internal* zi) {
    पूर्णाङ्क err=ZIP_OK;

    यदि (zi->ci.encrypt != 0)
    {
#ifndef NOCRYPT
        uInt i;
        पूर्णाङ्क t;
        प्रति (i=0;i<zi->ci.pos_in_buffered_data;i++)
            zi->ci.buffered_data[i] = zencode(zi->ci.कुंजियाँ, zi->ci.pcrc_32_tab, zi->ci.buffered_data[i],t);
#endif
    }

    यदि (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_data,zi->ci.pos_in_buffered_data) != zi->ci.pos_in_buffered_data)
      err = ZIP_ERRNO;

    zi->ci.totalCompressedData += zi->ci.pos_in_buffered_data;

#ifdef HAVE_BZIP2
    यदि(zi->ci.method == Z_BZIP2ED)
    {
      zi->ci.totalUncompressedData += zi->ci.bstream.total_in_lo32;
      zi->ci.bstream.total_in_lo32 = 0;
      zi->ci.bstream.total_in_hi32 = 0;
    }
    अन्यथा
#endif
    {
      zi->ci.totalUncompressedData += zi->ci.stream.total_in;
      zi->ci.stream.total_in = 0;
    }


    zi->ci.pos_in_buffered_data = 0;

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zipWriteInFileInZip(zipFile file, ध्रुव शून्य* buf, अचिन्हित पूर्णाङ्क len) {
    zip64_internal* zi;
    पूर्णाङ्क err=ZIP_OK;

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    यदि (zi->in_opened_file_inzip == 0)
        प्रत्यागम ZIP_PARAMERROR;

    zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uInt)len);

#ifdef HAVE_BZIP2
    यदि(zi->ci.method == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.next_in = (शून्य*)buf;
      zi->ci.bstream.avail_in = len;
      err = BZ_RUN_OK;

      यावत् ((err==BZ_RUN_OK) && (zi->ci.bstream.avail_in>0))
      {
        यदि (zi->ci.bstream.avail_out == 0)
        {
          यदि (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (वर्ण*)zi->ci.buffered_data;
        }


        यदि(err != BZ_RUN_OK)
          भङ्ग;

        यदि ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong uTotalOutBefore_lo = zi->ci.bstream.total_out_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_RUN);

          zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore_lo) ;
        }
      }

      यदि(err == BZ_RUN_OK)
        err = ZIP_OK;
    }
    अन्यथा
#endif
    {
      zi->ci.stream.next_in = buf;
      zi->ci.stream.avail_in = len;

      यावत् ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
      {
          यदि (zi->ci.stream.avail_out == 0)
          {
              यदि (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
              zi->ci.stream.next_out = zi->ci.buffered_data;
          }


          यदि(err != ZIP_OK)
              भङ्ग;

          यदि ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong uTotalOutBefore = zi->ci.stream.total_out;
              err=deflate(&zi->ci.stream,  Z_NO_FLUSH);

              zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
          }
          अन्यथा
          {
              uInt copy_this,i;
              यदि (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                  copy_this = zi->ci.stream.avail_in;
              अन्यथा
                  copy_this = zi->ci.stream.avail_out;

              प्रति (i = 0; i < copy_this; i++)
                  *(((वर्ण*)zi->ci.stream.next_out)+i) =
                      *(((ध्रुव वर्ण*)zi->ci.stream.next_in)+i);
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

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zipCloseFileInZipRaw(zipFile file, uLong uncompressed_size, uLong crc32) {
    प्रत्यागम zipCloseFileInZipRaw64 (file, uncompressed_size, crc32);
}

बाह्य पूर्णाङ्क ZEXPORT zipCloseFileInZipRaw64(zipFile file, ZPOS64_T uncompressed_size, uLong crc32) {
    zip64_internal* zi;
    ZPOS64_T compressed_size;
    uLong invalid_मूल्य = 0xffffffff;
    अचिन्हित datasize = 0;
    पूर्णाङ्क err=ZIP_OK;

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    यदि (zi->in_opened_file_inzip == 0)
        प्रत्यागम ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    यदि ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
                {
                        यावत् (err==ZIP_OK)
                        {
                                uLong uTotalOutBefore;
                                यदि (zi->ci.stream.avail_out == 0)
                                {
                                        यदि (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                                        zi->ci.stream.next_out = zi->ci.buffered_data;
                                }
                                uTotalOutBefore = zi->ci.stream.total_out;
                                err=deflate(&zi->ci.stream,  Z_FINISH);
                                zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
                        }
                }
    अन्यथा यदि ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ifdef HAVE_BZIP2
      err = BZ_FINISH_OK;
      यावत् (err==BZ_FINISH_OK)
      {
        uLong uTotalOutBefore;
        यदि (zi->ci.bstream.avail_out == 0)
        {
          यदि (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (वर्ण*)zi->ci.buffered_data;
        }
        uTotalOutBefore = zi->ci.bstream.total_out_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        यदि(err == BZ_STREAM_END)
          err = Z_STREAM_END;

        zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore);
      }

      यदि(err == BZ_FINISH_OK)
        err = ZIP_OK;
#endif
    }

    यदि (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    यदि ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
                {
        यदि (zip64FlushWriteBuffer(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    यदि ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
    {
        पूर्णाङ्क tmp_err = deflateEnd(&zi->ci.stream);
        यदि (err == ZIP_OK)
            err = tmp_err;
        zi->ci.stream_initialised = 0;
    }
#ifdef HAVE_BZIP2
    अन्यथा यदि((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
      पूर्णाङ्क tmperr = BZ2_bzCompressEnd(&zi->ci.bstream);
                        यदि (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.stream_initialised = 0;
    }
#endif

    यदि (!zi->ci.raw)
    {
        crc32 = (uLong)zi->ci.crc32;
        uncompressed_size = zi->ci.totalUncompressedData;
    }
    compressed_size = zi->ci.totalCompressedData;

#    ifndef NOCRYPT
    compressed_size += zi->ci.crypt_header_size;
#    endif

    /* update Current Item crc and sizes, */
    यदि(compressed_size >= 0xffffffff || uncompressed_size >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+6,(uLong)45,2);

    }

    zip64local_put_मूल्य_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


    यदि(compressed_size >= 0xffffffff)
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+20, invalid_मूल्य,4); /*compr size*/
    अन्यथा
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+20, compressed_size,4); /*compr size*/

    /* set internal file attributes field */
    यदि (zi->ci.stream.data_type == Z_ASCII)
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+36,(uLong)Z_ASCII,2);

    यदि(uncompressed_size >= 0xffffffff)
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+24, invalid_मूल्य,4); /*uncompr size*/
    अन्यथा
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+24, uncompressed_size,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    यदि(uncompressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    यदि(compressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    यदि(zi->ci.pos_local_header >= 0xffffffff)
      datasize += 8;

    यदि(datasize > 0)
    {
      वर्ण* p = रिक्त;

      यदि((uLong)(datasize + 4) > zi->ci.size_centralExtraFree)
      {
        /* we cannot write more data to the buffer that we have room for. */
        प्रत्यागम ZIP_BADZIPFILE;
      }

      p = zi->ci.central_header + zi->ci.size_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_मूल्य_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_मूल्य_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      यदि(uncompressed_size >= 0xffffffff)
      {
        zip64local_put_मूल्य_inmemory(p, uncompressed_size, 8);
        p += 8;
      }

      यदि(compressed_size >= 0xffffffff)
      {
        zip64local_put_मूल्य_inmemory(p, compressed_size, 8);
        p += 8;
      }

      यदि(zi->ci.pos_local_header >= 0xffffffff)
      {
        zip64local_put_मूल्य_inmemory(p, zi->ci.pos_local_header, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.size_centralExtraFree -= datasize + 4;
      zi->ci.size_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.size_centralExtra += datasize + 4;
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+30,(uLong)zi->ci.size_centralExtra,2);
    }

    यदि (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir, zi->ci.central_header, (uLong)zi->ci.size_centralheader);

    मुक्त(zi->ci.central_header);

    यदि (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        यदि (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;

        यदि (err==ZIP_OK)
            err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

        यदि(uncompressed_size >= 0xffffffff || compressed_size >= 0xffffffff )
        {
          यदि(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            यदि (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET)!=0)
              err = ZIP_ERRNO;

            यदि (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, uncompressed_size, 8);

            यदि (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, compressed_size, 8);
          }
          अन्यथा
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        अन्यथा
        {
          यदि (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,compressed_size,4);

          यदि (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,uncompressed_size,4);
        }

        यदि (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;
    }

    zi->संख्या_entry ++;
    zi->in_opened_file_inzip = 0;

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zipCloseFileInZip(zipFile file) {
    प्रत्यागम zipCloseFileInZipRaw (file,0,0);
}

local पूर्णाङ्क Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, ZPOS64_T zip64eocd_pos_inzip) {
  पूर्णाङ्क err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->add_position_when_writing_offset;

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    यदि (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    प्रत्यागम err;
}

local पूर्णाङ्क Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  पूर्णाङ्क err = ZIP_OK;

  uLong Zip64DataSize = 44;

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  यदि (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64DataSize,8); /* why ZPOS64_T of this ? */

  यदि (err==ZIP_OK) /* version made by */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  यदि (err==ZIP_OK) /* version needed */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  यदि (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  यदि (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, zi->संख्या_entry, 8);

  यदि (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, zi->संख्या_entry, 8);

  यदि (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(ZPOS64_T)size_centraldir,8);

  यदि (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  प्रत्यागम err;
}

local पूर्णाङ्क Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  पूर्णाङ्क err = ZIP_OK;

  /*signature*/
  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  यदि (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  यदि (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      यदि(zi->संख्या_entry >= 0xFFFF)
        err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      अन्यथा
        err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->संख्या_entry,2);
    }
  }

  यदि (err==ZIP_OK) /* total number of entries in the central dir */
  {
    यदि(zi->संख्या_entry >= 0xFFFF)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->संख्या_entry,2);
  }

  यदि (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)size_centraldir,4);

  यदि (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    यदि(pos >= 0xffffffff)
    {
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->add_position_when_writing_offset),4);
  }

   प्रत्यागम err;
}

local पूर्णाङ्क Write_GlobalComment(zip64_internal* zi, ध्रुव वर्ण* global_comment) {
  पूर्णाङ्क err = ZIP_OK;
  uInt size_global_comment = 0;

  यदि(global_comment != रिक्त)
    size_global_comment = (uInt)म_माप(global_comment);

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)size_global_comment,2);

  यदि (err == ZIP_OK && size_global_comment > 0)
  {
    यदि (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, size_global_comment) != size_global_comment)
      err = ZIP_ERRNO;
  }
  प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zipClose(zipFile file, ध्रुव वर्ण* global_comment) {
    zip64_internal* zi;
    पूर्णाङ्क err = 0;
    uLong size_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    यदि (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

#ifndef NO_ADDFILEINEXISTINGZIP
    यदि (global_comment==रिक्त)
        global_comment = zi->globalcomment;
#endif

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    यदि (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
        यावत् (ldi!=रिक्त)
        {
            यदि ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
            {
                यदि (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->data, ldi->filled_in_this_block) != ldi->filled_in_this_block)
                    err = ZIP_ERRNO;
            }

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    set_end(&zi->set);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    यदि(pos >= 0xffffffff || zi->संख्या_entry >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

      Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
    }

    यदि (err==ZIP_OK)
      err = Write_EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

    यदि(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    यदि (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        यदि (err == ZIP_OK)
            err = ZIP_ERRNO;

#ifndef NO_ADDFILEINEXISTINGZIP
    मुक्त(zi->globalcomment);
#endif
    मुक्त(zi);

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zipRemoveExtraInfoBlock(वर्ण* pData, पूर्णाङ्क* dataLen, लघु sHeader) {
  वर्ण* p = pData;
  पूर्णाङ्क size = 0;
  वर्ण* pNewHeader;
  वर्ण* pTmp;
  लघु header;
  लघु dataSize;

  पूर्णाङ्क retVal = ZIP_OK;

  यदि(pData == रिक्त || dataLen == रिक्त || *dataLen < 4)
    प्रत्यागम ZIP_PARAMERROR;

  pNewHeader = (वर्ण*)ALLOC((अचिन्हित)*dataLen);
  pTmp = pNewHeader;

  यावत्(p < (pData + *dataLen))
  {
    header = *(लघु*)p;
    dataSize = *(((लघु*)p)+1);

    यदि( header == sHeader ) /* Header found. */
    {
      p += dataSize + 4; /* skip it. do not copy to temp buffer */
    }
    अन्यथा
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      स_नकल(pTmp, p, dataSize + 4);
      p += dataSize + 4;
      size += dataSize + 4;
    }

  }

  यदि(size < *dataLen)
  {
    /* clean old extra info block. */
    स_रखो(pData,0, *dataLen);

    /* copy the new extra info block over the old */
    यदि(size > 0)
      स_नकल(pData, pNewHeader, size);

    /* set the new extra info size */
    *dataLen = size;

    retVal = ZIP_OK;
  }
  अन्यथा
    retVal = ZIP_ERRNO;

  मुक्त(pNewHeader);

  प्रत्यागम retVal;
}
