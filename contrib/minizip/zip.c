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


#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <time.h>
#నిర్వచితం_కాకపోతే ZLIB_CONST
#  define ZLIB_CONST
#అయితే_ముగింపు
#నిర్వచితమైతే ZLIB_DLL
#  undef ZLIB_DLL
#అయితే_ముగింపు
#అన్తర్భావ "zlib.h"
#అన్తర్భావ "zip.h"

#నిర్వచితమైతే STDC
#  include <stddef.h>
#అయితే_ముగింపు
#నిర్వచితమైతే NO_ERRNO_H
    బాహ్య పూర్ణాంకం errno;
#లేకపోతే
#   include <errno.h>
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే local
#  define local స్థిర
#అయితే_ముగింపు
/* compile with -Dlocal if your debugger can't find static symbols */

#నిర్వచితం_కాకపోతే VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే Z_BUFSIZE
#నిర్వచించు Z_BUFSIZE (64*1024) /* (16384) */
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే Z_MAXFILENAMEINZIP
#నిర్వచించు Z_MAXFILENAMEINZIP (256)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే ALLOC
# define ALLOC(కొలత) (జ్ఞాపకం_ఇవ్వు(కొలత))
#అయితే_ముగింపు

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#నిర్వచించు MAKEULONG64(a, b) ((ZPOS64_T)(((గుర్తులేని దీర్ఘ)(a)) | ((ZPOS64_T)((గుర్తులేని దీర్ఘ)(b))) << 32))

#నిర్వచితం_కాకపోతే SEEK_CUR
#నిర్వచించు SEEK_CUR    1
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SEEK_END
#నిర్వచించు SEEK_END    2
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SEEK_SET
#నిర్వచించు SEEK_SET    0
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే DEF_MEM_LEVEL
#ఒకవేళ MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#లేకపోతే
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#అయితే_ముగింపు
#అయితే_ముగింపు
స్థిరరాశి అక్షరం zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#నిర్వచించు SIZEDATA_INDATABLOCK (4096-(4*4))

#నిర్వచించు LOCALHEADERMAGIC    (0x04034b50)
#నిర్వచించు CENTRALHEADERMAGIC  (0x02014b50)
#నిర్వచించు ENDHEADERMAGIC      (0x06054b50)
#నిర్వచించు ZIP64ENDHEADERMAGIC      (0x6064b50)
#నిర్వచించు ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#నిర్వచించు FLAG_LOCALHEADER_స్థానభ్రంశం (0x06)
#నిర్వచించు చక్రీయ_తనిఖీ_LOCALHEADER_స్థానభ్రంశం  (0x0e)

#నిర్వచించు SIZECENTRALHEADER (0x2e) /* 46 */

