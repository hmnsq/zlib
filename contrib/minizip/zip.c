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


#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZLIB_CONST
#  define ZLIB_CONST
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DLL
#  undef ZLIB_DLL
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "zip.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ STDC
#  include <stddef.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_ERRNO_H
    ਬਾਹਰੀ ਪੂਰਨ errno;
#ਨਹੀਂਤਾਂ
#   include <errno.h>
#ਜੇ_ਅੰਤ


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ local
#  define local ਸਥਿਰ
#ਜੇ_ਅੰਤ
/* compile with -Dlocal if your debugger can't find static symbols */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_BUFSIZE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_BUFSIZE (64*1024) /* (16384) */
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_MAXFILENAMEINZIP
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_MAXFILENAMEINZIP (256)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ALLOC
# define ALLOC(ਮਾਪ) (ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ))
#ਜੇ_ਅੰਤ

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAKEULONG64(a, b) ((ZPOS64_T)(((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(a)) | ((ZPOS64_T)((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(b))) << 32))

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SEEK_CUR
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEEK_CUR    1
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SEEK_END
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEEK_END    2
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SEEK_SET
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SEEK_SET    0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEF_MEM_LEVEL
#ਜੇ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ਨਹੀਂਤਾਂ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
ਅਚਲ ਅੱਖਰ zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIZEDATA_INDATABLOCK (4096-(4*4))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOCALHEADERMAGIC    (0x04034b50)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CENTRALHEADERMAGIC  (0x02014b50)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENDHEADERMAGIC      (0x06054b50)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP64ENDHEADERMAGIC      (0x6064b50)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_LOCALHEADER_ਵਿੱਥ (0x06)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਚੱਕਰੀ_ਜਾਂਚ_LOCALHEADER_ਵਿੱਥ  (0x0e)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIZECENTRALHEADER (0x2e) /* 46 */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ linkedlist_datablock_ਅੰਦਰੂਨੀ_s
{
  ਬਣਤਰ linkedlist_datablock_ਅੰਦਰੂਨੀ_s* ਅਗਲਾ_datablock;
  uLong  avail_ਵਿੱਚ_this_ਖੰਡ;
  uLong  filled_ਵਿੱਚ_this_ਖੰਡ;
  uLong  ਅਣਵਰਤਿਆ; /* for future use and alignment */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅੰਕੜੇ[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_ਅੰਦਰੂਨੀ;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ linkedlist_ਅੰਕੜੇ_s
{
    linkedlist_datablock_ਅੰਦਰੂਨੀ* ਪਹਿਲਾ_ਖੰਡ;
    linkedlist_datablock_ਅੰਦਰੂਨੀ* ਆਖਰੀ_ਖੰਡ;
} linkedlist_ਅੰਕੜੇ;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SKIPSET_EXPORT ਸਥਿਰ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅੱਖਰ *ਸੈੱਟ_ਕੁੰਜੀ_t;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_cmp(a, b) strcmp(a, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_drop(s, k) ਸੈੱਟ_free(s, k)
#ਅਨ੍ਤਰ੍ਭਾਵ "skipset.h"
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅਗਲਾ;        /* next byte in datablock data */
    size_t ਖੱਬਾ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_ਅੰਦਰੂਨੀ *ਗੰਢ;    /* current datablock */
} ਖੰਡ_t;


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
    z_ਧਾਰਾ ਧਾਰਾ;            /* zLib stream structure for inflate */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    bz_ਧਾਰਾ bstream;          /* bzLib stream structure for bziped */
#ਜੇ_ਅੰਤ

    ਪੂਰਨ  ਧਾਰਾ_initialised;    /* 1 is stream is initialised */
    uInt pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_ਸਿਰਲੇਖ;     /* offset of the local header of the file
                                     currently writing */
    ਅੱਖਰ* central_ਸਿਰਲੇਖ;       /* central header data for the current file */
    uLong ਮਾਪ_central_Extra;
    uLong ਮਾਪ_centralheader;   /* size of the central header for cur file */
    uLong ਮਾਪ_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    ਪੂਰਨ  ਵਿਧੀ_2;                /* compression method of file currently wr.*/
    ਪੂਰਨ  raw;                   /* 1 for directly writing raw data */
    Byte buffered_ਅੰਕੜੇ[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong ਚੱਕਰੀ_ਜਾਂਚ_32_2;
    ਪੂਰਨ  encrypt;
    ਪੂਰਨ  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T ਕੁੱਲ_Compressed_ਅੰਕੜੇ;
    ZPOS64_T ਕੁੱਲ_Uncompressed_ਅੰਕੜੇ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NOCRYPT
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੰਜੀਆਂ[3];     /* keys defining the pseudo-random sequence */
    ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t* pcrc_32_tab;
    ਬਿਨਾਨਿਸ਼ਾਨੀ crypt_ਸਿਰਲੇਖ_ਮਾਪ;
#ਜੇ_ਅੰਤ
} curfile64_info;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_ਅੰਕੜੇ central_dir;/* datablock with central dir in construction*/
    ਪੂਰਨ  ਵਿੱਚ_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T ਜੋੜੋ_position_when_writing_ਵਿੱਥ;
    ZPOS64_T ਸੰਖਿਆ_ਇੰਦਰਾਜ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_ADDFILEINEXISTINGZIP
    ਅੱਖਰ *globalcomment;
#ਜੇ_ਅੰਤ

    /* Support for zipAlreadyThere(). */
    ਸੈੱਟ_t ਸੈੱਟ;              /* set for detecting name collisions */
    ਖੰਡ_t ਖੰਡ;          /* block for reading the central directory */

} zip64_ਅੰਦਰੂਨੀ;


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NOCRYPT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#ਅਨ੍ਤਰ੍ਭਾਵ "crypt.h"
#ਜੇ_ਅੰਤ

local linkedlist_datablock_ਅੰਦਰੂਨੀ* allocate_new_datablock(ਖਾਲੀ) {
    linkedlist_datablock_ਅੰਦਰੂਨੀ* ldi;
    ldi = (linkedlist_datablock_ਅੰਦਰੂਨੀ*)
                 ALLOC(ਆਕਾਰ(linkedlist_datablock_ਅੰਦਰੂਨੀ));
    ਜੇ (ldi!=ਨਿਰਾ)
    {
        ldi->ਅਗਲਾ_datablock = ਨਿਰਾ ;
        ldi->filled_ਵਿੱਚ_this_ਖੰਡ = 0 ;
        ldi->avail_ਵਿੱਚ_this_ਖੰਡ = SIZEDATA_INDATABLOCK ;
    }
    ਵਾਪਸ ldi;
}

local ਖਾਲੀ free_datablock(linkedlist_datablock_ਅੰਦਰੂਨੀ* ldi) {
    ਜਦੋਂ (ldi!=ਨਿਰਾ)
    {
        linkedlist_datablock_ਅੰਦਰੂਨੀ* ldinext = ldi->ਅਗਲਾ_datablock;
        ਮੁਕਤ(ldi);
        ldi = ldinext;
    }
}

local ਖਾਲੀ init_linkedlist(linkedlist_ਅੰਕੜੇ* ll) {
    ll->ਪਹਿਲਾ_ਖੰਡ = ll->ਆਖਰੀ_ਖੰਡ = ਨਿਰਾ;
}

local ਖਾਲੀ free_linkedlist(linkedlist_ਅੰਕੜੇ* ll) {
    free_datablock(ll->ਪਹਿਲਾ_ਖੰਡ);
    ll->ਪਹਿਲਾ_ਖੰਡ = ll->ਆਖਰੀ_ਖੰਡ = ਨਿਰਾ;
}


local ਪੂਰਨ ਜੋੜੋ_ਅੰਕੜੇ_ਵਿੱਚ_datablock(linkedlist_ਅੰਕੜੇ* ll, ਅਚਲ ਖਾਲੀ* buf, uLong len) {
    linkedlist_datablock_ਅੰਦਰੂਨੀ* ldi;
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* from_copy;

    ਜੇ (ll==ਨਿਰਾ)
        ਵਾਪਸ ZIP_INTERNALERROR;

    ਜੇ (ll->ਆਖਰੀ_ਖੰਡ == ਨਿਰਾ)
    {
        ll->ਪਹਿਲਾ_ਖੰਡ = ll->ਆਖਰੀ_ਖੰਡ = allocate_new_datablock();
        ਜੇ (ll->ਪਹਿਲਾ_ਖੰਡ == ਨਿਰਾ)
            ਵਾਪਸ ZIP_INTERNALERROR;
    }

    ldi = ll->ਆਖਰੀ_ਖੰਡ;
    from_copy = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)buf;

    ਜਦੋਂ (len>0)
    {
        uInt copy_this;
        uInt i;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* ਮੰਜ਼ਿਲ_copy;

        ਜੇ (ldi->avail_ਵਿੱਚ_this_ਖੰਡ==0)
        {
            ldi->ਅਗਲਾ_datablock = allocate_new_datablock();
            ਜੇ (ldi->ਅਗਲਾ_datablock == ਨਿਰਾ)
                ਵਾਪਸ ZIP_INTERNALERROR;
            ldi = ldi->ਅਗਲਾ_datablock ;
            ll->ਆਖਰੀ_ਖੰਡ = ldi;
        }

        ਜੇ (ldi->avail_ਵਿੱਚ_this_ਖੰਡ < len)
            copy_this = (uInt)ldi->avail_ਵਿੱਚ_this_ਖੰਡ;
        ਨਹੀਂਤਾਂ
            copy_this = (uInt)len;

        ਮੰਜ਼ਿਲ_copy = &(ldi->ਅੰਕੜੇ[ldi->filled_ਵਿੱਚ_this_ਖੰਡ]);

        ਲਈ (i=0;i<copy_this;i++)
            *(ਮੰਜ਼ਿਲ_copy+i)=*(from_copy+i);

        ldi->filled_ਵਿੱਚ_this_ਖੰਡ += copy_this;
        ldi->avail_ਵਿੱਚ_this_ਖੰਡ -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    ਵਾਪਸ ZIP_OK;
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
local ਖਾਲੀ ਖੰਡ_init(ਖੰਡ_t *ਖੰਡ, linkedlist_ਅੰਕੜੇ *list) {
    ਖੰਡ->ਗੰਢ = list->ਪਹਿਲਾ_ਖੰਡ;
    ਖੰਡ->ਅਗਲਾ = ਖੰਡ->ਗੰਢ->ਅੰਕੜੇ;
    ਖੰਡ->ਖੱਬਾ = ਖੰਡ->ਗੰਢ->filled_ਵਿੱਚ_this_ਖੰਡ;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local ਖਾਲੀ ਖੰਡ_stop(ਖੰਡ_t *ਖੰਡ) {
    ਖੰਡ->ਖੱਬਾ = 0;
    ਖੰਡ->ਅਗਲਾ = ਨਿਰਾ;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local ਪੂਰਨ ਖੰਡ_ਸਿਰਾ(ਖੰਡ_t *ਖੰਡ) {
    linkedlist_datablock_ਅੰਦਰੂਨੀ *ਗੰਢ = ਖੰਡ->ਗੰਢ;
    ਜੇ (ਗੰਢ == ਨਿਰਾ)
        /* This block was previously terminated with extreme prejudice. */
        ਵਾਪਸ 1;
    ਜੇ (ਖੰਡ->ਅਗਲਾ < ਗੰਢ->ਅੰਕੜੇ + ਗੰਢ->filled_ਵਿੱਚ_this_ਖੰਡ)
        /* There are more bytes to read in the current datablock. */
        ਵਾਪਸ 0;
    ਜਦੋਂ (ਗੰਢ->ਅਗਲਾ_datablock != ਨਿਰਾ) {
        ਜੇ (ਗੰਢ->filled_ਵਿੱਚ_this_ਖੰਡ != 0)
            /* There are some bytes in a later datablock. */
            ਵਾਪਸ 0;
        ਗੰਢ = ਗੰਢ->ਅਗਲਾ_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    ਵਾਪਸ 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local ਪੂਰਨ ਖੰਡ_get(ਖੰਡ_t *ਖੰਡ) {
    ਜਦੋਂ (ਖੰਡ->ਖੱਬਾ == 0) {
        ਜੇ (ਖੰਡ->ਗੰਢ == ਨਿਰਾ)
            /* We've been marked bad. Return end. */
            ਵਾਪਸ -1;
        /* Update left in case more was filled in since we were last here. */
        ਖੰਡ->ਖੱਬਾ = ਖੰਡ->ਗੰਢ->filled_ਵਿੱਚ_this_ਖੰਡ -
                      (size_t)(ਖੰਡ->ਅਗਲਾ - ਖੰਡ->ਗੰਢ->ਅੰਕੜੇ);
        ਜੇ (ਖੰਡ->ਖੱਬਾ != 0)
            /* There was indeed more data appended in the current datablock. */
            ਰੋਕੋ;
        ਜੇ (ਖੰਡ->ਗੰਢ->ਅਗਲਾ_datablock == ਨਿਰਾ)
            /* No more data here, and there is no next datablock. At the end. */
            ਵਾਪਸ -1;
        /* Try the next datablock for more data. */
        ਖੰਡ->ਗੰਢ = ਖੰਡ->ਗੰਢ->ਅਗਲਾ_datablock;
        ਖੰਡ->ਅਗਲਾ = ਖੰਡ->ਗੰਢ->ਅੰਕੜੇ;
        ਖੰਡ->ਖੱਬਾ = ਖੰਡ->ਗੰਢ->filled_ਵਿੱਚ_this_ਖੰਡ;
    }
    /* We have a byte to return. */
    ਖੰਡ->ਖੱਬਾ--;
    ਵਾਪਸ *ਖੰਡ->ਅਗਲਾ++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local ਲੰਮਾ ਖੰਡ_get2(ਖੰਡ_t *ਖੰਡ) {
    ਪੂਰਨ ਨੀਵਾਂ = ਖੰਡ_get(ਖੰਡ);
    ਪੂਰਨ ਉੱਚਾ = ਖੰਡ_get(ਖੰਡ);
    ਵਾਪਸ ਨੀਵਾਂ < 0 || ਉੱਚਾ < 0 ? -1 : ਨੀਵਾਂ | ((ਲੰਮਾ)ਉੱਚਾ << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t ਖੰਡ_read(ਖੰਡ_t *ਖੰਡ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, size_t len) {
    size_t need = len;
    size_t take;
    ਜਦੋਂ (need) {
        ਜੇ (ਖੰਡ->ਖੱਬਾ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            ਪੂਰਨ got = ਖੰਡ_get(ਖੰਡ);
            ਜੇ (got == -1)
                /* Reached the end. */
                ਰੋਕੋ;
            *buf++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)got;
            need--;
            ਜਾਰੀ;
        }
        take = need > ਖੰਡ->ਖੱਬਾ ? ਖੰਡ->ਖੱਬਾ : need;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, ਖੰਡ->ਅਗਲਾ, take);
        ਖੰਡ->ਅਗਲਾ += take;
        ਖੰਡ->ਖੱਬਾ -= take;
        buf += take;
        need -= take;
    }
    ਵਾਪਸ len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local ਪੂਰਨ ਖੰਡ_skip(ਖੰਡ_t *ਖੰਡ, size_t n) {
    ਜਦੋਂ (n > ਖੰਡ->ਖੱਬਾ) {
        n -= ਖੰਡ->ਖੱਬਾ;
        ਖੰਡ->ਅਗਲਾ += ਖੰਡ->ਖੱਬਾ;
        ਖੰਡ->ਖੱਬਾ = 0;
        ਜੇ (ਖੰਡ_get(ਖੰਡ) == -1)
            ਵਾਪਸ -1;
        n--;
    }
    ਖੰਡ->ਅਗਲਾ += n;
    ਖੰਡ->ਖੱਬਾ -= n;
    ਵਾਪਸ 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local ਅੱਖਰ *ਖੰਡ_central_name(ਖੰਡ_t *ਖੰਡ, ਸੈੱਟ_t *ਸੈੱਟ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ flen, xlen, clen;
    ਅੱਖਰ *name = ਨਿਰਾ;
    ਲਈ (;;) {
        ਜੇ (ਖੰਡ_ਸਿਰਾ(ਖੰਡ))
            /* At the end of the central directory (so far). */
            ਵਾਪਸ ਨਿਰਾ;

        /* Check for a central directory record signature. */
        ਜੇ (ਖੰਡ_get2(ਖੰਡ) != (CENTRALHEADERMAGIC & 0xffff) ||
            ਖੰਡ_get2(ਖੰਡ) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            ਰੋਕੋ;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        ਖੰਡ_skip(ਖੰਡ, 24);
        flen = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੰਡ_get2(ਖੰਡ);    /* file name length */
        xlen = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੰਡ_get2(ਖੰਡ);    /* extra length */
        clen = (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੰਡ_get2(ਖੰਡ);    /* comment length */
        ਜੇ (ਖੰਡ_skip(ਖੰਡ, 12) == -1)
            /* Premature end of the record. */
            ਰੋਕੋ;

        /* Extract the name and skip over the extra and comment fields. */
        name = ਸੈੱਟ_alloc(ਸੈੱਟ, ਨਿਰਾ, flen + 1);
        ਜੇ (ਖੰਡ_read(ਖੰਡ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)name, flen) < flen ||
            ਖੰਡ_skip(ਖੰਡ, xlen + clen) == -1)
            /* Premature end of the record. */
            ਰੋਕੋ;

        /* Check for embedded nuls in the name. */
        ਜੇ (memchr(name, 0, flen) != ਨਿਰਾ) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            ਸੈੱਟ_free(ਸੈੱਟ, name);
            ਜਾਰੀ;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        ਵਾਪਸ name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    ਸੈੱਟ_free(ਸੈੱਟ, name);
    ਖੰਡ_stop(ਖੰਡ);
    ਵਾਪਸ ਨਿਰਾ;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zipAlreadyThere(zipFile file, ਅੱਖਰ ਅਚਲ *name) {
    size_t len;
    ਅੱਖਰ *copy;
    ਪੂਰਨ ਮਿਲਿਆ;
    zip64_ਅੰਦਰੂਨੀ *zip = file;
    ਜੇ (zip == ਨਿਰਾ)
        ਵਾਪਸ ZIP_PARAMERROR;
    ਜੇ (zip->central_dir.ਪਹਿਲਾ_ਖੰਡ == ਨਿਰਾ)
        /* No central directory yet, so no, name isn't there. */
        ਵਾਪਸ 0;
    ਜੇ (setjmp(zip->ਸੈੱਟ.env)) {
        /* Memory allocation failure. */
        ਸੈੱਟ_ਸਿਰਾ(&zip->ਸੈੱਟ);
        ਵਾਪਸ -2;
    }
    ਜੇ (!ਸੈੱਟ_ok(&zip->ਸੈੱਟ)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        ਸੈੱਟ_ਸ਼ੁਰੂ(&zip->ਸੈੱਟ);
        ਖੰਡ_init(&zip->ਖੰਡ, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    ਲਈ (;;) {
        ਅੱਖਰ *there = ਖੰਡ_central_name(&zip->ਖੰਡ, &zip->ਸੈੱਟ);
        ਜੇ (there == ਨਿਰਾ) {
            ਜੇ (zip->ਖੰਡ.ਅਗਲਾ == ਨਿਰਾ)
                /* The central directory is invalid. */
                ਵਾਪਸ -1;
            ਰੋਕੋ;
        }

        /* Add there to the set. */
        ਜੇ (ਸੈੱਟ_ਪਾਓ(&zip->ਸੈੱਟ, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            ਸੈੱਟ_free(&zip->ਸੈੱਟ, there);
    }

    /* Return true if name is in the central directory. */
    len = ਲੜੀ_ਲੰਬਾਈ(name);
    copy = ਸੈੱਟ_alloc(&zip->ਸੈੱਟ, ਨਿਰਾ, len + 1);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(copy, name, len + 1);
    ਮਿਲਿਆ = ਸੈੱਟ_ਮਿਲਿਆ(&zip->ਸੈੱਟ, copy);
    ਸੈੱਟ_free(&zip->ਸੈੱਟ, copy);
    ਵਾਪਸ ਮਿਲਿਆ;
}


/****************************************************************************/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local ਪੂਰਨ zip64local_put_ਮੁੱਲ(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, ਪੂਰਨ nbByte) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[8];
    ਪੂਰਨ n;
    ਲਈ (n = 0; n < nbByte; n++)
    {
        buf[n] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x & 0xff);
        x >>= 8;
    }
    ਜੇ (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      ਲਈ (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    ਜੇ (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        ਵਾਪਸ ZIP_ERRNO;
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ ZIP_OK;
}

local ਖਾਲੀ zip64local_put_ਮੁੱਲ_inmemory (ਖਾਲੀ* dest, ZPOS64_T x, ਪੂਰਨ nbByte) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* buf=(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)dest;
    ਪੂਰਨ n;
    ਲਈ (n = 0; n < nbByte; n++) {
        buf[n] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x & 0xff);
        x >>= 8;
    }

    ਜੇ (x != 0)
    {     /* data overflow - hack for ZIP64 */
       ਲਈ (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_Tmz_Date_ਮੰਜ਼ਿਲ_Dos_Date(ਅਚਲ tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    ਜੇ (year>=1980)
        year-=1980;
    ਨਹੀਂਤਾਂ ਜੇ (year>=80)
        year-=80;
    ਵਾਪਸ
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local ਪੂਰਨ zip64local_getByte(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ਪੂਰਨ* pi) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c;
    ਪੂਰਨ err = (ਪੂਰਨ)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    ਜੇ (err==1)
    {
        *pi = (ਪੂਰਨ)c;
        ਵਾਪਸ ZIP_OK;
    }
    ਨਹੀਂਤਾਂ
    {
        ਜੇ (ZERROR64(*pzlib_filefunc_def,filestream))
            ਵਾਪਸ ZIP_ERRNO;
        ਨਹੀਂਤਾਂ
            ਵਾਪਸ ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local ਪੂਰਨ zip64local_getShort(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    ਪੂਰਨ i = 0;
    ਪੂਰਨ err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ਜੇ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ਜੇ (err==ZIP_OK)
        *pX = x;
    ਨਹੀਂਤਾਂ
        *pX = 0;
    ਵਾਪਸ err;
}

local ਪੂਰਨ zip64local_getLong(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    ਪੂਰਨ i = 0;
    ਪੂਰਨ err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ਜੇ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ਜੇ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    ਜੇ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    ਜੇ (err==ZIP_OK)
        *pX = x;
    ਨਹੀਂਤਾਂ
        *pX = 0;
    ਵਾਪਸ err;
}


local ਪੂਰਨ zip64local_getLong64(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  ਪੂਰਨ i = 0;
  ਪੂਰਨ err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  ਜੇ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  ਜੇ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  ਜੇ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  ਜੇ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  ਜੇ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  ਜੇ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  ਜੇ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  ਜੇ (err==ZIP_OK)
    *pX = x;
  ਨਹੀਂਤਾਂ
    *pX = 0;

  ਵਾਪਸ err;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ BUFREADCOMMENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFREADCOMMENT (0x400)
#ਜੇ_ਅੰਤ
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_ਖੋਜ_Central_Dir(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* buf;
  ZPOS64_T u_ਮਾਪ_File;
  ZPOS64_T u_ਪਿੱਠ_Read;
  ZPOS64_T u_Max_ਪਿੱਠ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_ਮਿਲਿਆ=0;

  ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ਸਿਰਾ) != 0)
    ਵਾਪਸ 0;


  u_ਮਾਪ_File = ZTELL64(*pzlib_filefunc_def,filestream);

  ਜੇ (u_Max_ਪਿੱਠ>u_ਮਾਪ_File)
    u_Max_ਪਿੱਠ = u_ਮਾਪ_File;

  buf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)ALLOC(BUFREADCOMMENT+4);
  ਜੇ (buf==ਨਿਰਾ)
    ਵਾਪਸ 0;

  u_ਪਿੱਠ_Read = 4;
  ਜਦੋਂ (u_ਪਿੱਠ_Read<u_Max_ਪਿੱਠ)
  {
    uLong u_Read_ਮਾਪ;
    ZPOS64_T uReadPos ;
    ਪੂਰਨ i;
    ਜੇ (u_ਪਿੱਠ_Read+BUFREADCOMMENT>u_Max_ਪਿੱਠ)
      u_ਪਿੱਠ_Read = u_Max_ਪਿੱਠ;
    ਨਹੀਂਤਾਂ
      u_ਪਿੱਠ_Read+=BUFREADCOMMENT;
    uReadPos = u_ਮਾਪ_File-u_ਪਿੱਠ_Read ;

    u_Read_ਮਾਪ = ((BUFREADCOMMENT+4) < (u_ਮਾਪ_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_ਮਾਪ_File-uReadPos);
    ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
      ਰੋਕੋ;

    ਜੇ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ਮਾਪ)!=u_Read_ਮਾਪ)
      ਰੋਕੋ;

    ਲਈ (i=(ਪੂਰਨ)u_Read_ਮਾਪ-3; (i--)>0;)
      ਜੇ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_ਮਿਲਿਆ = uReadPos+(ਬਿਨਾਨਿਸ਼ਾਨੀ)i;
        ਰੋਕੋ;
      }

    ਜੇ (u_Pos_ਮਿਲਿਆ!=0)
      ਰੋਕੋ;
  }
  ਮੁਕਤ(buf);
  ਵਾਪਸ u_Pos_ਮਿਲਿਆ;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_ਖੋਜ_Central_Dir64(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* buf;
  ZPOS64_T u_ਮਾਪ_File;
  ZPOS64_T u_ਪਿੱਠ_Read;
  ZPOS64_T u_Max_ਪਿੱਠ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_ਮਿਲਿਆ=0;
  uLong uL;
  ZPOS64_T relative_ਵਿੱਥ;

  ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ਸਿਰਾ) != 0)
    ਵਾਪਸ 0;

  u_ਮਾਪ_File = ZTELL64(*pzlib_filefunc_def,filestream);

  ਜੇ (u_Max_ਪਿੱਠ>u_ਮਾਪ_File)
    u_Max_ਪਿੱਠ = u_ਮਾਪ_File;

  buf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)ALLOC(BUFREADCOMMENT+4);
  ਜੇ (buf==ਨਿਰਾ)
    ਵਾਪਸ 0;

  u_ਪਿੱਠ_Read = 4;
  ਜਦੋਂ (u_ਪਿੱਠ_Read<u_Max_ਪਿੱਠ)
  {
    uLong u_Read_ਮਾਪ;
    ZPOS64_T uReadPos;
    ਪੂਰਨ i;
    ਜੇ (u_ਪਿੱਠ_Read+BUFREADCOMMENT>u_Max_ਪਿੱਠ)
      u_ਪਿੱਠ_Read = u_Max_ਪਿੱਠ;
    ਨਹੀਂਤਾਂ
      u_ਪਿੱਠ_Read+=BUFREADCOMMENT;
    uReadPos = u_ਮਾਪ_File-u_ਪਿੱਠ_Read ;

    u_Read_ਮਾਪ = ((BUFREADCOMMENT+4) < (u_ਮਾਪ_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_ਮਾਪ_File-uReadPos);
    ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
      ਰੋਕੋ;

    ਜੇ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ਮਾਪ)!=u_Read_ਮਾਪ)
      ਰੋਕੋ;

    ਲਈ (i=(ਪੂਰਨ)u_Read_ਮਾਪ-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      ਜੇ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_ਮਿਲਿਆ = uReadPos+(ਬਿਨਾਨਿਸ਼ਾਨੀ)i;
        ਰੋਕੋ;
      }
    }

      ਜੇ (u_Pos_ਮਿਲਿਆ!=0)
        ਰੋਕੋ;
  }

  ਮੁਕਤ(buf);
  ਜੇ (u_Pos_ਮਿਲਿਆ == 0)
    ਵਾਪਸ 0;

  /* Zip64 end of central directory locator */
  ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_ਮਿਲਿਆ,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
    ਵਾਪਸ 0;

  /* the signature, already checked */
  ਜੇ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ਵਾਪਸ 0;

  /* number of the disk with the start of the zip64 end of central directory */
  ਜੇ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ਵਾਪਸ 0;
  ਜੇ (uL != 0)
    ਵਾਪਸ 0;

  /* relative offset of the zip64 end of central directory record */
  ਜੇ (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_ਵਿੱਥ)!=ZIP_OK)
    ਵਾਪਸ 0;

  /* total number of disks */
  ਜੇ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ਵਾਪਸ 0;
  ਜੇ (uL != 1)
    ਵਾਪਸ 0;

  /* Goto Zip64 end of central directory record */
  ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream, relative_ਵਿੱਥ,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
    ਵਾਪਸ 0;

  /* the signature */
  ਜੇ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ਵਾਪਸ 0;

  ਜੇ (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    ਵਾਪਸ 0;

  ਵਾਪਸ relative_ਵਿੱਥ;
}

local ਪੂਰਨ ਲੱਦੋ_Central_Directory_ਰਿਕਾਰਡ(zip64_ਅੰਦਰੂਨੀ* pziinit) {
  ਪੂਰਨ err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T ਮਾਪ_central_dir;     /* size of the central directory  */
  ZPOS64_T ਵਿੱਥ_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong ਸੰਖਿਆ_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong ਸੰਖਿਆ_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T ਸੰਖਿਆ_ਇੰਦਰਾਜ;
  ZPOS64_T ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong ਸੰਸਕਰਣ_Made_By;
  uLong ਸੰਸਕਰਣ_Needed;
  uLong ਮਾਪ_comment;

  ਪੂਰਨ has_ZIP64_ਰਿਕਾਰਡ = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_ਖੋਜ_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  ਜੇ(central_pos > 0)
  {
    has_ZIP64_ਰਿਕਾਰਡ = 1;
  }
  ਨਹੀਂਤਾਂ ਜੇ(central_pos == 0)
  {
    central_pos = zip64local_ਖੋਜ_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  ਜੇ(has_ZIP64_ਰਿਕਾਰਡ)
  {
    ZPOS64_T ਮਾਪ_ਸਿਰਾ_ਦਾ_Central_Directory;
    ਜੇ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_ਸੈੱਟ) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ਜੇ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    ਜੇ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &ਮਾਪ_ਸਿਰਾ_ਦਾ_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    ਜੇ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &ਸੰਸਕਰਣ_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    ਜੇ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &ਸੰਸਕਰਣ_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ਜੇ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&ਸੰਖਿਆ_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ਜੇ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&ਸੰਖਿਆ_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    ਜੇ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &ਸੰਖਿਆ_ਇੰਦਰਾਜ)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    ਜੇ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    ਜੇ ((ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD!=ਸੰਖਿਆ_ਇੰਦਰਾਜ) || (ਸੰਖਿਆ_disk_with_CD!=0) || (ਸੰਖਿਆ_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ਜੇ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ਮਾਪ_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    ਜੇ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ਵਿੱਥ_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    ਮਾਪ_comment = 0;
  }
  ਨਹੀਂਤਾਂ
  {
    /* Read End of central Directory info */
    ਜੇ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ਜੇ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ਜੇ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&ਸੰਖਿਆ_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ਜੇ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&ਸੰਖਿਆ_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    ਸੰਖਿਆ_ਇੰਦਰਾਜ = 0;
    ਜੇ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ਨਹੀਂਤਾਂ
      ਸੰਖਿਆ_ਇੰਦਰਾਜ = uL;

    /* total number of entries in the central dir */
    ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD = 0;
    ਜੇ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ਨਹੀਂਤਾਂ
      ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD = uL;

    ਜੇ ((ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD!=ਸੰਖਿਆ_ਇੰਦਰਾਜ) || (ਸੰਖਿਆ_disk_with_CD!=0) || (ਸੰਖਿਆ_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ਮਾਪ_central_dir = 0;
    ਜੇ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ਨਹੀਂਤਾਂ
      ਮਾਪ_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    ਵਿੱਥ_central_dir = 0;
    ਜੇ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ਨਹੀਂਤਾਂ
      ਵਿੱਥ_central_dir = uL;


    /* zipfile global comment length */
    ਜੇ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &ਮਾਪ_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  ਜੇ ((central_pos<ਵਿੱਥ_central_dir+ਮਾਪ_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  ਜੇ (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    ਵਾਪਸ ZIP_ERRNO;
  }

  ਜੇ (ਮਾਪ_comment>0)
  {
    pziinit->globalcomment = (ਅੱਖਰ*)ALLOC(ਮਾਪ_comment+1);
    ਜੇ (pziinit->globalcomment)
    {
      ਮਾਪ_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,ਮਾਪ_comment);
      pziinit->globalcomment[ਮਾਪ_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (ਵਿੱਥ_central_dir+ਮਾਪ_central_dir);
  pziinit->ਜੋੜੋ_position_when_writing_ਵਿੱਥ = byte_before_the_zipfile;

  {
    ZPOS64_T ਮਾਪ_central_dir_ਮੰਜ਼ਿਲ_read = ਮਾਪ_central_dir;
    size_t buf_ਮਾਪ_2 = SIZEDATA_INDATABLOCK;
    ਖਾਲੀ* buf_read = (ਖਾਲੀ*)ALLOC(buf_ਮਾਪ_2);
    ਜੇ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, ਵਿੱਥ_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_ਸੈੱਟ) != 0)
      err=ZIP_ERRNO;

    ਜਦੋਂ ((ਮਾਪ_central_dir_ਮੰਜ਼ਿਲ_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      ਜੇ (read_this > ਮਾਪ_central_dir_ਮੰਜ਼ਿਲ_read)
        read_this = ਮਾਪ_central_dir_ਮੰਜ਼ਿਲ_read;

      ਜੇ (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      ਜੇ (err==ZIP_OK)
        err = ਜੋੜੋ_ਅੰਕੜੇ_ਵਿੱਚ_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      ਮਾਪ_central_dir_ਮੰਜ਼ਿਲ_read-=read_this;
    }
    ਮੁਕਤ(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->ਸੰਖਿਆ_ਇੰਦਰਾਜ = ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD;

  ਜੇ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, ਵਿੱਥ_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ) != 0)
    err=ZIP_ERRNO;

  ਵਾਪਸ err;
}


#ਜੇ_ਅੰਤ /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
ਬਾਹਰੀ zipFile ZEXPORT zipOpen3(ਅਚਲ ਖਾਲੀ *pathname, ਪੂਰਨ append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_ਅੰਦਰੂਨੀ ziinit;
    zip64_ਅੰਦਰੂਨੀ* zi;
    ਪੂਰਨ err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = ਨਿਰਾ;
    ziinit.z_filefunc.ztell32_file = ਨਿਰਾ;
    ਜੇ (pzlib_filefunc64_32_def==ਨਿਰਾ)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    ਨਹੀਂਤਾਂ
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_ਸਥਿਤੀ_CREATE) ?
                  (ZLIB_FILEFUNC_ਬਹੁਲਕ_READ | ZLIB_FILEFUNC_ਬਹੁਲਕ_WRITE | ZLIB_FILEFUNC_ਬਹੁਲਕ_CREATE) :
                    (ZLIB_FILEFUNC_ਬਹੁਲਕ_READ | ZLIB_FILEFUNC_ਬਹੁਲਕ_WRITE | ZLIB_FILEFUNC_ਬਹੁਲਕ_EXISTING));

    ਜੇ (ziinit.filestream == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;

    ਜੇ (append == APPEND_ਸਥਿਤੀ_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.ਵਿੱਚ_opened_file_inzip = 0;
    ziinit.ci.ਧਾਰਾ_initialised = 0;
    ziinit.ਸੰਖਿਆ_ਇੰਦਰਾਜ = 0;
    ziinit.ਜੋੜੋ_position_when_writing_ਵਿੱਥ = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.ਸੈੱਟ, 0, ਆਕਾਰ(ਸੈੱਟ_t));  /* make sure set appears dormant */



    zi = (zip64_ਅੰਦਰੂਨੀ*)ALLOC(ਆਕਾਰ(zip64_ਅੰਦਰੂਨੀ));
    ਜੇ (zi==ਨਿਰਾ)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        ਵਾਪਸ ਨਿਰਾ;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = ਨਿਰਾ;
    ਜੇ (append == APPEND_ਸਥਿਤੀ_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = ਲੱਦੋ_Central_Directory_ਰਿਕਾਰਡ(&ziinit);
    }

    ਜੇ (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    ਜੇ (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        ਮੁਕਤ(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        ਮੁਕਤ(zi);
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਨਹੀਂਤਾਂ
    {
        *zi = ziinit;
        ਵਾਪਸ (zipFile)zi;
    }
}

ਬਾਹਰੀ zipFile ZEXPORT zipOpen2(ਅਚਲ ਅੱਖਰ *pathname, ਪੂਰਨ append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    ਜੇ (pzlib_filefunc32_def != ਨਿਰਾ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        ਵਾਪਸ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ zipOpen3(pathname, append, globalcomment, ਨਿਰਾ);
}

ਬਾਹਰੀ zipFile ZEXPORT zipOpen2_64(ਅਚਲ ਖਾਲੀ *pathname, ਪੂਰਨ append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    ਜੇ (pzlib_filefunc_def != ਨਿਰਾ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ਨਿਰਾ;
        zlib_filefunc64_32_def_fill.ztell32_file = ਨਿਰਾ;
        zlib_filefunc64_32_def_fill.zseek32_file = ਨਿਰਾ;
        ਵਾਪਸ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ zipOpen3(pathname, append, globalcomment, ਨਿਰਾ);
}



ਬਾਹਰੀ zipFile ZEXPORT zipOpen(ਅਚਲ ਅੱਖਰ* pathname, ਪੂਰਨ append) {
    ਵਾਪਸ zipOpen3((ਅਚਲ ਖਾਲੀ*)pathname,append,ਨਿਰਾ,ਨਿਰਾ);
}

ਬਾਹਰੀ zipFile ZEXPORT zipOpen64(ਅਚਲ ਖਾਲੀ* pathname, ਪੂਰਨ append) {
    ਵਾਪਸ zipOpen3(pathname,append,ਨਿਰਾ,ਨਿਰਾ);
}

local ਪੂਰਨ Write_Local_File_ਸਿਰਲੇਖ(zip64_ਅੰਦਰੂਨੀ* zi, ਅਚਲ ਅੱਖਰ* filename, uInt ਮਾਪ_extrafield_local, ਅਚਲ ਖਾਲੀ* extrafield_local) {
  /* write the local header */
  ਪੂਰਨ err;
  uInt ਮਾਪ_filename = (uInt)ਲੜੀ_ਲੰਬਾਈ(filename);
  uInt ਮਾਪ_extrafield = ਮਾਪ_extrafield_local;

  err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  ਜੇ (err==ZIP_OK)
  {
    ਜੇ(zi->ci.zip64)
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    ਨਹੀਂਤਾਂ
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  ਜੇ (err==ZIP_OK)
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  ਜੇ (err==ZIP_OK)
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.ਵਿਧੀ_2,2);

  ਜੇ (err==ZIP_OK)
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  ਜੇ (err==ZIP_OK)
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  ਜੇ (err==ZIP_OK)
  {
    ਜੇ(zi->ci.zip64)
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    ਨਹੀਂਤਾਂ
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  ਜੇ (err==ZIP_OK)
  {
    ਜੇ(zi->ci.zip64)
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    ਨਹੀਂਤਾਂ
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  ਜੇ (err==ZIP_OK)
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)ਮਾਪ_filename,2);

  ਜੇ(zi->ci.zip64)
  {
    ਮਾਪ_extrafield += 20;
  }

  ਜੇ (err==ZIP_OK)
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)ਮਾਪ_extrafield,2);

  ਜੇ ((err==ZIP_OK) && (ਮਾਪ_filename > 0))
  {
    ਜੇ (ZWRITE64(zi->z_filefunc,zi->filestream,filename,ਮਾਪ_filename)!=ਮਾਪ_filename)
      err = ZIP_ERRNO;
  }

  ਜੇ ((err==ZIP_OK) && (ਮਾਪ_extrafield_local > 0))
  {
    ਜੇ (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, ਮਾਪ_extrafield_local) != ਮਾਪ_extrafield_local)
      err = ZIP_ERRNO;
  }


  ਜੇ ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      ਛੋਟਾ ਸਿਰਲੇਖ_ID = 1;
      ਛੋਟਾ ਅੰਕੜੇ_ਮਾਪ_2 = 16;
      ZPOS64_T Compressed_ਮਾਪ = 0;
      ZPOS64_T Uncompressed_ਮਾਪ = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)ਸਿਰਲੇਖ_ID,2);
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)ਅੰਕੜੇ_ਮਾਪ_2,2);

      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_ਮਾਪ,8);
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_ਮਾਪ,8);
  }

  ਵਾਪਸ err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local ਪੂਰਨ utf8len(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅਚਲ *str, size_t len) {
    ਵਾਪਸ
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
local ਪੂਰਨ isutf8(ਅੱਖਰ ਅਚਲ *str, size_t len) {
    ਪੂਰਨ utf8 = 0;
    ਜਦੋਂ (len) {
        ਪੂਰਨ ਕੋਡ = utf8len((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅਚਲ *)str, len);
        ਜੇ (ਕੋਡ < 0)
            ਵਾਪਸ 0;
        ਜੇ (ਕੋਡ > 1)
            utf8 = 1;
        str += ਕੋਡ;
        len -= (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਕੋਡ;
    }
    ਵਾਪਸ utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip4_64(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                           ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                           ਅਚਲ ਖਾਲੀ* extrafield_global, uInt ਮਾਪ_extrafield_global,
                                           ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level, ਪੂਰਨ raw,
                                           ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,ਪੂਰਨ memLevel, ਪੂਰਨ strategy,
                                           ਅਚਲ ਅੱਖਰ* password, uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting,
                                           uLong ਸੰਸਕਰਣ_Made_By_2, uLong flag_ਅਧਾਰ, ਪੂਰਨ zip64) {
    zip64_ਅੰਦਰੂਨੀ* zi;
    uInt ਮਾਪ_filename;
    uInt ਮਾਪ_comment;
    uInt i;
    ਪੂਰਨ err = ZIP_OK;

#    ifdef NOCRYPT
    ਜੇ (password != ਨਿਰਾ)
        ਵਾਪਸ ZIP_PARAMERROR;
#    endif

    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ ZIP_PARAMERROR;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    ਜੇ ((ਵਿਧੀ_2!=0) && (ਵਿਧੀ_2!=Z_DEFLATED) && (ਵਿਧੀ_2!=Z_BZIP2ED))
      ਵਾਪਸ ZIP_PARAMERROR;
#ਨਹੀਂਤਾਂ
    ਜੇ ((ਵਿਧੀ_2!=0) && (ਵਿਧੀ_2!=Z_DEFLATED))
      ਵਾਪਸ ZIP_PARAMERROR;
#ਜੇ_ਅੰਤ

    /* The filename and comment length must fit in 16 bits. */
    ਜੇ ((filename!=ਨਿਰਾ) && (ਲੜੀ_ਲੰਬਾਈ(filename)>0xffff))
        ਵਾਪਸ ZIP_PARAMERROR;
    ਜੇ ((comment!=ਨਿਰਾ) && (ਲੜੀ_ਲੰਬਾਈ(comment)>0xffff))
        ਵਾਪਸ ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    ਜੇ ((ਮਾਪ_extrafield_local>0xffff) || (ਮਾਪ_extrafield_global>0xffff))
        ਵਾਪਸ ZIP_PARAMERROR;

    zi = (zip64_ਅੰਦਰੂਨੀ*)file;

    ਜੇ (zi->ਵਿੱਚ_opened_file_inzip == 1)
    {
        err = zip_Close_File_ਵਿੱਚ_Zip (file);
        ਜੇ (err != ZIP_OK)
            ਵਾਪਸ err;
    }

    ਜੇ (filename==ਨਿਰਾ)
        filename="-";

    ਜੇ (comment==ਨਿਰਾ)
        ਮਾਪ_comment = 0;
    ਨਹੀਂਤਾਂ
        ਮਾਪ_comment = (uInt)ਲੜੀ_ਲੰਬਾਈ(comment);

    ਮਾਪ_filename = (uInt)ਲੜੀ_ਲੰਬਾਈ(filename);

    ਜੇ (zipfi == ਨਿਰਾ)
        zi->ci.dosDate = 0;
    ਨਹੀਂਤਾਂ
    {
        ਜੇ (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        ਨਹੀਂਤਾਂ
          zi->ci.dosDate = zip64local_Tmz_Date_ਮੰਜ਼ਿਲ_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_ਅਧਾਰ;
    ਜੇ ((level==8) || (level==9))
      zi->ci.flag |= 2;
    ਜੇ (level==2)
      zi->ci.flag |= 4;
    ਜੇ (level==1)
      zi->ci.flag |= 6;
    ਜੇ (password != ਨਿਰਾ)
      zi->ci.flag |= 1;
    ਜੇ (isutf8(filename, ਮਾਪ_filename) &&
        (ਮਾਪ_comment == 0 || isutf8(comment, ਮਾਪ_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.ਚੱਕਰੀ_ਜਾਂਚ_32_2 = 0;
    zi->ci.ਵਿਧੀ_2 = ਵਿਧੀ_2;
    zi->ci.encrypt = 0;
    zi->ci.ਧਾਰਾ_initialised = 0;
    zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_ਸਿਰਲੇਖ = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.ਮਾਪ_centralheader = SIZECENTRALHEADER + ਮਾਪ_filename + ਮਾਪ_extrafield_global + ਮਾਪ_comment;
    zi->ci.ਮਾਪ_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_ਸਿਰਲੇਖ = (ਅੱਖਰ*)ALLOC((uInt)zi->ci.ਮਾਪ_centralheader + zi->ci.ਮਾਪ_central_Extra_Free);

    zi->ci.ਮਾਪ_central_Extra = ਮਾਪ_extrafield_global;
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+4,(uLong)ਸੰਸਕਰਣ_Made_By_2,2);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+6,(uLong)20,2);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+8,(uLong)zi->ci.flag,2);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+10,(uLong)zi->ci.ਵਿਧੀ_2,2);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+16,(uLong)0,4); /*crc*/
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+20,(uLong)0,4); /*compr size*/
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+28,(uLong)ਮਾਪ_filename,2);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+30,(uLong)ਮਾਪ_extrafield_global,2);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+32,(uLong)ਮਾਪ_comment,2);
    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+34,(uLong)0,2); /*disk nm start*/

    ਜੇ (zipfi==ਨਿਰਾ)
        zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+36,(uLong)0,2);
    ਨਹੀਂਤਾਂ
        zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+36,(uLong)zipfi->ਅੰਦਰੂਨੀ_fa,2);

    ਜੇ (zipfi==ਨਿਰਾ)
        zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+38,(uLong)0,4);
    ਨਹੀਂਤਾਂ
        zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+38,(uLong)zipfi->external_fa,4);

    ਜੇ(zi->ci.pos_local_ਸਿਰਲੇਖ >= 0xffffffff)
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+42,(uLong)0xffffffff,4);
    ਨਹੀਂਤਾਂ
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+42,(uLong)zi->ci.pos_local_ਸਿਰਲੇਖ - zi->ਜੋੜੋ_position_when_writing_ਵਿੱਥ,4);

    ਲਈ (i=0;i<ਮਾਪ_filename;i++)
        *(zi->ci.central_ਸਿਰਲੇਖ+SIZECENTRALHEADER+i) = *(filename+i);

    ਲਈ (i=0;i<ਮਾਪ_extrafield_global;i++)
        *(zi->ci.central_ਸਿਰਲੇਖ+SIZECENTRALHEADER+ਮਾਪ_filename+i) =
              *(((ਅਚਲ ਅੱਖਰ*)extrafield_global)+i);

    ਲਈ (i=0;i<ਮਾਪ_comment;i++)
        *(zi->ci.central_ਸਿਰਲੇਖ+SIZECENTRALHEADER+ਮਾਪ_filename+
              ਮਾਪ_extrafield_global+i) = *(comment+i);
    ਜੇ (zi->ci.central_ਸਿਰਲੇਖ == ਨਿਰਾ)
        ਵਾਪਸ ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.ਕੁੱਲ_Compressed_ਅੰਕੜੇ = 0;
    zi->ci.ਕੁੱਲ_Uncompressed_ਅੰਕੜੇ = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_ਸਿਰਲੇਖ(zi, filename, ਮਾਪ_extrafield_local, extrafield_local);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    zi->ci.bstream.avail_ਵਿੱਚ = (uInt)0;
    zi->ci.bstream.avail_ਬਾਹਰਲਾ = (uInt)Z_BUFSIZE;
    zi->ci.bstream.ਅਗਲਾ_ਬਾਹਰਲਾ = (ਅੱਖਰ*)zi->ci.buffered_ਅੰਕੜੇ;
    zi->ci.bstream.ਕੁੱਲ_ਵਿੱਚ_hi32 = 0;
    zi->ci.bstream.ਕੁੱਲ_ਵਿੱਚ_lo32 = 0;
    zi->ci.bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_hi32 = 0;
    zi->ci.bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32 = 0;
#ਜੇ_ਅੰਤ

    zi->ci.ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)0;
    zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)Z_BUFSIZE;
    zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = zi->ci.buffered_ਅੰਕੜੇ;
    zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ = 0;
    zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ = 0;
    zi->ci.ਧਾਰਾ.ਅੰਕੜੇ_type = Z_ਦੁਆਧਾਰੀ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    ਜੇ ((err==ZIP_OK) && (zi->ci.ਵਿਧੀ_2 == Z_DEFLATED || zi->ci.ਵਿਧੀ_2 == Z_BZIP2ED) && (!zi->ci.raw))
#ਨਹੀਂਤਾਂ
    ਜੇ ((err==ZIP_OK) && (zi->ci.ਵਿਧੀ_2 == Z_DEFLATED) && (!zi->ci.raw))
#ਜੇ_ਅੰਤ
    {
        ਜੇ(zi->ci.ਵਿਧੀ_2 == Z_DEFLATED)
        {
          zi->ci.ਧਾਰਾ.zalloc = (alloc_func)0;
          zi->ci.ਧਾਰਾ.zfree = (free_func)0;
          zi->ci.ਧਾਰਾ.opaque = (voidpf)0;

          ਜੇ (ਖਿੜਕੀ_ਬਿੱਟ>0)
              ਖਿੜਕੀ_ਬਿੱਟ = -ਖਿੜਕੀ_ਬਿੱਟ;

          err = deflateInit2(&zi->ci.ਧਾਰਾ, level, Z_DEFLATED, ਖਿੜਕੀ_ਬਿੱਟ, memLevel, strategy);

          ਜੇ (err==Z_OK)
              zi->ci.ਧਾਰਾ_initialised = Z_DEFLATED;
        }
        ਨਹੀਂਤਾਂ ਜੇ(zi->ci.ਵਿਧੀ_2 == Z_BZIP2ED)
        {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          ਜੇ(err == BZ_OK)
            zi->ci.ਧਾਰਾ_initialised = Z_BZIP2ED;
#ਜੇ_ਅੰਤ
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_ਸਿਰਲੇਖ_ਮਾਪ = 0;
    ਜੇ ((err==Z_OK) && (password != ਨਿਰਾ))
    {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf_ਸਿਰ[RAND_ਸਿਰ_LEN];
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ_ਸਿਰ;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        ਮਾਪ_ਸਿਰ=crypthead(password,buf_ਸਿਰ,RAND_ਸਿਰ_LEN,zi->ci.ਕੁੰਜੀਆਂ,zi->ci.pcrc_32_tab,ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting);
        zi->ci.crypt_ਸਿਰਲੇਖ_ਮਾਪ = ਮਾਪ_ਸਿਰ;

        ਜੇ (ZWRITE64(zi->z_filefunc,zi->filestream,buf_ਸਿਰ,ਮਾਪ_ਸਿਰ) != ਮਾਪ_ਸਿਰ)
                err = ZIP_ERRNO;
    }
#    endif

    ਜੇ (err==Z_OK)
        zi->ਵਿੱਚ_opened_file_inzip = 1;
    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip4(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                        ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                        ਅਚਲ ਖਾਲੀ* extrafield_global, uInt ਮਾਪ_extrafield_global,
                                        ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level, ਪੂਰਨ raw,
                                        ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,ਪੂਰਨ memLevel, ਪੂਰਨ strategy,
                                        ਅਚਲ ਅੱਖਰ* password, uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting,
                                        uLong ਸੰਸਕਰਣ_Made_By_2, uLong flag_ਅਧਾਰ) {
    ਵਾਪਸ zip_Open_New_File_ਵਿੱਚ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ਮਾਪ_extrafield_local,
                                   extrafield_global, ਮਾਪ_extrafield_global,
                                   comment, ਵਿਧੀ_2, level, raw,
                                   ਖਿੜਕੀ_ਬਿੱਟ, memLevel, strategy,
                                   password, ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting, ਸੰਸਕਰਣ_Made_By_2, flag_ਅਧਾਰ, 0);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip3(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                        ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                        ਅਚਲ ਖਾਲੀ* extrafield_global, uInt ਮਾਪ_extrafield_global,
                                        ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level, ਪੂਰਨ raw,
                                        ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,ਪੂਰਨ memLevel, ਪੂਰਨ strategy,
                                        ਅਚਲ ਅੱਖਰ* password, uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting) {
    ਵਾਪਸ zip_Open_New_File_ਵਿੱਚ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ਮਾਪ_extrafield_local,
                                   extrafield_global, ਮਾਪ_extrafield_global,
                                   comment, ਵਿਧੀ_2, level, raw,
                                   ਖਿੜਕੀ_ਬਿੱਟ, memLevel, strategy,
                                   password, ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting, VERSIONMADEBY, 0, 0);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip3_64(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                         ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                         ਅਚਲ ਖਾਲੀ* extrafield_global, uInt ਮਾਪ_extrafield_global,
                                         ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level, ਪੂਰਨ raw,
                                         ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,ਪੂਰਨ memLevel, ਪੂਰਨ strategy,
                                         ਅਚਲ ਅੱਖਰ* password, uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting, ਪੂਰਨ zip64) {
    ਵਾਪਸ zip_Open_New_File_ਵਿੱਚ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ਮਾਪ_extrafield_local,
                                   extrafield_global, ਮਾਪ_extrafield_global,
                                   comment, ਵਿਧੀ_2, level, raw,
                                   ਖਿੜਕੀ_ਬਿੱਟ, memLevel, strategy,
                                   password, ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting, VERSIONMADEBY, 0, zip64);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip2(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                        ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                        ਅਚਲ ਖਾਲੀ* extrafield_global, uInt ਮਾਪ_extrafield_global,
                                        ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level, ਪੂਰਨ raw) {
    ਵਾਪਸ zip_Open_New_File_ਵਿੱਚ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ਮਾਪ_extrafield_local,
                                   extrafield_global, ਮਾਪ_extrafield_global,
                                   comment, ਵਿਧੀ_2, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ਨਿਰਾ, 0, VERSIONMADEBY, 0, 0);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip2_64(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                           ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                           ਅਚਲ ਖਾਲੀ* extrafield_global, uInt ਮਾਪ_extrafield_global,
                                           ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level, ਪੂਰਨ raw, ਪੂਰਨ zip64) {
    ਵਾਪਸ zip_Open_New_File_ਵਿੱਚ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ਮਾਪ_extrafield_local,
                                   extrafield_global, ਮਾਪ_extrafield_global,
                                   comment, ਵਿਧੀ_2, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ਨਿਰਾ, 0, VERSIONMADEBY, 0, zip64);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip64(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                         ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                         ਅਚਲ ਖਾਲੀ*extrafield_global, uInt ਮਾਪ_extrafield_global,
                                         ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level, ਪੂਰਨ zip64) {
    ਵਾਪਸ zip_Open_New_File_ਵਿੱਚ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ਮਾਪ_extrafield_local,
                                   extrafield_global, ਮਾਪ_extrafield_global,
                                   comment, ਵਿਧੀ_2, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ਨਿਰਾ, 0, VERSIONMADEBY, 0, zip64);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip(zipFile file, ਅਚਲ ਅੱਖਰ* filename, ਅਚਲ zip_fileinfo* zipfi,
                                       ਅਚਲ ਖਾਲੀ* extrafield_local, uInt ਮਾਪ_extrafield_local,
                                       ਅਚਲ ਖਾਲੀ*extrafield_global, uInt ਮਾਪ_extrafield_global,
                                       ਅਚਲ ਅੱਖਰ* comment, ਪੂਰਨ ਵਿਧੀ_2, ਪੂਰਨ level) {
    ਵਾਪਸ zip_Open_New_File_ਵਿੱਚ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ਮਾਪ_extrafield_local,
                                   extrafield_global, ਮਾਪ_extrafield_global,
                                   comment, ਵਿਧੀ_2, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ਨਿਰਾ, 0, VERSIONMADEBY, 0, 0);
}

local ਪੂਰਨ zip64_Flush_Write_ਬਫਰ(zip64_ਅੰਦਰੂਨੀ* zi) {
    ਪੂਰਨ err=ZIP_OK;

    ਜੇ (zi->ci.encrypt != 0)
    {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NOCRYPT
        uInt i;
        ਪੂਰਨ t;
        ਲਈ (i=0;i<zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ;i++)
            zi->ci.buffered_ਅੰਕੜੇ[i] = zencode(zi->ci.ਕੁੰਜੀਆਂ, zi->ci.pcrc_32_tab, zi->ci.buffered_ਅੰਕੜੇ[i],t);
#ਜੇ_ਅੰਤ
    }

    ਜੇ (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_ਅੰਕੜੇ,zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ) != zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ)
      err = ZIP_ERRNO;

    zi->ci.ਕੁੱਲ_Compressed_ਅੰਕੜੇ += zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    ਜੇ(zi->ci.ਵਿਧੀ_2 == Z_BZIP2ED)
    {
      zi->ci.ਕੁੱਲ_Uncompressed_ਅੰਕੜੇ += zi->ci.bstream.ਕੁੱਲ_ਵਿੱਚ_lo32;
      zi->ci.bstream.ਕੁੱਲ_ਵਿੱਚ_lo32 = 0;
      zi->ci.bstream.ਕੁੱਲ_ਵਿੱਚ_hi32 = 0;
    }
    ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      zi->ci.ਕੁੱਲ_Uncompressed_ਅੰਕੜੇ += zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ;
      zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ = 0;
    }


    zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ = 0;

    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Write_ਵਿੱਚ_File_ਵਿੱਚ_Zip(zipFile file, ਅਚਲ ਖਾਲੀ* buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ len) {
    zip64_ਅੰਦਰੂਨੀ* zi;
    ਪੂਰਨ err=ZIP_OK;

    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ ZIP_PARAMERROR;
    zi = (zip64_ਅੰਦਰੂਨੀ*)file;

    ਜੇ (zi->ਵਿੱਚ_opened_file_inzip == 0)
        ਵਾਪਸ ZIP_PARAMERROR;

    zi->ci.ਚੱਕਰੀ_ਜਾਂਚ_32_2 = ਚੱਕਰੀ_ਜਾਂਚ_32_2(zi->ci.ਚੱਕਰੀ_ਜਾਂਚ_32_2,buf,(uInt)len);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    ਜੇ(zi->ci.ਵਿਧੀ_2 == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.ਅਗਲਾ_ਵਿੱਚ = (ਖਾਲੀ*)buf;
      zi->ci.bstream.avail_ਵਿੱਚ = len;
      err = BZ_ਲੜੀਵਾਰ_OK;

      ਜਦੋਂ ((err==BZ_ਲੜੀਵਾਰ_OK) && (zi->ci.bstream.avail_ਵਿੱਚ>0))
      {
        ਜੇ (zi->ci.bstream.avail_ਬਾਹਰਲਾ == 0)
        {
          ਜੇ (zip64_Flush_Write_ਬਫਰ(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_ਬਾਹਰਲਾ = (uInt)Z_BUFSIZE;
          zi->ci.bstream.ਅਗਲਾ_ਬਾਹਰਲਾ = (ਅੱਖਰ*)zi->ci.buffered_ਅੰਕੜੇ;
        }


        ਜੇ(err != BZ_ਲੜੀਵਾਰ_OK)
          ਰੋਕੋ;

        ਜੇ ((zi->ci.ਵਿਧੀ_2 == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before_lo = zi->ci.bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_ਲੜੀਵਾਰ);

          zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ += (uInt)(zi->ci.bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32 - u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before_lo) ;
        }
      }

      ਜੇ(err == BZ_ਲੜੀਵਾਰ_OK)
        err = ZIP_OK;
    }
    ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = buf;
      zi->ci.ਧਾਰਾ.avail_ਵਿੱਚ = len;

      ਜਦੋਂ ((err==ZIP_OK) && (zi->ci.ਧਾਰਾ.avail_ਵਿੱਚ>0))
      {
          ਜੇ (zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ == 0)
          {
              ਜੇ (zip64_Flush_Write_ਬਫਰ(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)Z_BUFSIZE;
              zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = zi->ci.buffered_ਅੰਕੜੇ;
          }


          ਜੇ(err != ZIP_OK)
              ਰੋਕੋ;

          ਜੇ ((zi->ci.ਵਿਧੀ_2 == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before = zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ;
              err=deflate(&zi->ci.ਧਾਰਾ,  Z_NO_FLUSH);

              zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ += (uInt)(zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ - u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before) ;
          }
          ਨਹੀਂਤਾਂ
          {
              uInt copy_this,i;
              ਜੇ (zi->ci.ਧਾਰਾ.avail_ਵਿੱਚ < zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ)
                  copy_this = zi->ci.ਧਾਰਾ.avail_ਵਿੱਚ;
              ਨਹੀਂਤਾਂ
                  copy_this = zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ;

              ਲਈ (i = 0; i < copy_this; i++)
                  *(((ਅੱਖਰ*)zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ)+i) =
                      *(((ਅਚਲ ਅੱਖਰ*)zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ)+i);
              {
                  zi->ci.ਧਾਰਾ.avail_ਵਿੱਚ -= copy_this;
                  zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ-= copy_this;
                  zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ+= copy_this;
                  zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ+= copy_this;
                  zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ+= copy_this;
                  zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ+= copy_this;
                  zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ += copy_this;
              }
          }
      }/* while(...) */
    }

    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Close_File_ਵਿੱਚ_Zip_Raw(zipFile file, uLong uncompressed_ਮਾਪ, uLong ਚੱਕਰੀ_ਜਾਂਚ_32_2) {
    ਵਾਪਸ zip_Close_File_ਵਿੱਚ_Zip_Raw64 (file, uncompressed_ਮਾਪ, ਚੱਕਰੀ_ਜਾਂਚ_32_2);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Close_File_ਵਿੱਚ_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_ਮਾਪ, uLong ਚੱਕਰੀ_ਜਾਂਚ_32_2) {
    zip64_ਅੰਦਰੂਨੀ* zi;
    ZPOS64_T compressed_ਮਾਪ;
    uLong invalid_ਮੁੱਲ = 0xffffffff;
    ਬਿਨਾਨਿਸ਼ਾਨੀ datasize = 0;
    ਪੂਰਨ err=ZIP_OK;

    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ ZIP_PARAMERROR;
    zi = (zip64_ਅੰਦਰੂਨੀ*)file;

    ਜੇ (zi->ਵਿੱਚ_opened_file_inzip == 0)
        ਵਾਪਸ ZIP_PARAMERROR;
    zi->ci.ਧਾਰਾ.avail_ਵਿੱਚ = 0;

    ਜੇ ((zi->ci.ਵਿਧੀ_2 == Z_DEFLATED) && (!zi->ci.raw))
                {
                        ਜਦੋਂ (err==ZIP_OK)
                        {
                                uLong u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before;
                                ਜੇ (zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ == 0)
                                {
                                        ਜੇ (zip64_Flush_Write_ਬਫਰ(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)Z_BUFSIZE;
                                        zi->ci.ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = zi->ci.buffered_ਅੰਕੜੇ;
                                }
                                u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before = zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ;
                                err=deflate(&zi->ci.ਧਾਰਾ,  Z_ਅੰਤ);
                                zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ += (uInt)(zi->ci.ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ - u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before) ;
                        }
                }
    ਨਹੀਂਤਾਂ ਜੇ ((zi->ci.ਵਿਧੀ_2 == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
      err = BZ_ਅੰਤ_OK;
      ਜਦੋਂ (err==BZ_ਅੰਤ_OK)
      {
        uLong u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before;
        ਜੇ (zi->ci.bstream.avail_ਬਾਹਰਲਾ == 0)
        {
          ਜੇ (zip64_Flush_Write_ਬਫਰ(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_ਬਾਹਰਲਾ = (uInt)Z_BUFSIZE;
          zi->ci.bstream.ਅਗਲਾ_ਬਾਹਰਲਾ = (ਅੱਖਰ*)zi->ci.buffered_ਅੰਕੜੇ;
        }
        u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before = zi->ci.bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_ਅੰਤ);
        ਜੇ(err == BZ_ਧਾਰਾ_ਸਿਰਾ)
          err = Z_ਧਾਰਾ_ਸਿਰਾ;

        zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ += (uInt)(zi->ci.bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32 - u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before);
      }

      ਜੇ(err == BZ_ਅੰਤ_OK)
        err = ZIP_OK;
#ਜੇ_ਅੰਤ
    }

    ਜੇ (err==Z_ਧਾਰਾ_ਸਿਰਾ)
        err=ZIP_OK; /* this is normal */

    ਜੇ ((zi->ci.pos_ਵਿੱਚ_buffered_ਅੰਕੜੇ>0) && (err==ZIP_OK))
                {
        ਜੇ (zip64_Flush_Write_ਬਫਰ(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    ਜੇ ((zi->ci.ਵਿਧੀ_2 == Z_DEFLATED) && (!zi->ci.raw))
    {
        ਪੂਰਨ tmp_err = deflate_ਸਿਰਾ(&zi->ci.ਧਾਰਾ);
        ਜੇ (err == ZIP_OK)
            err = tmp_err;
        zi->ci.ਧਾਰਾ_initialised = 0;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    ਨਹੀਂਤਾਂ ਜੇ((zi->ci.ਵਿਧੀ_2 == Z_BZIP2ED) && (!zi->ci.raw))
    {
      ਪੂਰਨ tmperr = BZ2_bz_Compress_ਸਿਰਾ(&zi->ci.bstream);
                        ਜੇ (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.ਧਾਰਾ_initialised = 0;
    }
#ਜੇ_ਅੰਤ

    ਜੇ (!zi->ci.raw)
    {
        ਚੱਕਰੀ_ਜਾਂਚ_32_2 = (uLong)zi->ci.ਚੱਕਰੀ_ਜਾਂਚ_32_2;
        uncompressed_ਮਾਪ = zi->ci.ਕੁੱਲ_Uncompressed_ਅੰਕੜੇ;
    }
    compressed_ਮਾਪ = zi->ci.ਕੁੱਲ_Compressed_ਅੰਕੜੇ;

#    ifndef NOCRYPT
    compressed_ਮਾਪ += zi->ci.crypt_ਸਿਰਲੇਖ_ਮਾਪ;
#    endif

    /* update Current Item crc and sizes, */
    ਜੇ(compressed_ਮਾਪ >= 0xffffffff || uncompressed_ਮਾਪ >= 0xffffffff || zi->ci.pos_local_ਸਿਰਲੇਖ >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+6,(uLong)45,2);

    }

    zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+16,ਚੱਕਰੀ_ਜਾਂਚ_32_2,4); /*crc*/


    ਜੇ(compressed_ਮਾਪ >= 0xffffffff)
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+20, invalid_ਮੁੱਲ,4); /*compr size*/
    ਨਹੀਂਤਾਂ
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+20, compressed_ਮਾਪ,4); /*compr size*/

    /* set internal file attributes field */
    ਜੇ (zi->ci.ਧਾਰਾ.ਅੰਕੜੇ_type == Z_ASCII)
        zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+36,(uLong)Z_ASCII,2);

    ਜੇ(uncompressed_ਮਾਪ >= 0xffffffff)
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+24, invalid_ਮੁੱਲ,4); /*uncompr size*/
    ਨਹੀਂਤਾਂ
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+24, uncompressed_ਮਾਪ,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    ਜੇ(uncompressed_ਮਾਪ >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    ਜੇ(compressed_ਮਾਪ >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    ਜੇ(zi->ci.pos_local_ਸਿਰਲੇਖ >= 0xffffffff)
      datasize += 8;

    ਜੇ(datasize > 0)
    {
      ਅੱਖਰ* p = ਨਿਰਾ;

      ਜੇ((uLong)(datasize + 4) > zi->ci.ਮਾਪ_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        ਵਾਪਸ ZIP_BADZIPFILE;
      }

      p = zi->ci.central_ਸਿਰਲੇਖ + zi->ci.ਮਾਪ_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_ਮੁੱਲ_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_ਮੁੱਲ_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      ਜੇ(uncompressed_ਮਾਪ >= 0xffffffff)
      {
        zip64local_put_ਮੁੱਲ_inmemory(p, uncompressed_ਮਾਪ, 8);
        p += 8;
      }

      ਜੇ(compressed_ਮਾਪ >= 0xffffffff)
      {
        zip64local_put_ਮੁੱਲ_inmemory(p, compressed_ਮਾਪ, 8);
        p += 8;
      }

      ਜੇ(zi->ci.pos_local_ਸਿਰਲੇਖ >= 0xffffffff)
      {
        zip64local_put_ਮੁੱਲ_inmemory(p, zi->ci.pos_local_ਸਿਰਲੇਖ, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.ਮਾਪ_central_Extra_Free -= datasize + 4;
      zi->ci.ਮਾਪ_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.ਮਾਪ_central_Extra += datasize + 4;
      zip64local_put_ਮੁੱਲ_inmemory(zi->ci.central_ਸਿਰਲੇਖ+30,(uLong)zi->ci.ਮਾਪ_central_Extra,2);
    }

    ਜੇ (err==ZIP_OK)
        err = ਜੋੜੋ_ਅੰਕੜੇ_ਵਿੱਚ_datablock(&zi->central_dir, zi->ci.central_ਸਿਰਲੇਖ, (uLong)zi->ci.ਮਾਪ_centralheader);

    ਮੁਕਤ(zi->ci.central_ਸਿਰਲੇਖ);

    ਜੇ (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        ਜੇ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_ਸਿਰਲੇਖ + 14,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
            err = ZIP_ERRNO;

        ਜੇ (err==ZIP_OK)
            err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,ਚੱਕਰੀ_ਜਾਂਚ_32_2,4); /* crc 32, unknown */

        ਜੇ(uncompressed_ਮਾਪ >= 0xffffffff || compressed_ਮਾਪ >= 0xffffffff )
        {
          ਜੇ(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            ਜੇ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
              err = ZIP_ERRNO;

            ਜੇ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, uncompressed_ਮਾਪ, 8);

            ਜੇ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, compressed_ਮਾਪ, 8);
          }
          ਨਹੀਂਤਾਂ
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        ਨਹੀਂਤਾਂ
        {
          ਜੇ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,compressed_ਮਾਪ,4);

          ਜੇ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,uncompressed_ਮਾਪ,4);
        }

        ਜੇ (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
            err = ZIP_ERRNO;
    }

    zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ ++;
    zi->ਵਿੱਚ_opened_file_inzip = 0;

    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Close_File_ਵਿੱਚ_Zip(zipFile file) {
    ਵਾਪਸ zip_Close_File_ਵਿੱਚ_Zip_Raw (file,0,0);
}

local ਪੂਰਨ Write_Zip64_ਸਿਰਾ_ਦਾ_Central_Directory_Locator(zip64_ਅੰਦਰੂਨੀ* zi, ZPOS64_T zip64eocd_pos_inzip) {
  ਪੂਰਨ err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->ਜੋੜੋ_position_when_writing_ਵਿੱਥ;

  err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    ਜੇ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    ਜੇ (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    ਜੇ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    ਵਾਪਸ err;
}

local ਪੂਰਨ Write_Zip64_ਸਿਰਾ_ਦਾ_Central_Directory_ਰਿਕਾਰਡ(zip64_ਅੰਦਰੂਨੀ* zi, uLong ਮਾਪ_centraldir, ZPOS64_T centraldir_pos_inzip) {
  ਪੂਰਨ err = ZIP_OK;

  uLong Zip64_ਅੰਕੜੇ_ਮਾਪ = 44;

  err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  ਜੇ (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_ਅੰਕੜੇ_ਮਾਪ,8); /* why ZPOS64_T of this ? */

  ਜੇ (err==ZIP_OK) /* version made by */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ਜੇ (err==ZIP_OK) /* version needed */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ਜੇ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ਜੇ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ਜੇ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ, 8);

  ਜੇ (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc, zi->filestream, zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ, 8);

  ਜੇ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)ਮਾਪ_centraldir,8);

  ਜੇ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->ਜੋੜੋ_position_when_writing_ਵਿੱਥ;
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  ਵਾਪਸ err;
}

local ਪੂਰਨ Write_ਸਿਰਾ_ਦਾ_Central_Directory_ਰਿਕਾਰਡ(zip64_ਅੰਦਰੂਨੀ* zi, uLong ਮਾਪ_centraldir, ZPOS64_T centraldir_pos_inzip) {
  ਪੂਰਨ err = ZIP_OK;

  /*signature*/
  err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  ਜੇ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ਜੇ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ਜੇ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      ਜੇ(zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ >= 0xFFFF)
        err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      ਨਹੀਂਤਾਂ
        err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ,2);
    }
  }

  ਜੇ (err==ZIP_OK) /* total number of entries in the central dir */
  {
    ਜੇ(zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ >= 0xFFFF)
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    ਨਹੀਂਤਾਂ
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ,2);
  }

  ਜੇ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)ਮਾਪ_centraldir,4);

  ਜੇ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->ਜੋੜੋ_position_when_writing_ਵਿੱਥ;
    ਜੇ(pos >= 0xffffffff)
    {
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    ਨਹੀਂਤਾਂ
      err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->ਜੋੜੋ_position_when_writing_ਵਿੱਥ),4);
  }

   ਵਾਪਸ err;
}

local ਪੂਰਨ Write_GlobalComment(zip64_ਅੰਦਰੂਨੀ* zi, ਅਚਲ ਅੱਖਰ* global_comment) {
  ਪੂਰਨ err = ZIP_OK;
  uInt ਮਾਪ_global_comment = 0;

  ਜੇ(global_comment != ਨਿਰਾ)
    ਮਾਪ_global_comment = (uInt)ਲੜੀ_ਲੰਬਾਈ(global_comment);

  err = zip64local_put_ਮੁੱਲ(&zi->z_filefunc,zi->filestream,(uLong)ਮਾਪ_global_comment,2);

  ਜੇ (err == ZIP_OK && ਮਾਪ_global_comment > 0)
  {
    ਜੇ (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, ਮਾਪ_global_comment) != ਮਾਪ_global_comment)
      err = ZIP_ERRNO;
  }
  ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zipClose(zipFile file, ਅਚਲ ਅੱਖਰ* global_comment) {
    zip64_ਅੰਦਰੂਨੀ* zi;
    ਪੂਰਨ err = 0;
    uLong ਮਾਪ_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    ਜੇ (file == ਨਿਰਾ)
        ਵਾਪਸ ZIP_PARAMERROR;

    zi = (zip64_ਅੰਦਰੂਨੀ*)file;

    ਜੇ (zi->ਵਿੱਚ_opened_file_inzip == 1)
    {
        err = zip_Close_File_ਵਿੱਚ_Zip (file);
    }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_ADDFILEINEXISTINGZIP
    ਜੇ (global_comment==ਨਿਰਾ)
        global_comment = zi->globalcomment;
#ਜੇ_ਅੰਤ

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    ਜੇ (err==ZIP_OK)
    {
        linkedlist_datablock_ਅੰਦਰੂਨੀ* ldi = zi->central_dir.ਪਹਿਲਾ_ਖੰਡ;
        ਜਦੋਂ (ldi!=ਨਿਰਾ)
        {
            ਜੇ ((err==ZIP_OK) && (ldi->filled_ਵਿੱਚ_this_ਖੰਡ>0))
            {
                ਜੇ (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->ਅੰਕੜੇ, ldi->filled_ਵਿੱਚ_this_ਖੰਡ) != ldi->filled_ਵਿੱਚ_this_ਖੰਡ)
                    err = ZIP_ERRNO;
            }

            ਮਾਪ_centraldir += ldi->filled_ਵਿੱਚ_this_ਖੰਡ;
            ldi = ldi->ਅਗਲਾ_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    ਸੈੱਟ_ਸਿਰਾ(&zi->ਸੈੱਟ);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->ਜੋੜੋ_position_when_writing_ਵਿੱਥ;
    ਜੇ(pos >= 0xffffffff || zi->ਸੰਖਿਆ_ਇੰਦਰਾਜ >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_ਸਿਰਾ_ਦਾ_Central_Directory_ਰਿਕਾਰਡ(zi, ਮਾਪ_centraldir, centraldir_pos_inzip);

      Write_Zip64_ਸਿਰਾ_ਦਾ_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    ਜੇ (err==ZIP_OK)
      err = Write_ਸਿਰਾ_ਦਾ_Central_Directory_ਰਿਕਾਰਡ(zi, ਮਾਪ_centraldir, centraldir_pos_inzip);

    ਜੇ(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    ਜੇ (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        ਜੇ (err == ZIP_OK)
            err = ZIP_ERRNO;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_ADDFILEINEXISTINGZIP
    ਮੁਕਤ(zi->globalcomment);
#ਜੇ_ਅੰਤ
    ਮੁਕਤ(zi);

    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Remove_Extra_Info_ਖੰਡ(ਅੱਖਰ* p_ਅੰਕੜੇ, ਪੂਰਨ* ਅੰਕੜੇ_Len, ਛੋਟਾ s_ਸਿਰਲੇਖ) {
  ਅੱਖਰ* p = p_ਅੰਕੜੇ;
  ਪੂਰਨ ਮਾਪ = 0;
  ਅੱਖਰ* p_New_ਸਿਰਲੇਖ;
  ਅੱਖਰ* pTmp;
  ਛੋਟਾ ਸਿਰਲੇਖ;
  ਛੋਟਾ ਅੰਕੜੇ_ਮਾਪ;

  ਪੂਰਨ retVal = ZIP_OK;

  ਜੇ(p_ਅੰਕੜੇ == ਨਿਰਾ || ਅੰਕੜੇ_Len == ਨਿਰਾ || *ਅੰਕੜੇ_Len < 4)
    ਵਾਪਸ ZIP_PARAMERROR;

  p_New_ਸਿਰਲੇਖ = (ਅੱਖਰ*)ALLOC((ਬਿਨਾਨਿਸ਼ਾਨੀ)*ਅੰਕੜੇ_Len);
  pTmp = p_New_ਸਿਰਲੇਖ;

  ਜਦੋਂ(p < (p_ਅੰਕੜੇ + *ਅੰਕੜੇ_Len))
  {
    ਸਿਰਲੇਖ = *(ਛੋਟਾ*)p;
    ਅੰਕੜੇ_ਮਾਪ = *(((ਛੋਟਾ*)p)+1);

    ਜੇ( ਸਿਰਲੇਖ == s_ਸਿਰਲੇਖ ) /* Header found. */
    {
      p += ਅੰਕੜੇ_ਮਾਪ + 4; /* skip it. do not copy to temp buffer */
    }
    ਨਹੀਂਤਾਂ
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pTmp, p, ਅੰਕੜੇ_ਮਾਪ + 4);
      p += ਅੰਕੜੇ_ਮਾਪ + 4;
      ਮਾਪ += ਅੰਕੜੇ_ਮਾਪ + 4;
    }

  }

  ਜੇ(ਮਾਪ < *ਅੰਕੜੇ_Len)
  {
    /* clean old extra info block. */
    memset(p_ਅੰਕੜੇ,0, *ਅੰਕੜੇ_Len);

    /* copy the new extra info block over the old */
    ਜੇ(ਮਾਪ > 0)
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਅੰਕੜੇ, p_New_ਸਿਰਲੇਖ, ਮਾਪ);

    /* set the new extra info size */
    *ਅੰਕੜੇ_Len = ਮਾਪ;

    retVal = ZIP_OK;
  }
  ਨਹੀਂਤਾਂ
    retVal = ZIP_ERRNO;

  ਮੁਕਤ(p_New_ਸਿਰਲੇਖ);

  ਵਾਪਸ retVal;
}
