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


#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <time.h>
#अगर_अघोषित ZLIB_CONST
#  define ZLIB_CONST
#पूर्ण_अगर
#अगर_घोषित ZLIB_DLL
#  undef ZLIB_DLL
#पूर्ण_अगर
#समावेश "zlib.h"
#समावेश "zip.h"

#अगर_घोषित STDC
#  include <stddef.h>
#पूर्ण_अगर
#अगर_घोषित NO_ERRNO_H
    बाह्य पूर्णांक त्रुटि_सं;
#अन्यथा
#   include <errno.h>
#पूर्ण_अगर


#अगर_अघोषित local
#  define local अटल
#पूर्ण_अगर
/* compile with -Dlocal if your debugger can't find static symbols */

#अगर_अघोषित VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#पूर्ण_अगर

#अगर_अघोषित Z_BUFSIZE
#घोषणा Z_BUFSIZE (64*1024) /* (16384) */
#पूर्ण_अगर

#अगर_अघोषित Z_MAXFILENAMEINZIP
#घोषणा Z_MAXFILENAMEINZIP (256)
#पूर्ण_अगर

#अगर_अघोषित ALLOC
# define ALLOC(आकार) (दो_स्मृति(आकार))
#पूर्ण_अगर

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#घोषणा MAKEULONG64(a, b) ((ZPOS64_T)(((अचिन्हित दीर्घ)(a)) | ((ZPOS64_T)((अचिन्हित दीर्घ)(b))) << 32))

#अगर_अघोषित प्रस्तुत_से
#घोषणा प्रस्तुत_से    1
#पूर्ण_अगर

#अगर_अघोषित अंत_से
#घोषणा अंत_से    2
#पूर्ण_अगर

#अगर_अघोषित शुरू_से
#घोषणा शुरू_से    0
#पूर्ण_अगर

#अगर_अघोषित DEF_MEM_LEVEL
#अगर MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#पूर्ण_अगर
#पूर्ण_अगर
स्थिर अक्षर zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#घोषणा SIZEDATA_INDATABLOCK (4096-(4*4))

#घोषणा LOCALHEADERMAGIC    (0x04034b50)
#घोषणा CENTRALHEADERMAGIC  (0x02014b50)
#घोषणा ENDHEADERMAGIC      (0x06054b50)
#घोषणा ZIP64ENDHEADERMAGIC      (0x6064b50)
#घोषणा ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#घोषणा FLAG_LOCALHEADER_OFFSET (0x06)
#घोषणा CRC_LOCALHEADER_OFFSET  (0x0e)

#घोषणा SIZECENTRALHEADER (0x2e) /* 46 */