రకనిర్వచనం నిర్మాణం linkedlist_datablock_అంతర్గత_s
{
  నిర్మాణం linkedlist_datablock_అంతర్గత_s* తదుపరి_datablock;
  uLong  avail_లోపల_this_ఖండం;
  uLong  filled_లోపల_this_ఖండం;
  uLong  వాడని; /* for future use and alignment */
  గుర్తులేని అక్షరం దత్తాంశం[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_అంతర్గత;

రకనిర్వచనం నిర్మాణం linkedlist_దత్తాంశం_s
{
    linkedlist_datablock_అంతర్గత* మొదటి_ఖండం;
    linkedlist_datablock_అంతర్గత* చివరి_ఖండం;
} linkedlist_దత్తాంశం;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#నిర్వచించు SKIPSET_EXPORT స్థిర
రకనిర్వచనం అక్షరం *సమితి_కీలకం_t;
#నిర్వచించు సమితి_cmp(a, b) strcmp(a, b)
#నిర్వచించు సమితి_drop(s, k) సమితి_free(s, k)
#అన్తర్భావ "skipset.h"
రకనిర్వచనం నిర్మాణం {
    గుర్తులేని అక్షరం *తదుపరి;        /* next byte in datablock data */
    size_t ఎడమ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_అంతర్గత *కణుపు;    /* current datablock */
} ఖండం_t;


రకనిర్వచనం నిర్మాణం
{
    z_ప్రవాహం ప్రవాహం;            /* zLib stream structure for inflate */
#నిర్వచితమైతే HAVE_BZIP2
    bz_ప్రవాహం bstream;          /* bzLib stream structure for bziped */
#అయితే_ముగింపు

    పూర్ణాంకం  ప్రవాహం_initialised;    /* 1 is stream is initialised */
    uInt pos_లోపల_buffered_దత్తాంశం;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_శీర్షిక;     /* offset of the local header of the file
                                     currently writing */
    అక్షరం* central_శీర్షిక;       /* central header data for the current file */
    uLong కొలత_central_Extra;
    uLong కొలత_centralheader;   /* size of the central header for cur file */
    uLong కొలత_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    పూర్ణాంకం  పద్ధతి;                /* compression method of file currently wr.*/
    పూర్ణాంకం  raw;                   /* 1 for directly writing raw data */
    Byte buffered_దత్తాంశం[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong చక్రీయ_తనిఖీ_32_2;
    పూర్ణాంకం  encrypt;
    పూర్ణాంకం  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T మొత్తం_Compressed_దత్తాంశం;
    ZPOS64_T మొత్తం_Uncompressed_దత్తాంశం;
#నిర్వచితం_కాకపోతే NOCRYPT
    గుర్తులేని దీర్ఘ కీలకాలు[3];     /* keys defining the pseudo-random sequence */
    స్థిరరాశి z_చక్రీయ_తనిఖీ_t* pcrc_32_tab;
    గుర్తులేని crypt_శీర్షిక_కొలత;
#అయితే_ముగింపు
} curfile64_info;

రకనిర్వచనం నిర్మాణం
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_దత్తాంశం central_dir;/* datablock with central dir in construction*/
    పూర్ణాంకం  లోపల_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T కూడు_position_when_writing_స్థానభ్రంశం;
    ZPOS64_T సంఖ్య_నమోదు;

#నిర్వచితం_కాకపోతే NO_ADDFILEINEXISTINGZIP
    అక్షరం *globalcomment;
#అయితే_ముగింపు

    /* Support for zipAlreadyThere(). */
    సమితి_t సమితి;              /* set for detecting name collisions */
    ఖండం_t ఖండం;          /* block for reading the central directory */

} zip64_అంతర్గత;


#నిర్వచితం_కాకపోతే NOCRYPT
#నిర్వచించు INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#అన్తర్భావ "crypt.h"
#అయితే_ముగింపు

local linkedlist_datablock_అంతర్గత* allocate_new_datablock(శూన్య) {
    linkedlist_datablock_అంతర్గత* ldi;
    ldi = (linkedlist_datablock_అంతర్గత*)
                 ALLOC(పరిమాణం(linkedlist_datablock_అంతర్గత));
    ఒకవేళ (ldi!=ఖాళీ)
    {
        ldi->తదుపరి_datablock = ఖాళీ ;
        ldi->filled_లోపల_this_ఖండం = 0 ;
        ldi->avail_లోపల_this_ఖండం = SIZEDATA_INDATABLOCK ;
    }
    తిరిగి ldi;
}

local శూన్య free_datablock(linkedlist_datablock_అంతర్గత* ldi) {
    అయితే (ldi!=ఖాళీ)
    {
        linkedlist_datablock_అంతర్గత* ldinext = ldi->తదుపరి_datablock;
        విడుదల(ldi);
        ldi = ldinext;
    }
}

local శూన్య init_linkedlist(linkedlist_దత్తాంశం* ll) {
    ll->మొదటి_ఖండం = ll->చివరి_ఖండం = ఖాళీ;
}

local శూన్య free_linkedlist(linkedlist_దత్తాంశం* ll) {
    free_datablock(ll->మొదటి_ఖండం);
    ll->మొదటి_ఖండం = ll->చివరి_ఖండం = ఖాళీ;
}


local పూర్ణాంకం కూడు_దత్తాంశం_లోపల_datablock(linkedlist_దత్తాంశం* ll, స్థిరరాశి శూన్య* buf, uLong len) {
    linkedlist_datablock_అంతర్గత* ldi;
    స్థిరరాశి గుర్తులేని అక్షరం* from_copy;

    ఒకవేళ (ll==ఖాళీ)
        తిరిగి ZIP_INTERNALERROR;

    ఒకవేళ (ll->చివరి_ఖండం == ఖాళీ)
    {
        ll->మొదటి_ఖండం = ll->చివరి_ఖండం = allocate_new_datablock();
        ఒకవేళ (ll->మొదటి_ఖండం == ఖాళీ)
            తిరిగి ZIP_INTERNALERROR;
    }

    ldi = ll->చివరి_ఖండం;
    from_copy = (స్థిరరాశి గుర్తులేని అక్షరం*)buf;

    అయితే (len>0)
    {
        uInt copy_this;
        uInt i;
        గుర్తులేని అక్షరం* గమ్యం_copy;

        ఒకవేళ (ldi->avail_లోపల_this_ఖండం==0)
        {
            ldi->తదుపరి_datablock = allocate_new_datablock();
            ఒకవేళ (ldi->తదుపరి_datablock == ఖాళీ)
                తిరిగి ZIP_INTERNALERROR;
            ldi = ldi->తదుపరి_datablock ;
            ll->చివరి_ఖండం = ldi;
        }

        ఒకవేళ (ldi->avail_లోపల_this_ఖండం < len)
            copy_this = (uInt)ldi->avail_లోపల_this_ఖండం;
        లేకపోతే
            copy_this = (uInt)len;

        గమ్యం_copy = &(ldi->దత్తాంశం[ldi->filled_లోపల_this_ఖండం]);

        కొరకు (i=0;i<copy_this;i++)
            *(గమ్యం_copy+i)=*(from_copy+i);

        ldi->filled_లోపల_this_ఖండం += copy_this;
        ldi->avail_లోపల_this_ఖండం -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    తిరిగి ZIP_OK;
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
local శూన్య ఖండం_init(ఖండం_t *ఖండం, linkedlist_దత్తాంశం *list) {
    ఖండం->కణుపు = list->మొదటి_ఖండం;
    ఖండం->తదుపరి = ఖండం->కణుపు->దత్తాంశం;
    ఖండం->ఎడమ = ఖండం->కణుపు->filled_లోపల_this_ఖండం;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local శూన్య ఖండం_stop(ఖండం_t *ఖండం) {
    ఖండం->ఎడమ = 0;
    ఖండం->తదుపరి = ఖాళీ;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local పూర్ణాంకం ఖండం_చివర(ఖండం_t *ఖండం) {
    linkedlist_datablock_అంతర్గత *కణుపు = ఖండం->కణుపు;
    ఒకవేళ (కణుపు == ఖాళీ)
        /* This block was previously terminated with extreme prejudice. */
        తిరిగి 1;
    ఒకవేళ (ఖండం->తదుపరి < కణుపు->దత్తాంశం + కణుపు->filled_లోపల_this_ఖండం)
        /* There are more bytes to read in the current datablock. */
        తిరిగి 0;
    అయితే (కణుపు->తదుపరి_datablock != ఖాళీ) {
        ఒకవేళ (కణుపు->filled_లోపల_this_ఖండం != 0)
            /* There are some bytes in a later datablock. */
            తిరిగి 0;
        కణుపు = కణుపు->తదుపరి_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    తిరిగి 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local పూర్ణాంకం ఖండం_get(ఖండం_t *ఖండం) {
    అయితే (ఖండం->ఎడమ == 0) {
        ఒకవేళ (ఖండం->కణుపు == ఖాళీ)
            /* We've been marked bad. Return end. */
            తిరిగి -1;
        /* Update left in case more was filled in since we were last here. */
        ఖండం->ఎడమ = ఖండం->కణుపు->filled_లోపల_this_ఖండం -
                      (size_t)(ఖండం->తదుపరి - ఖండం->కణుపు->దత్తాంశం);
        ఒకవేళ (ఖండం->ఎడమ != 0)
            /* There was indeed more data appended in the current datablock. */
            ఆపు;
        ఒకవేళ (ఖండం->కణుపు->తదుపరి_datablock == ఖాళీ)
            /* No more data here, and there is no next datablock. At the end. */
            తిరిగి -1;
        /* Try the next datablock for more data. */
        ఖండం->కణుపు = ఖండం->కణుపు->తదుపరి_datablock;
        ఖండం->తదుపరి = ఖండం->కణుపు->దత్తాంశం;
        ఖండం->ఎడమ = ఖండం->కణుపు->filled_లోపల_this_ఖండం;
    }
    /* We have a byte to return. */
    ఖండం->ఎడమ--;
    తిరిగి *ఖండం->తదుపరి++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local దీర్ఘ ఖండం_get2(ఖండం_t *ఖండం) {
    పూర్ణాంకం దిగువ = ఖండం_get(ఖండం);
    పూర్ణాంకం ఎగువ = ఖండం_get(ఖండం);
    తిరిగి దిగువ < 0 || ఎగువ < 0 ? -1 : దిగువ | ((దీర్ఘ)ఎగువ << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t ఖండం_read(ఖండం_t *ఖండం, గుర్తులేని అక్షరం *buf, size_t len) {
    size_t need = len;
    size_t take;
    అయితే (need) {
        ఒకవేళ (ఖండం->ఎడమ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            పూర్ణాంకం got = ఖండం_get(ఖండం);
            ఒకవేళ (got == -1)
                /* Reached the end. */
                ఆపు;
            *buf++ = (గుర్తులేని అక్షరం)got;
            need--;
            కొనసాగు;
        }
        take = need > ఖండం->ఎడమ ? ఖండం->ఎడమ : need;
        జ్ఞాపకం_నకలు(buf, ఖండం->తదుపరి, take);
        ఖండం->తదుపరి += take;
        ఖండం->ఎడమ -= take;
        buf += take;
        need -= take;
    }
    తిరిగి len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local పూర్ణాంకం ఖండం_skip(ఖండం_t *ఖండం, size_t n) {
    అయితే (n > ఖండం->ఎడమ) {
        n -= ఖండం->ఎడమ;
        ఖండం->తదుపరి += ఖండం->ఎడమ;
        ఖండం->ఎడమ = 0;
        ఒకవేళ (ఖండం_get(ఖండం) == -1)
            తిరిగి -1;
        n--;
    }
    ఖండం->తదుపరి += n;
    ఖండం->ఎడమ -= n;
    తిరిగి 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local అక్షరం *ఖండం_central_name(ఖండం_t *ఖండం, సమితి_t *సమితి) {
    గుర్తులేని flen, xlen, clen;
    అక్షరం *name = ఖాళీ;
    కొరకు (;;) {
        ఒకవేళ (ఖండం_చివర(ఖండం))
            /* At the end of the central directory (so far). */
            తిరిగి ఖాళీ;

        /* Check for a central directory record signature. */
        ఒకవేళ (ఖండం_get2(ఖండం) != (CENTRALHEADERMAGIC & 0xffff) ||
            ఖండం_get2(ఖండం) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            ఆపు;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        ఖండం_skip(ఖండం, 24);
        flen = (గుర్తులేని)ఖండం_get2(ఖండం);    /* file name length */
        xlen = (గుర్తులేని)ఖండం_get2(ఖండం);    /* extra length */
        clen = (గుర్తులేని)ఖండం_get2(ఖండం);    /* comment length */
        ఒకవేళ (ఖండం_skip(ఖండం, 12) == -1)
            /* Premature end of the record. */
            ఆపు;

        /* Extract the name and skip over the extra and comment fields. */
        name = సమితి_alloc(సమితి, ఖాళీ, flen + 1);
        ఒకవేళ (ఖండం_read(ఖండం, (గుర్తులేని అక్షరం *)name, flen) < flen ||
            ఖండం_skip(ఖండం, xlen + clen) == -1)
            /* Premature end of the record. */
            ఆపు;

        /* Check for embedded nuls in the name. */
        ఒకవేళ (memchr(name, 0, flen) != ఖాళీ) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            సమితి_free(సమితి, name);
            కొనసాగు;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        తిరిగి name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    సమితి_free(సమితి, name);
    ఖండం_stop(ఖండం);
    తిరిగి ఖాళీ;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
బాహ్య పూర్ణాంకం ZEXPORT zipAlreadyThere(zipFile file, అక్షరం స్థిరరాశి *name) {
    size_t len;
    అక్షరం *copy;
    పూర్ణాంకం దొరికింది;
    zip64_అంతర్గత *zip = file;
    ఒకవేళ (zip == ఖాళీ)
        తిరిగి ZIP_PARAMERROR;
    ఒకవేళ (zip->central_dir.మొదటి_ఖండం == ఖాళీ)
        /* No central directory yet, so no, name isn't there. */
        తిరిగి 0;
    ఒకవేళ (setjmp(zip->సమితి.env)) {
        /* Memory allocation failure. */
        సమితి_చివర(&zip->సమితి);
        తిరిగి -2;
    }
    ఒకవేళ (!సమితి_ok(&zip->సమితి)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        సమితి_ప్రారంభం(&zip->సమితి);
        ఖండం_init(&zip->ఖండం, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    కొరకు (;;) {
        అక్షరం *there = ఖండం_central_name(&zip->ఖండం, &zip->సమితి);
        ఒకవేళ (there == ఖాళీ) {
            ఒకవేళ (zip->ఖండం.తదుపరి == ఖాళీ)
                /* The central directory is invalid. */
                తిరిగి -1;
            ఆపు;
        }

        /* Add there to the set. */
        ఒకవేళ (సమితి_చొప్పించు(&zip->సమితి, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            సమితి_free(&zip->సమితి, there);
    }

    /* Return true if name is in the central directory. */
    len = అక్షరశ్రేణి_పొడవు(name);
    copy = సమితి_alloc(&zip->సమితి, ఖాళీ, len + 1);
    జ్ఞాపకం_నకలు(copy, name, len + 1);
    దొరికింది = సమితి_దొరికింది(&zip->సమితి, copy);
    సమితి_free(&zip->సమితి, copy);
    తిరిగి దొరికింది;
}


/****************************************************************************/

#నిర్వచితం_కాకపోతే NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local పూర్ణాంకం zip64local_put_విలువ(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, పూర్ణాంకం nbByte) {
    గుర్తులేని అక్షరం buf[8];
    పూర్ణాంకం n;
    కొరకు (n = 0; n < nbByte; n++)
    {
        buf[n] = (గుర్తులేని అక్షరం)(x & 0xff);
        x >>= 8;
    }
    ఒకవేళ (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      కొరకు (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    ఒకవేళ (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        తిరిగి ZIP_ERRNO;
    లేకపోతే
        తిరిగి ZIP_OK;
}

local శూన్య zip64local_put_విలువ_inmemory (శూన్య* dest, ZPOS64_T x, పూర్ణాంకం nbByte) {
    గుర్తులేని అక్షరం* buf=(గుర్తులేని అక్షరం*)dest;
    పూర్ణాంకం n;
    కొరకు (n = 0; n < nbByte; n++) {
        buf[n] = (గుర్తులేని అక్షరం)(x & 0xff);
        x >>= 8;
    }

    ఒకవేళ (x != 0)
    {     /* data overflow - hack for ZIP64 */
       కొరకు (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_Tmz_Date_గమ్యం_Dos_Date(స్థిరరాశి tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    ఒకవేళ (year>=1980)
        year-=1980;
    లేకపోతే ఒకవేళ (year>=80)
        year-=80;
    తిరిగి
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local పూర్ణాంకం zip64local_getByte(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, పూర్ణాంకం* pi) {
    గుర్తులేని అక్షరం c;
    పూర్ణాంకం err = (పూర్ణాంకం)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    ఒకవేళ (err==1)
    {
        *pi = (పూర్ణాంకం)c;
        తిరిగి ZIP_OK;
    }
    లేకపోతే
    {
        ఒకవేళ (ZERROR64(*pzlib_filefunc_def,filestream))
            తిరిగి ZIP_ERRNO;
        లేకపోతే
            తిరిగి ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local పూర్ణాంకం zip64local_getShort(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    పూర్ణాంకం i = 0;
    పూర్ణాంకం err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ఒకవేళ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ఒకవేళ (err==ZIP_OK)
        *pX = x;
    లేకపోతే
        *pX = 0;
    తిరిగి err;
}

local పూర్ణాంకం zip64local_getLong(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    పూర్ణాంకం i = 0;
    పూర్ణాంకం err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    ఒకవేళ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    ఒకవేళ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    ఒకవేళ (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    ఒకవేళ (err==ZIP_OK)
        *pX = x;
    లేకపోతే
        *pX = 0;
    తిరిగి err;
}


local పూర్ణాంకం zip64local_getLong64(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  పూర్ణాంకం i = 0;
  పూర్ణాంకం err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  ఒకవేళ (err==ZIP_OK)
    *pX = x;
  లేకపోతే
    *pX = 0;

  తిరిగి err;
}

#నిర్వచితం_కాకపోతే BUFREADCOMMENT
#నిర్వచించు BUFREADCOMMENT (0x400)
#అయితే_ముగింపు
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_శోధన_Central_Dir(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  గుర్తులేని అక్షరం* buf;
  ZPOS64_T u_కొలత_File;
  ZPOS64_T u_వెనుక_Read;
  ZPOS64_T u_Max_వెనుక=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_దొరికింది=0;

  ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_చివర) != 0)
    తిరిగి 0;


  u_కొలత_File = ZTELL64(*pzlib_filefunc_def,filestream);

  ఒకవేళ (u_Max_వెనుక>u_కొలత_File)
    u_Max_వెనుక = u_కొలత_File;

  buf = (గుర్తులేని అక్షరం*)ALLOC(BUFREADCOMMENT+4);
  ఒకవేళ (buf==ఖాళీ)
    తిరిగి 0;

  u_వెనుక_Read = 4;
  అయితే (u_వెనుక_Read<u_Max_వెనుక)
  {
    uLong u_Read_కొలత;
    ZPOS64_T uReadPos ;
    పూర్ణాంకం i;
    ఒకవేళ (u_వెనుక_Read+BUFREADCOMMENT>u_Max_వెనుక)
      u_వెనుక_Read = u_Max_వెనుక;
    లేకపోతే
      u_వెనుక_Read+=BUFREADCOMMENT;
    uReadPos = u_కొలత_File-u_వెనుక_Read ;

    u_Read_కొలత = ((BUFREADCOMMENT+4) < (u_కొలత_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_కొలత_File-uReadPos);
    ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
      ఆపు;

    ఒకవేళ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_కొలత)!=u_Read_కొలత)
      ఆపు;

    కొరకు (i=(పూర్ణాంకం)u_Read_కొలత-3; (i--)>0;)
      ఒకవేళ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_దొరికింది = uReadPos+(గుర్తులేని)i;
        ఆపు;
      }

    ఒకవేళ (u_Pos_దొరికింది!=0)
      ఆపు;
  }
  విడుదల(buf);
  తిరిగి u_Pos_దొరికింది;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_శోధన_Central_Dir64(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  గుర్తులేని అక్షరం* buf;
  ZPOS64_T u_కొలత_File;
  ZPOS64_T u_వెనుక_Read;
  ZPOS64_T u_Max_వెనుక=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_దొరికింది=0;
  uLong uL;
  ZPOS64_T relative_స్థానభ్రంశం;

  ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_చివర) != 0)
    తిరిగి 0;

  u_కొలత_File = ZTELL64(*pzlib_filefunc_def,filestream);

  ఒకవేళ (u_Max_వెనుక>u_కొలత_File)
    u_Max_వెనుక = u_కొలత_File;

  buf = (గుర్తులేని అక్షరం*)ALLOC(BUFREADCOMMENT+4);
  ఒకవేళ (buf==ఖాళీ)
    తిరిగి 0;

  u_వెనుక_Read = 4;
  అయితే (u_వెనుక_Read<u_Max_వెనుక)
  {
    uLong u_Read_కొలత;
    ZPOS64_T uReadPos;
    పూర్ణాంకం i;
    ఒకవేళ (u_వెనుక_Read+BUFREADCOMMENT>u_Max_వెనుక)
      u_వెనుక_Read = u_Max_వెనుక;
    లేకపోతే
      u_వెనుక_Read+=BUFREADCOMMENT;
    uReadPos = u_కొలత_File-u_వెనుక_Read ;

    u_Read_కొలత = ((BUFREADCOMMENT+4) < (u_కొలత_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_కొలత_File-uReadPos);
    ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
      ఆపు;

    ఒకవేళ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_కొలత)!=u_Read_కొలత)
      ఆపు;

    కొరకు (i=(పూర్ణాంకం)u_Read_కొలత-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      ఒకవేళ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_దొరికింది = uReadPos+(గుర్తులేని)i;
        ఆపు;
      }
    }

      ఒకవేళ (u_Pos_దొరికింది!=0)
        ఆపు;
  }

  విడుదల(buf);
  ఒకవేళ (u_Pos_దొరికింది == 0)
    తిరిగి 0;

  /* Zip64 end of central directory locator */
  ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_దొరికింది,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
    తిరిగి 0;

  /* the signature, already checked */
  ఒకవేళ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    తిరిగి 0;

  /* number of the disk with the start of the zip64 end of central directory */
  ఒకవేళ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    తిరిగి 0;
  ఒకవేళ (uL != 0)
    తిరిగి 0;

  /* relative offset of the zip64 end of central directory record */
  ఒకవేళ (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_స్థానభ్రంశం)!=ZIP_OK)
    తిరిగి 0;

  /* total number of disks */
  ఒకవేళ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    తిరిగి 0;
  ఒకవేళ (uL != 1)
    తిరిగి 0;

  /* Goto Zip64 end of central directory record */
  ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream, relative_స్థానభ్రంశం,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
    తిరిగి 0;

  /* the signature */
  ఒకవేళ (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    తిరిగి 0;

  ఒకవేళ (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    తిరిగి 0;

  తిరిగి relative_స్థానభ్రంశం;
}

local పూర్ణాంకం ఎక్కించు_Central_Directory_రికార్డు(zip64_అంతర్గత* pziinit) {
  పూర్ణాంకం err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T కొలత_central_dir;     /* size of the central directory  */
  ZPOS64_T స్థానభ్రంశం_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong సంఖ్య_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong సంఖ్య_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T సంఖ్య_నమోదు;
  ZPOS64_T సంఖ్య_నమోదు_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong సంస్కరణ_Made_By;
  uLong సంస్కరణ_Needed;
  uLong కొలత_comment;

  పూర్ణాంకం has_ZIP64_రికార్డు = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_శోధన_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  ఒకవేళ(central_pos > 0)
  {
    has_ZIP64_రికార్డు = 1;
  }
  లేకపోతే ఒకవేళ(central_pos == 0)
  {
    central_pos = zip64local_శోధన_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  ఒకవేళ(has_ZIP64_రికార్డు)
  {
    ZPOS64_T కొలత_చివర_యొక్క_Central_Directory;
    ఒకవేళ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_సమితి) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ఒకవేళ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    ఒకవేళ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &కొలత_చివర_యొక్క_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    ఒకవేళ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &సంస్కరణ_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    ఒకవేళ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &సంస్కరణ_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ఒకవేళ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&సంఖ్య_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ఒకవేళ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&సంఖ్య_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    ఒకవేళ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &సంఖ్య_నమోదు)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    ఒకవేళ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&సంఖ్య_నమోదు_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    ఒకవేళ ((సంఖ్య_నమోదు_CD!=సంఖ్య_నమోదు) || (సంఖ్య_disk_with_CD!=0) || (సంఖ్య_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ఒకవేళ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&కొలత_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    ఒకవేళ (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&స్థానభ్రంశం_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    కొలత_comment = 0;
  }
  లేకపోతే
  {
    /* Read End of central Directory info */
    ఒకవేళ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    ఒకవేళ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    ఒకవేళ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&సంఖ్య_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    ఒకవేళ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&సంఖ్య_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    సంఖ్య_నమోదు = 0;
    ఒకవేళ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    లేకపోతే
      సంఖ్య_నమోదు = uL;

    /* total number of entries in the central dir */
    సంఖ్య_నమోదు_CD = 0;
    ఒకవేళ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    లేకపోతే
      సంఖ్య_నమోదు_CD = uL;

    ఒకవేళ ((సంఖ్య_నమోదు_CD!=సంఖ్య_నమోదు) || (సంఖ్య_disk_with_CD!=0) || (సంఖ్య_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    కొలత_central_dir = 0;
    ఒకవేళ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    లేకపోతే
      కొలత_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    స్థానభ్రంశం_central_dir = 0;
    ఒకవేళ (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    లేకపోతే
      స్థానభ్రంశం_central_dir = uL;


    /* zipfile global comment length */
    ఒకవేళ (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &కొలత_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  ఒకవేళ ((central_pos<స్థానభ్రంశం_central_dir+కొలత_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  ఒకవేళ (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    తిరిగి ZIP_ERRNO;
  }

  ఒకవేళ (కొలత_comment>0)
  {
    pziinit->globalcomment = (అక్షరం*)ALLOC(కొలత_comment+1);
    ఒకవేళ (pziinit->globalcomment)
    {
      కొలత_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,కొలత_comment);
      pziinit->globalcomment[కొలత_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (స్థానభ్రంశం_central_dir+కొలత_central_dir);
  pziinit->కూడు_position_when_writing_స్థానభ్రంశం = byte_before_the_zipfile;

  {
    ZPOS64_T కొలత_central_dir_గమ్యం_read = కొలత_central_dir;
    size_t buf_కొలత_2 = SIZEDATA_INDATABLOCK;
    శూన్య* buf_read = (శూన్య*)ALLOC(buf_కొలత_2);
    ఒకవేళ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, స్థానభ్రంశం_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_సమితి) != 0)
      err=ZIP_ERRNO;

    అయితే ((కొలత_central_dir_గమ్యం_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      ఒకవేళ (read_this > కొలత_central_dir_గమ్యం_read)
        read_this = కొలత_central_dir_గమ్యం_read;

      ఒకవేళ (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      ఒకవేళ (err==ZIP_OK)
        err = కూడు_దత్తాంశం_లోపల_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      కొలత_central_dir_గమ్యం_read-=read_this;
    }
    విడుదల(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->సంఖ్య_నమోదు = సంఖ్య_నమోదు_CD;

  ఒకవేళ (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, స్థానభ్రంశం_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_సమితి) != 0)
    err=ZIP_ERRNO;

  తిరిగి err;
}


#అయితే_ముగింపు /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
బాహ్య zipFile ZEXPORT zipOpen3(స్థిరరాశి శూన్య *pathname, పూర్ణాంకం append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_అంతర్గత ziinit;
    zip64_అంతర్గత* zi;
    పూర్ణాంకం err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = ఖాళీ;
    ziinit.z_filefunc.ztell32_file = ఖాళీ;
    ఒకవేళ (pzlib_filefunc64_32_def==ఖాళీ)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    లేకపోతే
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_స్థితి_CREATE) ?
                  (ZLIB_FILEFUNC_బహుళకం_READ | ZLIB_FILEFUNC_బహుళకం_WRITE | ZLIB_FILEFUNC_బహుళకం_CREATE) :
                    (ZLIB_FILEFUNC_బహుళకం_READ | ZLIB_FILEFUNC_బహుళకం_WRITE | ZLIB_FILEFUNC_బహుళకం_EXISTING));

    ఒకవేళ (ziinit.filestream == ఖాళీ)
        తిరిగి ఖాళీ;

    ఒకవేళ (append == APPEND_స్థితి_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.లోపల_opened_file_inzip = 0;
    ziinit.ci.ప్రవాహం_initialised = 0;
    ziinit.సంఖ్య_నమోదు = 0;
    ziinit.కూడు_position_when_writing_స్థానభ్రంశం = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.సమితి, 0, పరిమాణం(సమితి_t));  /* make sure set appears dormant */



    zi = (zip64_అంతర్గత*)ALLOC(పరిమాణం(zip64_అంతర్గత));
    ఒకవేళ (zi==ఖాళీ)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        తిరిగి ఖాళీ;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = ఖాళీ;
    ఒకవేళ (append == APPEND_స్థితి_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = ఎక్కించు_Central_Directory_రికార్డు(&ziinit);
    }

    ఒకవేళ (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    ఒకవేళ (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        విడుదల(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        విడుదల(zi);
        తిరిగి ఖాళీ;
    }
    లేకపోతే
    {
        *zi = ziinit;
        తిరిగి (zipFile)zi;
    }
}

బాహ్య zipFile ZEXPORT zipOpen2(స్థిరరాశి అక్షరం *pathname, పూర్ణాంకం append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    ఒకవేళ (pzlib_filefunc32_def != ఖాళీ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        తిరిగి zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    లేకపోతే
        తిరిగి zipOpen3(pathname, append, globalcomment, ఖాళీ);
}

బాహ్య zipFile ZEXPORT zipOpen2_64(స్థిరరాశి శూన్య *pathname, పూర్ణాంకం append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    ఒకవేళ (pzlib_filefunc_def != ఖాళీ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ఖాళీ;
        zlib_filefunc64_32_def_fill.ztell32_file = ఖాళీ;
        zlib_filefunc64_32_def_fill.zseek32_file = ఖాళీ;
        తిరిగి zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    లేకపోతే
        తిరిగి zipOpen3(pathname, append, globalcomment, ఖాళీ);
}



బాహ్య zipFile ZEXPORT zipOpen(స్థిరరాశి అక్షరం* pathname, పూర్ణాంకం append) {
    తిరిగి zipOpen3((స్థిరరాశి శూన్య*)pathname,append,ఖాళీ,ఖాళీ);
}

బాహ్య zipFile ZEXPORT zipOpen64(స్థిరరాశి శూన్య* pathname, పూర్ణాంకం append) {
    తిరిగి zipOpen3(pathname,append,ఖాళీ,ఖాళీ);
}

local పూర్ణాంకం Write_Local_File_శీర్షిక(zip64_అంతర్గత* zi, స్థిరరాశి అక్షరం* filename, uInt కొలత_extrafield_local, స్థిరరాశి శూన్య* extrafield_local) {
  /* write the local header */
  పూర్ణాంకం err;
  uInt కొలత_filename = (uInt)అక్షరశ్రేణి_పొడవు(filename);
  uInt కొలత_extrafield = కొలత_extrafield_local;

  err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  ఒకవేళ (err==ZIP_OK)
  {
    ఒకవేళ(zi->ci.zip64)
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    లేకపోతే
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.పద్ధతి,2);

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  ఒకవేళ (err==ZIP_OK)
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  ఒకవేళ (err==ZIP_OK)
  {
    ఒకవేళ(zi->ci.zip64)
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    లేకపోతే
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  ఒకవేళ (err==ZIP_OK)
  {
    ఒకవేళ(zi->ci.zip64)
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    లేకపోతే
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)కొలత_filename,2);

  ఒకవేళ(zi->ci.zip64)
  {
    కొలత_extrafield += 20;
  }

  ఒకవేళ (err==ZIP_OK)
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)కొలత_extrafield,2);

  ఒకవేళ ((err==ZIP_OK) && (కొలత_filename > 0))
  {
    ఒకవేళ (ZWRITE64(zi->z_filefunc,zi->filestream,filename,కొలత_filename)!=కొలత_filename)
      err = ZIP_ERRNO;
  }

  ఒకవేళ ((err==ZIP_OK) && (కొలత_extrafield_local > 0))
  {
    ఒకవేళ (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, కొలత_extrafield_local) != కొలత_extrafield_local)
      err = ZIP_ERRNO;
  }


  ఒకవేళ ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      పొట్టి శీర్షిక_ID = 1;
      పొట్టి దత్తాంశం_కొలత_2 = 16;
      ZPOS64_T Compressed_కొలత = 0;
      ZPOS64_T Uncompressed_కొలత = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)శీర్షిక_ID,2);
      err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)దత్తాంశం_కొలత_2,2);

      err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_కొలత,8);
      err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_కొలత,8);
  }

  తిరిగి err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local పూర్ణాంకం utf8len(గుర్తులేని అక్షరం స్థిరరాశి *str, size_t len) {
    తిరిగి
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
local పూర్ణాంకం isutf8(అక్షరం స్థిరరాశి *str, size_t len) {
    పూర్ణాంకం utf8 = 0;
    అయితే (len) {
        పూర్ణాంకం సంకేతం_2 = utf8len((గుర్తులేని అక్షరం స్థిరరాశి *)str, len);
        ఒకవేళ (సంకేతం_2 < 0)
            తిరిగి 0;
        ఒకవేళ (సంకేతం_2 > 1)
            utf8 = 1;
        str += సంకేతం_2;
        len -= (గుర్తులేని)సంకేతం_2;
    }
    తిరిగి utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip4_64(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                           స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                           స్థిరరాశి శూన్య* extrafield_global, uInt కొలత_extrafield_global,
                                           స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level, పూర్ణాంకం raw,
                                           పూర్ణాంకం కిటికీ_బిట్లు,పూర్ణాంకం memLevel, పూర్ణాంకం strategy,
                                           స్థిరరాశి అక్షరం* password, uLong చక్రీయ_తనిఖీ_For_Crypting,
                                           uLong సంస్కరణ_Made_By_2, uLong flag_పునాది, పూర్ణాంకం zip64) {
    zip64_అంతర్గత* zi;
    uInt కొలత_filename;
    uInt కొలత_comment;
    uInt i;
    పూర్ణాంకం err = ZIP_OK;

#    ifdef NOCRYPT
    ఒకవేళ (password != ఖాళీ)
        తిరిగి ZIP_PARAMERROR;
#    endif

    ఒకవేళ (file == ఖాళీ)
        తిరిగి ZIP_PARAMERROR;

#నిర్వచితమైతే HAVE_BZIP2
    ఒకవేళ ((పద్ధతి!=0) && (పద్ధతి!=Z_DEFLATED) && (పద్ధతి!=Z_BZIP2ED))
      తిరిగి ZIP_PARAMERROR;
#లేకపోతే
    ఒకవేళ ((పద్ధతి!=0) && (పద్ధతి!=Z_DEFLATED))
      తిరిగి ZIP_PARAMERROR;
#అయితే_ముగింపు

    /* The filename and comment length must fit in 16 bits. */
    ఒకవేళ ((filename!=ఖాళీ) && (అక్షరశ్రేణి_పొడవు(filename)>0xffff))
        తిరిగి ZIP_PARAMERROR;
    ఒకవేళ ((comment!=ఖాళీ) && (అక్షరశ్రేణి_పొడవు(comment)>0xffff))
        తిరిగి ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    ఒకవేళ ((కొలత_extrafield_local>0xffff) || (కొలత_extrafield_global>0xffff))
        తిరిగి ZIP_PARAMERROR;

    zi = (zip64_అంతర్గత*)file;

    ఒకవేళ (zi->లోపల_opened_file_inzip == 1)
    {
        err = zip_Close_File_లోపల_Zip (file);
        ఒకవేళ (err != ZIP_OK)
            తిరిగి err;
    }

    ఒకవేళ (filename==ఖాళీ)
        filename="-";

    ఒకవేళ (comment==ఖాళీ)
        కొలత_comment = 0;
    లేకపోతే
        కొలత_comment = (uInt)అక్షరశ్రేణి_పొడవు(comment);

    కొలత_filename = (uInt)అక్షరశ్రేణి_పొడవు(filename);

    ఒకవేళ (zipfi == ఖాళీ)
        zi->ci.dosDate = 0;
    లేకపోతే
    {
        ఒకవేళ (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        లేకపోతే
          zi->ci.dosDate = zip64local_Tmz_Date_గమ్యం_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_పునాది;
    ఒకవేళ ((level==8) || (level==9))
      zi->ci.flag |= 2;
    ఒకవేళ (level==2)
      zi->ci.flag |= 4;
    ఒకవేళ (level==1)
      zi->ci.flag |= 6;
    ఒకవేళ (password != ఖాళీ)
      zi->ci.flag |= 1;
    ఒకవేళ (isutf8(filename, కొలత_filename) &&
        (కొలత_comment == 0 || isutf8(comment, కొలత_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.చక్రీయ_తనిఖీ_32_2 = 0;
    zi->ci.పద్ధతి = పద్ధతి;
    zi->ci.encrypt = 0;
    zi->ci.ప్రవాహం_initialised = 0;
    zi->ci.pos_లోపల_buffered_దత్తాంశం = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_శీర్షిక = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.కొలత_centralheader = SIZECENTRALHEADER + కొలత_filename + కొలత_extrafield_global + కొలత_comment;
    zi->ci.కొలత_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_శీర్షిక = (అక్షరం*)ALLOC((uInt)zi->ci.కొలత_centralheader + zi->ci.కొలత_central_Extra_Free);

    zi->ci.కొలత_central_Extra = కొలత_extrafield_global;
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+4,(uLong)సంస్కరణ_Made_By_2,2);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+6,(uLong)20,2);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+8,(uLong)zi->ci.flag,2);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+10,(uLong)zi->ci.పద్ధతి,2);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+16,(uLong)0,4); /*crc*/
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+20,(uLong)0,4); /*compr size*/
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+28,(uLong)కొలత_filename,2);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+30,(uLong)కొలత_extrafield_global,2);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+32,(uLong)కొలత_comment,2);
    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+34,(uLong)0,2); /*disk nm start*/

    ఒకవేళ (zipfi==ఖాళీ)
        zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+36,(uLong)0,2);
    లేకపోతే
        zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+36,(uLong)zipfi->అంతర్గత_fa,2);

    ఒకవేళ (zipfi==ఖాళీ)
        zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+38,(uLong)0,4);
    లేకపోతే
        zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+38,(uLong)zipfi->external_fa,4);

    ఒకవేళ(zi->ci.pos_local_శీర్షిక >= 0xffffffff)
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+42,(uLong)0xffffffff,4);
    లేకపోతే
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+42,(uLong)zi->ci.pos_local_శీర్షిక - zi->కూడు_position_when_writing_స్థానభ్రంశం,4);

    కొరకు (i=0;i<కొలత_filename;i++)
        *(zi->ci.central_శీర్షిక+SIZECENTRALHEADER+i) = *(filename+i);

    కొరకు (i=0;i<కొలత_extrafield_global;i++)
        *(zi->ci.central_శీర్షిక+SIZECENTRALHEADER+కొలత_filename+i) =
              *(((స్థిరరాశి అక్షరం*)extrafield_global)+i);

    కొరకు (i=0;i<కొలత_comment;i++)
        *(zi->ci.central_శీర్షిక+SIZECENTRALHEADER+కొలత_filename+
              కొలత_extrafield_global+i) = *(comment+i);
    ఒకవేళ (zi->ci.central_శీర్షిక == ఖాళీ)
        తిరిగి ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.మొత్తం_Compressed_దత్తాంశం = 0;
    zi->ci.మొత్తం_Uncompressed_దత్తాంశం = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_శీర్షిక(zi, filename, కొలత_extrafield_local, extrafield_local);

#నిర్వచితమైతే HAVE_BZIP2
    zi->ci.bstream.avail_లోపల = (uInt)0;
    zi->ci.bstream.avail_బహిర్గతం = (uInt)Z_BUFSIZE;
    zi->ci.bstream.తదుపరి_బహిర్గతం = (అక్షరం*)zi->ci.buffered_దత్తాంశం;
    zi->ci.bstream.మొత్తం_లోపల_hi32 = 0;
    zi->ci.bstream.మొత్తం_లోపల_lo32 = 0;
    zi->ci.bstream.మొత్తం_బహిర్గతం_hi32 = 0;
    zi->ci.bstream.మొత్తం_బహిర్గతం_lo32 = 0;
#అయితే_ముగింపు

    zi->ci.ప్రవాహం.avail_లోపల = (uInt)0;
    zi->ci.ప్రవాహం.avail_బహిర్గతం = (uInt)Z_BUFSIZE;
    zi->ci.ప్రవాహం.తదుపరి_బహిర్గతం = zi->ci.buffered_దత్తాంశం;
    zi->ci.ప్రవాహం.మొత్తం_లోపల = 0;
    zi->ci.ప్రవాహం.మొత్తం_బహిర్గతం = 0;
    zi->ci.ప్రవాహం.దత్తాంశం_type = Z_ద్విమాన;

#నిర్వచితమైతే HAVE_BZIP2
    ఒకవేళ ((err==ZIP_OK) && (zi->ci.పద్ధతి == Z_DEFLATED || zi->ci.పద్ధతి == Z_BZIP2ED) && (!zi->ci.raw))
#లేకపోతే
    ఒకవేళ ((err==ZIP_OK) && (zi->ci.పద్ధతి == Z_DEFLATED) && (!zi->ci.raw))
#అయితే_ముగింపు
    {
        ఒకవేళ(zi->ci.పద్ధతి == Z_DEFLATED)
        {
          zi->ci.ప్రవాహం.zalloc = (alloc_func)0;
          zi->ci.ప్రవాహం.zfree = (free_func)0;
          zi->ci.ప్రవాహం.opaque = (voidpf)0;

          ఒకవేళ (కిటికీ_బిట్లు>0)
              కిటికీ_బిట్లు = -కిటికీ_బిట్లు;

          err = deflateInit2(&zi->ci.ప్రవాహం, level, Z_DEFLATED, కిటికీ_బిట్లు, memLevel, strategy);

          ఒకవేళ (err==Z_OK)
              zi->ci.ప్రవాహం_initialised = Z_DEFLATED;
        }
        లేకపోతే ఒకవేళ(zi->ci.పద్ధతి == Z_BZIP2ED)
        {
#నిర్వచితమైతే HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          ఒకవేళ(err == BZ_OK)
            zi->ci.ప్రవాహం_initialised = Z_BZIP2ED;
#అయితే_ముగింపు
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_శీర్షిక_కొలత = 0;
    ఒకవేళ ((err==Z_OK) && (password != ఖాళీ))
    {
        గుర్తులేని అక్షరం buf_తల[RAND_తల_LEN];
        గుర్తులేని పూర్ణాంకం కొలత_తల;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_చక్రీయ_తనిఖీ_పట్టిక();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        కొలత_తల=crypthead(password,buf_తల,RAND_తల_LEN,zi->ci.కీలకాలు,zi->ci.pcrc_32_tab,చక్రీయ_తనిఖీ_For_Crypting);
        zi->ci.crypt_శీర్షిక_కొలత = కొలత_తల;

        ఒకవేళ (ZWRITE64(zi->z_filefunc,zi->filestream,buf_తల,కొలత_తల) != కొలత_తల)
                err = ZIP_ERRNO;
    }
#    endif

    ఒకవేళ (err==Z_OK)
        zi->లోపల_opened_file_inzip = 1;
    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip4(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                        స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                        స్థిరరాశి శూన్య* extrafield_global, uInt కొలత_extrafield_global,
                                        స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level, పూర్ణాంకం raw,
                                        పూర్ణాంకం కిటికీ_బిట్లు,పూర్ణాంకం memLevel, పూర్ణాంకం strategy,
                                        స్థిరరాశి అక్షరం* password, uLong చక్రీయ_తనిఖీ_For_Crypting,
                                        uLong సంస్కరణ_Made_By_2, uLong flag_పునాది) {
    తిరిగి zip_Open_New_File_లోపల_Zip4_64(file, filename, zipfi,
                                   extrafield_local, కొలత_extrafield_local,
                                   extrafield_global, కొలత_extrafield_global,
                                   comment, పద్ధతి, level, raw,
                                   కిటికీ_బిట్లు, memLevel, strategy,
                                   password, చక్రీయ_తనిఖీ_For_Crypting, సంస్కరణ_Made_By_2, flag_పునాది, 0);
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip3(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                        స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                        స్థిరరాశి శూన్య* extrafield_global, uInt కొలత_extrafield_global,
                                        స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level, పూర్ణాంకం raw,
                                        పూర్ణాంకం కిటికీ_బిట్లు,పూర్ణాంకం memLevel, పూర్ణాంకం strategy,
                                        స్థిరరాశి అక్షరం* password, uLong చక్రీయ_తనిఖీ_For_Crypting) {
    తిరిగి zip_Open_New_File_లోపల_Zip4_64(file, filename, zipfi,
                                   extrafield_local, కొలత_extrafield_local,
                                   extrafield_global, కొలత_extrafield_global,
                                   comment, పద్ధతి, level, raw,
                                   కిటికీ_బిట్లు, memLevel, strategy,
                                   password, చక్రీయ_తనిఖీ_For_Crypting, VERSIONMADEBY, 0, 0);
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip3_64(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                         స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                         స్థిరరాశి శూన్య* extrafield_global, uInt కొలత_extrafield_global,
                                         స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level, పూర్ణాంకం raw,
                                         పూర్ణాంకం కిటికీ_బిట్లు,పూర్ణాంకం memLevel, పూర్ణాంకం strategy,
                                         స్థిరరాశి అక్షరం* password, uLong చక్రీయ_తనిఖీ_For_Crypting, పూర్ణాంకం zip64) {
    తిరిగి zip_Open_New_File_లోపల_Zip4_64(file, filename, zipfi,
                                   extrafield_local, కొలత_extrafield_local,
                                   extrafield_global, కొలత_extrafield_global,
                                   comment, పద్ధతి, level, raw,
                                   కిటికీ_బిట్లు, memLevel, strategy,
                                   password, చక్రీయ_తనిఖీ_For_Crypting, VERSIONMADEBY, 0, zip64);
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip2(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                        స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                        స్థిరరాశి శూన్య* extrafield_global, uInt కొలత_extrafield_global,
                                        స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level, పూర్ణాంకం raw) {
    తిరిగి zip_Open_New_File_లోపల_Zip4_64(file, filename, zipfi,
                                   extrafield_local, కొలత_extrafield_local,
                                   extrafield_global, కొలత_extrafield_global,
                                   comment, పద్ధతి, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ఖాళీ, 0, VERSIONMADEBY, 0, 0);
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip2_64(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                           స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                           స్థిరరాశి శూన్య* extrafield_global, uInt కొలత_extrafield_global,
                                           స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level, పూర్ణాంకం raw, పూర్ణాంకం zip64) {
    తిరిగి zip_Open_New_File_లోపల_Zip4_64(file, filename, zipfi,
                                   extrafield_local, కొలత_extrafield_local,
                                   extrafield_global, కొలత_extrafield_global,
                                   comment, పద్ధతి, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ఖాళీ, 0, VERSIONMADEBY, 0, zip64);
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip64(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                         స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                         స్థిరరాశి శూన్య*extrafield_global, uInt కొలత_extrafield_global,
                                         స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level, పూర్ణాంకం zip64) {
    తిరిగి zip_Open_New_File_లోపల_Zip4_64(file, filename, zipfi,
                                   extrafield_local, కొలత_extrafield_local,
                                   extrafield_global, కొలత_extrafield_global,
                                   comment, పద్ధతి, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ఖాళీ, 0, VERSIONMADEBY, 0, zip64);
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip(zipFile file, స్థిరరాశి అక్షరం* filename, స్థిరరాశి zip_fileinfo* zipfi,
                                       స్థిరరాశి శూన్య* extrafield_local, uInt కొలత_extrafield_local,
                                       స్థిరరాశి శూన్య*extrafield_global, uInt కొలత_extrafield_global,
                                       స్థిరరాశి అక్షరం* comment, పూర్ణాంకం పద్ధతి, పూర్ణాంకం level) {
    తిరిగి zip_Open_New_File_లోపల_Zip4_64(file, filename, zipfi,
                                   extrafield_local, కొలత_extrafield_local,
                                   extrafield_global, కొలత_extrafield_global,
                                   comment, పద్ధతి, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   ఖాళీ, 0, VERSIONMADEBY, 0, 0);
}

local పూర్ణాంకం zip64_Flush_Write_బఫరు(zip64_అంతర్గత* zi) {
    పూర్ణాంకం err=ZIP_OK;

    ఒకవేళ (zi->ci.encrypt != 0)
    {
#నిర్వచితం_కాకపోతే NOCRYPT
        uInt i;
        పూర్ణాంకం t;
        కొరకు (i=0;i<zi->ci.pos_లోపల_buffered_దత్తాంశం;i++)
            zi->ci.buffered_దత్తాంశం[i] = zencode(zi->ci.కీలకాలు, zi->ci.pcrc_32_tab, zi->ci.buffered_దత్తాంశం[i],t);
#అయితే_ముగింపు
    }

    ఒకవేళ (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_దత్తాంశం,zi->ci.pos_లోపల_buffered_దత్తాంశం) != zi->ci.pos_లోపల_buffered_దత్తాంశం)
      err = ZIP_ERRNO;

    zi->ci.మొత్తం_Compressed_దత్తాంశం += zi->ci.pos_లోపల_buffered_దత్తాంశం;

#నిర్వచితమైతే HAVE_BZIP2
    ఒకవేళ(zi->ci.పద్ధతి == Z_BZIP2ED)
    {
      zi->ci.మొత్తం_Uncompressed_దత్తాంశం += zi->ci.bstream.మొత్తం_లోపల_lo32;
      zi->ci.bstream.మొత్తం_లోపల_lo32 = 0;
      zi->ci.bstream.మొత్తం_లోపల_hi32 = 0;
    }
    లేకపోతే
#అయితే_ముగింపు
    {
      zi->ci.మొత్తం_Uncompressed_దత్తాంశం += zi->ci.ప్రవాహం.మొత్తం_లోపల;
      zi->ci.ప్రవాహం.మొత్తం_లోపల = 0;
    }


    zi->ci.pos_లోపల_buffered_దత్తాంశం = 0;

    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Write_లోపల_File_లోపల_Zip(zipFile file, స్థిరరాశి శూన్య* buf, గుర్తులేని పూర్ణాంకం len) {
    zip64_అంతర్గత* zi;
    పూర్ణాంకం err=ZIP_OK;

    ఒకవేళ (file == ఖాళీ)
        తిరిగి ZIP_PARAMERROR;
    zi = (zip64_అంతర్గత*)file;

    ఒకవేళ (zi->లోపల_opened_file_inzip == 0)
        తిరిగి ZIP_PARAMERROR;

    zi->ci.చక్రీయ_తనిఖీ_32_2 = చక్రీయ_తనిఖీ_32_2(zi->ci.చక్రీయ_తనిఖీ_32_2,buf,(uInt)len);

#నిర్వచితమైతే HAVE_BZIP2
    ఒకవేళ(zi->ci.పద్ధతి == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.తదుపరి_లోపల = (శూన్య*)buf;
      zi->ci.bstream.avail_లోపల = len;
      err = BZ_నిరంతరం_OK;

      అయితే ((err==BZ_నిరంతరం_OK) && (zi->ci.bstream.avail_లోపల>0))
      {
        ఒకవేళ (zi->ci.bstream.avail_బహిర్గతం == 0)
        {
          ఒకవేళ (zip64_Flush_Write_బఫరు(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_బహిర్గతం = (uInt)Z_BUFSIZE;
          zi->ci.bstream.తదుపరి_బహిర్గతం = (అక్షరం*)zi->ci.buffered_దత్తాంశం;
        }


        ఒకవేళ(err != BZ_నిరంతరం_OK)
          ఆపు;

        ఒకవేళ ((zi->ci.పద్ధతి == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_మొత్తం_బహిర్గతం_Before_lo = zi->ci.bstream.మొత్తం_బహిర్గతం_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_నిరంతరం);

          zi->ci.pos_లోపల_buffered_దత్తాంశం += (uInt)(zi->ci.bstream.మొత్తం_బహిర్గతం_lo32 - u_మొత్తం_బహిర్గతం_Before_lo) ;
        }
      }

      ఒకవేళ(err == BZ_నిరంతరం_OK)
        err = ZIP_OK;
    }
    లేకపోతే
#అయితే_ముగింపు
    {
      zi->ci.ప్రవాహం.తదుపరి_లోపల = buf;
      zi->ci.ప్రవాహం.avail_లోపల = len;

      అయితే ((err==ZIP_OK) && (zi->ci.ప్రవాహం.avail_లోపల>0))
      {
          ఒకవేళ (zi->ci.ప్రవాహం.avail_బహిర్గతం == 0)
          {
              ఒకవేళ (zip64_Flush_Write_బఫరు(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.ప్రవాహం.avail_బహిర్గతం = (uInt)Z_BUFSIZE;
              zi->ci.ప్రవాహం.తదుపరి_బహిర్గతం = zi->ci.buffered_దత్తాంశం;
          }


          ఒకవేళ(err != ZIP_OK)
              ఆపు;

          ఒకవేళ ((zi->ci.పద్ధతి == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_మొత్తం_బహిర్గతం_Before = zi->ci.ప్రవాహం.మొత్తం_బహిర్గతం;
              err=deflate(&zi->ci.ప్రవాహం,  Z_NO_FLUSH);

              zi->ci.pos_లోపల_buffered_దత్తాంశం += (uInt)(zi->ci.ప్రవాహం.మొత్తం_బహిర్గతం - u_మొత్తం_బహిర్గతం_Before) ;
          }
          లేకపోతే
          {
              uInt copy_this,i;
              ఒకవేళ (zi->ci.ప్రవాహం.avail_లోపల < zi->ci.ప్రవాహం.avail_బహిర్గతం)
                  copy_this = zi->ci.ప్రవాహం.avail_లోపల;
              లేకపోతే
                  copy_this = zi->ci.ప్రవాహం.avail_బహిర్గతం;

              కొరకు (i = 0; i < copy_this; i++)
                  *(((అక్షరం*)zi->ci.ప్రవాహం.తదుపరి_బహిర్గతం)+i) =
                      *(((స్థిరరాశి అక్షరం*)zi->ci.ప్రవాహం.తదుపరి_లోపల)+i);
              {
                  zi->ci.ప్రవాహం.avail_లోపల -= copy_this;
                  zi->ci.ప్రవాహం.avail_బహిర్గతం-= copy_this;
                  zi->ci.ప్రవాహం.తదుపరి_లోపల+= copy_this;
                  zi->ci.ప్రవాహం.తదుపరి_బహిర్గతం+= copy_this;
                  zi->ci.ప్రవాహం.మొత్తం_లోపల+= copy_this;
                  zi->ci.ప్రవాహం.మొత్తం_బహిర్గతం+= copy_this;
                  zi->ci.pos_లోపల_buffered_దత్తాంశం += copy_this;
              }
          }
      }/* while(...) */
    }

    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Close_File_లోపల_Zip_Raw(zipFile file, uLong uncompressed_కొలత, uLong చక్రీయ_తనిఖీ_32_2) {
    తిరిగి zip_Close_File_లోపల_Zip_Raw64 (file, uncompressed_కొలత, చక్రీయ_తనిఖీ_32_2);
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Close_File_లోపల_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_కొలత, uLong చక్రీయ_తనిఖీ_32_2) {
    zip64_అంతర్గత* zi;
    ZPOS64_T compressed_కొలత;
    uLong invalid_విలువ = 0xffffffff;
    గుర్తులేని datasize = 0;
    పూర్ణాంకం err=ZIP_OK;

    ఒకవేళ (file == ఖాళీ)
        తిరిగి ZIP_PARAMERROR;
    zi = (zip64_అంతర్గత*)file;

    ఒకవేళ (zi->లోపల_opened_file_inzip == 0)
        తిరిగి ZIP_PARAMERROR;
    zi->ci.ప్రవాహం.avail_లోపల = 0;

    ఒకవేళ ((zi->ci.పద్ధతి == Z_DEFLATED) && (!zi->ci.raw))
                {
                        అయితే (err==ZIP_OK)
                        {
                                uLong u_మొత్తం_బహిర్గతం_Before;
                                ఒకవేళ (zi->ci.ప్రవాహం.avail_బహిర్గతం == 0)
                                {
                                        ఒకవేళ (zip64_Flush_Write_బఫరు(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.ప్రవాహం.avail_బహిర్గతం = (uInt)Z_BUFSIZE;
                                        zi->ci.ప్రవాహం.తదుపరి_బహిర్గతం = zi->ci.buffered_దత్తాంశం;
                                }
                                u_మొత్తం_బహిర్గతం_Before = zi->ci.ప్రవాహం.మొత్తం_బహిర్గతం;
                                err=deflate(&zi->ci.ప్రవాహం,  Z_ముగింపు);
                                zi->ci.pos_లోపల_buffered_దత్తాంశం += (uInt)(zi->ci.ప్రవాహం.మొత్తం_బహిర్గతం - u_మొత్తం_బహిర్గతం_Before) ;
                        }
                }
    లేకపోతే ఒకవేళ ((zi->ci.పద్ధతి == Z_BZIP2ED) && (!zi->ci.raw))
    {
#నిర్వచితమైతే HAVE_BZIP2
      err = BZ_ముగింపు_OK;
      అయితే (err==BZ_ముగింపు_OK)
      {
        uLong u_మొత్తం_బహిర్గతం_Before;
        ఒకవేళ (zi->ci.bstream.avail_బహిర్గతం == 0)
        {
          ఒకవేళ (zip64_Flush_Write_బఫరు(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_బహిర్గతం = (uInt)Z_BUFSIZE;
          zi->ci.bstream.తదుపరి_బహిర్గతం = (అక్షరం*)zi->ci.buffered_దత్తాంశం;
        }
        u_మొత్తం_బహిర్గతం_Before = zi->ci.bstream.మొత్తం_బహిర్గతం_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_ముగింపు);
        ఒకవేళ(err == BZ_ప్రవాహం_చివర)
          err = Z_ప్రవాహం_చివర;

        zi->ci.pos_లోపల_buffered_దత్తాంశం += (uInt)(zi->ci.bstream.మొత్తం_బహిర్గతం_lo32 - u_మొత్తం_బహిర్గతం_Before);
      }

      ఒకవేళ(err == BZ_ముగింపు_OK)
        err = ZIP_OK;
#అయితే_ముగింపు
    }

    ఒకవేళ (err==Z_ప్రవాహం_చివర)
        err=ZIP_OK; /* this is normal */

    ఒకవేళ ((zi->ci.pos_లోపల_buffered_దత్తాంశం>0) && (err==ZIP_OK))
                {
        ఒకవేళ (zip64_Flush_Write_బఫరు(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    ఒకవేళ ((zi->ci.పద్ధతి == Z_DEFLATED) && (!zi->ci.raw))
    {
        పూర్ణాంకం tmp_err = deflate_చివర(&zi->ci.ప్రవాహం);
        ఒకవేళ (err == ZIP_OK)
            err = tmp_err;
        zi->ci.ప్రవాహం_initialised = 0;
    }
#నిర్వచితమైతే HAVE_BZIP2
    లేకపోతే ఒకవేళ((zi->ci.పద్ధతి == Z_BZIP2ED) && (!zi->ci.raw))
    {
      పూర్ణాంకం tmperr = BZ2_bz_Compress_చివర(&zi->ci.bstream);
                        ఒకవేళ (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.ప్రవాహం_initialised = 0;
    }
#అయితే_ముగింపు

    ఒకవేళ (!zi->ci.raw)
    {
        చక్రీయ_తనిఖీ_32_2 = (uLong)zi->ci.చక్రీయ_తనిఖీ_32_2;
        uncompressed_కొలత = zi->ci.మొత్తం_Uncompressed_దత్తాంశం;
    }
    compressed_కొలత = zi->ci.మొత్తం_Compressed_దత్తాంశం;

#    ifndef NOCRYPT
    compressed_కొలత += zi->ci.crypt_శీర్షిక_కొలత;
#    endif

    /* update Current Item crc and sizes, */
    ఒకవేళ(compressed_కొలత >= 0xffffffff || uncompressed_కొలత >= 0xffffffff || zi->ci.pos_local_శీర్షిక >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+6,(uLong)45,2);

    }

    zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+16,చక్రీయ_తనిఖీ_32_2,4); /*crc*/


    ఒకవేళ(compressed_కొలత >= 0xffffffff)
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+20, invalid_విలువ,4); /*compr size*/
    లేకపోతే
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+20, compressed_కొలత,4); /*compr size*/

    /* set internal file attributes field */
    ఒకవేళ (zi->ci.ప్రవాహం.దత్తాంశం_type == Z_ASCII)
        zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+36,(uLong)Z_ASCII,2);

    ఒకవేళ(uncompressed_కొలత >= 0xffffffff)
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+24, invalid_విలువ,4); /*uncompr size*/
    లేకపోతే
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+24, uncompressed_కొలత,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    ఒకవేళ(uncompressed_కొలత >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    ఒకవేళ(compressed_కొలత >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    ఒకవేళ(zi->ci.pos_local_శీర్షిక >= 0xffffffff)
      datasize += 8;

    ఒకవేళ(datasize > 0)
    {
      అక్షరం* p = ఖాళీ;

      ఒకవేళ((uLong)(datasize + 4) > zi->ci.కొలత_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        తిరిగి ZIP_BADZIPFILE;
      }

      p = zi->ci.central_శీర్షిక + zi->ci.కొలత_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_విలువ_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_విలువ_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      ఒకవేళ(uncompressed_కొలత >= 0xffffffff)
      {
        zip64local_put_విలువ_inmemory(p, uncompressed_కొలత, 8);
        p += 8;
      }

      ఒకవేళ(compressed_కొలత >= 0xffffffff)
      {
        zip64local_put_విలువ_inmemory(p, compressed_కొలత, 8);
        p += 8;
      }

      ఒకవేళ(zi->ci.pos_local_శీర్షిక >= 0xffffffff)
      {
        zip64local_put_విలువ_inmemory(p, zi->ci.pos_local_శీర్షిక, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.కొలత_central_Extra_Free -= datasize + 4;
      zi->ci.కొలత_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.కొలత_central_Extra += datasize + 4;
      zip64local_put_విలువ_inmemory(zi->ci.central_శీర్షిక+30,(uLong)zi->ci.కొలత_central_Extra,2);
    }

    ఒకవేళ (err==ZIP_OK)
        err = కూడు_దత్తాంశం_లోపల_datablock(&zi->central_dir, zi->ci.central_శీర్షిక, (uLong)zi->ci.కొలత_centralheader);

    విడుదల(zi->ci.central_శీర్షిక);

    ఒకవేళ (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        ఒకవేళ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_శీర్షిక + 14,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
            err = ZIP_ERRNO;

        ఒకవేళ (err==ZIP_OK)
            err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,చక్రీయ_తనిఖీ_32_2,4); /* crc 32, unknown */

        ఒకవేళ(uncompressed_కొలత >= 0xffffffff || compressed_కొలత >= 0xffffffff )
        {
          ఒకవేళ(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            ఒకవేళ (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
              err = ZIP_ERRNO;

            ఒకవేళ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, uncompressed_కొలత, 8);

            ఒకవేళ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, compressed_కొలత, 8);
          }
          లేకపోతే
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        లేకపోతే
        {
          ఒకవేళ (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,compressed_కొలత,4);

          ఒకవేళ (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,uncompressed_కొలత,4);
        }

        ఒకవేళ (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
            err = ZIP_ERRNO;
    }

    zi->సంఖ్య_నమోదు ++;
    zi->లోపల_opened_file_inzip = 0;

    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Close_File_లోపల_Zip(zipFile file) {
    తిరిగి zip_Close_File_లోపల_Zip_Raw (file,0,0);
}

local పూర్ణాంకం Write_Zip64_చివర_యొక్క_Central_Directory_Locator(zip64_అంతర్గత* zi, ZPOS64_T zip64eocd_pos_inzip) {
  పూర్ణాంకం err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->కూడు_position_when_writing_స్థానభ్రంశం;

  err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    ఒకవేళ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    ఒకవేళ (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    ఒకవేళ (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    తిరిగి err;
}

local పూర్ణాంకం Write_Zip64_చివర_యొక్క_Central_Directory_రికార్డు(zip64_అంతర్గత* zi, uLong కొలత_centraldir, ZPOS64_T centraldir_pos_inzip) {
  పూర్ణాంకం err = ZIP_OK;

  uLong Zip64_దత్తాంశం_కొలత = 44;

  err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  ఒకవేళ (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_దత్తాంశం_కొలత,8); /* why ZPOS64_T of this ? */

  ఒకవేళ (err==ZIP_OK) /* version made by */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ఒకవేళ (err==ZIP_OK) /* version needed */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  ఒకవేళ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ఒకవేళ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  ఒకవేళ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, zi->సంఖ్య_నమోదు, 8);

  ఒకవేళ (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_విలువ(&zi->z_filefunc, zi->filestream, zi->సంఖ్య_నమోదు, 8);

  ఒకవేళ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(ZPOS64_T)కొలత_centraldir,8);

  ఒకవేళ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->కూడు_position_when_writing_స్థానభ్రంశం;
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  తిరిగి err;
}

local పూర్ణాంకం Write_చివర_యొక్క_Central_Directory_రికార్డు(zip64_అంతర్గత* zi, uLong కొలత_centraldir, ZPOS64_T centraldir_pos_inzip) {
  పూర్ణాంకం err = ZIP_OK;

  /*signature*/
  err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  ఒకవేళ (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ఒకవేళ (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  ఒకవేళ (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      ఒకవేళ(zi->సంఖ్య_నమోదు >= 0xFFFF)
        err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      లేకపోతే
        err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)zi->సంఖ్య_నమోదు,2);
    }
  }

  ఒకవేళ (err==ZIP_OK) /* total number of entries in the central dir */
  {
    ఒకవేళ(zi->సంఖ్య_నమోదు >= 0xFFFF)
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    లేకపోతే
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)zi->సంఖ్య_నమోదు,2);
  }

  ఒకవేళ (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)కొలత_centraldir,4);

  ఒకవేళ (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->కూడు_position_when_writing_స్థానభ్రంశం;
    ఒకవేళ(pos >= 0xffffffff)
    {
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    లేకపోతే
      err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->కూడు_position_when_writing_స్థానభ్రంశం),4);
  }

   తిరిగి err;
}

local పూర్ణాంకం Write_GlobalComment(zip64_అంతర్గత* zi, స్థిరరాశి అక్షరం* global_comment) {
  పూర్ణాంకం err = ZIP_OK;
  uInt కొలత_global_comment = 0;

  ఒకవేళ(global_comment != ఖాళీ)
    కొలత_global_comment = (uInt)అక్షరశ్రేణి_పొడవు(global_comment);

  err = zip64local_put_విలువ(&zi->z_filefunc,zi->filestream,(uLong)కొలత_global_comment,2);

  ఒకవేళ (err == ZIP_OK && కొలత_global_comment > 0)
  {
    ఒకవేళ (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, కొలత_global_comment) != కొలత_global_comment)
      err = ZIP_ERRNO;
  }
  తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT zipClose(zipFile file, స్థిరరాశి అక్షరం* global_comment) {
    zip64_అంతర్గత* zi;
    పూర్ణాంకం err = 0;
    uLong కొలత_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    ఒకవేళ (file == ఖాళీ)
        తిరిగి ZIP_PARAMERROR;

    zi = (zip64_అంతర్గత*)file;

    ఒకవేళ (zi->లోపల_opened_file_inzip == 1)
    {
        err = zip_Close_File_లోపల_Zip (file);
    }

#నిర్వచితం_కాకపోతే NO_ADDFILEINEXISTINGZIP
    ఒకవేళ (global_comment==ఖాళీ)
        global_comment = zi->globalcomment;
#అయితే_ముగింపు

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    ఒకవేళ (err==ZIP_OK)
    {
        linkedlist_datablock_అంతర్గత* ldi = zi->central_dir.మొదటి_ఖండం;
        అయితే (ldi!=ఖాళీ)
        {
            ఒకవేళ ((err==ZIP_OK) && (ldi->filled_లోపల_this_ఖండం>0))
            {
                ఒకవేళ (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->దత్తాంశం, ldi->filled_లోపల_this_ఖండం) != ldi->filled_లోపల_this_ఖండం)
                    err = ZIP_ERRNO;
            }

            కొలత_centraldir += ldi->filled_లోపల_this_ఖండం;
            ldi = ldi->తదుపరి_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    సమితి_చివర(&zi->సమితి);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->కూడు_position_when_writing_స్థానభ్రంశం;
    ఒకవేళ(pos >= 0xffffffff || zi->సంఖ్య_నమోదు >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_చివర_యొక్క_Central_Directory_రికార్డు(zi, కొలత_centraldir, centraldir_pos_inzip);

      Write_Zip64_చివర_యొక్క_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    ఒకవేళ (err==ZIP_OK)
      err = Write_చివర_యొక్క_Central_Directory_రికార్డు(zi, కొలత_centraldir, centraldir_pos_inzip);

    ఒకవేళ(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    ఒకవేళ (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        ఒకవేళ (err == ZIP_OK)
            err = ZIP_ERRNO;

#నిర్వచితం_కాకపోతే NO_ADDFILEINEXISTINGZIP
    విడుదల(zi->globalcomment);
#అయితే_ముగింపు
    విడుదల(zi);

    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT zip_Remove_Extra_Info_ఖండం(అక్షరం* p_దత్తాంశం, పూర్ణాంకం* దత్తాంశం_Len, పొట్టి s_శీర్షిక) {
  అక్షరం* p = p_దత్తాంశం;
  పూర్ణాంకం కొలత = 0;
  అక్షరం* p_New_శీర్షిక;
  అక్షరం* pTmp;
  పొట్టి శీర్షిక;
  పొట్టి దత్తాంశం_కొలత;

  పూర్ణాంకం retVal = ZIP_OK;

  ఒకవేళ(p_దత్తాంశం == ఖాళీ || దత్తాంశం_Len == ఖాళీ || *దత్తాంశం_Len < 4)
    తిరిగి ZIP_PARAMERROR;

  p_New_శీర్షిక = (అక్షరం*)ALLOC((గుర్తులేని)*దత్తాంశం_Len);
  pTmp = p_New_శీర్షిక;

  అయితే(p < (p_దత్తాంశం + *దత్తాంశం_Len))
  {
    శీర్షిక = *(పొట్టి*)p;
    దత్తాంశం_కొలత = *(((పొట్టి*)p)+1);

    ఒకవేళ( శీర్షిక == s_శీర్షిక ) /* Header found. */
    {
      p += దత్తాంశం_కొలత + 4; /* skip it. do not copy to temp buffer */
    }
    లేకపోతే
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      జ్ఞాపకం_నకలు(pTmp, p, దత్తాంశం_కొలత + 4);
      p += దత్తాంశం_కొలత + 4;
      కొలత += దత్తాంశం_కొలత + 4;
    }

  }

  ఒకవేళ(కొలత < *దత్తాంశం_Len)
  {
    /* clean old extra info block. */
    memset(p_దత్తాంశం,0, *దత్తాంశం_Len);

    /* copy the new extra info block over the old */
    ఒకవేళ(కొలత > 0)
      జ్ఞాపకం_నకలు(p_దత్తాంశం, p_New_శీర్షిక, కొలత);

    /* set the new extra info size */
    *దత్తాంశం_Len = కొలత;

    retVal = ZIP_OK;
  }
  లేకపోతే
    retVal = ZIP_ERRNO;

  విడుదల(p_New_శీర్షిక);

  తిరిగి retVal;
}
