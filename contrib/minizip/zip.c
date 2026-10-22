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


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZLIB_CONST
#  define ZLIB_CONST
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DLL
#  undef ZLIB_DLL
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ "zip.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ STDC
#  include <stddef.h>
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_ERRNO_H
    ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ errno;
#ಇಲ್ಲದಿದ್ದರೆ
#   include <errno.h>
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ local
#  define local ಸ್ಥಿರ
#ಆದರೆ_ಅಂತ್ಯ
/* compile with -Dlocal if your debugger can't find static symbols */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_BUFSIZE
#ವ್ಯಾಖ್ಯಾನಿಸು Z_BUFSIZE (64*1024) /* (16384) */
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_MAXFILENAMEINZIP
#ವ್ಯಾಖ್ಯಾನಿಸು Z_MAXFILENAMEINZIP (256)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ALLOC
# define ALLOC(ಗಾತ್ರ_2) (ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2))
#ಆದರೆ_ಅಂತ್ಯ

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#ವ್ಯಾಖ್ಯಾನಿಸು MAKEULONG64(a, b) ((ZPOS64_T)(((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(a)) | ((ZPOS64_T)((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(b))) << 32))

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SEEK_CUR
#ವ್ಯಾಖ್ಯಾನಿಸು SEEK_CUR    1
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SEEK_END
#ವ್ಯಾಖ್ಯಾನಿಸು SEEK_END    2
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SEEK_SET
#ವ್ಯಾಖ್ಯಾನಿಸು SEEK_SET    0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DEF_MEM_LEVEL
#ಆದರೆ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ಇಲ್ಲದಿದ್ದರೆ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#ವ್ಯಾಖ್ಯಾನಿಸು SIZEDATA_INDATABLOCK (4096-(4*4))

#ವ್ಯಾಖ್ಯಾನಿಸು LOCALHEADERMAGIC    (0x04034b50)
#ವ್ಯಾಖ್ಯಾನಿಸು CENTRALHEADERMAGIC  (0x02014b50)
#ವ್ಯಾಖ್ಯಾನಿಸು ENDHEADERMAGIC      (0x06054b50)
#ವ್ಯಾಖ್ಯಾನಿಸು ZIP64ENDHEADERMAGIC      (0x6064b50)
#ವ್ಯಾಖ್ಯಾನಿಸು ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_LOCALHEADER_ಸ್ಥಾನಾಂತರ (0x06)
#ವ್ಯಾಖ್ಯಾನಿಸು ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_LOCALHEADER_ಸ್ಥಾನಾಂತರ  (0x0e)

#ವ್ಯಾಖ್ಯಾನಿಸು SIZECENTRALHEADER (0x2e) /* 46 */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ linkedlist_datablock_ಆಂತರಿಕ_s
{
  ರಚನೆ linkedlist_datablock_ಆಂತರಿಕ_s* ಮುಂದಿನ_datablock;
  uLong  avail_ಒಳ_this_ಖಂಡ;
  uLong  filled_ಒಳ_this_ಖಂಡ;
  uLong  ಬಳಸದ; /* for future use and alignment */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ದತ್ತಾಂಶ[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_ಆಂತರಿಕ;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ linkedlist_ದತ್ತಾಂಶ_s
{
    linkedlist_datablock_ಆಂತರಿಕ* ಮೊದಲ_ಖಂಡ;
    linkedlist_datablock_ಆಂತರಿಕ* ಕೊನೆಯ_ಖಂಡ;
} linkedlist_ದತ್ತಾಂಶ;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#ವ್ಯಾಖ್ಯಾನಿಸು SKIPSET_EXPORT ಸ್ಥಿರ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಕ್ಷರ *ಗಣ_ಕೀಲಿ_t;
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_cmp(a, b) strcmp(a, b)
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_drop(s, k) ಗಣ_free(s, k)
#ಅನ್ತರ್ಭಾವ "skipset.h"
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಮುಂದಿನ;        /* next byte in datablock data */
    size_t ಎಡ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_ಆಂತರಿಕ *ಗಂಟು;    /* current datablock */
} ಖಂಡ_t;


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
    z_ಹರಿವು ಹರಿವು;            /* zLib stream structure for inflate */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    bz_ಹರಿವು bstream;          /* bzLib stream structure for bziped */
#ಆದರೆ_ಅಂತ್ಯ

    ಪೂರ್ಣಾಂಕ  ಹರಿವು_initialised;    /* 1 is stream is initialised */
    uInt pos_ಒಳ_buffered_ದತ್ತಾಂಶ;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_ಶೀರ್ಷಿಕೆ;     /* offset of the local header of the file
                                     currently writing */
    ಅಕ್ಷರ* central_ಶೀರ್ಷಿಕೆ;       /* central header data for the current file */
    uLong ಗಾತ್ರ_central_Extra;
    uLong ಗಾತ್ರ_centralheader;   /* size of the central header for cur file */
    uLong ಗಾತ್ರ_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    ಪೂರ್ಣಾಂಕ  ವಿಧಾನ_2;                /* compression method of file currently wr.*/
    ಪೂರ್ಣಾಂಕ  raw;                   /* 1 for directly writing raw data */
    Byte buffered_ದತ್ತಾಂಶ[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2;
    ಪೂರ್ಣಾಂಕ  encrypt;
    ಪೂರ್ಣಾಂಕ  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T ಒಟ್ಟು_Compressed_ದತ್ತಾಂಶ;
    ZPOS64_T ಒಟ್ಟು_Uncompressed_ದತ್ತಾಂಶ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NOCRYPT
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಕೀಲಿಗಳು[3];     /* keys defining the pseudo-random sequence */
    ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t* pcrc_32_tab;
    ಚಿಹ್ನೆರಹಿತ crypt_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ
} curfile64_info;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_ದತ್ತಾಂಶ central_dir;/* datablock with central dir in construction*/
    ಪೂರ್ಣಾಂಕ  ಒಳ_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ;
    ZPOS64_T ಸಂಖ್ಯೆ_ನಮೂದು;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_ADDFILEINEXISTINGZIP
    ಅಕ್ಷರ *globalcomment;
#ಆದರೆ_ಅಂತ್ಯ

    /* Support for zipAlreadyThere(). */
    ಗಣ_t ಗಣ;              /* set for detecting name collisions */
    ಖಂಡ_t ಖಂಡ;          /* block for reading the central directory */

} zip64_ಆಂತರಿಕ;


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NOCRYPT
#ವ್ಯಾಖ್ಯಾನಿಸು INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#ಅನ್ತರ್ಭಾವ "crypt.h"
#ಆದರೆ_ಅಂತ್ಯ

local linkedlist_datablock_ಆಂತರಿಕ* allocate_new_datablock(ಶೂನ್ಯ) {
    linkedlist_datablock_ಆಂತರಿಕ* ldi;
    ldi = (linkedlist_datablock_ಆಂತರಿಕ*)
                 ALLOC(ಗಾತ್ರ(linkedlist_datablock_ಆಂತರಿಕ));
    ಆದರೆ (ldi!=ಖಾಲಿ)
    {
        ldi->ಮುಂದಿನ_datablock = ಖಾಲಿ ;
        ldi->filled_ಒಳ_this_ಖಂಡ = 0 ;
        ldi->avail_ಒಳ_this_ಖಂಡ = SIZEDATA_INDATABLOCK ;
    }
    ಹಿಂತಿರುಗಿ ldi;
}

local ಶೂನ್ಯ free_datablock(linkedlist_datablock_ಆಂತರಿಕ* ldi) {
    ಆಗ (ldi!=ಖಾಲಿ)
    {
        linkedlist_datablock_ಆಂತರಿಕ* ldinext = ldi->ಮುಂದಿನ_datablock;
        ಬಿಡುಗಡೆ(ldi);
        ldi = ldinext;
    }
}

local ಶೂನ್ಯ init_linkedlist(linkedlist_ದತ್ತಾಂಶ* ll) {
    ll->ಮೊದಲ_ಖಂಡ = ll->ಕೊನೆಯ_ಖಂಡ = ಖಾಲಿ;
}

local ಶೂನ್ಯ free_linkedlist(linkedlist_ದತ್ತಾಂಶ* ll) {
    free_datablock(ll->ಮೊದಲ_ಖಂಡ);
    ll->ಮೊದಲ_ಖಂಡ = ll->ಕೊನೆಯ_ಖಂಡ = ಖಾಲಿ;
}


local ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_ದತ್ತಾಂಶ_ಒಳ_datablock(linkedlist_ದತ್ತಾಂಶ* ll, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* buf, uLong len) {
    linkedlist_datablock_ಆಂತರಿಕ* ldi;
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* from_copy;

    ಆದರೆ (ll==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_INTERNALERROR;

    ಆದರೆ (ll->ಕೊನೆಯ_ಖಂಡ == ಖಾಲಿ)
    {
        ll->ಮೊದಲ_ಖಂಡ = ll->ಕೊನೆಯ_ಖಂಡ = allocate_new_datablock();
        ಆದರೆ (ll->ಮೊದಲ_ಖಂಡ == ಖಾಲಿ)
            ಹಿಂತಿರುಗಿ ZIP_INTERNALERROR;
    }

    ldi = ll->ಕೊನೆಯ_ಖಂಡ;
    from_copy = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)buf;

    ಆಗ (len>0)
    {
        uInt copy_this;
        uInt i;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* ಗಮ್ಯ_copy;

        ಆದರೆ (ldi->avail_ಒಳ_this_ಖಂಡ==0)
        {
            ldi->ಮುಂದಿನ_datablock = allocate_new_datablock();
            ಆದರೆ (ldi->ಮುಂದಿನ_datablock == ಖಾಲಿ)
                ಹಿಂತಿರುಗಿ ZIP_INTERNALERROR;
            ldi = ldi->ಮುಂದಿನ_datablock ;
            ll->ಕೊನೆಯ_ಖಂಡ = ldi;
        }

        ಆದರೆ (ldi->avail_ಒಳ_this_ಖಂಡ < len)
            copy_this = (uInt)ldi->avail_ಒಳ_this_ಖಂಡ;
        ಇಲ್ಲದಿದ್ದರೆ
            copy_this = (uInt)len;

        ಗಮ್ಯ_copy = &(ldi->ದತ್ತಾಂಶ[ldi->filled_ಒಳ_this_ಖಂಡ]);

        ಗಾಗಿ (i=0;i<copy_this;i++)
            *(ಗಮ್ಯ_copy+i)=*(from_copy+i);

        ldi->filled_ಒಳ_this_ಖಂಡ += copy_this;
        ldi->avail_ಒಳ_this_ಖಂಡ -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    ಹಿಂತಿರುಗಿ ZIP_OK;
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
local ಶೂನ್ಯ ಖಂಡ_init(ಖಂಡ_t *ಖಂಡ, linkedlist_ದತ್ತಾಂಶ *list) {
    ಖಂಡ->ಗಂಟು = list->ಮೊದಲ_ಖಂಡ;
    ಖಂಡ->ಮುಂದಿನ = ಖಂಡ->ಗಂಟು->ದತ್ತಾಂಶ;
    ಖಂಡ->ಎಡ = ಖಂಡ->ಗಂಟು->filled_ಒಳ_this_ಖಂಡ;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local ಶೂನ್ಯ ಖಂಡ_stop(ಖಂಡ_t *ಖಂಡ) {
    ಖಂಡ->ಎಡ = 0;
    ಖಂಡ->ಮುಂದಿನ = ಖಾಲಿ;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local ಪೂರ್ಣಾಂಕ ಖಂಡ_ಕೊನೆ(ಖಂಡ_t *ಖಂಡ) {
    linkedlist_datablock_ಆಂತರಿಕ *ಗಂಟು = ಖಂಡ->ಗಂಟು;
    ಆದರೆ (ಗಂಟು == ಖಾಲಿ)
        /* This block was previously terminated with extreme prejudice. */
        ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ (ಖಂಡ->ಮುಂದಿನ < ಗಂಟು->ದತ್ತಾಂಶ + ಗಂಟು->filled_ಒಳ_this_ಖಂಡ)
        /* There are more bytes to read in the current datablock. */
        ಹಿಂತಿರುಗಿ 0;
    ಆಗ (ಗಂಟು->ಮುಂದಿನ_datablock != ಖಾಲಿ) {
        ಆದರೆ (ಗಂಟು->filled_ಒಳ_this_ಖಂಡ != 0)
            /* There are some bytes in a later datablock. */
            ಹಿಂತಿರುಗಿ 0;
        ಗಂಟು = ಗಂಟು->ಮುಂದಿನ_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    ಹಿಂತಿರುಗಿ 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local ಪೂರ್ಣಾಂಕ ಖಂಡ_get(ಖಂಡ_t *ಖಂಡ) {
    ಆಗ (ಖಂಡ->ಎಡ == 0) {
        ಆದರೆ (ಖಂಡ->ಗಂಟು == ಖಾಲಿ)
            /* We've been marked bad. Return end. */
            ಹಿಂತಿರುಗಿ -1;
        /* Update left in case more was filled in since we were last here. */
        ಖಂಡ->ಎಡ = ಖಂಡ->ಗಂಟು->filled_ಒಳ_this_ಖಂಡ -
                      (size_t)(ಖಂಡ->ಮುಂದಿನ - ಖಂಡ->ಗಂಟು->ದತ್ತಾಂಶ);
        ಆದರೆ (ಖಂಡ->ಎಡ != 0)
            /* There was indeed more data appended in the current datablock. */
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (ಖಂಡ->ಗಂಟು->ಮುಂದಿನ_datablock == ಖಾಲಿ)
            /* No more data here, and there is no next datablock. At the end. */
            ಹಿಂತಿರುಗಿ -1;
        /* Try the next datablock for more data. */
        ಖಂಡ->ಗಂಟು = ಖಂಡ->ಗಂಟು->ಮುಂದಿನ_datablock;
        ಖಂಡ->ಮುಂದಿನ = ಖಂಡ->ಗಂಟು->ದತ್ತಾಂಶ;
        ಖಂಡ->ಎಡ = ಖಂಡ->ಗಂಟು->filled_ಒಳ_this_ಖಂಡ;
    }
    /* We have a byte to return. */
    ಖಂಡ->ಎಡ--;
    ಹಿಂತಿರುಗಿ *ಖಂಡ->ಮುಂದಿನ++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local ದೀರ್ಘ ಖಂಡ_get2(ಖಂಡ_t *ಖಂಡ) {
    ಪೂರ್ಣಾಂಕ ಕೆಳಗಿನ = ಖಂಡ_get(ಖಂಡ);
    ಪೂರ್ಣಾಂಕ ಮೇಲಿನ = ಖಂಡ_get(ಖಂಡ);
    ಹಿಂತಿರುಗಿ ಕೆಳಗಿನ < 0 || ಮೇಲಿನ < 0 ? -1 : ಕೆಳಗಿನ | ((ದೀರ್ಘ)ಮೇಲಿನ << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t ಖಂಡ_read(ಖಂಡ_t *ಖಂಡ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, size_t len) {
    size_t need = len;
    size_t take;
    ಆಗ (need) {
        ಆದರೆ (ಖಂಡ->ಎಡ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            ಪೂರ್ಣಾಂಕ got = ಖಂಡ_get(ಖಂಡ);
            ಆದರೆ (got == -1)
                /* Reached the end. */
                ನಿಲ್ಲಿಸು;
            *buf++ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)got;
            need--;
            ಮುಂದುವರಿ;
        }
        take = need > ಖಂಡ->ಎಡ ? ಖಂಡ->ಎಡ : need;
        ಸ್ಮರಣೆ_ನಕಲು(buf, ಖಂಡ->ಮುಂದಿನ, take);
        ಖಂಡ->ಮುಂದಿನ += take;
        ಖಂಡ->ಎಡ -= take;
        buf += take;
        need -= take;
    }
    ಹಿಂತಿರುಗಿ len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local ಪೂರ್ಣಾಂಕ ಖಂಡ_skip(ಖಂಡ_t *ಖಂಡ, size_t n) {
    ಆಗ (n > ಖಂಡ->ಎಡ) {
        n -= ಖಂಡ->ಎಡ;
        ಖಂಡ->ಮುಂದಿನ += ಖಂಡ->ಎಡ;
        ಖಂಡ->ಎಡ = 0;
        ಆದರೆ (ಖಂಡ_get(ಖಂಡ) == -1)
            ಹಿಂತಿರುಗಿ -1;
        n--;
    }
    ಖಂಡ->ಮುಂದಿನ += n;
    ಖಂಡ->ಎಡ -= n;
    ಹಿಂತಿರುಗಿ 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local ಅಕ್ಷರ *ಖಂಡ_central_name(ಖಂಡ_t *ಖಂಡ, ಗಣ_t *ಗಣ) {
    ಚಿಹ್ನೆರಹಿತ flen, xlen, clen;
    ಅಕ್ಷರ *name = ಖಾಲಿ;
    ಗಾಗಿ (;;) {
        ಆದರೆ (ಖಂಡ_ಕೊನೆ(ಖಂಡ))
            /* At the end of the central directory (so far). */
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;

        /* Check for a central directory record signature. */
        ಆದರೆ (ಖಂಡ_get2(ಖಂಡ) != (CENTRALHEADERMAGIC & 0xffff) ||
            ಖಂಡ_get2(ಖಂಡ) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            ನಿಲ್ಲಿಸು;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        ಖಂಡ_skip(ಖಂಡ, 24);
        flen = (ಚಿಹ್ನೆರಹಿತ)ಖಂಡ_get2(ಖಂಡ);    /* file name length */
        xlen = (ಚಿಹ್ನೆರಹಿತ)ಖಂಡ_get2(ಖಂಡ);    /* extra length */
        clen = (ಚಿಹ್ನೆರಹಿತ)ಖಂಡ_get2(ಖಂಡ);    /* comment length */
        ಆದರೆ (ಖಂಡ_skip(ಖಂಡ, 12) == -1)
            /* Premature end of the record. */
            ನಿಲ್ಲಿಸು;

        /* Extract the name and skip over the extra and comment fields. */
        name = ಗಣ_alloc(ಗಣ, ಖಾಲಿ, flen + 1);
        ಆದರೆ (ಖಂಡ_read(ಖಂಡ, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)name, flen) < flen ||
            ಖಂಡ_skip(ಖಂಡ, xlen + clen) == -1)
            /* Premature end of the record. */
            ನಿಲ್ಲಿಸು;

        /* Check for embedded nuls in the name. */
        ಆದರೆ (memchr(name, 0, flen) != ಖಾಲಿ) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            ಗಣ_free(ಗಣ, name);
            ಮುಂದುವರಿ;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        ಹಿಂತಿರುಗಿ name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    ಗಣ_free(ಗಣ, name);
    ಖಂಡ_stop(ಖಂಡ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zipAlreadyThere(zipFile file, ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *name) {
    size_t len;
    ಅಕ್ಷರ *copy;
    ಪೂರ್ಣಾಂಕ ಸಿಕ್ಕಿತು;
    zip64_ಆಂತರಿಕ *zip = file;
    ಆದರೆ (zip == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
    ಆದರೆ (zip->central_dir.ಮೊದಲ_ಖಂಡ == ಖಾಲಿ)
        /* No central directory yet, so no, name isn't there. */
        ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ (setjmp(zip->ಗಣ.env)) {
        /* Memory allocation failure. */
        ಗಣ_ಕೊನೆ(&zip->ಗಣ);
        ಹಿಂತಿರುಗಿ -2;
    }
    ಆದರೆ (!ಗಣ_ok(&zip->ಗಣ)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        ಗಣ_ಆರಂಭ(&zip->ಗಣ);
        ಖಂಡ_init(&zip->ಖಂಡ, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    ಗಾಗಿ (;;) {
        ಅಕ್ಷರ *there = ಖಂಡ_central_name(&zip->ಖಂಡ, &zip->ಗಣ);
        ಆದರೆ (there == ಖಾಲಿ) {
            ಆದರೆ (zip->ಖಂಡ.ಮುಂದಿನ == ಖಾಲಿ)
                /* The central directory is invalid. */
                ಹಿಂತಿರುಗಿ -1;
            ನಿಲ್ಲಿಸು;
        }

        /* Add there to the set. */
        ಆದರೆ (ಗಣ_ಸೇರಿಸು(&zip->ಗಣ, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            ಗಣ_free(&zip->ಗಣ, there);
    }

    /* Return true if name is in the central directory. */
    len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);
    copy = ಗಣ_alloc(&zip->ಗಣ, ಖಾಲಿ, len + 1);
    ಸ್ಮರಣೆ_ನಕಲು(copy, name, len + 1);
    ಸಿಕ್ಕಿತು = ಗಣ_ಸಿಕ್ಕಿತು(&zip->ಗಣ, copy);
    ಗಣ_free(&zip->ಗಣ, copy);
    ಹಿಂತಿರುಗಿ ಸಿಕ್ಕಿತು;
}


/****************************************************************************/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local ಪೂರ್ಣಾಂಕ zip64local_put_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, ಪೂರ್ಣಾಂಕ nbByte) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf[8];
    ಪೂರ್ಣಾಂಕ n;
    ಗಾಗಿ (n = 0; n < nbByte; n++)
    {
        buf[n] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x & 0xff);
        x >>= 8;
    }
    ಆದರೆ (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      ಗಾಗಿ (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    ಆದರೆ (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        ಹಿಂತಿರುಗಿ ZIP_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ ZIP_OK;
}

local ಶೂನ್ಯ zip64local_put_ಮೌಲ್ಯ_inmemory (ಶೂನ್ಯ* dest, ZPOS64_T x, ಪೂರ್ಣಾಂಕ nbByte) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* buf=(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)dest;
    ಪೂರ್ಣಾಂಕ n;
    ಗಾಗಿ (n = 0; n < nbByte; n++) {
        buf[n] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x & 0xff);
        x >>= 8;
    }

    ಆದರೆ (x != 0)
    {     /* data overflow - hack for ZIP64 */
       ಗಾಗಿ (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_Tmz_Date_ಗಮ್ಯ_Dos_Date(ಸ್ಥಿರಾಂಕ tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    ಆದರೆ (year>=1980)
        year-=1980;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (year>=80)
        year-=80;
    ಹಿಂತಿರುಗಿ
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local ಪೂರ್ಣಾಂಕ zip64local_getByte(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ಪೂರ್ಣಾಂಕ* pi) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c;
    ಪೂರ್ಣಾಂಕ err = (ಪೂರ್ಣಾಂಕ)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    ಆದರೆ (err==1)
    {
        *pi = (ಪೂರ್ಣಾಂಕ)c;
        ಹಿಂತಿರುಗಿ ZIP_OK;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        ಆದರೆ (ZERROR64(*pzlib_filefunc_def,filestream))
            ಹಿಂತಿರುಗಿ ZIP_ERRNO;
        ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local ಪೂರ್ಣಾಂಕ zip64local_getShort(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    ಪೂರ್ಣಾಂಕ i = 0;
    ಪೂರ್ಣಾಂಕ err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ಆದರೆ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ಆದರೆ (err==ZIP_OK)
        *pX = x;
    ಇಲ್ಲದಿದ್ದರೆ
        *pX = 0;
    ಹಿಂತಿರುಗಿ err;
}

local ಪೂರ್ಣಾಂಕ zip64local_getLong(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    ಪೂರ್ಣಾಂಕ i = 0;
    ಪೂರ್ಣಾಂಕ err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ಆದರೆ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ಆದರೆ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    ಆದರೆ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    ಆದರೆ (err==ZIP_OK)
        *pX = x;
    ಇಲ್ಲದಿದ್ದರೆ
        *pX = 0;
    ಹಿಂತಿರುಗಿ err;
}


local ಪೂರ್ಣಾಂಕ zip64local_getLong64(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  ಪೂರ್ಣಾಂಕ i = 0;
  ಪೂರ್ಣಾಂಕ err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  ಆದರೆ (err==ZIP_OK)
    *pX = x;
  ಇಲ್ಲದಿದ್ದರೆ
    *pX = 0;

  ಹಿಂತಿರುಗಿ err;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ BUFREADCOMMENT
#ವ್ಯಾಖ್ಯಾನಿಸು BUFREADCOMMENT (0x400)
#ಆದರೆ_ಅಂತ್ಯ
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_ಹುಡುಕು_Central_Dir(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* buf;
  ZPOS64_T u_ಗಾತ್ರ_File;
  ZPOS64_T u_ಹಿಂಭಾಗ_Read;
  ZPOS64_T u_Max_ಹಿಂಭಾಗ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_ಸಿಕ್ಕಿತು=0;

  ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ಕೊನೆ) != 0)
    ಹಿಂತಿರುಗಿ 0;


  u_ಗಾತ್ರ_File = ZTELL64(*pzlib_filefunc_def,filestream);

  ಆದರೆ (u_Max_ಹಿಂಭಾಗ>u_ಗಾತ್ರ_File)
    u_Max_ಹಿಂಭಾಗ = u_ಗಾತ್ರ_File;

  buf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)ALLOC(BUFREADCOMMENT+4);
  ಆದರೆ (buf==ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;

  u_ಹಿಂಭಾಗ_Read = 4;
  ಆಗ (u_ಹಿಂಭಾಗ_Read<u_Max_ಹಿಂಭಾಗ)
  {
    uLong u_Read_ಗಾತ್ರ;
    ZPOS64_T uReadPos ;
    ಪೂರ್ಣಾಂಕ i;
    ಆದರೆ (u_ಹಿಂಭಾಗ_Read+BUFREADCOMMENT>u_Max_ಹಿಂಭಾಗ)
      u_ಹಿಂಭಾಗ_Read = u_Max_ಹಿಂಭಾಗ;
    ಇಲ್ಲದಿದ್ದರೆ
      u_ಹಿಂಭಾಗ_Read+=BUFREADCOMMENT;
    uReadPos = u_ಗಾತ್ರ_File-u_ಹಿಂಭಾಗ_Read ;

    u_Read_ಗಾತ್ರ = ((BUFREADCOMMENT+4) < (u_ಗಾತ್ರ_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_ಗಾತ್ರ_File-uReadPos);
    ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
      ನಿಲ್ಲಿಸು;

    ಆದರೆ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ಗಾತ್ರ)!=u_Read_ಗಾತ್ರ)
      ನಿಲ್ಲಿಸು;

    ಗಾಗಿ (i=(ಪೂರ್ಣಾಂಕ)u_Read_ಗಾತ್ರ-3; (i--)>0;)
      ಆದರೆ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_ಸಿಕ್ಕಿತು = uReadPos+(ಚಿಹ್ನೆರಹಿತ)i;
        ನಿಲ್ಲಿಸು;
      }

    ಆದರೆ (u_Pos_ಸಿಕ್ಕಿತು!=0)
      ನಿಲ್ಲಿಸು;
  }
  ಬಿಡುಗಡೆ(buf);
  ಹಿಂತಿರುಗಿ u_Pos_ಸಿಕ್ಕಿತು;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_ಹುಡುಕು_Central_Dir64(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* buf;
  ZPOS64_T u_ಗಾತ್ರ_File;
  ZPOS64_T u_ಹಿಂಭಾಗ_Read;
  ZPOS64_T u_Max_ಹಿಂಭಾಗ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_ಸಿಕ್ಕಿತು=0;
  uLong uL;
  ZPOS64_T relative_ಸ್ಥಾನಾಂತರ;

  ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ಕೊನೆ) != 0)
    ಹಿಂತಿರುಗಿ 0;

  u_ಗಾತ್ರ_File = ZTELL64(*pzlib_filefunc_def,filestream);

  ಆದರೆ (u_Max_ಹಿಂಭಾಗ>u_ಗಾತ್ರ_File)
    u_Max_ಹಿಂಭಾಗ = u_ಗಾತ್ರ_File;

  buf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)ALLOC(BUFREADCOMMENT+4);
  ಆದರೆ (buf==ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;

  u_ಹಿಂಭಾಗ_Read = 4;
  ಆಗ (u_ಹಿಂಭಾಗ_Read<u_Max_ಹಿಂಭಾಗ)
  {
    uLong u_Read_ಗಾತ್ರ;
    ZPOS64_T uReadPos;
    ಪೂರ್ಣಾಂಕ i;
    ಆದರೆ (u_ಹಿಂಭಾಗ_Read+BUFREADCOMMENT>u_Max_ಹಿಂಭಾಗ)
      u_ಹಿಂಭಾಗ_Read = u_Max_ಹಿಂಭಾಗ;
    ಇಲ್ಲದಿದ್ದರೆ
      u_ಹಿಂಭಾಗ_Read+=BUFREADCOMMENT;
    uReadPos = u_ಗಾತ್ರ_File-u_ಹಿಂಭಾಗ_Read ;

    u_Read_ಗಾತ್ರ = ((BUFREADCOMMENT+4) < (u_ಗಾತ್ರ_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_ಗಾತ್ರ_File-uReadPos);
    ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
      ನಿಲ್ಲಿಸು;

    ಆದರೆ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ಗಾತ್ರ)!=u_Read_ಗಾತ್ರ)
      ನಿಲ್ಲಿಸು;

    ಗಾಗಿ (i=(ಪೂರ್ಣಾಂಕ)u_Read_ಗಾತ್ರ-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      ಆದರೆ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_ಸಿಕ್ಕಿತು = uReadPos+(ಚಿಹ್ನೆರಹಿತ)i;
        ನಿಲ್ಲಿಸು;
      }
    }

      ಆದರೆ (u_Pos_ಸಿಕ್ಕಿತು!=0)
        ನಿಲ್ಲಿಸು;
  }

  ಬಿಡುಗಡೆ(buf);
  ಆದರೆ (u_Pos_ಸಿಕ್ಕಿತು == 0)
    ಹಿಂತಿರುಗಿ 0;

  /* Zip64 end of central directory locator */
  ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_ಸಿಕ್ಕಿತು,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
    ಹಿಂತಿರುಗಿ 0;

  /* the signature, already checked */
  ಆದರೆ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ಹಿಂತಿರುಗಿ 0;

  /* number of the disk with the start of the zip64 end of central directory */
  ಆದರೆ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (uL != 0)
    ಹಿಂತಿರುಗಿ 0;

  /* relative offset of the zip64 end of central directory record */
  ಆದರೆ (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_ಸ್ಥಾನಾಂತರ)!=ZIP_OK)
    ಹಿಂತಿರುಗಿ 0;

  /* total number of disks */
  ಆದರೆ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (uL != 1)
    ಹಿಂತಿರುಗಿ 0;

  /* Goto Zip64 end of central directory record */
  ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream, relative_ಸ್ಥಾನಾಂತರ,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
    ಹಿಂತಿರುಗಿ 0;

  /* the signature */
  ಆದರೆ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    ಹಿಂತಿರುಗಿ 0;

  ಆದರೆ (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    ಹಿಂತಿರುಗಿ 0;

  ಹಿಂತಿರುಗಿ relative_ಸ್ಥಾನಾಂತರ;
}

local ಪೂರ್ಣಾಂಕ ತುಂಬು_Central_Directory_ದಾಖಲೆ(zip64_ಆಂತರಿಕ* pziinit) {
  ಪೂರ್ಣಾಂಕ err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T ಗಾತ್ರ_central_dir;     /* size of the central directory  */
  ZPOS64_T ಸ್ಥಾನಾಂತರ_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong ಸಂಖ್ಯೆ_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong ಸಂಖ್ಯೆ_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T ಸಂಖ್ಯೆ_ನಮೂದು;
  ZPOS64_T ಸಂಖ್ಯೆ_ನಮೂದು_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong ಆವೃತ್ತಿ_Made_By;
  uLong ಆವೃತ್ತಿ_Needed;
  uLong ಗಾತ್ರ_comment;

  ಪೂರ್ಣಾಂಕ has_ZIP64_ದಾಖಲೆ = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_ಹುಡುಕು_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  ಆದರೆ(central_pos > 0)
  {
    has_ZIP64_ದಾಖಲೆ = 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(central_pos == 0)
  {
    central_pos = zip64local_ಹುಡುಕು_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  ಆದರೆ(has_ZIP64_ದಾಖಲೆ)
  {
    ZPOS64_T ಗಾತ್ರ_ಕೊನೆ_ದ_Central_Directory;
    ಆದರೆ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_ಗಣ) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ಆದರೆ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    ಆದರೆ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &ಗಾತ್ರ_ಕೊನೆ_ದ_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    ಆದರೆ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &ಆವೃತ್ತಿ_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    ಆದರೆ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &ಆವೃತ್ತಿ_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ಆದರೆ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&ಸಂಖ್ಯೆ_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ಆದರೆ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&ಸಂಖ್ಯೆ_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    ಆದರೆ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &ಸಂಖ್ಯೆ_ನಮೂದು)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    ಆದರೆ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ಸಂಖ್ಯೆ_ನಮೂದು_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    ಆದರೆ ((ಸಂಖ್ಯೆ_ನಮೂದು_CD!=ಸಂಖ್ಯೆ_ನಮೂದು) || (ಸಂಖ್ಯೆ_disk_with_CD!=0) || (ಸಂಖ್ಯೆ_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ಆದರೆ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ಗಾತ್ರ_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    ಆದರೆ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ಸ್ಥಾನಾಂತರ_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    ಗಾತ್ರ_comment = 0;
  }
  ಇಲ್ಲದಿದ್ದರೆ
  {
    /* Read End of central Directory info */
    ಆದರೆ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ಆದರೆ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ಆದರೆ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&ಸಂಖ್ಯೆ_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ಆದರೆ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&ಸಂಖ್ಯೆ_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    ಸಂಖ್ಯೆ_ನಮೂದು = 0;
    ಆದರೆ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ
      ಸಂಖ್ಯೆ_ನಮೂದು = uL;

    /* total number of entries in the central dir */
    ಸಂಖ್ಯೆ_ನಮೂದು_CD = 0;
    ಆದರೆ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ
      ಸಂಖ್ಯೆ_ನಮೂದು_CD = uL;

    ಆದರೆ ((ಸಂಖ್ಯೆ_ನಮೂದು_CD!=ಸಂಖ್ಯೆ_ನಮೂದು) || (ಸಂಖ್ಯೆ_disk_with_CD!=0) || (ಸಂಖ್ಯೆ_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ಗಾತ್ರ_central_dir = 0;
    ಆದರೆ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ
      ಗಾತ್ರ_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    ಸ್ಥಾನಾಂತರ_central_dir = 0;
    ಆದರೆ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ
      ಸ್ಥಾನಾಂತರ_central_dir = uL;


    /* zipfile global comment length */
    ಆದರೆ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &ಗಾತ್ರ_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  ಆದರೆ ((central_pos<ಸ್ಥಾನಾಂತರ_central_dir+ಗಾತ್ರ_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  ಆದರೆ (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    ಹಿಂತಿರುಗಿ ZIP_ERRNO;
  }

  ಆದರೆ (ಗಾತ್ರ_comment>0)
  {
    pziinit->globalcomment = (ಅಕ್ಷರ*)ALLOC(ಗಾತ್ರ_comment+1);
    ಆದರೆ (pziinit->globalcomment)
    {
      ಗಾತ್ರ_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,ಗಾತ್ರ_comment);
      pziinit->globalcomment[ಗಾತ್ರ_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (ಸ್ಥಾನಾಂತರ_central_dir+ಗಾತ್ರ_central_dir);
  pziinit->ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ = byte_before_the_zipfile;

  {
    ZPOS64_T ಗಾತ್ರ_central_dir_ಗಮ್ಯ_read = ಗಾತ್ರ_central_dir;
    size_t buf_ಗಾತ್ರ_2 = SIZEDATA_INDATABLOCK;
    ಶೂನ್ಯ* buf_read = (ಶೂನ್ಯ*)ALLOC(buf_ಗಾತ್ರ_2);
    ಆದರೆ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, ಸ್ಥಾನಾಂತರ_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_ಗಣ) != 0)
      err=ZIP_ERRNO;

    ಆಗ ((ಗಾತ್ರ_central_dir_ಗಮ್ಯ_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      ಆದರೆ (read_this > ಗಾತ್ರ_central_dir_ಗಮ್ಯ_read)
        read_this = ಗಾತ್ರ_central_dir_ಗಮ್ಯ_read;

      ಆದರೆ (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      ಆದರೆ (err==ZIP_OK)
        err = ಕೂಡಿಸು_ದತ್ತಾಂಶ_ಒಳ_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      ಗಾತ್ರ_central_dir_ಗಮ್ಯ_read-=read_this;
    }
    ಬಿಡುಗಡೆ(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->ಸಂಖ್ಯೆ_ನಮೂದು = ಸಂಖ್ಯೆ_ನಮೂದು_CD;

  ಆದರೆ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, ಸ್ಥಾನಾಂತರ_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_ಗಣ) != 0)
    err=ZIP_ERRNO;

  ಹಿಂತಿರುಗಿ err;
}


#ಆದರೆ_ಅಂತ್ಯ /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
ಬಾಹ್ಯ zipFile ZEXPORT zipOpen3(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pathname, ಪೂರ್ಣಾಂಕ append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_ಆಂತರಿಕ ziinit;
    zip64_ಆಂತರಿಕ* zi;
    ಪೂರ್ಣಾಂಕ err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = ಖಾಲಿ;
    ziinit.z_filefunc.ztell32_file = ಖಾಲಿ;
    ಆದರೆ (pzlib_filefunc64_32_def==ಖಾಲಿ)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    ಇಲ್ಲದಿದ್ದರೆ
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_ಸ್ಥಿತಿ_CREATE) ?
                  (ZLIB_FILEFUNC_ಬಹುಲಕ_READ | ZLIB_FILEFUNC_ಬಹುಲಕ_WRITE | ZLIB_FILEFUNC_ಬಹುಲಕ_CREATE) :
                    (ZLIB_FILEFUNC_ಬಹುಲಕ_READ | ZLIB_FILEFUNC_ಬಹುಲಕ_WRITE | ZLIB_FILEFUNC_ಬಹುಲಕ_EXISTING));

    ಆದರೆ (ziinit.filestream == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    ಆದರೆ (append == APPEND_ಸ್ಥಿತಿ_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.ಒಳ_opened_file_inzip = 0;
    ziinit.ci.ಹರಿವು_initialised = 0;
    ziinit.ಸಂಖ್ಯೆ_ನಮೂದು = 0;
    ziinit.ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.ಗಣ, 0, ಗಾತ್ರ(ಗಣ_t));  /* make sure set appears dormant */



    zi = (zip64_ಆಂತರಿಕ*)ALLOC(ಗಾತ್ರ(zip64_ಆಂತರಿಕ));
    ಆದರೆ (zi==ಖಾಲಿ)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = ಖಾಲಿ;
    ಆದರೆ (append == APPEND_ಸ್ಥಿತಿ_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = ತುಂಬು_Central_Directory_ದಾಖಲೆ(&ziinit);
    }

    ಆದರೆ (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    ಆದರೆ (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        ಬಿಡುಗಡೆ(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        ಬಿಡುಗಡೆ(zi);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        *zi = ziinit;
        ಹಿಂತಿರುಗಿ (zipFile)zi;
    }
}

ಬಾಹ್ಯ zipFile ZEXPORT zipOpen2(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname, ಪೂರ್ಣಾಂಕ append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    ಆದರೆ (pzlib_filefunc32_def != ಖಾಲಿ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        ಹಿಂತಿರುಗಿ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ zipOpen3(pathname, append, globalcomment, ಖಾಲಿ);
}

ಬಾಹ್ಯ zipFile ZEXPORT zipOpen2_64(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pathname, ಪೂರ್ಣಾಂಕ append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    ಆದರೆ (pzlib_filefunc_def != ಖಾಲಿ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ಖಾಲಿ;
        zlib_filefunc64_32_def_fill.ztell32_file = ಖಾಲಿ;
        zlib_filefunc64_32_def_fill.zseek32_file = ಖಾಲಿ;
        ಹಿಂತಿರುಗಿ zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ zipOpen3(pathname, append, globalcomment, ಖಾಲಿ);
}



ಬಾಹ್ಯ zipFile ZEXPORT zipOpen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* pathname, ಪೂರ್ಣಾಂಕ append) {
    ಹಿಂತಿರುಗಿ zipOpen3((ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*)pathname,append,ಖಾಲಿ,ಖಾಲಿ);
}

ಬಾಹ್ಯ zipFile ZEXPORT zipOpen64(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* pathname, ಪೂರ್ಣಾಂಕ append) {
    ಹಿಂತಿರುಗಿ zipOpen3(pathname,append,ಖಾಲಿ,ಖಾಲಿ);
}

local ಪೂರ್ಣಾಂಕ Write_Local_File_ಶೀರ್ಷಿಕೆ(zip64_ಆಂತರಿಕ* zi, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, uInt ಗಾತ್ರ_extrafield_local, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local) {
  /* write the local header */
  ಪೂರ್ಣಾಂಕ err;
  uInt ಗಾತ್ರ_filename = (uInt)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(filename);
  uInt ಗಾತ್ರ_extrafield = ಗಾತ್ರ_extrafield_local;

  err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  ಆದರೆ (err==ZIP_OK)
  {
    ಆದರೆ(zi->ci.zip64)
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    ಇಲ್ಲದಿದ್ದರೆ
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.ವಿಧಾನ_2,2);

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  ಆದರೆ (err==ZIP_OK)
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  ಆದರೆ (err==ZIP_OK)
  {
    ಆದರೆ(zi->ci.zip64)
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    ಇಲ್ಲದಿದ್ದರೆ
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  ಆದರೆ (err==ZIP_OK)
  {
    ಆದರೆ(zi->ci.zip64)
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    ಇಲ್ಲದಿದ್ದರೆ
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)ಗಾತ್ರ_filename,2);

  ಆದರೆ(zi->ci.zip64)
  {
    ಗಾತ್ರ_extrafield += 20;
  }

  ಆದರೆ (err==ZIP_OK)
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)ಗಾತ್ರ_extrafield,2);

  ಆದರೆ ((err==ZIP_OK) && (ಗಾತ್ರ_filename > 0))
  {
    ಆದರೆ (ZWRITE64(zi->z_filefunc,zi->filestream,filename,ಗಾತ್ರ_filename)!=ಗಾತ್ರ_filename)
      err = ZIP_ERRNO;
  }

  ಆದರೆ ((err==ZIP_OK) && (ಗಾತ್ರ_extrafield_local > 0))
  {
    ಆದರೆ (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, ಗಾತ್ರ_extrafield_local) != ಗಾತ್ರ_extrafield_local)
      err = ZIP_ERRNO;
  }


  ಆದರೆ ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      ಗಿಡ್ಡ ಶೀರ್ಷಿಕೆ_ID = 1;
      ಗಿಡ್ಡ ದತ್ತಾಂಶ_ಗಾತ್ರ_2 = 16;
      ZPOS64_T Compressed_ಗಾತ್ರ = 0;
      ZPOS64_T Uncompressed_ಗಾತ್ರ = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)ಶೀರ್ಷಿಕೆ_ID,2);
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)ದತ್ತಾಂಶ_ಗಾತ್ರ_2,2);

      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_ಗಾತ್ರ,8);
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_ಗಾತ್ರ,8);
  }

  ಹಿಂತಿರುಗಿ err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local ಪೂರ್ಣಾಂಕ utf8len(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *str, size_t len) {
    ಹಿಂತಿರುಗಿ
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
local ಪೂರ್ಣಾಂಕ isutf8(ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *str, size_t len) {
    ಪೂರ್ಣಾಂಕ utf8 = 0;
    ಆಗ (len) {
        ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = utf8len((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *)str, len);
        ಆದರೆ (ಸಂಕೇತ_2 < 0)
            ಹಿಂತಿರುಗಿ 0;
        ಆದರೆ (ಸಂಕೇತ_2 > 1)
            utf8 = 1;
        str += ಸಂಕೇತ_2;
        len -= (ಚಿಹ್ನೆರಹಿತ)ಸಂಕೇತ_2;
    }
    ಹಿಂತಿರುಗಿ utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip4_64(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ raw,
                                           ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,ಪೂರ್ಣಾಂಕ memLevel, ಪೂರ್ಣಾಂಕ strategy,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password, uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting,
                                           uLong ಆವೃತ್ತಿ_Made_By_2, uLong flag_ತಳ, ಪೂರ್ಣಾಂಕ zip64) {
    zip64_ಆಂತರಿಕ* zi;
    uInt ಗಾತ್ರ_filename;
    uInt ಗಾತ್ರ_comment;
    uInt i;
    ಪೂರ್ಣಾಂಕ err = ZIP_OK;

#    ifdef NOCRYPT
    ಆದರೆ (password != ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
#    endif

    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    ಆದರೆ ((ವಿಧಾನ_2!=0) && (ವಿಧಾನ_2!=Z_DEFLATED) && (ವಿಧಾನ_2!=Z_BZIP2ED))
      ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((ವಿಧಾನ_2!=0) && (ವಿಧಾನ_2!=Z_DEFLATED))
      ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
#ಆದರೆ_ಅಂತ್ಯ

    /* The filename and comment length must fit in 16 bits. */
    ಆದರೆ ((filename!=ಖಾಲಿ) && (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(filename)>0xffff))
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
    ಆದರೆ ((comment!=ಖಾಲಿ) && (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(comment)>0xffff))
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    ಆದರೆ ((ಗಾತ್ರ_extrafield_local>0xffff) || (ಗಾತ್ರ_extrafield_global>0xffff))
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;

    zi = (zip64_ಆಂತರಿಕ*)file;

    ಆದರೆ (zi->ಒಳ_opened_file_inzip == 1)
    {
        err = zip_Close_File_ಒಳ_Zip (file);
        ಆದರೆ (err != ZIP_OK)
            ಹಿಂತಿರುಗಿ err;
    }

    ಆದರೆ (filename==ಖಾಲಿ)
        filename="-";

    ಆದರೆ (comment==ಖಾಲಿ)
        ಗಾತ್ರ_comment = 0;
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಾತ್ರ_comment = (uInt)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(comment);

    ಗಾತ್ರ_filename = (uInt)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(filename);

    ಆದರೆ (zipfi == ಖಾಲಿ)
        zi->ci.dosDate = 0;
    ಇಲ್ಲದಿದ್ದರೆ
    {
        ಆದರೆ (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        ಇಲ್ಲದಿದ್ದರೆ
          zi->ci.dosDate = zip64local_Tmz_Date_ಗಮ್ಯ_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_ತಳ;
    ಆದರೆ ((level==8) || (level==9))
      zi->ci.flag |= 2;
    ಆದರೆ (level==2)
      zi->ci.flag |= 4;
    ಆದರೆ (level==1)
      zi->ci.flag |= 6;
    ಆದರೆ (password != ಖಾಲಿ)
      zi->ci.flag |= 1;
    ಆದರೆ (isutf8(filename, ಗಾತ್ರ_filename) &&
        (ಗಾತ್ರ_comment == 0 || isutf8(comment, ಗಾತ್ರ_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2 = 0;
    zi->ci.ವಿಧಾನ_2 = ವಿಧಾನ_2;
    zi->ci.encrypt = 0;
    zi->ci.ಹರಿವು_initialised = 0;
    zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_ಶೀರ್ಷಿಕೆ = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.ಗಾತ್ರ_centralheader = SIZECENTRALHEADER + ಗಾತ್ರ_filename + ಗಾತ್ರ_extrafield_global + ಗಾತ್ರ_comment;
    zi->ci.ಗಾತ್ರ_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_ಶೀರ್ಷಿಕೆ = (ಅಕ್ಷರ*)ALLOC((uInt)zi->ci.ಗಾತ್ರ_centralheader + zi->ci.ಗಾತ್ರ_central_Extra_Free);

    zi->ci.ಗಾತ್ರ_central_Extra = ಗಾತ್ರ_extrafield_global;
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+4,(uLong)ಆವೃತ್ತಿ_Made_By_2,2);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+6,(uLong)20,2);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+8,(uLong)zi->ci.flag,2);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+10,(uLong)zi->ci.ವಿಧಾನ_2,2);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+16,(uLong)0,4); /*crc*/
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+20,(uLong)0,4); /*compr size*/
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+28,(uLong)ಗಾತ್ರ_filename,2);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+30,(uLong)ಗಾತ್ರ_extrafield_global,2);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+32,(uLong)ಗಾತ್ರ_comment,2);
    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+34,(uLong)0,2); /*disk nm start*/

    ಆದರೆ (zipfi==ಖಾಲಿ)
        zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+36,(uLong)0,2);
    ಇಲ್ಲದಿದ್ದರೆ
        zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+36,(uLong)zipfi->ಆಂತರಿಕ_fa,2);

    ಆದರೆ (zipfi==ಖಾಲಿ)
        zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+38,(uLong)0,4);
    ಇಲ್ಲದಿದ್ದರೆ
        zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+38,(uLong)zipfi->external_fa,4);

    ಆದರೆ(zi->ci.pos_local_ಶೀರ್ಷಿಕೆ >= 0xffffffff)
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+42,(uLong)0xffffffff,4);
    ಇಲ್ಲದಿದ್ದರೆ
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+42,(uLong)zi->ci.pos_local_ಶೀರ್ಷಿಕೆ - zi->ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ,4);

    ಗಾಗಿ (i=0;i<ಗಾತ್ರ_filename;i++)
        *(zi->ci.central_ಶೀರ್ಷಿಕೆ+SIZECENTRALHEADER+i) = *(filename+i);

    ಗಾಗಿ (i=0;i<ಗಾತ್ರ_extrafield_global;i++)
        *(zi->ci.central_ಶೀರ್ಷಿಕೆ+SIZECENTRALHEADER+ಗಾತ್ರ_filename+i) =
              *(((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)extrafield_global)+i);

    ಗಾಗಿ (i=0;i<ಗಾತ್ರ_comment;i++)
        *(zi->ci.central_ಶೀರ್ಷಿಕೆ+SIZECENTRALHEADER+ಗಾತ್ರ_filename+
              ಗಾತ್ರ_extrafield_global+i) = *(comment+i);
    ಆದರೆ (zi->ci.central_ಶೀರ್ಷಿಕೆ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.ಒಟ್ಟು_Compressed_ದತ್ತಾಂಶ = 0;
    zi->ci.ಒಟ್ಟು_Uncompressed_ದತ್ತಾಂಶ = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_ಶೀರ್ಷಿಕೆ(zi, filename, ಗಾತ್ರ_extrafield_local, extrafield_local);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    zi->ci.bstream.avail_ಒಳ = (uInt)0;
    zi->ci.bstream.avail_ಹೊರಹೋಗು = (uInt)Z_BUFSIZE;
    zi->ci.bstream.ಮುಂದಿನ_ಹೊರಹೋಗು = (ಅಕ್ಷರ*)zi->ci.buffered_ದತ್ತಾಂಶ;
    zi->ci.bstream.ಒಟ್ಟು_ಒಳ_hi32 = 0;
    zi->ci.bstream.ಒಟ್ಟು_ಒಳ_lo32 = 0;
    zi->ci.bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_hi32 = 0;
    zi->ci.bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32 = 0;
#ಆದರೆ_ಅಂತ್ಯ

    zi->ci.ಹರಿವು.avail_ಒಳ = (uInt)0;
    zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)Z_BUFSIZE;
    zi->ci.ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = zi->ci.buffered_ದತ್ತಾಂಶ;
    zi->ci.ಹರಿವು.ಒಟ್ಟು_ಒಳ = 0;
    zi->ci.ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು = 0;
    zi->ci.ಹರಿವು.ದತ್ತಾಂಶ_type = Z_ದ್ವಿಮಾನ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    ಆದರೆ ((err==ZIP_OK) && (zi->ci.ವಿಧಾನ_2 == Z_DEFLATED || zi->ci.ವಿಧಾನ_2 == Z_BZIP2ED) && (!zi->ci.raw))
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((err==ZIP_OK) && (zi->ci.ವಿಧಾನ_2 == Z_DEFLATED) && (!zi->ci.raw))
#ಆದರೆ_ಅಂತ್ಯ
    {
        ಆದರೆ(zi->ci.ವಿಧಾನ_2 == Z_DEFLATED)
        {
          zi->ci.ಹರಿವು.zalloc = (alloc_func)0;
          zi->ci.ಹರಿವು.zfree = (free_func)0;
          zi->ci.ಹರಿವು.opaque = (voidpf)0;

          ಆದರೆ (ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು>0)
              ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು = -ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು;

          err = deflateInit2(&zi->ci.ಹರಿವು, level, Z_DEFLATED, ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು, memLevel, strategy);

          ಆದರೆ (err==Z_OK)
              zi->ci.ಹರಿವು_initialised = Z_DEFLATED;
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(zi->ci.ವಿಧಾನ_2 == Z_BZIP2ED)
        {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          ಆದರೆ(err == BZ_OK)
            zi->ci.ಹರಿವು_initialised = Z_BZIP2ED;
#ಆದರೆ_ಅಂತ್ಯ
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ = 0;
    ಆದರೆ ((err==Z_OK) && (password != ಖಾಲಿ))
    {
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf_ತಲೆ[RAND_ತಲೆ_LEN];
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_ತಲೆ;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        ಗಾತ್ರ_ತಲೆ=crypthead(password,buf_ತಲೆ,RAND_ತಲೆ_LEN,zi->ci.ಕೀಲಿಗಳು,zi->ci.pcrc_32_tab,ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting);
        zi->ci.crypt_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ = ಗಾತ್ರ_ತಲೆ;

        ಆದರೆ (ZWRITE64(zi->z_filefunc,zi->filestream,buf_ತಲೆ,ಗಾತ್ರ_ತಲೆ) != ಗಾತ್ರ_ತಲೆ)
                err = ZIP_ERRNO;
    }
#    endif

    ಆದರೆ (err==Z_OK)
        zi->ಒಳ_opened_file_inzip = 1;
    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip4(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ raw,
                                        ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,ಪೂರ್ಣಾಂಕ memLevel, ಪೂರ್ಣಾಂಕ strategy,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password, uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting,
                                        uLong ಆವೃತ್ತಿ_Made_By_2, uLong flag_ತಳ) {
    ಹಿಂತಿರುಗಿ zip_Open_New_File_ಒಳ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ಗಾತ್ರ_extrafield_local,
                                   extrafield_global, ಗಾತ್ರ_extrafield_global,
                                   comment, ವಿಧಾನ_2, level, raw,
                                   ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು, memLevel, strategy,
                                   password, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting, ಆವೃತ್ತಿ_Made_By_2, flag_ತಳ, 0);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip3(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ raw,
                                        ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,ಪೂರ್ಣಾಂಕ memLevel, ಪೂರ್ಣಾಂಕ strategy,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password, uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting) {
    ಹಿಂತಿರುಗಿ zip_Open_New_File_ಒಳ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ಗಾತ್ರ_extrafield_local,
                                   extrafield_global, ಗಾತ್ರ_extrafield_global,
                                   comment, ವಿಧಾನ_2, level, raw,
                                   ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು, memLevel, strategy,
                                   password, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting, VERSIONMADEBY, 0, 0);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip3_64(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                         ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ raw,
                                         ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,ಪೂರ್ಣಾಂಕ memLevel, ಪೂರ್ಣಾಂಕ strategy,
                                         ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password, uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting, ಪೂರ್ಣಾಂಕ zip64) {
    ಹಿಂತಿರುಗಿ zip_Open_New_File_ಒಳ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ಗಾತ್ರ_extrafield_local,
                                   extrafield_global, ಗಾತ್ರ_extrafield_global,
                                   comment, ವಿಧಾನ_2, level, raw,
                                   ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು, memLevel, strategy,
                                   password, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting, VERSIONMADEBY, 0, zip64);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip2(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ raw) {
    ಹಿಂತಿರುಗಿ zip_Open_New_File_ಒಳ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ಗಾತ್ರ_extrafield_local,
                                   extrafield_global, ಗಾತ್ರ_extrafield_global,
                                   comment, ವಿಧಾನ_2, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ಖಾಲಿ, 0, VERSIONMADEBY, 0, 0);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip2_64(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ raw, ಪೂರ್ಣಾಂಕ zip64) {
    ಹಿಂತಿರುಗಿ zip_Open_New_File_ಒಳ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ಗಾತ್ರ_extrafield_local,
                                   extrafield_global, ಗಾತ್ರ_extrafield_global,
                                   comment, ವಿಧಾನ_2, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ಖಾಲಿ, 0, VERSIONMADEBY, 0, zip64);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip64(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                         ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level, ಪೂರ್ಣಾಂಕ zip64) {
    ಹಿಂತಿರುಗಿ zip_Open_New_File_ಒಳ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ಗಾತ್ರ_extrafield_local,
                                   extrafield_global, ಗಾತ್ರ_extrafield_global,
                                   comment, ವಿಧಾನ_2, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ಖಾಲಿ, 0, VERSIONMADEBY, 0, zip64);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                       ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local, uInt ಗಾತ್ರ_extrafield_local,
                                       ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*extrafield_global, uInt ಗಾತ್ರ_extrafield_global,
                                       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment, ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ level) {
    ಹಿಂತಿರುಗಿ zip_Open_New_File_ಒಳ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ಗಾತ್ರ_extrafield_local,
                                   extrafield_global, ಗಾತ್ರ_extrafield_global,
                                   comment, ವಿಧಾನ_2, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ಖಾಲಿ, 0, VERSIONMADEBY, 0, 0);
}

local ಪೂರ್ಣಾಂಕ zip64_Flush_Write_ಬಫರ್(zip64_ಆಂತರಿಕ* zi) {
    ಪೂರ್ಣಾಂಕ err=ZIP_OK;

    ಆದರೆ (zi->ci.encrypt != 0)
    {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NOCRYPT
        uInt i;
        ಪೂರ್ಣಾಂಕ t;
        ಗಾಗಿ (i=0;i<zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ;i++)
            zi->ci.buffered_ದತ್ತಾಂಶ[i] = zencode(zi->ci.ಕೀಲಿಗಳು, zi->ci.pcrc_32_tab, zi->ci.buffered_ದತ್ತಾಂಶ[i],t);
#ಆದರೆ_ಅಂತ್ಯ
    }

    ಆದರೆ (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_ದತ್ತಾಂಶ,zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ) != zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ)
      err = ZIP_ERRNO;

    zi->ci.ಒಟ್ಟು_Compressed_ದತ್ತಾಂಶ += zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    ಆದರೆ(zi->ci.ವಿಧಾನ_2 == Z_BZIP2ED)
    {
      zi->ci.ಒಟ್ಟು_Uncompressed_ದತ್ತಾಂಶ += zi->ci.bstream.ಒಟ್ಟು_ಒಳ_lo32;
      zi->ci.bstream.ಒಟ್ಟು_ಒಳ_lo32 = 0;
      zi->ci.bstream.ಒಟ್ಟು_ಒಳ_hi32 = 0;
    }
    ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      zi->ci.ಒಟ್ಟು_Uncompressed_ದತ್ತಾಂಶ += zi->ci.ಹರಿವು.ಒಟ್ಟು_ಒಳ;
      zi->ci.ಹರಿವು.ಒಟ್ಟು_ಒಳ = 0;
    }


    zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ = 0;

    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Write_ಒಳ_File_ಒಳ_Zip(zipFile file, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ len) {
    zip64_ಆಂತರಿಕ* zi;
    ಪೂರ್ಣಾಂಕ err=ZIP_OK;

    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
    zi = (zip64_ಆಂತರಿಕ*)file;

    ಆದರೆ (zi->ಒಳ_opened_file_inzip == 0)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;

    zi->ci.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(zi->ci.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2,buf,(uInt)len);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    ಆದರೆ(zi->ci.ವಿಧಾನ_2 == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.ಮುಂದಿನ_ಒಳ = (ಶೂನ್ಯ*)buf;
      zi->ci.bstream.avail_ಒಳ = len;
      err = BZ_ಸತತ_OK;

      ಆಗ ((err==BZ_ಸತತ_OK) && (zi->ci.bstream.avail_ಒಳ>0))
      {
        ಆದರೆ (zi->ci.bstream.avail_ಹೊರಹೋಗು == 0)
        {
          ಆದರೆ (zip64_Flush_Write_ಬಫರ್(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_ಹೊರಹೋಗು = (uInt)Z_BUFSIZE;
          zi->ci.bstream.ಮುಂದಿನ_ಹೊರಹೋಗು = (ಅಕ್ಷರ*)zi->ci.buffered_ದತ್ತಾಂಶ;
        }


        ಆದರೆ(err != BZ_ಸತತ_OK)
          ನಿಲ್ಲಿಸು;

        ಆದರೆ ((zi->ci.ವಿಧಾನ_2 == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before_lo = zi->ci.bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_ಸತತ);

          zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ += (uInt)(zi->ci.bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32 - u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before_lo) ;
        }
      }

      ಆದರೆ(err == BZ_ಸತತ_OK)
        err = ZIP_OK;
    }
    ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      zi->ci.ಹರಿವು.ಮುಂದಿನ_ಒಳ = buf;
      zi->ci.ಹರಿವು.avail_ಒಳ = len;

      ಆಗ ((err==ZIP_OK) && (zi->ci.ಹರಿವು.avail_ಒಳ>0))
      {
          ಆದರೆ (zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು == 0)
          {
              ಆದರೆ (zip64_Flush_Write_ಬಫರ್(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)Z_BUFSIZE;
              zi->ci.ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = zi->ci.buffered_ದತ್ತಾಂಶ;
          }


          ಆದರೆ(err != ZIP_OK)
              ನಿಲ್ಲಿಸು;

          ಆದರೆ ((zi->ci.ವಿಧಾನ_2 == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before = zi->ci.ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು;
              err=deflate(&zi->ci.ಹರಿವು,  Z_NO_FLUSH);

              zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ += (uInt)(zi->ci.ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು - u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before) ;
          }
          ಇಲ್ಲದಿದ್ದರೆ
          {
              uInt copy_this,i;
              ಆದರೆ (zi->ci.ಹರಿವು.avail_ಒಳ < zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು)
                  copy_this = zi->ci.ಹರಿವು.avail_ಒಳ;
              ಇಲ್ಲದಿದ್ದರೆ
                  copy_this = zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು;

              ಗಾಗಿ (i = 0; i < copy_this; i++)
                  *(((ಅಕ್ಷರ*)zi->ci.ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು)+i) =
                      *(((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)zi->ci.ಹರಿವು.ಮುಂದಿನ_ಒಳ)+i);
              {
                  zi->ci.ಹರಿವು.avail_ಒಳ -= copy_this;
                  zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು-= copy_this;
                  zi->ci.ಹರಿವು.ಮುಂದಿನ_ಒಳ+= copy_this;
                  zi->ci.ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು+= copy_this;
                  zi->ci.ಹರಿವು.ಒಟ್ಟು_ಒಳ+= copy_this;
                  zi->ci.ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು+= copy_this;
                  zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ += copy_this;
              }
          }
      }/* while(...) */
    }

    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Close_File_ಒಳ_Zip_Raw(zipFile file, uLong uncompressed_ಗಾತ್ರ, uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2) {
    ಹಿಂತಿರುಗಿ zip_Close_File_ಒಳ_Zip_Raw64 (file, uncompressed_ಗಾತ್ರ, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Close_File_ಒಳ_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_ಗಾತ್ರ, uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2) {
    zip64_ಆಂತರಿಕ* zi;
    ZPOS64_T compressed_ಗಾತ್ರ;
    uLong invalid_ಮೌಲ್ಯ = 0xffffffff;
    ಚಿಹ್ನೆರಹಿತ datasize = 0;
    ಪೂರ್ಣಾಂಕ err=ZIP_OK;

    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
    zi = (zip64_ಆಂತರಿಕ*)file;

    ಆದರೆ (zi->ಒಳ_opened_file_inzip == 0)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;
    zi->ci.ಹರಿವು.avail_ಒಳ = 0;

    ಆದರೆ ((zi->ci.ವಿಧಾನ_2 == Z_DEFLATED) && (!zi->ci.raw))
                {
                        ಆಗ (err==ZIP_OK)
                        {
                                uLong u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before;
                                ಆದರೆ (zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು == 0)
                                {
                                        ಆದರೆ (zip64_Flush_Write_ಬಫರ್(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)Z_BUFSIZE;
                                        zi->ci.ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = zi->ci.buffered_ದತ್ತಾಂಶ;
                                }
                                u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before = zi->ci.ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು;
                                err=deflate(&zi->ci.ಹರಿವು,  Z_ಮುಕ್ತಾಯ);
                                zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ += (uInt)(zi->ci.ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು - u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before) ;
                        }
                }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((zi->ci.ವಿಧಾನ_2 == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
      err = BZ_ಮುಕ್ತಾಯ_OK;
      ಆಗ (err==BZ_ಮುಕ್ತಾಯ_OK)
      {
        uLong u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before;
        ಆದರೆ (zi->ci.bstream.avail_ಹೊರಹೋಗು == 0)
        {
          ಆದರೆ (zip64_Flush_Write_ಬಫರ್(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_ಹೊರಹೋಗು = (uInt)Z_BUFSIZE;
          zi->ci.bstream.ಮುಂದಿನ_ಹೊರಹೋಗು = (ಅಕ್ಷರ*)zi->ci.buffered_ದತ್ತಾಂಶ;
        }
        u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before = zi->ci.bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_ಮುಕ್ತಾಯ);
        ಆದರೆ(err == BZ_ಹರಿವು_ಕೊನೆ)
          err = Z_ಹರಿವು_ಕೊನೆ;

        zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ += (uInt)(zi->ci.bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32 - u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before);
      }

      ಆದರೆ(err == BZ_ಮುಕ್ತಾಯ_OK)
        err = ZIP_OK;
#ಆದರೆ_ಅಂತ್ಯ
    }

    ಆದರೆ (err==Z_ಹರಿವು_ಕೊನೆ)
        err=ZIP_OK; /* this is normal */

    ಆದರೆ ((zi->ci.pos_ಒಳ_buffered_ದತ್ತಾಂಶ>0) && (err==ZIP_OK))
                {
        ಆದರೆ (zip64_Flush_Write_ಬಫರ್(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    ಆದರೆ ((zi->ci.ವಿಧಾನ_2 == Z_DEFLATED) && (!zi->ci.raw))
    {
        ಪೂರ್ಣಾಂಕ tmp_err = deflate_ಕೊನೆ(&zi->ci.ಹರಿವು);
        ಆದರೆ (err == ZIP_OK)
            err = tmp_err;
        zi->ci.ಹರಿವು_initialised = 0;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ((zi->ci.ವಿಧಾನ_2 == Z_BZIP2ED) && (!zi->ci.raw))
    {
      ಪೂರ್ಣಾಂಕ tmperr = BZ2_bz_Compress_ಕೊನೆ(&zi->ci.bstream);
                        ಆದರೆ (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.ಹರಿವು_initialised = 0;
    }
#ಆದರೆ_ಅಂತ್ಯ

    ಆದರೆ (!zi->ci.raw)
    {
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2 = (uLong)zi->ci.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2;
        uncompressed_ಗಾತ್ರ = zi->ci.ಒಟ್ಟು_Uncompressed_ದತ್ತಾಂಶ;
    }
    compressed_ಗಾತ್ರ = zi->ci.ಒಟ್ಟು_Compressed_ದತ್ತಾಂಶ;

#    ifndef NOCRYPT
    compressed_ಗಾತ್ರ += zi->ci.crypt_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
#    endif

    /* update Current Item crc and sizes, */
    ಆದರೆ(compressed_ಗಾತ್ರ >= 0xffffffff || uncompressed_ಗಾತ್ರ >= 0xffffffff || zi->ci.pos_local_ಶೀರ್ಷಿಕೆ >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+6,(uLong)45,2);

    }

    zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+16,ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2,4); /*crc*/


    ಆದರೆ(compressed_ಗಾತ್ರ >= 0xffffffff)
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+20, invalid_ಮೌಲ್ಯ,4); /*compr size*/
    ಇಲ್ಲದಿದ್ದರೆ
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+20, compressed_ಗಾತ್ರ,4); /*compr size*/

    /* set internal file attributes field */
    ಆದರೆ (zi->ci.ಹರಿವು.ದತ್ತಾಂಶ_type == Z_ASCII)
        zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+36,(uLong)Z_ASCII,2);

    ಆದರೆ(uncompressed_ಗಾತ್ರ >= 0xffffffff)
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+24, invalid_ಮೌಲ್ಯ,4); /*uncompr size*/
    ಇಲ್ಲದಿದ್ದರೆ
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+24, uncompressed_ಗಾತ್ರ,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    ಆದರೆ(uncompressed_ಗಾತ್ರ >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    ಆದರೆ(compressed_ಗಾತ್ರ >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    ಆದರೆ(zi->ci.pos_local_ಶೀರ್ಷಿಕೆ >= 0xffffffff)
      datasize += 8;

    ಆದರೆ(datasize > 0)
    {
      ಅಕ್ಷರ* p = ಖಾಲಿ;

      ಆದರೆ((uLong)(datasize + 4) > zi->ci.ಗಾತ್ರ_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        ಹಿಂತಿರುಗಿ ZIP_BADZIPFILE;
      }

      p = zi->ci.central_ಶೀರ್ಷಿಕೆ + zi->ci.ಗಾತ್ರ_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_ಮೌಲ್ಯ_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_ಮೌಲ್ಯ_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      ಆದರೆ(uncompressed_ಗಾತ್ರ >= 0xffffffff)
      {
        zip64local_put_ಮೌಲ್ಯ_inmemory(p, uncompressed_ಗಾತ್ರ, 8);
        p += 8;
      }

      ಆದರೆ(compressed_ಗಾತ್ರ >= 0xffffffff)
      {
        zip64local_put_ಮೌಲ್ಯ_inmemory(p, compressed_ಗಾತ್ರ, 8);
        p += 8;
      }

      ಆದರೆ(zi->ci.pos_local_ಶೀರ್ಷಿಕೆ >= 0xffffffff)
      {
        zip64local_put_ಮೌಲ್ಯ_inmemory(p, zi->ci.pos_local_ಶೀರ್ಷಿಕೆ, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.ಗಾತ್ರ_central_Extra_Free -= datasize + 4;
      zi->ci.ಗಾತ್ರ_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.ಗಾತ್ರ_central_Extra += datasize + 4;
      zip64local_put_ಮೌಲ್ಯ_inmemory(zi->ci.central_ಶೀರ್ಷಿಕೆ+30,(uLong)zi->ci.ಗಾತ್ರ_central_Extra,2);
    }

    ಆದರೆ (err==ZIP_OK)
        err = ಕೂಡಿಸು_ದತ್ತಾಂಶ_ಒಳ_datablock(&zi->central_dir, zi->ci.central_ಶೀರ್ಷಿಕೆ, (uLong)zi->ci.ಗಾತ್ರ_centralheader);

    ಬಿಡುಗಡೆ(zi->ci.central_ಶೀರ್ಷಿಕೆ);

    ಆದರೆ (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        ಆದರೆ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_ಶೀರ್ಷಿಕೆ + 14,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
            err = ZIP_ERRNO;

        ಆದರೆ (err==ZIP_OK)
            err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2,4); /* crc 32, unknown */

        ಆದರೆ(uncompressed_ಗಾತ್ರ >= 0xffffffff || compressed_ಗಾತ್ರ >= 0xffffffff )
        {
          ಆದರೆ(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            ಆದರೆ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
              err = ZIP_ERRNO;

            ಆದರೆ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, uncompressed_ಗಾತ್ರ, 8);

            ಆದರೆ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, compressed_ಗಾತ್ರ, 8);
          }
          ಇಲ್ಲದಿದ್ದರೆ
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        ಇಲ್ಲದಿದ್ದರೆ
        {
          ಆದರೆ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,compressed_ಗಾತ್ರ,4);

          ಆದರೆ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,uncompressed_ಗಾತ್ರ,4);
        }

        ಆದರೆ (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
            err = ZIP_ERRNO;
    }

    zi->ಸಂಖ್ಯೆ_ನಮೂದು ++;
    zi->ಒಳ_opened_file_inzip = 0;

    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Close_File_ಒಳ_Zip(zipFile file) {
    ಹಿಂತಿರುಗಿ zip_Close_File_ಒಳ_Zip_Raw (file,0,0);
}

local ಪೂರ್ಣಾಂಕ Write_Zip64_ಕೊನೆ_ದ_Central_Directory_Locator(zip64_ಆಂತರಿಕ* zi, ZPOS64_T zip64eocd_pos_inzip) {
  ಪೂರ್ಣಾಂಕ err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ;

  err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    ಆದರೆ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    ಆದರೆ (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    ಆದರೆ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    ಹಿಂತಿರುಗಿ err;
}

local ಪೂರ್ಣಾಂಕ Write_Zip64_ಕೊನೆ_ದ_Central_Directory_ದಾಖಲೆ(zip64_ಆಂತರಿಕ* zi, uLong ಗಾತ್ರ_centraldir, ZPOS64_T centraldir_pos_inzip) {
  ಪೂರ್ಣಾಂಕ err = ZIP_OK;

  uLong Zip64_ದತ್ತಾಂಶ_ಗಾತ್ರ = 44;

  err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  ಆದರೆ (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_ದತ್ತಾಂಶ_ಗಾತ್ರ,8); /* why ZPOS64_T of this ? */

  ಆದರೆ (err==ZIP_OK) /* version made by */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ಆದರೆ (err==ZIP_OK) /* version needed */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ಆದರೆ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ಆದರೆ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ಆದರೆ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, zi->ಸಂಖ್ಯೆ_ನಮೂದು, 8);

  ಆದರೆ (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc, zi->filestream, zi->ಸಂಖ್ಯೆ_ನಮೂದು, 8);

  ಆದರೆ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)ಗಾತ್ರ_centraldir,8);

  ಆದರೆ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ;
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  ಹಿಂತಿರುಗಿ err;
}

local ಪೂರ್ಣಾಂಕ Write_ಕೊನೆ_ದ_Central_Directory_ದಾಖಲೆ(zip64_ಆಂತರಿಕ* zi, uLong ಗಾತ್ರ_centraldir, ZPOS64_T centraldir_pos_inzip) {
  ಪೂರ್ಣಾಂಕ err = ZIP_OK;

  /*signature*/
  err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  ಆದರೆ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ಆದರೆ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ಆದರೆ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      ಆದರೆ(zi->ಸಂಖ್ಯೆ_ನಮೂದು >= 0xFFFF)
        err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      ಇಲ್ಲದಿದ್ದರೆ
        err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)zi->ಸಂಖ್ಯೆ_ನಮೂದು,2);
    }
  }

  ಆದರೆ (err==ZIP_OK) /* total number of entries in the central dir */
  {
    ಆದರೆ(zi->ಸಂಖ್ಯೆ_ನಮೂದು >= 0xFFFF)
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    ಇಲ್ಲದಿದ್ದರೆ
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)zi->ಸಂಖ್ಯೆ_ನಮೂದು,2);
  }

  ಆದರೆ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)ಗಾತ್ರ_centraldir,4);

  ಆದರೆ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ;
    ಆದರೆ(pos >= 0xffffffff)
    {
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    ಇಲ್ಲದಿದ್ದರೆ
      err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ),4);
  }

   ಹಿಂತಿರುಗಿ err;
}

local ಪೂರ್ಣಾಂಕ Write_GlobalComment(zip64_ಆಂತರಿಕ* zi, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* global_comment) {
  ಪೂರ್ಣಾಂಕ err = ZIP_OK;
  uInt ಗಾತ್ರ_global_comment = 0;

  ಆದರೆ(global_comment != ಖಾಲಿ)
    ಗಾತ್ರ_global_comment = (uInt)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(global_comment);

  err = zip64local_put_ಮೌಲ್ಯ(&zi->z_filefunc,zi->filestream,(uLong)ಗಾತ್ರ_global_comment,2);

  ಆದರೆ (err == ZIP_OK && ಗಾತ್ರ_global_comment > 0)
  {
    ಆದರೆ (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, ಗಾತ್ರ_global_comment) != ಗಾತ್ರ_global_comment)
      err = ZIP_ERRNO;
  }
  ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zipClose(zipFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* global_comment) {
    zip64_ಆಂತರಿಕ* zi;
    ಪೂರ್ಣಾಂಕ err = 0;
    uLong ಗಾತ್ರ_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    ಆದರೆ (file == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;

    zi = (zip64_ಆಂತರಿಕ*)file;

    ಆದರೆ (zi->ಒಳ_opened_file_inzip == 1)
    {
        err = zip_Close_File_ಒಳ_Zip (file);
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_ADDFILEINEXISTINGZIP
    ಆದರೆ (global_comment==ಖಾಲಿ)
        global_comment = zi->globalcomment;
#ಆದರೆ_ಅಂತ್ಯ

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    ಆದರೆ (err==ZIP_OK)
    {
        linkedlist_datablock_ಆಂತರಿಕ* ldi = zi->central_dir.ಮೊದಲ_ಖಂಡ;
        ಆಗ (ldi!=ಖಾಲಿ)
        {
            ಆದರೆ ((err==ZIP_OK) && (ldi->filled_ಒಳ_this_ಖಂಡ>0))
            {
                ಆದರೆ (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->ದತ್ತಾಂಶ, ldi->filled_ಒಳ_this_ಖಂಡ) != ldi->filled_ಒಳ_this_ಖಂಡ)
                    err = ZIP_ERRNO;
            }

            ಗಾತ್ರ_centraldir += ldi->filled_ಒಳ_this_ಖಂಡ;
            ldi = ldi->ಮುಂದಿನ_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    ಗಣ_ಕೊನೆ(&zi->ಗಣ);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->ಕೂಡಿಸು_position_when_writing_ಸ್ಥಾನಾಂತರ;
    ಆದರೆ(pos >= 0xffffffff || zi->ಸಂಖ್ಯೆ_ನಮೂದು >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_ಕೊನೆ_ದ_Central_Directory_ದಾಖಲೆ(zi, ಗಾತ್ರ_centraldir, centraldir_pos_inzip);

      Write_Zip64_ಕೊನೆ_ದ_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    ಆದರೆ (err==ZIP_OK)
      err = Write_ಕೊನೆ_ದ_Central_Directory_ದಾಖಲೆ(zi, ಗಾತ್ರ_centraldir, centraldir_pos_inzip);

    ಆದರೆ(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    ಆದರೆ (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        ಆದರೆ (err == ZIP_OK)
            err = ZIP_ERRNO;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_ADDFILEINEXISTINGZIP
    ಬಿಡುಗಡೆ(zi->globalcomment);
#ಆದರೆ_ಅಂತ್ಯ
    ಬಿಡುಗಡೆ(zi);

    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Remove_Extra_Info_ಖಂಡ(ಅಕ್ಷರ* p_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ* ದತ್ತಾಂಶ_Len, ಗಿಡ್ಡ s_ಶೀರ್ಷಿಕೆ) {
  ಅಕ್ಷರ* p = p_ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = 0;
  ಅಕ್ಷರ* p_New_ಶೀರ್ಷಿಕೆ;
  ಅಕ್ಷರ* pTmp;
  ಗಿಡ್ಡ ಶೀರ್ಷಿಕೆ;
  ಗಿಡ್ಡ ದತ್ತಾಂಶ_ಗಾತ್ರ;

  ಪೂರ್ಣಾಂಕ retVal = ZIP_OK;

  ಆದರೆ(p_ದತ್ತಾಂಶ == ಖಾಲಿ || ದತ್ತಾಂಶ_Len == ಖಾಲಿ || *ದತ್ತಾಂಶ_Len < 4)
    ಹಿಂತಿರುಗಿ ZIP_PARAMERROR;

  p_New_ಶೀರ್ಷಿಕೆ = (ಅಕ್ಷರ*)ALLOC((ಚಿಹ್ನೆರಹಿತ)*ದತ್ತಾಂಶ_Len);
  pTmp = p_New_ಶೀರ್ಷಿಕೆ;

  ಆಗ(p < (p_ದತ್ತಾಂಶ + *ದತ್ತಾಂಶ_Len))
  {
    ಶೀರ್ಷಿಕೆ = *(ಗಿಡ್ಡ*)p;
    ದತ್ತಾಂಶ_ಗಾತ್ರ = *(((ಗಿಡ್ಡ*)p)+1);

    ಆದರೆ( ಶೀರ್ಷಿಕೆ == s_ಶೀರ್ಷಿಕೆ ) /* Header found. */
    {
      p += ದತ್ತಾಂಶ_ಗಾತ್ರ + 4; /* skip it. do not copy to temp buffer */
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      ಸ್ಮರಣೆ_ನಕಲು(pTmp, p, ದತ್ತಾಂಶ_ಗಾತ್ರ + 4);
      p += ದತ್ತಾಂಶ_ಗಾತ್ರ + 4;
      ಗಾತ್ರ_2 += ದತ್ತಾಂಶ_ಗಾತ್ರ + 4;
    }

  }

  ಆದರೆ(ಗಾತ್ರ_2 < *ದತ್ತಾಂಶ_Len)
  {
    /* clean old extra info block. */
    memset(p_ದತ್ತಾಂಶ,0, *ದತ್ತಾಂಶ_Len);

    /* copy the new extra info block over the old */
    ಆದರೆ(ಗಾತ್ರ_2 > 0)
      ಸ್ಮರಣೆ_ನಕಲು(p_ದತ್ತಾಂಶ, p_New_ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ_2);

    /* set the new extra info size */
    *ದತ್ತಾಂಶ_Len = ಗಾತ್ರ_2;

    retVal = ZIP_OK;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    retVal = ZIP_ERRNO;

  ಬಿಡುಗಡೆ(p_New_ಶೀರ್ಷಿಕೆ);

  ಹಿಂತಿರುಗಿ retVal;
}