प्रकार काष्ठा linkedlist_datablock_internal_s
{
  काष्ठा linkedlist_datablock_internal_s* अग्र_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignment */
  अचिन्हित अक्षर तथ्य[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

प्रकार काष्ठा linkedlist_तथ्य_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_तथ्य;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#घोषणा SKIPSET_EXPORT अटल
प्रकार अक्षर *set_कुंजी_t;
#घोषणा set_cmp(a, b) म_भेद(a, b)
#घोषणा set_drop(s, k) set_free(s, k)
#समावेश "skipset.h"
प्रकार काष्ठा {
    अचिन्हित अक्षर *अग्र;        /* next byte in datablock data */
    माप_प्रकार बायाँ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_internal *गाँठ;    /* current datablock */
} block_t;


प्रकार काष्ठा
{
    z_stream stream;            /* zLib stream structure for inflate */
#अगर_घोषित HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#पूर्ण_अगर

    पूर्णांक  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_तथ्य;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_header;     /* offset of the local header of the file
                                     currently writing */
    अक्षर* central_header;       /* central header data for the current file */
    uLong आकार_central_Extra;
    uLong आकार_centralheader;   /* size of the central header for cur file */
    uLong आकार_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    पूर्णांक  method;                /* compression method of file currently wr.*/
    पूर्णांक  raw;                   /* 1 for directly writing raw data */
    Byte buffered_तथ्य[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
    पूर्णांक  encrypt;
    पूर्णांक  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T योग_Compressed_तथ्य;
    ZPOS64_T योग_Uncompressed_तथ्य;
#अगर_अघोषित NOCRYPT
    अचिन्हित दीर्घ कुंजियाँ[3];     /* keys defining the pseudo-random sequence */
    स्थिर z_crc_t* pcrc_32_tab;
    अचिन्हित crypt_header_आकार;
#पूर्ण_अगर
} curfile64_info;

प्रकार काष्ठा
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_तथ्य central_dir;/* datablock with central dir in construction*/
    पूर्णांक  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T add_position_when_writing_offset;
    ZPOS64_T संख्या_entry;

#अगर_अघोषित NO_ADDFILEINEXISTINGZIP
    अक्षर *globalcomment;
#पूर्ण_अगर

    /* Support for zipAlreadyThere(). */
    set_t set;              /* set for detecting name collisions */
    block_t block;          /* block for reading the central directory */

} zip64_internal;


#अगर_अघोषित NOCRYPT
#घोषणा INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#समावेश "crypt.h"
#पूर्ण_अगर

local linkedlist_datablock_internal* allocate_new_datablock(व्योम) {
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(माप(linkedlist_datablock_internal));
    अगर (ldi!=शून्य)
    {
        ldi->अग्र_datablock = शून्य ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    वापस ldi;
}

local व्योम free_datablock(linkedlist_datablock_internal* ldi) {
    जबतक (ldi!=शून्य)
    {
        linkedlist_datablock_internal* ldinext = ldi->अग्र_datablock;
        मुक्त(ldi);
        ldi = ldinext;
    }
}

local व्योम init_linkedlist(linkedlist_तथ्य* ll) {
    ll->first_block = ll->last_block = शून्य;
}

local व्योम free_linkedlist(linkedlist_तथ्य* ll) {
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = शून्य;
}


local पूर्णांक add_तथ्य_in_datablock(linkedlist_तथ्य* ll, स्थिर व्योम* buf, uLong len) {
    linkedlist_datablock_internal* ldi;
    स्थिर अचिन्हित अक्षर* from_copy;

    अगर (ll==शून्य)
        वापस ZIP_INTERNALERROR;

    अगर (ll->last_block == शून्य)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        अगर (ll->first_block == शून्य)
            वापस ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (स्थिर अचिन्हित अक्षर*)buf;

    जबतक (len>0)
    {
        uInt copy_this;
        uInt i;
        अचिन्हित अक्षर* to_copy;

        अगर (ldi->avail_in_this_block==0)
        {
            ldi->अग्र_datablock = allocate_new_datablock();
            अगर (ldi->अग्र_datablock == शून्य)
                वापस ZIP_INTERNALERROR;
            ldi = ldi->अग्र_datablock ;
            ll->last_block = ldi;
        }

        अगर (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        अन्यथा
            copy_this = (uInt)len;

        to_copy = &(ldi->तथ्य[ldi->filled_in_this_block]);

        क्रम (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    वापस ZIP_OK;
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
local व्योम block_init(block_t *block, linkedlist_तथ्य *list) {
    block->गाँठ = list->first_block;
    block->अग्र = block->गाँठ->तथ्य;
    block->बायाँ = block->गाँठ->filled_in_this_block;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local व्योम block_stop(block_t *block) {
    block->बायाँ = 0;
    block->अग्र = शून्य;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local पूर्णांक block_end(block_t *block) {
    linkedlist_datablock_internal *गाँठ = block->गाँठ;
    अगर (गाँठ == शून्य)
        /* This block was previously terminated with extreme prejudice. */
        वापस 1;
    अगर (block->अग्र < गाँठ->तथ्य + गाँठ->filled_in_this_block)
        /* There are more bytes to read in the current datablock. */
        वापस 0;
    जबतक (गाँठ->अग्र_datablock != शून्य) {
        अगर (गाँठ->filled_in_this_block != 0)
            /* There are some bytes in a later datablock. */
            वापस 0;
        गाँठ = गाँठ->अग्र_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    वापस 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local पूर्णांक block_get(block_t *block) {
    जबतक (block->बायाँ == 0) {
        अगर (block->गाँठ == शून्य)
            /* We've been marked bad. Return end. */
            वापस -1;
        /* Update left in case more was filled in since we were last here. */
        block->बायाँ = block->गाँठ->filled_in_this_block -
                      (माप_प्रकार)(block->अग्र - block->गाँठ->तथ्य);
        अगर (block->बायाँ != 0)
            /* There was indeed more data appended in the current datablock. */
            अवरोध;
        अगर (block->गाँठ->अग्र_datablock == शून्य)
            /* No more data here, and there is no next datablock. At the end. */
            वापस -1;
        /* Try the next datablock for more data. */
        block->गाँठ = block->गाँठ->अग्र_datablock;
        block->अग्र = block->गाँठ->तथ्य;
        block->बायाँ = block->गाँठ->filled_in_this_block;
    }
    /* We have a byte to return. */
    block->बायाँ--;
    वापस *block->अग्र++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local दीर्घ block_get2(block_t *block) {
    पूर्णांक निचला = block_get(block);
    पूर्णांक ऊपरी = block_get(block);
    वापस निचला < 0 || ऊपरी < 0 ? -1 : निचला | ((दीर्घ)ऊपरी << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local माप_प्रकार block_read(block_t *block, अचिन्हित अक्षर *buf, माप_प्रकार len) {
    माप_प्रकार need = len;
    माप_प्रकार take;
    जबतक (need) {
        अगर (block->बायाँ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            पूर्णांक got = block_get(block);
            अगर (got == -1)
                /* Reached the end. */
                अवरोध;
            *buf++ = (अचिन्हित अक्षर)got;
            need--;
            जारी;
        }
        take = need > block->बायाँ ? block->बायाँ : need;
        स_नकल(buf, block->अग्र, take);
        block->अग्र += take;
        block->बायाँ -= take;
        buf += take;
        need -= take;
    }
    वापस len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local पूर्णांक block_skip(block_t *block, माप_प्रकार n) {
    जबतक (n > block->बायाँ) {
        n -= block->बायाँ;
        block->अग्र += block->बायाँ;
        block->बायाँ = 0;
        अगर (block_get(block) == -1)
            वापस -1;
        n--;
    }
    block->अग्र += n;
    block->बायाँ -= n;
    वापस 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local अक्षर *block_central_name(block_t *block, set_t *set) {
    अचिन्हित flen, xlen, clen;
    अक्षर *name = शून्य;
    क्रम (;;) {
        अगर (block_end(block))
            /* At the end of the central directory (so far). */
            वापस शून्य;

        /* Check for a central directory record signature. */
        अगर (block_get2(block) != (CENTRALHEADERMAGIC & 0xffff) ||
            block_get2(block) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            अवरोध;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        block_skip(block, 24);
        flen = (अचिन्हित)block_get2(block);    /* file name length */
        xlen = (अचिन्हित)block_get2(block);    /* extra length */
        clen = (अचिन्हित)block_get2(block);    /* comment length */
        अगर (block_skip(block, 12) == -1)
            /* Premature end of the record. */
            अवरोध;

        /* Extract the name and skip over the extra and comment fields. */
        name = set_alloc(set, शून्य, flen + 1);
        अगर (block_read(block, (अचिन्हित अक्षर *)name, flen) < flen ||
            block_skip(block, xlen + clen) == -1)
            /* Premature end of the record. */
            अवरोध;

        /* Check for embedded nuls in the name. */
        अगर (स_प्रथम(name, 0, flen) != शून्य) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            set_free(set, name);
            जारी;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        वापस name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    set_free(set, name);
    block_stop(block);
    वापस शून्य;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
बाह्य पूर्णांक ZEXPORT zipAlreadyThere(zipFile file, अक्षर स्थिर *name) {
    माप_प्रकार len;
    अक्षर *copy;
    पूर्णांक found;
    zip64_internal *zip = file;
    अगर (zip == शून्य)
        वापस ZIP_PARAMERROR;
    अगर (zip->central_dir.first_block == शून्य)
        /* No central directory yet, so no, name isn't there. */
        वापस 0;
    अगर (बनाओ_लाँघ(zip->set.env)) {
        /* Memory allocation failure. */
        set_end(&zip->set);
        वापस -2;
    }
    अगर (!set_ok(&zip->set)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        set_start(&zip->set);
        block_init(&zip->block, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    क्रम (;;) {
        अक्षर *there = block_central_name(&zip->block, &zip->set);
        अगर (there == शून्य) {
            अगर (zip->block.अग्र == शून्य)
                /* The central directory is invalid. */
                वापस -1;
            अवरोध;
        }

        /* Add there to the set. */
        अगर (set_जोड़ो(&zip->set, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            set_free(&zip->set, there);
    }

    /* Return true if name is in the central directory. */
    len = म_माप(name);
    copy = set_alloc(&zip->set, शून्य, len + 1);
    स_नकल(copy, name, len + 1);
    found = set_found(&zip->set, copy);
    set_free(&zip->set, copy);
    वापस found;
}


/****************************************************************************/

#अगर_अघोषित NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local पूर्णांक zip64local_put_मूल्य(स्थिर zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, पूर्णांक nbByte) {
    अचिन्हित अक्षर buf[8];
    पूर्णांक n;
    क्रम (n = 0; n < nbByte; n++)
    {
        buf[n] = (अचिन्हित अक्षर)(x & 0xff);
        x >>= 8;
    }
    अगर (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      क्रम (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    अगर (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        वापस ZIP_ERRNO;
    अन्यथा
        वापस ZIP_OK;
}

local व्योम zip64local_put_मूल्य_inmemory (व्योम* dest, ZPOS64_T x, पूर्णांक nbByte) {
    अचिन्हित अक्षर* buf=(अचिन्हित अक्षर*)dest;
    पूर्णांक n;
    क्रम (n = 0; n < nbByte; n++) {
        buf[n] = (अचिन्हित अक्षर)(x & 0xff);
        x >>= 8;
    }

    अगर (x != 0)
    {     /* data overflow - hack for ZIP64 */
       क्रम (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_TmzDateToDosDate(स्थिर tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    अगर (year>=1980)
        year-=1980;
    अन्यथा अगर (year>=80)
        year-=80;
    वापस
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local पूर्णांक zip64local_getByte(स्थिर zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, पूर्णांक* pi) {
    अचिन्हित अक्षर c;
    पूर्णांक err = (पूर्णांक)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    अगर (err==1)
    {
        *pi = (पूर्णांक)c;
        वापस ZIP_OK;
    }
    अन्यथा
    {
        अगर (ZERROR64(*pzlib_filefunc_def,filestream))
            वापस ZIP_ERRNO;
        अन्यथा
            वापस ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local पूर्णांक zip64local_getShort(स्थिर zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    पूर्णांक i = 0;
    पूर्णांक err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    अगर (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    अगर (err==ZIP_OK)
        *pX = x;
    अन्यथा
        *pX = 0;
    वापस err;
}

local पूर्णांक zip64local_getLong(स्थिर zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    पूर्णांक i = 0;
    पूर्णांक err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    अगर (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    अगर (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    अगर (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    अगर (err==ZIP_OK)
        *pX = x;
    अन्यथा
        *pX = 0;
    वापस err;
}


local पूर्णांक zip64local_getLong64(स्थिर zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  पूर्णांक i = 0;
  पूर्णांक err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  अगर (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  अगर (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  अगर (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  अगर (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  अगर (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  अगर (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  अगर (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  अगर (err==ZIP_OK)
    *pX = x;
  अन्यथा
    *pX = 0;

  वापस err;
}

#अगर_अघोषित BUFREADCOMMENT
#घोषणा BUFREADCOMMENT (0x400)
#पूर्ण_अगर
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_खोज_Central_Dir(स्थिर zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  अचिन्हित अक्षर* buf;
  ZPOS64_T u_आकार_File;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  अगर (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    वापस 0;


  u_आकार_File = ZTELL64(*pzlib_filefunc_def,filestream);

  अगर (uMaxBack>u_आकार_File)
    uMaxBack = u_आकार_File;

  buf = (अचिन्हित अक्षर*)ALLOC(BUFREADCOMMENT+4);
  अगर (buf==शून्य)
    वापस 0;

  uBackRead = 4;
  जबतक (uBackRead<uMaxBack)
  {
    uLong u_Read_आकार;
    ZPOS64_T uReadPos ;
    पूर्णांक i;
    अगर (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    अन्यथा
      uBackRead+=BUFREADCOMMENT;
    uReadPos = u_आकार_File-uBackRead ;

    u_Read_आकार = ((BUFREADCOMMENT+4) < (u_आकार_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_आकार_File-uReadPos);
    अगर (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      अवरोध;

    अगर (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_आकार)!=u_Read_आकार)
      अवरोध;

    क्रम (i=(पूर्णांक)u_Read_आकार-3; (i--)>0;)
      अगर (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+(अचिन्हित)i;
        अवरोध;
      }

    अगर (uPosFound!=0)
      अवरोध;
  }
  मुक्त(buf);
  वापस uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_खोज_Central_Dir64(स्थिर zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  अचिन्हित अक्षर* buf;
  ZPOS64_T u_आकार_File;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  अगर (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    वापस 0;

  u_आकार_File = ZTELL64(*pzlib_filefunc_def,filestream);

  अगर (uMaxBack>u_आकार_File)
    uMaxBack = u_आकार_File;

  buf = (अचिन्हित अक्षर*)ALLOC(BUFREADCOMMENT+4);
  अगर (buf==शून्य)
    वापस 0;

  uBackRead = 4;
  जबतक (uBackRead<uMaxBack)
  {
    uLong u_Read_आकार;
    ZPOS64_T uReadPos;
    पूर्णांक i;
    अगर (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    अन्यथा
      uBackRead+=BUFREADCOMMENT;
    uReadPos = u_आकार_File-uBackRead ;

    u_Read_आकार = ((BUFREADCOMMENT+4) < (u_आकार_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_आकार_File-uReadPos);
    अगर (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      अवरोध;

    अगर (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_आकार)!=u_Read_आकार)
      अवरोध;

    क्रम (i=(पूर्णांक)u_Read_आकार-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      अगर (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+(अचिन्हित)i;
        अवरोध;
      }
    }

      अगर (uPosFound!=0)
        अवरोध;
  }

  मुक्त(buf);
  अगर (uPosFound == 0)
    वापस 0;

  /* Zip64 end of central directory locator */
  अगर (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    वापस 0;

  /* the signature, already checked */
  अगर (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    वापस 0;

  /* number of the disk with the start of the zip64 end of central directory */
  अगर (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    वापस 0;
  अगर (uL != 0)
    वापस 0;

  /* relative offset of the zip64 end of central directory record */
  अगर (zip64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    वापस 0;

  /* total number of disks */
  अगर (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    वापस 0;
  अगर (uL != 1)
    वापस 0;

  /* Goto Zip64 end of central directory record */
  अगर (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    वापस 0;

  /* the signature */
  अगर (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    वापस 0;

  अगर (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    वापस 0;

  वापस relativeOffset;
}

local पूर्णांक LoadCentralDirectoryRecord(zip64_internal* pziinit) {
  पूर्णांक err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T आकार_central_dir;     /* size of the central directory  */
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
  uLong आकार_comment;

  पूर्णांक hasZIP64Record = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_खोज_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  अगर(central_pos > 0)
  {
    hasZIP64Record = 1;
  }
  अन्यथा अगर(central_pos == 0)
  {
    central_pos = zip64local_खोज_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  अगर(hasZIP64Record)
  {
    ZPOS64_T आकार_End_Of_Central_Directory;
    अगर (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    अगर (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    अगर (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &आकार_End_Of_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    अगर (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    अगर (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    अगर (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    अगर (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    अगर (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &संख्या_entry)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    अगर (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&संख्या_entry_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    अगर ((संख्या_entry_CD!=संख्या_entry) || (संख्या_disk_with_CD!=0) || (संख्या_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    अगर (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&आकार_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    अगर (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&offset_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    आकार_comment = 0;
  }
  अन्यथा
  {
    /* Read End of central Directory info */
    अगर (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    अगर (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    अगर (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    अगर (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&संख्या_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    संख्या_entry = 0;
    अगर (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      संख्या_entry = uL;

    /* total number of entries in the central dir */
    संख्या_entry_CD = 0;
    अगर (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      संख्या_entry_CD = uL;

    अगर ((संख्या_entry_CD!=संख्या_entry) || (संख्या_disk_with_CD!=0) || (संख्या_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    आकार_central_dir = 0;
    अगर (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      आकार_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    offset_central_dir = 0;
    अगर (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      offset_central_dir = uL;


    /* zipfile global comment length */
    अगर (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &आकार_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  अगर ((central_pos<offset_central_dir+आकार_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  अगर (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    वापस ZIP_ERRNO;
  }

  अगर (आकार_comment>0)
  {
    pziinit->globalcomment = (अक्षर*)ALLOC(आकार_comment+1);
    अगर (pziinit->globalcomment)
    {
      आकार_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,आकार_comment);
      pziinit->globalcomment[आकार_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (offset_central_dir+आकार_central_dir);
  pziinit->add_position_when_writing_offset = byte_before_the_zipfile;

  {
    ZPOS64_T आकार_central_dir_to_read = आकार_central_dir;
    माप_प्रकार buf_आकार_2 = SIZEDATA_INDATABLOCK;
    व्योम* buf_read = (व्योम*)ALLOC(buf_आकार_2);
    अगर (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    जबतक ((आकार_central_dir_to_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      अगर (read_this > आकार_central_dir_to_read)
        read_this = आकार_central_dir_to_read;

      अगर (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      अगर (err==ZIP_OK)
        err = add_तथ्य_in_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      आकार_central_dir_to_read-=read_this;
    }
    मुक्त(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->संख्या_entry = संख्या_entry_CD;

  अगर (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET) != 0)
    err=ZIP_ERRNO;

  वापस err;
}


#पूर्ण_अगर /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
बाह्य zipFile ZEXPORT zipOpen3(स्थिर व्योम *pathname, पूर्णांक append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_internal ziinit;
    zip64_internal* zi;
    पूर्णांक err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = शून्य;
    ziinit.z_filefunc.ztell32_file = शून्य;
    अगर (pzlib_filefunc64_32_def==शून्य)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    अन्यथा
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_STATUS_CREATE) ?
                  (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
                    (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

    अगर (ziinit.filestream == शून्य)
        वापस शून्य;

    अगर (append == APPEND_STATUS_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,अंत_से);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.संख्या_entry = 0;
    ziinit.add_position_when_writing_offset = 0;
    init_linkedlist(&(ziinit.central_dir));
    स_रखो(&ziinit.set, 0, माप(set_t));  /* make sure set appears dormant */



    zi = (zip64_internal*)ALLOC(माप(zip64_internal));
    अगर (zi==शून्य)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        वापस शून्य;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = शून्य;
    अगर (append == APPEND_STATUS_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = LoadCentralDirectoryRecord(&ziinit);
    }

    अगर (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    अगर (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        मुक्त(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        मुक्त(zi);
        वापस शून्य;
    }
    अन्यथा
    {
        *zi = ziinit;
        वापस (zipFile)zi;
    }
}

बाह्य zipFile ZEXPORT zipOpen2(स्थिर अक्षर *pathname, पूर्णांक append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    अगर (pzlib_filefunc32_def != शून्य)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        वापस zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    अन्यथा
        वापस zipOpen3(pathname, append, globalcomment, शून्य);
}

बाह्य zipFile ZEXPORT zipOpen2_64(स्थिर व्योम *pathname, पूर्णांक append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    अगर (pzlib_filefunc_def != शून्य)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = शून्य;
        zlib_filefunc64_32_def_fill.ztell32_file = शून्य;
        zlib_filefunc64_32_def_fill.zseek32_file = शून्य;
        वापस zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    अन्यथा
        वापस zipOpen3(pathname, append, globalcomment, शून्य);
}



बाह्य zipFile ZEXPORT zipOpen(स्थिर अक्षर* pathname, पूर्णांक append) {
    वापस zipOpen3((स्थिर व्योम*)pathname,append,शून्य,शून्य);
}

बाह्य zipFile ZEXPORT zipOpen64(स्थिर व्योम* pathname, पूर्णांक append) {
    वापस zipOpen3(pathname,append,शून्य,शून्य);
}

local पूर्णांक Write_LocalFileHeader(zip64_internal* zi, स्थिर अक्षर* filename, uInt आकार_extrafield_local, स्थिर व्योम* extrafield_local) {
  /* write the local header */
  पूर्णांक err;
  uInt आकार_filename = (uInt)म_माप(filename);
  uInt आकार_extrafield = आकार_extrafield_local;

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  अगर (err==ZIP_OK)
  {
    अगर(zi->ci.zip64)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  अगर (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  अगर (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.method,2);

  अगर (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  अगर (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  अगर (err==ZIP_OK)
  {
    अगर(zi->ci.zip64)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  अगर (err==ZIP_OK)
  {
    अगर(zi->ci.zip64)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  अगर (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)आकार_filename,2);

  अगर(zi->ci.zip64)
  {
    आकार_extrafield += 20;
  }

  अगर (err==ZIP_OK)
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)आकार_extrafield,2);

  अगर ((err==ZIP_OK) && (आकार_filename > 0))
  {
    अगर (ZWRITE64(zi->z_filefunc,zi->filestream,filename,आकार_filename)!=आकार_filename)
      err = ZIP_ERRNO;
  }

  अगर ((err==ZIP_OK) && (आकार_extrafield_local > 0))
  {
    अगर (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, आकार_extrafield_local) != आकार_extrafield_local)
      err = ZIP_ERRNO;
  }


  अगर ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      लघु HeaderID = 1;
      लघु तथ्य_आकार_2 = 16;
      ZPOS64_T Compressed_आकार = 0;
      ZPOS64_T Uncompressed_आकार = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)HeaderID,2);
      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)तथ्य_आकार_2,2);

      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_आकार,8);
      err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_आकार,8);
  }

  वापस err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local पूर्णांक utf8len(अचिन्हित अक्षर स्थिर *str, माप_प्रकार len) {
    वापस
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
local पूर्णांक isutf8(अक्षर स्थिर *str, माप_प्रकार len) {
    पूर्णांक utf8 = 0;
    जबतक (len) {
        पूर्णांक code = utf8len((अचिन्हित अक्षर स्थिर *)str, len);
        अगर (code < 0)
            वापस 0;
        अगर (code > 1)
            utf8 = 1;
        str += code;
        len -= (अचिन्हित)code;
    }
    वापस utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip4_64(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                           स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                           स्थिर व्योम* extrafield_global, uInt आकार_extrafield_global,
                                           स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level, पूर्णांक raw,
                                           पूर्णांक windowBits,पूर्णांक memLevel, पूर्णांक strategy,
                                           स्थिर अक्षर* password, uLong crcForCrypting,
                                           uLong versionMadeBy, uLong flagBase, पूर्णांक zip64) {
    zip64_internal* zi;
    uInt आकार_filename;
    uInt आकार_comment;
    uInt i;
    पूर्णांक err = ZIP_OK;

#    ifdef NOCRYPT
    अगर (password != शून्य)
        वापस ZIP_PARAMERROR;
#    endif

    अगर (file == शून्य)
        वापस ZIP_PARAMERROR;

#अगर_घोषित HAVE_BZIP2
    अगर ((method!=0) && (method!=Z_DEFLATED) && (method!=Z_BZIP2ED))
      वापस ZIP_PARAMERROR;
#अन्यथा
    अगर ((method!=0) && (method!=Z_DEFLATED))
      वापस ZIP_PARAMERROR;
#पूर्ण_अगर

    /* The filename and comment length must fit in 16 bits. */
    अगर ((filename!=शून्य) && (म_माप(filename)>0xffff))
        वापस ZIP_PARAMERROR;
    अगर ((comment!=शून्य) && (म_माप(comment)>0xffff))
        वापस ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    अगर ((आकार_extrafield_local>0xffff) || (आकार_extrafield_global>0xffff))
        वापस ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    अगर (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        अगर (err != ZIP_OK)
            वापस err;
    }

    अगर (filename==शून्य)
        filename="-";

    अगर (comment==शून्य)
        आकार_comment = 0;
    अन्यथा
        आकार_comment = (uInt)म_माप(comment);

    आकार_filename = (uInt)म_माप(filename);

    अगर (zipfi == शून्य)
        zi->ci.dosDate = 0;
    अन्यथा
    {
        अगर (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        अन्यथा
          zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
    }

    zi->ci.flag = flagBase;
    अगर ((level==8) || (level==9))
      zi->ci.flag |= 2;
    अगर (level==2)
      zi->ci.flag |= 4;
    अगर (level==1)
      zi->ci.flag |= 6;
    अगर (password != शून्य)
      zi->ci.flag |= 1;
    अगर (isutf8(filename, आकार_filename) &&
        (आकार_comment == 0 || isutf8(comment, आकार_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.crc32 = 0;
    zi->ci.method = method;
    zi->ci.encrypt = 0;
    zi->ci.stream_initialised = 0;
    zi->ci.pos_in_buffered_तथ्य = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_header = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.आकार_centralheader = SIZECENTRALHEADER + आकार_filename + आकार_extrafield_global + आकार_comment;
    zi->ci.आकार_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_header = (अक्षर*)ALLOC((uInt)zi->ci.आकार_centralheader + zi->ci.आकार_central_Extra_Free);

    zi->ci.आकार_central_Extra = आकार_extrafield_global;
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
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+28,(uLong)आकार_filename,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+30,(uLong)आकार_extrafield_global,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+32,(uLong)आकार_comment,2);
    zip64local_put_मूल्य_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    अगर (zipfi==शून्य)
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+36,(uLong)0,2);
    अन्यथा
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    अगर (zipfi==शून्य)
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+38,(uLong)0,4);
    अन्यथा
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    अगर(zi->ci.pos_local_header >= 0xffffffff)
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+42,(uLong)0xffffffff,4);
    अन्यथा
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header - zi->add_position_when_writing_offset,4);

    क्रम (i=0;i<आकार_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

    क्रम (i=0;i<आकार_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+आकार_filename+i) =
              *(((स्थिर अक्षर*)extrafield_global)+i);

    क्रम (i=0;i<आकार_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+आकार_filename+
              आकार_extrafield_global+i) = *(comment+i);
    अगर (zi->ci.central_header == शून्य)
        वापस ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.योग_Compressed_तथ्य = 0;
    zi->ci.योग_Uncompressed_तथ्य = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_LocalFileHeader(zi, filename, आकार_extrafield_local, extrafield_local);

#अगर_घोषित HAVE_BZIP2
    zi->ci.bstream.avail_in = (uInt)0;
    zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.bstream.अग्र_out = (अक्षर*)zi->ci.buffered_तथ्य;
    zi->ci.bstream.योग_in_hi32 = 0;
    zi->ci.bstream.योग_in_lo32 = 0;
    zi->ci.bstream.योग_out_hi32 = 0;
    zi->ci.bstream.योग_out_lo32 = 0;
#पूर्ण_अगर

    zi->ci.stream.avail_in = (uInt)0;
    zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.stream.अग्र_out = zi->ci.buffered_तथ्य;
    zi->ci.stream.योग_in = 0;
    zi->ci.stream.योग_out = 0;
    zi->ci.stream.तथ्य_type = Z_द्विआधारी;

#अगर_घोषित HAVE_BZIP2
    अगर ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED || zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
#अन्यथा
    अगर ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
#पूर्ण_अगर
    {
        अगर(zi->ci.method == Z_DEFLATED)
        {
          zi->ci.stream.zalloc = (alloc_func)0;
          zi->ci.stream.zfree = (free_func)0;
          zi->ci.stream.opaque = (voidpf)0;

          अगर (windowBits>0)
              windowBits = -windowBits;

          err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

          अगर (err==Z_OK)
              zi->ci.stream_initialised = Z_DEFLATED;
        }
        अन्यथा अगर(zi->ci.method == Z_BZIP2ED)
        {
#अगर_घोषित HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          अगर(err == BZ_OK)
            zi->ci.stream_initialised = Z_BZIP2ED;
#पूर्ण_अगर
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_header_आकार = 0;
    अगर ((err==Z_OK) && (password != शून्य))
    {
        अचिन्हित अक्षर buf_शीर्ष[RAND_शीर्ष_LEN];
        अचिन्हित पूर्णांक आकार_शीर्ष;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_table();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        आकार_शीर्ष=crypthead(password,buf_शीर्ष,RAND_शीर्ष_LEN,zi->ci.कुंजियाँ,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_header_आकार = आकार_शीर्ष;

        अगर (ZWRITE64(zi->z_filefunc,zi->filestream,buf_शीर्ष,आकार_शीर्ष) != आकार_शीर्ष)
                err = ZIP_ERRNO;
    }
#    endif

    अगर (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    वापस err;
}

बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip4(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                        स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                        स्थिर व्योम* extrafield_global, uInt आकार_extrafield_global,
                                        स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level, पूर्णांक raw,
                                        पूर्णांक windowBits,पूर्णांक memLevel, पूर्णांक strategy,
                                        स्थिर अक्षर* password, uLong crcForCrypting,
                                        uLong versionMadeBy, uLong flagBase) {
    वापस zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, versionMadeBy, flagBase, 0);
}

बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip3(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                        स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                        स्थिर व्योम* extrafield_global, uInt आकार_extrafield_global,
                                        स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level, पूर्णांक raw,
                                        पूर्णांक windowBits,पूर्णांक memLevel, पूर्णांक strategy,
                                        स्थिर अक्षर* password, uLong crcForCrypting) {
    वापस zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip3_64(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                         स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                         स्थिर व्योम* extrafield_global, uInt आकार_extrafield_global,
                                         स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level, पूर्णांक raw,
                                         पूर्णांक windowBits,पूर्णांक memLevel, पूर्णांक strategy,
                                         स्थिर अक्षर* password, uLong crcForCrypting, पूर्णांक zip64) {
    वापस zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip2(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                        स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                        स्थिर व्योम* extrafield_global, uInt आकार_extrafield_global,
                                        स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level, पूर्णांक raw) {
    वापस zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   शून्य, 0, VERSIONMADEBY, 0, 0);
}

बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip2_64(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                           स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                           स्थिर व्योम* extrafield_global, uInt आकार_extrafield_global,
                                           स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level, पूर्णांक raw, पूर्णांक zip64) {
    वापस zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   शून्य, 0, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip64(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                         स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                         स्थिर व्योम*extrafield_global, uInt आकार_extrafield_global,
                                         स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level, पूर्णांक zip64) {
    वापस zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   शून्य, 0, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णांक ZEXPORT zipOpenNewFileInZip(zipFile file, स्थिर अक्षर* filename, स्थिर zip_fileinfo* zipfi,
                                       स्थिर व्योम* extrafield_local, uInt आकार_extrafield_local,
                                       स्थिर व्योम*extrafield_global, uInt आकार_extrafield_global,
                                       स्थिर अक्षर* comment, पूर्णांक method, पूर्णांक level) {
    वापस zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   शून्य, 0, VERSIONMADEBY, 0, 0);
}

local पूर्णांक zip64_Flush_Write_बफर(zip64_internal* zi) {
    पूर्णांक err=ZIP_OK;

    अगर (zi->ci.encrypt != 0)
    {
#अगर_अघोषित NOCRYPT
        uInt i;
        पूर्णांक t;
        क्रम (i=0;i<zi->ci.pos_in_buffered_तथ्य;i++)
            zi->ci.buffered_तथ्य[i] = zencode(zi->ci.कुंजियाँ, zi->ci.pcrc_32_tab, zi->ci.buffered_तथ्य[i],t);
#पूर्ण_अगर
    }

    अगर (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_तथ्य,zi->ci.pos_in_buffered_तथ्य) != zi->ci.pos_in_buffered_तथ्य)
      err = ZIP_ERRNO;

    zi->ci.योग_Compressed_तथ्य += zi->ci.pos_in_buffered_तथ्य;

#अगर_घोषित HAVE_BZIP2
    अगर(zi->ci.method == Z_BZIP2ED)
    {
      zi->ci.योग_Uncompressed_तथ्य += zi->ci.bstream.योग_in_lo32;
      zi->ci.bstream.योग_in_lo32 = 0;
      zi->ci.bstream.योग_in_hi32 = 0;
    }
    अन्यथा
#पूर्ण_अगर
    {
      zi->ci.योग_Uncompressed_तथ्य += zi->ci.stream.योग_in;
      zi->ci.stream.योग_in = 0;
    }


    zi->ci.pos_in_buffered_तथ्य = 0;

    वापस err;
}

बाह्य पूर्णांक ZEXPORT zipWriteInFileInZip(zipFile file, स्थिर व्योम* buf, अचिन्हित पूर्णांक len) {
    zip64_internal* zi;
    पूर्णांक err=ZIP_OK;

    अगर (file == शून्य)
        वापस ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    अगर (zi->in_opened_file_inzip == 0)
        वापस ZIP_PARAMERROR;

    zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uInt)len);

#अगर_घोषित HAVE_BZIP2
    अगर(zi->ci.method == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.अग्र_in = (व्योम*)buf;
      zi->ci.bstream.avail_in = len;
      err = BZ_RUN_OK;

      जबतक ((err==BZ_RUN_OK) && (zi->ci.bstream.avail_in>0))
      {
        अगर (zi->ci.bstream.avail_out == 0)
        {
          अगर (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.अग्र_out = (अक्षर*)zi->ci.buffered_तथ्य;
        }


        अगर(err != BZ_RUN_OK)
          अवरोध;

        अगर ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_योग_Out_Before_lo = zi->ci.bstream.योग_out_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_RUN);

          zi->ci.pos_in_buffered_तथ्य += (uInt)(zi->ci.bstream.योग_out_lo32 - u_योग_Out_Before_lo) ;
        }
      }

      अगर(err == BZ_RUN_OK)
        err = ZIP_OK;
    }
    अन्यथा
#पूर्ण_अगर
    {
      zi->ci.stream.अग्र_in = buf;
      zi->ci.stream.avail_in = len;

      जबतक ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
      {
          अगर (zi->ci.stream.avail_out == 0)
          {
              अगर (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
              zi->ci.stream.अग्र_out = zi->ci.buffered_तथ्य;
          }


          अगर(err != ZIP_OK)
              अवरोध;

          अगर ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_योग_Out_Before = zi->ci.stream.योग_out;
              err=deflate(&zi->ci.stream,  Z_NO_FLUSH);

              zi->ci.pos_in_buffered_तथ्य += (uInt)(zi->ci.stream.योग_out - u_योग_Out_Before) ;
          }
          अन्यथा
          {
              uInt copy_this,i;
              अगर (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                  copy_this = zi->ci.stream.avail_in;
              अन्यथा
                  copy_this = zi->ci.stream.avail_out;

              क्रम (i = 0; i < copy_this; i++)
                  *(((अक्षर*)zi->ci.stream.अग्र_out)+i) =
                      *(((स्थिर अक्षर*)zi->ci.stream.अग्र_in)+i);
              {
                  zi->ci.stream.avail_in -= copy_this;
                  zi->ci.stream.avail_out-= copy_this;
                  zi->ci.stream.अग्र_in+= copy_this;
                  zi->ci.stream.अग्र_out+= copy_this;
                  zi->ci.stream.योग_in+= copy_this;
                  zi->ci.stream.योग_out+= copy_this;
                  zi->ci.pos_in_buffered_तथ्य += copy_this;
              }
          }
      }/* while(...) */
    }

    वापस err;
}

बाह्य पूर्णांक ZEXPORT zipCloseFileInZipRaw(zipFile file, uLong uncompressed_आकार, uLong crc32) {
    वापस zipCloseFileInZipRaw64 (file, uncompressed_आकार, crc32);
}

बाह्य पूर्णांक ZEXPORT zipCloseFileInZipRaw64(zipFile file, ZPOS64_T uncompressed_आकार, uLong crc32) {
    zip64_internal* zi;
    ZPOS64_T compressed_आकार;
    uLong invalid_मूल्य = 0xffffffff;
    अचिन्हित datasize = 0;
    पूर्णांक err=ZIP_OK;

    अगर (file == शून्य)
        वापस ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    अगर (zi->in_opened_file_inzip == 0)
        वापस ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    अगर ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
                {
                        जबतक (err==ZIP_OK)
                        {
                                uLong u_योग_Out_Before;
                                अगर (zi->ci.stream.avail_out == 0)
                                {
                                        अगर (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                                        zi->ci.stream.अग्र_out = zi->ci.buffered_तथ्य;
                                }
                                u_योग_Out_Before = zi->ci.stream.योग_out;
                                err=deflate(&zi->ci.stream,  Z_FINISH);
                                zi->ci.pos_in_buffered_तथ्य += (uInt)(zi->ci.stream.योग_out - u_योग_Out_Before) ;
                        }
                }
    अन्यथा अगर ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
#अगर_घोषित HAVE_BZIP2
      err = BZ_FINISH_OK;
      जबतक (err==BZ_FINISH_OK)
      {
        uLong u_योग_Out_Before;
        अगर (zi->ci.bstream.avail_out == 0)
        {
          अगर (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.अग्र_out = (अक्षर*)zi->ci.buffered_तथ्य;
        }
        u_योग_Out_Before = zi->ci.bstream.योग_out_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        अगर(err == BZ_STREAM_END)
          err = Z_STREAM_END;

        zi->ci.pos_in_buffered_तथ्य += (uInt)(zi->ci.bstream.योग_out_lo32 - u_योग_Out_Before);
      }

      अगर(err == BZ_FINISH_OK)
        err = ZIP_OK;
#पूर्ण_अगर
    }

    अगर (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    अगर ((zi->ci.pos_in_buffered_तथ्य>0) && (err==ZIP_OK))
                {
        अगर (zip64_Flush_Write_बफर(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    अगर ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
    {
        पूर्णांक tmp_err = deflateEnd(&zi->ci.stream);
        अगर (err == ZIP_OK)
            err = tmp_err;
        zi->ci.stream_initialised = 0;
    }
#अगर_घोषित HAVE_BZIP2
    अन्यथा अगर((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
      पूर्णांक tmperr = BZ2_bzCompressEnd(&zi->ci.bstream);
                        अगर (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.stream_initialised = 0;
    }
#पूर्ण_अगर

    अगर (!zi->ci.raw)
    {
        crc32 = (uLong)zi->ci.crc32;
        uncompressed_आकार = zi->ci.योग_Uncompressed_तथ्य;
    }
    compressed_आकार = zi->ci.योग_Compressed_तथ्य;

#    ifndef NOCRYPT
    compressed_आकार += zi->ci.crypt_header_आकार;
#    endif

    /* update Current Item crc and sizes, */
    अगर(compressed_आकार >= 0xffffffff || uncompressed_आकार >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+6,(uLong)45,2);

    }

    zip64local_put_मूल्य_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


    अगर(compressed_आकार >= 0xffffffff)
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+20, invalid_मूल्य,4); /*compr size*/
    अन्यथा
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+20, compressed_आकार,4); /*compr size*/

    /* set internal file attributes field */
    अगर (zi->ci.stream.तथ्य_type == Z_ASCII)
        zip64local_put_मूल्य_inmemory(zi->ci.central_header+36,(uLong)Z_ASCII,2);

    अगर(uncompressed_आकार >= 0xffffffff)
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+24, invalid_मूल्य,4); /*uncompr size*/
    अन्यथा
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+24, uncompressed_आकार,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    अगर(uncompressed_आकार >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    अगर(compressed_आकार >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    अगर(zi->ci.pos_local_header >= 0xffffffff)
      datasize += 8;

    अगर(datasize > 0)
    {
      अक्षर* p = शून्य;

      अगर((uLong)(datasize + 4) > zi->ci.आकार_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        वापस ZIP_BADZIPFILE;
      }

      p = zi->ci.central_header + zi->ci.आकार_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_मूल्य_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_मूल्य_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      अगर(uncompressed_आकार >= 0xffffffff)
      {
        zip64local_put_मूल्य_inmemory(p, uncompressed_आकार, 8);
        p += 8;
      }

      अगर(compressed_आकार >= 0xffffffff)
      {
        zip64local_put_मूल्य_inmemory(p, compressed_आकार, 8);
        p += 8;
      }

      अगर(zi->ci.pos_local_header >= 0xffffffff)
      {
        zip64local_put_मूल्य_inmemory(p, zi->ci.pos_local_header, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.आकार_central_Extra_Free -= datasize + 4;
      zi->ci.आकार_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.आकार_central_Extra += datasize + 4;
      zip64local_put_मूल्य_inmemory(zi->ci.central_header+30,(uLong)zi->ci.आकार_central_Extra,2);
    }

    अगर (err==ZIP_OK)
        err = add_तथ्य_in_datablock(&zi->central_dir, zi->ci.central_header, (uLong)zi->ci.आकार_centralheader);

    मुक्त(zi->ci.central_header);

    अगर (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        अगर (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;

        अगर (err==ZIP_OK)
            err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

        अगर(uncompressed_आकार >= 0xffffffff || compressed_आकार >= 0xffffffff )
        {
          अगर(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            अगर (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET)!=0)
              err = ZIP_ERRNO;

            अगर (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, uncompressed_आकार, 8);

            अगर (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, compressed_आकार, 8);
          }
          अन्यथा
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        अन्यथा
        {
          अगर (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,compressed_आकार,4);

          अगर (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,uncompressed_आकार,4);
        }

        अगर (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;
    }

    zi->संख्या_entry ++;
    zi->in_opened_file_inzip = 0;

    वापस err;
}

बाह्य पूर्णांक ZEXPORT zipCloseFileInZip(zipFile file) {
    वापस zipCloseFileInZipRaw (file,0,0);
}

local पूर्णांक Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, ZPOS64_T zip64eocd_pos_inzip) {
  पूर्णांक err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->add_position_when_writing_offset;

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    अगर (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    अगर (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    अगर (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    वापस err;
}

local पूर्णांक Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uLong आकार_centraldir, ZPOS64_T centraldir_pos_inzip) {
  पूर्णांक err = ZIP_OK;

  uLong Zip64_तथ्य_आकार = 44;

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  अगर (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_तथ्य_आकार,8); /* why ZPOS64_T of this ? */

  अगर (err==ZIP_OK) /* version made by */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  अगर (err==ZIP_OK) /* version needed */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  अगर (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  अगर (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  अगर (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, zi->संख्या_entry, 8);

  अगर (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_मूल्य(&zi->z_filefunc, zi->filestream, zi->संख्या_entry, 8);

  अगर (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(ZPOS64_T)आकार_centraldir,8);

  अगर (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  वापस err;
}

local पूर्णांक Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uLong आकार_centraldir, ZPOS64_T centraldir_pos_inzip) {
  पूर्णांक err = ZIP_OK;

  /*signature*/
  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  अगर (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  अगर (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  अगर (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      अगर(zi->संख्या_entry >= 0xFFFF)
        err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      अन्यथा
        err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->संख्या_entry,2);
    }
  }

  अगर (err==ZIP_OK) /* total number of entries in the central dir */
  {
    अगर(zi->संख्या_entry >= 0xFFFF)
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)zi->संख्या_entry,2);
  }

  अगर (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)आकार_centraldir,4);

  अगर (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    अगर(pos >= 0xffffffff)
    {
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    अन्यथा
      err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->add_position_when_writing_offset),4);
  }

   वापस err;
}

local पूर्णांक Write_GlobalComment(zip64_internal* zi, स्थिर अक्षर* global_comment) {
  पूर्णांक err = ZIP_OK;
  uInt आकार_global_comment = 0;

  अगर(global_comment != शून्य)
    आकार_global_comment = (uInt)म_माप(global_comment);

  err = zip64local_put_मूल्य(&zi->z_filefunc,zi->filestream,(uLong)आकार_global_comment,2);

  अगर (err == ZIP_OK && आकार_global_comment > 0)
  {
    अगर (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, आकार_global_comment) != आकार_global_comment)
      err = ZIP_ERRNO;
  }
  वापस err;
}

बाह्य पूर्णांक ZEXPORT zipClose(zipFile file, स्थिर अक्षर* global_comment) {
    zip64_internal* zi;
    पूर्णांक err = 0;
    uLong आकार_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    अगर (file == शून्य)
        वापस ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    अगर (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

#अगर_अघोषित NO_ADDFILEINEXISTINGZIP
    अगर (global_comment==शून्य)
        global_comment = zi->globalcomment;
#पूर्ण_अगर

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    अगर (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
        जबतक (ldi!=शून्य)
        {
            अगर ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
            {
                अगर (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->तथ्य, ldi->filled_in_this_block) != ldi->filled_in_this_block)
                    err = ZIP_ERRNO;
            }

            आकार_centraldir += ldi->filled_in_this_block;
            ldi = ldi->अग्र_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    set_end(&zi->set);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    अगर(pos >= 0xffffffff || zi->संख्या_entry >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64EndOfCentralDirectoryRecord(zi, आकार_centraldir, centraldir_pos_inzip);

      Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
    }

    अगर (err==ZIP_OK)
      err = Write_EndOfCentralDirectoryRecord(zi, आकार_centraldir, centraldir_pos_inzip);

    अगर(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    अगर (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        अगर (err == ZIP_OK)
            err = ZIP_ERRNO;

#अगर_अघोषित NO_ADDFILEINEXISTINGZIP
    मुक्त(zi->globalcomment);
#पूर्ण_अगर
    मुक्त(zi);

    वापस err;
}

बाह्य पूर्णांक ZEXPORT zipRemoveExtraInfoBlock(अक्षर* p_तथ्य, पूर्णांक* तथ्य_Len, लघु sHeader) {
  अक्षर* p = p_तथ्य;
  पूर्णांक आकार = 0;
  अक्षर* pNewHeader;
  अक्षर* pTmp;
  लघु header;
  लघु तथ्य_आकार;

  पूर्णांक retVal = ZIP_OK;

  अगर(p_तथ्य == शून्य || तथ्य_Len == शून्य || *तथ्य_Len < 4)
    वापस ZIP_PARAMERROR;

  pNewHeader = (अक्षर*)ALLOC((अचिन्हित)*तथ्य_Len);
  pTmp = pNewHeader;

  जबतक(p < (p_तथ्य + *तथ्य_Len))
  {
    header = *(लघु*)p;
    तथ्य_आकार = *(((लघु*)p)+1);

    अगर( header == sHeader ) /* Header found. */
    {
      p += तथ्य_आकार + 4; /* skip it. do not copy to temp buffer */
    }
    अन्यथा
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      स_नकल(pTmp, p, तथ्य_आकार + 4);
      p += तथ्य_आकार + 4;
      आकार += तथ्य_आकार + 4;
    }

  }

  अगर(आकार < *तथ्य_Len)
  {
    /* clean old extra info block. */
    स_रखो(p_तथ्य,0, *तथ्य_Len);

    /* copy the new extra info block over the old */
    अगर(आकार > 0)
      स_नकल(p_तथ्य, pNewHeader, आकार);

    /* set the new extra info size */
    *तथ्य_Len = आकार;

    retVal = ZIP_OK;
  }
  अन्यथा
    retVal = ZIP_ERRNO;

  मुक्त(pNewHeader);

  वापस retVal;
}
